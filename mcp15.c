#include "util-2.h"

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 3);
  byteblock b = from_hex(p, argv[1]);
  int blocksz = atoi(argv[2]);
  int e;
  
  if (pkcs7_padding_ok(&b))
  {
    byteblock unpad = pkcs7_unpad(p, &b);
    printf("%s\n", to_hex(p, &unpad));
    e = 0;
  } else {
    printf("error");
    e = 1;
  }
  p->finish(p);
  return e;
}