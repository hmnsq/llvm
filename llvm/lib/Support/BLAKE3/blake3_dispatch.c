#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdint.h>

#ضمن "blake3_impl.h"

#ܐܢ defined(_MSC_VER)
#ضمن <Windows.h>
#endif

#ܐܢ defined(IS_X86)
#ܐܢ defined(_MSC_VER)
#ضمن <intrin.h>
#وإلاإذا defined(__GNUC__)
#ضمن <immintrin.h>
#ܐܚܪܢܐ
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#ܐܢ !defined(BLAKE3_ATOMICS)
#ܐܢ defined(__has_include)
#ܐܢ __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#ܐܚܪܢܐ
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#ܐܚܪܢܐ
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#ܐܢ BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic صحيح
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#وإلاإذا defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#ܐܚܪܢܐ
#define ATOMIC_INT صحيح
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (فراغ)((x))

#ܐܢ defined(IS_X86)
ساكن uint64_t xgetbv(فراغ) {
#ܐܢ defined(_MSC_VER)
  ܦܢܐ _xgetbv(0);
#ܐܚܪܢܐ
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  ܦܢܐ ((uint64_t)edx << 32) | eax;
#endif
}

ساكن فراغ cpuid(uint32_t out[4], uint32_t id) {
#ܐܢ defined(_MSC_VER)
  __cpuid((صحيح *)out, id);
#وإلاإذا defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#ܐܚܪܢܐ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

ساكن فراغ cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#ܐܢ defined(_MSC_VER)
  __cpuidex((صحيح *)out, id, sid);
#وإلاإذا defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#ܐܚܪܢܐ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#endif
}

#endif

تعداد cpu_feature {
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

#ܐܢ !defined(BLAKE3_TESTING)
ساكن /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#ܐܢ !defined(BLAKE3_TESTING)
ساكن
#endif
    تعداد cpu_feature
    get_cpu_features(فراغ) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  تعداد cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  ܐܢ (features != UNDEFINED) {
    ܦܢܐ features;
  } ܐܚܪܢܐ {
#ܐܢ defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (فراغ)edx;
    features = 0;
    cpuid(regs, 0);
    ثابت صحيح max_id = *eax;
    cpuid(regs, 1);
#ܐܢ defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#ܐܚܪܢܐ
    ܐܢ (*edx & (1UL << 26))
      features |= SSE2;
#endif
    ܐܢ (*ecx & (1UL << 9))
      features |= SSSE3;
    ܐܢ (*ecx & (1UL << 19))
      features |= SSE41;

    ܐܢ (*ecx & (1UL << 27)) { // OSXSAVE
      ثابت uint64_t mask = xgetbv();
      ܐܢ ((mask & 6) == 6) { // SSE and AVX states
        ܐܢ (*ecx & (1UL << 28))
          features |= AVX;
        ܐܢ (max_id >= 7) {
          cpuidex(regs, 7, 0);
          ܐܢ (*ebx & (1UL << 5))
            features |= AVX2;
          ܐܢ ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            ܐܢ (*ebx & (1UL << 31))
              features |= AVX512VL;
            ܐܢ (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    ܦܢܐ features;
#ܐܚܪܢܐ
    /* How to detect NEON? */
    ܦܢܐ 0;
#endif
  }
}

فراغ blake3_compress_in_place(uint32_t cv[8],
                              ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#ܐܢ defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ܐܢ !defined(BLAKE3_NO_AVX512)
  ܐܢ (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    ܦܢܐ;
  }
#endif
#ܐܢ !defined(BLAKE3_NO_SSE41)
  ܐܢ (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    ܦܢܐ;
  }
#endif
#ܐܢ !defined(BLAKE3_NO_SSE2)
  ܐܢ (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    ܦܢܐ;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

فراغ blake3_compress_xof(ثابت uint32_t cv[8],
                         ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#ܐܢ defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ܐܢ !defined(BLAKE3_NO_AVX512)
  ܐܢ (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    ܦܢܐ;
  }
#endif
#ܐܢ !defined(BLAKE3_NO_SSE41)
  ܐܢ (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    ܦܢܐ;
  }
#endif
#ܐܢ !defined(BLAKE3_NO_SSE2)
  ܐܢ (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    ܦܢܐ;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


فراغ blake3_xof_many(ثابت uint32_t cv[8],
                     ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  ܐܢ (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    ܦܢܐ;
  }
#ܐܢ defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ܐܢ !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  ܐܢ (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    ܦܢܐ;
  }
#endif
#endif
  لكل(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

فراغ blake3_hash_many(ثابت uint8_t *ثابت *inputs, size_t num_inputs,
                      size_t blocks, ثابت uint32_t key[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#ܐܢ defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ܐܢ !defined(BLAKE3_NO_AVX512)
  ܐܢ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    ܦܢܐ;
  }
#endif
#ܐܢ !defined(BLAKE3_NO_AVX2)
  ܐܢ (features & AVX2) {
    blake3_hash_many_avx2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ܦܢܐ;
  }
#endif
#ܐܢ !defined(BLAKE3_NO_SSE41)
  ܐܢ (features & SSE41) {
    blake3_hash_many_sse41(inputs, num_inputs, blocks, key, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    ܦܢܐ;
  }
#endif
#ܐܢ !defined(BLAKE3_NO_SSE2)
  ܐܢ (features & SSE2) {
    blake3_hash_many_sse2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ܦܢܐ;
  }
#endif
#endif

#ܐܢ BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, num_inputs, blocks, key, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  ܦܢܐ;
#endif

  blake3_hash_many_portable(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(فراغ) {
#ܐܢ defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ܐܢ !defined(BLAKE3_NO_AVX512)
  ܐܢ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    ܦܢܐ 16;
  }
#endif
#ܐܢ !defined(BLAKE3_NO_AVX2)
  ܐܢ (features & AVX2) {
    ܦܢܐ 8;
  }
#endif
#ܐܢ !defined(BLAKE3_NO_SSE41)
  ܐܢ (features & SSE41) {
    ܦܢܐ 4;
  }
#endif
#ܐܢ !defined(BLAKE3_NO_SSE2)
  ܐܢ (features & SSE2) {
    ܦܢܐ 4;
  }
#endif
#endif
#ܐܢ BLAKE3_USE_NEON == 1
  ܦܢܐ 4;
#endif
  ܦܢܐ 1;
}
