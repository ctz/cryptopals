#ifndef UTIL_2_H
#define UTIL_2_H

#include "util-1.h"

byteblock pkcs7_pad(pool *p, const byteblock *msg, size_t blocksize)
{
  size_t pad = blocksize - (msg->len % blocksize);
  byteblock r = { p->alloc(p, msg->len + pad), msg->len + pad };
  
  for (size_t i = 0; i < msg->len; i++)
    r.buf[i] = msg->buf[i];
  memset(r.buf + msg->len, pad, pad);
  return r;
}

int pkcs7_padding_ok(const byteblock *padded)
{
  if (padded->len == 0)
    return 0;
  
  uint8_t pad = padded->buf[padded->len - 1];
  if (pad > padded->len)
    return 0;
  
  for (size_t i = 0; i < pad; i++)
  {
    if (pad != padded->buf[padded->len - 1 - i])
      return 0;
  }
  
  return 1;
}

byteblock pkcs7_unpad(pool *p, const byteblock *padded)
{
  assert(pkcs7_padding_ok(padded));
  size_t pad = padded->buf[padded->len - 1];
  byteblock r = { p->alloc(p, padded->len - pad), padded->len - pad };
  memcpy(r.buf, padded->buf, r.len);
  return r;
}

typedef struct keyval
{
  byteblock key, val;
  struct keyval *next;
} keyval;

typedef struct
{
  keyval *pairs;
} map;

void add_pair(pool *p, map *r, keyval **pair, keyval ***next, byteblock **current, const char *c)
{
  keyval *pp = *pair;
  byteblock *bc = *current;
  
  if (pp && pp->key.len)
  {
    pp->next = **next;
    **next = pp;
    *next = &pp->next;
  }
  
  pp = *pair = p->alloc(p, sizeof(keyval));
  bc = *current = &pp->key;
  bc->buf = (uint8_t *) c;
  bc->len = 0;
}

map parse_cookie(pool *p, const char *c)
{
  map r = { NULL };
  keyval *pair = NULL;
  byteblock *current = NULL;
  keyval **next = &r.pairs;
  
  add_pair(p, &r, &pair, &next, &current, c);

  while (*c)
  {  
    switch (*c)
    {
      case '=':
        assert(current == &pair->key);
        current = &pair->val;
        current->buf = (uint8_t *) c + 1;
        break;
        
      case '&':
        add_pair(p, &r, &pair, &next, &current, c + 1);
        break;
        
      default:
        current->len++;
    }
    
    c++;
  }
  
  add_pair(p, &r, &pair, &next, &current, c);
  return r;
}

const char * format_cookie(pool *p, const map *m)
{
  size_t len = 1; // nul
  
  for (keyval *pair = m->pairs; pair; pair = pair->next)
  {
    len += pair->key.len + 1 + pair->val.len + 1;
  }
  
  char *r = p->alloc(p, len);
  char *ptr = r;
  
  for (keyval *pair = m->pairs; pair; pair = pair->next)
  {
    memcpy(ptr, pair->key.buf, pair->key.len);
    ptr += pair->key.len;
    *ptr++ = '=';
    
    memcpy(ptr, pair->val.buf, pair->val.len);
    ptr += pair->val.len;
    *ptr++ = pair->next ? '&' : 0;
  }
  
  return r;
}

#endif