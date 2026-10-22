#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdint.h>

#அந்தர்பாவ "blake3_impl.h"

#எனில் defined(_MSC_VER)
#அந்தர்பாவ <Windows.h>
#endif

#எனில் defined(IS_X86)
#எனில் defined(_MSC_VER)
#அந்தர்பாவ <intrin.h>
#அல்லதெனில் defined(__GNUC__)
#அந்தர்பாவ <immintrin.h>
#இல்லையெனில்
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#எனில் !defined(BLAKE3_ATOMICS)
#எனில் defined(__has_include)
#எனில் __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#இல்லையெனில்
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#இல்லையெனில்
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#எனில் BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic முழு
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#அல்லதெனில் defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#இல்லையெனில்
#define ATOMIC_INT முழு
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (வெறுமை)((x))

#எனில் defined(IS_X86)
நிலையான uint64_t xgetbv(வெறுமை) {
#எனில் defined(_MSC_VER)
  திரும்பு _xgetbv(0);
#இல்லையெனில்
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  திரும்பு ((uint64_t)edx << 32) | eax;
#endif
}

நிலையான வெறுமை cpuid(uint32_t out[4], uint32_t id) {
#எனில் defined(_MSC_VER)
  __cpuid((முழு *)out, id);
#அல்லதெனில் defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#இல்லையெனில்
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

நிலையான வெறுமை cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#எனில் defined(_MSC_VER)
  __cpuidex((முழு *)out, id, sid);
#அல்லதெனில் defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#இல்லையெனில்
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#endif
}

#endif

எண்ணிக்கை cpu_feature {
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

#எனில் !defined(BLAKE3_TESTING)
நிலையான /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#எனில் !defined(BLAKE3_TESTING)
நிலையான
#endif
    எண்ணிக்கை cpu_feature
    get_cpu_features(வெறுமை) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  எண்ணிக்கை cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  எனில் (features != UNDEFINED) {
    திரும்பு features;
  } இல்லையெனில் {
#எனில் defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (வெறுமை)edx;
    features = 0;
    cpuid(regs, 0);
    மாறிலி முழு max_id = *eax;
    cpuid(regs, 1);
#எனில் defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#இல்லையெனில்
    எனில் (*edx & (1UL << 26))
      features |= SSE2;
#endif
    எனில் (*ecx & (1UL << 9))
      features |= SSSE3;
    எனில் (*ecx & (1UL << 19))
      features |= SSE41;

    எனில் (*ecx & (1UL << 27)) { // OSXSAVE
      மாறிலி uint64_t mask = xgetbv();
      எனில் ((mask & 6) == 6) { // SSE and AVX states
        எனில் (*ecx & (1UL << 28))
          features |= AVX;
        எனில் (max_id >= 7) {
          cpuidex(regs, 7, 0);
          எனில் (*ebx & (1UL << 5))
            features |= AVX2;
          எனில் ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            எனில் (*ebx & (1UL << 31))
              features |= AVX512VL;
            எனில் (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    திரும்பு features;
#இல்லையெனில்
    /* How to detect NEON? */
    திரும்பு 0;
#endif
  }
}

வெறுமை blake3_compress_in_place(uint32_t cv[8],
                              மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#எனில் defined(IS_X86)
  மாறிலி எண்ணிக்கை cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#எனில் !defined(BLAKE3_NO_AVX512)
  எனில் (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    திரும்பு;
  }
#endif
#எனில் !defined(BLAKE3_NO_SSE41)
  எனில் (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    திரும்பு;
  }
#endif
#எனில் !defined(BLAKE3_NO_SSE2)
  எனில் (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    திரும்பு;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

வெறுமை blake3_compress_xof(மாறிலி uint32_t cv[8],
                         மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#எனில் defined(IS_X86)
  மாறிலி எண்ணிக்கை cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#எனில் !defined(BLAKE3_NO_AVX512)
  எனில் (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    திரும்பு;
  }
#endif
#எனில் !defined(BLAKE3_NO_SSE41)
  எனில் (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    திரும்பு;
  }
#endif
#எனில் !defined(BLAKE3_NO_SSE2)
  எனில் (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    திரும்பு;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


வெறுமை blake3_xof_many(மாறிலி uint32_t cv[8],
                     மாறிலி uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  எனில் (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    திரும்பு;
  }
#எனில் defined(IS_X86)
  மாறிலி எண்ணிக்கை cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#எனில் !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  எனில் (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    திரும்பு;
  }
#endif
#endif
  க்கு(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

வெறுமை blake3_hash_many(மாறிலி uint8_t *மாறிலி *inputs, size_t இலக்கம்_inputs,
                      size_t blocks, மாறிலி uint32_t சாவி_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#எனில் defined(IS_X86)
  மாறிலி எண்ணிக்கை cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#எனில் !defined(BLAKE3_NO_AVX512)
  எனில் ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, இலக்கம்_inputs, blocks, சாவி_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    திரும்பு;
  }
#endif
#எனில் !defined(BLAKE3_NO_AVX2)
  எனில் (features & AVX2) {
    blake3_hash_many_avx2(inputs, இலக்கம்_inputs, blocks, சாவி_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    திரும்பு;
  }
#endif
#எனில் !defined(BLAKE3_NO_SSE41)
  எனில் (features & SSE41) {
    blake3_hash_many_sse41(inputs, இலக்கம்_inputs, blocks, சாவி_2, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    திரும்பு;
  }
#endif
#எனில் !defined(BLAKE3_NO_SSE2)
  எனில் (features & SSE2) {
    blake3_hash_many_sse2(inputs, இலக்கம்_inputs, blocks, சாவி_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    திரும்பு;
  }
#endif
#endif

#எனில் BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, இலக்கம்_inputs, blocks, சாவி_2, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  திரும்பு;
#endif

  blake3_hash_many_portable(inputs, இலக்கம்_inputs, blocks, சாவி_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(வெறுமை) {
#எனில் defined(IS_X86)
  மாறிலி எண்ணிக்கை cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#எனில் !defined(BLAKE3_NO_AVX512)
  எனில் ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    திரும்பு 16;
  }
#endif
#எனில் !defined(BLAKE3_NO_AVX2)
  எனில் (features & AVX2) {
    திரும்பு 8;
  }
#endif
#எனில் !defined(BLAKE3_NO_SSE41)
  எனில் (features & SSE41) {
    திரும்பு 4;
  }
#endif
#எனில் !defined(BLAKE3_NO_SSE2)
  எனில் (features & SSE2) {
    திரும்பு 4;
  }
#endif
#endif
#எனில் BLAKE3_USE_NEON == 1
  திரும்பு 4;
#endif
  திரும்பு 1;
}
