#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdint.h>

#ضمن "blake3_impl.h"

#إذا defined(_MSC_VER)
#ضمن <Windows.h>
#endif

#إذا defined(IS_X86)
#إذا defined(_MSC_VER)
#ضمن <intrin.h>
#وإلاإذا defined(__GNUC__)
#ضمن <immintrin.h>
#وإلا
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#إذا !defined(BLAKE3_ATOMICS)
#إذا defined(__has_include)
#إذا __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#وإلا
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#وإلا
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#إذا BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic صحيح
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#وإلاإذا defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#وإلا
#define ATOMIC_INT صحيح
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (فراغ)((x))

#إذا defined(IS_X86)
ساكن uint64_t xgetbv(فراغ) {
#إذا defined(_MSC_VER)
  أرجع _xgetbv(0);
#وإلا
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  أرجع ((uint64_t)edx << 32) | eax;
#endif
}

ساكن فراغ cpuid(uint32_t out[4], uint32_t id) {
#إذا defined(_MSC_VER)
  __cpuid((صحيح *)out, id);
#وإلاإذا defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#وإلا
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

ساكن فراغ cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#إذا defined(_MSC_VER)
  __cpuidex((صحيح *)out, id, sid);
#وإلاإذا defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#وإلا
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

#إذا !defined(BLAKE3_TESTING)
ساكن /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#إذا !defined(BLAKE3_TESTING)
ساكن
#endif
    تعداد cpu_feature
    get_cpu_features(فراغ) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  تعداد cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  إذا (features != UNDEFINED) {
    أرجع features;
  } وإلا {
#إذا defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (فراغ)edx;
    features = 0;
    cpuid(regs, 0);
    ثابت صحيح max_id = *eax;
    cpuid(regs, 1);
#إذا defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#وإلا
    إذا (*edx & (1UL << 26))
      features |= SSE2;
#endif
    إذا (*ecx & (1UL << 9))
      features |= SSSE3;
    إذا (*ecx & (1UL << 19))
      features |= SSE41;

    إذا (*ecx & (1UL << 27)) { // OSXSAVE
      ثابت uint64_t mask = xgetbv();
      إذا ((mask & 6) == 6) { // SSE and AVX states
        إذا (*ecx & (1UL << 28))
          features |= AVX;
        إذا (max_id >= 7) {
          cpuidex(regs, 7, 0);
          إذا (*ebx & (1UL << 5))
            features |= AVX2;
          إذا ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            إذا (*ebx & (1UL << 31))
              features |= AVX512VL;
            إذا (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    أرجع features;
#وإلا
    /* How to detect NEON? */
    أرجع 0;
#endif
  }
}

فراغ blake3_compress_in_place(uint32_t cv[8],
                              ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#إذا !defined(BLAKE3_NO_AVX512)
  إذا (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    أرجع;
  }
#endif
#إذا !defined(BLAKE3_NO_SSE41)
  إذا (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    أرجع;
  }
#endif
#إذا !defined(BLAKE3_NO_SSE2)
  إذا (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    أرجع;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

فراغ blake3_compress_xof(ثابت uint32_t cv[8],
                         ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#إذا !defined(BLAKE3_NO_AVX512)
  إذا (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    أرجع;
  }
#endif
#إذا !defined(BLAKE3_NO_SSE41)
  إذا (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    أرجع;
  }
#endif
#إذا !defined(BLAKE3_NO_SSE2)
  إذا (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    أرجع;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


فراغ blake3_xof_many(ثابت uint32_t cv[8],
                     ثابت uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  إذا (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    أرجع;
  }
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#إذا !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  إذا (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    أرجع;
  }
#endif
#endif
  لكل(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

فراغ blake3_hash_many(ثابت uint8_t *ثابت *inputs, size_t اڠک_inputs,
                      size_t blocks, ثابت uint32_t کونچي_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#إذا !defined(BLAKE3_NO_AVX512)
  إذا ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, اڠک_inputs, blocks, کونچي_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    أرجع;
  }
#endif
#إذا !defined(BLAKE3_NO_AVX2)
  إذا (features & AVX2) {
    blake3_hash_many_avx2(inputs, اڠک_inputs, blocks, کونچي_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    أرجع;
  }
#endif
#إذا !defined(BLAKE3_NO_SSE41)
  إذا (features & SSE41) {
    blake3_hash_many_sse41(inputs, اڠک_inputs, blocks, کونچي_2, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    أرجع;
  }
#endif
#إذا !defined(BLAKE3_NO_SSE2)
  إذا (features & SSE2) {
    blake3_hash_many_sse2(inputs, اڠک_inputs, blocks, کونچي_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    أرجع;
  }
#endif
#endif

#إذا BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, اڠک_inputs, blocks, کونچي_2, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  أرجع;
#endif

  blake3_hash_many_portable(inputs, اڠک_inputs, blocks, کونچي_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(فراغ) {
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#إذا !defined(BLAKE3_NO_AVX512)
  إذا ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    أرجع 16;
  }
#endif
#إذا !defined(BLAKE3_NO_AVX2)
  إذا (features & AVX2) {
    أرجع 8;
  }
#endif
#إذا !defined(BLAKE3_NO_SSE41)
  إذا (features & SSE41) {
    أرجع 4;
  }
#endif
#إذا !defined(BLAKE3_NO_SSE2)
  إذا (features & SSE2) {
    أرجع 4;
  }
#endif
#endif
#إذا BLAKE3_USE_NEON == 1
  أرجع 4;
#endif
  أرجع 1;
}
