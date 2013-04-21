#include "util-2.h"

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 3);
  byteblock b = from_hex(p, argv[1]);
  int blocksize = atoi(argv[2]);
  assert(blocksize >= 1);
  
  byteblock padded = pkcs7_pad(p, &b, (size_t) blocksize);
  printf("%s\n", to_hex(p, &padded));
  
  p->finish(p);
  return 0;
}