#include "util-1.h"

#define BLKSZ 16

static int contains_other_than_at(const byteblock *b, const uint8_t *buf, size_t index)
{
  for (size_t i = 0; i < b->len; i += BLKSZ)
  {
    if (i == index)
      continue;
    if (memcmp(&b->buf[i], buf, BLKSZ) == 0)
      return 1;
  }
  
  return 0;
}

static int count_duplicated_blocks(const byteblock *b)
{
  int dupes = 0;

  for (size_t i = 0; i < b->len; i += BLKSZ)
  {
    if (contains_other_than_at(b, &b->buf[i], i))
      dupes++;
  }
  
  return dupes;
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc >= 1);
  
  int most_dupe_blocks = 0;
  int best_index = 0;
  byteblock best_cipher;
  
  for (int i = 1; i < argc; i++)
  {
    byteblock cipher = from_hex(p, argv[i]);
    int dupes = count_duplicated_blocks(&cipher);
    
    if (dupes > most_dupe_blocks)
    {
      most_dupe_blocks = dupes;
      best_index = i;
      best_cipher = cipher;
    }
  }
  
  printf("index: %d, cipher: %s\n", best_index - 1, to_hex(p, &best_cipher));
  
  p->finish(p);
  return 0;
}