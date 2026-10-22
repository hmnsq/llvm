#ifndef BLAKE3_IMPL_H
#define BLAKE3_IMPL_H

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "llvm-c/blake3.h"
// For \p LLVM_LIBRARY_VISIBILITY
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"

#ಅನ್ತರ್ಭಾವ "llvm_blake3_prefix.h"

#define BLAKE3_PRIVATE

// internal flags
ಗಣನಾ blake3_flags {
  CHUNK_START         = 1 << 0,
  CHUNK_END           = 1 << 1,
  PARENT              = 1 << 2,
  ಬೇರು                = 1 << 3,
  KEYED_HASH          = 1 << 4,
  DERIVE_ಕೀಲಿ_CONTEXT  = 1 << 5,
  DERIVE_ಕೀಲಿ_MATERIAL = 1 << 6,
};

// This C implementation tries to support recent versions of GCC, Clang, and
// MSVC.
#ಯದಿ defined(_MSC_VER)
#define INLINE ಸ್ಥಿರ __forceinline
#ಅನ್ಯಥಾ
#define INLINE ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ __attribute__((always_inline))
#endif

#ಯದಿ (defined(__x86_64__) || defined(_M_X64)) && !defined(_M_ARM64EC)
#define IS_X86
#define IS_X86_64
#endif

#ಯದಿ defined(__i386__) || defined(_M_IX86)
#define IS_X86
#define IS_X86_32
#endif

#ಯದಿ defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define IS_AARCH64
#endif

#ಯದಿ defined(IS_X86)
#ಯದಿ defined(_MSC_VER)
#ಅನ್ತರ್ಭಾವ <intrin.h>
#endif
#ಅನ್ತರ್ಭಾವ <immintrin.h>
#endif

#ಯದಿ !defined(BLAKE3_USE_NEON) 
  // If BLAKE3_USE_NEON not manually set, autodetect based on AArch64ness
  #ಯದಿ defined(IS_AARCH64)
    #ಯದಿ defined(__ARM_BIG_ENDIAN)
      #define BLAKE3_USE_NEON 0
    #ಅನ್ಯಥಾ
      #define BLAKE3_USE_NEON 1
    #endif
  #ಅನ್ಯಥಾ
    #define BLAKE3_USE_NEON 0
  #endif
#endif

#ಯದಿ defined(IS_X86)
#define MAX_SIMD_DEGREE 16
#ನೋಚೇದ್ಯದಿ BLAKE3_USE_NEON == 1
#define MAX_SIMD_DEGREE 4
#ಅನ್ಯಥಾ
#define MAX_SIMD_DEGREE 1
#endif

// There are some places where we want a static size that's equal to the
// MAX_SIMD_DEGREE, but also at least 2.
#define MAX_SIMD_DEGREE_OR_2 (MAX_SIMD_DEGREE > 2 ? MAX_SIMD_DEGREE : 2)

ಸ್ಥಿರ ಧ್ರುವ uint32_t IV[8] = {0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL,
                               0xA54FF53AUL, 0x510E527FUL, 0x9B05688CUL,
                               0x1F83D9ABUL, 0x5BE0CD19UL};

ಸ್ಥಿರ ಧ್ರುವ uint8_t ಸಂದೇಶ_SCHEDULE[7][16] = {
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
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ highest_one(uint64_t x) {
#ಯದಿ defined(__GNUC__) || defined(__clang__)
  ಪ್ರತ್ಯಾಗಮ 63 ^ (unsigned ಪೂರ್ಣಾಙ್ಕ)__builtin_clzll(x);
#ನೋಚೇದ್ಯದಿ defined(_MSC_VER) && defined(IS_X86_64)
  unsigned long index;
  _BitScanReverse64(&index, x);
  ಪ್ರತ್ಯಾಗಮ index;
#ನೋಚೇದ್ಯದಿ defined(_MSC_VER) && defined(IS_X86_32)
  ಯದಿ(x >> 32) {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)(x >> 32));
    ಪ್ರತ್ಯಾಗಮ 32 + index;
  } ಅನ್ಯಥಾ {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)x);
    ಪ್ರತ್ಯಾಗಮ index;
  }
#ಅನ್ಯಥಾ
  unsigned ಪೂರ್ಣಾಙ್ಕ c = 0;
  ಯದಿ(x & 0xffffffff00000000ULL) { x >>= 32; c += 32; }
  ಯದಿ(x & 0x00000000ffff0000ULL) { x >>= 16; c += 16; }
  ಯದಿ(x & 0x000000000000ff00ULL) { x >>=  8; c +=  8; }
  ಯದಿ(x & 0x00000000000000f0ULL) { x >>=  4; c +=  4; }
  ಯದಿ(x & 0x000000000000000cULL) { x >>=  2; c +=  2; }
  ಯದಿ(x & 0x0000000000000002ULL) {           c +=  1; }
  ಪ್ರತ್ಯಾಗಮ c;
#endif
}

// Count the number of 1 bits.
INLINE unsigned ಪೂರ್ಣಾಙ್ಕ popcnt(uint64_t x) {
#ಯದಿ defined(__GNUC__) || defined(__clang__)
  ಪ್ರತ್ಯಾಗಮ (unsigned ಪೂರ್ಣಾಙ್ಕ)__builtin_popcountll(x);
#ಅನ್ಯಥಾ
  unsigned ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;
  ಯಾವತ್ (x != 0) {
    ಎಣಿಕೆ += 1;
    x &= x - 1;
  }
  ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ;
#endif
}

// Largest power of two less than or equal to x. As a special case, returns 1
// when x is 0. 
INLINE uint64_t round_down_to_power_of_2(uint64_t x) {
  ಪ್ರತ್ಯಾಗಮ 1ULL << highest_one(x | 1);
}

INLINE uint32_t counter_ಕೆಳಗಿನ(uint64_t counter) { ಪ್ರತ್ಯಾಗಮ (uint32_t)counter; }

INLINE uint32_t counter_ಮೇಲಿನ(uint64_t counter) {
  ಪ್ರತ್ಯಾಗಮ (uint32_t)(counter >> 32);
}

INLINE uint32_t load32(ಧ್ರುವ ಶೂನ್ಯ *src) {
  ಧ್ರುವ uint8_t *p = (ಧ್ರುವ uint8_t *)src;
  ಪ್ರತ್ಯಾಗಮ ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

INLINE ಶೂನ್ಯ load_ಕೀಲಿ_ಪದಗಳು(ಧ್ರುವ uint8_t ಕೀಲಿ_2[BLAKE3_ಕೀಲಿ_LEN],
                           uint32_t ಕೀಲಿ_ಪದಗಳು[8]) {
  ಕೀಲಿ_ಪದಗಳು[0] = load32(&ಕೀಲಿ_2[0 * 4]);
  ಕೀಲಿ_ಪದಗಳು[1] = load32(&ಕೀಲಿ_2[1 * 4]);
  ಕೀಲಿ_ಪದಗಳು[2] = load32(&ಕೀಲಿ_2[2 * 4]);
  ಕೀಲಿ_ಪದಗಳು[3] = load32(&ಕೀಲಿ_2[3 * 4]);
  ಕೀಲಿ_ಪದಗಳು[4] = load32(&ಕೀಲಿ_2[4 * 4]);
  ಕೀಲಿ_ಪದಗಳು[5] = load32(&ಕೀಲಿ_2[5 * 4]);
  ಕೀಲಿ_ಪದಗಳು[6] = load32(&ಕೀಲಿ_2[6 * 4]);
  ಕೀಲಿ_ಪದಗಳು[7] = load32(&ಕೀಲಿ_2[7 * 4]);
}

INLINE ಶೂನ್ಯ load_block_ಪದಗಳು(ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                             uint32_t block_ಪದಗಳು[16]) {
  ಪ್ರತಿ (size_t i = 0; i < 16; i++) {
      block_ಪದಗಳು[i] = load32(&block[i * 4]);
  }
}

INLINE ಶೂನ್ಯ store32(ಶೂನ್ಯ *dst, uint32_t w) {
  uint8_t *p = (uint8_t *)dst;
  p[0] = (uint8_t)(w >> 0);
  p[1] = (uint8_t)(w >> 8);
  p[2] = (uint8_t)(w >> 16);
  p[3] = (uint8_t)(w >> 24);
}

INLINE ಶೂನ್ಯ store_cv_ಪದಗಳು(uint8_t bytes_out[32], uint32_t cv_ಪದಗಳು[8]) {
  store32(&bytes_out[0 * 4], cv_ಪದಗಳು[0]);
  store32(&bytes_out[1 * 4], cv_ಪದಗಳು[1]);
  store32(&bytes_out[2 * 4], cv_ಪದಗಳು[2]);
  store32(&bytes_out[3 * 4], cv_ಪದಗಳು[3]);
  store32(&bytes_out[4 * 4], cv_ಪದಗಳು[4]);
  store32(&bytes_out[5 * 4], cv_ಪದಗಳು[5]);
  store32(&bytes_out[6 * 4], cv_ಪದಗಳು[6]);
  store32(&bytes_out[7 * 4], cv_ಪದಗಳು[7]);
}

LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_compress_in_place(uint32_t cv[8],
                              ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags);

LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_compress_xof(ಧ್ರುವ uint32_t cv[8],
                         ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_xof_many(ಧ್ರುವ uint32_t cv[8],
                     ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks);

LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_hash_many(ಧ್ರುವ uint8_t *ಧ್ರುವ *inputs, size_t ಅಂಕೆ_inputs,
                      size_t blocks, ಧ್ರುವ uint32_t ಕೀಲಿ_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out);

LLVM_LIBRARY_VISIBILITY
size_t blake3_simd_degree(ಶೂನ್ಯ);

BLAKE3_PRIVATE size_t blake3_compress_subtree_wide(ಧ್ರುವ uint8_t *ಒಳಹರಿವು_2, size_t ಒಳಹರಿವು_len,
                                                   ಧ್ರುವ uint32_t ಕೀಲಿ_2[8],
                                                   uint64_t chunk_counter, uint8_t flags,
                                                   uint8_t *out, bool use_tbb);

#ಯದಿ defined(BLAKE3_USE_TBB)
BLAKE3_PRIVATE ಶೂನ್ಯ blake3_compress_subtree_wide_join_tbb(
    // shared params
    ಧ್ರುವ uint32_t ಕೀಲಿ_2[8], uint8_t flags, bool use_tbb,
    // left-hand side params
    ಧ್ರುವ uint8_t *l_ಒಳಹರಿವು, size_t l_ಒಳಹರಿವು_len, uint64_t l_chunk_counter,
    uint8_t *l_cvs, size_t *l_n,
    // right-hand side params
    ಧ್ರುವ uint8_t *r_ಒಳಹರಿವು, size_t r_ಒಳಹರಿವು_len, uint64_t r_chunk_counter,
    uint8_t *r_cvs, size_t *r_n) NOEXCEPT;
#endif

// Declarations for implementation-specific functions.
LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_compress_in_place_portable(uint32_t cv[8],
                                       ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                                       uint8_t block_len, uint64_t counter,
                                       uint8_t flags);

LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_compress_xof_portable(ಧ್ರುವ uint32_t cv[8],
                                  ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                                  uint8_t block_len, uint64_t counter,
                                  uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_hash_many_portable(ಧ್ರುವ uint8_t *ಧ್ರುವ *inputs, size_t ಅಂಕೆ_inputs,
                               size_t blocks, ಧ್ರುವ uint32_t ಕೀಲಿ_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_end, uint8_t *out);

#ಯದಿ defined(IS_X86)
#ಯದಿ !defined(BLAKE3_NO_SSE2)
LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_compress_in_place_sse2(uint32_t cv[8],
                                   ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                                   uint8_t block_len, uint64_t counter,
                                   uint8_t flags);
LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_compress_xof_sse2(ಧ್ರುವ uint32_t cv[8],
                              ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_hash_many_sse2(ಧ್ರುವ uint8_t *ಧ್ರುವ *inputs, size_t ಅಂಕೆ_inputs,
                           size_t blocks, ಧ್ರುವ uint32_t ಕೀಲಿ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#ಯದಿ !defined(BLAKE3_NO_SSE41)
LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_compress_in_place_sse41(uint32_t cv[8],
                                    ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                                    uint8_t block_len, uint64_t counter,
                                    uint8_t flags);
LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_compress_xof_sse41(ಧ್ರುವ uint32_t cv[8],
                               ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                               uint8_t block_len, uint64_t counter,
                               uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_hash_many_sse41(ಧ್ರುವ uint8_t *ಧ್ರುವ *inputs, size_t ಅಂಕೆ_inputs,
                            size_t blocks, ಧ್ರುವ uint32_t ಕೀಲಿ_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_start,
                            uint8_t flags_end, uint8_t *out);
#endif
#ಯದಿ !defined(BLAKE3_NO_AVX2)
LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_hash_many_avx2(ಧ್ರುವ uint8_t *ಧ್ರುವ *inputs, size_t ಅಂಕೆ_inputs,
                           size_t blocks, ಧ್ರುವ uint32_t ಕೀಲಿ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#ಯದಿ !defined(BLAKE3_NO_AVX512)
LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_compress_in_place_avx512(uint32_t cv[8],
                                     ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                                     uint8_t block_len, uint64_t counter,
                                     uint8_t flags);

LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_compress_xof_avx512(ಧ್ರುವ uint32_t cv[8],
                                ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                                uint8_t block_len, uint64_t counter,
                                uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_hash_many_avx512(ಧ್ರುವ uint8_t *ಧ್ರುವ *inputs, size_t ಅಂಕೆ_inputs,
                             size_t blocks, ಧ್ರುವ uint32_t ಕೀಲಿ_2[8],
                             uint64_t counter, bool increment_counter,
                             uint8_t flags, uint8_t flags_start,
                             uint8_t flags_end, uint8_t *out);

#ಯದಿ !defined(_WIN32) && !defined(__CYGWIN__)
LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_xof_many_avx512(ಧ್ರುವ uint32_t cv[8],
                            ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                            uint8_t block_len, uint64_t counter, uint8_t flags,
                            uint8_t* out, size_t outblocks);
#endif
#endif
#endif

#ಯದಿ BLAKE3_USE_NEON == 1
LLVM_LIBRARY_VISIBILITY
ಶೂನ್ಯ blake3_hash_many_neon(ಧ್ರುವ uint8_t *ಧ್ರುವ *inputs, size_t ಅಂಕೆ_inputs,
                           size_t blocks, ಧ್ರುವ uint32_t ಕೀಲಿ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif


#endif /* BLAKE3_IMPL_H */
