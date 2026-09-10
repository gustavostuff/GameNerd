#include "retr01_nano_emu/cart.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : R01NE_DEFAULT_CART;
    R01neCart cart;
    R01neWorldView w;
    char err[128];
    uint8_t map[R01NE_SCREEN_PAYLOAD];
    int di;

    memset(&cart, 0, sizeof(cart));
    if (r01ne_cart_load_path(&cart, path, err, sizeof(err)) != 0) {
        fprintf(stderr, "load: %s\n", err);
        return 1;
    }
    if (memcmp(cart.data, R01NE_CART_MAGIC, 6) != 0) {
        fprintf(stderr, "magic\n");
        return 1;
    }
    if (r01ne_cart_world(&cart, 0, &w) != 0) {
        fprintf(stderr, "world 0\n");
        return 1;
    }
    if (w.screen_count < 1) {
        fprintf(stderr, "no screens\n");
        return 1;
    }
    di = r01ne_world_find_screen(&cart, &w, w.spawn_col, w.spawn_row);
    if (di < 0) {
        fprintf(stderr, "spawn screen missing\n");
        return 1;
    }
    if (r01ne_world_load_screen(&cart, &w, di, map) != 0) {
        fprintf(stderr, "payload\n");
        return 1;
    }
    printf("ok cart=%s screens=%u spawn=(%u,%u)\n", path, (unsigned)w.screen_count,
           (unsigned)w.spawn_col, (unsigned)w.spawn_row);
    r01ne_cart_free(&cart);
    return 0;
}
