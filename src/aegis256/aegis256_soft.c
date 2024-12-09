/*
** Name:        aegis256_soft.c
** Purpose:     Implementation of AEGIS-256 - Software
** Copyright:   (c) 2023-2024 Frank Denis
** SPDX-License-Identifier: MIT
*/

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../common/common.h"
#include "../common/cpu.h"

#include "../common/softaes.h"
#include "aegis256.h"
#include "aegis256_soft.h"

#define AES_BLOCK_LENGTH 16

typedef SoftAesBlock aegis256_soft_aes_block_t;

#define AEGIS_AES_BLOCK_T aegis256_soft_aes_block_t
#define AEGIS_BLOCKS      aegis256_soft_blocks
#define AEGIS_STATE      _aegis256_soft_state
#define AEGIS_MAC_STATE  _aegis256_soft_mac_state

#define AEGIS_FUNC_PREFIX  aegis256_soft_impl

#include "../common/func_names_define.h"

static inline AEGIS_AES_BLOCK_T
AEGIS_AES_BLOCK_XOR(const AEGIS_AES_BLOCK_T a, const AEGIS_AES_BLOCK_T b)
{
  return softaes_block_xor(a, b);
}

static inline AEGIS_AES_BLOCK_T
AEGIS_AES_BLOCK_AND(const AEGIS_AES_BLOCK_T a, const AEGIS_AES_BLOCK_T b)
{
  return softaes_block_and(a, b);
}

static inline AEGIS_AES_BLOCK_T
AEGIS_AES_BLOCK_LOAD(const uint8_t *a)
{
  return softaes_block_load(a);
}

static inline AEGIS_AES_BLOCK_T
AEGIS_AES_BLOCK_LOAD_64x2(uint64_t a, uint64_t b)
{
  return softaes_block_load64x2(a, b);
}

static inline void
AEGIS_AES_BLOCK_STORE(uint8_t *a, const AEGIS_AES_BLOCK_T b)
{
  softaes_block_store(a, b);
}

static inline AEGIS_AES_BLOCK_T
AEGIS_AES_ENC(const AEGIS_AES_BLOCK_T a, const AEGIS_AES_BLOCK_T b)
{
  return softaes_block_encrypt(a, b);
}

static inline void
AEGIS_update(AEGIS_AES_BLOCK_T *const state, const AEGIS_AES_BLOCK_T d)
{
    AEGIS_AES_BLOCK_T tmp;

    tmp      = state[5];
    state[5] = AEGIS_AES_ENC(state[4], state[5]);
    state[4] = AEGIS_AES_ENC(state[3], state[4]);
    state[3] = AEGIS_AES_ENC(state[2], state[3]);
    state[2] = AEGIS_AES_ENC(state[1], state[2]);
    state[1] = AEGIS_AES_ENC(state[0], state[1]);
    state[0] = AEGIS_AES_BLOCK_XOR(AEGIS_AES_ENC(tmp, state[0]), d);
}

#include "aegis256_common.h"

struct aegis256_implementation aegis256_soft_implementation = {
    AEGIS_API_IMPL_LIST
};

#include "../common/type_names_undefine.h"
#include "../common/func_names_undefine.h"
