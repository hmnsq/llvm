/*
 *  xxHash - Extremely Fast Hash algorithm
 *  Copyright (C) 2012-2023, Yann Collet
 *
 *  BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You can contact the author at :
 *  - xxHash homepage: http://www.xxhash.com
 *  - xxHash source repository : https://github.com/Cyan4973/xxHash
 */

// xxh3_64bits is based on commit d5891596637d21366b9b1dcf2c0007a3edb26a9e (July
// 2023).

// xxh3_128bits is based on commit b0adcc54188c3130b1793e7b19c62eb1e669f7df
// (June 2024).

#অন্তর্ভাব "llvm/Support/xxhash.h"
#অন্তর্ভাব "llvm/Support/Compiler.h"
#অন্তর্ভাব "llvm/Support/Endian.h"

#অন্তর্ভাব <stdlib.h>

#যদি !defined(LLVM_XXH_USE_NEON)
#যদি (defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)) &&      \
    !defined(__ARM_BIG_ENDIAN)
#define LLVM_XXH_USE_NEON 1
#অন্যথা
#define LLVM_XXH_USE_NEON 0
#endif
#endif

#যদি LLVM_XXH_USE_NEON
#অন্তর্ভাব <arm_neon.h>
#endif

using নামাকাশ llvm;
using নামাকাশ support;

স্থির uint64_t rotl64(uint64_t X, size_t R) {
  ফেরত (X << R) | (X >> (64 - R));
}

constexpr uint32_t PRIME32_1 = 0x9E3779B1;
constexpr uint32_t PRIME32_2 = 0x85EBCA77;
constexpr uint32_t PRIME32_3 = 0xC2B2AE3D;

স্থির ধ্রুবক uint64_t PRIME64_1 = 11400714785074694791ULL;
স্থির ধ্রুবক uint64_t PRIME64_2 = 14029467366897019727ULL;
স্থির ধ্রুবক uint64_t PRIME64_3 = 1609587929392839161ULL;
স্থির ধ্রুবক uint64_t PRIME64_4 = 9650029242287828579ULL;
স্থির ধ্রুবক uint64_t PRIME64_5 = 2870177450012600261ULL;

স্থির uint64_t XXH64_avalanche(uint64_t hash) {
  hash ^= hash >> 33;
  hash *= PRIME64_2;
  hash ^= hash >> 29;
  hash *= PRIME64_3;
  hash ^= hash >> 32;
  ফেরত hash;
}

constexpr size_t XXH3_SECRETSIZE_MIN = 136;
constexpr size_t XXH_SECRET_DEFAULT_আকার = 192;

/* Pseudorandom data taken directly from FARSH */
// clang-format off
constexpr uint8_t kSecret[XXH_SECRET_DEFAULT_আকার] = {
    0xb8, 0xfe, 0x6c, 0x39, 0x23, 0xa4, 0x4b, 0xbe, 0x7c, 0x01, 0x81, 0x2c, 0xf7, 0x21, 0xad, 0x1c,
    0xde, 0xd4, 0x6d, 0xe9, 0x83, 0x90, 0x97, 0xdb, 0x72, 0x40, 0xa4, 0xa4, 0xb7, 0xb3, 0x67, 0x1f,
    0xcb, 0x79, 0xe6, 0x4e, 0xcc, 0xc0, 0xe5, 0x78, 0x82, 0x5a, 0xd0, 0x7d, 0xcc, 0xff, 0x72, 0x21,
    0xb8, 0x08, 0x46, 0x74, 0xf7, 0x43, 0x24, 0x8e, 0xe0, 0x35, 0x90, 0xe6, 0x81, 0x3a, 0x26, 0x4c,
    0x3c, 0x28, 0x52, 0xbb, 0x91, 0xc3, 0x00, 0xcb, 0x88, 0xd0, 0x65, 0x8b, 0x1b, 0x53, 0x2e, 0xa3,
    0x71, 0x64, 0x48, 0x97, 0xa2, 0x0d, 0xf9, 0x4e, 0x38, 0x19, 0xef, 0x46, 0xa9, 0xde, 0xac, 0xd8,
    0xa8, 0xfa, 0x76, 0x3f, 0xe3, 0x9c, 0x34, 0x3f, 0xf9, 0xdc, 0xbb, 0xc7, 0xc7, 0x0b, 0x4f, 0x1d,
    0x8a, 0x51, 0xe0, 0x4b, 0xcd, 0xb4, 0x59, 0x31, 0xc8, 0x9f, 0x7e, 0xc9, 0xd9, 0x78, 0x73, 0x64,
    0xea, 0xc5, 0xac, 0x83, 0x34, 0xd3, 0xeb, 0xc3, 0xc5, 0x81, 0xa0, 0xff, 0xfa, 0x13, 0x63, 0xeb,
    0x17, 0x0d, 0xdd, 0x51, 0xb7, 0xf0, 0xda, 0x49, 0xd3, 0x16, 0x55, 0x26, 0x29, 0xd4, 0x68, 0x9e,
    0x2b, 0x16, 0xbe, 0x58, 0x7d, 0x47, 0xa1, 0xfc, 0x8f, 0xf8, 0xb8, 0xd1, 0x7a, 0xd0, 0x31, 0xce,
    0x45, 0xcb, 0x3a, 0x8f, 0x95, 0x16, 0x04, 0x28, 0xaf, 0xd7, 0xfb, 0xca, 0xbb, 0x4b, 0x40, 0x7e,
};
// clang-format on

constexpr uint64_t মৌলিক_MX1 = 0x165667919E3779F9;
constexpr uint64_t মৌলিক_MX2 = 0x9FB21C651E98DF25;

// Calculates a 64-bit to 128-bit multiply, then XOR folds it.
স্থির uint64_t XXH3_mul128_fold64(uint64_t lhs, uint64_t rhs) {
#যদি defined(__SIZEOF_INT128__) ||                                              \
    (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
  __uint128_t product = (__uint128_t)lhs * (__uint128_t)rhs;
  ফেরত uint64_t(product) ^ uint64_t(product >> 64);

#অন্যথা
  /* First calculate all of the cross products. */
  ধ্রুবক uint64_t lo_lo = (lhs & 0xFFFFFFFF) * (rhs & 0xFFFFFFFF);
  ধ্রুবক uint64_t hi_lo = (lhs >> 32) * (rhs & 0xFFFFFFFF);
  ধ্রুবক uint64_t lo_hi = (lhs & 0xFFFFFFFF) * (rhs >> 32);
  ধ্রুবক uint64_t hi_hi = (lhs >> 32) * (rhs >> 32);

  /* Now add the products together. These will never overflow. */
  ধ্রুবক uint64_t cross = (lo_lo >> 32) + (hi_lo & 0xFFFFFFFF) + lo_hi;
  ধ্রুবক uint64_t upper = (hi_lo >> 32) + (cross >> 32) + hi_hi;
  ধ্রুবক uint64_t lower = (cross << 32) | (lo_lo & 0xFFFFFFFF);

  ফেরত upper ^ lower;
#endif
}

constexpr size_t XXH_STRIPE_LEN = 64;
constexpr size_t XXH_SECRET_CONSUME_RATE = 8;
constexpr size_t XXH_ACC_NB = XXH_STRIPE_LEN / আকার(uint64_t);

স্থির uint64_t XXH3_avalanche(uint64_t hash) {
  hash ^= hash >> 37;
  hash *= মৌলিক_MX1;
  hash ^= hash >> 32;
  ফেরত hash;
}

স্থির uint64_t XXH3_len_1to3_64b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len,
                                  ধ্রুবক uint8_t *secret, uint64_t seed) {
  ধ্রুবক uint8_t c1 = প্রবেশ_2[0];
  ধ্রুবক uint8_t c2 = প্রবেশ_2[len >> 1];
  ধ্রুবক uint8_t c3 = প্রবেশ_2[len - 1];
  uint32_t combined = ((uint32_t)c1 << 16) | ((uint32_t)c2 << 24) |
                      ((uint32_t)c3 << 0) | ((uint32_t)len << 8);
  uint64_t bitflip =
      (uint64_t)(endian::read32le(secret) ^ endian::read32le(secret + 4)) +
      seed;
  ফেরত XXH64_avalanche(uint64_t(combined) ^ bitflip);
}

স্থির uint64_t XXH3_len_4to8_64b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len,
                                  ধ্রুবক uint8_t *secret, uint64_t seed) {
  seed ^= (uint64_t)byteswap(uint32_t(seed)) << 32;
  ধ্রুবক uint32_t input1 = endian::read32le(প্রবেশ_2);
  ধ্রুবক uint32_t input2 = endian::read32le(প্রবেশ_2 + len - 4);
  uint64_t acc =
      (endian::read64le(secret + 8) ^ endian::read64le(secret + 16)) - seed;
  ধ্রুবক uint64_t input64 = (uint64_t)input2 | ((uint64_t)input1 << 32);
  acc ^= input64;
  // XXH3_rrmxmx(acc, len)
  acc ^= rotl64(acc, 49) ^ rotl64(acc, 24);
  acc *= মৌলিক_MX2;
  acc ^= (acc >> 35) + (uint64_t)len;
  acc *= মৌলিক_MX2;
  ফেরত acc ^ (acc >> 28);
}

স্থির uint64_t XXH3_len_9to16_64b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len,
                                   ধ্রুবক uint8_t *secret, uint64_t ধ্রুবক seed) {
  uint64_t প্রবেশ_lo =
      (endian::read64le(secret + 24) ^ endian::read64le(secret + 32)) + seed;
  uint64_t প্রবেশ_hi =
      (endian::read64le(secret + 40) ^ endian::read64le(secret + 48)) - seed;
  প্রবেশ_lo ^= endian::read64le(প্রবেশ_2);
  প্রবেশ_hi ^= endian::read64le(প্রবেশ_2 + len - 8);
  uint64_t acc = uint64_t(len) + byteswap(প্রবেশ_lo) + প্রবেশ_hi +
                 XXH3_mul128_fold64(প্রবেশ_lo, প্রবেশ_hi);
  ফেরত XXH3_avalanche(acc);
}

LLVM_ATTRIBUTE_ALWAYS_INLINE
স্থির uint64_t XXH3_len_0to16_64b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len,
                                   ধ্রুবক uint8_t *secret, uint64_t ধ্রুবক seed) {
  যদি (LLVM_LIKELY(len > 8))
    ফেরত XXH3_len_9to16_64b(প্রবেশ_2, len, secret, seed);
  যদি (LLVM_LIKELY(len >= 4))
    ফেরত XXH3_len_4to8_64b(প্রবেশ_2, len, secret, seed);
  যদি (len != 0)
    ফেরত XXH3_len_1to3_64b(প্রবেশ_2, len, secret, seed);
  ফেরত XXH64_avalanche(seed ^ endian::read64le(secret + 56) ^
                         endian::read64le(secret + 64));
}

স্থির uint64_t XXH3_mix16B(ধ্রুবক uint8_t *প্রবেশ_2, uint8_t ধ্রুবক *secret,
                            uint64_t seed) {
  uint64_t lhs = seed;
  uint64_t rhs = 0U - seed;
  lhs += endian::read64le(secret);
  rhs += endian::read64le(secret + 8);
  lhs ^= endian::read64le(প্রবেশ_2);
  rhs ^= endian::read64le(প্রবেশ_2 + 8);
  ফেরত XXH3_mul128_fold64(lhs, rhs);
}

/* For mid range keys, XXH3 uses a Mum-hash variant. */
LLVM_ATTRIBUTE_ALWAYS_INLINE
স্থির uint64_t XXH3_len_17to128_64b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len,
                                     ধ্রুবক uint8_t *secret,
                                     uint64_t ধ্রুবক seed) {
  uint64_t acc = len * PRIME64_1, acc_end;
  acc += XXH3_mix16B(প্রবেশ_2 + 0, secret + 0, seed);
  acc_end = XXH3_mix16B(প্রবেশ_2 + len - 16, secret + 16, seed);
  যদি (len > 32) {
    acc += XXH3_mix16B(প্রবেশ_2 + 16, secret + 32, seed);
    acc_end += XXH3_mix16B(প্রবেশ_2 + len - 32, secret + 48, seed);
    যদি (len > 64) {
      acc += XXH3_mix16B(প্রবেশ_2 + 32, secret + 64, seed);
      acc_end += XXH3_mix16B(প্রবেশ_2 + len - 48, secret + 80, seed);
      যদি (len > 96) {
        acc += XXH3_mix16B(প্রবেশ_2 + 48, secret + 96, seed);
        acc_end += XXH3_mix16B(প্রবেশ_2 + len - 64, secret + 112, seed);
      }
    }
  }
  ফেরত XXH3_avalanche(acc + acc_end);
}

constexpr size_t XXH3_MIDSIZE_MAX = 240;
constexpr size_t XXH3_MIDSIZE_STARTOFFSET = 3;
constexpr size_t XXH3_MIDSIZE_LASTOFFSET = 17;

LLVM_ATTRIBUTE_NOINLINE
স্থির uint64_t XXH3_len_129to240_64b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len,
                                      ধ্রুবক uint8_t *secret, uint64_t seed) {
  uint64_t acc = (uint64_t)len * PRIME64_1;
  ধ্রুবক unsigned nbRounds = len / 16;
  জন্য (unsigned i = 0; i < 8; ++i)
    acc += XXH3_mix16B(প্রবেশ_2 + 16 * i, secret + 16 * i, seed);
  acc = XXH3_avalanche(acc);

  জন্য (unsigned i = 8; i < nbRounds; ++i) {
    acc += XXH3_mix16B(প্রবেশ_2 + 16 * i,
                       secret + 16 * (i - 8) + XXH3_MIDSIZE_STARTOFFSET, seed);
  }
  /* last bytes */
  acc +=
      XXH3_mix16B(প্রবেশ_2 + len - 16,
                  secret + XXH3_SECRETSIZE_MIN - XXH3_MIDSIZE_LASTOFFSET, seed);
  ফেরত XXH3_avalanche(acc);
}

#যদি LLVM_XXH_USE_NEON

#define XXH3_accumulate_512 XXH3_accumulate_512_neon
#define XXH3_scrambleAcc XXH3_scrambleAcc_neon

// NEON implementation based on commit a57f6cce2698049863af8c25787084ae0489d849
// (July 2024), with the following removed:
// - workaround for suboptimal codegen on older GCC
// - compiler barriers against instruction reordering
// - WebAssembly SIMD support
// - configurable split between NEON and scalar lanes (benchmarking shows no
//   penalty when fully doing SIMD on the Apple M1)

#যদি defined(__GNUC__) || defined(__clang__)
#define XXH_ALIASING __attribute__((__may_alias__))
#অন্যথা
#define XXH_ALIASING /* nothing */
#endif

প্রকারলেখ uint64x2_t xxh_aliasing_uint64x2_t XXH_ALIASING;

LLVM_ATTRIBUTE_ALWAYS_INLINE স্থির uint64x2_t XXH_vld1q_u64(শূন্য ধ্রুবক *ptr) {
  ফেরত vreinterpretq_u64_u8(vld1q_u8((uint8_t ধ্রুবক *)ptr));
}

LLVM_ATTRIBUTE_ALWAYS_INLINE
স্থির শূন্য XXH3_accumulate_512_neon(uint64_t *acc, ধ্রুবক uint8_t *প্রবেশ_2,
                                     ধ্রুবক uint8_t *secret) {
  xxh_aliasing_uint64x2_t *ধ্রুবক xacc = (xxh_aliasing_uint64x2_t *)acc;

#ifdef __clang__
#pragma clang loop unroll(full)
#endif
  জন্য (size_t i = 0; i < XXH_ACC_NB / 2; i += 2) {
    /* data_vec = input[i]; */
    uint64x2_t তথ্য_vec_1 = XXH_vld1q_u64(প্রবেশ_2 + (i * 16));
    uint64x2_t তথ্য_vec_2 = XXH_vld1q_u64(প্রবেশ_2 + ((i + 1) * 16));

    /* key_vec  = secret[i];  */
    uint64x2_t চাবি_vec_1 = XXH_vld1q_u64(secret + (i * 16));
    uint64x2_t চাবি_vec_2 = XXH_vld1q_u64(secret + ((i + 1) * 16));

    /* data_swap = swap(data_vec) */
    uint64x2_t তথ্য_swap_1 = vextq_u64(তথ্য_vec_1, তথ্য_vec_1, 1);
    uint64x2_t তথ্য_swap_2 = vextq_u64(তথ্য_vec_2, তথ্য_vec_2, 1);

    /* data_key = data_vec ^ key_vec; */
    uint64x2_t তথ্য_চাবি_1 = veorq_u64(তথ্য_vec_1, চাবি_vec_1);
    uint64x2_t তথ্য_চাবি_2 = veorq_u64(তথ্য_vec_2, চাবি_vec_2);

    /*
     * If we reinterpret the 64x2 vectors as 32x4 vectors, we can use a
     * de-interleave operation for 4 lanes in 1 step with `vuzpq_u32` to
     * get one vector with the low 32 bits of each lane, and one vector
     * with the high 32 bits of each lane.
     *
     * The intrinsic returns a double vector because the original ARMv7-a
     * instruction modified both arguments in place. AArch64 and SIMD128 emit
     * two instructions from this intrinsic.
     *
     *  [ dk11L | dk11H | dk12L | dk12H ] -> [ dk11L | dk12L | dk21L | dk22L ]
     *  [ dk21L | dk21H | dk22L | dk22H ] -> [ dk11H | dk12H | dk21H | dk22H ]
     */
    uint32x4x2_t unzipped = vuzpq_u32(vreinterpretq_u32_u64(তথ্য_চাবি_1),
                                      vreinterpretq_u32_u64(তথ্য_চাবি_2));

    /* data_key_lo = data_key & 0xFFFFFFFF */
    uint32x4_t তথ্য_চাবি_lo = unzipped.val[0];
    /* data_key_hi = data_key >> 32 */
    uint32x4_t তথ্য_চাবি_hi = unzipped.val[1];

    /*
     * Then, we can split the vectors horizontally and multiply which, as for
     * most widening intrinsics, have a variant that works on both high half
     * vectors for free on AArch64. A similar instruction is available on
     * SIMD128.
     *
     * sum = data_swap + (u64x2) data_key_lo * (u64x2) data_key_hi
     */
    uint64x2_t যোগ_1 = vmlal_u32(তথ্য_swap_1, vget_নিচের_u32(তথ্য_চাবি_lo),
                                 vget_নিচের_u32(তথ্য_চাবি_hi));
    uint64x2_t যোগ_2_2 = vmlal_u32(তথ্য_swap_2, vget_উপরের_u32(তথ্য_চাবি_lo),
                                 vget_উপরের_u32(তথ্য_চাবি_hi));

    /* xacc[i] = acc_vec + sum; */
    xacc[i] = vaddq_u64(xacc[i], যোগ_1);
    xacc[i + 1] = vaddq_u64(xacc[i + 1], যোগ_2_2);
  }
}

LLVM_ATTRIBUTE_ALWAYS_INLINE
স্থির শূন্য XXH3_scrambleAcc_neon(uint64_t *acc, ধ্রুবক uint8_t *secret) {
  xxh_aliasing_uint64x2_t *ধ্রুবক xacc = (xxh_aliasing_uint64x2_t *)acc;

  /* { prime32_1, prime32_1 } */
  uint32x2_t ধ্রুবক k_মৌলিক_Lo = vdup_n_u32(PRIME32_1);
  /* { 0, prime32_1, 0, prime32_1 } */
  uint32x4_t ধ্রুবক k_মৌলিক_Hi =
      vreinterpretq_u32_u64(vdupq_n_u64((uint64_t)PRIME32_1 << 32));

  জন্য (size_t i = 0; i < XXH_ACC_NB / 2; ++i) {
    /* xacc[i] ^= (xacc[i] >> 47); */
    uint64x2_t acc_vec = XXH_vld1q_u64(acc + (2 * i));
    uint64x2_t shifted = vshrq_n_u64(acc_vec, 47);
    uint64x2_t তথ্য_vec = veorq_u64(acc_vec, shifted);

    /* xacc[i] ^= secret[i]; */
    uint64x2_t চাবি_vec = XXH_vld1q_u64(secret + (i * 16));
    uint64x2_t তথ্য_চাবি = veorq_u64(তথ্য_vec, চাবি_vec);

    /*
     * xacc[i] *= XXH_PRIME32_1
     *
     * Expanded version with portable NEON intrinsics
     *
     *    lo(x) * lo(y) + (hi(x) * lo(y) << 32)
     *
     * prod_hi = hi(data_key) * lo(prime) << 32
     *
     * Since we only need 32 bits of this multiply a trick can be used,
     * reinterpreting the vector as a uint32x4_t and multiplying by
     * { 0, prime, 0, prime } to cancel out the unwanted bits and avoid the
     * shift.
     */
    uint32x4_t prod_hi = vmulq_u32(vreinterpretq_u32_u64(তথ্য_চাবি), k_মৌলিক_Hi);

    /* Extract low bits for vmlal_u32  */
    uint32x2_t তথ্য_চাবি_lo = vmovn_u64(তথ্য_চাবি);

    /* xacc[i] = prod_hi + lo(data_key) * XXH_PRIME32_1; */
    xacc[i] = vmlal_u32(vreinterpretq_u64_u32(prod_hi), তথ্য_চাবি_lo, k_মৌলিক_Lo);
  }
}
#অন্যথা

#define XXH3_accumulate_512 XXH3_accumulate_512_scalar
#define XXH3_scrambleAcc XXH3_scrambleAcc_scalar

LLVM_ATTRIBUTE_ALWAYS_INLINE
স্থির শূন্য XXH3_accumulate_512_scalar(uint64_t *acc, ধ্রুবক uint8_t *প্রবেশ_2,
                                       ধ্রুবক uint8_t *secret) {
  জন্য (size_t i = 0; i < XXH_ACC_NB; ++i) {
    uint64_t তথ্য_val = endian::read64le(প্রবেশ_2 + 8 * i);
    uint64_t তথ্য_চাবি = তথ্য_val ^ endian::read64le(secret + 8 * i);
    acc[i ^ 1] += তথ্য_val;
    acc[i] += uint32_t(তথ্য_চাবি) * (তথ্য_চাবি >> 32);
  }
}

LLVM_ATTRIBUTE_ALWAYS_INLINE
স্থির শূন্য XXH3_scrambleAcc_scalar(uint64_t *acc, ধ্রুবক uint8_t *secret) {
  জন্য (size_t i = 0; i < XXH_ACC_NB; ++i) {
    acc[i] ^= acc[i] >> 47;
    acc[i] ^= endian::read64le(secret + 8 * i);
    acc[i] *= PRIME32_1;
  }
}
#endif

LLVM_ATTRIBUTE_ALWAYS_INLINE
স্থির শূন্য XXH3_accumulate(uint64_t *acc, ধ্রুবক uint8_t *প্রবেশ_2,
                            ধ্রুবক uint8_t *secret, size_t nbStripes) {
  জন্য (size_t n = 0; n < nbStripes; ++n) {
    XXH3_accumulate_512(acc, প্রবেশ_2 + n * XXH_STRIPE_LEN,
                        secret + n * XXH_SECRET_CONSUME_RATE);
  }
}

স্থির uint64_t XXH3_mix2Accs(ধ্রুবক uint64_t *acc, ধ্রুবক uint8_t *secret) {
  ফেরত XXH3_mul128_fold64(acc[0] ^ endian::read64le(secret),
                            acc[1] ^ endian::read64le(secret + 8));
}

স্থির uint64_t XXH3_mergeAccs(ধ্রুবক uint64_t *acc, ধ্রুবক uint8_t *চাবি_2,
                               uint64_t start) {
  uint64_t result64 = start;
  জন্য (size_t i = 0; i < 4; ++i)
    result64 += XXH3_mix2Accs(acc + 2 * i, চাবি_2 + 16 * i);
  ফেরত XXH3_avalanche(result64);
}

LLVM_ATTRIBUTE_NOINLINE
স্থির uint64_t XXH3_hashLong_64b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len,
                                  ধ্রুবক uint8_t *secret, size_t secret_আকার) {
  ধ্রুবক size_t nbStripesPerBlock =
      (secret_আকার - XXH_STRIPE_LEN) / XXH_SECRET_CONSUME_RATE;
  ধ্রুবক size_t block_len = XXH_STRIPE_LEN * nbStripesPerBlock;
  ধ্রুবক size_t nb_blocks = (len - 1) / block_len;
  alignas(16) uint64_t acc[XXH_ACC_NB] = {
      PRIME32_3, PRIME64_1, PRIME64_2, PRIME64_3,
      PRIME64_4, PRIME32_2, PRIME64_5, PRIME32_1,
  };
  জন্য (size_t n = 0; n < nb_blocks; ++n) {
    XXH3_accumulate(acc, প্রবেশ_2 + n * block_len, secret, nbStripesPerBlock);
    XXH3_scrambleAcc(acc, secret + secret_আকার - XXH_STRIPE_LEN);
  }

  /* last partial block */
  ধ্রুবক size_t nbStripes = (len - 1 - (block_len * nb_blocks)) / XXH_STRIPE_LEN;
  প্রতিজ্ঞা(nbStripes <= secret_আকার / XXH_SECRET_CONSUME_RATE);
  XXH3_accumulate(acc, প্রবেশ_2 + nb_blocks * block_len, secret, nbStripes);

  /* last stripe */
  constexpr size_t XXH_SECRET_LASTACC_START = 7;
  XXH3_accumulate_512(acc, প্রবেশ_2 + len - XXH_STRIPE_LEN,
                      secret + secret_আকার - XXH_STRIPE_LEN -
                          XXH_SECRET_LASTACC_START);

  /* converge into final hash */
  constexpr size_t XXH_SECRET_MERGEACCS_START = 11;
  ফেরত XXH3_mergeAccs(acc, secret + XXH_SECRET_MERGEACCS_START,
                        (uint64_t)len * PRIME64_1);
}

uint64_t llvm::xxh3_64bits(ধ্রুবক uint8_t *in, size_t len) {
  যদি (len <= 16)
    ফেরত XXH3_len_0to16_64b(in, len, kSecret, 0);
  যদি (len <= 128)
    ফেরত XXH3_len_17to128_64b(in, len, kSecret, 0);
  যদি (len <= XXH3_MIDSIZE_MAX)
    ফেরত XXH3_len_129to240_64b(in, len, kSecret, 0);
  ফেরত XXH3_hashLong_64b(in, len, kSecret, আকার(kSecret));
}

/* ==========================================
 * XXH3 128 bits (a.k.a XXH128)
 * ==========================================
 * XXH3's 128-bit variant has better mixing and strength than the 64-bit
 * variant, even without counting the significantly larger output size.
 *
 * For example, extra steps are taken to avoid the seed-dependent collisions
 * in 17-240 byte inputs (See XXH3_mix16B and XXH128_mix32B).
 *
 * This strength naturally comes at the cost of some speed, especially on short
 * lengths. Note that longer hashes are about as fast as the 64-bit version
 * due to it using only a slight modification of the 64-bit loop.
 *
 * XXH128 is also more oriented towards 64-bit machines. It is still extremely
 * fast for a _128-bit_ hash on 32-bit (it usually clears XXH64).
 */

/*!
 * @internal
 * @def XXH_rotl32(x,r)
 * @brief 32-bit rotate left.
 *
 * @param x The 32-bit integer to be rotated.
 * @param r The number of bits to rotate.
 * @pre
 *   @p r > 0 && @p r < 32
 * @note
 *   @p x and @p r may be evaluated multiple times.
 * @return The rotated result.
 */
#যদি __has_builtin(__builtin_rotateleft32) &&                                   \
    __has_builtin(__builtin_rotateleft64)
#define XXH_rotl32 __builtin_rotateleft32
#define XXH_rotl64 __builtin_rotateleft64
/* Note: although _rotl exists for minGW (GCC under windows), performance seems
 * poor */
#নতুবা_যদি defined(_MSC_VER)
#define XXH_rotl32(x, r) _rotl(x, r)
#define XXH_rotl64(x, r) _rotl64(x, r)
#অন্যথা
#define XXH_rotl32(x, r) (((x) << (r)) | ((x) >> (32 - (r))))
#define XXH_rotl64(x, r) (((x) << (r)) | ((x) >> (64 - (r))))
#endif

#define XXH_mult32to64(x, y) ((uint64_t)(uint32_t)(x) * (uint64_t)(uint32_t)(y))

/*!
 * @brief Calculates a 64->128-bit long multiply.
 *
 * Uses `__uint128_t` and `_umul128` if available, otherwise uses a scalar
 * version.
 *
 * @param lhs , rhs The 64-bit integers to be multiplied
 * @return The 128-bit result represented in an @ref XXH128_hash_t.
 */
স্থির XXH128_hash_t XXH_mult64to128(uint64_t lhs, uint64_t rhs) {
  /*
   * GCC/Clang __uint128_t method.
   *
   * On most 64-bit targets, GCC and Clang define a __uint128_t type.
   * This is usually the best way as it usually uses a native long 64-bit
   * multiply, such as MULQ on x86_64 or MUL + UMULH on aarch64.
   *
   * Usually.
   *
   * Despite being a 32-bit platform, Clang (and emscripten) define this type
   * despite not having the arithmetic for it. This results in a laggy
   * compiler builtin call which calculates a full 128-bit multiply.
   * In that case it is best to use the portable one.
   * https://github.com/Cyan4973/xxHash/issues/211#issuecomment-515575677
   */
#যদি (defined(__GNUC__) || defined(__clang__)) && !defined(__wasm__) &&         \
        defined(__SIZEOF_INT128__) ||                                          \
    (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)

  __uint128_t ধ্রুবক product = (__uint128_t)lhs * (__uint128_t)rhs;
  XXH128_hash_t r128;
  r128.low64 = (uint64_t)(product);
  r128.high64 = (uint64_t)(product >> 64);
  ফেরত r128;

  /*
   * MSVC for x64's _umul128 method.
   *
   * uint64_t _umul128(uint64_t Multiplier, uint64_t Multiplicand, uint64_t
   * *HighProduct);
   *
   * This compiles to single operand MUL on x64.
   */
#নতুবা_যদি (defined(_M_X64) || defined(_M_IA64)) && !defined(_M_ARM64EC)

#ifndef _MSC_VER
#pragma intrinsic(_umul128)
#endif
  uint64_t product_উপরের;
  uint64_t ধ্রুবক product_নিচের = _umul128(lhs, rhs, &product_উপরের);
  XXH128_hash_t r128;
  r128.low64 = product_নিচের;
  r128.high64 = product_উপরের;
  ফেরত r128;

  /*
   * MSVC for ARM64's __umulh method.
   *
   * This compiles to the same MUL + UMULH as GCC/Clang's __uint128_t method.
   */
#নতুবা_যদি defined(_M_ARM64) || defined(_M_ARM64EC)

#ifndef _MSC_VER
#pragma intrinsic(__umulh)
#endif
  XXH128_hash_t r128;
  r128.low64 = lhs * rhs;
  r128.high64 = __umulh(lhs, rhs);
  ফেরত r128;

#অন্যথা
  /*
   * Portable scalar method. Optimized for 32-bit and 64-bit ALUs.
   *
   * This is a fast and simple grade school multiply, which is shown below
   * with base 10 arithmetic instead of base 0x100000000.
   *
   *           9 3 // D2 lhs = 93
   *         x 7 5 // D2 rhs = 75
   *     ----------
   *           1 5 // D2 lo_lo = (93 % 10) * (75 % 10) = 15
   *         4 5 | // D2 hi_lo = (93 / 10) * (75 % 10) = 45
   *         2 1 | // D2 lo_hi = (93 % 10) * (75 / 10) = 21
   *     + 6 3 | | // D2 hi_hi = (93 / 10) * (75 / 10) = 63
   *     ---------
   *         2 7 | // D2 cross = (15 / 10) + (45 % 10) + 21 = 27
   *     + 6 7 | | // D2 upper = (27 / 10) + (45 / 10) + 63 = 67
   *     ---------
   *       6 9 7 5 // D4 res = (27 * 10) + (15 % 10) + (67 * 100) = 6975
   *
   * The reasons for adding the products like this are:
   *  1. It avoids manual carry tracking. Just like how
   *     (9 * 9) + 9 + 9 = 99, the same applies with this for UINT64_MAX.
   *     This avoids a lot of complexity.
   *
   *  2. It hints for, and on Clang, compiles to, the powerful UMAAL
   *     instruction available in ARM's Digital Signal Processing extension
   *     in 32-bit ARMv6 and later, which is shown below:
   *
   *         void UMAAL(xxh_u32 *RdLo, xxh_u32 *RdHi, xxh_u32 Rn, xxh_u32 Rm)
   *         {
   *             uint64_t product = (uint64_t)*RdLo * (uint64_t)*RdHi + Rn + Rm;
   *             *RdLo = (xxh_u32)(product & 0xFFFFFFFF);
   *             *RdHi = (xxh_u32)(product >> 32);
   *         }
   *
   *     This instruction was designed for efficient long multiplication, and
   *     allows this to be calculated in only 4 instructions at speeds
   *     comparable to some 64-bit ALUs.
   *
   *  3. It isn't terrible on other platforms. Usually this will be a couple
   *     of 32-bit ADD/ADCs.
   */

  /* First calculate all of the cross products. */
  uint64_t ধ্রুবক lo_lo = XXH_mult32to64(lhs & 0xFFFFFFFF, rhs & 0xFFFFFFFF);
  uint64_t ধ্রুবক hi_lo = XXH_mult32to64(lhs >> 32, rhs & 0xFFFFFFFF);
  uint64_t ধ্রুবক lo_hi = XXH_mult32to64(lhs & 0xFFFFFFFF, rhs >> 32);
  uint64_t ধ্রুবক hi_hi = XXH_mult32to64(lhs >> 32, rhs >> 32);

  /* Now add the products together. These will never overflow. */
  uint64_t ধ্রুবক cross = (lo_lo >> 32) + (hi_lo & 0xFFFFFFFF) + lo_hi;
  uint64_t ধ্রুবক upper = (hi_lo >> 32) + (cross >> 32) + hi_hi;
  uint64_t ধ্রুবক lower = (cross << 32) | (lo_lo & 0xFFFFFFFF);

  XXH128_hash_t r128;
  r128.low64 = lower;
  r128.high64 = upper;
  ফেরত r128;
#endif
}

/*! Seems to produce slightly better code on GCC for some reason. */
LLVM_ATTRIBUTE_ALWAYS_INLINE constexpr uint64_t XXH_xorshift64(uint64_t v64,
                                                               পূর্ণসংখ্যা shift) {
  ফেরত v64 ^ (v64 >> shift);
}

LLVM_ATTRIBUTE_ALWAYS_INLINE স্থির XXH128_hash_t
XXH3_len_1to3_128b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len, ধ্রুবক uint8_t *secret,
                   uint64_t seed) {
  /* A doubled version of 1to3_64b with different constants. */
  /*
   * len = 1: combinedl = { input[0], 0x01, input[0], input[0] }
   * len = 2: combinedl = { input[1], 0x02, input[0], input[1] }
   * len = 3: combinedl = { input[2], 0x03, input[0], input[1] }
   */
  uint8_t ধ্রুবক c1 = প্রবেশ_2[0];
  uint8_t ধ্রুবক c2 = প্রবেশ_2[len >> 1];
  uint8_t ধ্রুবক c3 = প্রবেশ_2[len - 1];
  uint32_t ধ্রুবক combinedl = ((uint32_t)c1 << 16) | ((uint32_t)c2 << 24) |
                             ((uint32_t)c3 << 0) | ((uint32_t)len << 8);
  uint32_t ধ্রুবক combinedh = XXH_rotl32(byteswap(combinedl), 13);
  uint64_t ধ্রুবক bitflipl =
      (endian::read32le(secret) ^ endian::read32le(secret + 4)) + seed;
  uint64_t ধ্রুবক bitfliph =
      (endian::read32le(secret + 8) ^ endian::read32le(secret + 12)) - seed;
  uint64_t ধ্রুবক keyed_lo = (uint64_t)combinedl ^ bitflipl;
  uint64_t ধ্রুবক keyed_hi = (uint64_t)combinedh ^ bitfliph;
  XXH128_hash_t h128;
  h128.low64 = XXH64_avalanche(keyed_lo);
  h128.high64 = XXH64_avalanche(keyed_hi);
  ফেরত h128;
}

LLVM_ATTRIBUTE_ALWAYS_INLINE স্থির XXH128_hash_t
XXH3_len_4to8_128b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len, ধ্রুবক uint8_t *secret,
                   uint64_t seed) {
  seed ^= (uint64_t)byteswap((uint32_t)seed) << 32;
  uint32_t ধ্রুবক প্রবেশ_lo = endian::read32le(প্রবেশ_2);
  uint32_t ধ্রুবক প্রবেশ_hi = endian::read32le(প্রবেশ_2 + len - 4);
  uint64_t ধ্রুবক প্রবেশ_64 = প্রবেশ_lo + ((uint64_t)প্রবেশ_hi << 32);
  uint64_t ধ্রুবক bitflip =
      (endian::read64le(secret + 16) ^ endian::read64le(secret + 24)) + seed;
  uint64_t ধ্রুবক keyed = প্রবেশ_64 ^ bitflip;

  /* Shift len to the left to ensure it is even, this avoids even multiplies.
   */
  XXH128_hash_t m128 = XXH_mult64to128(keyed, PRIME64_1 + (len << 2));

  m128.high64 += (m128.low64 << 1);
  m128.low64 ^= (m128.high64 >> 3);

  m128.low64 = XXH_xorshift64(m128.low64, 35);
  m128.low64 *= মৌলিক_MX2;
  m128.low64 = XXH_xorshift64(m128.low64, 28);
  m128.high64 = XXH3_avalanche(m128.high64);
  ফেরত m128;
}

LLVM_ATTRIBUTE_ALWAYS_INLINE স্থির XXH128_hash_t
XXH3_len_9to16_128b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len, ধ্রুবক uint8_t *secret,
                    uint64_t seed) {
  uint64_t ধ্রুবক bitflipl =
      (endian::read64le(secret + 32) ^ endian::read64le(secret + 40)) - seed;
  uint64_t ধ্রুবক bitfliph =
      (endian::read64le(secret + 48) ^ endian::read64le(secret + 56)) + seed;
  uint64_t ধ্রুবক প্রবেশ_lo = endian::read64le(প্রবেশ_2);
  uint64_t প্রবেশ_hi = endian::read64le(প্রবেশ_2 + len - 8);
  XXH128_hash_t m128 =
      XXH_mult64to128(প্রবেশ_lo ^ প্রবেশ_hi ^ bitflipl, PRIME64_1);
  /*
   * Put len in the middle of m128 to ensure that the length gets mixed to
   * both the low and high bits in the 128x64 multiply below.
   */
  m128.low64 += (uint64_t)(len - 1) << 54;
  প্রবেশ_hi ^= bitfliph;
  /*
   * Add the high 32 bits of input_hi to the high 32 bits of m128, then
   * add the long product of the low 32 bits of input_hi and PRIME32_2 to
   * the high 64 bits of m128.
   *
   * The best approach to this operation is different on 32-bit and 64-bit.
   */
  যদি (আকার(শূন্য *) < আকার(uint64_t)) { /* 32-bit */
    /*
     * 32-bit optimized version, which is more readable.
     *
     * On 32-bit, it removes an ADC and delays a dependency between the two
     * halves of m128.high64, but it generates an extra mask on 64-bit.
     */
    m128.high64 += (প্রবেশ_hi & 0xFFFFFFFF00000000ULL) +
                   XXH_mult32to64((uint32_t)প্রবেশ_hi, PRIME32_2);
  } অন্যথা {
    /*
     * 64-bit optimized (albeit more confusing) version.
     *
     * Uses some properties of addition and multiplication to remove the mask:
     *
     * Let:
     *    a = input_hi.lo = (input_hi & 0x00000000FFFFFFFF)
     *    b = input_hi.hi = (input_hi & 0xFFFFFFFF00000000)
     *    c = PRIME32_2
     *
     *    a + (b * c)
     * Inverse Property: x + y - x == y
     *    a + (b * (1 + c - 1))
     * Distributive Property: x * (y + z) == (x * y) + (x * z)
     *    a + (b * 1) + (b * (c - 1))
     * Identity Property: x * 1 == x
     *    a + b + (b * (c - 1))
     *
     * Substitute a, b, and c:
     *    input_hi.hi + input_hi.lo + ((uint64_t)input_hi.lo * (PRIME32_2
     * - 1))
     *
     * Since input_hi.hi + input_hi.lo == input_hi, we get this:
     *    input_hi + ((uint64_t)input_hi.lo * (PRIME32_2 - 1))
     */
    m128.high64 += প্রবেশ_hi + XXH_mult32to64((uint32_t)প্রবেশ_hi, PRIME32_2 - 1);
  }
  /* m128 ^= XXH_swap64(m128 >> 64); */
  m128.low64 ^= byteswap(m128.high64);

  /* 128x64 multiply: h128 = m128 * PRIME64_2; */
  XXH128_hash_t h128 = XXH_mult64to128(m128.low64, PRIME64_2);
  h128.high64 += m128.high64 * PRIME64_2;

  h128.low64 = XXH3_avalanche(h128.low64);
  h128.high64 = XXH3_avalanche(h128.high64);
  ফেরত h128;
}

/*
 * Assumption: `secret` size is >= XXH3_SECRET_SIZE_MIN
 */
LLVM_ATTRIBUTE_ALWAYS_INLINE স্থির XXH128_hash_t
XXH3_len_0to16_128b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len, ধ্রুবক uint8_t *secret,
                    uint64_t seed) {
  যদি (len > 8)
    ফেরত XXH3_len_9to16_128b(প্রবেশ_2, len, secret, seed);
  যদি (len >= 4)
    ফেরত XXH3_len_4to8_128b(প্রবেশ_2, len, secret, seed);
  যদি (len)
    ফেরত XXH3_len_1to3_128b(প্রবেশ_2, len, secret, seed);
  XXH128_hash_t h128;
  uint64_t ধ্রুবক bitflipl =
      endian::read64le(secret + 64) ^ endian::read64le(secret + 72);
  uint64_t ধ্রুবক bitfliph =
      endian::read64le(secret + 80) ^ endian::read64le(secret + 88);
  h128.low64 = XXH64_avalanche(seed ^ bitflipl);
  h128.high64 = XXH64_avalanche(seed ^ bitfliph);
  ফেরত h128;
}

/*
 * A bit slower than XXH3_mix16B, but handles multiply by zero better.
 */
LLVM_ATTRIBUTE_ALWAYS_INLINE স্থির XXH128_hash_t
XXH128_mix32B(XXH128_hash_t acc, ধ্রুবক uint8_t *প্রবেশ_1, ধ্রুবক uint8_t *প্রবেশ_2_2,
              ধ্রুবক uint8_t *secret, uint64_t seed) {
  acc.low64 += XXH3_mix16B(প্রবেশ_1, secret + 0, seed);
  acc.low64 ^= endian::read64le(প্রবেশ_2_2) + endian::read64le(প্রবেশ_2_2 + 8);
  acc.high64 += XXH3_mix16B(প্রবেশ_2_2, secret + 16, seed);
  acc.high64 ^= endian::read64le(প্রবেশ_1) + endian::read64le(প্রবেশ_1 + 8);
  ফেরত acc;
}

LLVM_ATTRIBUTE_ALWAYS_INLINE স্থির XXH128_hash_t
XXH3_len_17to128_128b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len, ধ্রুবক uint8_t *secret,
                      size_t secret_আকার, uint64_t seed) {
  (শূন্য)secret_আকার;

  XXH128_hash_t acc;
  acc.low64 = len * PRIME64_1;
  acc.high64 = 0;

  যদি (len > 32) {
    যদি (len > 64) {
      যদি (len > 96) {
        acc =
            XXH128_mix32B(acc, প্রবেশ_2 + 48, প্রবেশ_2 + len - 64, secret + 96, seed);
      }
      acc = XXH128_mix32B(acc, প্রবেশ_2 + 32, প্রবেশ_2 + len - 48, secret + 64, seed);
    }
    acc = XXH128_mix32B(acc, প্রবেশ_2 + 16, প্রবেশ_2 + len - 32, secret + 32, seed);
  }
  acc = XXH128_mix32B(acc, প্রবেশ_2, প্রবেশ_2 + len - 16, secret, seed);
  XXH128_hash_t h128;
  h128.low64 = acc.low64 + acc.high64;
  h128.high64 = (acc.low64 * PRIME64_1) + (acc.high64 * PRIME64_4) +
                ((len - seed) * PRIME64_2);
  h128.low64 = XXH3_avalanche(h128.low64);
  h128.high64 = (uint64_t)0 - XXH3_avalanche(h128.high64);
  ফেরত h128;
}

LLVM_ATTRIBUTE_NOINLINE স্থির XXH128_hash_t
XXH3_len_129to240_128b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len, ধ্রুবক uint8_t *secret,
                       size_t secret_আকার, uint64_t seed) {
  (শূন্য)secret_আকার;

  XXH128_hash_t acc;
  unsigned i;
  acc.low64 = len * PRIME64_1;
  acc.high64 = 0;
  /*
   *  We set as `i` as offset + 32. We do this so that unchanged
   * `len` can be used as upper bound. This reaches a sweet spot
   * where both x86 and aarch64 get simple agen and good codegen
   * for the loop.
   */
  জন্য (i = 32; i < 160; i += 32) {
    acc = XXH128_mix32B(acc, প্রবেশ_2 + i - 32, প্রবেশ_2 + i - 16, secret + i - 32,
                        seed);
  }
  acc.low64 = XXH3_avalanche(acc.low64);
  acc.high64 = XXH3_avalanche(acc.high64);
  /*
   * NB: `i <= len` will duplicate the last 32-bytes if
   * len % 32 was zero. This is an unfortunate necessity to keep
   * the hash result stable.
   */
  জন্য (i = 160; i <= len; i += 32) {
    acc = XXH128_mix32B(acc, প্রবেশ_2 + i - 32, প্রবেশ_2 + i - 16,
                        secret + XXH3_MIDSIZE_STARTOFFSET + i - 160, seed);
  }
  /* last bytes */
  acc =
      XXH128_mix32B(acc, প্রবেশ_2 + len - 16, প্রবেশ_2 + len - 32,
                    secret + XXH3_SECRETSIZE_MIN - XXH3_MIDSIZE_LASTOFFSET - 16,
                    (uint64_t)0 - seed);

  XXH128_hash_t h128;
  h128.low64 = acc.low64 + acc.high64;
  h128.high64 = (acc.low64 * PRIME64_1) + (acc.high64 * PRIME64_4) +
                ((len - seed) * PRIME64_2);
  h128.low64 = XXH3_avalanche(h128.low64);
  h128.high64 = (uint64_t)0 - XXH3_avalanche(h128.high64);
  ফেরত h128;
}

LLVM_ATTRIBUTE_ALWAYS_INLINE XXH128_hash_t
XXH3_hashLong_128b(ধ্রুবক uint8_t *প্রবেশ_2, size_t len, ধ্রুবক uint8_t *secret,
                   size_t secret_আকার) {
  ধ্রুবক size_t nbStripesPerBlock =
      (secret_আকার - XXH_STRIPE_LEN) / XXH_SECRET_CONSUME_RATE;
  ধ্রুবক size_t block_len = XXH_STRIPE_LEN * nbStripesPerBlock;
  ধ্রুবক size_t nb_blocks = (len - 1) / block_len;
  alignas(16) uint64_t acc[XXH_ACC_NB] = {
      PRIME32_3, PRIME64_1, PRIME64_2, PRIME64_3,
      PRIME64_4, PRIME32_2, PRIME64_5, PRIME32_1,
  };

  জন্য (size_t n = 0; n < nb_blocks; ++n) {
    XXH3_accumulate(acc, প্রবেশ_2 + n * block_len, secret, nbStripesPerBlock);
    XXH3_scrambleAcc(acc, secret + secret_আকার - XXH_STRIPE_LEN);
  }

  /* last partial block */
  ধ্রুবক size_t nbStripes = (len - 1 - (block_len * nb_blocks)) / XXH_STRIPE_LEN;
  প্রতিজ্ঞা(nbStripes <= secret_আকার / XXH_SECRET_CONSUME_RATE);
  XXH3_accumulate(acc, প্রবেশ_2 + nb_blocks * block_len, secret, nbStripes);

  /* last stripe */
  constexpr size_t XXH_SECRET_LASTACC_START = 7;
  XXH3_accumulate_512(acc, প্রবেশ_2 + len - XXH_STRIPE_LEN,
                      secret + secret_আকার - XXH_STRIPE_LEN -
                          XXH_SECRET_LASTACC_START);

  /* converge into final hash */
  static_assert(আকার(acc) == 64);
  XXH128_hash_t h128;
  constexpr size_t XXH_SECRET_MERGEACCS_START = 11;
  h128.low64 = XXH3_mergeAccs(acc, secret + XXH_SECRET_MERGEACCS_START,
                              (uint64_t)len * PRIME64_1);
  h128.high64 = XXH3_mergeAccs(
      acc, secret + secret_আকার - আকার(acc) - XXH_SECRET_MERGEACCS_START,
      ~((uint64_t)len * PRIME64_2));
  ফেরত h128;
}

llvm::XXH128_hash_t llvm::xxh3_128bits(ধ্রুবক uint8_t *প্রবেশ_2, size_t len) {
  /*
   * If an action is to be taken if `secret` conditions are not respected,
   * it should be done here.
   * For now, it's a contract pre-condition.
   * Adding a check and a branch here would cost performance at every hash.
   */
  যদি (len <= 16)
    ফেরত XXH3_len_0to16_128b(প্রবেশ_2, len, kSecret, /*seed64=*/0);
  যদি (len <= 128)
    ফেরত XXH3_len_17to128_128b(প্রবেশ_2, len, kSecret, আকার(kSecret),
                                 /*seed64=*/0);
  যদি (len <= XXH3_MIDSIZE_MAX)
    ফেরত XXH3_len_129to240_128b(প্রবেশ_2, len, kSecret, আকার(kSecret),
                                  /*seed64=*/0);
  ফেরত XXH3_hashLong_128b(প্রবেশ_2, len, kSecret, আকার(kSecret));
}
