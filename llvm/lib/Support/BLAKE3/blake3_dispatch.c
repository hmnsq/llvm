#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdint.h>

#അന്തര്ഭാവ "blake3_impl.h"

#എങ്കിൽ defined(_MSC_VER)
#അന്തര്ഭാവ <Windows.h>
#endif

#എങ്കിൽ defined(IS_X86)
#എങ്കിൽ defined(_MSC_VER)
#അന്തര്ഭാവ <intrin.h>
#അല്ലെങ്കിലെങ്കിൽ defined(__GNUC__)
#അന്തര്ഭാവ <immintrin.h>
#അല്ലെങ്കിൽ
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#എങ്കിൽ !defined(BLAKE3_ATOMICS)
#എങ്കിൽ defined(__has_include)
#എങ്കിൽ __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#അല്ലെങ്കിൽ
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#അല്ലെങ്കിൽ
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#എങ്കിൽ BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic പൂർണ്ണസംഖ്യ
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#അല്ലെങ്കിലെങ്കിൽ defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#അല്ലെങ്കിൽ
#define ATOMIC_INT പൂർണ്ണസംഖ്യ
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (ശൂന്യം)((x))

#എങ്കിൽ defined(IS_X86)
സ്ഥിതം uint64_t xgetbv(ശൂന്യം) {
#എങ്കിൽ defined(_MSC_VER)
  തിരികെ _xgetbv(0);
#അല്ലെങ്കിൽ
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  തിരികെ ((uint64_t)edx << 32) | eax;
#endif
}

സ്ഥിതം ശൂന്യം cpuid(uint32_t out[4], uint32_t id) {
#എങ്കിൽ defined(_MSC_VER)
  __cpuid((പൂർണ്ണസംഖ്യ *)out, id);
#അല്ലെങ്കിലെങ്കിൽ defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#അല്ലെങ്കിൽ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

സ്ഥിതം ശൂന്യം cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#എങ്കിൽ defined(_MSC_VER)
  __cpuidex((പൂർണ്ണസംഖ്യ *)out, id, sid);
#അല്ലെങ്കിലെങ്കിൽ defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#അല്ലെങ്കിൽ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#endif
}

#endif

എണ്ണൽ cpu_feature {
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

#എങ്കിൽ !defined(BLAKE3_TESTING)
സ്ഥിതം /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#എങ്കിൽ !defined(BLAKE3_TESTING)
സ്ഥിതം
#endif
    എണ്ണൽ cpu_feature
    get_cpu_features(ശൂന്യം) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  എണ്ണൽ cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  എങ്കിൽ (features != UNDEFINED) {
    തിരികെ features;
  } അല്ലെങ്കിൽ {
#എങ്കിൽ defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (ശൂന്യം)edx;
    features = 0;
    cpuid(regs, 0);
    സ്ഥിരം പൂർണ്ണസംഖ്യ max_id = *eax;
    cpuid(regs, 1);
#എങ്കിൽ defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#അല്ലെങ്കിൽ
    എങ്കിൽ (*edx & (1UL << 26))
      features |= SSE2;
#endif
    എങ്കിൽ (*ecx & (1UL << 9))
      features |= SSSE3;
    എങ്കിൽ (*ecx & (1UL << 19))
      features |= SSE41;

    എങ്കിൽ (*ecx & (1UL << 27)) { // OSXSAVE
      സ്ഥിരം uint64_t mask = xgetbv();
      എങ്കിൽ ((mask & 6) == 6) { // SSE and AVX states
        എങ്കിൽ (*ecx & (1UL << 28))
          features |= AVX;
        എങ്കിൽ (max_id >= 7) {
          cpuidex(regs, 7, 0);
          എങ്കിൽ (*ebx & (1UL << 5))
            features |= AVX2;
          എങ്കിൽ ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            എങ്കിൽ (*ebx & (1UL << 31))
              features |= AVX512VL;
            എങ്കിൽ (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    തിരികെ features;
#അല്ലെങ്കിൽ
    /* How to detect NEON? */
    തിരികെ 0;
#endif
  }
}

ശൂന്യം blake3_compress_in_place(uint32_t cv[8],
                              സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#എങ്കിൽ defined(IS_X86)
  സ്ഥിരം എണ്ണൽ cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#എങ്കിൽ !defined(BLAKE3_NO_AVX512)
  എങ്കിൽ (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    തിരികെ;
  }
#endif
#എങ്കിൽ !defined(BLAKE3_NO_SSE41)
  എങ്കിൽ (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    തിരികെ;
  }
#endif
#എങ്കിൽ !defined(BLAKE3_NO_SSE2)
  എങ്കിൽ (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    തിരികെ;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

ശൂന്യം blake3_compress_xof(സ്ഥിരം uint32_t cv[8],
                         സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#എങ്കിൽ defined(IS_X86)
  സ്ഥിരം എണ്ണൽ cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#എങ്കിൽ !defined(BLAKE3_NO_AVX512)
  എങ്കിൽ (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    തിരികെ;
  }
#endif
#എങ്കിൽ !defined(BLAKE3_NO_SSE41)
  എങ്കിൽ (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    തിരികെ;
  }
#endif
#എങ്കിൽ !defined(BLAKE3_NO_SSE2)
  എങ്കിൽ (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    തിരികെ;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


ശൂന്യം blake3_xof_many(സ്ഥിരം uint32_t cv[8],
                     സ്ഥിരം uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  എങ്കിൽ (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    തിരികെ;
  }
#എങ്കിൽ defined(IS_X86)
  സ്ഥിരം എണ്ണൽ cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#എങ്കിൽ !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  എങ്കിൽ (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    തിരികെ;
  }
#endif
#endif
  വേണ്ടി(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

ശൂന്യം blake3_hash_many(സ്ഥിരം uint8_t *സ്ഥിരം *inputs, size_t അക്കം_inputs,
                      size_t blocks, സ്ഥിരം uint32_t താക്കോൽ_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#എങ്കിൽ defined(IS_X86)
  സ്ഥിരം എണ്ണൽ cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#എങ്കിൽ !defined(BLAKE3_NO_AVX512)
  എങ്കിൽ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, അക്കം_inputs, blocks, താക്കോൽ_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    തിരികെ;
  }
#endif
#എങ്കിൽ !defined(BLAKE3_NO_AVX2)
  എങ്കിൽ (features & AVX2) {
    blake3_hash_many_avx2(inputs, അക്കം_inputs, blocks, താക്കോൽ_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    തിരികെ;
  }
#endif
#എങ്കിൽ !defined(BLAKE3_NO_SSE41)
  എങ്കിൽ (features & SSE41) {
    blake3_hash_many_sse41(inputs, അക്കം_inputs, blocks, താക്കോൽ_2, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    തിരികെ;
  }
#endif
#എങ്കിൽ !defined(BLAKE3_NO_SSE2)
  എങ്കിൽ (features & SSE2) {
    blake3_hash_many_sse2(inputs, അക്കം_inputs, blocks, താക്കോൽ_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    തിരികെ;
  }
#endif
#endif

#എങ്കിൽ BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, അക്കം_inputs, blocks, താക്കോൽ_2, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  തിരികെ;
#endif

  blake3_hash_many_portable(inputs, അക്കം_inputs, blocks, താക്കോൽ_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(ശൂന്യം) {
#എങ്കിൽ defined(IS_X86)
  സ്ഥിരം എണ്ണൽ cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#എങ്കിൽ !defined(BLAKE3_NO_AVX512)
  എങ്കിൽ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    തിരികെ 16;
  }
#endif
#എങ്കിൽ !defined(BLAKE3_NO_AVX2)
  എങ്കിൽ (features & AVX2) {
    തിരികെ 8;
  }
#endif
#എങ്കിൽ !defined(BLAKE3_NO_SSE41)
  എങ്കിൽ (features & SSE41) {
    തിരികെ 4;
  }
#endif
#എങ്കിൽ !defined(BLAKE3_NO_SSE2)
  എങ്കിൽ (features & SSE2) {
    തിരികെ 4;
  }
#endif
#endif
#എങ്കിൽ BLAKE3_USE_NEON == 1
  തിരികെ 4;
#endif
  തിരികെ 1;
}
