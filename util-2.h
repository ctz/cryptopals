
#include "util-1.h"

byteblock pkcs7_pad(pool *p, const byteblock *msg, size_t blocksize)
{
  size_t pad = blocksize - (msg->len % blocksize);
  byteblock r = { p->alloc(p, msg->len + pad), msg->len + pad };
  
  for (size_t i = 0; i < msg->len; i++)
    r.buf[i] = msg->buf[i];
  memset(r.buf + msg->len, pad, pad);
  return r;
}