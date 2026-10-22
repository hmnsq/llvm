#شامل <stdbool.h>
#شامل <stddef.h>
#شامل <stdint.h>

#شامل "blake3_impl.h"

#اگر defined(_MSC_VER)
#شامل <Windows.h>
#اگر_ختم

#اگر defined(IS_X86)
#اگر defined(_MSC_VER)
#شامل <intrin.h>
#وگرنہ_اگر defined(__GNUC__)
#شامل <immintrin.h>
#ورنہ
#تعریف_منسوخ IS_X86 /* Unimplemented! */
#اگر_ختم
#اگر_ختم

#اگر !defined(BLAKE3_ATOMICS)
#اگر defined(__has_include)
#اگر __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#تعریف_کرو BLAKE3_ATOMICS 1
#ورنہ
#تعریف_کرو BLAKE3_ATOMICS 0
#اگر_ختم /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#ورنہ
#تعریف_کرو BLAKE3_ATOMICS 0
#اگر_ختم /* defined(__has_include) */
#اگر_ختم /* BLAKE3_ATOMICS */

#اگر BLAKE3_ATOMICS
#تعریف_کرو ATOMIC_INT _Atomic صحیح
#تعریف_کرو ATOMIC_بھرو(x) x
#تعریف_کرو ATOMIC_STORE(x, y) x = y
#وگرنہ_اگر defined(_MSC_VER)
#تعریف_کرو ATOMIC_INT LONG
#تعریف_کرو ATOMIC_بھرو(x) InterlockedOr(&x, 0)
#تعریف_کرو ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#ورنہ
#تعریف_کرو ATOMIC_INT صحیح
#تعریف_کرو ATOMIC_بھرو(x) x
#تعریف_کرو ATOMIC_STORE(x, y) x = y
#اگر_ختم

#تعریف_کرو MAYBE_غیر_مستعمل(x) (خالی)((x))

#اگر defined(IS_X86)
ساکن uint64_t xgetbv(خالی) {
#اگر defined(_MSC_VER)
  واپس _xgetbv(0);
#ورنہ
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  واپس ((uint64_t)edx << 32) | eax;
#اگر_ختم
}

ساکن خالی cpuid(uint32_t برآمد_2[4], uint32_t id) {
#اگر defined(_MSC_VER)
  __cpuid((صحیح *)برآمد_2, id);
#وگرنہ_اگر defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(برآمد_2[0]), "=r"(برآمد_2[1]), "=c"(برآمد_2[2]), "=d"(برآمد_2[3])
                       : "a"(id));
#ورنہ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(برآمد_2[0]), "=b"(برآمد_2[1]), "=c"(برآمد_2[2]), "=d"(برآمد_2[3])
                       : "a"(id));
#اگر_ختم
}

ساکن خالی cpuidex(uint32_t برآمد_2[4], uint32_t id, uint32_t sid) {
#اگر defined(_MSC_VER)
  __cpuidex((صحیح *)برآمد_2, id, sid);
#وگرنہ_اگر defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(برآمد_2[0]), "=r"(برآمد_2[1]), "=c"(برآمد_2[2]), "=d"(برآمد_2[3])
                       : "a"(id), "c"(sid));
#ورنہ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(برآمد_2[0]), "=b"(برآمد_2[1]), "=c"(برآمد_2[2]), "=d"(برآمد_2[3])
                       : "a"(id), "c"(sid));
#اگر_ختم
}

#اگر_ختم

شمار cpu_feature {
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
ساکن /* Allow the variable to be controlled manually for testing */
#اگر_ختم
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_مستعمل
#اگر !defined(BLAKE3_TESTING)
ساکن
#اگر_ختم
    شمار cpu_feature
    get_cpu_features(خالی) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  شمار cpu_feature features = ATOMIC_بھرو(g_cpu_features);
  اگر (features != UNDEFINED) {
    واپس features;
  } ورنہ {
#اگر defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (خالی)edx;
    features = 0;
    cpuid(regs, 0);
    ثابت صحیح max_id = *eax;
    cpuid(regs, 1);
#اگر defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#ورنہ
    اگر (*edx & (1UL << 26))
      features |= SSE2;
#اگر_ختم
    اگر (*ecx & (1UL << 9))
      features |= SSSE3;
    اگر (*ecx & (1UL << 19))
      features |= SSE41;

    اگر (*ecx & (1UL << 27)) { // OSXSAVE
      ثابت uint64_t نقاب_2 = xgetbv();
      اگر ((نقاب_2 & 6) == 6) { // SSE and AVX states
        اگر (*ecx & (1UL << 28))
          features |= AVX;
        اگر (max_id >= 7) {
          cpuidex(regs, 7, 0);
          اگر (*ebx & (1UL << 5))
            features |= AVX2;
          اگر ((نقاب_2 & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            اگر (*ebx & (1UL << 31))
              features |= AVX512VL;
            اگر (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    واپس features;
#ورنہ
    /* How to detect NEON? */
    واپس 0;
#اگر_ختم
  }
}

خالی blake3_compress_اندر_رکھو(uint32_t cv[8],
                              ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                              uint8_t بلاک_len, uint64_t شمار_3,
                              uint8_t flags) {
#اگر defined(IS_X86)
  ثابت شمار cpu_feature features = get_cpu_features();
  MAYBE_غیر_مستعمل(features);
#اگر !defined(BLAKE3_NO_AVX512)
  اگر (features & AVX512VL) {
    blake3_compress_اندر_رکھو_avx512(cv, بلاک, بلاک_len, شمار_3, flags);
    واپس;
  }
#اگر_ختم
#اگر !defined(BLAKE3_NO_SSE41)
  اگر (features & SSE41) {
    blake3_compress_اندر_رکھو_sse41(cv, بلاک, بلاک_len, شمار_3, flags);
    واپس;
  }
#اگر_ختم
#اگر !defined(BLAKE3_NO_SSE2)
  اگر (features & SSE2) {
    blake3_compress_اندر_رکھو_sse2(cv, بلاک, بلاک_len, شمار_3, flags);
    واپس;
  }
#اگر_ختم
#اگر_ختم
  blake3_compress_اندر_رکھو_portable(cv, بلاک, بلاک_len, شمار_3, flags);
}

خالی blake3_compress_xof(ثابت uint32_t cv[8],
                         ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                         uint8_t بلاک_len, uint64_t شمار_3, uint8_t flags,
                         uint8_t برآمد_2[64]) {
#اگر defined(IS_X86)
  ثابت شمار cpu_feature features = get_cpu_features();
  MAYBE_غیر_مستعمل(features);
#اگر !defined(BLAKE3_NO_AVX512)
  اگر (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, بلاک, بلاک_len, شمار_3, flags, برآمد_2);
    واپس;
  }
#اگر_ختم
#اگر !defined(BLAKE3_NO_SSE41)
  اگر (features & SSE41) {
    blake3_compress_xof_sse41(cv, بلاک, بلاک_len, شمار_3, flags, برآمد_2);
    واپس;
  }
#اگر_ختم
#اگر !defined(BLAKE3_NO_SSE2)
  اگر (features & SSE2) {
    blake3_compress_xof_sse2(cv, بلاک, بلاک_len, شمار_3, flags, برآمد_2);
    واپس;
  }
#اگر_ختم
#اگر_ختم
  blake3_compress_xof_portable(cv, بلاک, بلاک_len, شمار_3, flags, برآمد_2);
}


خالی blake3_xof_many(ثابت uint32_t cv[8],
                     ثابت uint8_t بلاک[BLAKE3_بلاک_LEN],
                     uint8_t بلاک_len, uint64_t شمار_3, uint8_t flags,
                     uint8_t برآمد_2[64], size_t outblocks) {
  اگر (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    واپس;
  }
#اگر defined(IS_X86)
  ثابت شمار cpu_feature features = get_cpu_features();
  MAYBE_غیر_مستعمل(features);
#اگر !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  اگر (features & AVX512VL) {
    blake3_xof_many_avx512(cv, بلاک, بلاک_len, شمار_3, flags, برآمد_2, outblocks);
    واپس;
  }
#اگر_ختم
#اگر_ختم
  برائے(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, بلاک, بلاک_len, شمار_3 + i, flags, برآمد_2 + 64*i);
  }
}

خالی blake3_hash_many(ثابت uint8_t *ثابت *inputs, size_t عدد_inputs,
                      size_t بلاکیں, ثابت uint32_t کلید_2[8], uint64_t شمار_3,
                      bool increment_شمار, uint8_t flags,
                      uint8_t flags_آغاز, uint8_t flags_انتہا, uint8_t *برآمد_2) {
#اگر defined(IS_X86)
  ثابت شمار cpu_feature features = get_cpu_features();
  MAYBE_غیر_مستعمل(features);
#اگر !defined(BLAKE3_NO_AVX512)
  اگر ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, عدد_inputs, بلاکیں, کلید_2, شمار_3,
                            increment_شمار, flags, flags_آغاز, flags_انتہا,
                            برآمد_2);
    واپس;
  }
#اگر_ختم
#اگر !defined(BLAKE3_NO_AVX2)
  اگر (features & AVX2) {
    blake3_hash_many_avx2(inputs, عدد_inputs, بلاکیں, کلید_2, شمار_3,
                          increment_شمار, flags, flags_آغاز, flags_انتہا,
                          برآمد_2);
    واپس;
  }
#اگر_ختم
#اگر !defined(BLAKE3_NO_SSE41)
  اگر (features & SSE41) {
    blake3_hash_many_sse41(inputs, عدد_inputs, بلاکیں, کلید_2, شمار_3,
                           increment_شمار, flags, flags_آغاز, flags_انتہا,
                           برآمد_2);
    واپس;
  }
#اگر_ختم
#اگر !defined(BLAKE3_NO_SSE2)
  اگر (features & SSE2) {
    blake3_hash_many_sse2(inputs, عدد_inputs, بلاکیں, کلید_2, شمار_3,
                          increment_شمار, flags, flags_آغاز, flags_انتہا,
                          برآمد_2);
    واپس;
  }
#اگر_ختم
#اگر_ختم

#اگر BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, عدد_inputs, بلاکیں, کلید_2, شمار_3,
                        increment_شمار, flags, flags_آغاز, flags_انتہا, برآمد_2);
  واپس;
#اگر_ختم

  blake3_hash_many_portable(inputs, عدد_inputs, بلاکیں, کلید_2, شمار_3,
                            increment_شمار, flags, flags_آغاز, flags_انتہا,
                            برآمد_2);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(خالی) {
#اگر defined(IS_X86)
  ثابت شمار cpu_feature features = get_cpu_features();
  MAYBE_غیر_مستعمل(features);
#اگر !defined(BLAKE3_NO_AVX512)
  اگر ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    واپس 16;
  }
#اگر_ختم
#اگر !defined(BLAKE3_NO_AVX2)
  اگر (features & AVX2) {
    واپس 8;
  }
#اگر_ختم
#اگر !defined(BLAKE3_NO_SSE41)
  اگر (features & SSE41) {
    واپس 4;
  }
#اگر_ختم
#اگر !defined(BLAKE3_NO_SSE2)
  اگر (features & SSE2) {
    واپس 4;
  }
#اگر_ختم
#اگر_ختم
#اگر BLAKE3_USE_NEON == 1
  واپس 4;
#اگر_ختم
  واپس 1;
}
