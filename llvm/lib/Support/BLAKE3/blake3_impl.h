#ifndef BLAKE3_IMPL_H
#define BLAKE3_IMPL_H

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "llvm-c/blake3.h"
// For \p LLVM_LIBRARY_VISIBILITY
#અન્તર્ભાવ "llvm/Support/Compiler.h"

#અન્તર્ભાવ "llvm_blake3_prefix.h"

#define BLAKE3_PRIVATE

// internal flags
ગણના blake3_flags {
  CHUNK_START         = 1 << 0,
  CHUNK_END           = 1 << 1,
  PARENT              = 1 << 2,
  મૂળ                = 1 << 3,
  KEYED_HASH          = 1 << 4,
  DERIVE_ચાવી_CONTEXT  = 1 << 5,
  DERIVE_ચાવી_MATERIAL = 1 << 6,
};

// This C implementation tries to support recent versions of GCC, Clang, and
// MSVC.
#જો defined(_MSC_VER)
#define INLINE સ્થિર __forceinline
#નહીંતર
#define INLINE સ્થિર અંતઃરેખ __attribute__((always_inline))
#endif

#જો (defined(__x86_64__) || defined(_M_X64)) && !defined(_M_ARM64EC)
#define IS_X86
#define IS_X86_64
#endif

#જો defined(__i386__) || defined(_M_IX86)
#define IS_X86
#define IS_X86_32
#endif

#જો defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define IS_AARCH64
#endif

#જો defined(IS_X86)
#જો defined(_MSC_VER)
#અન્તર્ભાવ <intrin.h>
#endif
#અન્તર્ભાવ <immintrin.h>
#endif

#જો !defined(BLAKE3_USE_NEON) 
  // If BLAKE3_USE_NEON not manually set, autodetect based on AArch64ness
  #જો defined(IS_AARCH64)
    #જો defined(__ARM_BIG_ENDIAN)
      #define BLAKE3_USE_NEON 0
    #નહીંતર
      #define BLAKE3_USE_NEON 1
    #endif
  #નહીંતર
    #define BLAKE3_USE_NEON 0
  #endif
#endif

#જો defined(IS_X86)
#define MAX_SIMD_DEGREE 16
#અથવાજો BLAKE3_USE_NEON == 1
#define MAX_SIMD_DEGREE 4
#નહીંતર
#define MAX_SIMD_DEGREE 1
#endif

// There are some places where we want a static size that's equal to the
// MAX_SIMD_DEGREE, but also at least 2.
#define MAX_SIMD_DEGREE_OR_2 (MAX_SIMD_DEGREE > 2 ? MAX_SIMD_DEGREE : 2)

સ્થિર અચલ uint32_t IV[8] = {0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL,
                               0xA54FF53AUL, 0x510E527FUL, 0x9B05688CUL,
                               0x1F83D9ABUL, 0x5BE0CD19UL};

સ્થિર અચલ uint8_t સંદેશ_SCHEDULE[7][16] = {
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
સ્થિર unsigned પૂર્ણાંક highest_one(uint64_t x) {
#જો defined(__GNUC__) || defined(__clang__)
  પાછા 63 ^ (unsigned પૂર્ણાંક)__builtin_clzll(x);
#અથવાજો defined(_MSC_VER) && defined(IS_X86_64)
  unsigned long index;
  _BitScanReverse64(&index, x);
  પાછા index;
#અથવાજો defined(_MSC_VER) && defined(IS_X86_32)
  જો(x >> 32) {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)(x >> 32));
    પાછા 32 + index;
  } નહીંતર {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)x);
    પાછા index;
  }
#નહીંતર
  unsigned પૂર્ણાંક c = 0;
  જો(x & 0xffffffff00000000ULL) { x >>= 32; c += 32; }
  જો(x & 0x00000000ffff0000ULL) { x >>= 16; c += 16; }
  જો(x & 0x000000000000ff00ULL) { x >>=  8; c +=  8; }
  જો(x & 0x00000000000000f0ULL) { x >>=  4; c +=  4; }
  જો(x & 0x000000000000000cULL) { x >>=  2; c +=  2; }
  જો(x & 0x0000000000000002ULL) {           c +=  1; }
  પાછા c;
#endif
}

// Count the number of 1 bits.
INLINE unsigned પૂર્ણાંક popcnt(uint64_t x) {
#જો defined(__GNUC__) || defined(__clang__)
  પાછા (unsigned પૂર્ણાંક)__builtin_popcountll(x);
#નહીંતર
  unsigned પૂર્ણાંક ગણતરી = 0;
  જ્યારે (x != 0) {
    ગણતરી += 1;
    x &= x - 1;
  }
  પાછા ગણતરી;
#endif
}

// Largest power of two less than or equal to x. As a special case, returns 1
// when x is 0. 
INLINE uint64_t round_down_to_power_of_2(uint64_t x) {
  પાછા 1ULL << highest_one(x | 1);
}

INLINE uint32_t counter_નીચલું(uint64_t counter) { પાછા (uint32_t)counter; }

INLINE uint32_t counter_ઉપલું(uint64_t counter) {
  પાછા (uint32_t)(counter >> 32);
}

INLINE uint32_t load32(અચલ શૂન્ય *src) {
  અચલ uint8_t *p = (અચલ uint8_t *)src;
  પાછા ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

INLINE શૂન્ય load_ચાવી_શબ્દો(અચલ uint8_t ચાવી_2[BLAKE3_ચાવી_LEN],
                           uint32_t ચાવી_શબ્દો[8]) {
  ચાવી_શબ્દો[0] = load32(&ચાવી_2[0 * 4]);
  ચાવી_શબ્દો[1] = load32(&ચાવી_2[1 * 4]);
  ચાવી_શબ્દો[2] = load32(&ચાવી_2[2 * 4]);
  ચાવી_શબ્દો[3] = load32(&ચાવી_2[3 * 4]);
  ચાવી_શબ્દો[4] = load32(&ચાવી_2[4 * 4]);
  ચાવી_શબ્દો[5] = load32(&ચાવી_2[5 * 4]);
  ચાવી_શબ્દો[6] = load32(&ચાવી_2[6 * 4]);
  ચાવી_શબ્દો[7] = load32(&ચાવી_2[7 * 4]);
}

INLINE શૂન્ય load_block_શબ્દો(અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                             uint32_t block_શબ્દો[16]) {
  માટે (size_t i = 0; i < 16; i++) {
      block_શબ્દો[i] = load32(&block[i * 4]);
  }
}

INLINE શૂન્ય store32(શૂન્ય *dst, uint32_t w) {
  uint8_t *p = (uint8_t *)dst;
  p[0] = (uint8_t)(w >> 0);
  p[1] = (uint8_t)(w >> 8);
  p[2] = (uint8_t)(w >> 16);
  p[3] = (uint8_t)(w >> 24);
}

INLINE શૂન્ય store_cv_શબ્દો(uint8_t bytes_out[32], uint32_t cv_શબ્દો[8]) {
  store32(&bytes_out[0 * 4], cv_શબ્દો[0]);
  store32(&bytes_out[1 * 4], cv_શબ્દો[1]);
  store32(&bytes_out[2 * 4], cv_શબ્દો[2]);
  store32(&bytes_out[3 * 4], cv_શબ્દો[3]);
  store32(&bytes_out[4 * 4], cv_શબ્દો[4]);
  store32(&bytes_out[5 * 4], cv_શબ્દો[5]);
  store32(&bytes_out[6 * 4], cv_શબ્દો[6]);
  store32(&bytes_out[7 * 4], cv_શબ્દો[7]);
}

LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_compress_in_place(uint32_t cv[8],
                              અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags);

LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_compress_xof(અચલ uint32_t cv[8],
                         અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_xof_many(અચલ uint32_t cv[8],
                     અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks);

LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_hash_many(અચલ uint8_t *અચલ *inputs, size_t અંક_inputs,
                      size_t blocks, અચલ uint32_t ચાવી_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out);

LLVM_LIBRARY_VISIBILITY
size_t blake3_simd_degree(શૂન્ય);

BLAKE3_PRIVATE size_t blake3_compress_subtree_wide(અચલ uint8_t *આગત_2, size_t આગત_len,
                                                   અચલ uint32_t ચાવી_2[8],
                                                   uint64_t chunk_counter, uint8_t flags,
                                                   uint8_t *out, bool use_tbb);

#જો defined(BLAKE3_USE_TBB)
BLAKE3_PRIVATE શૂન્ય blake3_compress_subtree_wide_join_tbb(
    // shared params
    અચલ uint32_t ચાવી_2[8], uint8_t flags, bool use_tbb,
    // left-hand side params
    અચલ uint8_t *l_આગત, size_t l_આગત_len, uint64_t l_chunk_counter,
    uint8_t *l_cvs, size_t *l_n,
    // right-hand side params
    અચલ uint8_t *r_આગત, size_t r_આગત_len, uint64_t r_chunk_counter,
    uint8_t *r_cvs, size_t *r_n) NOEXCEPT;
#endif

// Declarations for implementation-specific functions.
LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_compress_in_place_portable(uint32_t cv[8],
                                       અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                                       uint8_t block_len, uint64_t counter,
                                       uint8_t flags);

LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_compress_xof_portable(અચલ uint32_t cv[8],
                                  અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                                  uint8_t block_len, uint64_t counter,
                                  uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_hash_many_portable(અચલ uint8_t *અચલ *inputs, size_t અંક_inputs,
                               size_t blocks, અચલ uint32_t ચાવી_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_end, uint8_t *out);

#જો defined(IS_X86)
#જો !defined(BLAKE3_NO_SSE2)
LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_compress_in_place_sse2(uint32_t cv[8],
                                   અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                                   uint8_t block_len, uint64_t counter,
                                   uint8_t flags);
LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_compress_xof_sse2(અચલ uint32_t cv[8],
                              અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_hash_many_sse2(અચલ uint8_t *અચલ *inputs, size_t અંક_inputs,
                           size_t blocks, અચલ uint32_t ચાવી_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#જો !defined(BLAKE3_NO_SSE41)
LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_compress_in_place_sse41(uint32_t cv[8],
                                    અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                                    uint8_t block_len, uint64_t counter,
                                    uint8_t flags);
LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_compress_xof_sse41(અચલ uint32_t cv[8],
                               અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                               uint8_t block_len, uint64_t counter,
                               uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_hash_many_sse41(અચલ uint8_t *અચલ *inputs, size_t અંક_inputs,
                            size_t blocks, અચલ uint32_t ચાવી_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_start,
                            uint8_t flags_end, uint8_t *out);
#endif
#જો !defined(BLAKE3_NO_AVX2)
LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_hash_many_avx2(અચલ uint8_t *અચલ *inputs, size_t અંક_inputs,
                           size_t blocks, અચલ uint32_t ચાવી_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#જો !defined(BLAKE3_NO_AVX512)
LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_compress_in_place_avx512(uint32_t cv[8],
                                     અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                                     uint8_t block_len, uint64_t counter,
                                     uint8_t flags);

LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_compress_xof_avx512(અચલ uint32_t cv[8],
                                અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                                uint8_t block_len, uint64_t counter,
                                uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_hash_many_avx512(અચલ uint8_t *અચલ *inputs, size_t અંક_inputs,
                             size_t blocks, અચલ uint32_t ચાવી_2[8],
                             uint64_t counter, bool increment_counter,
                             uint8_t flags, uint8_t flags_start,
                             uint8_t flags_end, uint8_t *out);

#જો !defined(_WIN32) && !defined(__CYGWIN__)
LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_xof_many_avx512(અચલ uint32_t cv[8],
                            અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                            uint8_t block_len, uint64_t counter, uint8_t flags,
                            uint8_t* out, size_t outblocks);
#endif
#endif
#endif

#જો BLAKE3_USE_NEON == 1
LLVM_LIBRARY_VISIBILITY
શૂન્ય blake3_hash_many_neon(અચલ uint8_t *અચલ *inputs, size_t અંક_inputs,
                           size_t blocks, અચલ uint32_t ચાવી_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif


#endif /* BLAKE3_IMPL_H */
