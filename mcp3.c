#include "util.h"

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  byteblock candidates[256];
  
  assert(argc == 2);
  
  byteblock cipher = from_hex(p, argv[1]);
  
  for (int i = 0; i < 256; i++)
  {
    uint8_t byte = (uint8_t) i;
    byteblock key = { &byte, 1 };
    candidates[i] = byteblock_xor(p, &cipher, &key);
  }
  
  int lowest_score = INT_MAX;
  int lowest_index = 0;
  
  for (int i = 0; i < 256; i++)
  {
    int score = score_english(&candidates[i]);
    if (score < lowest_score)
    {
      lowest_score = score;
      lowest_index = i;
    }
  }
  
  printf("key: %02x, msg: %s\n", lowest_index, to_ascii(p, &candidates[lowest_index]));
  
  p->finish(p);
  return 0;
}