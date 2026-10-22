#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>

#ಅನ್ತರ್ಭಾವ "blake3_impl.h"

#ಆದರೆ defined(_MSC_VER)
#ಅನ್ತರ್ಭಾವ <Windows.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(IS_X86)
#ಆದರೆ defined(_MSC_VER)
#ಅನ್ತರ್ಭಾವ <intrin.h>
#ಅಥವಾಆದರೆ defined(__GNUC__)
#ಅನ್ತರ್ಭಾವ <immintrin.h>
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು IS_X86 /* Unimplemented! */
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(BLAKE3_ATOMICS)
#ಆದರೆ defined(__has_include)
#ಆದರೆ __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#ವ್ಯಾಖ್ಯಾನಿಸು BLAKE3_ATOMICS 1
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು BLAKE3_ATOMICS 0
#ಆದರೆ_ಅಂತ್ಯ /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು BLAKE3_ATOMICS 0
#ಆದರೆ_ಅಂತ್ಯ /* defined(__has_include) */
#ಆದರೆ_ಅಂತ್ಯ /* BLAKE3_ATOMICS */

#ಆದರೆ BLAKE3_ATOMICS
#ವ್ಯಾಖ್ಯಾನಿಸು ATOMIC_INT _Atomic ಪೂರ್ಣಾಂಕ
#ವ್ಯಾಖ್ಯಾನಿಸು ATOMIC_ತುಂಬು(x) x
#ವ್ಯಾಖ್ಯಾನಿಸು ATOMIC_STORE(x, y) x = y
#ಅಥವಾಆದರೆ defined(_MSC_VER)
#ವ್ಯಾಖ್ಯಾನಿಸು ATOMIC_INT LONG
#ವ್ಯಾಖ್ಯಾನಿಸು ATOMIC_ತುಂಬು(x) InterlockedOr(&x, 0)
#ವ್ಯಾಖ್ಯಾನಿಸು ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು ATOMIC_INT ಪೂರ್ಣಾಂಕ
#ವ್ಯಾಖ್ಯಾನಿಸು ATOMIC_ತುಂಬು(x) x
#ವ್ಯಾಖ್ಯಾನಿಸು ATOMIC_STORE(x, y) x = y
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು MAYBE_ಬಳಸದ(x) (ಶೂನ್ಯ)((x))

#ಆದರೆ defined(IS_X86)
ಸ್ಥಿರ uint64_t xgetbv(ಶೂನ್ಯ) {
#ಆದರೆ defined(_MSC_VER)
  ಹಿಂತಿರುಗಿ _xgetbv(0);
#ಇಲ್ಲದಿದ್ದರೆ
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  ಹಿಂತಿರುಗಿ ((uint64_t)edx << 32) | eax;
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಶೂನ್ಯ cpuid(uint32_t ಹೊರಹೋಗು_2[4], uint32_t id) {
#ಆದರೆ defined(_MSC_VER)
  __cpuid((ಪೂರ್ಣಾಂಕ *)ಹೊರಹೋಗು_2, id);
#ಅಥವಾಆದರೆ defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(ಹೊರಹೋಗು_2[0]), "=r"(ಹೊರಹೋಗು_2[1]), "=c"(ಹೊರಹೋಗು_2[2]), "=d"(ಹೊರಹೋಗು_2[3])
                       : "a"(id));
#ಇಲ್ಲದಿದ್ದರೆ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(ಹೊರಹೋಗು_2[0]), "=b"(ಹೊರಹೋಗು_2[1]), "=c"(ಹೊರಹೋಗು_2[2]), "=d"(ಹೊರಹೋಗು_2[3])
                       : "a"(id));
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಶೂನ್ಯ cpuidex(uint32_t ಹೊರಹೋಗು_2[4], uint32_t id, uint32_t sid) {
#ಆದರೆ defined(_MSC_VER)
  __cpuidex((ಪೂರ್ಣಾಂಕ *)ಹೊರಹೋಗು_2, id, sid);
#ಅಥವಾಆದರೆ defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(ಹೊರಹೋಗು_2[0]), "=r"(ಹೊರಹೋಗು_2[1]), "=c"(ಹೊರಹೋಗು_2[2]), "=d"(ಹೊರಹೋಗು_2[3])
                       : "a"(id), "c"(sid));
#ಇಲ್ಲದಿದ್ದರೆ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(ಹೊರಹೋಗು_2[0]), "=b"(ಹೊರಹೋಗು_2[1]), "=c"(ಹೊರಹೋಗು_2[2]), "=d"(ಹೊರಹೋಗು_2[3])
                       : "a"(id), "c"(sid));
#ಆದರೆ_ಅಂತ್ಯ
}

#ಆದರೆ_ಅಂತ್ಯ

ಎಣಿಕೆ cpu_feature {
  SSE2 = 1 << 0,
  SSSE3 = 1 << 1,
  SSE41 = 1 << 2,
  AVX = 1 << 3,
  AVX2 = 1 << 4,
  AVX512F = 1 << 5,
  AVX512VL = 1 << 6,
  /* ... */
  UNDEFINED = 1 << 30
};

#ಆದರೆ !defined(BLAKE3_TESTING)
ಸ್ಥಿರ /* Allow the variable to be controlled manually for testing */
#ಆದರೆ_ಅಂತ್ಯ
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_ಬಳಸಿದ
#ಆದರೆ !defined(BLAKE3_TESTING)
ಸ್ಥಿರ
#ಆದರೆ_ಅಂತ್ಯ
    ಎಣಿಕೆ cpu_feature
    get_cpu_features(ಶೂನ್ಯ) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  ಎಣಿಕೆ cpu_feature features = ATOMIC_ತುಂಬು(g_cpu_features);
  ಆದರೆ (features != UNDEFINED) {
    ಹಿಂತಿರುಗಿ features;
  } ಇಲ್ಲದಿದ್ದರೆ {
#ಆದರೆ defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (ಶೂನ್ಯ)edx;
    features = 0;
    cpuid(regs, 0);
    ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ max_id = *eax;
    cpuid(regs, 1);
#ಆದರೆ defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ (*edx & (1UL << 26))
      features |= SSE2;
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ (*ecx & (1UL << 9))
      features |= SSSE3;
    ಆದರೆ (*ecx & (1UL << 19))
      features |= SSE41;

    ಆದರೆ (*ecx & (1UL << 27)) { // OSXSAVE
      ಸ್ಥಿರಾಂಕ uint64_t ಮುಖವಾಡ_2 = xgetbv();
      ಆದರೆ ((ಮುಖವಾಡ_2 & 6) == 6) { // SSE and AVX states
        ಆದರೆ (*ecx & (1UL << 28))
          features |= AVX;
        ಆದರೆ (max_id >= 7) {
          cpuidex(regs, 7, 0);
          ಆದರೆ (*ebx & (1UL << 5))
            features |= AVX2;
          ಆದರೆ ((ಮುಖವಾಡ_2 & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            ಆದರೆ (*ebx & (1UL << 31))
              features |= AVX512VL;
            ಆದರೆ (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    ಹಿಂತಿರುಗಿ features;
#ಇಲ್ಲದಿದ್ದರೆ
    /* How to detect NEON? */
    ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  }
}

ಶೂನ್ಯ blake3_compress_ಒಳ_ಇರಿಸು(uint32_t cv[8],
                              ಸ್ಥಿರಾಂಕ uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN],
                              uint8_t ಖಂಡ_len, uint64_t counter,
                              uint8_t flags) {
#ಆದರೆ defined(IS_X86)
  ಸ್ಥಿರಾಂಕ ಎಣಿಕೆ cpu_feature features = get_cpu_features();
  MAYBE_ಬಳಸದ(features);
#ಆದರೆ !defined(BLAKE3_NO_AVX512)
  ಆದರೆ (features & AVX512VL) {
    blake3_compress_ಒಳ_ಇರಿಸು_avx512(cv, ಖಂಡ, ಖಂಡ_len, counter, flags);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(BLAKE3_NO_SSE41)
  ಆದರೆ (features & SSE41) {
    blake3_compress_ಒಳ_ಇರಿಸು_sse41(cv, ಖಂಡ, ಖಂಡ_len, counter, flags);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(BLAKE3_NO_SSE2)
  ಆದರೆ (features & SSE2) {
    blake3_compress_ಒಳ_ಇರಿಸು_sse2(cv, ಖಂಡ, ಖಂಡ_len, counter, flags);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
  blake3_compress_ಒಳ_ಇರಿಸು_portable(cv, ಖಂಡ, ಖಂಡ_len, counter, flags);
}

ಶೂನ್ಯ blake3_compress_xof(ಸ್ಥಿರಾಂಕ uint32_t cv[8],
                         ಸ್ಥಿರಾಂಕ uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN],
                         uint8_t ಖಂಡ_len, uint64_t counter, uint8_t flags,
                         uint8_t ಹೊರಹೋಗು_2[64]) {
#ಆದರೆ defined(IS_X86)
  ಸ್ಥಿರಾಂಕ ಎಣಿಕೆ cpu_feature features = get_cpu_features();
  MAYBE_ಬಳಸದ(features);
#ಆದರೆ !defined(BLAKE3_NO_AVX512)
  ಆದರೆ (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, ಖಂಡ, ಖಂಡ_len, counter, flags, ಹೊರಹೋಗು_2);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(BLAKE3_NO_SSE41)
  ಆದರೆ (features & SSE41) {
    blake3_compress_xof_sse41(cv, ಖಂಡ, ಖಂಡ_len, counter, flags, ಹೊರಹೋಗು_2);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(BLAKE3_NO_SSE2)
  ಆದರೆ (features & SSE2) {
    blake3_compress_xof_sse2(cv, ಖಂಡ, ಖಂಡ_len, counter, flags, ಹೊರಹೋಗು_2);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
  blake3_compress_xof_portable(cv, ಖಂಡ, ಖಂಡ_len, counter, flags, ಹೊರಹೋಗು_2);
}


ಶೂನ್ಯ blake3_xof_many(ಸ್ಥಿರಾಂಕ uint32_t cv[8],
                     ಸ್ಥಿರಾಂಕ uint8_t ಖಂಡ[BLAKE3_ಖಂಡ_LEN],
                     uint8_t ಖಂಡ_len, uint64_t counter, uint8_t flags,
                     uint8_t ಹೊರಹೋಗು_2[64], size_t outblocks) {
  ಆದರೆ (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ defined(IS_X86)
  ಸ್ಥಿರಾಂಕ ಎಣಿಕೆ cpu_feature features = get_cpu_features();
  MAYBE_ಬಳಸದ(features);
#ಆದರೆ !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  ಆದರೆ (features & AVX512VL) {
    blake3_xof_many_avx512(cv, ಖಂಡ, ಖಂಡ_len, counter, flags, ಹೊರಹೋಗು_2, outblocks);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
  ಗಾಗಿ(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, ಖಂಡ, ಖಂಡ_len, counter + i, flags, ಹೊರಹೋಗು_2 + 64*i);
  }
}

ಶೂನ್ಯ blake3_hash_many(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *inputs, size_t ಅಂಕೆ_inputs,
                      size_t ಖಂಡಗಳು, ಸ್ಥಿರಾಂಕ uint32_t ಕೀಲಿ_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_ಆರಂಭ, uint8_t flags_ಕೊನೆ, uint8_t *ಹೊರಹೋಗು_2) {
#ಆದರೆ defined(IS_X86)
  ಸ್ಥಿರಾಂಕ ಎಣಿಕೆ cpu_feature features = get_cpu_features();
  MAYBE_ಬಳಸದ(features);
#ಆದರೆ !defined(BLAKE3_NO_AVX512)
  ಆದರೆ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, ಅಂಕೆ_inputs, ಖಂಡಗಳು, ಕೀಲಿ_2, counter,
                            increment_counter, flags, flags_ಆರಂಭ, flags_ಕೊನೆ,
                            ಹೊರಹೋಗು_2);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(BLAKE3_NO_AVX2)
  ಆದರೆ (features & AVX2) {
    blake3_hash_many_avx2(inputs, ಅಂಕೆ_inputs, ಖಂಡಗಳು, ಕೀಲಿ_2, counter,
                          increment_counter, flags, flags_ಆರಂಭ, flags_ಕೊನೆ,
                          ಹೊರಹೋಗು_2);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(BLAKE3_NO_SSE41)
  ಆದರೆ (features & SSE41) {
    blake3_hash_many_sse41(inputs, ಅಂಕೆ_inputs, ಖಂಡಗಳು, ಕೀಲಿ_2, counter,
                           increment_counter, flags, flags_ಆರಂಭ, flags_ಕೊನೆ,
                           ಹೊರಹೋಗು_2);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(BLAKE3_NO_SSE2)
  ಆದರೆ (features & SSE2) {
    blake3_hash_many_sse2(inputs, ಅಂಕೆ_inputs, ಖಂಡಗಳು, ಕೀಲಿ_2, counter,
                          increment_counter, flags, flags_ಆರಂಭ, flags_ಕೊನೆ,
                          ಹೊರಹೋಗು_2);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, ಅಂಕೆ_inputs, ಖಂಡಗಳು, ಕೀಲಿ_2, counter,
                        increment_counter, flags, flags_ಆರಂಭ, flags_ಕೊನೆ, ಹೊರಹೋಗು_2);
  ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ

  blake3_hash_many_portable(inputs, ಅಂಕೆ_inputs, ಖಂಡಗಳು, ಕೀಲಿ_2, counter,
                            increment_counter, flags, flags_ಆರಂಭ, flags_ಕೊನೆ,
                            ಹೊರಹೋಗು_2);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(ಶೂನ್ಯ) {
#ಆದರೆ defined(IS_X86)
  ಸ್ಥಿರಾಂಕ ಎಣಿಕೆ cpu_feature features = get_cpu_features();
  MAYBE_ಬಳಸದ(features);
#ಆದರೆ !defined(BLAKE3_NO_AVX512)
  ಆದರೆ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    ಹಿಂತಿರುಗಿ 16;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(BLAKE3_NO_AVX2)
  ಆದರೆ (features & AVX2) {
    ಹಿಂತಿರುಗಿ 8;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(BLAKE3_NO_SSE41)
  ಆದರೆ (features & SSE41) {
    ಹಿಂತಿರುಗಿ 4;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(BLAKE3_NO_SSE2)
  ಆದರೆ (features & SSE2) {
    ಹಿಂತಿರುಗಿ 4;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ BLAKE3_USE_NEON == 1
  ಹಿಂತಿರುಗಿ 4;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ 1;
}
