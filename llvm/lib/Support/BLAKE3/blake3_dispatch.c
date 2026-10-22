#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "blake3_impl.h"

#만약 defined(_MSC_VER)
#include <Windows.h>
#endif

#만약 defined(IS_X86)
#만약 defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)
#include <immintrin.h>
#아니면
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#만약 !defined(BLAKE3_ATOMICS)
#만약 defined(__has_include)
#만약 __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#아니면
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#아니면
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#만약 BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic 정수
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#elif defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#아니면
#define ATOMIC_INT 정수
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (void)((x))

#만약 defined(IS_X86)
static uint64_t xgetbv(void) {
#만약 defined(_MSC_VER)
  반환 _xgetbv(0);
#아니면
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  반환 ((uint64_t)edx << 32) | eax;
#endif
}

static void cpuid(uint32_t out[4], uint32_t id) {
#만약 defined(_MSC_VER)
  __cpuid((정수 *)out, id);
#elif defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#아니면
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

static void cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#만약 defined(_MSC_VER)
  __cpuidex((정수 *)out, id, sid);
#elif defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#아니면
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

#만약 !defined(BLAKE3_TESTING)
static /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#만약 !defined(BLAKE3_TESTING)
static
#endif
    enum cpu_feature
    get_cpu_features(void) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  enum cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  만약 (features != UNDEFINED) {
    반환 features;
  } 아니면 {
#만약 defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (void)edx;
    features = 0;
    cpuid(regs, 0);
    const 정수 max_id = *eax;
    cpuid(regs, 1);
#만약 defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#아니면
    만약 (*edx & (1UL << 26))
      features |= SSE2;
#endif
    만약 (*ecx & (1UL << 9))
      features |= SSSE3;
    만약 (*ecx & (1UL << 19))
      features |= SSE41;

    만약 (*ecx & (1UL << 27)) { // OSXSAVE
      const uint64_t mask = xgetbv();
      만약 ((mask & 6) == 6) { // SSE and AVX states
        만약 (*ecx & (1UL << 28))
          features |= AVX;
        만약 (max_id >= 7) {
          cpuidex(regs, 7, 0);
          만약 (*ebx & (1UL << 5))
            features |= AVX2;
          만약 ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            만약 (*ebx & (1UL << 31))
              features |= AVX512VL;
            만약 (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    반환 features;
#아니면
    /* How to detect NEON? */
    반환 0;
#endif
  }
}

void blake3_compress_in_place(uint32_t cv[8],
                              const uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#만약 defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#만약 !defined(BLAKE3_NO_AVX512)
  만약 (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    반환;
  }
#endif
#만약 !defined(BLAKE3_NO_SSE41)
  만약 (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    반환;
  }
#endif
#만약 !defined(BLAKE3_NO_SSE2)
  만약 (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    반환;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

void blake3_compress_xof(const uint32_t cv[8],
                         const uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#만약 defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#만약 !defined(BLAKE3_NO_AVX512)
  만약 (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    반환;
  }
#endif
#만약 !defined(BLAKE3_NO_SSE41)
  만약 (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    반환;
  }
#endif
#만약 !defined(BLAKE3_NO_SSE2)
  만약 (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    반환;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


void blake3_xof_many(const uint32_t cv[8],
                     const uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  만약 (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    반환;
  }
#만약 defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#만약 !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  만약 (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    반환;
  }
#endif
#endif
  위해(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

void blake3_hash_many(const uint8_t *const *inputs, size_t num_inputs,
                      size_t blocks, const uint32_t key[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#만약 defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#만약 !defined(BLAKE3_NO_AVX512)
  만약 ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    반환;
  }
#endif
#만약 !defined(BLAKE3_NO_AVX2)
  만약 (features & AVX2) {
    blake3_hash_many_avx2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    반환;
  }
#endif
#만약 !defined(BLAKE3_NO_SSE41)
  만약 (features & SSE41) {
    blake3_hash_many_sse41(inputs, num_inputs, blocks, key, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    반환;
  }
#endif
#만약 !defined(BLAKE3_NO_SSE2)
  만약 (features & SSE2) {
    blake3_hash_many_sse2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    반환;
  }
#endif
#endif

#만약 BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, num_inputs, blocks, key, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  반환;
#endif

  blake3_hash_many_portable(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(void) {
#만약 defined(IS_X86)
  const enum cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#만약 !defined(BLAKE3_NO_AVX512)
  만약 ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    반환 16;
  }
#endif
#만약 !defined(BLAKE3_NO_AVX2)
  만약 (features & AVX2) {
    반환 8;
  }
#endif
#만약 !defined(BLAKE3_NO_SSE41)
  만약 (features & SSE41) {
    반환 4;
  }
#endif
#만약 !defined(BLAKE3_NO_SSE2)
  만약 (features & SSE2) {
    반환 4;
  }
#endif
#endif
#만약 BLAKE3_USE_NEON == 1
  반환 4;
#endif
  반환 1;
}
