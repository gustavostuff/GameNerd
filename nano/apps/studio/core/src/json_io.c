#include "retr01_studio/json_io.h"
#include "retr01_studio/chr_pack.h"
#include "retr01_studio/project.h"
#include "retr01_studio/palette.h"
#include "retr01_studio/metatiles.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char B64_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + c - 'a';
    }
    if (c >= 'A' && c <= 'F') {
        return 10 + c - 'A';
    }
    return -1;
}

static int b64_value(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 26;
    }
    if (c >= '0' && c <= '9') {
        return c - '0' + 52;
    }
    if (c == '+') {
        return 62;
    }
    if (c == '/') {
        return 63;
    }
    if (c == '=') {
        return -2;
    }
    return -1;
}

static char *encode_b64(const uint8_t *in, size_t in_len) {
    size_t out_len = 4u * ((in_len + 2u) / 3u);
    char *out;
    size_t i;
    size_t j = 0;
    if (!in && in_len > 0) {
        return NULL;
    }
    out = (char *)malloc(out_len + 1u);
    if (!out) {
        return NULL;
    }
    for (i = 0; i < in_len; i += 3u) {
        uint32_t v = (uint32_t)in[i] << 16;
        if (i + 1u < in_len) {
            v |= (uint32_t)in[i + 1u] << 8;
        }
        if (i + 2u < in_len) {
            v |= (uint32_t)in[i + 2u];
        }
        out[j++] = B64_TABLE[(v >> 18) & 63u];
        out[j++] = B64_TABLE[(v >> 12) & 63u];
        out[j++] = (i + 1u < in_len) ? B64_TABLE[(v >> 6) & 63u] : '=';
        out[j++] = (i + 2u < in_len) ? B64_TABLE[v & 63u] : '=';
    }
    out[j] = '\0';
    return out;
}

static uint8_t *decode_b64(const char *in, size_t *out_len) {
    size_t o = 0;
    size_t cap;
    uint8_t *out;
    int val = 0;
    int valb = -8;
    size_t i;
    if (!in || !out_len) {
        return NULL;
    }
    cap = strlen(in) * 3u / 4u + 4u;
    out = (uint8_t *)malloc(cap);
    if (!out) {
        return NULL;
    }
    for (i = 0; in[i]; i++) {
        int d = b64_value(in[i]);
        if (d == -1) {
            continue;
        }
        if (d == -2) {
            break;
        }
        val = (val << 6) | d;
        valb += 6;
        if (valb >= 0) {
            out[o++] = (uint8_t)((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    *out_len = o;
    return out;
}

static int decode_hex(const char *hex, uint8_t *out, size_t out_len) {
    size_t i;
    size_t n;
    if (!hex || !out) {
        return -1;
    }
    n = strlen(hex);
    if (n != out_len * 2u) {
        return -1;
    }
    for (i = 0; i < out_len; i++) {
        int hi = hex_nibble(hex[i * 2]);
        int lo = hex_nibble(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0) {
            return -1;
        }
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return 0;
}

static uint8_t *decode_hex_alloc(const char *hex, size_t *out_len) {
    size_t n;
    uint8_t *out;
    size_t i;
    if (!hex || !out_len) {
        return NULL;
    }
    n = strlen(hex);
    if (n == 0 || (n & 1u) != 0) {
        return NULL;
    }
    *out_len = n / 2u;
    out = (uint8_t *)malloc(*out_len);
    if (!out) {
        return NULL;
    }
    for (i = 0; i < *out_len; i++) {
        int hi = hex_nibble(hex[i * 2]);
        int lo = hex_nibble(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0) {
            free(out);
            return NULL;
        }
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return out;
}

static int r01_rle_decode(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len) {
    size_t i = 0;
    size_t o = 0;
    if (!in || !out) {
        return -1;
    }
    while (i < in_len) {
        uint8_t tag = in[i++];
        if (tag & 0x80u) {
            uint8_t val;
            size_t count = tag & 0x7Fu;
            if (i >= in_len || count == 0) {
                return -1;
            }
            val = in[i++];
            if (o + count > out_len) {
                return -1;
            }
            memset(out + o, val, count);
            o += count;
        } else {
            size_t count = tag;
            if (count == 0 || i + count > in_len || o + count > out_len) {
                return -1;
            }
            memcpy(out + o, in + i, count);
            i += count;
            o += count;
        }
    }
    if (o != out_len) {
        return -1;
    }
    return 0;
}

static const char *json_array_end(const char *section_key) {
    const char *p;
    int depth;
    int in_str;
    if (!section_key) {
        return NULL;
    }
    p = strchr(section_key, '[');
    if (!p) {
        return NULL;
    }
    depth = 0;
    in_str = 0;
    for (; *p; p++) {
        if (in_str) {
            if (*p == '\\' && p[1]) {
                p++;
                continue;
            }
            if (*p == '\"') {
                in_str = 0;
            }
            continue;
        }
        if (*p == '\"') {
            in_str = 1;
            continue;
        }
        if (*p == '[') {
            depth++;
        } else if (*p == ']') {
            depth--;
            if (depth == 0) {
                return p;
            }
        }
    }
    return NULL;
}

/* obj_start must point at '{'. Returns matching '}' or NULL. */
static const char *json_object_end(const char *obj_start) {
    const char *p;
    int depth;
    int in_str;
    if (!obj_start || *obj_start != '{') {
        return NULL;
    }
    depth = 0;
    in_str = 0;
    for (p = obj_start; *p; p++) {
        if (in_str) {
            if (*p == '\\' && p[1]) {
                p++;
                continue;
            }
            if (*p == '\"') {
                in_str = 0;
            }
            continue;
        }
        if (*p == '\"') {
            in_str = 1;
            continue;
        }
        if (*p == '{') {
            depth++;
        } else if (*p == '}') {
            depth--;
            if (depth == 0) {
                return p;
            }
        }
    }
    return NULL;
}

static char *json_string_field_dup(const char *obj, const char *key);

static int json_string_after(const char *p, const char *key, char *out, size_t out_cap) {
    char *dup;
    if (!out || out_cap < 1) {
        return 0;
    }
    out[0] = '\0';
    dup = json_string_field_dup(p, key);
    if (!dup) {
        return 0;
    }
    strncpy(out, dup, out_cap - 1u);
    out[out_cap - 1u] = '\0';
    free(dup);
    return 1;
}

static void set_err(char *err_buf, size_t err_cap, const char *msg) {
    if (err_buf && err_cap > 0) {
        snprintf(err_buf, err_cap, "%s", msg ? msg : "error");
    }
}

static void json_fprint_escaped(FILE *f, const char *text) {
    const unsigned char *p;
    if (!f) {
        return;
    }
    if (!text) {
        return;
    }
    for (p = (const unsigned char *)text; *p; p++) {
        if (*p == '\"' || *p == '\\') {
            fputc('\\', f);
            fputc((int)*p, f);
        } else if (*p == '\n') {
            fputs("\\n", f);
        } else if (*p == '\r') {
            fputs("\\r", f);
        } else if (*p == '\t') {
            fputs("\\t", f);
        } else if (*p < 0x20u) {
            fprintf(f, "\\u%04x", (unsigned)*p);
        } else {
            fputc((int)*p, f);
        }
    }
}

int r01_project_save_json(const R01Project *p, const char *path, char *err_buf, size_t err_cap) {
    FILE *f;
    const R01World *w;
    int i;
    int wrote = 0;
    if (!p || !path) {
        set_err(err_buf, err_cap, "bad args");
        return -1;
    }
    if (r01_path_ensure_parent(path, err_buf, err_cap) != 0) {
        return -1;
    }
    /* Persist world 0 only -- load always applies the file into worlds[0]. */
    w = r01_project_world0_const(p);
    if (!w) {
        set_err(err_buf, err_cap, "bad project");
        return -1;
    }
    f = fopen(path, "w");
    if (!f) {
        set_err(err_buf, err_cap, "cannot write json");
        return -1;
    }
    fprintf(f, "{\n");
    fprintf(f, "  \"version\": %d,\n", R01_JSON_VER);
    fprintf(f, "  \"platform\": \"%s\",\n", R01_JSON_PLATFORM);
    fprintf(f, "  \"name\": \"%s\",\n", p->name);
    fprintf(f, "  \"default_world\": %d,\n", p->default_world);
    fprintf(f, "  \"active_world\": %d,\n", p->active_world);
    fprintf(f, "  \"active_screen\": %d,\n", p->active_screen);
    fprintf(f, "  \"default_screen\": %d,\n", w->default_screen);
    fprintf(f, "  \"default_pal_row\": %d,\n", w->default_pal_row);
    fprintf(f, "  \"grid_cols\": %d,\n", w->grid_cols);
    fprintf(f, "  \"grid_rows\": %d,\n", w->grid_rows);
    fprintf(f, "  \"global_pal_bg\": [");
    {
        int row, pal, j, first = 1;
        for (row = 0; row < R01_PAL_ROWS; row++) {
            for (pal = 0; pal < R01_PALS_PER_ROW; pal++) {
                fprintf(f, "%s[", first ? "" : ", ");
                first = 0;
                for (j = 0; j < R01_PAL_COLORS; j++) {
                    fprintf(f, "%s%d", j ? ", " : "", p->global_pal_bg[row][pal].idx[j]);
                }
                fprintf(f, "]");
            }
        }
    }
    fprintf(f, "],\n");
    fprintf(f, "  \"global_pal_spr\": [");
    {
        int row, pal, j, first = 1;
        for (row = 0; row < R01_PAL_ROWS; row++) {
            for (pal = 0; pal < R01_PALS_PER_ROW; pal++) {
                fprintf(f, "%s[", first ? "" : ", ");
                first = 0;
                for (j = 0; j < R01_PAL_COLORS; j++) {
                    fprintf(f, "%s%d", j ? ", " : "", p->global_pal_spr[row][pal].idx[j]);
                }
                fprintf(f, "]");
            }
        }
    }
    fprintf(f, "],\n");
    {
        size_t chr_bytes = (size_t)w->bg_banks[0].tile_count * R01_TILE_BYTES;
        char *bank_b64 = encode_b64(w->bg_banks[0].chr, chr_bytes);
        if (!bank_b64) {
            fclose(f);
            set_err(err_buf, err_cap, "oom");
            return -1;
        }
        fprintf(f, "  \"bg_bank0_tiles\": %d,\n", w->bg_banks[0].tile_count);
        fprintf(f, "  \"bg_bank0_b64\": \"%s\",\n", bank_b64);
        free(bank_b64);
    }
    fprintf(f, "  \"metatiles\": [\n");
    {
        int ti;
        for (ti = 0; ti < w->metatile_count; ti++) {
            const R01MetatileDef *mt = &w->metatiles[ti];
            fprintf(f,
                    "    {\"name\": \"%s\", \"tiles\": [%u,%u,%u,%u], \"attrs\": [%u,%u,%u,%u]}%s\n",
                    mt->name[0] ? mt->name : "Metatile", (unsigned)mt->tile[0], (unsigned)mt->tile[1],
                    (unsigned)mt->tile[2], (unsigned)mt->tile[3], (unsigned)mt->attr[0], (unsigned)mt->attr[1],
                    (unsigned)mt->attr[2], (unsigned)mt->attr[3], ti + 1 < w->metatile_count ? "," : "");
        }
    }
    fprintf(f, "  ],\n");
    fprintf(f, "  \"other_screens\": [\n");
    {
        int oi;
        int wrote_os = 0;
        for (oi = 0; oi < R01_CART_OTHER_MAX; oi++) {
            const R01OtherScreen *os = &p->other_screens[oi];
            char *tl;
            char *at;
            int force = (oi == R01_CART_OTHER_TITLE || oi == R01_CART_OTHER_INTER);
            if (!force && !os->present) {
                continue;
            }
            tl = encode_b64(os->tiles, sizeof(os->tiles));
            at = encode_b64(os->attrs, sizeof(os->attrs));
            if (!tl || !at) {
                free(tl);
                free(at);
                fclose(f);
                set_err(err_buf, err_cap, "oom");
                return -1;
            }
            fprintf(f, "%s    {\"id\": %d, \"present\": 1, \"tiles_b64\": \"%s\", \"attrs_b64\": \"%s\"}",
                    wrote_os ? ",\n" : "", oi, tl, at);
            wrote_os = 1;
            free(tl);
            free(at);
        }
        if (wrote_os) {
            fprintf(f, "\n");
        }
    }
    fprintf(f, "  ],\n");
    fprintf(f, "  \"screens\": [\n");
    for (i = 0; i < w->screen_count; i++) {
        const R01Screen *s = &w->screens[i];
        char *tl;
        char *at;
        if (!s->present) {
            continue;
        }
        tl = encode_b64(s->tiles, sizeof(s->tiles));
        at = encode_b64(s->attrs, sizeof(s->attrs));
        if (!tl || !at) {
            free(tl);
            free(at);
            fclose(f);
            set_err(err_buf, err_cap, "oom");
            return -1;
        }
        fprintf(f, "%s    {\"col\": %d, \"row\": %d,\n", wrote ? ",\n" : "", s->col, s->row);
        fprintf(f, "     \"tiles_b64\": \"%s\",\n", tl);
        fprintf(f, "     \"attrs_b64\": \"%s\"}", at);
        wrote = 1;
        free(tl);
        free(at);
    }
    fprintf(f, "\n  ],\n");
    fprintf(f, "  \"bg0_cols\": %d,\n", w->bg0_cols);
    fprintf(f, "  \"bg0_rows\": %d,\n", w->bg0_rows);
    fprintf(f, "  \"bg0_active_screen\": %d,\n", w->bg0_active_screen);
    fprintf(f, "  \"bg0_screens\": [\n");
    wrote = 0;
    for (i = 0; i < w->bg0_screen_count && i < R01_BG0_SCREENS_MAX; i++) {
        const R01Screen *s = &w->bg0_screens[i];
        char *tl;
        char *at;
        if (!s->present) {
            continue;
        }
        tl = encode_b64(s->tiles, sizeof(s->tiles));
        at = encode_b64(s->attrs, sizeof(s->attrs));
        if (!tl || !at) {
            free(tl);
            free(at);
            fclose(f);
            set_err(err_buf, err_cap, "oom");
            return -1;
        }
        fprintf(f, "%s    {\"col\": %d, \"row\": %d,\n", wrote ? ",\n" : "", s->col, s->row);
        fprintf(f, "     \"tiles_b64\": \"%s\",\n", tl);
        fprintf(f, "     \"attrs_b64\": \"%s\"}", at);
        wrote = 1;
        free(tl);
        free(at);
    }
    fprintf(f, "\n  ],\n");
    fprintf(f, "  \"bgm\": {\n");
    fprintf(f, "    \"track_count\": %d,\n", p->bgm.present ? p->bgm.track_count : 0);
    fprintf(f, "    \"tracks\": [\n");
    {
        int ti, first_t = 1;
        int tc = p->bgm.present ? p->bgm.track_count : 0;
        if (tc < 0) {
            tc = 0;
        }
        if (tc > R01_BGM_TRACKS_MAX) {
            tc = R01_BGM_TRACKS_MAX;
        }
        for (ti = 0; ti < tc; ti++) {
            int ch, first_ch;
            fprintf(f, "%s      {\"name\": \"%s\", \"channels\": [\n", first_t ? "" : ",\n",
                    p->bgm.track_name[ti][0] ? p->bgm.track_name[ti] : "Track");
            first_t = 0;
            first_ch = 1;
            for (ch = 0; ch < R01_BGM_CH_COUNT; ch++) {
                int ri, n = p->bgm.region_count[ti][ch];
                int first_r = 1;
                if (n < 0) {
                    n = 0;
                }
                if (n > R01_BGM_REGIONS_MAX) {
                    n = R01_BGM_REGIONS_MAX;
                }
                fprintf(f, "%s        [", first_ch ? "" : ",\n");
                first_ch = 0;
                for (ri = 0; ri < n; ri++) {
                    const R01BgmRegion *rg = &p->bgm.region[ti][ch][ri];
                    fprintf(f, "%s{\"s\":%d,\"l\":%d,\"m\":%d,\"t\":\"%s\"}", first_r ? "" : ",", rg->start,
                            rg->len, rg->midi, rg->tok[0] ? rg->tok : "--");
                    first_r = 0;
                }
                fprintf(f, "]");
            }
            fprintf(f, "\n      ]}");
        }
    }
    fprintf(f, "\n    ]\n");
    fprintf(f, "  }\n");
    fprintf(f, "}\n");
    fclose(f);
    return 0;
}

static const char *json_find(const char *hay, const char *needle) {
    return hay ? strstr(hay, needle) : NULL;
}

static int json_int_after(const char *p, const char *key, int *out) {
    const char *k = json_find(p, key);
    char *end;
    long v;
    if (!k || !out) {
        return 0;
    }
    k += strlen(key);
    while (*k == ' ' || *k == '\t' || *k == ':' || *k == '\"') {
        k++;
    }
    v = strtol(k, &end, 10);
    if (end == k) {
        return 0;
    }
    *out = (int)v;
    return 1;
}

static char *json_string_field_dup(const char *obj, const char *key) {
    const char *k = json_find(obj, key);
    const char *start;
    const char *end;
    size_t n;
    char *out;
    if (!k) {
        return NULL;
    }
    k += strlen(key);
    while (*k && *k != '\"') {
        k++;
    }
    if (*k != '\"') {
        return NULL;
    }
    start = k + 1;
    end = start;
    while (*end && *end != '\"') {
        end++;
    }
    n = (size_t)(end - start);
    out = (char *)malloc(n + 1u);
    if (!out) {
        return NULL;
    }
    memcpy(out, start, n);
    out[n] = '\0';
    return out;
}

static int load_screen_field(const char *slice, const char *b64_key, const char *rle_key, const char *hex_key,
                             uint8_t *out, size_t out_len) {
    char *text = json_string_field_dup(slice, b64_key);
    if (text) {
        size_t bin_len = 0;
        uint8_t *bin = decode_b64(text, &bin_len);
        int rc = -1;
        free(text);
        if (!bin) {
            return -1;
        }
        if (bin_len == out_len) {
            memcpy(out, bin, out_len);
            rc = 0;
        }
        free(bin);
        return rc;
    }
    text = json_string_field_dup(slice, rle_key);
    if (text) {
        size_t bin_len = 0;
        uint8_t *bin = decode_hex_alloc(text, &bin_len);
        int rc = -1;
        free(text);
        if (!bin) {
            return -1;
        }
        rc = r01_rle_decode(bin, bin_len, out, out_len);
        free(bin);
        return rc;
    }
    text = json_string_field_dup(slice, hex_key);
    if (!text) {
        return 0;
    }
    {
        int rc = decode_hex(text, out, out_len);
        free(text);
        return rc;
    }
}

static int load_other_screens(R01Project *p, const char *buf) {
    const char *section = json_find(buf, "\"other_screens\"");
    const char *obj;
    int n;

    if (!p || !section) {
        return 0;
    }
    obj = strchr(section, '[');
    if (!obj) {
        return 0;
    }
    obj++;
    for (n = 0; n < R01_CART_OTHER_MAX; n++) {
        const char *next = strchr(obj, '{');
        char *slice;
        const char *end;
        size_t slen;
        int id = n;
        int present = 1;
        if (!next) {
            break;
        }
        end = strchr(next, '}');
        if (!end) {
            break;
        }
        slen = (size_t)(end - next + 1);
        slice = (char *)malloc(slen + 1u);
        if (!slice) {
            return -1;
        }
        memcpy(slice, next, slen);
        slice[slen] = '\0';
        json_int_after(slice, "\"id\"", &id);
        json_int_after(slice, "\"present\"", &present);
        if (id < 0 || id >= R01_CART_OTHER_MAX) {
            free(slice);
            return -1;
        }
        if (load_screen_field(slice, "\"tiles_b64\"", "\"tiles_rle_hex\"", "\"tiles_hex\"",
                              p->other_screens[id].tiles, sizeof(p->other_screens[id].tiles)) != 0 ||
            load_screen_field(slice, "\"attrs_b64\"", "\"attrs_rle_hex\"", "\"attrs_hex\"",
                              p->other_screens[id].attrs, sizeof(p->other_screens[id].attrs)) != 0) {
            free(slice);
            return -1;
        }
        p->other_screens[id].present = present ? 1 : 0;
        free(slice);
        obj = end + 1;
    }
    p->other_screens[R01_CART_OTHER_TITLE].present = 1;
    p->other_screens[R01_CART_OTHER_INTER].present = 1;
    return 0;
}

static int parse_bracket_row(const char *start, uint8_t out[R01_PAL_COLORS]) {
    const char *k = start;
    int i = 0;
    if (!start || *start != '[') {
        return 0;
    }
    k++;
    while (*k && i < R01_PAL_COLORS) {
        char *end;
        long v;
        while (*k == ' ' || *k == '\t' || *k == ',') {
            k++;
        }
        if (*k == ']') {
            break;
        }
        v = strtol(k, &end, 10);
        if (end == k) {
            break;
        }
        out[i++] = (uint8_t)v;
        k = end;
    }
    return i == R01_PAL_COLORS;
}

static int load_palette_plane(const char *buf, const char *key, R01PalRow plane[R01_PAL_ROWS][R01_PALS_PER_ROW]) {
    const char *section = json_find(buf, key);
    const char *row;
    R01PalRow flat[R01_PAL_COUNT];
    int i = 0;
    int n;
    if (!section) {
        return 0;
    }
    row = strchr(section, '[');
    if (row) {
        row = strchr(row + 1, '[');
    }
    while (row && i < R01_PAL_COUNT) {
        if (parse_bracket_row(row, flat[i].idx)) {
            i++;
        }
        row = strchr(row + 1, '[');
    }
    n = i;
    if (n == R01_PAL_COUNT) {
        for (i = 0; i < R01_PAL_COUNT; i++) {
            plane[i / R01_PALS_PER_ROW][i % R01_PALS_PER_ROW] = flat[i];
        }
        return n;
    }
    /* Legacy v3: 4 pals = one row. Copy into row 0; caller keeps other rows from init. */
    if (n == R01_PALS_PER_ROW) {
        for (i = 0; i < R01_PALS_PER_ROW; i++) {
            plane[0][i] = flat[i];
        }
        return n;
    }
    return n;
}

int r01_project_load_json(R01Project *p, const char *path, char *err_buf, size_t err_cap) {
    FILE *f;
    long sz;
    char *buf = NULL;
    const char *section;
    const char *obj;
    int active = R01_START_ROW * R01_DEFAULT_GRID + R01_START_COL;
    int default_world = 0;
    int active_world = 0;
    int default_screen = -1;
    int default_pal_row = 0;
    int grid_cols = R01_DEFAULT_GRID;
    int grid_rows = R01_DEFAULT_GRID;

    if (!p || !path) {
        set_err(err_buf, err_cap, "bad args");
        return -1;
    }
    f = fopen(path, "rb");
    if (!f) {
        set_err(err_buf, err_cap, "cannot open json");
        return -1;
    }
    if (fseek(f, 0, SEEK_END) != 0 || (sz = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        set_err(err_buf, err_cap, "read failed");
        return -1;
    }
    buf = (char *)malloc((size_t)sz + 1u);
    if (!buf || fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf);
        fclose(f);
        set_err(err_buf, err_cap, "read failed");
        return -1;
    }
    buf[sz] = '\0';
    fclose(f);

    r01_project_init(p, "untitled");
    {
        char *name_str = json_string_field_dup(buf, "\"name\"");
        if (name_str && name_str[0]) {
            strncpy(p->name, name_str, R01_NAME_MAX - 1);
            p->name[R01_NAME_MAX - 1] = '\0';
        }
        free(name_str);
    }
    json_int_after(buf, "\"default_world\"", &default_world);
    json_int_after(buf, "\"active_world\"", &active_world);
    json_int_after(buf, "\"active_screen\"", &active);
    json_int_after(buf, "\"default_screen\"", &default_screen);
    json_int_after(buf, "\"default_pal_row\"", &default_pal_row);
    json_int_after(buf, "\"grid_cols\"", &grid_cols);
    json_int_after(buf, "\"grid_rows\"", &grid_rows);
    if (grid_cols >= 1 && grid_cols <= R01_GRID_MAX && grid_rows >= 1 && grid_rows <= R01_GRID_MAX) {
        r01_world_set_grid(r01_project_world0(p), grid_cols, grid_rows);
    }
    {
        R01World *ww = r01_project_world0(p);
        if (ww) {
            r01_world_bg0_clear(ww);
        }
    }
    if (default_world >= 0 && default_world < R01_MAX_WORLDS) {
        p->default_world = default_world;
    }
    if (active_world >= 0 && active_world < R01_MAX_WORLDS) {
        p->active_world = active_world;
    }
    /* Legacy "credits" ASCII string ignored -- credits are other-screen pages now. */
    if (load_other_screens(p, buf) != 0) {
        free(buf);
        set_err(err_buf, err_cap, "bad other_screens");
        return -1;
    }
    if (active >= 0 && active < r01_project_world0(p)->screen_count) {
        p->active_screen = active;
    }
    {
        int nbg = load_palette_plane(buf, "\"global_pal_bg\"", p->global_pal_bg);
        int nspr = load_palette_plane(buf, "\"global_pal_spr\"", p->global_pal_spr);
        (void)nbg;
        (void)nspr;
        /* Full plane (32) or legacy row-0 (4) already applied; other rows keep phase1 init. */
    }

    section = json_find(buf, "\"screens\"");
    {
        int tilemaps_loaded = 0;
        const char *screens_end = json_find(buf, "\"bg0_screens\"");
        if (section) {
            R01World *w = r01_project_world0(p);
            obj = strchr(section, '{');
            while (obj && obj < buf + sz && (!screens_end || obj < screens_end)) {
                const char *end = strchr(obj, '}');
                size_t olen;
                char *slice;
                int col = 0, row = 0, present = 1, si;
                int has_present = 0;
                R01Screen *s;
                if (!end) {
                    break;
                }
                olen = (size_t)(end - obj + 1);
                slice = (char *)malloc(olen + 1u);
                if (!slice) {
                    break;
                }
                memcpy(slice, obj, olen);
                slice[olen] = '\0';
                json_int_after(slice, "\"col\"", &col);
                json_int_after(slice, "\"row\"", &row);
                has_present = json_int_after(slice, "\"present\"", &present);
                si = r01_world_screen_index(w, col, row);
                if (si < 0) {
                    free(slice);
                    obj = strchr(end + 1, '{');
                    continue;
                }
                s = &w->screens[si];
                s->present = 1;
                if (has_present && !present) {
                    s->present = 0;
                }
                if (load_screen_field(slice, "\"pixels_b64\"", "\"pixels_rle_hex\"", "\"pixels_hex\"", s->pixels,
                                      sizeof(s->pixels)) != 0) {
                    free(slice);
                    free(buf);
                    set_err(err_buf, err_cap, "screen decode failed");
                    return -1;
                }
                if (load_screen_field(slice, "\"tiles_b64\"", "\"tiles_rle_hex\"", "\"tiles_hex\"", s->tiles,
                                      sizeof(s->tiles)) != 0 ||
                    load_screen_field(slice, "\"attrs_b64\"", "\"attrs_rle_hex\"", "\"attrs_hex\"", s->attrs,
                                      sizeof(s->attrs)) != 0) {
                    free(slice);
                    free(buf);
                    set_err(err_buf, err_cap, "screen decode failed");
                    return -1;
                }
                if (json_find(slice, "\"tiles_b64\"")) {
                    tilemaps_loaded = 1;
                }
                free(slice);
                obj = strchr(end + 1, '{');
            }
        }

        {
            const char *bg0sec = json_find(buf, "\"bg0_screens\"");
            R01World *w = r01_project_world0(p);
            int bg0_active = -1;
            json_int_after(buf, "\"bg0_active_screen\"", &bg0_active);
            if (bg0sec && w) {
                const char *bg0_end = json_array_end(bg0sec);
                r01_world_bg0_clear(w);
                obj = strchr(bg0sec, '{');
                while (obj && obj < buf + sz && (!bg0_end || obj < bg0_end)) {
                    const char *end = strchr(obj, '}');
                    size_t olen;
                    char *slice;
                    int col = 0, row = 0, si;
                    R01Screen *s;
                    if (!end || (bg0_end && end > bg0_end)) {
                        break;
                    }
                    olen = (size_t)(end - obj + 1);
                    slice = (char *)malloc(olen + 1u);
                    if (!slice) {
                        break;
                    }
                    memcpy(slice, obj, olen);
                    slice[olen] = '\0';
                    json_int_after(slice, "\"col\"", &col);
                    json_int_after(slice, "\"row\"", &row);
                    si = r01_world_bg0_create_screen(w, col, row);
                    if (si < 0) {
                        free(slice);
                        obj = strchr(end + 1, '{');
                        continue;
                    }
                    s = &w->bg0_screens[si];
                    if (load_screen_field(slice, "\"tiles_b64\"", "\"tiles_rle_hex\"", "\"tiles_hex\"", s->tiles,
                                          sizeof(s->tiles)) != 0 ||
                        load_screen_field(slice, "\"attrs_b64\"", "\"attrs_rle_hex\"", "\"attrs_hex\"", s->attrs,
                                          sizeof(s->attrs)) != 0) {
                        free(slice);
                        free(buf);
                        set_err(err_buf, err_cap, "bg0 screen decode failed");
                        return -1;
                    }
                    free(slice);
                    obj = strchr(end + 1, '{');
                }
                r01_world_bg0_recompute_extent(w);
            }
            if (w) {
                if (bg0_active >= 0 && bg0_active < w->bg0_screen_count &&
                    w->bg0_screens[bg0_active].present) {
                    w->bg0_active_screen = bg0_active;
                } else {
                    w->bg0_active_screen = -1;
                    {
                        int bi;
                        for (bi = 0; bi < w->bg0_screen_count; bi++) {
                            if (w->bg0_screens[bi].present) {
                                w->bg0_active_screen = bi;
                                break;
                            }
                        }
                    }
                }
            }
        }

        {
            R01World *w = r01_project_world0(p);
            int bank_tiles = 0;
            int bank_loaded = 0;
            char *bank_b64 = json_string_field_dup(buf, "\"bg_bank0_b64\"");
            json_int_after(buf, "\"bg_bank0_tiles\"", &bank_tiles);
            if (bank_b64 && bank_tiles > 0 && bank_tiles <= R01_TILES_PER_BANK) {
                size_t bin_len = 0;
                size_t expect = (size_t)bank_tiles * R01_TILE_BYTES;
                uint8_t *bin = decode_b64(bank_b64, &bin_len);
                if (bin && bin_len == expect) {
                    memset(w->bg_banks[0].chr, 0, R01_BANK_CHR_BYTES);
                    memcpy(w->bg_banks[0].chr, bin, expect);
                    w->bg_banks[0].tile_count = bank_tiles;
                    bank_loaded = 1;
                }
                free(bin);
            }
            free(bank_b64);
            if (bank_loaded) {
                int si;
                for (si = 0; si < w->screen_count; si++) {
                    R01Screen *s = &w->screens[si];
                    if (s->present) {
                        r01_screen_fill_pixels_from_bank(w, s);
                    }
                }
            } else if (!tilemaps_loaded) {
                r01_chr_pack_world_bank0(w);
            }
        }

        /* Metatiles only; legacy sprite/entity/warp keys ignored. */
        {
            R01World *w = r01_project_world0(p);
            const char *mt_sec = json_find(buf, "\"metatiles\":");
            const char *mt_end = json_array_end(mt_sec);
            w->metatile_count = 0;
            memset(w->metatiles, 0, sizeof(w->metatiles));
            if (mt_sec && mt_end) {
                const char *obj2 = strchr(mt_sec, '{');
                while (obj2 && obj2 < mt_end && w->metatile_count < R01_MAX_METATILES) {
                    const char *end = json_object_end(obj2);
                    size_t olen;
                    char *slice;
                    char *name_str;
                    int midx;
                    R01MetatileDef *mt;
                    int t0 = 0, t1 = 0, t2 = 0, t3 = 0;
                    int a0 = 0, a1 = 0, a2 = 0, a3 = 0;
                    if (!end || end >= mt_end) {
                        break;
                    }
                    olen = (size_t)(end - obj2 + 1);
                    slice = (char *)malloc(olen + 1u);
                    if (!slice) {
                        break;
                    }
                    memcpy(slice, obj2, olen);
                    slice[olen] = '\0';
                    midx = r01_world_metatile_add(w);
                    if (midx < 0) {
                        free(slice);
                        break;
                    }
                    mt = &w->metatiles[midx];
                    name_str = json_string_field_dup(slice, "\"name\"");
                    if (name_str && name_str[0]) {
                        strncpy(mt->name, name_str, R01_LABEL_MAX - 1);
                    }
                    free(name_str);
                    {
                        const char *tiles = strstr(slice, "\"tiles\"");
                        const char *attrs = strstr(slice, "\"attrs\"");
                        if (tiles) {
                            (void)sscanf(tiles, "%*[^[][%d,%d,%d,%d]", &t0, &t1, &t2, &t3);
                        }
                        if (attrs) {
                            (void)sscanf(attrs, "%*[^[][%d,%d,%d,%d]", &a0, &a1, &a2, &a3);
                        }
                    }
                    mt->tile[0] = (uint8_t)t0;
                    mt->tile[1] = (uint8_t)t1;
                    mt->tile[2] = (uint8_t)t2;
                    mt->tile[3] = (uint8_t)t3;
                    mt->attr[0] = (uint8_t)a0;
                    mt->attr[1] = (uint8_t)a1;
                    mt->attr[2] = (uint8_t)a2;
                    mt->attr[3] = (uint8_t)a3;
                    free(slice);
                    obj2 = strchr(end + 1, '{');
                }
            }
        }
    }

    {
        R01World *w0 = r01_project_world0(p);
        if (default_screen >= 0 && default_screen < w0->screen_count && w0->screens[default_screen].present) {
            w0->default_screen = default_screen;
        } else {
            r01_world_sync_default_screen(w0);
        }
        if (default_pal_row >= 0 && default_pal_row < R01_PAL_ROWS) {
            w0->default_pal_row = default_pal_row;
        }
    }

    if (active >= 0 && active < r01_project_world0(p)->screen_count &&
        r01_project_world0(p)->screens[active].present) {
        p->active_screen = active;
    } else {
        r01_project_select_start_screen(p);
    }
    /* Migrate older 8x8 (or partial) projects to the full 16x16 slot map. */
    {
        R01World *w0 = r01_project_world0(p);
        int ac = -1;
        int ar = -1;
        if (p->active_screen >= 0 && p->active_screen < w0->screen_count &&
            w0->screens[p->active_screen].present) {
            ac = w0->screens[p->active_screen].col;
            ar = w0->screens[p->active_screen].row;
        }
        r01_world_ensure_full_grid(w0);
        if (ac >= 0) {
            p->active_screen = r01_world_screen_index(w0, ac, ar);
        } else {
            r01_project_select_start_screen(p);
        }
    }
    /* File data lives in world 0; show that world after load. */
    p->active_world = 0;
    {
        const char *bgm_sec = json_find(buf, "\"bgm\"");
        memset(&p->bgm, 0, sizeof(p->bgm));
        if (bgm_sec) {
            const char *tracks = json_find(bgm_sec, "\"tracks\"");
            const char *tracks_end = json_array_end(tracks);
            const char *tobj;
            int ti = 0;
            json_int_after(bgm_sec, "\"track_count\"", &p->bgm.track_count);
            tobj = tracks ? strchr(tracks, '{') : NULL;
            while (tobj && tracks_end && tobj < tracks_end && ti < R01_BGM_TRACKS_MAX) {
                const char *tend = json_object_end(tobj);
                char *slice;
                size_t slen;
                char *name;
                const char *ch_sec;
                const char *ch_end;
                const char *carr;
                int ch;
                if (!tend || tend >= tracks_end) {
                    break;
                }
                slen = (size_t)(tend - tobj + 1);
                slice = (char *)malloc(slen + 1u);
                if (!slice) {
                    break;
                }
                memcpy(slice, tobj, slen);
                slice[slen] = '\0';
                name = json_string_field_dup(slice, "\"name\"");
                if (name) {
                    snprintf(p->bgm.track_name[ti], sizeof(p->bgm.track_name[ti]), "%s", name);
                    free(name);
                } else {
                    snprintf(p->bgm.track_name[ti], sizeof(p->bgm.track_name[ti]), "Track %d", ti + 1);
                }
                ch_sec = json_find(slice, "\"channels\"");
                ch_end = json_array_end(ch_sec);
                carr = ch_sec ? strchr(ch_sec, '[') : NULL;
                if (carr) {
                    carr++; /* past outer '[' */
                }
                for (ch = 0; ch < R01_BGM_CH_COUNT && carr && ch_end && carr < ch_end; ch++) {
                    const char *lb = strchr(carr, '[');
                    const char *rb;
                    int depth;
                    int in_str;
                    const char *robj;
                    int n = 0;
                    if (!lb || lb >= ch_end) {
                        break;
                    }
                    depth = 0;
                    in_str = 0;
                    rb = NULL;
                    for (robj = lb; *robj && robj < ch_end; robj++) {
                        if (in_str) {
                            if (*robj == '\\' && robj[1]) {
                                robj++;
                                continue;
                            }
                            if (*robj == '\"') {
                                in_str = 0;
                            }
                            continue;
                        }
                        if (*robj == '\"') {
                            in_str = 1;
                            continue;
                        }
                        if (*robj == '[') {
                            depth++;
                        } else if (*robj == ']') {
                            depth--;
                            if (depth == 0) {
                                rb = robj;
                                break;
                            }
                        }
                    }
                    if (!rb) {
                        break;
                    }
                    robj = strchr(lb, '{');
                    while (robj && robj < rb && n < R01_BGM_REGIONS_MAX) {
                        const char *ro_end = json_object_end(robj);
                        char *rslice;
                        size_t rlen;
                        char *tok;
                        int s = 0, l = 1, m = 0;
                        if (!ro_end || ro_end > rb) {
                            break;
                        }
                        rlen = (size_t)(ro_end - robj + 1);
                        rslice = (char *)malloc(rlen + 1u);
                        if (!rslice) {
                            break;
                        }
                        memcpy(rslice, robj, rlen);
                        rslice[rlen] = '\0';
                        json_int_after(rslice, "\"s\"", &s);
                        json_int_after(rslice, "\"l\"", &l);
                        json_int_after(rslice, "\"m\"", &m);
                        tok = json_string_field_dup(rslice, "\"t\"");
                        free(rslice);
                        if (l < 1) {
                            l = 1;
                        }
                        p->bgm.region[ti][ch][n].start = s;
                        p->bgm.region[ti][ch][n].len = l;
                        p->bgm.region[ti][ch][n].midi = m;
                        if (tok) {
                            snprintf(p->bgm.region[ti][ch][n].tok, sizeof(p->bgm.region[ti][ch][n].tok), "%s",
                                     tok);
                            free(tok);
                        } else {
                            snprintf(p->bgm.region[ti][ch][n].tok, sizeof(p->bgm.region[ti][ch][n].tok), "--");
                        }
                        n++;
                        robj = strchr(ro_end + 1, '{');
                    }
                    p->bgm.region_count[ti][ch] = n;
                    carr = rb + 1;
                }
                free(slice);
                ti++;
                tobj = strchr(tend + 1, '{');
            }
            if (p->bgm.track_count < ti) {
                p->bgm.track_count = ti;
            }
            if (p->bgm.track_count > R01_BGM_TRACKS_MAX) {
                p->bgm.track_count = R01_BGM_TRACKS_MAX;
            }
            if (ti > 0 || p->bgm.track_count > 0) {
                p->bgm.present = 1;
            }
        }
    }
    free(buf);
    return 0;
}
