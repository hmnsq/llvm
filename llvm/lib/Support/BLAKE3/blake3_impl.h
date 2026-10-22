#ifndef BLAKE3_IMPL_H
#define BLAKE3_IMPL_H

#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "llvm-c/blake3.h"
// For \p LLVM_LIBRARY_VISIBILITY
#अन्तर्भाव "llvm/Support/Compiler.h"

#अन्तर्भाव "llvm_blake3_prefix.h"

#define BLAKE3_PRIVATE

// internal flags
गणना blake3_flags {
  CHUNK_START         = 1 << 0,
  CHUNK_END           = 1 << 1,
  PARENT              = 1 << 2,
  मूळ                = 1 << 3,
  KEYED_HASH          = 1 << 4,
  DERIVE_किल्ली_CONTEXT  = 1 << 5,
  DERIVE_किल्ली_MATERIAL = 1 << 6,
};

// This C implementation tries to support recent versions of GCC, Clang, and
// MSVC.
#जर defined(_MSC_VER)
#define INLINE स्थिर __forceinline
#अन्यथा
#define INLINE स्थिर ओळीत __attribute__((always_inline))
#endif

#जर (defined(__x86_64__) || defined(_M_X64)) && !defined(_M_ARM64EC)
#define IS_X86
#define IS_X86_64
#endif

#जर defined(__i386__) || defined(_M_IX86)
#define IS_X86
#define IS_X86_32
#endif

#जर defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define IS_AARCH64
#endif

#जर defined(IS_X86)
#जर defined(_MSC_VER)
#अन्तर्भाव <intrin.h>
#endif
#अन्तर्भाव <immintrin.h>
#endif

#जर !defined(BLAKE3_USE_NEON) 
  // If BLAKE3_USE_NEON not manually set, autodetect based on AArch64ness
  #जर defined(IS_AARCH64)
    #जर defined(__ARM_BIG_ENDIAN)
      #define BLAKE3_USE_NEON 0
    #अन्यथा
      #define BLAKE3_USE_NEON 1
    #endif
  #अन्यथा
    #define BLAKE3_USE_NEON 0
  #endif
#endif

#जर defined(IS_X86)
#define MAX_SIMD_DEGREE 16
#किंवाजर BLAKE3_USE_NEON == 1
#define MAX_SIMD_DEGREE 4
#अन्यथा
#define MAX_SIMD_DEGREE 1
#endif

// There are some places where we want a static size that's equal to the
// MAX_SIMD_DEGREE, but also at least 2.
#define MAX_SIMD_DEGREE_OR_2 (MAX_SIMD_DEGREE > 2 ? MAX_SIMD_DEGREE : 2)

स्थिर स्थिरांक uint32_t IV[8] = {0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL,
                               0xA54FF53AUL, 0x510E527FUL, 0x9B05688CUL,
                               0x1F83D9ABUL, 0x5BE0CD19UL};

स्थिर स्थिरांक uint8_t संदेश_SCHEDULE[7][16] = {
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
स्थिर अचिन्हित पूर्णांक highest_one(uint64_t x) {
#जर defined(__GNUC__) || defined(__clang__)
  परत 63 ^ (अचिन्हित पूर्णांक)__builtin_clzll(x);
#किंवाजर defined(_MSC_VER) && defined(IS_X86_64)
  अचिन्हित दीर्घ index;
  _BitScanReverse64(&index, x);
  परत index;
#किंवाजर defined(_MSC_VER) && defined(IS_X86_32)
  जर(x >> 32) {
    अचिन्हित दीर्घ index;
    _BitScanReverse(&index, (अचिन्हित दीर्घ)(x >> 32));
    परत 32 + index;
  } अन्यथा {
    अचिन्हित दीर्घ index;
    _BitScanReverse(&index, (अचिन्हित दीर्घ)x);
    परत index;
  }
#अन्यथा
  अचिन्हित पूर्णांक c = 0;
  जर(x & 0xffffffff00000000ULL) { x >>= 32; c += 32; }
  जर(x & 0x00000000ffff0000ULL) { x >>= 16; c += 16; }
  जर(x & 0x000000000000ff00ULL) { x >>=  8; c +=  8; }
  जर(x & 0x00000000000000f0ULL) { x >>=  4; c +=  4; }
  जर(x & 0x000000000000000cULL) { x >>=  2; c +=  2; }
  जर(x & 0x0000000000000002ULL) {           c +=  1; }
  परत c;
#endif
}

// Count the number of 1 bits.
INLINE अचिन्हित पूर्णांक popcnt(uint64_t x) {
#जर defined(__GNUC__) || defined(__clang__)
  परत (अचिन्हित पूर्णांक)__builtin_popcountll(x);
#अन्यथा
  अचिन्हित पूर्णांक मोजणी = 0;
  पर्यंत (x != 0) {
    मोजणी += 1;
    x &= x - 1;
  }
  परत मोजणी;
#endif
}

// Largest power of two less than or equal to x. As a special case, returns 1
// when x is 0. 
INLINE uint64_t round_down_to_power_of_2(uint64_t x) {
  परत 1ULL << highest_one(x | 1);
}

INLINE uint32_t counter_खालचा(uint64_t counter) { परत (uint32_t)counter; }

INLINE uint32_t counter_वरचा(uint64_t counter) {
  परत (uint32_t)(counter >> 32);
}

INLINE uint32_t load32(स्थिरांक शून्य *src) {
  स्थिरांक uint8_t *p = (स्थिरांक uint8_t *)src;
  परत ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

INLINE शून्य load_किल्ली_शब्द(स्थिरांक uint8_t किल्ली_2[BLAKE3_किल्ली_LEN],
                           uint32_t किल्ली_शब्द[8]) {
  किल्ली_शब्द[0] = load32(&किल्ली_2[0 * 4]);
  किल्ली_शब्द[1] = load32(&किल्ली_2[1 * 4]);
  किल्ली_शब्द[2] = load32(&किल्ली_2[2 * 4]);
  किल्ली_शब्द[3] = load32(&किल्ली_2[3 * 4]);
  किल्ली_शब्द[4] = load32(&किल्ली_2[4 * 4]);
  किल्ली_शब्द[5] = load32(&किल्ली_2[5 * 4]);
  किल्ली_शब्द[6] = load32(&किल्ली_2[6 * 4]);
  किल्ली_शब्द[7] = load32(&किल्ली_2[7 * 4]);
}

INLINE शून्य load_block_शब्द(स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                             uint32_t block_शब्द[16]) {
  साठी (माप_प्रकार i = 0; i < 16; i++) {
      block_शब्द[i] = load32(&block[i * 4]);
  }
}

INLINE शून्य store32(शून्य *dst, uint32_t w) {
  uint8_t *p = (uint8_t *)dst;
  p[0] = (uint8_t)(w >> 0);
  p[1] = (uint8_t)(w >> 8);
  p[2] = (uint8_t)(w >> 16);
  p[3] = (uint8_t)(w >> 24);
}

INLINE शून्य store_cv_शब्द(uint8_t bytes_out[32], uint32_t cv_शब्द[8]) {
  store32(&bytes_out[0 * 4], cv_शब्द[0]);
  store32(&bytes_out[1 * 4], cv_शब्द[1]);
  store32(&bytes_out[2 * 4], cv_शब्द[2]);
  store32(&bytes_out[3 * 4], cv_शब्द[3]);
  store32(&bytes_out[4 * 4], cv_शब्द[4]);
  store32(&bytes_out[5 * 4], cv_शब्द[5]);
  store32(&bytes_out[6 * 4], cv_शब्द[6]);
  store32(&bytes_out[7 * 4], cv_शब्द[7]);
}

LLVM_LIBRARY_VISIBILITY
शून्य blake3_compress_in_place(uint32_t cv[8],
                              स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags);

LLVM_LIBRARY_VISIBILITY
शून्य blake3_compress_xof(स्थिरांक uint32_t cv[8],
                         स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
शून्य blake3_xof_many(स्थिरांक uint32_t cv[8],
                     स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], माप_प्रकार outblocks);

LLVM_LIBRARY_VISIBILITY
शून्य blake3_hash_many(स्थिरांक uint8_t *स्थिरांक *inputs, माप_प्रकार अंक_inputs,
                      माप_प्रकार blocks, स्थिरांक uint32_t किल्ली_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out);

LLVM_LIBRARY_VISIBILITY
माप_प्रकार blake3_simd_degree(शून्य);

BLAKE3_PRIVATE माप_प्रकार blake3_compress_subtree_wide(स्थिरांक uint8_t *आदान_2, माप_प्रकार आदान_len,
                                                   स्थिरांक uint32_t किल्ली_2[8],
                                                   uint64_t chunk_counter, uint8_t flags,
                                                   uint8_t *out, bool use_tbb);

#जर defined(BLAKE3_USE_TBB)
BLAKE3_PRIVATE शून्य blake3_compress_subtree_wide_join_tbb(
    // shared params
    स्थिरांक uint32_t किल्ली_2[8], uint8_t flags, bool use_tbb,
    // left-hand side params
    स्थिरांक uint8_t *l_आदान, माप_प्रकार l_आदान_len, uint64_t l_chunk_counter,
    uint8_t *l_cvs, माप_प्रकार *l_n,
    // right-hand side params
    स्थिरांक uint8_t *r_आदान, माप_प्रकार r_आदान_len, uint64_t r_chunk_counter,
    uint8_t *r_cvs, माप_प्रकार *r_n) NOEXCEPT;
#endif

// Declarations for implementation-specific functions.
LLVM_LIBRARY_VISIBILITY
शून्य blake3_compress_in_place_portable(uint32_t cv[8],
                                       स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                                       uint8_t block_len, uint64_t counter,
                                       uint8_t flags);

LLVM_LIBRARY_VISIBILITY
शून्य blake3_compress_xof_portable(स्थिरांक uint32_t cv[8],
                                  स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                                  uint8_t block_len, uint64_t counter,
                                  uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
शून्य blake3_hash_many_portable(स्थिरांक uint8_t *स्थिरांक *inputs, माप_प्रकार अंक_inputs,
                               माप_प्रकार blocks, स्थिरांक uint32_t किल्ली_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_end, uint8_t *out);

#जर defined(IS_X86)
#जर !defined(BLAKE3_NO_SSE2)
LLVM_LIBRARY_VISIBILITY
शून्य blake3_compress_in_place_sse2(uint32_t cv[8],
                                   स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                                   uint8_t block_len, uint64_t counter,
                                   uint8_t flags);
LLVM_LIBRARY_VISIBILITY
शून्य blake3_compress_xof_sse2(स्थिरांक uint32_t cv[8],
                              स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
शून्य blake3_hash_many_sse2(स्थिरांक uint8_t *स्थिरांक *inputs, माप_प्रकार अंक_inputs,
                           माप_प्रकार blocks, स्थिरांक uint32_t किल्ली_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#जर !defined(BLAKE3_NO_SSE41)
LLVM_LIBRARY_VISIBILITY
शून्य blake3_compress_in_place_sse41(uint32_t cv[8],
                                    स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                                    uint8_t block_len, uint64_t counter,
                                    uint8_t flags);
LLVM_LIBRARY_VISIBILITY
शून्य blake3_compress_xof_sse41(स्थिरांक uint32_t cv[8],
                               स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                               uint8_t block_len, uint64_t counter,
                               uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
शून्य blake3_hash_many_sse41(स्थिरांक uint8_t *स्थिरांक *inputs, माप_प्रकार अंक_inputs,
                            माप_प्रकार blocks, स्थिरांक uint32_t किल्ली_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_start,
                            uint8_t flags_end, uint8_t *out);
#endif
#जर !defined(BLAKE3_NO_AVX2)
LLVM_LIBRARY_VISIBILITY
शून्य blake3_hash_many_avx2(स्थिरांक uint8_t *स्थिरांक *inputs, माप_प्रकार अंक_inputs,
                           माप_प्रकार blocks, स्थिरांक uint32_t किल्ली_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#जर !defined(BLAKE3_NO_AVX512)
LLVM_LIBRARY_VISIBILITY
शून्य blake3_compress_in_place_avx512(uint32_t cv[8],
                                     स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                                     uint8_t block_len, uint64_t counter,
                                     uint8_t flags);

LLVM_LIBRARY_VISIBILITY
शून्य blake3_compress_xof_avx512(स्थिरांक uint32_t cv[8],
                                स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                                uint8_t block_len, uint64_t counter,
                                uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
शून्य blake3_hash_many_avx512(स्थिरांक uint8_t *स्थिरांक *inputs, माप_प्रकार अंक_inputs,
                             माप_प्रकार blocks, स्थिरांक uint32_t किल्ली_2[8],
                             uint64_t counter, bool increment_counter,
                             uint8_t flags, uint8_t flags_start,
                             uint8_t flags_end, uint8_t *out);

#जर !defined(_WIN32) && !defined(__CYGWIN__)
LLVM_LIBRARY_VISIBILITY
शून्य blake3_xof_many_avx512(स्थिरांक uint32_t cv[8],
                            स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                            uint8_t block_len, uint64_t counter, uint8_t flags,
                            uint8_t* out, माप_प्रकार outblocks);
#endif
#endif
#endif

#जर BLAKE3_USE_NEON == 1
LLVM_LIBRARY_VISIBILITY
शून्य blake3_hash_many_neon(स्थिरांक uint8_t *स्थिरांक *inputs, माप_प्रकार अंक_inputs,
                           माप_प्रकार blocks, स्थिरांक uint32_t किल्ली_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif


#endif /* BLAKE3_IMPL_H */
