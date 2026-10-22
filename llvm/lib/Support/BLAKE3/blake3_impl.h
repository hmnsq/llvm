#جے_بنا_تعریف BLAKE3_IMPL_H
#تعریف_کرو BLAKE3_IMPL_H

#ضمن <assert.h>
#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdint.h>
#ضمن <string.h>

#ضمن "llvm-c/blake3.h"
// For \p LLVM_LIBRARY_VISIBILITY
#ضمن "llvm/Support/Compiler.h"

#ضمن "llvm_blake3_prefix.h"

#تعریف_کرو BLAKE3_نجی

// internal flags
تعداد blake3_flags {
  CHUNK_شروع         = 1 << 0,
  CHUNK_سرا           = 1 << 1,
  ماپا              = 1 << 2,
  جڑ                = 1 << 3,
  KEYED_HASH          = 1 << 4,
  DERIVE_کلید_CONTEXT  = 1 << 5,
  DERIVE_کلید_MATERIAL = 1 << 6,
};

// This C implementation tries to support recent versions of GCC, Clang, and
// MSVC.
#إذا defined(_MSC_VER)
#تعریف_کرو INLINE ساكن __forceinline
#وإلا
#تعریف_کرو INLINE ساكن مضمن __attribute__((always_inline))
#جے_انت

#إذا (defined(__x86_64__) || defined(_M_X64)) && !defined(_M_ARM64EC)
#تعریف_کرو IS_X86
#تعریف_کرو IS_X86_64
#جے_انت

#إذا defined(__i386__) || defined(_M_IX86)
#تعریف_کرو IS_X86
#تعریف_کرو IS_X86_32
#جے_انت

#إذا defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#تعریف_کرو IS_AARCH64
#جے_انت

#إذا defined(IS_X86)
#إذا defined(_MSC_VER)
#ضمن <intrin.h>
#جے_انت
#ضمن <immintrin.h>
#جے_انت

#إذا !defined(BLAKE3_USE_NEON) 
  // If BLAKE3_USE_NEON not manually set, autodetect based on AArch64ness
  #إذا defined(IS_AARCH64)
    #إذا defined(__ARM_BIG_ENDIAN)
      #تعریف_کرو BLAKE3_USE_NEON 0
    #وإلا
      #تعریف_کرو BLAKE3_USE_NEON 1
    #جے_انت
  #وإلا
    #تعریف_کرو BLAKE3_USE_NEON 0
  #جے_انت
#جے_انت

#إذا defined(IS_X86)
#تعریف_کرو MAX_SIMD_DEGREE 16
#وإلاإذا BLAKE3_USE_NEON == 1
#تعریف_کرو MAX_SIMD_DEGREE 4
#وإلا
#تعریف_کرو MAX_SIMD_DEGREE 1
#جے_انت

// There are some places where we want a static size that's equal to the
// MAX_SIMD_DEGREE, but also at least 2.
#تعریف_کرو MAX_SIMD_DEGREE_OR_2 (MAX_SIMD_DEGREE > 2 ? MAX_SIMD_DEGREE : 2)

ساكن ثابت uint32_t IV[8] = {0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL,
                               0xA54FF53AUL, 0x510E527FUL, 0x9B05688CUL,
                               0x1F83D9ABUL, 0x5BE0CD19UL};

ساكن ثابت uint8_t پیغام_ویلا_سارنی[7][16] = {
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
ساكن بنانشانی صحيح highest_one(uint64_t x) {
#إذا defined(__GNUC__) || defined(__clang__)
  أرجع 63 ^ (بنانشانی صحيح)__builtin_clzll(x);
#وإلاإذا defined(_MSC_VER) && defined(IS_X86_64)
  بنانشانی لما اشاریہ;
  ثنائی_ہندسہ_جانچ_Reverse64(&اشاریہ, x);
  أرجع اشاریہ;
#وإلاإذا defined(_MSC_VER) && defined(IS_X86_32)
  إذا(x >> 32) {
    بنانشانی لما اشاریہ;
    ثنائی_ہندسہ_جانچ_الٹ(&اشاریہ, (بنانشانی لما)(x >> 32));
    أرجع 32 + اشاریہ;
  } وإلا {
    بنانشانی لما اشاریہ;
    ثنائی_ہندسہ_جانچ_الٹ(&اشاریہ, (بنانشانی لما)x);
    أرجع اشاریہ;
  }
#وإلا
  بنانشانی صحيح c = 0;
  إذا(x & 0xffffffff00000000ULL) { x >>= 32; c += 32; }
  إذا(x & 0x00000000ffff0000ULL) { x >>= 16; c += 16; }
  إذا(x & 0x000000000000ff00ULL) { x >>=  8; c +=  8; }
  إذا(x & 0x00000000000000f0ULL) { x >>=  4; c +=  4; }
  إذا(x & 0x000000000000000cULL) { x >>=  2; c +=  2; }
  إذا(x & 0x0000000000000002ULL) {           c +=  1; }
  أرجع c;
#جے_انت
}

// Count the number of 1 bits.
INLINE بنانشانی صحيح popcnt(uint64_t x) {
#إذا defined(__GNUC__) || defined(__clang__)
  أرجع (بنانشانی صحيح)__builtin_popcountll(x);
#وإلا
  بنانشانی صحيح گنتی = 0;
  طالما (x != 0) {
    گنتی += 1;
    x &= x - 1;
  }
  أرجع گنتی;
#جے_انت
}

// Largest power of two less than or equal to x. As a special case, returns 1
// when x is 0. 
INLINE uint64_t گیڑا_تھلے_منزل_power_دا_2(uint64_t x) {
  أرجع 1ULL << highest_one(x | 1);
}

INLINE uint32_t counter_کم(uint64_t counter) { أرجع (uint32_t)counter; }

INLINE uint32_t counter_بلند(uint64_t counter) {
  أرجع (uint32_t)(counter >> 32);
}

INLINE uint32_t load32(ثابت فراغ *src) {
  ثابت uint8_t *p = (ثابت uint8_t *)src;
  أرجع ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

INLINE فراغ لدو_کلید_الفاظ(ثابت uint8_t کلید_2[BLAKE3_کلید_LEN],
                           uint32_t کلید_الفاظ[8]) {
  کلید_الفاظ[0] = load32(&کلید_2[0 * 4]);
  کلید_الفاظ[1] = load32(&کلید_2[1 * 4]);
  کلید_الفاظ[2] = load32(&کلید_2[2 * 4]);
  کلید_الفاظ[3] = load32(&کلید_2[3 * 4]);
  کلید_الفاظ[4] = load32(&کلید_2[4 * 4]);
  کلید_الفاظ[5] = load32(&کلید_2[5 * 4]);
  کلید_الفاظ[6] = load32(&کلید_2[6 * 4]);
  کلید_الفاظ[7] = load32(&کلید_2[7 * 4]);
}

INLINE فراغ لدو_بلاک_الفاظ(ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                             uint32_t بلاک_الفاظ[16]) {
  لكل (size_t i = 0; i < 16; i++) {
      بلاک_الفاظ[i] = load32(&بلاک[i * 4]);
  }
}

INLINE فراغ store32(فراغ *dst, uint32_t w) {
  uint8_t *p = (uint8_t *)dst;
  p[0] = (uint8_t)(w >> 0);
  p[1] = (uint8_t)(w >> 8);
  p[2] = (uint8_t)(w >> 16);
  p[3] = (uint8_t)(w >> 24);
}

INLINE فراغ store_cv_الفاظ(uint8_t بائٹاں_باہرلا[32], uint32_t cv_الفاظ[8]) {
  store32(&بائٹاں_باہرلا[0 * 4], cv_الفاظ[0]);
  store32(&بائٹاں_باہرلا[1 * 4], cv_الفاظ[1]);
  store32(&بائٹاں_باہرلا[2 * 4], cv_الفاظ[2]);
  store32(&بائٹاں_باہرلا[3 * 4], cv_الفاظ[3]);
  store32(&بائٹاں_باہرلا[4 * 4], cv_الفاظ[4]);
  store32(&بائٹاں_باہرلا[5 * 4], cv_الفاظ[5]);
  store32(&بائٹاں_باہرلا[6 * 4], cv_الفاظ[6]);
  store32(&بائٹاں_باہرلا[7 * 4], cv_الفاظ[7]);
}

LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_وچ_رکھو(uint32_t cv[8],
                              ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                              uint8_t بلاک_len, uint64_t counter,
                              uint8_t flags);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_xof(ثابت uint32_t cv[8],
                         ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                         uint8_t بلاک_len, uint64_t counter, uint8_t flags,
                         uint8_t باہرلا_2[64]);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_xof_many(ثابت uint32_t cv[8],
                     ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                     uint8_t بلاک_len, uint64_t counter, uint8_t flags,
                     uint8_t باہرلا_2[64], size_t outblocks);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                      size_t بلاکاں, ثابت uint32_t کلید_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_شروع, uint8_t flags_سرا, uint8_t *باہرلا_2);

LLVM_LIBRARY_VISIBILITY
size_t blake3_simd_degree(فراغ);

BLAKE3_نجی size_t blake3_compress_subtree_wide(ثابت uint8_t *داخلہ_2, size_t داخلہ_len,
                                                   ثابت uint32_t کلید_2[8],
                                                   uint64_t chunk_counter, uint8_t flags,
                                                   uint8_t *باہرلا_2, bool use_tbb);

#إذا defined(BLAKE3_USE_TBB)
BLAKE3_نجی فراغ blake3_compress_subtree_wide_join_tbb(
    // shared params
    ثابت uint32_t کلید_2[8], uint8_t flags, bool use_tbb,
    // left-hand side params
    ثابت uint8_t *l_داخلہ, size_t l_داخلہ_len, uint64_t l_chunk_counter,
    uint8_t *l_cvs, size_t *l_n,
    // right-hand side params
    ثابت uint8_t *r_داخلہ, size_t r_داخلہ_len, uint64_t r_chunk_counter,
    uint8_t *r_cvs, size_t *r_n) NOEXCEPT;
#جے_انت

// Declarations for implementation-specific functions.
LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_وچ_رکھو_portable(uint32_t cv[8],
                                       ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                       uint8_t بلاک_len, uint64_t counter,
                                       uint8_t flags);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_xof_portable(ثابت uint32_t cv[8],
                                  ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                  uint8_t بلاک_len, uint64_t counter,
                                  uint8_t flags, uint8_t باہرلا_2[64]);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many_portable(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                               size_t بلاکاں, ثابت uint32_t کلید_2[8],
                               uint64_t counter, bool increment_counter,
                               uint8_t flags, uint8_t flags_شروع,
                               uint8_t flags_سرا, uint8_t *باہرلا_2);

#إذا defined(IS_X86)
#إذا !defined(BLAKE3_NO_SSE2)
LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_وچ_رکھو_sse2(uint32_t cv[8],
                                   ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                   uint8_t بلاک_len, uint64_t counter,
                                   uint8_t flags);
LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_xof_sse2(ثابت uint32_t cv[8],
                              ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                              uint8_t بلاک_len, uint64_t counter,
                              uint8_t flags, uint8_t باہرلا_2[64]);
LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many_sse2(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                           size_t بلاکاں, ثابت uint32_t کلید_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_شروع,
                           uint8_t flags_سرا, uint8_t *باہرلا_2);
#جے_انت
#إذا !defined(BLAKE3_NO_SSE41)
LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_وچ_رکھو_sse41(uint32_t cv[8],
                                    ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                    uint8_t بلاک_len, uint64_t counter,
                                    uint8_t flags);
LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_xof_sse41(ثابت uint32_t cv[8],
                               ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                               uint8_t بلاک_len, uint64_t counter,
                               uint8_t flags, uint8_t باہرلا_2[64]);
LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many_sse41(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                            size_t بلاکاں, ثابت uint32_t کلید_2[8],
                            uint64_t counter, bool increment_counter,
                            uint8_t flags, uint8_t flags_شروع,
                            uint8_t flags_سرا, uint8_t *باہرلا_2);
#جے_انت
#إذا !defined(BLAKE3_NO_AVX2)
LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many_avx2(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                           size_t بلاکاں, ثابت uint32_t کلید_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_شروع,
                           uint8_t flags_سرا, uint8_t *باہرلا_2);
#جے_انت
#إذا !defined(BLAKE3_NO_AVX512)
LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_وچ_رکھو_avx512(uint32_t cv[8],
                                     ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                     uint8_t بلاک_len, uint64_t counter,
                                     uint8_t flags);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_compress_xof_avx512(ثابت uint32_t cv[8],
                                ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                                uint8_t بلاک_len, uint64_t counter,
                                uint8_t flags, uint8_t باہرلا_2[64]);

LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many_avx512(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                             size_t بلاکاں, ثابت uint32_t کلید_2[8],
                             uint64_t counter, bool increment_counter,
                             uint8_t flags, uint8_t flags_شروع,
                             uint8_t flags_سرا, uint8_t *باہرلا_2);

#إذا !defined(_WIN32) && !defined(__CYGWIN__)
LLVM_LIBRARY_VISIBILITY
فراغ blake3_xof_many_avx512(ثابت uint32_t cv[8],
                            ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                            uint8_t بلاک_len, uint64_t counter, uint8_t flags,
                            uint8_t* باہرلا_2, size_t outblocks);
#جے_انت
#جے_انت
#جے_انت

#إذا BLAKE3_USE_NEON == 1
LLVM_LIBRARY_VISIBILITY
فراغ blake3_hash_many_neon(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                           size_t بلاکاں, ثابت uint32_t کلید_2[8],
                           uint64_t counter, bool increment_counter,
                           uint8_t flags, uint8_t flags_شروع,
                           uint8_t flags_سرا, uint8_t *باہرلا_2);
#جے_انت


#جے_انت /* BLAKE3_IMPL_H */
