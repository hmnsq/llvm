#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdint.h>

#అన్తర్భావ "blake3_impl.h"

#ఒకవేళ defined(_MSC_VER)
#అన్తర్భావ <Windows.h>
#అయితే_ముగింపు

#ఒకవేళ defined(IS_X86)
#ఒకవేళ defined(_MSC_VER)
#అన్తర్భావ <intrin.h>
#లేదావేళ defined(__GNUC__)
#అన్తర్భావ <immintrin.h>
#లేకపోతే
#నిర్వచనం_రద్దు IS_X86 /* Unimplemented! */
#అయితే_ముగింపు
#అయితే_ముగింపు

#ఒకవేళ !defined(BLAKE3_ATOMICS)
#ఒకవేళ defined(__has_include)
#ఒకవేళ __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#నిర్వచించు BLAKE3_ATOMICS 1
#లేకపోతే
#నిర్వచించు BLAKE3_ATOMICS 0
#అయితే_ముగింపు /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#లేకపోతే
#నిర్వచించు BLAKE3_ATOMICS 0
#అయితే_ముగింపు /* defined(__has_include) */
#అయితే_ముగింపు /* BLAKE3_ATOMICS */

#ఒకవేళ BLAKE3_ATOMICS
#నిర్వచించు ATOMIC_INT _Atomic పూర్ణాంకం
#నిర్వచించు ATOMIC_ఎక్కించు(x) x
#నిర్వచించు ATOMIC_STORE(x, y) x = y
#లేదావేళ defined(_MSC_VER)
#నిర్వచించు ATOMIC_INT LONG
#నిర్వచించు ATOMIC_ఎక్కించు(x) InterlockedOr(&x, 0)
#నిర్వచించు ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#లేకపోతే
#నిర్వచించు ATOMIC_INT పూర్ణాంకం
#నిర్వచించు ATOMIC_ఎక్కించు(x) x
#నిర్వచించు ATOMIC_STORE(x, y) x = y
#అయితే_ముగింపు

#నిర్వచించు MAYBE_వాడని(x) (శూన్య)((x))

#ఒకవేళ defined(IS_X86)
స్థిర uint64_t xgetbv(శూన్య) {
#ఒకవేళ defined(_MSC_VER)
  తిరిగి _xgetbv(0);
#లేకపోతే
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  తిరిగి ((uint64_t)edx << 32) | eax;
#అయితే_ముగింపు
}

స్థిర శూన్య cpuid(uint32_t బహిర్గతం_2[4], uint32_t id) {
#ఒకవేళ defined(_MSC_VER)
  __cpuid((పూర్ణాంకం *)బహిర్గతం_2, id);
#లేదావేళ defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(బహిర్గతం_2[0]), "=r"(బహిర్గతం_2[1]), "=c"(బహిర్గతం_2[2]), "=d"(బహిర్గతం_2[3])
                       : "a"(id));
#లేకపోతే
  __asm__ __volatile__("cpuid\n"
                       : "=a"(బహిర్గతం_2[0]), "=b"(బహిర్గతం_2[1]), "=c"(బహిర్గతం_2[2]), "=d"(బహిర్గతం_2[3])
                       : "a"(id));
#అయితే_ముగింపు
}

స్థిర శూన్య cpuidex(uint32_t బహిర్గతం_2[4], uint32_t id, uint32_t sid) {
#ఒకవేళ defined(_MSC_VER)
  __cpuidex((పూర్ణాంకం *)బహిర్గతం_2, id, sid);
#లేదావేళ defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(బహిర్గతం_2[0]), "=r"(బహిర్గతం_2[1]), "=c"(బహిర్గతం_2[2]), "=d"(బహిర్గతం_2[3])
                       : "a"(id), "c"(sid));
#లేకపోతే
  __asm__ __volatile__("cpuid\n"
                       : "=a"(బహిర్గతం_2[0]), "=b"(బహిర్గతం_2[1]), "=c"(బహిర్గతం_2[2]), "=d"(బహిర్గతం_2[3])
                       : "a"(id), "c"(sid));
#అయితే_ముగింపు
}

#అయితే_ముగింపు

గణన cpu_feature {
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

#ఒకవేళ !defined(BLAKE3_TESTING)
స్థిర /* Allow the variable to be controlled manually for testing */
#అయితే_ముగింపు
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_వాడిన
#ఒకవేళ !defined(BLAKE3_TESTING)
స్థిర
#అయితే_ముగింపు
    గణన cpu_feature
    get_cpu_features(శూన్య) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  గణన cpu_feature features = ATOMIC_ఎక్కించు(g_cpu_features);
  ఒకవేళ (features != UNDEFINED) {
    తిరిగి features;
  } లేకపోతే {
#ఒకవేళ defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (శూన్య)edx;
    features = 0;
    cpuid(regs, 0);
    స్థిరరాశి పూర్ణాంకం max_id = *eax;
    cpuid(regs, 1);
#ఒకవేళ defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#లేకపోతే
    ఒకవేళ (*edx & (1UL << 26))
      features |= SSE2;
#అయితే_ముగింపు
    ఒకవేళ (*ecx & (1UL << 9))
      features |= SSSE3;
    ఒకవేళ (*ecx & (1UL << 19))
      features |= SSE41;

    ఒకవేళ (*ecx & (1UL << 27)) { // OSXSAVE
      స్థిరరాశి uint64_t ముసుగు_2 = xgetbv();
      ఒకవేళ ((ముసుగు_2 & 6) == 6) { // SSE and AVX states
        ఒకవేళ (*ecx & (1UL << 28))
          features |= AVX;
        ఒకవేళ (max_id >= 7) {
          cpuidex(regs, 7, 0);
          ఒకవేళ (*ebx & (1UL << 5))
            features |= AVX2;
          ఒకవేళ ((ముసుగు_2 & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            ఒకవేళ (*ebx & (1UL << 31))
              features |= AVX512VL;
            ఒకవేళ (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    తిరిగి features;
#లేకపోతే
    /* How to detect NEON? */
    తిరిగి 0;
#అయితే_ముగింపు
  }
}

శూన్య blake3_compress_లోపల_ఉంచు(uint32_t cv[8],
                              స్థిరరాశి uint8_t ఖండం[BLAKE3_ఖండం_LEN],
                              uint8_t ఖండం_len, uint64_t counter,
                              uint8_t flags) {
#ఒకవేళ defined(IS_X86)
  స్థిరరాశి గణన cpu_feature features = get_cpu_features();
  MAYBE_వాడని(features);
#ఒకవేళ !defined(BLAKE3_NO_AVX512)
  ఒకవేళ (features & AVX512VL) {
    blake3_compress_లోపల_ఉంచు_avx512(cv, ఖండం, ఖండం_len, counter, flags);
    తిరిగి;
  }
#అయితే_ముగింపు
#ఒకవేళ !defined(BLAKE3_NO_SSE41)
  ఒకవేళ (features & SSE41) {
    blake3_compress_లోపల_ఉంచు_sse41(cv, ఖండం, ఖండం_len, counter, flags);
    తిరిగి;
  }
#అయితే_ముగింపు
#ఒకవేళ !defined(BLAKE3_NO_SSE2)
  ఒకవేళ (features & SSE2) {
    blake3_compress_లోపల_ఉంచు_sse2(cv, ఖండం, ఖండం_len, counter, flags);
    తిరిగి;
  }
#అయితే_ముగింపు
#అయితే_ముగింపు
  blake3_compress_లోపల_ఉంచు_portable(cv, ఖండం, ఖండం_len, counter, flags);
}

శూన్య blake3_compress_xof(స్థిరరాశి uint32_t cv[8],
                         స్థిరరాశి uint8_t ఖండం[BLAKE3_ఖండం_LEN],
                         uint8_t ఖండం_len, uint64_t counter, uint8_t flags,
                         uint8_t బహిర్గతం_2[64]) {
#ఒకవేళ defined(IS_X86)
  స్థిరరాశి గణన cpu_feature features = get_cpu_features();
  MAYBE_వాడని(features);
#ఒకవేళ !defined(BLAKE3_NO_AVX512)
  ఒకవేళ (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, ఖండం, ఖండం_len, counter, flags, బహిర్గతం_2);
    తిరిగి;
  }
#అయితే_ముగింపు
#ఒకవేళ !defined(BLAKE3_NO_SSE41)
  ఒకవేళ (features & SSE41) {
    blake3_compress_xof_sse41(cv, ఖండం, ఖండం_len, counter, flags, బహిర్గతం_2);
    తిరిగి;
  }
#అయితే_ముగింపు
#ఒకవేళ !defined(BLAKE3_NO_SSE2)
  ఒకవేళ (features & SSE2) {
    blake3_compress_xof_sse2(cv, ఖండం, ఖండం_len, counter, flags, బహిర్గతం_2);
    తిరిగి;
  }
#అయితే_ముగింపు
#అయితే_ముగింపు
  blake3_compress_xof_portable(cv, ఖండం, ఖండం_len, counter, flags, బహిర్గతం_2);
}


శూన్య blake3_xof_many(స్థిరరాశి uint32_t cv[8],
                     స్థిరరాశి uint8_t ఖండం[BLAKE3_ఖండం_LEN],
                     uint8_t ఖండం_len, uint64_t counter, uint8_t flags,
                     uint8_t బహిర్గతం_2[64], size_t outblocks) {
  ఒకవేళ (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    తిరిగి;
  }
#ఒకవేళ defined(IS_X86)
  స్థిరరాశి గణన cpu_feature features = get_cpu_features();
  MAYBE_వాడని(features);
#ఒకవేళ !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  ఒకవేళ (features & AVX512VL) {
    blake3_xof_many_avx512(cv, ఖండం, ఖండం_len, counter, flags, బహిర్గతం_2, outblocks);
    తిరిగి;
  }
#అయితే_ముగింపు
#అయితే_ముగింపు
  కొరకు(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, ఖండం, ఖండం_len, counter + i, flags, బహిర్గతం_2 + 64*i);
  }
}

శూన్య blake3_hash_many(స్థిరరాశి uint8_t *స్థిరరాశి *inputs, size_t అంకె_inputs,
                      size_t ఖండాలు, స్థిరరాశి uint32_t కీలకం_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_ప్రారంభం, uint8_t flags_చివర, uint8_t *బహిర్గతం_2) {
#ఒకవేళ defined(IS_X86)
  స్థిరరాశి గణన cpu_feature features = get_cpu_features();
  MAYBE_వాడని(features);
#ఒకవేళ !defined(BLAKE3_NO_AVX512)
  ఒకవేళ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, అంకె_inputs, ఖండాలు, కీలకం_2, counter,
                            increment_counter, flags, flags_ప్రారంభం, flags_చివర,
                            బహిర్గతం_2);
    తిరిగి;
  }
#అయితే_ముగింపు
#ఒకవేళ !defined(BLAKE3_NO_AVX2)
  ఒకవేళ (features & AVX2) {
    blake3_hash_many_avx2(inputs, అంకె_inputs, ఖండాలు, కీలకం_2, counter,
                          increment_counter, flags, flags_ప్రారంభం, flags_చివర,
                          బహిర్గతం_2);
    తిరిగి;
  }
#అయితే_ముగింపు
#ఒకవేళ !defined(BLAKE3_NO_SSE41)
  ఒకవేళ (features & SSE41) {
    blake3_hash_many_sse41(inputs, అంకె_inputs, ఖండాలు, కీలకం_2, counter,
                           increment_counter, flags, flags_ప్రారంభం, flags_చివర,
                           బహిర్గతం_2);
    తిరిగి;
  }
#అయితే_ముగింపు
#ఒకవేళ !defined(BLAKE3_NO_SSE2)
  ఒకవేళ (features & SSE2) {
    blake3_hash_many_sse2(inputs, అంకె_inputs, ఖండాలు, కీలకం_2, counter,
                          increment_counter, flags, flags_ప్రారంభం, flags_చివర,
                          బహిర్గతం_2);
    తిరిగి;
  }
#అయితే_ముగింపు
#అయితే_ముగింపు

#ఒకవేళ BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, అంకె_inputs, ఖండాలు, కీలకం_2, counter,
                        increment_counter, flags, flags_ప్రారంభం, flags_చివర, బహిర్గతం_2);
  తిరిగి;
#అయితే_ముగింపు

  blake3_hash_many_portable(inputs, అంకె_inputs, ఖండాలు, కీలకం_2, counter,
                            increment_counter, flags, flags_ప్రారంభం, flags_చివర,
                            బహిర్గతం_2);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(శూన్య) {
#ఒకవేళ defined(IS_X86)
  స్థిరరాశి గణన cpu_feature features = get_cpu_features();
  MAYBE_వాడని(features);
#ఒకవేళ !defined(BLAKE3_NO_AVX512)
  ఒకవేళ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    తిరిగి 16;
  }
#అయితే_ముగింపు
#ఒకవేళ !defined(BLAKE3_NO_AVX2)
  ఒకవేళ (features & AVX2) {
    తిరిగి 8;
  }
#అయితే_ముగింపు
#ఒకవేళ !defined(BLAKE3_NO_SSE41)
  ఒకవేళ (features & SSE41) {
    తిరిగి 4;
  }
#అయితే_ముగింపు
#ఒకవేళ !defined(BLAKE3_NO_SSE2)
  ఒకవేళ (features & SSE2) {
    తిరిగి 4;
  }
#అయితే_ముగింపు
#అయితే_ముగింపు
#ఒకవేళ BLAKE3_USE_NEON == 1
  తిరిగి 4;
#అయితే_ముగింపు
  తిరిగి 1;
}
