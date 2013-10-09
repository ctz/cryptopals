#include "util-1.h"

/* assume we know this (we can find it out with oracle('') if not...) */
#define SECRET_LEN 30

/* 2^24 seems enough for reliable recovery */
#define ROUNDS 0x1000000

/* weighting for big and slight biases */
#define FULL_WEIGHT 4
#define HALF_WEIGHT 1

/* --- quick and dirty rc4 --- */
static void swap8(uint8_t *left, uint8_t *right)
{
  uint8_t tmp = *left;
  *left = *right;
  *right = tmp;
}

static void rc4(const uint8_t *key, size_t keylen, uint8_t *xorbuf, size_t xorbuflen)
{
  uint8_t S[256];
  for (unsigned i = 0; i < 256; i++)
    S[i] = i;
    
  for (unsigned i = 0, j = 0; i < 256; i++)
  {
    j = (j + S[i] + key[i % keylen]) & 0xff;
    swap8(S + i, S + j);
  }
  
  for (unsigned i = 0, j = 0, n = 0; n < xorbuflen; n++)
  {
    i = (i + 1) & 0xff;
    j = (j + S[i]) & 0xff;
    swap8(S + i, S + j);
    xorbuf[n] ^= S[(S[i] + S[j]) & 0xff];
  }
}

static void oracle(const byteblock *prefix, const byteblock *secret, byteblock *out)
{
  uint8_t key[16];
  random_fill(key, sizeof key);
  
  out->len = prefix->len + secret->len;
  
  memcpy(out->buf, prefix->buf, prefix->len);
  memcpy(out->buf + prefix->len, secret->buf, secret->len);
  
  rc4(key, sizeof key, out->buf, out->len);
}

static uint8_t winner(unsigned byte[256])
{
  uint8_t win = 0;
  unsigned best_score = 0;
  
  for (size_t i = 0; i < 256; i++)
  {
    if (byte[i] > best_score)
    {
      best_score = byte[i];
      win = i;
    }
  }
  return win;
}

static byteblock recover(pool *p, unsigned counts[SECRET_LEN][256])
{
  byteblock r = { p->alloc(p, SECRET_LEN), SECRET_LEN };
  
  for (size_t i = 0; i < SECRET_LEN; i++)
  {
    r.buf[i] = winner(counts[i]);
  }
  
  return r;
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };  
  random_init();

  /* biases at 16 and 32, so we prefix our 30 byte secret with
   * two bytes to start with, up to 17 */
  byteblock prefix = { (uint8_t *) "AAAAAAAAAAAAAAAAAA", 2 };
  
  assert(argc == 2);
  byteblock secret = from_base64(p, argv[1]);
  
  assert(secret.len == SECRET_LEN);
  
  unsigned counts[SECRET_LEN][256];
  memset(counts, 0, sizeof(counts));
  
  while (prefix.len < 18)
  {
    uint8_t buf[64];
    byteblock bb = { buf, 0 };
    
    for (size_t i = 0; i < ROUNDS; i++)
    {
      oracle(&prefix, &secret, &bb);
      
      if (prefix.len <= 15)
      {
        /* bias at 16 towards 240 (full) 0 (half) 16 (half) */
        uint8_t b16 = buf[15];
        counts[15 - prefix.len][b16 ^ 240] += FULL_WEIGHT;
        counts[15 - prefix.len][b16 ^ 0] += HALF_WEIGHT;
        counts[15 - prefix.len][b16 ^ 16] += HALF_WEIGHT;
      }
        
      /* bias at 32 towards 224 (full) 0 (half) 32 (half) */
      uint8_t b32 = buf[31];
      counts[31 - prefix.len][b32 ^ 224] += FULL_WEIGHT;
      counts[31 - prefix.len][b32 ^ 0] += HALF_WEIGHT;
      counts[31 - prefix.len][b32 ^ 32] += HALF_WEIGHT;
    }
    
    prefix.len++;
    
    byteblock plaintext = recover(p, counts);
    printf("guess: %s\n", to_ascii(p, &plaintext));
  }
  
  byteblock recovered = recover(p, counts);
  printf("message: %s\n", to_ascii(p, &recovered));
  
  p->finish(p);
  return 0;
}