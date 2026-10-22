#ifndef BLAKE3_IMPL_H
#define BLAKE3_IMPL_H

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "llvm-c/blake3.h"
// For \p LLVM_LIBRARY_VISIBILITY
#അന്തര്ഭാവ "llvm/Support/Compiler.h"

#അന്തര്ഭാവ "llvm_blake3_prefix.h"

#define BLAKE3_PRIVATE

// internal flags
എണ്ണൽ blake3_flags {
  CHUNK_START         = 1 << 0,
  CHUNK_END           = 1 << 1,
  PARENT              = 1 << 2,
  വേര്                = 1 << 3,
  KEYED_HASH          = 1 << 4,
  DERIVE_താക്കോൽ_CONTEXT  = 1 << 5,
  DERIVE_താക്കോൽ_MATERIAL = 1 << 6,
};

// This C implementation tries to support recent versions of GCC, Clang, and
// MSVC.
#എങ്കിൽ defined(_MSC_VER)
#define INLINE സ്ഥിതം __forceinline
#അല്ലെങ്കിൽ
#define INLINE സ്ഥിതം വരിയിൽ __attribute__((always_inline))
#endif

#എങ്കിൽ (defined(__x86_64__) || defined(_M_X64)) && !defined(_M_ARM64EC)
#define IS_X86
#define IS_X86_64
#endif

#എങ്കിൽ defined(__i386__) || defined(_M_IX86)
#define IS_X86
#define IS_X86_32
#endif

#എങ്കിൽ defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define IS_AARCH64
#endif

#എങ്കിൽ defined(IS_X86)
#എങ്കിൽ defined(_MSC_VER)
#അന്തര്ഭാവ <intrin.h>
#endif
#അന്തര്ഭാവ <immintrin.h>
#endif

#എങ്കിൽ !defined(BLAKE3_USE_NEON) 
  // If BLAKE3_USE_NEON not manually set, autodetect based on AArch64ness
  #എങ്കിൽ defined(IS_AARCH64)
    #എങ്കിൽ defined(__ARM_BIG_ENDIAN)
      #define BLAKE3_USE_NEON 0
    #അല്ലെങ്കിൽ
      #define BLAKE3_USE_NEON 1
    #endif
  #അല്ലെങ്കിൽ
    #define BLAKE3_USE_NEON 0
  #endif
#endif

#എങ്കിൽ defined(IS_X86)
#define MAX_SIMD_DEGREE 16
#അല്ലെങ്കിലെങ്കിൽ BLAKE3_USE_NEON == 1
#define MAX_SIMD_DEGREE 4
#അല്ലെങ്കിൽ
#define MAX_SIMD_DEGREE 1
#endif

// There are some places where we want a static size that's equal to the
// MAX_SIMD_DEGREE, but also at least 2.
#define MAX_SIMD_DEGREE_OR_2 (MAX_SIMD_DEGREE > 2 ? MAX_SIMD_DEGREE : 2)

സ്ഥിതം സ്ഥിരം uint32_t IV[8] = {0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL,
                               0xA54FF53AUL, 0x510E527FUL, 0x9B05688CUL,
                               0x1F83D9ABUL, 0x5BE0CD19UL};

സ്ഥിതം സ്ഥിരം uint8_t സന്ദേശം_SCHEDULE[7][16] = {
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
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ highest_one(uint64_t x) {
#എങ്കിൽ defined(__GNUC__) || defined(__clang__)
  തിരികെ 63 ^ (unsigned പൂർണ്ണസംഖ്യ)__builtin_clzll(x);
#അല്ലെങ്കിലെങ്കിൽ defined(_MSC_VER) && defined(IS_X86_64)
  unsigned long index;
  _BitScanReverse64(&index, x);
  തിരികെ index;
#അല്ലെങ്കിലെങ്കിൽ defined(_MSC_VER) && defined(IS_X86_32)
  എങ്കിൽ(x >> 32) {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)(x >> 32));
    തിരികെ 32 + index;
  } അല്ലെങ്കിൽ {
    unsigned long index;
    _BitScanReverse(&index, (unsigned long)x);
    തിരികെ index;
  }
#അല്ലെങ്കിൽ
  unsigned പൂർണ്ണസംഖ്യ c = 0;
  എങ്കിൽ(x & 0xffffffff00000000ULL) { x >>= 32; c += 32; }
  എങ്കിൽ(x & 0x00000000ffff0000ULL) { x >>= 16; c += 16; }
  എങ്കിൽ(x & 0x000000000000ff00ULL) { x >>=  8; c +=  8; }
  എങ്കിൽ(x & 0x00000000000000f0ULL) { x >>=  4; c +=  4; }
  എങ്കിൽ(x & 0x000000000000000cULL) { x >>=  2; c +=  2; }
  എങ്കിൽ(x & 0x0000000000000002ULL) {           c +=  1; }
  തിരികെ c;
#endif
}

// Count the number of 1 bits.
INLINE unsigned പൂർണ്ണസംഖ്യ popcnt(uint64_t x) {
#എങ്കിൽ defined(__GNUC__) || defined(__clang__)
  തിരികെ (unsigned പൂർണ്ണസംഖ്യ)__builtin_popcountll(x);
#അല്ലെങ്കിൽ
  unsigned പൂർണ്ണസംഖ്യ എണ്ണം = 0;
  ആകുമ്പോൾ (x != 0) {
    എണ്ണം += 1;
    x &= x - 1;
  }
  തിരികെ എണ്ണം;
#endif
}

// Largest power of two less than or equal to x. As a special case, returns 1
// when x is 0. 
INLINE uint64_t round_down_to_power_of_2(uint64_t x) {
  തിരികെ 1ULL << highest_one(x | 1);
}

INLINE uint32_t counter_താഴെയുള്ള(uint64_t counter) { തിരികെ (uint32_t)counter; }

INLINE uint32_t counter_മുകളിലെ(uint64_t counter) {
  തിരികെ (uint32_t)(counter >> 32);
}

INLINE uint32_t load32(സ്ഥിരം ശൂന്യം *src) {
  സ്ഥിരം uint8_t *p = (സ്ഥിരം uint8_t *)src;
  തിരികെ ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

INLINE ശൂന്യം load_താക്കോൽ_വാക്കുകൾ(സ്ഥിരം uint8_t താക്കോൽ_2[BLAKE3_താക്കോൽ_LEN],
                           uint32_t താക്കോൽ_വാക്കുകൾ[8]) {
  താക്കോൽ_വാക്കുകൾ[0] = load32(&താക്കോൽ_2[0 * 4]);
  താക്കോൽ_വാക്കുകൾ[1] = load32(&താക്കോൽ_2[1 * 4]);
  താക്കോൽ_വാക്കുകൾ[2] = load32(&താക്കോൽ_2[2 * 4]);
  താക്കോൽ_വാക്കുകൾ[3] = load32(&താക്കോൽ_2[3 * 4]);
  താക്കോൽ_വാക്കുകൾ[4] = load32(&താക്കോൽ_2[4 * 4]);
  താക്കോൽ_വാക്കുകൾ[5] = load32(&താക്കോൽ_2[5 * 4]);
  താക്കോൽ_വാക്കുകൾ[6] = load32(&താക്കോൽ_2[6 * 4]);
  താക്കോൽ_വാക്കുകൾ[7] = load32(&താക്കോൽ_2[7 * 4]);
}

INLINE ശൂന്യം load_block_വാക്കുകൾ(സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                             uint32_t block_വാക്കുകൾ[16]) {
  വേണ്ടി (size_t i = 0; i < 16; i++) {
      block_വാക്കുകൾ[i] = load32(&block[i * 4]);
  }
}

INLINE ശൂന്യം store32(ശൂന്യം *dst, uint32_t w) {
  uint8_t *p = (uint8_t *)dst;
  p[0] = (uint8_t)(w >> 0);
  p[1] = (uint8_t)(w >> 8);
  p[2] = (uint8_t)(w >> 16);
  p[3] = (uint8_t)(w >> 24);
}

INLINE ശൂന്യം store_cv_വാക്കുകൾ(uint8_t bytes_out[32], uint32_t cv_വാക്കുകൾ[8]) {
  store32(&bytes_out[0 * 4], cv_വാക്കുകൾ[0]);
  store32(&bytes_out[1 * 4], cv_വാക്കുകൾ[1]);
  store32(&bytes_out[2 * 4], cv_വാക്കുകൾ[2]);
  store32(&bytes_out[3 * 4], cv_വാക്കുകൾ[3]);
  store32(&bytes_out[4 * 4], cv_വാക്കുകൾ[4]);
  store32(&bytes_out[5 * 4], cv_വാക്കുകൾ[5]);
  store32(&bytes_out[6 * 4], cv_വാക്കുകൾ[6]);
  store32(&bytes_out[7 * 4], cv_വാക്കുകൾ[7]);
}

LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_compress_in_place(uint32_t cv[8],
                              സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags);

LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_compress_xof(സ്ഥിരം uint32_t cv[8],
                         സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_xof_many(സ്ഥിരം uint32_t cv[8],
                     സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks);

LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_hash_many(സ്ഥിരം uint8_t *സ്ഥിരം *inputs, size_t അക്കം_inputs,
                      size_t blocks, സ്ഥിരം uint32_t താക്കോൽ_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out);

LLVM_LIBRARY_VISIBILITY
size_t blake3_simd_degree(ശൂന്യം);

BLAKE3_PRIVATE size_t blake3_compress_subtree_wide(സ്ഥിരം uint8_t *നിവേശം_2, size_t നിവേശം_len,
                                                   സ്ഥിരം uint32_t താക്കോൽ_2[8],
                                                   uint64_t chunk_counter, uint8_t flags,
                                                   uint8_t *out, bool use_tbb);

#എങ്കിൽ defined(BLAKE3_USE_TBB)
BLAKE3_PRIVATE ശൂന്യം blake3_compress_subtree_wide_join_tbb(
    // shared params
    സ്ഥിരം uint32_t താക്കോൽ_2[8], uint8_t flags, bool use_tbb,
    // left-hand side params
    സ്ഥിരം uint8_t *l_നിവേശം, size_t l_നിവേശം_len, uint64_t l_chunk_counter,
    uint8_t *l_cvs, size_t *l_n,
    // right-hand side params
    സ്ഥിരം uint8_t *r_നിവേശം, size_t r_നിവേശം_len, uint64_t r_chunk_counter,
    uint8_t *r_cvs, size_t *r_n) NOEXCEPT;
#endif

// Declarations for implementation-specific functions.
LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_compress_in_place_portable(uint32_t cv[8],
                                       സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                                       uint8_t block_len, uint64_t counter,
                                       uint8_t flags);

LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_compress_xof_portable(സ്ഥിരം uint32_t cv[8],
                                  സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                                  uint8_t block_len, uint64_t counter,
                                  uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_hash_many_portable(സ്ഥിരം uint8_t *സ്ഥിരം *inputs, size_t അക്കം_inputs,
                               size_t blocks, സ്ഥിരം uint32_t താക്കോൽ_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_start,
                               uint8_t flags_end, uint8_t *out);

#എങ്കിൽ defined(IS_X86)
#എങ്കിൽ !defined(BLAKE3_NO_SSE2)
LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_compress_in_place_sse2(uint32_t cv[8],
                                   സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                                   uint8_t block_len, uint64_t counter,
                                   uint8_t flags);
LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_compress_xof_sse2(സ്ഥിരം uint32_t cv[8],
                              സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_hash_many_sse2(സ്ഥിരം uint8_t *സ്ഥിരം *inputs, size_t അക്കം_inputs,
                           size_t blocks, സ്ഥിരം uint32_t താക്കോൽ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#എങ്കിൽ !defined(BLAKE3_NO_SSE41)
LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_compress_in_place_sse41(uint32_t cv[8],
                                    സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                                    uint8_t block_len, uint64_t counter,
                                    uint8_t flags);
LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_compress_xof_sse41(സ്ഥിരം uint32_t cv[8],
                               സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                               uint8_t block_len, uint64_t counter,
                               uint8_t flags, uint8_t out[64]);
LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_hash_many_sse41(സ്ഥിരം uint8_t *സ്ഥിരം *inputs, size_t അക്കം_inputs,
                            size_t blocks, സ്ഥിരം uint32_t താക്കോൽ_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_start,
                            uint8_t flags_end, uint8_t *out);
#endif
#എങ്കിൽ !defined(BLAKE3_NO_AVX2)
LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_hash_many_avx2(സ്ഥിരം uint8_t *സ്ഥിരം *inputs, size_t അക്കം_inputs,
                           size_t blocks, സ്ഥിരം uint32_t താക്കോൽ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif
#എങ്കിൽ !defined(BLAKE3_NO_AVX512)
LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_compress_in_place_avx512(uint32_t cv[8],
                                     സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                                     uint8_t block_len, uint64_t counter,
                                     uint8_t flags);

LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_compress_xof_avx512(സ്ഥിരം uint32_t cv[8],
                                സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                                uint8_t block_len, uint64_t counter,
                                uint8_t flags, uint8_t out[64]);

LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_hash_many_avx512(സ്ഥിരം uint8_t *സ്ഥിരം *inputs, size_t അക്കം_inputs,
                             size_t blocks, സ്ഥിരം uint32_t താക്കോൽ_2[8],
                             uint64_t counter, bool increment_counter,
                             uint8_t flags, uint8_t flags_start,
                             uint8_t flags_end, uint8_t *out);

#എങ്കിൽ !defined(_WIN32) && !defined(__CYGWIN__)
LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_xof_many_avx512(സ്ഥിരം uint32_t cv[8],
                            സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                            uint8_t block_len, uint64_t counter, uint8_t flags,
                            uint8_t* out, size_t outblocks);
#endif
#endif
#endif

#എങ്കിൽ BLAKE3_USE_NEON == 1
LLVM_LIBRARY_VISIBILITY
ശൂന്യം blake3_hash_many_neon(സ്ഥിരം uint8_t *സ്ഥിരം *inputs, size_t അക്കം_inputs,
                           size_t blocks, സ്ഥിരം uint32_t താക്കോൽ_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_start,
                           uint8_t flags_end, uint8_t *out);
#endif


#endif /* BLAKE3_IMPL_H */
