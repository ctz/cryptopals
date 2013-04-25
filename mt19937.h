#ifndef MT19937_H
#define MT19937_H

#include "util-1.h"

#define MT19937_N 624
#define N MT19937_N

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

uint32_t mt19937_temper(uint32_t y)
{
  y ^= y >> 11;
  y ^= (y << 7) & 0x9d2c5680;
  y ^= (y << 15) & 0xefc60000;
  y ^= y >> 18;
  return y;
}

uint32_t mt19937_extract(mt19937 *ctx)
{
  if (ctx->index == 0)
    _mt19937_generate(ctx);
  
  uint32_t y = ctx->mt[ctx->index];
  
  ctx->index++;
  ctx->index %= N;
  return mt19937_temper(y);
}

/* from: http://jazzy.id.au/default/2010/09/22/cracking_random_number_generators_part_3.html , sorry */
uint32_t _unshift_right(uint32_t v, unsigned shift)
{
  uint32_t i = 0;
  uint32_t r = 0;

  while (i * shift < 32)
  {
    uint32_t mask = (0xffffffff << (32 - shift)) >> (shift * i);
    uint32_t part = v & mask;
    v ^= part >> shift;
    r |= part;
    i++;
  }

  return r;
}

uint32_t _unshift_left_mask(uint32_t v, unsigned shift, uint32_t mask)
{
  uint32_t i = 0;
  uint32_t r = 0;

  while (i * shift < 32)
  {
    uint32_t pmask = (0xffffffff >> (32 - shift)) << (shift * i);
    uint32_t part = v & pmask;
    v ^= (part << shift) & mask;
    r |= part;
    i++;
  }

  return r;
}

uint32_t mt19937_untemper(uint32_t v)
{
  uint32_t r = v;

  r = _unshift_right(r, 18);
  r = _unshift_left_mask(r, 15, 0xefc60000);
  r = _unshift_left_mask(r, 7, 0x9d2c5680);
  r = _unshift_right(r, 11);

  assert(mt19937_temper(r) == v);
  return r;
}

#undef N

#endif
