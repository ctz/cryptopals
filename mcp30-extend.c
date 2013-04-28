#include "util-2.h"
#include "sph/sph_md4.h"

uint32_t le32(const uint8_t *p)
{
  return p[3] << 24 |
         p[2] << 16 |
         p[1] << 8 |
         p[0];
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 4);
  int padlen = atoi(argv[1]);
  byteblock sig = from_hex(p, argv[2]);
  byteblock add = from_hex(p, argv[3]);

  sph_md4_context ctx;
  sph_md4_init(&ctx);
  
  /* trash internal state with actual hash, and adjust count */
  ctx.val[0] = le32(sig.buf + 0);
  ctx.val[1] = le32(sig.buf + 4);
  ctx.val[2] = le32(sig.buf + 8);
  ctx.val[3] = le32(sig.buf + 12);
  
  ctx.count = padlen;
  ctx.count += 64 - (ctx.count % 64);
  
  /* now we can continue hash... */
  sph_md4(&ctx, add.buf, add.len);
  
  uint8_t hashbuf[16];
  sph_md4_close(&ctx, hashbuf);
  
  byteblock hash = { hashbuf, sizeof hashbuf };

  printf("%s\n", to_hex(p, &hash));
  p->finish(p);
  return 0;
}
