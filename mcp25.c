#include "util-1.h"
#include "cipher.h"

#define BLOCKS(n) ((n + AES_BLOCKSIZE - 1) / AES_BLOCKSIZE)

byteblock edit(pool *p, const byteblock *cipher, const byteblock *key, const byteblock *nonce,
               size_t offset, const byteblock *newplain)
{
  size_t byteoffs = offset * AES_BLOCKSIZE;
  size_t piecesize = min(AES_BLOCKSIZE, cipher->len - byteoffs);
  assert(offset <= BLOCKS(cipher->len));
  
  byteblock plain = rijndael_ctr_process(p, key, nonce, cipher);
  byteblock prefix = { plain.buf, byteoffs };
  byteblock suffix = { plain.buf + byteoffs + piecesize, plain.len - byteoffs - piecesize };
  byteblock tmp = byteblock_concat(p, &prefix, newplain);
  tmp = byteblock_concat(p, &tmp, &suffix);
  tmp = rijndael_ctr_process(p, key, nonce, &tmp);
  
  byteblock ret = { tmp.buf + byteoffs, piecesize };
  return ret;
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 2);
  
  byteblock plain = from_base64(p, argv[1]);
  byteblock key = byteblock_random(p, 16);
  byteblock nonce = byteblock_random(p, 8);
  byteblock cipher = rijndael_ctr_process(p, &key, &nonce, &plain);
  
  /* now recover plain from cipher, using edit.
   * assume same nonce(?):
   * "edit(ciphertext, key, offet, newtext)".
   * 
   */
  byteblock recovered = { p->alloc(p, plain.len), plain.len };
  uint8_t zerobuf[AES_BLOCKSIZE] = { 0 };
  byteblock zero = { zerobuf, sizeof zerobuf };
  
  for (size_t offs = 0; offs < BLOCKS(plain.len); offs++)
  {
    size_t idx = offs * AES_BLOCKSIZE;
    size_t piecesize = cipher.len - idx;
    byteblock newblock = edit(p, &cipher, &key, &nonce, offs, &zero);
    byteblock cipherblock = { cipher.buf + idx, piecesize };
    byteblock plainblock = byteblock_xor(p, &cipherblock, &newblock);
    
    memcpy(recovered.buf + idx, plainblock.buf, plainblock.len);
  }
  
  printf("%s\n", to_ascii(p, &recovered));
  p->finish(p);
  return 0;
}