#include "util.h"

typedef struct
{
  byteblock cipher;
  byteblock candidates[256];
} answer;

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  
  assert(argc >= 1);
  
  size_t nanswers = argc - 1;
  answer *answers = p->alloc(p, sizeof(answer) * nanswers);
  
  for (size_t a = 0; a < nanswers; a++)
  {
    answer *ans = &answers[a];
    ans->cipher = from_hex(p, argv[1 + a]);
  
    for (int i = 0; i < 256; i++)
    {
      uint8_t byte = (uint8_t) i;
      byteblock key = { &byte, 1 };
      ans->candidates[i] = byteblock_xor(p, &ans->cipher, &key);
    }
  }
  
  int best_score = INT_MAX;
  int best_index = -1;
  answer *best_answer = NULL;
  
  for (size_t a = 0; a < nanswers; a++)
  {
    answer *ans = &answers[a];
    
    for (int i = 0; i < 256; i++)
    {
      int score = score_english(&ans->candidates[i]);
      if (score < best_score)
      {
        best_score = score;
        best_index = i;
        best_answer = ans;
      }
    }
  }
  
  if (best_answer)
    printf("cipher: %s, key: %02x, msg: %s\n",
           to_hex(p, &best_answer->cipher),
           best_index,
           to_ascii(p, &best_answer->candidates[best_index]));
  else
    printf("cannot decide\n");
  
  p->finish(p);
  return 0;
}