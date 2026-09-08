#ifndef RETR01_NANO_EMU_CUSTOM_HOST_H
#define RETR01_NANO_EMU_CUSTOM_HOST_H

struct R01neMachine;

/* Bind machine for map/soft hooks; call before custom_logic. */
void r01ne_custom_bind(struct R01neMachine *m);

/* Init R01GameCtx, run r01_custom_on_init, apply pose policy to Host Play anim. */
void r01ne_custom_start(struct R01neMachine *m);

/* Sync pad/pose into ctx, dispatch button events, r01_custom_on_tick. */
void r01ne_custom_frame(struct R01neMachine *m);

#endif
