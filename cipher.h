#ifndef CIPHER_H
#define CIPHER_H

#include "rijndael.h"

#define AES_BLOCKSIZE 16
#define AES_KEYSIZE 16

void xor_inplace(uint8_t *x, const uint8_t *y, size_t n)
{
  for (size_t i = 0; i < n; i++)
    x[i] ^= y[i];
}

byteblock rijndael_cbc_encrypt(pool *p, const byteblock *plain, const byteblock *key, const byteblock *iv)
{
  assert(key->len == AES_KEYSIZE);
  assert(iv->len == AES_BLOCKSIZE);
  assert(plain->len % AES_BLOCKSIZE == 0);
  
  uint8_t ivbuf[AES_BLOCKSIZE];
  memcpy(ivbuf, iv->buf, AES_BLOCKSIZE);
  
  byteblock cipher = { p->alloc(p, plain->len), 0 };
  
  size_t blocks = plain->len / AES_BLOCKSIZE;
  uint32_t subkeys[RIJNDAEL_CTX];
  rijndaelKeySetupEnc(subkeys, key->buf, key->len * 8);
  
  for (size_t i = 0; i < blocks; i++)
  {
    const uint8_t *pt = &plain->buf[AES_BLOCKSIZE * i];
    uint8_t *ct = &cipher.buf[AES_BLOCKSIZE * i];
    
    xor_inplace(ivbuf, pt, AES_BLOCKSIZE);
    
    rijndaelEncrypt(subkeys, RIJNDAEL_128KEY_ROUNDS, ivbuf, ct);
    memcpy(ivbuf, ct, AES_BLOCKSIZE);
    
    cipher.len += AES_BLOCKSIZE;
  }
  
  return cipher;
}

byteblock rijndael_cbc_decrypt(pool *p, const byteblock *cipher, const byteblock *key, const byteblock *iv)
{
  assert(key->len == AES_KEYSIZE);
  assert(iv->len == AES_BLOCKSIZE);
  assert(cipher->len % AES_BLOCKSIZE == 0);
  uint8_t ivbuf[AES_BLOCKSIZE];
  memcpy(ivbuf, iv->buf, AES_BLOCKSIZE);
  
  byteblock plain = { p->alloc(p, cipher->len), 0 };
  
  size_t blocks = cipher->len / AES_BLOCKSIZE;
  uint32_t subkeys[RIJNDAEL_CTX];
  rijndaelKeySetupDec(subkeys, key->buf, key->len * 8);
  
  for (size_t i = 0; i < blocks; i++)
  {
    const uint8_t *ct = &cipher->buf[AES_BLOCKSIZE * i];
    uint8_t *pt = &plain.buf[AES_BLOCKSIZE * i];
    rijndaelDecrypt(subkeys, RIJNDAEL_128KEY_ROUNDS, ct, pt);
    xor_inplace(pt, ivbuf, AES_BLOCKSIZE);
    memcpy(ivbuf, ct, AES_BLOCKSIZE);
    plain.len += AES_BLOCKSIZE;
  }
  
  return plain;
}

void rijndael_ecb_encrypt_inplace(const byteblock *plain, const byteblock *key, const byteblock *out)
{
  uint32_t roundkeys[RIJNDAEL_CTX];
  rijndaelKeySetupEnc(roundkeys, key->buf, key->len * 8);
  
  for (size_t i = 0; i < plain->len; i += AES_BLOCKSIZE)
  {
    rijndaelEncrypt(roundkeys, RIJNDAEL_128KEY_ROUNDS, &plain->buf[i], &out->buf[i]);
  }
}

byteblock rijndael_ecb_encrypt(pool *p, const byteblock *plain, const byteblock *key)
{
  assert(key->len == AES_KEYSIZE);
  assert(plain->len % AES_BLOCKSIZE == 0);
  
  byteblock cipher = { p->alloc(p, plain->len), plain->len };
  rijndael_ecb_encrypt_inplace(plain, key, &cipher);
  return cipher;
}

byteblock rijndael_ecb_decrypt(pool *p, const byteblock *cipher, const byteblock *key)
{
  assert(key->len == AES_KEYSIZE);
  assert(cipher->len % AES_BLOCKSIZE == 0);
  
  byteblock plain = { p->alloc(p, cipher->len), cipher->len };
  
  uint32_t roundkeys[RIJNDAEL_CTX];
  rijndaelKeySetupDec(roundkeys, key->buf, key->len * 8);
  
  for (size_t i = 0; i < cipher->len; i += AES_BLOCKSIZE)
  {
    rijndaelDecrypt(roundkeys, RIJNDAEL_128KEY_ROUNDS, &cipher->buf[i], &plain.buf[i]);
  }
  
  return plain;
}

byteblock rijndael_ctr_process(pool *p, const byteblock *key, const byteblock *nonce, const byteblock *in)
{
  assert(key->len == AES_KEYSIZE);
  assert(nonce->len == 8);

  uint8_t block[AES_BLOCKSIZE] = { 0 };
  memcpy(block, nonce->buf, nonce->len);
  byteblock blbb = { block, sizeof block };

  uint8_t keystream[AES_BLOCKSIZE];
  byteblock ksbb = { keystream, sizeof keystream };

  byteblock out = { p->alloc(p, in->len), in->len };
  for (size_t i = 0; i < in->len; i++)
  {
    if (i % AES_BLOCKSIZE == 0)
    {
      rijndael_ecb_encrypt_inplace(&blbb, key, &ksbb);

      uint8_t *ctr = block + 8;
      while (++(*ctr) == 0)
        ctr++;
    }

    out.buf[i] = keystream[i % AES_BLOCKSIZE] ^ in->buf[i];
  }

  return out;
}

#endif
