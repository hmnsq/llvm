#समावेश <stdbool.h>
#समावेश <stddef.h>
#समावेश <stdint.h>

#समावेश "blake3_impl.h"

#अगर defined(_MSC_VER)
#समावेश <Windows.h>
#पूर्ण_अगर

#अगर defined(IS_X86)
#अगर defined(_MSC_VER)
#समावेश <intrin.h>
#या_अगर defined(__GNUC__)
#समावेश <immintrin.h>
#अन्यथा
#अघोषित IS_X86 /* Unimplemented! */
#पूर्ण_अगर
#पूर्ण_अगर

#अगर !defined(BLAKE3_ATOMICS)
#अगर defined(__has_include)
#अगर __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#घोषणा BLAKE3_ATOMICS 1
#अन्यथा
#घोषणा BLAKE3_ATOMICS 0
#पूर्ण_अगर /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#अन्यथा
#घोषणा BLAKE3_ATOMICS 0
#पूर्ण_अगर /* defined(__has_include) */
#पूर्ण_अगर /* BLAKE3_ATOMICS */

#अगर BLAKE3_ATOMICS
#घोषणा ATOMIC_INT _Atomic पूर्णांक
#घोषणा ATOMIC_LOAD(x) x
#घोषणा ATOMIC_STORE(x, y) x = y
#या_अगर defined(_MSC_VER)
#घोषणा ATOMIC_INT LONG
#घोषणा ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#घोषणा ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#अन्यथा
#घोषणा ATOMIC_INT पूर्णांक
#घोषणा ATOMIC_LOAD(x) x
#घोषणा ATOMIC_STORE(x, y) x = y
#पूर्ण_अगर

#घोषणा MAYBE_UNUSED(x) (व्योम)((x))

#अगर defined(IS_X86)
अटल uint64_t xgetbv(व्योम) {
#अगर defined(_MSC_VER)
  वापस _xgetbv(0);
#अन्यथा
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  वापस ((uint64_t)edx << 32) | eax;
#पूर्ण_अगर
}

अटल व्योम cpuid(uint32_t out[4], uint32_t id) {
#अगर defined(_MSC_VER)
  __cpuid((पूर्णांक *)out, id);
#या_अगर defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#अन्यथा
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#पूर्ण_अगर
}

अटल व्योम cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#अगर defined(_MSC_VER)
  __cpuidex((पूर्णांक *)out, id, sid);
#या_अगर defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#अन्यथा
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#पूर्ण_अगर
}

#पूर्ण_अगर

क्रमागत cpu_feature {
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

#अगर !defined(BLAKE3_TESTING)
अटल /* Allow the variable to be controlled manually for testing */
#पूर्ण_अगर
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#अगर !defined(BLAKE3_TESTING)
अटल
#पूर्ण_अगर
    क्रमागत cpu_feature
    get_cpu_features(व्योम) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  क्रमागत cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  अगर (features != UNDEFINED) {
    वापस features;
  } अन्यथा {
#अगर defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (व्योम)edx;
    features = 0;
    cpuid(regs, 0);
    स्थिर पूर्णांक max_id = *eax;
    cpuid(regs, 1);
#अगर defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#अन्यथा
    अगर (*edx & (1UL << 26))
      features |= SSE2;
#पूर्ण_अगर
    अगर (*ecx & (1UL << 9))
      features |= SSSE3;
    अगर (*ecx & (1UL << 19))
      features |= SSE41;

    अगर (*ecx & (1UL << 27)) { // OSXSAVE
      स्थिर uint64_t mask = xgetbv();
      अगर ((mask & 6) == 6) { // SSE and AVX states
        अगर (*ecx & (1UL << 28))
          features |= AVX;
        अगर (max_id >= 7) {
          cpuidex(regs, 7, 0);
          अगर (*ebx & (1UL << 5))
            features |= AVX2;
          अगर ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            अगर (*ebx & (1UL << 31))
              features |= AVX512VL;
            अगर (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    वापस features;
#अन्यथा
    /* How to detect NEON? */
    वापस 0;
#पूर्ण_अगर
  }
}

व्योम blake3_compress_in_place(uint32_t cv[8],
                              स्थिर uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t गणक_2,
                              uint8_t flags) {
#अगर defined(IS_X86)
  स्थिर क्रमागत cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#अगर !defined(BLAKE3_NO_AVX512)
  अगर (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, गणक_2, flags);
    वापस;
  }
#पूर्ण_अगर
#अगर !defined(BLAKE3_NO_SSE41)
  अगर (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, गणक_2, flags);
    वापस;
  }
#पूर्ण_अगर
#अगर !defined(BLAKE3_NO_SSE2)
  अगर (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, गणक_2, flags);
    वापस;
  }
#पूर्ण_अगर
#पूर्ण_अगर
  blake3_compress_in_place_portable(cv, block, block_len, गणक_2, flags);
}

व्योम blake3_compress_xof(स्थिर uint32_t cv[8],
                         स्थिर uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t गणक_2, uint8_t flags,
                         uint8_t out[64]) {
#अगर defined(IS_X86)
  स्थिर क्रमागत cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#अगर !defined(BLAKE3_NO_AVX512)
  अगर (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, गणक_2, flags, out);
    वापस;
  }
#पूर्ण_अगर
#अगर !defined(BLAKE3_NO_SSE41)
  अगर (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, गणक_2, flags, out);
    वापस;
  }
#पूर्ण_अगर
#अगर !defined(BLAKE3_NO_SSE2)
  अगर (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, गणक_2, flags, out);
    वापस;
  }
#पूर्ण_अगर
#पूर्ण_अगर
  blake3_compress_xof_portable(cv, block, block_len, गणक_2, flags, out);
}


व्योम blake3_xof_many(स्थिर uint32_t cv[8],
                     स्थिर uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t गणक_2, uint8_t flags,
                     uint8_t out[64], माप_प्रकार outblocks) {
  अगर (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    वापस;
  }
#अगर defined(IS_X86)
  स्थिर क्रमागत cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#अगर !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  अगर (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, गणक_2, flags, out, outblocks);
    वापस;
  }
#पूर्ण_अगर
#पूर्ण_अगर
  क्रम(माप_प्रकार i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, गणक_2 + i, flags, out + 64*i);
  }
}

व्योम blake3_hash_many(स्थिर uint8_t *स्थिर *inputs, माप_प्रकार अंक_inputs,
                      माप_प्रकार blocks, स्थिर uint32_t कुंजी_2[8], uint64_t गणक_2,
                      bool increment_गणक, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#अगर defined(IS_X86)
  स्थिर क्रमागत cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#अगर !defined(BLAKE3_NO_AVX512)
  अगर ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, अंक_inputs, blocks, कुंजी_2, गणक_2,
                            increment_गणक, flags, flags_start, flags_end,
                            out);
    वापस;
  }
#पूर्ण_अगर
#अगर !defined(BLAKE3_NO_AVX2)
  अगर (features & AVX2) {
    blake3_hash_many_avx2(inputs, अंक_inputs, blocks, कुंजी_2, गणक_2,
                          increment_गणक, flags, flags_start, flags_end,
                          out);
    वापस;
  }
#पूर्ण_अगर
#अगर !defined(BLAKE3_NO_SSE41)
  अगर (features & SSE41) {
    blake3_hash_many_sse41(inputs, अंक_inputs, blocks, कुंजी_2, गणक_2,
                           increment_गणक, flags, flags_start, flags_end,
                           out);
    वापस;
  }
#पूर्ण_अगर
#अगर !defined(BLAKE3_NO_SSE2)
  अगर (features & SSE2) {
    blake3_hash_many_sse2(inputs, अंक_inputs, blocks, कुंजी_2, गणक_2,
                          increment_गणक, flags, flags_start, flags_end,
                          out);
    वापस;
  }
#पूर्ण_अगर
#पूर्ण_अगर

#अगर BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, अंक_inputs, blocks, कुंजी_2, गणक_2,
                        increment_गणक, flags, flags_start, flags_end, out);
  वापस;
#पूर्ण_अगर

  blake3_hash_many_portable(inputs, अंक_inputs, blocks, कुंजी_2, गणक_2,
                            increment_गणक, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
माप_प्रकार blake3_simd_degree(व्योम) {
#अगर defined(IS_X86)
  स्थिर क्रमागत cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#अगर !defined(BLAKE3_NO_AVX512)
  अगर ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    वापस 16;
  }
#पूर्ण_अगर
#अगर !defined(BLAKE3_NO_AVX2)
  अगर (features & AVX2) {
    वापस 8;
  }
#पूर्ण_अगर
#अगर !defined(BLAKE3_NO_SSE41)
  अगर (features & SSE41) {
    वापस 4;
  }
#पूर्ण_अगर
#अगर !defined(BLAKE3_NO_SSE2)
  अगर (features & SSE2) {
    वापस 4;
  }
#पूर्ण_अगर
#पूर्ण_अगर
#अगर BLAKE3_USE_NEON == 1
  वापस 4;
#पूर्ण_अगर
  वापस 1;
}
