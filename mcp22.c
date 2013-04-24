#include "util-1.h"
#include "mt19937.h"

uint32_t first_output_given_seed(uint32_t seed)
{
  mt19937 mt = mt19937_init(seed);
  return mt19937_extract(&mt);
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  
  int basis = (int) time(NULL);
  
  assert(argc == 1);
  
  random_init();
  int offset = rand() % 10000;
  
  uint32_t target = first_output_given_seed(basis - offset);
  
  // we have our target, just search!
  uint32_t check = basis;
  while (1)
  {
    if (first_output_given_seed(check) == target)
    {
      printf("ok\n");
      break;
    }
    check--;
  }
  
  p->finish(p);
  return 0;
}
