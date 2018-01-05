/*
 * SHA1cc.h
 * SHA1ハッシュを出す。
 * これは https://osdn.jp/projects/sha1cc/ の改造版です。
 */
#pragma once

#include <algorithm>
#include <stdint.h>
#define NOT_ROL //_rolが使える場合はこのdefineを外したほうがいいことあるかも

/* SHA1_Context */
struct SHA1_Context {
    uint8_t  Block[0x40];
    uint32_t State[5 + 1];
    uint64_t Count;

};
/* SHA1_Context */

typedef struct SHA1_Context SHA1_Context_t;
void SHA1cc_Init(SHA1_Context_t* t);
void SHA1cc_Update(SHA1_Context_t* t, const void* pv, size_t cb);
void SHA1cc_Finalize(SHA1_Context_t* t, uint8_t digest[20]);
