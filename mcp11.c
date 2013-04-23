#include "util-2.h"
#include "cipher.h"

enum mech
{
  CBC,
  ECB
};

byteblock add_prefix_suffix(pool *p, const byteblock *data)
{
  size_t prefix = rand() % 6 + 5;
  size_t suffix = rand() % 6 + 5;
  byteblock r = { p->alloc(p, data->len + prefix + suffix), data->len + prefix + suffix };
  
  random_fill(r.buf, prefix);
  memcpy(r.buf + prefix, data->buf, data->len);
  random_fill(r.buf + data->len + prefix, suffix);
  
  return pkcs7_pad(p, &r, 16);
}

byteblock random_encrypt(pool *p, const byteblock *plaintext, enum mech *actual)
{
  byteblock key = byteblock_random(p, 16);
  byteblock iv = byteblock_random(p, 16);
  
  byteblock plain = add_prefix_suffix(p, plaintext);
  
  if (rand() % 2 == 0)
  {
    *actual = CBC;
    return rijndael_cbc_encrypt(p, &plain, &key, &iv);
  } else {
    *actual = ECB;
    return rijndael_ecb_encrypt(p, &plain, &key);
  }
}

enum mech encryption_type_oracle(const byteblock *cipher)
{
  // first and last two blocks are unpredictable: check second and third
  return memcmp(&cipher->buf[16], &cipher->buf[32], 16) == 0 ? ECB : CBC;
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 1);
  
  random_init();
  
  // 4 blocks of zeroes
  uint8_t bytes[16 * 4] = { 0 };
  byteblock plain = { bytes, sizeof bytes };
  
  for (int round = 0; round < 65536; round++)
  {
    enum mech actual, detected;
    byteblock cipher = random_encrypt(p, &plain, &actual);
    detected = encryption_type_oracle(&cipher);
    assert(actual == detected);
  }
  
  printf("ok\n");
  
  p->finish(p);
  return 0;
}
