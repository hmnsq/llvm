#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdint.h>

#ضمن "blake3_impl.h"

#إذا defined(_MSC_VER)
#ضمن <Windows.h>
#جيڪڏهن_پورو

#إذا defined(IS_X86)
#إذا defined(_MSC_VER)
#ضمن <intrin.h>
#وإلاإذا defined(__GNUC__)
#ضمن <immintrin.h>
#وإلا
#وصف_رد IS_X86 /* Unimplemented! */
#جيڪڏهن_پورو
#جيڪڏهن_پورو

#إذا !defined(BLAKE3_ATOMICS)
#إذا defined(__has_include)
#إذا __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#وصف_ڪريو BLAKE3_ATOMICS 1
#وإلا
#وصف_ڪريو BLAKE3_ATOMICS 0
#جيڪڏهن_پورو /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#وإلا
#وصف_ڪريو BLAKE3_ATOMICS 0
#جيڪڏهن_پورو /* defined(__has_include) */
#جيڪڏهن_پورو /* BLAKE3_ATOMICS */

#إذا BLAKE3_ATOMICS
#وصف_ڪريو ATOMIC_INT _Atomic صحيح
#وصف_ڪريو ATOMIC_ڀريو(x) x
#وصف_ڪريو ATOMIC_STORE(x, y) x = y
#وإلاإذا defined(_MSC_VER)
#وصف_ڪريو ATOMIC_INT LONG
#وصف_ڪريو ATOMIC_ڀريو(x) InterlockedOr(&x, 0)
#وصف_ڪريو ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#وإلا
#وصف_ڪريو ATOMIC_INT صحيح
#وصف_ڪريو ATOMIC_ڀريو(x) x
#وصف_ڪريو ATOMIC_STORE(x, y) x = y
#جيڪڏهن_پورو

#وصف_ڪريو MAYBE_اڻ_استعمال_ٿيل(x) (فراغ)((x))

#إذا defined(IS_X86)
ساكن uint64_t xgetbv(فراغ) {
#إذا defined(_MSC_VER)
  أرجع _xgetbv(0);
#وإلا
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  أرجع ((uint64_t)edx << 32) | eax;
#جيڪڏهن_پورو
}

ساكن فراغ cpuid(uint32_t ٻاهريون_2[4], uint32_t id) {
#إذا defined(_MSC_VER)
  __cpuid((صحيح *)ٻاهريون_2, id);
#وإلاإذا defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(ٻاهريون_2[0]), "=r"(ٻاهريون_2[1]), "=c"(ٻاهريون_2[2]), "=d"(ٻاهريون_2[3])
                       : "a"(id));
#وإلا
  __asm__ __volatile__("cpuid\n"
                       : "=a"(ٻاهريون_2[0]), "=b"(ٻاهريون_2[1]), "=c"(ٻاهريون_2[2]), "=d"(ٻاهريون_2[3])
                       : "a"(id));
#جيڪڏهن_پورو
}

ساكن فراغ cpuidex(uint32_t ٻاهريون_2[4], uint32_t id, uint32_t sid) {
#إذا defined(_MSC_VER)
  __cpuidex((صحيح *)ٻاهريون_2, id, sid);
#وإلاإذا defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(ٻاهريون_2[0]), "=r"(ٻاهريون_2[1]), "=c"(ٻاهريون_2[2]), "=d"(ٻاهريون_2[3])
                       : "a"(id), "c"(sid));
#وإلا
  __asm__ __volatile__("cpuid\n"
                       : "=a"(ٻاهريون_2[0]), "=b"(ٻاهريون_2[1]), "=c"(ٻاهريون_2[2]), "=d"(ٻاهريون_2[3])
                       : "a"(id), "c"(sid));
#جيڪڏهن_پورو
}

#جيڪڏهن_پورو

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
#جيڪڏهن_پورو
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_استعمال_ٿيل
#إذا !defined(BLAKE3_TESTING)
ساكن
#جيڪڏهن_پورو
    تعداد cpu_feature
    get_cpu_features(فراغ) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  تعداد cpu_feature features = ATOMIC_ڀريو(g_cpu_features);
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
#جيڪڏهن_پورو
    إذا (*ecx & (1UL << 9))
      features |= SSSE3;
    إذا (*ecx & (1UL << 19))
      features |= SSE41;

    إذا (*ecx & (1UL << 27)) { // OSXSAVE
      ثابت uint64_t نقاب_2 = xgetbv();
      إذا ((نقاب_2 & 6) == 6) { // SSE and AVX states
        إذا (*ecx & (1UL << 28))
          features |= AVX;
        إذا (max_id >= 7) {
          cpuidex(regs, 7, 0);
          إذا (*ebx & (1UL << 5))
            features |= AVX2;
          إذا ((نقاب_2 & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
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
#جيڪڏهن_پورو
  }
}

فراغ blake3_compress_اندريون_رکو(uint32_t cv[8],
                              ثابت uint8_t بلاڪ[BLAKE3_بلاڪ_LEN],
                              uint8_t بلاڪ_len, uint64_t counter,
                              uint8_t flags) {
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_اڻ_استعمال_ٿيل(features);
#إذا !defined(BLAKE3_NO_AVX512)
  إذا (features & AVX512VL) {
    blake3_compress_اندريون_رکو_avx512(cv, بلاڪ, بلاڪ_len, counter, flags);
    أرجع;
  }
#جيڪڏهن_پورو
#إذا !defined(BLAKE3_NO_SSE41)
  إذا (features & SSE41) {
    blake3_compress_اندريون_رکو_sse41(cv, بلاڪ, بلاڪ_len, counter, flags);
    أرجع;
  }
#جيڪڏهن_پورو
#إذا !defined(BLAKE3_NO_SSE2)
  إذا (features & SSE2) {
    blake3_compress_اندريون_رکو_sse2(cv, بلاڪ, بلاڪ_len, counter, flags);
    أرجع;
  }
#جيڪڏهن_پورو
#جيڪڏهن_پورو
  blake3_compress_اندريون_رکو_portable(cv, بلاڪ, بلاڪ_len, counter, flags);
}

فراغ blake3_compress_xof(ثابت uint32_t cv[8],
                         ثابت uint8_t بلاڪ[BLAKE3_بلاڪ_LEN],
                         uint8_t بلاڪ_len, uint64_t counter, uint8_t flags,
                         uint8_t ٻاهريون_2[64]) {
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_اڻ_استعمال_ٿيل(features);
#إذا !defined(BLAKE3_NO_AVX512)
  إذا (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, بلاڪ, بلاڪ_len, counter, flags, ٻاهريون_2);
    أرجع;
  }
#جيڪڏهن_پورو
#إذا !defined(BLAKE3_NO_SSE41)
  إذا (features & SSE41) {
    blake3_compress_xof_sse41(cv, بلاڪ, بلاڪ_len, counter, flags, ٻاهريون_2);
    أرجع;
  }
#جيڪڏهن_پورو
#إذا !defined(BLAKE3_NO_SSE2)
  إذا (features & SSE2) {
    blake3_compress_xof_sse2(cv, بلاڪ, بلاڪ_len, counter, flags, ٻاهريون_2);
    أرجع;
  }
#جيڪڏهن_پورو
#جيڪڏهن_پورو
  blake3_compress_xof_portable(cv, بلاڪ, بلاڪ_len, counter, flags, ٻاهريون_2);
}


فراغ blake3_xof_many(ثابت uint32_t cv[8],
                     ثابت uint8_t بلاڪ[BLAKE3_بلاڪ_LEN],
                     uint8_t بلاڪ_len, uint64_t counter, uint8_t flags,
                     uint8_t ٻاهريون_2[64], size_t outblocks) {
  إذا (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    أرجع;
  }
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_اڻ_استعمال_ٿيل(features);
#إذا !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  إذا (features & AVX512VL) {
    blake3_xof_many_avx512(cv, بلاڪ, بلاڪ_len, counter, flags, ٻاهريون_2, outblocks);
    أرجع;
  }
#جيڪڏهن_پورو
#جيڪڏهن_پورو
  لكل(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, بلاڪ, بلاڪ_len, counter + i, flags, ٻاهريون_2 + 64*i);
  }
}

فراغ blake3_hash_many(ثابت uint8_t *ثابت *inputs, size_t انگ_inputs,
                      size_t بلاڪون, ثابت uint32_t ڪنجي_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_شروعات, uint8_t flags_پڇاڙي, uint8_t *ٻاهريون_2) {
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_اڻ_استعمال_ٿيل(features);
#إذا !defined(BLAKE3_NO_AVX512)
  إذا ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, انگ_inputs, بلاڪون, ڪنجي_2, counter,
                            increment_counter, flags, flags_شروعات, flags_پڇاڙي,
                            ٻاهريون_2);
    أرجع;
  }
#جيڪڏهن_پورو
#إذا !defined(BLAKE3_NO_AVX2)
  إذا (features & AVX2) {
    blake3_hash_many_avx2(inputs, انگ_inputs, بلاڪون, ڪنجي_2, counter,
                          increment_counter, flags, flags_شروعات, flags_پڇاڙي,
                          ٻاهريون_2);
    أرجع;
  }
#جيڪڏهن_پورو
#إذا !defined(BLAKE3_NO_SSE41)
  إذا (features & SSE41) {
    blake3_hash_many_sse41(inputs, انگ_inputs, بلاڪون, ڪنجي_2, counter,
                           increment_counter, flags, flags_شروعات, flags_پڇاڙي,
                           ٻاهريون_2);
    أرجع;
  }
#جيڪڏهن_پورو
#إذا !defined(BLAKE3_NO_SSE2)
  إذا (features & SSE2) {
    blake3_hash_many_sse2(inputs, انگ_inputs, بلاڪون, ڪنجي_2, counter,
                          increment_counter, flags, flags_شروعات, flags_پڇاڙي,
                          ٻاهريون_2);
    أرجع;
  }
#جيڪڏهن_پورو
#جيڪڏهن_پورو

#إذا BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, انگ_inputs, بلاڪون, ڪنجي_2, counter,
                        increment_counter, flags, flags_شروعات, flags_پڇاڙي, ٻاهريون_2);
  أرجع;
#جيڪڏهن_پورو

  blake3_hash_many_portable(inputs, انگ_inputs, بلاڪون, ڪنجي_2, counter,
                            increment_counter, flags, flags_شروعات, flags_پڇاڙي,
                            ٻاهريون_2);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(فراغ) {
#إذا defined(IS_X86)
  ثابت تعداد cpu_feature features = get_cpu_features();
  MAYBE_اڻ_استعمال_ٿيل(features);
#إذا !defined(BLAKE3_NO_AVX512)
  إذا ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    أرجع 16;
  }
#جيڪڏهن_پورو
#إذا !defined(BLAKE3_NO_AVX2)
  إذا (features & AVX2) {
    أرجع 8;
  }
#جيڪڏهن_پورو
#إذا !defined(BLAKE3_NO_SSE41)
  إذا (features & SSE41) {
    أرجع 4;
  }
#جيڪڏهن_پورو
#إذا !defined(BLAKE3_NO_SSE2)
  إذا (features & SSE2) {
    أرجع 4;
  }
#جيڪڏهن_پورو
#جيڪڏهن_پورو
#إذا BLAKE3_USE_NEON == 1
  أرجع 4;
#جيڪڏهن_پورو
  أرجع 1;
}
