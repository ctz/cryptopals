#include "util-2.h"
#include "cipher.h"

byteblock key = { (void *) "123qwerty123qwer", 16 };

byteblock prefix = { (void *) "comment1=cooking%20MCs;userdata=", 32 };
byteblock suffix = { (void *) ";comment2=%20like%20a%20pound%20of%20bacon", 42 };
const char *magic = ";admin=true;";

byteblock strip_metas(pool *p, const byteblock *in)
{
  byteblock r = { p->alloc(p, in->len), in->len };
  
  for (size_t i = 0; i < in->len; i++)
    if (in->buf[i] == ';' || in->buf[i] == '=')
      r.buf[i] = ' ';
    else
      r.buf[i] = in->buf[i];
  
  return r;
}

byteblock encrypt_userdata(pool *p, const byteblock *plain)
{
  byteblock stripped = strip_metas(p, plain);
  byteblock prefixed = byteblock_concat(p, &prefix, &stripped);
  byteblock total = byteblock_concat(p, &prefixed, &suffix);
  byteblock padded = pkcs7_pad(p, &total, 16);
  return rijndael_cbc_encrypt(p, &padded, &key, &key);
}

int mem_contains(const uint8_t *haystack, size_t sz, const char *needle)
{
  size_t needlesz = strlen(needle);
  if (needlesz > sz)
    return 0;
  
  for (size_t i = 0; i < sz - needlesz; i++)
  {
    if (memcmp(haystack + i, needle, needlesz) == 0)
      return 1;
  }
  
  return 0;
}

int high_ascii(const byteblock *p)
{
  for (size_t i = 0; i < p->len; i++)
    if (p->buf[i] & 0x80)
      return 1;
  return 0;
}

int is_admin(pool *p, const byteblock *cipher, byteblock *errout)
{
  byteblock plain = rijndael_cbc_decrypt(p, cipher, &key, &key);
  if (high_ascii(&plain))
  {
    if (errout) *errout = plain;
    return 0;
  } else
    return mem_contains(plain.buf, plain.len, magic);
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 1);
  
  random_init();
  byteblock test = { (void *) "joe", 3 };
  byteblock token = encrypt_userdata(p, &test);
  assert(!is_admin(p, &token, NULL));
  
  while (1)
  {
    byteblock plain = byteblock_random(p, AES_BLOCKSIZE * 3);
    plain.buf[0] |= 0x80; /* ensure error */
    byteblock cipher = encrypt_userdata(p, &plain);
    
    uint8_t attackbuf[AES_BLOCKSIZE * 3];
    byteblock attack = { attackbuf, sizeof attackbuf };
    
    memcpy(attackbuf, plain.buf, AES_BLOCKSIZE);
    memset(attackbuf + AES_BLOCKSIZE, 0, AES_BLOCKSIZE);
    memcpy(attackbuf + AES_BLOCKSIZE + AES_BLOCKSIZE, plain.buf, AES_BLOCKSIZE);
    
    byteblock err = { NULL, 0 };
    is_admin(p, &attack, &err);
    if (err.buf == NULL)
      continue;
    
    for (size_t i = 0; i < AES_BLOCKSIZE; i++)
      err.buf[i] ^= err.buf[i + AES_BLOCKSIZE * 2];
    
    assert(memcmp(err.buf, key.buf, AES_BLOCKSIZE) == 0);
    break;
  }
  
  printf("ok\n");
  
  p->finish(p);
  return 0;
}