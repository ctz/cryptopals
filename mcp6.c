#include "util-1.h"

#define MAX_KEYLEN 40

// unused
static float score_keylen(const byteblock *b, size_t len)
{
  byteblock x = { &b->buf[len * 0], len };
  byteblock y = { &b->buf[len * 1], len };
  byteblock z = { &b->buf[len * 2], len };
  byteblock w = { &b->buf[len * 3], len };
  
  float ham = byteblock_hamming(&x, &y) + byteblock_hamming(&x, &z) + byteblock_hamming(&x, &w);
  return ham / len;
}

static uint8_t find_key_byte(pool *p, const byteblock *b, size_t offs, size_t block)
{
  byteblock cp = { p->alloc(p, b->len), 0 };
  
  for (size_t x = 0, i = offs; i < b->len; i += block, x++)
  {
    cp.buf[x] = b->buf[i];
    cp.len++;
  }
  
  int best_score = INT_MAX;
  uint8_t best_keybyte = 0;
  
  for (int keybyte = 0; keybyte < 256; keybyte++)
  {
    uint8_t kb = (uint8_t) keybyte;
    byteblock key = { &kb, 1 };
    byteblock plain = byteblock_xor(p, &cp, &key);
    int score = score_english(&plain);
    
    if (score < best_score)
    {
      best_score = score;
      best_keybyte = keybyte;
    }
  }
  
  return best_keybyte;
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  
  assert(argc == 2);
  byteblock cipher = from_base64(p, argv[1]);
  
  int best_score = INT_MAX;
  byteblock best_key;
  byteblock best_plaintext;
  
  // this is actually quick enough and easier to bruteforce: 40 * 256 * 256 tries
  
  for (size_t k = 2; k < MAX_KEYLEN; k++)
  {
    byteblock key = { p->alloc(p, k), k };
    for (size_t i = 0; i < key.len; i++)
    {
      key.buf[i] = find_key_byte(p, &cipher, i, key.len);
    }
    
    byteblock plain = byteblock_xor(p, &cipher, &key);
    int score = score_english(&plain);
    
    if (score < best_score)
    {
      best_score = score;
      best_key = key;
      best_plaintext = plain;
    }
  }
  
  printf("key: %s, plain: %s\n", to_hex(p, &best_key), to_ascii(p, &best_plaintext));
  
  p->finish(p);
  return 0;
}