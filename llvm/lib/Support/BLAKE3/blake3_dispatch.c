#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>

#ಅನ್ತರ್ಭಾವ "blake3_impl.h"

#ಯದಿ defined(_MSC_VER)
#ಅನ್ತರ್ಭಾವ <Windows.h>
#endif

#ಯದಿ defined(IS_X86)
#ಯದಿ defined(_MSC_VER)
#ಅನ್ತರ್ಭಾವ <intrin.h>
#ನೋಚೇದ್ಯದಿ defined(__GNUC__)
#ಅನ್ತರ್ಭಾವ <immintrin.h>
#ಅನ್ಯಥಾ
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#ಯದಿ !defined(BLAKE3_ATOMICS)
#ಯದಿ defined(__has_include)
#ಯದಿ __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#ಅನ್ಯಥಾ
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#ಅನ್ಯಥಾ
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#ಯದಿ BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic ಪೂರ್ಣಾಙ್ಕ
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#ನೋಚೇದ್ಯದಿ defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#ಅನ್ಯಥಾ
#define ATOMIC_INT ಪೂರ್ಣಾಙ್ಕ
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (ಶೂನ್ಯ)((x))

#ಯದಿ defined(IS_X86)
ಸ್ಥಿರ uint64_t xgetbv(ಶೂನ್ಯ) {
#ಯದಿ defined(_MSC_VER)
  ಪ್ರತ್ಯಾಗಮ _xgetbv(0);
#ಅನ್ಯಥಾ
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  ಪ್ರತ್ಯಾಗಮ ((uint64_t)edx << 32) | eax;
#endif
}

ಸ್ಥಿರ ಶೂನ್ಯ cpuid(uint32_t out[4], uint32_t id) {
#ಯದಿ defined(_MSC_VER)
  __cpuid((ಪೂರ್ಣಾಙ್ಕ *)out, id);
#ನೋಚೇದ್ಯದಿ defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#ಅನ್ಯಥಾ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

ಸ್ಥಿರ ಶೂನ್ಯ cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#ಯದಿ defined(_MSC_VER)
  __cpuidex((ಪೂರ್ಣಾಙ್ಕ *)out, id, sid);
#ನೋಚೇದ್ಯದಿ defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#ಅನ್ಯಥಾ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#endif
}

#endif

ಗಣನಾ cpu_feature {
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

#ಯದಿ !defined(BLAKE3_TESTING)
ಸ್ಥಿರ /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#ಯದಿ !defined(BLAKE3_TESTING)
ಸ್ಥಿರ
#endif
    ಗಣನಾ cpu_feature
    get_cpu_features(ಶೂನ್ಯ) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  ಗಣನಾ cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  ಯದಿ (features != UNDEFINED) {
    ಪ್ರತ್ಯಾಗಮ features;
  } ಅನ್ಯಥಾ {
#ಯದಿ defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (ಶೂನ್ಯ)edx;
    features = 0;
    cpuid(regs, 0);
    ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ max_id = *eax;
    cpuid(regs, 1);
#ಯದಿ defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#ಅನ್ಯಥಾ
    ಯದಿ (*edx & (1UL << 26))
      features |= SSE2;
#endif
    ಯದಿ (*ecx & (1UL << 9))
      features |= SSSE3;
    ಯದಿ (*ecx & (1UL << 19))
      features |= SSE41;

    ಯದಿ (*ecx & (1UL << 27)) { // OSXSAVE
      ಧ್ರುವ uint64_t mask = xgetbv();
      ಯದಿ ((mask & 6) == 6) { // SSE and AVX states
        ಯದಿ (*ecx & (1UL << 28))
          features |= AVX;
        ಯದಿ (max_id >= 7) {
          cpuidex(regs, 7, 0);
          ಯದಿ (*ebx & (1UL << 5))
            features |= AVX2;
          ಯದಿ ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            ಯದಿ (*ebx & (1UL << 31))
              features |= AVX512VL;
            ಯದಿ (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    ಪ್ರತ್ಯಾಗಮ features;
#ಅನ್ಯಥಾ
    /* How to detect NEON? */
    ಪ್ರತ್ಯಾಗಮ 0;
#endif
  }
}

ಶೂನ್ಯ blake3_compress_in_place(uint32_t cv[8],
                              ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#ಯದಿ defined(IS_X86)
  ಧ್ರುವ ಗಣನಾ cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ಯದಿ !defined(BLAKE3_NO_AVX512)
  ಯದಿ (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
#ಯದಿ !defined(BLAKE3_NO_SSE41)
  ಯದಿ (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
#ಯದಿ !defined(BLAKE3_NO_SSE2)
  ಯದಿ (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

ಶೂನ್ಯ blake3_compress_xof(ಧ್ರುವ uint32_t cv[8],
                         ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#ಯದಿ defined(IS_X86)
  ಧ್ರುವ ಗಣನಾ cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ಯದಿ !defined(BLAKE3_NO_AVX512)
  ಯದಿ (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
#ಯದಿ !defined(BLAKE3_NO_SSE41)
  ಯದಿ (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
#ಯದಿ !defined(BLAKE3_NO_SSE2)
  ಯದಿ (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


ಶೂನ್ಯ blake3_xof_many(ಧ್ರುವ uint32_t cv[8],
                     ಧ್ರುವ uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  ಯದಿ (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    ಪ್ರತ್ಯಾಗಮ;
  }
#ಯದಿ defined(IS_X86)
  ಧ್ರುವ ಗಣನಾ cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ಯದಿ !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  ಯದಿ (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
#endif
  ಪ್ರತಿ(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

ಶೂನ್ಯ blake3_hash_many(ಧ್ರುವ uint8_t *ಧ್ರುವ *inputs, size_t ಅಂಕೆ_inputs,
                      size_t blocks, ಧ್ರುವ uint32_t ಕೀಲಿ_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#ಯದಿ defined(IS_X86)
  ಧ್ರುವ ಗಣನಾ cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ಯದಿ !defined(BLAKE3_NO_AVX512)
  ಯದಿ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, ಅಂಕೆ_inputs, blocks, ಕೀಲಿ_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
#ಯದಿ !defined(BLAKE3_NO_AVX2)
  ಯದಿ (features & AVX2) {
    blake3_hash_many_avx2(inputs, ಅಂಕೆ_inputs, blocks, ಕೀಲಿ_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
#ಯದಿ !defined(BLAKE3_NO_SSE41)
  ಯದಿ (features & SSE41) {
    blake3_hash_many_sse41(inputs, ಅಂಕೆ_inputs, blocks, ಕೀಲಿ_2, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
#ಯದಿ !defined(BLAKE3_NO_SSE2)
  ಯದಿ (features & SSE2) {
    blake3_hash_many_sse2(inputs, ಅಂಕೆ_inputs, blocks, ಕೀಲಿ_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif
#endif

#ಯದಿ BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, ಅಂಕೆ_inputs, blocks, ಕೀಲಿ_2, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  ಪ್ರತ್ಯಾಗಮ;
#endif

  blake3_hash_many_portable(inputs, ಅಂಕೆ_inputs, blocks, ಕೀಲಿ_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(ಶೂನ್ಯ) {
#ಯದಿ defined(IS_X86)
  ಧ್ರುವ ಗಣನಾ cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ಯದಿ !defined(BLAKE3_NO_AVX512)
  ಯದಿ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    ಪ್ರತ್ಯಾಗಮ 16;
  }
#endif
#ಯದಿ !defined(BLAKE3_NO_AVX2)
  ಯದಿ (features & AVX2) {
    ಪ್ರತ್ಯಾಗಮ 8;
  }
#endif
#ಯದಿ !defined(BLAKE3_NO_SSE41)
  ಯದಿ (features & SSE41) {
    ಪ್ರತ್ಯಾಗಮ 4;
  }
#endif
#ಯದಿ !defined(BLAKE3_NO_SSE2)
  ಯದಿ (features & SSE2) {
    ಪ್ರತ್ಯಾಗಮ 4;
  }
#endif
#endif
#ಯದಿ BLAKE3_USE_NEON == 1
  ಪ್ರತ್ಯಾಗಮ 4;
#endif
  ಪ್ರತ್ಯಾಗಮ 1;
}
