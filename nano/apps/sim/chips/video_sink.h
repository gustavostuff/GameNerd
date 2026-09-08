#ifndef RETR01_NANO_SIM_VIDEO_SINK_H
#define RETR01_NANO_SIM_VIDEO_SINK_H

#include "retr01_sim/entity.h"

#include <stddef.h>
#include <stdint.h>

/* Nano RGBS: 256x192 (2x of 128x96). Soft compose writes full RGB24. */
#define R01NS_VIDEO_W 256
#define R01NS_VIDEO_H 192

typedef struct R01nsVideoSink {
    R01sEntity base;
    uint8_t rgb[R01NS_VIDEO_W * R01NS_VIDEO_H * 3];
    uint32_t frames;
    uint8_t vblank_edge;
} R01nsVideoSink;

void r01ns_video_sink_init(R01nsVideoSink *chip, const char *refdes);
R01sEntity *r01ns_video_sink_entity(R01nsVideoSink *chip);

void r01ns_video_sink_blit_rgb(R01nsVideoSink *chip, const uint8_t *rgb, size_t nbytes);
void r01ns_video_sink_on_vblank(R01nsVideoSink *chip);
const uint8_t *r01ns_video_sink_rgb(const R01nsVideoSink *chip);

#endif
