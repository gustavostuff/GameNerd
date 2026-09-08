/* User game logic — created once by Studio export; never overwritten. */
#include "include/r01_engine.h"

void r01_custom_on_init(R01GameCtx *ctx) {
    /* Player states: 0 idle, 1 slide_x (L/R), 2 slide_up, 3 slide_down */
    r01_player_anim_set_idle_state(ctx, 0);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_RIGHT, 1);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_LEFT, 1);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_DOWN_RIGHT, 1);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_DOWN_LEFT, 1);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_UP_RIGHT, 1);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_UP_LEFT, 1);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_UP, 2);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_DOWN, 3);
    r01_player_default_face_set(ctx, R01_PLAYER_FACE_RIGHT);
    r01_entity_state_frame_delay_set(ctx, 0, 10);
    r01_entity_state_frame_delay_set(ctx, 1, 4);
    r01_entity_state_frame_delay_set(ctx, 2, 4);
    r01_entity_state_frame_delay_set(ctx, 3, 4);
    /* Nano: no camera scroll — screen switches are instant. */
}

void r01_custom_on_tick(R01GameCtx *ctx) {
    (void)ctx;
}

void r01_custom_on_vblank(R01GameCtx *ctx) {
    (void)ctx;
}
