#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdint.h>

#ضمن "blake3_impl.h"

#إذا defined(_MSC_VER)
#ضمن <Windows.h>
#پایان_اگر

#إذا defined(IS_X86)
#إذا defined(_MSC_VER)
#ضمن <intrin.h>
#وإلاإذا defined(__GNUC__)
#ضمن <immintrin.h>
#وإلا
#لغو_تعریف IS_X86 /* Unimplemented! */
#پایان_اگر
#پایان_اگر

#إذا !defined(BLAKE3_ATOMICS)
#إذا defined(__has_include)
#إذا __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#تعریف_کن BLAKE3_ATOMICS 1
#وإلا
#تعریف_کن BLAKE3_ATOMICS 0
#پایان_اگر /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#وإلا
#تعریف_کن BLAKE3_ATOMICS 0
#پایان_اگر /* defined(__has_include) */
#پایان_اگر /* BLAKE3_ATOMICS */

#إذا BLAKE3_ATOMICS
#تعریف_کن ATOMIC_INT _Atomic صحيح
#تعریف_کن ATOMIC_بارگیری(x) x
#تعریف_کن ATOMIC_STORE(x, y) x = y
#وإلاإذا defined(_MSC_VER)
#تعریف_کن ATOMIC_INT LONG
#تعریف_کن ATOMIC_بارگیری(x) InterlockedOr(&x, 0)
#تعریف_کن ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#وإلا
#تعریف_کن ATOMIC_INT صحيح
#تعریف_کن ATOMIC_بارگیری(x) x
#تعریف_کن ATOMIC_STORE(x, y) x = y
#پایان_اگر

#تعریف_کن MAYBE_استفادهنشده(x) (فراغ)((x))

#إذا defined(IS_X86)
ساكن uint64_t xgetbv(فراغ) {
#إذا defined(_MSC_VER)
  أرجع _xgetbv(0);
#وإلا
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  أرجع ((uint64_t)edx << 32) | eax;
#پایان_اگر
}

ساكن فراغ cpuid(uint32_t بیرون_2[4], uint32_t id) {
#إذا defined(_MSC_VER)
  __cpuid((صحيح *)بیرون_2, id);
#وإلاإذا defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(بیرون_2[0]), "=r"(بیرون_2[1]), "=c"(بیرون_2[2]), "=d"(بیرون_2[3])
                       : "a"(id));
#وإلا
  __asm__ __volatile__("cpuid\n"
                       : "=a"(بیرون_2[0]), "=b"(بیرون_2[1]), "=c"(بیرون_2[2]), "=d"(بیرون_2[3])
                       : "a"(id));
#پایان_اگر
}

ساكن فراغ cpuidex(uint32_t بیرون_2[4], uint32_t id, uint32_t sid) {
#إذا defined(_MSC_VER)
  __cpuidex((صحيح *)بیرون_2, id, sid);
#وإلاإذا defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(بیرون_2[0]), "=r"(بیرون_2[1]), "=c"(بیرون_2[2]), "=d"(بیرون_2[3])
                       : "a"(id), "c"(sid));
#وإلا
  __asm__ __volatile__("cpuid\n"
                       : "=a"(بیرون_2[0]), "=b"(بیرون_2[1]), "=c"(بیرون_2[2]), "=d"(بیرون_2[3])
                       : "a"(id), "c"(sid));
#پایان_اگر
}

#پایان_اگر

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
#پایان_اگر
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_استفاده_شده
#إذا !defined(BLAKE3_TESTING)
ساكن
#پایان_اگر
    تعداد cpu_feature
    get_cpu_features(فراغ) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  تعداد cpu_feature features = ATOMIC_بارگیری(g_cpu_features);
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
#پایان_اگر
    إذا (*ecx & (1UL << 9))
      features |= SSSE3;
    إذا (*ecx & (1UL << 19))
      features |= SSE41;

    إذا (*ecx & (1UL << 27)) { // OSXSAVE
      ثابت uint64_t ماسک_2 = xgetbv();
      إذا ((ماسک_2 & 6) == 6) { // SSE and AVX states
        إذا (*ecx & (1UL << 28))
          features |= AVX;
        إذا (max_id >= 7) {
          cpuidex(regs, 7, 0);
          إذا (*ebx & (1UL << 5))
            features |= AVX2;
          إذا ((ماسک_2 & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
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
#پایان_اگر
  }
}

فراغ blake3_compress_درون_قراربده(uint32_t cv[8],
                              ثابت uint8_t بلوک[BLAKE3_بلوک_LEN],
                              uint8_t بلوک_len, uint64_t counter,
                              uint8_t flags) {
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_استفادهنشده(features);
#إذا !defined(BLAKE3_NO_AVX512)
  إذا (features & AVX512VL) {
    blake3_compress_درون_قراربده_avx512(cv, بلوک, بلوک_len, counter, flags);
    أرجع;
  }
#پایان_اگر
#إذا !defined(BLAKE3_NO_SSE41)
  إذا (features & SSE41) {
    blake3_compress_درون_قراربده_sse41(cv, بلوک, بلوک_len, counter, flags);
    أرجع;
  }
#پایان_اگر
#إذا !defined(BLAKE3_NO_SSE2)
  إذا (features & SSE2) {
    blake3_compress_درون_قراربده_sse2(cv, بلوک, بلوک_len, counter, flags);
    أرجع;
  }
#پایان_اگر
#پایان_اگر
  blake3_compress_درون_قراربده_portable(cv, بلوک, بلوک_len, counter, flags);
}

فراغ blake3_compress_xof(ثابت uint32_t cv[8],
                         ثابت uint8_t بلوک[BLAKE3_بلوک_LEN],
                         uint8_t بلوک_len, uint64_t counter, uint8_t flags,
                         uint8_t بیرون_2[64]) {
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_استفادهنشده(features);
#إذا !defined(BLAKE3_NO_AVX512)
  إذا (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, بلوک, بلوک_len, counter, flags, بیرون_2);
    أرجع;
  }
#پایان_اگر
#إذا !defined(BLAKE3_NO_SSE41)
  إذا (features & SSE41) {
    blake3_compress_xof_sse41(cv, بلوک, بلوک_len, counter, flags, بیرون_2);
    أرجع;
  }
#پایان_اگر
#إذا !defined(BLAKE3_NO_SSE2)
  إذا (features & SSE2) {
    blake3_compress_xof_sse2(cv, بلوک, بلوک_len, counter, flags, بیرون_2);
    أرجع;
  }
#پایان_اگر
#پایان_اگر
  blake3_compress_xof_portable(cv, بلوک, بلوک_len, counter, flags, بیرون_2);
}


فراغ blake3_xof_many(ثابت uint32_t cv[8],
                     ثابت uint8_t بلوک[BLAKE3_بلوک_LEN],
                     uint8_t بلوک_len, uint64_t counter, uint8_t flags,
                     uint8_t بیرون_2[64], size_t outblocks) {
  إذا (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    أرجع;
  }
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_استفادهنشده(features);
#إذا !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  إذا (features & AVX512VL) {
    blake3_xof_many_avx512(cv, بلوک, بلوک_len, counter, flags, بیرون_2, outblocks);
    أرجع;
  }
#پایان_اگر
#پایان_اگر
  لكل(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, بلوک, بلوک_len, counter + i, flags, بیرون_2 + 64*i);
  }
}

فراغ blake3_hash_many(ثابت uint8_t *ثابت *inputs, size_t رقم_inputs,
                      size_t بلوکها, ثابت uint32_t کلید_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_شروع, uint8_t flags_انتها, uint8_t *بیرون_2) {
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_استفادهنشده(features);
#إذا !defined(BLAKE3_NO_AVX512)
  إذا ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, رقم_inputs, بلوکها, کلید_2, counter,
                            increment_counter, flags, flags_شروع, flags_انتها,
                            بیرون_2);
    أرجع;
  }
#پایان_اگر
#إذا !defined(BLAKE3_NO_AVX2)
  إذا (features & AVX2) {
    blake3_hash_many_avx2(inputs, رقم_inputs, بلوکها, کلید_2, counter,
                          increment_counter, flags, flags_شروع, flags_انتها,
                          بیرون_2);
    أرجع;
  }
#پایان_اگر
#إذا !defined(BLAKE3_NO_SSE41)
  إذا (features & SSE41) {
    blake3_hash_many_sse41(inputs, رقم_inputs, بلوکها, کلید_2, counter,
                           increment_counter, flags, flags_شروع, flags_انتها,
                           بیرون_2);
    أرجع;
  }
#پایان_اگر
#إذا !defined(BLAKE3_NO_SSE2)
  إذا (features & SSE2) {
    blake3_hash_many_sse2(inputs, رقم_inputs, بلوکها, کلید_2, counter,
                          increment_counter, flags, flags_شروع, flags_انتها,
                          بیرون_2);
    أرجع;
  }
#پایان_اگر
#پایان_اگر

#إذا BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, رقم_inputs, بلوکها, کلید_2, counter,
                        increment_counter, flags, flags_شروع, flags_انتها, بیرون_2);
  أرجع;
#پایان_اگر

  blake3_hash_many_portable(inputs, رقم_inputs, بلوکها, کلید_2, counter,
                            increment_counter, flags, flags_شروع, flags_انتها,
                            بیرون_2);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(فراغ) {
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_استفادهنشده(features);
#إذا !defined(BLAKE3_NO_AVX512)
  إذا ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    أرجع 16;
  }
#پایان_اگر
#إذا !defined(BLAKE3_NO_AVX2)
  إذا (features & AVX2) {
    أرجع 8;
  }
#پایان_اگر
#إذا !defined(BLAKE3_NO_SSE41)
  إذا (features & SSE41) {
    أرجع 4;
  }
#پایان_اگر
#إذا !defined(BLAKE3_NO_SSE2)
  إذا (features & SSE2) {
    أرجع 4;
  }
#پایان_اگر
#پایان_اگر
#إذا BLAKE3_USE_NEON == 1
  أرجع 4;
#پایان_اگر
  أرجع 1;
}
