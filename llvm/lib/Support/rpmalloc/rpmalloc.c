//===---------------------- rpmalloc.c ------------------*- C -*-=============//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This library provides a cross-platform lock free thread caching malloc
// implementation in C11.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#ಆದರೆ defined(__clang__)
#ಸೂಚನೆ clang diagnostic ignored "-Wunused-macros"
#ಸೂಚನೆ clang diagnostic ignored "-Wunused-function"
#ಆದರೆ __has_warning("-Wreserved-identifier")
#ಸೂಚನೆ clang diagnostic ignored "-Wreserved-identifier"
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ __has_warning("-Wstatic-in-inline")
#ಸೂಚನೆ clang diagnostic ignored "-Wstatic-in-inline"
#ಆದರೆ_ಅಂತ್ಯ
#ಅಥವಾಆದರೆ defined(__GNUC__)
#ಸೂಚನೆ GCC diagnostic ignored "-Wunused-macros"
#ಸೂಚನೆ GCC diagnostic ignored "-Wunused-function"
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(__has_builtin)
#ವ್ಯಾಖ್ಯಾನಿಸು __has_builtin(b) 0
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__GNUC__) || defined(__clang__)

#ಆದರೆ __has_builtin(__builtin_memcpy_inline)
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_memcpy_const(x, y, s)                                        \
  ಮಾಡು {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    ಸ್ಮರಣೆ_ನಕಲು(x, y, s);                                                           \
  } ಆಗ (0)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ __has_builtin(__builtin_memset_inline)
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_memset_const(x, y, s)                                        \
  ಮಾಡು {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } ಆಗ (0)
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_memcpy_const(x, y, s) ಸ್ಮರಣೆ_ನಕಲು(x, y, s)
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ __has_builtin(__builtin_assume)
#ವ್ಯಾಖ್ಯಾನಿಸು rpmalloc_assume(cond) __builtin_assume(cond)
#ಅಥವಾಆದರೆ defined(__GNUC__)
#ವ್ಯಾಖ್ಯಾನಿಸು rpmalloc_assume(cond)                                                  \
  ಮಾಡು {                                                                         \
    ಆದರೆ (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } ಆಗ (0)
#ಅಥವಾಆದರೆ defined(_MSC_VER)
#ವ್ಯಾಖ್ಯಾನಿಸು rpmalloc_assume(cond) __assume(cond)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು rpmalloc_assume(cond) 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ರಾಶಿ_ARRAY_ಗಾತ್ರ
//! Size of heap hashmap
#ವ್ಯಾಖ್ಯಾನಿಸು ರಾಶಿ_ARRAY_ಗಾತ್ರ 47
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ENABLE_THREAD_CACHE
//! Enable per-thread cache
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_THREAD_CACHE 1
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ENABLE_GLOBAL_CACHE
//! Enable global cache shared between all threads, requires thread cache
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_GLOBAL_CACHE 1
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ENABLE_VALIDATE_ವಾದಗಳು
//! Enable validation of args to public entry points
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_VALIDATE_ವಾದಗಳು 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ENABLE_STATISTICS
//! Enable statistics collection
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_STATISTICS 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ENABLE_ASSERTS
//! Enable asserts
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_ASSERTS 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ENABLE_OVERRIDE
//! Override standard library malloc/free and new/delete entry points
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_OVERRIDE 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ENABLE_PRELOAD
//! Support preloading
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_PRELOAD 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ DISABLE_UNMAP
//! Disable unmapping memory pages (also enables unlimited cache)
#ವ್ಯಾಖ್ಯಾನಿಸು DISABLE_UNMAP 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ENABLE_UNLIMITED_CACHE
//! Enable unlimited global cache (no unmapping until finalization)
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_UNLIMITED_CACHE 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ENABLE_ADAPTIVE_THREAD_CACHE
//! Enable adaptive thread cache size based on use heuristics
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_ADAPTIVE_THREAD_CACHE 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ DEFAULT_ಹರವು_MAP_ಎಣಿಕೆ
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#ವ್ಯಾಖ್ಯಾನಿಸು DEFAULT_ಹರವು_MAP_ಎಣಿಕೆ 64
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#ವ್ಯಾಖ್ಯಾನಿಸು GLOBAL_CACHE_MULTIPLIER 8
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#ದೋಷ Must use global cache ಆದರೆ unmap is disabled
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ DISABLE_UNMAP
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ENABLE_UNLIMITED_CACHE
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_UNLIMITED_CACHE 1
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !ENABLE_GLOBAL_CACHE
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ENABLE_UNLIMITED_CACHE
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_UNLIMITED_CACHE 0
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !ENABLE_THREAD_CACHE
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ENABLE_ADAPTIVE_THREAD_CACHE
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_ADAPTIVE_THREAD_CACHE 0
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#ವ್ಯಾಖ್ಯಾನಿಸು PLATFORM_WINDOWS 1
#ವ್ಯಾಖ್ಯಾನಿಸು PLATFORM_POSIX 0
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು PLATFORM_WINDOWS 0
#ವ್ಯಾಖ್ಯಾನಿಸು PLATFORM_POSIX 1
#ಆದರೆ_ಅಂತ್ಯ

/// Platform and arch specifics
#ಆದರೆ defined(_MSC_VER) && !defined(__clang__)
#ಸೂಚನೆ warning(disable : 5105)
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FORCEINLINE
#ವ್ಯಾಖ್ಯಾನಿಸು FORCEINLINE ಸಾಲೊಳಗೆ __forceinline
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು _Static_assert static_assert
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FORCEINLINE
#ವ್ಯಾಖ್ಯಾನಿಸು FORCEINLINE ಸಾಲೊಳಗೆ __attribute__((__always_inline__))
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ PLATFORM_WINDOWS
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ WIN32_LEAN_AND_ಸರಾಸರಿ
#ವ್ಯಾಖ್ಯಾನಿಸು WIN32_LEAN_AND_ಸರಾಸರಿ
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <windows.h>
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
#ಅನ್ತರ್ಭಾವ <intsafe.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಆದರೆ defined(__linux__) || defined(__ANDROID__)
#ಅನ್ತರ್ಭಾವ <sys/prctl.h>
#ಆದರೆ !defined(PR_ಗಣ_VMA)
#ವ್ಯಾಖ್ಯಾನಿಸು PR_ಗಣ_VMA 0x53564d41
#ವ್ಯಾಖ್ಯಾನಿಸು PR_ಗಣ_VMA_ANON_NAME 0
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(__APPLE__)
#ಅನ್ತರ್ಭಾವ <TargetConditionals.h>
#ಆದರೆ !ಗುರಿ_OS_IPHONE && !ಗುರಿ_OS_SIMULATOR
#ಅನ್ತರ್ಭಾವ <mach/mach_vm.h>
#ಅನ್ತರ್ಭಾವ <mach/vm_statistics.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <pthread.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(__HAIKU__) || defined(__TINYC__)
#ಅನ್ತರ್ಭಾವ <pthread.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಆದರೆ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#ಅನ್ತರ್ಭಾವ <fibersapi.h>
ಸ್ಥಿರ DWORD fls_ಕೀಲಿ;
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ PLATFORM_POSIX
#ಅನ್ತರ್ಭಾವ <sched.h>
#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __FreeBSD__
#ಅನ್ತರ್ಭಾವ <sys/sysctl.h>
#ವ್ಯಾಖ್ಯಾನಿಸು MAP_HUGETLB MAP_ALIGNED_SUPER
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ PROT_MAX
#ವ್ಯಾಖ್ಯಾನಿಸು PROT_MAX(f) 0
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು PROT_MAX(f) 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __sun
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ madvise(caddr_t, size_t, ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ MAP_UNINITIALIZED
#ವ್ಯಾಖ್ಯಾನಿಸು MAP_UNINITIALIZED 0
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <errno.h>

#ಆದರೆ ENABLE_ASSERTS
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು NDEBUG
#ಆದರೆ defined(_MSC_VER) && !defined(_DEBUG)
#ವ್ಯಾಖ್ಯಾನಿಸು _DEBUG
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <assert.h>
#ವ್ಯಾಖ್ಯಾನಿಸು RPMALLOC_TOSTRING_M(x) #x
#ವ್ಯಾಖ್ಯಾನಿಸು RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#ವ್ಯಾಖ್ಯಾನಿಸು rpmalloc_assert(truth, ಸಂದೇಶ_2)                                        \
  ಮಾಡು {                                                                         \
    ಆದರೆ (!(truth)) {                                                            \
      ಆದರೆ (ಸ್ಮರಣೆ_config.error_callback) {                                     \
        ಸ್ಮರಣೆ_config.error_callback(ಸಂದೇಶ_2 " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } ಇಲ್ಲದಿದ್ದರೆ {                                                                 \
        ಪ್ರತಿಜ್ಞಾ((truth) && ಸಂದೇಶ_2);                                            \
      }                                                                        \
    }                                                                          \
  } ಆಗ (0)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು rpmalloc_assert(truth, ಸಂದೇಶ_2)                                        \
  ಮಾಡು {                                                                         \
  } ಆಗ (0)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_STATISTICS
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಆದರೆ_ಅಂತ್ಯ

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#ಆದರೆ defined(_MSC_VER) && !defined(__clang__)

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಅಸ್ಥಿರ ದೀರ್ಘ atomic32_t;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಅಸ್ಥಿರ ದೀರ್ಘ ದೀರ್ಘ atomic64_t;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಅಸ್ಥಿರ ಶೂನ್ಯ *atomicptr_t;

ಸ್ಥಿರ FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  ಹಿಂತಿರುಗಿ (int32_t)InterlockedOr(src, 0);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ಸ್ಥಿರ FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  ಹಿಂತಿರುಗಿ (int32_t)InterlockedIncrement(val);
}
ಸ್ಥಿರ FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  ಹಿಂತಿರುಗಿ (int32_t)InterlockedDecrement(val);
}
ಸ್ಥಿರ FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t ಕೂಡಿಸು) {
  ಹಿಂತಿರುಗಿ (int32_t)Interlocked_Exchange_ಕೂಡಿಸು(val, ಕೂಡಿಸು) + ಕೂಡಿಸು;
}
ಸ್ಥಿರ FORCEINLINE ಪೂರ್ಣಾಂಕ atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  ಹಿಂತಿರುಗಿ (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ಸ್ಥಿರ FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  ಹಿಂತಿರುಗಿ (int64_t)InterlockedOr64(src, 0);
}
ಸ್ಥಿರ FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t ಕೂಡಿಸು) {
  ಹಿಂತಿರುಗಿ (int64_t)InterlockedExchangeAdd64(val, ಕೂಡಿಸು) + ಕೂಡಿಸು;
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ *atomic_ತುಂಬು_ptr(atomicptr_t *src) {
  ಹಿಂತಿರುಗಿ Interlocked_Compare_Exchange_ಸೂಚಕ(src, 0, 0);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store_ptr(atomicptr_t *dst, ಶೂನ್ಯ *val) {
  Interlocked_Exchange_ಸೂಚಕ(dst, val);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store_ptr_release(atomicptr_t *dst, ಶೂನ್ಯ *val) {
  Interlocked_Exchange_ಸೂಚಕ(dst, val);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     ಶೂನ್ಯ *val) {
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)Interlocked_Exchange_ಸೂಚಕ((ಶೂನ್ಯ *ಅಸ್ಥಿರ *)dst, val);
}
ಸ್ಥಿರ FORCEINLINE ಪೂರ್ಣಾಂಕ atomic_cas_ptr(atomicptr_t *dst, ಶೂನ್ಯ *val, ಶೂನ್ಯ *ref) {
  ಹಿಂತಿರುಗಿ (Interlocked_Compare_Exchange_ಸೂಚಕ((ಶೂನ್ಯ *ಅಸ್ಥಿರ *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#ವ್ಯಾಖ್ಯಾನಿಸು EXPECTED(x) (x)
#ವ್ಯಾಖ್ಯಾನಿಸು UNEXPECTED(x) (x)

#ಇಲ್ಲದಿದ್ದರೆ

#ಅನ್ತರ್ಭಾವ <stdatomic.h>

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಅಸ್ಥಿರ _Atomic(int32_t) atomic32_t;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಅಸ್ಥಿರ _Atomic(int64_t) atomic64_t;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಅಸ್ಥಿರ _Atomic(ಶೂನ್ಯ *) atomicptr_t;

ಸ್ಥಿರ FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  ಹಿಂತಿರುಗಿ atomic_ತುಂಬು_explicit(src, memory_order_relaxed);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ಸ್ಥಿರ FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  ಹಿಂತಿರುಗಿ atomic_fetch_ಕೂಡಿಸು_explicit(val, 1, memory_order_relaxed) + 1;
}
ಸ್ಥಿರ FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  ಹಿಂತಿರುಗಿ atomic_fetch_ಕೂಡಿಸು_explicit(val, -1, memory_order_relaxed) - 1;
}
ಸ್ಥಿರ FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t ಕೂಡಿಸು) {
  ಹಿಂತಿರುಗಿ atomic_fetch_ಕೂಡಿಸು_explicit(val, ಕೂಡಿಸು, memory_order_relaxed) + ಕೂಡಿಸು;
}
ಸ್ಥಿರ FORCEINLINE ಪೂರ್ಣಾಂಕ atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  ಹಿಂತಿರುಗಿ atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ಸ್ಥಿರ FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  ಹಿಂತಿರುಗಿ atomic_ತುಂಬು_explicit(val, memory_order_relaxed);
}
ಸ್ಥಿರ FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t ಕೂಡಿಸು) {
  ಹಿಂತಿರುಗಿ atomic_fetch_ಕೂಡಿಸು_explicit(val, ಕೂಡಿಸು, memory_order_relaxed) + ಕೂಡಿಸು;
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ *atomic_ತುಂಬು_ptr(atomicptr_t *src) {
  ಹಿಂತಿರುಗಿ atomic_ತುಂಬು_explicit(src, memory_order_relaxed);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store_ptr(atomicptr_t *dst, ಶೂನ್ಯ *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store_ptr_release(atomicptr_t *dst, ಶೂನ್ಯ *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     ಶೂನ್ಯ *val) {
  ಹಿಂತಿರುಗಿ atomic_exchange_explicit(dst, val, memory_order_acquire);
}
ಸ್ಥಿರ FORCEINLINE ಪೂರ್ಣಾಂಕ atomic_cas_ptr(atomicptr_t *dst, ಶೂನ್ಯ *val, ಶೂನ್ಯ *ref) {
  ಹಿಂತಿರುಗಿ atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_relaxed, memory_order_relaxed);
}

#ವ್ಯಾಖ್ಯಾನಿಸು EXPECTED(x) __builtin_expect((x), 1)
#ವ್ಯಾಖ್ಯಾನಿಸು UNEXPECTED(x) __builtin_expect((x), 0)

#ಆದರೆ_ಅಂತ್ಯ

////////////
///
/// Statistics related functions (evaluate to nothing when statistics not
/// enabled)
///
//////

#ಆದರೆ ENABLE_STATISTICS
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#ವ್ಯಾಖ್ಯಾನಿಸು rpmalloc_stat_ಕೂಡಿಸು(counter, ಮೌಲ್ಯ_2)                                     \
  atomic_add32(counter, (int32_t)(ಮೌಲ್ಯ_2))
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_stat_add64(counter, ಮೌಲ್ಯ_2)                                   \
  atomic_add64(counter, (int64_t)(ಮೌಲ್ಯ_2))
#ವ್ಯಾಖ್ಯಾನಿಸು rpmalloc_stat_ಕೂಡಿಸು_peak(counter, ಮೌಲ್ಯ_2, peak)                          \
  ಮಾಡು {                                                                         \
    int32_t cur_ಎಣಿಕೆ = atomic_add32(counter, (int32_t)(ಮೌಲ್ಯ_2));              \
    ಆದರೆ (cur_ಎಣಿಕೆ > (peak))                                                   \
      peak = cur_ಎಣಿಕೆ;                                                       \
  } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_stat_sub(counter, ಮೌಲ್ಯ_2)                                     \
  atomic_add32(counter, -(int32_t)(ಮೌಲ್ಯ_2))
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_stat_inc_alloc(ರಾಶಿ_2, class_idx)                              \
  ಮಾಡು {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].alloc_current);         \
    ಆದರೆ (alloc_current > ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].alloc_peak)            \
      ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].alloc_ಒಟ್ಟು);               \
  } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_stat_inc_free(ರಾಶಿ_2, class_idx)                               \
  ಮಾಡು {                                                                         \
    atomic_decr32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].alloc_current);             \
    atomic_incr32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].free_ಒಟ್ಟು);                \
  } ಆಗ (0)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_stat_inc(counter)                                            \
  ಮಾಡು {                                                                         \
  } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_stat_dec(counter)                                            \
  ಮಾಡು {                                                                         \
  } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು rpmalloc_stat_ಕೂಡಿಸು(counter, ಮೌಲ್ಯ_2)                                     \
  ಮಾಡು {                                                                         \
  } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_stat_add64(counter, ಮೌಲ್ಯ_2)                                   \
  ಮಾಡು {                                                                         \
  } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು rpmalloc_stat_ಕೂಡಿಸು_peak(counter, ಮೌಲ್ಯ_2, peak)                          \
  ಮಾಡು {                                                                         \
  } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_stat_sub(counter, ಮೌಲ್ಯ_2)                                     \
  ಮಾಡು {                                                                         \
  } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_stat_inc_alloc(ರಾಶಿ_2, class_idx)                              \
  ಮಾಡು {                                                                         \
  } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು _rpmalloc_stat_inc_free(ರಾಶಿ_2, class_idx)                               \
  ಮಾಡು {                                                                         \
  } ಆಗ (0)
#ಆದರೆ_ಅಂತ್ಯ

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#ವ್ಯಾಖ್ಯಾನಿಸು SMALL_GRANULARITY 16
//! Small granularity shift count
#ವ್ಯಾಖ್ಯಾನಿಸು SMALL_GRANULARITY_ಸರಿಸು 4
//! Number of small block size classes
#ವ್ಯಾಖ್ಯಾನಿಸು SMALL_CLASS_ಎಣಿಕೆ 65
//! Maximum size of a small block
#ವ್ಯಾಖ್ಯಾನಿಸು SMALL_ಗಾತ್ರ_ಎಲ್ಲೆ (SMALL_GRANULARITY * (SMALL_CLASS_ಎಣಿಕೆ - 1))
//! Granularity of a medium allocation block
#ವ್ಯಾಖ್ಯಾನಿಸು MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#ವ್ಯಾಖ್ಯಾನಿಸು MEDIUM_GRANULARITY_ಸರಿಸು 9
//! Number of medium block size classes
#ವ್ಯಾಖ್ಯಾನಿಸು MEDIUM_CLASS_ಎಣಿಕೆ 61
//! Total number of small + medium size classes
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಾತ್ರ_CLASS_ಎಣಿಕೆ (SMALL_CLASS_ಎಣಿಕೆ + MEDIUM_CLASS_ಎಣಿಕೆ)
//! Number of large block size classes
#ವ್ಯಾಖ್ಯಾನಿಸು LARGE_CLASS_ಎಣಿಕೆ 63
//! Maximum size of a medium block
#ವ್ಯಾಖ್ಯಾನಿಸು MEDIUM_ಗಾತ್ರ_ಎಲ್ಲೆ                                                      \
  (SMALL_ಗಾತ್ರ_ಎಲ್ಲೆ + (MEDIUM_GRANULARITY * MEDIUM_CLASS_ಎಣಿಕೆ))
//! Maximum size of a large block
#ವ್ಯಾಖ್ಯಾನಿಸು LARGE_ಗಾತ್ರ_ಎಲ್ಲೆ                                                       \
  ((LARGE_CLASS_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) - ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ)
//! Size of a span header (must be a multiple of SMALL_GRANULARITY and a power
//! of two)
#ವ್ಯಾಖ್ಯಾನಿಸು ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ 128
//! Number of spans in thread cache
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_THREAD_ಹರವು_CACHE 400
//! Number of spans to transfer between thread and global cache
#ವ್ಯಾಖ್ಯಾನಿಸು THREAD_ಹರವು_CACHE_TRANSFER 64
//! Number of spans in thread cache for large spans (must be greater than
//! LARGE_CLASS_COUNT / 2)
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_THREAD_ಹರವು_LARGE_CACHE 100
//! Number of spans to transfer between thread and global cache for large spans
#ವ್ಯಾಖ್ಯಾನಿಸು THREAD_ಹರವು_LARGE_CACHE_TRANSFER 6

_Static_assert((SMALL_GRANULARITY & (SMALL_GRANULARITY - 1)) == 0,
               "Small granularity must be power of two");
_Static_assert((ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ & (ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ - 1)) == 0,
               "Span header size must be power of two");

#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
//! Maximum allocation size to avoid integer overflow
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು MAX_ALLOC_ಗಾತ್ರ
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_ALLOC_ಗಾತ್ರ (((size_t) - 1) - ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ptr, ofs) (ಶೂನ್ಯ *)((ಅಕ್ಷರ *)(ptr) + (ptrdiff_t)(ofs))
#ವ್ಯಾಖ್ಯಾನಿಸು ಸೂಚಕ_diff(ಮೊದಲ_2, ಎರಡನೇ_2)                                            \
  (ptrdiff_t)((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)(ಮೊದಲ_2) - (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)(ಎರಡನೇ_2))

#ವ್ಯಾಖ್ಯಾನಿಸು INVALID_ಸೂಚಕ ((ಶೂನ್ಯ *)((uintptr_t) - 1))

#ವ್ಯಾಖ್ಯಾನಿಸು ಗಾತ್ರ_CLASS_LARGE ಗಾತ್ರ_CLASS_ಎಣಿಕೆ
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಾತ್ರ_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ರಾಶಿ_t ರಾಶಿ_t;
//! Span of memory pages
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಹರವು_t ಹರವು_t;
//! Span list
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಹರವು_list_t ಹರವು_list_t;
//! Span active data
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಹರವು_ಸಕ್ರಿಯ_t ಹರವು_ಸಕ್ರಿಯ_t;
//! Size class definition
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಗಾತ್ರ_class_t ಗಾತ್ರ_class_t;
//! Global cache
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#ವ್ಯಾಖ್ಯಾನಿಸು ಹರವು_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#ವ್ಯಾಖ್ಯಾನಿಸು ಹರವು_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#ವ್ಯಾಖ್ಯಾನಿಸು ಹರವು_FLAG_ALIGNED_ಖಂಡಗಳು 4U
//! Flag indicating an unmapped master span
#ವ್ಯಾಖ್ಯಾನಿಸು ಹರವು_FLAG_UNMAPPED_MASTER 8U

#ಆದರೆ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
ರಚನೆ ಹರವು_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t ಮೇಲಿನ;
#ಆದರೆ ENABLE_STATISTICS
  //! Number of spans in deferred list
  atomic32_t spans_deferred;
  //! Number of spans transitioned to global cache
  atomic32_t spans_ಗಮ್ಯ_global;
  //! Number of spans transitioned from global cache
  atomic32_t spans_from_global;
  //! Number of spans transitioned to thread cache
  atomic32_t spans_ಗಮ್ಯ_cache;
  //! Number of spans transitioned from thread cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned to reserved state
  atomic32_t spans_ಗಮ್ಯ_reserved;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of raw memory map calls
  atomic32_t spans_map_calls;
#ಆದರೆ_ಅಂತ್ಯ
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಹರವು_use_t ಹರವು_use_t;
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_STATISTICS
ರಚನೆ ಗಾತ್ರ_class_use_t {
  //! Current number of allocations
  atomic32_t alloc_current;
  //! Peak number of allocations
  int32_t alloc_peak;
  //! Total number of allocations
  atomic32_t alloc_ಒಟ್ಟು;
  //! Total number of frees
  atomic32_t free_ಒಟ್ಟು;
  //! Number of spans in use
  atomic32_t spans_current;
  //! Number of spans transitioned to cache
  int32_t spans_peak;
  //! Number of spans transitioned to cache
  atomic32_t spans_ಗಮ್ಯ_cache;
  //! Number of spans transitioned from cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of spans mapped
  atomic32_t spans_map_calls;
  int32_t ಬಳಸದ_2;
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಗಾತ್ರ_class_use_t ಗಾತ್ರ_class_use_t;
#ಆದರೆ_ಅಂತ್ಯ

// A span can either represent a single span of memory pages with size declared
// by span_map_count configuration variable, or a set of spans in a continuous
// region, a super span. Any reference to the term "span" usually refers to both
// a single span or a super span. A super span can further be divided into
// multiple spans (or this, super spans), where the first (super)span is the
// master and subsequent (super)spans are subspans. The master span keeps track
// of how many subspans that are still alive and mapped in virtual memory, and
// once all subspans and master have been unmapped the entire superspan region
// is released and unmapped (on Windows for example, the entire superspan range
// has to be released in the same call to release the virtual memory range, but
// individual subranges can be decommitted individually to reduce physical
// memory use).
ರಚನೆ ಹರವು_t {
  //! Free list
  ಶೂನ್ಯ *free_list;
  //! Total block count of size class
  uint32_t ಖಂಡ_ಎಣಿಕೆ;
  //! Size class
  uint32_t ಗಾತ್ರ_class;
  //! Index of last block initialized in free list
  uint32_t free_list_ಎಲ್ಲೆ;
  //! Number of used blocks remaining when in partial state
  uint32_t ಬಳಸಿದ_ಎಣಿಕೆ;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_ಗಾತ್ರ;
  //! Size of a block
  uint32_t ಖಂಡ_ಗಾತ್ರ_2;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t ಹರವು_ಎಣಿಕೆ;
  //! Total span counter for master spans
  uint32_t ಒಟ್ಟು_spans;
  //! Offset from master span for subspans
  uint32_t ಸ್ಥಾನಾಂತರ_from_master;
  //! Remaining span counter, for master spans
  atomic32_t ಉಳಿದ_spans;
  //! Alignment offset
  uint32_t align_ಸ್ಥಾನಾಂತರ;
  //! Owning heap
  ರಾಶಿ_t *ರಾಶಿ_2;
  //! Next span
  ಹರವು_t *ಮುಂದಿನ;
  //! Previous span
  ಹರವು_t *prev;
};
_Static_assert(ಗಾತ್ರ(ಹರವು_t) <= ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ, "span size mismatch");

ರಚನೆ ಹರವು_cache_t {
  size_t ಎಣಿಕೆ_2;
  ಹರವು_t *ಹರವು[MAX_THREAD_ಹರವು_CACHE];
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಹರವು_cache_t ಹರವು_cache_t;

ರಚನೆ ಹರವು_large_cache_t {
  size_t ಎಣಿಕೆ_2;
  ಹರವು_t *ಹರವು[MAX_THREAD_ಹರವು_LARGE_CACHE];
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಹರವು_large_cache_t ಹರವು_large_cache_t;

ರಚನೆ ರಾಶಿ_ಗಾತ್ರ_class_t {
  //! Free list of active span
  ಶೂನ್ಯ *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  ಹರವು_t *partial_ಹರವು;
  //! Early level cache of fully free spans
  ಹರವು_t *cache;
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ರಾಶಿ_ಗಾತ್ರ_class_t ರಾಶಿ_ಗಾತ್ರ_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
ರಚನೆ ರಾಶಿ_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  ರಾಶಿ_ಗಾತ್ರ_class_t ಗಾತ್ರ_class[ಗಾತ್ರ_CLASS_ಎಣಿಕೆ];
#ಆದರೆ ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  ಹರವು_cache_t ಹರವು_cache;
#ಆದರೆ_ಅಂತ್ಯ
  //! List of deferred free spans (single linked list)
  atomicptr_t ಹರವು_free_deferred;
  //! Number of full spans
  size_t ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ;
  //! Mapped but unused spans
  ಹರವು_t *ಹರವು_reserve;
  //! Master span for mapped but unused spans
  ಹರವು_t *ಹರವು_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t ಮಗು_ಎಣಿಕೆ;
  //! Next heap in id list
  ರಾಶಿ_t *ಮುಂದಿನ_ರಾಶಿ;
  //! Next heap in orphan list
  ರಾಶಿ_t *ಮುಂದಿನ_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  ಪೂರ್ಣಾಂಕ finalize;
  //! Master heap owning the memory pages
  ರಾಶಿ_t *master_ರಾಶಿ;
#ಆದರೆ ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  ಹರವು_large_cache_t ಹರವು_large_cache[LARGE_CLASS_ಎಣಿಕೆ - 1];
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  ಹರವು_t *ತುಂಬಿದ_ಹರವು[ಗಾತ್ರ_CLASS_ಎಣಿಕೆ];
  //! Double linked list of large and huge spans allocated by this heap
  ಹರವು_t *large_huge_ಹರವು;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  ಹರವು_use_t ಹರವು_use[LARGE_CLASS_ಎಣಿಕೆ];
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_STATISTICS
  //! Allocation stats per size class
  ಗಾತ್ರ_class_use_t ಗಾತ್ರ_class_use[ಗಾತ್ರ_CLASS_ಎಣಿಕೆ + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_ಗಮ್ಯ_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_ಗಮ್ಯ_thread;
#ಆದರೆ_ಅಂತ್ಯ
};

// Size class for defining a block size bucket
ರಚನೆ ಗಾತ್ರ_class_t {
  //! Size of blocks in this class
  uint32_t ಖಂಡ_ಗಾತ್ರ_2;
  //! Number of blocks in each chunk
  uint16_t ಖಂಡ_ಎಣಿಕೆ;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(ಗಾತ್ರ(ಗಾತ್ರ_class_t) == 8, "Size class size mismatch");

ರಚನೆ global_cache_t {
  //! Cache lock
  atomic32_t ಬೀಗ_2;
  //! Cache count
  uint32_t ಎಣಿಕೆ_2;
#ಆದರೆ ENABLE_STATISTICS
  //! Insert count
  size_t ಸೇರಿಸು_ಎಣಿಕೆ;
  //! Extract count
  size_t ಹೊರತೆಗೆ_ಎಣಿಕೆ;
#ಆದರೆ_ಅಂತ್ಯ
  //! Cached spans
  ಹರವು_t *ಹರವು[GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_ಹರವು_CACHE];
  //! Unlimited cache overflow
  ಹರವು_t *ಅತಿಪ್ರವಾಹ_2;
};

////////////
///
/// Global data
///
//////

//! Default span size (64KiB)
#ವ್ಯಾಖ್ಯಾನಿಸು ಸ್ಮರಣೆ_default_ಹರವು_ಗಾತ್ರ (64 * 1024)
#ವ್ಯಾಖ್ಯಾನಿಸು ಸ್ಮರಣೆ_default_ಹರವು_ಗಾತ್ರ_ಸರಿಸು 16
#ವ್ಯಾಖ್ಯಾನಿಸು ಸ್ಮರಣೆ_default_ಹರವು_ಮುಖವಾಡ (~((uintptr_t)(ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ - 1)))

//! Initialized flag
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ _rpmalloc_initialized;
//! Main thread ID
ಸ್ಥಿರ uintptr_t _rpmalloc_main_thread_id;
//! Configuration
ಸ್ಥಿರ rpmalloc_config_t ಸ್ಮರಣೆ_config;
//! Memory page size
ಸ್ಥಿರ size_t ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
//! Shift to divide by page size
ಸ್ಥಿರ size_t ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ_ಸರಿಸು;
//! Granularity at which memory pages are mapped by OS
ಸ್ಥಿರ size_t ಸ್ಮರಣೆ_map_granularity;
#ಆದರೆ RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
ಸ್ಥಿರ size_t ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ;
//! Shift to divide by span size
ಸ್ಥಿರ size_t ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ_ಸರಿಸು;
//! Mask to get to start of a memory span
ಸ್ಥಿರ uintptr_t ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ;
#ಇಲ್ಲದಿದ್ದರೆ
//! Hardwired span size
#ವ್ಯಾಖ್ಯಾನಿಸು ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ ಸ್ಮರಣೆ_default_ಹರವು_ಗಾತ್ರ
#ವ್ಯಾಖ್ಯಾನಿಸು ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ_ಸರಿಸು ಸ್ಮರಣೆ_default_ಹರವು_ಗಾತ್ರ_ಸರಿಸು
#ವ್ಯಾಖ್ಯಾನಿಸು ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ ಸ್ಮರಣೆ_default_ಹರವು_ಮುಖವಾಡ
#ಆದರೆ_ಅಂತ್ಯ
//! Number of spans to map in each map call
ಸ್ಥಿರ size_t ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ;
//! Number of spans to keep reserved in each heap
ಸ್ಥಿರ size_t ಸ್ಮರಣೆ_ರಾಶಿ_reserve_ಎಣಿಕೆ;
//! Global size classes
ಸ್ಥಿರ ಗಾತ್ರ_class_t ಸ್ಮರಣೆ_ಗಾತ್ರ_class[ಗಾತ್ರ_CLASS_ಎಣಿಕೆ];
//! Run-time size limit of medium blocks
ಸ್ಥಿರ size_t ಸ್ಮರಣೆ_medium_ಗಾತ್ರ_ಎಲ್ಲೆ;
//! Heap ID counter
ಸ್ಥಿರ atomic32_t ಸ್ಮರಣೆ_ರಾಶಿ_id;
//! Huge page support
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸ್ಮರಣೆ_huge_ಪುಟಗಳು;
#ಆದರೆ ENABLE_GLOBAL_CACHE
//! Global span cache
ಸ್ಥಿರ global_cache_t ಸ್ಮರಣೆ_ಹರವು_cache[LARGE_CLASS_ಎಣಿಕೆ];
#ಆದರೆ_ಅಂತ್ಯ
//! Global reserved spans
ಸ್ಥಿರ ಹರವು_t *ಸ್ಮರಣೆ_global_reserve;
//! Global reserved count
ಸ್ಥಿರ size_t ಸ್ಮರಣೆ_global_reserve_ಎಣಿಕೆ;
//! Global reserved master
ಸ್ಥಿರ ಹರವು_t *ಸ್ಮರಣೆ_global_reserve_master;
//! All heaps
ಸ್ಥಿರ ರಾಶಿ_t *ಸ್ಮರಣೆ_heaps[ರಾಶಿ_ARRAY_ಗಾತ್ರ];
//! Used to restrict access to mapping memory for huge pages
ಸ್ಥಿರ atomic32_t ಸ್ಮರಣೆ_global_ಬೀಗ;
//! Orphaned heaps
ಸ್ಥಿರ ರಾಶಿ_t *ಸ್ಮರಣೆ_orphan_heaps;
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
ಸ್ಥಿರ ರಾಶಿ_t *ಸ್ಮರಣೆ_ಮೊದಲ_class_orphan_heaps;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_STATISTICS
//! Allocations counter
ಸ್ಥಿರ atomic64_t ಹಂಚಿಕೆ_counter;
//! Deallocations counter
ಸ್ಥಿರ atomic64_t _deallocation_counter;
//! Active heap count
ಸ್ಥಿರ atomic32_t ಸ್ಮರಣೆ_ಸಕ್ರಿಯ_heaps;
//! Number of currently mapped memory pages
ಸ್ಥಿರ atomic32_t mapped_ಪುಟಗಳು;
//! Peak number of concurrently mapped memory pages
ಸ್ಥಿರ int32_t mapped_ಪುಟಗಳು_peak;
//! Number of mapped master spans
ಸ್ಥಿರ atomic32_t _master_spans;
//! Number of unmapped dangling master spans
ಸ್ಥಿರ atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
ಸ್ಥಿರ atomic32_t mapped_ಒಟ್ಟು;
//! Running counter of total number of unmapped memory pages since start
ಸ್ಥಿರ atomic32_t unmapped_ಒಟ್ಟು;
//! Number of currently mapped memory pages in OS calls
ಸ್ಥಿರ atomic32_t mapped_ಪುಟಗಳು_os;
//! Number of currently allocated pages in huge allocations
ಸ್ಥಿರ atomic32_t huge_ಪುಟಗಳು_current;
//! Peak number of currently allocated pages in huge allocations
ಸ್ಥಿರ int32_t huge_ಪುಟಗಳು_peak;
#ಆದರೆ_ಅಂತ್ಯ

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#ಆದರೆ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
ಸ್ಥಿರ pthread_key_t ಸ್ಮರಣೆ_thread_ರಾಶಿ;
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
#ವ್ಯಾಖ್ಯಾನಿಸು _Thread_local __declspec(thread)
#ವ್ಯಾಖ್ಯಾನಿಸು TLS_MODEL
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __HAIKU__
#ವ್ಯಾಖ್ಯಾನಿಸು TLS_MODEL __attribute__((tls_model("initial-exec")))
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು TLS_MODEL
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(__clang__) && defined(__GNUC__)
#ವ್ಯಾಖ್ಯಾನಿಸು _Thread_local __thread
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ _Thread_local ರಾಶಿ_t *ಸ್ಮರಣೆ_thread_ರಾಶಿ TLS_MODEL;
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಾಶಿ_t *get_thread_ರಾಶಿ_raw(ಶೂನ್ಯ) {
#ಆದರೆ (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  ಹಿಂತಿರುಗಿ pthread_getspecific(ಸ್ಮರಣೆ_thread_ರಾಶಿ);
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_thread_ರಾಶಿ;
#ಆದರೆ_ಅಂತ್ಯ
}

//! Get the current thread heap
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಾಶಿ_t *get_thread_ರಾಶಿ(ಶೂನ್ಯ) {
  ರಾಶಿ_t *ರಾಶಿ_2 = get_thread_ರಾಶಿ_raw();
#ಆದರೆ ENABLE_PRELOAD
  ಆದರೆ (EXPECTED(ರಾಶಿ_2 != 0))
    ಹಿಂತಿರುಗಿ ರಾಶಿ_2;
  rpmalloc_initialize();
  ಹಿಂತಿರುಗಿ get_thread_ರಾಶಿ_raw();
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ ರಾಶಿ_2;
#ಆದರೆ_ಅಂತ್ಯ
}

//! Fast thread ID
ಸ್ಥಿರ ಸಾಲೊಳಗೆ uintptr_t get_thread_id(ಶೂನ್ಯ) {
#ಆದರೆ defined(_WIN32)
  ಹಿಂತಿರುಗಿ (uintptr_t)((ಶೂನ್ಯ *)NtCurrentTeb());
#ಅಥವಾಆದರೆ (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#ಆದರೆ defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#ಅಥವಾಆದರೆ defined(__x86_64__)
#ಆದರೆ defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#ಇಲ್ಲದಿದ್ದರೆ
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#ಆದರೆ_ಅಂತ್ಯ
#ಅಥವಾಆದರೆ defined(__arm__)
  __asm__ ಅಸ್ಥಿರ("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#ಅಥವಾಆದರೆ defined(__aarch64__)
#ಆದರೆ defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ ಅಸ್ಥಿರ("mrs %0, tpidrro_el0" : "=r"(tid));
#ಇಲ್ಲದಿದ್ದರೆ
  __asm__ ಅಸ್ಥಿರ("mrs %0, tpidr_el0" : "=r"(tid));
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ This platform needs implementation ದ_3 get_thread_id()
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ tid;
#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ This platform needs implementation ದ_3 get_thread_id()
#ಆದರೆ_ಅಂತ್ಯ
}

//! Set the current thread heap
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_thread_ರಾಶಿ(ರಾಶಿ_t *ರಾಶಿ_2) {
#ಆದರೆ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(ಸ್ಮರಣೆ_thread_ರಾಶಿ, ರಾಶಿ_2);
#ಇಲ್ಲದಿದ್ದರೆ
  ಸ್ಮರಣೆ_thread_ರಾಶಿ = ರಾಶಿ_2;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (ರಾಶಿ_2)
    ರಾಶಿ_2->owner_thread = get_thread_id();
}

//! Set main thread ID
ಬಾಹ್ಯ ಶೂನ್ಯ rpmalloc_ಗಣ_main_thread(ಶೂನ್ಯ);

ಶೂನ್ಯ rpmalloc_ಗಣ_main_thread(ಶೂನ್ಯ) {
  _rpmalloc_main_thread_id = get_thread_id();
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_spin(ಶೂನ್ಯ) {
#ಆದರೆ defined(_MSC_VER)
#ಆದರೆ defined(_M_ARM64)
  __yield();
#ಇಲ್ಲದಿದ್ದರೆ
  _mm_pause();
#ಆದರೆ_ಅಂತ್ಯ
#ಅಥವಾಆದರೆ defined(__x86_64__) || defined(__i386__)
  __asm__ ಅಸ್ಥಿರ("pause" ::: "memory");
#ಅಥವಾಆದರೆ defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ ಅಸ್ಥಿರ("yield" ::: "memory");
#ಅಥವಾಆದರೆ defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ ಅಸ್ಥಿರ("or 27,27,27");
#ಅಥವಾಆದರೆ defined(__sparc__)
  __asm__ ಅಸ್ಥಿರ("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#ಇಲ್ಲದಿದ್ದರೆ
  ರಚನೆ timespec ts = {0};
  nanosleep(&ts, 0);
#ಆದರೆ_ಅಂತ್ಯ
}

#ಆದರೆ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

ಸ್ಥಿರ ಶೂನ್ಯ NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  ಆಯ್ಕೆ (reason) {
  ಸ್ಥಿತಿ DLL_PROCESS_ATTACH:
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ DLL_THREAD_ATTACH:
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    ನಿಲ್ಲಿಸು;
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN64
#ಸೂಚನೆ comment(linker, "/INCLUDE:_tls_used")
#ಸೂಚನೆ comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#ಸೂಚನೆ const_seg(".CRT$XLY")

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ಸ್ಥಿರಾಂಕ PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#ಸೂಚನೆ const_seg()
#ಇಲ್ಲದಿದ್ದರೆ // _WIN64
#ಸೂಚನೆ comment(linker, "/INCLUDE:__tls_used")
#ಸೂಚನೆ comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#ಸೂಚನೆ ದತ್ತಾಂಶ_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#ಸೂಚನೆ ದತ್ತಾಂಶ_seg()
#ಆದರೆ_ಅಂತ್ಯ // _WIN64

ಸ್ಥಿರ ಶೂನ್ಯ NTAPI _rpmalloc_thread_destructor(ಶೂನ್ಯ *ಮೌಲ್ಯ_2) {
#ಆದರೆ ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  ಆದರೆ (get_thread_id() == _rpmalloc_main_thread_id)
    ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (ಮೌಲ್ಯ_2)
    rpmalloc_thread_finalize(1);
}
#ಆದರೆ_ಅಂತ್ಯ

////////////
///
/// Low level memory map/unmap
///
//////

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ಗಣ_name(ಶೂನ್ಯ *ವಿಳಾಸ_2, size_t ಗಾತ್ರ_3) {
#ಆದರೆ defined(__linux__) || defined(__ANDROID__)
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = ಸ್ಮರಣೆ_huge_ಪುಟಗಳು ? ಸ್ಮರಣೆ_config.huge_ಪುಟ_name
                                        : ಸ್ಮರಣೆ_config.ಪುಟ_name;
  ಆದರೆ (ವಿಳಾಸ_2 == MAP_FAILED || !name)
    ಹಿಂತಿರುಗಿ;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (ಶೂನ್ಯ)prctl(PR_ಗಣ_VMA, PR_ಗಣ_VMA_ANON_NAME, (uintptr_t)ವಿಳಾಸ_2, ಗಾತ್ರ_3,
              (uintptr_t)name);
#ಇಲ್ಲದಿದ್ದರೆ
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
  (ಶೂನ್ಯ)ಗಾತ್ರ(ವಿಳಾಸ_2);
#ಆದರೆ_ಅಂತ್ಯ
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_mmap(size_t ಗಾತ್ರ_3, size_t *ಸ್ಥಾನಾಂತರ_2) {
  rpmalloc_assert(!(ಗಾತ್ರ_3 % ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ), "Invalid mmap size");
  rpmalloc_assert(ಗಾತ್ರ_3 >= ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ, "Invalid mmap size");
  ಶೂನ್ಯ *ವಿಳಾಸ_2 = ಸ್ಮರಣೆ_config.ಸ್ಮರಣೆ_map(ಗಾತ್ರ_3, ಸ್ಥಾನಾಂತರ_2);
  ಆದರೆ (EXPECTED(ವಿಳಾಸ_2 != 0)) {
    rpmalloc_stat_ಕೂಡಿಸು_peak(&mapped_ಪುಟಗಳು, (ಗಾತ್ರ_3 >> ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ_ಸರಿಸು),
                            mapped_ಪುಟಗಳು_peak);
    rpmalloc_stat_ಕೂಡಿಸು(&mapped_ಒಟ್ಟು, (ಗಾತ್ರ_3 >> ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ_ಸರಿಸು));
  }
  ಹಿಂತಿರುಗಿ ವಿಳಾಸ_2;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_unmap(ಶೂನ್ಯ *ವಿಳಾಸ_2, size_t ಗಾತ್ರ_3, size_t ಸ್ಥಾನಾಂತರ_2,
                            size_t release) {
  rpmalloc_assert(!release || (release >= ಗಾತ್ರ_3), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ),
                  "Invalid unmap size");
  ಆದರೆ (release) {
    rpmalloc_assert(!(release % ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ), "Invalid unmap size");
    _rpmalloc_stat_sub(&mapped_ಪುಟಗಳು, (release >> ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ_ಸರಿಸು));
    rpmalloc_stat_ಕೂಡಿಸು(&unmapped_ಒಟ್ಟು, (release >> ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ_ಸರಿಸು));
  }
  ಸ್ಮರಣೆ_config.ಸ್ಮರಣೆ_unmap(ವಿಳಾಸ_2, ಗಾತ್ರ_3, ಸ್ಥಾನಾಂತರ_2, release);
}

//! Default implementation to map new pages to virtual memory
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_mmap_os(size_t ಗಾತ್ರ_3, size_t *ಸ್ಥಾನಾಂತರ_2) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((ಗಾತ್ರ_3 >= ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) &&
                    (ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ > ಸ್ಮರಣೆ_map_granularity))
                       ? ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ
                       : 0;
  rpmalloc_assert(ಗಾತ್ರ_3 >= ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ, "Invalid mmap size");
#ಆದರೆ PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  ಶೂನ್ಯ *ptr = VirtualAlloc(0, ಗಾತ್ರ_3 + padding,
                           (ಸ್ಮರಣೆ_huge_ಪುಟಗಳು ? MEM_LARGE_ಪುಟಗಳು : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           ಪುಟ_READWRITE);
  ಆದರೆ (!ptr) {
    ಆದರೆ (ಸ್ಮರಣೆ_config.map_fail_callback) {
      ಆದರೆ (ಸ್ಮರಣೆ_config.map_fail_callback(ಗಾತ್ರ_3 + padding))
        ಹಿಂತಿರುಗಿ _rpmalloc_mmap_os(ಗಾತ್ರ_3, ಸ್ಥಾನಾಂತರ_2);
    } ಇಲ್ಲದಿದ್ದರೆ {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    ಹಿಂತಿರುಗಿ 0;
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ flags = MAP_ಖಾಸಗಿ | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#ಆದರೆ defined(__APPLE__) && !ಗುರಿ_OS_IPHONE && !ಗುರಿ_OS_SIMULATOR
  ಪೂರ್ಣಾಂಕ fd = (ಪೂರ್ಣಾಂಕ)VM_MAKE_ಟ್ಯಾಗ್(240U);
  ಆದರೆ (ಸ್ಮರಣೆ_huge_ಪುಟಗಳು)
    fd |= VM_FLAGS_SUPERPAGE_ಗಾತ್ರ_2_MB;
  ಶೂನ್ಯ *ptr = mmap(0, ಗಾತ್ರ_3 + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#ಅಥವಾಆದರೆ defined(MAP_HUGETLB)
  ಶೂನ್ಯ *ptr = mmap(0, ಗಾತ್ರ_3 + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (ಸ್ಮರಣೆ_huge_ಪುಟಗಳು ? MAP_HUGETLB : 0) | flags, -1, 0);
#ಆದರೆ defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  ಆದರೆ ((ptr == MAP_FAILED || !ptr) && ಸ್ಮರಣೆ_huge_ಪುಟಗಳು) {
    ptr = mmap(0, ಗಾತ್ರ_3 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    ಆದರೆ (ptr && ptr != MAP_FAILED) {
      ಪೂರ್ಣಾಂಕ prm = madvise(ptr, ಗಾತ್ರ_3 + padding, MADV_HUGEPAGE);
      (ಶೂನ್ಯ)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  rpmalloc_ಗಣ_name(ptr, ಗಾತ್ರ_3 + padding);
#ಅಥವಾಆದರೆ defined(MAP_ALIGNED)
  ಸ್ಥಿರಾಂಕ size_t align =
      (ಗಾತ್ರ(size_t) * 8) - (size_t)(__builtin_clzl(ಗಾತ್ರ_3 - 1));
  ಶೂನ್ಯ *ptr =
      mmap(0, ಗಾತ್ರ_3 + padding, PROT_READ | PROT_WRITE,
           (ಸ್ಮರಣೆ_huge_ಪುಟಗಳು ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#ಅಥವಾಆದರೆ defined(MAP_ALIGN)
  caddr_t ತಳ_2 = (ಸ್ಮರಣೆ_huge_ಪುಟಗಳು ? (caddr_t)(4 << 20) : 0);
  ಶೂನ್ಯ *ptr = mmap(ತಳ_2, ಗಾತ್ರ_3 + padding, PROT_READ | PROT_WRITE,
                   (ಸ್ಮರಣೆ_huge_ಪುಟಗಳು ? MAP_ALIGN : 0) | flags, -1, 0);
#ಇಲ್ಲದಿದ್ದರೆ
  ಶೂನ್ಯ *ptr = mmap(0, ಗಾತ್ರ_3 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ ((ptr == MAP_FAILED) || !ptr) {
    ಆದರೆ (ಸ್ಮರಣೆ_config.map_fail_callback) {
      ಆದರೆ (ಸ್ಮರಣೆ_config.map_fail_callback(ಗಾತ್ರ_3 + padding))
        ಹಿಂತಿರುಗಿ _rpmalloc_mmap_os(ಗಾತ್ರ_3, ಸ್ಥಾನಾಂತರ_2);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  rpmalloc_stat_ಕೂಡಿಸು(&mapped_ಪುಟಗಳು_os,
                     (int32_t)((ಗಾತ್ರ_3 + padding) >> ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ_ಸರಿಸು));
  ಆದರೆ (padding) {
    size_t final_padding = padding - ((uintptr_t)ptr & ~ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ);
    rpmalloc_assert(final_padding <= ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ,
                    "Internal failure in padding");
    rpmalloc_assert(final_padding <= padding, "Internal failure in padding");
    rpmalloc_assert(!(final_padding % 8), "Internal failure in padding");
    ptr = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ptr, final_padding);
    *ಸ್ಥಾನಾಂತರ_2 = final_padding >> 3;
  }
  rpmalloc_assert((ಗಾತ್ರ_3 < ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) ||
                      !((uintptr_t)ptr & ~ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ),
                  "Internal failure in padding");
  ಹಿಂತಿರುಗಿ ptr;
}

//! Default implementation to unmap pages from virtual memory
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_unmap_os(ಶೂನ್ಯ *ವಿಳಾಸ_2, size_t ಗಾತ್ರ_3, size_t ಸ್ಥಾನಾಂತರ_2,
                               size_t release) {
  rpmalloc_assert(release || (ಸ್ಥಾನಾಂತರ_2 == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ),
                  "Invalid unmap size");
  rpmalloc_assert(ಗಾತ್ರ_3 >= ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ, "Invalid unmap size");
  ಆದರೆ (release && ಸ್ಥಾನಾಂತರ_2) {
    ಸ್ಥಾನಾಂತರ_2 <<= 3;
    ವಿಳಾಸ_2 = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ವಿಳಾಸ_2, -(int32_t)ಸ್ಥಾನಾಂತರ_2);
    ಆದರೆ ((release >= ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) &&
        (ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ > ಸ್ಮರಣೆ_map_granularity)) {
      // Padding is always one span size
      release += ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ;
    }
  }
#ಆದರೆ !DISABLE_UNMAP
#ಆದರೆ PLATFORM_WINDOWS
  ಆದರೆ (!VirtualFree(ವಿಳಾಸ_2, release ? 0 : ಗಾತ್ರ_3,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ (release) {
    ಆದರೆ (munmap(ವಿಳಾಸ_2, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
#ಆದರೆ defined(MADV_FREE_REUSABLE)
    ಪೂರ್ಣಾಂಕ ret;
    ಆಗ ((ret = madvise(ವಿಳಾಸ_2, ಗಾತ್ರ_3, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    ಆದರೆ ((ret == -1) && (errno != 0)) {
#ಅಥವಾಆದರೆ defined(MADV_DONTNEED)
    ಆದರೆ (madvise(ವಿಳಾಸ_2, ಗಾತ್ರ_3, MADV_DONTNEED)) {
#ಅಥವಾಆದರೆ defined(MADV_PAGEOUT)
    ಆದರೆ (madvise(ವಿಳಾಸ_2, ಗಾತ್ರ_3, MADV_PAGEOUT)) {
#ಅಥವಾಆದರೆ defined(MADV_FREE)
    ಆದರೆ (madvise(ವಿಳಾಸ_2, ಗಾತ್ರ_3, MADV_FREE)) {
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ (posix_madvise(ವಿಳಾಸ_2, ಗಾತ್ರ_3, POSIX_MADV_DONTNEED)) {
#ಆದರೆ_ಅಂತ್ಯ
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (release)
    _rpmalloc_stat_sub(&mapped_ಪುಟಗಳು_os, release >> ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ_ಸರಿಸು);
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ಹರವು_mark_as_subspan_unless_master(ಹರವು_t *master,
                                                         ಹರವು_t *subspan,
                                                         size_t ಹರವು_ಎಣಿಕೆ);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
ಸ್ಥಿರ ಹರವು_t *_rpmalloc_global_get_reserved_spans(size_t ಹರವು_ಎಣಿಕೆ) {
  ಹರವು_t *ಹರವು = ಸ್ಮರಣೆ_global_reserve;
  rpmalloc_ಹರವು_mark_as_subspan_unless_master(ಸ್ಮರಣೆ_global_reserve_master,
                                               ಹರವು, ಹರವು_ಎಣಿಕೆ);
  ಸ್ಮರಣೆ_global_reserve_ಎಣಿಕೆ -= ಹರವು_ಎಣಿಕೆ;
  ಆದರೆ (ಸ್ಮರಣೆ_global_reserve_ಎಣಿಕೆ)
    ಸ್ಮರಣೆ_global_reserve =
        (ಹರವು_t *)ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಹರವು_ಎಣಿಕೆ << ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ_ಸರಿಸು);
  ಇಲ್ಲದಿದ್ದರೆ
    ಸ್ಮರಣೆ_global_reserve = 0;
  ಹಿಂತಿರುಗಿ ಹರವು;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_global_ಗಣ_reserved_spans(ಹರವು_t *master, ಹರವು_t *reserve,
                                                size_t reserve_ಹರವು_ಎಣಿಕೆ) {
  ಸ್ಮರಣೆ_global_reserve_master = master;
  ಸ್ಮರಣೆ_global_reserve_ಎಣಿಕೆ = reserve_ಹರವು_ಎಣಿಕೆ;
  ಸ್ಮರಣೆ_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ಹರವು_double_link_list_ಕೂಡಿಸು(ಹರವು_t **ತಲೆ, ಹರವು_t *ಹರವು) {
  ಆದರೆ (*ತಲೆ)
    (*ತಲೆ)->prev = ಹರವು;
  ಹರವು->ಮುಂದಿನ = *ತಲೆ;
  *ತಲೆ = ಹರವು;
}

//! Pop head span from double linked list
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ಹರವು_double_link_list_ತೆಗೆ_ತಲೆ(ಹರವು_t **ತಲೆ,
                                                     ಹರವು_t *ಹರವು) {
  rpmalloc_assert(*ತಲೆ == ಹರವು, "Linked list corrupted");
  ಹರವು = *ತಲೆ;
  *ತಲೆ = ಹರವು->ಮುಂದಿನ;
}

//! Remove a span from double linked list
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ಹರವು_double_link_list_remove(ಹರವು_t **ತಲೆ,
                                                   ಹರವು_t *ಹರವು) {
  rpmalloc_assert(*ತಲೆ, "Linked list corrupted");
  ಆದರೆ (*ತಲೆ == ಹರವು) {
    *ತಲೆ = ಹರವು->ಮುಂದಿನ;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಹರವು_t *ಮುಂದಿನ_ಹರವು = ಹರವು->ಮುಂದಿನ;
    ಹರವು_t *prev_ಹರವು = ಹರವು->prev;
    prev_ಹರವು->ಮುಂದಿನ = ಮುಂದಿನ_ಹರವು;
    ಆದರೆ (EXPECTED(ಮುಂದಿನ_ಹರವು != 0))
      ಮುಂದಿನ_ಹರವು->prev = prev_ಹರವು;
  }
}

////////////
///
/// Span control
///
//////

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_cache_ಸೇರಿಸು(ರಾಶಿ_t *ರಾಶಿ_2, ಹರವು_t *ಹರವು);

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_finalize(ರಾಶಿ_t *ರಾಶಿ_2);

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_ಗಣ_reserved_spans(ರಾಶಿ_t *ರಾಶಿ_2, ಹರವು_t *master,
                                              ಹರವು_t *reserve,
                                              size_t reserve_ಹರವು_ಎಣಿಕೆ);

//! Declare the span to be a subspan and store distance from master span and
//! span count
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ಹರವು_mark_as_subspan_unless_master(ಹರವು_t *master,
                                                         ಹರವು_t *subspan,
                                                         size_t ಹರವು_ಎಣಿಕೆ) {
  rpmalloc_assert((subspan != master) || (subspan->flags & ಹರವು_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  ಆದರೆ (subspan != master) {
    subspan->flags = ಹರವು_FLAG_SUBSPAN;
    subspan->ಸ್ಥಾನಾಂತರ_from_master =
        (uint32_t)((uintptr_t)ಸೂಚಕ_diff(subspan, master) >>
                   ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ_ಸರಿಸು);
    subspan->align_ಸ್ಥಾನಾಂತರ = 0;
  }
  subspan->ಹರವು_ಎಣಿಕೆ = (uint32_t)ಹರವು_ಎಣಿಕೆ;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
ಸ್ಥಿರ ಹರವು_t *rpmalloc_ಹರವು_map_from_reserve(ರಾಶಿ_t *ರಾಶಿ_2,
                                               size_t ಹರವು_ಎಣಿಕೆ) {
  // Update the heap span reserve
  ಹರವು_t *ಹರವು = ರಾಶಿ_2->ಹರವು_reserve;
  ರಾಶಿ_2->ಹರವು_reserve =
      (ಹರವು_t *)ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಹರವು_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
  ರಾಶಿ_2->spans_reserved -= (uint32_t)ಹರವು_ಎಣಿಕೆ;

  rpmalloc_ಹರವು_mark_as_subspan_unless_master(ರಾಶಿ_2->ಹರವು_reserve_master, ಹರವು,
                                               ಹರವು_ಎಣಿಕೆ);
  ಆದರೆ (ಹರವು_ಎಣಿಕೆ <= LARGE_CLASS_ಎಣಿಕೆ)
    _rpmalloc_stat_inc(&ರಾಶಿ_2->ಹರವು_use[ಹರವು_ಎಣಿಕೆ - 1].spans_from_reserved);

  ಹಿಂತಿರುಗಿ ಹರವು;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
ಸ್ಥಿರ size_t rpmalloc_ಹರವು_align_ಎಣಿಕೆ(size_t ಹರವು_ಎಣಿಕೆ) {
  size_t request_ಎಣಿಕೆ = (ಹರವು_ಎಣಿಕೆ > ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ)
                             ? ಹರವು_ಎಣಿಕೆ
                             : ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ;
  ಆದರೆ ((ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ > ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) &&
      ((request_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) % ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ))
    request_ಎಣಿಕೆ +=
        ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ - (request_ಎಣಿಕೆ % ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ);
  ಹಿಂತಿರುಗಿ request_ಎಣಿಕೆ;
}

//! Setup a newly mapped span
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ಹರವು_initialize(ಹರವು_t *ಹರವು, size_t ಒಟ್ಟು_ಹರವು_ಎಣಿಕೆ,
                                      size_t ಹರವು_ಎಣಿಕೆ, size_t align_ಸ್ಥಾನಾಂತರ) {
  ಹರವು->ಒಟ್ಟು_spans = (uint32_t)ಒಟ್ಟು_ಹರವು_ಎಣಿಕೆ;
  ಹರವು->ಹರವು_ಎಣಿಕೆ = (uint32_t)ಹರವು_ಎಣಿಕೆ;
  ಹರವು->align_ಸ್ಥಾನಾಂತರ = (uint32_t)align_ಸ್ಥಾನಾಂತರ;
  ಹರವು->flags = ಹರವು_FLAG_MASTER;
  atomic_store32(&ಹರವು->ಉಳಿದ_spans, (int32_t)ಒಟ್ಟು_ಹರವು_ಎಣಿಕೆ);
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ಹರವು_unmap(ಹರವು_t *ಹರವು);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
ಸ್ಥಿರ ಹರವು_t *rpmalloc_ಹರವು_map_aligned_ಎಣಿಕೆ(ರಾಶಿ_t *ರಾಶಿ_2,
                                                size_t ಹರವು_ಎಣಿಕೆ) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_ಹರವು_ಎಣಿಕೆ = rpmalloc_ಹರವು_align_ಎಣಿಕೆ(ಹರವು_ಎಣಿಕೆ);
  size_t align_ಸ್ಥಾನಾಂತರ = 0;
  ಹರವು_t *ಹರವು = (ಹರವು_t *)_rpmalloc_mmap(
      aligned_ಹರವು_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ, &align_ಸ್ಥಾನಾಂತರ);
  ಆದರೆ (!ಹರವು)
    ಹಿಂತಿರುಗಿ 0;
  rpmalloc_ಹರವು_initialize(ಹರವು, aligned_ಹರವು_ಎಣಿಕೆ, ಹರವು_ಎಣಿಕೆ, align_ಸ್ಥಾನಾಂತರ);
  _rpmalloc_stat_inc(&_master_spans);
  ಆದರೆ (ಹರವು_ಎಣಿಕೆ <= LARGE_CLASS_ಎಣಿಕೆ)
    _rpmalloc_stat_inc(&ರಾಶಿ_2->ಹರವು_use[ಹರವು_ಎಣಿಕೆ - 1].spans_map_calls);
  ಆದರೆ (aligned_ಹರವು_ಎಣಿಕೆ > ಹರವು_ಎಣಿಕೆ) {
    ಹರವು_t *reserved_spans =
        (ಹರವು_t *)ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಹರವು_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
    size_t reserved_ಎಣಿಕೆ = aligned_ಹರವು_ಎಣಿಕೆ - ಹರವು_ಎಣಿಕೆ;
    ಆದರೆ (ರಾಶಿ_2->spans_reserved) {
      rpmalloc_ಹರವು_mark_as_subspan_unless_master(
          ರಾಶಿ_2->ಹರವು_reserve_master, ರಾಶಿ_2->ಹರವು_reserve, ರಾಶಿ_2->spans_reserved);
      rpmalloc_ರಾಶಿ_cache_ಸೇರಿಸು(ರಾಶಿ_2, ರಾಶಿ_2->ಹರವು_reserve);
    }
    ಆದರೆ (reserved_ಎಣಿಕೆ > ಸ್ಮರಣೆ_ರಾಶಿ_reserve_ಎಣಿಕೆ) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&ಸ್ಮರಣೆ_global_ಬೀಗ) == 1,
                      "Global spin lock not held as expected");
      size_t remain_ಎಣಿಕೆ = reserved_ಎಣಿಕೆ - ಸ್ಮರಣೆ_ರಾಶಿ_reserve_ಎಣಿಕೆ;
      reserved_ಎಣಿಕೆ = ಸ್ಮರಣೆ_ರಾಶಿ_reserve_ಎಣಿಕೆ;
      ಹರವು_t *remain_ಹರವು = (ಹರವು_t *)ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(
          reserved_spans, reserved_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
      ಆದರೆ (ಸ್ಮರಣೆ_global_reserve) {
        rpmalloc_ಹರವು_mark_as_subspan_unless_master(
            ಸ್ಮರಣೆ_global_reserve_master, ಸ್ಮರಣೆ_global_reserve,
            ಸ್ಮರಣೆ_global_reserve_ಎಣಿಕೆ);
        rpmalloc_ಹರವು_unmap(ಸ್ಮರಣೆ_global_reserve);
      }
      rpmalloc_global_ಗಣ_reserved_spans(ಹರವು, remain_ಹರವು, remain_ಎಣಿಕೆ);
    }
    rpmalloc_ರಾಶಿ_ಗಣ_reserved_spans(ರಾಶಿ_2, ಹರವು, reserved_spans,
                                      reserved_ಎಣಿಕೆ);
  }
  ಹಿಂತಿರುಗಿ ಹರವು;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
ಸ್ಥಿರ ಹರವು_t *rpmalloc_ಹರವು_map(ರಾಶಿ_t *ರಾಶಿ_2, size_t ಹರವು_ಎಣಿಕೆ) {
  ಆದರೆ (ಹರವು_ಎಣಿಕೆ <= ರಾಶಿ_2->spans_reserved)
    ಹಿಂತಿರುಗಿ rpmalloc_ಹರವು_map_from_reserve(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ);
  ಹರವು_t *ಹರವು = 0;
  ಪೂರ್ಣಾಂಕ use_global_reserve =
      (ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ > ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) ||
      (ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ > ಸ್ಮರಣೆ_ರಾಶಿ_reserve_ಎಣಿಕೆ);
  ಆದರೆ (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    ಆಗ (!atomic_cas32_acquire(&ಸ್ಮರಣೆ_global_ಬೀಗ, 1, 0))
      _rpmalloc_spin();
    ಆದರೆ (ಸ್ಮರಣೆ_global_reserve_ಎಣಿಕೆ >= ಹರವು_ಎಣಿಕೆ) {
      size_t reserve_ಎಣಿಕೆ =
          (!ರಾಶಿ_2->spans_reserved ? ಸ್ಮರಣೆ_ರಾಶಿ_reserve_ಎಣಿಕೆ : ಹರವು_ಎಣಿಕೆ);
      ಆದರೆ (ಸ್ಮರಣೆ_global_reserve_ಎಣಿಕೆ < reserve_ಎಣಿಕೆ)
        reserve_ಎಣಿಕೆ = ಸ್ಮರಣೆ_global_reserve_ಎಣಿಕೆ;
      ಹರವು = _rpmalloc_global_get_reserved_spans(reserve_ಎಣಿಕೆ);
      ಆದರೆ (ಹರವು) {
        ಆದರೆ (reserve_ಎಣಿಕೆ > ಹರವು_ಎಣಿಕೆ) {
          ಹರವು_t *reserved_ಹರವು = (ಹರವು_t *)ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(
              ಹರವು, ಹರವು_ಎಣಿಕೆ << ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ_ಸರಿಸು);
          rpmalloc_ರಾಶಿ_ಗಣ_reserved_spans(ರಾಶಿ_2, ಸ್ಮರಣೆ_global_reserve_master,
                                            reserved_ಹರವು,
                                            reserve_ಎಣಿಕೆ - ಹರವು_ಎಣಿಕೆ);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        ಹರವು->ಹರವು_ಎಣಿಕೆ = (uint32_t)ಹರವು_ಎಣಿಕೆ;
      }
    }
  }
  ಆದರೆ (!ಹರವು)
    ಹರವು = rpmalloc_ಹರವು_map_aligned_ಎಣಿಕೆ(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ);
  ಆದರೆ (use_global_reserve)
    atomic_store32_release(&ಸ್ಮರಣೆ_global_ಬೀಗ, 0);
  ಹಿಂತಿರುಗಿ ಹರವು;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ಹರವು_unmap(ಹರವು_t *ಹರವು) {
  rpmalloc_assert((ಹರವು->flags & ಹರವು_FLAG_MASTER) ||
                      (ಹರವು->flags & ಹರವು_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(ಹರವು->flags & ಹರವು_FLAG_MASTER) ||
                      !(ಹರವು->flags & ಹರವು_FLAG_SUBSPAN),
                  "Span flag corrupted");

  ಪೂರ್ಣಾಂಕ is_master = !!(ಹರವು->flags & ಹರವು_FLAG_MASTER);
  ಹರವು_t *master =
      is_master ? ಹರವು
                : ((ಹರವು_t *)ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(
                      ಹರವು, -(intptr_t)((uintptr_t)ಹರವು->ಸ್ಥಾನಾಂತರ_from_master *
                                        ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ)));
  rpmalloc_assert(is_master || (ಹರವು->flags & ಹರವು_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & ಹರವು_FLAG_MASTER, "Span flag corrupted");

  size_t ಹರವು_ಎಣಿಕೆ = ಹರವು->ಹರವು_ಎಣಿಕೆ;
  ಆದರೆ (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(ಹರವು->align_ಸ್ಥಾನಾಂತರ == 0, "Span align offset corrupted");
    ಆದರೆ (ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ >= ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ)
      _rpmalloc_unmap(ಹರವು, ಹರವು_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ, 0, 0);
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    ಹರವು->flags |=
        ಹರವು_FLAG_MASTER | ಹರವು_FLAG_SUBSPAN | ಹರವು_FLAG_UNMAPPED_MASTER;
    rpmalloc_stat_ಕೂಡಿಸು(&_unmapped_master_spans, 1);
  }

  ಆದರೆ (atomic_add32(&master->ಉಳಿದ_spans, -(int32_t)ಹರವು_ಎಣಿಕೆ) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & ಹರವು_FLAG_MASTER) &&
                        !!(master->flags & ಹರವು_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_ಎಣಿಕೆ = master->ಹರವು_ಎಣಿಕೆ;
    ಆದರೆ (ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ < ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ)
      unmap_ಎಣಿಕೆ = master->ಒಟ್ಟು_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ,
                    master->align_ಸ್ಥಾನಾಂತರ,
                    (size_t)master->ಒಟ್ಟು_spans * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ಹರವು_release_ಗಮ್ಯ_cache(ರಾಶಿ_t *ರಾಶಿ_2, ಹರವು_t *ಹರವು) {
  rpmalloc_assert(ರಾಶಿ_2 == ಹರವು->ರಾಶಿ_2, "Span heap pointer corrupted");
  rpmalloc_assert(ಹರವು->ಗಾತ್ರ_class < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ,
                  "Invalid span size class");
  rpmalloc_assert(ಹರವು->ಹರವು_ಎಣಿಕೆ == 1, "Invalid span count");
#ಆದರೆ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&ರಾಶಿ_2->ಹರವು_use[0].current);
#ಆದರೆ_ಅಂತ್ಯ
  _rpmalloc_stat_dec(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[ಹರವು->ಗಾತ್ರ_class].spans_current);
  ಆದರೆ (!ರಾಶಿ_2->finalize) {
    _rpmalloc_stat_inc(&ರಾಶಿ_2->ಹರವು_use[0].spans_ಗಮ್ಯ_cache);
    _rpmalloc_stat_inc(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[ಹರವು->ಗಾತ್ರ_class].spans_ಗಮ್ಯ_cache);
    ಆದರೆ (ರಾಶಿ_2->ಗಾತ್ರ_class[ಹರವು->ಗಾತ್ರ_class].cache)
      rpmalloc_ರಾಶಿ_cache_ಸೇರಿಸು(ರಾಶಿ_2,
                                  ರಾಶಿ_2->ಗಾತ್ರ_class[ಹರವು->ಗಾತ್ರ_class].cache);
    ರಾಶಿ_2->ಗಾತ್ರ_class[ಹರವು->ಗಾತ್ರ_class].cache = ಹರವು;
  } ಇಲ್ಲದಿದ್ದರೆ {
    rpmalloc_ಹರವು_unmap(ಹರವು);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
ಸ್ಥಿರ uint32_t free_list_partial_init(ಶೂನ್ಯ **list, ಶೂನ್ಯ **ಮೊದಲ_ಖಂಡ,
                                       ಶೂನ್ಯ *ಪುಟ_ಆರಂಭ, ಶೂನ್ಯ *ಖಂಡ_ಆರಂಭ,
                                       uint32_t ಖಂಡ_ಎಣಿಕೆ,
                                       uint32_t ಖಂಡ_ಗಾತ್ರ_2) {
  rpmalloc_assert(ಖಂಡ_ಎಣಿಕೆ, "Internal failure");
  *ಮೊದಲ_ಖಂಡ = ಖಂಡ_ಆರಂಭ;
  ಆದರೆ (ಖಂಡ_ಎಣಿಕೆ > 1) {
    ಶೂನ್ಯ *free_ಖಂಡ = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಖಂಡ_ಆರಂಭ, ಖಂಡ_ಗಾತ್ರ_2);
    ಶೂನ್ಯ *ಖಂಡ_ಕೊನೆ =
        ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಖಂಡ_ಆರಂಭ, (size_t)ಖಂಡ_ಗಾತ್ರ_2 * ಖಂಡ_ಎಣಿಕೆ);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    ಆದರೆ (ಖಂಡ_ಗಾತ್ರ_2 < (ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ >> 1)) {
      ಶೂನ್ಯ *ಪುಟ_ಕೊನೆ = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಪುಟ_ಆರಂಭ, ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ);
      ಆದರೆ (ಪುಟ_ಕೊನೆ < ಖಂಡ_ಕೊನೆ)
        ಖಂಡ_ಕೊನೆ = ಪುಟ_ಕೊನೆ;
    }
    *list = free_ಖಂಡ;
    ಖಂಡ_ಎಣಿಕೆ = 2;
    ಶೂನ್ಯ *ಮುಂದಿನ_ಖಂಡ = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(free_ಖಂಡ, ಖಂಡ_ಗಾತ್ರ_2);
    ಆಗ (ಮುಂದಿನ_ಖಂಡ < ಖಂಡ_ಕೊನೆ) {
      *((ಶೂನ್ಯ **)free_ಖಂಡ) = ಮುಂದಿನ_ಖಂಡ;
      free_ಖಂಡ = ಮುಂದಿನ_ಖಂಡ;
      ++ಖಂಡ_ಎಣಿಕೆ;
      ಮುಂದಿನ_ಖಂಡ = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಮುಂದಿನ_ಖಂಡ, ಖಂಡ_ಗಾತ್ರ_2);
    }
    *((ಶೂನ್ಯ **)free_ಖಂಡ) = 0;
  } ಇಲ್ಲದಿದ್ದರೆ {
    *list = 0;
  }
  ಹಿಂತಿರುಗಿ ಖಂಡ_ಎಣಿಕೆ;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
ಸ್ಥಿರ ಶೂನ್ಯ *rpmalloc_ಹರವು_initialize_new(ರಾಶಿ_t *ರಾಶಿ_2,
                                           ರಾಶಿ_ಗಾತ್ರ_class_t *ರಾಶಿ_ಗಾತ್ರ_class,
                                           ಹರವು_t *ಹರವು, uint32_t class_idx) {
  rpmalloc_assert(ಹರವು->ಹರವು_ಎಣಿಕೆ == 1, "Internal failure");
  ಗಾತ್ರ_class_t *ಗಾತ್ರ_class = ಸ್ಮರಣೆ_ಗಾತ್ರ_class + class_idx;
  ಹರವು->ಗಾತ್ರ_class = class_idx;
  ಹರವು->ರಾಶಿ_2 = ರಾಶಿ_2;
  ಹರವು->flags &= ~ಹರವು_FLAG_ALIGNED_ಖಂಡಗಳು;
  ಹರವು->ಖಂಡ_ಗಾತ್ರ_2 = ಗಾತ್ರ_class->ಖಂಡ_ಗಾತ್ರ_2;
  ಹರವು->ಖಂಡ_ಎಣಿಕೆ = ಗಾತ್ರ_class->ಖಂಡ_ಎಣಿಕೆ;
  ಹರವು->free_list = 0;
  ಹರವು->list_ಗಾತ್ರ = 0;
  atomic_store_ptr_release(&ಹರವು->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  ಶೂನ್ಯ *ಖಂಡ;
  ಹರವು->free_list_ಎಲ್ಲೆ =
      free_list_partial_init(&ರಾಶಿ_ಗಾತ್ರ_class->free_list, &ಖಂಡ, ಹರವು,
                             ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ),
                             ಗಾತ್ರ_class->ಖಂಡ_ಎಣಿಕೆ, ಗಾತ್ರ_class->ಖಂಡ_ಗಾತ್ರ_2);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  ಆದರೆ (ಹರವು->free_list_ಎಲ್ಲೆ < ಹರವು->ಖಂಡ_ಎಣಿಕೆ) {
    rpmalloc_ಹರವು_double_link_list_ಕೂಡಿಸು(&ರಾಶಿ_ಗಾತ್ರ_class->partial_ಹರವು, ಹರವು);
    ಹರವು->ಬಳಸಿದ_ಎಣಿಕೆ = ಹರವು->free_list_ಎಲ್ಲೆ;
  } ಇಲ್ಲದಿದ್ದರೆ {
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
    rpmalloc_ಹರವು_double_link_list_ಕೂಡಿಸು(&ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು[class_idx], ಹರವು);
#ಆದರೆ_ಅಂತ್ಯ
    ++ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ;
    ಹರವು->ಬಳಸಿದ_ಎಣಿಕೆ = ಹರವು->ಖಂಡ_ಎಣಿಕೆ;
  }
  ಹಿಂತಿರುಗಿ ಖಂಡ;
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ಹರವು_ಹೊರತೆಗೆ_free_list_deferred(ಹರವು_t *ಹರವು) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  ಮಾಡು {
    ಹರವು->free_list =
        atomic_exchange_ptr_acquire(&ಹರವು->free_list_deferred, INVALID_ಸೂಚಕ);
  } ಆಗ (ಹರವು->free_list == INVALID_ಸೂಚಕ);
  ಹರವು->ಬಳಸಿದ_ಎಣಿಕೆ -= ಹರವು->list_ಗಾತ್ರ;
  ಹರವು->list_ಗಾತ್ರ = 0;
  atomic_store_ptr_release(&ಹರವು->free_list_deferred, 0);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rpmalloc_ಹರವು_is_fully_utilized(ಹರವು_t *ಹರವು) {
  rpmalloc_assert(ಹರವು->free_list_ಎಲ್ಲೆ <= ಹರವು->ಖಂಡ_ಎಣಿಕೆ,
                  "Span free list corrupted");
  ಹಿಂತಿರುಗಿ !ಹರವು->free_list && (ಹರವು->free_list_ಎಲ್ಲೆ >= ಹರವು->ಖಂಡ_ಎಣಿಕೆ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rpmalloc_ಹರವು_finalize(ರಾಶಿ_t *ರಾಶಿ_2, size_t iclass, ಹರವು_t *ಹರವು,
                                   ಹರವು_t **list_ತಲೆ) {
  ಶೂನ್ಯ *free_list = ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].free_list;
  ಹರವು_t *class_ಹರವು = (ಹರವು_t *)((uintptr_t)free_list & ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ);
  ಆದರೆ (ಹರವು == class_ಹರವು) {
    // Adopt the heap class free list back into the span free list
    ಶೂನ್ಯ *ಖಂಡ = ಹರವು->free_list;
    ಶೂನ್ಯ *ಕೊನೆಯ_ಖಂಡ = 0;
    ಆಗ (ಖಂಡ) {
      ಕೊನೆಯ_ಖಂಡ = ಖಂಡ;
      ಖಂಡ = *((ಶೂನ್ಯ **)ಖಂಡ);
    }
    uint32_t free_ಎಣಿಕೆ = 0;
    ಖಂಡ = free_list;
    ಆಗ (ಖಂಡ) {
      ++free_ಎಣಿಕೆ;
      ಖಂಡ = *((ಶೂನ್ಯ **)ಖಂಡ);
    }
    ಆದರೆ (ಕೊನೆಯ_ಖಂಡ) {
      *((ಶೂನ್ಯ **)ಕೊನೆಯ_ಖಂಡ) = free_list;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಹರವು->free_list = free_list;
    }
    ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].free_list = 0;
    ಹರವು->ಬಳಸಿದ_ಎಣಿಕೆ -= free_ಎಣಿಕೆ;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(ಹರವು->list_ಗಾತ್ರ == ಹರವು->ಬಳಸಿದ_ಎಣಿಕೆ, "Memory leak detected");
  ಆದರೆ (ಹರವು->list_ಗಾತ್ರ == ಹರವು->ಬಳಸಿದ_ಎಣಿಕೆ) {
    _rpmalloc_stat_dec(&ರಾಶಿ_2->ಹರವು_use[0].current);
    _rpmalloc_stat_dec(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    ಆದರೆ (list_ತಲೆ)
      rpmalloc_ಹರವು_double_link_list_remove(list_ತಲೆ, ಹರವು);
    rpmalloc_ಹರವು_unmap(ಹರವು);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}

////////////
///
/// Global cache
///
//////

#ಆದರೆ ENABLE_GLOBAL_CACHE

//! Finalize a global cache
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  ಆಗ (!atomic_cas32_acquire(&cache->ಬೀಗ_2, 1, 0))
    _rpmalloc_spin();

  ಗಾಗಿ (size_t ispan = 0; ispan < cache->ಎಣಿಕೆ_2; ++ispan)
    rpmalloc_ಹರವು_unmap(cache->ಹರವು[ispan]);
  cache->ಎಣಿಕೆ_2 = 0;

  ಆಗ (cache->ಅತಿಪ್ರವಾಹ_2) {
    ಹರವು_t *ಹರವು = cache->ಅತಿಪ್ರವಾಹ_2;
    cache->ಅತಿಪ್ರವಾಹ_2 = ಹರವು->ಮುಂದಿನ;
    rpmalloc_ಹರವು_unmap(ಹರವು);
  }

  atomic_store32_release(&cache->ಬೀಗ_2, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_global_cache_ಸೇರಿಸು_spans(ಹರವು_t **ಹರವು,
                                                size_t ಹರವು_ಎಣಿಕೆ,
                                                size_t ಎಣಿಕೆ_2) {
  ಸ್ಥಿರಾಂಕ size_t cache_ಎಲ್ಲೆ =
      (ಹರವು_ಎಣಿಕೆ == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_ಹರವು_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_ಹರವು_LARGE_CACHE - (ಹರವು_ಎಣಿಕೆ >> 1));

  global_cache_t *cache = &ಸ್ಮರಣೆ_ಹರವು_cache[ಹರವು_ಎಣಿಕೆ - 1];

  size_t ಸೇರಿಸು_ಎಣಿಕೆ = ಎಣಿಕೆ_2;
  ಆಗ (!atomic_cas32_acquire(&cache->ಬೀಗ_2, 1, 0))
    _rpmalloc_spin();

#ಆದರೆ ENABLE_STATISTICS
  cache->ಸೇರಿಸು_ಎಣಿಕೆ += ಎಣಿಕೆ_2;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ ((cache->ಎಣಿಕೆ_2 + ಸೇರಿಸು_ಎಣಿಕೆ) > cache_ಎಲ್ಲೆ)
    ಸೇರಿಸು_ಎಣಿಕೆ = cache_ಎಲ್ಲೆ - cache->ಎಣಿಕೆ_2;

  ಸ್ಮರಣೆ_ನಕಲು(cache->ಹರವು + cache->ಎಣಿಕೆ_2, ಹರವು, ಗಾತ್ರ(ಹರವು_t *) * ಸೇರಿಸು_ಎಣಿಕೆ);
  cache->ಎಣಿಕೆ_2 += (uint32_t)ಸೇರಿಸು_ಎಣಿಕೆ;

#ಆದರೆ ENABLE_UNLIMITED_CACHE
  ಆಗ (ಸೇರಿಸು_ಎಣಿಕೆ < ಎಣಿಕೆ_2) {
#ಇಲ್ಲದಿದ್ದರೆ
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  ಆಗ ((ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ > ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) && (ಸೇರಿಸು_ಎಣಿಕೆ < ಎಣಿಕೆ_2)) {
#ಆದರೆ_ಅಂತ್ಯ
    ಹರವು_t *current_ಹರವು = ಹರವು[ಸೇರಿಸು_ಎಣಿಕೆ++];
    current_ಹರವು->ಮುಂದಿನ = cache->ಅತಿಪ್ರವಾಹ_2;
    cache->ಅತಿಪ್ರವಾಹ_2 = current_ಹರವು;
  }
  atomic_store32_release(&cache->ಬೀಗ_2, 0);

  ಹರವು_t *keep = 0;
  ಗಾಗಿ (size_t ispan = ಸೇರಿಸು_ಎಣಿಕೆ; ispan < ಎಣಿಕೆ_2; ++ispan) {
    ಹರವು_t *current_ಹರವು = ಹರವು[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    ಆದರೆ ((current_ಹರವು->flags & ಹರವು_FLAG_MASTER) &&
        (atomic_load32(&current_ಹರವು->ಉಳಿದ_spans) >
         (int32_t)current_ಹರವು->ಹರವು_ಎಣಿಕೆ)) {
      current_ಹರವು->ಮುಂದಿನ = keep;
      keep = current_ಹರವು;
    } ಇಲ್ಲದಿದ್ದರೆ {
      rpmalloc_ಹರವು_unmap(current_ಹರವು);
    }
  }

  ಆದರೆ (keep) {
    ಆಗ (!atomic_cas32_acquire(&cache->ಬೀಗ_2, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    ಆಗ (keep) {
      ಗಾಗಿ (; islot < cache->ಎಣಿಕೆ_2; ++islot) {
        ಹರವು_t *current_ಹರವು = cache->ಹರವು[islot];
        ಆದರೆ (!(current_ಹರವು->flags & ಹರವು_FLAG_MASTER) ||
            ((current_ಹರವು->flags & ಹರವು_FLAG_MASTER) &&
             (atomic_load32(&current_ಹರವು->ಉಳಿದ_spans) <=
              (int32_t)current_ಹರವು->ಹರವು_ಎಣಿಕೆ))) {
          rpmalloc_ಹರವು_unmap(current_ಹರವು);
          cache->ಹರವು[islot] = keep;
          ನಿಲ್ಲಿಸು;
        }
      }
      ಆದರೆ (islot == cache->ಎಣಿಕೆ_2)
        ನಿಲ್ಲಿಸು;
      keep = keep->ಮುಂದಿನ;
    }

    ಆದರೆ (keep) {
      ಹರವು_t *ಬಾಲ_2 = keep;
      ಆಗ (ಬಾಲ_2->ಮುಂದಿನ)
        ಬಾಲ_2 = ಬಾಲ_2->ಮುಂದಿನ;
      ಬಾಲ_2->ಮುಂದಿನ = cache->ಅತಿಪ್ರವಾಹ_2;
      cache->ಅತಿಪ್ರವಾಹ_2 = keep;
    }

    atomic_store32_release(&cache->ಬೀಗ_2, 0);
  }
}

ಸ್ಥಿರ size_t rpmalloc_global_cache_ಹೊರತೆಗೆ_spans(ಹರವು_t **ಹರವು,
                                                   size_t ಹರವು_ಎಣಿಕೆ,
                                                   size_t ಎಣಿಕೆ_2) {
  global_cache_t *cache = &ಸ್ಮರಣೆ_ಹರವು_cache[ಹರವು_ಎಣಿಕೆ - 1];

  size_t ಹೊರತೆಗೆ_ಎಣಿಕೆ = 0;
  ಆಗ (!atomic_cas32_acquire(&cache->ಬೀಗ_2, 1, 0))
    _rpmalloc_spin();

#ಆದರೆ ENABLE_STATISTICS
  cache->ಹೊರತೆಗೆ_ಎಣಿಕೆ += ಎಣಿಕೆ_2;
#ಆದರೆ_ಅಂತ್ಯ
  size_t want = ಎಣಿಕೆ_2 - ಹೊರತೆಗೆ_ಎಣಿಕೆ;
  ಆದರೆ (want > cache->ಎಣಿಕೆ_2)
    want = cache->ಎಣಿಕೆ_2;

  ಸ್ಮರಣೆ_ನಕಲು(ಹರವು + ಹೊರತೆಗೆ_ಎಣಿಕೆ, cache->ಹರವು + (cache->ಎಣಿಕೆ_2 - want),
         ಗಾತ್ರ(ಹರವು_t *) * want);
  cache->ಎಣಿಕೆ_2 -= (uint32_t)want;
  ಹೊರತೆಗೆ_ಎಣಿಕೆ += want;

  ಆಗ ((ಹೊರತೆಗೆ_ಎಣಿಕೆ < ಎಣಿಕೆ_2) && cache->ಅತಿಪ್ರವಾಹ_2) {
    ಹರವು_t *current_ಹರವು = cache->ಅತಿಪ್ರವಾಹ_2;
    ಹರವು[ಹೊರತೆಗೆ_ಎಣಿಕೆ++] = current_ಹರವು;
    cache->ಅತಿಪ್ರವಾಹ_2 = current_ಹರವು->ಮುಂದಿನ;
  }

#ಆದರೆ ENABLE_ASSERTS
  ಗಾಗಿ (size_t ispan = 0; ispan < ಹೊರತೆಗೆ_ಎಣಿಕೆ; ++ispan) {
    rpmalloc_assert(ಹರವು[ispan]->ಹರವು_ಎಣಿಕೆ == ಹರವು_ಎಣಿಕೆ,
                    "Global cache span count mismatch");
  }
#ಆದರೆ_ಅಂತ್ಯ

  atomic_store32_release(&cache->ಬೀಗ_2, 0);

  ಹಿಂತಿರುಗಿ ಹೊರತೆಗೆ_ಎಣಿಕೆ;
}

#ಆದರೆ_ಅಂತ್ಯ

////////////
///
/// Heap control
///
//////

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate_huge(ಹರವು_t *);

//! Store the given spans as reserve in the given heap
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_ಗಣ_reserved_spans(ರಾಶಿ_t *ರಾಶಿ_2, ಹರವು_t *master,
                                              ಹರವು_t *reserve,
                                              size_t reserve_ಹರವು_ಎಣಿಕೆ) {
  ರಾಶಿ_2->ಹರವು_reserve_master = master;
  ರಾಶಿ_2->ಹರವು_reserve = reserve;
  ರಾಶಿ_2->spans_reserved = (uint32_t)reserve_ಹರವು_ಎಣಿಕೆ;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_cache_adopt_deferred(ರಾಶಿ_t *ರಾಶಿ_2,
                                                ಹರವು_t **single_ಹರವು) {
  ಹರವು_t *ಹರವು = (ಹರವು_t *)((ಶೂನ್ಯ *)atomic_exchange_ptr_acquire(
      &ರಾಶಿ_2->ಹರವು_free_deferred, 0));
  ಆಗ (ಹರವು) {
    ಹರವು_t *ಮುಂದಿನ_ಹರವು = (ಹರವು_t *)ಹರವು->free_list;
    rpmalloc_assert(ಹರವು->ರಾಶಿ_2 == ರಾಶಿ_2, "Span heap pointer corrupted");
    ಆದರೆ (EXPECTED(ಹರವು->ಗಾತ್ರ_class < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ)) {
      rpmalloc_assert(ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ, "Heap span counter corrupted");
      --ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ;
      _rpmalloc_stat_dec(&ರಾಶಿ_2->ಹರವು_use[0].spans_deferred);
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
      rpmalloc_ಹರವು_double_link_list_remove(&ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು[ಹರವು->ಗಾತ್ರ_class],
                                             ಹರವು);
#ಆದರೆ_ಅಂತ್ಯ
      _rpmalloc_stat_dec(&ರಾಶಿ_2->ಹರವು_use[0].current);
      _rpmalloc_stat_dec(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[ಹರವು->ಗಾತ್ರ_class].spans_current);
      ಆದರೆ (single_ಹರವು && !*single_ಹರವು)
        *single_ಹರವು = ಹರವು;
      ಇಲ್ಲದಿದ್ದರೆ
        rpmalloc_ರಾಶಿ_cache_ಸೇರಿಸು(ರಾಶಿ_2, ಹರವು);
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಆದರೆ (ಹರವು->ಗಾತ್ರ_class == ಗಾತ್ರ_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(ಹರವು);
      } ಇಲ್ಲದಿದ್ದರೆ {
        rpmalloc_assert(ಹರವು->ಗಾತ್ರ_class == ಗಾತ್ರ_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ, "Heap span counter corrupted");
        --ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ;
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
        rpmalloc_ಹರವು_double_link_list_remove(&ರಾಶಿ_2->large_huge_ಹರವು, ಹರವು);
#ಆದರೆ_ಅಂತ್ಯ
        uint32_t idx = ಹರವು->ಹರವು_ಎಣಿಕೆ - 1;
        _rpmalloc_stat_dec(&ರಾಶಿ_2->ಹರವು_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&ರಾಶಿ_2->ಹರವು_use[idx].current);
        ಆದರೆ (!idx && single_ಹರವು && !*single_ಹರವು)
          *single_ಹರವು = ಹರವು;
        ಇಲ್ಲದಿದ್ದರೆ
          rpmalloc_ರಾಶಿ_cache_ಸೇರಿಸು(ರಾಶಿ_2, ಹರವು);
      }
    }
    ಹರವು = ಮುಂದಿನ_ಹರವು;
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_unmap(ರಾಶಿ_t *ರಾಶಿ_2) {
  ಆದರೆ (!ರಾಶಿ_2->master_ರಾಶಿ) {
    ಆದರೆ ((ರಾಶಿ_2->finalize > 1) && !atomic_load32(&ರಾಶಿ_2->ಮಗು_ಎಣಿಕೆ)) {
      ಹರವು_t *ಹರವು = (ಹರವು_t *)((uintptr_t)ರಾಶಿ_2 & ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ);
      rpmalloc_ಹರವು_unmap(ಹರವು);
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (atomic_decr32(&ರಾಶಿ_2->master_ರಾಶಿ->ಮಗು_ಎಣಿಕೆ) == 0) {
      rpmalloc_ರಾಶಿ_unmap(ರಾಶಿ_2->master_ರಾಶಿ);
    }
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_global_finalize(ರಾಶಿ_t *ರಾಶಿ_2) {
  ಆದರೆ (ರಾಶಿ_2->finalize++ > 1) {
    --ರಾಶಿ_2->finalize;
    ಹಿಂತಿರುಗಿ;
  }

  rpmalloc_ರಾಶಿ_finalize(ರಾಶಿ_2);

#ಆದರೆ ENABLE_THREAD_CACHE
  ಗಾಗಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಹರವು_cache_t *ಹರವು_cache;
    ಆದರೆ (!iclass)
      ಹರವು_cache = &ರಾಶಿ_2->ಹರವು_cache;
    ಇಲ್ಲದಿದ್ದರೆ
      ಹರವು_cache = (ಹರವು_cache_t *)(ರಾಶಿ_2->ಹರವು_large_cache + (iclass - 1));
    ಗಾಗಿ (size_t ispan = 0; ispan < ಹರವು_cache->ಎಣಿಕೆ_2; ++ispan)
      rpmalloc_ಹರವು_unmap(ಹರವು_cache->ಹರವು[ispan]);
    ಹರವು_cache->ಎಣಿಕೆ_2 = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ (ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ) {
    --ರಾಶಿ_2->finalize;
    ಹಿಂತಿರುಗಿ;
  }

  ಗಾಗಿ (size_t iclass = 0; iclass < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಆದರೆ (ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].free_list ||
        ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].partial_ಹರವು) {
      --ರಾಶಿ_2->finalize;
      ಹಿಂತಿರುಗಿ;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)ರಾಶಿ_2->id % ರಾಶಿ_ARRAY_ಗಾತ್ರ;
  ರಾಶಿ_t *list_ರಾಶಿ = ಸ್ಮರಣೆ_heaps[list_idx];
  ಆದರೆ (list_ರಾಶಿ == ರಾಶಿ_2) {
    ಸ್ಮರಣೆ_heaps[list_idx] = ರಾಶಿ_2->ಮುಂದಿನ_ರಾಶಿ;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆಗ (list_ರಾಶಿ->ಮುಂದಿನ_ರಾಶಿ != ರಾಶಿ_2)
      list_ರಾಶಿ = list_ರಾಶಿ->ಮುಂದಿನ_ರಾಶಿ;
    list_ರಾಶಿ->ಮುಂದಿನ_ರಾಶಿ = ರಾಶಿ_2->ಮುಂದಿನ_ರಾಶಿ;
  }

  rpmalloc_ರಾಶಿ_unmap(ರಾಶಿ_2);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_cache_ಸೇರಿಸು(ರಾಶಿ_t *ರಾಶಿ_2, ಹರವು_t *ಹರವು) {
  ಆದರೆ (UNEXPECTED(ರಾಶಿ_2->finalize != 0)) {
    rpmalloc_ಹರವು_unmap(ಹರವು);
    rpmalloc_ರಾಶಿ_global_finalize(ರಾಶಿ_2);
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ ENABLE_THREAD_CACHE
  size_t ಹರವು_ಎಣಿಕೆ = ಹರವು->ಹರವು_ಎಣಿಕೆ;
  _rpmalloc_stat_inc(&ರಾಶಿ_2->ಹರವು_use[ಹರವು_ಎಣಿಕೆ - 1].spans_ಗಮ್ಯ_cache);
  ಆದರೆ (ಹರವು_ಎಣಿಕೆ == 1) {
    ಹರವು_cache_t *ಹರವು_cache = &ರಾಶಿ_2->ಹರವು_cache;
    ಹರವು_cache->ಹರವು[ಹರವು_cache->ಎಣಿಕೆ_2++] = ಹರವು;
    ಆದರೆ (ಹರವು_cache->ಎಣಿಕೆ_2 == MAX_THREAD_ಹರವು_CACHE) {
      ಸ್ಥಿರಾಂಕ size_t remain_ಎಣಿಕೆ =
          MAX_THREAD_ಹರವು_CACHE - THREAD_ಹರವು_CACHE_TRANSFER;
#ಆದರೆ ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&ರಾಶಿ_2->thread_ಗಮ್ಯ_global,
                           THREAD_ಹರವು_CACHE_TRANSFER * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
      rpmalloc_stat_ಕೂಡಿಸು(&ರಾಶಿ_2->ಹರವು_use[ಹರವು_ಎಣಿಕೆ - 1].spans_ಗಮ್ಯ_global,
                         THREAD_ಹರವು_CACHE_TRANSFER);
      rpmalloc_global_cache_ಸೇರಿಸು_spans(ಹರವು_cache->ಹರವು + remain_ಎಣಿಕೆ,
                                          ಹರವು_ಎಣಿಕೆ,
                                          THREAD_ಹರವು_CACHE_TRANSFER);
#ಇಲ್ಲದಿದ್ದರೆ
      ಗಾಗಿ (size_t ispan = 0; ispan < THREAD_ಹರವು_CACHE_TRANSFER; ++ispan)
        rpmalloc_ಹರವು_unmap(ಹರವು_cache->ಹರವು[remain_ಎಣಿಕೆ + ispan]);
#ಆದರೆ_ಅಂತ್ಯ
      ಹರವು_cache->ಎಣಿಕೆ_2 = remain_ಎಣಿಕೆ;
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    size_t cache_idx = ಹರವು_ಎಣಿಕೆ - 2;
    ಹರವು_large_cache_t *ಹರವು_cache = ರಾಶಿ_2->ಹರವು_large_cache + cache_idx;
    ಹರವು_cache->ಹರವು[ಹರವು_cache->ಎಣಿಕೆ_2++] = ಹರವು;
    ಸ್ಥಿರಾಂಕ size_t cache_ಎಲ್ಲೆ =
        (MAX_THREAD_ಹರವು_LARGE_CACHE - (ಹರವು_ಎಣಿಕೆ >> 1));
    ಆದರೆ (ಹರವು_cache->ಎಣಿಕೆ_2 == cache_ಎಲ್ಲೆ) {
      ಸ್ಥಿರಾಂಕ size_t transfer_ಎಲ್ಲೆ = 2 + (cache_ಎಲ್ಲೆ >> 2);
      ಸ್ಥಿರಾಂಕ size_t transfer_ಎಣಿಕೆ =
          (THREAD_ಹರವು_LARGE_CACHE_TRANSFER <= transfer_ಎಲ್ಲೆ
               ? THREAD_ಹರವು_LARGE_CACHE_TRANSFER
               : transfer_ಎಲ್ಲೆ);
      ಸ್ಥಿರಾಂಕ size_t remain_ಎಣಿಕೆ = cache_ಎಲ್ಲೆ - transfer_ಎಣಿಕೆ;
#ಆದರೆ ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&ರಾಶಿ_2->thread_ಗಮ್ಯ_global,
                           transfer_ಎಣಿಕೆ * ಹರವು_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
      rpmalloc_stat_ಕೂಡಿಸು(&ರಾಶಿ_2->ಹರವು_use[ಹರವು_ಎಣಿಕೆ - 1].spans_ಗಮ್ಯ_global,
                         transfer_ಎಣಿಕೆ);
      rpmalloc_global_cache_ಸೇರಿಸು_spans(ಹರವು_cache->ಹರವು + remain_ಎಣಿಕೆ,
                                          ಹರವು_ಎಣಿಕೆ, transfer_ಎಣಿಕೆ);
#ಇಲ್ಲದಿದ್ದರೆ
      ಗಾಗಿ (size_t ispan = 0; ispan < transfer_ಎಣಿಕೆ; ++ispan)
        rpmalloc_ಹರವು_unmap(ಹರವು_cache->ಹರವು[remain_ಎಣಿಕೆ + ispan]);
#ಆದರೆ_ಅಂತ್ಯ
      ಹರವು_cache->ಎಣಿಕೆ_2 = remain_ಎಣಿಕೆ;
    }
  }
#ಇಲ್ಲದಿದ್ದರೆ
  (ಶೂನ್ಯ)ಗಾತ್ರ(ರಾಶಿ_2);
  rpmalloc_ಹರವು_unmap(ಹರವು);
#ಆದರೆ_ಅಂತ್ಯ
}

//! Extract the given number of spans from the different cache levels
ಸ್ಥಿರ ಹರವು_t *rpmalloc_ರಾಶಿ_thread_cache_ಹೊರತೆಗೆ(ರಾಶಿ_t *ರಾಶಿ_2,
                                                   size_t ಹರವು_ಎಣಿಕೆ) {
  ಹರವು_t *ಹರವು = 0;
#ಆದರೆ ENABLE_THREAD_CACHE
  ಹರವು_cache_t *ಹರವು_cache;
  ಆದರೆ (ಹರವು_ಎಣಿಕೆ == 1)
    ಹರವು_cache = &ರಾಶಿ_2->ಹರವು_cache;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹರವು_cache = (ಹರವು_cache_t *)(ರಾಶಿ_2->ಹರವು_large_cache + (ಹರವು_ಎಣಿಕೆ - 2));
  ಆದರೆ (ಹರವು_cache->ಎಣಿಕೆ_2) {
    _rpmalloc_stat_inc(&ರಾಶಿ_2->ಹರವು_use[ಹರವು_ಎಣಿಕೆ - 1].spans_from_cache);
    ಹಿಂತಿರುಗಿ ಹರವು_cache->ಹರವು[--ಹರವು_cache->ಎಣಿಕೆ_2];
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ ಹರವು;
}

ಸ್ಥಿರ ಹರವು_t *rpmalloc_ರಾಶಿ_thread_cache_deferred_ಹೊರತೆಗೆ(ರಾಶಿ_t *ರಾಶಿ_2,
                                                            size_t ಹರವು_ಎಣಿಕೆ) {
  ಹರವು_t *ಹರವು = 0;
  ಆದರೆ (ಹರವು_ಎಣಿಕೆ == 1) {
    rpmalloc_ರಾಶಿ_cache_adopt_deferred(ರಾಶಿ_2, &ಹರವು);
  } ಇಲ್ಲದಿದ್ದರೆ {
    rpmalloc_ರಾಶಿ_cache_adopt_deferred(ರಾಶಿ_2, 0);
    ಹರವು = rpmalloc_ರಾಶಿ_thread_cache_ಹೊರತೆಗೆ(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ);
  }
  ಹಿಂತಿರುಗಿ ಹರವು;
}

ಸ್ಥಿರ ಹರವು_t *rpmalloc_ರಾಶಿ_reserved_ಹೊರತೆಗೆ(ರಾಶಿ_t *ರಾಶಿ_2,
                                               size_t ಹರವು_ಎಣಿಕೆ) {
  ಆದರೆ (ರಾಶಿ_2->spans_reserved >= ಹರವು_ಎಣಿಕೆ)
    ಹಿಂತಿರುಗಿ rpmalloc_ಹರವು_map(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ);
  ಹಿಂತಿರುಗಿ 0;
}

//! Extract a span from the global cache
ಸ್ಥಿರ ಹರವು_t *rpmalloc_ರಾಶಿ_global_cache_ಹೊರತೆಗೆ(ರಾಶಿ_t *ರಾಶಿ_2,
                                                   size_t ಹರವು_ಎಣಿಕೆ) {
#ಆದರೆ ENABLE_GLOBAL_CACHE
#ಆದರೆ ENABLE_THREAD_CACHE
  ಹರವು_cache_t *ಹರವು_cache;
  size_t wanted_ಎಣಿಕೆ;
  ಆದರೆ (ಹರವು_ಎಣಿಕೆ == 1) {
    ಹರವು_cache = &ರಾಶಿ_2->ಹರವು_cache;
    wanted_ಎಣಿಕೆ = THREAD_ಹರವು_CACHE_TRANSFER;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಹರವು_cache = (ಹರವು_cache_t *)(ರಾಶಿ_2->ಹರವು_large_cache + (ಹರವು_ಎಣಿಕೆ - 2));
    wanted_ಎಣಿಕೆ = THREAD_ಹರವು_LARGE_CACHE_TRANSFER;
  }
  ಹರವು_cache->ಎಣಿಕೆ_2 = rpmalloc_global_cache_ಹೊರತೆಗೆ_spans(
      ಹರವು_cache->ಹರವು, ಹರವು_ಎಣಿಕೆ, wanted_ಎಣಿಕೆ);
  ಆದರೆ (ಹರವು_cache->ಎಣಿಕೆ_2) {
    _rpmalloc_stat_add64(&ರಾಶಿ_2->global_ಗಮ್ಯ_thread,
                         ಹರವು_ಎಣಿಕೆ * ಹರವು_cache->ಎಣಿಕೆ_2 * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
    rpmalloc_stat_ಕೂಡಿಸು(&ರಾಶಿ_2->ಹರವು_use[ಹರವು_ಎಣಿಕೆ - 1].spans_from_global,
                       ಹರವು_cache->ಎಣಿಕೆ_2);
    ಹಿಂತಿರುಗಿ ಹರವು_cache->ಹರವು[--ಹರವು_cache->ಎಣಿಕೆ_2];
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಹರವು_t *ಹರವು = 0;
  size_t ಎಣಿಕೆ_2 = rpmalloc_global_cache_ಹೊರತೆಗೆ_spans(&ಹರವು, ಹರವು_ಎಣಿಕೆ, 1);
  ಆದರೆ (ಎಣಿಕೆ_2) {
    _rpmalloc_stat_add64(&ರಾಶಿ_2->global_ಗಮ್ಯ_thread,
                         ಹರವು_ಎಣಿಕೆ * ಎಣಿಕೆ_2 * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
    rpmalloc_stat_ಕೂಡಿಸು(&ರಾಶಿ_2->ಹರವು_use[ಹರವು_ಎಣಿಕೆ - 1].spans_from_global,
                       ಎಣಿಕೆ_2);
    ಹಿಂತಿರುಗಿ ಹರವು;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
  (ಶೂನ್ಯ)ಗಾತ್ರ(ರಾಶಿ_2);
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಹರವು_ಎಣಿಕೆ);
  ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_inc_ಹರವು_statistics(ರಾಶಿ_t *ರಾಶಿ_2, size_t ಹರವು_ಎಣಿಕೆ,
                                          uint32_t class_idx) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ರಾಶಿ_2);
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಹರವು_ಎಣಿಕೆ);
  (ಶೂನ್ಯ)ಗಾತ್ರ(class_idx);
#ಆದರೆ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)ಹರವು_ಎಣಿಕೆ - 1;
  uint32_t current_ಎಣಿಕೆ =
      (uint32_t)atomic_incr32(&ರಾಶಿ_2->ಹರವು_use[idx].current);
  ಆದರೆ (current_ಎಣಿಕೆ > (uint32_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[idx].ಮೇಲಿನ))
    atomic_store32(&ರಾಶಿ_2->ಹರವು_use[idx].ಮೇಲಿನ, (int32_t)current_ಎಣಿಕೆ);
  rpmalloc_stat_ಕೂಡಿಸು_peak(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].spans_current, 1,
                          ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].spans_peak);
#ಆದರೆ_ಅಂತ್ಯ
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
ಸ್ಥಿರ ಹರವು_t *
rpmalloc_ರಾಶಿ_ಹೊರತೆಗೆ_new_ಹರವು(ರಾಶಿ_t *ರಾಶಿ_2,
                                ರಾಶಿ_ಗಾತ್ರ_class_t *ರಾಶಿ_ಗಾತ್ರ_class,
                                size_t ಹರವು_ಎಣಿಕೆ, uint32_t class_idx) {
  ಹರವು_t *ಹರವು;
#ಆದರೆ ENABLE_THREAD_CACHE
  ಆದರೆ (ರಾಶಿ_ಗಾತ್ರ_class && ರಾಶಿ_ಗಾತ್ರ_class->cache) {
    ಹರವು = ರಾಶಿ_ಗಾತ್ರ_class->cache;
    ರಾಶಿ_ಗಾತ್ರ_class->cache =
        (ರಾಶಿ_2->ಹರವು_cache.ಎಣಿಕೆ_2
             ? ರಾಶಿ_2->ಹರವು_cache.ಹರವು[--ರಾಶಿ_2->ಹರವು_cache.ಎಣಿಕೆ_2]
             : 0);
    rpmalloc_inc_ಹರವು_statistics(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ, class_idx);
    ಹಿಂತಿರುಗಿ ಹರವು;
  }
#ಆದರೆ_ಅಂತ್ಯ
  (ಶೂನ್ಯ)ಗಾತ್ರ(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t ತಳ_ಹರವು_ಎಣಿಕೆ = ಹರವು_ಎಣಿಕೆ;
  size_t ಎಲ್ಲೆ_ಹರವು_ಎಣಿಕೆ =
      (ಹರವು_ಎಣಿಕೆ > 2) ? (ಹರವು_ಎಣಿಕೆ + (ಹರವು_ಎಣಿಕೆ >> 1)) : ಹರವು_ಎಣಿಕೆ;
  ಆದರೆ (ಎಲ್ಲೆ_ಹರವು_ಎಣಿಕೆ > LARGE_CLASS_ಎಣಿಕೆ)
    ಎಲ್ಲೆ_ಹರವು_ಎಣಿಕೆ = LARGE_CLASS_ಎಣಿಕೆ;
  ಮಾಡು {
    ಹರವು = rpmalloc_ರಾಶಿ_thread_cache_ಹೊರತೆಗೆ(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ);
    ಆದರೆ (EXPECTED(ಹರವು != 0)) {
      _rpmalloc_stat_inc(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_ಹರವು_statistics(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ, class_idx);
      ಹಿಂತಿರುಗಿ ಹರವು;
    }
    ಹರವು = rpmalloc_ರಾಶಿ_thread_cache_deferred_ಹೊರತೆಗೆ(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ);
    ಆದರೆ (EXPECTED(ಹರವು != 0)) {
      _rpmalloc_stat_inc(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_ಹರವು_statistics(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ, class_idx);
      ಹಿಂತಿರುಗಿ ಹರವು;
    }
    ಹರವು = rpmalloc_ರಾಶಿ_global_cache_ಹೊರತೆಗೆ(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ);
    ಆದರೆ (EXPECTED(ಹರವು != 0)) {
      _rpmalloc_stat_inc(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_ಹರವು_statistics(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ, class_idx);
      ಹಿಂತಿರುಗಿ ಹರವು;
    }
    ಹರವು = rpmalloc_ರಾಶಿ_reserved_ಹೊರತೆಗೆ(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ);
    ಆದರೆ (EXPECTED(ಹರವು != 0)) {
      _rpmalloc_stat_inc(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].spans_from_reserved);
      rpmalloc_inc_ಹರವು_statistics(ರಾಶಿ_2, ಹರವು_ಎಣಿಕೆ, class_idx);
      ಹಿಂತಿರುಗಿ ಹರವು;
    }
    ++ಹರವು_ಎಣಿಕೆ;
  } ಆಗ (ಹರವು_ಎಣಿಕೆ <= ಎಲ್ಲೆ_ಹರವು_ಎಣಿಕೆ);
  // Final fallback, map in more virtual memory
  ಹರವು = rpmalloc_ಹರವು_map(ರಾಶಿ_2, ತಳ_ಹರವು_ಎಣಿಕೆ);
  rpmalloc_inc_ಹರವು_statistics(ರಾಶಿ_2, ತಳ_ಹರವು_ಎಣಿಕೆ, class_idx);
  _rpmalloc_stat_inc(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[class_idx].spans_map_calls);
  ಹಿಂತಿರುಗಿ ಹರವು;
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_initialize(ರಾಶಿ_t *ರಾಶಿ_2) {
  _rpmalloc_memset_const(ರಾಶಿ_2, 0, ಗಾತ್ರ(ರಾಶಿ_t));
  // Get a new heap ID
  ರಾಶಿ_2->id = 1 + atomic_incr32(&ಸ್ಮರಣೆ_ರಾಶಿ_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)ರಾಶಿ_2->id % ರಾಶಿ_ARRAY_ಗಾತ್ರ;
  ರಾಶಿ_2->ಮುಂದಿನ_ರಾಶಿ = ಸ್ಮರಣೆ_heaps[list_idx];
  ಸ್ಮರಣೆ_heaps[list_idx] = ರಾಶಿ_2;
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_orphan(ರಾಶಿ_t *ರಾಶಿ_2, ಪೂರ್ಣಾಂಕ ಮೊದಲ_class) {
  ರಾಶಿ_2->owner_thread = (uintptr_t)-1;
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
  ರಾಶಿ_t **ರಾಶಿ_list =
      (ಮೊದಲ_class ? &ಸ್ಮರಣೆ_ಮೊದಲ_class_orphan_heaps : &ಸ್ಮರಣೆ_orphan_heaps);
#ಇಲ್ಲದಿದ್ದರೆ
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಮೊದಲ_class);
  ರಾಶಿ_t **ರಾಶಿ_list = &ಸ್ಮರಣೆ_orphan_heaps;
#ಆದರೆ_ಅಂತ್ಯ
  ರಾಶಿ_2->ಮುಂದಿನ_orphan = *ರಾಶಿ_list;
  *ರಾಶಿ_list = ರಾಶಿ_2;
}

//! Allocate a new heap from newly mapped memory pages
ಸ್ಥಿರ ರಾಶಿ_t *rpmalloc_ರಾಶಿ_allocate_new(ಶೂನ್ಯ) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t ರಾಶಿ_ಗಾತ್ರ_2 = ಗಾತ್ರ(ರಾಶಿ_t);
  size_t aligned_ರಾಶಿ_ಗಾತ್ರ = 16 * ((ರಾಶಿ_ಗಾತ್ರ_2 + 15) / 16);
  size_t request_ರಾಶಿ_ಎಣಿಕೆ = 16;
  size_t ರಾಶಿ_ಹರವು_ಎಣಿಕೆ = ((aligned_ರಾಶಿ_ಗಾತ್ರ * request_ರಾಶಿ_ಎಣಿಕೆ) +
                            ಗಾತ್ರ(ಹರವು_t) + ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ - 1) /
                           ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ;
  size_t ಖಂಡ_ಗಾತ್ರ_2 = ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ * ರಾಶಿ_ಹರವು_ಎಣಿಕೆ;
  size_t ಹರವು_ಎಣಿಕೆ = ರಾಶಿ_ಹರವು_ಎಣಿಕೆ;
  ಹರವು_t *ಹರವು = 0;
  // If there are global reserved spans, use these first
  ಆದರೆ (ಸ್ಮರಣೆ_global_reserve_ಎಣಿಕೆ >= ರಾಶಿ_ಹರವು_ಎಣಿಕೆ) {
    ಹರವು = _rpmalloc_global_get_reserved_spans(ರಾಶಿ_ಹರವು_ಎಣಿಕೆ);
  }
  ಆದರೆ (!ಹರವು) {
    ಆದರೆ (ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ > ಖಂಡ_ಗಾತ್ರ_2) {
      ಹರವು_ಎಣಿಕೆ = ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ / ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ;
      ಖಂಡ_ಗಾತ್ರ_2 = ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_ರಾಶಿ_ಎಣಿಕೆ =
          (ಖಂಡ_ಗಾತ್ರ_2 - ಗಾತ್ರ(ಹರವು_t)) / aligned_ರಾಶಿ_ಗಾತ್ರ;
      ಆದರೆ (possible_ರಾಶಿ_ಎಣಿಕೆ >= (request_ರಾಶಿ_ಎಣಿಕೆ * 16))
        request_ರಾಶಿ_ಎಣಿಕೆ *= 16;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (possible_ರಾಶಿ_ಎಣಿಕೆ < request_ರಾಶಿ_ಎಣಿಕೆ)
        request_ರಾಶಿ_ಎಣಿಕೆ = possible_ರಾಶಿ_ಎಣಿಕೆ;
      ರಾಶಿ_ಹರವು_ಎಣಿಕೆ = ((aligned_ರಾಶಿ_ಗಾತ್ರ * request_ರಾಶಿ_ಎಣಿಕೆ) +
                         ಗಾತ್ರ(ಹರವು_t) + ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ - 1) /
                        ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ;
    }

    size_t align_ಸ್ಥಾನಾಂತರ = 0;
    ಹರವು = (ಹರವು_t *)_rpmalloc_mmap(ಖಂಡ_ಗಾತ್ರ_2, &align_ಸ್ಥಾನಾಂತರ);
    ಆದರೆ (!ಹರವು)
      ಹಿಂತಿರುಗಿ 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    rpmalloc_ಹರವು_initialize(ಹರವು, ಹರವು_ಎಣಿಕೆ, ರಾಶಿ_ಹರವು_ಎಣಿಕೆ, align_ಸ್ಥಾನಾಂತರ);
  }

  size_t remain_ಗಾತ್ರ = ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ - ಗಾತ್ರ(ಹರವು_t);
  ರಾಶಿ_t *ರಾಶಿ_2 = (ರಾಶಿ_t *)ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಗಾತ್ರ(ಹರವು_t));
  rpmalloc_ರಾಶಿ_initialize(ರಾಶಿ_2);

  // Put extra heaps as orphans
  size_t ಅಂಕೆ_heaps = remain_ಗಾತ್ರ / aligned_ರಾಶಿ_ಗಾತ್ರ;
  ಆದರೆ (ಅಂಕೆ_heaps < request_ರಾಶಿ_ಎಣಿಕೆ)
    ಅಂಕೆ_heaps = request_ರಾಶಿ_ಎಣಿಕೆ;
  atomic_store32(&ರಾಶಿ_2->ಮಗು_ಎಣಿಕೆ, (int32_t)ಅಂಕೆ_heaps - 1);
  ರಾಶಿ_t *extra_ರಾಶಿ = (ರಾಶಿ_t *)ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ರಾಶಿ_2, aligned_ರಾಶಿ_ಗಾತ್ರ);
  ಆಗ (ಅಂಕೆ_heaps > 1) {
    rpmalloc_ರಾಶಿ_initialize(extra_ರಾಶಿ);
    extra_ರಾಶಿ->master_ರಾಶಿ = ರಾಶಿ_2;
    rpmalloc_ರಾಶಿ_orphan(extra_ರಾಶಿ, 1);
    extra_ರಾಶಿ = (ರಾಶಿ_t *)ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(extra_ರಾಶಿ, aligned_ರಾಶಿ_ಗಾತ್ರ);
    --ಅಂಕೆ_heaps;
  }

  ಆದರೆ (ಹರವು_ಎಣಿಕೆ > ರಾಶಿ_ಹರವು_ಎಣಿಕೆ) {
    // Cap reserved spans
    size_t remain_ಎಣಿಕೆ = ಹರವು_ಎಣಿಕೆ - ರಾಶಿ_ಹರವು_ಎಣಿಕೆ;
    size_t reserve_ಎಣಿಕೆ =
        (remain_ಎಣಿಕೆ > ಸ್ಮರಣೆ_ರಾಶಿ_reserve_ಎಣಿಕೆ ? ಸ್ಮರಣೆ_ರಾಶಿ_reserve_ಎಣಿಕೆ
                                                   : remain_ಎಣಿಕೆ);
    ಹರವು_t *remain_ಹರವು =
        (ಹರವು_t *)ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ರಾಶಿ_ಹರವು_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
    rpmalloc_ರಾಶಿ_ಗಣ_reserved_spans(ರಾಶಿ_2, ಹರವು, remain_ಹರವು, reserve_ಎಣಿಕೆ);

    ಆದರೆ (remain_ಎಣಿಕೆ > reserve_ಎಣಿಕೆ) {
      // Set to global reserved spans
      remain_ಹರವು = (ಹರವು_t *)ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(remain_ಹರವು,
                                             reserve_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
      reserve_ಎಣಿಕೆ = remain_ಎಣಿಕೆ - reserve_ಎಣಿಕೆ;
      rpmalloc_global_ಗಣ_reserved_spans(ಹರವು, remain_ಹರವು, reserve_ಎಣಿಕೆ);
    }
  }

  ಹಿಂತಿರುಗಿ ರಾಶಿ_2;
}

ಸ್ಥಿರ ರಾಶಿ_t *rpmalloc_ರಾಶಿ_ಹೊರತೆಗೆ_orphan(ರಾಶಿ_t **ರಾಶಿ_list) {
  ರಾಶಿ_t *ರಾಶಿ_2 = *ರಾಶಿ_list;
  *ರಾಶಿ_list = (ರಾಶಿ_2 ? ರಾಶಿ_2->ಮುಂದಿನ_orphan : 0);
  ಹಿಂತಿರುಗಿ ರಾಶಿ_2;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
ಸ್ಥಿರ ರಾಶಿ_t *rpmalloc_ರಾಶಿ_allocate(ಪೂರ್ಣಾಂಕ ಮೊದಲ_class) {
  ರಾಶಿ_t *ರಾಶಿ_2 = 0;
  ಆಗ (!atomic_cas32_acquire(&ಸ್ಮರಣೆ_global_ಬೀಗ, 1, 0))
    _rpmalloc_spin();
  ಆದರೆ (ಮೊದಲ_class == 0)
    ರಾಶಿ_2 = rpmalloc_ರಾಶಿ_ಹೊರತೆಗೆ_orphan(&ಸ್ಮರಣೆ_orphan_heaps);
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
  ಆದರೆ (!ರಾಶಿ_2)
    ರಾಶಿ_2 = rpmalloc_ರಾಶಿ_ಹೊರತೆಗೆ_orphan(&ಸ್ಮರಣೆ_ಮೊದಲ_class_orphan_heaps);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (!ರಾಶಿ_2)
    ರಾಶಿ_2 = rpmalloc_ರಾಶಿ_allocate_new();
  atomic_store32_release(&ಸ್ಮರಣೆ_global_ಬೀಗ, 0);
  ಆದರೆ (ರಾಶಿ_2)
    rpmalloc_ರಾಶಿ_cache_adopt_deferred(ರಾಶಿ_2, 0);
  ಹಿಂತಿರುಗಿ ರಾಶಿ_2;
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_release(ಶೂನ್ಯ *heapptr, ಪೂರ್ಣಾಂಕ ಮೊದಲ_class,
                                   ಪೂರ್ಣಾಂಕ release_cache) {
  ರಾಶಿ_t *ರಾಶಿ_2 = (ರಾಶಿ_t *)heapptr;
  ಆದರೆ (!ರಾಶಿ_2)
    ಹಿಂತಿರುಗಿ;
  // Release thread cache spans back to global cache
  rpmalloc_ರಾಶಿ_cache_adopt_deferred(ರಾಶಿ_2, 0);
  ಆದರೆ (release_cache || ರಾಶಿ_2->finalize) {
#ಆದರೆ ENABLE_THREAD_CACHE
    ಗಾಗಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
      ಹರವು_cache_t *ಹರವು_cache;
      ಆದರೆ (!iclass)
        ಹರವು_cache = &ರಾಶಿ_2->ಹರವು_cache;
      ಇಲ್ಲದಿದ್ದರೆ
        ಹರವು_cache = (ಹರವು_cache_t *)(ರಾಶಿ_2->ಹರವು_large_cache + (iclass - 1));
      ಆದರೆ (!ಹರವು_cache->ಎಣಿಕೆ_2)
        ಮುಂದುವರಿ;
#ಆದರೆ ENABLE_GLOBAL_CACHE
      ಆದರೆ (ರಾಶಿ_2->finalize) {
        ಗಾಗಿ (size_t ispan = 0; ispan < ಹರವು_cache->ಎಣಿಕೆ_2; ++ispan)
          rpmalloc_ಹರವು_unmap(ಹರವು_cache->ಹರವು[ispan]);
      } ಇಲ್ಲದಿದ್ದರೆ {
        _rpmalloc_stat_add64(&ರಾಶಿ_2->thread_ಗಮ್ಯ_global, ಹರವು_cache->ಎಣಿಕೆ_2 *
                                                          (iclass + 1) *
                                                          ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
        rpmalloc_stat_ಕೂಡಿಸು(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_ಗಮ್ಯ_global,
                           ಹರವು_cache->ಎಣಿಕೆ_2);
        rpmalloc_global_cache_ಸೇರಿಸು_spans(ಹರವು_cache->ಹರವು, iclass + 1,
                                            ಹರವು_cache->ಎಣಿಕೆ_2);
      }
#ಇಲ್ಲದಿದ್ದರೆ
      ಗಾಗಿ (size_t ispan = 0; ispan < ಹರವು_cache->ಎಣಿಕೆ_2; ++ispan)
        rpmalloc_ಹರವು_unmap(ಹರವು_cache->ಹರವು[ispan]);
#ಆದರೆ_ಅಂತ್ಯ
      ಹರವು_cache->ಎಣಿಕೆ_2 = 0;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }

  ಆದರೆ (get_thread_ರಾಶಿ_raw() == ರಾಶಿ_2)
    ಗಣ_thread_ರಾಶಿ(0);

#ಆದರೆ ENABLE_STATISTICS
  atomic_decr32(&ಸ್ಮರಣೆ_ಸಕ್ರಿಯ_heaps);
  rpmalloc_assert(atomic_load32(&ಸ್ಮರಣೆ_ಸಕ್ರಿಯ_heaps) >= 0,
                  "Still active heaps during finalization");
#ಆದರೆ_ಅಂತ್ಯ

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  ಆದರೆ (get_thread_id() != _rpmalloc_main_thread_id) {
    ಆಗ (!atomic_cas32_acquire(&ಸ್ಮರಣೆ_global_ಬೀಗ, 1, 0))
      _rpmalloc_spin();
  }
  rpmalloc_ರಾಶಿ_orphan(ರಾಶಿ_2, ಮೊದಲ_class);
  atomic_store32_release(&ಸ್ಮರಣೆ_global_ಬೀಗ, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_release_raw(ಶೂನ್ಯ *heapptr, ಪೂರ್ಣಾಂಕ release_cache) {
  rpmalloc_ರಾಶಿ_release(heapptr, 0, release_cache);
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_release_raw_fc(ಶೂನ್ಯ *heapptr) {
  rpmalloc_ರಾಶಿ_release_raw(heapptr, 1);
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_finalize(ರಾಶಿ_t *ರಾಶಿ_2) {
  ಆದರೆ (ರಾಶಿ_2->spans_reserved) {
    ಹರವು_t *ಹರವು = rpmalloc_ಹರವು_map(ರಾಶಿ_2, ರಾಶಿ_2->spans_reserved);
    rpmalloc_ಹರವು_unmap(ಹರವು);
    ರಾಶಿ_2->spans_reserved = 0;
  }

  rpmalloc_ರಾಶಿ_cache_adopt_deferred(ರಾಶಿ_2, 0);

  ಗಾಗಿ (size_t iclass = 0; iclass < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಆದರೆ (ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].cache)
      rpmalloc_ಹರವು_unmap(ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].cache);
    ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].cache = 0;
    ಹರವು_t *ಹರವು = ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].partial_ಹರವು;
    ಆಗ (ಹರವು) {
      ಹರವು_t *ಮುಂದಿನ = ಹರವು->ಮುಂದಿನ;
      rpmalloc_ಹರವು_finalize(ರಾಶಿ_2, iclass, ಹರವು,
                              &ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].partial_ಹರವು);
      ಹರವು = ಮುಂದಿನ;
    }
    // If class still has a free list it must be a full span
    ಆದರೆ (ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].free_list) {
      ಹರವು_t *class_ಹರವು =
          (ಹರವು_t *)((uintptr_t)ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].free_list &
                     ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ);
      ಹರವು_t **list = 0;
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
      list = &ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು[iclass];
#ಆದರೆ_ಅಂತ್ಯ
      --ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ;
      ಆದರೆ (!rpmalloc_ಹರವು_finalize(ರಾಶಿ_2, iclass, class_ಹರವು, list)) {
        ಆದರೆ (list)
          rpmalloc_ಹರವು_double_link_list_remove(list, class_ಹರವು);
        rpmalloc_ಹರವು_double_link_list_ಕೂಡಿಸು(
            &ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].partial_ಹರವು, class_ಹರವು);
      }
    }
  }

#ಆದರೆ ENABLE_THREAD_CACHE
  ಗಾಗಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಹರವು_cache_t *ಹರವು_cache;
    ಆದರೆ (!iclass)
      ಹರವು_cache = &ರಾಶಿ_2->ಹರವು_cache;
    ಇಲ್ಲದಿದ್ದರೆ
      ಹರವು_cache = (ಹರವು_cache_t *)(ರಾಶಿ_2->ಹರವು_large_cache + (iclass - 1));
    ಗಾಗಿ (size_t ispan = 0; ispan < ಹರವು_cache->ಎಣಿಕೆ_2; ++ispan)
      rpmalloc_ಹರವು_unmap(ಹರವು_cache->ಹರವು[ispan]);
    ಹರವು_cache->ಎಣಿಕೆ_2 = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  rpmalloc_assert(!atomic_ತುಂಬು_ptr(&ರಾಶಿ_2->ಹರವು_free_deferred),
                  "Heaps still active during finalization");
}

////////////
///
/// Allocation entry points
///
//////

//! Pop first block from a free list
ಸ್ಥಿರ ಶೂನ್ಯ *free_list_ತೆಗೆ(ಶೂನ್ಯ **list) {
  ಶೂನ್ಯ *ಖಂಡ = *list;
  *list = *((ಶೂನ್ಯ **)ಖಂಡ);
  ಹಿಂತಿರುಗಿ ಖಂಡ;
}

//! Allocate a small/medium sized memory block from the given heap
ಸ್ಥಿರ ಶೂನ್ಯ *rpmalloc_allocate_from_ರಾಶಿ_fallback(
    ರಾಶಿ_t *ರಾಶಿ_2, ರಾಶಿ_ಗಾತ್ರ_class_t *ರಾಶಿ_ಗಾತ್ರ_class, uint32_t class_idx) {
  ಹರವು_t *ಹರವು = ರಾಶಿ_ಗಾತ್ರ_class->partial_ಹರವು;
  rpmalloc_assume(ರಾಶಿ_2 != 0);
  ಆದರೆ (EXPECTED(ಹರವು != 0)) {
    rpmalloc_assert(ಹರವು->ಖಂಡ_ಎಣಿಕೆ ==
                        ಸ್ಮರಣೆ_ಗಾತ್ರ_class[ಹರವು->ಗಾತ್ರ_class].ಖಂಡ_ಎಣಿಕೆ,
                    "Span block count corrupted");
    rpmalloc_assert(!rpmalloc_ಹರವು_is_fully_utilized(ಹರವು),
                    "Internal failure");
    ಶೂನ್ಯ *ಖಂಡ;
    ಆದರೆ (ಹರವು->free_list) {
      // Span local free list is not empty, swap to size class free list
      ಖಂಡ = free_list_ತೆಗೆ(&ಹರವು->free_list);
      ರಾಶಿ_ಗಾತ್ರ_class->free_list = ಹರವು->free_list;
      ಹರವು->free_list = 0;
    } ಇಲ್ಲದಿದ್ದರೆ {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      ಶೂನ್ಯ *ಖಂಡ_ಆರಂಭ = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(
          ಹರವು, ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ +
                    ((size_t)ಹರವು->free_list_ಎಲ್ಲೆ * ಹರವು->ಖಂಡ_ಗಾತ್ರ_2));
      ಹರವು->free_list_ಎಲ್ಲೆ += free_list_partial_init(
          &ರಾಶಿ_ಗಾತ್ರ_class->free_list, &ಖಂಡ,
          (ಶೂನ್ಯ *)((uintptr_t)ಖಂಡ_ಆರಂಭ & ~(ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ - 1)),
          ಖಂಡ_ಆರಂಭ, ಹರವು->ಖಂಡ_ಎಣಿಕೆ - ಹರವು->free_list_ಎಲ್ಲೆ,
          ಹರವು->ಖಂಡ_ಗಾತ್ರ_2);
    }
    rpmalloc_assert(ಹರವು->free_list_ಎಲ್ಲೆ <= ಹರವು->ಖಂಡ_ಎಣಿಕೆ,
                    "Span block count corrupted");
    ಹರವು->ಬಳಸಿದ_ಎಣಿಕೆ = ಹರವು->free_list_ಎಲ್ಲೆ;

    // Swap in deferred free list if present
    ಆದರೆ (atomic_ತುಂಬು_ptr(&ಹರವು->free_list_deferred))
      rpmalloc_ಹರವು_ಹೊರತೆಗೆ_free_list_deferred(ಹರವು);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    ಆದರೆ (!rpmalloc_ಹರವು_is_fully_utilized(ಹರವು))
      ಹಿಂತಿರುಗಿ ಖಂಡ;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_ಹರವು_double_link_list_ತೆಗೆ_ತಲೆ(&ರಾಶಿ_ಗಾತ್ರ_class->partial_ಹರವು,
                                             ಹರವು);
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
    rpmalloc_ಹರವು_double_link_list_ಕೂಡಿಸು(&ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು[class_idx], ಹರವು);
#ಆದರೆ_ಅಂತ್ಯ
    ++ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ;
    ಹಿಂತಿರುಗಿ ಖಂಡ;
  }

  // Find a span in one of the cache levels
  ಹರವು = rpmalloc_ರಾಶಿ_ಹೊರತೆಗೆ_new_ಹರವು(ರಾಶಿ_2, ರಾಶಿ_ಗಾತ್ರ_class, 1, class_idx);
  ಆದರೆ (EXPECTED(ಹರವು != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    ಹಿಂತಿರುಗಿ rpmalloc_ಹರವು_initialize_new(ರಾಶಿ_2, ರಾಶಿ_ಗಾತ್ರ_class, ಹರವು,
                                         class_idx);
  }

  ಹಿಂತಿರುಗಿ 0;
}

//! Allocate a small sized memory block from the given heap
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_allocate_small(ರಾಶಿ_t *ರಾಶಿ_2, size_t ಗಾತ್ರ_3) {
  rpmalloc_assert(ರಾಶಿ_2, "No thread heap");
  // Small sizes have unique size classes
  ಸ್ಥಿರಾಂಕ uint32_t class_idx =
      (uint32_t)((ಗಾತ್ರ_3 + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_ಸರಿಸು);
  ರಾಶಿ_ಗಾತ್ರ_class_t *ರಾಶಿ_ಗಾತ್ರ_class = ರಾಶಿ_2->ಗಾತ್ರ_class + class_idx;
  _rpmalloc_stat_inc_alloc(ರಾಶಿ_2, class_idx);
  ಆದರೆ (EXPECTED(ರಾಶಿ_ಗಾತ್ರ_class->free_list != 0))
    ಹಿಂತಿರುಗಿ free_list_ತೆಗೆ(&ರಾಶಿ_ಗಾತ್ರ_class->free_list);
  ಹಿಂತಿರುಗಿ rpmalloc_allocate_from_ರಾಶಿ_fallback(ರಾಶಿ_2, ರಾಶಿ_ಗಾತ್ರ_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_allocate_medium(ರಾಶಿ_t *ರಾಶಿ_2, size_t ಗಾತ್ರ_3) {
  rpmalloc_assert(ರಾಶಿ_2, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ಸ್ಥಿರಾಂಕ uint32_t ತಳ_idx =
      (uint32_t)(SMALL_CLASS_ಎಣಿಕೆ +
                 ((ಗಾತ್ರ_3 - (SMALL_ಗಾತ್ರ_ಎಲ್ಲೆ + 1)) >> MEDIUM_GRANULARITY_ಸರಿಸು));
  ಸ್ಥಿರಾಂಕ uint32_t class_idx = ಸ್ಮರಣೆ_ಗಾತ್ರ_class[ತಳ_idx].class_idx;
  ರಾಶಿ_ಗಾತ್ರ_class_t *ರಾಶಿ_ಗಾತ್ರ_class = ರಾಶಿ_2->ಗಾತ್ರ_class + class_idx;
  _rpmalloc_stat_inc_alloc(ರಾಶಿ_2, class_idx);
  ಆದರೆ (EXPECTED(ರಾಶಿ_ಗಾತ್ರ_class->free_list != 0))
    ಹಿಂತಿರುಗಿ free_list_ತೆಗೆ(&ರಾಶಿ_ಗಾತ್ರ_class->free_list);
  ಹಿಂತಿರುಗಿ rpmalloc_allocate_from_ರಾಶಿ_fallback(ರಾಶಿ_2, ರಾಶಿ_ಗಾತ್ರ_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_allocate_large(ರಾಶಿ_t *ರಾಶಿ_2, size_t ಗಾತ್ರ_3) {
  rpmalloc_assert(ರಾಶಿ_2, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  ಗಾತ್ರ_3 += ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ;
  size_t ಹರವು_ಎಣಿಕೆ = ಗಾತ್ರ_3 >> ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ_ಸರಿಸು;
  ಆದರೆ (ಗಾತ್ರ_3 & (ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ - 1))
    ++ಹರವು_ಎಣಿಕೆ;

  // Find a span in one of the cache levels
  ಹರವು_t *ಹರವು =
      rpmalloc_ರಾಶಿ_ಹೊರತೆಗೆ_new_ಹರವು(ರಾಶಿ_2, 0, ಹರವು_ಎಣಿಕೆ, ಗಾತ್ರ_CLASS_LARGE);
  ಆದರೆ (!ಹರವು)
    ಹಿಂತಿರುಗಿ ಹರವು;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(ಹರವು->ಹರವು_ಎಣಿಕೆ >= ಹರವು_ಎಣಿಕೆ, "Internal failure");
  ಹರವು->ಗಾತ್ರ_class = ಗಾತ್ರ_CLASS_LARGE;
  ಹರವು->ರಾಶಿ_2 = ರಾಶಿ_2;

#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
  rpmalloc_ಹರವು_double_link_list_ಕೂಡಿಸು(&ರಾಶಿ_2->large_huge_ಹರವು, ಹರವು);
#ಆದರೆ_ಅಂತ್ಯ
  ++ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ;

  ಹಿಂತಿರುಗಿ ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ);
}

//! Allocate a huge block by mapping memory pages directly
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_allocate_huge(ರಾಶಿ_t *ರಾಶಿ_2, size_t ಗಾತ್ರ_3) {
  rpmalloc_assert(ರಾಶಿ_2, "No thread heap");
  rpmalloc_ರಾಶಿ_cache_adopt_deferred(ರಾಶಿ_2, 0);
  ಗಾತ್ರ_3 += ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ;
  size_t ಅಂಕೆ_ಪುಟಗಳು = ಗಾತ್ರ_3 >> ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ_ಸರಿಸು;
  ಆದರೆ (ಗಾತ್ರ_3 & (ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ - 1))
    ++ಅಂಕೆ_ಪುಟಗಳು;
  size_t align_ಸ್ಥಾನಾಂತರ = 0;
  ಹರವು_t *ಹರವು =
      (ಹರವು_t *)_rpmalloc_mmap(ಅಂಕೆ_ಪುಟಗಳು * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ, &align_ಸ್ಥಾನಾಂತರ);
  ಆದರೆ (!ಹರವು)
    ಹಿಂತಿರುಗಿ ಹರವು;

  // Store page count in span_count
  ಹರವು->ಗಾತ್ರ_class = ಗಾತ್ರ_CLASS_HUGE;
  ಹರವು->ಹರವು_ಎಣಿಕೆ = (uint32_t)ಅಂಕೆ_ಪುಟಗಳು;
  ಹರವು->align_ಸ್ಥಾನಾಂತರ = (uint32_t)align_ಸ್ಥಾನಾಂತರ;
  ಹರವು->ರಾಶಿ_2 = ರಾಶಿ_2;
  rpmalloc_stat_ಕೂಡಿಸು_peak(&huge_ಪುಟಗಳು_current, ಅಂಕೆ_ಪುಟಗಳು, huge_ಪುಟಗಳು_peak);

#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
  rpmalloc_ಹರವು_double_link_list_ಕೂಡಿಸು(&ರಾಶಿ_2->large_huge_ಹರವು, ಹರವು);
#ಆದರೆ_ಅಂತ್ಯ
  ++ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ;

  ಹಿಂತಿರುಗಿ ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ);
}

//! Allocate a block of the given size
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_allocate(ರಾಶಿ_t *ರಾಶಿ_2, size_t ಗಾತ್ರ_3) {
  _rpmalloc_stat_add64(&ಹಂಚಿಕೆ_counter, 1);
  ಆದರೆ (EXPECTED(ಗಾತ್ರ_3 <= SMALL_ಗಾತ್ರ_ಎಲ್ಲೆ))
    ಹಿಂತಿರುಗಿ _rpmalloc_allocate_small(ರಾಶಿ_2, ಗಾತ್ರ_3);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_3 <= ಸ್ಮರಣೆ_medium_ಗಾತ್ರ_ಎಲ್ಲೆ)
    ಹಿಂತಿರುಗಿ _rpmalloc_allocate_medium(ರಾಶಿ_2, ಗಾತ್ರ_3);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_3 <= LARGE_ಗಾತ್ರ_ಎಲ್ಲೆ)
    ಹಿಂತಿರುಗಿ _rpmalloc_allocate_large(ರಾಶಿ_2, ಗಾತ್ರ_3);
  ಹಿಂತಿರುಗಿ _rpmalloc_allocate_huge(ರಾಶಿ_2, ಗಾತ್ರ_3);
}

ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_aligned_allocate(ರಾಶಿ_t *ರಾಶಿ_2, size_t alignment,
                                        size_t ಗಾತ್ರ_3) {
  ಆದರೆ (alignment <= SMALL_GRANULARITY)
    ಹಿಂತಿರುಗಿ _rpmalloc_allocate(ರಾಶಿ_2, ಗಾತ್ರ_3);

#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
  ಆದರೆ ((ಗಾತ್ರ_3 + alignment) < ಗಾತ್ರ_3) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ (alignment & (alignment - 1)) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ ((alignment <= ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ) &&
      ((ಗಾತ್ರ_3 + ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ) < ಸ್ಮರಣೆ_medium_ಗಾತ್ರ_ಎಲ್ಲೆ)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t ಅಪವರ್ತ್ಯ_ಗಾತ್ರ = ಗಾತ್ರ_3 ? (ಗಾತ್ರ_3 + (ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ - 1)) &
                                      ~(uintptr_t)(ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ - 1)
                                : ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ;
    rpmalloc_assert(!(ಅಪವರ್ತ್ಯ_ಗಾತ್ರ % ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ),
                    "Failed alignment calculation");
    ಆದರೆ (ಅಪವರ್ತ್ಯ_ಗಾತ್ರ <= (ಗಾತ್ರ_3 + alignment))
      ಹಿಂತಿರುಗಿ _rpmalloc_allocate(ರಾಶಿ_2, ಅಪವರ್ತ್ಯ_ಗಾತ್ರ);
  }

  ಶೂನ್ಯ *ptr = 0;
  size_t align_ಮುಖವಾಡ = alignment - 1;
  ಆದರೆ (alignment <= ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ) {
    ptr = _rpmalloc_allocate(ರಾಶಿ_2, ಗಾತ್ರ_3 + alignment);
    ಆದರೆ ((uintptr_t)ptr & align_ಮುಖವಾಡ) {
      ptr = (ಶೂನ್ಯ *)(((uintptr_t)ptr & ~(uintptr_t)align_ಮುಖವಾಡ) + alignment);
      // Mark as having aligned blocks
      ಹರವು_t *ಹರವು = (ಹರವು_t *)((uintptr_t)ptr & ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ);
      ಹರವು->flags |= ಹರವು_FLAG_ALIGNED_ಖಂಡಗಳು;
    }
    ಹಿಂತಿರುಗಿ ptr;
  }

  // Fallback to mapping new pages for this request. Since pointers passed
  // to rpfree must be able to reach the start of the span by bitmasking of
  // the address with the span size, the returned aligned pointer from this
  // function must be with a span size of the start of the mapped area.
  // In worst case this requires us to loop and map pages until we get a
  // suitable memory address. It also means we can never align to span size
  // or greater, since the span header will push alignment more than one
  // span size away from span start (thus causing pointer mask to give us
  // an invalid span start on free)
  ಆದರೆ (alignment & align_ಮುಖವಾಡ) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ (alignment >= ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }

  size_t extra_ಪುಟಗಳು = alignment / ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;

  // Since each span has a header, we will at least need one extra memory page
  size_t ಅಂಕೆ_ಪುಟಗಳು = 1 + (ಗಾತ್ರ_3 / ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ);
  ಆದರೆ (ಗಾತ್ರ_3 & (ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ - 1))
    ++ಅಂಕೆ_ಪುಟಗಳು;

  ಆದರೆ (extra_ಪುಟಗಳು > ಅಂಕೆ_ಪುಟಗಳು)
    ಅಂಕೆ_ಪುಟಗಳು = 1 + extra_ಪುಟಗಳು;

  size_t original_ಪುಟಗಳು = ಅಂಕೆ_ಪುಟಗಳು;
  size_t ಎಲ್ಲೆ_ಪುಟಗಳು = (ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ / ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ) * 2;
  ಆದರೆ (ಎಲ್ಲೆ_ಪುಟಗಳು < (original_ಪುಟಗಳು * 2))
    ಎಲ್ಲೆ_ಪುಟಗಳು = original_ಪುಟಗಳು * 2;

  size_t mapped_ಗಾತ್ರ, align_ಸ್ಥಾನಾಂತರ;
  ಹರವು_t *ಹರವು;

retry:
  align_ಸ್ಥಾನಾಂತರ = 0;
  mapped_ಗಾತ್ರ = ಅಂಕೆ_ಪುಟಗಳು * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;

  ಹರವು = (ಹರವು_t *)_rpmalloc_mmap(mapped_ಗಾತ್ರ, &align_ಸ್ಥಾನಾಂತರ);
  ಆದರೆ (!ಹರವು) {
    errno = ENOMEM;
    ಹಿಂತಿರುಗಿ 0;
  }
  ptr = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ);

  ಆದರೆ ((uintptr_t)ptr & align_ಮುಖವಾಡ)
    ptr = (ಶೂನ್ಯ *)(((uintptr_t)ptr & ~(uintptr_t)align_ಮುಖವಾಡ) + alignment);

  ಆದರೆ (((size_t)ಸೂಚಕ_diff(ptr, ಹರವು) >= ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) ||
      (ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ptr, ಗಾತ್ರ_3) > ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, mapped_ಗಾತ್ರ)) ||
      (((uintptr_t)ptr & ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ) != (uintptr_t)ಹರವು)) {
    _rpmalloc_unmap(ಹರವು, mapped_ಗಾತ್ರ, align_ಸ್ಥಾನಾಂತರ, mapped_ಗಾತ್ರ);
    ++ಅಂಕೆ_ಪುಟಗಳು;
    ಆದರೆ (ಅಂಕೆ_ಪುಟಗಳು > ಎಲ್ಲೆ_ಪುಟಗಳು) {
      errno = EINVAL;
      ಹಿಂತಿರುಗಿ 0;
    }
    ಹೋಗು retry;
  }

  // Store page count in span_count
  ಹರವು->ಗಾತ್ರ_class = ಗಾತ್ರ_CLASS_HUGE;
  ಹರವು->ಹರವು_ಎಣಿಕೆ = (uint32_t)ಅಂಕೆ_ಪುಟಗಳು;
  ಹರವು->align_ಸ್ಥಾನಾಂತರ = (uint32_t)align_ಸ್ಥಾನಾಂತರ;
  ಹರವು->ರಾಶಿ_2 = ರಾಶಿ_2;
  rpmalloc_stat_ಕೂಡಿಸು_peak(&huge_ಪುಟಗಳು_current, ಅಂಕೆ_ಪುಟಗಳು, huge_ಪುಟಗಳು_peak);

#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
  rpmalloc_ಹರವು_double_link_list_ಕೂಡಿಸು(&ರಾಶಿ_2->large_huge_ಹರವು, ಹರವು);
#ಆದರೆ_ಅಂತ್ಯ
  ++ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ;

  _rpmalloc_stat_add64(&ಹಂಚಿಕೆ_counter, 1);

  ಹಿಂತಿರುಗಿ ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_deallocate_ನೇರ_small_or_medium(ಹರವು_t *ಹರವು,
                                                        ಶೂನ್ಯ *ಖಂಡ) {
  ರಾಶಿ_t *ರಾಶಿ_2 = ಹರವು->ರಾಶಿ_2;
  rpmalloc_assert(ರಾಶಿ_2->owner_thread == get_thread_id() ||
                      !ರಾಶಿ_2->owner_thread || ರಾಶಿ_2->finalize,
                  "Internal failure");
  // Add block to free list
  ಆದರೆ (UNEXPECTED(rpmalloc_ಹರವು_is_fully_utilized(ಹರವು))) {
    ಹರವು->ಬಳಸಿದ_ಎಣಿಕೆ = ಹರವು->ಖಂಡ_ಎಣಿಕೆ;
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
    rpmalloc_ಹರವು_double_link_list_remove(&ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು[ಹರವು->ಗಾತ್ರ_class],
                                           ಹರವು);
#ಆದರೆ_ಅಂತ್ಯ
    rpmalloc_ಹರವು_double_link_list_ಕೂಡಿಸು(
        &ರಾಶಿ_2->ಗಾತ್ರ_class[ಹರವು->ಗಾತ್ರ_class].partial_ಹರವು, ಹರವು);
    --ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ;
  }
  *((ಶೂನ್ಯ **)ಖಂಡ) = ಹರವು->free_list;
  --ಹರವು->ಬಳಸಿದ_ಎಣಿಕೆ;
  ಹರವು->free_list = ಖಂಡ;
  ಆದರೆ (UNEXPECTED(ಹರವು->ಬಳಸಿದ_ಎಣಿಕೆ == ಹರವು->list_ಗಾತ್ರ)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    ಆದರೆ (ಹರವು->ಬಳಸಿದ_ಎಣಿಕೆ) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      ಶೂನ್ಯ *free_list;
      ಮಾಡು {
        free_list = atomic_exchange_ptr_acquire(&ಹರವು->free_list_deferred,
                                                INVALID_ಸೂಚಕ);
      } ಆಗ (free_list == INVALID_ಸೂಚಕ);
      atomic_store_ptr_release(&ಹರವು->free_list_deferred, free_list);
    }
    rpmalloc_ಹರವು_double_link_list_remove(
        &ರಾಶಿ_2->ಗಾತ್ರ_class[ಹರವು->ಗಾತ್ರ_class].partial_ಹರವು, ಹರವು);
    rpmalloc_ಹರವು_release_ಗಮ್ಯ_cache(ರಾಶಿ_2, ಹರವು);
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_deallocate_defer_free_ಹರವು(ರಾಶಿ_t *ರಾಶಿ_2, ಹರವು_t *ಹರವು) {
  ಆದರೆ (ಹರವು->ಗಾತ್ರ_class != ಗಾತ್ರ_CLASS_HUGE)
    _rpmalloc_stat_inc(&ರಾಶಿ_2->ಹರವು_use[ಹರವು->ಹರವು_ಎಣಿಕೆ - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  ಮಾಡು {
    ಹರವು->free_list = (ಶೂನ್ಯ *)atomic_ತುಂಬು_ptr(&ರಾಶಿ_2->ಹರವು_free_deferred);
  } ಆಗ (!atomic_cas_ptr(&ರಾಶಿ_2->ಹರವು_free_deferred, ಹರವು, ಹರವು->free_list));
}

//! Put the block in the deferred free list of the owning span
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate_defer_small_or_medium(ಹರವು_t *ಹರವು,
                                                       ಶೂನ್ಯ *ಖಂಡ) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  ಶೂನ್ಯ *free_list;
  ಮಾಡು {
    free_list =
        atomic_exchange_ptr_acquire(&ಹರವು->free_list_deferred, INVALID_ಸೂಚಕ);
  } ಆಗ (free_list == INVALID_ಸೂಚಕ);
  *((ಶೂನ್ಯ **)ಖಂಡ) = free_list;
  uint32_t free_ಎಣಿಕೆ = ++ಹರವು->list_ಗಾತ್ರ;
  ಪೂರ್ಣಾಂಕ all_deferred_free = (free_ಎಣಿಕೆ == ಹರವು->ಖಂಡ_ಎಣಿಕೆ);
  atomic_store_ptr_release(&ಹರವು->free_list_deferred, ಖಂಡ);
  ಆದರೆ (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    rpmalloc_deallocate_defer_free_ಹರವು(ಹರವು->ರಾಶಿ_2, ಹರವು);
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate_small_or_medium(ಹರವು_t *ಹರವು, ಶೂನ್ಯ *p) {
  _rpmalloc_stat_inc_free(ಹರವು->ರಾಶಿ_2, ಹರವು->ಗಾತ್ರ_class);
  ಆದರೆ (ಹರವು->flags & ಹರವು_FLAG_ALIGNED_ಖಂಡಗಳು) {
    // Realign pointer to block start
    ಶೂನ್ಯ *ಖಂಡಗಳು_ಆರಂಭ = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ);
    uint32_t ಖಂಡ_ಸ್ಥಾನಾಂತರ = (uint32_t)ಸೂಚಕ_diff(p, ಖಂಡಗಳು_ಆರಂಭ);
    p = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(p, -(int32_t)(ಖಂಡ_ಸ್ಥಾನಾಂತರ % ಹರವು->ಖಂಡ_ಗಾತ್ರ_2));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
  ಪೂರ್ಣಾಂಕ defer =
      (ಹರವು->ರಾಶಿ_2->owner_thread &&
       (ಹರವು->ರಾಶಿ_2->owner_thread != get_thread_id()) && !ಹರವು->ರಾಶಿ_2->finalize);
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ defer =
      ((ಹರವು->ರಾಶಿ_2->owner_thread != get_thread_id()) && !ಹರವು->ರಾಶಿ_2->finalize);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (!defer)
    rpmalloc_deallocate_ನೇರ_small_or_medium(ಹರವು, p);
  ಇಲ್ಲದಿದ್ದರೆ
    _rpmalloc_deallocate_defer_small_or_medium(ಹರವು, p);
}

//! Deallocate the given large memory block to the current heap
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate_large(ಹರವು_t *ಹರವು) {
  rpmalloc_assert(ಹರವು->ಗಾತ್ರ_class == ಗಾತ್ರ_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(ಹರವು->flags & ಹರವು_FLAG_MASTER) ||
                      !(ಹರವು->flags & ಹರವು_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((ಹರವು->flags & ಹರವು_FLAG_MASTER) ||
                      (ಹರವು->flags & ಹರವು_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
  ಪೂರ್ಣಾಂಕ defer =
      (ಹರವು->ರಾಶಿ_2->owner_thread &&
       (ಹರವು->ರಾಶಿ_2->owner_thread != get_thread_id()) && !ಹರವು->ರಾಶಿ_2->finalize);
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ defer =
      ((ಹರವು->ರಾಶಿ_2->owner_thread != get_thread_id()) && !ಹರವು->ರಾಶಿ_2->finalize);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (defer) {
    rpmalloc_deallocate_defer_free_ಹರವು(ಹರವು->ರಾಶಿ_2, ಹರವು);
    ಹಿಂತಿರುಗಿ;
  }
  rpmalloc_assert(ಹರವು->ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ, "Heap span counter corrupted");
  --ಹರವು->ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ;
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
  rpmalloc_ಹರವು_double_link_list_remove(&ಹರವು->ರಾಶಿ_2->large_huge_ಹರವು, ಹರವು);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = ಹರವು->ಹರವು_ಎಣಿಕೆ - 1;
  atomic_decr32(&ಹರವು->ರಾಶಿ_2->ಹರವು_use[idx].current);
#ಆದರೆ_ಅಂತ್ಯ
  ರಾಶಿ_t *ರಾಶಿ_2 = ಹರವು->ರಾಶಿ_2;
  rpmalloc_assert(ರಾಶಿ_2, "No thread heap");
#ಆದರೆ ENABLE_THREAD_CACHE
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಗಣ_as_reserved =
      ((ಹರವು->ಹರವು_ಎಣಿಕೆ > 1) && (ರಾಶಿ_2->ಹರವು_cache.ಎಣಿಕೆ_2 == 0) &&
       !ರಾಶಿ_2->finalize && !ರಾಶಿ_2->spans_reserved);
#ಇಲ್ಲದಿದ್ದರೆ
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಗಣ_as_reserved =
      ((ಹರವು->ಹರವು_ಎಣಿಕೆ > 1) && !ರಾಶಿ_2->finalize && !ರಾಶಿ_2->spans_reserved);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (ಗಣ_as_reserved) {
    ರಾಶಿ_2->ಹರವು_reserve = ಹರವು;
    ರಾಶಿ_2->spans_reserved = ಹರವು->ಹರವು_ಎಣಿಕೆ;
    ಆದರೆ (ಹರವು->flags & ಹರವು_FLAG_MASTER) {
      ರಾಶಿ_2->ಹರವು_reserve_master = ಹರವು;
    } ಇಲ್ಲದಿದ್ದರೆ { // SPAN_FLAG_SUBSPAN
      ಹರವು_t *master = (ಹರವು_t *)ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(
          ಹರವು,
          -(intptr_t)((size_t)ಹರವು->ಸ್ಥಾನಾಂತರ_from_master * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ));
      ರಾಶಿ_2->ಹರವು_reserve_master = master;
      rpmalloc_assert(master->flags & ಹರವು_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->ಉಳಿದ_spans) >=
                          (int32_t)ಹರವು->ಹರವು_ಎಣಿಕೆ,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&ರಾಶಿ_2->ಹರವು_use[idx].spans_ಗಮ್ಯ_reserved);
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Insert into cache list
    rpmalloc_ರಾಶಿ_cache_ಸೇರಿಸು(ರಾಶಿ_2, ಹರವು);
  }
}

//! Deallocate the given huge span
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate_huge(ಹರವು_t *ಹರವು) {
  rpmalloc_assert(ಹರವು->ರಾಶಿ_2, "No span heap");
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
  ಪೂರ್ಣಾಂಕ defer =
      (ಹರವು->ರಾಶಿ_2->owner_thread &&
       (ಹರವು->ರಾಶಿ_2->owner_thread != get_thread_id()) && !ಹರವು->ರಾಶಿ_2->finalize);
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ defer =
      ((ಹರವು->ರಾಶಿ_2->owner_thread != get_thread_id()) && !ಹರವು->ರಾಶಿ_2->finalize);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (defer) {
    rpmalloc_deallocate_defer_free_ಹರವು(ಹರವು->ರಾಶಿ_2, ಹರವು);
    ಹಿಂತಿರುಗಿ;
  }
  rpmalloc_assert(ಹರವು->ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ, "Heap span counter corrupted");
  --ಹರವು->ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ;
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
  rpmalloc_ಹರವು_double_link_list_remove(&ಹರವು->ರಾಶಿ_2->large_huge_ಹರವು, ಹರವು);
#ಆದರೆ_ಅಂತ್ಯ

  // Oversized allocation, page count is stored in span_count
  size_t ಅಂಕೆ_ಪುಟಗಳು = ಹರವು->ಹರವು_ಎಣಿಕೆ;
  _rpmalloc_unmap(ಹರವು, ಅಂಕೆ_ಪುಟಗಳು * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ, ಹರವು->align_ಸ್ಥಾನಾಂತರ,
                  ಅಂಕೆ_ಪುಟಗಳು * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ);
  _rpmalloc_stat_sub(&huge_ಪುಟಗಳು_current, ಅಂಕೆ_ಪುಟಗಳು);
}

//! Deallocate the given block
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate(ಶೂನ್ಯ *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  ಹರವು_t *ಹರವು = (ಹರವು_t *)((uintptr_t)p & ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ);
  ಆದರೆ (UNEXPECTED(!ಹರವು))
    ಹಿಂತಿರುಗಿ;
  ಆದರೆ (EXPECTED(ಹರವು->ಗಾತ್ರ_class < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ))
    _rpmalloc_deallocate_small_or_medium(ಹರವು, p);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಹರವು->ಗಾತ್ರ_class == ಗಾತ್ರ_CLASS_LARGE)
    _rpmalloc_deallocate_large(ಹರವು);
  ಇಲ್ಲದಿದ್ದರೆ
    _rpmalloc_deallocate_huge(ಹರವು);
}

////////////
///
/// Reallocation entry points
///
//////

ಸ್ಥಿರ size_t rpmalloc_usable_ಗಾತ್ರ_2(ಶೂನ್ಯ *p);

//! Reallocate the given block to the given size
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_reallocate(ರಾಶಿ_t *ರಾಶಿ_2, ಶೂನ್ಯ *p, size_t ಗಾತ್ರ_3,
                                  size_t oldsize, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags) {
  ಆದರೆ (p) {
    // Grab the span using guaranteed span alignment
    ಹರವು_t *ಹರವು = (ಹರವು_t *)((uintptr_t)p & ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ);
    ಆದರೆ (EXPECTED(ಹರವು->ಗಾತ್ರ_class < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ)) {
      // Small/medium sized block
      rpmalloc_assert(ಹರವು->ಹರವು_ಎಣಿಕೆ == 1, "Span counter corrupted");
      ಶೂನ್ಯ *ಖಂಡಗಳು_ಆರಂಭ = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ);
      uint32_t ಖಂಡ_ಸ್ಥಾನಾಂತರ = (uint32_t)ಸೂಚಕ_diff(p, ಖಂಡಗಳು_ಆರಂಭ);
      uint32_t ಖಂಡ_idx = ಖಂಡ_ಸ್ಥಾನಾಂತರ / ಹರವು->ಖಂಡ_ಗಾತ್ರ_2;
      ಶೂನ್ಯ *ಖಂಡ =
          ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಖಂಡಗಳು_ಆರಂಭ, (size_t)ಖಂಡ_idx * ಹರವು->ಖಂಡ_ಗಾತ್ರ_2);
      ಆದರೆ (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)ಹರವು->ಖಂಡ_ಗಾತ್ರ_2 - ಸೂಚಕ_diff(p, ಖಂಡ));
      ಆದರೆ ((size_t)ಹರವು->ಖಂಡ_ಗಾತ್ರ_2 >= ಗಾತ್ರ_3) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ಆದರೆ ((p != ಖಂಡ) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(ಖಂಡ, p, oldsize);
        ಹಿಂತಿರುಗಿ ಖಂಡ;
      }
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಹರವು->ಗಾತ್ರ_class == ಗಾತ್ರ_CLASS_LARGE) {
      // Large block
      size_t ಒಟ್ಟು_ಗಾತ್ರ_2 = ಗಾತ್ರ_3 + ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ;
      size_t ಅಂಕೆ_spans = ಒಟ್ಟು_ಗಾತ್ರ_2 >> ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ_ಸರಿಸು;
      ಆದರೆ (ಒಟ್ಟು_ಗಾತ್ರ_2 & (ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ - 1))
        ++ಅಂಕೆ_spans;
      size_t current_spans = ಹರವು->ಹರವು_ಎಣಿಕೆ;
      ಶೂನ್ಯ *ಖಂಡ = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ);
      ಆದರೆ (!oldsize)
        oldsize = (current_spans * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) -
                  (size_t)ಸೂಚಕ_diff(p, ಖಂಡ) - ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ;
      ಆದರೆ ((current_spans >= ಅಂಕೆ_spans) && (ಒಟ್ಟು_ಗಾತ್ರ_2 >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ಆದರೆ ((p != ಖಂಡ) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(ಖಂಡ, p, oldsize);
        ಹಿಂತಿರುಗಿ ಖಂಡ;
      }
    } ಇಲ್ಲದಿದ್ದರೆ {
      // Oversized block
      size_t ಒಟ್ಟು_ಗಾತ್ರ_2 = ಗಾತ್ರ_3 + ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ;
      size_t ಅಂಕೆ_ಪುಟಗಳು = ಒಟ್ಟು_ಗಾತ್ರ_2 >> ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ_ಸರಿಸು;
      ಆದರೆ (ಒಟ್ಟು_ಗಾತ್ರ_2 & (ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ - 1))
        ++ಅಂಕೆ_ಪುಟಗಳು;
      // Page count is stored in span_count
      size_t current_ಪುಟಗಳು = ಹರವು->ಹರವು_ಎಣಿಕೆ;
      ಶೂನ್ಯ *ಖಂಡ = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ);
      ಆದರೆ (!oldsize)
        oldsize = (current_ಪುಟಗಳು * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ) -
                  (size_t)ಸೂಚಕ_diff(p, ಖಂಡ) - ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ;
      ಆದರೆ ((current_ಪುಟಗಳು >= ಅಂಕೆ_ಪುಟಗಳು) && (ಅಂಕೆ_ಪುಟಗಳು >= (current_ಪುಟಗಳು / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ಆದರೆ ((p != ಖಂಡ) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(ಖಂಡ, p, oldsize);
        ಹಿಂತಿರುಗಿ ಖಂಡ;
      }
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    oldsize = 0;
  }

  ಆದರೆ (!!(flags & RPMALLOC_GROW_OR_FAIL))
    ಹಿಂತಿರುಗಿ 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t ಅಧೋ_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_ಗಾತ್ರ =
      (ಗಾತ್ರ_3 > ಅಧೋ_bound) ? ಗಾತ್ರ_3 : ((ಗಾತ್ರ_3 > oldsize) ? ಅಧೋ_bound : ಗಾತ್ರ_3);
  ಶೂನ್ಯ *ಖಂಡ = _rpmalloc_allocate(ರಾಶಿ_2, new_ಗಾತ್ರ);
  ಆದರೆ (p && ಖಂಡ) {
    ಆದರೆ (!(flags & RPMALLOC_NO_PRESERVE))
      ಸ್ಮರಣೆ_ನಕಲು(ಖಂಡ, p, oldsize < new_ಗಾತ್ರ ? oldsize : new_ಗಾತ್ರ);
    _rpmalloc_deallocate(p);
  }

  ಹಿಂತಿರುಗಿ ಖಂಡ;
}

ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_aligned_reallocate(ರಾಶಿ_t *ರಾಶಿ_2, ಶೂನ್ಯ *ptr,
                                          size_t alignment, size_t ಗಾತ್ರ_3,
                                          size_t oldsize, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags) {
  ಆದರೆ (alignment <= SMALL_GRANULARITY)
    ಹಿಂತಿರುಗಿ _rpmalloc_reallocate(ರಾಶಿ_2, ptr, ಗಾತ್ರ_3, oldsize, flags);

  ಪೂರ್ಣಾಂಕ no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? rpmalloc_usable_ಗಾತ್ರ_2(ptr) : 0);
  ಆದರೆ ((usablesize >= ಗಾತ್ರ_3) && !((uintptr_t)ptr & (alignment - 1))) {
    ಆದರೆ (no_alloc || (ಗಾತ್ರ_3 >= (usablesize / 2)))
      ಹಿಂತಿರುಗಿ ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  ಶೂನ್ಯ *ಖಂಡ =
      (!no_alloc ? _rpmalloc_aligned_allocate(ರಾಶಿ_2, alignment, ಗಾತ್ರ_3) : 0);
  ಆದರೆ (EXPECTED(ಖಂಡ != 0)) {
    ಆದರೆ (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      ಆದರೆ (!oldsize)
        oldsize = usablesize;
      ಸ್ಮರಣೆ_ನಕಲು(ಖಂಡ, ptr, oldsize < ಗಾತ್ರ_3 ? oldsize : ಗಾತ್ರ_3);
    }
    _rpmalloc_deallocate(ptr);
  }
  ಹಿಂತಿರುಗಿ ಖಂಡ;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
ಸ್ಥಿರ size_t rpmalloc_usable_ಗಾತ್ರ_2(ಶೂನ್ಯ *p) {
  // Grab the span using guaranteed span alignment
  ಹರವು_t *ಹರವು = (ಹರವು_t *)((uintptr_t)p & ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ);
  ಆದರೆ (ಹರವು->ಗಾತ್ರ_class < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ) {
    // Small/medium block
    ಶೂನ್ಯ *ಖಂಡಗಳು_ಆರಂಭ = ಸೂಚಕ_ಸ್ಥಾನಾಂತರ(ಹರವು, ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ);
    ಹಿಂತಿರುಗಿ ಹರವು->ಖಂಡ_ಗಾತ್ರ_2 -
           ((size_t)ಸೂಚಕ_diff(p, ಖಂಡಗಳು_ಆರಂಭ) % ಹರವು->ಖಂಡ_ಗಾತ್ರ_2);
  }
  ಆದರೆ (ಹರವು->ಗಾತ್ರ_class == ಗಾತ್ರ_CLASS_LARGE) {
    // Large block
    size_t current_spans = ಹರವು->ಹರವು_ಎಣಿಕೆ;
    ಹಿಂತಿರುಗಿ (current_spans * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) - (size_t)ಸೂಚಕ_diff(p, ಹರವು);
  }
  // Oversized block, page count is stored in span_count
  size_t current_ಪುಟಗಳು = ಹರವು->ಹರವು_ಎಣಿಕೆ;
  ಹಿಂತಿರುಗಿ (current_ಪುಟಗಳು * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ) - (size_t)ಸೂಚಕ_diff(p, ಹರವು);
}

//! Adjust and optimize the size class properties for the given class
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_adjust_ಗಾತ್ರ_class(size_t iclass) {
  size_t ಖಂಡ_ಗಾತ್ರ_2 = ಸ್ಮರಣೆ_ಗಾತ್ರ_class[iclass].ಖಂಡ_ಗಾತ್ರ_2;
  size_t ಖಂಡ_ಎಣಿಕೆ = (ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ - ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ) / ಖಂಡ_ಗಾತ್ರ_2;

  ಸ್ಮರಣೆ_ಗಾತ್ರ_class[iclass].ಖಂಡ_ಎಣಿಕೆ = (uint16_t)ಖಂಡ_ಎಣಿಕೆ;
  ಸ್ಮರಣೆ_ಗಾತ್ರ_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  ಆದರೆ (iclass >= SMALL_CLASS_ಎಣಿಕೆ) {
    size_t prevclass = iclass;
    ಆಗ (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      ಆದರೆ (ಸ್ಮರಣೆ_ಗಾತ್ರ_class[prevclass].ಖಂಡ_ಎಣಿಕೆ ==
          ಸ್ಮರಣೆ_ಗಾತ್ರ_class[iclass].ಖಂಡ_ಎಣಿಕೆ)
        _rpmalloc_memcpy_const(ಸ್ಮರಣೆ_ಗಾತ್ರ_class + prevclass,
                               ಸ್ಮರಣೆ_ಗಾತ್ರ_class + iclass,
                               ಗಾತ್ರ(ಸ್ಮರಣೆ_ಗಾತ್ರ_class[iclass]));
      ಇಲ್ಲದಿದ್ದರೆ
        ನಿಲ್ಲಿಸು;
    }
  }
}

//! Initialize the allocator and setup global data
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ rpmalloc_initialize(ಶೂನ್ಯ) {
  ಆದರೆ (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ rpmalloc_initialize_config(0);
}

ಪೂರ್ಣಾಂಕ rpmalloc_initialize_config(ಸ್ಥಿರಾಂಕ rpmalloc_config_t *config) {
  ಆದರೆ (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    ಹಿಂತಿರುಗಿ 0;
  }
  _rpmalloc_initialized = 1;

  ಆದರೆ (config)
    ಸ್ಮರಣೆ_ನಕಲು(&ಸ್ಮರಣೆ_config, config, ಗಾತ್ರ(rpmalloc_config_t));
  ಇಲ್ಲದಿದ್ದರೆ
    _rpmalloc_memset_const(&ಸ್ಮರಣೆ_config, 0, ಗಾತ್ರ(rpmalloc_config_t));

  ಆದರೆ (!ಸ್ಮರಣೆ_config.ಸ್ಮರಣೆ_map || !ಸ್ಮರಣೆ_config.ಸ್ಮರಣೆ_unmap) {
    ಸ್ಮರಣೆ_config.ಸ್ಮರಣೆ_map = _rpmalloc_mmap_os;
    ಸ್ಮರಣೆ_config.ಸ್ಮರಣೆ_unmap = _rpmalloc_unmap_os;
  }

#ಆದರೆ PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, ಗಾತ್ರ(system_info));
  GetSystemInfo(&system_info);
  ಸ್ಮರಣೆ_map_granularity = system_info.dw_ಹಂಚಿಕೆ_Granularity;
#ಇಲ್ಲದಿದ್ದರೆ
  ಸ್ಮರಣೆ_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ RPMALLOC_CONFIGURABLE
  ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ = ಸ್ಮರಣೆ_config.ಪುಟ_ಗಾತ್ರ_2;
#ಇಲ್ಲದಿದ್ದರೆ
  ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ = 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಸ್ಮರಣೆ_huge_ಪುಟಗಳು = 0;
  ಆದರೆ (!ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ) {
#ಆದರೆ PLATFORM_WINDOWS
    ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ = system_info.dw_ಪುಟ_ಗಾತ್ರ;
#ಇಲ್ಲದಿದ್ದರೆ
    ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ = ಸ್ಮರಣೆ_map_granularity;
    ಆದರೆ (ಸ್ಮರಣೆ_config.enable_huge_ಪುಟಗಳು) {
#ಆದರೆ defined(__linux__)
      size_t huge_ಪುಟ_ಗಾತ್ರ = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      ಆದರೆ (meminfo) {
        ಅಕ್ಷರ ಸಾಲು_2[128];
        ಆಗ (!huge_ಪುಟ_ಗಾತ್ರ && fgets(ಸಾಲು_2, ಗಾತ್ರ(ಸಾಲು_2) - 1, meminfo)) {
          ಸಾಲು_2[ಗಾತ್ರ(ಸಾಲು_2) - 1] = 0;
          ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(ಸಾಲು_2, "Hugepagesize:"))
            huge_ಪುಟ_ಗಾತ್ರ = (size_t)strtol(ಸಾಲು_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      ಆದರೆ (huge_ಪುಟ_ಗಾತ್ರ) {
        ಸ್ಮರಣೆ_huge_ಪುಟಗಳು = 1;
        ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ = huge_ಪುಟ_ಗಾತ್ರ;
        ಸ್ಮರಣೆ_map_granularity = huge_ಪುಟ_ಗಾತ್ರ;
      }
#ಅಥವಾಆದರೆ defined(__FreeBSD__)
      ಪೂರ್ಣಾಂಕ rc;
      size_t sz = ಗಾತ್ರ(rc);

      ಆದರೆ (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, ಖಾಲಿ, 0) == 0 &&
          rc == 1) {
        ಸ್ಥಿರ size_t defsize = 2 * 1024 * 1024;
        ಪೂರ್ಣಾಂಕ nsize = 0;
        size_t sizes[4] = {0};
        ಸ್ಮರಣೆ_huge_ಪುಟಗಳು = 1;
        ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ = defsize;
        ಆದರೆ ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          ಗಾಗಿ (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            ಆದರೆ (defsize < csize) {
              ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ = csize;
              ನಿಲ್ಲಿಸು;
            }
          }
        }
        ಸ್ಮರಣೆ_map_granularity = ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
      }
#ಅಥವಾಆದರೆ defined(__APPLE__) || defined(__NetBSD__)
      ಸ್ಮರಣೆ_huge_ಪುಟಗಳು = 1;
      ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ = 2 * 1024 * 1024;
      ಸ್ಮರಣೆ_map_granularity = ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
#ಆದರೆ_ಅಂತ್ಯ
    }
#ಆದರೆ_ಅಂತ್ಯ
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (ಸ್ಮರಣೆ_config.enable_huge_ಪುಟಗಳು)
      ಸ್ಮರಣೆ_huge_ಪುಟಗಳು = 1;
  }

#ಆದರೆ PLATFORM_WINDOWS
  ಆದರೆ (ಸ್ಮರಣೆ_config.enable_huge_ಪುಟಗಳು) {
    HANDLE token = 0;
    size_t large_ಪುಟ_minimum = Get_Large_ಪುಟ_Minimum();
    ಆದರೆ (large_ಪುಟ_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    ಆದರೆ (token) {
      LUID luid;
      ಆದರೆ (Lookup_Privilege_ಮೌಲ್ಯ(0, SE_ಬೀಗ_ಸ್ಮರಣೆ_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, ಗಾತ್ರ(token_privileges));
        token_privileges.Privilege_ಎಣಿಕೆ = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].ಗುಣಲಕ್ಷಣಗಳು = SE_PRIVILEGE_ENABLED;
        ಆದರೆ (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          ಆದರೆ (Get_ಕೊನೆಯ_Error() == ERROR_SUCCESS)
            ಸ್ಮರಣೆ_huge_ಪುಟಗಳು = 1;
        }
      }
      CloseHandle(token);
    }
    ಆದರೆ (ಸ್ಮರಣೆ_huge_ಪುಟಗಳು) {
      ಆದರೆ (large_ಪುಟ_minimum > ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ)
        ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ = large_ಪುಟ_minimum;
      ಆದರೆ (large_ಪುಟ_minimum > ಸ್ಮರಣೆ_map_granularity)
        ಸ್ಮರಣೆ_map_granularity = large_ಪುಟ_minimum;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  size_t min_ಹರವು_ಗಾತ್ರ = 256;
  size_t max_ಪುಟ_ಗಾತ್ರ;
#ಆದರೆ UINTPTR_MAX > 0xFFFFFFFF
  max_ಪುಟ_ಗಾತ್ರ = 4096ULL * 1024ULL * 1024ULL;
#ಇಲ್ಲದಿದ್ದರೆ
  max_ಪುಟ_ಗಾತ್ರ = 4 * 1024 * 1024;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ < min_ಹರವು_ಗಾತ್ರ)
    ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ = min_ಹರವು_ಗಾತ್ರ;
  ಆದರೆ (ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ > max_ಪುಟ_ಗಾತ್ರ)
    ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ = max_ಪುಟ_ಗಾತ್ರ;
  ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ_ಸರಿಸು = 0;
  size_t ಪುಟ_ಗಾತ್ರ_ಬಿಟ್ = ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  ಆಗ (ಪುಟ_ಗಾತ್ರ_ಬಿಟ್ != 1) {
    ++ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ_ಸರಿಸು;
    ಪುಟ_ಗಾತ್ರ_ಬಿಟ್ >>= 1;
  }
  ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ = ((size_t)1 << ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ_ಸರಿಸು);

#ಆದರೆ RPMALLOC_CONFIGURABLE
  ಆದರೆ (!ಸ್ಮರಣೆ_config.ಹರವು_ಗಾತ್ರ) {
    ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ = ಸ್ಮರಣೆ_default_ಹರವು_ಗಾತ್ರ;
    ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ_ಸರಿಸು = ಸ್ಮರಣೆ_default_ಹರವು_ಗಾತ್ರ_ಸರಿಸು;
    ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ = ಸ್ಮರಣೆ_default_ಹರವು_ಮುಖವಾಡ;
  } ಇಲ್ಲದಿದ್ದರೆ {
    size_t ಹರವು_ಗಾತ್ರ = ಸ್ಮರಣೆ_config.ಹರವು_ಗಾತ್ರ;
    ಆದರೆ (ಹರವು_ಗಾತ್ರ > (256 * 1024))
      ಹರವು_ಗಾತ್ರ = (256 * 1024);
    ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ = 4096;
    ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ_ಸರಿಸು = 12;
    ಆಗ (ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ < ಹರವು_ಗಾತ್ರ) {
      ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ <<= 1;
      ++ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ_ಸರಿಸು;
    }
    ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ = ~(uintptr_t)(ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ - 1);
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ =
      (ಸ್ಮರಣೆ_config.ಹರವು_map_ಎಣಿಕೆ ? ಸ್ಮರಣೆ_config.ಹರವು_map_ಎಣಿಕೆ
                                     : DEFAULT_ಹರವು_MAP_ಎಣಿಕೆ);
  ಆದರೆ ((ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ * ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ) < ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ)
    ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ = (ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ / ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
  ಆದರೆ ((ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ >= ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) &&
      ((ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) % ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ))
    ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ = (ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ / ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
  ಸ್ಮರಣೆ_ರಾಶಿ_reserve_ಎಣಿಕೆ = (ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ > DEFAULT_ಹರವು_MAP_ಎಣಿಕೆ)
                                   ? DEFAULT_ಹರವು_MAP_ಎಣಿಕೆ
                                   : ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ;

  ಸ್ಮರಣೆ_config.ಪುಟ_ಗಾತ್ರ_2 = ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  ಸ್ಮರಣೆ_config.ಹರವು_ಗಾತ್ರ = ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ;
  ಸ್ಮರಣೆ_config.ಹರವು_map_ಎಣಿಕೆ = ಸ್ಮರಣೆ_ಹರವು_map_ಎಣಿಕೆ;
  ಸ್ಮರಣೆ_config.enable_huge_ಪುಟಗಳು = ಸ್ಮರಣೆ_huge_ಪುಟಗಳು;

#ಆದರೆ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  ಆದರೆ (pthread_ಕೀಲಿ_create(&ಸ್ಮರಣೆ_thread_ರಾಶಿ, rpmalloc_ರಾಶಿ_release_raw_fc))
    ಹಿಂತಿರುಗಿ -1;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_ಕೀಲಿ = FlsAlloc(&_rpmalloc_thread_destructor);
#ಆದರೆ_ಅಂತ್ಯ

  // Setup all small and medium size classes
  size_t iclass = 0;
  ಸ್ಮರಣೆ_ಗಾತ್ರ_class[iclass].ಖಂಡ_ಗಾತ್ರ_2 = SMALL_GRANULARITY;
  rpmalloc_adjust_ಗಾತ್ರ_class(iclass);
  ಗಾಗಿ (iclass = 1; iclass < SMALL_CLASS_ಎಣಿಕೆ; ++iclass) {
    size_t ಗಾತ್ರ_3 = iclass * SMALL_GRANULARITY;
    ಸ್ಮರಣೆ_ಗಾತ್ರ_class[iclass].ಖಂಡ_ಗಾತ್ರ_2 = (uint32_t)ಗಾತ್ರ_3;
    rpmalloc_adjust_ಗಾತ್ರ_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  ಸ್ಮರಣೆ_medium_ಗಾತ್ರ_ಎಲ್ಲೆ = (ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ - ಹರವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ) >> 1;
  ಆದರೆ (ಸ್ಮರಣೆ_medium_ಗಾತ್ರ_ಎಲ್ಲೆ > MEDIUM_ಗಾತ್ರ_ಎಲ್ಲೆ)
    ಸ್ಮರಣೆ_medium_ಗಾತ್ರ_ಎಲ್ಲೆ = MEDIUM_ಗಾತ್ರ_ಎಲ್ಲೆ;
  ಗಾಗಿ (iclass = 0; iclass < MEDIUM_CLASS_ಎಣಿಕೆ; ++iclass) {
    size_t ಗಾತ್ರ_3 = SMALL_ಗಾತ್ರ_ಎಲ್ಲೆ + ((iclass + 1) * MEDIUM_GRANULARITY);
    ಆದರೆ (ಗಾತ್ರ_3 > ಸ್ಮರಣೆ_medium_ಗಾತ್ರ_ಎಲ್ಲೆ) {
      ಸ್ಮರಣೆ_medium_ಗಾತ್ರ_ಎಲ್ಲೆ =
          SMALL_ಗಾತ್ರ_ಎಲ್ಲೆ + (iclass * MEDIUM_GRANULARITY);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಮರಣೆ_ಗಾತ್ರ_class[SMALL_CLASS_ಎಣಿಕೆ + iclass].ಖಂಡ_ಗಾತ್ರ_2 = (uint32_t)ಗಾತ್ರ_3;
    rpmalloc_adjust_ಗಾತ್ರ_class(SMALL_CLASS_ಎಣಿಕೆ + iclass);
  }

  ಸ್ಮರಣೆ_orphan_heaps = 0;
#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS
  ಸ್ಮರಣೆ_ಮೊದಲ_class_orphan_heaps = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_STATISTICS
  atomic_store32(&ಸ್ಮರಣೆ_ಸಕ್ರಿಯ_heaps, 0);
  atomic_store32(&mapped_ಪುಟಗಳು, 0);
  mapped_ಪುಟಗಳು_peak = 0;
  atomic_store32(&_master_spans, 0);
  atomic_store32(&mapped_ಒಟ್ಟು, 0);
  atomic_store32(&unmapped_ಒಟ್ಟು, 0);
  atomic_store32(&mapped_ಪುಟಗಳು_os, 0);
  atomic_store32(&huge_ಪುಟಗಳು_current, 0);
  huge_ಪುಟಗಳು_peak = 0;
#ಆದರೆ_ಅಂತ್ಯ
  memset(ಸ್ಮರಣೆ_heaps, 0, ಗಾತ್ರ(ಸ್ಮರಣೆ_heaps));
  atomic_store32_release(&ಸ್ಮರಣೆ_global_ಬೀಗ, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  ಹಿಂತಿರುಗಿ 0;
}

//! Finalize the allocator
ಶೂನ್ಯ rpmalloc_finalize(ಶೂನ್ಯ) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  ಆದರೆ (ಸ್ಮರಣೆ_global_reserve) {
    atomic_add32(&ಸ್ಮರಣೆ_global_reserve_master->ಉಳಿದ_spans,
                 -(int32_t)ಸ್ಮರಣೆ_global_reserve_ಎಣಿಕೆ);
    ಸ್ಮರಣೆ_global_reserve_master = 0;
    ಸ್ಮರಣೆ_global_reserve_ಎಣಿಕೆ = 0;
    ಸ್ಮರಣೆ_global_reserve = 0;
  }
  atomic_store32_release(&ಸ್ಮರಣೆ_global_ಬೀಗ, 0);

  // Free all thread caches and fully free spans
  ಗಾಗಿ (size_t list_idx = 0; list_idx < ರಾಶಿ_ARRAY_ಗಾತ್ರ; ++list_idx) {
    ರಾಶಿ_t *ರಾಶಿ_2 = ಸ್ಮರಣೆ_heaps[list_idx];
    ಆಗ (ರಾಶಿ_2) {
      ರಾಶಿ_t *ಮುಂದಿನ_ರಾಶಿ = ರಾಶಿ_2->ಮುಂದಿನ_ರಾಶಿ;
      ರಾಶಿ_2->finalize = 1;
      rpmalloc_ರಾಶಿ_global_finalize(ರಾಶಿ_2);
      ರಾಶಿ_2 = ಮುಂದಿನ_ರಾಶಿ;
    }
  }

#ಆದರೆ ENABLE_GLOBAL_CACHE
  // Free global caches
  ಗಾಗಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass)
    _rpmalloc_global_cache_finalize(&ಸ್ಮರಣೆ_ಹರವು_cache[iclass]);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_ಕೀಲಿ_delete(ಸ್ಮರಣೆ_thread_ರಾಶಿ);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_ಕೀಲಿ);
  fls_ಕೀಲಿ = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&mapped_ಪುಟಗಳು) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&mapped_ಪುಟಗಳು_os) == 0,
                  "Memory leak detected");
#ಆದರೆ_ಅಂತ್ಯ

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ rpmalloc_thread_initialize(ಶೂನ್ಯ) {
  ಆದರೆ (!get_thread_ರಾಶಿ_raw()) {
    ರಾಶಿ_t *ರಾಶಿ_2 = rpmalloc_ರಾಶಿ_allocate(0);
    ಆದರೆ (ರಾಶಿ_2) {
      _rpmalloc_stat_inc(&ಸ್ಮರಣೆ_ಸಕ್ರಿಯ_heaps);
      ಗಣ_thread_ರಾಶಿ(ರಾಶಿ_2);
#ಆದರೆ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_ಗಣ_ಮೌಲ್ಯ(fls_ಕೀಲಿ, ರಾಶಿ_2);
#ಆದರೆ_ಅಂತ್ಯ
    }
  }
}

//! Finalize thread, orphan heap
ಶೂನ್ಯ rpmalloc_thread_finalize(ಪೂರ್ಣಾಂಕ release_caches) {
  ರಾಶಿ_t *ರಾಶಿ_2 = get_thread_ರಾಶಿ_raw();
  ಆದರೆ (ರಾಶಿ_2)
    rpmalloc_ರಾಶಿ_release_raw(ರಾಶಿ_2, release_caches);
  ಗಣ_thread_ರಾಶಿ(0);
#ಆದರೆ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_ಗಣ_ಮೌಲ್ಯ(fls_ಕೀಲಿ, 0);
#ಆದರೆ_ಅಂತ್ಯ
}

ಪೂರ್ಣಾಂಕ rpmalloc_is_thread_initialized(ಶೂನ್ಯ) {
  ಹಿಂತಿರುಗಿ (get_thread_ರಾಶಿ_raw() != 0) ? 1 : 0;
}

ಸ್ಥಿರಾಂಕ rpmalloc_config_t *rpmalloc_config(ಶೂನ್ಯ) { ಹಿಂತಿರುಗಿ &ಸ್ಮರಣೆ_config; }

// Extern interface

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *rpmalloc(size_t ಗಾತ್ರ_3) {
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
  ಆದರೆ (ಗಾತ್ರ_3 >= MAX_ALLOC_ಗಾತ್ರ) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ರಾಶಿ_t *ರಾಶಿ_2 = get_thread_ರಾಶಿ();
  ಹಿಂತಿರುಗಿ _rpmalloc_allocate(ರಾಶಿ_2, ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ rpfree(ಶೂನ್ಯ *ptr) { _rpmalloc_deallocate(ptr); }

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *rpcalloc(size_t ಅಂಕೆ_2, size_t ಗಾತ್ರ_3) {
  size_t ಒಟ್ಟು_2;
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
#ಆದರೆ PLATFORM_WINDOWS
  ಪೂರ್ಣಾಂಕ err = ಗಾತ್ರ_TMult(ಅಂಕೆ_2, ಗಾತ್ರ_3, &ಒಟ್ಟು_2);
  ಆದರೆ ((err != S_OK) || (ಒಟ್ಟು_2 >= MAX_ALLOC_ಗಾತ್ರ)) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ err = __builtin_umull_overflow(ಅಂಕೆ_2, ಗಾತ್ರ_3, &ಒಟ್ಟು_2);
  ಆದರೆ (err || (ಒಟ್ಟು_2 >= MAX_ALLOC_ಗಾತ್ರ)) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
  ಒಟ್ಟು_2 = ಅಂಕೆ_2 * ಗಾತ್ರ_3;
#ಆದರೆ_ಅಂತ್ಯ
  ರಾಶಿ_t *ರಾಶಿ_2 = get_thread_ರಾಶಿ();
  ಶೂನ್ಯ *ಖಂಡ = _rpmalloc_allocate(ರಾಶಿ_2, ಒಟ್ಟು_2);
  ಆದರೆ (ಖಂಡ)
    memset(ಖಂಡ, 0, ಒಟ್ಟು_2);
  ಹಿಂತಿರುಗಿ ಖಂಡ;
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *rprealloc(ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_3) {
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
  ಆದರೆ (ಗಾತ್ರ_3 >= MAX_ALLOC_ಗಾತ್ರ) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ ptr;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ರಾಶಿ_t *ರಾಶಿ_2 = get_thread_ರಾಶಿ();
  ಹಿಂತಿರುಗಿ _rpmalloc_reallocate(ರಾಶಿ_2, ptr, ಗಾತ್ರ_3, 0, 0);
}

ಬಾಹ್ಯ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *rpaligned_realloc(ಶೂನ್ಯ *ptr, size_t alignment,
                                                  size_t ಗಾತ್ರ_3, size_t oldsize,
                                                  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags) {
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
  ಆದರೆ ((ಗಾತ್ರ_3 + alignment < ಗಾತ್ರ_3) || (alignment > ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ)) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ರಾಶಿ_t *ರಾಶಿ_2 = get_thread_ರಾಶಿ();
  ಹಿಂತಿರುಗಿ _rpmalloc_aligned_reallocate(ರಾಶಿ_2, ptr, alignment, ಗಾತ್ರ_3, oldsize,
                                      flags);
}

ಬಾಹ್ಯ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *rpaligned_alloc(size_t alignment, size_t ಗಾತ್ರ_3) {
  ರಾಶಿ_t *ರಾಶಿ_2 = get_thread_ರಾಶಿ();
  ಹಿಂತಿರುಗಿ _rpmalloc_aligned_allocate(ರಾಶಿ_2, alignment, ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpaligned_calloc(size_t alignment, size_t ಅಂಕೆ_2, size_t ಗಾತ್ರ_3) {
  size_t ಒಟ್ಟು_2;
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
#ಆದರೆ PLATFORM_WINDOWS
  ಪೂರ್ಣಾಂಕ err = ಗಾತ್ರ_TMult(ಅಂಕೆ_2, ಗಾತ್ರ_3, &ಒಟ್ಟು_2);
  ಆದರೆ ((err != S_OK) || (ಒಟ್ಟು_2 >= MAX_ALLOC_ಗಾತ್ರ)) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ err = __builtin_umull_overflow(ಅಂಕೆ_2, ಗಾತ್ರ_3, &ಒಟ್ಟು_2);
  ಆದರೆ (err || (ಒಟ್ಟು_2 >= MAX_ALLOC_ಗಾತ್ರ)) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
  ಒಟ್ಟು_2 = ಅಂಕೆ_2 * ಗಾತ್ರ_3;
#ಆದರೆ_ಅಂತ್ಯ
  ಶೂನ್ಯ *ಖಂಡ = rpaligned_alloc(alignment, ಒಟ್ಟು_2);
  ಆದರೆ (ಖಂಡ)
    memset(ಖಂಡ, 0, ಒಟ್ಟು_2);
  ಹಿಂತಿರುಗಿ ಖಂಡ;
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *rpmemalign(size_t alignment,
                                                  size_t ಗಾತ್ರ_3) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(alignment, ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ rpposix_memalign(ಶೂನ್ಯ **memptr, size_t alignment,
                                   size_t ಗಾತ್ರ_3) {
  ಆದರೆ (memptr)
    *memptr = rpaligned_alloc(alignment, ಗಾತ್ರ_3);
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ EINVAL;
  ಹಿಂತಿರುಗಿ *memptr ? 0 : ENOMEM;
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ size_t rpmalloc_usable_ಗಾತ್ರ(ಶೂನ್ಯ *ptr) {
  ಹಿಂತಿರುಗಿ (ptr ? rpmalloc_usable_ಗಾತ್ರ_2(ptr) : 0);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ rpmalloc_thread_collect(ಶೂನ್ಯ) {}

ಶೂನ್ಯ rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, ಗಾತ್ರ(rpmalloc_thread_statistics_t));
  ರಾಶಿ_t *ರಾಶಿ_2 = get_thread_ರಾಶಿ_raw();
  ಆದರೆ (!ರಾಶಿ_2)
    ಹಿಂತಿರುಗಿ;

  ಗಾಗಿ (size_t iclass = 0; iclass < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಗಾತ್ರ_class_t *ಗಾತ್ರ_class = ಸ್ಮರಣೆ_ಗಾತ್ರ_class + iclass;
    ಹರವು_t *ಹರವು = ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].partial_ಹರವು;
    ಆಗ (ಹರವು) {
      size_t free_ಎಣಿಕೆ = ಹರವು->list_ಗಾತ್ರ;
      size_t ಖಂಡ_ಎಣಿಕೆ = ಗಾತ್ರ_class->ಖಂಡ_ಎಣಿಕೆ;
      ಆದರೆ (ಹರವು->free_list_ಎಲ್ಲೆ < ಖಂಡ_ಎಣಿಕೆ)
        ಖಂಡ_ಎಣಿಕೆ = ಹರವು->free_list_ಎಲ್ಲೆ;
      free_ಎಣಿಕೆ += (ಖಂಡ_ಎಣಿಕೆ - ಹರವು->ಬಳಸಿದ_ಎಣಿಕೆ);
      stats->sizecache += free_ಎಣಿಕೆ * ಗಾತ್ರ_class->ಖಂಡ_ಗಾತ್ರ_2;
      ಹರವು = ಹರವು->ಮುಂದಿನ;
    }
  }

#ಆದರೆ ENABLE_THREAD_CACHE
  ಗಾಗಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಹರವು_cache_t *ಹರವು_cache;
    ಆದರೆ (!iclass)
      ಹರವು_cache = &ರಾಶಿ_2->ಹರವು_cache;
    ಇಲ್ಲದಿದ್ದರೆ
      ಹರವು_cache = (ಹರವು_cache_t *)(ರಾಶಿ_2->ಹರವು_large_cache + (iclass - 1));
    stats->spancache += ಹರವು_cache->ಎಣಿಕೆ_2 * (iclass + 1) * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಹರವು_t *deferred = (ಹರವು_t *)atomic_ತುಂಬು_ptr(&ರಾಶಿ_2->ಹರವು_free_deferred);
  ಆಗ (deferred) {
    ಆದರೆ (deferred->ಗಾತ್ರ_class != ಗಾತ್ರ_CLASS_HUGE)
      stats->spancache += (size_t)deferred->ಹರವು_ಎಣಿಕೆ * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ;
    deferred = (ಹರವು_t *)deferred->free_list;
  }

#ಆದರೆ ENABLE_STATISTICS
  stats->thread_ಗಮ್ಯ_global = (size_t)atomic_load64(&ರಾಶಿ_2->thread_ಗಮ್ಯ_global);
  stats->global_ಗಮ್ಯ_thread = (size_t)atomic_load64(&ರಾಶಿ_2->global_ಗಮ್ಯ_thread);

  ಗಾಗಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    stats->ಹರವು_use[iclass].current =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].current);
    stats->ಹರವು_use[iclass].peak =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].ಮೇಲಿನ);
    stats->ಹರವು_use[iclass].ಗಮ್ಯ_global =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_ಗಮ್ಯ_global);
    stats->ಹರವು_use[iclass].from_global =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_from_global);
    stats->ಹರವು_use[iclass].ಗಮ್ಯ_cache =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_ಗಮ್ಯ_cache);
    stats->ಹರವು_use[iclass].from_cache =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_from_cache);
    stats->ಹರವು_use[iclass].ಗಮ್ಯ_reserved =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_ಗಮ್ಯ_reserved);
    stats->ಹರವು_use[iclass].from_reserved =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_from_reserved);
    stats->ಹರವು_use[iclass].map_calls =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_map_calls);
  }
  ಗಾಗಿ (size_t iclass = 0; iclass < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ; ++iclass) {
    stats->ಗಾತ್ರ_use[iclass].alloc_current =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].alloc_current);
    stats->ಗಾತ್ರ_use[iclass].alloc_peak =
        (size_t)ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].alloc_peak;
    stats->ಗಾತ್ರ_use[iclass].alloc_ಒಟ್ಟು =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].alloc_ಒಟ್ಟು);
    stats->ಗಾತ್ರ_use[iclass].free_ಒಟ್ಟು =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].free_ಒಟ್ಟು);
    stats->ಗಾತ್ರ_use[iclass].spans_ಗಮ್ಯ_cache =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_ಗಮ್ಯ_cache);
    stats->ಗಾತ್ರ_use[iclass].spans_from_cache =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_from_cache);
    stats->ಗಾತ್ರ_use[iclass].spans_from_reserved = (size_t)atomic_load32(
        &ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_from_reserved);
    stats->ಗಾತ್ರ_use[iclass].map_calls =
        (size_t)atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_map_calls);
  }
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, ಗಾತ್ರ(rpmalloc_global_statistics_t));
#ಆದರೆ ENABLE_STATISTICS
  stats->mapped = (size_t)atomic_load32(&mapped_ಪುಟಗಳು) * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  stats->mapped_peak = (size_t)mapped_ಪುಟಗಳು_peak * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  stats->mapped_ಒಟ್ಟು_2 =
      (size_t)atomic_load32(&mapped_ಒಟ್ಟು) * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  stats->unmapped_ಒಟ್ಟು_2 =
      (size_t)atomic_load32(&unmapped_ಒಟ್ಟು) * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  stats->huge_alloc =
      (size_t)atomic_load32(&huge_ಪುಟಗಳು_current) * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  stats->huge_alloc_peak = (size_t)huge_ಪುಟಗಳು_peak * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_GLOBAL_CACHE
  ಗಾಗಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    global_cache_t *cache = &ಸ್ಮರಣೆ_ಹರವು_cache[iclass];
    ಆಗ (!atomic_cas32_acquire(&cache->ಬೀಗ_2, 1, 0))
      _rpmalloc_spin();
    uint32_t ಎಣಿಕೆ_2 = cache->ಎಣಿಕೆ_2;
#ಆದರೆ ENABLE_UNLIMITED_CACHE
    ಹರವು_t *current_ಹರವು = cache->ಅತಿಪ್ರವಾಹ_2;
    ಆಗ (current_ಹರವು) {
      ++ಎಣಿಕೆ_2;
      current_ಹರವು = current_ಹರವು->ಮುಂದಿನ;
    }
#ಆದರೆ_ಅಂತ್ಯ
    atomic_store32_release(&cache->ಬೀಗ_2, 0);
    stats->cached += ಎಣಿಕೆ_2 * (iclass + 1) * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ;
  }
#ಆದರೆ_ಅಂತ್ಯ
}

#ಆದರೆ ENABLE_STATISTICS

ಸ್ಥಿರ ಶೂನ್ಯ ಸ್ಮರಣೆ_ರಾಶಿ_dump_statistics(ರಾಶಿ_t *ರಾಶಿ_2, ಶೂನ್ಯ *file) {
  fprintf(file, "Heap %d stats:\n", ರಾಶಿ_2->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  ಗಾಗಿ (size_t iclass = 0; iclass < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಆದರೆ (!atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].alloc_ಒಟ್ಟು))
      ಮುಂದುವರಿ;
    fprintf(
        file,
        "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu "
        "%9u\n",
        (uint32_t)iclass,
        atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].alloc_current),
        ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].alloc_peak,
        atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].alloc_ಒಟ್ಟು),
        atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].free_ಒಟ್ಟು),
        ಸ್ಮರಣೆ_ಗಾತ್ರ_class[iclass].ಖಂಡ_ಗಾತ್ರ_2,
        ಸ್ಮರಣೆ_ಗಾತ್ರ_class[iclass].ಖಂಡ_ಎಣಿಕೆ,
        atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_current),
        ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_peak,
        ((size_t)ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].alloc_peak *
         (size_t)ಸ್ಮರಣೆ_ಗಾತ್ರ_class[iclass].ಖಂಡ_ಗಾತ್ರ_2) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_ಗಮ್ಯ_cache) *
         ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_from_cache) *
         ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(
             &ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_from_reserved) *
         ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) /
            (size_t)(1024 * 1024),
        atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_map_calls));
  }
  fprintf(file, "Spans  Current     Peak Deferred  PeakMiB  Cached  ToCacheMiB "
                "FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB "
                "FromGlobalMiB  MmapCalls\n");
  ಗಾಗಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಆದರೆ (!atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].ಮೇಲಿನ) &&
        !atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_map_calls))
      ಮುಂದುವರಿ;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].current),
        atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].ಮೇಲಿನ),
        atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].ಮೇಲಿನ) *
         (size_t)ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ * (iclass + 1)) /
            (size_t)(1024 * 1024),
#ಆದರೆ ENABLE_THREAD_CACHE
        (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(!iclass ? ರಾಶಿ_2->ಹರವು_cache.ಎಣಿಕೆ_2
                               : ರಾಶಿ_2->ಹರವು_large_cache[iclass - 1].ಎಣಿಕೆ_2),
        ((size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_ಗಮ್ಯ_cache) *
         (iclass + 1) * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_from_cache) *
         (iclass + 1) * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) /
            (size_t)(1024 * 1024),
#ಇಲ್ಲದಿದ್ದರೆ
        0, (size_t)0, (size_t)0,
#ಆದರೆ_ಅಂತ್ಯ
        ((size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_ಗಮ್ಯ_reserved) *
         (iclass + 1) * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_from_reserved) *
         (iclass + 1) * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_ಗಮ್ಯ_global) *
         (size_t)ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ * (iclass + 1)) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_from_global) *
         (size_t)ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ * (iclass + 1)) /
            (size_t)(1024 * 1024),
        atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_map_calls));
  }
  fprintf(file, "Full spans: %zu\n", ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&ರಾಶಿ_2->thread_ಗಮ್ಯ_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&ರಾಶಿ_2->global_ಗಮ್ಯ_thread) / (size_t)(1024 * 1024));
}

#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ rpmalloc_dump_statistics(ಶೂನ್ಯ *file) {
#ಆದರೆ ENABLE_STATISTICS
  ಗಾಗಿ (size_t list_idx = 0; list_idx < ರಾಶಿ_ARRAY_ಗಾತ್ರ; ++list_idx) {
    ರಾಶಿ_t *ರಾಶಿ_2 = ಸ್ಮರಣೆ_heaps[list_idx];
    ಆಗ (ರಾಶಿ_2) {
      ಪೂರ್ಣಾಂಕ need_dump = 0;
      ಗಾಗಿ (size_t iclass = 0; !need_dump && (iclass < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ);
           ++iclass) {
        ಆದರೆ (!atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].alloc_ಒಟ್ಟು)) {
          rpmalloc_assert(
              !atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].free_ಒಟ್ಟು),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          ಮುಂದುವರಿ;
        }
        need_dump = 1;
      }
      ಗಾಗಿ (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_ಎಣಿಕೆ);
           ++iclass) {
        ಆದರೆ (!atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].ಮೇಲಿನ) &&
            !atomic_load32(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_map_calls))
          ಮುಂದುವರಿ;
        need_dump = 1;
      }
      ಆದರೆ (need_dump)
        ಸ್ಮರಣೆ_ರಾಶಿ_dump_statistics(ರಾಶಿ_2, file);
      ರಾಶಿ_2 = ರಾಶಿ_2->ಮುಂದಿನ_ರಾಶಿ;
    }
  }
  fprintf(file, "Global stats:\n");
  size_t huge_current =
      (size_t)atomic_load32(&huge_ಪುಟಗಳು_current) * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  size_t huge_peak = (size_t)huge_ಪುಟಗಳು_peak * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  fprintf(file, "HugeCurrentMiB HugePeakMiB\n");
  fprintf(file, "%14zu %11zu\n", huge_current / (size_t)(1024 * 1024),
          huge_peak / (size_t)(1024 * 1024));

#ಆದರೆ ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  ಗಾಗಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    global_cache_t *cache = ಸ್ಮರಣೆ_ಹರವು_cache + iclass;
    size_t global_cache = (size_t)cache->ಎಣಿಕೆ_2 * iclass * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ;

    size_t global_ಅತಿಪ್ರವಾಹ_cache = 0;
    ಹರವು_t *ಹರವು = cache->ಅತಿಪ್ರವಾಹ_2;
    ಆಗ (ಹರವು) {
      global_ಅತಿಪ್ರವಾಹ_cache += iclass * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ;
      ಹರವು = ಹರವು->ಮುಂದಿನ;
    }
    ಆದರೆ (global_cache || global_ಅತಿಪ್ರವಾಹ_cache || cache->ಸೇರಿಸು_ಎಣಿಕೆ ||
        cache->ಹೊರತೆಗೆ_ಎಣಿಕೆ)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_ಅತಿಪ್ರವಾಹ_cache / (size_t)(1024 * 1024),
              cache->ಸೇರಿಸು_ಎಣಿಕೆ, cache->ಹೊರತೆಗೆ_ಎಣಿಕೆ);
  }
#ಆದರೆ_ಅಂತ್ಯ

  size_t mapped = (size_t)atomic_load32(&mapped_ಪುಟಗಳು) * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  size_t mapped_os =
      (size_t)atomic_load32(&mapped_ಪುಟಗಳು_os) * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  size_t mapped_peak = (size_t)mapped_ಪುಟಗಳು_peak * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  size_t mapped_ಒಟ್ಟು_2 =
      (size_t)atomic_load32(&mapped_ಒಟ್ಟು) * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  size_t unmapped_ಒಟ್ಟು_2 =
      (size_t)atomic_load32(&unmapped_ಒಟ್ಟು) * ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
  fprintf(
      file,
      "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB\n");
  fprintf(file, "%9zu %11zu %13zu %14zu %16zu\n",
          mapped / (size_t)(1024 * 1024), mapped_os / (size_t)(1024 * 1024),
          mapped_peak / (size_t)(1024 * 1024),
          mapped_ಒಟ್ಟು_2 / (size_t)(1024 * 1024),
          unmapped_ಒಟ್ಟು_2 / (size_t)(1024 * 1024));

  fprintf(file, "\n");
#ಆದರೆ 0
	int64_t allocated = atomic_load64(&ಹಂಚಿಕೆ_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
  (ಶೂನ್ಯ)ಗಾತ್ರ(file);
}

#ಆದರೆ RPMALLOC_ಮೊದಲ_CLASS_HEAPS

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ rpmalloc_ರಾಶಿ_t *rpmalloc_ರಾಶಿ_acquire(ಶೂನ್ಯ) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  ರಾಶಿ_t *ರಾಶಿ_2 = rpmalloc_ರಾಶಿ_allocate(1);
  rpmalloc_assume(ರಾಶಿ_2 != ಖಾಲಿ);
  ರಾಶಿ_2->owner_thread = 0;
  _rpmalloc_stat_inc(&ಸ್ಮರಣೆ_ಸಕ್ರಿಯ_heaps);
  ಹಿಂತಿರುಗಿ ರಾಶಿ_2;
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_release_2(rpmalloc_ರಾಶಿ_t *ರಾಶಿ_2) {
  ಆದರೆ (ರಾಶಿ_2)
    rpmalloc_ರಾಶಿ_release(ರಾಶಿ_2, 1, 1);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpmalloc_ರಾಶಿ_alloc(rpmalloc_ರಾಶಿ_t *ರಾಶಿ_2, size_t ಗಾತ್ರ_3) {
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
  ಆದರೆ (ಗಾತ್ರ_3 >= MAX_ALLOC_ಗಾತ್ರ) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ _rpmalloc_allocate(ರಾಶಿ_2, ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpmalloc_ರಾಶಿ_aligned_alloc(rpmalloc_ರಾಶಿ_t *ರಾಶಿ_2, size_t alignment,
                            size_t ಗಾತ್ರ_3) {
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
  ಆದರೆ (ಗಾತ್ರ_3 >= MAX_ALLOC_ಗಾತ್ರ) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ _rpmalloc_aligned_allocate(ರಾಶಿ_2, alignment, ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpmalloc_ರಾಶಿ_calloc(rpmalloc_ರಾಶಿ_t *ರಾಶಿ_2, size_t ಅಂಕೆ_2, size_t ಗಾತ್ರ_3) {
  ಹಿಂತಿರುಗಿ rpmalloc_ರಾಶಿ_aligned_calloc(ರಾಶಿ_2, 0, ಅಂಕೆ_2, ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpmalloc_ರಾಶಿ_aligned_calloc(rpmalloc_ರಾಶಿ_t *ರಾಶಿ_2, size_t alignment,
                             size_t ಅಂಕೆ_2, size_t ಗಾತ್ರ_3) {
  size_t ಒಟ್ಟು_2;
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
#ಆದರೆ PLATFORM_WINDOWS
  ಪೂರ್ಣಾಂಕ err = ಗಾತ್ರ_TMult(ಅಂಕೆ_2, ಗಾತ್ರ_3, &ಒಟ್ಟು_2);
  ಆದರೆ ((err != S_OK) || (ಒಟ್ಟು_2 >= MAX_ALLOC_ಗಾತ್ರ)) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ err = __builtin_umull_overflow(ಅಂಕೆ_2, ಗಾತ್ರ_3, &ಒಟ್ಟು_2);
  ಆದರೆ (err || (ಒಟ್ಟು_2 >= MAX_ALLOC_ಗಾತ್ರ)) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
  ಒಟ್ಟು_2 = ಅಂಕೆ_2 * ಗಾತ್ರ_3;
#ಆದರೆ_ಅಂತ್ಯ
  ಶೂನ್ಯ *ಖಂಡ = _rpmalloc_aligned_allocate(ರಾಶಿ_2, alignment, ಒಟ್ಟು_2);
  ಆದರೆ (ಖಂಡ)
    memset(ಖಂಡ, 0, ಒಟ್ಟು_2);
  ಹಿಂತಿರುಗಿ ಖಂಡ;
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpmalloc_ರಾಶಿ_realloc(rpmalloc_ರಾಶಿ_t *ರಾಶಿ_2, ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_3,
                      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags) {
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
  ಆದರೆ (ಗಾತ್ರ_3 >= MAX_ALLOC_ಗಾತ್ರ) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ ptr;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ _rpmalloc_reallocate(ರಾಶಿ_2, ptr, ಗಾತ್ರ_3, 0, flags);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpmalloc_ರಾಶಿ_aligned_realloc(rpmalloc_ರಾಶಿ_t *ರಾಶಿ_2, ಶೂನ್ಯ *ptr,
                              size_t alignment, size_t ಗಾತ್ರ_3,
                              ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags) {
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
  ಆದರೆ ((ಗಾತ್ರ_3 + alignment < ಗಾತ್ರ_3) || (alignment > ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ)) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ _rpmalloc_aligned_reallocate(ರಾಶಿ_2, ptr, alignment, ಗಾತ್ರ_3, 0, flags);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_free(rpmalloc_ರಾಶಿ_t *ರಾಶಿ_2, ಶೂನ್ಯ *ptr) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ರಾಶಿ_2);
  _rpmalloc_deallocate(ptr);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_free_all(rpmalloc_ರಾಶಿ_t *ರಾಶಿ_2) {
  ಹರವು_t *ಹರವು;
  ಹರವು_t *ಮುಂದಿನ_ಹರವು;

  rpmalloc_ರಾಶಿ_cache_adopt_deferred(ರಾಶಿ_2, 0);

  ಗಾಗಿ (size_t iclass = 0; iclass < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಹರವು = ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].partial_ಹರವು;
    ಆಗ (ಹರವು) {
      ಮುಂದಿನ_ಹರವು = ಹರವು->ಮುಂದಿನ;
      rpmalloc_ರಾಶಿ_cache_ಸೇರಿಸು(ರಾಶಿ_2, ಹರವು);
      ಹರವು = ಮುಂದಿನ_ಹರವು;
    }
    ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].partial_ಹರವು = 0;
    ಹರವು = ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು[iclass];
    ಆಗ (ಹರವು) {
      ಮುಂದಿನ_ಹರವು = ಹರವು->ಮುಂದಿನ;
      rpmalloc_ರಾಶಿ_cache_ಸೇರಿಸು(ರಾಶಿ_2, ಹರವು);
      ಹರವು = ಮುಂದಿನ_ಹರವು;
    }

    ಹರವು = ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].cache;
    ಆದರೆ (ಹರವು)
      rpmalloc_ರಾಶಿ_cache_ಸೇರಿಸು(ರಾಶಿ_2, ಹರವು);
    ರಾಶಿ_2->ಗಾತ್ರ_class[iclass].cache = 0;
  }
  memset(ರಾಶಿ_2->ಗಾತ್ರ_class, 0, ಗಾತ್ರ(ರಾಶಿ_2->ಗಾತ್ರ_class));
  memset(ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು, 0, ಗಾತ್ರ(ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು));

  ಹರವು = ರಾಶಿ_2->large_huge_ಹರವು;
  ಆಗ (ಹರವು) {
    ಮುಂದಿನ_ಹರವು = ಹರವು->ಮುಂದಿನ;
    ಆದರೆ (UNEXPECTED(ಹರವು->ಗಾತ್ರ_class == ಗಾತ್ರ_CLASS_HUGE))
      _rpmalloc_deallocate_huge(ಹರವು);
    ಇಲ್ಲದಿದ್ದರೆ
      rpmalloc_ರಾಶಿ_cache_ಸೇರಿಸು(ರಾಶಿ_2, ಹರವು);
    ಹರವು = ಮುಂದಿನ_ಹರವು;
  }
  ರಾಶಿ_2->large_huge_ಹರವು = 0;
  ರಾಶಿ_2->ತುಂಬಿದ_ಹರವು_ಎಣಿಕೆ = 0;

#ಆದರೆ ENABLE_THREAD_CACHE
  ಗಾಗಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಹರವು_cache_t *ಹರವು_cache;
    ಆದರೆ (!iclass)
      ಹರವು_cache = &ರಾಶಿ_2->ಹರವು_cache;
    ಇಲ್ಲದಿದ್ದರೆ
      ಹರವು_cache = (ಹರವು_cache_t *)(ರಾಶಿ_2->ಹರವು_large_cache + (iclass - 1));
    ಆದರೆ (!ಹರವು_cache->ಎಣಿಕೆ_2)
      ಮುಂದುವರಿ;
#ಆದರೆ ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&ರಾಶಿ_2->thread_ಗಮ್ಯ_global,
                         ಹರವು_cache->ಎಣಿಕೆ_2 * (iclass + 1) * ಸ್ಮರಣೆ_ಹರವು_ಗಾತ್ರ);
    rpmalloc_stat_ಕೂಡಿಸು(&ರಾಶಿ_2->ಹರವು_use[iclass].spans_ಗಮ್ಯ_global,
                       ಹರವು_cache->ಎಣಿಕೆ_2);
    rpmalloc_global_cache_ಸೇರಿಸು_spans(ಹರವು_cache->ಹರವು, iclass + 1,
                                        ಹರವು_cache->ಎಣಿಕೆ_2);
#ಇಲ್ಲದಿದ್ದರೆ
    ಗಾಗಿ (size_t ispan = 0; ispan < ಹರವು_cache->ಎಣಿಕೆ_2; ++ispan)
      rpmalloc_ಹರವು_unmap(ಹರವು_cache->ಹರವು[ispan]);
#ಆದರೆ_ಅಂತ್ಯ
    ಹರವು_cache->ಎಣಿಕೆ_2 = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_STATISTICS
  ಗಾಗಿ (size_t iclass = 0; iclass < ಗಾತ್ರ_CLASS_ಎಣಿಕೆ; ++iclass) {
    atomic_store32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].alloc_current, 0);
    atomic_store32(&ರಾಶಿ_2->ಗಾತ್ರ_class_use[iclass].spans_current, 0);
  }
  ಗಾಗಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    atomic_store32(&ರಾಶಿ_2->ಹರವು_use[iclass].current, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ rpmalloc_ರಾಶಿ_thread_ಗಣ_current(rpmalloc_ರಾಶಿ_t *ರಾಶಿ_2) {
  ರಾಶಿ_t *prev_ರಾಶಿ = get_thread_ರಾಶಿ_raw();
  ಆದರೆ (prev_ರಾಶಿ != ರಾಶಿ_2) {
    ಗಣ_thread_ರಾಶಿ(ರಾಶಿ_2);
    ಆದರೆ (prev_ರಾಶಿ)
      rpmalloc_ರಾಶಿ_release_2(prev_ರಾಶಿ);
  }
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ rpmalloc_ರಾಶಿ_t *rpmalloc_get_ರಾಶಿ_for_ptr(ಶೂನ್ಯ *ptr) {
  // Grab the span, and then the heap from the span
  ಹರವು_t *ಹರವು = (ಹರವು_t *)((uintptr_t)ptr & ಸ್ಮರಣೆ_ಹರವು_ಮುಖವಾಡ);
  ಆದರೆ (ಹರವು) {
    ಹಿಂತಿರುಗಿ ಹರವು->ರಾಶಿ_2;
  }
  ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_PRELOAD || ENABLE_OVERRIDE

#ಅನ್ತರ್ಭಾವ "malloc.c"

#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ rpmalloc_linker_reference(ಶೂನ್ಯ) { (ಶೂನ್ಯ)ಗಾತ್ರ(_rpmalloc_initialized); }
