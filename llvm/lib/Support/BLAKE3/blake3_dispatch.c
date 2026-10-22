#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "blake3_impl.h"

#ਜੇ defined(_MSC_VER)
#ਅਨ੍ਤਰ੍ਭਾਵ <Windows.h>
#ਜੇ_ਅੰਤ

#ਜੇ defined(IS_X86)
#ਜੇ defined(_MSC_VER)
#ਅਨ੍ਤਰ੍ਭਾਵ <intrin.h>
#ਜਾਂਜੇ defined(__GNUC__)
#ਅਨ੍ਤਰ੍ਭਾਵ <immintrin.h>
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ IS_X86 /* Unimplemented! */
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ !defined(BLAKE3_ATOMICS)
#ਜੇ defined(__has_include)
#ਜੇ __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BLAKE3_ATOMICS 1
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BLAKE3_ATOMICS 0
#ਜੇ_ਅੰਤ /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BLAKE3_ATOMICS 0
#ਜੇ_ਅੰਤ /* defined(__has_include) */
#ਜੇ_ਅੰਤ /* BLAKE3_ATOMICS */

#ਜੇ BLAKE3_ATOMICS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATOMIC_INT _Atomic ਪੂਰਨ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATOMIC_ਲੱਦੋ(x) x
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATOMIC_STORE(x, y) x = y
#ਜਾਂਜੇ defined(_MSC_VER)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATOMIC_INT LONG
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATOMIC_ਲੱਦੋ(x) InterlockedOr(&x, 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATOMIC_INT ਪੂਰਨ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATOMIC_ਲੱਦੋ(x) x
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATOMIC_STORE(x, y) x = y
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAYBE_ਅਣਵਰਤਿਆ(x) (ਖਾਲੀ)((x))

#ਜੇ defined(IS_X86)
ਸਥਿਰ uint64_t xgetbv(ਖਾਲੀ) {
#ਜੇ defined(_MSC_VER)
  ਵਾਪਸ _xgetbv(0);
#ਨਹੀਂਤਾਂ
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  ਵਾਪਸ ((uint64_t)edx << 32) | eax;
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਖਾਲੀ cpuid(uint32_t ਬਾਹਰਲਾ_2[4], uint32_t id) {
#ਜੇ defined(_MSC_VER)
  __cpuid((ਪੂਰਨ *)ਬਾਹਰਲਾ_2, id);
#ਜਾਂਜੇ defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(ਬਾਹਰਲਾ_2[0]), "=r"(ਬਾਹਰਲਾ_2[1]), "=c"(ਬਾਹਰਲਾ_2[2]), "=d"(ਬਾਹਰਲਾ_2[3])
                       : "a"(id));
#ਨਹੀਂਤਾਂ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(ਬਾਹਰਲਾ_2[0]), "=b"(ਬਾਹਰਲਾ_2[1]), "=c"(ਬਾਹਰਲਾ_2[2]), "=d"(ਬਾਹਰਲਾ_2[3])
                       : "a"(id));
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਖਾਲੀ cpuidex(uint32_t ਬਾਹਰਲਾ_2[4], uint32_t id, uint32_t sid) {
#ਜੇ defined(_MSC_VER)
  __cpuidex((ਪੂਰਨ *)ਬਾਹਰਲਾ_2, id, sid);
#ਜਾਂਜੇ defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(ਬਾਹਰਲਾ_2[0]), "=r"(ਬਾਹਰਲਾ_2[1]), "=c"(ਬਾਹਰਲਾ_2[2]), "=d"(ਬਾਹਰਲਾ_2[3])
                       : "a"(id), "c"(sid));
#ਨਹੀਂਤਾਂ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(ਬਾਹਰਲਾ_2[0]), "=b"(ਬਾਹਰਲਾ_2[1]), "=c"(ਬਾਹਰਲਾ_2[2]), "=d"(ਬਾਹਰਲਾ_2[3])
                       : "a"(id), "c"(sid));
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਅੰਤ

ਗਿਣਤੀ cpu_feature {
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

#ਜੇ !defined(BLAKE3_TESTING)
ਸਥਿਰ /* Allow the variable to be controlled manually for testing */
#ਜੇ_ਅੰਤ
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_ਵਰਤਿਆ
#ਜੇ !defined(BLAKE3_TESTING)
ਸਥਿਰ
#ਜੇ_ਅੰਤ
    ਗਿਣਤੀ cpu_feature
    get_cpu_features(ਖਾਲੀ) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  ਗਿਣਤੀ cpu_feature features = ATOMIC_ਲੱਦੋ(g_cpu_features);
  ਜੇ (features != UNDEFINED) {
    ਵਾਪਸ features;
  } ਨਹੀਂਤਾਂ {
#ਜੇ defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (ਖਾਲੀ)edx;
    features = 0;
    cpuid(regs, 0);
    ਅਚਲ ਪੂਰਨ max_id = *eax;
    cpuid(regs, 1);
#ਜੇ defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#ਨਹੀਂਤਾਂ
    ਜੇ (*edx & (1UL << 26))
      features |= SSE2;
#ਜੇ_ਅੰਤ
    ਜੇ (*ecx & (1UL << 9))
      features |= SSSE3;
    ਜੇ (*ecx & (1UL << 19))
      features |= SSE41;

    ਜੇ (*ecx & (1UL << 27)) { // OSXSAVE
      ਅਚਲ uint64_t ਨਕਾਬ_2 = xgetbv();
      ਜੇ ((ਨਕਾਬ_2 & 6) == 6) { // SSE and AVX states
        ਜੇ (*ecx & (1UL << 28))
          features |= AVX;
        ਜੇ (max_id >= 7) {
          cpuidex(regs, 7, 0);
          ਜੇ (*ebx & (1UL << 5))
            features |= AVX2;
          ਜੇ ((ਨਕਾਬ_2 & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            ਜੇ (*ebx & (1UL << 31))
              features |= AVX512VL;
            ਜੇ (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    ਵਾਪਸ features;
#ਨਹੀਂਤਾਂ
    /* How to detect NEON? */
    ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  }
}

ਖਾਲੀ blake3_compress_ਵਿੱਚ_ਰੱਖੋ(uint32_t cv[8],
                              ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                              uint8_t ਖੰਡ_len, uint64_t counter,
                              uint8_t flags) {
#ਜੇ defined(IS_X86)
  ਅਚਲ ਗਿਣਤੀ cpu_feature features = get_cpu_features();
  MAYBE_ਅਣਵਰਤਿਆ(features);
#ਜੇ !defined(BLAKE3_NO_AVX512)
  ਜੇ (features & AVX512VL) {
    blake3_compress_ਵਿੱਚ_ਰੱਖੋ_avx512(cv, ਖੰਡ, ਖੰਡ_len, counter, flags);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_SSE41)
  ਜੇ (features & SSE41) {
    blake3_compress_ਵਿੱਚ_ਰੱਖੋ_sse41(cv, ਖੰਡ, ਖੰਡ_len, counter, flags);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_SSE2)
  ਜੇ (features & SSE2) {
    blake3_compress_ਵਿੱਚ_ਰੱਖੋ_sse2(cv, ਖੰਡ, ਖੰਡ_len, counter, flags);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
  blake3_compress_ਵਿੱਚ_ਰੱਖੋ_portable(cv, ਖੰਡ, ਖੰਡ_len, counter, flags);
}

ਖਾਲੀ blake3_compress_xof(ਅਚਲ uint32_t cv[8],
                         ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                         uint8_t ਖੰਡ_len, uint64_t counter, uint8_t flags,
                         uint8_t ਬਾਹਰਲਾ_2[64]) {
#ਜੇ defined(IS_X86)
  ਅਚਲ ਗਿਣਤੀ cpu_feature features = get_cpu_features();
  MAYBE_ਅਣਵਰਤਿਆ(features);
#ਜੇ !defined(BLAKE3_NO_AVX512)
  ਜੇ (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, ਖੰਡ, ਖੰਡ_len, counter, flags, ਬਾਹਰਲਾ_2);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_SSE41)
  ਜੇ (features & SSE41) {
    blake3_compress_xof_sse41(cv, ਖੰਡ, ਖੰਡ_len, counter, flags, ਬਾਹਰਲਾ_2);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_SSE2)
  ਜੇ (features & SSE2) {
    blake3_compress_xof_sse2(cv, ਖੰਡ, ਖੰਡ_len, counter, flags, ਬਾਹਰਲਾ_2);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
  blake3_compress_xof_portable(cv, ਖੰਡ, ਖੰਡ_len, counter, flags, ਬਾਹਰਲਾ_2);
}


ਖਾਲੀ blake3_xof_many(ਅਚਲ uint32_t cv[8],
                     ਅਚਲ uint8_t ਖੰਡ[BLAKE3_ਖੰਡ_LEN],
                     uint8_t ਖੰਡ_len, uint64_t counter, uint8_t flags,
                     uint8_t ਬਾਹਰਲਾ_2[64], size_t outblocks) {
  ਜੇ (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    ਵਾਪਸ;
  }
#ਜੇ defined(IS_X86)
  ਅਚਲ ਗਿਣਤੀ cpu_feature features = get_cpu_features();
  MAYBE_ਅਣਵਰਤਿਆ(features);
#ਜੇ !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  ਜੇ (features & AVX512VL) {
    blake3_xof_many_avx512(cv, ਖੰਡ, ਖੰਡ_len, counter, flags, ਬਾਹਰਲਾ_2, outblocks);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
  ਲਈ(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, ਖੰਡ, ਖੰਡ_len, counter + i, flags, ਬਾਹਰਲਾ_2 + 64*i);
  }
}

ਖਾਲੀ blake3_hash_many(ਅਚਲ uint8_t *ਅਚਲ *inputs, size_t ਅੰਕ_inputs,
                      size_t ਖੰਡਾਂ, ਅਚਲ uint32_t ਕੁੰਜੀ_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_ਸ਼ੁਰੂ, uint8_t flags_ਸਿਰਾ, uint8_t *ਬਾਹਰਲਾ_2) {
#ਜੇ defined(IS_X86)
  ਅਚਲ ਗਿਣਤੀ cpu_feature features = get_cpu_features();
  MAYBE_ਅਣਵਰਤਿਆ(features);
#ਜੇ !defined(BLAKE3_NO_AVX512)
  ਜੇ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, ਅੰਕ_inputs, ਖੰਡਾਂ, ਕੁੰਜੀ_2, counter,
                            increment_counter, flags, flags_ਸ਼ੁਰੂ, flags_ਸਿਰਾ,
                            ਬਾਹਰਲਾ_2);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_AVX2)
  ਜੇ (features & AVX2) {
    blake3_hash_many_avx2(inputs, ਅੰਕ_inputs, ਖੰਡਾਂ, ਕੁੰਜੀ_2, counter,
                          increment_counter, flags, flags_ਸ਼ੁਰੂ, flags_ਸਿਰਾ,
                          ਬਾਹਰਲਾ_2);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_SSE41)
  ਜੇ (features & SSE41) {
    blake3_hash_many_sse41(inputs, ਅੰਕ_inputs, ਖੰਡਾਂ, ਕੁੰਜੀ_2, counter,
                           increment_counter, flags, flags_ਸ਼ੁਰੂ, flags_ਸਿਰਾ,
                           ਬਾਹਰਲਾ_2);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_SSE2)
  ਜੇ (features & SSE2) {
    blake3_hash_many_sse2(inputs, ਅੰਕ_inputs, ਖੰਡਾਂ, ਕੁੰਜੀ_2, counter,
                          increment_counter, flags, flags_ਸ਼ੁਰੂ, flags_ਸਿਰਾ,
                          ਬਾਹਰਲਾ_2);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, ਅੰਕ_inputs, ਖੰਡਾਂ, ਕੁੰਜੀ_2, counter,
                        increment_counter, flags, flags_ਸ਼ੁਰੂ, flags_ਸਿਰਾ, ਬਾਹਰਲਾ_2);
  ਵਾਪਸ;
#ਜੇ_ਅੰਤ

  blake3_hash_many_portable(inputs, ਅੰਕ_inputs, ਖੰਡਾਂ, ਕੁੰਜੀ_2, counter,
                            increment_counter, flags, flags_ਸ਼ੁਰੂ, flags_ਸਿਰਾ,
                            ਬਾਹਰਲਾ_2);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(ਖਾਲੀ) {
#ਜੇ defined(IS_X86)
  ਅਚਲ ਗਿਣਤੀ cpu_feature features = get_cpu_features();
  MAYBE_ਅਣਵਰਤਿਆ(features);
#ਜੇ !defined(BLAKE3_NO_AVX512)
  ਜੇ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    ਵਾਪਸ 16;
  }
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_AVX2)
  ਜੇ (features & AVX2) {
    ਵਾਪਸ 8;
  }
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_SSE41)
  ਜੇ (features & SSE41) {
    ਵਾਪਸ 4;
  }
#ਜੇ_ਅੰਤ
#ਜੇ !defined(BLAKE3_NO_SSE2)
  ਜੇ (features & SSE2) {
    ਵਾਪਸ 4;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ BLAKE3_USE_NEON == 1
  ਵਾਪਸ 4;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ 1;
}
