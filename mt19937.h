#ifndef MT19937_H
#define MT19937_H

#include "util-1.h"

#define N 624

typedef struct
{
  uint32_t mt[N];
  uint32_t index;
} mt19937;

mt19937 mt19937_init(uint32_t seed)
{
  mt19937 rv;
  memset(&rv, 0, sizeof rv);
  rv.mt[0] = seed;
  for (size_t i = 1; i < N; i++)
    rv.mt[i] = 0xFFFFFFFF & (0x6c078965 * (rv.mt[i - 1] ^ (rv.mt[i - 1] >> 30)) + i);
  return rv;
}

void _mt19937_generate(mt19937 *ctx)
{
  for (size_t i = 0; i < N; i++)
  {
    uint32_t y = (ctx->mt[i] & 0x80000000) + 
                 (ctx->mt[(i + 1) % N] & 0x7fffffff);
                 
    ctx->mt[i] = ctx->mt[(i + 397) % N] ^ (y >> 1);
    if (y % 2)
      ctx->mt[i] ^= 0x9908b0df;
  }
}

uint32_t mt19937_extract(mt19937 *ctx)
{
  if (ctx->index == 0)
    _mt19937_generate(ctx);
  
  uint32_t y = ctx->mt[ctx->index];
  y ^= y >> 11;
  y ^= (y << 7) & 0x9d2c5680;
  y ^= (y << 15) & 0xefc60000;
  y ^= y >> 18;
  
  ctx->index++;
  ctx->index %= N;
  return y;
}

#undef N

#endif
