#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdint.h>

#અન્તર્ભાવ "blake3_impl.h"

#જો defined(_MSC_VER)
#અન્તર્ભાવ <Windows.h>
#endif

#જો defined(IS_X86)
#જો defined(_MSC_VER)
#અન્તર્ભાવ <intrin.h>
#અથવાજો defined(__GNUC__)
#અન્તર્ભાવ <immintrin.h>
#નહીંતર
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#જો !defined(BLAKE3_ATOMICS)
#જો defined(__has_include)
#જો __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#નહીંતર
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#નહીંતર
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#જો BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic પૂર્ણાંક
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#અથવાજો defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#નહીંતર
#define ATOMIC_INT પૂર્ણાંક
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (શૂન્ય)((x))

#જો defined(IS_X86)
સ્થિર uint64_t xgetbv(શૂન્ય) {
#જો defined(_MSC_VER)
  પાછા _xgetbv(0);
#નહીંતર
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  પાછા ((uint64_t)edx << 32) | eax;
#endif
}

સ્થિર શૂન્ય cpuid(uint32_t out[4], uint32_t id) {
#જો defined(_MSC_VER)
  __cpuid((પૂર્ણાંક *)out, id);
#અથવાજો defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#નહીંતર
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

સ્થિર શૂન્ય cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#જો defined(_MSC_VER)
  __cpuidex((પૂર્ણાંક *)out, id, sid);
#અથવાજો defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#નહીંતર
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#endif
}

#endif

ગણના cpu_feature {
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

#જો !defined(BLAKE3_TESTING)
સ્થિર /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#જો !defined(BLAKE3_TESTING)
સ્થિર
#endif
    ગણના cpu_feature
    get_cpu_features(શૂન્ય) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  ગણના cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  જો (features != UNDEFINED) {
    પાછા features;
  } નહીંતર {
#જો defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (શૂન્ય)edx;
    features = 0;
    cpuid(regs, 0);
    અચલ પૂર્ણાંક max_id = *eax;
    cpuid(regs, 1);
#જો defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#નહીંતર
    જો (*edx & (1UL << 26))
      features |= SSE2;
#endif
    જો (*ecx & (1UL << 9))
      features |= SSSE3;
    જો (*ecx & (1UL << 19))
      features |= SSE41;

    જો (*ecx & (1UL << 27)) { // OSXSAVE
      અચલ uint64_t mask = xgetbv();
      જો ((mask & 6) == 6) { // SSE and AVX states
        જો (*ecx & (1UL << 28))
          features |= AVX;
        જો (max_id >= 7) {
          cpuidex(regs, 7, 0);
          જો (*ebx & (1UL << 5))
            features |= AVX2;
          જો ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            જો (*ebx & (1UL << 31))
              features |= AVX512VL;
            જો (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    પાછા features;
#નહીંતર
    /* How to detect NEON? */
    પાછા 0;
#endif
  }
}

શૂન્ય blake3_compress_in_place(uint32_t cv[8],
                              અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#જો defined(IS_X86)
  અચલ ગણના cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#જો !defined(BLAKE3_NO_AVX512)
  જો (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    પાછા;
  }
#endif
#જો !defined(BLAKE3_NO_SSE41)
  જો (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    પાછા;
  }
#endif
#જો !defined(BLAKE3_NO_SSE2)
  જો (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    પાછા;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

શૂન્ય blake3_compress_xof(અચલ uint32_t cv[8],
                         અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#જો defined(IS_X86)
  અચલ ગણના cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#જો !defined(BLAKE3_NO_AVX512)
  જો (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    પાછા;
  }
#endif
#જો !defined(BLAKE3_NO_SSE41)
  જો (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    પાછા;
  }
#endif
#જો !defined(BLAKE3_NO_SSE2)
  જો (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    પાછા;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


શૂન્ય blake3_xof_many(અચલ uint32_t cv[8],
                     અચલ uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  જો (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    પાછા;
  }
#જો defined(IS_X86)
  અચલ ગણના cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#જો !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  જો (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    પાછા;
  }
#endif
#endif
  માટે(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

શૂન્ય blake3_hash_many(અચલ uint8_t *અચલ *inputs, size_t અંક_inputs,
                      size_t blocks, અચલ uint32_t ચાવી_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#જો defined(IS_X86)
  અચલ ગણના cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#જો !defined(BLAKE3_NO_AVX512)
  જો ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, અંક_inputs, blocks, ચાવી_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    પાછા;
  }
#endif
#જો !defined(BLAKE3_NO_AVX2)
  જો (features & AVX2) {
    blake3_hash_many_avx2(inputs, અંક_inputs, blocks, ચાવી_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    પાછા;
  }
#endif
#જો !defined(BLAKE3_NO_SSE41)
  જો (features & SSE41) {
    blake3_hash_many_sse41(inputs, અંક_inputs, blocks, ચાવી_2, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    પાછા;
  }
#endif
#જો !defined(BLAKE3_NO_SSE2)
  જો (features & SSE2) {
    blake3_hash_many_sse2(inputs, અંક_inputs, blocks, ચાવી_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    પાછા;
  }
#endif
#endif

#જો BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, અંક_inputs, blocks, ચાવી_2, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  પાછા;
#endif

  blake3_hash_many_portable(inputs, અંક_inputs, blocks, ચાવી_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(શૂન્ય) {
#જો defined(IS_X86)
  અચલ ગણના cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#જો !defined(BLAKE3_NO_AVX512)
  જો ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    પાછા 16;
  }
#endif
#જો !defined(BLAKE3_NO_AVX2)
  જો (features & AVX2) {
    પાછા 8;
  }
#endif
#જો !defined(BLAKE3_NO_SSE41)
  જો (features & SSE41) {
    પાછા 4;
  }
#endif
#જો !defined(BLAKE3_NO_SSE2)
  જો (features & SSE2) {
    પાછા 4;
  }
#endif
#endif
#જો BLAKE3_USE_NEON == 1
  પાછા 4;
#endif
  પાછા 1;
}
