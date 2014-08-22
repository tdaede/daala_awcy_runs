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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
# include "stdlib.h"
# include "internal.h"

/*Stronger luma plane deringing.*/
#define OD_SDERINGY  (0)
/*Stronger chroma plane deringing.*/
#define OD_SDERINGC  (0)

/*Deringing scale constant*/
#define OD_DERING_Q (0.00625)

#define OD_DERING_THRESH1 (384)
#define OD_DERING_THRESH2 (4*OD_DERING_THRESH1)
#define OD_DERING_THRESH3 (5*OD_DERING_THRESH1)
#define OD_DERING_THRESH4 (10*OD_DERING_THRESH1)

void od_dering(int pli, int scale, unsigned char *data, int ystride,
 int h, int w);
void od_dering_block(unsigned char *data, int ystride, int b,
 int scale, int strong);
void od_hedge(unsigned char *src, int ystride, int *variance0,
 int *variance1);
void od_vedge(unsigned char *src, int ystride, int *variances);
