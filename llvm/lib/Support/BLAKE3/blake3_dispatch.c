#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "blake3_impl.h"

#නම් defined(_MSC_VER)
#include <Windows.h>
#endif

#නම් defined(IS_X86)
#නම් defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)
#include <immintrin.h>
#නැතහොත්
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#නම් !defined(BLAKE3_ATOMICS)
#නම් defined(__has_include)
#නම් __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#නැතහොත්
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#නැතහොත්
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#නම් BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic පූර්ණ_සංඛ්‍යා
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#elif defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#නැතහොත්
#define ATOMIC_INT පූර්ණ_සංඛ්‍යා
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (void)((x))

#නම් defined(IS_X86)
static uint64_t xgetbv(void) {
#නම් defined(_MSC_VER)
  ආපසු _xgetbv(0);
#නැතහොත්
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  ආපසු ((uint64_t)edx << 32) | eax;
#endif
}

static void cpuid(uint32_t out[4], uint32_t id) {
#නම් defined(_MSC_VER)
  __cpuid((පූර්ණ_සංඛ්‍යා *)out, id);
#elif defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#නැතහොත්
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

static void cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#නම් defined(_MSC_VER)
  __cpuidex((පූර්ණ_සංඛ්‍යා *)out, id, sid);
#elif defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#නැතහොත්
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

#නම් !defined(BLAKE3_TESTING)
static /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#නම් !defined(BLAKE3_TESTING)
static
#endif
    ga_nanaa cpu_feature
    get_cpu_features(void) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  ga_nanaa cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  නම් (features != UNDEFINED) {
    ආපසු features;
  } නැතහොත් {
#නම් defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (void)edx;
    features = 0;
    cpuid(regs, 0);
    const පූර්ණ_සංඛ්‍යා max_id = *eax;
    cpuid(regs, 1);
#නම් defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#නැතහොත්
    නම් (*edx & (1UL << 26))
      features |= SSE2;
#endif
    නම් (*ecx & (1UL << 9))
      features |= SSSE3;
    නම් (*ecx & (1UL << 19))
      features |= SSE41;

    නම් (*ecx & (1UL << 27)) { // OSXSAVE
      const uint64_t mask = xgetbv();
      නම් ((mask & 6) == 6) { // SSE and AVX states
        නම් (*ecx & (1UL << 28))
          features |= AVX;
        නම් (max_id >= 7) {
          cpuidex(regs, 7, 0);
          නම් (*ebx & (1UL << 5))
            features |= AVX2;
          නම් ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            නම් (*ebx & (1UL << 31))
              features |= AVX512VL;
            නම් (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    ආපසු features;
#නැතහොත්
    /* How to detect NEON? */
    ආපසු 0;
#endif
  }
}

void blake3_compress_in_place(uint32_t cv[8],
                              const uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#නම් defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#නම් !defined(BLAKE3_NO_AVX512)
  නම් (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    ආපසු;
  }
#endif
#නම් !defined(BLAKE3_NO_SSE41)
  නම් (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    ආපසු;
  }
#endif
#නම් !defined(BLAKE3_NO_SSE2)
  නම් (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    ආපසු;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

void blake3_compress_xof(const uint32_t cv[8],
                         const uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#නම් defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#නම් !defined(BLAKE3_NO_AVX512)
  නම් (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    ආපසු;
  }
#endif
#නම් !defined(BLAKE3_NO_SSE41)
  නම් (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    ආපසු;
  }
#endif
#නම් !defined(BLAKE3_NO_SSE2)
  නම් (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    ආපසු;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


void blake3_xof_many(const uint32_t cv[8],
                     const uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  නම් (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    ආපසු;
  }
#නම් defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#නම් !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  නම් (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    ආපසු;
  }
#endif
#endif
  සඳහා(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

void blake3_hash_many(const uint8_t *const *inputs, size_t num_inputs,
                      size_t blocks, const uint32_t key[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#නම් defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#නම් !defined(BLAKE3_NO_AVX512)
  නම් ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    ආපසු;
  }
#endif
#නම් !defined(BLAKE3_NO_AVX2)
  නම් (features & AVX2) {
    blake3_hash_many_avx2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ආපසු;
  }
#endif
#නම් !defined(BLAKE3_NO_SSE41)
  නම් (features & SSE41) {
    blake3_hash_many_sse41(inputs, num_inputs, blocks, key, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    ආපසු;
  }
#endif
#නම් !defined(BLAKE3_NO_SSE2)
  නම් (features & SSE2) {
    blake3_hash_many_sse2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ආපසු;
  }
#endif
#endif

#නම් BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, num_inputs, blocks, key, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  ආපසු;
#endif

  blake3_hash_many_portable(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(void) {
#නම් defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#නම් !defined(BLAKE3_NO_AVX512)
  නම් ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    ආපසු 16;
  }
#endif
#නම් !defined(BLAKE3_NO_AVX2)
  නම් (features & AVX2) {
    ආපසු 8;
  }
#endif
#නම් !defined(BLAKE3_NO_SSE41)
  නම් (features & SSE41) {
    ආපසු 4;
  }
#endif
#නම් !defined(BLAKE3_NO_SSE2)
  නම් (features & SSE2) {
    ආපසු 4;
  }
#endif
#endif
#නම් BLAKE3_USE_NEON == 1
  ආපසු 4;
#endif
  ආපසු 1;
}
