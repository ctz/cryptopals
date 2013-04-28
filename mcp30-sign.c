#include "util-2.h"
#include "sph/sph_md4.h"

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 3);
  byteblock key = from_hex(p, argv[1]);
  byteblock data = from_hex(p, argv[2]);

  sph_md4_context ctx;
  sph_md4_init(&ctx);
  sph_md4(&ctx, key.buf, key.len);
  sph_md4(&ctx, data.buf, data.len);
  
  uint8_t hashbuf[16];
  sph_md4_close(&ctx, hashbuf);
  
  byteblock hash = { hashbuf, sizeof hashbuf };

  printf("%s\n", to_hex(p, &hash));
  p->finish(p);
  return 0;
}
