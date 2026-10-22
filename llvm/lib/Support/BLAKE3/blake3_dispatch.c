#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "blake3_impl.h"

#ако defined(_MSC_VER)
#include <Windows.h>
#endif

#ако defined(IS_X86)
#ако defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)
#include <immintrin.h>
#инако
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#ако !defined(BLAKE3_ATOMICS)
#ако defined(__has_include)
#ако __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#инако
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#инако
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#ако BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic цео
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#elif defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#инако
#define ATOMIC_INT цео
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (void)((x))

#ако defined(IS_X86)
static uint64_t xgetbv(void) {
#ако defined(_MSC_VER)
  врати _xgetbv(0);
#инако
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  врати ((uint64_t)edx << 32) | eax;
#endif
}

static void cpuid(uint32_t out[4], uint32_t id) {
#ако defined(_MSC_VER)
  __cpuid((цео *)out, id);
#elif defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#инако
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

static void cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#ако defined(_MSC_VER)
  __cpuidex((цео *)out, id, sid);
#elif defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#инако
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#endif
}

#endif

enum cpu_feature {
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

#ако !defined(BLAKE3_TESTING)
static /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#ако !defined(BLAKE3_TESTING)
static
#endif
    enum cpu_feature
    get_cpu_features(void) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  enum cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  ако (features != UNDEFINED) {
    врати features;
  } инако {
#ако defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (void)edx;
    features = 0;
    cpuid(regs, 0);
    const цео max_id = *eax;
    cpuid(regs, 1);
#ако defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#инако
    ако (*edx & (1UL << 26))
      features |= SSE2;
#endif
    ако (*ecx & (1UL << 9))
      features |= SSSE3;
    ако (*ecx & (1UL << 19))
      features |= SSE41;

    ако (*ecx & (1UL << 27)) { // OSXSAVE
      const uint64_t mask = xgetbv();
      ако ((mask & 6) == 6) { // SSE and AVX states
        ако (*ecx & (1UL << 28))
          features |= AVX;
        ако (max_id >= 7) {
          cpuidex(regs, 7, 0);
          ако (*ebx & (1UL << 5))
            features |= AVX2;
          ако ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            ако (*ebx & (1UL << 31))
              features |= AVX512VL;
            ако (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    врати features;
#инако
    /* How to detect NEON? */
    врати 0;
#endif
  }
}

void blake3_compress_in_place(uint32_t cv[8],
                              const uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#ако defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ако !defined(BLAKE3_NO_AVX512)
  ако (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    врати;
  }
#endif
#ако !defined(BLAKE3_NO_SSE41)
  ако (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    врати;
  }
#endif
#ако !defined(BLAKE3_NO_SSE2)
  ако (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    врати;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

void blake3_compress_xof(const uint32_t cv[8],
                         const uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#ако defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ако !defined(BLAKE3_NO_AVX512)
  ако (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    врати;
  }
#endif
#ако !defined(BLAKE3_NO_SSE41)
  ако (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    врати;
  }
#endif
#ако !defined(BLAKE3_NO_SSE2)
  ако (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    врати;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


void blake3_xof_many(const uint32_t cv[8],
                     const uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  ако (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    врати;
  }
#ако defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ако !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  ако (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    врати;
  }
#endif
#endif
  за(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

void blake3_hash_many(const uint8_t *const *inputs, size_t num_inputs,
                      size_t blocks, const uint32_t key[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#ако defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ако !defined(BLAKE3_NO_AVX512)
  ако ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    врати;
  }
#endif
#ако !defined(BLAKE3_NO_AVX2)
  ако (features & AVX2) {
    blake3_hash_many_avx2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    врати;
  }
#endif
#ако !defined(BLAKE3_NO_SSE41)
  ако (features & SSE41) {
    blake3_hash_many_sse41(inputs, num_inputs, blocks, key, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    врати;
  }
#endif
#ако !defined(BLAKE3_NO_SSE2)
  ако (features & SSE2) {
    blake3_hash_many_sse2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    врати;
  }
#endif
#endif

#ако BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, num_inputs, blocks, key, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  врати;
#endif

  blake3_hash_many_portable(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(void) {
#ако defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ако !defined(BLAKE3_NO_AVX512)
  ако ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    врати 16;
  }
#endif
#ако !defined(BLAKE3_NO_AVX2)
  ако (features & AVX2) {
    врати 8;
  }
#endif
#ако !defined(BLAKE3_NO_SSE41)
  ако (features & SSE41) {
    врати 4;
  }
#endif
#ако !defined(BLAKE3_NO_SSE2)
  ако (features & SSE2) {
    врати 4;
  }
#endif
#endif
#ако BLAKE3_USE_NEON == 1
  врати 4;
#endif
  врати 1;
}
