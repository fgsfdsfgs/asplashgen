#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "gif.h"
#include "animsplash.h"

struct gifdata_s {
  void *oldbuf;
  void *newbuf;
  spl_frame_t *frame;
  FILE *fout;
};

static void gif_frame(void *data, GIF_WHDR *whdr) {
  uint32_t x, y, yoff, iter, ifin, dsrc, tx, ty;
  spl_delta_t dline, dcol;
  struct gifdata_s *gifdata = data;

  uint32_t *old_buf = gifdata->oldbuf;
  uint32_t *buf = gifdata->newbuf;
  FILE *fout = gifdata->fout;
  spl_frame_t *frame = gifdata->frame;
  uint32_t *tileptr = frame->data;

  memset(frame, 0, sizeof(spl_frame_t));

  #define BGRA(i) \
    ((uint32_t)(whdr->cpal[whdr->bptr[i]].R <<  0) | \
     (uint32_t)(whdr->cpal[whdr->bptr[i]].G <<  8) | \
     (uint32_t)(whdr->cpal[whdr->bptr[i]].B << 16) | \
                      0xFF000000)

  if (!whdr->ifrm) {
    whdr->nfrm = ((whdr->nfrm < 0)? -whdr->nfrm : whdr->nfrm) * whdr->ydim;
    whdr->nfrm = (whdr->nfrm < 0xFFFF)? whdr->nfrm : 0xFFFF;
  }

  printf("frame %ld\n", whdr->ifrm);

  uint32_t bkcolor = (whdr->mode == GIF_BKGD) ? BGRA(whdr->bkgd) : 0xFF000000;

  ifin = (!(iter = (whdr->intr)? 0 : 4))? 4 : 5; /** interlacing support **/
  for (dsrc = (uint32_t)-1; iter < ifin; iter++)
    for (yoff = 16U >> ((iter > 1)? iter : 1), y = (8 >> iter) & 7;
       y < (uint32_t)whdr->fryd; y += yoff)
      for (x = 0; x < (uint32_t)whdr->frxd; x++) {
        if (whdr->tran != (long)whdr->bptr[++dsrc])
          buf[(uint32_t)whdr->xdim * y + x] = BGRA(dsrc);
        else
          buf[(uint32_t)whdr->xdim * y + x] = bkcolor;
      }

  for (y = 0; y < whdr->ydim; ++y) {
    for (x = 0; x < whdr->xdim; ++x) {
      if (old_buf[(uint32_t)whdr->xdim * y + x] == buf[(uint32_t)whdr->xdim * y + x])
        continue;
      tx = x / SPL_TILEW;
      ty = y / SPL_TILEH;
      if ((frame->delta[ty] >> tx) & 1)
        continue;
      frame->delta[ty] |= 1 << tx;
    }
  }

  for (ty = 0; ty < SPL_YTILES; ++ty) {
    dline = frame->delta[ty];
    dcol = dline;
    for (tx = 0; tx < SPL_XTILES; dcol >>= 1, ++tx) { 
      if (dcol & 1) {
        for (y = ty * SPL_TILEH; y < ty * SPL_TILEH + SPL_TILEH; ++y)
          for (x = tx * SPL_TILEW; x < tx * SPL_TILEW + SPL_TILEW; ++x)
            *(tileptr++) = buf[(uint32_t)whdr->xdim * y + x];
        frame->numtiles++;
      }
    }
  }

  printf("  numtiles %d framesz %ld\n", frame->numtiles, sizeof(uint32_t) * SPL_TILEW * SPL_TILEH * frame->numtiles);

  fwrite(frame, sizeof(spl_frame_t) + frame->numtiles * SPL_TILEW * SPL_TILEH * sizeof(uint32_t), 1, fout);

  memcpy(old_buf, buf, sizeof(uint32_t) * whdr->xdim * whdr->ydim);

  #undef BGRA
}

int gif_convert(const char *infname, const char *outfname) {
  FILE *fin = fopen(infname, "rb");
  if (!fin) return -1;

  fseek(fin, 0, SEEK_END);
  size_t fin_size = ftell(fin);
  fseek(fin, 0, SEEK_SET);

  if (!fin_size) { fclose(fin); return -2; }

  uint8_t *data = malloc(fin_size);
  if (!data) { fclose(fin); return -3; }

  fread(data, fin_size, 1, fin);
  fclose(fin);

  uint32_t *buf_a = calloc(SPL_W * SPL_H, sizeof(uint32_t));
  uint32_t *buf_b = calloc(SPL_W * SPL_H, sizeof(uint32_t));
  spl_frame_t *frame = calloc(1, sizeof(spl_frame_t) + SPL_W * SPL_H * sizeof(uint32_t));
  if (!buf_a || !buf_b || !frame) return -4; // don't care about memory leaks

  FILE *fout = fopen(outfname, "wb");
  if (!fout) return -5;

  struct gifdata_s gifdata = {
    buf_a, buf_b, frame, fout,
  };

  GIF_Load(data, (long)fin_size, gif_frame, 0, (void*)&gifdata, 0L);

  free(buf_a);
  free(buf_b);
  fclose(fout);
  return 0;
}
