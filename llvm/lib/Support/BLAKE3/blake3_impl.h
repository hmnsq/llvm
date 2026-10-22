#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ BLAKE3_IMPL_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BLAKE3_IMPL_H

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm-c/blake3.h"
// For \p LLVM_LIBRARY_VISIBILITY
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm_blake3_prefix.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BLAKE3_ਨਿੱਜੀ

// internal flags
ਗਿਣਤੀ blake3_flags {
  CHUNK_ਸ਼ੁਰੂ         = 1 << 0,
  CHUNK_ਸਿਰਾ           = 1 << 1,
  ਮਾਪਾ              = 1 << 2,
  ਜੜ੍ਹ                = 1 << 3,
  KEYED_HASH          = 1 << 4,
  DERIVE_ਕੁੰਜੀ_CONTEXT  = 1 << 5,
  DERIVE_ਕੁੰਜੀ_MATERIAL = 1 << 6,
};

// This C implementation tries to support recent versions of GCC, Clang, and
// MSVC.
#ਜੇ defined(_MSC_VER)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INLINE ਸਥਿਰ __forceinline
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INLINE ਸਥਿਰ ਸਤਰਅੰਦਰ __attribute__((always_inline))
#ਜੇ_ਅੰਤ

#ਜੇ (defined(__x86_64__) || defined(_M_X64)) && !defined(_M_ARM64EC)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_X86
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_X86_64
#ਜੇ_ਅੰਤ

#ਜੇ defined(__i386__) || defined(_M_IX86)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_X86
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_X86_32
#ਜੇ_ਅੰਤ

#ਜੇ defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_AARCH64
#ਜੇ_ਅੰਤ

#ਜੇ defined(IS_X86)
#ਜੇ defined(_MSC_VER)
#ਅਨ੍ਤਰ੍ਭਾਵ <intrin.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <immintrin.h>
#ਜੇ_ਅੰਤ

#ਜੇ !defined(BLAKE3_USE_NEON) 
  // If BLAKE3_USE_NEON not manually set, autodetect based on AArch64ness
  #ਜੇ defined(IS_AARCH64)
    #ਜੇ defined(__ARM_BIG_ENDIAN)
      #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BLAKE3_USE_NEON 0
    #ਨਹੀਂਤਾਂ
      #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BLAKE3_USE_NEON 1
    #ਜੇ_ਅੰਤ
  #ਨਹੀਂਤਾਂ
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BLAKE3_USE_NEON 0
  #ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ defined(IS_X86)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_SIMD_DEGREE 16
#ਜਾਂਜੇ BLAKE3_USE_NEON == 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_SIMD_DEGREE 4
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_SIMD_DEGREE 1
#ਜੇ_ਅੰਤ

// There are some places where we want a static size that's equal to the
// MAX_SIMD_DEGREE, but also at least 2.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_SIMD_DEGREE_OR_2 (MAX_SIMD_DEGREE > 2 ? MAX_SIMD_DEGREE : 2)

ਸਥਿਰ ਅਚਲ uint32_t IV[8] = {0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL,
                               0xA54FF53AUL, 0x510E527FUL, 0x9B05688CUL,
                               0x1F83D9ABUL, 0x5BE0CD19UL};

ਸਥਿਰ ਅਚਲ uint8_t ਸੁਨੇਹਾ_ਸਮਾਂ_ਸਾਰਣੀ[7][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {2, 6, 3, 10, 7, 0, 4, 13, 1, 11, 12, 5, 9, 14, 15, 8},
    {3, 4, 10, 12, 13, 2, 7, 14, 6, 5, 9, 0, 11, 15, 8, 1},
    {10, 7, 12, 9, 14, 3, 13, 15, 4, 0, 11, 2, 5, 8, 1, 6},
    {12, 13, 9, 11, 15, 10, 14, 8, 7, 2, 5, 3, 0, 1, 6, 4},
    {9, 14, 11, 5, 8, 12, 15, 1, 13, 3, 0, 10, 2, 6, 4, 7},
    {11, 15, 5, 0, 1, 9, 8, 6, 14, 10, 2, 12, 3, 4, 7, 13},
};

/* Find index of the highest set bit */
/* x is assumed to be nonzero.       */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ highest_one(uint64_t x) {
#ਜੇ defined(__GNUC__) || defined(__clang__)
  ਵਾਪਸ 63 ^ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)__builtin_clzll(x);
#ਜਾਂਜੇ defined(_MSC_VER) && defined(IS_X86_64)
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸੂਚਕਾਂਕ;
  ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_Reverse64(&ਸੂਚਕਾਂਕ, x);
  ਵਾਪਸ ਸੂਚਕਾਂਕ;
#ਜਾਂਜੇ defined(_MSC_VER) && defined(IS_X86_32)
  ਜੇ(x >> 32) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸੂਚਕਾਂਕ;
    ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_ਉਲਟ(&ਸੂਚਕਾਂਕ, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(x >> 32));
    ਵਾਪਸ 32 + ਸੂਚਕਾਂਕ;
  } ਨਹੀਂਤਾਂ {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸੂਚਕਾਂਕ;
    ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_ਉਲਟ(&ਸੂਚਕਾਂਕ, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)x);
    ਵਾਪਸ ਸੂਚਕਾਂਕ;
  }
#ਨਹੀਂਤਾਂ
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ c = 0;
  ਜੇ(x & 0xffffffff00000000ULL) { x >>= 32; c += 32; }
  ਜੇ(x & 0x00000000ffff0000ULL) { x >>= 16; c += 16; }
  ਜੇ(x & 0x000000000000ff00ULL) { x >>=  8; c +=  8; }
  ਜੇ(x & 0x00000000000000f0ULL) { x >>=  4; c +=  4; }
  ਜੇ(x & 0x000000000000000cULL) { x >>=  2; c +=  2; }
  ਜੇ(x & 0x0000000000000002ULL) {           c +=  1; }
  ਵਾਪਸ c;
#ਜੇ_ਅੰਤ
}

// Count the number of 1 bits.
INLINE ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ popcnt(uint64_t x) {
#ਜੇ defined(__GNUC__) || defined(__clang__)
  ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)__builtin_popcountll(x);
#ਨਹੀਂਤਾਂ
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
  ਜਦੋਂ (x != 0) {
    ਗਿਣਤੀ_2 += 1;
    x &= x - 1;
  }
  ਵਾਪਸ ਗਿਣਤੀ_2;
#ਜੇ_ਅੰਤ
}

// Largest power of two less than or equal to x. As a special case, returns 1
// when x is 0. 
INLINE uint64_t ਗੇੜਾ_ਥੱਲੇ_ਮੰਜ਼ਿਲ_power_ਦਾ_2(uint64_t x) {
  ਵਾਪਸ 1ULL << highest_one(x | 1);
}

INLINE uint32_t counter_ਨੀਵਾਂ(uint64_t counter) { ਵਾਪਸ (uint32_t)counter; }

INLINE uint32_t counter_ਉੱਚਾ(uint64_t counter) {
  ਵਾਪਸ (uint32_t)(counter >> 32);
}

INLINE uint32_t load32(ਅਚਲ ਖਾਲੀ *src) {
  ਅਚਲ uint8_t *p = (ਅਚਲ uint8_t *)src;
  ਵਾਪਸ ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

INLINE ਖਾਲੀ ਲੱਦੋ_ਕੁੰਜੀ_ਸ਼ਬਦ(ਅਚਲ uint8_t ਕੁੰਜੀ_2[BLAKE3_ਕੁੰਜੀ_LEN],
                           uint32_t ਕੁੰਜੀ_ਸ਼ਬਦ[8]) {
  ਕੁੰਜੀ_ਸ਼ਬਦ[0] = load32(&ਕੁੰਜੀ_2[0 * 4]);
  ਕੁੰਜੀ_ਸ਼ਬਦ[1] = load32(&ਕੁੰਜੀ_2[1 * 4]);
  ਕੁੰਜੀ_ਸ਼ਬਦ[2] = load32(&ਕੁੰਜੀ_2[2 * 4]);
  ਕੁੰਜੀ_ਸ਼ਬਦ[3] = load32(&ਕੁੰਜੀ_2[3 * 4]);
  ਕੁੰਜੀ_ਸ਼ਬਦ[4] = load32(&ਕੁੰਜੀ_2[4 * 4]);
  ਕੁੰਜੀ_ਸ਼ਬਦ[5] = load32(&ਕੁੰਜੀ_2[5 * 4]);
  ਕੁੰਜੀ_ਸ਼ਬਦ[6] = load32(&ਕੁੰਜੀ_2[6 * 4]);
  ਕੁੰਜੀ_ਸ਼ਬਦ[7] = load32(&ਕੁੰਜੀ_2[7 * 4]);
}

INLINE ਖਾਲੀ ਲੱਦੋ_ਖੰਡ_ਸ਼ਬਦ(ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                             uint32_t ਖੰਡ_ਸ਼ਬਦ[16]) {
  ਲਈ (size_t i = 0; i < 16; i++) {
      ਖੰਡ_ਸ਼ਬਦ[i] = load32(&ਖੰਡ[i * 4]);
  }
}

INLINE ਖਾਲੀ store32(ਖਾਲੀ *dst, uint32_t w) {
  uint8_t *p = (uint8_t *)dst;
  p[0] = (uint8_t)(w >> 0);
  p[1] = (uint8_t)(w >> 8);
  p[2] = (uint8_t)(w >> 16);
  p[3] = (uint8_t)(w >> 24);
}

INLINE ਖਾਲੀ store_cv_ਸ਼ਬਦ(uint8_t ਬਾਈਟਾਂ_ਬਾਹਰਲਾ[32], uint32_t cv_ਸ਼ਬਦ[8]) {
  store32(&ਬਾਈਟਾਂ_ਬਾਹਰਲਾ[0 * 4], cv_ਸ਼ਬਦ[0]);
  store32(&ਬਾਈਟਾਂ_ਬਾਹਰਲਾ[1 * 4], cv_ਸ਼ਬਦ[1]);
  store32(&ਬਾਈਟਾਂ_ਬਾਹਰਲਾ[2 * 4], cv_ਸ਼ਬਦ[2]);
  store32(&ਬਾਈਟਾਂ_ਬਾਹਰਲਾ[3 * 4], cv_ਸ਼ਬਦ[3]);
  store32(&ਬਾਈਟਾਂ_ਬਾਹਰਲਾ[4 * 4], cv_ਸ਼ਬਦ[4]);
  store32(&ਬਾਈਟਾਂ_ਬਾਹਰਲਾ[5 * 4], cv_ਸ਼ਬਦ[5]);
  store32(&ਬਾਈਟਾਂ_ਬਾਹਰਲਾ[6 * 4], cv_ਸ਼ਬਦ[6]);
  store32(&ਬਾਈਟਾਂ_ਬਾਹਰਲਾ[7 * 4], cv_ਸ਼ਬਦ[7]);
}

LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_compress_ਵਿੱਚ_ਰੱਖੋ(uint32_t cv[8],
                              ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                              uint8_t ਖੰਡ_len, uint64_t counter,
                              uint8_t flags);

LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_compress_xof(ਅਚਲ uint32_t cv[8],
                         ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                         uint8_t ਖੰਡ_len, uint64_t counter, uint8_t flags,
                         uint8_t ਬਾਹਰਲਾ_2[64]);

LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_xof_many(ਅਚਲ uint32_t cv[8],
                     ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                     uint8_t ਖੰਡ_len, uint64_t counter, uint8_t flags,
                     uint8_t ਬਾਹਰਲਾ_2[64], size_t outblocks);

LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_hash_many(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਅੰਕ_inputs,
                      size_t ਖੰਡਾਂ, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_ਸ਼ੁਰੂ, uint8_t flags_ਸਿਰਾ, uint8_t *ਬਾਹਰਲਾ_2);

LLVM_LIBRARY_VISIBILITY
size_t blake3_simd_degree(ਖਾਲੀ);

BLAKE3_ਨਿੱਜੀ size_t blake3_compress_subtree_wide(ਅਚਲ uint8_t *ਨਿਵੇਸ਼_2, size_t ਨਿਵੇਸ਼_len,
                                                   ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                                                   uint64_t chunk_counter, uint8_t flags,
                                                   uint8_t *ਬਾਹਰਲਾ_2, bool use_tbb);

#ਜੇ defined(BLAKE3_USE_TBB)
BLAKE3_ਨਿੱਜੀ ਖਾਲੀ blake3_compress_subtree_wide_join_tbb(
    // shared params
    ਅਚਲ uint32_t ਕੁੰਜੀ_2[8], uint8_t flags, bool use_tbb,
    // left-hand side params
    ਅਚਲ uint8_t *l_ਨਿਵੇਸ਼, size_t l_ਨਿਵੇਸ਼_len, uint64_t l_chunk_counter,
    uint8_t *l_cvs, size_t *l_n,
    // right-hand side params
    ਅਚਲ uint8_t *r_ਨਿਵੇਸ਼, size_t r_ਨਿਵੇਸ਼_len, uint64_t r_chunk_counter,
    uint8_t *r_cvs, size_t *r_n) NOEXCEPT;
#ਜੇ_ਅੰਤ

// Declarations for implementation-specific functions.
LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_compress_ਵਿੱਚ_ਰੱਖੋ_portable(uint32_t cv[8],
                                       ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                                       uint8_t ਖੰਡ_len, uint64_t counter,
                                       uint8_t flags);

LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_compress_xof_portable(ਅਚਲ uint32_t cv[8],
                                  ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                                  uint8_t ਖੰਡ_len, uint64_t counter,
                                  uint8_t flags, uint8_t ਬਾਹਰਲਾ_2[64]);

LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_hash_many_portable(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਅੰਕ_inputs,
                               size_t ਖੰਡਾਂ, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_ਸ਼ੁਰੂ,
                               uint8_t flags_ਸਿਰਾ, uint8_t *ਬਾਹਰਲਾ_2);

#ਜੇ defined(IS_X86)
#ਜੇ !defined(BLAKE3_NO_SSE2)
LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_compress_ਵਿੱਚ_ਰੱਖੋ_sse2(uint32_t cv[8],
                                   ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                                   uint8_t ਖੰਡ_len, uint64_t counter,
                                   uint8_t flags);
LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_compress_xof_sse2(ਅਚਲ uint32_t cv[8],
                              ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                              uint8_t ਖੰਡ_len, uint64_t counter,
                              uint8_t flags, uint8_t ਬਾਹਰਲਾ_2[64]);
LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_hash_many_sse2(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਅੰਕ_inputs,
                           size_t ਖੰਡਾਂ, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_ਸ਼ੁਰੂ,
                           uint8_t flags_ਸਿਰਾ, uint8_t *ਬਾਹਰਲਾ_2);
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_SSE41)
LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_compress_ਵਿੱਚ_ਰੱਖੋ_sse41(uint32_t cv[8],
                                    ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                                    uint8_t ਖੰਡ_len, uint64_t counter,
                                    uint8_t flags);
LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_compress_xof_sse41(ਅਚਲ uint32_t cv[8],
                               ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                               uint8_t ਖੰਡ_len, uint64_t counter,
                               uint8_t flags, uint8_t ਬਾਹਰਲਾ_2[64]);
LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_hash_many_sse41(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਅੰਕ_inputs,
                            size_t ਖੰਡਾਂ, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_ਸ਼ੁਰੂ,
                            uint8_t flags_ਸਿਰਾ, uint8_t *ਬਾਹਰਲਾ_2);
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_AVX2)
LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_hash_many_avx2(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਅੰਕ_inputs,
                           size_t ਖੰਡਾਂ, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_ਸ਼ੁਰੂ,
                           uint8_t flags_ਸਿਰਾ, uint8_t *ਬਾਹਰਲਾ_2);
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_AVX512)
LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_compress_ਵਿੱਚ_ਰੱਖੋ_avx512(uint32_t cv[8],
                                     ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                                     uint8_t ਖੰਡ_len, uint64_t counter,
                                     uint8_t flags);

LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_compress_xof_avx512(ਅਚਲ uint32_t cv[8],
                                ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                                uint8_t ਖੰਡ_len, uint64_t counter,
                                uint8_t flags, uint8_t ਬਾਹਰਲਾ_2[64]);

LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_hash_many_avx512(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਅੰਕ_inputs,
                             size_t ਖੰਡਾਂ, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                             uint64_t counter, bool increment_counter,
                             uint8_t flags, uint8_t flags_ਸ਼ੁਰੂ,
                             uint8_t flags_ਸਿਰਾ, uint8_t *ਬਾਹਰਲਾ_2);

#ਜੇ !defined(_WIN32) && !defined(__CYGWIN__)
LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_xof_many_avx512(ਅਚਲ uint32_t cv[8],
                            ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                            uint8_t ਖੰਡ_len, uint64_t counter, uint8_t flags,
                            uint8_t* ਬਾਹਰਲਾ_2, size_t outblocks);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ BLAKE3_USE_NEON == 1
LLVM_LIBRARY_VISIBILITY
ਖਾਲੀ blake3_hash_many_neon(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਅੰਕ_inputs,
                           size_t ਖੰਡਾਂ, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_ਸ਼ੁਰੂ,
                           uint8_t flags_ਸਿਰਾ, uint8_t *ਬਾਹਰਲਾ_2);
#ਜੇ_ਅੰਤ


#ਜੇ_ਅੰਤ /* BLAKE3_IMPL_H */
