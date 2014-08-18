/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*Daala video codec
Copyright (c) 2006-2014 Daala project contributors.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

static long long iabs(long long a) { return a > 0 ? a : -a; }
static int min(int a, int b) { return a < b ? a : b; }

typedef struct {
  unsigned char *yf;
  unsigned char *uf;
  unsigned char *vf;
  unsigned char *yc;
  unsigned char *uc;
  unsigned char *vc;
  unsigned char *yp;
  unsigned char *up;
  unsigned char *vp;
  int w;
  int h;
  int strength; /* 256 is a reasonable value */
} filter_data;


static int check(int s, unsigned char *filt, unsigned char *curr, unsigned char *prev, int stride, int threshold, int strength)
{
  long long csd = 0;
  int curr_avg = 0;
  int prev_avg = 0;
  int x, y;

  for (y = 0; y < s; y++)
    for (x = 0; x < s; x++) {
      int diff = curr[y*stride+x] - prev[y*stride+x];
      csd += iabs(diff*diff*diff);
      curr_avg += curr[y*stride+x];
      prev_avg += prev[y*stride+x];
      if (filt) { /* Apply temporal filter */
        int strength = 3;
        unsigned int w = min(iabs(curr[y*stride+x] - prev[y*stride+x]), (1 << strength) - 1) + 1;
        filt[y*stride+x] = ((w * curr[y*stride+x] + ((1 << strength) - w) * filt[y*stride+x]) + (1 << (strength-1))) / (1 << strength);
      }
    }

  return (curr_avg > prev_avg ? curr_avg - prev_avg : prev_avg - curr_avg) <
          s*s && csd < s*s*threshold*threshold*threshold*strength/256;
}

static void memcpy2d(unsigned char *dest, unsigned char *src, int size, int stride)
{
  int y;
  for (y = 0; y < size; y++)
    memcpy(dest + y*stride, src + y*stride, size);
}

static void filter_block(int size, filter_data *data, int x, int y, int threshold)
{
  int yoff = y * data->w + x;
  int uvoff = y/2 * data->w/2 + x/2;
  unsigned char *yp = data->yp;
  unsigned char *up = data->up;
  unsigned char *vp = data->vp;

  /* Static video? */
  if (x + size < data->w && y + size < data->h &&
      check(size,   data->yf + yoff,  data->yc + yoff,  yp + yoff,  data->w,   threshold, data->strength) &&
      check(size/2, data->uf + uvoff, data->uc + uvoff, up + uvoff, data->w/2, threshold, data->strength) &&
      check(size/2, data->vf + uvoff, data->vc + uvoff, vp + uvoff, data->w/2, threshold, data->strength)) {
    
    /* Use temporally filtered video if diff is small */
    if (check(size,   0, data->yc + yoff,  data->yf + yoff,  data->w,   threshold, data->strength) &&
        check(size/2, 0, data->uc + uvoff, data->uf + uvoff, data->w/2, threshold, data->strength) &&
        check(size/2, 0, data->vc + uvoff, data->vf + uvoff, data->w/2, threshold, data->strength)) {
      yp = data->yf;
      up = data->uf;
      vp = data->vf;
    }
    
    /* Reuse previous (filtered) frame */
    memcpy2d(data->yc + yoff,  yp + yoff,  size,   data->w);
    memcpy2d(data->uc + uvoff, up + uvoff, size/2, data->w/2);
    memcpy2d(data->vc + uvoff, vp + uvoff, size/2, data->w/2);
    return;
  }
  
  if (size > 2) {
    int a, b;

    for (a = 0; a < size; a += size >> 1)
      for (b = 0; b < size; b += size >> 1)
        filter_block(size >> 1, data, x + b, y + a, threshold);
  }
}

/* Apply a filter helping the encoder to avoid spending bits on static video */
void pre_filter(unsigned char *yf, unsigned char *uf, unsigned char *vf,
                unsigned char *yc, unsigned char *uc, unsigned char *vc,
                unsigned char *yp, unsigned char *up, unsigned char *vp,
                int w, int h, int strength)
{
  static char gop[] = { 2, 4, 5, 4, 5, 4, 5, 4 };
  static int frame = 0;
  int threshold = gop[frame++ & 7];
  int a, b;
  filter_data data;

  data.yf = yf;
  data.uf = uf;
  data.vf = vf;
  data.yc = yc;
  data.uc = uc;
  data.vc = vc;
  data.yp = yp;
  data.up = up;
  data.vp = vp;
  data.w = w;
  data.h = h;
  data.strength = strength;

  for (a = 0; a < h; a += 128)
    for (b = 0; b < w; b += 128)
      filter_block(128, &data, b, a, threshold);
}
