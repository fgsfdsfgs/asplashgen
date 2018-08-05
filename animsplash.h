#pragma once

#include <stdint.h>

#define SPL_W 720
#define SPL_H 1280
#define SPL_STRIDE (720 + 48)
#define SPL_XTILES 15
#define SPL_YTILES 20
#define SPL_TILEW 48
#define SPL_TILEH 64

typedef uint16_t spl_delta_t;

typedef struct spl_frame_s {
  uint32_t numtiles;
  spl_delta_t delta[SPL_YTILES];
  uint32_t data[];
} spl_frame_t;

int gif_convert(const char *infname, const char *outfname);