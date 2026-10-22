#شامل <stdbool.h>
#شامل <stddef.h>
#شامل <stdint.h>

#شامل "blake3_impl.h"

#اگر defined(_MSC_VER)
#شامل <Windows.h>
#endif

#اگر defined(IS_X86)
#اگر defined(_MSC_VER)
#شامل <intrin.h>
#وگرنهاگر defined(__GNUC__)
#شامل <immintrin.h>
#وگرنه
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#اگر !defined(BLAKE3_ATOMICS)
#اگر defined(__has_include)
#اگر __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#وگرنه
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#وگرنه
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#اگر BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic صحیح
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#وگرنهاگر defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#وگرنه
#define ATOMIC_INT صحیح
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (تهی)((x))

#اگر defined(IS_X86)
ایستا uint64_t xgetbv(تهی) {
#اگر defined(_MSC_VER)
  بازگشت _xgetbv(0);
#وگرنه
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  بازگشت ((uint64_t)edx << 32) | eax;
#endif
}

ایستا تهی cpuid(uint32_t out[4], uint32_t id) {
#اگر defined(_MSC_VER)
  __cpuid((صحیح *)out, id);
#وگرنهاگر defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#وگرنه
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

ایستا تهی cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#اگر defined(_MSC_VER)
  __cpuidex((صحیح *)out, id, sid);
#وگرنهاگر defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#وگرنه
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#endif
}

#endif

شمارش cpu_feature {
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

#اگر !defined(BLAKE3_TESTING)
ایستا /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#اگر !defined(BLAKE3_TESTING)
ایستا
#endif
    شمارش cpu_feature
    get_cpu_features(تهی) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  شمارش cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  اگر (features != UNDEFINED) {
    بازگشت features;
  } وگرنه {
#اگر defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (تهی)edx;
    features = 0;
    cpuid(regs, 0);
    ثابت صحیح max_id = *eax;
    cpuid(regs, 1);
#اگر defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#وگرنه
    اگر (*edx & (1UL << 26))
      features |= SSE2;
#endif
    اگر (*ecx & (1UL << 9))
      features |= SSSE3;
    اگر (*ecx & (1UL << 19))
      features |= SSE41;

    اگر (*ecx & (1UL << 27)) { // OSXSAVE
      ثابت uint64_t mask = xgetbv();
      اگر ((mask & 6) == 6) { // SSE and AVX states
        اگر (*ecx & (1UL << 28))
          features |= AVX;
        اگر (max_id >= 7) {
          cpuidex(regs, 7, 0);
          اگر (*ebx & (1UL << 5))
            features |= AVX2;
          اگر ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            اگر (*ebx & (1UL << 31))
              features |= AVX512VL;
            اگر (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    بازگشت features;
#وگرنه
    /* How to detect NEON? */
    بازگشت 0;
#endif
  }
}

تهی blake3_compress_in_place(uint32_t cv[8],
                              ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#اگر defined(IS_X86)
  ثابت شمارش cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#اگر !defined(BLAKE3_NO_AVX512)
  اگر (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    بازگشت;
  }
#endif
#اگر !defined(BLAKE3_NO_SSE41)
  اگر (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    بازگشت;
  }
#endif
#اگر !defined(BLAKE3_NO_SSE2)
  اگر (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    بازگشت;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

تهی blake3_compress_xof(ثابت uint32_t cv[8],
                         ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#اگر defined(IS_X86)
  ثابت شمارش cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#اگر !defined(BLAKE3_NO_AVX512)
  اگر (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    بازگشت;
  }
#endif
#اگر !defined(BLAKE3_NO_SSE41)
  اگر (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    بازگشت;
  }
#endif
#اگر !defined(BLAKE3_NO_SSE2)
  اگر (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    بازگشت;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


تهی blake3_xof_many(ثابت uint32_t cv[8],
                     ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  اگر (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    بازگشت;
  }
#اگر defined(IS_X86)
  ثابت شمارش cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#اگر !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  اگر (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    بازگشت;
  }
#endif
#endif
  برای(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

تهی blake3_hash_many(ثابت uint8_t *ثابت *inputs, size_t رقم_inputs,
                      size_t blocks, ثابت uint32_t کلید_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#اگر defined(IS_X86)
  ثابت شمارش cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#اگر !defined(BLAKE3_NO_AVX512)
  اگر ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, رقم_inputs, blocks, کلید_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    بازگشت;
  }
#endif
#اگر !defined(BLAKE3_NO_AVX2)
  اگر (features & AVX2) {
    blake3_hash_many_avx2(inputs, رقم_inputs, blocks, کلید_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    بازگشت;
  }
#endif
#اگر !defined(BLAKE3_NO_SSE41)
  اگر (features & SSE41) {
    blake3_hash_many_sse41(inputs, رقم_inputs, blocks, کلید_2, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    بازگشت;
  }
#endif
#اگر !defined(BLAKE3_NO_SSE2)
  اگر (features & SSE2) {
    blake3_hash_many_sse2(inputs, رقم_inputs, blocks, کلید_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    بازگشت;
  }
#endif
#endif

#اگر BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, رقم_inputs, blocks, کلید_2, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  بازگشت;
#endif

  blake3_hash_many_portable(inputs, رقم_inputs, blocks, کلید_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(تهی) {
#اگر defined(IS_X86)
  ثابت شمارش cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#اگر !defined(BLAKE3_NO_AVX512)
  اگر ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    بازگشت 16;
  }
#endif
#اگر !defined(BLAKE3_NO_AVX2)
  اگر (features & AVX2) {
    بازگشت 8;
  }
#endif
#اگر !defined(BLAKE3_NO_SSE41)
  اگر (features & SSE41) {
    بازگشت 4;
  }
#endif
#اگر !defined(BLAKE3_NO_SSE2)
  اگر (features & SSE2) {
    بازگشت 4;
  }
#endif
#endif
#اگر BLAKE3_USE_NEON == 1
  بازگشت 4;
#endif
  بازگشت 1;
}
