#include "util-1.h"
#include "sph/sph_md4.h"

#define MD4_INITIAL { , 

static void md4_initial(uint32_t state[4])
{
  state[0] = 0x67452301u;
  state[1] = 0xEFCDAB89u;
  state[2] = 0x98BADCFEu;
  state[3] = 0x10325476u;
}

static void random_block(uint32_t block[16])
{
  random_fill((void *) block, 16 * 4);
}

#define F(B, C, D)     ((((C) ^ (D)) & (B)) ^ (D))
#define G(B, C, D)     (((D) & (C)) | (((D) | (C)) & (B)))
#define H(B, C, D)     ((B) ^ (C) ^ (D))
#define ROTL   SPH_ROTL32
#define ROTR   SPH_ROTR32
#define T32    SPH_T32

static unsigned bit(uint32_t word, unsigned b)
{
  return (word >> b) & 1;
}

static void first_round_properties(uint32_t state[4], uint32_t m[16])
{
  uint32_t A0 = state[0];
  uint32_t B0 = state[1];
  uint32_t C0 = state[2];
  uint32_t D0 = state[3];
  
  uint32_t TT;
  
#define EQL(VV, n) TT ^= (bit(TT, n-1) ^ bit(VV, n-1)) << (n-1)
#define INV(VV, n) TT ^= (bit(TT, n-1) ^ bit(VV, n-1) ^ 1) << (n-1)
#define SET(n) TT |= (1 << (n-1))
#define CLR(n) TT &= ~(1 << (n-1))
  
  /* a1: a1,7 = b0,7 */
  uint32_t A1 = TT = ROTL(T32(A0 + F(B0, C0, D0) + m[0]), 3);
  EQL(B0, 7);
  A1 = TT;
  m[0] = T32(ROTR(TT, 3) - F(B0, C0, D0) - A0);
    
  /* d1: d1,7 = 0
   *     d1,8 = a1,8
   *     d1,11 = a1,11
   */
  uint32_t D1 = TT = ROTL(T32(D0 + F(A1, B0, C0) + m[1]), 7);
  CLR(7);
  EQL(A1,8);
  EQL(A1,11);
  D1 = TT;
  m[1] = T32(ROTR(TT, 7) - F(A1, B0, C0) - D0);
  
  /* c1: c1,7 = 1
   *     c1,8 = 1
   *     c1,11 = 0
   *     c1,26 = d1,26
   */
  uint32_t C1 = TT = ROTL(SPH_T32(C0 + F(D1, A1, B0) + m[2]), 11);
  SET(7);
  SET(8);
  CLR(11);
  EQL(D1,26);
  C1 = TT;
  m[2] = T32(ROTR(TT, 11) - F(D1, A1, B0) - C0);
  
  /* b1: b1,7 = 1
   *     b1,8 = 0
   *     b1,11 = 0
   *     b1,26 = 0
   */
  uint32_t B1 = TT = ROTL(SPH_T32(B0 + F(C1, D1, A1) + m[3]), 19);
  SET(7);
  CLR(8);
  CLR(11);
  CLR(26);
  B1 = TT;
  m[3] = T32(ROTR(TT, 19) - F(C1, D1, A1) - B0);
  
  /* a2 a2,8 = 1, a2,11 = 1, a2,26 = 0, a2,14 = b1,14 */
  uint32_t A2 = TT = ROTL(T32(A1 + F(B1, C1, D1) + m[4]), 3);
  SET(8);
  SET(11);
  CLR(26);
  EQL(B1,14);
  A2 = TT;
  m[4] = T32(ROTR(TT, 3) - F(B1, C1, D1) - A1);
  
  /* d2 d2,14 = 0, d2,19 = a2,19, d2,20 = a2,20, d2,21 = a2,21, d2,22 = a2,22, d2,26 = 1 */
  uint32_t D2 = TT = ROTL(T32(D1 + F(A2, B1, C1) + m[5]), 7);
  CLR(14);
  EQL(A2,19);
  EQL(A2,20);
  EQL(A2,21);
  EQL(A2,22);
  SET(26);
  D2 = TT;
  m[5] = T32(ROTR(TT, 7) - F(A2, B1, C1) - D1);
  
  /* c2 c2,13 = d2,13, c2,14 = 0, c2,15 = d2,15, c2,19 = 0, c2,20 = 0, c2,21 = 1, c2,22 = 0 */
  uint32_t C2 = TT = ROTL(SPH_T32(C1 + F(D2, A2, B1) + m[6]), 11);
  EQL(D2,13);
  CLR(14);
  EQL(D2,15);
  CLR(19);
  CLR(20);
  SET(21);
  CLR(22);
  C2 = TT;
  m[6] = T32(ROTR(TT, 11) - F(D2, A2, B1) - C1);
  
  /* b2 b2,13 = 1, b2,14 = 1, b2,15 = 0, b2,17 = c2,17, b2,19 = 0, b2,20 = 0, b2,21 = 0
b2,22 = 0 */
  uint32_t B2 = TT = ROTL(SPH_T32(B1 + F(C2, D2, A2) + m[7]), 19);
  SET(13);
  SET(14);
  CLR(15);
  EQL(C2,17);
  CLR(19);
  CLR(20);
  CLR(21);
  B2 = TT;
  m[7] = T32(ROTR(TT, 19) - F(C2, D2, A2) - B1);
  
  /* a3 a3,13 = 1, a3,14 = 1, a3,15 = 1, a3,17 = 0, a3,19 = 0, a3,20 = 0, a3,21 = 0,
a3,23 = b2,23 a3,22 = 1, a3,26 = b2,26 */
  uint32_t A3 = TT = ROTL(T32(A2 + F(B2, C2, D2) + m[8]), 3);
  SET(13);
  SET(14);
  SET(15);
  CLR(17);
  CLR(19);
  CLR(20);
  CLR(21);
  EQL(B2,23);
  SET(22);
  EQL(B2,26);
  A3 = TT;
  m[8] = T32(ROTR(TT, 3) - F(B2, C2, D2) - A2);
  
  /* d3 d3,13 = 1, d3,14 = 1, d3,15 = 1, d3,17 = 0, d3,20 = 0, d3,21 = 1, d3,22 = 1, d3,23 = 0,
d3,26 = 1, d3,30 = a3,30 */
  uint32_t D3 = TT = ROTL(T32(D2 + F(A3, B2, C2) + m[9]), 7);
  SET(13);
  SET(14);
  SET(15);
  CLR(17);
  CLR(20);
  SET(21);
  SET(22);
  CLR(23);
  SET(26);
  EQL(A3,30);
  D3 = TT;
  m[9] = T32(ROTR(TT, 7) - F(A3, B2, C2) - D2);
  
  /* c3 c3,17 = 1, c3,20 = 0, c3,21 = 0, c3,22 = 0, c3,23 = 0, c3,26 = 0, c3,30 = 1, c3,32 = d3,32 */
  uint32_t C3 = TT = ROTL(SPH_T32(C2 + F(D3, A3, B2) + m[10]), 11);
  SET(17);
  CLR(20);
  CLR(21);
  CLR(22);
  CLR(23);
  CLR(26);
  SET(30);
  EQL(D3,32);
  C3 = TT;
  m[10] = T32(ROTR(TT, 11) - F(D3, A3, B2) - C2);
  
  /* b3 b3,20 = 0, b3,21 = 1, b3,22 = 1, b3,23 = c3,23, b3,26 = 1, b3,30 = 0, b3,32 = 0 */
  uint32_t B3 = TT = ROTL(SPH_T32(B2 + F(C3, D3, A3) + m[11]), 19);
  CLR(20);
  SET(21);
  SET(22);
  EQL(C3,23);
  SET(26);
  CLR(30);
  CLR(32);
  B3 = TT;
  m[11] = T32(ROTR(TT, 19) - F(C3, D3, A3) - B2);
  
  /* a4 a4,23 = 0, a4,26 = 0, a4,27 = b3,27, a4,29 = b3,29, a4,30 = 1, a4,32 = 0 */
  uint32_t A4 = TT = ROTL(T32(A3 + F(B3, C3, D3) + m[12]), 3);
  CLR(23);
  CLR(26);
  EQL(B3,27);
  EQL(B3,29);
  SET(30);
  CLR(32);
  A4 = TT;
  m[12] = T32(ROTR(TT, 3) - F(B3, C3, D3) - A3);
  
  /* d4 d4,23 = 0, d4,26 = 0, d4,27 = 1, d4,29 = 1, d4,30 = 0, d4,32 = 1 */
  uint32_t D4 = TT = ROTL(T32(D3 + F(A4, B3, C3) + m[13]), 7);
  CLR(23);
  CLR(26);
  SET(27);
  SET(29);
  CLR(30);
  SET(32);
  D4 = TT;
  m[13] = T32(ROTR(TT, 7) - F(A4, B3, C3) - D3);
  
  /* c4 c4,19 = d4,19, c4,23 = 1, c4,26 = 1, c4,27 = 0, c4,29 = 0, c4,30 = 0 */
  uint32_t C4 = TT = ROTL(SPH_T32(C3 + F(D4, A4, B3) + m[14]), 11);
  EQL(D4,19);
  SET(23);
  SET(26);
  CLR(27);
  CLR(29);
  CLR(30);
  C4 = TT;
  m[14] = T32(ROTR(TT, 11) - F(D4, A4, B3) - C3);
  
  /* b4 b4,19 = 0, b4,26 = c4,26 = 1, b4,27 = 1, b4,29 = 1, b4,30 = 0 */
  uint32_t B4 = TT = ROTL(T32(B3 + F(C4, D4, A4) + m[15]), 19);
  CLR(19);
  EQL(C4,26);
  SET(27);
  SET(29);
  CLR(30);
  B4 = TT;
  m[15] = T32(ROTR(TT, 19) - F(C4, D4, A4) - B3);

  /* a5 a5,19 = c4,19, a5,26 = 1, a5,27 = 0, a5,29 = 1, a5,32 = 1 */
  uint32_t A5 = TT = ROTL(T32(A4 + G(B4, C4, D4) + m[0] + SPH_C32(0x5A827999)), 3);
  EQL(C4,19);
  SET(26);
  CLR(27);
  SET(29);
  SET(32);
  A5 = TT;
  m[0] = T32(ROTR(TT, 3) - G(B4, C4, D4) - A4 - SPH_C32(0x5A827999));
  
  /* d5 d5,19 = a5,19, d5,26 = b4,26, d5,27 = b4,27, d5,29 = b4,29, d5,32 = b4,32 */
  uint32_t D5 = TT = ROTL(T32(D4 + G(A5, B4, C4) + m[4] + SPH_C32(0x5A827999)), 5);
  EQL(A5,19);
  EQL(B4,26);
  EQL(B4,27);
  EQL(B4,29);
  EQL(B4,32);
  D5 = TT;
  m[4] = T32(ROTR(TT, 5) - G(A5, B4, C4) - D4 - SPH_C32(0x5A827999));
  
  /* c5 c5,26 = d5,26, c5,27 = d5,27, c5,29 = d5,29, c5,30 = d5,30, c5,32 = d5,32 */
  uint32_t C5 = TT = ROTL(T32(C4 + G(D5, A5, B4) + m[8] + SPH_C32(0x5A827999)), 9);
  EQL(D5,26);
  EQL(D5,27);
  EQL(D5,29);
  EQL(D5,30);
  EQL(D5,32);
  C5 = TT;
  m[8] = T32(ROTR(TT, 9) - G(D5, A5, B4) - C4 - SPH_C32(0x5A827999));
  
  /* b5 b5,29 = c5,29, b5,30 = 1, b5,32 = 0 */
  uint32_t B5 = TT = ROTL(T32(B4 + G(C5, D5, A5) + m[12] + SPH_C32(0x5A827999)), 13);
  EQL(C5,29);
  SET(30);
  CLR(32);
  B5 = TT;
  m[12] = T32(ROTR(TT, 13) - G(C5, D5, A5) - B4 - SPH_C32(0x5A827999));
  
  /* a6 a6,29 = 1, a6,32 = 1 */
  uint32_t A6 = TT = ROTL(T32(A5 + G(B5, C5, D5) + m[1] + SPH_C32(0x5A827999)), 3);
  SET(29);
  SET(32);
  A6 = TT;
  m[1] = T32(ROTR(TT, 3) - G(B5, C5, D5) - A5 - SPH_C32(0x5A827999));
  
  /* d6 d6,29 = b5,29 */
  uint32_t D6 = TT = ROTL(T32(D5 + G(A6, B5, C5) + m[5] + SPH_C32(0x5A827999)), 5);
  EQL(B5,29);
  D6 = TT;
  m[5] = T32(ROTR(TT, 5) - G(A6, B5, C5) - D5 - SPH_C32(0x5A827999));
  
  /* c6 c6,29 = d6,29, c6,30 = d6,30 + 1, c6,32 = d6,32 + 1 */
  uint32_t C6 = TT = ROTL(T32(C5 + G(D6, A6, B5) + m[9] + SPH_C32(0x5A827999)), 9);
  EQL(D6,29);
  INV(D6,30);
  INV(D6,32);
  C6 = TT;
  m[9] = T32(ROTR(TT, 9) - G(D6, A6, B5) - C5 - SPH_C32(0x5A827999));
  
  /* b9 b9,32 = 1 */
  
  /* a10 a10,32 = 1 */
}

static void md4_collision_m1(uint32_t block[16])
{
  block[0]  = 0x4d7a9c83u;
  block[1]  = 0x56cb927au;
  block[2]  = 0xb9d5a578u;
  block[3]  = 0x57a7a5eeu;
  block[4]  = 0xde748a3cu;
  block[5]  = 0xdcc366b3u;
  block[6]  = 0xb683a020u;
  block[7]  = 0x3b2a5d9fu;
  block[8]  = 0xc69d71b3u;
  block[9]  = 0xf9e99198u;
  block[10] = 0xd79f805eu;
  block[11] = 0xa63bb2e8u;
  block[12] = 0x45dd8e31u;
  block[13] = 0x97e31fe5u;
  block[14] = 0x2794bf08u;
  block[15] = 0xb9e8c3e9u;
}

static void md4_collision_m1b(uint32_t block[16])
{

  block[0]  = 0x4d7a9c83u;
  block[1]  = 0xd6cb927au;
  block[2]  = 0x29d5a578u;
  block[3]  = 0x57a7a5eeu;
  block[4]  = 0xde748a3cu;
  block[5]  = 0xdcc366b3u;
  block[6]  = 0xb683a020u;
  block[7]  = 0x3b2a5d9fu;
  block[8]  = 0xc69d71b3u;
  block[9]  = 0xf9e99198u;
  block[10] = 0xd79f805eu;
  block[11] = 0xa63bb2e8u;
  block[12] = 0x45dc8e31u;
  block[13] = 0x97e31fe5u;
  block[14] = 0x2794bf08u;
  block[15] = 0xb9e8c3e9u;
}

static void check_paper_collision(void)
{
  uint32_t state[4] = { 0 };
  uint32_t msg[16] = { 0 };
  uint32_t msg2[16] = { 0 };
  
  md4_collision_m1(msg);
  
  md4_initial(state);
  md4_collision_m1(msg2);
  first_round_properties(state, msg2);
  assert(0 == memcmp(msg, msg2, sizeof msg));
  
  md4_initial(state);
  sph_md4_comp(msg, state);
  assert(state[0] == 0x5f5c1a0du &&
         state[1] == 0x71b36046u &&
         state[2] == 0x1b5435dau &&
         state[3] == 0x9b0d807au);
         
  
  md4_initial(state);
  md4_collision_m1b(msg);
  sph_md4_comp(msg, state);
  assert(state[0] == 0x5f5c1a0du &&
         state[1] == 0x71b36046u &&
         state[2] == 0x1b5435dau &&
         state[3] == 0x9b0d807au);
}

static void print_pair(uint32_t msg[16], uint32_t st[4])
{
  printf("message: ");
  for (int i = 0; i < 16; i++)
    printf("%08x ", msg[i]);
  printf("= ");
  for (int i = 0; i < 4; i++)
    printf("%08x ", st[i]);
  printf("\n");
}

static void find_collision(void)
{
  uint32_t message1[16] = { 0 };
  uint32_t hash1[4] = { 0 };
  uint32_t message2[16] = { 0 };
  uint32_t hash2[4] = { 0 };
  uint32_t state[4] = { 0 };
  
  while (1)
  {
    /* start with random message block */
    random_block(message1);
    
    /* do the appropriate twiddles */
    md4_initial(state);
    first_round_properties(state, message1);
    
    md4_initial(hash1);
    sph_md4_comp(message1, hash1);
    
    memcpy(message2, message1, sizeof message2);
    
    /* apply delta-M */
    message2[1] ^= 0x80000000;
    message2[2] ^= 0x90000000;
    message2[12] ^= 0x00010000;
    
    md4_initial(hash2);
    sph_md4_comp(message2, hash2);
    
    if (memcmp(hash1, hash2, sizeof hash1) == 0 && memcmp(message1, message2, sizeof message1) != 0)
    {
      printf("collision found\n");
      print_pair(message1, hash1);
      print_pair(message2, hash2);
      
      return;
    }
  }
}

int main(int argc, char **argv)
{
  pool p[1] = { pool_create() };  
  random_init();
  
  /* first check the given collisions in the paper */
  check_paper_collision();

  /* now, build our own! */
  find_collision();
  
  p->finish(p);
  return 0;
}