/*
** Name:        aegis128x2_altivec.c
** Purpose:     Implementation of AEGIS-128x2 - AltiVec
** Copyright:   (c) 2023-2024 Frank Denis
** SPDX-License-Identifier: MIT
*/

#if defined(__ALTIVEC__) && defined(__CRYPTO__)

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../common/common.h"
#include "aegis128x2.h"
#include "aegis128x2_altivec.h"

#include <altivec.h>

#ifdef __clang__
#  pragma clang attribute push(__attribute__((target("altivec,crypto"))), apply_to = function)
#elif defined(__GNUC__)
#  pragma GCC target("+altivec+crypto")
#endif

#define AES_BLOCK_LENGTH 32

typedef struct {
    vector unsigned char b0;
    vector unsigned char b1;
} aegis128x2_aes_block_t;

#define AEGIS_AES_BLOCK_T aegis128x2_aes_block_t
#define AEGIS_BLOCKS      aegis128x2_blocks
#define AEGIS_STATE      _aegis128x2_state
#define AEGIS_MAC_STATE  _aegis128x2_mac_state

#define AEGIS_FUNC_PREFIX  aegis128x2_impl

#include "../common/func_names_define.h"

static inline AEGIS_AES_BLOCK_T
AEGIS_AES_BLOCK_XOR(const AEGIS_AES_BLOCK_T a, const AEGIS_AES_BLOCK_T b)
{
    return (AEGIS_AES_BLOCK_T) { vec_xor(a.b0, b.b0), vec_xor(a.b1, b.b1) };
}

static inline AEGIS_AES_BLOCK_T
AEGIS_AES_BLOCK_AND(const AEGIS_AES_BLOCK_T a, const AEGIS_AES_BLOCK_T b)
{
    return (AEGIS_AES_BLOCK_T) { vec_and(a.b0, b.b0), vec_and(a.b1, b.b1) };
}

static inline AEGIS_AES_BLOCK_T
AEGIS_AES_BLOCK_LOAD(const uint8_t *a)
{
    return (AEGIS_AES_BLOCK_T) { vec_xl_be(0, a), vec_xl_be(0, a + 16) };
}

static inline AEGIS_AES_BLOCK_T
AEGIS_AES_BLOCK_LOAD_64x2(uint64_t a, uint64_t b)
{
    const vector unsigned char t =
        vec_revb(vec_insert(a, vec_promote((unsigned long long) (b), 1), 0));
    return (AEGIS_AES_BLOCK_T) { t, t };
}
static inline void
AEGIS_AES_BLOCK_STORE(uint8_t *a, const AEGIS_AES_BLOCK_T b)
{
    vec_xst_be(b.b0, 0, a);
    vec_xst_be(b.b1, 0, a + 16);
}

static inline AEGIS_AES_BLOCK_T
AEGIS_AES_ENC(const AEGIS_AES_BLOCK_T a, const AEGIS_AES_BLOCK_T b)
{
    return (AEGIS_AES_BLOCK_T) { vec_cipher_be(a.b0, b.b0), vec_cipher_be(a.b1, b.b1) };
}

static inline void
AEGIS_update(AEGIS_AES_BLOCK_T *const state, const AEGIS_AES_BLOCK_T d1, const AEGIS_AES_BLOCK_T d2)
{
    AEGIS_AES_BLOCK_T tmp;

    tmp      = state[7];
    state[7] = AEGIS_AES_ENC(state[6], state[7]);
    state[6] = AEGIS_AES_ENC(state[5], state[6]);
    state[5] = AEGIS_AES_ENC(state[4], state[5]);
    state[4] = AEGIS_AES_BLOCK_XOR(AEGIS_AES_ENC(state[3], state[4]), d2);
    state[3] = AEGIS_AES_ENC(state[2], state[3]);
    state[2] = AEGIS_AES_ENC(state[1], state[2]);
    state[1] = AEGIS_AES_ENC(state[0], state[1]);
    state[0] = AEGIS_AES_BLOCK_XOR(AEGIS_AES_ENC(tmp, state[0]), d1);
}

#include "aegis128x2_common.h"

struct aegis128x2_implementation aegis128x2_altivec_implementation = {
    AEGIS_API_IMPL_LIST
};

#include "../common/type_names_undefine.h"
#include "../common/func_names_undefine.h"

#ifdef __clang__
#  pragma clang attribute pop
#endif

#endif
