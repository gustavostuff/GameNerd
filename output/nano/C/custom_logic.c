/* User game logic — created once by Studio export; never overwritten. */
#include "include/r01_engine.h"

void r01_custom_on_init(R01GameCtx *ctx) {
    /* player entity: state 0 = Idle, state 1 = Walk (all dirs for now) */
    r01_player_anim_set_idle_state(ctx, 0);
    r01_player_anim_set_walk_all(ctx, 1);
    r01_player_default_face_set(ctx, R01_PLAYER_FACE_RIGHT);
    r01_entity_state_frame_delay_set(ctx, 0, 10);
    r01_entity_state_frame_delay_set(ctx, 1, 4);
    /* Nano: no camera scroll — screen switches are instant. */
}

void r01_custom_on_tick(R01GameCtx *ctx) {
    (void)ctx;
}

void r01_custom_on_vblank(R01GameCtx *ctx) {
    (void)ctx;
}
