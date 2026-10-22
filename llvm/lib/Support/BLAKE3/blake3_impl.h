#ifndef BLAKE3_IMPL_H
#define BLAKE3_IMPL_H

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "llvm-c/blake3.h"
// For \p LLVM_LIBRARY_VISIBILITY
#அந்தர்பாவ "llvm/Support/Compiler.h"

#அந்தர்பாவ "llvm_blake3_prefix.h"

#define BLAKE3_PRIVATE

// internal flags
எண்ணிக்கை blake3_flags {
  CHUNK_START         = 1 << 0,
  CHUNK_END           = 1 << 1,
  PARENT              = 1 << 2,
  வேர்                = 1 << 3,
  KEYED_HASH          = 1 << 4,
  DERIVE_சாவி_CONTEXT  = 1 << 5,
  DERIVE_சாவி_MATERIAL = 1 << 6,
};

// This C implementation tries to support recent versions of GCC, Clang, and
// MSVC.
#எனில் defined(_MSC_VER)
#define INLINE நிலையான __forceinline
#இல்லையெனில்
#define INLINE நிலையான உள்வரி __attribute__((always_inline))
#endif

#எனில் (defined(__x86_64__) || defined(_M_X64)) && !defined(_M_ARM64EC)
#define IS_X86
#define IS_X86_64
#endif

#எனில் defined(__i386__) || defined(_M_IX86)
#define IS_X86
#define IS_X86_32
#endif

#எனில் defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define IS_AARCH64
#endif

#எனில் defined(IS_X86)
#எனில் defined(_MSC_VER)
#அந்தர்பாவ <intrin.h>
#endif
#அந்தர்பாவ <immintrin.h>
#endif

#எனில் !defined(BLAKE3_USE_NEON) 
  // If BLAKE3_USE_NEON not manually set, autodetect based on AArch64ness
  #எனில் defined(IS_AARCH64)
    #எனில் defined(__ARM_BIG_ENDIAN)
      #define BLAKE3_USE_NEON 0
    #இல்லையெனில்
      #define BLAKE3_USE_NEON 1
    #endif
  #இல்லையெனில்
    #define BLAKE3_USE_NEON 0
  #endif
#endif

#எனில் defined(IS_X86)
#define MAX_SIMD_DEGREE 16
#அல்லதெனில் BLAKE3_USE_NEON == 1
#define MAX_SIMD_DEGREE 4
#இல்லையெனில்
#define MAX_SIMD_DEGREE 1
#endif

// There are some places where we want a static size that's equal to the
// MAX_SIMD_DEGREE, but also at least 2.
#define MAX_SIMD_DEGREE_OR_2 (MAX_SIMD_DEGREE > 2 ? MAX_SIMD_DEGREE : 2)

நிலையான மாறிலி uint32_t IV[8] = {0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL,
                               0xA54FF53AUL, 0x510E527FUL, 0x9B05688CUL,
                               0x1F83D9ABUL, 0x5BE0CD19UL};

நிலையான மாறிலி uint8_t செய்தி_SCHEDULE[7][16] = {
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
நிலையான unsigned முழு highest_one(uint64_t x) {
#எனில் defined(__GNUC__) || defined(__clang__)
  திரும்பு 63 ^ (unsigned முழு)__builtin_clzll(x);
#அல்லதெனில் defined(_MSC_VER) && defined(IS_X86_64)
  unsigned long index;
  _BitScanReverse64(&index, x);
  திரும்பு index;
#அல்லதெனில் defined(_MSC_VER) && defined(IS_X86_32)
  எனில்(x >> 32) {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)(x >> 32));
    திரும்பு 32 + index;
  } இல்லையெனில் {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)x);
    திரும்பு index;
  }
#இல்லையெனில்
  unsigned முழு c = 0;
  எனில்(x & 0xffffffff00000000ULL) { x >>= 32; c += 32; }
  எனில்(x & 0x00000000ffff0000ULL) { x >>= 16; c += 16; }
  எனில்(x & 0x000000000000ff00ULL) { x >>=  8; c +=  8; }
  எனில்(x & 0x00000000000000f0ULL) { x >>=  4; c +=  4; }
  எனில்(x & 0x000000000000000cULL) { x >>=  2; c +=  2; }
  எனில்(x & 0x0000000000000002ULL) {           c +=  1; }
  திரும்பு c;
#endif
}

// Count the number of 1 bits.
INLINE unsigned முழு popcnt(uint64_t x) {
#எனில் defined(__GNUC__) || defined(__clang__)
  திரும்பு (unsigned முழு)__builtin_popcountll(x);
#இல்லையெனில்
  unsigned முழு மொத்தம் = 0;
  போது (x != 0) {
    மொத்தம் += 1;
    x &= x - 1;
  }
  திரும்பு மொத்தம்;
#endif
}

// Largest power of two less than or equal to x. As a special case, returns 1
// when x is 0. 
INLINE uint64_t round_down_to_power_of_2(uint64_t x) {
  திரும்பு 1ULL << highest_one(x | 1);
}

INLINE uint32_t counter_கீழ்(uint64_t counter) { திரும்பு (uint32_t)counter; }

INLINE uint32_t counter_உயர்வு(uint64_t counter) {
  திரும்பு (uint32_t)(counter >> 32);
}

INLINE uint32_t load32(மாறிலி வெறுமை *src) {
  மாறிலி uint8_t *p = (மாறிலி uint8_t *)src;
  திரும்பு ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

INLINE வெறுமை load_சாவி_சொற்கள்(மாறிலி uint8_t சாவி_2[BLAKE3_சாவி_LEN],
                           uint32_t சாவி_சொற்கள்[8]) {
  சாவி_சொற்கள்[0] = load32(&சாவி_2[0 * 4]);
  சாவி_சொற்கள்[1] = load32(&சாவி_2[1 * 4]);
  சாவி_சொற்கள்[2] = load32(&சாவி_2[2 * 4]);
  சாவி_சொற்கள்[3] = load32(&சாவி_2[3 * 4]);
  சாவி_சொற்கள்[4] = load32(&சாவி_2[4 * 4]);
  சாவி_சொற்கள்[5] = load32(&சாவி_2[5 * 4]);
  சாவி_சொற்கள்[6] = load32(&சாவி_2[6 * 4]);
  சாவி_சொற்கள்[7] = load32(&சாவி_2[7 * 4]);
}

INLINE வெறுமை load_block_சொற்கள்(மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                             uint32_t block_சொற்கள்[16]) {
  க்கு (size_t i = 0; i < 16; i++) {
      block_சொற்கள்[i] = load32(&block[i * 4]);
  }
}

INLINE வெறுமை store32(வெறுமை *dst, uint32_t w) {
  uint8_t *p = (uint8_t *)dst;
  p[0] = (uint8_t)(w >> 0);
  p[1] = (uint8_t)(w >> 8);
  p[2] = (uint8_t)(w >> 16);
  p[3] = (uint8_t)(w >> 24);
}

INLINE வெறுமை store_cv_சொற்கள்(uint8_t bytes_out[32], uint32_t cv_சொற்கள்[8]) {
  store32(&bytes_out[0 * 4], cv_சொற்கள்[0]);
  store32(&bytes_out[1 * 4], cv_சொற்கள்[1]);
  store32(&bytes_out[2 * 4], cv_சொற்கள்[2]);
  store32(&bytes_out[3 * 4], cv_சொற்கள்[3]);
  store32(&bytes_out[4 * 4], cv_சொற்கள்[4]);
  store32(&bytes_out[5 * 4], cv_சொற்கள்[5]);
  store32(&bytes_out[6 * 4], cv_சொற்கள்[6]);
  store32(&bytes_out[7 * 4], cv_சொற்கள்[7]);
}

LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_compress_in_place(uint32_t cv[8],
                              மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags);

LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_compress_xof(மாறிலி uint32_t cv[8],
                         மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_xof_many(மாறிலி uint32_t cv[8],
                     மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks);

LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_hash_many(மாறிலி uint8_t *மாறிலி *inputs, size_t இலக்கம்_inputs,
                      size_t blocks, மாறிலி uint32_t சாவி_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out);

LLVM_LIBRARY_VISIBILITY
size_t blake3_simd_degree(வெறுமை);

BLAKE3_PRIVATE size_t blake3_compress_subtree_wide(மாறிலி uint8_t *உள்ளீடு_2, size_t உள்ளீடு_len,
                                                   மாறிலி uint32_t சாவி_2[8],
                                                   uint64_t chunk_counter, uint8_t flags,
                                                   uint8_t *out, bool use_tbb);

#எனில் defined(BLAKE3_USE_TBB)
BLAKE3_PRIVATE வெறுமை blake3_compress_subtree_wide_join_tbb(
    // shared params
    மாறிலி uint32_t சாவி_2[8], uint8_t flags, bool use_tbb,
    // left-hand side params
    மாறிலி uint8_t *l_உள்ளீடு, size_t l_உள்ளீடு_len, uint64_t l_chunk_counter,
    uint8_t *l_cvs, size_t *l_n,
    // right-hand side params
    மாறிலி uint8_t *r_உள்ளீடு, size_t r_உள்ளீடு_len, uint64_t r_chunk_counter,
    uint8_t *r_cvs, size_t *r_n) NOEXCEPT;
#endif

// Declarations for implementation-specific functions.
LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_compress_in_place_portable(uint32_t cv[8],
                                       மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                                       uint8_t block_len, uint64_t counter,
                                       uint8_t flags);

LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_compress_xof_portable(மாறிலி uint32_t cv[8],
                                  மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                                  uint8_t block_len, uint64_t counter,
                                  uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_hash_many_portable(மாறிலி uint8_t *மாறிலி *inputs, size_t இலக்கம்_inputs,
                               size_t blocks, மாறிலி uint32_t சாவி_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_end, uint8_t *out);

#எனில் defined(IS_X86)
#எனில் !defined(BLAKE3_NO_SSE2)
LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_compress_in_place_sse2(uint32_t cv[8],
                                   மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                                   uint8_t block_len, uint64_t counter,
                                   uint8_t flags);
LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_compress_xof_sse2(மாறிலி uint32_t cv[8],
                              மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_hash_many_sse2(மாறிலி uint8_t *மாறிலி *inputs, size_t இலக்கம்_inputs,
                           size_t blocks, மாறிலி uint32_t சாவி_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#எனில் !defined(BLAKE3_NO_SSE41)
LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_compress_in_place_sse41(uint32_t cv[8],
                                    மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                                    uint8_t block_len, uint64_t counter,
                                    uint8_t flags);
LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_compress_xof_sse41(மாறிலி uint32_t cv[8],
                               மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                               uint8_t block_len, uint64_t counter,
                               uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_hash_many_sse41(மாறிலி uint8_t *மாறிலி *inputs, size_t இலக்கம்_inputs,
                            size_t blocks, மாறிலி uint32_t சாவி_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_start,
                            uint8_t flags_end, uint8_t *out);
#endif
#எனில் !defined(BLAKE3_NO_AVX2)
LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_hash_many_avx2(மாறிலி uint8_t *மாறிலி *inputs, size_t இலக்கம்_inputs,
                           size_t blocks, மாறிலி uint32_t சாவி_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#எனில் !defined(BLAKE3_NO_AVX512)
LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_compress_in_place_avx512(uint32_t cv[8],
                                     மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                                     uint8_t block_len, uint64_t counter,
                                     uint8_t flags);

LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_compress_xof_avx512(மாறிலி uint32_t cv[8],
                                மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                                uint8_t block_len, uint64_t counter,
                                uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_hash_many_avx512(மாறிலி uint8_t *மாறிலி *inputs, size_t இலக்கம்_inputs,
                             size_t blocks, மாறிலி uint32_t சாவி_2[8],
                             uint64_t counter, bool increment_counter,
                             uint8_t flags, uint8_t flags_start,
                             uint8_t flags_end, uint8_t *out);

#எனில் !defined(_WIN32) && !defined(__CYGWIN__)
LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_xof_many_avx512(மாறிலி uint32_t cv[8],
                            மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                            uint8_t block_len, uint64_t counter, uint8_t flags,
                            uint8_t* out, size_t outblocks);
#endif
#endif
#endif

#எனில் BLAKE3_USE_NEON == 1
LLVM_LIBRARY_VISIBILITY
வெறுமை blake3_hash_many_neon(மாறிலி uint8_t *மாறிலி *inputs, size_t இலக்கம்_inputs,
                           size_t blocks, மாறிலி uint32_t சாவி_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif


#endif /* BLAKE3_IMPL_H */
