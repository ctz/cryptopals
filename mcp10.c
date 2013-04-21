#include "util-2.h"
#include "cipher.h"

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 4);
  byteblock cipher = from_base64(p, argv[1]);
  byteblock key = from_hex(p, argv[2]);
  byteblock iv = from_hex(p, argv[3]);
  
  byteblock plain = rijndael_cbc_decrypt(p, &cipher, &key, &iv);
  printf("%s\n", to_ascii(p, &plain));
  
  p->finish(p);
  return 0;
}