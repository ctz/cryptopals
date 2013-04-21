#include "util.h"
#include "rijndael.h"

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 3);
  byteblock cipher = from_base64(p, argv[1]);
  byteblock key = { (void *) argv[2], strlen(argv[2]) };
  byteblock plain = { p->alloc(p, cipher.len), cipher.len };
  
  assert(cipher.len % 16 == 0);
  
  uint32_t roundkeys[4 * (MAXNR + 1)];
  rijndaelKeySetupDec(roundkeys, key.buf, key.len * 8);
  
  for (size_t i = 0; i < cipher.len; i += 16)
  {
    rijndaelDecrypt(roundkeys, RIJNDAEL_128KEY_ROUNDS, &cipher.buf[i], &plain.buf[i]);
  }
  
  printf("%s\n", to_ascii(p, &plain));
  p->finish(p);
  return 0;
}