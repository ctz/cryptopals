#include "util-2.h"
#include "cipher.h"

byteblock key = { (void *) "123qwerty123qwer", 16 };
byteblock iv = { (void *) "-=12356-=12356", 16 };

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
  return rijndael_cbc_encrypt(p, &padded, &key, &iv);
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

int is_admin(pool *p, const byteblock *cipher)
{
  byteblock padded = rijndael_cbc_decrypt(p, cipher, &key, &iv);
  byteblock plain = pkcs7_unpad(p, &padded);
  return mem_contains(plain.buf, plain.len, magic);
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 1);
  
  byteblock test = { (void *) "joe", 3 };
  byteblock token = encrypt_userdata(p, &test);
  assert(!is_admin(p, &token));
  
  byteblock test2 = { (void *) magic, strlen(magic) };
  token = encrypt_userdata(p, &test2);
  assert(!is_admin(p, &token));
  
  /* strategy: first, we need a userdata with two blocks:
   *  dontcare || ;admin=true; ...
   * we can't directly encode this, but can get close
   * and then flip the bits in the dontcare block:
   * :admin<true:
   * ^     ^    ^
   * 0     6    11
   */
  uint8_t vector[32] = { 0 };
  memcpy(vector + 16, ":admin<true:", 12);
  byteblock vec = { vector, sizeof vector };
  token = encrypt_userdata(p, &vec);
  token.buf[32 + 0] ^= 0x01;
  token.buf[32 + 6] ^= 0x01;
  token.buf[32 + 11] ^= 0x01;
  assert(is_admin(p, &token));
  
  printf("ok\n");
  
  p->finish(p);
  return 0;
}