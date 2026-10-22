#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "blake3_impl.h"

#もし defined(_MSC_VER)
#include <Windows.h>
#endif

#もし defined(IS_X86)
#もし defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)
#include <immintrin.h>
#否则
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#もし !defined(BLAKE3_ATOMICS)
#もし defined(__has_include)
#もし __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#否则
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#否则
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#もし BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic 整数
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#elif defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#否则
#define ATOMIC_INT 整数
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (void)((x))

#もし defined(IS_X86)
static uint64_t xgetbv(void) {
#もし defined(_MSC_VER)
  戻る _xgetbv(0);
#否则
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  戻る ((uint64_t)edx << 32) | eax;
#endif
}

static void cpuid(uint32_t out[4], uint32_t id) {
#もし defined(_MSC_VER)
  __cpuid((整数 *)out, id);
#elif defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#否则
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

static void cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#もし defined(_MSC_VER)
  __cpuidex((整数 *)out, id, sid);
#elif defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#否则
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

#もし !defined(BLAKE3_TESTING)
static /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#もし !defined(BLAKE3_TESTING)
static
#endif
    enum cpu_feature
    get_cpu_features(void) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  enum cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  もし (features != UNDEFINED) {
    戻る features;
  } 否则 {
#もし defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (void)edx;
    features = 0;
    cpuid(regs, 0);
    const 整数 max_id = *eax;
    cpuid(regs, 1);
#もし defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#否则
    もし (*edx & (1UL << 26))
      features |= SSE2;
#endif
    もし (*ecx & (1UL << 9))
      features |= SSSE3;
    もし (*ecx & (1UL << 19))
      features |= SSE41;

    もし (*ecx & (1UL << 27)) { // OSXSAVE
      const uint64_t mask = xgetbv();
      もし ((mask & 6) == 6) { // SSE and AVX states
        もし (*ecx & (1UL << 28))
          features |= AVX;
        もし (max_id >= 7) {
          cpuidex(regs, 7, 0);
          もし (*ebx & (1UL << 5))
            features |= AVX2;
          もし ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            もし (*ebx & (1UL << 31))
              features |= AVX512VL;
            もし (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    戻る features;
#否则
    /* How to detect NEON? */
    戻る 0;
#endif
  }
}

void blake3_compress_in_place(uint32_t cv[8],
                              const uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#もし defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#もし !defined(BLAKE3_NO_AVX512)
  もし (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    戻る;
  }
#endif
#もし !defined(BLAKE3_NO_SSE41)
  もし (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    戻る;
  }
#endif
#もし !defined(BLAKE3_NO_SSE2)
  もし (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    戻る;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

void blake3_compress_xof(const uint32_t cv[8],
                         const uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#もし defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#もし !defined(BLAKE3_NO_AVX512)
  もし (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    戻る;
  }
#endif
#もし !defined(BLAKE3_NO_SSE41)
  もし (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    戻る;
  }
#endif
#もし !defined(BLAKE3_NO_SSE2)
  もし (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    戻る;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


void blake3_xof_many(const uint32_t cv[8],
                     const uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  もし (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    戻る;
  }
#もし defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#もし !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  もし (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    戻る;
  }
#endif
#endif
  ために(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

void blake3_hash_many(const uint8_t *const *inputs, size_t num_inputs,
                      size_t blocks, const uint32_t key[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#もし defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#もし !defined(BLAKE3_NO_AVX512)
  もし ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    戻る;
  }
#endif
#もし !defined(BLAKE3_NO_AVX2)
  もし (features & AVX2) {
    blake3_hash_many_avx2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    戻る;
  }
#endif
#もし !defined(BLAKE3_NO_SSE41)
  もし (features & SSE41) {
    blake3_hash_many_sse41(inputs, num_inputs, blocks, key, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    戻る;
  }
#endif
#もし !defined(BLAKE3_NO_SSE2)
  もし (features & SSE2) {
    blake3_hash_many_sse2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    戻る;
  }
#endif
#endif

#もし BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, num_inputs, blocks, key, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  戻る;
#endif

  blake3_hash_many_portable(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(void) {
#もし defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#もし !defined(BLAKE3_NO_AVX512)
  もし ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    戻る 16;
  }
#endif
#もし !defined(BLAKE3_NO_AVX2)
  もし (features & AVX2) {
    戻る 8;
  }
#endif
#もし !defined(BLAKE3_NO_SSE41)
  もし (features & SSE41) {
    戻る 4;
  }
#endif
#もし !defined(BLAKE3_NO_SSE2)
  もし (features & SSE2) {
    戻る 4;
  }
#endif
#endif
#もし BLAKE3_USE_NEON == 1
  戻る 4;
#endif
  戻る 1;
}
