#include "util-1.h"
#include "mt19937.h"

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  
  assert(argc == 3);
  uint32_t seed = atoi(argv[1]);
  size_t count = atoi(argv[2]);
  
  mt19937 mt = mt19937_init(seed);
  
  while (count--)
  {
    printf("%u", mt19937_extract(&mt));
    if (count)
      printf(" ");
  }
  printf("\n");
  
  p->finish(p);
  return 0;
}
