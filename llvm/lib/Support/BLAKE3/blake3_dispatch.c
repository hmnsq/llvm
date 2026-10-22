#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "blake3_impl.h"

#ᬬᬾᬦ᭄ defined(_MSC_VER)
#include <Windows.h>
#endif

#ᬬᬾᬦ᭄ defined(IS_X86)
#ᬬᬾᬦ᭄ defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)
#include <immintrin.h>
#ᬮᬶᬬ
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#ᬬᬾᬦ᭄ !defined(BLAKE3_ATOMICS)
#ᬬᬾᬦ᭄ defined(__has_include)
#ᬬᬾᬦ᭄ __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#ᬮᬶᬬ
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#ᬮᬶᬬ
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#ᬬᬾᬦ᭄ BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic puur_naa_nka
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#elif defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#ᬮᬶᬬ
#define ATOMIC_INT puur_naa_nka
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (void)((x))

#ᬬᬾᬦ᭄ defined(IS_X86)
static uint64_t xgetbv(void) {
#ᬬᬾᬦ᭄ defined(_MSC_VER)
  ᬯᬮᬶ _xgetbv(0);
#ᬮᬶᬬ
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  ᬯᬮᬶ ((uint64_t)edx << 32) | eax;
#endif
}

static void cpuid(uint32_t out[4], uint32_t id) {
#ᬬᬾᬦ᭄ defined(_MSC_VER)
  __cpuid((puur_naa_nka *)out, id);
#elif defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#ᬮᬶᬬ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

static void cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#ᬬᬾᬦ᭄ defined(_MSC_VER)
  __cpuidex((puur_naa_nka *)out, id, sid);
#elif defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#ᬮᬶᬬ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#endif
}

#endif

ga_nanaa cpu_feature {
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

#ᬬᬾᬦ᭄ !defined(BLAKE3_TESTING)
static /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#ᬬᬾᬦ᭄ !defined(BLAKE3_TESTING)
static
#endif
    ga_nanaa cpu_feature
    get_cpu_features(void) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  ga_nanaa cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  ᬬᬾᬦ᭄ (features != UNDEFINED) {
    ᬯᬮᬶ features;
  } ᬮᬶᬬ {
#ᬬᬾᬦ᭄ defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (void)edx;
    features = 0;
    cpuid(regs, 0);
    const puur_naa_nka max_id = *eax;
    cpuid(regs, 1);
#ᬬᬾᬦ᭄ defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#ᬮᬶᬬ
    ᬬᬾᬦ᭄ (*edx & (1UL << 26))
      features |= SSE2;
#endif
    ᬬᬾᬦ᭄ (*ecx & (1UL << 9))
      features |= SSSE3;
    ᬬᬾᬦ᭄ (*ecx & (1UL << 19))
      features |= SSE41;

    ᬬᬾᬦ᭄ (*ecx & (1UL << 27)) { // OSXSAVE
      const uint64_t mask = xgetbv();
      ᬬᬾᬦ᭄ ((mask & 6) == 6) { // SSE and AVX states
        ᬬᬾᬦ᭄ (*ecx & (1UL << 28))
          features |= AVX;
        ᬬᬾᬦ᭄ (max_id >= 7) {
          cpuidex(regs, 7, 0);
          ᬬᬾᬦ᭄ (*ebx & (1UL << 5))
            features |= AVX2;
          ᬬᬾᬦ᭄ ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            ᬬᬾᬦ᭄ (*ebx & (1UL << 31))
              features |= AVX512VL;
            ᬬᬾᬦ᭄ (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    ᬯᬮᬶ features;
#ᬮᬶᬬ
    /* How to detect NEON? */
    ᬯᬮᬶ 0;
#endif
  }
}

void blake3_compress_in_place(uint32_t cv[8],
                              const uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#ᬬᬾᬦ᭄ defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_AVX512)
  ᬬᬾᬦ᭄ (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    ᬯᬮᬶ;
  }
#endif
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_SSE41)
  ᬬᬾᬦ᭄ (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    ᬯᬮᬶ;
  }
#endif
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_SSE2)
  ᬬᬾᬦ᭄ (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    ᬯᬮᬶ;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

void blake3_compress_xof(const uint32_t cv[8],
                         const uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#ᬬᬾᬦ᭄ defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_AVX512)
  ᬬᬾᬦ᭄ (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    ᬯᬮᬶ;
  }
#endif
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_SSE41)
  ᬬᬾᬦ᭄ (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    ᬯᬮᬶ;
  }
#endif
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_SSE2)
  ᬬᬾᬦ᭄ (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    ᬯᬮᬶ;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


void blake3_xof_many(const uint32_t cv[8],
                     const uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  ᬬᬾᬦ᭄ (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    ᬯᬮᬶ;
  }
#ᬬᬾᬦ᭄ defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ᬬᬾᬦ᭄ !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  ᬬᬾᬦ᭄ (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    ᬯᬮᬶ;
  }
#endif
#endif
  for(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

void blake3_hash_many(const uint8_t *const *inputs, size_t num_inputs,
                      size_t blocks, const uint32_t key[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#ᬬᬾᬦ᭄ defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_AVX512)
  ᬬᬾᬦ᭄ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    ᬯᬮᬶ;
  }
#endif
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_AVX2)
  ᬬᬾᬦ᭄ (features & AVX2) {
    blake3_hash_many_avx2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ᬯᬮᬶ;
  }
#endif
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_SSE41)
  ᬬᬾᬦ᭄ (features & SSE41) {
    blake3_hash_many_sse41(inputs, num_inputs, blocks, key, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    ᬯᬮᬶ;
  }
#endif
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_SSE2)
  ᬬᬾᬦ᭄ (features & SSE2) {
    blake3_hash_many_sse2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ᬯᬮᬶ;
  }
#endif
#endif

#ᬬᬾᬦ᭄ BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, num_inputs, blocks, key, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  ᬯᬮᬶ;
#endif

  blake3_hash_many_portable(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(void) {
#ᬬᬾᬦ᭄ defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_AVX512)
  ᬬᬾᬦ᭄ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    ᬯᬮᬶ 16;
  }
#endif
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_AVX2)
  ᬬᬾᬦ᭄ (features & AVX2) {
    ᬯᬮᬶ 8;
  }
#endif
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_SSE41)
  ᬬᬾᬦ᭄ (features & SSE41) {
    ᬯᬮᬶ 4;
  }
#endif
#ᬬᬾᬦ᭄ !defined(BLAKE3_NO_SSE2)
  ᬬᬾᬦ᭄ (features & SSE2) {
    ᬯᬮᬶ 4;
  }
#endif
#endif
#ᬬᬾᬦ᭄ BLAKE3_USE_NEON == 1
  ᬯᬮᬶ 4;
#endif
  ᬯᬮᬶ 1;
}
