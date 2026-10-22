#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "blake3_impl.h"

#ຖ້າ defined(_MSC_VER)
#include <Windows.h>
#endif

#ຖ້າ defined(IS_X86)
#ຖ້າ defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)
#include <immintrin.h>
#ບໍ່ດັ່ງນັ້ນ
#undef IS_X86 /* Unimplemented! */
#endif
#endif

#ຖ້າ !defined(BLAKE3_ATOMICS)
#ຖ້າ defined(__has_include)
#ຖ້າ __has_include(<stdatomic.h>) && !defined(_MSC_VER)
#define BLAKE3_ATOMICS 1
#ບໍ່ດັ່ງນັ້ນ
#define BLAKE3_ATOMICS 0
#endif /* __has_include(<stdatomic.h>) && !defined(_MSC_VER) */
#ບໍ່ດັ່ງນັ້ນ
#define BLAKE3_ATOMICS 0
#endif /* defined(__has_include) */
#endif /* BLAKE3_ATOMICS */

#ຖ້າ BLAKE3_ATOMICS
#define ATOMIC_INT _Atomic ຈຳນວນເຕັມ
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#elif defined(_MSC_VER)
#define ATOMIC_INT LONG
#define ATOMIC_LOAD(x) InterlockedOr(&x, 0)
#define ATOMIC_STORE(x, y) InterlockedExchange(&x, y)
#ບໍ່ດັ່ງນັ້ນ
#define ATOMIC_INT ຈຳນວນເຕັມ
#define ATOMIC_LOAD(x) x
#define ATOMIC_STORE(x, y) x = y
#endif

#define MAYBE_UNUSED(x) (void)((x))

#ຖ້າ defined(IS_X86)
static uint64_t xgetbv(void) {
#ຖ້າ defined(_MSC_VER)
  ສົ່ງຄືນ _xgetbv(0);
#ບໍ່ດັ່ງນັ້ນ
  uint32_t eax = 0, edx = 0;
  __asm__ __volatile__("xgetbv\n" : "=a"(eax), "=d"(edx) : "c"(0));
  ສົ່ງຄືນ ((uint64_t)edx << 32) | eax;
#endif
}

static void cpuid(uint32_t out[4], uint32_t id) {
#ຖ້າ defined(_MSC_VER)
  __cpuid((ຈຳນວນເຕັມ *)out, id);
#elif defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#ບໍ່ດັ່ງນັ້ນ
  __asm__ __volatile__("cpuid\n"
                       : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id));
#endif
}

static void cpuidex(uint32_t out[4], uint32_t id, uint32_t sid) {
#ຖ້າ defined(_MSC_VER)
  __cpuidex((ຈຳນວນເຕັມ *)out, id, sid);
#elif defined(__i386__) || defined(_M_IX86)
  __asm__ __volatile__("movl %%ebx, %1\n"
                       "cpuid\n"
                       "xchgl %1, %%ebx\n"
                       : "=a"(out[0]), "=r"(out[1]), "=c"(out[2]), "=d"(out[3])
                       : "a"(id), "c"(sid));
#ບໍ່ດັ່ງນັ້ນ
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

#ຖ້າ !defined(BLAKE3_TESTING)
static /* Allow the variable to be controlled manually for testing */
#endif
    ATOMIC_INT g_cpu_features = UNDEFINED;

LLVM_ATTRIBUTE_USED
#ຖ້າ !defined(BLAKE3_TESTING)
static
#endif
    ga_nanaa cpu_feature
    get_cpu_features(void) {

  /* If TSAN detects a data race here, try compiling with -DBLAKE3_ATOMICS=1 */
  ga_nanaa cpu_feature features = ATOMIC_LOAD(g_cpu_features);
  ຖ້າ (features != UNDEFINED) {
    ສົ່ງຄືນ features;
  } ບໍ່ດັ່ງນັ້ນ {
#ຖ້າ defined(IS_X86)
    uint32_t regs[4] = {0};
    uint32_t *eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    (void)edx;
    features = 0;
    cpuid(regs, 0);
    const ຈຳນວນເຕັມ max_id = *eax;
    cpuid(regs, 1);
#ຖ້າ defined(__amd64__) || defined(_M_X64)
    features |= SSE2;
#ບໍ່ດັ່ງນັ້ນ
    ຖ້າ (*edx & (1UL << 26))
      features |= SSE2;
#endif
    ຖ້າ (*ecx & (1UL << 9))
      features |= SSSE3;
    ຖ້າ (*ecx & (1UL << 19))
      features |= SSE41;

    ຖ້າ (*ecx & (1UL << 27)) { // OSXSAVE
      const uint64_t mask = xgetbv();
      ຖ້າ ((mask & 6) == 6) { // SSE and AVX states
        ຖ້າ (*ecx & (1UL << 28))
          features |= AVX;
        ຖ້າ (max_id >= 7) {
          cpuidex(regs, 7, 0);
          ຖ້າ (*ebx & (1UL << 5))
            features |= AVX2;
          ຖ້າ ((mask & 224) == 224) { // Opmask, ZMM_Hi256, Hi16_Zmm
            ຖ້າ (*ebx & (1UL << 31))
              features |= AVX512VL;
            ຖ້າ (*ebx & (1UL << 16))
              features |= AVX512F;
          }
        }
      }
    }
    ATOMIC_STORE(g_cpu_features, features);
    ສົ່ງຄືນ features;
#ບໍ່ດັ່ງນັ້ນ
    /* How to detect NEON? */
    ສົ່ງຄືນ 0;
#endif
  }
}

void blake3_compress_in_place(uint32_t cv[8],
                              const uint8_t block[BLAKE3_BLOCK_LEN],
                              uint8_t block_len, uint64_t counter,
                              uint8_t flags) {
#ຖ້າ defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ຖ້າ !defined(BLAKE3_NO_AVX512)
  ຖ້າ (features & AVX512VL) {
    blake3_compress_in_place_avx512(cv, block, block_len, counter, flags);
    ສົ່ງຄືນ;
  }
#endif
#ຖ້າ !defined(BLAKE3_NO_SSE41)
  ຖ້າ (features & SSE41) {
    blake3_compress_in_place_sse41(cv, block, block_len, counter, flags);
    ສົ່ງຄືນ;
  }
#endif
#ຖ້າ !defined(BLAKE3_NO_SSE2)
  ຖ້າ (features & SSE2) {
    blake3_compress_in_place_sse2(cv, block, block_len, counter, flags);
    ສົ່ງຄືນ;
  }
#endif
#endif
  blake3_compress_in_place_portable(cv, block, block_len, counter, flags);
}

void blake3_compress_xof(const uint32_t cv[8],
                         const uint8_t block[BLAKE3_BLOCK_LEN],
                         uint8_t block_len, uint64_t counter, uint8_t flags,
                         uint8_t out[64]) {
#ຖ້າ defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ຖ້າ !defined(BLAKE3_NO_AVX512)
  ຖ້າ (features & AVX512VL) {
    blake3_compress_xof_avx512(cv, block, block_len, counter, flags, out);
    ສົ່ງຄືນ;
  }
#endif
#ຖ້າ !defined(BLAKE3_NO_SSE41)
  ຖ້າ (features & SSE41) {
    blake3_compress_xof_sse41(cv, block, block_len, counter, flags, out);
    ສົ່ງຄືນ;
  }
#endif
#ຖ້າ !defined(BLAKE3_NO_SSE2)
  ຖ້າ (features & SSE2) {
    blake3_compress_xof_sse2(cv, block, block_len, counter, flags, out);
    ສົ່ງຄືນ;
  }
#endif
#endif
  blake3_compress_xof_portable(cv, block, block_len, counter, flags, out);
}


void blake3_xof_many(const uint32_t cv[8],
                     const uint8_t block[BLAKE3_BLOCK_LEN],
                     uint8_t block_len, uint64_t counter, uint8_t flags,
                     uint8_t out[64], size_t outblocks) {
  ຖ້າ (outblocks == 0) {
    // The current assembly implementation always outputs at least 1 block.
    ສົ່ງຄືນ;
  }
#ຖ້າ defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ຖ້າ !defined(_WIN32) && !defined(__CYGWIN__) && !defined(BLAKE3_NO_AVX512)
  ຖ້າ (features & AVX512VL) {
    blake3_xof_many_avx512(cv, block, block_len, counter, flags, out, outblocks);
    ສົ່ງຄືນ;
  }
#endif
#endif
  ສຳລັບ(size_t i = 0; i < outblocks; ++i) {
    blake3_compress_xof(cv, block, block_len, counter + i, flags, out + 64*i);
  }
}

void blake3_hash_many(const uint8_t *const *inputs, size_t num_inputs,
                      size_t blocks, const uint32_t key[8], uint64_t counter,
                      bool increment_counter, uint8_t flags,
                      uint8_t flags_start, uint8_t flags_end, uint8_t *out) {
#ຖ້າ defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ຖ້າ !defined(BLAKE3_NO_AVX512)
  ຖ້າ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    blake3_hash_many_avx512(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
    ສົ່ງຄືນ;
  }
#endif
#ຖ້າ !defined(BLAKE3_NO_AVX2)
  ຖ້າ (features & AVX2) {
    blake3_hash_many_avx2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ສົ່ງຄືນ;
  }
#endif
#ຖ້າ !defined(BLAKE3_NO_SSE41)
  ຖ້າ (features & SSE41) {
    blake3_hash_many_sse41(inputs, num_inputs, blocks, key, counter,
                           increment_counter, flags, flags_start, flags_end,
                           out);
    ສົ່ງຄືນ;
  }
#endif
#ຖ້າ !defined(BLAKE3_NO_SSE2)
  ຖ້າ (features & SSE2) {
    blake3_hash_many_sse2(inputs, num_inputs, blocks, key, counter,
                          increment_counter, flags, flags_start, flags_end,
                          out);
    ສົ່ງຄືນ;
  }
#endif
#endif

#ຖ້າ BLAKE3_USE_NEON == 1
  blake3_hash_many_neon(inputs, num_inputs, blocks, key, counter,
                        increment_counter, flags, flags_start, flags_end, out);
  ສົ່ງຄືນ;
#endif

  blake3_hash_many_portable(inputs, num_inputs, blocks, key, counter,
                            increment_counter, flags, flags_start, flags_end,
                            out);
}

// The dynamically detected SIMD degree of the current platform.
size_t blake3_simd_degree(void) {
#ຖ້າ defined(IS_X86)
  const ga_nanaa cpu_feature features = get_cpu_features();
  MAYBE_UNUSED(features);
#ຖ້າ !defined(BLAKE3_NO_AVX512)
  ຖ້າ ((features & (AVX512F|AVX512VL)) == (AVX512F|AVX512VL)) {
    ສົ່ງຄືນ 16;
  }
#endif
#ຖ້າ !defined(BLAKE3_NO_AVX2)
  ຖ້າ (features & AVX2) {
    ສົ່ງຄືນ 8;
  }
#endif
#ຖ້າ !defined(BLAKE3_NO_SSE41)
  ຖ້າ (features & SSE41) {
    ສົ່ງຄືນ 4;
  }
#endif
#ຖ້າ !defined(BLAKE3_NO_SSE2)
  ຖ້າ (features & SSE2) {
    ສົ່ງຄືນ 4;
  }
#endif
#endif
#ຖ້າ BLAKE3_USE_NEON == 1
  ສົ່ງຄືນ 4;
#endif
  ສົ່ງຄືນ 1;
}
