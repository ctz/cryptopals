#include "util-2.h"
#include "cipher.h"

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 4);
  byteblock data = from_base64(p, argv[1]);
  byteblock nonce = from_hex(p, argv[2]);
  byteblock key = from_hex(p, argv[3]);

  byteblock result = rijndael_ctr_process(p, &key, &nonce, &data);

  printf("%s\n", to_ascii(p, &result));
  p->finish(p);
  return 0;
}
