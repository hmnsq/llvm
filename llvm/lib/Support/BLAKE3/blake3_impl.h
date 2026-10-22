#ifndef BLAKE3_IMPL_H
#define BLAKE3_IMPL_H

#ضمن <assert.h>
#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdint.h>
#ضمن <string.h>

#ضمن "llvm-c/blake3.h"
// For \p LLVM_LIBRARY_VISIBILITY
#ضمن "llvm/Support/Compiler.h"

#ضمن "llvm_blake3_prefix.h"

#define BLAKE3_PRIVATE

// internal flags
تعداد blake3_flags {
  CHUNK_START         = 1 << 0,
  CHUNK_END           = 1 << 1,
  PARENT              = 1 << 2,
  يىلتىز                = 1 << 3,
  KEYED_HASH          = 1 << 4,
  DERIVE_ئاچقۇچ_CONTEXT  = 1 << 5,
  DERIVE_ئاچقۇچ_MATERIAL = 1 << 6,
};

// This C implementation tries to support recent versions of GCC, Clang, and
// MSVC.
#إذا defined(_MSC_VER)
#define INLINE ساكن __forceinline
#وإلا
#define INLINE ساكن مضمن __attribute__((always_inline))
#endif

#إذا (defined(__x86_64__) || defined(_M_X64)) && !defined(_M_ARM64EC)
#define IS_X86
#define IS_X86_64
#endif

#إذا defined(__i386__) || defined(_M_IX86)
#define IS_X86
#define IS_X86_32
#endif

#إذا defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define IS_AARCH64
#endif

#إذا defined(IS_X86)
#إذا defined(_MSC_VER)
#ضمن <intrin.h>
#endif
#ضمن <immintrin.h>
#endif

#إذا !defined(BLAKE3_USE_NEON) 
  // If BLAKE3_USE_NEON not manually set, autodetect based on AArch64ness
  #إذا defined(IS_AARCH64)
    #إذا defined(__ARM_BIG_ENDIAN)
      #define BLAKE3_USE_NEON 0
    #وإلا
      #define BLAKE3_USE_NEON 1
    #endif
  #وإلا
    #define BLAKE3_USE_NEON 0
  #endif
#endif

#إذا defined(IS_X86)
#define MAX_SIMD_DEGREE 16
#وإلاإذا BLAKE3_USE_NEON == 1
#define MAX_SIMD_DEGREE 4
#وإلا
#define MAX_SIMD_DEGREE 1
#endif

// There are some places where we want a static size that's equal to the
// MAX_SIMD_DEGREE, but also at least 2.
#define MAX_SIMD_DEGREE_OR_2 (MAX_SIMD_DEGREE > 2 ? MAX_SIMD_DEGREE : 2)

ساكن ثابت uint32_t IV[8] = {0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL,
                               0xA54FF53AUL, 0x510E527FUL, 0x9B05688CUL,
                               0x1F83D9ABUL, 0x5BE0CD19UL};

ساكن ثابت uint8_t ئۇچۇر_SCHEDULE[7][16] = {
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
ساكن unsigned صحيح highest_one(uint64_t x) {
#إذا defined(__GNUC__) || defined(__clang__)
  أرجع 63 ^ (unsigned صحيح)__builtin_clzll(x);
#وإلاإذا defined(_MSC_VER) && defined(IS_X86_64)
  unsigned long index;
  _BitScanReverse64(&index, x);
  أرجع index;
#وإلاإذا defined(_MSC_VER) && defined(IS_X86_32)
  إذا(x >> 32) {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)(x >> 32));
    أرجع 32 + index;
  } وإلا {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)x);
    أرجع index;
  }
#وإلا
  unsigned صحيح c = 0;
  إذا(x & 0xffffffff00000000ULL) { x >>= 32; c += 32; }
  إذا(x & 0x00000000ffff0000ULL) { x >>= 16; c += 16; }
  إذا(x & 0x000000000000ff00ULL) { x >>=  8; c +=  8; }
  إذا(x & 0x00000000000000f0ULL) { x >>=  4; c +=  4; }
  إذا(x & 0x000000000000000cULL) { x >>=  2; c +=  2; }
  إذا(x & 0x0000000000000002ULL) {           c +=  1; }
  أرجع c;
#endif
}

// Count the number of 1 bits.
INLINE unsigned صحيح popcnt(uint64_t x) {
#إذا defined(__GNUC__) || defined(__clang__)
  أرجع (unsigned صحيح)__builtin_popcountll(x);
#وإلا
  unsigned صحيح سانى = 0;
  طالما (x != 0) {
    سانى += 1;
    x &= x - 1;
  }
  أرجع سانى;
#endif
}

// Largest power of two less than or equal to x. As a special case, returns 1
// when x is 0. 
INLINE uint64_t round_down_to_power_of_2(uint64_t x) {
  أرجع 1ULL << highest_one(x | 1);
}

INLINE uint32_t counter_تۆۋەن(uint64_t counter) { أرجع (uint32_t)counter; }

INLINE uint32_t counter_يۇقىرى(uint64_t counter) {
  أرجع (uint32_t)(counter >> 32);
}

INLINE uint32_t load32(ثابت فراغ *src) {
  ثابت uint8_t *p = (ثابت uint8_t *)src;
  أرجع ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

INLINE فراغ load_ئاچقۇچ_سۆزلەر(ثابت uint8_t ئاچقۇچ_2[BLAKE3_ئاچقۇچ_LEN],
                           uint32_t ئاچقۇچ_سۆزلەر[8]) {
  ئاچقۇچ_سۆزلەر[0] = load32(&ئاچقۇچ_2[0 * 4]);
  ئاچقۇچ_سۆزلەر[1] = load32(&ئاچقۇچ_2[1 * 4]);
  ئاچقۇچ_سۆزلەر[2] = load32(&ئاچقۇچ_2[2 * 4]);
  ئاچقۇچ_سۆزلەر[3] = load32(&ئاچقۇچ_2[3 * 4]);
  ئاچقۇچ_سۆزلەر[4] = load32(&ئاچقۇچ_2[4 * 4]);
  ئاچقۇچ_سۆزلەر[5] = load32(&ئاچقۇچ_2[5 * 4]);
  ئاچقۇچ_سۆزلەر[6] = load32(&ئاچقۇچ_2[6 * 4]);
  ئاچقۇچ_سۆزلەر[7] = load32(&ئاچقۇچ_2[7 * 4]);
}

INLINE فراغ load_block_سۆزلەر(ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                             uint32_t block_سۆزلەر[16]) {
  لكل (size_t i = 0; i < 16; i++) {
      block_سۆزلەر[i] = load32(&block[i * 4]);
  }
}

INLINE فراغ store32(فراغ *dst, uint32_t w) {
  uint8_t *p = (uint8_t *)dst;
  p[0] = (uint8_t)(w >> 0);
  p[1] = (uint8_t)(w >> 8);
  p[2] = (uint8_t)(w >> 16);
  p[3] = (uint8_t)(w >> 24);
}

INLINE فراغ store_cv_سۆزلەر(uint8_t bytes_out[32], uint32_t cv_سۆزلەر[8]) {
  store32(&bytes_out[0 * 4], cv_سۆزلەر[0]);
  store32(&bytes_out[1 * 4], cv_سۆزلەر[1]);
  store32(&bytes_out[2 * 4], cv_سۆزلەر[2]);
  store32(&bytes_out[3 * 4], cv_سۆزلەر[3]);
  store32(&bytes_out[4 * 4], cv_سۆزلەر[4]);
  store32(&bytes_out[5 * 4], cv_سۆزلەر[5]);
  store32(&bytes_out[6 * 4], cv_سۆزلەر[6]);
  store32(&bytes_out[7 * 4], cv_سۆزلەر[7]);
}

LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_in_place(uint32_t cv[8],
                              ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_xof(ثابت uint32_t cv[8],
                         ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_xof_many(ثابت uint32_t cv[8],
                     ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many(ثابت uint8_t *ثابت *inputs, size_t رەقەم_inputs,
                      size_t blocks, ثابت uint32_t ئاچقۇچ_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out);

LLVM_LIBRARY_VISIBILITY
size_t blake3_simd_degree(فراغ);

BLAKE3_PRIVATE size_t blake3_compress_subtree_wide(ثابت uint8_t *كىرگۈزۈش_2, size_t كىرگۈزۈش_len,
                                                   ثابت uint32_t ئاچقۇچ_2[8],
                                                   uint64_t chunk_counter, uint8_t flags,
                                                   uint8_t *out, bool use_tbb);

#إذا defined(BLAKE3_USE_TBB)
BLAKE3_PRIVATE فراغ blake3_compress_subtree_wide_join_tbb(
    // shared params
    ثابت uint32_t ئاچقۇچ_2[8], uint8_t flags, bool use_tbb,
    // left-hand side params
    ثابت uint8_t *l_كىرگۈزۈش, size_t l_كىرگۈزۈش_len, uint64_t l_chunk_counter,
    uint8_t *l_cvs, size_t *l_n,
    // right-hand side params
    ثابت uint8_t *r_كىرگۈزۈش, size_t r_كىرگۈزۈش_len, uint64_t r_chunk_counter,
    uint8_t *r_cvs, size_t *r_n) NOEXCEPT;
#endif

// Declarations for implementation-specific functions.
LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_in_place_portable(uint32_t cv[8],
                                       ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                                       uint8_t block_len, uint64_t counter,
                                       uint8_t flags);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_xof_portable(ثابت uint32_t cv[8],
                                  ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                                  uint8_t block_len, uint64_t counter,
                                  uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many_portable(ثابت uint8_t *ثابت *inputs, size_t رەقەم_inputs,
                               size_t blocks, ثابت uint32_t ئاچقۇچ_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_end, uint8_t *out);

#إذا defined(IS_X86)
#إذا !defined(BLAKE3_NO_SSE2)
LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_in_place_sse2(uint32_t cv[8],
                                   ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                                   uint8_t block_len, uint64_t counter,
                                   uint8_t flags);
LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_xof_sse2(ثابت uint32_t cv[8],
                              ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many_sse2(ثابت uint8_t *ثابت *inputs, size_t رەقەم_inputs,
                           size_t blocks, ثابت uint32_t ئاچقۇچ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#إذا !defined(BLAKE3_NO_SSE41)
LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_in_place_sse41(uint32_t cv[8],
                                    ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                                    uint8_t block_len, uint64_t counter,
                                    uint8_t flags);
LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_xof_sse41(ثابت uint32_t cv[8],
                               ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                               uint8_t block_len, uint64_t counter,
                               uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many_sse41(ثابت uint8_t *ثابت *inputs, size_t رەقەم_inputs,
                            size_t blocks, ثابت uint32_t ئاچقۇچ_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_start,
                            uint8_t flags_end, uint8_t *out);
#endif
#إذا !defined(BLAKE3_NO_AVX2)
LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many_avx2(ثابت uint8_t *ثابت *inputs, size_t رەقەم_inputs,
                           size_t blocks, ثابت uint32_t ئاچقۇچ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#إذا !defined(BLAKE3_NO_AVX512)
LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_in_place_avx512(uint32_t cv[8],
                                     ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                                     uint8_t block_len, uint64_t counter,
                                     uint8_t flags);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_xof_avx512(ثابت uint32_t cv[8],
                                ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                                uint8_t block_len, uint64_t counter,
                                uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many_avx512(ثابت uint8_t *ثابت *inputs, size_t رەقەم_inputs,
                             size_t blocks, ثابت uint32_t ئاچقۇچ_2[8],
                             uint64_t counter, bool increment_counter,
                             uint8_t flags, uint8_t flags_start,
                             uint8_t flags_end, uint8_t *out);

#إذا !defined(_WIN32) && !defined(__CYGWIN__)
LLVM_LIBRARY_VISIBILITY
فراغ blake3_xof_many_avx512(ثابت uint32_t cv[8],
                            ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                            uint8_t block_len, uint64_t counter, uint8_t flags,
                            uint8_t* out, size_t outblocks);
#endif
#endif
#endif

#إذا BLAKE3_USE_NEON == 1
LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many_neon(ثابت uint8_t *ثابت *inputs, size_t رەقەم_inputs,
                           size_t blocks, ثابت uint32_t ئاچقۇچ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif


#endif /* BLAKE3_IMPL_H */
