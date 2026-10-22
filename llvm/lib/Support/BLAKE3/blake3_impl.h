#ifndef BLAKE3_IMPL_H
#define BLAKE3_IMPL_H

#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stddef.h>
#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "llvm-c/blake3.h"
// For \p LLVM_LIBRARY_VISIBILITY
#অন্তর্ভাৱ "llvm/Support/Compiler.h"

#অন্তর্ভাৱ "llvm_blake3_prefix.h"

#define BLAKE3_PRIVATE

// internal flags
গণনা blake3_flags {
  CHUNK_START         = 1 << 0,
  CHUNK_END           = 1 << 1,
  PARENT              = 1 << 2,
  মূল                = 1 << 3,
  KEYED_HASH          = 1 << 4,
  DERIVE_চাবি_CONTEXT  = 1 << 5,
  DERIVE_চাবি_MATERIAL = 1 << 6,
};

// This C implementation tries to support recent versions of GCC, Clang, and
// MSVC.
#যদি defined(_MSC_VER)
#define INLINE স্থির __forceinline
#অন্যথা
#define INLINE স্থির অন্তঃস্থ __attribute__((always_inline))
#endif

#যদি (defined(__x86_64__) || defined(_M_X64)) && !defined(_M_ARM64EC)
#define IS_X86
#define IS_X86_64
#endif

#যদি defined(__i386__) || defined(_M_IX86)
#define IS_X86
#define IS_X86_32
#endif

#যদি defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define IS_AARCH64
#endif

#যদি defined(IS_X86)
#যদি defined(_MSC_VER)
#অন্তর্ভাৱ <intrin.h>
#endif
#অন্তর্ভাৱ <immintrin.h>
#endif

#যদি !defined(BLAKE3_USE_NEON) 
  // If BLAKE3_USE_NEON not manually set, autodetect based on AArch64ness
  #যদি defined(IS_AARCH64)
    #যদি defined(__ARM_BIG_ENDIAN)
      #define BLAKE3_USE_NEON 0
    #অন্যথা
      #define BLAKE3_USE_NEON 1
    #endif
  #অন্যথা
    #define BLAKE3_USE_NEON 0
  #endif
#endif

#যদি defined(IS_X86)
#define MAX_SIMD_DEGREE 16
#নোচেদ্যদি BLAKE3_USE_NEON == 1
#define MAX_SIMD_DEGREE 4
#অন্যথা
#define MAX_SIMD_DEGREE 1
#endif

// There are some places where we want a static size that's equal to the
// MAX_SIMD_DEGREE, but also at least 2.
#define MAX_SIMD_DEGREE_OR_2 (MAX_SIMD_DEGREE > 2 ? MAX_SIMD_DEGREE : 2)

স্থির ধ্রুৱ uint32_t IV[8] = {0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL,
                               0xA54FF53AUL, 0x510E527FUL, 0x9B05688CUL,
                               0x1F83D9ABUL, 0x5BE0CD19UL};

স্থির ধ্রুৱ uint8_t বাৰ্তা_SCHEDULE[7][16] = {
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
স্থির unsigned পূর্ণাঙ্ক highest_one(uint64_t x) {
#যদি defined(__GNUC__) || defined(__clang__)
  ঘূৰাই 63 ^ (unsigned পূর্ণাঙ্ক)__builtin_clzll(x);
#নোচেদ্যদি defined(_MSC_VER) && defined(IS_X86_64)
  unsigned long index;
  _BitScanReverse64(&index, x);
  ঘূৰাই index;
#নোচেদ্যদি defined(_MSC_VER) && defined(IS_X86_32)
  যদি(x >> 32) {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)(x >> 32));
    ঘূৰাই 32 + index;
  } অন্যথা {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)x);
    ঘূৰাই index;
  }
#অন্যথা
  unsigned পূর্ণাঙ্ক c = 0;
  যদি(x & 0xffffffff00000000ULL) { x >>= 32; c += 32; }
  যদি(x & 0x00000000ffff0000ULL) { x >>= 16; c += 16; }
  যদি(x & 0x000000000000ff00ULL) { x >>=  8; c +=  8; }
  যদি(x & 0x00000000000000f0ULL) { x >>=  4; c +=  4; }
  যদি(x & 0x000000000000000cULL) { x >>=  2; c +=  2; }
  যদি(x & 0x0000000000000002ULL) {           c +=  1; }
  ঘূৰাই c;
#endif
}

// Count the number of 1 bits.
INLINE unsigned পূর্ণাঙ্ক popcnt(uint64_t x) {
#যদি defined(__GNUC__) || defined(__clang__)
  ঘূৰাই (unsigned পূর্ণাঙ্ক)__builtin_popcountll(x);
#অন্যথা
  unsigned পূর্ণাঙ্ক গুনতি = 0;
  যেতিয়ালৈ (x != 0) {
    গুনতি += 1;
    x &= x - 1;
  }
  ঘূৰাই গুনতি;
#endif
}

// Largest power of two less than or equal to x. As a special case, returns 1
// when x is 0. 
INLINE uint64_t round_down_to_power_of_2(uint64_t x) {
  ঘূৰাই 1ULL << highest_one(x | 1);
}

INLINE uint32_t counter_নিচেৰ(uint64_t counter) { ঘূৰাই (uint32_t)counter; }

INLINE uint32_t counter_উপৰেৰ(uint64_t counter) {
  ঘূৰাই (uint32_t)(counter >> 32);
}

INLINE uint32_t load32(ধ্রুৱ শূন্য *src) {
  ধ্রুৱ uint8_t *p = (ধ্রুৱ uint8_t *)src;
  ঘূৰাই ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

INLINE শূন্য load_চাবি_শব্দগুলি(ধ্রুৱ uint8_t চাবি_2[BLAKE3_চাবি_LEN],
                           uint32_t চাবি_শব্দগুলি[8]) {
  চাবি_শব্দগুলি[0] = load32(&চাবি_2[0 * 4]);
  চাবি_শব্দগুলি[1] = load32(&চাবি_2[1 * 4]);
  চাবি_শব্দগুলি[2] = load32(&চাবি_2[2 * 4]);
  চাবি_শব্দগুলি[3] = load32(&চাবি_2[3 * 4]);
  চাবি_শব্দগুলি[4] = load32(&চাবি_2[4 * 4]);
  চাবি_শব্দগুলি[5] = load32(&চাবি_2[5 * 4]);
  চাবি_শব্দগুলি[6] = load32(&চাবি_2[6 * 4]);
  চাবি_শব্দগুলি[7] = load32(&চাবি_2[7 * 4]);
}

INLINE শূন্য load_block_শব্দগুলি(ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                             uint32_t block_শব্দগুলি[16]) {
  বাবে (size_t i = 0; i < 16; i++) {
      block_শব্দগুলি[i] = load32(&block[i * 4]);
  }
}

INLINE শূন্য store32(শূন্য *dst, uint32_t w) {
  uint8_t *p = (uint8_t *)dst;
  p[0] = (uint8_t)(w >> 0);
  p[1] = (uint8_t)(w >> 8);
  p[2] = (uint8_t)(w >> 16);
  p[3] = (uint8_t)(w >> 24);
}

INLINE শূন্য store_cv_শব্দগুলি(uint8_t bytes_out[32], uint32_t cv_শব্দগুলি[8]) {
  store32(&bytes_out[0 * 4], cv_শব্দগুলি[0]);
  store32(&bytes_out[1 * 4], cv_শব্দগুলি[1]);
  store32(&bytes_out[2 * 4], cv_শব্দগুলি[2]);
  store32(&bytes_out[3 * 4], cv_শব্দগুলি[3]);
  store32(&bytes_out[4 * 4], cv_শব্দগুলি[4]);
  store32(&bytes_out[5 * 4], cv_শব্দগুলি[5]);
  store32(&bytes_out[6 * 4], cv_শব্দগুলি[6]);
  store32(&bytes_out[7 * 4], cv_শব্দগুলি[7]);
}

LLVM_LIBRARY_VISIBILITY
শূন্য blake3_compress_in_place(uint32_t cv[8],
                              ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags);

LLVM_LIBRARY_VISIBILITY
শূন্য blake3_compress_xof(ধ্রুৱ uint32_t cv[8],
                         ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
শূন্য blake3_xof_many(ধ্রুৱ uint32_t cv[8],
                     ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks);

LLVM_LIBRARY_VISIBILITY
শূন্য blake3_hash_many(ধ্রুৱ uint8_t *ধ্রুৱ *inputs, size_t অঙ্ক_inputs,
                      size_t blocks, ধ্রুৱ uint32_t চাবি_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out);

LLVM_LIBRARY_VISIBILITY
size_t blake3_simd_degree(শূন্য);

BLAKE3_PRIVATE size_t blake3_compress_subtree_wide(ধ্রুৱ uint8_t *প্ৰবেশ_2, size_t প্ৰবেশ_len,
                                                   ধ্রুৱ uint32_t চাবি_2[8],
                                                   uint64_t chunk_counter, uint8_t flags,
                                                   uint8_t *out, bool use_tbb);

#যদি defined(BLAKE3_USE_TBB)
BLAKE3_PRIVATE শূন্য blake3_compress_subtree_wide_join_tbb(
    // shared params
    ধ্রুৱ uint32_t চাবি_2[8], uint8_t flags, bool use_tbb,
    // left-hand side params
    ধ্রুৱ uint8_t *l_প্ৰবেশ, size_t l_প্ৰবেশ_len, uint64_t l_chunk_counter,
    uint8_t *l_cvs, size_t *l_n,
    // right-hand side params
    ধ্রুৱ uint8_t *r_প্ৰবেশ, size_t r_প্ৰবেশ_len, uint64_t r_chunk_counter,
    uint8_t *r_cvs, size_t *r_n) NOEXCEPT;
#endif

// Declarations for implementation-specific functions.
LLVM_LIBRARY_VISIBILITY
শূন্য blake3_compress_in_place_portable(uint32_t cv[8],
                                       ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                                       uint8_t block_len, uint64_t counter,
                                       uint8_t flags);

LLVM_LIBRARY_VISIBILITY
শূন্য blake3_compress_xof_portable(ধ্রুৱ uint32_t cv[8],
                                  ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                                  uint8_t block_len, uint64_t counter,
                                  uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
শূন্য blake3_hash_many_portable(ধ্রুৱ uint8_t *ধ্রুৱ *inputs, size_t অঙ্ক_inputs,
                               size_t blocks, ধ্রুৱ uint32_t চাবি_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_end, uint8_t *out);

#যদি defined(IS_X86)
#যদি !defined(BLAKE3_NO_SSE2)
LLVM_LIBRARY_VISIBILITY
শূন্য blake3_compress_in_place_sse2(uint32_t cv[8],
                                   ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                                   uint8_t block_len, uint64_t counter,
                                   uint8_t flags);
LLVM_LIBRARY_VISIBILITY
শূন্য blake3_compress_xof_sse2(ধ্রুৱ uint32_t cv[8],
                              ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
শূন্য blake3_hash_many_sse2(ধ্রুৱ uint8_t *ধ্রুৱ *inputs, size_t অঙ্ক_inputs,
                           size_t blocks, ধ্রুৱ uint32_t চাবি_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#যদি !defined(BLAKE3_NO_SSE41)
LLVM_LIBRARY_VISIBILITY
শূন্য blake3_compress_in_place_sse41(uint32_t cv[8],
                                    ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                                    uint8_t block_len, uint64_t counter,
                                    uint8_t flags);
LLVM_LIBRARY_VISIBILITY
শূন্য blake3_compress_xof_sse41(ধ্রুৱ uint32_t cv[8],
                               ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                               uint8_t block_len, uint64_t counter,
                               uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
শূন্য blake3_hash_many_sse41(ধ্রুৱ uint8_t *ধ্রুৱ *inputs, size_t অঙ্ক_inputs,
                            size_t blocks, ধ্রুৱ uint32_t চাবি_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_start,
                            uint8_t flags_end, uint8_t *out);
#endif
#যদি !defined(BLAKE3_NO_AVX2)
LLVM_LIBRARY_VISIBILITY
শূন্য blake3_hash_many_avx2(ধ্রুৱ uint8_t *ধ্রুৱ *inputs, size_t অঙ্ক_inputs,
                           size_t blocks, ধ্রুৱ uint32_t চাবি_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#যদি !defined(BLAKE3_NO_AVX512)
LLVM_LIBRARY_VISIBILITY
শূন্য blake3_compress_in_place_avx512(uint32_t cv[8],
                                     ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                                     uint8_t block_len, uint64_t counter,
                                     uint8_t flags);

LLVM_LIBRARY_VISIBILITY
শূন্য blake3_compress_xof_avx512(ধ্রুৱ uint32_t cv[8],
                                ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                                uint8_t block_len, uint64_t counter,
                                uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
শূন্য blake3_hash_many_avx512(ধ্রুৱ uint8_t *ধ্রুৱ *inputs, size_t অঙ্ক_inputs,
                             size_t blocks, ধ্রুৱ uint32_t চাবি_2[8],
                             uint64_t counter, bool increment_counter,
                             uint8_t flags, uint8_t flags_start,
                             uint8_t flags_end, uint8_t *out);

#যদি !defined(_WIN32) && !defined(__CYGWIN__)
LLVM_LIBRARY_VISIBILITY
শূন্য blake3_xof_many_avx512(ধ্রুৱ uint32_t cv[8],
                            ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                            uint8_t block_len, uint64_t counter, uint8_t flags,
                            uint8_t* out, size_t outblocks);
#endif
#endif
#endif

#যদি BLAKE3_USE_NEON == 1
LLVM_LIBRARY_VISIBILITY
শূন্য blake3_hash_many_neon(ধ্রুৱ uint8_t *ধ্রুৱ *inputs, size_t অঙ্ক_inputs,
                           size_t blocks, ধ্রুৱ uint32_t চাবি_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif


#endif /* BLAKE3_IMPL_H */
