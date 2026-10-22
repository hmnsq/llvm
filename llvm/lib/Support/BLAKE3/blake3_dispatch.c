#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdint.h>

#अन्तर्भाव "blake3_impl.h"

#जर defined(_MSC_VER)
#अन्तर्भाव <Windows.h>
#endif

#जर defined(IS_X86)
#जर defined(_MSC_VER)
#अन्तर्भाव <intrin.h>
#किंवाजर defined(__GNUC__)
#अन्तर्भाव <immintrin.h>
#अन्यथा
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#जर !defined(BLAKE3_ATOMICS)
#जर defined(__has_include)
#जर __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#अन्यथा
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#अन्यथा
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#जर BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic पूर्णांक
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#किंवाजर defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#अन्यथा
#define ATOMIC_INT पूर्णांक
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (शून्य)((x))

#जर defined(IS_X86)
स्थिर uint64_t xgetbv(शून्य) {
#जर defined(_MSC_VER)
  परत _xgetbv(0);
#अन्यथा
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  परत ((uint64_t)edx << 32) | eax;
#endif
}

स्थिर शून्य cpuid(uint32_t out[4], uint32_t id) {
#जर defined(_MSC_VER)
  __cpuid((पूर्णांक *)out, id);
#किंवाजर defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#अन्यथा
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

स्थिर शून्य cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#जर defined(_MSC_VER)
  __cpuidex((पूर्णांक *)out, id, sid);
#किंवाजर defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#अन्यथा
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

#जर !defined(BLAKE3_TESTING)
स्थिर /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#जर !defined(BLAKE3_TESTING)
स्थिर
#endif
    गणना cpu_feature
    get_cpu_features(शून्य) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  गणना cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  जर (features != UNDEFINED) {
    परत features;
  } अन्यथा {
#जर defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (शून्य)edx;
    features = 0;
    cpuid(regs, 0);
    स्थिरांक पूर्णांक max_id = *eax;
    cpuid(regs, 1);
#जर defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#अन्यथा
    जर (*edx & (1UL << 26))
      features |= SSE2;
#endif
    जर (*ecx & (1UL << 9))
      features |= SSSE3;
    जर (*ecx & (1UL << 19))
      features |= SSE41;

    जर (*ecx & (1UL << 27)) { // OSXSAVE
      स्थिरांक uint64_t mask = xgetbv();
      जर ((mask & 6) == 6) { // SSE and AVX states
        जर (*ecx & (1UL << 28))
          features |= AVX;
        जर (max_id >= 7) {
          cpuidex(regs, 7, 0);
          जर (*ebx & (1UL << 5))
            features |= AVX2;
          जर ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            जर (*ebx & (1UL << 31))
              features |= AVX512VL;
            जर (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    परत features;
#अन्यथा
    /* How to detect NEON? */
    परत 0;
#endif
  }
}

शून्य blake3_compress_in_place(uint32_t cv[8],
                              स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#जर defined(IS_X86)
  स्थिरांक गणना cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#जर !defined(BLAKE3_NO_AVX512)
  जर (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    परत;
  }
#endif
#जर !defined(BLAKE3_NO_SSE41)
  जर (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    परत;
  }
#endif
#जर !defined(BLAKE3_NO_SSE2)
  जर (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    परत;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

शून्य blake3_compress_xof(स्थिरांक uint32_t cv[8],
                         स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#जर defined(IS_X86)
  स्थिरांक गणना cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#जर !defined(BLAKE3_NO_AVX512)
  जर (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    परत;
  }
#endif
#जर !defined(BLAKE3_NO_SSE41)
  जर (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    परत;
  }
#endif
#जर !defined(BLAKE3_NO_SSE2)
  जर (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    परत;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


शून्य blake3_xof_many(स्थिरांक uint32_t cv[8],
                     स्थिरांक uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], माप_प्रकार outblocks) {
  जर (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    परत;
  }
#जर defined(IS_X86)
  स्थिरांक गणना cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#जर !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  जर (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    परत;
  }
#endif
#endif
  साठी(माप_प्रकार i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

शून्य blake3_hash_many(स्थिरांक uint8_t *स्थिरांक *inputs, माप_प्रकार अंक_inputs,
                      माप_प्रकार blocks, स्थिरांक uint32_t किल्ली_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#जर defined(IS_X86)
  स्थिरांक गणना cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#जर !defined(BLAKE3_NO_AVX512)
  जर ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, अंक_inputs, blocks, किल्ली_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    परत;
  }
#endif
#जर !defined(BLAKE3_NO_AVX2)
  जर (features & AVX2) {
    blake3_hash_many_avx2(inputs, अंक_inputs, blocks, किल्ली_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    परत;
  }
#endif
#जर !defined(BLAKE3_NO_SSE41)
  जर (features & SSE41) {
    blake3_hash_many_sse41(inputs, अंक_inputs, blocks, किल्ली_2, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    परत;
  }
#endif
#जर !defined(BLAKE3_NO_SSE2)
  जर (features & SSE2) {
    blake3_hash_many_sse2(inputs, अंक_inputs, blocks, किल्ली_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    परत;
  }
#endif
#endif

#जर BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, अंक_inputs, blocks, किल्ली_2, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  परत;
#endif

  blake3_hash_many_portable(inputs, अंक_inputs, blocks, किल्ली_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
माप_प्रकार blake3_simd_degree(शून्य) {
#जर defined(IS_X86)
  स्थिरांक गणना cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#जर !defined(BLAKE3_NO_AVX512)
  जर ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    परत 16;
  }
#endif
#जर !defined(BLAKE3_NO_AVX2)
  जर (features & AVX2) {
    परत 8;
  }
#endif
#जर !defined(BLAKE3_NO_SSE41)
  जर (features & SSE41) {
    परत 4;
  }
#endif
#जर !defined(BLAKE3_NO_SSE2)
  जर (features & SSE2) {
    परत 4;
  }
#endif
#endif
#जर BLAKE3_USE_NEON == 1
  परत 4;
#endif
  परत 1;
}
