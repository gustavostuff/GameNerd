#include "test_harness.h"

#include "retr01_studio/cart.h"
#include "retr01_studio/chr_pack.h"
#include "retr01_studio/project.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t rd_u24(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16);
}

static int tile_1bpp_nonzero(const uint8_t tile[R01_NANO_TILE_BYTES]) {
    int i;
    for (i = 0; i < R01_NANO_TILE_BYTES; i++) {
        if (tile[i]) {
            return 1;
        }
    }
    return 0;
}

TEST_MAIN() {
    R01Project *p = (R01Project *)calloc(1, sizeof(R01Project));
    R01World *w;
    uint8_t tile[R01_TILE_BYTES];
    char err[128];
    int tile_id;

    EXPECT(p != NULL, "alloc project");
    if (!p) {
        return 1;
    }

    r01_project_init(p, "cart");
    w = &p->worlds[0];
    {
        int i;
        int keep = r01_world_default_screen(w);
        for (i = 0; i < w->screen_count; i++) {
            w->screens[i].present = (i == keep) ? 1 : 0;
        }
    }

    tile_id = r01_chr_alloc_tile(w, 0);
    EXPECT(tile_id == 1, "bg tile 1 (0 reserved blank)");
    memset(tile, 0, sizeof(tile));
    tile[0] = 0xFF;
    tile[8] = 0xFF;
    EXPECT(r01_chr_write_tile(w, 0, tile_id, tile) == 0, "write bg tile");

    EXPECT(r01_cart_write(p, "test_cart.r01nano", err, sizeof(err)) == 0, "cart write");
    {
        FILE *f = fopen("test_cart.r01nano", "rb");
        uint8_t *img = NULL;
        long flen = 0;
        char magic[6];

        EXPECT(f != NULL, "open cart");
        if (f) {
            EXPECT(fread(magic, 1, 6, f) == 6, "read cart magic");
            EXPECT(memcmp(magic, R01_CART_MAGIC, 6) == 0, "cart magic r01nan");
            EXPECT(fseek(f, 6, SEEK_SET) == 0, "seek format_ver");
            {
                uint8_t fmt = 0;
                EXPECT(fread(&fmt, 1, 1, f) == 1, "read format_ver");
                EXPECT(fmt == R01_CART_FORMAT_VER, "cart format_ver");
            }

            EXPECT(fseek(f, 0, SEEK_END) == 0, "seek end");
            flen = ftell(f);
            EXPECT(flen > 0, "cart size");
            img = (uint8_t *)malloc((size_t)flen);
            EXPECT(img != NULL, "cart buf");
            if (img) {
                uint8_t ptrs[R01_CART_PTR_TABLE_BYTES];
                uint8_t slot[8];
                uint8_t hdr[R01_CART_WORLD_HDR_BYTES];
                uint32_t off_wdir, world_base, off_chr;
                uint8_t chr_tile[R01_NANO_TILE_BYTES];

                EXPECT(fseek(f, 0, SEEK_SET) == 0, "rewind");
                EXPECT(fread(img, 1, (size_t)flen, f) == (size_t)flen, "read cart");
                memcpy(ptrs, img + R01_CART_HDR_BYTES, R01_CART_PTR_TABLE_BYTES);
                off_wdir = rd_u24(ptrs + 0);
                EXPECT(off_wdir == R01_CART_HDR_BYTES + R01_CART_PTR_TABLE_BYTES, "slot0 world dir off");
                EXPECT(rd_u24(ptrs + 3) == R01_CART_WORLD_DIR_BYTES, "slot0 world dir len");
                EXPECT(rd_u24(ptrs + 6) == 0, "music off 0");
                EXPECT(rd_u24(ptrs + 9) == 0, "music len 0");

                memcpy(slot, img + off_wdir, 8);
                EXPECT(slot[0] != 0, "world0 present");
                world_base = rd_u24(slot + 2);
                memcpy(hdr, img + world_base, R01_CART_WORLD_HDR_BYTES);
                EXPECT(hdr[R01_CART_WHDR_TYPE_COUNT] == 0, "type count zero");
                EXPECT(hdr[R01_CART_WHDR_INST_COUNT] == 0, "inst count zero");
                EXPECT(rd_u24(hdr + R01_CART_WHDR_OFF_TYPES) == 0, "off_types zero");
                EXPECT(rd_u24(hdr + R01_CART_WHDR_OFF_INSTS) == 0, "off_insts zero");
                EXPECT(hdr[R01_CART_WHDR_PLAYER_ENTITY] == R01_CART_PLAYER_ENTITY_NONE, "no player entity");
                EXPECT(hdr[R01_CART_WHDR_SCREEN_COUNT] == 1, "one present screen");

                off_chr = rd_u24(hdr + R01_CART_WHDR_OFF_CHR);
                EXPECT(off_chr == R01_CART_WORLD_HDR_BYTES, "off_chr");
                memcpy(chr_tile, img + world_base + off_chr + R01_NANO_TILE_BYTES, R01_NANO_TILE_BYTES);
                EXPECT(tile_1bpp_nonzero(chr_tile), "chr tile 1bpp nonzero");
                {
                    uint32_t bank1 = world_base + off_chr + R01_NANO_BANK_CHR_BYTES;
                    uint8_t pad[R01_NANO_TILE_BYTES];
                    memcpy(pad, img + bank1, R01_NANO_TILE_BYTES);
                    EXPECT(!tile_1bpp_nonzero(pad), "bank1 padded zero");
                }

                {
                    uint32_t off_sdir = rd_u24(hdr + R01_CART_WHDR_OFF_SCREEN_DIR);
                    uint32_t payload_off = rd_u24(img + world_base + off_sdir + 3);
                    EXPECT(payload_off >= R01_CART_WORLD_HDR_BYTES + R01_BG_BANKS * R01_NANO_BANK_CHR_BYTES +
                                       R01_CART_SCREEN_DIR_BYTES,
                           "screen payload relative");
                    EXPECT(payload_off + R01_CART_SCREEN_PAYLOAD <= (uint32_t)flen - world_base,
                           "screen payload 384 fits");
                }
                free(img);
            }
            fclose(f);
        }
    }

    EXPECT(r01_cart_write_flash(p, "test_cart_flash.bin", err, sizeof(err)) == 0, "flash write");
    {
        FILE *f = fopen("test_cart_flash.bin", "rb");
        long flen = 0;
        uint8_t tail;
        EXPECT(f != NULL, "open flash");
        if (f) {
            EXPECT(fseek(f, 0, SEEK_END) == 0, "flash seek end");
            flen = ftell(f);
            EXPECT(flen == (long)R01_CART_FLASH_BYTES, "flash size 128KB");
            EXPECT(fseek(f, flen - 1, SEEK_SET) == 0, "flash seek tail");
            EXPECT(fread(&tail, 1, 1, f) == 1, "flash read tail");
            EXPECT(tail == 0xFF, "flash padded 0xFF");
            fclose(f);
        }
    }

    free(p);
    TEST_EXIT();
}
