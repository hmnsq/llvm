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

#ଅନ୍ତର୍ଭାଵ "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#ଯଦି defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wunused-function"
#ଯଦି __has_warning("-Wreserved-identifier")
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
#ଯଦି __has_warning("-Wstatic-in-inline")
#pragma clang diagnostic ignored "-Wstatic-in-inline"
#endif
#ନଚେତଯଦି defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ଯଦି !defined(__has_builtin)
#define __has_builtin(b) 0
#endif

#ଯଦି defined(__GNUC__) || defined(__clang__)

#ଯଦି __has_builtin(__builtin_memcpy_inline)
#define _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#ଅନ୍ୟଥା
#define _rpmalloc_memcpy_const(x, y, s)                                        \
  କର {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memcpy(x, y, s);                                                           \
  } ଯେତେବେଳେ (0)
#endif

#ଯଦି __has_builtin(__builtin_memset_inline)
#define _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#ଅନ୍ୟଥା
#define _rpmalloc_memset_const(x, y, s)                                        \
  କର {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } ଯେତେବେଳେ (0)
#endif
#ଅନ୍ୟଥା
#define _rpmalloc_memcpy_const(x, y, s) memcpy(x, y, s)
#define _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#endif

#ଯଦି __has_builtin(__builtin_assume)
#define rpmalloc_assume(cond) __builtin_assume(cond)
#ନଚେତଯଦି defined(__GNUC__)
#define rpmalloc_assume(cond)                                                  \
  କର {                                                                         \
    ଯଦି (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } ଯେତେବେଳେ (0)
#ନଚେତଯଦି defined(_MSC_VER)
#define rpmalloc_assume(cond) __assume(cond)
#ଅନ୍ୟଥା
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
#ifndef DEFAULT_SPAN_MAP_ଗଣନା
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#define DEFAULT_SPAN_MAP_ଗଣନା 64
#endif
#ifndef GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#define GLOBAL_CACHE_MULTIPLIER 8
#endif

#ଯଦି DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#error Must use global cache ଯଦି unmap is disabled
#endif

#ଯଦି DISABLE_UNMAP
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 1
#endif

#ଯଦି !ENABLE_GLOBAL_CACHE
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 0
#endif

#ଯଦି !ENABLE_THREAD_CACHE
#undef ENABLE_ADAPTIVE_THREAD_CACHE
#define ENABLE_ADAPTIVE_THREAD_CACHE 0
#endif

#ଯଦି defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#define PLATFORM_POSIX 0
#ଅନ୍ୟଥା
#define PLATFORM_WINDOWS 0
#define PLATFORM_POSIX 1
#endif

/// Platform and arch specifics
#ଯଦି defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 5105)
#ifndef FORCEINLINE
#define FORCEINLINE ଧାଡ଼ିଭିତରେ __forceinline
#endif
#define _Static_assert static_assert
#ଅନ୍ୟଥା
#ifndef FORCEINLINE
#define FORCEINLINE ଧାଡ଼ିଭିତରେ __attribute__((__always_inline__))
#endif
#endif
#ଯଦି PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ଅନ୍ତର୍ଭାଵ <windows.h>
#ଯଦି ENABLE_VALIDATE_ARGS
#ଅନ୍ତର୍ଭାଵ <intsafe.h>
#endif
#ଅନ୍ୟଥା
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଯଦି defined(__linux__) || defined(__ANDROID__)
#ଅନ୍ତର୍ଭାଵ <sys/prctl.h>
#ଯଦି !defined(PR_SET_VMA)
#define PR_SET_VMA 0x53564d41
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif
#ଯଦି defined(__APPLE__)
#ଅନ୍ତର୍ଭାଵ <TargetConditionals.h>
#ଯଦି !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#ଅନ୍ତର୍ଭାଵ <mach/mach_vm.h>
#ଅନ୍ତର୍ଭାଵ <mach/vm_statistics.h>
#endif
#ଅନ୍ତର୍ଭାଵ <pthread.h>
#endif
#ଯଦି defined(__HAIKU__) || defined(__TINYC__)
#ଅନ୍ତର୍ଭାଵ <pthread.h>
#endif
#endif

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଯଦି defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#ଅନ୍ତର୍ଭାଵ <fibersapi.h>
ସ୍ଥିର DWORD fls_ଚାବି;
#endif

#ଯଦି PLATFORM_POSIX
#ଅନ୍ତର୍ଭାଵ <sched.h>
#ଅନ୍ତର୍ଭାଵ <sys/mman.h>
#ifdef __FreeBSD__
#ଅନ୍ତର୍ଭାଵ <sys/sysctl.h>
#define MAP_HUGETLB MAP_ALIGNED_SUPER
#ifndef PROT_MAX
#define PROT_MAX(f) 0
#endif
#ଅନ୍ୟଥା
#define PROT_MAX(f) 0
#endif
#ifdef __sun
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା madvise(caddr_t, size_t, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
#endif
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif
#endif
#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଯଦି ENABLE_ASSERTS
#undef NDEBUG
#ଯଦି defined(_MSC_VER) && !defined(_DEBUG)
#define _DEBUG
#endif
#ଅନ୍ତର୍ଭାଵ <assert.h>
#define RPMALLOC_TOSTRING_M(x) #x
#define RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#define rpmalloc_assert(truth, message)                                        \
  କର {                                                                         \
    ଯଦି (!(truth)) {                                                            \
      ଯଦି (_memory_config.error_callback) {                                     \
        _memory_config.error_callback(message " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } ଅନ୍ୟଥା {                                                                 \
        ପ୍ରତିଜ୍ଞା((truth) && message);                                            \
      }                                                                        \
    }                                                                          \
  } ଯେତେବେଳେ (0)
#ଅନ୍ୟଥା
#define rpmalloc_assert(truth, message)                                        \
  କର {                                                                         \
  } ଯେତେବେଳେ (0)
#endif
#ଯଦି ENABLE_STATISTICS
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#endif

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#ଯଦି defined(_MSC_VER) && !defined(__clang__)

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଅସ୍ଥିର long atomic32_t;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଅସ୍ଥିର long long atomic64_t;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଅସ୍ଥିର ଶୂନ୍ୟ *atomicptr_t;

ସ୍ଥିର FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  ଫେରନ୍ତୁ (int32_t)InterlockedOr(src, 0);
}
ସ୍ଥିର FORCEINLINE ଶୂନ୍ୟ atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ସ୍ଥିର FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  ଫେରନ୍ତୁ (int32_t)InterlockedIncrement(val);
}
ସ୍ଥିର FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  ଫେରନ୍ତୁ (int32_t)InterlockedDecrement(val);
}
ସ୍ଥିର FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  ଫେରନ୍ତୁ (int32_t)InterlockedExchangeAdd(val, add) + add;
}
ସ୍ଥିର FORCEINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  ଫେରନ୍ତୁ (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
ସ୍ଥିର FORCEINLINE ଶୂନ୍ୟ atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ସ୍ଥିର FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  ଫେରନ୍ତୁ (int64_t)InterlockedOr64(src, 0);
}
ସ୍ଥିର FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  ଫେରନ୍ତୁ (int64_t)InterlockedExchangeAdd64(val, add) + add;
}
ସ୍ଥିର FORCEINLINE ଶୂନ୍ୟ *atomic_load_ptr(atomicptr_t *src) {
  ଫେରନ୍ତୁ InterlockedCompareExchangePointer(src, 0, 0);
}
ସ୍ଥିର FORCEINLINE ଶୂନ୍ୟ atomic_store_ptr(atomicptr_t *dst, ଶୂନ୍ୟ *val) {
  InterlockedExchangePointer(dst, val);
}
ସ୍ଥିର FORCEINLINE ଶୂନ୍ୟ atomic_store_ptr_release(atomicptr_t *dst, ଶୂନ୍ୟ *val) {
  InterlockedExchangePointer(dst, val);
}
ସ୍ଥିର FORCEINLINE ଶୂନ୍ୟ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     ଶୂନ୍ୟ *val) {
  ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)InterlockedExchangePointer((ଶୂନ୍ୟ *ଅସ୍ଥିର *)dst, val);
}
ସ୍ଥିର FORCEINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା atomic_cas_ptr(atomicptr_t *dst, ଶୂନ୍ୟ *val, ଶୂନ୍ୟ *ref) {
  ଫେରନ୍ତୁ (InterlockedCompareExchangePointer((ଶୂନ୍ୟ *ଅସ୍ଥିର *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#define EXPECTED(x) (x)
#define UNEXPECTED(x) (x)

#ଅନ୍ୟଥା

#ଅନ୍ତର୍ଭାଵ <stdatomic.h>

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଅସ୍ଥିର _Atomic(int32_t) atomic32_t;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଅସ୍ଥିର _Atomic(int64_t) atomic64_t;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଅସ୍ଥିର _Atomic(ଶୂନ୍ୟ *) atomicptr_t;

ସ୍ଥିର FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  ଫେରନ୍ତୁ atomic_load_explicit(src, memory_order_relaxed);
}
ସ୍ଥିର FORCEINLINE ଶୂନ୍ୟ atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ସ୍ଥିର FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  ଫେରନ୍ତୁ atomic_fetch_add_explicit(val, 1, memory_order_relaxed) + 1;
}
ସ୍ଥିର FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  ଫେରନ୍ତୁ atomic_fetch_add_explicit(val, -1, memory_order_relaxed) - 1;
}
ସ୍ଥିର FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  ଫେରନ୍ତୁ atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
ସ୍ଥିର FORCEINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  ଫେରନ୍ତୁ atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
ସ୍ଥିର FORCEINLINE ଶୂନ୍ୟ atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ସ୍ଥିର FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  ଫେରନ୍ତୁ atomic_load_explicit(val, memory_order_relaxed);
}
ସ୍ଥିର FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  ଫେରନ୍ତୁ atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
ସ୍ଥିର FORCEINLINE ଶୂନ୍ୟ *atomic_load_ptr(atomicptr_t *src) {
  ଫେରନ୍ତୁ atomic_load_explicit(src, memory_order_relaxed);
}
ସ୍ଥିର FORCEINLINE ଶୂନ୍ୟ atomic_store_ptr(atomicptr_t *dst, ଶୂନ୍ୟ *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ସ୍ଥିର FORCEINLINE ଶୂନ୍ୟ atomic_store_ptr_release(atomicptr_t *dst, ଶୂନ୍ୟ *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ସ୍ଥିର FORCEINLINE ଶୂନ୍ୟ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     ଶୂନ୍ୟ *val) {
  ଫେରନ୍ତୁ atomic_exchange_explicit(dst, val, memory_order_acquire);
}
ସ୍ଥିର FORCEINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା atomic_cas_ptr(atomicptr_t *dst, ଶୂନ୍ୟ *val, ଶୂନ୍ୟ *ref) {
  ଫେରନ୍ତୁ atomic_compare_exchange_weak_explicit(
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

#ଯଦି ENABLE_STATISTICS
#define _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#define _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#define _rpmalloc_stat_add(counter, ମୂଲ୍ୟ_2)                                     \
  atomic_add32(counter, (int32_t)(ମୂଲ୍ୟ_2))
#define _rpmalloc_stat_add64(counter, ମୂଲ୍ୟ_2)                                   \
  atomic_add64(counter, (int64_t)(ମୂଲ୍ୟ_2))
#define _rpmalloc_stat_add_peak(counter, ମୂଲ୍ୟ_2, peak)                          \
  କର {                                                                         \
    int32_t cur_ଗଣନା = atomic_add32(counter, (int32_t)(ମୂଲ୍ୟ_2));              \
    ଯଦି (cur_ଗଣନା > (peak))                                                   \
      peak = cur_ଗଣନା;                                                       \
  } ଯେତେବେଳେ (0)
#define _rpmalloc_stat_sub(counter, ମୂଲ୍ୟ_2)                                     \
  atomic_add32(counter, -(int32_t)(ମୂଲ୍ୟ_2))
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  କର {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&heap->size_class_use[class_idx].alloc_current);         \
    ଯଦି (alloc_current > heap->size_class_use[class_idx].alloc_peak)            \
      heap->size_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&heap->size_class_use[class_idx].alloc_total);               \
  } ଯେତେବେଳେ (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  କର {                                                                         \
    atomic_decr32(&heap->size_class_use[class_idx].alloc_current);             \
    atomic_incr32(&heap->size_class_use[class_idx].free_total);                \
  } ଯେତେବେଳେ (0)
#ଅନ୍ୟଥା
#define _rpmalloc_stat_inc(counter)                                            \
  କର {                                                                         \
  } ଯେତେବେଳେ (0)
#define _rpmalloc_stat_dec(counter)                                            \
  କର {                                                                         \
  } ଯେତେବେଳେ (0)
#define _rpmalloc_stat_add(counter, ମୂଲ୍ୟ_2)                                     \
  କର {                                                                         \
  } ଯେତେବେଳେ (0)
#define _rpmalloc_stat_add64(counter, ମୂଲ୍ୟ_2)                                   \
  କର {                                                                         \
  } ଯେତେବେଳେ (0)
#define _rpmalloc_stat_add_peak(counter, ମୂଲ୍ୟ_2, peak)                          \
  କର {                                                                         \
  } ଯେତେବେଳେ (0)
#define _rpmalloc_stat_sub(counter, ମୂଲ୍ୟ_2)                                     \
  କର {                                                                         \
  } ଯେତେବେଳେ (0)
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  କର {                                                                         \
  } ଯେତେବେଳେ (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  କର {                                                                         \
  } ଯେତେବେଳେ (0)
#endif

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#define SMALL_GRANULARITY 16
//! Small granularity shift count
#define SMALL_GRANULARITY_SHIFT 4
//! Number of small block size classes
#define SMALL_CLASS_ଗଣନା 65
//! Maximum size of a small block
#define SMALL_SIZE_LIMIT (SMALL_GRANULARITY * (SMALL_CLASS_ଗଣନା - 1))
//! Granularity of a medium allocation block
#define MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#define MEDIUM_GRANULARITY_SHIFT 9
//! Number of medium block size classes
#define MEDIUM_CLASS_ଗଣନା 61
//! Total number of small + medium size classes
#define SIZE_CLASS_ଗଣନା (SMALL_CLASS_ଗଣନା + MEDIUM_CLASS_ଗଣନା)
//! Number of large block size classes
#define LARGE_CLASS_ଗଣନା 63
//! Maximum size of a medium block
#define MEDIUM_SIZE_LIMIT                                                      \
  (SMALL_SIZE_LIMIT + (MEDIUM_GRANULARITY * MEDIUM_CLASS_ଗଣନା))
//! Maximum size of a large block
#define LARGE_SIZE_LIMIT                                                       \
  ((LARGE_CLASS_ଗଣନା * _memory_span_size) - SPAN_HEADER_SIZE)
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

#ଯଦି ENABLE_VALIDATE_ARGS
//! Maximum allocation size to avoid integer overflow
#undef MAX_ALLOC_SIZE
#define MAX_ALLOC_SIZE (((size_t) - 1) - _memory_span_size)
#endif

#define pointer_offset(ptr, ofs) (ଶୂନ୍ୟ *)((ଅକ୍ଷର *)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff(first, second)                                            \
  (ptrdiff_t)((ଅଚଳ ଅକ୍ଷର *)(first) - (ଅଚଳ ଅକ୍ଷର *)(second))

#define INVALID_POINTER ((ଶୂନ୍ୟ *)((uintptr_t) - 1))

#define SIZE_CLASS_LARGE SIZE_CLASS_ଗଣନା
#define SIZE_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା heap_t heap_t;
//! Span of memory pages
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା span_t span_t;
//! Span list
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା span_list_t span_list_t;
//! Span active data
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା span_active_t span_active_t;
//! Size class definition
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା size_class_t size_class_t;
//! Global cache
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#define SPAN_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#define SPAN_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#define SPAN_FLAG_ALIGNED_BLOCKS 4U
//! Flag indicating an unmapped master span
#define SPAN_FLAG_UNMAPPED_MASTER 8U

#ଯଦି ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
ସଂରଚନା span_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t ଉପର;
#ଯଦି ENABLE_STATISTICS
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା span_use_t span_use_t;
#endif

#ଯଦି ENABLE_STATISTICS
ସଂରଚନା size_class_use_t {
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା size_class_use_t size_class_use_t;
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
ସଂରଚନା span_t {
  //! Free list
  ଶୂନ୍ୟ *free_list;
  //! Total block count of size class
  uint32_t block_ଗଣନା;
  //! Size class
  uint32_t size_class;
  //! Index of last block initialized in free list
  uint32_t free_list_limit;
  //! Number of used blocks remaining when in partial state
  uint32_t used_ଗଣନା;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_size;
  //! Size of a block
  uint32_t block_size;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t span_ଗଣନା;
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
_Static_assert(ଆକାର(span_t) <= SPAN_HEADER_SIZE, "span size mismatch");

ସଂରଚନା span_cache_t {
  size_t ଗଣନା_2;
  span_t *span[MAX_THREAD_SPAN_CACHE];
};
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା span_cache_t span_cache_t;

ସଂରଚନା span_large_cache_t {
  size_t ଗଣନା_2;
  span_t *span[MAX_THREAD_SPAN_LARGE_CACHE];
};
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା span_large_cache_t span_large_cache_t;

ସଂରଚନା heap_size_class_t {
  //! Free list of active span
  ଶୂନ୍ୟ *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  span_t *partial_span;
  //! Early level cache of fully free spans
  span_t *cache;
};
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା heap_size_class_t heap_size_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
ସଂରଚନା heap_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  heap_size_class_t size_class[SIZE_CLASS_ଗଣନା];
#ଯଦି ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  span_cache_t span_cache;
#endif
  //! List of deferred free spans (single linked list)
  atomicptr_t span_free_deferred;
  //! Number of full spans
  size_t full_span_ଗଣନା;
  //! Mapped but unused spans
  span_t *span_reserve;
  //! Master span for mapped but unused spans
  span_t *span_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t child_ଗଣନା;
  //! Next heap in id list
  heap_t *next_heap;
  //! Next heap in orphan list
  heap_t *next_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା finalize;
  //! Master heap owning the memory pages
  heap_t *master_heap;
#ଯଦି ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  span_large_cache_t span_large_cache[LARGE_CLASS_ଗଣନା - 1];
#endif
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  span_t *full_span[SIZE_CLASS_ଗଣନା];
  //! Double linked list of large and huge spans allocated by this heap
  span_t *large_huge_span;
#endif
#ଯଦି ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  span_use_t span_use[LARGE_CLASS_ଗଣନା];
#endif
#ଯଦି ENABLE_STATISTICS
  //! Allocation stats per size class
  size_class_use_t size_class_use[SIZE_CLASS_ଗଣନା + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_to_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_to_thread;
#endif
};

// Size class for defining a block size bucket
ସଂରଚନା size_class_t {
  //! Size of blocks in this class
  uint32_t block_size;
  //! Number of blocks in each chunk
  uint16_t block_ଗଣନା;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(ଆକାର(size_class_t) == 8, "Size class size mismatch");

ସଂରଚନା global_cache_t {
  //! Cache lock
  atomic32_t lock;
  //! Cache count
  uint32_t ଗଣନା_2;
#ଯଦି ENABLE_STATISTICS
  //! Insert count
  size_t ଭର୍ତ୍ତି_ଗଣନା;
  //! Extract count
  size_t extract_ଗଣନା;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା _rpmalloc_initialized;
//! Main thread ID
ସ୍ଥିର uintptr_t _rpmalloc_main_thread_id;
//! Configuration
ସ୍ଥିର rpmalloc_config_t _memory_config;
//! Memory page size
ସ୍ଥିର size_t _memory_page_size;
//! Shift to divide by page size
ସ୍ଥିର size_t _memory_page_size_shift;
//! Granularity at which memory pages are mapped by OS
ସ୍ଥିର size_t _memory_map_granularity;
#ଯଦି RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
ସ୍ଥିର size_t _memory_span_size;
//! Shift to divide by span size
ସ୍ଥିର size_t _memory_span_size_shift;
//! Mask to get to start of a memory span
ସ୍ଥିର uintptr_t _memory_span_mask;
#ଅନ୍ୟଥା
//! Hardwired span size
#define _memory_span_size _memory_default_span_size
#define _memory_span_size_shift _memory_default_span_size_shift
#define _memory_span_mask _memory_default_span_mask
#endif
//! Number of spans to map in each map call
ସ୍ଥିର size_t memory_span_map_ଗଣନା;
//! Number of spans to keep reserved in each heap
ସ୍ଥିର size_t memory_heap_reserve_ଗଣନା;
//! Global size classes
ସ୍ଥିର size_class_t _memory_size_class[SIZE_CLASS_ଗଣନା];
//! Run-time size limit of medium blocks
ସ୍ଥିର size_t _memory_medium_size_limit;
//! Heap ID counter
ସ୍ଥିର atomic32_t _memory_heap_id;
//! Huge page support
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା _memory_huge_pages;
#ଯଦି ENABLE_GLOBAL_CACHE
//! Global span cache
ସ୍ଥିର global_cache_t _memory_span_cache[LARGE_CLASS_ଗଣନା];
#endif
//! Global reserved spans
ସ୍ଥିର span_t *_memory_global_reserve;
//! Global reserved count
ସ୍ଥିର size_t memory_global_reserve_ଗଣନା;
//! Global reserved master
ସ୍ଥିର span_t *_memory_global_reserve_master;
//! All heaps
ସ୍ଥିର heap_t *_memory_heaps[HEAP_ARRAY_SIZE];
//! Used to restrict access to mapping memory for huge pages
ସ୍ଥିର atomic32_t _memory_global_lock;
//! Orphaned heaps
ସ୍ଥିର heap_t *_memory_orphan_heaps;
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
ସ୍ଥିର heap_t *_memory_first_class_orphan_heaps;
#endif
#ଯଦି ENABLE_STATISTICS
//! Allocations counter
ସ୍ଥିର atomic64_t _allocation_counter;
//! Deallocations counter
ସ୍ଥିର atomic64_t _deallocation_counter;
//! Active heap count
ସ୍ଥିର atomic32_t _memory_active_heaps;
//! Number of currently mapped memory pages
ସ୍ଥିର atomic32_t _mapped_pages;
//! Peak number of concurrently mapped memory pages
ସ୍ଥିର int32_t _mapped_pages_peak;
//! Number of mapped master spans
ସ୍ଥିର atomic32_t _master_spans;
//! Number of unmapped dangling master spans
ସ୍ଥିର atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
ସ୍ଥିର atomic32_t _mapped_total;
//! Running counter of total number of unmapped memory pages since start
ସ୍ଥିର atomic32_t _unmapped_total;
//! Number of currently mapped memory pages in OS calls
ସ୍ଥିର atomic32_t _mapped_pages_os;
//! Number of currently allocated pages in huge allocations
ସ୍ଥିର atomic32_t _huge_pages_current;
//! Peak number of currently allocated pages in huge allocations
ସ୍ଥିର int32_t _huge_pages_peak;
#endif

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#ଯଦି ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
ସ୍ଥିର pthread_key_t _memory_thread_heap;
#ଅନ୍ୟଥା
#ifdef _MSC_VER
#define _Thread_local __declspec(thread)
#define TLS_MODEL
#ଅନ୍ୟଥା
#ifndef __HAIKU__
#define TLS_MODEL __attribute__((tls_model("initial-exec")))
#ଅନ୍ୟଥା
#define TLS_MODEL
#endif
#ଯଦି !defined(__clang__) && defined(__GNUC__)
#define _Thread_local __thread
#endif
#endif
ସ୍ଥିର _Thread_local heap_t *_memory_thread_heap TLS_MODEL;
#endif

ସ୍ଥିର ଧାଡ଼ିଭିତରେ heap_t *get_thread_heap_raw(ଶୂନ୍ୟ) {
#ଯଦି (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  ଫେରନ୍ତୁ pthread_getspecific(_memory_thread_heap);
#ଅନ୍ୟଥା
  ଫେରନ୍ତୁ _memory_thread_heap;
#endif
}

//! Get the current thread heap
ସ୍ଥିର ଧାଡ଼ିଭିତରେ heap_t *get_thread_heap(ଶୂନ୍ୟ) {
  heap_t *heap = get_thread_heap_raw();
#ଯଦି ENABLE_PRELOAD
  ଯଦି (EXPECTED(heap != 0))
    ଫେରନ୍ତୁ heap;
  rpmalloc_initialize();
  ଫେରନ୍ତୁ get_thread_heap_raw();
#ଅନ୍ୟଥା
  ଫେରନ୍ତୁ heap;
#endif
}

//! Fast thread ID
ସ୍ଥିର ଧାଡ଼ିଭିତରେ uintptr_t get_thread_id(ଶୂନ୍ୟ) {
#ଯଦି defined(_WIN32)
  ଫେରନ୍ତୁ (uintptr_t)((ଶୂନ୍ୟ *)NtCurrentTeb());
#ନଚେତଯଦି (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#ଯଦି defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#ନଚେତଯଦି defined(__x86_64__)
#ଯଦି defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#ଅନ୍ୟଥା
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#endif
#ନଚେତଯଦି defined(__arm__)
  __asm__ ଅସ୍ଥିର("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#ନଚେତଯଦି defined(__aarch64__)
#ଯଦି defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ ଅସ୍ଥିର("mrs %0, tpidrro_el0" : "=r"(tid));
#ଅନ୍ୟଥା
  __asm__ ଅସ୍ଥିର("mrs %0, tpidr_el0" : "=r"(tid));
#endif
#ଅନ୍ୟଥା
#error This platform needs implementation of get_thread_id()
#endif
  ଫେରନ୍ତୁ tid;
#ଅନ୍ୟଥା
#error This platform needs implementation of get_thread_id()
#endif
}

//! Set the current thread heap
ସ୍ଥିର ଶୂନ୍ୟ set_thread_heap(heap_t *heap) {
#ଯଦି ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(_memory_thread_heap, heap);
#ଅନ୍ୟଥା
  _memory_thread_heap = heap;
#endif
  ଯଦି (heap)
    heap->owner_thread = get_thread_id();
}

//! Set main thread ID
ବାହ୍ୟ ଶୂନ୍ୟ rpmalloc_set_main_thread(ଶୂନ୍ୟ);

ଶୂନ୍ୟ rpmalloc_set_main_thread(ଶୂନ୍ୟ) {
  _rpmalloc_main_thread_id = get_thread_id();
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_spin(ଶୂନ୍ୟ) {
#ଯଦି defined(_MSC_VER)
#ଯଦି defined(_M_ARM64)
  __yield();
#ଅନ୍ୟଥା
  _mm_pause();
#endif
#ନଚେତଯଦି defined(__x86_64__) || defined(__i386__)
  __asm__ ଅସ୍ଥିର("pause" ::: "memory");
#ନଚେତଯଦି defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ ଅସ୍ଥିର("yield" ::: "memory");
#ନଚେତଯଦି defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ ଅସ୍ଥିର("or 27,27,27");
#ନଚେତଯଦି defined(__sparc__)
  __asm__ ଅସ୍ଥିର("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#ଅନ୍ୟଥା
  ସଂରଚନା timespec ts = {0};
  nanosleep(&ts, 0);
#endif
}

#ଯଦି defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

ସ୍ଥିର ଶୂନ୍ୟ NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  ବାଛ (reason) {
  ସ୍ଥିତି DLL_PROCESS_ATTACH:
    ଭାଙ୍ଗ;
  ସ୍ଥିତି DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    ଭାଙ୍ଗ;
  ସ୍ଥିତି DLL_THREAD_ATTACH:
    ଭାଙ୍ଗ;
  ସ୍ଥିତି DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    ଭାଙ୍ଗ;
  }
}

#ifdef _WIN64
#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#pragma const_seg(".CRT$XLY")

ବାହ୍ୟ ଅଚଳ PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ଅଚଳ PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#pragma const_seg()
#ଅନ୍ୟଥା // _WIN64
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#pragma data_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#pragma data_seg()
#endif // _WIN64

ସ୍ଥିର ଶୂନ୍ୟ NTAPI _rpmalloc_thread_destructor(ଶୂନ୍ୟ *ମୂଲ୍ୟ_2) {
#ଯଦି ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  ଯଦି (get_thread_id() == _rpmalloc_main_thread_id)
    ଫେରନ୍ତୁ;
#endif
  ଯଦି (ମୂଲ୍ୟ_2)
    rpmalloc_thread_finalize(1);
}
#endif

////////////
///
/// Low level memory map/unmap
///
//////

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_set_name(ଶୂନ୍ୟ *address, size_t size) {
#ଯଦି defined(__linux__) || defined(__ANDROID__)
  ଅଚଳ ଅକ୍ଷର *name = _memory_huge_pages ? _memory_config.huge_page_name
                                        : _memory_config.page_name;
  ଯଦି (address == MAP_FAILED || !name)
    ଫେରନ୍ତୁ;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (ଶୂନ୍ୟ)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (uintptr_t)address, size,
              (uintptr_t)name);
#ଅନ୍ୟଥା
  (ଶୂନ୍ୟ)ଆକାର(size);
  (ଶୂନ୍ୟ)ଆକାର(address);
#endif
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
ସ୍ଥିର ଶୂନ୍ୟ *_rpmalloc_mmap(size_t size, size_t *offset) {
  rpmalloc_assert(!(size % _memory_page_size), "Invalid mmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
  ଶୂନ୍ୟ *address = _memory_config.memory_map(size, offset);
  ଯଦି (EXPECTED(address != 0)) {
    _rpmalloc_stat_add_peak(&_mapped_pages, (size >> _memory_page_size_shift),
                            _mapped_pages_peak);
    _rpmalloc_stat_add(&_mapped_total, (size >> _memory_page_size_shift));
  }
  ଫେରନ୍ତୁ address;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_unmap(ଶୂନ୍ୟ *address, size_t size, size_t offset,
                            size_t release) {
  rpmalloc_assert(!release || (release >= size), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  ଯଦି (release) {
    rpmalloc_assert(!(release % _memory_page_size), "Invalid unmap size");
    _rpmalloc_stat_sub(&_mapped_pages, (release >> _memory_page_size_shift));
    _rpmalloc_stat_add(&_unmapped_total, (release >> _memory_page_size_shift));
  }
  _memory_config.memory_unmap(address, size, offset, release);
}

//! Default implementation to map new pages to virtual memory
ସ୍ଥିର ଶୂନ୍ୟ *_rpmalloc_mmap_os(size_t size, size_t *offset) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((size >= _memory_span_size) &&
                    (_memory_span_size > _memory_map_granularity))
                       ? _memory_span_size
                       : 0;
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
#ଯଦି PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  ଶୂନ୍ୟ *ptr = VirtualAlloc(0, size + padding,
                           (_memory_huge_pages ? MEM_LARGE_PAGES : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
  ଯଦି (!ptr) {
    ଯଦି (_memory_config.map_fail_callback) {
      ଯଦି (_memory_config.map_fail_callback(size + padding))
        ଫେରନ୍ତୁ _rpmalloc_mmap_os(size, offset);
    } ଅନ୍ୟଥା {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    ଫେରନ୍ତୁ 0;
  }
#ଅନ୍ୟଥା
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#ଯଦି defined(__APPLE__) && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)VM_MAKE_TAG(240U);
  ଯଦି (_memory_huge_pages)
    fd |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
  ଶୂନ୍ୟ *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#ନଚେତଯଦି defined(MAP_HUGETLB)
  ଶୂନ୍ୟ *ptr = mmap(0, size + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (_memory_huge_pages ? MAP_HUGETLB : 0) | flags, -1, 0);
#ଯଦି defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  ଯଦି ((ptr == MAP_FAILED || !ptr) && _memory_huge_pages) {
    ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    ଯଦି (ptr && ptr != MAP_FAILED) {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା prm = madvise(ptr, size + padding, MADV_HUGEPAGE);
      (ଶୂନ୍ୟ)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#endif
  _rpmalloc_set_name(ptr, size + padding);
#ନଚେତଯଦି defined(MAP_ALIGNED)
  ଅଚଳ size_t align =
      (ଆକାର(size_t) * 8) - (size_t)(__builtin_clzl(size - 1));
  ଶୂନ୍ୟ *ptr =
      mmap(0, size + padding, PROT_READ | PROT_WRITE,
           (_memory_huge_pages ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#ନଚେତଯଦି defined(MAP_ALIGN)
  caddr_t base = (_memory_huge_pages ? (caddr_t)(4 << 20) : 0);
  ଶୂନ୍ୟ *ptr = mmap(base, size + padding, PROT_READ | PROT_WRITE,
                   (_memory_huge_pages ? MAP_ALIGN : 0) | flags, -1, 0);
#ଅନ୍ୟଥା
  ଶୂନ୍ୟ *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#endif
  ଯଦି ((ptr == MAP_FAILED) || !ptr) {
    ଯଦି (_memory_config.map_fail_callback) {
      ଯଦି (_memory_config.map_fail_callback(size + padding))
        ଫେରନ୍ତୁ _rpmalloc_mmap_os(size, offset);
    } ଅନ୍ୟଥା ଯଦି (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    ଫେରନ୍ତୁ 0;
  }
#endif
  _rpmalloc_stat_add(&_mapped_pages_os,
                     (int32_t)((size + padding) >> _memory_page_size_shift));
  ଯଦି (padding) {
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
  ଫେରନ୍ତୁ ptr;
}

//! Default implementation to unmap pages from virtual memory
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_unmap_os(ଶୂନ୍ୟ *address, size_t size, size_t offset,
                               size_t release) {
  rpmalloc_assert(release || (offset == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid unmap size");
  ଯଦି (release && offset) {
    offset <<= 3;
    address = pointer_offset(address, -(int32_t)offset);
    ଯଦି ((release >= _memory_span_size) &&
        (_memory_span_size > _memory_map_granularity)) {
      // Padding is always one span size
      release += _memory_span_size;
    }
  }
#ଯଦି !DISABLE_UNMAP
#ଯଦି PLATFORM_WINDOWS
  ଯଦି (!VirtualFree(address, release ? 0 : size,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#ଅନ୍ୟଥା
  ଯଦି (release) {
    ଯଦି (munmap(address, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } ଅନ୍ୟଥା {
#ଯଦି defined(MADV_FREE_REUSABLE)
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    ଯେତେବେଳେ ((ret = madvise(address, size, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    ଯଦି ((ret == -1) && (errno != 0)) {
#ନଚେତଯଦି defined(MADV_DONTNEED)
    ଯଦି (madvise(address, size, MADV_DONTNEED)) {
#ନଚେତଯଦି defined(MADV_PAGEOUT)
    ଯଦି (madvise(address, size, MADV_PAGEOUT)) {
#ନଚେତଯଦି defined(MADV_FREE)
    ଯଦି (madvise(address, size, MADV_FREE)) {
#ଅନ୍ୟଥା
    ଯଦି (posix_madvise(address, size, POSIX_MADV_DONTNEED)) {
#endif
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#endif
#endif
  ଯଦି (release)
    _rpmalloc_stat_sub(&_mapped_pages_os, release >> _memory_page_size_shift);
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_ଗଣନା);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
ସ୍ଥିର span_t *_rpmalloc_global_get_reserved_spans(size_t span_ଗଣନା) {
  span_t *span = _memory_global_reserve;
  _rpmalloc_span_mark_as_subspan_unless_master(_memory_global_reserve_master,
                                               span, span_ଗଣନା);
  memory_global_reserve_ଗଣନା -= span_ଗଣନା;
  ଯଦି (memory_global_reserve_ଗଣନା)
    _memory_global_reserve =
        (span_t *)pointer_offset(span, span_ଗଣନା << _memory_span_size_shift);
  ଅନ୍ୟଥା
    _memory_global_reserve = 0;
  ଫେରନ୍ତୁ span;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_global_set_reserved_spans(span_t *master, span_t *reserve,
                                                size_t reserve_span_ଗଣନା) {
  _memory_global_reserve_master = master;
  memory_global_reserve_ଗଣନା = reserve_span_ଗଣନା;
  _memory_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_span_double_link_list_add(span_t **head, span_t *span) {
  ଯଦି (*head)
    (*head)->prev = span;
  span->next = *head;
  *head = span;
}

//! Pop head span from double linked list
ସ୍ଥିର ଶୂନ୍ୟ rpmalloc_span_double_link_list_କାଢ଼_head(span_t **head,
                                                     span_t *span) {
  rpmalloc_assert(*head == span, "Linked list corrupted");
  span = *head;
  *head = span->next;
}

//! Remove a span from double linked list
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_span_double_link_list_remove(span_t **head,
                                                   span_t *span) {
  rpmalloc_assert(*head, "Linked list corrupted");
  ଯଦି (*head == span) {
    *head = span->next;
  } ଅନ୍ୟଥା {
    span_t *next_span = span->next;
    span_t *prev_span = span->prev;
    prev_span->next = next_span;
    ଯଦି (EXPECTED(next_span != 0))
      next_span->prev = prev_span;
  }
}

////////////
///
/// Span control
///
//////

ସ୍ଥିର ଶୂନ୍ୟ rpmalloc_heap_cache_ଭର୍ତ୍ତି(heap_t *heap, span_t *span);

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_heap_finalize(heap_t *heap);

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_ଗଣନା);

//! Declare the span to be a subspan and store distance from master span and
//! span count
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_ଗଣନା) {
  rpmalloc_assert((subspan != master) || (subspan->flags & SPAN_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  ଯଦି (subspan != master) {
    subspan->flags = SPAN_FLAG_SUBSPAN;
    subspan->offset_from_master =
        (uint32_t)((uintptr_t)pointer_diff(subspan, master) >>
                   _memory_span_size_shift);
    subspan->align_offset = 0;
  }
  subspan->span_ଗଣନା = (uint32_t)span_ଗଣନା;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
ସ୍ଥିର span_t *_rpmalloc_span_map_from_reserve(heap_t *heap,
                                               size_t span_ଗଣନା) {
  // Update the heap span reserve
  span_t *span = heap->span_reserve;
  heap->span_reserve =
      (span_t *)pointer_offset(span, span_ଗଣନା * _memory_span_size);
  heap->spans_reserved -= (uint32_t)span_ଗଣନା;

  _rpmalloc_span_mark_as_subspan_unless_master(heap->span_reserve_master, span,
                                               span_ଗଣନା);
  ଯଦି (span_ଗଣନା <= LARGE_CLASS_ଗଣନା)
    _rpmalloc_stat_inc(&heap->span_use[span_ଗଣନା - 1].spans_from_reserved);

  ଫେରନ୍ତୁ span;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
ସ୍ଥିର size_t rpmalloc_span_align_ଗଣନା(size_t span_ଗଣନା) {
  size_t request_ଗଣନା = (span_ଗଣନା > memory_span_map_ଗଣନା)
                             ? span_ଗଣନା
                             : memory_span_map_ଗଣନା;
  ଯଦି ((_memory_page_size > _memory_span_size) &&
      ((request_ଗଣନା * _memory_span_size) % _memory_page_size))
    request_ଗଣନା +=
        memory_span_map_ଗଣନା - (request_ଗଣନା % memory_span_map_ଗଣନା);
  ଫେରନ୍ତୁ request_ଗଣନା;
}

//! Setup a newly mapped span
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_span_initialize(span_t *span, size_t total_span_ଗଣନା,
                                      size_t span_ଗଣନା, size_t align_offset) {
  span->total_spans = (uint32_t)total_span_ଗଣନା;
  span->span_ଗଣନା = (uint32_t)span_ଗଣନା;
  span->align_offset = (uint32_t)align_offset;
  span->flags = SPAN_FLAG_MASTER;
  atomic_store32(&span->remaining_spans, (int32_t)total_span_ଗଣନା);
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_span_unmap(span_t *span);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
ସ୍ଥିର span_t *rpmalloc_span_map_aligned_ଗଣନା(heap_t *heap,
                                                size_t span_ଗଣନା) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_span_ଗଣନା = rpmalloc_span_align_ଗଣନା(span_ଗଣନା);
  size_t align_offset = 0;
  span_t *span = (span_t *)_rpmalloc_mmap(
      aligned_span_ଗଣନା * _memory_span_size, &align_offset);
  ଯଦି (!span)
    ଫେରନ୍ତୁ 0;
  _rpmalloc_span_initialize(span, aligned_span_ଗଣନା, span_ଗଣନା, align_offset);
  _rpmalloc_stat_inc(&_master_spans);
  ଯଦି (span_ଗଣନା <= LARGE_CLASS_ଗଣନା)
    _rpmalloc_stat_inc(&heap->span_use[span_ଗଣନା - 1].spans_map_calls);
  ଯଦି (aligned_span_ଗଣନା > span_ଗଣନା) {
    span_t *reserved_spans =
        (span_t *)pointer_offset(span, span_ଗଣନା * _memory_span_size);
    size_t reserved_ଗଣନା = aligned_span_ଗଣନା - span_ଗଣନା;
    ଯଦି (heap->spans_reserved) {
      _rpmalloc_span_mark_as_subspan_unless_master(
          heap->span_reserve_master, heap->span_reserve, heap->spans_reserved);
      rpmalloc_heap_cache_ଭର୍ତ୍ତି(heap, heap->span_reserve);
    }
    ଯଦି (reserved_ଗଣନା > memory_heap_reserve_ଗଣନା) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&_memory_global_lock) == 1,
                      "Global spin lock not held as expected");
      size_t remain_ଗଣନା = reserved_ଗଣନା - memory_heap_reserve_ଗଣନା;
      reserved_ଗଣନା = memory_heap_reserve_ଗଣନା;
      span_t *remain_span = (span_t *)pointer_offset(
          reserved_spans, reserved_ଗଣନା * _memory_span_size);
      ଯଦି (_memory_global_reserve) {
        _rpmalloc_span_mark_as_subspan_unless_master(
            _memory_global_reserve_master, _memory_global_reserve,
            memory_global_reserve_ଗଣନା);
        _rpmalloc_span_unmap(_memory_global_reserve);
      }
      _rpmalloc_global_set_reserved_spans(span, remain_span, remain_ଗଣନା);
    }
    _rpmalloc_heap_set_reserved_spans(heap, span, reserved_spans,
                                      reserved_ଗଣନା);
  }
  ଫେରନ୍ତୁ span;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
ସ୍ଥିର span_t *_rpmalloc_span_map(heap_t *heap, size_t span_ଗଣନା) {
  ଯଦି (span_ଗଣନା <= heap->spans_reserved)
    ଫେରନ୍ତୁ _rpmalloc_span_map_from_reserve(heap, span_ଗଣନା);
  span_t *span = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା use_global_reserve =
      (_memory_page_size > _memory_span_size) ||
      (memory_span_map_ଗଣନା > memory_heap_reserve_ଗଣନା);
  ଯଦି (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    ଯେତେବେଳେ (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
    ଯଦି (memory_global_reserve_ଗଣନା >= span_ଗଣନା) {
      size_t reserve_ଗଣନା =
          (!heap->spans_reserved ? memory_heap_reserve_ଗଣନା : span_ଗଣନା);
      ଯଦି (memory_global_reserve_ଗଣନା < reserve_ଗଣନା)
        reserve_ଗଣନା = memory_global_reserve_ଗଣନା;
      span = _rpmalloc_global_get_reserved_spans(reserve_ଗଣନା);
      ଯଦି (span) {
        ଯଦି (reserve_ଗଣନା > span_ଗଣନା) {
          span_t *reserved_span = (span_t *)pointer_offset(
              span, span_ଗଣନା << _memory_span_size_shift);
          _rpmalloc_heap_set_reserved_spans(heap, _memory_global_reserve_master,
                                            reserved_span,
                                            reserve_ଗଣନା - span_ଗଣନା);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        span->span_ଗଣନା = (uint32_t)span_ଗଣନା;
      }
    }
  }
  ଯଦି (!span)
    span = rpmalloc_span_map_aligned_ଗଣନା(heap, span_ଗଣନା);
  ଯଦି (use_global_reserve)
    atomic_store32_release(&_memory_global_lock, 0);
  ଫେରନ୍ତୁ span;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_span_unmap(span_t *span) {
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_master = !!(span->flags & SPAN_FLAG_MASTER);
  span_t *master =
      is_master ? span
                : ((span_t *)pointer_offset(
                      span, -(intptr_t)((uintptr_t)span->offset_from_master *
                                        _memory_span_size)));
  rpmalloc_assert(is_master || (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");

  size_t span_ଗଣନା = span->span_ଗଣନା;
  ଯଦି (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(span->align_offset == 0, "Span align offset corrupted");
    ଯଦି (_memory_span_size >= _memory_page_size)
      _rpmalloc_unmap(span, span_ଗଣନା * _memory_span_size, 0, 0);
  } ଅନ୍ୟଥା {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    span->flags |=
        SPAN_FLAG_MASTER | SPAN_FLAG_SUBSPAN | SPAN_FLAG_UNMAPPED_MASTER;
    _rpmalloc_stat_add(&_unmapped_master_spans, 1);
  }

  ଯଦି (atomic_add32(&master->remaining_spans, -(int32_t)span_ଗଣନା) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & SPAN_FLAG_MASTER) &&
                        !!(master->flags & SPAN_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_ଗଣନା = master->span_ଗଣନା;
    ଯଦି (_memory_span_size < _memory_page_size)
      unmap_ଗଣନା = master->total_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_ଗଣନା * _memory_span_size,
                    master->align_offset,
                    (size_t)master->total_spans * _memory_span_size);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_span_release_to_cache(heap_t *heap, span_t *span) {
  rpmalloc_assert(heap == span->heap, "Span heap pointer corrupted");
  rpmalloc_assert(span->size_class < SIZE_CLASS_ଗଣନା,
                  "Invalid span size class");
  rpmalloc_assert(span->span_ଗଣନା == 1, "Invalid span count");
#ଯଦି ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&heap->span_use[0].current);
#endif
  _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
  ଯଦି (!heap->finalize) {
    _rpmalloc_stat_inc(&heap->span_use[0].spans_to_cache);
    _rpmalloc_stat_inc(&heap->size_class_use[span->size_class].spans_to_cache);
    ଯଦି (heap->size_class[span->size_class].cache)
      rpmalloc_heap_cache_ଭର୍ତ୍ତି(heap,
                                  heap->size_class[span->size_class].cache);
    heap->size_class[span->size_class].cache = span;
  } ଅନ୍ୟଥା {
    _rpmalloc_span_unmap(span);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
ସ୍ଥିର uint32_t free_list_partial_init(ଶୂନ୍ୟ **list, ଶୂନ୍ୟ **first_block,
                                       ଶୂନ୍ୟ *page_start, ଶୂନ୍ୟ *block_start,
                                       uint32_t block_ଗଣନା,
                                       uint32_t block_size) {
  rpmalloc_assert(block_ଗଣନା, "Internal failure");
  *first_block = block_start;
  ଯଦି (block_ଗଣନା > 1) {
    ଶୂନ୍ୟ *free_block = pointer_offset(block_start, block_size);
    ଶୂନ୍ୟ *block_end =
        pointer_offset(block_start, (size_t)block_size * block_ଗଣନା);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    ଯଦି (block_size < (_memory_page_size >> 1)) {
      ଶୂନ୍ୟ *page_end = pointer_offset(page_start, _memory_page_size);
      ଯଦି (page_end < block_end)
        block_end = page_end;
    }
    *list = free_block;
    block_ଗଣନା = 2;
    ଶୂନ୍ୟ *next_block = pointer_offset(free_block, block_size);
    ଯେତେବେଳେ (next_block < block_end) {
      *((ଶୂନ୍ୟ **)free_block) = next_block;
      free_block = next_block;
      ++block_ଗଣନା;
      next_block = pointer_offset(next_block, block_size);
    }
    *((ଶୂନ୍ୟ **)free_block) = 0;
  } ଅନ୍ୟଥା {
    *list = 0;
  }
  ଫେରନ୍ତୁ block_ଗଣନା;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
ସ୍ଥିର ଶୂନ୍ୟ *_rpmalloc_span_initialize_new(heap_t *heap,
                                           heap_size_class_t *heap_size_class,
                                           span_t *span, uint32_t class_idx) {
  rpmalloc_assert(span->span_ଗଣନା == 1, "Internal failure");
  size_class_t *size_class = _memory_size_class + class_idx;
  span->size_class = class_idx;
  span->heap = heap;
  span->flags &= ~SPAN_FLAG_ALIGNED_BLOCKS;
  span->block_size = size_class->block_size;
  span->block_ଗଣନା = size_class->block_ଗଣନା;
  span->free_list = 0;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  ଶୂନ୍ୟ *block;
  span->free_list_limit =
      free_list_partial_init(&heap_size_class->free_list, &block, span,
                             pointer_offset(span, SPAN_HEADER_SIZE),
                             size_class->block_ଗଣନା, size_class->block_size);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  ଯଦି (span->free_list_limit < span->block_ଗଣନା) {
    _rpmalloc_span_double_link_list_add(&heap_size_class->partial_span, span);
    span->used_ଗଣନା = span->free_list_limit;
  } ଅନ୍ୟଥା {
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_ଗଣନା;
    span->used_ଗଣନା = span->block_ଗଣନା;
  }
  ଫେରନ୍ତୁ block;
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_span_extract_free_list_deferred(span_t *span) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  କର {
    span->free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } ଯେତେବେଳେ (span->free_list == INVALID_POINTER);
  span->used_ଗଣନା -= span->list_size;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା _rpmalloc_span_is_fully_utilized(span_t *span) {
  rpmalloc_assert(span->free_list_limit <= span->block_ଗଣନା,
                  "Span free list corrupted");
  ଫେରନ୍ତୁ !span->free_list && (span->free_list_limit >= span->block_ଗଣନା);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା _rpmalloc_span_finalize(heap_t *heap, size_t iclass, span_t *span,
                                   span_t **list_head) {
  ଶୂନ୍ୟ *free_list = heap->size_class[iclass].free_list;
  span_t *class_span = (span_t *)((uintptr_t)free_list & _memory_span_mask);
  ଯଦି (span == class_span) {
    // Adopt the heap class free list back into the span free list
    ଶୂନ୍ୟ *block = span->free_list;
    ଶୂନ୍ୟ *last_block = 0;
    ଯେତେବେଳେ (block) {
      last_block = block;
      block = *((ଶୂନ୍ୟ **)block);
    }
    uint32_t free_ଗଣନା = 0;
    block = free_list;
    ଯେତେବେଳେ (block) {
      ++free_ଗଣନା;
      block = *((ଶୂନ୍ୟ **)block);
    }
    ଯଦି (last_block) {
      *((ଶୂନ୍ୟ **)last_block) = free_list;
    } ଅନ୍ୟଥା {
      span->free_list = free_list;
    }
    heap->size_class[iclass].free_list = 0;
    span->used_ଗଣନା -= free_ଗଣନା;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(span->list_size == span->used_ଗଣନା, "Memory leak detected");
  ଯଦି (span->list_size == span->used_ଗଣନା) {
    _rpmalloc_stat_dec(&heap->span_use[0].current);
    _rpmalloc_stat_dec(&heap->size_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    ଯଦି (list_head)
      _rpmalloc_span_double_link_list_remove(list_head, span);
    _rpmalloc_span_unmap(span);
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
}

////////////
///
/// Global cache
///
//////

#ଯଦି ENABLE_GLOBAL_CACHE

//! Finalize a global cache
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  ଯେତେବେଳେ (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

  ପାଇଁ (size_t ispan = 0; ispan < cache->ଗଣନା_2; ++ispan)
    _rpmalloc_span_unmap(cache->span[ispan]);
  cache->ଗଣନା_2 = 0;

  ଯେତେବେଳେ (cache->overflow) {
    span_t *span = cache->overflow;
    cache->overflow = span->next;
    _rpmalloc_span_unmap(span);
  }

  atomic_store32_release(&cache->lock, 0);
}

ସ୍ଥିର ଶୂନ୍ୟ rpmalloc_global_cache_ଭର୍ତ୍ତି_spans(span_t **span,
                                                size_t span_ଗଣନା,
                                                size_t ଗଣନା_2) {
  ଅଚଳ size_t cache_limit =
      (span_ଗଣନା == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_SPAN_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_SPAN_LARGE_CACHE - (span_ଗଣନା >> 1));

  global_cache_t *cache = &_memory_span_cache[span_ଗଣନା - 1];

  size_t ଭର୍ତ୍ତି_ଗଣନା = ଗଣନା_2;
  ଯେତେବେଳେ (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#ଯଦି ENABLE_STATISTICS
  cache->ଭର୍ତ୍ତି_ଗଣନା += ଗଣନା_2;
#endif
  ଯଦି ((cache->ଗଣନା_2 + ଭର୍ତ୍ତି_ଗଣନା) > cache_limit)
    ଭର୍ତ୍ତି_ଗଣନା = cache_limit - cache->ଗଣନା_2;

  memcpy(cache->span + cache->ଗଣନା_2, span, ଆକାର(span_t *) * ଭର୍ତ୍ତି_ଗଣନା);
  cache->ଗଣନା_2 += (uint32_t)ଭର୍ତ୍ତି_ଗଣନା;

#ଯଦି ENABLE_UNLIMITED_CACHE
  ଯେତେବେଳେ (ଭର୍ତ୍ତି_ଗଣନା < ଗଣନା_2) {
#ଅନ୍ୟଥା
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  ଯେତେବେଳେ ((_memory_page_size > _memory_span_size) && (ଭର୍ତ୍ତି_ଗଣନା < ଗଣନା_2)) {
#endif
    span_t *current_span = span[ଭର୍ତ୍ତି_ଗଣନା++];
    current_span->next = cache->overflow;
    cache->overflow = current_span;
  }
  atomic_store32_release(&cache->lock, 0);

  span_t *keep = 0;
  ପାଇଁ (size_t ispan = ଭର୍ତ୍ତି_ଗଣନା; ispan < ଗଣନା_2; ++ispan) {
    span_t *current_span = span[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    ଯଦି ((current_span->flags & SPAN_FLAG_MASTER) &&
        (atomic_load32(&current_span->remaining_spans) >
         (int32_t)current_span->span_ଗଣନା)) {
      current_span->next = keep;
      keep = current_span;
    } ଅନ୍ୟଥା {
      _rpmalloc_span_unmap(current_span);
    }
  }

  ଯଦି (keep) {
    ଯେତେବେଳେ (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    ଯେତେବେଳେ (keep) {
      ପାଇଁ (; islot < cache->ଗଣନା_2; ++islot) {
        span_t *current_span = cache->span[islot];
        ଯଦି (!(current_span->flags & SPAN_FLAG_MASTER) ||
            ((current_span->flags & SPAN_FLAG_MASTER) &&
             (atomic_load32(&current_span->remaining_spans) <=
              (int32_t)current_span->span_ଗଣନା))) {
          _rpmalloc_span_unmap(current_span);
          cache->span[islot] = keep;
          ଭାଙ୍ଗ;
        }
      }
      ଯଦି (islot == cache->ଗଣନା_2)
        ଭାଙ୍ଗ;
      keep = keep->next;
    }

    ଯଦି (keep) {
      span_t *tail = keep;
      ଯେତେବେଳେ (tail->next)
        tail = tail->next;
      tail->next = cache->overflow;
      cache->overflow = keep;
    }

    atomic_store32_release(&cache->lock, 0);
  }
}

ସ୍ଥିର size_t _rpmalloc_global_cache_extract_spans(span_t **span,
                                                   size_t span_ଗଣନା,
                                                   size_t ଗଣନା_2) {
  global_cache_t *cache = &_memory_span_cache[span_ଗଣନା - 1];

  size_t extract_ଗଣନା = 0;
  ଯେତେବେଳେ (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#ଯଦି ENABLE_STATISTICS
  cache->extract_ଗଣନା += ଗଣନା_2;
#endif
  size_t want = ଗଣନା_2 - extract_ଗଣନା;
  ଯଦି (want > cache->ଗଣନା_2)
    want = cache->ଗଣନା_2;

  memcpy(span + extract_ଗଣନା, cache->span + (cache->ଗଣନା_2 - want),
         ଆକାର(span_t *) * want);
  cache->ଗଣନା_2 -= (uint32_t)want;
  extract_ଗଣନା += want;

  ଯେତେବେଳେ ((extract_ଗଣନା < ଗଣନା_2) && cache->overflow) {
    span_t *current_span = cache->overflow;
    span[extract_ଗଣନା++] = current_span;
    cache->overflow = current_span->next;
  }

#ଯଦି ENABLE_ASSERTS
  ପାଇଁ (size_t ispan = 0; ispan < extract_ଗଣନା; ++ispan) {
    rpmalloc_assert(span[ispan]->span_ଗଣନା == span_ଗଣନା,
                    "Global cache span count mismatch");
  }
#endif

  atomic_store32_release(&cache->lock, 0);

  ଫେରନ୍ତୁ extract_ଗଣନା;
}

#endif

////////////
///
/// Heap control
///
//////

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_deallocate_huge(span_t *);

//! Store the given spans as reserve in the given heap
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_ଗଣନା) {
  heap->span_reserve_master = master;
  heap->span_reserve = reserve;
  heap->spans_reserved = (uint32_t)reserve_span_ଗଣନା;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_heap_cache_adopt_deferred(heap_t *heap,
                                                span_t **single_span) {
  span_t *span = (span_t *)((ଶୂନ୍ୟ *)atomic_exchange_ptr_acquire(
      &heap->span_free_deferred, 0));
  ଯେତେବେଳେ (span) {
    span_t *next_span = (span_t *)span->free_list;
    rpmalloc_assert(span->heap == heap, "Span heap pointer corrupted");
    ଯଦି (EXPECTED(span->size_class < SIZE_CLASS_ଗଣନା)) {
      rpmalloc_assert(heap->full_span_ଗଣନା, "Heap span counter corrupted");
      --heap->full_span_ଗଣନା;
      _rpmalloc_stat_dec(&heap->span_use[0].spans_deferred);
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
      _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                             span);
#endif
      _rpmalloc_stat_dec(&heap->span_use[0].current);
      _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
      ଯଦି (single_span && !*single_span)
        *single_span = span;
      ଅନ୍ୟଥା
        rpmalloc_heap_cache_ଭର୍ତ୍ତି(heap, span);
    } ଅନ୍ୟଥା {
      ଯଦି (span->size_class == SIZE_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(span);
      } ଅନ୍ୟଥା {
        rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(heap->full_span_ଗଣନା, "Heap span counter corrupted");
        --heap->full_span_ଗଣନା;
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
        _rpmalloc_span_double_link_list_remove(&heap->large_huge_span, span);
#endif
        uint32_t idx = span->span_ଗଣନା - 1;
        _rpmalloc_stat_dec(&heap->span_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&heap->span_use[idx].current);
        ଯଦି (!idx && single_span && !*single_span)
          *single_span = span;
        ଅନ୍ୟଥା
          rpmalloc_heap_cache_ଭର୍ତ୍ତି(heap, span);
      }
    }
    span = next_span;
  }
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_heap_unmap(heap_t *heap) {
  ଯଦି (!heap->master_heap) {
    ଯଦି ((heap->finalize > 1) && !atomic_load32(&heap->child_ଗଣନା)) {
      span_t *span = (span_t *)((uintptr_t)heap & _memory_span_mask);
      _rpmalloc_span_unmap(span);
    }
  } ଅନ୍ୟଥା {
    ଯଦି (atomic_decr32(&heap->master_heap->child_ଗଣନା) == 0) {
      _rpmalloc_heap_unmap(heap->master_heap);
    }
  }
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_heap_global_finalize(heap_t *heap) {
  ଯଦି (heap->finalize++ > 1) {
    --heap->finalize;
    ଫେରନ୍ତୁ;
  }

  _rpmalloc_heap_finalize(heap);

#ଯଦି ENABLE_THREAD_CACHE
  ପାଇଁ (size_t iclass = 0; iclass < LARGE_CLASS_ଗଣନା; ++iclass) {
    span_cache_t *span_cache;
    ଯଦି (!iclass)
      span_cache = &heap->span_cache;
    ଅନ୍ୟଥା
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    ପାଇଁ (size_t ispan = 0; ispan < span_cache->ଗଣନା_2; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->ଗଣନା_2 = 0;
  }
#endif

  ଯଦି (heap->full_span_ଗଣନା) {
    --heap->finalize;
    ଫେରନ୍ତୁ;
  }

  ପାଇଁ (size_t iclass = 0; iclass < SIZE_CLASS_ଗଣନା; ++iclass) {
    ଯଦି (heap->size_class[iclass].free_list ||
        heap->size_class[iclass].partial_span) {
      --heap->finalize;
      ଫେରନ୍ତୁ;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap_t *list_heap = _memory_heaps[list_idx];
  ଯଦି (list_heap == heap) {
    _memory_heaps[list_idx] = heap->next_heap;
  } ଅନ୍ୟଥା {
    ଯେତେବେଳେ (list_heap->next_heap != heap)
      list_heap = list_heap->next_heap;
    list_heap->next_heap = heap->next_heap;
  }

  _rpmalloc_heap_unmap(heap);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
ସ୍ଥିର ଶୂନ୍ୟ rpmalloc_heap_cache_ଭର୍ତ୍ତି(heap_t *heap, span_t *span) {
  ଯଦି (UNEXPECTED(heap->finalize != 0)) {
    _rpmalloc_span_unmap(span);
    _rpmalloc_heap_global_finalize(heap);
    ଫେରନ୍ତୁ;
  }
#ଯଦି ENABLE_THREAD_CACHE
  size_t span_ଗଣନା = span->span_ଗଣନା;
  _rpmalloc_stat_inc(&heap->span_use[span_ଗଣନା - 1].spans_to_cache);
  ଯଦି (span_ଗଣନା == 1) {
    span_cache_t *span_cache = &heap->span_cache;
    span_cache->span[span_cache->ଗଣନା_2++] = span;
    ଯଦି (span_cache->ଗଣନା_2 == MAX_THREAD_SPAN_CACHE) {
      ଅଚଳ size_t remain_ଗଣନା =
          MAX_THREAD_SPAN_CACHE - THREAD_SPAN_CACHE_TRANSFER;
#ଯଦି ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           THREAD_SPAN_CACHE_TRANSFER * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_ଗଣନା - 1].spans_to_global,
                         THREAD_SPAN_CACHE_TRANSFER);
      rpmalloc_global_cache_ଭର୍ତ୍ତି_spans(span_cache->span + remain_ଗଣନା,
                                          span_ଗଣନା,
                                          THREAD_SPAN_CACHE_TRANSFER);
#ଅନ୍ୟଥା
      ପାଇଁ (size_t ispan = 0; ispan < THREAD_SPAN_CACHE_TRANSFER; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_ଗଣନା + ispan]);
#endif
      span_cache->ଗଣନା_2 = remain_ଗଣନା;
    }
  } ଅନ୍ୟଥା {
    size_t cache_idx = span_ଗଣନା - 2;
    span_large_cache_t *span_cache = heap->span_large_cache + cache_idx;
    span_cache->span[span_cache->ଗଣନା_2++] = span;
    ଅଚଳ size_t cache_limit =
        (MAX_THREAD_SPAN_LARGE_CACHE - (span_ଗଣନା >> 1));
    ଯଦି (span_cache->ଗଣନା_2 == cache_limit) {
      ଅଚଳ size_t transfer_limit = 2 + (cache_limit >> 2);
      ଅଚଳ size_t transfer_ଗଣନା =
          (THREAD_SPAN_LARGE_CACHE_TRANSFER <= transfer_limit
               ? THREAD_SPAN_LARGE_CACHE_TRANSFER
               : transfer_limit);
      ଅଚଳ size_t remain_ଗଣନା = cache_limit - transfer_ଗଣନା;
#ଯଦି ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           transfer_ଗଣନା * span_ଗଣନା * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_ଗଣନା - 1].spans_to_global,
                         transfer_ଗଣନା);
      rpmalloc_global_cache_ଭର୍ତ୍ତି_spans(span_cache->span + remain_ଗଣନା,
                                          span_ଗଣନା, transfer_ଗଣନା);
#ଅନ୍ୟଥା
      ପାଇଁ (size_t ispan = 0; ispan < transfer_ଗଣନା; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_ଗଣନା + ispan]);
#endif
      span_cache->ଗଣନା_2 = remain_ଗଣନା;
    }
  }
#ଅନ୍ୟଥା
  (ଶୂନ୍ୟ)ଆକାର(heap);
  _rpmalloc_span_unmap(span);
#endif
}

//! Extract the given number of spans from the different cache levels
ସ୍ଥିର span_t *_rpmalloc_heap_thread_cache_extract(heap_t *heap,
                                                   size_t span_ଗଣନା) {
  span_t *span = 0;
#ଯଦି ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  ଯଦି (span_ଗଣନା == 1)
    span_cache = &heap->span_cache;
  ଅନ୍ୟଥା
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_ଗଣନା - 2));
  ଯଦି (span_cache->ଗଣନା_2) {
    _rpmalloc_stat_inc(&heap->span_use[span_ଗଣନା - 1].spans_from_cache);
    ଫେରନ୍ତୁ span_cache->span[--span_cache->ଗଣନା_2];
  }
#endif
  ଫେରନ୍ତୁ span;
}

ସ୍ଥିର span_t *_rpmalloc_heap_thread_cache_deferred_extract(heap_t *heap,
                                                            size_t span_ଗଣନା) {
  span_t *span = 0;
  ଯଦି (span_ଗଣନା == 1) {
    _rpmalloc_heap_cache_adopt_deferred(heap, &span);
  } ଅନ୍ୟଥା {
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
    span = _rpmalloc_heap_thread_cache_extract(heap, span_ଗଣନା);
  }
  ଫେରନ୍ତୁ span;
}

ସ୍ଥିର span_t *_rpmalloc_heap_reserved_extract(heap_t *heap,
                                               size_t span_ଗଣନା) {
  ଯଦି (heap->spans_reserved >= span_ଗଣନା)
    ଫେରନ୍ତୁ _rpmalloc_span_map(heap, span_ଗଣନା);
  ଫେରନ୍ତୁ 0;
}

//! Extract a span from the global cache
ସ୍ଥିର span_t *_rpmalloc_heap_global_cache_extract(heap_t *heap,
                                                   size_t span_ଗଣନା) {
#ଯଦି ENABLE_GLOBAL_CACHE
#ଯଦି ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  size_t wanted_ଗଣନା;
  ଯଦି (span_ଗଣନା == 1) {
    span_cache = &heap->span_cache;
    wanted_ଗଣନା = THREAD_SPAN_CACHE_TRANSFER;
  } ଅନ୍ୟଥା {
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_ଗଣନା - 2));
    wanted_ଗଣନା = THREAD_SPAN_LARGE_CACHE_TRANSFER;
  }
  span_cache->ଗଣନା_2 = _rpmalloc_global_cache_extract_spans(
      span_cache->span, span_ଗଣନା, wanted_ଗଣନା);
  ଯଦି (span_cache->ଗଣନା_2) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_ଗଣନା * span_cache->ଗଣନା_2 * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_ଗଣନା - 1].spans_from_global,
                       span_cache->ଗଣନା_2);
    ଫେରନ୍ତୁ span_cache->span[--span_cache->ଗଣନା_2];
  }
#ଅନ୍ୟଥା
  span_t *span = 0;
  size_t ଗଣନା_2 = _rpmalloc_global_cache_extract_spans(&span, span_ଗଣନା, 1);
  ଯଦି (ଗଣନା_2) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_ଗଣନା * ଗଣନା_2 * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_ଗଣନା - 1].spans_from_global,
                       ଗଣନା_2);
    ଫେରନ୍ତୁ span;
  }
#endif
#endif
  (ଶୂନ୍ୟ)ଆକାର(heap);
  (ଶୂନ୍ୟ)ଆକାର(span_ଗଣନା);
  ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_inc_span_statistics(heap_t *heap, size_t span_ଗଣନା,
                                          uint32_t class_idx) {
  (ଶୂନ୍ୟ)ଆକାର(heap);
  (ଶୂନ୍ୟ)ଆକାର(span_ଗଣନା);
  (ଶୂନ୍ୟ)ଆକାର(class_idx);
#ଯଦି ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)span_ଗଣନା - 1;
  uint32_t current_ଗଣନା =
      (uint32_t)atomic_incr32(&heap->span_use[idx].current);
  ଯଦି (current_ଗଣନା > (uint32_t)atomic_load32(&heap->span_use[idx].ଉପର))
    atomic_store32(&heap->span_use[idx].ଉପର, (int32_t)current_ଗଣନା);
  _rpmalloc_stat_add_peak(&heap->size_class_use[class_idx].spans_current, 1,
                          heap->size_class_use[class_idx].spans_peak);
#endif
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
ସ୍ଥିର span_t *
_rpmalloc_heap_extract_new_span(heap_t *heap,
                                heap_size_class_t *heap_size_class,
                                size_t span_ଗଣନା, uint32_t class_idx) {
  span_t *span;
#ଯଦି ENABLE_THREAD_CACHE
  ଯଦି (heap_size_class && heap_size_class->cache) {
    span = heap_size_class->cache;
    heap_size_class->cache =
        (heap->span_cache.ଗଣନା_2
             ? heap->span_cache.span[--heap->span_cache.ଗଣନା_2]
             : 0);
    _rpmalloc_inc_span_statistics(heap, span_ଗଣନା, class_idx);
    ଫେରନ୍ତୁ span;
  }
#endif
  (ଶୂନ୍ୟ)ଆକାର(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t base_span_ଗଣନା = span_ଗଣନା;
  size_t limit_span_ଗଣନା =
      (span_ଗଣନା > 2) ? (span_ଗଣନା + (span_ଗଣନା >> 1)) : span_ଗଣନା;
  ଯଦି (limit_span_ଗଣନା > LARGE_CLASS_ଗଣନା)
    limit_span_ଗଣନା = LARGE_CLASS_ଗଣନା;
  କର {
    span = _rpmalloc_heap_thread_cache_extract(heap, span_ଗଣନା);
    ଯଦି (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_ଗଣନା, class_idx);
      ଫେରନ୍ତୁ span;
    }
    span = _rpmalloc_heap_thread_cache_deferred_extract(heap, span_ଗଣନା);
    ଯଦି (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_ଗଣନା, class_idx);
      ଫେରନ୍ତୁ span;
    }
    span = _rpmalloc_heap_global_cache_extract(heap, span_ଗଣନା);
    ଯଦି (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_ଗଣନା, class_idx);
      ଫେରନ୍ତୁ span;
    }
    span = _rpmalloc_heap_reserved_extract(heap, span_ଗଣନା);
    ଯଦି (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_reserved);
      _rpmalloc_inc_span_statistics(heap, span_ଗଣନା, class_idx);
      ଫେରନ୍ତୁ span;
    }
    ++span_ଗଣନା;
  } ଯେତେବେଳେ (span_ଗଣନା <= limit_span_ଗଣନା);
  // Final fallback, map in more virtual memory
  span = _rpmalloc_span_map(heap, base_span_ଗଣନା);
  _rpmalloc_inc_span_statistics(heap, base_span_ଗଣନା, class_idx);
  _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_map_calls);
  ଫେରନ୍ତୁ span;
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_heap_initialize(heap_t *heap) {
  _rpmalloc_memset_const(heap, 0, ଆକାର(heap_t));
  // Get a new heap ID
  heap->id = 1 + atomic_incr32(&_memory_heap_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap->next_heap = _memory_heaps[list_idx];
  _memory_heaps[list_idx] = heap;
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_heap_orphan(heap_t *heap, ପୂର୍ଣ୍ଣସଂଖ୍ୟା first_class) {
  heap->owner_thread = (uintptr_t)-1;
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
  heap_t **heap_list =
      (first_class ? &_memory_first_class_orphan_heaps : &_memory_orphan_heaps);
#ଅନ୍ୟଥା
  (ଶୂନ୍ୟ)ଆକାର(first_class);
  heap_t **heap_list = &_memory_orphan_heaps;
#endif
  heap->next_orphan = *heap_list;
  *heap_list = heap;
}

//! Allocate a new heap from newly mapped memory pages
ସ୍ଥିର heap_t *_rpmalloc_heap_allocate_new(ଶୂନ୍ୟ) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t heap_size = ଆକାର(heap_t);
  size_t aligned_heap_size = 16 * ((heap_size + 15) / 16);
  size_t request_heap_ଗଣନା = 16;
  size_t heap_span_ଗଣନା = ((aligned_heap_size * request_heap_ଗଣନା) +
                            ଆକାର(span_t) + _memory_span_size - 1) /
                           _memory_span_size;
  size_t block_size = _memory_span_size * heap_span_ଗଣନା;
  size_t span_ଗଣନା = heap_span_ଗଣନା;
  span_t *span = 0;
  // If there are global reserved spans, use these first
  ଯଦି (memory_global_reserve_ଗଣନା >= heap_span_ଗଣନା) {
    span = _rpmalloc_global_get_reserved_spans(heap_span_ଗଣନା);
  }
  ଯଦି (!span) {
    ଯଦି (_memory_page_size > block_size) {
      span_ଗଣନା = _memory_page_size / _memory_span_size;
      block_size = _memory_page_size;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_heap_ଗଣନା =
          (block_size - ଆକାର(span_t)) / aligned_heap_size;
      ଯଦି (possible_heap_ଗଣନା >= (request_heap_ଗଣନା * 16))
        request_heap_ଗଣନା *= 16;
      ଅନ୍ୟଥା ଯଦି (possible_heap_ଗଣନା < request_heap_ଗଣନା)
        request_heap_ଗଣନା = possible_heap_ଗଣନା;
      heap_span_ଗଣନା = ((aligned_heap_size * request_heap_ଗଣନା) +
                         ଆକାର(span_t) + _memory_span_size - 1) /
                        _memory_span_size;
    }

    size_t align_offset = 0;
    span = (span_t *)_rpmalloc_mmap(block_size, &align_offset);
    ଯଦି (!span)
      ଫେରନ୍ତୁ 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    _rpmalloc_span_initialize(span, span_ଗଣନା, heap_span_ଗଣନା, align_offset);
  }

  size_t remain_size = _memory_span_size - ଆକାର(span_t);
  heap_t *heap = (heap_t *)pointer_offset(span, ଆକାର(span_t));
  _rpmalloc_heap_initialize(heap);

  // Put extra heaps as orphans
  size_t ଅଙ୍କ_heaps = remain_size / aligned_heap_size;
  ଯଦି (ଅଙ୍କ_heaps < request_heap_ଗଣନା)
    ଅଙ୍କ_heaps = request_heap_ଗଣନା;
  atomic_store32(&heap->child_ଗଣନା, (int32_t)ଅଙ୍କ_heaps - 1);
  heap_t *extra_heap = (heap_t *)pointer_offset(heap, aligned_heap_size);
  ଯେତେବେଳେ (ଅଙ୍କ_heaps > 1) {
    _rpmalloc_heap_initialize(extra_heap);
    extra_heap->master_heap = heap;
    _rpmalloc_heap_orphan(extra_heap, 1);
    extra_heap = (heap_t *)pointer_offset(extra_heap, aligned_heap_size);
    --ଅଙ୍କ_heaps;
  }

  ଯଦି (span_ଗଣନା > heap_span_ଗଣନା) {
    // Cap reserved spans
    size_t remain_ଗଣନା = span_ଗଣନା - heap_span_ଗଣନା;
    size_t reserve_ଗଣନା =
        (remain_ଗଣନା > memory_heap_reserve_ଗଣନା ? memory_heap_reserve_ଗଣନା
                                                   : remain_ଗଣନା);
    span_t *remain_span =
        (span_t *)pointer_offset(span, heap_span_ଗଣନା * _memory_span_size);
    _rpmalloc_heap_set_reserved_spans(heap, span, remain_span, reserve_ଗଣନା);

    ଯଦି (remain_ଗଣନା > reserve_ଗଣନା) {
      // Set to global reserved spans
      remain_span = (span_t *)pointer_offset(remain_span,
                                             reserve_ଗଣନା * _memory_span_size);
      reserve_ଗଣନା = remain_ଗଣନା - reserve_ଗଣନା;
      _rpmalloc_global_set_reserved_spans(span, remain_span, reserve_ଗଣନା);
    }
  }

  ଫେରନ୍ତୁ heap;
}

ସ୍ଥିର heap_t *_rpmalloc_heap_extract_orphan(heap_t **heap_list) {
  heap_t *heap = *heap_list;
  *heap_list = (heap ? heap->next_orphan : 0);
  ଫେରନ୍ତୁ heap;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
ସ୍ଥିର heap_t *_rpmalloc_heap_allocate(ପୂର୍ଣ୍ଣସଂଖ୍ୟା first_class) {
  heap_t *heap = 0;
  ଯେତେବେଳେ (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
    _rpmalloc_spin();
  ଯଦି (first_class == 0)
    heap = _rpmalloc_heap_extract_orphan(&_memory_orphan_heaps);
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
  ଯଦି (!heap)
    heap = _rpmalloc_heap_extract_orphan(&_memory_first_class_orphan_heaps);
#endif
  ଯଦି (!heap)
    heap = _rpmalloc_heap_allocate_new();
  atomic_store32_release(&_memory_global_lock, 0);
  ଯଦି (heap)
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  ଫେରନ୍ତୁ heap;
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_heap_release(ଶୂନ୍ୟ *heapptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା first_class,
                                   ପୂର୍ଣ୍ଣସଂଖ୍ୟା release_cache) {
  heap_t *heap = (heap_t *)heapptr;
  ଯଦି (!heap)
    ଫେରନ୍ତୁ;
  // Release thread cache spans back to global cache
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  ଯଦି (release_cache || heap->finalize) {
#ଯଦି ENABLE_THREAD_CACHE
    ପାଇଁ (size_t iclass = 0; iclass < LARGE_CLASS_ଗଣନା; ++iclass) {
      span_cache_t *span_cache;
      ଯଦି (!iclass)
        span_cache = &heap->span_cache;
      ଅନ୍ୟଥା
        span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
      ଯଦି (!span_cache->ଗଣନା_2)
        ଜାରି;
#ଯଦି ENABLE_GLOBAL_CACHE
      ଯଦି (heap->finalize) {
        ପାଇଁ (size_t ispan = 0; ispan < span_cache->ଗଣନା_2; ++ispan)
          _rpmalloc_span_unmap(span_cache->span[ispan]);
      } ଅନ୍ୟଥା {
        _rpmalloc_stat_add64(&heap->thread_to_global, span_cache->ଗଣନା_2 *
                                                          (iclass + 1) *
                                                          _memory_span_size);
        _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                           span_cache->ଗଣନା_2);
        rpmalloc_global_cache_ଭର୍ତ୍ତି_spans(span_cache->span, iclass + 1,
                                            span_cache->ଗଣନା_2);
      }
#ଅନ୍ୟଥା
      ପାଇଁ (size_t ispan = 0; ispan < span_cache->ଗଣନା_2; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
      span_cache->ଗଣନା_2 = 0;
    }
#endif
  }

  ଯଦି (get_thread_heap_raw() == heap)
    set_thread_heap(0);

#ଯଦି ENABLE_STATISTICS
  atomic_decr32(&_memory_active_heaps);
  rpmalloc_assert(atomic_load32(&_memory_active_heaps) >= 0,
                  "Still active heaps during finalization");
#endif

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  ଯଦି (get_thread_id() != _rpmalloc_main_thread_id) {
    ଯେତେବେଳେ (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
  }
  _rpmalloc_heap_orphan(heap, first_class);
  atomic_store32_release(&_memory_global_lock, 0);
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_heap_release_raw(ଶୂନ୍ୟ *heapptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା release_cache) {
  _rpmalloc_heap_release(heapptr, 0, release_cache);
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_heap_release_raw_fc(ଶୂନ୍ୟ *heapptr) {
  _rpmalloc_heap_release_raw(heapptr, 1);
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_heap_finalize(heap_t *heap) {
  ଯଦି (heap->spans_reserved) {
    span_t *span = _rpmalloc_span_map(heap, heap->spans_reserved);
    _rpmalloc_span_unmap(span);
    heap->spans_reserved = 0;
  }

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  ପାଇଁ (size_t iclass = 0; iclass < SIZE_CLASS_ଗଣନା; ++iclass) {
    ଯଦି (heap->size_class[iclass].cache)
      _rpmalloc_span_unmap(heap->size_class[iclass].cache);
    heap->size_class[iclass].cache = 0;
    span_t *span = heap->size_class[iclass].partial_span;
    ଯେତେବେଳେ (span) {
      span_t *next = span->next;
      _rpmalloc_span_finalize(heap, iclass, span,
                              &heap->size_class[iclass].partial_span);
      span = next;
    }
    // If class still has a free list it must be a full span
    ଯଦି (heap->size_class[iclass].free_list) {
      span_t *class_span =
          (span_t *)((uintptr_t)heap->size_class[iclass].free_list &
                     _memory_span_mask);
      span_t **list = 0;
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
      list = &heap->full_span[iclass];
#endif
      --heap->full_span_ଗଣନା;
      ଯଦି (!_rpmalloc_span_finalize(heap, iclass, class_span, list)) {
        ଯଦି (list)
          _rpmalloc_span_double_link_list_remove(list, class_span);
        _rpmalloc_span_double_link_list_add(
            &heap->size_class[iclass].partial_span, class_span);
      }
    }
  }

#ଯଦି ENABLE_THREAD_CACHE
  ପାଇଁ (size_t iclass = 0; iclass < LARGE_CLASS_ଗଣନା; ++iclass) {
    span_cache_t *span_cache;
    ଯଦି (!iclass)
      span_cache = &heap->span_cache;
    ଅନ୍ୟଥା
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    ପାଇଁ (size_t ispan = 0; ispan < span_cache->ଗଣନା_2; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->ଗଣନା_2 = 0;
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
ସ୍ଥିର ଶୂନ୍ୟ *free_list_କାଢ଼(ଶୂନ୍ୟ **list) {
  ଶୂନ୍ୟ *block = *list;
  *list = *((ଶୂନ୍ୟ **)block);
  ଫେରନ୍ତୁ block;
}

//! Allocate a small/medium sized memory block from the given heap
ସ୍ଥିର ଶୂନ୍ୟ *_rpmalloc_allocate_from_heap_fallback(
    heap_t *heap, heap_size_class_t *heap_size_class, uint32_t class_idx) {
  span_t *span = heap_size_class->partial_span;
  rpmalloc_assume(heap != 0);
  ଯଦି (EXPECTED(span != 0)) {
    rpmalloc_assert(span->block_ଗଣନା ==
                        _memory_size_class[span->size_class].block_ଗଣନା,
                    "Span block count corrupted");
    rpmalloc_assert(!_rpmalloc_span_is_fully_utilized(span),
                    "Internal failure");
    ଶୂନ୍ୟ *block;
    ଯଦି (span->free_list) {
      // Span local free list is not empty, swap to size class free list
      block = free_list_କାଢ଼(&span->free_list);
      heap_size_class->free_list = span->free_list;
      span->free_list = 0;
    } ଅନ୍ୟଥା {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      ଶୂନ୍ୟ *block_start = pointer_offset(
          span, SPAN_HEADER_SIZE +
                    ((size_t)span->free_list_limit * span->block_size));
      span->free_list_limit += free_list_partial_init(
          &heap_size_class->free_list, &block,
          (ଶୂନ୍ୟ *)((uintptr_t)block_start & ~(_memory_page_size - 1)),
          block_start, span->block_ଗଣନା - span->free_list_limit,
          span->block_size);
    }
    rpmalloc_assert(span->free_list_limit <= span->block_ଗଣନା,
                    "Span block count corrupted");
    span->used_ଗଣନା = span->free_list_limit;

    // Swap in deferred free list if present
    ଯଦି (atomic_load_ptr(&span->free_list_deferred))
      _rpmalloc_span_extract_free_list_deferred(span);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    ଯଦି (!_rpmalloc_span_is_fully_utilized(span))
      ଫେରନ୍ତୁ block;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_span_double_link_list_କାଢ଼_head(&heap_size_class->partial_span,
                                             span);
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_ଗଣନା;
    ଫେରନ୍ତୁ block;
  }

  // Find a span in one of the cache levels
  span = _rpmalloc_heap_extract_new_span(heap, heap_size_class, 1, class_idx);
  ଯଦି (EXPECTED(span != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    ଫେରନ୍ତୁ _rpmalloc_span_initialize_new(heap, heap_size_class, span,
                                         class_idx);
  }

  ଫେରନ୍ତୁ 0;
}

//! Allocate a small sized memory block from the given heap
ସ୍ଥିର ଶୂନ୍ୟ *_rpmalloc_allocate_small(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Small sizes have unique size classes
  ଅଚଳ uint32_t class_idx =
      (uint32_t)((size + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_SHIFT);
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  ଯଦି (EXPECTED(heap_size_class->free_list != 0))
    ଫେରନ୍ତୁ free_list_କାଢ଼(&heap_size_class->free_list);
  ଫେରନ୍ତୁ _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
ସ୍ଥିର ଶୂନ୍ୟ *_rpmalloc_allocate_medium(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ଅଚଳ uint32_t base_idx =
      (uint32_t)(SMALL_CLASS_ଗଣନା +
                 ((size - (SMALL_SIZE_LIMIT + 1)) >> MEDIUM_GRANULARITY_SHIFT));
  ଅଚଳ uint32_t class_idx = _memory_size_class[base_idx].class_idx;
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  ଯଦି (EXPECTED(heap_size_class->free_list != 0))
    ଫେରନ୍ତୁ free_list_କାଢ଼(&heap_size_class->free_list);
  ଫେରନ୍ତୁ _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
ସ୍ଥିର ଶୂନ୍ୟ *_rpmalloc_allocate_large(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  size += SPAN_HEADER_SIZE;
  size_t span_ଗଣନା = size >> _memory_span_size_shift;
  ଯଦି (size & (_memory_span_size - 1))
    ++span_ଗଣନା;

  // Find a span in one of the cache levels
  span_t *span =
      _rpmalloc_heap_extract_new_span(heap, 0, span_ଗଣନା, SIZE_CLASS_LARGE);
  ଯଦି (!span)
    ଫେରନ୍ତୁ span;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(span->span_ଗଣନା >= span_ଗଣନା, "Internal failure");
  span->size_class = SIZE_CLASS_LARGE;
  span->heap = heap;

#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_ଗଣନା;

  ଫେରନ୍ତୁ pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a huge block by mapping memory pages directly
ସ୍ଥିର ଶୂନ୍ୟ *_rpmalloc_allocate_huge(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  size += SPAN_HEADER_SIZE;
  size_t ଅଙ୍କ_pages = size >> _memory_page_size_shift;
  ଯଦି (size & (_memory_page_size - 1))
    ++ଅଙ୍କ_pages;
  size_t align_offset = 0;
  span_t *span =
      (span_t *)_rpmalloc_mmap(ଅଙ୍କ_pages * _memory_page_size, &align_offset);
  ଯଦି (!span)
    ଫେରନ୍ତୁ span;

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_ଗଣନା = (uint32_t)ଅଙ୍କ_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, ଅଙ୍କ_pages, _huge_pages_peak);

#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_ଗଣନା;

  ଫେରନ୍ତୁ pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a block of the given size
ସ୍ଥିର ଶୂନ୍ୟ *_rpmalloc_allocate(heap_t *heap, size_t size) {
  _rpmalloc_stat_add64(&_allocation_counter, 1);
  ଯଦି (EXPECTED(size <= SMALL_SIZE_LIMIT))
    ଫେରନ୍ତୁ _rpmalloc_allocate_small(heap, size);
  ଅନ୍ୟଥା ଯଦି (size <= _memory_medium_size_limit)
    ଫେରନ୍ତୁ _rpmalloc_allocate_medium(heap, size);
  ଅନ୍ୟଥା ଯଦି (size <= LARGE_SIZE_LIMIT)
    ଫେରନ୍ତୁ _rpmalloc_allocate_large(heap, size);
  ଫେରନ୍ତୁ _rpmalloc_allocate_huge(heap, size);
}

ସ୍ଥିର ଶୂନ୍ୟ *_rpmalloc_aligned_allocate(heap_t *heap, size_t alignment,
                                        size_t size) {
  ଯଦି (alignment <= SMALL_GRANULARITY)
    ଫେରନ୍ତୁ _rpmalloc_allocate(heap, size);

#ଯଦି ENABLE_VALIDATE_ARGS
  ଯଦି ((size + alignment) < size) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି (alignment & (alignment - 1)) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#endif

  ଯଦି ((alignment <= SPAN_HEADER_SIZE) &&
      ((size + SPAN_HEADER_SIZE) < _memory_medium_size_limit)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t multiple_size = size ? (size + (SPAN_HEADER_SIZE - 1)) &
                                      ~(uintptr_t)(SPAN_HEADER_SIZE - 1)
                                : SPAN_HEADER_SIZE;
    rpmalloc_assert(!(multiple_size % SPAN_HEADER_SIZE),
                    "Failed alignment calculation");
    ଯଦି (multiple_size <= (size + alignment))
      ଫେରନ୍ତୁ _rpmalloc_allocate(heap, multiple_size);
  }

  ଶୂନ୍ୟ *ptr = 0;
  size_t align_mask = alignment - 1;
  ଯଦି (alignment <= _memory_page_size) {
    ptr = _rpmalloc_allocate(heap, size + alignment);
    ଯଦି ((uintptr_t)ptr & align_mask) {
      ptr = (ଶୂନ୍ୟ *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);
      // Mark as having aligned blocks
      span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
      span->flags |= SPAN_FLAG_ALIGNED_BLOCKS;
    }
    ଫେରନ୍ତୁ ptr;
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
  ଯଦି (alignment & align_mask) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି (alignment >= _memory_span_size) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }

  size_t extra_pages = alignment / _memory_page_size;

  // Since each span has a header, we will at least need one extra memory page
  size_t ଅଙ୍କ_pages = 1 + (size / _memory_page_size);
  ଯଦି (size & (_memory_page_size - 1))
    ++ଅଙ୍କ_pages;

  ଯଦି (extra_pages > ଅଙ୍କ_pages)
    ଅଙ୍କ_pages = 1 + extra_pages;

  size_t original_pages = ଅଙ୍କ_pages;
  size_t limit_pages = (_memory_span_size / _memory_page_size) * 2;
  ଯଦି (limit_pages < (original_pages * 2))
    limit_pages = original_pages * 2;

  size_t mapped_size, align_offset;
  span_t *span;

retry:
  align_offset = 0;
  mapped_size = ଅଙ୍କ_pages * _memory_page_size;

  span = (span_t *)_rpmalloc_mmap(mapped_size, &align_offset);
  ଯଦି (!span) {
    errno = ENOMEM;
    ଫେରନ୍ତୁ 0;
  }
  ptr = pointer_offset(span, SPAN_HEADER_SIZE);

  ଯଦି ((uintptr_t)ptr & align_mask)
    ptr = (ଶୂନ୍ୟ *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);

  ଯଦି (((size_t)pointer_diff(ptr, span) >= _memory_span_size) ||
      (pointer_offset(ptr, size) > pointer_offset(span, mapped_size)) ||
      (((uintptr_t)ptr & _memory_span_mask) != (uintptr_t)span)) {
    _rpmalloc_unmap(span, mapped_size, align_offset, mapped_size);
    ++ଅଙ୍କ_pages;
    ଯଦି (ଅଙ୍କ_pages > limit_pages) {
      errno = EINVAL;
      ଫେରନ୍ତୁ 0;
    }
    ଯାଅ retry;
  }

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_ଗଣନା = (uint32_t)ଅଙ୍କ_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, ଅଙ୍କ_pages, _huge_pages_peak);

#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_ଗଣନା;

  _rpmalloc_stat_add64(&_allocation_counter, 1);

  ଫେରନ୍ତୁ ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_deallocate_direct_small_or_medium(span_t *span,
                                                        ଶୂନ୍ୟ *block) {
  heap_t *heap = span->heap;
  rpmalloc_assert(heap->owner_thread == get_thread_id() ||
                      !heap->owner_thread || heap->finalize,
                  "Internal failure");
  // Add block to free list
  ଯଦି (UNEXPECTED(_rpmalloc_span_is_fully_utilized(span))) {
    span->used_ଗଣନା = span->block_ଗଣନା;
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                           span);
#endif
    _rpmalloc_span_double_link_list_add(
        &heap->size_class[span->size_class].partial_span, span);
    --heap->full_span_ଗଣନା;
  }
  *((ଶୂନ୍ୟ **)block) = span->free_list;
  --span->used_ଗଣନା;
  span->free_list = block;
  ଯଦି (UNEXPECTED(span->used_ଗଣନା == span->list_size)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    ଯଦି (span->used_ଗଣନା) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      ଶୂନ୍ୟ *free_list;
      କର {
        free_list = atomic_exchange_ptr_acquire(&span->free_list_deferred,
                                                INVALID_POINTER);
      } ଯେତେବେଳେ (free_list == INVALID_POINTER);
      atomic_store_ptr_release(&span->free_list_deferred, free_list);
    }
    _rpmalloc_span_double_link_list_remove(
        &heap->size_class[span->size_class].partial_span, span);
    _rpmalloc_span_release_to_cache(heap, span);
  }
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_deallocate_defer_free_span(heap_t *heap, span_t *span) {
  ଯଦି (span->size_class != SIZE_CLASS_HUGE)
    _rpmalloc_stat_inc(&heap->span_use[span->span_ଗଣନା - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  କର {
    span->free_list = (ଶୂନ୍ୟ *)atomic_load_ptr(&heap->span_free_deferred);
  } ଯେତେବେଳେ (!atomic_cas_ptr(&heap->span_free_deferred, span, span->free_list));
}

//! Put the block in the deferred free list of the owning span
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_deallocate_defer_small_or_medium(span_t *span,
                                                       ଶୂନ୍ୟ *block) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  ଶୂନ୍ୟ *free_list;
  କର {
    free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } ଯେତେବେଳେ (free_list == INVALID_POINTER);
  *((ଶୂନ୍ୟ **)block) = free_list;
  uint32_t free_ଗଣନା = ++span->list_size;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା all_deferred_free = (free_ଗଣନା == span->block_ଗଣନା);
  atomic_store_ptr_release(&span->free_list_deferred, block);
  ଯଦି (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
  }
}

ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_deallocate_small_or_medium(span_t *span, ଶୂନ୍ୟ *p) {
  _rpmalloc_stat_inc_free(span->heap, span->size_class);
  ଯଦି (span->flags & SPAN_FLAG_ALIGNED_BLOCKS) {
    // Realign pointer to block start
    ଶୂନ୍ୟ *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
    p = pointer_offset(p, -(int32_t)(block_offset % span->block_size));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#ଅନ୍ୟଥା
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  ଯଦି (!defer)
    _rpmalloc_deallocate_direct_small_or_medium(span, p);
  ଅନ୍ୟଥା
    _rpmalloc_deallocate_defer_small_or_medium(span, p);
}

//! Deallocate the given large memory block to the current heap
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_deallocate_large(span_t *span) {
  rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#ଅନ୍ୟଥା
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  ଯଦି (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    ଫେରନ୍ତୁ;
  }
  rpmalloc_assert(span->heap->full_span_ଗଣନା, "Heap span counter corrupted");
  --span->heap->full_span_ଗଣନା;
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif
#ଯଦି ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = span->span_ଗଣନା - 1;
  atomic_decr32(&span->heap->span_use[idx].current);
#endif
  heap_t *heap = span->heap;
  rpmalloc_assert(heap, "No thread heap");
#ଯଦି ENABLE_THREAD_CACHE
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା set_as_reserved =
      ((span->span_ଗଣନା > 1) && (heap->span_cache.ଗଣନା_2 == 0) &&
       !heap->finalize && !heap->spans_reserved);
#ଅନ୍ୟଥା
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା set_as_reserved =
      ((span->span_ଗଣନା > 1) && !heap->finalize && !heap->spans_reserved);
#endif
  ଯଦି (set_as_reserved) {
    heap->span_reserve = span;
    heap->spans_reserved = span->span_ଗଣନା;
    ଯଦି (span->flags & SPAN_FLAG_MASTER) {
      heap->span_reserve_master = span;
    } ଅନ୍ୟଥା { // SPAN_FLAG_SUBSPAN
      span_t *master = (span_t *)pointer_offset(
          span,
          -(intptr_t)((size_t)span->offset_from_master * _memory_span_size));
      heap->span_reserve_master = master;
      rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->remaining_spans) >=
                          (int32_t)span->span_ଗଣନା,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&heap->span_use[idx].spans_to_reserved);
  } ଅନ୍ୟଥା {
    // Insert into cache list
    rpmalloc_heap_cache_ଭର୍ତ୍ତି(heap, span);
  }
}

//! Deallocate the given huge span
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_deallocate_huge(span_t *span) {
  rpmalloc_assert(span->heap, "No span heap");
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#ଅନ୍ୟଥା
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  ଯଦି (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    ଫେରନ୍ତୁ;
  }
  rpmalloc_assert(span->heap->full_span_ଗଣନା, "Heap span counter corrupted");
  --span->heap->full_span_ଗଣନା;
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif

  // Oversized allocation, page count is stored in span_count
  size_t ଅଙ୍କ_pages = span->span_ଗଣନା;
  _rpmalloc_unmap(span, ଅଙ୍କ_pages * _memory_page_size, span->align_offset,
                  ଅଙ୍କ_pages * _memory_page_size);
  _rpmalloc_stat_sub(&_huge_pages_current, ଅଙ୍କ_pages);
}

//! Deallocate the given block
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_deallocate(ଶୂନ୍ୟ *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  ଯଦି (UNEXPECTED(!span))
    ଫେରନ୍ତୁ;
  ଯଦି (EXPECTED(span->size_class < SIZE_CLASS_ଗଣନା))
    _rpmalloc_deallocate_small_or_medium(span, p);
  ଅନ୍ୟଥା ଯଦି (span->size_class == SIZE_CLASS_LARGE)
    _rpmalloc_deallocate_large(span);
  ଅନ୍ୟଥା
    _rpmalloc_deallocate_huge(span);
}

////////////
///
/// Reallocation entry points
///
//////

ସ୍ଥିର size_t _rpmalloc_usable_size(ଶୂନ୍ୟ *p);

//! Reallocate the given block to the given size
ସ୍ଥିର ଶୂନ୍ୟ *_rpmalloc_reallocate(heap_t *heap, ଶୂନ୍ୟ *p, size_t size,
                                  size_t oldsize, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags) {
  ଯଦି (p) {
    // Grab the span using guaranteed span alignment
    span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
    ଯଦି (EXPECTED(span->size_class < SIZE_CLASS_ଗଣନା)) {
      // Small/medium sized block
      rpmalloc_assert(span->span_ଗଣନା == 1, "Span counter corrupted");
      ଶୂନ୍ୟ *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
      uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
      uint32_t block_idx = block_offset / span->block_size;
      ଶୂନ୍ୟ *block =
          pointer_offset(blocks_start, (size_t)block_idx * span->block_size);
      ଯଦି (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)span->block_size - pointer_diff(p, block));
      ଯଦି ((size_t)span->block_size >= size) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ଯଦି ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        ଫେରନ୍ତୁ block;
      }
    } ଅନ୍ୟଥା ଯଦି (span->size_class == SIZE_CLASS_LARGE) {
      // Large block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t ଅଙ୍କ_spans = total_size >> _memory_span_size_shift;
      ଯଦି (total_size & (_memory_span_mask - 1))
        ++ଅଙ୍କ_spans;
      size_t current_spans = span->span_ଗଣନା;
      ଶୂନ୍ୟ *block = pointer_offset(span, SPAN_HEADER_SIZE);
      ଯଦି (!oldsize)
        oldsize = (current_spans * _memory_span_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      ଯଦି ((current_spans >= ଅଙ୍କ_spans) && (total_size >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ଯଦି ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        ଫେରନ୍ତୁ block;
      }
    } ଅନ୍ୟଥା {
      // Oversized block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t ଅଙ୍କ_pages = total_size >> _memory_page_size_shift;
      ଯଦି (total_size & (_memory_page_size - 1))
        ++ଅଙ୍କ_pages;
      // Page count is stored in span_count
      size_t current_pages = span->span_ଗଣନା;
      ଶୂନ୍ୟ *block = pointer_offset(span, SPAN_HEADER_SIZE);
      ଯଦି (!oldsize)
        oldsize = (current_pages * _memory_page_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      ଯଦି ((current_pages >= ଅଙ୍କ_pages) && (ଅଙ୍କ_pages >= (current_pages / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ଯଦି ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        ଫେରନ୍ତୁ block;
      }
    }
  } ଅନ୍ୟଥା {
    oldsize = 0;
  }

  ଯଦି (!!(flags & RPMALLOC_GROW_OR_FAIL))
    ଫେରନ୍ତୁ 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t lower_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_size =
      (size > lower_bound) ? size : ((size > oldsize) ? lower_bound : size);
  ଶୂନ୍ୟ *block = _rpmalloc_allocate(heap, new_size);
  ଯଦି (p && block) {
    ଯଦି (!(flags & RPMALLOC_NO_PRESERVE))
      memcpy(block, p, oldsize < new_size ? oldsize : new_size);
    _rpmalloc_deallocate(p);
  }

  ଫେରନ୍ତୁ block;
}

ସ୍ଥିର ଶୂନ୍ୟ *_rpmalloc_aligned_reallocate(heap_t *heap, ଶୂନ୍ୟ *ptr,
                                          size_t alignment, size_t size,
                                          size_t oldsize, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags) {
  ଯଦି (alignment <= SMALL_GRANULARITY)
    ଫେରନ୍ତୁ _rpmalloc_reallocate(heap, ptr, size, oldsize, flags);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? _rpmalloc_usable_size(ptr) : 0);
  ଯଦି ((usablesize >= size) && !((uintptr_t)ptr & (alignment - 1))) {
    ଯଦି (no_alloc || (size >= (usablesize / 2)))
      ଫେରନ୍ତୁ ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  ଶୂନ୍ୟ *block =
      (!no_alloc ? _rpmalloc_aligned_allocate(heap, alignment, size) : 0);
  ଯଦି (EXPECTED(block != 0)) {
    ଯଦି (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      ଯଦି (!oldsize)
        oldsize = usablesize;
      memcpy(block, ptr, oldsize < size ? oldsize : size);
    }
    _rpmalloc_deallocate(ptr);
  }
  ଫେରନ୍ତୁ block;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
ସ୍ଥିର size_t _rpmalloc_usable_size(ଶୂନ୍ୟ *p) {
  // Grab the span using guaranteed span alignment
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  ଯଦି (span->size_class < SIZE_CLASS_ଗଣନା) {
    // Small/medium block
    ଶୂନ୍ୟ *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    ଫେରନ୍ତୁ span->block_size -
           ((size_t)pointer_diff(p, blocks_start) % span->block_size);
  }
  ଯଦି (span->size_class == SIZE_CLASS_LARGE) {
    // Large block
    size_t current_spans = span->span_ଗଣନା;
    ଫେରନ୍ତୁ (current_spans * _memory_span_size) - (size_t)pointer_diff(p, span);
  }
  // Oversized block, page count is stored in span_count
  size_t current_pages = span->span_ଗଣନା;
  ଫେରନ୍ତୁ (current_pages * _memory_page_size) - (size_t)pointer_diff(p, span);
}

//! Adjust and optimize the size class properties for the given class
ସ୍ଥିର ଶୂନ୍ୟ _rpmalloc_adjust_size_class(size_t iclass) {
  size_t block_size = _memory_size_class[iclass].block_size;
  size_t block_ଗଣନା = (_memory_span_size - SPAN_HEADER_SIZE) / block_size;

  _memory_size_class[iclass].block_ଗଣନା = (uint16_t)block_ଗଣନା;
  _memory_size_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  ଯଦି (iclass >= SMALL_CLASS_ଗଣନା) {
    size_t prevclass = iclass;
    ଯେତେବେଳେ (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      ଯଦି (_memory_size_class[prevclass].block_ଗଣନା ==
          _memory_size_class[iclass].block_ଗଣନା)
        _rpmalloc_memcpy_const(_memory_size_class + prevclass,
                               _memory_size_class + iclass,
                               ଆକାର(_memory_size_class[iclass]));
      ଅନ୍ୟଥା
        ଭାଙ୍ଗ;
    }
  }
}

//! Initialize the allocator and setup global data
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା rpmalloc_initialize(ଶୂନ୍ୟ) {
  ଯଦି (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    ଫେରନ୍ତୁ 0;
  }
  ଫେରନ୍ତୁ rpmalloc_initialize_config(0);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା rpmalloc_initialize_config(ଅଚଳ rpmalloc_config_t *config) {
  ଯଦି (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    ଫେରନ୍ତୁ 0;
  }
  _rpmalloc_initialized = 1;

  ଯଦି (config)
    memcpy(&_memory_config, config, ଆକାର(rpmalloc_config_t));
  ଅନ୍ୟଥା
    _rpmalloc_memset_const(&_memory_config, 0, ଆକାର(rpmalloc_config_t));

  ଯଦି (!_memory_config.memory_map || !_memory_config.memory_unmap) {
    _memory_config.memory_map = _rpmalloc_mmap_os;
    _memory_config.memory_unmap = _rpmalloc_unmap_os;
  }

#ଯଦି PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, ଆକାର(system_info));
  GetSystemInfo(&system_info);
  _memory_map_granularity = system_info.dwAllocationGranularity;
#ଅନ୍ୟଥା
  _memory_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#endif

#ଯଦି RPMALLOC_CONFIGURABLE
  _memory_page_size = _memory_config.page_size;
#ଅନ୍ୟଥା
  _memory_page_size = 0;
#endif
  _memory_huge_pages = 0;
  ଯଦି (!_memory_page_size) {
#ଯଦି PLATFORM_WINDOWS
    _memory_page_size = system_info.dwPageSize;
#ଅନ୍ୟଥା
    _memory_page_size = _memory_map_granularity;
    ଯଦି (_memory_config.enable_huge_pages) {
#ଯଦି defined(__linux__)
      size_t huge_page_size = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      ଯଦି (meminfo) {
        ଅକ୍ଷର ଧାଡ଼ି_2[128];
        ଯେତେବେଳେ (!huge_page_size && fgets(ଧାଡ଼ି_2, ଆକାର(ଧାଡ଼ି_2) - 1, meminfo)) {
          ଧାଡ଼ି_2[ଆକାର(ଧାଡ଼ି_2) - 1] = 0;
          ଯଦି (strstr(ଧାଡ଼ି_2, "Hugepagesize:"))
            huge_page_size = (size_t)strtol(ଧାଡ଼ି_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      ଯଦି (huge_page_size) {
        _memory_huge_pages = 1;
        _memory_page_size = huge_page_size;
        _memory_map_granularity = huge_page_size;
      }
#ନଚେତଯଦି defined(__FreeBSD__)
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
      size_t sz = ଆକାର(rc);

      ଯଦି (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, ଖାଲି, 0) == 0 &&
          rc == 1) {
        ସ୍ଥିର size_t defsize = 2 * 1024 * 1024;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nsize = 0;
        size_t sizes[4] = {0};
        _memory_huge_pages = 1;
        _memory_page_size = defsize;
        ଯଦି ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          ପାଇଁ (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            ଯଦି (defsize < csize) {
              _memory_page_size = csize;
              ଭାଙ୍ଗ;
            }
          }
        }
        _memory_map_granularity = _memory_page_size;
      }
#ନଚେତଯଦି defined(__APPLE__) || defined(__NetBSD__)
      _memory_huge_pages = 1;
      _memory_page_size = 2 * 1024 * 1024;
      _memory_map_granularity = _memory_page_size;
#endif
    }
#endif
  } ଅନ୍ୟଥା {
    ଯଦି (_memory_config.enable_huge_pages)
      _memory_huge_pages = 1;
  }

#ଯଦି PLATFORM_WINDOWS
  ଯଦି (_memory_config.enable_huge_pages) {
    HANDLE token = 0;
    size_t large_page_minimum = GetLargePageMinimum();
    ଯଦି (large_page_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    ଯଦି (token) {
      LUID luid;
      ଯଦି (Lookup_Privilege_ମୂଲ୍ୟ(0, SE_LOCK_MEMORY_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, ଆକାର(token_privileges));
        token_privileges.Privilege_ଗଣନା = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        ଯଦି (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          ଯଦି (GetLastError() == ERROR_SUCCESS)
            _memory_huge_pages = 1;
        }
      }
      CloseHandle(token);
    }
    ଯଦି (_memory_huge_pages) {
      ଯଦି (large_page_minimum > _memory_page_size)
        _memory_page_size = large_page_minimum;
      ଯଦି (large_page_minimum > _memory_map_granularity)
        _memory_map_granularity = large_page_minimum;
    }
  }
#endif

  size_t min_span_size = 256;
  size_t max_page_size;
#ଯଦି UINTPTR_MAX > 0xFFFFFFFF
  max_page_size = 4096ULL * 1024ULL * 1024ULL;
#ଅନ୍ୟଥା
  max_page_size = 4 * 1024 * 1024;
#endif
  ଯଦି (_memory_page_size < min_span_size)
    _memory_page_size = min_span_size;
  ଯଦି (_memory_page_size > max_page_size)
    _memory_page_size = max_page_size;
  _memory_page_size_shift = 0;
  size_t page_size_bit = _memory_page_size;
  ଯେତେବେଳେ (page_size_bit != 1) {
    ++_memory_page_size_shift;
    page_size_bit >>= 1;
  }
  _memory_page_size = ((size_t)1 << _memory_page_size_shift);

#ଯଦି RPMALLOC_CONFIGURABLE
  ଯଦି (!_memory_config.span_size) {
    _memory_span_size = _memory_default_span_size;
    _memory_span_size_shift = _memory_default_span_size_shift;
    _memory_span_mask = _memory_default_span_mask;
  } ଅନ୍ୟଥା {
    size_t span_size = _memory_config.span_size;
    ଯଦି (span_size > (256 * 1024))
      span_size = (256 * 1024);
    _memory_span_size = 4096;
    _memory_span_size_shift = 12;
    ଯେତେବେଳେ (_memory_span_size < span_size) {
      _memory_span_size <<= 1;
      ++_memory_span_size_shift;
    }
    _memory_span_mask = ~(uintptr_t)(_memory_span_size - 1);
  }
#endif

  memory_span_map_ଗଣନା =
      (_memory_config.span_map_ଗଣନା ? _memory_config.span_map_ଗଣନା
                                     : DEFAULT_SPAN_MAP_ଗଣନା);
  ଯଦି ((_memory_span_size * memory_span_map_ଗଣନା) < _memory_page_size)
    memory_span_map_ଗଣନା = (_memory_page_size / _memory_span_size);
  ଯଦି ((_memory_page_size >= _memory_span_size) &&
      ((memory_span_map_ଗଣନା * _memory_span_size) % _memory_page_size))
    memory_span_map_ଗଣନା = (_memory_page_size / _memory_span_size);
  memory_heap_reserve_ଗଣନା = (memory_span_map_ଗଣନା > DEFAULT_SPAN_MAP_ଗଣନା)
                                   ? DEFAULT_SPAN_MAP_ଗଣନା
                                   : memory_span_map_ଗଣନା;

  _memory_config.page_size = _memory_page_size;
  _memory_config.span_size = _memory_span_size;
  _memory_config.span_map_ଗଣନା = memory_span_map_ଗଣନା;
  _memory_config.enable_huge_pages = _memory_huge_pages;

#ଯଦି ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  ଯଦି (pthread_ଚାବି_create(&_memory_thread_heap, _rpmalloc_heap_release_raw_fc))
    ଫେରନ୍ତୁ -1;
#endif
#ଯଦି defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_ଚାବି = FlsAlloc(&_rpmalloc_thread_destructor);
#endif

  // Setup all small and medium size classes
  size_t iclass = 0;
  _memory_size_class[iclass].block_size = SMALL_GRANULARITY;
  _rpmalloc_adjust_size_class(iclass);
  ପାଇଁ (iclass = 1; iclass < SMALL_CLASS_ଗଣନା; ++iclass) {
    size_t size = iclass * SMALL_GRANULARITY;
    _memory_size_class[iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  _memory_medium_size_limit = (_memory_span_size - SPAN_HEADER_SIZE) >> 1;
  ଯଦି (_memory_medium_size_limit > MEDIUM_SIZE_LIMIT)
    _memory_medium_size_limit = MEDIUM_SIZE_LIMIT;
  ପାଇଁ (iclass = 0; iclass < MEDIUM_CLASS_ଗଣନା; ++iclass) {
    size_t size = SMALL_SIZE_LIMIT + ((iclass + 1) * MEDIUM_GRANULARITY);
    ଯଦି (size > _memory_medium_size_limit) {
      _memory_medium_size_limit =
          SMALL_SIZE_LIMIT + (iclass * MEDIUM_GRANULARITY);
      ଭାଙ୍ଗ;
    }
    _memory_size_class[SMALL_CLASS_ଗଣନା + iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(SMALL_CLASS_ଗଣନା + iclass);
  }

  _memory_orphan_heaps = 0;
#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS
  _memory_first_class_orphan_heaps = 0;
#endif
#ଯଦି ENABLE_STATISTICS
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
  memset(_memory_heaps, 0, ଆକାର(_memory_heaps));
  atomic_store32_release(&_memory_global_lock, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  ଫେରନ୍ତୁ 0;
}

//! Finalize the allocator
ଶୂନ୍ୟ rpmalloc_finalize(ଶୂନ୍ୟ) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  ଯଦି (_memory_global_reserve) {
    atomic_add32(&_memory_global_reserve_master->remaining_spans,
                 -(int32_t)memory_global_reserve_ଗଣନା);
    _memory_global_reserve_master = 0;
    memory_global_reserve_ଗଣନା = 0;
    _memory_global_reserve = 0;
  }
  atomic_store32_release(&_memory_global_lock, 0);

  // Free all thread caches and fully free spans
  ପାଇଁ (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    ଯେତେବେଳେ (heap) {
      heap_t *next_heap = heap->next_heap;
      heap->finalize = 1;
      _rpmalloc_heap_global_finalize(heap);
      heap = next_heap;
    }
  }

#ଯଦି ENABLE_GLOBAL_CACHE
  // Free global caches
  ପାଇଁ (size_t iclass = 0; iclass < LARGE_CLASS_ଗଣନା; ++iclass)
    _rpmalloc_global_cache_finalize(&_memory_span_cache[iclass]);
#endif

#ଯଦି (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_ଚାବି_delete(_memory_thread_heap);
#endif
#ଯଦି defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_ଚାବି);
  fls_ଚାବି = 0;
#endif
#ଯଦି ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&_mapped_pages) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&_mapped_pages_os) == 0,
                  "Memory leak detected");
#endif

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ rpmalloc_thread_initialize(ଶୂନ୍ୟ) {
  ଯଦି (!get_thread_heap_raw()) {
    heap_t *heap = _rpmalloc_heap_allocate(0);
    ଯଦି (heap) {
      _rpmalloc_stat_inc(&_memory_active_heaps);
      set_thread_heap(heap);
#ଯଦି defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_Set_ମୂଲ୍ୟ(fls_ଚାବି, heap);
#endif
    }
  }
}

//! Finalize thread, orphan heap
ଶୂନ୍ୟ rpmalloc_thread_finalize(ପୂର୍ଣ୍ଣସଂଖ୍ୟା release_caches) {
  heap_t *heap = get_thread_heap_raw();
  ଯଦି (heap)
    _rpmalloc_heap_release_raw(heap, release_caches);
  set_thread_heap(0);
#ଯଦି defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_Set_ମୂଲ୍ୟ(fls_ଚାବି, 0);
#endif
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା rpmalloc_is_thread_initialized(ଶୂନ୍ୟ) {
  ଫେରନ୍ତୁ (get_thread_heap_raw() != 0) ? 1 : 0;
}

ଅଚଳ rpmalloc_config_t *rpmalloc_config(ଶୂନ୍ୟ) { ଫେରନ୍ତୁ &_memory_config; }

// Extern interface

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *rpmalloc(size_t size) {
#ଯଦି ENABLE_VALIDATE_ARGS
  ଯଦି (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  ଫେରନ୍ତୁ _rpmalloc_allocate(heap, size);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ rpfree(ଶୂନ୍ୟ *ptr) { _rpmalloc_deallocate(ptr); }

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *rpcalloc(size_t ଅଙ୍କ_2, size_t size) {
  size_t total;
#ଯଦି ENABLE_VALIDATE_ARGS
#ଯଦି PLATFORM_WINDOWS
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = SizeTMult(ଅଙ୍କ_2, size, &total);
  ଯଦି ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#ଅନ୍ୟଥା
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = __builtin_umull_overflow(ଅଙ୍କ_2, size, &total);
  ଯଦି (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#endif
#ଅନ୍ୟଥା
  total = ଅଙ୍କ_2 * size;
#endif
  heap_t *heap = get_thread_heap();
  ଶୂନ୍ୟ *block = _rpmalloc_allocate(heap, total);
  ଯଦି (block)
    memset(block, 0, total);
  ଫେରନ୍ତୁ block;
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *rprealloc(ଶୂନ୍ୟ *ptr, size_t size) {
#ଯଦି ENABLE_VALIDATE_ARGS
  ଯଦି (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    ଫେରନ୍ତୁ ptr;
  }
#endif
  heap_t *heap = get_thread_heap();
  ଫେରନ୍ତୁ _rpmalloc_reallocate(heap, ptr, size, 0, 0);
}

ବାହ୍ୟ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *rpaligned_realloc(ଶୂନ୍ୟ *ptr, size_t alignment,
                                                  size_t size, size_t oldsize,
                                                  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags) {
#ଯଦି ENABLE_VALIDATE_ARGS
  ଯଦି ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  ଫେରନ୍ତୁ _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, oldsize,
                                      flags);
}

ବାହ୍ୟ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *rpaligned_alloc(size_t alignment, size_t size) {
  heap_t *heap = get_thread_heap();
  ଫେରନ୍ତୁ _rpmalloc_aligned_allocate(heap, alignment, size);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *
rpaligned_calloc(size_t alignment, size_t ଅଙ୍କ_2, size_t size) {
  size_t total;
#ଯଦି ENABLE_VALIDATE_ARGS
#ଯଦି PLATFORM_WINDOWS
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = SizeTMult(ଅଙ୍କ_2, size, &total);
  ଯଦି ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#ଅନ୍ୟଥା
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = __builtin_umull_overflow(ଅଙ୍କ_2, size, &total);
  ଯଦି (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#endif
#ଅନ୍ୟଥା
  total = ଅଙ୍କ_2 * size;
#endif
  ଶୂନ୍ୟ *block = rpaligned_alloc(alignment, total);
  ଯଦି (block)
    memset(block, 0, total);
  ଫେରନ୍ତୁ block;
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *rpmemalign(size_t alignment,
                                                  size_t size) {
  ଫେରନ୍ତୁ rpaligned_alloc(alignment, size);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା rpposix_memalign(ଶୂନ୍ୟ **memptr, size_t alignment,
                                   size_t size) {
  ଯଦି (memptr)
    *memptr = rpaligned_alloc(alignment, size);
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ EINVAL;
  ଫେରନ୍ତୁ *memptr ? 0 : ENOMEM;
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ size_t rpmalloc_usable_size(ଶୂନ୍ୟ *ptr) {
  ଫେରନ୍ତୁ (ptr ? _rpmalloc_usable_size(ptr) : 0);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ rpmalloc_thread_collect(ଶୂନ୍ୟ) {}

ଶୂନ୍ୟ rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, ଆକାର(rpmalloc_thread_statistics_t));
  heap_t *heap = get_thread_heap_raw();
  ଯଦି (!heap)
    ଫେରନ୍ତୁ;

  ପାଇଁ (size_t iclass = 0; iclass < SIZE_CLASS_ଗଣନା; ++iclass) {
    size_class_t *size_class = _memory_size_class + iclass;
    span_t *span = heap->size_class[iclass].partial_span;
    ଯେତେବେଳେ (span) {
      size_t free_ଗଣନା = span->list_size;
      size_t block_ଗଣନା = size_class->block_ଗଣନା;
      ଯଦି (span->free_list_limit < block_ଗଣନା)
        block_ଗଣନା = span->free_list_limit;
      free_ଗଣନା += (block_ଗଣନା - span->used_ଗଣନା);
      stats->sizecache += free_ଗଣନା * size_class->block_size;
      span = span->next;
    }
  }

#ଯଦି ENABLE_THREAD_CACHE
  ପାଇଁ (size_t iclass = 0; iclass < LARGE_CLASS_ଗଣନା; ++iclass) {
    span_cache_t *span_cache;
    ଯଦି (!iclass)
      span_cache = &heap->span_cache;
    ଅନ୍ୟଥା
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    stats->spancache += span_cache->ଗଣନା_2 * (iclass + 1) * _memory_span_size;
  }
#endif

  span_t *deferred = (span_t *)atomic_load_ptr(&heap->span_free_deferred);
  ଯେତେବେଳେ (deferred) {
    ଯଦି (deferred->size_class != SIZE_CLASS_HUGE)
      stats->spancache += (size_t)deferred->span_ଗଣନା * _memory_span_size;
    deferred = (span_t *)deferred->free_list;
  }

#ଯଦି ENABLE_STATISTICS
  stats->thread_to_global = (size_t)atomic_load64(&heap->thread_to_global);
  stats->global_to_thread = (size_t)atomic_load64(&heap->global_to_thread);

  ପାଇଁ (size_t iclass = 0; iclass < LARGE_CLASS_ଗଣନା; ++iclass) {
    stats->span_use[iclass].current =
        (size_t)atomic_load32(&heap->span_use[iclass].current);
    stats->span_use[iclass].peak =
        (size_t)atomic_load32(&heap->span_use[iclass].ଉପର);
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
  ପାଇଁ (size_t iclass = 0; iclass < SIZE_CLASS_ଗଣନା; ++iclass) {
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

ଶୂନ୍ୟ rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, ଆକାର(rpmalloc_global_statistics_t));
#ଯଦି ENABLE_STATISTICS
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
#ଯଦି ENABLE_GLOBAL_CACHE
  ପାଇଁ (size_t iclass = 0; iclass < LARGE_CLASS_ଗଣନା; ++iclass) {
    global_cache_t *cache = &_memory_span_cache[iclass];
    ଯେତେବେଳେ (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();
    uint32_t ଗଣନା_2 = cache->ଗଣନା_2;
#ଯଦି ENABLE_UNLIMITED_CACHE
    span_t *current_span = cache->overflow;
    ଯେତେବେଳେ (current_span) {
      ++ଗଣନା_2;
      current_span = current_span->next;
    }
#endif
    atomic_store32_release(&cache->lock, 0);
    stats->cached += ଗଣନା_2 * (iclass + 1) * _memory_span_size;
  }
#endif
}

#ଯଦି ENABLE_STATISTICS

ସ୍ଥିର ଶୂନ୍ୟ _memory_heap_dump_statistics(heap_t *heap, ଶୂନ୍ୟ *file) {
  fprintf(file, "Heap %d stats:\n", heap->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  ପାଇଁ (size_t iclass = 0; iclass < SIZE_CLASS_ଗଣନା; ++iclass) {
    ଯଦି (!atomic_load32(&heap->size_class_use[iclass].alloc_total))
      ଜାରି;
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
        _memory_size_class[iclass].block_ଗଣନା,
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
  ପାଇଁ (size_t iclass = 0; iclass < LARGE_CLASS_ଗଣନା; ++iclass) {
    ଯଦି (!atomic_load32(&heap->span_use[iclass].ଉପର) &&
        !atomic_load32(&heap->span_use[iclass].spans_map_calls))
      ଜାରି;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&heap->span_use[iclass].current),
        atomic_load32(&heap->span_use[iclass].ଉପର),
        atomic_load32(&heap->span_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&heap->span_use[iclass].ଉପର) *
         (size_t)_memory_span_size * (iclass + 1)) /
            (size_t)(1024 * 1024),
#ଯଦି ENABLE_THREAD_CACHE
        (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(!iclass ? heap->span_cache.ଗଣନା_2
                               : heap->span_large_cache[iclass - 1].ଗଣନା_2),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_to_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_from_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
#ଅନ୍ୟଥା
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
  fprintf(file, "Full spans: %zu\n", heap->full_span_ଗଣନା);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&heap->thread_to_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&heap->global_to_thread) / (size_t)(1024 * 1024));
}

#endif

ଶୂନ୍ୟ rpmalloc_dump_statistics(ଶୂନ୍ୟ *file) {
#ଯଦି ENABLE_STATISTICS
  ପାଇଁ (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    ଯେତେବେଳେ (heap) {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା need_dump = 0;
      ପାଇଁ (size_t iclass = 0; !need_dump && (iclass < SIZE_CLASS_ଗଣନା);
           ++iclass) {
        ଯଦି (!atomic_load32(&heap->size_class_use[iclass].alloc_total)) {
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].free_total),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          ଜାରି;
        }
        need_dump = 1;
      }
      ପାଇଁ (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_ଗଣନା);
           ++iclass) {
        ଯଦି (!atomic_load32(&heap->span_use[iclass].ଉପର) &&
            !atomic_load32(&heap->span_use[iclass].spans_map_calls))
          ଜାରି;
        need_dump = 1;
      }
      ଯଦି (need_dump)
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

#ଯଦି ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  ପାଇଁ (size_t iclass = 0; iclass < LARGE_CLASS_ଗଣନା; ++iclass) {
    global_cache_t *cache = _memory_span_cache + iclass;
    size_t global_cache = (size_t)cache->ଗଣନା_2 * iclass * _memory_span_size;

    size_t global_overflow_cache = 0;
    span_t *span = cache->overflow;
    ଯେତେବେଳେ (span) {
      global_overflow_cache += iclass * _memory_span_size;
      span = span->next;
    }
    ଯଦି (global_cache || global_overflow_cache || cache->ଭର୍ତ୍ତି_ଗଣନା ||
        cache->extract_ଗଣନା)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_overflow_cache / (size_t)(1024 * 1024),
              cache->ଭର୍ତ୍ତି_ଗଣନା, cache->extract_ଗଣନା);
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
#ଯଦି 0
	int64_t allocated = atomic_load64(&_allocation_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#endif
#endif
  (ଶୂନ୍ୟ)ଆକାର(file);
}

#ଯଦି RPMALLOC_FIRST_CLASS_HEAPS

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ rpmalloc_heap_t *rpmalloc_heap_acquire(ଶୂନ୍ୟ) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  heap_t *heap = _rpmalloc_heap_allocate(1);
  rpmalloc_assume(heap != ଖାଲି);
  heap->owner_thread = 0;
  _rpmalloc_stat_inc(&_memory_active_heaps);
  ଫେରନ୍ତୁ heap;
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ rpmalloc_heap_release(rpmalloc_heap_t *heap) {
  ଯଦି (heap)
    _rpmalloc_heap_release(heap, 1, 1);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, size_t size) {
#ଯଦି ENABLE_VALIDATE_ARGS
  ଯଦି (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#endif
  ଫେରନ୍ତୁ _rpmalloc_allocate(heap, size);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, size_t alignment,
                            size_t size) {
#ଯଦି ENABLE_VALIDATE_ARGS
  ଯଦି (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#endif
  ଫେରନ୍ତୁ _rpmalloc_aligned_allocate(heap, alignment, size);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, size_t ଅଙ୍କ_2, size_t size) {
  ଫେରନ୍ତୁ rpmalloc_heap_aligned_calloc(heap, 0, ଅଙ୍କ_2, size);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, size_t alignment,
                             size_t ଅଙ୍କ_2, size_t size) {
  size_t total;
#ଯଦି ENABLE_VALIDATE_ARGS
#ଯଦି PLATFORM_WINDOWS
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = SizeTMult(ଅଙ୍କ_2, size, &total);
  ଯଦି ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#ଅନ୍ୟଥା
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = __builtin_umull_overflow(ଅଙ୍କ_2, size, &total);
  ଯଦି (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#endif
#ଅନ୍ୟଥା
  total = ଅଙ୍କ_2 * size;
#endif
  ଶୂନ୍ୟ *block = _rpmalloc_aligned_allocate(heap, alignment, total);
  ଯଦି (block)
    memset(block, 0, total);
  ଫେରନ୍ତୁ block;
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, ଶୂନ୍ୟ *ptr, size_t size,
                      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags) {
#ଯଦି ENABLE_VALIDATE_ARGS
  ଯଦି (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    ଫେରନ୍ତୁ ptr;
  }
#endif
  ଫେରନ୍ତୁ _rpmalloc_reallocate(heap, ptr, size, 0, flags);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_ALLOCATOR ଶୂନ୍ୟ *
rpmalloc_heap_aligned_realloc(rpmalloc_heap_t *heap, ଶୂନ୍ୟ *ptr,
                              size_t alignment, size_t size,
                              unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags) {
#ଯଦି ENABLE_VALIDATE_ARGS
  ଯଦି ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#endif
  ଫେରନ୍ତୁ _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, 0, flags);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ rpmalloc_heap_free(rpmalloc_heap_t *heap, ଶୂନ୍ୟ *ptr) {
  (ଶୂନ୍ୟ)ଆକାର(heap);
  _rpmalloc_deallocate(ptr);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ rpmalloc_heap_free_all(rpmalloc_heap_t *heap) {
  span_t *span;
  span_t *next_span;

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  ପାଇଁ (size_t iclass = 0; iclass < SIZE_CLASS_ଗଣନା; ++iclass) {
    span = heap->size_class[iclass].partial_span;
    ଯେତେବେଳେ (span) {
      next_span = span->next;
      rpmalloc_heap_cache_ଭର୍ତ୍ତି(heap, span);
      span = next_span;
    }
    heap->size_class[iclass].partial_span = 0;
    span = heap->full_span[iclass];
    ଯେତେବେଳେ (span) {
      next_span = span->next;
      rpmalloc_heap_cache_ଭର୍ତ୍ତି(heap, span);
      span = next_span;
    }

    span = heap->size_class[iclass].cache;
    ଯଦି (span)
      rpmalloc_heap_cache_ଭର୍ତ୍ତି(heap, span);
    heap->size_class[iclass].cache = 0;
  }
  memset(heap->size_class, 0, ଆକାର(heap->size_class));
  memset(heap->full_span, 0, ଆକାର(heap->full_span));

  span = heap->large_huge_span;
  ଯେତେବେଳେ (span) {
    next_span = span->next;
    ଯଦି (UNEXPECTED(span->size_class == SIZE_CLASS_HUGE))
      _rpmalloc_deallocate_huge(span);
    ଅନ୍ୟଥା
      rpmalloc_heap_cache_ଭର୍ତ୍ତି(heap, span);
    span = next_span;
  }
  heap->large_huge_span = 0;
  heap->full_span_ଗଣନା = 0;

#ଯଦି ENABLE_THREAD_CACHE
  ପାଇଁ (size_t iclass = 0; iclass < LARGE_CLASS_ଗଣନା; ++iclass) {
    span_cache_t *span_cache;
    ଯଦି (!iclass)
      span_cache = &heap->span_cache;
    ଅନ୍ୟଥା
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    ଯଦି (!span_cache->ଗଣନା_2)
      ଜାରି;
#ଯଦି ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&heap->thread_to_global,
                         span_cache->ଗଣନା_2 * (iclass + 1) * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                       span_cache->ଗଣନା_2);
    rpmalloc_global_cache_ଭର୍ତ୍ତି_spans(span_cache->span, iclass + 1,
                                        span_cache->ଗଣନା_2);
#ଅନ୍ୟଥା
    ପାଇଁ (size_t ispan = 0; ispan < span_cache->ଗଣନା_2; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
    span_cache->ଗଣନା_2 = 0;
  }
#endif

#ଯଦି ENABLE_STATISTICS
  ପାଇଁ (size_t iclass = 0; iclass < SIZE_CLASS_ଗଣନା; ++iclass) {
    atomic_store32(&heap->size_class_use[iclass].alloc_current, 0);
    atomic_store32(&heap->size_class_use[iclass].spans_current, 0);
  }
  ପାଇଁ (size_t iclass = 0; iclass < LARGE_CLASS_ଗଣନା; ++iclass) {
    atomic_store32(&heap->span_use[iclass].current, 0);
  }
#endif
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ rpmalloc_heap_thread_set_current(rpmalloc_heap_t *heap) {
  heap_t *prev_heap = get_thread_heap_raw();
  ଯଦି (prev_heap != heap) {
    set_thread_heap(heap);
    ଯଦି (prev_heap)
      rpmalloc_heap_release(prev_heap);
  }
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(ଶୂନ୍ୟ *ptr) {
  // Grab the span, and then the heap from the span
  span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
  ଯଦି (span) {
    ଫେରନ୍ତୁ span->heap;
  }
  ଫେରନ୍ତୁ 0;
}

#endif

#ଯଦି ENABLE_PRELOAD || ENABLE_OVERRIDE

#ଅନ୍ତର୍ଭାଵ "malloc.c"

#endif

ଶୂନ୍ୟ rpmalloc_linker_reference(ଶୂନ୍ୟ) { (ଶୂନ୍ୟ)ଆକାର(_rpmalloc_initialized); }
