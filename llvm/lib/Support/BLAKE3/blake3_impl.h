#ifndef BLAKE3_IMPL_H
#define BLAKE3_IMPL_H

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "llvm-c/blake3.h"
// For \p LLVM_LIBRARY_VISIBILITY
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"

#ଅନ୍ତର୍ଭାଵ "llvm_blake3_prefix.h"

#define BLAKE3_PRIVATE

// internal flags
ଗଣନା blake3_flags {
  CHUNK_START         = 1 << 0,
  CHUNK_END           = 1 << 1,
  PARENT              = 1 << 2,
  ମୂଳ                = 1 << 3,
  KEYED_HASH          = 1 << 4,
  DERIVE_ଚାବି_CONTEXT  = 1 << 5,
  DERIVE_ଚାବି_MATERIAL = 1 << 6,
};

// This C implementation tries to support recent versions of GCC, Clang, and
// MSVC.
#ଯଦି defined(_MSC_VER)
#define INLINE ସ୍ଥିର __forceinline
#ଅନ୍ୟଥା
#define INLINE ସ୍ଥିର ଧାଡ଼ିଭିତରେ __attribute__((always_inline))
#endif

#ଯଦି (defined(__x86_64__) || defined(_M_X64)) && !defined(_M_ARM64EC)
#define IS_X86
#define IS_X86_64
#endif

#ଯଦି defined(__i386__) || defined(_M_IX86)
#define IS_X86
#define IS_X86_32
#endif

#ଯଦି defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define IS_AARCH64
#endif

#ଯଦି defined(IS_X86)
#ଯଦି defined(_MSC_VER)
#ଅନ୍ତର୍ଭାଵ <intrin.h>
#endif
#ଅନ୍ତର୍ଭାଵ <immintrin.h>
#endif

#ଯଦି !defined(BLAKE3_USE_NEON) 
  // If BLAKE3_USE_NEON not manually set, autodetect based on AArch64ness
  #ଯଦି defined(IS_AARCH64)
    #ଯଦି defined(__ARM_BIG_ENDIAN)
      #define BLAKE3_USE_NEON 0
    #ଅନ୍ୟଥା
      #define BLAKE3_USE_NEON 1
    #endif
  #ଅନ୍ୟଥା
    #define BLAKE3_USE_NEON 0
  #endif
#endif

#ଯଦି defined(IS_X86)
#define MAX_SIMD_DEGREE 16
#ନଚେତଯଦି BLAKE3_USE_NEON == 1
#define MAX_SIMD_DEGREE 4
#ଅନ୍ୟଥା
#define MAX_SIMD_DEGREE 1
#endif

// There are some places where we want a static size that's equal to the
// MAX_SIMD_DEGREE, but also at least 2.
#define MAX_SIMD_DEGREE_OR_2 (MAX_SIMD_DEGREE > 2 ? MAX_SIMD_DEGREE : 2)

ସ୍ଥିର ଅଚଳ uint32_t IV[8] = {0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL,
                               0xA54FF53AUL, 0x510E527FUL, 0x9B05688CUL,
                               0x1F83D9ABUL, 0x5BE0CD19UL};

ସ୍ଥିର ଅଚଳ uint8_t ସନ୍ଦେଶ_SCHEDULE[7][16] = {
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
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା highest_one(uint64_t x) {
#ଯଦି defined(__GNUC__) || defined(__clang__)
  ଫେରନ୍ତୁ 63 ^ (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)__builtin_clzll(x);
#ନଚେତଯଦି defined(_MSC_VER) && defined(IS_X86_64)
  unsigned long index;
  _BitScanReverse64(&index, x);
  ଫେରନ୍ତୁ index;
#ନଚେତଯଦି defined(_MSC_VER) && defined(IS_X86_32)
  ଯଦି(x >> 32) {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)(x >> 32));
    ଫେରନ୍ତୁ 32 + index;
  } ଅନ୍ୟଥା {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)x);
    ଫେରନ୍ତୁ index;
  }
#ଅନ୍ୟଥା
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = 0;
  ଯଦି(x & 0xffffffff00000000ULL) { x >>= 32; c += 32; }
  ଯଦି(x & 0x00000000ffff0000ULL) { x >>= 16; c += 16; }
  ଯଦି(x & 0x000000000000ff00ULL) { x >>=  8; c +=  8; }
  ଯଦି(x & 0x00000000000000f0ULL) { x >>=  4; c +=  4; }
  ଯଦି(x & 0x000000000000000cULL) { x >>=  2; c +=  2; }
  ଯଦି(x & 0x0000000000000002ULL) {           c +=  1; }
  ଫେରନ୍ତୁ c;
#endif
}

// Count the number of 1 bits.
INLINE unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା popcnt(uint64_t x) {
#ଯଦି defined(__GNUC__) || defined(__clang__)
  ଫେରନ୍ତୁ (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)__builtin_popcountll(x);
#ଅନ୍ୟଥା
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;
  ଯେତେବେଳେ (x != 0) {
    ଗଣନା_2 += 1;
    x &= x - 1;
  }
  ଫେରନ୍ତୁ ଗଣନା_2;
#endif
}

// Largest power of two less than or equal to x. As a special case, returns 1
// when x is 0. 
INLINE uint64_t round_down_to_power_of_2(uint64_t x) {
  ଫେରନ୍ତୁ 1ULL << highest_one(x | 1);
}

INLINE uint32_t counter_ତଳ(uint64_t counter) { ଫେରନ୍ତୁ (uint32_t)counter; }

INLINE uint32_t counter_ଉପର(uint64_t counter) {
  ଫେରନ୍ତୁ (uint32_t)(counter >> 32);
}

INLINE uint32_t load32(ଅଚଳ ଶୂନ୍ୟ *src) {
  ଅଚଳ uint8_t *p = (ଅଚଳ uint8_t *)src;
  ଫେରନ୍ତୁ ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

INLINE ଶୂନ୍ୟ load_ଚାବି_ଶବ୍ଦଗୁଡ଼ିକ(ଅଚଳ uint8_t ଚାବି_2[BLAKE3_ଚାବି_LEN],
                           uint32_t ଚାବି_ଶବ୍ଦଗୁଡ଼ିକ[8]) {
  ଚାବି_ଶବ୍ଦଗୁଡ଼ିକ[0] = load32(&ଚାବି_2[0 * 4]);
  ଚାବି_ଶବ୍ଦଗୁଡ଼ିକ[1] = load32(&ଚାବି_2[1 * 4]);
  ଚାବି_ଶବ୍ଦଗୁଡ଼ିକ[2] = load32(&ଚାବି_2[2 * 4]);
  ଚାବି_ଶବ୍ଦଗୁଡ଼ିକ[3] = load32(&ଚାବି_2[3 * 4]);
  ଚାବି_ଶବ୍ଦଗୁଡ଼ିକ[4] = load32(&ଚାବି_2[4 * 4]);
  ଚାବି_ଶବ୍ଦଗୁଡ଼ିକ[5] = load32(&ଚାବି_2[5 * 4]);
  ଚାବି_ଶବ୍ଦଗୁଡ଼ିକ[6] = load32(&ଚାବି_2[6 * 4]);
  ଚାବି_ଶବ୍ଦଗୁଡ଼ିକ[7] = load32(&ଚାବି_2[7 * 4]);
}

INLINE ଶୂନ୍ୟ load_block_ଶବ୍ଦଗୁଡ଼ିକ(ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                             uint32_t block_ଶବ୍ଦଗୁଡ଼ିକ[16]) {
  ପାଇଁ (size_t i = 0; i < 16; i++) {
      block_ଶବ୍ଦଗୁଡ଼ିକ[i] = load32(&block[i * 4]);
  }
}

INLINE ଶୂନ୍ୟ store32(ଶୂନ୍ୟ *dst, uint32_t w) {
  uint8_t *p = (uint8_t *)dst;
  p[0] = (uint8_t)(w >> 0);
  p[1] = (uint8_t)(w >> 8);
  p[2] = (uint8_t)(w >> 16);
  p[3] = (uint8_t)(w >> 24);
}

INLINE ଶୂନ୍ୟ store_cv_ଶବ୍ଦଗୁଡ଼ିକ(uint8_t bytes_out[32], uint32_t cv_ଶବ୍ଦଗୁଡ଼ିକ[8]) {
  store32(&bytes_out[0 * 4], cv_ଶବ୍ଦଗୁଡ଼ିକ[0]);
  store32(&bytes_out[1 * 4], cv_ଶବ୍ଦଗୁଡ଼ିକ[1]);
  store32(&bytes_out[2 * 4], cv_ଶବ୍ଦଗୁଡ଼ିକ[2]);
  store32(&bytes_out[3 * 4], cv_ଶବ୍ଦଗୁଡ଼ିକ[3]);
  store32(&bytes_out[4 * 4], cv_ଶବ୍ଦଗୁଡ଼ିକ[4]);
  store32(&bytes_out[5 * 4], cv_ଶବ୍ଦଗୁଡ଼ିକ[5]);
  store32(&bytes_out[6 * 4], cv_ଶବ୍ଦଗୁଡ଼ିକ[6]);
  store32(&bytes_out[7 * 4], cv_ଶବ୍ଦଗୁଡ଼ିକ[7]);
}

LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_compress_in_place(uint32_t cv[8],
                              ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags);

LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_compress_xof(ଅଚଳ uint32_t cv[8],
                         ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_xof_many(ଅଚଳ uint32_t cv[8],
                     ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks);

LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_hash_many(ଅଚଳ uint8_t *ଅଚଳ *inputs, size_t ଅଙ୍କ_inputs,
                      size_t blocks, ଅଚଳ uint32_t ଚାବି_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out);

LLVM_LIBRARY_VISIBILITY
size_t blake3_simd_degree(ଶୂନ୍ୟ);

BLAKE3_PRIVATE size_t blake3_compress_subtree_wide(ଅଚଳ uint8_t *ନିବେଶ_2, size_t ନିବେଶ_len,
                                                   ଅଚଳ uint32_t ଚାବି_2[8],
                                                   uint64_t chunk_counter, uint8_t flags,
                                                   uint8_t *out, bool use_tbb);

#ଯଦି defined(BLAKE3_USE_TBB)
BLAKE3_PRIVATE ଶୂନ୍ୟ blake3_compress_subtree_wide_join_tbb(
    // shared params
    ଅଚଳ uint32_t ଚାବି_2[8], uint8_t flags, bool use_tbb,
    // left-hand side params
    ଅଚଳ uint8_t *l_ନିବେଶ, size_t l_ନିବେଶ_len, uint64_t l_chunk_counter,
    uint8_t *l_cvs, size_t *l_n,
    // right-hand side params
    ଅଚଳ uint8_t *r_ନିବେଶ, size_t r_ନିବେଶ_len, uint64_t r_chunk_counter,
    uint8_t *r_cvs, size_t *r_n) NOEXCEPT;
#endif

// Declarations for implementation-specific functions.
LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_compress_in_place_portable(uint32_t cv[8],
                                       ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                                       uint8_t block_len, uint64_t counter,
                                       uint8_t flags);

LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_compress_xof_portable(ଅଚଳ uint32_t cv[8],
                                  ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                                  uint8_t block_len, uint64_t counter,
                                  uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_hash_many_portable(ଅଚଳ uint8_t *ଅଚଳ *inputs, size_t ଅଙ୍କ_inputs,
                               size_t blocks, ଅଚଳ uint32_t ଚାବି_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_end, uint8_t *out);

#ଯଦି defined(IS_X86)
#ଯଦି !defined(BLAKE3_NO_SSE2)
LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_compress_in_place_sse2(uint32_t cv[8],
                                   ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                                   uint8_t block_len, uint64_t counter,
                                   uint8_t flags);
LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_compress_xof_sse2(ଅଚଳ uint32_t cv[8],
                              ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_hash_many_sse2(ଅଚଳ uint8_t *ଅଚଳ *inputs, size_t ଅଙ୍କ_inputs,
                           size_t blocks, ଅଚଳ uint32_t ଚାବି_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#ଯଦି !defined(BLAKE3_NO_SSE41)
LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_compress_in_place_sse41(uint32_t cv[8],
                                    ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                                    uint8_t block_len, uint64_t counter,
                                    uint8_t flags);
LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_compress_xof_sse41(ଅଚଳ uint32_t cv[8],
                               ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                               uint8_t block_len, uint64_t counter,
                               uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_hash_many_sse41(ଅଚଳ uint8_t *ଅଚଳ *inputs, size_t ଅଙ୍କ_inputs,
                            size_t blocks, ଅଚଳ uint32_t ଚାବି_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_start,
                            uint8_t flags_end, uint8_t *out);
#endif
#ଯଦି !defined(BLAKE3_NO_AVX2)
LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_hash_many_avx2(ଅଚଳ uint8_t *ଅଚଳ *inputs, size_t ଅଙ୍କ_inputs,
                           size_t blocks, ଅଚଳ uint32_t ଚାବି_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#ଯଦି !defined(BLAKE3_NO_AVX512)
LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_compress_in_place_avx512(uint32_t cv[8],
                                     ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                                     uint8_t block_len, uint64_t counter,
                                     uint8_t flags);

LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_compress_xof_avx512(ଅଚଳ uint32_t cv[8],
                                ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                                uint8_t block_len, uint64_t counter,
                                uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_hash_many_avx512(ଅଚଳ uint8_t *ଅଚଳ *inputs, size_t ଅଙ୍କ_inputs,
                             size_t blocks, ଅଚଳ uint32_t ଚାବି_2[8],
                             uint64_t counter, bool increment_counter,
                             uint8_t flags, uint8_t flags_start,
                             uint8_t flags_end, uint8_t *out);

#ଯଦି !defined(_WIN32) && !defined(__CYGWIN__)
LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_xof_many_avx512(ଅଚଳ uint32_t cv[8],
                            ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                            uint8_t block_len, uint64_t counter, uint8_t flags,
                            uint8_t* out, size_t outblocks);
#endif
#endif
#endif

#ଯଦି BLAKE3_USE_NEON == 1
LLVM_LIBRARY_VISIBILITY
ଶୂନ୍ୟ blake3_hash_many_neon(ଅଚଳ uint8_t *ଅଚଳ *inputs, size_t ଅଙ୍କ_inputs,
                           size_t blocks, ଅଚଳ uint32_t ଚାବି_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif


#endif /* BLAKE3_IMPL_H */
