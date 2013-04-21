#include "util-1.h"
#include "cipher.h"

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 3);
  byteblock cipher = from_base64(p, argv[1]);
  byteblock key = { (void *) argv[2], strlen(argv[2]) };
  byteblock plain = rijndael_ecb_decrypt(p, &cipher, &key);
  
  printf("%s\n", to_ascii(p, &plain));
  p->finish(p);
  return 0;
}