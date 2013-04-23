#include "util-2.h"
#include "cipher.h"

#define CIPHERS 40

uint8_t noncebuf[8];
uint8_t keybuf[16];
byteblock nonce = { noncebuf, sizeof noncebuf };
byteblock key = { keybuf, sizeof keybuf };

#define GUESSLEN (30)

/* which ciphertext to attack */
#define ATTACK 5

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == CIPHERS + 1);

  byteblock cts[CIPHERS];

  argc--;
  argv++;

  for (int i = 0; i < argc; i++)
  {
    byteblock plain = from_base64(p, argv[i]);
    cts[i] = rijndael_ctr_process(p, &key, &nonce, &plain);
  }

  /* build a table of probablities of key values */
  int guesses[GUESSLEN][256] = { { 0 } };
  for (size_t i = 0; i < GUESSLEN; i++)
  {
    for (int guess = 0; guess < 256; guess++)
    {
      for (size_t sample = 0; sample < CIPHERS; sample++)
      {
        if (cts[sample].len >= i)
        {
          int result = guess ^ cts[sample].buf[i];
          guesses[i][guess] += score_english_char(result);
        }
      }
    }
  }

  /* construct the key from the highest probability values */
  uint8_t key[GUESSLEN];

  for (size_t i = 0; i < GUESSLEN; i++)
  {
    uint8_t best;
    int best_score = 0xffff;

    for (int guess = 0; guess < 256; guess++)
    {
      if (guesses[i][guess] < best_score)
      {
        best = guess;
        best_score = guesses[i][guess];
      }
    }

    key[i] = best;
  }

  /* now decrypt and print */
  for (size_t i = 0; i < GUESSLEN; i++)
  {
    if (i >= cts[ATTACK].len)
      break;
    printf("%c", cts[ATTACK].buf[i] ^ key[i]);
  }
  printf("\n");

  p->finish(p);
  return 0;
}
