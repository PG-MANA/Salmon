/*
 * 
 * SHA1cc.c
 * SHA1ハッシュを出す。
 * これは https://osdn.jp/projects/sha1cc/ の改造版です。
 */
#include "SHA1cc.h"
#include <string.h>


/* */
#ifdef NOT_ROL
#define ROL(v, b) (((v) << (b)) | ((v) >> (32-(b))))
#else
#define ROL(v, b) _rotl((v), (b))
#endif
#define BSW(v) __builtin_bswap32((v))

/* */

#define B_(i) (b[i] = BSW(b[i])) /* LE */
#define BL(i) (b[i&0xf] = ROL(b[(i+0xd)&0xf] ^ b[(i+0x8)&0xf] ^ b[(i+0x2)&0xf] ^ b[i&0xf], 1))

#define R_(v,w,x,y,z,i) r##z += ((r##w & (r##x ^ r##y)) ^ r##y)     + B_(i) + 0x5a827999 + ROL(r##v, 5); r##w = ROL(r##w, 30);
#define R0(v,w,x,y,z,i) r##z += ((r##w & (r##x ^ r##y)) ^ r##y)     + BL(i) + 0x5a827999 + ROL(r##v, 5); r##w = ROL(r##w, 30);
#define R1(v,w,x,y,z,i) r##z += (r##w ^ r##x ^ r##y)                    + BL(i) + 0x6ed9eba1 + ROL(r##v, 5); r##w = ROL(r##w, 30);
#define R2(v,w,x,y,z,i) r##z += (((r##w | r##x) & r##y) | (r##w & r##x)) + BL(i) + 0x8f1bbcdc + ROL(r##v, 5); r##w = ROL(r##w, 30);
#define R3(v,w,x,y,z,i) r##z += (r##w ^ r##x ^ r##y)                    + BL(i) + 0xca62c1d6 + ROL(r##v, 5); r##w = ROL(r##w, 30);

/* */

void SHA1cc_Transform(SHA1_Context_t* t)
{
    uint32_t r0 = t->State[0];
    uint32_t r1 = t->State[1];
    uint32_t r2 = t->State[2];
    uint32_t r3 = t->State[3];
    uint32_t r4 = t->State[4];

    uint32_t* b = (uint32_t*)(t->Block);

    R_(0, 1, 2, 3, 4, 0)
    R_(4, 0, 1, 2, 3, 1)
    R_(3, 4, 0, 1, 2, 2)
    R_(2, 3, 4, 0, 1, 3)
    R_(1, 2, 3, 4, 0, 4)
    R_(0, 1, 2, 3, 4, 5)
    R_(4, 0, 1, 2, 3, 6)
    R_(3, 4, 0, 1, 2, 7)
    R_(2, 3, 4, 0, 1, 8)
    R_(1, 2, 3, 4, 0, 9)
    R_(0, 1, 2, 3, 4, 10)
    R_(4, 0, 1, 2, 3, 11)
    R_(3, 4, 0, 1, 2, 12)
    R_(2, 3, 4, 0, 1, 13)
    R_(1, 2, 3, 4, 0, 14)
    R_(0, 1, 2, 3, 4, 15)
    R0(4, 0, 1, 2, 3, 16)
    R0(3, 4, 0, 1, 2, 17)
    R0(2, 3, 4, 0, 1, 18)
    R0(1, 2, 3, 4, 0, 19)

    R1(0, 1, 2, 3, 4, 20)
    R1(4, 0, 1, 2, 3, 21)
    R1(3, 4, 0, 1, 2, 22)
    R1(2, 3, 4, 0, 1, 23)
    R1(1, 2, 3, 4, 0, 24)
    R1(0, 1, 2, 3, 4, 25)
    R1(4, 0, 1, 2, 3, 26)
    R1(3, 4, 0, 1, 2, 27)
    R1(2, 3, 4, 0, 1, 28)
    R1(1, 2, 3, 4, 0, 29)
    R1(0, 1, 2, 3, 4, 30)
    R1(4, 0, 1, 2, 3, 31)
    R1(3, 4, 0, 1, 2, 32)
    R1(2, 3, 4, 0, 1, 33)
    R1(1, 2, 3, 4, 0, 34)
    R1(0, 1, 2, 3, 4, 35)
    R1(4, 0, 1, 2, 3, 36)
    R1(3, 4, 0, 1, 2, 37)
    R1(2, 3, 4, 0, 1, 38)
    R1(1, 2, 3, 4, 0, 39)

    R2(0, 1, 2, 3, 4, 40)
    R2(4, 0, 1, 2, 3, 41)
    R2(3, 4, 0, 1, 2, 42)
    R2(2, 3, 4, 0, 1, 43)
    R2(1, 2, 3, 4, 0, 44)
    R2(0, 1, 2, 3, 4, 45)
    R2(4, 0, 1, 2, 3, 46)
    R2(3, 4, 0, 1, 2, 47)
    R2(2, 3, 4, 0, 1, 48)
    R2(1, 2, 3, 4, 0, 49)
    R2(0, 1, 2, 3, 4, 50)
    R2(4, 0, 1, 2, 3, 51)
    R2(3, 4, 0, 1, 2, 52)
    R2(2, 3, 4, 0, 1, 53)
    R2(1, 2, 3, 4, 0, 54)
    R2(0, 1, 2, 3, 4, 55)
    R2(4, 0, 1, 2, 3, 56)
    R2(3, 4, 0, 1, 2, 57)
    R2(2, 3, 4, 0, 1, 58)
    R2(1, 2, 3, 4, 0, 59)

    R3(0, 1, 2, 3, 4, 60)
    R3(4, 0, 1, 2, 3, 61)
    R3(3, 4, 0, 1, 2, 62)
    R3(2, 3, 4, 0, 1, 63)
    R3(1, 2, 3, 4, 0, 64)
    R3(0, 1, 2, 3, 4, 65)
    R3(4, 0, 1, 2, 3, 66)
    R3(3, 4, 0, 1, 2, 67)
    R3(2, 3, 4, 0, 1, 68)
    R3(1, 2, 3, 4, 0, 69)
    R3(0, 1, 2, 3, 4, 70)
    R3(4, 0, 1, 2, 3, 71)
    R3(3, 4, 0, 1, 2, 72)
    R3(2, 3, 4, 0, 1, 73)
    R3(1, 2, 3, 4, 0, 74)
    R3(0, 1, 2, 3, 4, 75)
    R3(4, 0, 1, 2, 3, 76)
    R3(3, 4, 0, 1, 2, 77)
    R3(2, 3, 4, 0, 1, 78)
    R3(1, 2, 3, 4, 0, 79)

    t->State[0] += r0;
    t->State[1] += r1;
    t->State[2] += r2;
    t->State[3] += r3;
    t->State[4] += r4;
}



void SHA1cc_Init(SHA1_Context_t* t)
{
    memset(t, 0, sizeof(SHA1_Context_t));

    t->State[0] = 0x67452301;
    t->State[1] = 0xefcdab89;
    t->State[2] = 0x98badcfe;
    t->State[3] = 0x10325476;
    t->State[4] = 0xc3d2e1f0;
}

void SHA1cc_Update(
    SHA1_Context_t* t,
    const void*  pv,
    size_t        cb)
{
    const uint8_t* ss = (const uint8_t*)pv;
    const uint8_t* se = ss + cb;

    uint32_t i = (uint32_t)(t->Count) & 0x3f;
    if (i > 0) {
        size_t s_sz = se - ss;
        size_t d_sz = 0x40 - i;

        if (s_sz < d_sz) {
            memcpy(t->Block + i, ss, s_sz);
            t->Count += s_sz;
            return;
        }

        memcpy(t->Block + i, ss, d_sz);
        t->Count += d_sz;
        ss += d_sz;

        SHA1cc_Transform(t);
    }

    for (; ; ) {
        size_t s_sz = se - ss;
        if (s_sz < 0x40) {
            memcpy(t->Block, ss, s_sz);
            t->Count += s_sz;
            return;
        }

        memcpy(t->Block, ss, 0x40);
        t->Count += 0x40;
        ss += 0x40;

        SHA1cc_Transform(t);
    }
}

#define UI64_0(v) BSW((uint32_t)((v) >> 32)) /* LE */
#define UI64_1(v) BSW((uint32_t) (v)       ) /* LE */
#define UI32(v)   BSW((v))               /* LE */

void SHA1cc_Finalize(
    SHA1_Context_t* t,
    uint8_t        digest[20])
{
    uint64_t bit_count = t->Count << 3;

    uint32_t i = (uint32_t)(t->Count) & 0x3f;

    t->Block[i++] = 0x80;
    memset(t->Block + i, 0, 0x40 - i);

    if (i > 0x38) {
        SHA1cc_Transform(t);

        memset(t->Block, 0, 0x40);
    }

    *((uint32_t*)(t->Block + 0x38)) = UI64_0(bit_count);
    *((uint32_t*)(t->Block + 0x3c)) = UI64_1(bit_count);

    SHA1cc_Transform(t);

    *((uint32_t*)(digest + 0x00)) = UI32(t->State[0]);
    *((uint32_t*)(digest + 0x04)) = UI32(t->State[1]);
    *((uint32_t*)(digest + 0x08)) = UI32(t->State[2]);
    *((uint32_t*)(digest + 0x0c)) = UI32(t->State[3]);
    *((uint32_t*)(digest + 0x10)) = UI32(t->State[4]);
}

