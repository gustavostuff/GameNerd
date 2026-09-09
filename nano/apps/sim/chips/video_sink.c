#include "video_sink.h"

#include "retr01_sim/bus.h"

#include <string.h>

static void sink_reset(R01sEntity *e) {
    R01nsVideoSink *c = (R01nsVideoSink *)e;
    memset(c->rgb, 0, sizeof(c->rgb));
    c->frames = 0;
    c->vblank_edge = 0;
}

static void sink_eval(R01sEntity *e) {
    (void)e;
}

static void sink_tick(R01sEntity *e) {
    (void)e;
}

static void sink_destroy(R01sEntity *e) {
    (void)e;
}

static const R01sEntityVTable SINK_VT = {sink_reset, sink_eval, sink_tick, sink_destroy};

void r01ns_video_sink_init(R01nsVideoSink *chip, const char *refdes) {
    if (!chip) {
        return;
    }
    memset(chip, 0, sizeof(*chip));
    r01s_entity_init(&chip->base, &SINK_VT, "SCREEN_SINK", refdes ? refdes : "SCR1");
    chip->base.impl = chip;
    r01s_entity_add_pin(&chip->base, 1, "RGB", R01S_PIN_IN);
    r01s_entity_add_pin(&chip->base, 2, "HSYNC", R01S_PIN_IN);
    r01s_entity_add_pin(&chip->base, 3, "VSYNC", R01S_PIN_IN);
    r01s_entity_add_pin(&chip->base, 4, "VCC", R01S_PIN_PWR);
    /* Board glyph = half RGBS field (128x96). Do not use snap5_up — that becomes
     * 130x100 and nearest-scales the 256x192 texture with uneven pixel sizes. */
    r01s_entity_set_glyph(&chip->base, R01S_ENTITY_VIS_DISPLAY, R01NS_VIDEO_W / 2, R01NS_VIDEO_H / 2);
    chip->base.body_w = R01NS_VIDEO_W / 2;
    chip->base.body_h = R01NS_VIDEO_H / 2;
    r01s_entity_reset(&chip->base);
}

R01sEntity *r01ns_video_sink_entity(R01nsVideoSink *chip) {
    return chip ? &chip->base : NULL;
}

void r01ns_video_sink_blit_rgb(R01nsVideoSink *chip, const uint8_t *rgb, size_t nbytes) {
    size_t n;
    if (!chip || !rgb) {
        return;
    }
    n = sizeof(chip->rgb);
    if (nbytes < n) {
        n = nbytes;
    }
    memcpy(chip->rgb, rgb, n);
    chip->frames++;
    r01s_entity_drive(&chip->base, "RGB", R01S_LVL_H);
    r01s_entity_drive(&chip->base, "HSYNC", R01S_LVL_H);
}

void r01ns_video_sink_plot_line_from_fb(R01nsVideoSink *chip, int y, const uint8_t *fb) {
    size_t off;
    if (!chip || !fb || y < 0 || y >= R01NS_VIDEO_H) {
        return;
    }
    off = (size_t)y * (size_t)R01NS_VIDEO_W * 3u;
    memcpy(chip->rgb + off, fb + off, (size_t)R01NS_VIDEO_W * 3u);
    r01s_entity_drive(&chip->base, "RGB", R01S_LVL_H);
    r01s_entity_drive(&chip->base, "HSYNC", R01S_LVL_L);
    r01s_entity_drive(&chip->base, "HSYNC", R01S_LVL_H);
}

void r01ns_video_sink_on_vblank(R01nsVideoSink *chip) {
    if (!chip) {
        return;
    }
    chip->vblank_edge = 1;
    r01s_entity_drive(&chip->base, "VSYNC", R01S_LVL_H);
}

const uint8_t *r01ns_video_sink_rgb(const R01nsVideoSink *chip) {
    return chip ? chip->rgb : NULL;
}
