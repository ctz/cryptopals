#include "util-2.h"
#include "cipher.h"

#define CIPHERS 60

uint8_t noncebuf[8];
uint8_t keybuf[16];
byteblock nonce = { noncebuf, sizeof noncebuf };
byteblock key = { keybuf, sizeof keybuf };

uint8_t guess_key_byte(pool *p, byteblock *cts, size_t ncts, size_t offs)
{
  byteblock cipher = { p->alloc(p, ncts), ncts };

  for (size_t i = 0; i < ncts; i++)
  {
    cipher.buf[i] = cts[i].buf[offs];
  }

  int best_score = 0xffff;
  uint8_t best_byte = 0;
  
  for (int k = 0; k < 256; k++)
  {
    uint8_t byte = k;
    byteblock bb = { &byte, 1 };

    byteblock test = byteblock_xor(p, &cipher, &bb);
    int score = score_english(&test);

    if (score < best_score)
    {
      best_byte = byte;
      best_score = score;
    }
  }

  return best_byte;
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };

  byteblock cts[CIPHERS];

  argc--;
  argv++;
  assert(argc == CIPHERS);
  byteblock target;

  size_t minlen = 0xffff;
  size_t attack_ciphertext = 0;
  for (size_t i = 0; i < CIPHERS; i++)
  {
    byteblock plain = from_base64(p, argv[i]);

    cts[i] = rijndael_ctr_process(p, &key, &nonce, &plain);

    if (plain.len <= minlen)
    {
      /* attack shortest ciphertext */
      minlen = plain.len;
      attack_ciphertext = i;
      target = plain;
    }
  }

  for (size_t i = 0; i < CIPHERS; i++)
    cts[i].len = minlen;

  #define BUFLEN 64
  uint8_t kstream[BUFLEN];
  assert(BUFLEN >= minlen);

  for (size_t i = 0; i < minlen; i++)
  {
    kstream[i] = guess_key_byte(p, cts, CIPHERS, i);
  }

  byteblock stream = { kstream, minlen };
  byteblock plaintext = byteblock_xor(p, &cts[attack_ciphertext], &stream);

  printf("%s\n", to_ascii(p, &plaintext));

  p->finish(p);
  return 0;
}
