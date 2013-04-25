#include "util-1.h"
#include "mt19937.h"

#define CHECK 5

uint32_t parse_u32(const char *v)
{
  return strtoul(v, NULL, 10);
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  
  argv++;
  argc--;
  assert(argc == MT19937_N + CHECK);

  uint32_t state[MT19937_N];

  for (size_t i = 0; i < MT19937_N; i++)
    state[i] = parse_u32(argv[i]);

  for (size_t i = 0; i < MT19937_N; i++)
    state[i] = mt19937_untemper(state[i]);

  mt19937 st = mt19937_init(0);
  memcpy(st.mt, state, sizeof state);

  for (int i = 0; i < CHECK; i++)
  {
    uint32_t target = parse_u32(argv[MT19937_N + i]);
    assert(target == mt19937_extract(&st));
  }

  printf("ok\n");
  
  p->finish(p);
  return 0;
}
