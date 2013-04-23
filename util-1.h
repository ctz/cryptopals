#ifndef UTIL_1_H
#define UTIL_1_H

#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>

typedef struct block
{
  size_t ulen;
  struct block *next;
  uint8_t u[1];
} block;

typedef struct pool
{
  block *blocks;
  void * (*alloc)(struct pool *p, size_t nbytes);
  void (*finish)(struct pool *p);
} pool;

void * pool_alloc(pool *p, size_t nbytes)
{
  size_t sz = nbytes + sizeof(block);
  assert(sz > nbytes);
  block *b = malloc(sz);
  if (!b)
  {
    perror("allocation failed");
    abort();
  }
  memset(b, 0, sz);
  
  b->ulen = nbytes;
  b->next = p->blocks;
  p->blocks = b;
  return b->u;
}

void pool_report(const pool *p)
{
  size_t nblocks = 0;
  size_t nbytes = 0;

  for (block *b = p->blocks;
       b != NULL;
       b = b->next)
  {
    nblocks++;
    nbytes += b->ulen;
  }
  
  printf("pool has %zu bytes in %zu blocks\n", nbytes, nblocks);
}

void pool_finish(pool *p)
{
  for (block *b = p->blocks;
       b != NULL;
       )
  {
    block *next = b->next;
    free(b);
    b = next;
  }
  
  memset(p, 0, sizeof(*p));
}

pool pool_create(void)
{
  pool r = { NULL, pool_alloc, pool_finish };
  return r;
}

typedef struct
{
  uint8_t *buf;
  size_t len;
} byteblock;

// return x ^ y. x may be longer than y, in which case y wraps.
byteblock byteblock_xor(pool *p, const byteblock *x, const byteblock *y)
{
  assert(x->len >= y->len);
  byteblock out = { p->alloc(p, x->len), x->len };
  for (size_t i = 0; i < x->len; i++)
    out.buf[i] = x->buf[i] ^ y->buf[i % y->len];
  return out;
}

int byte_hamming(uint8_t x, uint8_t y)
{
#define BIT(n) ( (x & (1 << n)) == (y & (1 << n)) ? 0 : 1 )
  return BIT(7) + BIT(6) + BIT(5) + BIT(4) + BIT(3) + BIT(2) + BIT(1) + BIT(0);
#undef BIT
}

int byteblock_hamming(const byteblock *x, const byteblock *y)
{
  assert(x->len == y->len);
  int r = 0;
  for (size_t i = 0; i < x->len; i++)
    r += byte_hamming(x->buf[i], y->buf[i]);
  return r;
}

byteblock byteblock_concat(pool *p, const byteblock *x, const byteblock *y)
{
  size_t len = x->len + y->len;
  byteblock out = { p->alloc(p, len), len };
  memcpy(out.buf, x->buf, x->len);
  memcpy(out.buf + x->len, y->buf, y->len);
  return out;
}

void random_init(void)
{
  srand((int) time(NULL));
}

void random_fill(uint8_t *out, size_t len)
{
  for (size_t i = 0; i < len; i++)
    out[i] = rand() & 0xff;
}

byteblock byteblock_random(pool *p, size_t len)
{
  byteblock out = { p->alloc(p, len), len };
  random_fill(out.buf, out.len);
  return out;
}

const char *hex_table = "0123456789abcdef";
const char *b64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char b64_pad = '=';

uint8_t hex_nibble(int v)
{
  v = tolower(v);
  if (v >= '0' && v <= '9')
    return v - '0';
  if (v >= 'a' && v <= 'f')
    return 10 + v - 'a';
  else {
    assert(!"invalid hex");
    abort();
  }
}

byteblock from_hex(pool *p, const char *hex)
{
  size_t hlen = strlen(hex);
  assert(hlen % 2 == 0);
  size_t rlen = hlen / 2;
  
  byteblock out = { p->alloc(p, rlen), rlen };
  for (size_t i = 0; i < out.len; i++)
  {
    uint8_t v = hex_nibble(hex[i * 2]) << 4 |
                hex_nibble(hex[i * 2 + 1]);
    out.buf[i] = v;
  }
  return out;
}

const char *to_hex(pool *p, const byteblock *bytes)
{
  size_t hlen = bytes->len * 2 + 1;
  char *h = p->alloc(p, hlen);
  
  for (size_t i = 0; i < bytes->len; i++)
  {
    uint8_t b = bytes->buf[i];
    h[i * 2] = hex_table[b >> 4 & 0xf];
    h[i * 2 + 1] = hex_table[b & 0xf];
  }
  
  return h;
}

const char *to_ascii(pool *p, const byteblock *bytes)
{
  size_t alen = bytes->len + 1;
  char *h = p->alloc(p, alen);
  
  for (size_t i = 0; i < bytes->len; i++)
  {
    if (isprint(bytes->buf[i]))
      h[i] = bytes->buf[i];
    else
      h[i] = '.';
  }
  
  return h;
}

void b64_encode_triple(uint8_t buf[3], char out[4])
{
  out[0] = b64_table[buf[0] >> 2];
  out[1] = b64_table[(buf[0] & 0x3) << 4 | (buf[1] >> 4)];
  out[2] = b64_table[(buf[1] & 0xf) << 2 | (buf[2] >> 6)];
  out[3] = b64_table[buf[2] & 0x3f];
}

size_t min(size_t x, size_t y)
{
  return x > y ? y : x;
}

const char *to_base64(pool *p, const byteblock *bytes)
{
  size_t triples = (bytes->len + 2) / 3;
  size_t blen = triples * 4 + 1;
  char *b = p->alloc(p, blen);
  
  for (size_t i = 0; i < triples; i++)
  {
    size_t offs = i * 3;
    size_t copy = min(3, bytes->len - offs);
    uint8_t buf[3] = { 0 };
    
    switch (copy)
    {
      case 3:
        buf[2] = bytes->buf[offs + 2];
        /* fallthru */
      case 2:
        buf[1] = bytes->buf[offs + 1];
        /* fallthru */
      case 1:
        buf[0] = bytes->buf[offs + 0];
    }
    
    b64_encode_triple(buf, &b[i * 4]);
    
    /* pad if we don't have a full block */
    switch (copy)
    {
      case 1:
        b[i * 4 + 2] = b64_pad;
        /* fallthru */
      case 2:
        b[i * 4 + 3] = b64_pad;
    }
  }
  
  return b;
}

char next_b64(const char **p)
{
  while (1)
  {
    char c = **p;
    if (c)
      *p = *p + 1;
    switch (c)
    {
      case '\n':
      case '\r':
      case '\t':
      case ' ':
        continue;
      
      case 0:
        return 0;
      
      default:
        return c;
    }
  }
}

uint8_t b64_decode_char(char x)
{
  if (x == b64_pad)
    return 0;
  
  const char *offs = strchr(b64_table, x);
  assert(offs != NULL);
  return offs - b64_table;
}

void b64_decode_triple(char a, char b, char c, char d, uint8_t out[3])
{
  uint8_t xa = b64_decode_char(a);
  uint8_t xb = b64_decode_char(b);
  uint8_t xc = b64_decode_char(c);
  uint8_t xd = b64_decode_char(d);
  
  out[0] = (xa << 2) | (xb >> 4);
  out[1] = ((xb & 0xf) << 4) | (xc >> 2);
  out[2] = ((xc & 0x3) << 6) | xd;
}

byteblock from_base64(pool *p, const char *b64)
{
  size_t b64len = strlen(b64);
  size_t quads = (b64len + 3) / 4;  // probably over estimate
  
  byteblock out = { p->alloc(p, quads * 3), 0 };
  for (size_t q = 0; q < quads; q++)
  {
    char a = next_b64(&b64);
    char b = next_b64(&b64);
    char c = next_b64(&b64);
    char d = next_b64(&b64);
    
    if (a == 0 && b == 0 && c == 0 && d == 0)
      break; // eof
    
    b64_decode_triple(a, b, c, d, &out.buf[q * 3]);
    
    out.len += 3;
    
    if (d == b64_pad)
      out.len -= 1;
    if (c == b64_pad)
      out.len -= 1;
  }
  
  return out;
}

// common ascii characters, in order of descending frequency(ish)
const char *english_letter_scores = " etaonrishd.,\nlfcmugypwbvkjxqz-_!'\"";

int score_english_char(uint8_t c)
{
  int uppercase_punishment = 0;
  
  if (isupper(c))
  {
    // slightly punish uppercase letters
    c = tolower(c);
    uppercase_punishment = 5;
  }
  
  const char *where = strchr(english_letter_scores, c);
  if (where == NULL)
    return 255; // probably control or weird character: punish
  else
    return uppercase_punishment + (where - english_letter_scores);
}

// low scores reflect byteblocks containing probably-english ASCII text
int score_english(const byteblock *bytes)
{
  int r = 0;
  
  for (size_t i = 0; i < bytes->len; i++)
    r += score_english_char(bytes->buf[i]);
  
  return r;
}

#endif
