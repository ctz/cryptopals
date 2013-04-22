#include "util-2.h"
#include "cipher.h"

byteblock key = { (void *) "abcdef12abcdef12", 16 };
byteblock secret_prefix = { (void *) "abcdef12", 8 };
uint8_t buffer[64] = { 0 };
byteblock zeroes = { buffer, sizeof buffer };

byteblock secret_function(pool *p, const byteblock *plain, const byteblock *secret)
{
  byteblock prefixed = byteblock_concat(p, &secret_prefix, plain);
  byteblock total = byteblock_concat(p, &prefixed, secret);
  byteblock padded = pkcs7_pad(p, &total, 16);
  return rijndael_ecb_encrypt(p, &padded, &key);
}

uint8_t guess_byte(pool *p, const byteblock *suffix, const byteblock *secret, size_t padlen, size_t junklen)
{
  size_t skipjunk = 16 - junklen;
  size_t inlen = 16 + skipjunk + padlen + suffix->len + 1;
  size_t suffixblocks = (suffix->len + 1) / 16;
  byteblock input = { p->alloc(p, inlen), inlen };
  
  /* construct skipjunk_0 || bb || suffix || pad || zeroes */
  
  memset(input.buf, 0xaa, skipjunk);
  memcpy(input.buf + 1 + skipjunk, suffix->buf, suffix->len);
  size_t fakepad = 16 - ((1 + suffix->len) % 16);
  memset(input.buf + 1 + suffix->len + skipjunk, fakepad, fakepad);
  
  for (int byte = 0; byte < 256; byte++)
  {
    input.buf[skipjunk] = byte;
    byteblock cipher = secret_function(p, &input, secret);
    int correct = memcmp(cipher.buf + 16, cipher.buf + cipher.len - 16 * suffixblocks - 16, 16) == 0;
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
  
  size_t junklen = 0;
  
  /* first block is useless, just work out when we spill into second and third blocks
   * to determine how long the prefix is */
  for (size_t i = 0; i < 64; i++)
  {
    byteblock plain = zeroes;
    plain.len = i;
    byteblock cipher = secret_function(p, &plain, &secret);
    
    if (memcmp(cipher.buf + 16, cipher.buf + 32, 16) == 0)
    {
      junklen = i - 32;
      break;
    }
  }
  
  assert(junklen == 8 && junklen == secret_prefix.len);
  
  size_t first_edge = 0xffff, second_edge = 0xffff;
  size_t last = 0;
  for (size_t i = 0; i < 64 - junklen; i++)
  {
    byteblock plain = zeroes;
    plain.len = i + junklen;
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
    uint8_t next = guess_byte(p, &guessed, &secret, padlen, junklen);
    
    memmove(guessed.buf + 1, guessed.buf, guessed.len);
    guessed.buf[0] = next;
    guessed.len++;
  }
  
  printf("blocklen: %zu, padlen: %zu, junklen: %zu, secret: %s\n", blocklen, padlen, junklen, to_ascii(p, &guessed));
  
  p->finish(p);
  return 0;
}