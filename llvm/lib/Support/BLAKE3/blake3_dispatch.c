#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdint.h>

#अन्तर्भाव "blake3_impl.h"

#जइ defined(_MSC_VER)
#अन्तर्भाव <Windows.h>
#endif

#जइ defined(IS_X86)
#जइ defined(_MSC_VER)
#अन्तर्भाव <intrin.h>
#नोचेद्यदि defined(__GNUC__)
#अन्तर्भाव <immintrin.h>
#अण्णहा
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#जइ !defined(BLAKE3_ATOMICS)
#जइ defined(__has_include)
#जइ __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#अण्णहा
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#अण्णहा
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#जइ BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic पूर्णाङ्क
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#नोचेद्यदि defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#अण्णहा
#define ATOMIC_INT पूर्णाङ्क
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (शून्य)((x))

#जइ defined(IS_X86)
स्थिर uint64_t xgetbv(शून्य) {
#जइ defined(_MSC_VER)
  प्रत्यागम _xgetbv(0);
#अण्णहा
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  प्रत्यागम ((uint64_t)edx << 32) | eax;
#endif
}

स्थिर शून्य cpuid(uint32_t out[4], uint32_t id) {
#जइ defined(_MSC_VER)
  __cpuid((पूर्णाङ्क *)out, id);
#नोचेद्यदि defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#अण्णहा
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

स्थिर शून्य cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#जइ defined(_MSC_VER)
  __cpuidex((पूर्णाङ्क *)out, id, sid);
#नोचेद्यदि defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#अण्णहा
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#endif
}

#endif

गणना cpu_feature {
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

#जइ !defined(BLAKE3_TESTING)
स्थिर /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#जइ !defined(BLAKE3_TESTING)
स्थिर
#endif
    गणना cpu_feature
    get_cpu_features(शून्य) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  गणना cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  जइ (features != UNDEFINED) {
    प्रत्यागम features;
  } अण्णहा {
#जइ defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (शून्य)edx;
    features = 0;
    cpuid(regs, 0);
    ध्रुव पूर्णाङ्क max_id = *eax;
    cpuid(regs, 1);
#जइ defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#अण्णहा
    जइ (*edx & (1UL << 26))
      features |= SSE2;
#endif
    जइ (*ecx & (1UL << 9))
      features |= SSSE3;
    जइ (*ecx & (1UL << 19))
      features |= SSE41;

    जइ (*ecx & (1UL << 27)) { // OSXSAVE
      ध्रुव uint64_t mask = xgetbv();
      जइ ((mask & 6) == 6) { // SSE and AVX states
        जइ (*ecx & (1UL << 28))
          features |= AVX;
        जइ (max_id >= 7) {
          cpuidex(regs, 7, 0);
          जइ (*ebx & (1UL << 5))
            features |= AVX2;
          जइ ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            जइ (*ebx & (1UL << 31))
              features |= AVX512VL;
            जइ (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    प्रत्यागम features;
#अण्णहा
    /* How to detect NEON? */
    प्रत्यागम 0;
#endif
  }
}

शून्य blake3_compress_in_place(uint32_t cv[8],
                              ध्रुव uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#जइ defined(IS_X86)
  ध्रुव गणना cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#जइ !defined(BLAKE3_NO_AVX512)
  जइ (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    प्रत्यागम;
  }
#endif
#जइ !defined(BLAKE3_NO_SSE41)
  जइ (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    प्रत्यागम;
  }
#endif
#जइ !defined(BLAKE3_NO_SSE2)
  जइ (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    प्रत्यागम;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

शून्य blake3_compress_xof(ध्रुव uint32_t cv[8],
                         ध्रुव uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#जइ defined(IS_X86)
  ध्रुव गणना cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#जइ !defined(BLAKE3_NO_AVX512)
  जइ (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    प्रत्यागम;
  }
#endif
#जइ !defined(BLAKE3_NO_SSE41)
  जइ (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    प्रत्यागम;
  }
#endif
#जइ !defined(BLAKE3_NO_SSE2)
  जइ (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    प्रत्यागम;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


शून्य blake3_xof_many(ध्रुव uint32_t cv[8],
                     ध्रुव uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  जइ (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    प्रत्यागम;
  }
#जइ defined(IS_X86)
  ध्रुव गणना cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#जइ !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  जइ (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    प्रत्यागम;
  }
#endif
#endif
  प्रति(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

शून्य blake3_hash_many(ध्रुव uint8_t *ध्रुव *inputs, size_t अङ्कः_inputs,
                      size_t blocks, ध्रुव uint32_t कुञ्चिका_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#जइ defined(IS_X86)
  ध्रुव गणना cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#जइ !defined(BLAKE3_NO_AVX512)
  जइ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, अङ्कः_inputs, blocks, कुञ्चिका_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    प्रत्यागम;
  }
#endif
#जइ !defined(BLAKE3_NO_AVX2)
  जइ (features & AVX2) {
    blake3_hash_many_avx2(inputs, अङ्कः_inputs, blocks, कुञ्चिका_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    प्रत्यागम;
  }
#endif
#जइ !defined(BLAKE3_NO_SSE41)
  जइ (features & SSE41) {
    blake3_hash_many_sse41(inputs, अङ्कः_inputs, blocks, कुञ्चिका_2, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    प्रत्यागम;
  }
#endif
#जइ !defined(BLAKE3_NO_SSE2)
  जइ (features & SSE2) {
    blake3_hash_many_sse2(inputs, अङ्कः_inputs, blocks, कुञ्चिका_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    प्रत्यागम;
  }
#endif
#endif

#जइ BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, अङ्कः_inputs, blocks, कुञ्चिका_2, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  प्रत्यागम;
#endif

  blake3_hash_many_portable(inputs, अङ्कः_inputs, blocks, कुञ्चिका_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(शून्य) {
#जइ defined(IS_X86)
  ध्रुव गणना cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#जइ !defined(BLAKE3_NO_AVX512)
  जइ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    प्रत्यागम 16;
  }
#endif
#जइ !defined(BLAKE3_NO_AVX2)
  जइ (features & AVX2) {
    प्रत्यागम 8;
  }
#endif
#जइ !defined(BLAKE3_NO_SSE41)
  जइ (features & SSE41) {
    प्रत्यागम 4;
  }
#endif
#जइ !defined(BLAKE3_NO_SSE2)
  जइ (features & SSE2) {
    प्रत्यागम 4;
  }
#endif
#endif
#जइ BLAKE3_USE_NEON == 1
  प्रत्यागम 4;
#endif
  प्रत्यागम 1;
}
