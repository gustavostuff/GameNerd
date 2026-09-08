/* User game logic — created once by Studio export; never overwritten. */
#include "include/r01_engine.h"

/* Player entity states (Studio names): idle, slide_x, slide_up, slide_down */
enum {
    PLAYER_STATE_IDLE = 0,
    PLAYER_STATE_SLIDE_X = 1,
    PLAYER_STATE_SLIDE_UP = 2,
    PLAYER_STATE_SLIDE_DOWN = 3
};

void r01_custom_on_init(R01GameCtx *ctx) {
    /*
     * Pose policy lives here. Runtime/base_game only draw the active state index
     * (player_anim_state) — they do not decide idle vs slide themselves.cls
     */
    r01_player_anim_set_idle_state(ctx, PLAYER_STATE_IDLE);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_RIGHT, PLAYER_STATE_SLIDE_X);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_LEFT, PLAYER_STATE_SLIDE_X);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_DOWN_RIGHT, PLAYER_STATE_SLIDE_X);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_DOWN_LEFT, PLAYER_STATE_SLIDE_X);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_UP_RIGHT, PLAYER_STATE_SLIDE_X);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_UP_LEFT, PLAYER_STATE_SLIDE_X);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_UP, PLAYER_STATE_SLIDE_UP);
    r01_player_anim_set_walk_state(ctx, R01_PLAYER_DIR_DOWN, PLAYER_STATE_SLIDE_DOWN);
    /* slide_x → idle on release; slide_up / slide_down keep their pose. */
    r01_player_anim_set_release_to_idle(ctx, PLAYER_STATE_SLIDE_X, 1);
    r01_player_default_face_set(ctx, R01_PLAYER_FACE_RIGHT);
    r01_entity_state_frame_delay_set(ctx, PLAYER_STATE_IDLE, 10);
    r01_entity_state_frame_delay_set(ctx, PLAYER_STATE_SLIDE_X, 4);
    r01_entity_state_frame_delay_set(ctx, PLAYER_STATE_SLIDE_UP, 4);
    r01_entity_state_frame_delay_set(ctx, PLAYER_STATE_SLIDE_DOWN, 4);
    /* Nano: no camera scroll — screen switches are instant. */
}

void r01_custom_on_tick(R01GameCtx *ctx) {
    (void)ctx;
}

void r01_custom_on_vblank(R01GameCtx *ctx) {
    (void)ctx;
}
