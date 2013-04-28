#include "util-2.h"
#include "sph/sph_sha1.h"

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 3);
  byteblock key = from_hex(p, argv[1]);
  byteblock data = from_hex(p, argv[2]);

  sph_sha1_context ctx;
  sph_sha1_init(&ctx);
  sph_sha1(&ctx, key.buf, key.len);
  sph_sha1(&ctx, data.buf, data.len);
  
  uint8_t hashbuf[20];
  sph_sha1_close(&ctx, hashbuf);
  
  byteblock hash = { hashbuf, sizeof hashbuf };

  printf("%s\n", to_hex(p, &hash));
  p->finish(p);
  return 0;
}
