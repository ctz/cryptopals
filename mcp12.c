#include "util-2.h"
#include "cipher.h"

byteblock key = { (void *) "abcdef12abcdef12", 16 };
uint8_t buffer[64] = { 0 };
byteblock zeroes = { buffer, sizeof buffer };

byteblock secret_function(pool *p, const byteblock *plain, const byteblock *secret)
{
  byteblock total = byteblock_concat(p, plain, secret);
  byteblock padded = pkcs7_pad(p, &total, 16);
  return rijndael_ecb_encrypt(p, &padded, &key);
}

uint8_t guess_byte(pool *p, const byteblock *suffix, const byteblock *secret, size_t padlen)
{
  size_t inlen = 16 + padlen + suffix->len + 1;
  size_t suffixblocks = (suffix->len + 1) / 16;
  byteblock input = { p->alloc(p, inlen), inlen };
  
  /* construct bb || suffix || pad || zeroes */
  
  memcpy(input.buf + 1, suffix->buf, suffix->len);
  size_t fakepad = 16 - ((1 + suffix->len) % 16);
  memset(input.buf + 1 + suffix->len, fakepad, fakepad);
  
  for (int byte = 0; byte < 256; byte++)
  {
    input.buf[0] = byte;
    byteblock cipher = secret_function(p, &input, secret);
    int correct = memcmp(cipher.buf, cipher.buf + cipher.len - 16 * suffixblocks - 16, 16) == 0;
    if (correct)
      return byte;
  }
  
  printf("failed to guess byte!\n");
  abort();
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 2);
  byteblock secret = from_base64(p, argv[1]);
  
  size_t first_edge = 0xffff, second_edge = 0xffff;
  size_t last = 0;
  
  for (size_t i = 0; i < 64; i++)
  {
    byteblock plain = zeroes;
    plain.len = i;
    byteblock cipher = secret_function(p, &plain, &secret);
    
    if (last == 0)
      last = cipher.len;
    else {
      if (last != cipher.len)
      {
        if (first_edge > i)
          first_edge = i;
        else {
          second_edge = i;
          break;
        }
          
        last = cipher.len;
      }
    }
  }
  
  size_t blocklen = second_edge - first_edge;
  size_t padlen = first_edge;
  
  assert(blocklen == 16);
  assert(padlen == 6);
  
  // successively spill bytes from the secret into the last block
  byteblock guessed = { p->alloc(p, secret.len), 0 };
  
  for (size_t i = 0; i < secret.len; i++)
  {
    uint8_t next = guess_byte(p, &guessed, &secret, padlen);
    
    memmove(guessed.buf + 1, guessed.buf, guessed.len);
    guessed.buf[0] = next;
    guessed.len++;
  }
  
  printf("blocklen: %zu, padlen: %zu, secret: %s\n", blocklen, padlen, to_ascii(p, &guessed));
  
  p->finish(p);
  return 0;
}