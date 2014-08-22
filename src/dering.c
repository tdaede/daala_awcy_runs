/*Daala video codec
Copyright (c) 2014 Daala project contributors.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#include "dering.h"

void od_dering(int pli, int _scale, unsigned char *_data, int ystride,
 int h, int w){
  unsigned char *data;
  int b;
  int bh;
  int bw;
  int sthresh;
  int strong;
  int scale;
  int x;
  int y;
  int *var;
  int *_var;
  bh = h >> 3;
  bw = w >> 3;
  var = _var = (int*) _ogg_calloc(bh * bw, sizeof(int));
  data = _data;
  strong = pli ? OD_SDERINGC : OD_SDERINGY;
  sthresh = pli ? OD_DERING_THRESH4 : OD_DERING_THRESH3;
  scale = (int)(_scale * OD_DERING_Q);
  for(y = 0; y < 4; y++){
    data += ystride;
  }
  for(; y < h - 8; y += 8){
    od_hedge(data - ystride, ystride, _var ,_var + bw);
    _var++;
    for(x = 8; x < w; x += 8){
      od_hedge(data + x - ystride, ystride, _var, _var + bw);
      od_vedge((data + x - (ystride << 2) - 4), ystride, _var - 1);
      _var++;
    }
    data += ystride << 3;
  }
  for(; y < h; y++){
    data += ystride;
  }
  for(x = 8; x < w; x += 8){
    od_vedge((data + x - (ystride << 3) - 4), ystride, _var++);
  }
  _var=var;
  for (y = 0; y < h; y += 8){
    for (x = 0; x < w; x += 8){
      int val;
      val = *var;
      b = (x <= 0) | (x + 8 >= w) << 1 | (y <= 0) << 2 | (y + 8 >= h) << 3;
      if(strong && val > sthresh){
        od_dering_block(_data + x, ystride, b, scale, 1);
        if(pli || !(b & 1) && *(var - 1) > OD_DERING_THRESH4 ||
         !(b & 2) && var[1] > OD_DERING_THRESH4 ||
         !(b & 4) && *(var - bw) > OD_DERING_THRESH4 ||
         !(b & 8) && var[bw] > OD_DERING_THRESH4){
          od_dering_block(_data + x, ystride, b, scale, 1);
          od_dering_block(_data + x, ystride, b, scale, 1);
        }
      }
      else if(val > OD_DERING_THRESH2){
        od_dering_block(_data + x, ystride, b, scale, 1);
      }
      else if(val > OD_DERING_THRESH1){
        od_dering_block(_data + x, ystride, b, scale, 0);
      }
      var++;
    }
    _data += ystride << 3;
  }
  _ogg_free(_var);
}

void od_dering_block(unsigned char *data, int ystride, int b,
 int scale, int strong){
  static const unsigned char OD_MOD_MAX[2] = {16, 24};
  static const unsigned char OD_MOD_SHIFT[2] = {1, 0};
  const unsigned char *psrc;
  const unsigned char *src;
  const unsigned char *nsrc;
  unsigned char *dst;
  int vmod[72];
  int hmod[72];
  int mod_hi;
  int by;
  int bx;
  mod_hi = OD_MINI(3 * scale, OD_MOD_MAX[strong]);
  dst = data;
  src = dst;
  psrc = src - (ystride & -!(b & 4));
  for(by = 0; by < 9; by++){
    for(bx = 0; bx < 8; bx++){
      int mod;
      mod = 24 + scale - (abs(src[bx] - psrc[bx]) << OD_MOD_SHIFT[strong]);
      vmod[(by << 3) + bx] = OD_CLAMPI(0, mod, mod_hi);
    }
    psrc = src;
    src += ystride & -(!(b & 8)| by < 7);
  }
  nsrc = dst;
  psrc = dst - !(b & 1);
  for(bx = 0; bx < 9; bx++){
    src = nsrc;
    for(by = 0; by < 8; by++){
      int mod;
      mod = 24 + scale - (abs(*src - *psrc) << OD_MOD_SHIFT[strong]);
      hmod[(bx << 3) + by] = OD_CLAMPI(0, mod, mod_hi);
      psrc += ystride;
      src += ystride;
    }
    psrc = nsrc;
    nsrc += !(b & 2) | bx < 7;
  }
  src = dst;
  psrc = src - (ystride & - !(b & 4));
  nsrc = src + ystride;
  for(by = 0; by < 8; by++){
    int a;
    int b;
    int w;
    a = 128;
    b = 64;
    w = hmod[by];
    a -= w;
    b += w * *(src - !(b & 1));
    w = vmod[by << 3];
    a -= w;
    b += w * psrc[0];
    w = vmod[by + 1 << 3];
    a -= w;
    b += w * nsrc[0];
    w = hmod[(1 << 3) + by];
    a -= w;
    b += w * src[1];
    dst[0] = OD_CLAMP255(a * src[0] + b >> 7);
    for(bx = 1; bx < 7; bx++){
      a = 128;
      b = 64;
      w = hmod[(bx << 3) + by];
      a -= w;
      b += w * src[bx - 1];
      w = vmod[(by << 3) + bx];
      a -= w;
      b += w * psrc[bx];
      w = vmod[(by + 1 << 3) + bx];
      a -= w;
      b += w * nsrc[bx];
      w = hmod[(bx + 1 << 3) + by];
      a -= w;
      b += w * src[bx + 1];
      dst[bx] = OD_CLAMP255(a * src[bx] + b >> 7);
    }
    a = 128;
    b = 64;
    w = hmod[(7 << 3) + by];
    a -= w;
    b += w * src[6];
    w = vmod[(by << 3) + 7];
    a -= w;
    b += w * psrc[7];
    w = vmod[(by + 1 << 3) + 7];
    a -= w;
    b += w * nsrc[7];
    w = hmod[(8 << 3) + by];
    a -= w;
    b += w * src[7 + !(b & 2)];
    dst[7] = OD_CLAMP255(a * src[7] + b >> 7);
    dst += ystride;
    psrc = src;
    src = nsrc;
    nsrc += ystride & -(!(b & 8) | by < 6);
  }
}

void od_hedge(unsigned char *src, int ystride, int *variance0, int *variance1){
  unsigned char *rsrc;
  unsigned char *csrc;
  int r[10];
  int sum0;
  int sum1;
  int bx;
  int by;
  rsrc = src;
  for(bx = 0; bx < 8; bx++){
    csrc = rsrc;
    for(by = 0; by < 10; by++){
      r[by] = *csrc;
      csrc += ystride;
    }
    sum0 = sum1 = 0;
    for(by = 0; by < 4; by++){
      sum0 += abs(r[by + 1] - r[by]);
      sum1 += abs(r[by + 5] - r[by + 6]);
    }
    *variance0 += OD_MINI(255, sum0);
    *variance1 += OD_MINI(255, sum1);
    rsrc++;
  }
}

void od_vedge(unsigned char *src, int ystride, int *variances){
  unsigned char *rsrc;
  unsigned char *csrc;
  int r[10];
  int sum0;
  int sum1;
  int bx;
  int by;
  rsrc = src;
  for(by = 0; by < 8; by++){
    csrc = rsrc - 1;
    for(bx = 0; bx < 10; bx++){
      r[bx] = *csrc++;
    }
    sum0 = sum1 = 0;
    for(bx = 0; bx < 4; bx++){
      sum0 += abs(r[bx + 1] - r[bx]);
      sum1 += abs(r[bx + 5] - r[bx + 6]);
    }
    variances[0] += OD_MINI(255, sum0);
    variances[1] += OD_MINI(255, sum1);
    rsrc += ystride;
  }
}
