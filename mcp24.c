#include "util-1.h"
#include "mt19937.h"

byteblock mt19937_encryptdecrypt(pool *p, uint32_t key, const byteblock *plaintext)
{
  byteblock plain = { p->alloc(p, plaintext->len), plaintext->len };
  mt19937 st = mt19937_init(key);
  
  for (size_t i = 0; i < plain.len; i++)
    plain.buf[i] = plaintext->buf[i] ^ (mt19937_extract(&st) & 0xff);
  
  return plain;
}

byteblock encrypt(pool *p, uint32_t key, const byteblock *plaintext)
{
  size_t prefixlen = (rand() % 16) + 4;
  byteblock prefix = byteblock_random(p, prefixlen);
  byteblock plain = byteblock_concat(p, &prefix, plaintext);
  return mt19937_encryptdecrypt(p, key, &plain);
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  
  argv++;
  argc--;
  assert(argc == 0);
  
  random_init();
  uint32_t maxkey = 0xffff;
  uint32_t key = rand() % maxkey;
  
  byteblock plaintext = { (void *) "AAAAAAAAAAAAAA", 14 };
  byteblock ciphertext = encrypt(p, key, &plaintext);
  
  assert(ciphertext.len >= plaintext.len);
  
  byteblock pt = mt19937_encryptdecrypt(p, key, &ciphertext);
  assert(memcmp(pt.buf + (pt.len - plaintext.len), plaintext.buf, plaintext.len) == 0);
  
  // known plaintext + key search
  size_t randlen = ciphertext.len - plaintext.len;
  uint32_t foundkey = 0;
  for (uint32_t try = 0; try < maxkey; try++)
  {
    pool inner[1] = { pool_create() };
    byteblock candidate = mt19937_encryptdecrypt(inner, try, &ciphertext);
    
    if (memcmp(candidate.buf + randlen, plaintext.buf, plaintext.len) == 0)
    {
      foundkey = try;
      break;
    }
    
    inner->finish(inner);
  }

  assert(foundkey == key);
  printf("ok\n");
  
  p->finish(p);
  return 0;
}
