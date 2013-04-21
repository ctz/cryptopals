
#include "rijndael.h"

void xor_inplace(uint8_t *x, const uint8_t *y, size_t n)
{
  for (size_t i = 0; i < n; i++)
    x[i] ^= y[i];
}

byteblock rijndael_cbc_encrypt(pool *p, const byteblock *plain, const byteblock *key, const byteblock *iv)
{
  assert(key->len == 16);
  assert(iv->len == 16);
  assert(plain->len % 16 == 0);
  
  uint8_t ivbuf[16];
  memcpy(ivbuf, iv->buf, 16);
  
  byteblock cipher = { p->alloc(p, plain->len), 0 };
  
  size_t blocks = plain->len / 16;
  uint32_t subkeys[RIJNDAEL_CTX];
  rijndaelKeySetupEnc(subkeys, key->buf, key->len * 8);
  
  for (size_t i = 0; i < blocks; i++)
  {
    const uint8_t *pt = &plain->buf[16 * i];
    uint8_t *ct = &cipher.buf[16 * i];
    
    xor_inplace(ivbuf, pt, 16);
    
    rijndaelEncrypt(subkeys, RIJNDAEL_128KEY_ROUNDS, ivbuf, ct);
    memcpy(ivbuf, ct, 16);
    
    cipher.len += 16;
  }
  
  return cipher;
}


byteblock rijndael_cbc_decrypt(pool *p, const byteblock *cipher, const byteblock *key, const byteblock *iv)
{
  assert(key->len == 16);
  assert(iv->len == 16);
  assert(cipher->len % 16 == 0);
  uint8_t ivbuf[16];
  memcpy(ivbuf, iv->buf, 16);
  
  byteblock plain = { p->alloc(p, cipher->len), 0 };
  
  size_t blocks = cipher->len / 16;
  uint32_t subkeys[RIJNDAEL_CTX];
  rijndaelKeySetupDec(subkeys, key->buf, key->len * 8);
  
  for (size_t i = 0; i < blocks; i++)
  {
    const uint8_t *ct = &cipher->buf[16 * i];
    uint8_t *pt = &plain.buf[16 * i];
    rijndaelDecrypt(subkeys, RIJNDAEL_128KEY_ROUNDS, ct, pt);
    xor_inplace(pt, ivbuf, 16);
    memcpy(ivbuf, ct, 16);
    plain.len += 16;
  }
  
  return plain;
}
