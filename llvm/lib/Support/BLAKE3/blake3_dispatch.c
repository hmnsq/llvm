#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stddef.h>
#অন্তর্ভাৱ <stdint.h>

#অন্তর্ভাৱ "blake3_impl.h"

#যদি defined(_MSC_VER)
#অন্তর্ভাৱ <Windows.h>
#endif

#যদি defined(IS_X86)
#যদি defined(_MSC_VER)
#অন্তর্ভাৱ <intrin.h>
#নোচেদ্যদি defined(__GNUC__)
#অন্তর্ভাৱ <immintrin.h>
#অন্যথা
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#যদি !defined(BLAKE3_ATOMICS)
#যদি defined(__has_include)
#যদি __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#অন্যথা
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#অন্যথা
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#যদি BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic পূর্ণাঙ্ক
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#নোচেদ্যদি defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#অন্যথা
#define ATOMIC_INT পূর্ণাঙ্ক
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (শূন্য)((x))

#যদি defined(IS_X86)
স্থির uint64_t xgetbv(শূন্য) {
#যদি defined(_MSC_VER)
  ঘূৰাই _xgetbv(0);
#অন্যথা
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  ঘূৰাই ((uint64_t)edx << 32) | eax;
#endif
}

স্থির শূন্য cpuid(uint32_t out[4], uint32_t id) {
#যদি defined(_MSC_VER)
  __cpuid((পূর্ণাঙ্ক *)out, id);
#নোচেদ্যদি defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#অন্যথা
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

স্থির শূন্য cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#যদি defined(_MSC_VER)
  __cpuidex((পূর্ণাঙ্ক *)out, id, sid);
#নোচেদ্যদি defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#অন্যথা
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#endif
}

#endif

গণনা cpu_feature {
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

#যদি !defined(BLAKE3_TESTING)
স্থির /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#যদি !defined(BLAKE3_TESTING)
স্থির
#endif
    গণনা cpu_feature
    get_cpu_features(শূন্য) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  গণনা cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  যদি (features != UNDEFINED) {
    ঘূৰাই features;
  } অন্যথা {
#যদি defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (শূন্য)edx;
    features = 0;
    cpuid(regs, 0);
    ধ্রুৱ পূর্ণাঙ্ক max_id = *eax;
    cpuid(regs, 1);
#যদি defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#অন্যথা
    যদি (*edx & (1UL << 26))
      features |= SSE2;
#endif
    যদি (*ecx & (1UL << 9))
      features |= SSSE3;
    যদি (*ecx & (1UL << 19))
      features |= SSE41;

    যদি (*ecx & (1UL << 27)) { // OSXSAVE
      ধ্রুৱ uint64_t mask = xgetbv();
      যদি ((mask & 6) == 6) { // SSE and AVX states
        যদি (*ecx & (1UL << 28))
          features |= AVX;
        যদি (max_id >= 7) {
          cpuidex(regs, 7, 0);
          যদি (*ebx & (1UL << 5))
            features |= AVX2;
          যদি ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            যদি (*ebx & (1UL << 31))
              features |= AVX512VL;
            যদি (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    ঘূৰাই features;
#অন্যথা
    /* How to detect NEON? */
    ঘূৰাই 0;
#endif
  }
}

শূন্য blake3_compress_in_place(uint32_t cv[8],
                              ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#যদি defined(IS_X86)
  ধ্রুৱ গণনা cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#যদি !defined(BLAKE3_NO_AVX512)
  যদি (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    ঘূৰাই;
  }
#endif
#যদি !defined(BLAKE3_NO_SSE41)
  যদি (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    ঘূৰাই;
  }
#endif
#যদি !defined(BLAKE3_NO_SSE2)
  যদি (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    ঘূৰাই;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

শূন্য blake3_compress_xof(ধ্রুৱ uint32_t cv[8],
                         ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#যদি defined(IS_X86)
  ধ্রুৱ গণনা cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#যদি !defined(BLAKE3_NO_AVX512)
  যদি (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    ঘূৰাই;
  }
#endif
#যদি !defined(BLAKE3_NO_SSE41)
  যদি (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    ঘূৰাই;
  }
#endif
#যদি !defined(BLAKE3_NO_SSE2)
  যদি (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    ঘূৰাই;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


শূন্য blake3_xof_many(ধ্রুৱ uint32_t cv[8],
                     ধ্রুৱ uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  যদি (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    ঘূৰাই;
  }
#যদি defined(IS_X86)
  ধ্রুৱ গণনা cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#যদি !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  যদি (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    ঘূৰাই;
  }
#endif
#endif
  বাবে(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

শূন্য blake3_hash_many(ধ্রুৱ uint8_t *ধ্রুৱ *inputs, size_t অঙ্ক_inputs,
                      size_t blocks, ধ্রুৱ uint32_t চাবি_2[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#যদি defined(IS_X86)
  ধ্রুৱ গণনা cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#যদি !defined(BLAKE3_NO_AVX512)
  যদি ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, অঙ্ক_inputs, blocks, চাবি_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    ঘূৰাই;
  }
#endif
#যদি !defined(BLAKE3_NO_AVX2)
  যদি (features & AVX2) {
    blake3_hash_many_avx2(inputs, অঙ্ক_inputs, blocks, চাবি_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ঘূৰাই;
  }
#endif
#যদি !defined(BLAKE3_NO_SSE41)
  যদি (features & SSE41) {
    blake3_hash_many_sse41(inputs, অঙ্ক_inputs, blocks, চাবি_2, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    ঘূৰাই;
  }
#endif
#যদি !defined(BLAKE3_NO_SSE2)
  যদি (features & SSE2) {
    blake3_hash_many_sse2(inputs, অঙ্ক_inputs, blocks, চাবি_2, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ঘূৰাই;
  }
#endif
#endif

#যদি BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, অঙ্ক_inputs, blocks, চাবি_2, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  ঘূৰাই;
#endif

  blake3_hash_many_portable(inputs, অঙ্ক_inputs, blocks, চাবি_2, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(শূন্য) {
#যদি defined(IS_X86)
  ধ্রুৱ গণনা cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#যদি !defined(BLAKE3_NO_AVX512)
  যদি ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    ঘূৰাই 16;
  }
#endif
#যদি !defined(BLAKE3_NO_AVX2)
  যদি (features & AVX2) {
    ঘূৰাই 8;
  }
#endif
#যদি !defined(BLAKE3_NO_SSE41)
  যদি (features & SSE41) {
    ঘূৰাই 4;
  }
#endif
#যদি !defined(BLAKE3_NO_SSE2)
  যদি (features & SSE2) {
    ঘূৰাই 4;
  }
#endif
#endif
#যদি BLAKE3_USE_NEON == 1
  ঘূৰাই 4;
#endif
  ঘূৰাই 1;
}
