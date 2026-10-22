#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>

#ଅନ୍ତର୍ଭାଵ "blake3_impl.h"

#ଯଦି defined(_MSC_VER)
#ଅନ୍ତର୍ଭାଵ <Windows.h>
#endif

#ଯଦି defined(IS_X86)
#ଯଦି defined(_MSC_VER)
#ଅନ୍ତର୍ଭାଵ <intrin.h>
#ନଚେତଯଦି defined(__GNUC__)
#ଅନ୍ତର୍ଭାଵ <immintrin.h>
#ଅନ୍ୟଥା
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#ଯଦି !defined(BLAKE3_ATOMICS)
#ଯଦି defined(__has_include)
#ଯଦି __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#ଅନ୍ୟଥା
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#ଅନ୍ୟଥା
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#ଯଦି BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic ପୂର୍ଣ୍ଣସଂଖ୍ୟା
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#ନଚେତଯଦି defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#ଅନ୍ୟଥା
#define ATOMIC_INT ପୂର୍ଣ୍ଣସଂଖ୍ୟା
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (ଶୂନ୍ୟ)((x))

#ଯଦି defined(IS_X86)
ସ୍ଥିର uint64_t xgetbv(ଶୂନ୍ୟ) {
#ଯଦି defined(_MSC_VER)
  ଫେରନ୍ତୁ _xgetbv(0);
#ଅନ୍ୟଥା
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  ଫେରନ୍ତୁ ((uint64_t)edx << 32) | eax;
#endif
}

ସ୍ଥିର ଶୂନ୍ୟ cpuid(uint32_t out[4], uint32_t id) {
#ଯଦି defined(_MSC_VER)
  __cpuid((ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)out, id);
#ନଚେତଯଦି defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#ଅନ୍ୟଥା
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

ସ୍ଥିର ଶୂନ୍ୟ cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#ଯଦି defined(_MSC_VER)
  __cpuidex((ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)out, id, sid);
#ନଚେତଯଦି defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#ଅନ୍ୟଥା
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#endif
}

#endif

ଗଣନା cpu_feature {
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

#ଯଦି !defined(BLAKE3_TESTING)
ସ୍ଥିର /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#ଯଦି !defined(BLAKE3_TESTING)
ସ୍ଥିର
#endif
    ଗଣନା cpu_feature
    get_cpu_features(ଶୂନ୍ୟ) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  ଗଣନା cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  ଯଦି (features != UNDEFINED) {
    ଫେରନ୍ତୁ features;
  } ଅନ୍ୟଥା {
#ଯଦି defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (ଶୂନ୍ୟ)edx;
    features = 0;
    cpuid(regs, 0);
    ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_id = *eax;
    cpuid(regs, 1);
#ଯଦି defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#ଅନ୍ୟଥା
    ଯଦି (*edx & (1UL << 26))
      features |= SSE2;
#endif
    ଯଦି (*ecx & (1UL << 9))
      features |= SSSE3;
    ଯଦି (*ecx & (1UL << 19))
      features |= SSE41;

    ଯଦି (*ecx & (1UL << 27)) { // OSXSAVE
      ଅଚଳ uint64_t mask = xgetbv();
      ଯଦି ((mask & 6) == 6) { // SSE and AVX states
        ଯଦି (*ecx & (1UL << 28))
          features |= AVX;
        ଯଦି (max_id >= 7) {
          cpuidex(regs, 7, 0);
          ଯଦି (*ebx & (1UL << 5))
            features |= AVX2;
          ଯଦି ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            ଯଦି (*ebx & (1UL << 31))
              features |= AVX512VL;
            ଯଦି (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    ଫେରନ୍ତୁ features;
#ଅନ୍ୟଥା
    /* How to detect NEON? */
    ଫେରନ୍ତୁ 0;
#endif
  }
}

ଶୂନ୍ୟ blake3_compress_in_place(uint32_t cv[8],
                              ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#ଯଦି defined(IS_X86)
  ଅଚଳ ଗଣନା cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ଯଦି !defined(BLAKE3_NO_AVX512)
  ଯଦି (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    ଫେରନ୍ତୁ;
  }
#endif
#ଯଦି !defined(BLAKE3_NO_SSE41)
  ଯଦି (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    ଫେରନ୍ତୁ;
  }
#endif
#ଯଦି !defined(BLAKE3_NO_SSE2)
  ଯଦି (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    ଫେରନ୍ତୁ;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

ଶୂନ୍ୟ blake3_compress_xof(ଅଚଳ uint32_t cv[8],
                         ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#ଯଦି defined(IS_X86)
  ଅଚଳ ଗଣନା cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ଯଦି !defined(BLAKE3_NO_AVX512)
  ଯଦି (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    ଫେରନ୍ତୁ;
  }
#endif
#ଯଦି !defined(BLAKE3_NO_SSE41)
  ଯଦି (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    ଫେରନ୍ତୁ;
  }
#endif
#ଯଦି !defined(BLAKE3_NO_SSE2)
  ଯଦି (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    ଫେରନ୍ତୁ;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


ଶୂନ୍ୟ blake3_xof_many(ଅଚଳ uint32_t cv[8],
                     ଅଚଳ uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  ଯଦି (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    ଫେରନ୍ତୁ;
  }
#ଯଦି defined(IS_X86)
  ଅଚଳ ଗଣନା cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ଯଦି !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  ଯଦି (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    ଫେରନ୍ତୁ;
  }
#endif
#endif
  ପାଇଁ(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

ଶୂନ୍ୟ blake3_hash_many(ଅଚଳ uint8_t *ଅଚଳ *inputs, size_t ଅଙ୍କ_inputs,
                      size_t blocks, ଅଚଳ uint32_t ଚାବି_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#ଯଦି defined(IS_X86)
  ଅଚଳ ଗଣନା cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ଯଦି !defined(BLAKE3_NO_AVX512)
  ଯଦି ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, ଅଙ୍କ_inputs, blocks, ଚାବି_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    ଫେରନ୍ତୁ;
  }
#endif
#ଯଦି !defined(BLAKE3_NO_AVX2)
  ଯଦି (features & AVX2) {
    blake3_hash_many_avx2(inputs, ଅଙ୍କ_inputs, blocks, ଚାବି_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ଫେରନ୍ତୁ;
  }
#endif
#ଯଦି !defined(BLAKE3_NO_SSE41)
  ଯଦି (features & SSE41) {
    blake3_hash_many_sse41(inputs, ଅଙ୍କ_inputs, blocks, ଚାବି_2, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    ଫେରନ୍ତୁ;
  }
#endif
#ଯଦି !defined(BLAKE3_NO_SSE2)
  ଯଦି (features & SSE2) {
    blake3_hash_many_sse2(inputs, ଅଙ୍କ_inputs, blocks, ଚାବି_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ଫେରନ୍ତୁ;
  }
#endif
#endif

#ଯଦି BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, ଅଙ୍କ_inputs, blocks, ଚାବି_2, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  ଫେରନ୍ତୁ;
#endif

  blake3_hash_many_portable(inputs, ଅଙ୍କ_inputs, blocks, ଚାବି_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(ଶୂନ୍ୟ) {
#ଯଦି defined(IS_X86)
  ଅଚଳ ଗଣନା cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ଯଦି !defined(BLAKE3_NO_AVX512)
  ଯଦି ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    ଫେରନ୍ତୁ 16;
  }
#endif
#ଯଦି !defined(BLAKE3_NO_AVX2)
  ଯଦି (features & AVX2) {
    ଫେରନ୍ତୁ 8;
  }
#endif
#ଯଦି !defined(BLAKE3_NO_SSE41)
  ଯଦି (features & SSE41) {
    ଫେରନ୍ତୁ 4;
  }
#endif
#ଯଦି !defined(BLAKE3_NO_SSE2)
  ଯଦି (features & SSE2) {
    ଫେରନ୍ତୁ 4;
  }
#endif
#endif
#ଯଦି BLAKE3_USE_NEON == 1
  ଫେରନ୍ତୁ 4;
#endif
  ଫେରନ୍ତୁ 1;
}
