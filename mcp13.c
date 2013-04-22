#include "util-2.h"
#include "cipher.h"

byteblock key = { (void *) "qwertyqwerty1234", 16 };
const char *emailaddr = "qweasdasdd@asdasdsad.com";

const char * profile_for(pool *p, const char *mail)
{
  keyval role = { { (void *) "role", 4 }, { (void *) "user", 4 }, NULL };
  keyval uid = { { (void *) "uid", 3 }, { (void *) "1", 1 }, &role };
  keyval email = { { (void *) "email", 5 }, { (void *) mail, strlen(mail) }, &uid };
  map m = { &email };
  
  return format_cookie(p, &m);
}

byteblock encrypted_profile_for(pool *p, const char *mail, const byteblock *key)
{
  const char *pt = profile_for(p, mail);
  byteblock ptbb = { (void *) pt, strlen(pt) };
  byteblock padded = pkcs7_pad(p, &ptbb, 16);
  
  return rijndael_ecb_encrypt(p, &padded, key);
}

map decrypt_profile(pool *p, const byteblock *cipher, const byteblock *key)
{
  byteblock padded = rijndael_ecb_decrypt(p, cipher, key);
  byteblock plain = pkcs7_unpad(p, &padded);
  char *cstr = p->alloc(p, plain.len + 1);
  memcpy(cstr, plain.buf, plain.len);
  cstr[plain.len] = 0;
  return parse_cookie(p, cstr);
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };
  assert(argc == 1);
  
  /* broken run: first get an encryption of 'admin<padding>' in a block alone.
   * we need blocks:
   * email=<dontcare>... || admin<padding> || <dontcare> */
  const char *probe = "0123456789admin\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b";
  byteblock probect = encrypted_profile_for(p, probe, &key);
  map profile = decrypt_profile(p, &probect, &key);
  
  uint8_t magicblock[16];
  memcpy(magicblock, probect.buf + 16, 16);
  
  /* now we need blocks:
   * email=realemail&uid=1&role= || user
   * 'realemail' needs to be exactly -2 in length mod 16
   */
  const char *exploit = "hack@hacks.com";
  byteblock exploitct = encrypted_profile_for(p, exploit, &key);
  memcpy(exploitct.buf + exploitct.len - 16, magicblock, 16);
  profile = decrypt_profile(p, &exploitct, &key);
  printf("hacked: %s\n", format_cookie(p, &profile));
  
  p->finish(p);
  return 0;
}