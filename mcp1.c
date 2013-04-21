#include "util-1.h"

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 2);
  byteblock b = from_hex(p, argv[1]);
  printf("%s\n", to_base64(p, &b));
  p->finish(p);
  return 0;
}