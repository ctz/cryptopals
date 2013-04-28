#include "util-2.h"
#include "sph/sph_sha1.h"

uint32_t be32(const uint8_t *p)
{
  return p[0] << 24 |
         p[1] << 16 |
         p[2] << 8 |
         p[3];
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 4);
  int padlen = atoi(argv[1]);
  byteblock sig = from_hex(p, argv[2]);
  byteblock add = from_hex(p, argv[3]);

  sph_sha1_context ctx;
  sph_sha1_init(&ctx);
  
  /* trash internal state with actual hash, and adjust count */
  ctx.val[0] = be32(sig.buf + 0);
  ctx.val[1] = be32(sig.buf + 4);
  ctx.val[2] = be32(sig.buf + 8);
  ctx.val[3] = be32(sig.buf + 12);
  ctx.val[4] = be32(sig.buf + 16);
  
  ctx.count = padlen;
  ctx.count += 64 - (ctx.count % 64);
  
  /* now we can continue hash... */
  sph_sha1(&ctx, add.buf, add.len);
  
  uint8_t hashbuf[20];
  sph_sha1_close(&ctx, hashbuf);
  
  byteblock hash = { hashbuf, sizeof hashbuf };

  printf("%s\n", to_hex(p, &hash));
  p->finish(p);
  return 0;
}
