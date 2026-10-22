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

#ಯದಿ defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wunused-function"
#ಯದಿ __has_warning("-Wreserved-identifier")
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
#ಯದಿ __has_warning("-Wstatic-in-inline")
#pragma clang diagnostic ignored "-Wstatic-in-inline"
#endif
#ನೋಚೇದ್ಯದಿ defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ಯದಿ !defined(__has_builtin)
#define __has_builtin(b) 0
#endif

#ಯದಿ defined(__GNUC__) || defined(__clang__)

#ಯದಿ __has_builtin(__builtin_memcpy_inline)
#define _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#ಅನ್ಯಥಾ
#define _rpmalloc_memcpy_const(x, y, s)                                        \
  ಕುರು {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memcpy(x, y, s);                                                           \
  } ಯಾವತ್ (0)
#endif

#ಯದಿ __has_builtin(__builtin_memset_inline)
#define _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#ಅನ್ಯಥಾ
#define _rpmalloc_memset_const(x, y, s)                                        \
  ಕುರು {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } ಯಾವತ್ (0)
#endif
#ಅನ್ಯಥಾ
#define _rpmalloc_memcpy_const(x, y, s) memcpy(x, y, s)
#define _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#endif

#ಯದಿ __has_builtin(__builtin_assume)
#define rpmalloc_assume(cond) __builtin_assume(cond)
#ನೋಚೇದ್ಯದಿ defined(__GNUC__)
#define rpmalloc_assume(cond)                                                  \
  ಕುರು {                                                                         \
    ಯದಿ (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } ಯಾವತ್ (0)
#ನೋಚೇದ್ಯದಿ defined(_MSC_VER)
#define rpmalloc_assume(cond) __assume(cond)
#ಅನ್ಯಥಾ
#define rpmalloc_assume(cond) 0
#endif

#ifndef HEAP_ARRAY_SIZE
//! Size of heap hashmap
#define HEAP_ARRAY_SIZE 47
#endif
#ifndef ENABLE_THREAD_CACHE
//! Enable per-thread cache
#define ENABLE_THREAD_CACHE 1
#endif
#ifndef ENABLE_GLOBAL_CACHE
//! Enable global cache shared between all threads, requires thread cache
#define ENABLE_GLOBAL_CACHE 1
#endif
#ifndef ENABLE_VALIDATE_ARGS
//! Enable validation of args to public entry points
#define ENABLE_VALIDATE_ARGS 0
#endif
#ifndef ENABLE_STATISTICS
//! Enable statistics collection
#define ENABLE_STATISTICS 0
#endif
#ifndef ENABLE_ASSERTS
//! Enable asserts
#define ENABLE_ASSERTS 0
#endif
#ifndef ENABLE_OVERRIDE
//! Override standard library malloc/free and new/delete entry points
#define ENABLE_OVERRIDE 0
#endif
#ifndef ENABLE_PRELOAD
//! Support preloading
#define ENABLE_PRELOAD 0
#endif
#ifndef DISABLE_UNMAP
//! Disable unmapping memory pages (also enables unlimited cache)
#define DISABLE_UNMAP 0
#endif
#ifndef ENABLE_UNLIMITED_CACHE
//! Enable unlimited global cache (no unmapping until finalization)
#define ENABLE_UNLIMITED_CACHE 0
#endif
#ifndef ENABLE_ADAPTIVE_THREAD_CACHE
//! Enable adaptive thread cache size based on use heuristics
#define ENABLE_ADAPTIVE_THREAD_CACHE 0
#endif
#ifndef DEFAULT_SPAN_MAP_ಎಣಿಕೆ
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#define DEFAULT_SPAN_MAP_ಎಣಿಕೆ 64
#endif
#ifndef GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#define GLOBAL_CACHE_MULTIPLIER 8
#endif

#ಯದಿ DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#error Must use global cache ಯದಿ unmap is disabled
#endif

#ಯದಿ DISABLE_UNMAP
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 1
#endif

#ಯದಿ !ENABLE_GLOBAL_CACHE
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 0
#endif

#ಯದಿ !ENABLE_THREAD_CACHE
#undef ENABLE_ADAPTIVE_THREAD_CACHE
#define ENABLE_ADAPTIVE_THREAD_CACHE 0
#endif

#ಯದಿ defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#define PLATFORM_POSIX 0
#ಅನ್ಯಥಾ
#define PLATFORM_WINDOWS 0
#define PLATFORM_POSIX 1
#endif

/// Platform and arch specifics
#ಯದಿ defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 5105)
#ifndef FORCEINLINE
#define FORCEINLINE ಅನ್ತಃಸ್ಥ __forceinline
#endif
#define _Static_assert static_assert
#ಅನ್ಯಥಾ
#ifndef FORCEINLINE
#define FORCEINLINE ಅನ್ತಃಸ್ಥ __attribute__((__always_inline__))
#endif
#endif
#ಯದಿ PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ಅನ್ತರ್ಭಾವ <windows.h>
#ಯದಿ ENABLE_VALIDATE_ARGS
#ಅನ್ತರ್ಭಾವ <intsafe.h>
#endif
#ಅನ್ಯಥಾ
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಯದಿ defined(__linux__) || defined(__ANDROID__)
#ಅನ್ತರ್ಭಾವ <sys/prctl.h>
#ಯದಿ !defined(PR_SET_VMA)
#define PR_SET_VMA 0x53564d41
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif
#ಯದಿ defined(__APPLE__)
#ಅನ್ತರ್ಭಾವ <TargetConditionals.h>
#ಯದಿ !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#ಅನ್ತರ್ಭಾವ <mach/mach_vm.h>
#ಅನ್ತರ್ಭಾವ <mach/vm_statistics.h>
#endif
#ಅನ್ತರ್ಭಾವ <pthread.h>
#endif
#ಯದಿ defined(__HAIKU__) || defined(__TINYC__)
#ಅನ್ತರ್ಭಾವ <pthread.h>
#endif
#endif

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಯದಿ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#ಅನ್ತರ್ಭಾವ <fibersapi.h>
ಸ್ಥಿರ DWORD fls_ಕೀಲಿ;
#endif

#ಯದಿ PLATFORM_POSIX
#ಅನ್ತರ್ಭಾವ <sched.h>
#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#ifdef __FreeBSD__
#ಅನ್ತರ್ಭಾವ <sys/sysctl.h>
#define MAP_HUGETLB MAP_ALIGNED_SUPER
#ifndef PROT_MAX
#define PROT_MAX(f) 0
#endif
#ಅನ್ಯಥಾ
#define PROT_MAX(f) 0
#endif
#ifdef __sun
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ madvise(caddr_t, size_t, ಪೂರ್ಣಾಙ್ಕ);
#endif
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif
#endif
#ಅನ್ತರ್ಭಾವ <errno.h>

#ಯದಿ ENABLE_ASSERTS
#undef NDEBUG
#ಯದಿ defined(_MSC_VER) && !defined(_DEBUG)
#define _DEBUG
#endif
#ಅನ್ತರ್ಭಾವ <assert.h>
#define RPMALLOC_TOSTRING_M(x) #x
#define RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#define rpmalloc_assert(truth, message)                                        \
  ಕುರು {                                                                         \
    ಯದಿ (!(truth)) {                                                            \
      ಯದಿ (_memory_config.error_callback) {                                     \
        _memory_config.error_callback(message " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } ಅನ್ಯಥಾ {                                                                 \
        ಪ್ರತಿಜ್ಞಾ((truth) && message);                                            \
      }                                                                        \
    }                                                                          \
  } ಯಾವತ್ (0)
#ಅನ್ಯಥಾ
#define rpmalloc_assert(truth, message)                                        \
  ಕುರು {                                                                         \
  } ಯಾವತ್ (0)
#endif
#ಯದಿ ENABLE_STATISTICS
#ಅನ್ತರ್ಭಾವ <stdio.h>
#endif

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#ಯದಿ defined(_MSC_VER) && !defined(__clang__)

ಪ್ರಕಾರನಿರ್ದೇಶ ಚಞ್ಚಲ long atomic32_t;
ಪ್ರಕಾರನಿರ್ದೇಶ ಚಞ್ಚಲ long long atomic64_t;
ಪ್ರಕಾರನಿರ್ದೇಶ ಚಞ್ಚಲ ಶೂನ್ಯ *atomicptr_t;

ಸ್ಥಿರ FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  ಪ್ರತ್ಯಾಗಮ (int32_t)InterlockedOr(src, 0);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ಸ್ಥಿರ FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  ಪ್ರತ್ಯಾಗಮ (int32_t)InterlockedIncrement(val);
}
ಸ್ಥಿರ FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  ಪ್ರತ್ಯಾಗಮ (int32_t)InterlockedDecrement(val);
}
ಸ್ಥಿರ FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  ಪ್ರತ್ಯಾಗಮ (int32_t)InterlockedExchangeAdd(val, add) + add;
}
ಸ್ಥಿರ FORCEINLINE ಪೂರ್ಣಾಙ್ಕ atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  ಪ್ರತ್ಯಾಗಮ (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ಸ್ಥಿರ FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  ಪ್ರತ್ಯಾಗಮ (int64_t)InterlockedOr64(src, 0);
}
ಸ್ಥಿರ FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  ಪ್ರತ್ಯಾಗಮ (int64_t)InterlockedExchangeAdd64(val, add) + add;
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ *atomic_load_ptr(atomicptr_t *src) {
  ಪ್ರತ್ಯಾಗಮ InterlockedCompareExchangePointer(src, 0, 0);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store_ptr(atomicptr_t *dst, ಶೂನ್ಯ *val) {
  InterlockedExchangePointer(dst, val);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store_ptr_release(atomicptr_t *dst, ಶೂನ್ಯ *val) {
  InterlockedExchangePointer(dst, val);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     ಶೂನ್ಯ *val) {
  ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ *)InterlockedExchangePointer((ಶೂನ್ಯ *ಚಞ್ಚಲ *)dst, val);
}
ಸ್ಥಿರ FORCEINLINE ಪೂರ್ಣಾಙ್ಕ atomic_cas_ptr(atomicptr_t *dst, ಶೂನ್ಯ *val, ಶೂನ್ಯ *ref) {
  ಪ್ರತ್ಯಾಗಮ (InterlockedCompareExchangePointer((ಶೂನ್ಯ *ಚಞ್ಚಲ *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#define EXPECTED(x) (x)
#define UNEXPECTED(x) (x)

#ಅನ್ಯಥಾ

#ಅನ್ತರ್ಭಾವ <stdatomic.h>

ಪ್ರಕಾರನಿರ್ದೇಶ ಚಞ್ಚಲ _Atomic(int32_t) atomic32_t;
ಪ್ರಕಾರನಿರ್ದೇಶ ಚಞ್ಚಲ _Atomic(int64_t) atomic64_t;
ಪ್ರಕಾರನಿರ್ದೇಶ ಚಞ್ಚಲ _Atomic(ಶೂನ್ಯ *) atomicptr_t;

ಸ್ಥಿರ FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  ಪ್ರತ್ಯಾಗಮ atomic_load_explicit(src, memory_order_relaxed);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ಸ್ಥಿರ FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  ಪ್ರತ್ಯಾಗಮ atomic_fetch_add_explicit(val, 1, memory_order_relaxed) + 1;
}
ಸ್ಥಿರ FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  ಪ್ರತ್ಯಾಗಮ atomic_fetch_add_explicit(val, -1, memory_order_relaxed) - 1;
}
ಸ್ಥಿರ FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  ಪ್ರತ್ಯಾಗಮ atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
ಸ್ಥಿರ FORCEINLINE ಪೂರ್ಣಾಙ್ಕ atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  ಪ್ರತ್ಯಾಗಮ atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ಸ್ಥಿರ FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  ಪ್ರತ್ಯಾಗಮ atomic_load_explicit(val, memory_order_relaxed);
}
ಸ್ಥಿರ FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  ಪ್ರತ್ಯಾಗಮ atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ *atomic_load_ptr(atomicptr_t *src) {
  ಪ್ರತ್ಯಾಗಮ atomic_load_explicit(src, memory_order_relaxed);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store_ptr(atomicptr_t *dst, ಶೂನ್ಯ *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ atomic_store_ptr_release(atomicptr_t *dst, ಶೂನ್ಯ *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ಸ್ಥಿರ FORCEINLINE ಶೂನ್ಯ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     ಶೂನ್ಯ *val) {
  ಪ್ರತ್ಯಾಗಮ atomic_exchange_explicit(dst, val, memory_order_acquire);
}
ಸ್ಥಿರ FORCEINLINE ಪೂರ್ಣಾಙ್ಕ atomic_cas_ptr(atomicptr_t *dst, ಶೂನ್ಯ *val, ಶೂನ್ಯ *ref) {
  ಪ್ರತ್ಯಾಗಮ atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_relaxed, memory_order_relaxed);
}

#define EXPECTED(x) __builtin_expect((x), 1)
#define UNEXPECTED(x) __builtin_expect((x), 0)

#endif

////////////
///
/// Statistics related functions (evaluate to nothing when statistics not
/// enabled)
///
//////

#ಯದಿ ENABLE_STATISTICS
#define _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#define _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#define _rpmalloc_stat_add(counter, ಮೌಲ್ಯ_2)                                     \
  atomic_add32(counter, (int32_t)(ಮೌಲ್ಯ_2))
#define _rpmalloc_stat_add64(counter, ಮೌಲ್ಯ_2)                                   \
  atomic_add64(counter, (int64_t)(ಮೌಲ್ಯ_2))
#define _rpmalloc_stat_add_peak(counter, ಮೌಲ್ಯ_2, peak)                          \
  ಕುರು {                                                                         \
    int32_t cur_ಎಣಿಕೆ = atomic_add32(counter, (int32_t)(ಮೌಲ್ಯ_2));              \
    ಯದಿ (cur_ಎಣಿಕೆ > (peak))                                                   \
      peak = cur_ಎಣಿಕೆ;                                                       \
  } ಯಾವತ್ (0)
#define _rpmalloc_stat_sub(counter, ಮೌಲ್ಯ_2)                                     \
  atomic_add32(counter, -(int32_t)(ಮೌಲ್ಯ_2))
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  ಕುರು {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&heap->size_class_use[class_idx].alloc_current);         \
    ಯದಿ (alloc_current > heap->size_class_use[class_idx].alloc_peak)            \
      heap->size_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&heap->size_class_use[class_idx].alloc_total);               \
  } ಯಾವತ್ (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  ಕುರು {                                                                         \
    atomic_decr32(&heap->size_class_use[class_idx].alloc_current);             \
    atomic_incr32(&heap->size_class_use[class_idx].free_total);                \
  } ಯಾವತ್ (0)
#ಅನ್ಯಥಾ
#define _rpmalloc_stat_inc(counter)                                            \
  ಕುರು {                                                                         \
  } ಯಾವತ್ (0)
#define _rpmalloc_stat_dec(counter)                                            \
  ಕುರು {                                                                         \
  } ಯಾವತ್ (0)
#define _rpmalloc_stat_add(counter, ಮೌಲ್ಯ_2)                                     \
  ಕುರು {                                                                         \
  } ಯಾವತ್ (0)
#define _rpmalloc_stat_add64(counter, ಮೌಲ್ಯ_2)                                   \
  ಕುರು {                                                                         \
  } ಯಾವತ್ (0)
#define _rpmalloc_stat_add_peak(counter, ಮೌಲ್ಯ_2, peak)                          \
  ಕುರು {                                                                         \
  } ಯಾವತ್ (0)
#define _rpmalloc_stat_sub(counter, ಮೌಲ್ಯ_2)                                     \
  ಕುರು {                                                                         \
  } ಯಾವತ್ (0)
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  ಕುರು {                                                                         \
  } ಯಾವತ್ (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  ಕುರು {                                                                         \
  } ಯಾವತ್ (0)
#endif

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#define SMALL_GRANULARITY 16
//! Small granularity shift count
#define SMALL_GRANULARITY_SHIFT 4
//! Number of small block size classes
#define SMALL_CLASS_ಎಣಿಕೆ 65
//! Maximum size of a small block
#define SMALL_SIZE_LIMIT (SMALL_GRANULARITY * (SMALL_CLASS_ಎಣಿಕೆ - 1))
//! Granularity of a medium allocation block
#define MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#define MEDIUM_GRANULARITY_SHIFT 9
//! Number of medium block size classes
#define MEDIUM_CLASS_ಎಣಿಕೆ 61
//! Total number of small + medium size classes
#define SIZE_CLASS_ಎಣಿಕೆ (SMALL_CLASS_ಎಣಿಕೆ + MEDIUM_CLASS_ಎಣಿಕೆ)
//! Number of large block size classes
#define LARGE_CLASS_ಎಣಿಕೆ 63
//! Maximum size of a medium block
#define MEDIUM_SIZE_LIMIT                                                      \
  (SMALL_SIZE_LIMIT + (MEDIUM_GRANULARITY * MEDIUM_CLASS_ಎಣಿಕೆ))
//! Maximum size of a large block
#define LARGE_SIZE_LIMIT                                                       \
  ((LARGE_CLASS_ಎಣಿಕೆ * _memory_span_size) - SPAN_HEADER_SIZE)
//! Size of a span header (must be a multiple of SMALL_GRANULARITY and a power
//! of two)
#define SPAN_HEADER_SIZE 128
//! Number of spans in thread cache
#define MAX_THREAD_SPAN_CACHE 400
//! Number of spans to transfer between thread and global cache
#define THREAD_SPAN_CACHE_TRANSFER 64
//! Number of spans in thread cache for large spans (must be greater than
//! LARGE_CLASS_COUNT / 2)
#define MAX_THREAD_SPAN_LARGE_CACHE 100
//! Number of spans to transfer between thread and global cache for large spans
#define THREAD_SPAN_LARGE_CACHE_TRANSFER 6

_Static_assert((SMALL_GRANULARITY & (SMALL_GRANULARITY - 1)) == 0,
               "Small granularity must be power of two");
_Static_assert((SPAN_HEADER_SIZE & (SPAN_HEADER_SIZE - 1)) == 0,
               "Span header size must be power of two");

#ಯದಿ ENABLE_VALIDATE_ARGS
//! Maximum allocation size to avoid integer overflow
#undef MAX_ALLOC_SIZE
#define MAX_ALLOC_SIZE (((size_t) - 1) - _memory_span_size)
#endif

#define pointer_offset(ptr, ofs) (ಶೂನ್ಯ *)((ವರ್ಣ *)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff(first, second)                                            \
  (ptrdiff_t)((ಧ್ರುವ ವರ್ಣ *)(first) - (ಧ್ರುವ ವರ್ಣ *)(second))

#define INVALID_POINTER ((ಶೂನ್ಯ *)((uintptr_t) - 1))

#define SIZE_CLASS_LARGE SIZE_CLASS_ಎಣಿಕೆ
#define SIZE_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ heap_t heap_t;
//! Span of memory pages
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ span_t span_t;
//! Span list
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ span_list_t span_list_t;
//! Span active data
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ span_active_t span_active_t;
//! Size class definition
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ size_class_t size_class_t;
//! Global cache
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#define SPAN_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#define SPAN_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#define SPAN_FLAG_ALIGNED_BLOCKS 4U
//! Flag indicating an unmapped master span
#define SPAN_FLAG_UNMAPPED_MASTER 8U

#ಯದಿ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
ಸಂರಚನಾ span_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t ಮೇಲಿನ;
#ಯದಿ ENABLE_STATISTICS
  //! Number of spans in deferred list
  atomic32_t spans_deferred;
  //! Number of spans transitioned to global cache
  atomic32_t spans_to_global;
  //! Number of spans transitioned from global cache
  atomic32_t spans_from_global;
  //! Number of spans transitioned to thread cache
  atomic32_t spans_to_cache;
  //! Number of spans transitioned from thread cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned to reserved state
  atomic32_t spans_to_reserved;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of raw memory map calls
  atomic32_t spans_map_calls;
#endif
};
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ span_use_t span_use_t;
#endif

#ಯದಿ ENABLE_STATISTICS
ಸಂರಚನಾ size_class_use_t {
  //! Current number of allocations
  atomic32_t alloc_current;
  //! Peak number of allocations
  int32_t alloc_peak;
  //! Total number of allocations
  atomic32_t alloc_total;
  //! Total number of frees
  atomic32_t free_total;
  //! Number of spans in use
  atomic32_t spans_current;
  //! Number of spans transitioned to cache
  int32_t spans_peak;
  //! Number of spans transitioned to cache
  atomic32_t spans_to_cache;
  //! Number of spans transitioned from cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of spans mapped
  atomic32_t spans_map_calls;
  int32_t unused;
};
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ size_class_use_t size_class_use_t;
#endif

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
ಸಂರಚನಾ span_t {
  //! Free list
  ಶೂನ್ಯ *free_list;
  //! Total block count of size class
  uint32_t block_ಎಣಿಕೆ;
  //! Size class
  uint32_t size_class;
  //! Index of last block initialized in free list
  uint32_t free_list_limit;
  //! Number of used blocks remaining when in partial state
  uint32_t used_ಎಣಿಕೆ;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_size;
  //! Size of a block
  uint32_t block_size;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t span_ಎಣಿಕೆ;
  //! Total span counter for master spans
  uint32_t total_spans;
  //! Offset from master span for subspans
  uint32_t offset_from_master;
  //! Remaining span counter, for master spans
  atomic32_t remaining_spans;
  //! Alignment offset
  uint32_t align_offset;
  //! Owning heap
  heap_t *heap;
  //! Next span
  span_t *next;
  //! Previous span
  span_t *prev;
};
_Static_assert(ಪರಿಮಾಣ(span_t) <= SPAN_HEADER_SIZE, "span size mismatch");

ಸಂರಚನಾ span_cache_t {
  size_t ಎಣಿಕೆ;
  span_t *span[MAX_THREAD_SPAN_CACHE];
};
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ span_cache_t span_cache_t;

ಸಂರಚನಾ span_large_cache_t {
  size_t ಎಣಿಕೆ;
  span_t *span[MAX_THREAD_SPAN_LARGE_CACHE];
};
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ span_large_cache_t span_large_cache_t;

ಸಂರಚನಾ heap_size_class_t {
  //! Free list of active span
  ಶೂನ್ಯ *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  span_t *partial_span;
  //! Early level cache of fully free spans
  span_t *cache;
};
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ heap_size_class_t heap_size_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
ಸಂರಚನಾ heap_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  heap_size_class_t size_class[SIZE_CLASS_ಎಣಿಕೆ];
#ಯದಿ ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  span_cache_t span_cache;
#endif
  //! List of deferred free spans (single linked list)
  atomicptr_t span_free_deferred;
  //! Number of full spans
  size_t full_span_ಎಣಿಕೆ;
  //! Mapped but unused spans
  span_t *span_reserve;
  //! Master span for mapped but unused spans
  span_t *span_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t child_ಎಣಿಕೆ;
  //! Next heap in id list
  heap_t *next_heap;
  //! Next heap in orphan list
  heap_t *next_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  ಪೂರ್ಣಾಙ್ಕ finalize;
  //! Master heap owning the memory pages
  heap_t *master_heap;
#ಯದಿ ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  span_large_cache_t span_large_cache[LARGE_CLASS_ಎಣಿಕೆ - 1];
#endif
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  span_t *full_span[SIZE_CLASS_ಎಣಿಕೆ];
  //! Double linked list of large and huge spans allocated by this heap
  span_t *large_huge_span;
#endif
#ಯದಿ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  span_use_t span_use[LARGE_CLASS_ಎಣಿಕೆ];
#endif
#ಯದಿ ENABLE_STATISTICS
  //! Allocation stats per size class
  size_class_use_t size_class_use[SIZE_CLASS_ಎಣಿಕೆ + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_to_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_to_thread;
#endif
};

// Size class for defining a block size bucket
ಸಂರಚನಾ size_class_t {
  //! Size of blocks in this class
  uint32_t block_size;
  //! Number of blocks in each chunk
  uint16_t block_ಎಣಿಕೆ;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(ಪರಿಮಾಣ(size_class_t) == 8, "Size class size mismatch");

ಸಂರಚನಾ global_cache_t {
  //! Cache lock
  atomic32_t lock;
  //! Cache count
  uint32_t ಎಣಿಕೆ;
#ಯದಿ ENABLE_STATISTICS
  //! Insert count
  size_t ಸೇರಿಸು_ಎಣಿಕೆ;
  //! Extract count
  size_t extract_ಎಣಿಕೆ;
#endif
  //! Cached spans
  span_t *span[GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_SPAN_CACHE];
  //! Unlimited cache overflow
  span_t *overflow;
};

////////////
///
/// Global data
///
//////

//! Default span size (64KiB)
#define _memory_default_span_size (64 * 1024)
#define _memory_default_span_size_shift 16
#define _memory_default_span_mask (~((uintptr_t)(_memory_span_size - 1)))

//! Initialized flag
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ _rpmalloc_initialized;
//! Main thread ID
ಸ್ಥಿರ uintptr_t _rpmalloc_main_thread_id;
//! Configuration
ಸ್ಥಿರ rpmalloc_config_t _memory_config;
//! Memory page size
ಸ್ಥಿರ size_t _memory_page_size;
//! Shift to divide by page size
ಸ್ಥಿರ size_t _memory_page_size_shift;
//! Granularity at which memory pages are mapped by OS
ಸ್ಥಿರ size_t _memory_map_granularity;
#ಯದಿ RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
ಸ್ಥಿರ size_t _memory_span_size;
//! Shift to divide by span size
ಸ್ಥಿರ size_t _memory_span_size_shift;
//! Mask to get to start of a memory span
ಸ್ಥಿರ uintptr_t _memory_span_mask;
#ಅನ್ಯಥಾ
//! Hardwired span size
#define _memory_span_size _memory_default_span_size
#define _memory_span_size_shift _memory_default_span_size_shift
#define _memory_span_mask _memory_default_span_mask
#endif
//! Number of spans to map in each map call
ಸ್ಥಿರ size_t memory_span_map_ಎಣಿಕೆ;
//! Number of spans to keep reserved in each heap
ಸ್ಥಿರ size_t memory_heap_reserve_ಎಣಿಕೆ;
//! Global size classes
ಸ್ಥಿರ size_class_t _memory_size_class[SIZE_CLASS_ಎಣಿಕೆ];
//! Run-time size limit of medium blocks
ಸ್ಥಿರ size_t _memory_medium_size_limit;
//! Heap ID counter
ಸ್ಥಿರ atomic32_t _memory_heap_id;
//! Huge page support
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ _memory_huge_pages;
#ಯದಿ ENABLE_GLOBAL_CACHE
//! Global span cache
ಸ್ಥಿರ global_cache_t _memory_span_cache[LARGE_CLASS_ಎಣಿಕೆ];
#endif
//! Global reserved spans
ಸ್ಥಿರ span_t *_memory_global_reserve;
//! Global reserved count
ಸ್ಥಿರ size_t memory_global_reserve_ಎಣಿಕೆ;
//! Global reserved master
ಸ್ಥಿರ span_t *_memory_global_reserve_master;
//! All heaps
ಸ್ಥಿರ heap_t *_memory_heaps[HEAP_ARRAY_SIZE];
//! Used to restrict access to mapping memory for huge pages
ಸ್ಥಿರ atomic32_t _memory_global_lock;
//! Orphaned heaps
ಸ್ಥಿರ heap_t *_memory_orphan_heaps;
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
ಸ್ಥಿರ heap_t *_memory_first_class_orphan_heaps;
#endif
#ಯದಿ ENABLE_STATISTICS
//! Allocations counter
ಸ್ಥಿರ atomic64_t _allocation_counter;
//! Deallocations counter
ಸ್ಥಿರ atomic64_t _deallocation_counter;
//! Active heap count
ಸ್ಥಿರ atomic32_t _memory_active_heaps;
//! Number of currently mapped memory pages
ಸ್ಥಿರ atomic32_t _mapped_pages;
//! Peak number of concurrently mapped memory pages
ಸ್ಥಿರ int32_t _mapped_pages_peak;
//! Number of mapped master spans
ಸ್ಥಿರ atomic32_t _master_spans;
//! Number of unmapped dangling master spans
ಸ್ಥಿರ atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
ಸ್ಥಿರ atomic32_t _mapped_total;
//! Running counter of total number of unmapped memory pages since start
ಸ್ಥಿರ atomic32_t _unmapped_total;
//! Number of currently mapped memory pages in OS calls
ಸ್ಥಿರ atomic32_t _mapped_pages_os;
//! Number of currently allocated pages in huge allocations
ಸ್ಥಿರ atomic32_t _huge_pages_current;
//! Peak number of currently allocated pages in huge allocations
ಸ್ಥಿರ int32_t _huge_pages_peak;
#endif

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#ಯದಿ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
ಸ್ಥಿರ pthread_key_t _memory_thread_heap;
#ಅನ್ಯಥಾ
#ifdef _MSC_VER
#define _Thread_local __declspec(thread)
#define TLS_MODEL
#ಅನ್ಯಥಾ
#ifndef __HAIKU__
#define TLS_MODEL __attribute__((tls_model("initial-exec")))
#ಅನ್ಯಥಾ
#define TLS_MODEL
#endif
#ಯದಿ !defined(__clang__) && defined(__GNUC__)
#define _Thread_local __thread
#endif
#endif
ಸ್ಥಿರ _Thread_local heap_t *_memory_thread_heap TLS_MODEL;
#endif

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ heap_t *get_thread_heap_raw(ಶೂನ್ಯ) {
#ಯದಿ (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  ಪ್ರತ್ಯಾಗಮ pthread_getspecific(_memory_thread_heap);
#ಅನ್ಯಥಾ
  ಪ್ರತ್ಯಾಗಮ _memory_thread_heap;
#endif
}

//! Get the current thread heap
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ heap_t *get_thread_heap(ಶೂನ್ಯ) {
  heap_t *heap = get_thread_heap_raw();
#ಯದಿ ENABLE_PRELOAD
  ಯದಿ (EXPECTED(heap != 0))
    ಪ್ರತ್ಯಾಗಮ heap;
  rpmalloc_initialize();
  ಪ್ರತ್ಯಾಗಮ get_thread_heap_raw();
#ಅನ್ಯಥಾ
  ಪ್ರತ್ಯಾಗಮ heap;
#endif
}

//! Fast thread ID
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ uintptr_t get_thread_id(ಶೂನ್ಯ) {
#ಯದಿ defined(_WIN32)
  ಪ್ರತ್ಯಾಗಮ (uintptr_t)((ಶೂನ್ಯ *)NtCurrentTeb());
#ನೋಚೇದ್ಯದಿ (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#ಯದಿ defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#ನೋಚೇದ್ಯದಿ defined(__x86_64__)
#ಯದಿ defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#ಅನ್ಯಥಾ
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#endif
#ನೋಚೇದ್ಯದಿ defined(__arm__)
  __asm__ ಚಞ್ಚಲ("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#ನೋಚೇದ್ಯದಿ defined(__aarch64__)
#ಯದಿ defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ ಚಞ್ಚಲ("mrs %0, tpidrro_el0" : "=r"(tid));
#ಅನ್ಯಥಾ
  __asm__ ಚಞ್ಚಲ("mrs %0, tpidr_el0" : "=r"(tid));
#endif
#ಅನ್ಯಥಾ
#error This platform needs implementation of get_thread_id()
#endif
  ಪ್ರತ್ಯಾಗಮ tid;
#ಅನ್ಯಥಾ
#error This platform needs implementation of get_thread_id()
#endif
}

//! Set the current thread heap
ಸ್ಥಿರ ಶೂನ್ಯ set_thread_heap(heap_t *heap) {
#ಯದಿ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(_memory_thread_heap, heap);
#ಅನ್ಯಥಾ
  _memory_thread_heap = heap;
#endif
  ಯದಿ (heap)
    heap->owner_thread = get_thread_id();
}

//! Set main thread ID
ಬಾಹ್ಯ ಶೂನ್ಯ rpmalloc_set_main_thread(ಶೂನ್ಯ);

ಶೂನ್ಯ rpmalloc_set_main_thread(ಶೂನ್ಯ) {
  _rpmalloc_main_thread_id = get_thread_id();
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_spin(ಶೂನ್ಯ) {
#ಯದಿ defined(_MSC_VER)
#ಯದಿ defined(_M_ARM64)
  __yield();
#ಅನ್ಯಥಾ
  _mm_pause();
#endif
#ನೋಚೇದ್ಯದಿ defined(__x86_64__) || defined(__i386__)
  __asm__ ಚಞ್ಚಲ("pause" ::: "memory");
#ನೋಚೇದ್ಯದಿ defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ ಚಞ್ಚಲ("yield" ::: "memory");
#ನೋಚೇದ್ಯದಿ defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ ಚಞ್ಚಲ("or 27,27,27");
#ನೋಚೇದ್ಯದಿ defined(__sparc__)
  __asm__ ಚಞ್ಚಲ("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#ಅನ್ಯಥಾ
  ಸಂರಚನಾ timespec ts = {0};
  nanosleep(&ts, 0);
#endif
}

#ಯದಿ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

ಸ್ಥಿರ ಶೂನ್ಯ NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  ವಿಕಲ್ಪ (reason) {
  ಪಕ್ಷ DLL_PROCESS_ATTACH:
    ಭಙ್ಗ;
  ಪಕ್ಷ DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    ಭಙ್ಗ;
  ಪಕ್ಷ DLL_THREAD_ATTACH:
    ಭಙ್ಗ;
  ಪಕ್ಷ DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    ಭಙ್ಗ;
  }
}

#ifdef _WIN64
#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#pragma const_seg(".CRT$XLY")

ಬಾಹ್ಯ ಧ್ರುವ PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ಧ್ರುವ PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#pragma const_seg()
#ಅನ್ಯಥಾ // _WIN64
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#pragma data_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#pragma data_seg()
#endif // _WIN64

ಸ್ಥಿರ ಶೂನ್ಯ NTAPI _rpmalloc_thread_destructor(ಶೂನ್ಯ *ಮೌಲ್ಯ_2) {
#ಯದಿ ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  ಯದಿ (get_thread_id() == _rpmalloc_main_thread_id)
    ಪ್ರತ್ಯಾಗಮ;
#endif
  ಯದಿ (ಮೌಲ್ಯ_2)
    rpmalloc_thread_finalize(1);
}
#endif

////////////
///
/// Low level memory map/unmap
///
//////

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_set_name(ಶೂನ್ಯ *address, size_t size) {
#ಯದಿ defined(__linux__) || defined(__ANDROID__)
  ಧ್ರುವ ವರ್ಣ *name = _memory_huge_pages ? _memory_config.huge_page_name
                                        : _memory_config.page_name;
  ಯದಿ (address == MAP_FAILED || !name)
    ಪ್ರತ್ಯಾಗಮ;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (ಶೂನ್ಯ)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (uintptr_t)address, size,
              (uintptr_t)name);
#ಅನ್ಯಥಾ
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(address);
#endif
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_mmap(size_t size, size_t *offset) {
  rpmalloc_assert(!(size % _memory_page_size), "Invalid mmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
  ಶೂನ್ಯ *address = _memory_config.memory_map(size, offset);
  ಯದಿ (EXPECTED(address != 0)) {
    _rpmalloc_stat_add_peak(&_mapped_pages, (size >> _memory_page_size_shift),
                            _mapped_pages_peak);
    _rpmalloc_stat_add(&_mapped_total, (size >> _memory_page_size_shift));
  }
  ಪ್ರತ್ಯಾಗಮ address;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_unmap(ಶೂನ್ಯ *address, size_t size, size_t offset,
                            size_t release) {
  rpmalloc_assert(!release || (release >= size), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  ಯದಿ (release) {
    rpmalloc_assert(!(release % _memory_page_size), "Invalid unmap size");
    _rpmalloc_stat_sub(&_mapped_pages, (release >> _memory_page_size_shift));
    _rpmalloc_stat_add(&_unmapped_total, (release >> _memory_page_size_shift));
  }
  _memory_config.memory_unmap(address, size, offset, release);
}

//! Default implementation to map new pages to virtual memory
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_mmap_os(size_t size, size_t *offset) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((size >= _memory_span_size) &&
                    (_memory_span_size > _memory_map_granularity))
                       ? _memory_span_size
                       : 0;
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
#ಯದಿ PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  ಶೂನ್ಯ *ptr = VirtualAlloc(0, size + padding,
                           (_memory_huge_pages ? MEM_LARGE_PAGES : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
  ಯದಿ (!ptr) {
    ಯದಿ (_memory_config.map_fail_callback) {
      ಯದಿ (_memory_config.map_fail_callback(size + padding))
        ಪ್ರತ್ಯಾಗಮ _rpmalloc_mmap_os(size, offset);
    } ಅನ್ಯಥಾ {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#ಅನ್ಯಥಾ
  ಪೂರ್ಣಾಙ್ಕ flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#ಯದಿ defined(__APPLE__) && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
  ಪೂರ್ಣಾಙ್ಕ fd = (ಪೂರ್ಣಾಙ್ಕ)VM_MAKE_TAG(240U);
  ಯದಿ (_memory_huge_pages)
    fd |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
  ಶೂನ್ಯ *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#ನೋಚೇದ್ಯದಿ defined(MAP_HUGETLB)
  ಶೂನ್ಯ *ptr = mmap(0, size + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (_memory_huge_pages ? MAP_HUGETLB : 0) | flags, -1, 0);
#ಯದಿ defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  ಯದಿ ((ptr == MAP_FAILED || !ptr) && _memory_huge_pages) {
    ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    ಯದಿ (ptr && ptr != MAP_FAILED) {
      ಪೂರ್ಣಾಙ್ಕ prm = madvise(ptr, size + padding, MADV_HUGEPAGE);
      (ಶೂನ್ಯ)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#endif
  _rpmalloc_set_name(ptr, size + padding);
#ನೋಚೇದ್ಯದಿ defined(MAP_ALIGNED)
  ಧ್ರುವ size_t align =
      (ಪರಿಮಾಣ(size_t) * 8) - (size_t)(__builtin_clzl(size - 1));
  ಶೂನ್ಯ *ptr =
      mmap(0, size + padding, PROT_READ | PROT_WRITE,
           (_memory_huge_pages ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#ನೋಚೇದ್ಯದಿ defined(MAP_ALIGN)
  caddr_t base = (_memory_huge_pages ? (caddr_t)(4 << 20) : 0);
  ಶೂನ್ಯ *ptr = mmap(base, size + padding, PROT_READ | PROT_WRITE,
                   (_memory_huge_pages ? MAP_ALIGN : 0) | flags, -1, 0);
#ಅನ್ಯಥಾ
  ಶೂನ್ಯ *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#endif
  ಯದಿ ((ptr == MAP_FAILED) || !ptr) {
    ಯದಿ (_memory_config.map_fail_callback) {
      ಯದಿ (_memory_config.map_fail_callback(size + padding))
        ಪ್ರತ್ಯಾಗಮ _rpmalloc_mmap_os(size, offset);
    } ಅನ್ಯಥಾ ಯದಿ (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
  _rpmalloc_stat_add(&_mapped_pages_os,
                     (int32_t)((size + padding) >> _memory_page_size_shift));
  ಯದಿ (padding) {
    size_t final_padding = padding - ((uintptr_t)ptr & ~_memory_span_mask);
    rpmalloc_assert(final_padding <= _memory_span_size,
                    "Internal failure in padding");
    rpmalloc_assert(final_padding <= padding, "Internal failure in padding");
    rpmalloc_assert(!(final_padding % 8), "Internal failure in padding");
    ptr = pointer_offset(ptr, final_padding);
    *offset = final_padding >> 3;
  }
  rpmalloc_assert((size < _memory_span_size) ||
                      !((uintptr_t)ptr & ~_memory_span_mask),
                  "Internal failure in padding");
  ಪ್ರತ್ಯಾಗಮ ptr;
}

//! Default implementation to unmap pages from virtual memory
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_unmap_os(ಶೂನ್ಯ *address, size_t size, size_t offset,
                               size_t release) {
  rpmalloc_assert(release || (offset == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid unmap size");
  ಯದಿ (release && offset) {
    offset <<= 3;
    address = pointer_offset(address, -(int32_t)offset);
    ಯದಿ ((release >= _memory_span_size) &&
        (_memory_span_size > _memory_map_granularity)) {
      // Padding is always one span size
      release += _memory_span_size;
    }
  }
#ಯದಿ !DISABLE_UNMAP
#ಯದಿ PLATFORM_WINDOWS
  ಯದಿ (!VirtualFree(address, release ? 0 : size,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#ಅನ್ಯಥಾ
  ಯದಿ (release) {
    ಯದಿ (munmap(address, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } ಅನ್ಯಥಾ {
#ಯದಿ defined(MADV_FREE_REUSABLE)
    ಪೂರ್ಣಾಙ್ಕ ret;
    ಯಾವತ್ ((ret = madvise(address, size, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    ಯದಿ ((ret == -1) && (errno != 0)) {
#ನೋಚೇದ್ಯದಿ defined(MADV_DONTNEED)
    ಯದಿ (madvise(address, size, MADV_DONTNEED)) {
#ನೋಚೇದ್ಯದಿ defined(MADV_PAGEOUT)
    ಯದಿ (madvise(address, size, MADV_PAGEOUT)) {
#ನೋಚೇದ್ಯದಿ defined(MADV_FREE)
    ಯದಿ (madvise(address, size, MADV_FREE)) {
#ಅನ್ಯಥಾ
    ಯದಿ (posix_madvise(address, size, POSIX_MADV_DONTNEED)) {
#endif
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#endif
#endif
  ಯದಿ (release)
    _rpmalloc_stat_sub(&_mapped_pages_os, release >> _memory_page_size_shift);
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_ಎಣಿಕೆ);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
ಸ್ಥಿರ span_t *_rpmalloc_global_get_reserved_spans(size_t span_ಎಣಿಕೆ) {
  span_t *span = _memory_global_reserve;
  _rpmalloc_span_mark_as_subspan_unless_master(_memory_global_reserve_master,
                                               span, span_ಎಣಿಕೆ);
  memory_global_reserve_ಎಣಿಕೆ -= span_ಎಣಿಕೆ;
  ಯದಿ (memory_global_reserve_ಎಣಿಕೆ)
    _memory_global_reserve =
        (span_t *)pointer_offset(span, span_ಎಣಿಕೆ << _memory_span_size_shift);
  ಅನ್ಯಥಾ
    _memory_global_reserve = 0;
  ಪ್ರತ್ಯಾಗಮ span;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_global_set_reserved_spans(span_t *master, span_t *reserve,
                                                size_t reserve_span_ಎಣಿಕೆ) {
  _memory_global_reserve_master = master;
  memory_global_reserve_ಎಣಿಕೆ = reserve_span_ಎಣಿಕೆ;
  _memory_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_span_double_link_list_add(span_t **head, span_t *span) {
  ಯದಿ (*head)
    (*head)->prev = span;
  span->next = *head;
  *head = span;
}

//! Pop head span from double linked list
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_span_double_link_list_ತೆಗೆ_head(span_t **head,
                                                     span_t *span) {
  rpmalloc_assert(*head == span, "Linked list corrupted");
  span = *head;
  *head = span->next;
}

//! Remove a span from double linked list
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_span_double_link_list_remove(span_t **head,
                                                   span_t *span) {
  rpmalloc_assert(*head, "Linked list corrupted");
  ಯದಿ (*head == span) {
    *head = span->next;
  } ಅನ್ಯಥಾ {
    span_t *next_span = span->next;
    span_t *prev_span = span->prev;
    prev_span->next = next_span;
    ಯದಿ (EXPECTED(next_span != 0))
      next_span->prev = prev_span;
  }
}

////////////
///
/// Span control
///
//////

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_heap_cache_ಸೇರಿಸು(heap_t *heap, span_t *span);

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_heap_finalize(heap_t *heap);

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_ಎಣಿಕೆ);

//! Declare the span to be a subspan and store distance from master span and
//! span count
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_ಎಣಿಕೆ) {
  rpmalloc_assert((subspan != master) || (subspan->flags & SPAN_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  ಯದಿ (subspan != master) {
    subspan->flags = SPAN_FLAG_SUBSPAN;
    subspan->offset_from_master =
        (uint32_t)((uintptr_t)pointer_diff(subspan, master) >>
                   _memory_span_size_shift);
    subspan->align_offset = 0;
  }
  subspan->span_ಎಣಿಕೆ = (uint32_t)span_ಎಣಿಕೆ;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
ಸ್ಥಿರ span_t *_rpmalloc_span_map_from_reserve(heap_t *heap,
                                               size_t span_ಎಣಿಕೆ) {
  // Update the heap span reserve
  span_t *span = heap->span_reserve;
  heap->span_reserve =
      (span_t *)pointer_offset(span, span_ಎಣಿಕೆ * _memory_span_size);
  heap->spans_reserved -= (uint32_t)span_ಎಣಿಕೆ;

  _rpmalloc_span_mark_as_subspan_unless_master(heap->span_reserve_master, span,
                                               span_ಎಣಿಕೆ);
  ಯದಿ (span_ಎಣಿಕೆ <= LARGE_CLASS_ಎಣಿಕೆ)
    _rpmalloc_stat_inc(&heap->span_use[span_ಎಣಿಕೆ - 1].spans_from_reserved);

  ಪ್ರತ್ಯಾಗಮ span;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
ಸ್ಥಿರ size_t rpmalloc_span_align_ಎಣಿಕೆ(size_t span_ಎಣಿಕೆ) {
  size_t request_ಎಣಿಕೆ = (span_ಎಣಿಕೆ > memory_span_map_ಎಣಿಕೆ)
                             ? span_ಎಣಿಕೆ
                             : memory_span_map_ಎಣಿಕೆ;
  ಯದಿ ((_memory_page_size > _memory_span_size) &&
      ((request_ಎಣಿಕೆ * _memory_span_size) % _memory_page_size))
    request_ಎಣಿಕೆ +=
        memory_span_map_ಎಣಿಕೆ - (request_ಎಣಿಕೆ % memory_span_map_ಎಣಿಕೆ);
  ಪ್ರತ್ಯಾಗಮ request_ಎಣಿಕೆ;
}

//! Setup a newly mapped span
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_span_initialize(span_t *span, size_t total_span_ಎಣಿಕೆ,
                                      size_t span_ಎಣಿಕೆ, size_t align_offset) {
  span->total_spans = (uint32_t)total_span_ಎಣಿಕೆ;
  span->span_ಎಣಿಕೆ = (uint32_t)span_ಎಣಿಕೆ;
  span->align_offset = (uint32_t)align_offset;
  span->flags = SPAN_FLAG_MASTER;
  atomic_store32(&span->remaining_spans, (int32_t)total_span_ಎಣಿಕೆ);
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_span_unmap(span_t *span);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
ಸ್ಥಿರ span_t *rpmalloc_span_map_aligned_ಎಣಿಕೆ(heap_t *heap,
                                                size_t span_ಎಣಿಕೆ) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_span_ಎಣಿಕೆ = rpmalloc_span_align_ಎಣಿಕೆ(span_ಎಣಿಕೆ);
  size_t align_offset = 0;
  span_t *span = (span_t *)_rpmalloc_mmap(
      aligned_span_ಎಣಿಕೆ * _memory_span_size, &align_offset);
  ಯದಿ (!span)
    ಪ್ರತ್ಯಾಗಮ 0;
  _rpmalloc_span_initialize(span, aligned_span_ಎಣಿಕೆ, span_ಎಣಿಕೆ, align_offset);
  _rpmalloc_stat_inc(&_master_spans);
  ಯದಿ (span_ಎಣಿಕೆ <= LARGE_CLASS_ಎಣಿಕೆ)
    _rpmalloc_stat_inc(&heap->span_use[span_ಎಣಿಕೆ - 1].spans_map_calls);
  ಯದಿ (aligned_span_ಎಣಿಕೆ > span_ಎಣಿಕೆ) {
    span_t *reserved_spans =
        (span_t *)pointer_offset(span, span_ಎಣಿಕೆ * _memory_span_size);
    size_t reserved_ಎಣಿಕೆ = aligned_span_ಎಣಿಕೆ - span_ಎಣಿಕೆ;
    ಯದಿ (heap->spans_reserved) {
      _rpmalloc_span_mark_as_subspan_unless_master(
          heap->span_reserve_master, heap->span_reserve, heap->spans_reserved);
      rpmalloc_heap_cache_ಸೇರಿಸು(heap, heap->span_reserve);
    }
    ಯದಿ (reserved_ಎಣಿಕೆ > memory_heap_reserve_ಎಣಿಕೆ) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&_memory_global_lock) == 1,
                      "Global spin lock not held as expected");
      size_t remain_ಎಣಿಕೆ = reserved_ಎಣಿಕೆ - memory_heap_reserve_ಎಣಿಕೆ;
      reserved_ಎಣಿಕೆ = memory_heap_reserve_ಎಣಿಕೆ;
      span_t *remain_span = (span_t *)pointer_offset(
          reserved_spans, reserved_ಎಣಿಕೆ * _memory_span_size);
      ಯದಿ (_memory_global_reserve) {
        _rpmalloc_span_mark_as_subspan_unless_master(
            _memory_global_reserve_master, _memory_global_reserve,
            memory_global_reserve_ಎಣಿಕೆ);
        _rpmalloc_span_unmap(_memory_global_reserve);
      }
      _rpmalloc_global_set_reserved_spans(span, remain_span, remain_ಎಣಿಕೆ);
    }
    _rpmalloc_heap_set_reserved_spans(heap, span, reserved_spans,
                                      reserved_ಎಣಿಕೆ);
  }
  ಪ್ರತ್ಯಾಗಮ span;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
ಸ್ಥಿರ span_t *_rpmalloc_span_map(heap_t *heap, size_t span_ಎಣಿಕೆ) {
  ಯದಿ (span_ಎಣಿಕೆ <= heap->spans_reserved)
    ಪ್ರತ್ಯಾಗಮ _rpmalloc_span_map_from_reserve(heap, span_ಎಣಿಕೆ);
  span_t *span = 0;
  ಪೂರ್ಣಾಙ್ಕ use_global_reserve =
      (_memory_page_size > _memory_span_size) ||
      (memory_span_map_ಎಣಿಕೆ > memory_heap_reserve_ಎಣಿಕೆ);
  ಯದಿ (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    ಯಾವತ್ (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
    ಯದಿ (memory_global_reserve_ಎಣಿಕೆ >= span_ಎಣಿಕೆ) {
      size_t reserve_ಎಣಿಕೆ =
          (!heap->spans_reserved ? memory_heap_reserve_ಎಣಿಕೆ : span_ಎಣಿಕೆ);
      ಯದಿ (memory_global_reserve_ಎಣಿಕೆ < reserve_ಎಣಿಕೆ)
        reserve_ಎಣಿಕೆ = memory_global_reserve_ಎಣಿಕೆ;
      span = _rpmalloc_global_get_reserved_spans(reserve_ಎಣಿಕೆ);
      ಯದಿ (span) {
        ಯದಿ (reserve_ಎಣಿಕೆ > span_ಎಣಿಕೆ) {
          span_t *reserved_span = (span_t *)pointer_offset(
              span, span_ಎಣಿಕೆ << _memory_span_size_shift);
          _rpmalloc_heap_set_reserved_spans(heap, _memory_global_reserve_master,
                                            reserved_span,
                                            reserve_ಎಣಿಕೆ - span_ಎಣಿಕೆ);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        span->span_ಎಣಿಕೆ = (uint32_t)span_ಎಣಿಕೆ;
      }
    }
  }
  ಯದಿ (!span)
    span = rpmalloc_span_map_aligned_ಎಣಿಕೆ(heap, span_ಎಣಿಕೆ);
  ಯದಿ (use_global_reserve)
    atomic_store32_release(&_memory_global_lock, 0);
  ಪ್ರತ್ಯಾಗಮ span;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_span_unmap(span_t *span) {
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");

  ಪೂರ್ಣಾಙ್ಕ is_master = !!(span->flags & SPAN_FLAG_MASTER);
  span_t *master =
      is_master ? span
                : ((span_t *)pointer_offset(
                      span, -(intptr_t)((uintptr_t)span->offset_from_master *
                                        _memory_span_size)));
  rpmalloc_assert(is_master || (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");

  size_t span_ಎಣಿಕೆ = span->span_ಎಣಿಕೆ;
  ಯದಿ (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(span->align_offset == 0, "Span align offset corrupted");
    ಯದಿ (_memory_span_size >= _memory_page_size)
      _rpmalloc_unmap(span, span_ಎಣಿಕೆ * _memory_span_size, 0, 0);
  } ಅನ್ಯಥಾ {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    span->flags |=
        SPAN_FLAG_MASTER | SPAN_FLAG_SUBSPAN | SPAN_FLAG_UNMAPPED_MASTER;
    _rpmalloc_stat_add(&_unmapped_master_spans, 1);
  }

  ಯದಿ (atomic_add32(&master->remaining_spans, -(int32_t)span_ಎಣಿಕೆ) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & SPAN_FLAG_MASTER) &&
                        !!(master->flags & SPAN_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_ಎಣಿಕೆ = master->span_ಎಣಿಕೆ;
    ಯದಿ (_memory_span_size < _memory_page_size)
      unmap_ಎಣಿಕೆ = master->total_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_ಎಣಿಕೆ * _memory_span_size,
                    master->align_offset,
                    (size_t)master->total_spans * _memory_span_size);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_span_release_to_cache(heap_t *heap, span_t *span) {
  rpmalloc_assert(heap == span->heap, "Span heap pointer corrupted");
  rpmalloc_assert(span->size_class < SIZE_CLASS_ಎಣಿಕೆ,
                  "Invalid span size class");
  rpmalloc_assert(span->span_ಎಣಿಕೆ == 1, "Invalid span count");
#ಯದಿ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&heap->span_use[0].current);
#endif
  _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
  ಯದಿ (!heap->finalize) {
    _rpmalloc_stat_inc(&heap->span_use[0].spans_to_cache);
    _rpmalloc_stat_inc(&heap->size_class_use[span->size_class].spans_to_cache);
    ಯದಿ (heap->size_class[span->size_class].cache)
      rpmalloc_heap_cache_ಸೇರಿಸು(heap,
                                  heap->size_class[span->size_class].cache);
    heap->size_class[span->size_class].cache = span;
  } ಅನ್ಯಥಾ {
    _rpmalloc_span_unmap(span);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
ಸ್ಥಿರ uint32_t free_list_partial_init(ಶೂನ್ಯ **list, ಶೂನ್ಯ **first_block,
                                       ಶೂನ್ಯ *page_start, ಶೂನ್ಯ *block_start,
                                       uint32_t block_ಎಣಿಕೆ,
                                       uint32_t block_size) {
  rpmalloc_assert(block_ಎಣಿಕೆ, "Internal failure");
  *first_block = block_start;
  ಯದಿ (block_ಎಣಿಕೆ > 1) {
    ಶೂನ್ಯ *free_block = pointer_offset(block_start, block_size);
    ಶೂನ್ಯ *block_end =
        pointer_offset(block_start, (size_t)block_size * block_ಎಣಿಕೆ);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    ಯದಿ (block_size < (_memory_page_size >> 1)) {
      ಶೂನ್ಯ *page_end = pointer_offset(page_start, _memory_page_size);
      ಯದಿ (page_end < block_end)
        block_end = page_end;
    }
    *list = free_block;
    block_ಎಣಿಕೆ = 2;
    ಶೂನ್ಯ *next_block = pointer_offset(free_block, block_size);
    ಯಾವತ್ (next_block < block_end) {
      *((ಶೂನ್ಯ **)free_block) = next_block;
      free_block = next_block;
      ++block_ಎಣಿಕೆ;
      next_block = pointer_offset(next_block, block_size);
    }
    *((ಶೂನ್ಯ **)free_block) = 0;
  } ಅನ್ಯಥಾ {
    *list = 0;
  }
  ಪ್ರತ್ಯಾಗಮ block_ಎಣಿಕೆ;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_span_initialize_new(heap_t *heap,
                                           heap_size_class_t *heap_size_class,
                                           span_t *span, uint32_t class_idx) {
  rpmalloc_assert(span->span_ಎಣಿಕೆ == 1, "Internal failure");
  size_class_t *size_class = _memory_size_class + class_idx;
  span->size_class = class_idx;
  span->heap = heap;
  span->flags &= ~SPAN_FLAG_ALIGNED_BLOCKS;
  span->block_size = size_class->block_size;
  span->block_ಎಣಿಕೆ = size_class->block_ಎಣಿಕೆ;
  span->free_list = 0;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  ಶೂನ್ಯ *block;
  span->free_list_limit =
      free_list_partial_init(&heap_size_class->free_list, &block, span,
                             pointer_offset(span, SPAN_HEADER_SIZE),
                             size_class->block_ಎಣಿಕೆ, size_class->block_size);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  ಯದಿ (span->free_list_limit < span->block_ಎಣಿಕೆ) {
    _rpmalloc_span_double_link_list_add(&heap_size_class->partial_span, span);
    span->used_ಎಣಿಕೆ = span->free_list_limit;
  } ಅನ್ಯಥಾ {
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_ಎಣಿಕೆ;
    span->used_ಎಣಿಕೆ = span->block_ಎಣಿಕೆ;
  }
  ಪ್ರತ್ಯಾಗಮ block;
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_span_extract_free_list_deferred(span_t *span) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  ಕುರು {
    span->free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } ಯಾವತ್ (span->free_list == INVALID_POINTER);
  span->used_ಎಣಿಕೆ -= span->list_size;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ _rpmalloc_span_is_fully_utilized(span_t *span) {
  rpmalloc_assert(span->free_list_limit <= span->block_ಎಣಿಕೆ,
                  "Span free list corrupted");
  ಪ್ರತ್ಯಾಗಮ !span->free_list && (span->free_list_limit >= span->block_ಎಣಿಕೆ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ _rpmalloc_span_finalize(heap_t *heap, size_t iclass, span_t *span,
                                   span_t **list_head) {
  ಶೂನ್ಯ *free_list = heap->size_class[iclass].free_list;
  span_t *class_span = (span_t *)((uintptr_t)free_list & _memory_span_mask);
  ಯದಿ (span == class_span) {
    // Adopt the heap class free list back into the span free list
    ಶೂನ್ಯ *block = span->free_list;
    ಶೂನ್ಯ *last_block = 0;
    ಯಾವತ್ (block) {
      last_block = block;
      block = *((ಶೂನ್ಯ **)block);
    }
    uint32_t free_ಎಣಿಕೆ = 0;
    block = free_list;
    ಯಾವತ್ (block) {
      ++free_ಎಣಿಕೆ;
      block = *((ಶೂನ್ಯ **)block);
    }
    ಯದಿ (last_block) {
      *((ಶೂನ್ಯ **)last_block) = free_list;
    } ಅನ್ಯಥಾ {
      span->free_list = free_list;
    }
    heap->size_class[iclass].free_list = 0;
    span->used_ಎಣಿಕೆ -= free_ಎಣಿಕೆ;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(span->list_size == span->used_ಎಣಿಕೆ, "Memory leak detected");
  ಯದಿ (span->list_size == span->used_ಎಣಿಕೆ) {
    _rpmalloc_stat_dec(&heap->span_use[0].current);
    _rpmalloc_stat_dec(&heap->size_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    ಯದಿ (list_head)
      _rpmalloc_span_double_link_list_remove(list_head, span);
    _rpmalloc_span_unmap(span);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

////////////
///
/// Global cache
///
//////

#ಯದಿ ENABLE_GLOBAL_CACHE

//! Finalize a global cache
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  ಯಾವತ್ (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

  ಪ್ರತಿ (size_t ispan = 0; ispan < cache->ಎಣಿಕೆ; ++ispan)
    _rpmalloc_span_unmap(cache->span[ispan]);
  cache->ಎಣಿಕೆ = 0;

  ಯಾವತ್ (cache->overflow) {
    span_t *span = cache->overflow;
    cache->overflow = span->next;
    _rpmalloc_span_unmap(span);
  }

  atomic_store32_release(&cache->lock, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_global_cache_ಸೇರಿಸು_spans(span_t **span,
                                                size_t span_ಎಣಿಕೆ,
                                                size_t ಎಣಿಕೆ) {
  ಧ್ರುವ size_t cache_limit =
      (span_ಎಣಿಕೆ == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_SPAN_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_SPAN_LARGE_CACHE - (span_ಎಣಿಕೆ >> 1));

  global_cache_t *cache = &_memory_span_cache[span_ಎಣಿಕೆ - 1];

  size_t ಸೇರಿಸು_ಎಣಿಕೆ = ಎಣಿಕೆ;
  ಯಾವತ್ (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#ಯದಿ ENABLE_STATISTICS
  cache->ಸೇರಿಸು_ಎಣಿಕೆ += ಎಣಿಕೆ;
#endif
  ಯದಿ ((cache->ಎಣಿಕೆ + ಸೇರಿಸು_ಎಣಿಕೆ) > cache_limit)
    ಸೇರಿಸು_ಎಣಿಕೆ = cache_limit - cache->ಎಣಿಕೆ;

  memcpy(cache->span + cache->ಎಣಿಕೆ, span, ಪರಿಮಾಣ(span_t *) * ಸೇರಿಸು_ಎಣಿಕೆ);
  cache->ಎಣಿಕೆ += (uint32_t)ಸೇರಿಸು_ಎಣಿಕೆ;

#ಯದಿ ENABLE_UNLIMITED_CACHE
  ಯಾವತ್ (ಸೇರಿಸು_ಎಣಿಕೆ < ಎಣಿಕೆ) {
#ಅನ್ಯಥಾ
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  ಯಾವತ್ ((_memory_page_size > _memory_span_size) && (ಸೇರಿಸು_ಎಣಿಕೆ < ಎಣಿಕೆ)) {
#endif
    span_t *current_span = span[ಸೇರಿಸು_ಎಣಿಕೆ++];
    current_span->next = cache->overflow;
    cache->overflow = current_span;
  }
  atomic_store32_release(&cache->lock, 0);

  span_t *keep = 0;
  ಪ್ರತಿ (size_t ispan = ಸೇರಿಸು_ಎಣಿಕೆ; ispan < ಎಣಿಕೆ; ++ispan) {
    span_t *current_span = span[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    ಯದಿ ((current_span->flags & SPAN_FLAG_MASTER) &&
        (atomic_load32(&current_span->remaining_spans) >
         (int32_t)current_span->span_ಎಣಿಕೆ)) {
      current_span->next = keep;
      keep = current_span;
    } ಅನ್ಯಥಾ {
      _rpmalloc_span_unmap(current_span);
    }
  }

  ಯದಿ (keep) {
    ಯಾವತ್ (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    ಯಾವತ್ (keep) {
      ಪ್ರತಿ (; islot < cache->ಎಣಿಕೆ; ++islot) {
        span_t *current_span = cache->span[islot];
        ಯದಿ (!(current_span->flags & SPAN_FLAG_MASTER) ||
            ((current_span->flags & SPAN_FLAG_MASTER) &&
             (atomic_load32(&current_span->remaining_spans) <=
              (int32_t)current_span->span_ಎಣಿಕೆ))) {
          _rpmalloc_span_unmap(current_span);
          cache->span[islot] = keep;
          ಭಙ್ಗ;
        }
      }
      ಯದಿ (islot == cache->ಎಣಿಕೆ)
        ಭಙ್ಗ;
      keep = keep->next;
    }

    ಯದಿ (keep) {
      span_t *tail = keep;
      ಯಾವತ್ (tail->next)
        tail = tail->next;
      tail->next = cache->overflow;
      cache->overflow = keep;
    }

    atomic_store32_release(&cache->lock, 0);
  }
}

ಸ್ಥಿರ size_t _rpmalloc_global_cache_extract_spans(span_t **span,
                                                   size_t span_ಎಣಿಕೆ,
                                                   size_t ಎಣಿಕೆ) {
  global_cache_t *cache = &_memory_span_cache[span_ಎಣಿಕೆ - 1];

  size_t extract_ಎಣಿಕೆ = 0;
  ಯಾವತ್ (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#ಯದಿ ENABLE_STATISTICS
  cache->extract_ಎಣಿಕೆ += ಎಣಿಕೆ;
#endif
  size_t want = ಎಣಿಕೆ - extract_ಎಣಿಕೆ;
  ಯದಿ (want > cache->ಎಣಿಕೆ)
    want = cache->ಎಣಿಕೆ;

  memcpy(span + extract_ಎಣಿಕೆ, cache->span + (cache->ಎಣಿಕೆ - want),
         ಪರಿಮಾಣ(span_t *) * want);
  cache->ಎಣಿಕೆ -= (uint32_t)want;
  extract_ಎಣಿಕೆ += want;

  ಯಾವತ್ ((extract_ಎಣಿಕೆ < ಎಣಿಕೆ) && cache->overflow) {
    span_t *current_span = cache->overflow;
    span[extract_ಎಣಿಕೆ++] = current_span;
    cache->overflow = current_span->next;
  }

#ಯದಿ ENABLE_ASSERTS
  ಪ್ರತಿ (size_t ispan = 0; ispan < extract_ಎಣಿಕೆ; ++ispan) {
    rpmalloc_assert(span[ispan]->span_ಎಣಿಕೆ == span_ಎಣಿಕೆ,
                    "Global cache span count mismatch");
  }
#endif

  atomic_store32_release(&cache->lock, 0);

  ಪ್ರತ್ಯಾಗಮ extract_ಎಣಿಕೆ;
}

#endif

////////////
///
/// Heap control
///
//////

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate_huge(span_t *);

//! Store the given spans as reserve in the given heap
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_ಎಣಿಕೆ) {
  heap->span_reserve_master = master;
  heap->span_reserve = reserve;
  heap->spans_reserved = (uint32_t)reserve_span_ಎಣಿಕೆ;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_heap_cache_adopt_deferred(heap_t *heap,
                                                span_t **single_span) {
  span_t *span = (span_t *)((ಶೂನ್ಯ *)atomic_exchange_ptr_acquire(
      &heap->span_free_deferred, 0));
  ಯಾವತ್ (span) {
    span_t *next_span = (span_t *)span->free_list;
    rpmalloc_assert(span->heap == heap, "Span heap pointer corrupted");
    ಯದಿ (EXPECTED(span->size_class < SIZE_CLASS_ಎಣಿಕೆ)) {
      rpmalloc_assert(heap->full_span_ಎಣಿಕೆ, "Heap span counter corrupted");
      --heap->full_span_ಎಣಿಕೆ;
      _rpmalloc_stat_dec(&heap->span_use[0].spans_deferred);
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
      _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                             span);
#endif
      _rpmalloc_stat_dec(&heap->span_use[0].current);
      _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
      ಯದಿ (single_span && !*single_span)
        *single_span = span;
      ಅನ್ಯಥಾ
        rpmalloc_heap_cache_ಸೇರಿಸು(heap, span);
    } ಅನ್ಯಥಾ {
      ಯದಿ (span->size_class == SIZE_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(span);
      } ಅನ್ಯಥಾ {
        rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(heap->full_span_ಎಣಿಕೆ, "Heap span counter corrupted");
        --heap->full_span_ಎಣಿಕೆ;
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
        _rpmalloc_span_double_link_list_remove(&heap->large_huge_span, span);
#endif
        uint32_t idx = span->span_ಎಣಿಕೆ - 1;
        _rpmalloc_stat_dec(&heap->span_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&heap->span_use[idx].current);
        ಯದಿ (!idx && single_span && !*single_span)
          *single_span = span;
        ಅನ್ಯಥಾ
          rpmalloc_heap_cache_ಸೇರಿಸು(heap, span);
      }
    }
    span = next_span;
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_heap_unmap(heap_t *heap) {
  ಯದಿ (!heap->master_heap) {
    ಯದಿ ((heap->finalize > 1) && !atomic_load32(&heap->child_ಎಣಿಕೆ)) {
      span_t *span = (span_t *)((uintptr_t)heap & _memory_span_mask);
      _rpmalloc_span_unmap(span);
    }
  } ಅನ್ಯಥಾ {
    ಯದಿ (atomic_decr32(&heap->master_heap->child_ಎಣಿಕೆ) == 0) {
      _rpmalloc_heap_unmap(heap->master_heap);
    }
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_heap_global_finalize(heap_t *heap) {
  ಯದಿ (heap->finalize++ > 1) {
    --heap->finalize;
    ಪ್ರತ್ಯಾಗಮ;
  }

  _rpmalloc_heap_finalize(heap);

#ಯದಿ ENABLE_THREAD_CACHE
  ಪ್ರತಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    span_cache_t *span_cache;
    ಯದಿ (!iclass)
      span_cache = &heap->span_cache;
    ಅನ್ಯಥಾ
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    ಪ್ರತಿ (size_t ispan = 0; ispan < span_cache->ಎಣಿಕೆ; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->ಎಣಿಕೆ = 0;
  }
#endif

  ಯದಿ (heap->full_span_ಎಣಿಕೆ) {
    --heap->finalize;
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಪ್ರತಿ (size_t iclass = 0; iclass < SIZE_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಯದಿ (heap->size_class[iclass].free_list ||
        heap->size_class[iclass].partial_span) {
      --heap->finalize;
      ಪ್ರತ್ಯಾಗಮ;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap_t *list_heap = _memory_heaps[list_idx];
  ಯದಿ (list_heap == heap) {
    _memory_heaps[list_idx] = heap->next_heap;
  } ಅನ್ಯಥಾ {
    ಯಾವತ್ (list_heap->next_heap != heap)
      list_heap = list_heap->next_heap;
    list_heap->next_heap = heap->next_heap;
  }

  _rpmalloc_heap_unmap(heap);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
ಸ್ಥಿರ ಶೂನ್ಯ rpmalloc_heap_cache_ಸೇರಿಸು(heap_t *heap, span_t *span) {
  ಯದಿ (UNEXPECTED(heap->finalize != 0)) {
    _rpmalloc_span_unmap(span);
    _rpmalloc_heap_global_finalize(heap);
    ಪ್ರತ್ಯಾಗಮ;
  }
#ಯದಿ ENABLE_THREAD_CACHE
  size_t span_ಎಣಿಕೆ = span->span_ಎಣಿಕೆ;
  _rpmalloc_stat_inc(&heap->span_use[span_ಎಣಿಕೆ - 1].spans_to_cache);
  ಯದಿ (span_ಎಣಿಕೆ == 1) {
    span_cache_t *span_cache = &heap->span_cache;
    span_cache->span[span_cache->ಎಣಿಕೆ++] = span;
    ಯದಿ (span_cache->ಎಣಿಕೆ == MAX_THREAD_SPAN_CACHE) {
      ಧ್ರುವ size_t remain_ಎಣಿಕೆ =
          MAX_THREAD_SPAN_CACHE - THREAD_SPAN_CACHE_TRANSFER;
#ಯದಿ ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           THREAD_SPAN_CACHE_TRANSFER * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_ಎಣಿಕೆ - 1].spans_to_global,
                         THREAD_SPAN_CACHE_TRANSFER);
      rpmalloc_global_cache_ಸೇರಿಸು_spans(span_cache->span + remain_ಎಣಿಕೆ,
                                          span_ಎಣಿಕೆ,
                                          THREAD_SPAN_CACHE_TRANSFER);
#ಅನ್ಯಥಾ
      ಪ್ರತಿ (size_t ispan = 0; ispan < THREAD_SPAN_CACHE_TRANSFER; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_ಎಣಿಕೆ + ispan]);
#endif
      span_cache->ಎಣಿಕೆ = remain_ಎಣಿಕೆ;
    }
  } ಅನ್ಯಥಾ {
    size_t cache_idx = span_ಎಣಿಕೆ - 2;
    span_large_cache_t *span_cache = heap->span_large_cache + cache_idx;
    span_cache->span[span_cache->ಎಣಿಕೆ++] = span;
    ಧ್ರುವ size_t cache_limit =
        (MAX_THREAD_SPAN_LARGE_CACHE - (span_ಎಣಿಕೆ >> 1));
    ಯದಿ (span_cache->ಎಣಿಕೆ == cache_limit) {
      ಧ್ರುವ size_t transfer_limit = 2 + (cache_limit >> 2);
      ಧ್ರುವ size_t transfer_ಎಣಿಕೆ =
          (THREAD_SPAN_LARGE_CACHE_TRANSFER <= transfer_limit
               ? THREAD_SPAN_LARGE_CACHE_TRANSFER
               : transfer_limit);
      ಧ್ರುವ size_t remain_ಎಣಿಕೆ = cache_limit - transfer_ಎಣಿಕೆ;
#ಯದಿ ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           transfer_ಎಣಿಕೆ * span_ಎಣಿಕೆ * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_ಎಣಿಕೆ - 1].spans_to_global,
                         transfer_ಎಣಿಕೆ);
      rpmalloc_global_cache_ಸೇರಿಸು_spans(span_cache->span + remain_ಎಣಿಕೆ,
                                          span_ಎಣಿಕೆ, transfer_ಎಣಿಕೆ);
#ಅನ್ಯಥಾ
      ಪ್ರತಿ (size_t ispan = 0; ispan < transfer_ಎಣಿಕೆ; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_ಎಣಿಕೆ + ispan]);
#endif
      span_cache->ಎಣಿಕೆ = remain_ಎಣಿಕೆ;
    }
  }
#ಅನ್ಯಥಾ
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(heap);
  _rpmalloc_span_unmap(span);
#endif
}

//! Extract the given number of spans from the different cache levels
ಸ್ಥಿರ span_t *_rpmalloc_heap_thread_cache_extract(heap_t *heap,
                                                   size_t span_ಎಣಿಕೆ) {
  span_t *span = 0;
#ಯದಿ ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  ಯದಿ (span_ಎಣಿಕೆ == 1)
    span_cache = &heap->span_cache;
  ಅನ್ಯಥಾ
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_ಎಣಿಕೆ - 2));
  ಯದಿ (span_cache->ಎಣಿಕೆ) {
    _rpmalloc_stat_inc(&heap->span_use[span_ಎಣಿಕೆ - 1].spans_from_cache);
    ಪ್ರತ್ಯಾಗಮ span_cache->span[--span_cache->ಎಣಿಕೆ];
  }
#endif
  ಪ್ರತ್ಯಾಗಮ span;
}

ಸ್ಥಿರ span_t *_rpmalloc_heap_thread_cache_deferred_extract(heap_t *heap,
                                                            size_t span_ಎಣಿಕೆ) {
  span_t *span = 0;
  ಯದಿ (span_ಎಣಿಕೆ == 1) {
    _rpmalloc_heap_cache_adopt_deferred(heap, &span);
  } ಅನ್ಯಥಾ {
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
    span = _rpmalloc_heap_thread_cache_extract(heap, span_ಎಣಿಕೆ);
  }
  ಪ್ರತ್ಯಾಗಮ span;
}

ಸ್ಥಿರ span_t *_rpmalloc_heap_reserved_extract(heap_t *heap,
                                               size_t span_ಎಣಿಕೆ) {
  ಯದಿ (heap->spans_reserved >= span_ಎಣಿಕೆ)
    ಪ್ರತ್ಯಾಗಮ _rpmalloc_span_map(heap, span_ಎಣಿಕೆ);
  ಪ್ರತ್ಯಾಗಮ 0;
}

//! Extract a span from the global cache
ಸ್ಥಿರ span_t *_rpmalloc_heap_global_cache_extract(heap_t *heap,
                                                   size_t span_ಎಣಿಕೆ) {
#ಯದಿ ENABLE_GLOBAL_CACHE
#ಯದಿ ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  size_t wanted_ಎಣಿಕೆ;
  ಯದಿ (span_ಎಣಿಕೆ == 1) {
    span_cache = &heap->span_cache;
    wanted_ಎಣಿಕೆ = THREAD_SPAN_CACHE_TRANSFER;
  } ಅನ್ಯಥಾ {
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_ಎಣಿಕೆ - 2));
    wanted_ಎಣಿಕೆ = THREAD_SPAN_LARGE_CACHE_TRANSFER;
  }
  span_cache->ಎಣಿಕೆ = _rpmalloc_global_cache_extract_spans(
      span_cache->span, span_ಎಣಿಕೆ, wanted_ಎಣಿಕೆ);
  ಯದಿ (span_cache->ಎಣಿಕೆ) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_ಎಣಿಕೆ * span_cache->ಎಣಿಕೆ * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_ಎಣಿಕೆ - 1].spans_from_global,
                       span_cache->ಎಣಿಕೆ);
    ಪ್ರತ್ಯಾಗಮ span_cache->span[--span_cache->ಎಣಿಕೆ];
  }
#ಅನ್ಯಥಾ
  span_t *span = 0;
  size_t ಎಣಿಕೆ = _rpmalloc_global_cache_extract_spans(&span, span_ಎಣಿಕೆ, 1);
  ಯದಿ (ಎಣಿಕೆ) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_ಎಣಿಕೆ * ಎಣಿಕೆ * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_ಎಣಿಕೆ - 1].spans_from_global,
                       ಎಣಿಕೆ);
    ಪ್ರತ್ಯಾಗಮ span;
  }
#endif
#endif
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(heap);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(span_ಎಣಿಕೆ);
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_inc_span_statistics(heap_t *heap, size_t span_ಎಣಿಕೆ,
                                          uint32_t class_idx) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(heap);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(span_ಎಣಿಕೆ);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(class_idx);
#ಯದಿ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)span_ಎಣಿಕೆ - 1;
  uint32_t current_ಎಣಿಕೆ =
      (uint32_t)atomic_incr32(&heap->span_use[idx].current);
  ಯದಿ (current_ಎಣಿಕೆ > (uint32_t)atomic_load32(&heap->span_use[idx].ಮೇಲಿನ))
    atomic_store32(&heap->span_use[idx].ಮೇಲಿನ, (int32_t)current_ಎಣಿಕೆ);
  _rpmalloc_stat_add_peak(&heap->size_class_use[class_idx].spans_current, 1,
                          heap->size_class_use[class_idx].spans_peak);
#endif
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
ಸ್ಥಿರ span_t *
_rpmalloc_heap_extract_new_span(heap_t *heap,
                                heap_size_class_t *heap_size_class,
                                size_t span_ಎಣಿಕೆ, uint32_t class_idx) {
  span_t *span;
#ಯದಿ ENABLE_THREAD_CACHE
  ಯದಿ (heap_size_class && heap_size_class->cache) {
    span = heap_size_class->cache;
    heap_size_class->cache =
        (heap->span_cache.ಎಣಿಕೆ
             ? heap->span_cache.span[--heap->span_cache.ಎಣಿಕೆ]
             : 0);
    _rpmalloc_inc_span_statistics(heap, span_ಎಣಿಕೆ, class_idx);
    ಪ್ರತ್ಯಾಗಮ span;
  }
#endif
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t base_span_ಎಣಿಕೆ = span_ಎಣಿಕೆ;
  size_t limit_span_ಎಣಿಕೆ =
      (span_ಎಣಿಕೆ > 2) ? (span_ಎಣಿಕೆ + (span_ಎಣಿಕೆ >> 1)) : span_ಎಣಿಕೆ;
  ಯದಿ (limit_span_ಎಣಿಕೆ > LARGE_CLASS_ಎಣಿಕೆ)
    limit_span_ಎಣಿಕೆ = LARGE_CLASS_ಎಣಿಕೆ;
  ಕುರು {
    span = _rpmalloc_heap_thread_cache_extract(heap, span_ಎಣಿಕೆ);
    ಯದಿ (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_ಎಣಿಕೆ, class_idx);
      ಪ್ರತ್ಯಾಗಮ span;
    }
    span = _rpmalloc_heap_thread_cache_deferred_extract(heap, span_ಎಣಿಕೆ);
    ಯದಿ (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_ಎಣಿಕೆ, class_idx);
      ಪ್ರತ್ಯಾಗಮ span;
    }
    span = _rpmalloc_heap_global_cache_extract(heap, span_ಎಣಿಕೆ);
    ಯದಿ (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_ಎಣಿಕೆ, class_idx);
      ಪ್ರತ್ಯಾಗಮ span;
    }
    span = _rpmalloc_heap_reserved_extract(heap, span_ಎಣಿಕೆ);
    ಯದಿ (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_reserved);
      _rpmalloc_inc_span_statistics(heap, span_ಎಣಿಕೆ, class_idx);
      ಪ್ರತ್ಯಾಗಮ span;
    }
    ++span_ಎಣಿಕೆ;
  } ಯಾವತ್ (span_ಎಣಿಕೆ <= limit_span_ಎಣಿಕೆ);
  // Final fallback, map in more virtual memory
  span = _rpmalloc_span_map(heap, base_span_ಎಣಿಕೆ);
  _rpmalloc_inc_span_statistics(heap, base_span_ಎಣಿಕೆ, class_idx);
  _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_map_calls);
  ಪ್ರತ್ಯಾಗಮ span;
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_heap_initialize(heap_t *heap) {
  _rpmalloc_memset_const(heap, 0, ಪರಿಮಾಣ(heap_t));
  // Get a new heap ID
  heap->id = 1 + atomic_incr32(&_memory_heap_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap->next_heap = _memory_heaps[list_idx];
  _memory_heaps[list_idx] = heap;
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_heap_orphan(heap_t *heap, ಪೂರ್ಣಾಙ್ಕ first_class) {
  heap->owner_thread = (uintptr_t)-1;
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
  heap_t **heap_list =
      (first_class ? &_memory_first_class_orphan_heaps : &_memory_orphan_heaps);
#ಅನ್ಯಥಾ
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(first_class);
  heap_t **heap_list = &_memory_orphan_heaps;
#endif
  heap->next_orphan = *heap_list;
  *heap_list = heap;
}

//! Allocate a new heap from newly mapped memory pages
ಸ್ಥಿರ heap_t *_rpmalloc_heap_allocate_new(ಶೂನ್ಯ) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t heap_size = ಪರಿಮಾಣ(heap_t);
  size_t aligned_heap_size = 16 * ((heap_size + 15) / 16);
  size_t request_heap_ಎಣಿಕೆ = 16;
  size_t heap_span_ಎಣಿಕೆ = ((aligned_heap_size * request_heap_ಎಣಿಕೆ) +
                            ಪರಿಮಾಣ(span_t) + _memory_span_size - 1) /
                           _memory_span_size;
  size_t block_size = _memory_span_size * heap_span_ಎಣಿಕೆ;
  size_t span_ಎಣಿಕೆ = heap_span_ಎಣಿಕೆ;
  span_t *span = 0;
  // If there are global reserved spans, use these first
  ಯದಿ (memory_global_reserve_ಎಣಿಕೆ >= heap_span_ಎಣಿಕೆ) {
    span = _rpmalloc_global_get_reserved_spans(heap_span_ಎಣಿಕೆ);
  }
  ಯದಿ (!span) {
    ಯದಿ (_memory_page_size > block_size) {
      span_ಎಣಿಕೆ = _memory_page_size / _memory_span_size;
      block_size = _memory_page_size;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_heap_ಎಣಿಕೆ =
          (block_size - ಪರಿಮಾಣ(span_t)) / aligned_heap_size;
      ಯದಿ (possible_heap_ಎಣಿಕೆ >= (request_heap_ಎಣಿಕೆ * 16))
        request_heap_ಎಣಿಕೆ *= 16;
      ಅನ್ಯಥಾ ಯದಿ (possible_heap_ಎಣಿಕೆ < request_heap_ಎಣಿಕೆ)
        request_heap_ಎಣಿಕೆ = possible_heap_ಎಣಿಕೆ;
      heap_span_ಎಣಿಕೆ = ((aligned_heap_size * request_heap_ಎಣಿಕೆ) +
                         ಪರಿಮಾಣ(span_t) + _memory_span_size - 1) /
                        _memory_span_size;
    }

    size_t align_offset = 0;
    span = (span_t *)_rpmalloc_mmap(block_size, &align_offset);
    ಯದಿ (!span)
      ಪ್ರತ್ಯಾಗಮ 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    _rpmalloc_span_initialize(span, span_ಎಣಿಕೆ, heap_span_ಎಣಿಕೆ, align_offset);
  }

  size_t remain_size = _memory_span_size - ಪರಿಮಾಣ(span_t);
  heap_t *heap = (heap_t *)pointer_offset(span, ಪರಿಮಾಣ(span_t));
  _rpmalloc_heap_initialize(heap);

  // Put extra heaps as orphans
  size_t ಅಂಕೆ_heaps = remain_size / aligned_heap_size;
  ಯದಿ (ಅಂಕೆ_heaps < request_heap_ಎಣಿಕೆ)
    ಅಂಕೆ_heaps = request_heap_ಎಣಿಕೆ;
  atomic_store32(&heap->child_ಎಣಿಕೆ, (int32_t)ಅಂಕೆ_heaps - 1);
  heap_t *extra_heap = (heap_t *)pointer_offset(heap, aligned_heap_size);
  ಯಾವತ್ (ಅಂಕೆ_heaps > 1) {
    _rpmalloc_heap_initialize(extra_heap);
    extra_heap->master_heap = heap;
    _rpmalloc_heap_orphan(extra_heap, 1);
    extra_heap = (heap_t *)pointer_offset(extra_heap, aligned_heap_size);
    --ಅಂಕೆ_heaps;
  }

  ಯದಿ (span_ಎಣಿಕೆ > heap_span_ಎಣಿಕೆ) {
    // Cap reserved spans
    size_t remain_ಎಣಿಕೆ = span_ಎಣಿಕೆ - heap_span_ಎಣಿಕೆ;
    size_t reserve_ಎಣಿಕೆ =
        (remain_ಎಣಿಕೆ > memory_heap_reserve_ಎಣಿಕೆ ? memory_heap_reserve_ಎಣಿಕೆ
                                                   : remain_ಎಣಿಕೆ);
    span_t *remain_span =
        (span_t *)pointer_offset(span, heap_span_ಎಣಿಕೆ * _memory_span_size);
    _rpmalloc_heap_set_reserved_spans(heap, span, remain_span, reserve_ಎಣಿಕೆ);

    ಯದಿ (remain_ಎಣಿಕೆ > reserve_ಎಣಿಕೆ) {
      // Set to global reserved spans
      remain_span = (span_t *)pointer_offset(remain_span,
                                             reserve_ಎಣಿಕೆ * _memory_span_size);
      reserve_ಎಣಿಕೆ = remain_ಎಣಿಕೆ - reserve_ಎಣಿಕೆ;
      _rpmalloc_global_set_reserved_spans(span, remain_span, reserve_ಎಣಿಕೆ);
    }
  }

  ಪ್ರತ್ಯಾಗಮ heap;
}

ಸ್ಥಿರ heap_t *_rpmalloc_heap_extract_orphan(heap_t **heap_list) {
  heap_t *heap = *heap_list;
  *heap_list = (heap ? heap->next_orphan : 0);
  ಪ್ರತ್ಯಾಗಮ heap;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
ಸ್ಥಿರ heap_t *_rpmalloc_heap_allocate(ಪೂರ್ಣಾಙ್ಕ first_class) {
  heap_t *heap = 0;
  ಯಾವತ್ (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
    _rpmalloc_spin();
  ಯದಿ (first_class == 0)
    heap = _rpmalloc_heap_extract_orphan(&_memory_orphan_heaps);
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
  ಯದಿ (!heap)
    heap = _rpmalloc_heap_extract_orphan(&_memory_first_class_orphan_heaps);
#endif
  ಯದಿ (!heap)
    heap = _rpmalloc_heap_allocate_new();
  atomic_store32_release(&_memory_global_lock, 0);
  ಯದಿ (heap)
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  ಪ್ರತ್ಯಾಗಮ heap;
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_heap_release(ಶೂನ್ಯ *heapptr, ಪೂರ್ಣಾಙ್ಕ first_class,
                                   ಪೂರ್ಣಾಙ್ಕ release_cache) {
  heap_t *heap = (heap_t *)heapptr;
  ಯದಿ (!heap)
    ಪ್ರತ್ಯಾಗಮ;
  // Release thread cache spans back to global cache
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  ಯದಿ (release_cache || heap->finalize) {
#ಯದಿ ENABLE_THREAD_CACHE
    ಪ್ರತಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
      span_cache_t *span_cache;
      ಯದಿ (!iclass)
        span_cache = &heap->span_cache;
      ಅನ್ಯಥಾ
        span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
      ಯದಿ (!span_cache->ಎಣಿಕೆ)
        ಅನುವರ್ತ;
#ಯದಿ ENABLE_GLOBAL_CACHE
      ಯದಿ (heap->finalize) {
        ಪ್ರತಿ (size_t ispan = 0; ispan < span_cache->ಎಣಿಕೆ; ++ispan)
          _rpmalloc_span_unmap(span_cache->span[ispan]);
      } ಅನ್ಯಥಾ {
        _rpmalloc_stat_add64(&heap->thread_to_global, span_cache->ಎಣಿಕೆ *
                                                          (iclass + 1) *
                                                          _memory_span_size);
        _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                           span_cache->ಎಣಿಕೆ);
        rpmalloc_global_cache_ಸೇರಿಸು_spans(span_cache->span, iclass + 1,
                                            span_cache->ಎಣಿಕೆ);
      }
#ಅನ್ಯಥಾ
      ಪ್ರತಿ (size_t ispan = 0; ispan < span_cache->ಎಣಿಕೆ; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
      span_cache->ಎಣಿಕೆ = 0;
    }
#endif
  }

  ಯದಿ (get_thread_heap_raw() == heap)
    set_thread_heap(0);

#ಯದಿ ENABLE_STATISTICS
  atomic_decr32(&_memory_active_heaps);
  rpmalloc_assert(atomic_load32(&_memory_active_heaps) >= 0,
                  "Still active heaps during finalization");
#endif

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  ಯದಿ (get_thread_id() != _rpmalloc_main_thread_id) {
    ಯಾವತ್ (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
  }
  _rpmalloc_heap_orphan(heap, first_class);
  atomic_store32_release(&_memory_global_lock, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_heap_release_raw(ಶೂನ್ಯ *heapptr, ಪೂರ್ಣಾಙ್ಕ release_cache) {
  _rpmalloc_heap_release(heapptr, 0, release_cache);
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_heap_release_raw_fc(ಶೂನ್ಯ *heapptr) {
  _rpmalloc_heap_release_raw(heapptr, 1);
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_heap_finalize(heap_t *heap) {
  ಯದಿ (heap->spans_reserved) {
    span_t *span = _rpmalloc_span_map(heap, heap->spans_reserved);
    _rpmalloc_span_unmap(span);
    heap->spans_reserved = 0;
  }

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  ಪ್ರತಿ (size_t iclass = 0; iclass < SIZE_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಯದಿ (heap->size_class[iclass].cache)
      _rpmalloc_span_unmap(heap->size_class[iclass].cache);
    heap->size_class[iclass].cache = 0;
    span_t *span = heap->size_class[iclass].partial_span;
    ಯಾವತ್ (span) {
      span_t *next = span->next;
      _rpmalloc_span_finalize(heap, iclass, span,
                              &heap->size_class[iclass].partial_span);
      span = next;
    }
    // If class still has a free list it must be a full span
    ಯದಿ (heap->size_class[iclass].free_list) {
      span_t *class_span =
          (span_t *)((uintptr_t)heap->size_class[iclass].free_list &
                     _memory_span_mask);
      span_t **list = 0;
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
      list = &heap->full_span[iclass];
#endif
      --heap->full_span_ಎಣಿಕೆ;
      ಯದಿ (!_rpmalloc_span_finalize(heap, iclass, class_span, list)) {
        ಯದಿ (list)
          _rpmalloc_span_double_link_list_remove(list, class_span);
        _rpmalloc_span_double_link_list_add(
            &heap->size_class[iclass].partial_span, class_span);
      }
    }
  }

#ಯದಿ ENABLE_THREAD_CACHE
  ಪ್ರತಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    span_cache_t *span_cache;
    ಯದಿ (!iclass)
      span_cache = &heap->span_cache;
    ಅನ್ಯಥಾ
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    ಪ್ರತಿ (size_t ispan = 0; ispan < span_cache->ಎಣಿಕೆ; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->ಎಣಿಕೆ = 0;
  }
#endif
  rpmalloc_assert(!atomic_load_ptr(&heap->span_free_deferred),
                  "Heaps still active during finalization");
}

////////////
///
/// Allocation entry points
///
//////

//! Pop first block from a free list
ಸ್ಥಿರ ಶೂನ್ಯ *free_list_ತೆಗೆ(ಶೂನ್ಯ **list) {
  ಶೂನ್ಯ *block = *list;
  *list = *((ಶೂನ್ಯ **)block);
  ಪ್ರತ್ಯಾಗಮ block;
}

//! Allocate a small/medium sized memory block from the given heap
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_allocate_from_heap_fallback(
    heap_t *heap, heap_size_class_t *heap_size_class, uint32_t class_idx) {
  span_t *span = heap_size_class->partial_span;
  rpmalloc_assume(heap != 0);
  ಯದಿ (EXPECTED(span != 0)) {
    rpmalloc_assert(span->block_ಎಣಿಕೆ ==
                        _memory_size_class[span->size_class].block_ಎಣಿಕೆ,
                    "Span block count corrupted");
    rpmalloc_assert(!_rpmalloc_span_is_fully_utilized(span),
                    "Internal failure");
    ಶೂನ್ಯ *block;
    ಯದಿ (span->free_list) {
      // Span local free list is not empty, swap to size class free list
      block = free_list_ತೆಗೆ(&span->free_list);
      heap_size_class->free_list = span->free_list;
      span->free_list = 0;
    } ಅನ್ಯಥಾ {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      ಶೂನ್ಯ *block_start = pointer_offset(
          span, SPAN_HEADER_SIZE +
                    ((size_t)span->free_list_limit * span->block_size));
      span->free_list_limit += free_list_partial_init(
          &heap_size_class->free_list, &block,
          (ಶೂನ್ಯ *)((uintptr_t)block_start & ~(_memory_page_size - 1)),
          block_start, span->block_ಎಣಿಕೆ - span->free_list_limit,
          span->block_size);
    }
    rpmalloc_assert(span->free_list_limit <= span->block_ಎಣಿಕೆ,
                    "Span block count corrupted");
    span->used_ಎಣಿಕೆ = span->free_list_limit;

    // Swap in deferred free list if present
    ಯದಿ (atomic_load_ptr(&span->free_list_deferred))
      _rpmalloc_span_extract_free_list_deferred(span);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    ಯದಿ (!_rpmalloc_span_is_fully_utilized(span))
      ಪ್ರತ್ಯಾಗಮ block;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_span_double_link_list_ತೆಗೆ_head(&heap_size_class->partial_span,
                                             span);
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_ಎಣಿಕೆ;
    ಪ್ರತ್ಯಾಗಮ block;
  }

  // Find a span in one of the cache levels
  span = _rpmalloc_heap_extract_new_span(heap, heap_size_class, 1, class_idx);
  ಯದಿ (EXPECTED(span != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    ಪ್ರತ್ಯಾಗಮ _rpmalloc_span_initialize_new(heap, heap_size_class, span,
                                         class_idx);
  }

  ಪ್ರತ್ಯಾಗಮ 0;
}

//! Allocate a small sized memory block from the given heap
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_allocate_small(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Small sizes have unique size classes
  ಧ್ರುವ uint32_t class_idx =
      (uint32_t)((size + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_SHIFT);
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  ಯದಿ (EXPECTED(heap_size_class->free_list != 0))
    ಪ್ರತ್ಯಾಗಮ free_list_ತೆಗೆ(&heap_size_class->free_list);
  ಪ್ರತ್ಯಾಗಮ _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_allocate_medium(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ಧ್ರುವ uint32_t base_idx =
      (uint32_t)(SMALL_CLASS_ಎಣಿಕೆ +
                 ((size - (SMALL_SIZE_LIMIT + 1)) >> MEDIUM_GRANULARITY_SHIFT));
  ಧ್ರುವ uint32_t class_idx = _memory_size_class[base_idx].class_idx;
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  ಯದಿ (EXPECTED(heap_size_class->free_list != 0))
    ಪ್ರತ್ಯಾಗಮ free_list_ತೆಗೆ(&heap_size_class->free_list);
  ಪ್ರತ್ಯಾಗಮ _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_allocate_large(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  size += SPAN_HEADER_SIZE;
  size_t span_ಎಣಿಕೆ = size >> _memory_span_size_shift;
  ಯದಿ (size & (_memory_span_size - 1))
    ++span_ಎಣಿಕೆ;

  // Find a span in one of the cache levels
  span_t *span =
      _rpmalloc_heap_extract_new_span(heap, 0, span_ಎಣಿಕೆ, SIZE_CLASS_LARGE);
  ಯದಿ (!span)
    ಪ್ರತ್ಯಾಗಮ span;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(span->span_ಎಣಿಕೆ >= span_ಎಣಿಕೆ, "Internal failure");
  span->size_class = SIZE_CLASS_LARGE;
  span->heap = heap;

#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_ಎಣಿಕೆ;

  ಪ್ರತ್ಯಾಗಮ pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a huge block by mapping memory pages directly
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_allocate_huge(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  size += SPAN_HEADER_SIZE;
  size_t ಅಂಕೆ_pages = size >> _memory_page_size_shift;
  ಯದಿ (size & (_memory_page_size - 1))
    ++ಅಂಕೆ_pages;
  size_t align_offset = 0;
  span_t *span =
      (span_t *)_rpmalloc_mmap(ಅಂಕೆ_pages * _memory_page_size, &align_offset);
  ಯದಿ (!span)
    ಪ್ರತ್ಯಾಗಮ span;

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_ಎಣಿಕೆ = (uint32_t)ಅಂಕೆ_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, ಅಂಕೆ_pages, _huge_pages_peak);

#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_ಎಣಿಕೆ;

  ಪ್ರತ್ಯಾಗಮ pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a block of the given size
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_allocate(heap_t *heap, size_t size) {
  _rpmalloc_stat_add64(&_allocation_counter, 1);
  ಯದಿ (EXPECTED(size <= SMALL_SIZE_LIMIT))
    ಪ್ರತ್ಯಾಗಮ _rpmalloc_allocate_small(heap, size);
  ಅನ್ಯಥಾ ಯದಿ (size <= _memory_medium_size_limit)
    ಪ್ರತ್ಯಾಗಮ _rpmalloc_allocate_medium(heap, size);
  ಅನ್ಯಥಾ ಯದಿ (size <= LARGE_SIZE_LIMIT)
    ಪ್ರತ್ಯಾಗಮ _rpmalloc_allocate_large(heap, size);
  ಪ್ರತ್ಯಾಗಮ _rpmalloc_allocate_huge(heap, size);
}

ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_aligned_allocate(heap_t *heap, size_t alignment,
                                        size_t size) {
  ಯದಿ (alignment <= SMALL_GRANULARITY)
    ಪ್ರತ್ಯಾಗಮ _rpmalloc_allocate(heap, size);

#ಯದಿ ENABLE_VALIDATE_ARGS
  ಯದಿ ((size + alignment) < size) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ (alignment & (alignment - 1)) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif

  ಯದಿ ((alignment <= SPAN_HEADER_SIZE) &&
      ((size + SPAN_HEADER_SIZE) < _memory_medium_size_limit)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t multiple_size = size ? (size + (SPAN_HEADER_SIZE - 1)) &
                                      ~(uintptr_t)(SPAN_HEADER_SIZE - 1)
                                : SPAN_HEADER_SIZE;
    rpmalloc_assert(!(multiple_size % SPAN_HEADER_SIZE),
                    "Failed alignment calculation");
    ಯದಿ (multiple_size <= (size + alignment))
      ಪ್ರತ್ಯಾಗಮ _rpmalloc_allocate(heap, multiple_size);
  }

  ಶೂನ್ಯ *ptr = 0;
  size_t align_mask = alignment - 1;
  ಯದಿ (alignment <= _memory_page_size) {
    ptr = _rpmalloc_allocate(heap, size + alignment);
    ಯದಿ ((uintptr_t)ptr & align_mask) {
      ptr = (ಶೂನ್ಯ *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);
      // Mark as having aligned blocks
      span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
      span->flags |= SPAN_FLAG_ALIGNED_BLOCKS;
    }
    ಪ್ರತ್ಯಾಗಮ ptr;
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
  ಯದಿ (alignment & align_mask) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ (alignment >= _memory_span_size) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }

  size_t extra_pages = alignment / _memory_page_size;

  // Since each span has a header, we will at least need one extra memory page
  size_t ಅಂಕೆ_pages = 1 + (size / _memory_page_size);
  ಯದಿ (size & (_memory_page_size - 1))
    ++ಅಂಕೆ_pages;

  ಯದಿ (extra_pages > ಅಂಕೆ_pages)
    ಅಂಕೆ_pages = 1 + extra_pages;

  size_t original_pages = ಅಂಕೆ_pages;
  size_t limit_pages = (_memory_span_size / _memory_page_size) * 2;
  ಯದಿ (limit_pages < (original_pages * 2))
    limit_pages = original_pages * 2;

  size_t mapped_size, align_offset;
  span_t *span;

retry:
  align_offset = 0;
  mapped_size = ಅಂಕೆ_pages * _memory_page_size;

  span = (span_t *)_rpmalloc_mmap(mapped_size, &align_offset);
  ಯದಿ (!span) {
    errno = ENOMEM;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ptr = pointer_offset(span, SPAN_HEADER_SIZE);

  ಯದಿ ((uintptr_t)ptr & align_mask)
    ptr = (ಶೂನ್ಯ *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);

  ಯದಿ (((size_t)pointer_diff(ptr, span) >= _memory_span_size) ||
      (pointer_offset(ptr, size) > pointer_offset(span, mapped_size)) ||
      (((uintptr_t)ptr & _memory_span_mask) != (uintptr_t)span)) {
    _rpmalloc_unmap(span, mapped_size, align_offset, mapped_size);
    ++ಅಂಕೆ_pages;
    ಯದಿ (ಅಂಕೆ_pages > limit_pages) {
      errno = EINVAL;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಗಚ್ಛ retry;
  }

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_ಎಣಿಕೆ = (uint32_t)ಅಂಕೆ_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, ಅಂಕೆ_pages, _huge_pages_peak);

#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_ಎಣಿಕೆ;

  _rpmalloc_stat_add64(&_allocation_counter, 1);

  ಪ್ರತ್ಯಾಗಮ ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate_direct_small_or_medium(span_t *span,
                                                        ಶೂನ್ಯ *block) {
  heap_t *heap = span->heap;
  rpmalloc_assert(heap->owner_thread == get_thread_id() ||
                      !heap->owner_thread || heap->finalize,
                  "Internal failure");
  // Add block to free list
  ಯದಿ (UNEXPECTED(_rpmalloc_span_is_fully_utilized(span))) {
    span->used_ಎಣಿಕೆ = span->block_ಎಣಿಕೆ;
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                           span);
#endif
    _rpmalloc_span_double_link_list_add(
        &heap->size_class[span->size_class].partial_span, span);
    --heap->full_span_ಎಣಿಕೆ;
  }
  *((ಶೂನ್ಯ **)block) = span->free_list;
  --span->used_ಎಣಿಕೆ;
  span->free_list = block;
  ಯದಿ (UNEXPECTED(span->used_ಎಣಿಕೆ == span->list_size)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    ಯದಿ (span->used_ಎಣಿಕೆ) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      ಶೂನ್ಯ *free_list;
      ಕುರು {
        free_list = atomic_exchange_ptr_acquire(&span->free_list_deferred,
                                                INVALID_POINTER);
      } ಯಾವತ್ (free_list == INVALID_POINTER);
      atomic_store_ptr_release(&span->free_list_deferred, free_list);
    }
    _rpmalloc_span_double_link_list_remove(
        &heap->size_class[span->size_class].partial_span, span);
    _rpmalloc_span_release_to_cache(heap, span);
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate_defer_free_span(heap_t *heap, span_t *span) {
  ಯದಿ (span->size_class != SIZE_CLASS_HUGE)
    _rpmalloc_stat_inc(&heap->span_use[span->span_ಎಣಿಕೆ - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  ಕುರು {
    span->free_list = (ಶೂನ್ಯ *)atomic_load_ptr(&heap->span_free_deferred);
  } ಯಾವತ್ (!atomic_cas_ptr(&heap->span_free_deferred, span, span->free_list));
}

//! Put the block in the deferred free list of the owning span
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate_defer_small_or_medium(span_t *span,
                                                       ಶೂನ್ಯ *block) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  ಶೂನ್ಯ *free_list;
  ಕುರು {
    free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } ಯಾವತ್ (free_list == INVALID_POINTER);
  *((ಶೂನ್ಯ **)block) = free_list;
  uint32_t free_ಎಣಿಕೆ = ++span->list_size;
  ಪೂರ್ಣಾಙ್ಕ all_deferred_free = (free_ಎಣಿಕೆ == span->block_ಎಣಿಕೆ);
  atomic_store_ptr_release(&span->free_list_deferred, block);
  ಯದಿ (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate_small_or_medium(span_t *span, ಶೂನ್ಯ *p) {
  _rpmalloc_stat_inc_free(span->heap, span->size_class);
  ಯದಿ (span->flags & SPAN_FLAG_ALIGNED_BLOCKS) {
    // Realign pointer to block start
    ಶೂನ್ಯ *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
    p = pointer_offset(p, -(int32_t)(block_offset % span->block_size));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
  ಪೂರ್ಣಾಙ್ಕ defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#ಅನ್ಯಥಾ
  ಪೂರ್ಣಾಙ್ಕ defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  ಯದಿ (!defer)
    _rpmalloc_deallocate_direct_small_or_medium(span, p);
  ಅನ್ಯಥಾ
    _rpmalloc_deallocate_defer_small_or_medium(span, p);
}

//! Deallocate the given large memory block to the current heap
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate_large(span_t *span) {
  rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
  ಪೂರ್ಣಾಙ್ಕ defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#ಅನ್ಯಥಾ
  ಪೂರ್ಣಾಙ್ಕ defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  ಯದಿ (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    ಪ್ರತ್ಯಾಗಮ;
  }
  rpmalloc_assert(span->heap->full_span_ಎಣಿಕೆ, "Heap span counter corrupted");
  --span->heap->full_span_ಎಣಿಕೆ;
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif
#ಯದಿ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = span->span_ಎಣಿಕೆ - 1;
  atomic_decr32(&span->heap->span_use[idx].current);
#endif
  heap_t *heap = span->heap;
  rpmalloc_assert(heap, "No thread heap");
#ಯದಿ ENABLE_THREAD_CACHE
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ set_as_reserved =
      ((span->span_ಎಣಿಕೆ > 1) && (heap->span_cache.ಎಣಿಕೆ == 0) &&
       !heap->finalize && !heap->spans_reserved);
#ಅನ್ಯಥಾ
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ set_as_reserved =
      ((span->span_ಎಣಿಕೆ > 1) && !heap->finalize && !heap->spans_reserved);
#endif
  ಯದಿ (set_as_reserved) {
    heap->span_reserve = span;
    heap->spans_reserved = span->span_ಎಣಿಕೆ;
    ಯದಿ (span->flags & SPAN_FLAG_MASTER) {
      heap->span_reserve_master = span;
    } ಅನ್ಯಥಾ { // SPAN_FLAG_SUBSPAN
      span_t *master = (span_t *)pointer_offset(
          span,
          -(intptr_t)((size_t)span->offset_from_master * _memory_span_size));
      heap->span_reserve_master = master;
      rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->remaining_spans) >=
                          (int32_t)span->span_ಎಣಿಕೆ,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&heap->span_use[idx].spans_to_reserved);
  } ಅನ್ಯಥಾ {
    // Insert into cache list
    rpmalloc_heap_cache_ಸೇರಿಸು(heap, span);
  }
}

//! Deallocate the given huge span
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate_huge(span_t *span) {
  rpmalloc_assert(span->heap, "No span heap");
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
  ಪೂರ್ಣಾಙ್ಕ defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#ಅನ್ಯಥಾ
  ಪೂರ್ಣಾಙ್ಕ defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  ಯದಿ (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    ಪ್ರತ್ಯಾಗಮ;
  }
  rpmalloc_assert(span->heap->full_span_ಎಣಿಕೆ, "Heap span counter corrupted");
  --span->heap->full_span_ಎಣಿಕೆ;
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif

  // Oversized allocation, page count is stored in span_count
  size_t ಅಂಕೆ_pages = span->span_ಎಣಿಕೆ;
  _rpmalloc_unmap(span, ಅಂಕೆ_pages * _memory_page_size, span->align_offset,
                  ಅಂಕೆ_pages * _memory_page_size);
  _rpmalloc_stat_sub(&_huge_pages_current, ಅಂಕೆ_pages);
}

//! Deallocate the given block
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_deallocate(ಶೂನ್ಯ *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  ಯದಿ (UNEXPECTED(!span))
    ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ (EXPECTED(span->size_class < SIZE_CLASS_ಎಣಿಕೆ))
    _rpmalloc_deallocate_small_or_medium(span, p);
  ಅನ್ಯಥಾ ಯದಿ (span->size_class == SIZE_CLASS_LARGE)
    _rpmalloc_deallocate_large(span);
  ಅನ್ಯಥಾ
    _rpmalloc_deallocate_huge(span);
}

////////////
///
/// Reallocation entry points
///
//////

ಸ್ಥಿರ size_t _rpmalloc_usable_size(ಶೂನ್ಯ *p);

//! Reallocate the given block to the given size
ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_reallocate(heap_t *heap, ಶೂನ್ಯ *p, size_t size,
                                  size_t oldsize, unsigned ಪೂರ್ಣಾಙ್ಕ flags) {
  ಯದಿ (p) {
    // Grab the span using guaranteed span alignment
    span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
    ಯದಿ (EXPECTED(span->size_class < SIZE_CLASS_ಎಣಿಕೆ)) {
      // Small/medium sized block
      rpmalloc_assert(span->span_ಎಣಿಕೆ == 1, "Span counter corrupted");
      ಶೂನ್ಯ *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
      uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
      uint32_t block_idx = block_offset / span->block_size;
      ಶೂನ್ಯ *block =
          pointer_offset(blocks_start, (size_t)block_idx * span->block_size);
      ಯದಿ (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)span->block_size - pointer_diff(p, block));
      ಯದಿ ((size_t)span->block_size >= size) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ಯದಿ ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        ಪ್ರತ್ಯಾಗಮ block;
      }
    } ಅನ್ಯಥಾ ಯದಿ (span->size_class == SIZE_CLASS_LARGE) {
      // Large block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t ಅಂಕೆ_spans = total_size >> _memory_span_size_shift;
      ಯದಿ (total_size & (_memory_span_mask - 1))
        ++ಅಂಕೆ_spans;
      size_t current_spans = span->span_ಎಣಿಕೆ;
      ಶೂನ್ಯ *block = pointer_offset(span, SPAN_HEADER_SIZE);
      ಯದಿ (!oldsize)
        oldsize = (current_spans * _memory_span_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      ಯದಿ ((current_spans >= ಅಂಕೆ_spans) && (total_size >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ಯದಿ ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        ಪ್ರತ್ಯಾಗಮ block;
      }
    } ಅನ್ಯಥಾ {
      // Oversized block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t ಅಂಕೆ_pages = total_size >> _memory_page_size_shift;
      ಯದಿ (total_size & (_memory_page_size - 1))
        ++ಅಂಕೆ_pages;
      // Page count is stored in span_count
      size_t current_pages = span->span_ಎಣಿಕೆ;
      ಶೂನ್ಯ *block = pointer_offset(span, SPAN_HEADER_SIZE);
      ಯದಿ (!oldsize)
        oldsize = (current_pages * _memory_page_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      ಯದಿ ((current_pages >= ಅಂಕೆ_pages) && (ಅಂಕೆ_pages >= (current_pages / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ಯದಿ ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        ಪ್ರತ್ಯಾಗಮ block;
      }
    }
  } ಅನ್ಯಥಾ {
    oldsize = 0;
  }

  ಯದಿ (!!(flags & RPMALLOC_GROW_OR_FAIL))
    ಪ್ರತ್ಯಾಗಮ 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t lower_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_size =
      (size > lower_bound) ? size : ((size > oldsize) ? lower_bound : size);
  ಶೂನ್ಯ *block = _rpmalloc_allocate(heap, new_size);
  ಯದಿ (p && block) {
    ಯದಿ (!(flags & RPMALLOC_NO_PRESERVE))
      memcpy(block, p, oldsize < new_size ? oldsize : new_size);
    _rpmalloc_deallocate(p);
  }

  ಪ್ರತ್ಯಾಗಮ block;
}

ಸ್ಥಿರ ಶೂನ್ಯ *_rpmalloc_aligned_reallocate(heap_t *heap, ಶೂನ್ಯ *ptr,
                                          size_t alignment, size_t size,
                                          size_t oldsize, unsigned ಪೂರ್ಣಾಙ್ಕ flags) {
  ಯದಿ (alignment <= SMALL_GRANULARITY)
    ಪ್ರತ್ಯಾಗಮ _rpmalloc_reallocate(heap, ptr, size, oldsize, flags);

  ಪೂರ್ಣಾಙ್ಕ no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? _rpmalloc_usable_size(ptr) : 0);
  ಯದಿ ((usablesize >= size) && !((uintptr_t)ptr & (alignment - 1))) {
    ಯದಿ (no_alloc || (size >= (usablesize / 2)))
      ಪ್ರತ್ಯಾಗಮ ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  ಶೂನ್ಯ *block =
      (!no_alloc ? _rpmalloc_aligned_allocate(heap, alignment, size) : 0);
  ಯದಿ (EXPECTED(block != 0)) {
    ಯದಿ (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      ಯದಿ (!oldsize)
        oldsize = usablesize;
      memcpy(block, ptr, oldsize < size ? oldsize : size);
    }
    _rpmalloc_deallocate(ptr);
  }
  ಪ್ರತ್ಯಾಗಮ block;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
ಸ್ಥಿರ size_t _rpmalloc_usable_size(ಶೂನ್ಯ *p) {
  // Grab the span using guaranteed span alignment
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  ಯದಿ (span->size_class < SIZE_CLASS_ಎಣಿಕೆ) {
    // Small/medium block
    ಶೂನ್ಯ *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    ಪ್ರತ್ಯಾಗಮ span->block_size -
           ((size_t)pointer_diff(p, blocks_start) % span->block_size);
  }
  ಯದಿ (span->size_class == SIZE_CLASS_LARGE) {
    // Large block
    size_t current_spans = span->span_ಎಣಿಕೆ;
    ಪ್ರತ್ಯಾಗಮ (current_spans * _memory_span_size) - (size_t)pointer_diff(p, span);
  }
  // Oversized block, page count is stored in span_count
  size_t current_pages = span->span_ಎಣಿಕೆ;
  ಪ್ರತ್ಯಾಗಮ (current_pages * _memory_page_size) - (size_t)pointer_diff(p, span);
}

//! Adjust and optimize the size class properties for the given class
ಸ್ಥಿರ ಶೂನ್ಯ _rpmalloc_adjust_size_class(size_t iclass) {
  size_t block_size = _memory_size_class[iclass].block_size;
  size_t block_ಎಣಿಕೆ = (_memory_span_size - SPAN_HEADER_SIZE) / block_size;

  _memory_size_class[iclass].block_ಎಣಿಕೆ = (uint16_t)block_ಎಣಿಕೆ;
  _memory_size_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  ಯದಿ (iclass >= SMALL_CLASS_ಎಣಿಕೆ) {
    size_t prevclass = iclass;
    ಯಾವತ್ (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      ಯದಿ (_memory_size_class[prevclass].block_ಎಣಿಕೆ ==
          _memory_size_class[iclass].block_ಎಣಿಕೆ)
        _rpmalloc_memcpy_const(_memory_size_class + prevclass,
                               _memory_size_class + iclass,
                               ಪರಿಮಾಣ(_memory_size_class[iclass]));
      ಅನ್ಯಥಾ
        ಭಙ್ಗ;
    }
  }
}

//! Initialize the allocator and setup global data
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ rpmalloc_initialize(ಶೂನ್ಯ) {
  ಯದಿ (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತ್ಯಾಗಮ rpmalloc_initialize_config(0);
}

ಪೂರ್ಣಾಙ್ಕ rpmalloc_initialize_config(ಧ್ರುವ rpmalloc_config_t *config) {
  ಯದಿ (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  _rpmalloc_initialized = 1;

  ಯದಿ (config)
    memcpy(&_memory_config, config, ಪರಿಮಾಣ(rpmalloc_config_t));
  ಅನ್ಯಥಾ
    _rpmalloc_memset_const(&_memory_config, 0, ಪರಿಮಾಣ(rpmalloc_config_t));

  ಯದಿ (!_memory_config.memory_map || !_memory_config.memory_unmap) {
    _memory_config.memory_map = _rpmalloc_mmap_os;
    _memory_config.memory_unmap = _rpmalloc_unmap_os;
  }

#ಯದಿ PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, ಪರಿಮಾಣ(system_info));
  GetSystemInfo(&system_info);
  _memory_map_granularity = system_info.dwAllocationGranularity;
#ಅನ್ಯಥಾ
  _memory_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#endif

#ಯದಿ RPMALLOC_CONFIGURABLE
  _memory_page_size = _memory_config.page_size;
#ಅನ್ಯಥಾ
  _memory_page_size = 0;
#endif
  _memory_huge_pages = 0;
  ಯದಿ (!_memory_page_size) {
#ಯದಿ PLATFORM_WINDOWS
    _memory_page_size = system_info.dwPageSize;
#ಅನ್ಯಥಾ
    _memory_page_size = _memory_map_granularity;
    ಯದಿ (_memory_config.enable_huge_pages) {
#ಯದಿ defined(__linux__)
      size_t huge_page_size = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      ಯದಿ (meminfo) {
        ವರ್ಣ ಸಾಲು_2[128];
        ಯಾವತ್ (!huge_page_size && fgets(ಸಾಲು_2, ಪರಿಮಾಣ(ಸಾಲು_2) - 1, meminfo)) {
          ಸಾಲು_2[ಪರಿಮಾಣ(ಸಾಲು_2) - 1] = 0;
          ಯದಿ (strstr(ಸಾಲು_2, "Hugepagesize:"))
            huge_page_size = (size_t)strtol(ಸಾಲು_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      ಯದಿ (huge_page_size) {
        _memory_huge_pages = 1;
        _memory_page_size = huge_page_size;
        _memory_map_granularity = huge_page_size;
      }
#ನೋಚೇದ್ಯದಿ defined(__FreeBSD__)
      ಪೂರ್ಣಾಙ್ಕ rc;
      size_t sz = ಪರಿಮಾಣ(rc);

      ಯದಿ (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, ರಿಕ್ತ, 0) == 0 &&
          rc == 1) {
        ಸ್ಥಿರ size_t defsize = 2 * 1024 * 1024;
        ಪೂರ್ಣಾಙ್ಕ nsize = 0;
        size_t sizes[4] = {0};
        _memory_huge_pages = 1;
        _memory_page_size = defsize;
        ಯದಿ ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          ಪ್ರತಿ (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            ಯದಿ (defsize < csize) {
              _memory_page_size = csize;
              ಭಙ್ಗ;
            }
          }
        }
        _memory_map_granularity = _memory_page_size;
      }
#ನೋಚೇದ್ಯದಿ defined(__APPLE__) || defined(__NetBSD__)
      _memory_huge_pages = 1;
      _memory_page_size = 2 * 1024 * 1024;
      _memory_map_granularity = _memory_page_size;
#endif
    }
#endif
  } ಅನ್ಯಥಾ {
    ಯದಿ (_memory_config.enable_huge_pages)
      _memory_huge_pages = 1;
  }

#ಯದಿ PLATFORM_WINDOWS
  ಯದಿ (_memory_config.enable_huge_pages) {
    HANDLE token = 0;
    size_t large_page_minimum = GetLargePageMinimum();
    ಯದಿ (large_page_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    ಯದಿ (token) {
      LUID luid;
      ಯದಿ (Lookup_Privilege_ಮೌಲ್ಯ(0, SE_LOCK_MEMORY_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, ಪರಿಮಾಣ(token_privileges));
        token_privileges.Privilege_ಎಣಿಕೆ = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        ಯದಿ (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          ಯದಿ (GetLastError() == ERROR_SUCCESS)
            _memory_huge_pages = 1;
        }
      }
      CloseHandle(token);
    }
    ಯದಿ (_memory_huge_pages) {
      ಯದಿ (large_page_minimum > _memory_page_size)
        _memory_page_size = large_page_minimum;
      ಯದಿ (large_page_minimum > _memory_map_granularity)
        _memory_map_granularity = large_page_minimum;
    }
  }
#endif

  size_t min_span_size = 256;
  size_t max_page_size;
#ಯದಿ UINTPTR_MAX > 0xFFFFFFFF
  max_page_size = 4096ULL * 1024ULL * 1024ULL;
#ಅನ್ಯಥಾ
  max_page_size = 4 * 1024 * 1024;
#endif
  ಯದಿ (_memory_page_size < min_span_size)
    _memory_page_size = min_span_size;
  ಯದಿ (_memory_page_size > max_page_size)
    _memory_page_size = max_page_size;
  _memory_page_size_shift = 0;
  size_t page_size_bit = _memory_page_size;
  ಯಾವತ್ (page_size_bit != 1) {
    ++_memory_page_size_shift;
    page_size_bit >>= 1;
  }
  _memory_page_size = ((size_t)1 << _memory_page_size_shift);

#ಯದಿ RPMALLOC_CONFIGURABLE
  ಯದಿ (!_memory_config.span_size) {
    _memory_span_size = _memory_default_span_size;
    _memory_span_size_shift = _memory_default_span_size_shift;
    _memory_span_mask = _memory_default_span_mask;
  } ಅನ್ಯಥಾ {
    size_t span_size = _memory_config.span_size;
    ಯದಿ (span_size > (256 * 1024))
      span_size = (256 * 1024);
    _memory_span_size = 4096;
    _memory_span_size_shift = 12;
    ಯಾವತ್ (_memory_span_size < span_size) {
      _memory_span_size <<= 1;
      ++_memory_span_size_shift;
    }
    _memory_span_mask = ~(uintptr_t)(_memory_span_size - 1);
  }
#endif

  memory_span_map_ಎಣಿಕೆ =
      (_memory_config.span_map_ಎಣಿಕೆ ? _memory_config.span_map_ಎಣಿಕೆ
                                     : DEFAULT_SPAN_MAP_ಎಣಿಕೆ);
  ಯದಿ ((_memory_span_size * memory_span_map_ಎಣಿಕೆ) < _memory_page_size)
    memory_span_map_ಎಣಿಕೆ = (_memory_page_size / _memory_span_size);
  ಯದಿ ((_memory_page_size >= _memory_span_size) &&
      ((memory_span_map_ಎಣಿಕೆ * _memory_span_size) % _memory_page_size))
    memory_span_map_ಎಣಿಕೆ = (_memory_page_size / _memory_span_size);
  memory_heap_reserve_ಎಣಿಕೆ = (memory_span_map_ಎಣಿಕೆ > DEFAULT_SPAN_MAP_ಎಣಿಕೆ)
                                   ? DEFAULT_SPAN_MAP_ಎಣಿಕೆ
                                   : memory_span_map_ಎಣಿಕೆ;

  _memory_config.page_size = _memory_page_size;
  _memory_config.span_size = _memory_span_size;
  _memory_config.span_map_ಎಣಿಕೆ = memory_span_map_ಎಣಿಕೆ;
  _memory_config.enable_huge_pages = _memory_huge_pages;

#ಯದಿ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  ಯದಿ (pthread_ಕೀಲಿ_create(&_memory_thread_heap, _rpmalloc_heap_release_raw_fc))
    ಪ್ರತ್ಯಾಗಮ -1;
#endif
#ಯದಿ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_ಕೀಲಿ = FlsAlloc(&_rpmalloc_thread_destructor);
#endif

  // Setup all small and medium size classes
  size_t iclass = 0;
  _memory_size_class[iclass].block_size = SMALL_GRANULARITY;
  _rpmalloc_adjust_size_class(iclass);
  ಪ್ರತಿ (iclass = 1; iclass < SMALL_CLASS_ಎಣಿಕೆ; ++iclass) {
    size_t size = iclass * SMALL_GRANULARITY;
    _memory_size_class[iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  _memory_medium_size_limit = (_memory_span_size - SPAN_HEADER_SIZE) >> 1;
  ಯದಿ (_memory_medium_size_limit > MEDIUM_SIZE_LIMIT)
    _memory_medium_size_limit = MEDIUM_SIZE_LIMIT;
  ಪ್ರತಿ (iclass = 0; iclass < MEDIUM_CLASS_ಎಣಿಕೆ; ++iclass) {
    size_t size = SMALL_SIZE_LIMIT + ((iclass + 1) * MEDIUM_GRANULARITY);
    ಯದಿ (size > _memory_medium_size_limit) {
      _memory_medium_size_limit =
          SMALL_SIZE_LIMIT + (iclass * MEDIUM_GRANULARITY);
      ಭಙ್ಗ;
    }
    _memory_size_class[SMALL_CLASS_ಎಣಿಕೆ + iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(SMALL_CLASS_ಎಣಿಕೆ + iclass);
  }

  _memory_orphan_heaps = 0;
#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS
  _memory_first_class_orphan_heaps = 0;
#endif
#ಯದಿ ENABLE_STATISTICS
  atomic_store32(&_memory_active_heaps, 0);
  atomic_store32(&_mapped_pages, 0);
  _mapped_pages_peak = 0;
  atomic_store32(&_master_spans, 0);
  atomic_store32(&_mapped_total, 0);
  atomic_store32(&_unmapped_total, 0);
  atomic_store32(&_mapped_pages_os, 0);
  atomic_store32(&_huge_pages_current, 0);
  _huge_pages_peak = 0;
#endif
  memset(_memory_heaps, 0, ಪರಿಮಾಣ(_memory_heaps));
  atomic_store32_release(&_memory_global_lock, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  ಪ್ರತ್ಯಾಗಮ 0;
}

//! Finalize the allocator
ಶೂನ್ಯ rpmalloc_finalize(ಶೂನ್ಯ) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  ಯದಿ (_memory_global_reserve) {
    atomic_add32(&_memory_global_reserve_master->remaining_spans,
                 -(int32_t)memory_global_reserve_ಎಣಿಕೆ);
    _memory_global_reserve_master = 0;
    memory_global_reserve_ಎಣಿಕೆ = 0;
    _memory_global_reserve = 0;
  }
  atomic_store32_release(&_memory_global_lock, 0);

  // Free all thread caches and fully free spans
  ಪ್ರತಿ (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    ಯಾವತ್ (heap) {
      heap_t *next_heap = heap->next_heap;
      heap->finalize = 1;
      _rpmalloc_heap_global_finalize(heap);
      heap = next_heap;
    }
  }

#ಯದಿ ENABLE_GLOBAL_CACHE
  // Free global caches
  ಪ್ರತಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass)
    _rpmalloc_global_cache_finalize(&_memory_span_cache[iclass]);
#endif

#ಯದಿ (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_ಕೀಲಿ_delete(_memory_thread_heap);
#endif
#ಯದಿ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_ಕೀಲಿ);
  fls_ಕೀಲಿ = 0;
#endif
#ಯದಿ ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&_mapped_pages) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&_mapped_pages_os) == 0,
                  "Memory leak detected");
#endif

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ rpmalloc_thread_initialize(ಶೂನ್ಯ) {
  ಯದಿ (!get_thread_heap_raw()) {
    heap_t *heap = _rpmalloc_heap_allocate(0);
    ಯದಿ (heap) {
      _rpmalloc_stat_inc(&_memory_active_heaps);
      set_thread_heap(heap);
#ಯದಿ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_Set_ಮೌಲ್ಯ(fls_ಕೀಲಿ, heap);
#endif
    }
  }
}

//! Finalize thread, orphan heap
ಶೂನ್ಯ rpmalloc_thread_finalize(ಪೂರ್ಣಾಙ್ಕ release_caches) {
  heap_t *heap = get_thread_heap_raw();
  ಯದಿ (heap)
    _rpmalloc_heap_release_raw(heap, release_caches);
  set_thread_heap(0);
#ಯದಿ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_Set_ಮೌಲ್ಯ(fls_ಕೀಲಿ, 0);
#endif
}

ಪೂರ್ಣಾಙ್ಕ rpmalloc_is_thread_initialized(ಶೂನ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ (get_thread_heap_raw() != 0) ? 1 : 0;
}

ಧ್ರುವ rpmalloc_config_t *rpmalloc_config(ಶೂನ್ಯ) { ಪ್ರತ್ಯಾಗಮ &_memory_config; }

// Extern interface

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *rpmalloc(size_t size) {
#ಯದಿ ENABLE_VALIDATE_ARGS
  ಯದಿ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  ಪ್ರತ್ಯಾಗಮ _rpmalloc_allocate(heap, size);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ rpfree(ಶೂನ್ಯ *ptr) { _rpmalloc_deallocate(ptr); }

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *rpcalloc(size_t ಅಂಕೆ_2, size_t size) {
  size_t total;
#ಯದಿ ENABLE_VALIDATE_ARGS
#ಯದಿ PLATFORM_WINDOWS
  ಪೂರ್ಣಾಙ್ಕ err = SizeTMult(ಅಂಕೆ_2, size, &total);
  ಯದಿ ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#ಅನ್ಯಥಾ
  ಪೂರ್ಣಾಙ್ಕ err = __builtin_umull_overflow(ಅಂಕೆ_2, size, &total);
  ಯದಿ (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
#ಅನ್ಯಥಾ
  total = ಅಂಕೆ_2 * size;
#endif
  heap_t *heap = get_thread_heap();
  ಶೂನ್ಯ *block = _rpmalloc_allocate(heap, total);
  ಯದಿ (block)
    memset(block, 0, total);
  ಪ್ರತ್ಯಾಗಮ block;
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *rprealloc(ಶೂನ್ಯ *ptr, size_t size) {
#ಯದಿ ENABLE_VALIDATE_ARGS
  ಯದಿ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ ptr;
  }
#endif
  heap_t *heap = get_thread_heap();
  ಪ್ರತ್ಯಾಗಮ _rpmalloc_reallocate(heap, ptr, size, 0, 0);
}

ಬಾಹ್ಯ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *rpaligned_realloc(ಶೂನ್ಯ *ptr, size_t alignment,
                                                  size_t size, size_t oldsize,
                                                  unsigned ಪೂರ್ಣಾಙ್ಕ flags) {
#ಯದಿ ENABLE_VALIDATE_ARGS
  ಯದಿ ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  ಪ್ರತ್ಯಾಗಮ _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, oldsize,
                                      flags);
}

ಬಾಹ್ಯ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *rpaligned_alloc(size_t alignment, size_t size) {
  heap_t *heap = get_thread_heap();
  ಪ್ರತ್ಯಾಗಮ _rpmalloc_aligned_allocate(heap, alignment, size);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpaligned_calloc(size_t alignment, size_t ಅಂಕೆ_2, size_t size) {
  size_t total;
#ಯದಿ ENABLE_VALIDATE_ARGS
#ಯದಿ PLATFORM_WINDOWS
  ಪೂರ್ಣಾಙ್ಕ err = SizeTMult(ಅಂಕೆ_2, size, &total);
  ಯದಿ ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#ಅನ್ಯಥಾ
  ಪೂರ್ಣಾಙ್ಕ err = __builtin_umull_overflow(ಅಂಕೆ_2, size, &total);
  ಯದಿ (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
#ಅನ್ಯಥಾ
  total = ಅಂಕೆ_2 * size;
#endif
  ಶೂನ್ಯ *block = rpaligned_alloc(alignment, total);
  ಯದಿ (block)
    memset(block, 0, total);
  ಪ್ರತ್ಯಾಗಮ block;
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *rpmemalign(size_t alignment,
                                                  size_t size) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(alignment, size);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ rpposix_memalign(ಶೂನ್ಯ **memptr, size_t alignment,
                                   size_t size) {
  ಯದಿ (memptr)
    *memptr = rpaligned_alloc(alignment, size);
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ EINVAL;
  ಪ್ರತ್ಯಾಗಮ *memptr ? 0 : ENOMEM;
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ size_t rpmalloc_usable_size(ಶೂನ್ಯ *ptr) {
  ಪ್ರತ್ಯಾಗಮ (ptr ? _rpmalloc_usable_size(ptr) : 0);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ rpmalloc_thread_collect(ಶೂನ್ಯ) {}

ಶೂನ್ಯ rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, ಪರಿಮಾಣ(rpmalloc_thread_statistics_t));
  heap_t *heap = get_thread_heap_raw();
  ಯದಿ (!heap)
    ಪ್ರತ್ಯಾಗಮ;

  ಪ್ರತಿ (size_t iclass = 0; iclass < SIZE_CLASS_ಎಣಿಕೆ; ++iclass) {
    size_class_t *size_class = _memory_size_class + iclass;
    span_t *span = heap->size_class[iclass].partial_span;
    ಯಾವತ್ (span) {
      size_t free_ಎಣಿಕೆ = span->list_size;
      size_t block_ಎಣಿಕೆ = size_class->block_ಎಣಿಕೆ;
      ಯದಿ (span->free_list_limit < block_ಎಣಿಕೆ)
        block_ಎಣಿಕೆ = span->free_list_limit;
      free_ಎಣಿಕೆ += (block_ಎಣಿಕೆ - span->used_ಎಣಿಕೆ);
      stats->sizecache += free_ಎಣಿಕೆ * size_class->block_size;
      span = span->next;
    }
  }

#ಯದಿ ENABLE_THREAD_CACHE
  ಪ್ರತಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    span_cache_t *span_cache;
    ಯದಿ (!iclass)
      span_cache = &heap->span_cache;
    ಅನ್ಯಥಾ
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    stats->spancache += span_cache->ಎಣಿಕೆ * (iclass + 1) * _memory_span_size;
  }
#endif

  span_t *deferred = (span_t *)atomic_load_ptr(&heap->span_free_deferred);
  ಯಾವತ್ (deferred) {
    ಯದಿ (deferred->size_class != SIZE_CLASS_HUGE)
      stats->spancache += (size_t)deferred->span_ಎಣಿಕೆ * _memory_span_size;
    deferred = (span_t *)deferred->free_list;
  }

#ಯದಿ ENABLE_STATISTICS
  stats->thread_to_global = (size_t)atomic_load64(&heap->thread_to_global);
  stats->global_to_thread = (size_t)atomic_load64(&heap->global_to_thread);

  ಪ್ರತಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    stats->span_use[iclass].current =
        (size_t)atomic_load32(&heap->span_use[iclass].current);
    stats->span_use[iclass].peak =
        (size_t)atomic_load32(&heap->span_use[iclass].ಮೇಲಿನ);
    stats->span_use[iclass].to_global =
        (size_t)atomic_load32(&heap->span_use[iclass].spans_to_global);
    stats->span_use[iclass].from_global =
        (size_t)atomic_load32(&heap->span_use[iclass].spans_from_global);
    stats->span_use[iclass].to_cache =
        (size_t)atomic_load32(&heap->span_use[iclass].spans_to_cache);
    stats->span_use[iclass].from_cache =
        (size_t)atomic_load32(&heap->span_use[iclass].spans_from_cache);
    stats->span_use[iclass].to_reserved =
        (size_t)atomic_load32(&heap->span_use[iclass].spans_to_reserved);
    stats->span_use[iclass].from_reserved =
        (size_t)atomic_load32(&heap->span_use[iclass].spans_from_reserved);
    stats->span_use[iclass].map_calls =
        (size_t)atomic_load32(&heap->span_use[iclass].spans_map_calls);
  }
  ಪ್ರತಿ (size_t iclass = 0; iclass < SIZE_CLASS_ಎಣಿಕೆ; ++iclass) {
    stats->size_use[iclass].alloc_current =
        (size_t)atomic_load32(&heap->size_class_use[iclass].alloc_current);
    stats->size_use[iclass].alloc_peak =
        (size_t)heap->size_class_use[iclass].alloc_peak;
    stats->size_use[iclass].alloc_total =
        (size_t)atomic_load32(&heap->size_class_use[iclass].alloc_total);
    stats->size_use[iclass].free_total =
        (size_t)atomic_load32(&heap->size_class_use[iclass].free_total);
    stats->size_use[iclass].spans_to_cache =
        (size_t)atomic_load32(&heap->size_class_use[iclass].spans_to_cache);
    stats->size_use[iclass].spans_from_cache =
        (size_t)atomic_load32(&heap->size_class_use[iclass].spans_from_cache);
    stats->size_use[iclass].spans_from_reserved = (size_t)atomic_load32(
        &heap->size_class_use[iclass].spans_from_reserved);
    stats->size_use[iclass].map_calls =
        (size_t)atomic_load32(&heap->size_class_use[iclass].spans_map_calls);
  }
#endif
}

ಶೂನ್ಯ rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, ಪರಿಮಾಣ(rpmalloc_global_statistics_t));
#ಯದಿ ENABLE_STATISTICS
  stats->mapped = (size_t)atomic_load32(&_mapped_pages) * _memory_page_size;
  stats->mapped_peak = (size_t)_mapped_pages_peak * _memory_page_size;
  stats->mapped_total =
      (size_t)atomic_load32(&_mapped_total) * _memory_page_size;
  stats->unmapped_total =
      (size_t)atomic_load32(&_unmapped_total) * _memory_page_size;
  stats->huge_alloc =
      (size_t)atomic_load32(&_huge_pages_current) * _memory_page_size;
  stats->huge_alloc_peak = (size_t)_huge_pages_peak * _memory_page_size;
#endif
#ಯದಿ ENABLE_GLOBAL_CACHE
  ಪ್ರತಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    global_cache_t *cache = &_memory_span_cache[iclass];
    ಯಾವತ್ (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();
    uint32_t ಎಣಿಕೆ = cache->ಎಣಿಕೆ;
#ಯದಿ ENABLE_UNLIMITED_CACHE
    span_t *current_span = cache->overflow;
    ಯಾವತ್ (current_span) {
      ++ಎಣಿಕೆ;
      current_span = current_span->next;
    }
#endif
    atomic_store32_release(&cache->lock, 0);
    stats->cached += ಎಣಿಕೆ * (iclass + 1) * _memory_span_size;
  }
#endif
}

#ಯದಿ ENABLE_STATISTICS

ಸ್ಥಿರ ಶೂನ್ಯ _memory_heap_dump_statistics(heap_t *heap, ಶೂನ್ಯ *file) {
  fprintf(file, "Heap %d stats:\n", heap->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  ಪ್ರತಿ (size_t iclass = 0; iclass < SIZE_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಯದಿ (!atomic_load32(&heap->size_class_use[iclass].alloc_total))
      ಅನುವರ್ತ;
    fprintf(
        file,
        "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu "
        "%9u\n",
        (uint32_t)iclass,
        atomic_load32(&heap->size_class_use[iclass].alloc_current),
        heap->size_class_use[iclass].alloc_peak,
        atomic_load32(&heap->size_class_use[iclass].alloc_total),
        atomic_load32(&heap->size_class_use[iclass].free_total),
        _memory_size_class[iclass].block_size,
        _memory_size_class[iclass].block_ಎಣಿಕೆ,
        atomic_load32(&heap->size_class_use[iclass].spans_current),
        heap->size_class_use[iclass].spans_peak,
        ((size_t)heap->size_class_use[iclass].alloc_peak *
         (size_t)_memory_size_class[iclass].block_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->size_class_use[iclass].spans_to_cache) *
         _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->size_class_use[iclass].spans_from_cache) *
         _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(
             &heap->size_class_use[iclass].spans_from_reserved) *
         _memory_span_size) /
            (size_t)(1024 * 1024),
        atomic_load32(&heap->size_class_use[iclass].spans_map_calls));
  }
  fprintf(file, "Spans  Current     Peak Deferred  PeakMiB  Cached  ToCacheMiB "
                "FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB "
                "FromGlobalMiB  MmapCalls\n");
  ಪ್ರತಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    ಯದಿ (!atomic_load32(&heap->span_use[iclass].ಮೇಲಿನ) &&
        !atomic_load32(&heap->span_use[iclass].spans_map_calls))
      ಅನುವರ್ತ;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&heap->span_use[iclass].current),
        atomic_load32(&heap->span_use[iclass].ಮೇಲಿನ),
        atomic_load32(&heap->span_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&heap->span_use[iclass].ಮೇಲಿನ) *
         (size_t)_memory_span_size * (iclass + 1)) /
            (size_t)(1024 * 1024),
#ಯದಿ ENABLE_THREAD_CACHE
        (unsigned ಪೂರ್ಣಾಙ್ಕ)(!iclass ? heap->span_cache.ಎಣಿಕೆ
                               : heap->span_large_cache[iclass - 1].ಎಣಿಕೆ),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_to_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_from_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
#ಅನ್ಯಥಾ
        0, (size_t)0, (size_t)0,
#endif
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_to_reserved) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_from_reserved) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_to_global) *
         (size_t)_memory_span_size * (iclass + 1)) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_from_global) *
         (size_t)_memory_span_size * (iclass + 1)) /
            (size_t)(1024 * 1024),
        atomic_load32(&heap->span_use[iclass].spans_map_calls));
  }
  fprintf(file, "Full spans: %zu\n", heap->full_span_ಎಣಿಕೆ);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&heap->thread_to_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&heap->global_to_thread) / (size_t)(1024 * 1024));
}

#endif

ಶೂನ್ಯ rpmalloc_dump_statistics(ಶೂನ್ಯ *file) {
#ಯದಿ ENABLE_STATISTICS
  ಪ್ರತಿ (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    ಯಾವತ್ (heap) {
      ಪೂರ್ಣಾಙ್ಕ need_dump = 0;
      ಪ್ರತಿ (size_t iclass = 0; !need_dump && (iclass < SIZE_CLASS_ಎಣಿಕೆ);
           ++iclass) {
        ಯದಿ (!atomic_load32(&heap->size_class_use[iclass].alloc_total)) {
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].free_total),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          ಅನುವರ್ತ;
        }
        need_dump = 1;
      }
      ಪ್ರತಿ (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_ಎಣಿಕೆ);
           ++iclass) {
        ಯದಿ (!atomic_load32(&heap->span_use[iclass].ಮೇಲಿನ) &&
            !atomic_load32(&heap->span_use[iclass].spans_map_calls))
          ಅನುವರ್ತ;
        need_dump = 1;
      }
      ಯದಿ (need_dump)
        _memory_heap_dump_statistics(heap, file);
      heap = heap->next_heap;
    }
  }
  fprintf(file, "Global stats:\n");
  size_t huge_current =
      (size_t)atomic_load32(&_huge_pages_current) * _memory_page_size;
  size_t huge_peak = (size_t)_huge_pages_peak * _memory_page_size;
  fprintf(file, "HugeCurrentMiB HugePeakMiB\n");
  fprintf(file, "%14zu %11zu\n", huge_current / (size_t)(1024 * 1024),
          huge_peak / (size_t)(1024 * 1024));

#ಯದಿ ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  ಪ್ರತಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    global_cache_t *cache = _memory_span_cache + iclass;
    size_t global_cache = (size_t)cache->ಎಣಿಕೆ * iclass * _memory_span_size;

    size_t global_overflow_cache = 0;
    span_t *span = cache->overflow;
    ಯಾವತ್ (span) {
      global_overflow_cache += iclass * _memory_span_size;
      span = span->next;
    }
    ಯದಿ (global_cache || global_overflow_cache || cache->ಸೇರಿಸು_ಎಣಿಕೆ ||
        cache->extract_ಎಣಿಕೆ)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_overflow_cache / (size_t)(1024 * 1024),
              cache->ಸೇರಿಸು_ಎಣಿಕೆ, cache->extract_ಎಣಿಕೆ);
  }
#endif

  size_t mapped = (size_t)atomic_load32(&_mapped_pages) * _memory_page_size;
  size_t mapped_os =
      (size_t)atomic_load32(&_mapped_pages_os) * _memory_page_size;
  size_t mapped_peak = (size_t)_mapped_pages_peak * _memory_page_size;
  size_t mapped_total =
      (size_t)atomic_load32(&_mapped_total) * _memory_page_size;
  size_t unmapped_total =
      (size_t)atomic_load32(&_unmapped_total) * _memory_page_size;
  fprintf(
      file,
      "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB\n");
  fprintf(file, "%9zu %11zu %13zu %14zu %16zu\n",
          mapped / (size_t)(1024 * 1024), mapped_os / (size_t)(1024 * 1024),
          mapped_peak / (size_t)(1024 * 1024),
          mapped_total / (size_t)(1024 * 1024),
          unmapped_total / (size_t)(1024 * 1024));

  fprintf(file, "\n");
#ಯದಿ 0
	int64_t allocated = atomic_load64(&_allocation_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#endif
#endif
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(file);
}

#ಯದಿ RPMALLOC_FIRST_CLASS_HEAPS

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ rpmalloc_heap_t *rpmalloc_heap_acquire(ಶೂನ್ಯ) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  heap_t *heap = _rpmalloc_heap_allocate(1);
  rpmalloc_assume(heap != ರಿಕ್ತ);
  heap->owner_thread = 0;
  _rpmalloc_stat_inc(&_memory_active_heaps);
  ಪ್ರತ್ಯಾಗಮ heap;
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ rpmalloc_heap_release(rpmalloc_heap_t *heap) {
  ಯದಿ (heap)
    _rpmalloc_heap_release(heap, 1, 1);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, size_t size) {
#ಯದಿ ENABLE_VALIDATE_ARGS
  ಯದಿ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
  ಪ್ರತ್ಯಾಗಮ _rpmalloc_allocate(heap, size);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, size_t alignment,
                            size_t size) {
#ಯದಿ ENABLE_VALIDATE_ARGS
  ಯದಿ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
  ಪ್ರತ್ಯಾಗಮ _rpmalloc_aligned_allocate(heap, alignment, size);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, size_t ಅಂಕೆ_2, size_t size) {
  ಪ್ರತ್ಯಾಗಮ rpmalloc_heap_aligned_calloc(heap, 0, ಅಂಕೆ_2, size);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, size_t alignment,
                             size_t ಅಂಕೆ_2, size_t size) {
  size_t total;
#ಯದಿ ENABLE_VALIDATE_ARGS
#ಯದಿ PLATFORM_WINDOWS
  ಪೂರ್ಣಾಙ್ಕ err = SizeTMult(ಅಂಕೆ_2, size, &total);
  ಯದಿ ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#ಅನ್ಯಥಾ
  ಪೂರ್ಣಾಙ್ಕ err = __builtin_umull_overflow(ಅಂಕೆ_2, size, &total);
  ಯದಿ (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
#ಅನ್ಯಥಾ
  total = ಅಂಕೆ_2 * size;
#endif
  ಶೂನ್ಯ *block = _rpmalloc_aligned_allocate(heap, alignment, total);
  ಯದಿ (block)
    memset(block, 0, total);
  ಪ್ರತ್ಯಾಗಮ block;
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, ಶೂನ್ಯ *ptr, size_t size,
                      unsigned ಪೂರ್ಣಾಙ್ಕ flags) {
#ಯದಿ ENABLE_VALIDATE_ARGS
  ಯದಿ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ ptr;
  }
#endif
  ಪ್ರತ್ಯಾಗಮ _rpmalloc_reallocate(heap, ptr, size, 0, flags);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_ALLOCATOR ಶೂನ್ಯ *
rpmalloc_heap_aligned_realloc(rpmalloc_heap_t *heap, ಶೂನ್ಯ *ptr,
                              size_t alignment, size_t size,
                              unsigned ಪೂರ್ಣಾಙ್ಕ flags) {
#ಯದಿ ENABLE_VALIDATE_ARGS
  ಯದಿ ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
  ಪ್ರತ್ಯಾಗಮ _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, 0, flags);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ rpmalloc_heap_free(rpmalloc_heap_t *heap, ಶೂನ್ಯ *ptr) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(heap);
  _rpmalloc_deallocate(ptr);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ rpmalloc_heap_free_all(rpmalloc_heap_t *heap) {
  span_t *span;
  span_t *next_span;

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  ಪ್ರತಿ (size_t iclass = 0; iclass < SIZE_CLASS_ಎಣಿಕೆ; ++iclass) {
    span = heap->size_class[iclass].partial_span;
    ಯಾವತ್ (span) {
      next_span = span->next;
      rpmalloc_heap_cache_ಸೇರಿಸು(heap, span);
      span = next_span;
    }
    heap->size_class[iclass].partial_span = 0;
    span = heap->full_span[iclass];
    ಯಾವತ್ (span) {
      next_span = span->next;
      rpmalloc_heap_cache_ಸೇರಿಸು(heap, span);
      span = next_span;
    }

    span = heap->size_class[iclass].cache;
    ಯದಿ (span)
      rpmalloc_heap_cache_ಸೇರಿಸು(heap, span);
    heap->size_class[iclass].cache = 0;
  }
  memset(heap->size_class, 0, ಪರಿಮಾಣ(heap->size_class));
  memset(heap->full_span, 0, ಪರಿಮಾಣ(heap->full_span));

  span = heap->large_huge_span;
  ಯಾವತ್ (span) {
    next_span = span->next;
    ಯದಿ (UNEXPECTED(span->size_class == SIZE_CLASS_HUGE))
      _rpmalloc_deallocate_huge(span);
    ಅನ್ಯಥಾ
      rpmalloc_heap_cache_ಸೇರಿಸು(heap, span);
    span = next_span;
  }
  heap->large_huge_span = 0;
  heap->full_span_ಎಣಿಕೆ = 0;

#ಯದಿ ENABLE_THREAD_CACHE
  ಪ್ರತಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    span_cache_t *span_cache;
    ಯದಿ (!iclass)
      span_cache = &heap->span_cache;
    ಅನ್ಯಥಾ
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    ಯದಿ (!span_cache->ಎಣಿಕೆ)
      ಅನುವರ್ತ;
#ಯದಿ ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&heap->thread_to_global,
                         span_cache->ಎಣಿಕೆ * (iclass + 1) * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                       span_cache->ಎಣಿಕೆ);
    rpmalloc_global_cache_ಸೇರಿಸು_spans(span_cache->span, iclass + 1,
                                        span_cache->ಎಣಿಕೆ);
#ಅನ್ಯಥಾ
    ಪ್ರತಿ (size_t ispan = 0; ispan < span_cache->ಎಣಿಕೆ; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
    span_cache->ಎಣಿಕೆ = 0;
  }
#endif

#ಯದಿ ENABLE_STATISTICS
  ಪ್ರತಿ (size_t iclass = 0; iclass < SIZE_CLASS_ಎಣಿಕೆ; ++iclass) {
    atomic_store32(&heap->size_class_use[iclass].alloc_current, 0);
    atomic_store32(&heap->size_class_use[iclass].spans_current, 0);
  }
  ಪ್ರತಿ (size_t iclass = 0; iclass < LARGE_CLASS_ಎಣಿಕೆ; ++iclass) {
    atomic_store32(&heap->span_use[iclass].current, 0);
  }
#endif
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ rpmalloc_heap_thread_set_current(rpmalloc_heap_t *heap) {
  heap_t *prev_heap = get_thread_heap_raw();
  ಯದಿ (prev_heap != heap) {
    set_thread_heap(heap);
    ಯದಿ (prev_heap)
      rpmalloc_heap_release(prev_heap);
  }
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(ಶೂನ್ಯ *ptr) {
  // Grab the span, and then the heap from the span
  span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
  ಯದಿ (span) {
    ಪ್ರತ್ಯಾಗಮ span->heap;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

#endif

#ಯದಿ ENABLE_PRELOAD || ENABLE_OVERRIDE

#ಅನ್ತರ್ಭಾವ "malloc.c"

#endif

ಶೂನ್ಯ rpmalloc_linker_reference(ಶೂನ್ಯ) { (ಶೂನ್ಯ)ಪರಿಮಾಣ(_rpmalloc_initialized); }
