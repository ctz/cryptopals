#include "util-2.h"
#include "cipher.h"

static byteblock encrypt(pool *p, const byteblock *plain, const byteblock *key, byteblock *iv)
{
  byteblock padded = pkcs7_pad(p, plain, AES_BLOCKSIZE);
  *iv = byteblock_random(p, AES_BLOCKSIZE);
  return rijndael_cbc_encrypt(p, &padded, key, iv);
}

static int padding_ok(pool *p, const byteblock *cipher, const byteblock *iv, const byteblock *key)
{
  byteblock plain = rijndael_cbc_decrypt(p, cipher, key, iv);
  return pkcs7_padding_ok(&plain);
}

static byteblock attack_block(pool *p, const byteblock *cipher, size_t block, const byteblock *origiv, const byteblock *key)
{
  uint8_t ivbuf[AES_BLOCKSIZE] = { 0 };
  byteblock iv = { ivbuf, sizeof ivbuf };
  uint8_t buf[AES_BLOCKSIZE] = { 0 };
  byteblock work = { buf, sizeof buf };

  uint8_t found[AES_BLOCKSIZE] = { 0 };
  
  /* get target block */
  uint8_t *targblock = cipher->buf + block * AES_BLOCKSIZE;
  uint8_t *prevblock = block == 0 ? origiv->buf : (targblock - AES_BLOCKSIZE);
  memcpy(buf, targblock, AES_BLOCKSIZE);
  
  random_fill(iv.buf, iv.len);

  /* attack bytes from last backwards */
  for (size_t i = 0; i < AES_BLOCKSIZE; i++)
  {
    uint8_t padbyte = i + 1;
    if (i)
      for (size_t j = 0; j < i; j++)
        ivbuf[AES_BLOCKSIZE - 1 - j] = prevblock[AES_BLOCKSIZE - 1 - j] ^ found[AES_BLOCKSIZE - 1 - j] ^ padbyte;

    for (int byte = 0; byte < 256; byte++)
    {
      ivbuf[AES_BLOCKSIZE - 1 - i] = byte;

      if (padding_ok(p, &work, &iv, key))
      {
        uint8_t decrypted = prevblock[AES_BLOCKSIZE - 1 - i] ^ byte ^ padbyte;
        found[AES_BLOCKSIZE - 1 - i] = decrypted;
        break;
      }
    }
  }

  byteblock out = { p->alloc(p, AES_BLOCKSIZE), AES_BLOCKSIZE };
  memcpy(out.buf, found, AES_BLOCKSIZE);
  return out;
}

static byteblock recover_plaintext(pool *p, const byteblock *cipher, const byteblock *iv, const byteblock *key)
{
  assert(padding_ok(p, cipher, iv, key));

  byteblock out = { p->alloc(p, cipher->len), cipher->len };

  assert(out.len % AES_BLOCKSIZE == 0);

  for (size_t i = 0; i < out.len / AES_BLOCKSIZE; i++)
  {
    byteblock block = attack_block(p, cipher, i, iv, key);
    memcpy(out.buf + i * AES_BLOCKSIZE, block.buf, block.len);
  }

  return out;
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 2);
  byteblock plain = from_base64(p, argv[1]);
  random_init();

  uint8_t keybuf[16] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  byteblock key = { keybuf, sizeof keybuf };
  byteblock iv;
  byteblock cipher = encrypt(p, &plain, &key, &iv);

  byteblock recovered = recover_plaintext(p, &cipher, &iv, &key);
  byteblock unpad = pkcs7_unpad(p, &recovered);

  printf("%s\n", to_ascii(p, &unpad));
  p->finish(p);
  return 0;
}
