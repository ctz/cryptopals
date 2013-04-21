#include "util-1.h"

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 3);
  byteblock left = from_hex(p, argv[1]);
  byteblock right = from_hex(p, argv[2]);
  byteblock xor = byteblock_xor(p, &left, &right);
  printf("%s\n", to_hex(p, &xor));
  p->finish(p);
  return 0;
}