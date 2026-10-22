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

#અન્તર્ભાવ "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#જો defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wunused-function"
#જો __has_warning("-Wreserved-identifier")
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
#જો __has_warning("-Wstatic-in-inline")
#pragma clang diagnostic ignored "-Wstatic-in-inline"
#endif
#અથવાજો defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#જો !defined(__has_builtin)
#define __has_builtin(b) 0
#endif

#જો defined(__GNUC__) || defined(__clang__)

#જો __has_builtin(__builtin_memcpy_inline)
#define _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#નહીંતર
#define _rpmalloc_memcpy_const(x, y, s)                                        \
  કરો {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memcpy(x, y, s);                                                           \
  } જ્યારે (0)
#endif

#જો __has_builtin(__builtin_memset_inline)
#define _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#નહીંતર
#define _rpmalloc_memset_const(x, y, s)                                        \
  કરો {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } જ્યારે (0)
#endif
#નહીંતર
#define _rpmalloc_memcpy_const(x, y, s) memcpy(x, y, s)
#define _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#endif

#જો __has_builtin(__builtin_assume)
#define rpmalloc_assume(cond) __builtin_assume(cond)
#અથવાજો defined(__GNUC__)
#define rpmalloc_assume(cond)                                                  \
  કરો {                                                                         \
    જો (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } જ્યારે (0)
#અથવાજો defined(_MSC_VER)
#define rpmalloc_assume(cond) __assume(cond)
#નહીંતર
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
#ifndef DEFAULT_SPAN_MAP_ગણતરી
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#define DEFAULT_SPAN_MAP_ગણતરી 64
#endif
#ifndef GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#define GLOBAL_CACHE_MULTIPLIER 8
#endif

#જો DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#error Must use global cache જો unmap is disabled
#endif

#જો DISABLE_UNMAP
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 1
#endif

#જો !ENABLE_GLOBAL_CACHE
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 0
#endif

#જો !ENABLE_THREAD_CACHE
#undef ENABLE_ADAPTIVE_THREAD_CACHE
#define ENABLE_ADAPTIVE_THREAD_CACHE 0
#endif

#જો defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#define PLATFORM_POSIX 0
#નહીંતર
#define PLATFORM_WINDOWS 0
#define PLATFORM_POSIX 1
#endif

/// Platform and arch specifics
#જો defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 5105)
#ifndef FORCEINLINE
#define FORCEINLINE અંતઃરેખ __forceinline
#endif
#define _Static_assert static_assert
#નહીંતર
#ifndef FORCEINLINE
#define FORCEINLINE અંતઃરેખ __attribute__((__always_inline__))
#endif
#endif
#જો PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#અન્તર્ભાવ <windows.h>
#જો ENABLE_VALIDATE_ARGS
#અન્તર્ભાવ <intsafe.h>
#endif
#નહીંતર
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <unistd.h>
#જો defined(__linux__) || defined(__ANDROID__)
#અન્તર્ભાવ <sys/prctl.h>
#જો !defined(PR_SET_VMA)
#define PR_SET_VMA 0x53564d41
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif
#જો defined(__APPLE__)
#અન્તર્ભાવ <TargetConditionals.h>
#જો !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#અન્તર્ભાવ <mach/mach_vm.h>
#અન્તર્ભાવ <mach/vm_statistics.h>
#endif
#અન્તર્ભાવ <pthread.h>
#endif
#જો defined(__HAIKU__) || defined(__TINYC__)
#અન્તર્ભાવ <pthread.h>
#endif
#endif

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <string.h>

#જો defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#અન્તર્ભાવ <fibersapi.h>
સ્થિર DWORD fls_ચાવી;
#endif

#જો PLATFORM_POSIX
#અન્તર્ભાવ <sched.h>
#અન્તર્ભાવ <sys/mman.h>
#ifdef __FreeBSD__
#અન્તર્ભાવ <sys/sysctl.h>
#define MAP_HUGETLB MAP_ALIGNED_SUPER
#ifndef PROT_MAX
#define PROT_MAX(f) 0
#endif
#નહીંતર
#define PROT_MAX(f) 0
#endif
#ifdef __sun
બાહ્ય પૂર્ણાંક madvise(caddr_t, size_t, પૂર્ણાંક);
#endif
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif
#endif
#અન્તર્ભાવ <errno.h>

#જો ENABLE_ASSERTS
#undef NDEBUG
#જો defined(_MSC_VER) && !defined(_DEBUG)
#define _DEBUG
#endif
#અન્તર્ભાવ <assert.h>
#define RPMALLOC_TOSTRING_M(x) #x
#define RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#define rpmalloc_assert(truth, message)                                        \
  કરો {                                                                         \
    જો (!(truth)) {                                                            \
      જો (_memory_config.error_callback) {                                     \
        _memory_config.error_callback(message " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } નહીંતર {                                                                 \
        પ્રતિજ્ઞા((truth) && message);                                            \
      }                                                                        \
    }                                                                          \
  } જ્યારે (0)
#નહીંતર
#define rpmalloc_assert(truth, message)                                        \
  કરો {                                                                         \
  } જ્યારે (0)
#endif
#જો ENABLE_STATISTICS
#અન્તર્ભાવ <stdio.h>
#endif

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#જો defined(_MSC_VER) && !defined(__clang__)

પ્રકારવ્યાખ્યા અસ્થિર long atomic32_t;
પ્રકારવ્યાખ્યા અસ્થિર long long atomic64_t;
પ્રકારવ્યાખ્યા અસ્થિર શૂન્ય *atomicptr_t;

સ્થિર FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  પાછા (int32_t)InterlockedOr(src, 0);
}
સ્થિર FORCEINLINE શૂન્ય atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
સ્થિર FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  પાછા (int32_t)InterlockedIncrement(val);
}
સ્થિર FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  પાછા (int32_t)InterlockedDecrement(val);
}
સ્થિર FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  પાછા (int32_t)InterlockedExchangeAdd(val, add) + add;
}
સ્થિર FORCEINLINE પૂર્ણાંક atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  પાછા (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
સ્થિર FORCEINLINE શૂન્ય atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
સ્થિર FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  પાછા (int64_t)InterlockedOr64(src, 0);
}
સ્થિર FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  પાછા (int64_t)InterlockedExchangeAdd64(val, add) + add;
}
સ્થિર FORCEINLINE શૂન્ય *atomic_load_ptr(atomicptr_t *src) {
  પાછા InterlockedCompareExchangePointer(src, 0, 0);
}
સ્થિર FORCEINLINE શૂન્ય atomic_store_ptr(atomicptr_t *dst, શૂન્ય *val) {
  InterlockedExchangePointer(dst, val);
}
સ્થિર FORCEINLINE શૂન્ય atomic_store_ptr_release(atomicptr_t *dst, શૂન્ય *val) {
  InterlockedExchangePointer(dst, val);
}
સ્થિર FORCEINLINE શૂન્ય *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     શૂન્ય *val) {
  પાછા (શૂન્ય *)InterlockedExchangePointer((શૂન્ય *અસ્થિર *)dst, val);
}
સ્થિર FORCEINLINE પૂર્ણાંક atomic_cas_ptr(atomicptr_t *dst, શૂન્ય *val, શૂન્ય *ref) {
  પાછા (InterlockedCompareExchangePointer((શૂન્ય *અસ્થિર *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#define EXPECTED(x) (x)
#define UNEXPECTED(x) (x)

#નહીંતર

#અન્તર્ભાવ <stdatomic.h>

પ્રકારવ્યાખ્યા અસ્થિર _Atomic(int32_t) atomic32_t;
પ્રકારવ્યાખ્યા અસ્થિર _Atomic(int64_t) atomic64_t;
પ્રકારવ્યાખ્યા અસ્થિર _Atomic(શૂન્ય *) atomicptr_t;

સ્થિર FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  પાછા atomic_load_explicit(src, memory_order_relaxed);
}
સ્થિર FORCEINLINE શૂન્ય atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
સ્થિર FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  પાછા atomic_fetch_add_explicit(val, 1, memory_order_relaxed) + 1;
}
સ્થિર FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  પાછા atomic_fetch_add_explicit(val, -1, memory_order_relaxed) - 1;
}
સ્થિર FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  પાછા atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
સ્થિર FORCEINLINE પૂર્ણાંક atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  પાછા atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
સ્થિર FORCEINLINE શૂન્ય atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
સ્થિર FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  પાછા atomic_load_explicit(val, memory_order_relaxed);
}
સ્થિર FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  પાછા atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
સ્થિર FORCEINLINE શૂન્ય *atomic_load_ptr(atomicptr_t *src) {
  પાછા atomic_load_explicit(src, memory_order_relaxed);
}
સ્થિર FORCEINLINE શૂન્ય atomic_store_ptr(atomicptr_t *dst, શૂન્ય *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
સ્થિર FORCEINLINE શૂન્ય atomic_store_ptr_release(atomicptr_t *dst, શૂન્ય *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
સ્થિર FORCEINLINE શૂન્ય *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     શૂન્ય *val) {
  પાછા atomic_exchange_explicit(dst, val, memory_order_acquire);
}
સ્થિર FORCEINLINE પૂર્ણાંક atomic_cas_ptr(atomicptr_t *dst, શૂન્ય *val, શૂન્ય *ref) {
  પાછા atomic_compare_exchange_weak_explicit(
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

#જો ENABLE_STATISTICS
#define _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#define _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#define _rpmalloc_stat_add(counter, કિંમત_2)                                     \
  atomic_add32(counter, (int32_t)(કિંમત_2))
#define _rpmalloc_stat_add64(counter, કિંમત_2)                                   \
  atomic_add64(counter, (int64_t)(કિંમત_2))
#define _rpmalloc_stat_add_peak(counter, કિંમત_2, peak)                          \
  કરો {                                                                         \
    int32_t cur_ગણતરી = atomic_add32(counter, (int32_t)(કિંમત_2));              \
    જો (cur_ગણતરી > (peak))                                                   \
      peak = cur_ગણતરી;                                                       \
  } જ્યારે (0)
#define _rpmalloc_stat_sub(counter, કિંમત_2)                                     \
  atomic_add32(counter, -(int32_t)(કિંમત_2))
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  કરો {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&heap->size_class_use[class_idx].alloc_current);         \
    જો (alloc_current > heap->size_class_use[class_idx].alloc_peak)            \
      heap->size_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&heap->size_class_use[class_idx].alloc_total);               \
  } જ્યારે (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  કરો {                                                                         \
    atomic_decr32(&heap->size_class_use[class_idx].alloc_current);             \
    atomic_incr32(&heap->size_class_use[class_idx].free_total);                \
  } જ્યારે (0)
#નહીંતર
#define _rpmalloc_stat_inc(counter)                                            \
  કરો {                                                                         \
  } જ્યારે (0)
#define _rpmalloc_stat_dec(counter)                                            \
  કરો {                                                                         \
  } જ્યારે (0)
#define _rpmalloc_stat_add(counter, કિંમત_2)                                     \
  કરો {                                                                         \
  } જ્યારે (0)
#define _rpmalloc_stat_add64(counter, કિંમત_2)                                   \
  કરો {                                                                         \
  } જ્યારે (0)
#define _rpmalloc_stat_add_peak(counter, કિંમત_2, peak)                          \
  કરો {                                                                         \
  } જ્યારે (0)
#define _rpmalloc_stat_sub(counter, કિંમત_2)                                     \
  કરો {                                                                         \
  } જ્યારે (0)
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  કરો {                                                                         \
  } જ્યારે (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  કરો {                                                                         \
  } જ્યારે (0)
#endif

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#define SMALL_GRANULARITY 16
//! Small granularity shift count
#define SMALL_GRANULARITY_SHIFT 4
//! Number of small block size classes
#define SMALL_CLASS_ગણતરી 65
//! Maximum size of a small block
#define SMALL_SIZE_LIMIT (SMALL_GRANULARITY * (SMALL_CLASS_ગણતરી - 1))
//! Granularity of a medium allocation block
#define MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#define MEDIUM_GRANULARITY_SHIFT 9
//! Number of medium block size classes
#define MEDIUM_CLASS_ગણતરી 61
//! Total number of small + medium size classes
#define SIZE_CLASS_ગણતરી (SMALL_CLASS_ગણતરી + MEDIUM_CLASS_ગણતરી)
//! Number of large block size classes
#define LARGE_CLASS_ગણતરી 63
//! Maximum size of a medium block
#define MEDIUM_SIZE_LIMIT                                                      \
  (SMALL_SIZE_LIMIT + (MEDIUM_GRANULARITY * MEDIUM_CLASS_ગણતરી))
//! Maximum size of a large block
#define LARGE_SIZE_LIMIT                                                       \
  ((LARGE_CLASS_ગણતરી * _memory_span_size) - SPAN_HEADER_SIZE)
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

#જો ENABLE_VALIDATE_ARGS
//! Maximum allocation size to avoid integer overflow
#undef MAX_ALLOC_SIZE
#define MAX_ALLOC_SIZE (((size_t) - 1) - _memory_span_size)
#endif

#define pointer_offset(ptr, ofs) (શૂન્ય *)((અક્ષર *)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff(first, second)                                            \
  (ptrdiff_t)((અચલ અક્ષર *)(first) - (અચલ અક્ષર *)(second))

#define INVALID_POINTER ((શૂન્ય *)((uintptr_t) - 1))

#define SIZE_CLASS_LARGE SIZE_CLASS_ગણતરી
#define SIZE_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
પ્રકારવ્યાખ્યા સંરચના heap_t heap_t;
//! Span of memory pages
પ્રકારવ્યાખ્યા સંરચના span_t span_t;
//! Span list
પ્રકારવ્યાખ્યા સંરચના span_list_t span_list_t;
//! Span active data
પ્રકારવ્યાખ્યા સંરચના span_active_t span_active_t;
//! Size class definition
પ્રકારવ્યાખ્યા સંરચના size_class_t size_class_t;
//! Global cache
પ્રકારવ્યાખ્યા સંરચના global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#define SPAN_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#define SPAN_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#define SPAN_FLAG_ALIGNED_BLOCKS 4U
//! Flag indicating an unmapped master span
#define SPAN_FLAG_UNMAPPED_MASTER 8U

#જો ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
સંરચના span_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t ઉપલું;
#જો ENABLE_STATISTICS
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
પ્રકારવ્યાખ્યા સંરચના span_use_t span_use_t;
#endif

#જો ENABLE_STATISTICS
સંરચના size_class_use_t {
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
પ્રકારવ્યાખ્યા સંરચના size_class_use_t size_class_use_t;
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
સંરચના span_t {
  //! Free list
  શૂન્ય *free_list;
  //! Total block count of size class
  uint32_t block_ગણતરી;
  //! Size class
  uint32_t size_class;
  //! Index of last block initialized in free list
  uint32_t free_list_limit;
  //! Number of used blocks remaining when in partial state
  uint32_t used_ગણતરી;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_size;
  //! Size of a block
  uint32_t block_size;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t span_ગણતરી;
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
_Static_assert(કદ(span_t) <= SPAN_HEADER_SIZE, "span size mismatch");

સંરચના span_cache_t {
  size_t ગણતરી;
  span_t *span[MAX_THREAD_SPAN_CACHE];
};
પ્રકારવ્યાખ્યા સંરચના span_cache_t span_cache_t;

સંરચના span_large_cache_t {
  size_t ગણતરી;
  span_t *span[MAX_THREAD_SPAN_LARGE_CACHE];
};
પ્રકારવ્યાખ્યા સંરચના span_large_cache_t span_large_cache_t;

સંરચના heap_size_class_t {
  //! Free list of active span
  શૂન્ય *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  span_t *partial_span;
  //! Early level cache of fully free spans
  span_t *cache;
};
પ્રકારવ્યાખ્યા સંરચના heap_size_class_t heap_size_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
સંરચના heap_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  heap_size_class_t size_class[SIZE_CLASS_ગણતરી];
#જો ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  span_cache_t span_cache;
#endif
  //! List of deferred free spans (single linked list)
  atomicptr_t span_free_deferred;
  //! Number of full spans
  size_t full_span_ગણતરી;
  //! Mapped but unused spans
  span_t *span_reserve;
  //! Master span for mapped but unused spans
  span_t *span_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t child_ગણતરી;
  //! Next heap in id list
  heap_t *next_heap;
  //! Next heap in orphan list
  heap_t *next_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  પૂર્ણાંક finalize;
  //! Master heap owning the memory pages
  heap_t *master_heap;
#જો ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  span_large_cache_t span_large_cache[LARGE_CLASS_ગણતરી - 1];
#endif
#જો RPMALLOC_FIRST_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  span_t *full_span[SIZE_CLASS_ગણતરી];
  //! Double linked list of large and huge spans allocated by this heap
  span_t *large_huge_span;
#endif
#જો ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  span_use_t span_use[LARGE_CLASS_ગણતરી];
#endif
#જો ENABLE_STATISTICS
  //! Allocation stats per size class
  size_class_use_t size_class_use[SIZE_CLASS_ગણતરી + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_to_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_to_thread;
#endif
};

// Size class for defining a block size bucket
સંરચના size_class_t {
  //! Size of blocks in this class
  uint32_t block_size;
  //! Number of blocks in each chunk
  uint16_t block_ગણતરી;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(કદ(size_class_t) == 8, "Size class size mismatch");

સંરચના global_cache_t {
  //! Cache lock
  atomic32_t lock;
  //! Cache count
  uint32_t ગણતરી;
#જો ENABLE_STATISTICS
  //! Insert count
  size_t ઉમેરો_ગણતરી;
  //! Extract count
  size_t extract_ગણતરી;
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
સ્થિર પૂર્ણાંક _rpmalloc_initialized;
//! Main thread ID
સ્થિર uintptr_t _rpmalloc_main_thread_id;
//! Configuration
સ્થિર rpmalloc_config_t _memory_config;
//! Memory page size
સ્થિર size_t _memory_page_size;
//! Shift to divide by page size
સ્થિર size_t _memory_page_size_shift;
//! Granularity at which memory pages are mapped by OS
સ્થિર size_t _memory_map_granularity;
#જો RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
સ્થિર size_t _memory_span_size;
//! Shift to divide by span size
સ્થિર size_t _memory_span_size_shift;
//! Mask to get to start of a memory span
સ્થિર uintptr_t _memory_span_mask;
#નહીંતર
//! Hardwired span size
#define _memory_span_size _memory_default_span_size
#define _memory_span_size_shift _memory_default_span_size_shift
#define _memory_span_mask _memory_default_span_mask
#endif
//! Number of spans to map in each map call
સ્થિર size_t memory_span_map_ગણતરી;
//! Number of spans to keep reserved in each heap
સ્થિર size_t memory_heap_reserve_ગણતરી;
//! Global size classes
સ્થિર size_class_t _memory_size_class[SIZE_CLASS_ગણતરી];
//! Run-time size limit of medium blocks
સ્થિર size_t _memory_medium_size_limit;
//! Heap ID counter
સ્થિર atomic32_t _memory_heap_id;
//! Huge page support
સ્થિર પૂર્ણાંક _memory_huge_pages;
#જો ENABLE_GLOBAL_CACHE
//! Global span cache
સ્થિર global_cache_t _memory_span_cache[LARGE_CLASS_ગણતરી];
#endif
//! Global reserved spans
સ્થિર span_t *_memory_global_reserve;
//! Global reserved count
સ્થિર size_t memory_global_reserve_ગણતરી;
//! Global reserved master
સ્થિર span_t *_memory_global_reserve_master;
//! All heaps
સ્થિર heap_t *_memory_heaps[HEAP_ARRAY_SIZE];
//! Used to restrict access to mapping memory for huge pages
સ્થિર atomic32_t _memory_global_lock;
//! Orphaned heaps
સ્થિર heap_t *_memory_orphan_heaps;
#જો RPMALLOC_FIRST_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
સ્થિર heap_t *_memory_first_class_orphan_heaps;
#endif
#જો ENABLE_STATISTICS
//! Allocations counter
સ્થિર atomic64_t _allocation_counter;
//! Deallocations counter
સ્થિર atomic64_t _deallocation_counter;
//! Active heap count
સ્થિર atomic32_t _memory_active_heaps;
//! Number of currently mapped memory pages
સ્થિર atomic32_t _mapped_pages;
//! Peak number of concurrently mapped memory pages
સ્થિર int32_t _mapped_pages_peak;
//! Number of mapped master spans
સ્થિર atomic32_t _master_spans;
//! Number of unmapped dangling master spans
સ્થિર atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
સ્થિર atomic32_t _mapped_total;
//! Running counter of total number of unmapped memory pages since start
સ્થિર atomic32_t _unmapped_total;
//! Number of currently mapped memory pages in OS calls
સ્થિર atomic32_t _mapped_pages_os;
//! Number of currently allocated pages in huge allocations
સ્થિર atomic32_t _huge_pages_current;
//! Peak number of currently allocated pages in huge allocations
સ્થિર int32_t _huge_pages_peak;
#endif

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#જો ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
સ્થિર pthread_key_t _memory_thread_heap;
#નહીંતર
#ifdef _MSC_VER
#define _Thread_local __declspec(thread)
#define TLS_MODEL
#નહીંતર
#ifndef __HAIKU__
#define TLS_MODEL __attribute__((tls_model("initial-exec")))
#નહીંતર
#define TLS_MODEL
#endif
#જો !defined(__clang__) && defined(__GNUC__)
#define _Thread_local __thread
#endif
#endif
સ્થિર _Thread_local heap_t *_memory_thread_heap TLS_MODEL;
#endif

સ્થિર અંતઃરેખ heap_t *get_thread_heap_raw(શૂન્ય) {
#જો (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  પાછા pthread_getspecific(_memory_thread_heap);
#નહીંતર
  પાછા _memory_thread_heap;
#endif
}

//! Get the current thread heap
સ્થિર અંતઃરેખ heap_t *get_thread_heap(શૂન્ય) {
  heap_t *heap = get_thread_heap_raw();
#જો ENABLE_PRELOAD
  જો (EXPECTED(heap != 0))
    પાછા heap;
  rpmalloc_initialize();
  પાછા get_thread_heap_raw();
#નહીંતર
  પાછા heap;
#endif
}

//! Fast thread ID
સ્થિર અંતઃરેખ uintptr_t get_thread_id(શૂન્ય) {
#જો defined(_WIN32)
  પાછા (uintptr_t)((શૂન્ય *)NtCurrentTeb());
#અથવાજો (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#જો defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#અથવાજો defined(__x86_64__)
#જો defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#નહીંતર
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#endif
#અથવાજો defined(__arm__)
  __asm__ અસ્થિર("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#અથવાજો defined(__aarch64__)
#જો defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ અસ્થિર("mrs %0, tpidrro_el0" : "=r"(tid));
#નહીંતર
  __asm__ અસ્થિર("mrs %0, tpidr_el0" : "=r"(tid));
#endif
#નહીંતર
#error This platform needs implementation of get_thread_id()
#endif
  પાછા tid;
#નહીંતર
#error This platform needs implementation of get_thread_id()
#endif
}

//! Set the current thread heap
સ્થિર શૂન્ય set_thread_heap(heap_t *heap) {
#જો ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(_memory_thread_heap, heap);
#નહીંતર
  _memory_thread_heap = heap;
#endif
  જો (heap)
    heap->owner_thread = get_thread_id();
}

//! Set main thread ID
બાહ્ય શૂન્ય rpmalloc_set_main_thread(શૂન્ય);

શૂન્ય rpmalloc_set_main_thread(શૂન્ય) {
  _rpmalloc_main_thread_id = get_thread_id();
}

સ્થિર શૂન્ય _rpmalloc_spin(શૂન્ય) {
#જો defined(_MSC_VER)
#જો defined(_M_ARM64)
  __yield();
#નહીંતર
  _mm_pause();
#endif
#અથવાજો defined(__x86_64__) || defined(__i386__)
  __asm__ અસ્થિર("pause" ::: "memory");
#અથવાજો defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ અસ્થિર("yield" ::: "memory");
#અથવાજો defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ અસ્થિર("or 27,27,27");
#અથવાજો defined(__sparc__)
  __asm__ અસ્થિર("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#નહીંતર
  સંરચના timespec ts = {0};
  nanosleep(&ts, 0);
#endif
}

#જો defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

સ્થિર શૂન્ય NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  પસંદગી (reason) {
  સ્થિતિ DLL_PROCESS_ATTACH:
    તોડો;
  સ્થિતિ DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    તોડો;
  સ્થિતિ DLL_THREAD_ATTACH:
    તોડો;
  સ્થિતિ DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    તોડો;
  }
}

#ifdef _WIN64
#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#pragma const_seg(".CRT$XLY")

બાહ્ય અચલ PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
અચલ PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#pragma const_seg()
#નહીંતર // _WIN64
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#pragma data_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#pragma data_seg()
#endif // _WIN64

સ્થિર શૂન્ય NTAPI _rpmalloc_thread_destructor(શૂન્ય *કિંમત_2) {
#જો ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  જો (get_thread_id() == _rpmalloc_main_thread_id)
    પાછા;
#endif
  જો (કિંમત_2)
    rpmalloc_thread_finalize(1);
}
#endif

////////////
///
/// Low level memory map/unmap
///
//////

સ્થિર શૂન્ય _rpmalloc_set_name(શૂન્ય *address, size_t size) {
#જો defined(__linux__) || defined(__ANDROID__)
  અચલ અક્ષર *name = _memory_huge_pages ? _memory_config.huge_page_name
                                        : _memory_config.page_name;
  જો (address == MAP_FAILED || !name)
    પાછા;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (શૂન્ય)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (uintptr_t)address, size,
              (uintptr_t)name);
#નહીંતર
  (શૂન્ય)કદ(size);
  (શૂન્ય)કદ(address);
#endif
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
સ્થિર શૂન્ય *_rpmalloc_mmap(size_t size, size_t *offset) {
  rpmalloc_assert(!(size % _memory_page_size), "Invalid mmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
  શૂન્ય *address = _memory_config.memory_map(size, offset);
  જો (EXPECTED(address != 0)) {
    _rpmalloc_stat_add_peak(&_mapped_pages, (size >> _memory_page_size_shift),
                            _mapped_pages_peak);
    _rpmalloc_stat_add(&_mapped_total, (size >> _memory_page_size_shift));
  }
  પાછા address;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
સ્થિર શૂન્ય _rpmalloc_unmap(શૂન્ય *address, size_t size, size_t offset,
                            size_t release) {
  rpmalloc_assert(!release || (release >= size), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  જો (release) {
    rpmalloc_assert(!(release % _memory_page_size), "Invalid unmap size");
    _rpmalloc_stat_sub(&_mapped_pages, (release >> _memory_page_size_shift));
    _rpmalloc_stat_add(&_unmapped_total, (release >> _memory_page_size_shift));
  }
  _memory_config.memory_unmap(address, size, offset, release);
}

//! Default implementation to map new pages to virtual memory
સ્થિર શૂન્ય *_rpmalloc_mmap_os(size_t size, size_t *offset) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((size >= _memory_span_size) &&
                    (_memory_span_size > _memory_map_granularity))
                       ? _memory_span_size
                       : 0;
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
#જો PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  શૂન્ય *ptr = VirtualAlloc(0, size + padding,
                           (_memory_huge_pages ? MEM_LARGE_PAGES : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
  જો (!ptr) {
    જો (_memory_config.map_fail_callback) {
      જો (_memory_config.map_fail_callback(size + padding))
        પાછા _rpmalloc_mmap_os(size, offset);
    } નહીંતર {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    પાછા 0;
  }
#નહીંતર
  પૂર્ણાંક flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#જો defined(__APPLE__) && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
  પૂર્ણાંક fd = (પૂર્ણાંક)VM_MAKE_TAG(240U);
  જો (_memory_huge_pages)
    fd |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
  શૂન્ય *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#અથવાજો defined(MAP_HUGETLB)
  શૂન્ય *ptr = mmap(0, size + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (_memory_huge_pages ? MAP_HUGETLB : 0) | flags, -1, 0);
#જો defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  જો ((ptr == MAP_FAILED || !ptr) && _memory_huge_pages) {
    ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    જો (ptr && ptr != MAP_FAILED) {
      પૂર્ણાંક prm = madvise(ptr, size + padding, MADV_HUGEPAGE);
      (શૂન્ય)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#endif
  _rpmalloc_set_name(ptr, size + padding);
#અથવાજો defined(MAP_ALIGNED)
  અચલ size_t align =
      (કદ(size_t) * 8) - (size_t)(__builtin_clzl(size - 1));
  શૂન્ય *ptr =
      mmap(0, size + padding, PROT_READ | PROT_WRITE,
           (_memory_huge_pages ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#અથવાજો defined(MAP_ALIGN)
  caddr_t base = (_memory_huge_pages ? (caddr_t)(4 << 20) : 0);
  શૂન્ય *ptr = mmap(base, size + padding, PROT_READ | PROT_WRITE,
                   (_memory_huge_pages ? MAP_ALIGN : 0) | flags, -1, 0);
#નહીંતર
  શૂન્ય *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#endif
  જો ((ptr == MAP_FAILED) || !ptr) {
    જો (_memory_config.map_fail_callback) {
      જો (_memory_config.map_fail_callback(size + padding))
        પાછા _rpmalloc_mmap_os(size, offset);
    } નહીંતર જો (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    પાછા 0;
  }
#endif
  _rpmalloc_stat_add(&_mapped_pages_os,
                     (int32_t)((size + padding) >> _memory_page_size_shift));
  જો (padding) {
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
  પાછા ptr;
}

//! Default implementation to unmap pages from virtual memory
સ્થિર શૂન્ય _rpmalloc_unmap_os(શૂન્ય *address, size_t size, size_t offset,
                               size_t release) {
  rpmalloc_assert(release || (offset == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid unmap size");
  જો (release && offset) {
    offset <<= 3;
    address = pointer_offset(address, -(int32_t)offset);
    જો ((release >= _memory_span_size) &&
        (_memory_span_size > _memory_map_granularity)) {
      // Padding is always one span size
      release += _memory_span_size;
    }
  }
#જો !DISABLE_UNMAP
#જો PLATFORM_WINDOWS
  જો (!VirtualFree(address, release ? 0 : size,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#નહીંતર
  જો (release) {
    જો (munmap(address, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } નહીંતર {
#જો defined(MADV_FREE_REUSABLE)
    પૂર્ણાંક ret;
    જ્યારે ((ret = madvise(address, size, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    જો ((ret == -1) && (errno != 0)) {
#અથવાજો defined(MADV_DONTNEED)
    જો (madvise(address, size, MADV_DONTNEED)) {
#અથવાજો defined(MADV_PAGEOUT)
    જો (madvise(address, size, MADV_PAGEOUT)) {
#અથવાજો defined(MADV_FREE)
    જો (madvise(address, size, MADV_FREE)) {
#નહીંતર
    જો (posix_madvise(address, size, POSIX_MADV_DONTNEED)) {
#endif
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#endif
#endif
  જો (release)
    _rpmalloc_stat_sub(&_mapped_pages_os, release >> _memory_page_size_shift);
}

સ્થિર શૂન્ય _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_ગણતરી);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
સ્થિર span_t *_rpmalloc_global_get_reserved_spans(size_t span_ગણતરી) {
  span_t *span = _memory_global_reserve;
  _rpmalloc_span_mark_as_subspan_unless_master(_memory_global_reserve_master,
                                               span, span_ગણતરી);
  memory_global_reserve_ગણતરી -= span_ગણતરી;
  જો (memory_global_reserve_ગણતરી)
    _memory_global_reserve =
        (span_t *)pointer_offset(span, span_ગણતરી << _memory_span_size_shift);
  નહીંતર
    _memory_global_reserve = 0;
  પાછા span;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
સ્થિર શૂન્ય _rpmalloc_global_set_reserved_spans(span_t *master, span_t *reserve,
                                                size_t reserve_span_ગણતરી) {
  _memory_global_reserve_master = master;
  memory_global_reserve_ગણતરી = reserve_span_ગણતરી;
  _memory_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
સ્થિર શૂન્ય _rpmalloc_span_double_link_list_add(span_t **head, span_t *span) {
  જો (*head)
    (*head)->prev = span;
  span->next = *head;
  *head = span;
}

//! Pop head span from double linked list
સ્થિર શૂન્ય rpmalloc_span_double_link_list_કાઢો_head(span_t **head,
                                                     span_t *span) {
  rpmalloc_assert(*head == span, "Linked list corrupted");
  span = *head;
  *head = span->next;
}

//! Remove a span from double linked list
સ્થિર શૂન્ય _rpmalloc_span_double_link_list_remove(span_t **head,
                                                   span_t *span) {
  rpmalloc_assert(*head, "Linked list corrupted");
  જો (*head == span) {
    *head = span->next;
  } નહીંતર {
    span_t *next_span = span->next;
    span_t *prev_span = span->prev;
    prev_span->next = next_span;
    જો (EXPECTED(next_span != 0))
      next_span->prev = prev_span;
  }
}

////////////
///
/// Span control
///
//////

સ્થિર શૂન્ય rpmalloc_heap_cache_ઉમેરો(heap_t *heap, span_t *span);

સ્થિર શૂન્ય _rpmalloc_heap_finalize(heap_t *heap);

સ્થિર શૂન્ય _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_ગણતરી);

//! Declare the span to be a subspan and store distance from master span and
//! span count
સ્થિર શૂન્ય _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_ગણતરી) {
  rpmalloc_assert((subspan != master) || (subspan->flags & SPAN_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  જો (subspan != master) {
    subspan->flags = SPAN_FLAG_SUBSPAN;
    subspan->offset_from_master =
        (uint32_t)((uintptr_t)pointer_diff(subspan, master) >>
                   _memory_span_size_shift);
    subspan->align_offset = 0;
  }
  subspan->span_ગણતરી = (uint32_t)span_ગણતરી;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
સ્થિર span_t *_rpmalloc_span_map_from_reserve(heap_t *heap,
                                               size_t span_ગણતરી) {
  // Update the heap span reserve
  span_t *span = heap->span_reserve;
  heap->span_reserve =
      (span_t *)pointer_offset(span, span_ગણતરી * _memory_span_size);
  heap->spans_reserved -= (uint32_t)span_ગણતરી;

  _rpmalloc_span_mark_as_subspan_unless_master(heap->span_reserve_master, span,
                                               span_ગણતરી);
  જો (span_ગણતરી <= LARGE_CLASS_ગણતરી)
    _rpmalloc_stat_inc(&heap->span_use[span_ગણતરી - 1].spans_from_reserved);

  પાછા span;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
સ્થિર size_t rpmalloc_span_align_ગણતરી(size_t span_ગણતરી) {
  size_t request_ગણતરી = (span_ગણતરી > memory_span_map_ગણતરી)
                             ? span_ગણતરી
                             : memory_span_map_ગણતરી;
  જો ((_memory_page_size > _memory_span_size) &&
      ((request_ગણતરી * _memory_span_size) % _memory_page_size))
    request_ગણતરી +=
        memory_span_map_ગણતરી - (request_ગણતરી % memory_span_map_ગણતરી);
  પાછા request_ગણતરી;
}

//! Setup a newly mapped span
સ્થિર શૂન્ય _rpmalloc_span_initialize(span_t *span, size_t total_span_ગણતરી,
                                      size_t span_ગણતરી, size_t align_offset) {
  span->total_spans = (uint32_t)total_span_ગણતરી;
  span->span_ગણતરી = (uint32_t)span_ગણતરી;
  span->align_offset = (uint32_t)align_offset;
  span->flags = SPAN_FLAG_MASTER;
  atomic_store32(&span->remaining_spans, (int32_t)total_span_ગણતરી);
}

સ્થિર શૂન્ય _rpmalloc_span_unmap(span_t *span);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
સ્થિર span_t *rpmalloc_span_map_aligned_ગણતરી(heap_t *heap,
                                                size_t span_ગણતરી) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_span_ગણતરી = rpmalloc_span_align_ગણતરી(span_ગણતરી);
  size_t align_offset = 0;
  span_t *span = (span_t *)_rpmalloc_mmap(
      aligned_span_ગણતરી * _memory_span_size, &align_offset);
  જો (!span)
    પાછા 0;
  _rpmalloc_span_initialize(span, aligned_span_ગણતરી, span_ગણતરી, align_offset);
  _rpmalloc_stat_inc(&_master_spans);
  જો (span_ગણતરી <= LARGE_CLASS_ગણતરી)
    _rpmalloc_stat_inc(&heap->span_use[span_ગણતરી - 1].spans_map_calls);
  જો (aligned_span_ગણતરી > span_ગણતરી) {
    span_t *reserved_spans =
        (span_t *)pointer_offset(span, span_ગણતરી * _memory_span_size);
    size_t reserved_ગણતરી = aligned_span_ગણતરી - span_ગણતરી;
    જો (heap->spans_reserved) {
      _rpmalloc_span_mark_as_subspan_unless_master(
          heap->span_reserve_master, heap->span_reserve, heap->spans_reserved);
      rpmalloc_heap_cache_ઉમેરો(heap, heap->span_reserve);
    }
    જો (reserved_ગણતરી > memory_heap_reserve_ગણતરી) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&_memory_global_lock) == 1,
                      "Global spin lock not held as expected");
      size_t remain_ગણતરી = reserved_ગણતરી - memory_heap_reserve_ગણતરી;
      reserved_ગણતરી = memory_heap_reserve_ગણતરી;
      span_t *remain_span = (span_t *)pointer_offset(
          reserved_spans, reserved_ગણતરી * _memory_span_size);
      જો (_memory_global_reserve) {
        _rpmalloc_span_mark_as_subspan_unless_master(
            _memory_global_reserve_master, _memory_global_reserve,
            memory_global_reserve_ગણતરી);
        _rpmalloc_span_unmap(_memory_global_reserve);
      }
      _rpmalloc_global_set_reserved_spans(span, remain_span, remain_ગણતરી);
    }
    _rpmalloc_heap_set_reserved_spans(heap, span, reserved_spans,
                                      reserved_ગણતરી);
  }
  પાછા span;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
સ્થિર span_t *_rpmalloc_span_map(heap_t *heap, size_t span_ગણતરી) {
  જો (span_ગણતરી <= heap->spans_reserved)
    પાછા _rpmalloc_span_map_from_reserve(heap, span_ગણતરી);
  span_t *span = 0;
  પૂર્ણાંક use_global_reserve =
      (_memory_page_size > _memory_span_size) ||
      (memory_span_map_ગણતરી > memory_heap_reserve_ગણતરી);
  જો (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    જ્યારે (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
    જો (memory_global_reserve_ગણતરી >= span_ગણતરી) {
      size_t reserve_ગણતરી =
          (!heap->spans_reserved ? memory_heap_reserve_ગણતરી : span_ગણતરી);
      જો (memory_global_reserve_ગણતરી < reserve_ગણતરી)
        reserve_ગણતરી = memory_global_reserve_ગણતરી;
      span = _rpmalloc_global_get_reserved_spans(reserve_ગણતરી);
      જો (span) {
        જો (reserve_ગણતરી > span_ગણતરી) {
          span_t *reserved_span = (span_t *)pointer_offset(
              span, span_ગણતરી << _memory_span_size_shift);
          _rpmalloc_heap_set_reserved_spans(heap, _memory_global_reserve_master,
                                            reserved_span,
                                            reserve_ગણતરી - span_ગણતરી);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        span->span_ગણતરી = (uint32_t)span_ગણતરી;
      }
    }
  }
  જો (!span)
    span = rpmalloc_span_map_aligned_ગણતરી(heap, span_ગણતરી);
  જો (use_global_reserve)
    atomic_store32_release(&_memory_global_lock, 0);
  પાછા span;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
સ્થિર શૂન્ય _rpmalloc_span_unmap(span_t *span) {
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");

  પૂર્ણાંક is_master = !!(span->flags & SPAN_FLAG_MASTER);
  span_t *master =
      is_master ? span
                : ((span_t *)pointer_offset(
                      span, -(intptr_t)((uintptr_t)span->offset_from_master *
                                        _memory_span_size)));
  rpmalloc_assert(is_master || (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");

  size_t span_ગણતરી = span->span_ગણતરી;
  જો (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(span->align_offset == 0, "Span align offset corrupted");
    જો (_memory_span_size >= _memory_page_size)
      _rpmalloc_unmap(span, span_ગણતરી * _memory_span_size, 0, 0);
  } નહીંતર {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    span->flags |=
        SPAN_FLAG_MASTER | SPAN_FLAG_SUBSPAN | SPAN_FLAG_UNMAPPED_MASTER;
    _rpmalloc_stat_add(&_unmapped_master_spans, 1);
  }

  જો (atomic_add32(&master->remaining_spans, -(int32_t)span_ગણતરી) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & SPAN_FLAG_MASTER) &&
                        !!(master->flags & SPAN_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_ગણતરી = master->span_ગણતરી;
    જો (_memory_span_size < _memory_page_size)
      unmap_ગણતરી = master->total_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_ગણતરી * _memory_span_size,
                    master->align_offset,
                    (size_t)master->total_spans * _memory_span_size);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
સ્થિર શૂન્ય _rpmalloc_span_release_to_cache(heap_t *heap, span_t *span) {
  rpmalloc_assert(heap == span->heap, "Span heap pointer corrupted");
  rpmalloc_assert(span->size_class < SIZE_CLASS_ગણતરી,
                  "Invalid span size class");
  rpmalloc_assert(span->span_ગણતરી == 1, "Invalid span count");
#જો ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&heap->span_use[0].current);
#endif
  _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
  જો (!heap->finalize) {
    _rpmalloc_stat_inc(&heap->span_use[0].spans_to_cache);
    _rpmalloc_stat_inc(&heap->size_class_use[span->size_class].spans_to_cache);
    જો (heap->size_class[span->size_class].cache)
      rpmalloc_heap_cache_ઉમેરો(heap,
                                  heap->size_class[span->size_class].cache);
    heap->size_class[span->size_class].cache = span;
  } નહીંતર {
    _rpmalloc_span_unmap(span);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
સ્થિર uint32_t free_list_partial_init(શૂન્ય **list, શૂન્ય **first_block,
                                       શૂન્ય *page_start, શૂન્ય *block_start,
                                       uint32_t block_ગણતરી,
                                       uint32_t block_size) {
  rpmalloc_assert(block_ગણતરી, "Internal failure");
  *first_block = block_start;
  જો (block_ગણતરી > 1) {
    શૂન્ય *free_block = pointer_offset(block_start, block_size);
    શૂન્ય *block_end =
        pointer_offset(block_start, (size_t)block_size * block_ગણતરી);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    જો (block_size < (_memory_page_size >> 1)) {
      શૂન્ય *page_end = pointer_offset(page_start, _memory_page_size);
      જો (page_end < block_end)
        block_end = page_end;
    }
    *list = free_block;
    block_ગણતરી = 2;
    શૂન્ય *next_block = pointer_offset(free_block, block_size);
    જ્યારે (next_block < block_end) {
      *((શૂન્ય **)free_block) = next_block;
      free_block = next_block;
      ++block_ગણતરી;
      next_block = pointer_offset(next_block, block_size);
    }
    *((શૂન્ય **)free_block) = 0;
  } નહીંતર {
    *list = 0;
  }
  પાછા block_ગણતરી;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
સ્થિર શૂન્ય *_rpmalloc_span_initialize_new(heap_t *heap,
                                           heap_size_class_t *heap_size_class,
                                           span_t *span, uint32_t class_idx) {
  rpmalloc_assert(span->span_ગણતરી == 1, "Internal failure");
  size_class_t *size_class = _memory_size_class + class_idx;
  span->size_class = class_idx;
  span->heap = heap;
  span->flags &= ~SPAN_FLAG_ALIGNED_BLOCKS;
  span->block_size = size_class->block_size;
  span->block_ગણતરી = size_class->block_ગણતરી;
  span->free_list = 0;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  શૂન્ય *block;
  span->free_list_limit =
      free_list_partial_init(&heap_size_class->free_list, &block, span,
                             pointer_offset(span, SPAN_HEADER_SIZE),
                             size_class->block_ગણતરી, size_class->block_size);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  જો (span->free_list_limit < span->block_ગણતરી) {
    _rpmalloc_span_double_link_list_add(&heap_size_class->partial_span, span);
    span->used_ગણતરી = span->free_list_limit;
  } નહીંતર {
#જો RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_ગણતરી;
    span->used_ગણતરી = span->block_ગણતરી;
  }
  પાછા block;
}

સ્થિર શૂન્ય _rpmalloc_span_extract_free_list_deferred(span_t *span) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  કરો {
    span->free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } જ્યારે (span->free_list == INVALID_POINTER);
  span->used_ગણતરી -= span->list_size;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);
}

સ્થિર પૂર્ણાંક _rpmalloc_span_is_fully_utilized(span_t *span) {
  rpmalloc_assert(span->free_list_limit <= span->block_ગણતરી,
                  "Span free list corrupted");
  પાછા !span->free_list && (span->free_list_limit >= span->block_ગણતરી);
}

સ્થિર પૂર્ણાંક _rpmalloc_span_finalize(heap_t *heap, size_t iclass, span_t *span,
                                   span_t **list_head) {
  શૂન્ય *free_list = heap->size_class[iclass].free_list;
  span_t *class_span = (span_t *)((uintptr_t)free_list & _memory_span_mask);
  જો (span == class_span) {
    // Adopt the heap class free list back into the span free list
    શૂન્ય *block = span->free_list;
    શૂન્ય *last_block = 0;
    જ્યારે (block) {
      last_block = block;
      block = *((શૂન્ય **)block);
    }
    uint32_t free_ગણતરી = 0;
    block = free_list;
    જ્યારે (block) {
      ++free_ગણતરી;
      block = *((શૂન્ય **)block);
    }
    જો (last_block) {
      *((શૂન્ય **)last_block) = free_list;
    } નહીંતર {
      span->free_list = free_list;
    }
    heap->size_class[iclass].free_list = 0;
    span->used_ગણતરી -= free_ગણતરી;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(span->list_size == span->used_ગણતરી, "Memory leak detected");
  જો (span->list_size == span->used_ગણતરી) {
    _rpmalloc_stat_dec(&heap->span_use[0].current);
    _rpmalloc_stat_dec(&heap->size_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    જો (list_head)
      _rpmalloc_span_double_link_list_remove(list_head, span);
    _rpmalloc_span_unmap(span);
    પાછા 1;
  }
  પાછા 0;
}

////////////
///
/// Global cache
///
//////

#જો ENABLE_GLOBAL_CACHE

//! Finalize a global cache
સ્થિર શૂન્ય _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  જ્યારે (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

  માટે (size_t ispan = 0; ispan < cache->ગણતરી; ++ispan)
    _rpmalloc_span_unmap(cache->span[ispan]);
  cache->ગણતરી = 0;

  જ્યારે (cache->overflow) {
    span_t *span = cache->overflow;
    cache->overflow = span->next;
    _rpmalloc_span_unmap(span);
  }

  atomic_store32_release(&cache->lock, 0);
}

સ્થિર શૂન્ય rpmalloc_global_cache_ઉમેરો_spans(span_t **span,
                                                size_t span_ગણતરી,
                                                size_t ગણતરી) {
  અચલ size_t cache_limit =
      (span_ગણતરી == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_SPAN_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_SPAN_LARGE_CACHE - (span_ગણતરી >> 1));

  global_cache_t *cache = &_memory_span_cache[span_ગણતરી - 1];

  size_t ઉમેરો_ગણતરી = ગણતરી;
  જ્યારે (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#જો ENABLE_STATISTICS
  cache->ઉમેરો_ગણતરી += ગણતરી;
#endif
  જો ((cache->ગણતરી + ઉમેરો_ગણતરી) > cache_limit)
    ઉમેરો_ગણતરી = cache_limit - cache->ગણતરી;

  memcpy(cache->span + cache->ગણતરી, span, કદ(span_t *) * ઉમેરો_ગણતરી);
  cache->ગણતરી += (uint32_t)ઉમેરો_ગણતરી;

#જો ENABLE_UNLIMITED_CACHE
  જ્યારે (ઉમેરો_ગણતરી < ગણતરી) {
#નહીંતર
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  જ્યારે ((_memory_page_size > _memory_span_size) && (ઉમેરો_ગણતરી < ગણતરી)) {
#endif
    span_t *current_span = span[ઉમેરો_ગણતરી++];
    current_span->next = cache->overflow;
    cache->overflow = current_span;
  }
  atomic_store32_release(&cache->lock, 0);

  span_t *keep = 0;
  માટે (size_t ispan = ઉમેરો_ગણતરી; ispan < ગણતરી; ++ispan) {
    span_t *current_span = span[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    જો ((current_span->flags & SPAN_FLAG_MASTER) &&
        (atomic_load32(&current_span->remaining_spans) >
         (int32_t)current_span->span_ગણતરી)) {
      current_span->next = keep;
      keep = current_span;
    } નહીંતર {
      _rpmalloc_span_unmap(current_span);
    }
  }

  જો (keep) {
    જ્યારે (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    જ્યારે (keep) {
      માટે (; islot < cache->ગણતરી; ++islot) {
        span_t *current_span = cache->span[islot];
        જો (!(current_span->flags & SPAN_FLAG_MASTER) ||
            ((current_span->flags & SPAN_FLAG_MASTER) &&
             (atomic_load32(&current_span->remaining_spans) <=
              (int32_t)current_span->span_ગણતરી))) {
          _rpmalloc_span_unmap(current_span);
          cache->span[islot] = keep;
          તોડો;
        }
      }
      જો (islot == cache->ગણતરી)
        તોડો;
      keep = keep->next;
    }

    જો (keep) {
      span_t *tail = keep;
      જ્યારે (tail->next)
        tail = tail->next;
      tail->next = cache->overflow;
      cache->overflow = keep;
    }

    atomic_store32_release(&cache->lock, 0);
  }
}

સ્થિર size_t _rpmalloc_global_cache_extract_spans(span_t **span,
                                                   size_t span_ગણતરી,
                                                   size_t ગણતરી) {
  global_cache_t *cache = &_memory_span_cache[span_ગણતરી - 1];

  size_t extract_ગણતરી = 0;
  જ્યારે (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#જો ENABLE_STATISTICS
  cache->extract_ગણતરી += ગણતરી;
#endif
  size_t want = ગણતરી - extract_ગણતરી;
  જો (want > cache->ગણતરી)
    want = cache->ગણતરી;

  memcpy(span + extract_ગણતરી, cache->span + (cache->ગણતરી - want),
         કદ(span_t *) * want);
  cache->ગણતરી -= (uint32_t)want;
  extract_ગણતરી += want;

  જ્યારે ((extract_ગણતરી < ગણતરી) && cache->overflow) {
    span_t *current_span = cache->overflow;
    span[extract_ગણતરી++] = current_span;
    cache->overflow = current_span->next;
  }

#જો ENABLE_ASSERTS
  માટે (size_t ispan = 0; ispan < extract_ગણતરી; ++ispan) {
    rpmalloc_assert(span[ispan]->span_ગણતરી == span_ગણતરી,
                    "Global cache span count mismatch");
  }
#endif

  atomic_store32_release(&cache->lock, 0);

  પાછા extract_ગણતરી;
}

#endif

////////////
///
/// Heap control
///
//////

સ્થિર શૂન્ય _rpmalloc_deallocate_huge(span_t *);

//! Store the given spans as reserve in the given heap
સ્થિર શૂન્ય _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_ગણતરી) {
  heap->span_reserve_master = master;
  heap->span_reserve = reserve;
  heap->spans_reserved = (uint32_t)reserve_span_ગણતરી;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
સ્થિર શૂન્ય _rpmalloc_heap_cache_adopt_deferred(heap_t *heap,
                                                span_t **single_span) {
  span_t *span = (span_t *)((શૂન્ય *)atomic_exchange_ptr_acquire(
      &heap->span_free_deferred, 0));
  જ્યારે (span) {
    span_t *next_span = (span_t *)span->free_list;
    rpmalloc_assert(span->heap == heap, "Span heap pointer corrupted");
    જો (EXPECTED(span->size_class < SIZE_CLASS_ગણતરી)) {
      rpmalloc_assert(heap->full_span_ગણતરી, "Heap span counter corrupted");
      --heap->full_span_ગણતરી;
      _rpmalloc_stat_dec(&heap->span_use[0].spans_deferred);
#જો RPMALLOC_FIRST_CLASS_HEAPS
      _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                             span);
#endif
      _rpmalloc_stat_dec(&heap->span_use[0].current);
      _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
      જો (single_span && !*single_span)
        *single_span = span;
      નહીંતર
        rpmalloc_heap_cache_ઉમેરો(heap, span);
    } નહીંતર {
      જો (span->size_class == SIZE_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(span);
      } નહીંતર {
        rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(heap->full_span_ગણતરી, "Heap span counter corrupted");
        --heap->full_span_ગણતરી;
#જો RPMALLOC_FIRST_CLASS_HEAPS
        _rpmalloc_span_double_link_list_remove(&heap->large_huge_span, span);
#endif
        uint32_t idx = span->span_ગણતરી - 1;
        _rpmalloc_stat_dec(&heap->span_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&heap->span_use[idx].current);
        જો (!idx && single_span && !*single_span)
          *single_span = span;
        નહીંતર
          rpmalloc_heap_cache_ઉમેરો(heap, span);
      }
    }
    span = next_span;
  }
}

સ્થિર શૂન્ય _rpmalloc_heap_unmap(heap_t *heap) {
  જો (!heap->master_heap) {
    જો ((heap->finalize > 1) && !atomic_load32(&heap->child_ગણતરી)) {
      span_t *span = (span_t *)((uintptr_t)heap & _memory_span_mask);
      _rpmalloc_span_unmap(span);
    }
  } નહીંતર {
    જો (atomic_decr32(&heap->master_heap->child_ગણતરી) == 0) {
      _rpmalloc_heap_unmap(heap->master_heap);
    }
  }
}

સ્થિર શૂન્ય _rpmalloc_heap_global_finalize(heap_t *heap) {
  જો (heap->finalize++ > 1) {
    --heap->finalize;
    પાછા;
  }

  _rpmalloc_heap_finalize(heap);

#જો ENABLE_THREAD_CACHE
  માટે (size_t iclass = 0; iclass < LARGE_CLASS_ગણતરી; ++iclass) {
    span_cache_t *span_cache;
    જો (!iclass)
      span_cache = &heap->span_cache;
    નહીંતર
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    માટે (size_t ispan = 0; ispan < span_cache->ગણતરી; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->ગણતરી = 0;
  }
#endif

  જો (heap->full_span_ગણતરી) {
    --heap->finalize;
    પાછા;
  }

  માટે (size_t iclass = 0; iclass < SIZE_CLASS_ગણતરી; ++iclass) {
    જો (heap->size_class[iclass].free_list ||
        heap->size_class[iclass].partial_span) {
      --heap->finalize;
      પાછા;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap_t *list_heap = _memory_heaps[list_idx];
  જો (list_heap == heap) {
    _memory_heaps[list_idx] = heap->next_heap;
  } નહીંતર {
    જ્યારે (list_heap->next_heap != heap)
      list_heap = list_heap->next_heap;
    list_heap->next_heap = heap->next_heap;
  }

  _rpmalloc_heap_unmap(heap);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
સ્થિર શૂન્ય rpmalloc_heap_cache_ઉમેરો(heap_t *heap, span_t *span) {
  જો (UNEXPECTED(heap->finalize != 0)) {
    _rpmalloc_span_unmap(span);
    _rpmalloc_heap_global_finalize(heap);
    પાછા;
  }
#જો ENABLE_THREAD_CACHE
  size_t span_ગણતરી = span->span_ગણતરી;
  _rpmalloc_stat_inc(&heap->span_use[span_ગણતરી - 1].spans_to_cache);
  જો (span_ગણતરી == 1) {
    span_cache_t *span_cache = &heap->span_cache;
    span_cache->span[span_cache->ગણતરી++] = span;
    જો (span_cache->ગણતરી == MAX_THREAD_SPAN_CACHE) {
      અચલ size_t remain_ગણતરી =
          MAX_THREAD_SPAN_CACHE - THREAD_SPAN_CACHE_TRANSFER;
#જો ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           THREAD_SPAN_CACHE_TRANSFER * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_ગણતરી - 1].spans_to_global,
                         THREAD_SPAN_CACHE_TRANSFER);
      rpmalloc_global_cache_ઉમેરો_spans(span_cache->span + remain_ગણતરી,
                                          span_ગણતરી,
                                          THREAD_SPAN_CACHE_TRANSFER);
#નહીંતર
      માટે (size_t ispan = 0; ispan < THREAD_SPAN_CACHE_TRANSFER; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_ગણતરી + ispan]);
#endif
      span_cache->ગણતરી = remain_ગણતરી;
    }
  } નહીંતર {
    size_t cache_idx = span_ગણતરી - 2;
    span_large_cache_t *span_cache = heap->span_large_cache + cache_idx;
    span_cache->span[span_cache->ગણતરી++] = span;
    અચલ size_t cache_limit =
        (MAX_THREAD_SPAN_LARGE_CACHE - (span_ગણતરી >> 1));
    જો (span_cache->ગણતરી == cache_limit) {
      અચલ size_t transfer_limit = 2 + (cache_limit >> 2);
      અચલ size_t transfer_ગણતરી =
          (THREAD_SPAN_LARGE_CACHE_TRANSFER <= transfer_limit
               ? THREAD_SPAN_LARGE_CACHE_TRANSFER
               : transfer_limit);
      અચલ size_t remain_ગણતરી = cache_limit - transfer_ગણતરી;
#જો ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           transfer_ગણતરી * span_ગણતરી * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_ગણતરી - 1].spans_to_global,
                         transfer_ગણતરી);
      rpmalloc_global_cache_ઉમેરો_spans(span_cache->span + remain_ગણતરી,
                                          span_ગણતરી, transfer_ગણતરી);
#નહીંતર
      માટે (size_t ispan = 0; ispan < transfer_ગણતરી; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_ગણતરી + ispan]);
#endif
      span_cache->ગણતરી = remain_ગણતરી;
    }
  }
#નહીંતર
  (શૂન્ય)કદ(heap);
  _rpmalloc_span_unmap(span);
#endif
}

//! Extract the given number of spans from the different cache levels
સ્થિર span_t *_rpmalloc_heap_thread_cache_extract(heap_t *heap,
                                                   size_t span_ગણતરી) {
  span_t *span = 0;
#જો ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  જો (span_ગણતરી == 1)
    span_cache = &heap->span_cache;
  નહીંતર
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_ગણતરી - 2));
  જો (span_cache->ગણતરી) {
    _rpmalloc_stat_inc(&heap->span_use[span_ગણતરી - 1].spans_from_cache);
    પાછા span_cache->span[--span_cache->ગણતરી];
  }
#endif
  પાછા span;
}

સ્થિર span_t *_rpmalloc_heap_thread_cache_deferred_extract(heap_t *heap,
                                                            size_t span_ગણતરી) {
  span_t *span = 0;
  જો (span_ગણતરી == 1) {
    _rpmalloc_heap_cache_adopt_deferred(heap, &span);
  } નહીંતર {
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
    span = _rpmalloc_heap_thread_cache_extract(heap, span_ગણતરી);
  }
  પાછા span;
}

સ્થિર span_t *_rpmalloc_heap_reserved_extract(heap_t *heap,
                                               size_t span_ગણતરી) {
  જો (heap->spans_reserved >= span_ગણતરી)
    પાછા _rpmalloc_span_map(heap, span_ગણતરી);
  પાછા 0;
}

//! Extract a span from the global cache
સ્થિર span_t *_rpmalloc_heap_global_cache_extract(heap_t *heap,
                                                   size_t span_ગણતરી) {
#જો ENABLE_GLOBAL_CACHE
#જો ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  size_t wanted_ગણતરી;
  જો (span_ગણતરી == 1) {
    span_cache = &heap->span_cache;
    wanted_ગણતરી = THREAD_SPAN_CACHE_TRANSFER;
  } નહીંતર {
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_ગણતરી - 2));
    wanted_ગણતરી = THREAD_SPAN_LARGE_CACHE_TRANSFER;
  }
  span_cache->ગણતરી = _rpmalloc_global_cache_extract_spans(
      span_cache->span, span_ગણતરી, wanted_ગણતરી);
  જો (span_cache->ગણતરી) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_ગણતરી * span_cache->ગણતરી * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_ગણતરી - 1].spans_from_global,
                       span_cache->ગણતરી);
    પાછા span_cache->span[--span_cache->ગણતરી];
  }
#નહીંતર
  span_t *span = 0;
  size_t ગણતરી = _rpmalloc_global_cache_extract_spans(&span, span_ગણતરી, 1);
  જો (ગણતરી) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_ગણતરી * ગણતરી * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_ગણતરી - 1].spans_from_global,
                       ગણતરી);
    પાછા span;
  }
#endif
#endif
  (શૂન્ય)કદ(heap);
  (શૂન્ય)કદ(span_ગણતરી);
  પાછા 0;
}

સ્થિર શૂન્ય _rpmalloc_inc_span_statistics(heap_t *heap, size_t span_ગણતરી,
                                          uint32_t class_idx) {
  (શૂન્ય)કદ(heap);
  (શૂન્ય)કદ(span_ગણતરી);
  (શૂન્ય)કદ(class_idx);
#જો ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)span_ગણતરી - 1;
  uint32_t current_ગણતરી =
      (uint32_t)atomic_incr32(&heap->span_use[idx].current);
  જો (current_ગણતરી > (uint32_t)atomic_load32(&heap->span_use[idx].ઉપલું))
    atomic_store32(&heap->span_use[idx].ઉપલું, (int32_t)current_ગણતરી);
  _rpmalloc_stat_add_peak(&heap->size_class_use[class_idx].spans_current, 1,
                          heap->size_class_use[class_idx].spans_peak);
#endif
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
સ્થિર span_t *
_rpmalloc_heap_extract_new_span(heap_t *heap,
                                heap_size_class_t *heap_size_class,
                                size_t span_ગણતરી, uint32_t class_idx) {
  span_t *span;
#જો ENABLE_THREAD_CACHE
  જો (heap_size_class && heap_size_class->cache) {
    span = heap_size_class->cache;
    heap_size_class->cache =
        (heap->span_cache.ગણતરી
             ? heap->span_cache.span[--heap->span_cache.ગણતરી]
             : 0);
    _rpmalloc_inc_span_statistics(heap, span_ગણતરી, class_idx);
    પાછા span;
  }
#endif
  (શૂન્ય)કદ(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t base_span_ગણતરી = span_ગણતરી;
  size_t limit_span_ગણતરી =
      (span_ગણતરી > 2) ? (span_ગણતરી + (span_ગણતરી >> 1)) : span_ગણતરી;
  જો (limit_span_ગણતરી > LARGE_CLASS_ગણતરી)
    limit_span_ગણતરી = LARGE_CLASS_ગણતરી;
  કરો {
    span = _rpmalloc_heap_thread_cache_extract(heap, span_ગણતરી);
    જો (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_ગણતરી, class_idx);
      પાછા span;
    }
    span = _rpmalloc_heap_thread_cache_deferred_extract(heap, span_ગણતરી);
    જો (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_ગણતરી, class_idx);
      પાછા span;
    }
    span = _rpmalloc_heap_global_cache_extract(heap, span_ગણતરી);
    જો (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_ગણતરી, class_idx);
      પાછા span;
    }
    span = _rpmalloc_heap_reserved_extract(heap, span_ગણતરી);
    જો (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_reserved);
      _rpmalloc_inc_span_statistics(heap, span_ગણતરી, class_idx);
      પાછા span;
    }
    ++span_ગણતરી;
  } જ્યારે (span_ગણતરી <= limit_span_ગણતરી);
  // Final fallback, map in more virtual memory
  span = _rpmalloc_span_map(heap, base_span_ગણતરી);
  _rpmalloc_inc_span_statistics(heap, base_span_ગણતરી, class_idx);
  _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_map_calls);
  પાછા span;
}

સ્થિર શૂન્ય _rpmalloc_heap_initialize(heap_t *heap) {
  _rpmalloc_memset_const(heap, 0, કદ(heap_t));
  // Get a new heap ID
  heap->id = 1 + atomic_incr32(&_memory_heap_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap->next_heap = _memory_heaps[list_idx];
  _memory_heaps[list_idx] = heap;
}

સ્થિર શૂન્ય _rpmalloc_heap_orphan(heap_t *heap, પૂર્ણાંક first_class) {
  heap->owner_thread = (uintptr_t)-1;
#જો RPMALLOC_FIRST_CLASS_HEAPS
  heap_t **heap_list =
      (first_class ? &_memory_first_class_orphan_heaps : &_memory_orphan_heaps);
#નહીંતર
  (શૂન્ય)કદ(first_class);
  heap_t **heap_list = &_memory_orphan_heaps;
#endif
  heap->next_orphan = *heap_list;
  *heap_list = heap;
}

//! Allocate a new heap from newly mapped memory pages
સ્થિર heap_t *_rpmalloc_heap_allocate_new(શૂન્ય) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t heap_size = કદ(heap_t);
  size_t aligned_heap_size = 16 * ((heap_size + 15) / 16);
  size_t request_heap_ગણતરી = 16;
  size_t heap_span_ગણતરી = ((aligned_heap_size * request_heap_ગણતરી) +
                            કદ(span_t) + _memory_span_size - 1) /
                           _memory_span_size;
  size_t block_size = _memory_span_size * heap_span_ગણતરી;
  size_t span_ગણતરી = heap_span_ગણતરી;
  span_t *span = 0;
  // If there are global reserved spans, use these first
  જો (memory_global_reserve_ગણતરી >= heap_span_ગણતરી) {
    span = _rpmalloc_global_get_reserved_spans(heap_span_ગણતરી);
  }
  જો (!span) {
    જો (_memory_page_size > block_size) {
      span_ગણતરી = _memory_page_size / _memory_span_size;
      block_size = _memory_page_size;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_heap_ગણતરી =
          (block_size - કદ(span_t)) / aligned_heap_size;
      જો (possible_heap_ગણતરી >= (request_heap_ગણતરી * 16))
        request_heap_ગણતરી *= 16;
      નહીંતર જો (possible_heap_ગણતરી < request_heap_ગણતરી)
        request_heap_ગણતરી = possible_heap_ગણતરી;
      heap_span_ગણતરી = ((aligned_heap_size * request_heap_ગણતરી) +
                         કદ(span_t) + _memory_span_size - 1) /
                        _memory_span_size;
    }

    size_t align_offset = 0;
    span = (span_t *)_rpmalloc_mmap(block_size, &align_offset);
    જો (!span)
      પાછા 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    _rpmalloc_span_initialize(span, span_ગણતરી, heap_span_ગણતરી, align_offset);
  }

  size_t remain_size = _memory_span_size - કદ(span_t);
  heap_t *heap = (heap_t *)pointer_offset(span, કદ(span_t));
  _rpmalloc_heap_initialize(heap);

  // Put extra heaps as orphans
  size_t અંક_heaps = remain_size / aligned_heap_size;
  જો (અંક_heaps < request_heap_ગણતરી)
    અંક_heaps = request_heap_ગણતરી;
  atomic_store32(&heap->child_ગણતરી, (int32_t)અંક_heaps - 1);
  heap_t *extra_heap = (heap_t *)pointer_offset(heap, aligned_heap_size);
  જ્યારે (અંક_heaps > 1) {
    _rpmalloc_heap_initialize(extra_heap);
    extra_heap->master_heap = heap;
    _rpmalloc_heap_orphan(extra_heap, 1);
    extra_heap = (heap_t *)pointer_offset(extra_heap, aligned_heap_size);
    --અંક_heaps;
  }

  જો (span_ગણતરી > heap_span_ગણતરી) {
    // Cap reserved spans
    size_t remain_ગણતરી = span_ગણતરી - heap_span_ગણતરી;
    size_t reserve_ગણતરી =
        (remain_ગણતરી > memory_heap_reserve_ગણતરી ? memory_heap_reserve_ગણતરી
                                                   : remain_ગણતરી);
    span_t *remain_span =
        (span_t *)pointer_offset(span, heap_span_ગણતરી * _memory_span_size);
    _rpmalloc_heap_set_reserved_spans(heap, span, remain_span, reserve_ગણતરી);

    જો (remain_ગણતરી > reserve_ગણતરી) {
      // Set to global reserved spans
      remain_span = (span_t *)pointer_offset(remain_span,
                                             reserve_ગણતરી * _memory_span_size);
      reserve_ગણતરી = remain_ગણતરી - reserve_ગણતરી;
      _rpmalloc_global_set_reserved_spans(span, remain_span, reserve_ગણતરી);
    }
  }

  પાછા heap;
}

સ્થિર heap_t *_rpmalloc_heap_extract_orphan(heap_t **heap_list) {
  heap_t *heap = *heap_list;
  *heap_list = (heap ? heap->next_orphan : 0);
  પાછા heap;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
સ્થિર heap_t *_rpmalloc_heap_allocate(પૂર્ણાંક first_class) {
  heap_t *heap = 0;
  જ્યારે (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
    _rpmalloc_spin();
  જો (first_class == 0)
    heap = _rpmalloc_heap_extract_orphan(&_memory_orphan_heaps);
#જો RPMALLOC_FIRST_CLASS_HEAPS
  જો (!heap)
    heap = _rpmalloc_heap_extract_orphan(&_memory_first_class_orphan_heaps);
#endif
  જો (!heap)
    heap = _rpmalloc_heap_allocate_new();
  atomic_store32_release(&_memory_global_lock, 0);
  જો (heap)
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  પાછા heap;
}

સ્થિર શૂન્ય _rpmalloc_heap_release(શૂન્ય *heapptr, પૂર્ણાંક first_class,
                                   પૂર્ણાંક release_cache) {
  heap_t *heap = (heap_t *)heapptr;
  જો (!heap)
    પાછા;
  // Release thread cache spans back to global cache
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  જો (release_cache || heap->finalize) {
#જો ENABLE_THREAD_CACHE
    માટે (size_t iclass = 0; iclass < LARGE_CLASS_ગણતરી; ++iclass) {
      span_cache_t *span_cache;
      જો (!iclass)
        span_cache = &heap->span_cache;
      નહીંતર
        span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
      જો (!span_cache->ગણતરી)
        ચાલુ;
#જો ENABLE_GLOBAL_CACHE
      જો (heap->finalize) {
        માટે (size_t ispan = 0; ispan < span_cache->ગણતરી; ++ispan)
          _rpmalloc_span_unmap(span_cache->span[ispan]);
      } નહીંતર {
        _rpmalloc_stat_add64(&heap->thread_to_global, span_cache->ગણતરી *
                                                          (iclass + 1) *
                                                          _memory_span_size);
        _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                           span_cache->ગણતરી);
        rpmalloc_global_cache_ઉમેરો_spans(span_cache->span, iclass + 1,
                                            span_cache->ગણતરી);
      }
#નહીંતર
      માટે (size_t ispan = 0; ispan < span_cache->ગણતરી; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
      span_cache->ગણતરી = 0;
    }
#endif
  }

  જો (get_thread_heap_raw() == heap)
    set_thread_heap(0);

#જો ENABLE_STATISTICS
  atomic_decr32(&_memory_active_heaps);
  rpmalloc_assert(atomic_load32(&_memory_active_heaps) >= 0,
                  "Still active heaps during finalization");
#endif

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  જો (get_thread_id() != _rpmalloc_main_thread_id) {
    જ્યારે (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
  }
  _rpmalloc_heap_orphan(heap, first_class);
  atomic_store32_release(&_memory_global_lock, 0);
}

સ્થિર શૂન્ય _rpmalloc_heap_release_raw(શૂન્ય *heapptr, પૂર્ણાંક release_cache) {
  _rpmalloc_heap_release(heapptr, 0, release_cache);
}

સ્થિર શૂન્ય _rpmalloc_heap_release_raw_fc(શૂન્ય *heapptr) {
  _rpmalloc_heap_release_raw(heapptr, 1);
}

સ્થિર શૂન્ય _rpmalloc_heap_finalize(heap_t *heap) {
  જો (heap->spans_reserved) {
    span_t *span = _rpmalloc_span_map(heap, heap->spans_reserved);
    _rpmalloc_span_unmap(span);
    heap->spans_reserved = 0;
  }

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  માટે (size_t iclass = 0; iclass < SIZE_CLASS_ગણતરી; ++iclass) {
    જો (heap->size_class[iclass].cache)
      _rpmalloc_span_unmap(heap->size_class[iclass].cache);
    heap->size_class[iclass].cache = 0;
    span_t *span = heap->size_class[iclass].partial_span;
    જ્યારે (span) {
      span_t *next = span->next;
      _rpmalloc_span_finalize(heap, iclass, span,
                              &heap->size_class[iclass].partial_span);
      span = next;
    }
    // If class still has a free list it must be a full span
    જો (heap->size_class[iclass].free_list) {
      span_t *class_span =
          (span_t *)((uintptr_t)heap->size_class[iclass].free_list &
                     _memory_span_mask);
      span_t **list = 0;
#જો RPMALLOC_FIRST_CLASS_HEAPS
      list = &heap->full_span[iclass];
#endif
      --heap->full_span_ગણતરી;
      જો (!_rpmalloc_span_finalize(heap, iclass, class_span, list)) {
        જો (list)
          _rpmalloc_span_double_link_list_remove(list, class_span);
        _rpmalloc_span_double_link_list_add(
            &heap->size_class[iclass].partial_span, class_span);
      }
    }
  }

#જો ENABLE_THREAD_CACHE
  માટે (size_t iclass = 0; iclass < LARGE_CLASS_ગણતરી; ++iclass) {
    span_cache_t *span_cache;
    જો (!iclass)
      span_cache = &heap->span_cache;
    નહીંતર
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    માટે (size_t ispan = 0; ispan < span_cache->ગણતરી; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->ગણતરી = 0;
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
સ્થિર શૂન્ય *free_list_કાઢો(શૂન્ય **list) {
  શૂન્ય *block = *list;
  *list = *((શૂન્ય **)block);
  પાછા block;
}

//! Allocate a small/medium sized memory block from the given heap
સ્થિર શૂન્ય *_rpmalloc_allocate_from_heap_fallback(
    heap_t *heap, heap_size_class_t *heap_size_class, uint32_t class_idx) {
  span_t *span = heap_size_class->partial_span;
  rpmalloc_assume(heap != 0);
  જો (EXPECTED(span != 0)) {
    rpmalloc_assert(span->block_ગણતરી ==
                        _memory_size_class[span->size_class].block_ગણતરી,
                    "Span block count corrupted");
    rpmalloc_assert(!_rpmalloc_span_is_fully_utilized(span),
                    "Internal failure");
    શૂન્ય *block;
    જો (span->free_list) {
      // Span local free list is not empty, swap to size class free list
      block = free_list_કાઢો(&span->free_list);
      heap_size_class->free_list = span->free_list;
      span->free_list = 0;
    } નહીંતર {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      શૂન્ય *block_start = pointer_offset(
          span, SPAN_HEADER_SIZE +
                    ((size_t)span->free_list_limit * span->block_size));
      span->free_list_limit += free_list_partial_init(
          &heap_size_class->free_list, &block,
          (શૂન્ય *)((uintptr_t)block_start & ~(_memory_page_size - 1)),
          block_start, span->block_ગણતરી - span->free_list_limit,
          span->block_size);
    }
    rpmalloc_assert(span->free_list_limit <= span->block_ગણતરી,
                    "Span block count corrupted");
    span->used_ગણતરી = span->free_list_limit;

    // Swap in deferred free list if present
    જો (atomic_load_ptr(&span->free_list_deferred))
      _rpmalloc_span_extract_free_list_deferred(span);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    જો (!_rpmalloc_span_is_fully_utilized(span))
      પાછા block;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_span_double_link_list_કાઢો_head(&heap_size_class->partial_span,
                                             span);
#જો RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_ગણતરી;
    પાછા block;
  }

  // Find a span in one of the cache levels
  span = _rpmalloc_heap_extract_new_span(heap, heap_size_class, 1, class_idx);
  જો (EXPECTED(span != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    પાછા _rpmalloc_span_initialize_new(heap, heap_size_class, span,
                                         class_idx);
  }

  પાછા 0;
}

//! Allocate a small sized memory block from the given heap
સ્થિર શૂન્ય *_rpmalloc_allocate_small(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Small sizes have unique size classes
  અચલ uint32_t class_idx =
      (uint32_t)((size + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_SHIFT);
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  જો (EXPECTED(heap_size_class->free_list != 0))
    પાછા free_list_કાઢો(&heap_size_class->free_list);
  પાછા _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
સ્થિર શૂન્ય *_rpmalloc_allocate_medium(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  અચલ uint32_t base_idx =
      (uint32_t)(SMALL_CLASS_ગણતરી +
                 ((size - (SMALL_SIZE_LIMIT + 1)) >> MEDIUM_GRANULARITY_SHIFT));
  અચલ uint32_t class_idx = _memory_size_class[base_idx].class_idx;
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  જો (EXPECTED(heap_size_class->free_list != 0))
    પાછા free_list_કાઢો(&heap_size_class->free_list);
  પાછા _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
સ્થિર શૂન્ય *_rpmalloc_allocate_large(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  size += SPAN_HEADER_SIZE;
  size_t span_ગણતરી = size >> _memory_span_size_shift;
  જો (size & (_memory_span_size - 1))
    ++span_ગણતરી;

  // Find a span in one of the cache levels
  span_t *span =
      _rpmalloc_heap_extract_new_span(heap, 0, span_ગણતરી, SIZE_CLASS_LARGE);
  જો (!span)
    પાછા span;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(span->span_ગણતરી >= span_ગણતરી, "Internal failure");
  span->size_class = SIZE_CLASS_LARGE;
  span->heap = heap;

#જો RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_ગણતરી;

  પાછા pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a huge block by mapping memory pages directly
સ્થિર શૂન્ય *_rpmalloc_allocate_huge(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  size += SPAN_HEADER_SIZE;
  size_t અંક_pages = size >> _memory_page_size_shift;
  જો (size & (_memory_page_size - 1))
    ++અંક_pages;
  size_t align_offset = 0;
  span_t *span =
      (span_t *)_rpmalloc_mmap(અંક_pages * _memory_page_size, &align_offset);
  જો (!span)
    પાછા span;

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_ગણતરી = (uint32_t)અંક_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, અંક_pages, _huge_pages_peak);

#જો RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_ગણતરી;

  પાછા pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a block of the given size
સ્થિર શૂન્ય *_rpmalloc_allocate(heap_t *heap, size_t size) {
  _rpmalloc_stat_add64(&_allocation_counter, 1);
  જો (EXPECTED(size <= SMALL_SIZE_LIMIT))
    પાછા _rpmalloc_allocate_small(heap, size);
  નહીંતર જો (size <= _memory_medium_size_limit)
    પાછા _rpmalloc_allocate_medium(heap, size);
  નહીંતર જો (size <= LARGE_SIZE_LIMIT)
    પાછા _rpmalloc_allocate_large(heap, size);
  પાછા _rpmalloc_allocate_huge(heap, size);
}

સ્થિર શૂન્ય *_rpmalloc_aligned_allocate(heap_t *heap, size_t alignment,
                                        size_t size) {
  જો (alignment <= SMALL_GRANULARITY)
    પાછા _rpmalloc_allocate(heap, size);

#જો ENABLE_VALIDATE_ARGS
  જો ((size + alignment) < size) {
    errno = EINVAL;
    પાછા 0;
  }
  જો (alignment & (alignment - 1)) {
    errno = EINVAL;
    પાછા 0;
  }
#endif

  જો ((alignment <= SPAN_HEADER_SIZE) &&
      ((size + SPAN_HEADER_SIZE) < _memory_medium_size_limit)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t multiple_size = size ? (size + (SPAN_HEADER_SIZE - 1)) &
                                      ~(uintptr_t)(SPAN_HEADER_SIZE - 1)
                                : SPAN_HEADER_SIZE;
    rpmalloc_assert(!(multiple_size % SPAN_HEADER_SIZE),
                    "Failed alignment calculation");
    જો (multiple_size <= (size + alignment))
      પાછા _rpmalloc_allocate(heap, multiple_size);
  }

  શૂન્ય *ptr = 0;
  size_t align_mask = alignment - 1;
  જો (alignment <= _memory_page_size) {
    ptr = _rpmalloc_allocate(heap, size + alignment);
    જો ((uintptr_t)ptr & align_mask) {
      ptr = (શૂન્ય *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);
      // Mark as having aligned blocks
      span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
      span->flags |= SPAN_FLAG_ALIGNED_BLOCKS;
    }
    પાછા ptr;
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
  જો (alignment & align_mask) {
    errno = EINVAL;
    પાછા 0;
  }
  જો (alignment >= _memory_span_size) {
    errno = EINVAL;
    પાછા 0;
  }

  size_t extra_pages = alignment / _memory_page_size;

  // Since each span has a header, we will at least need one extra memory page
  size_t અંક_pages = 1 + (size / _memory_page_size);
  જો (size & (_memory_page_size - 1))
    ++અંક_pages;

  જો (extra_pages > અંક_pages)
    અંક_pages = 1 + extra_pages;

  size_t original_pages = અંક_pages;
  size_t limit_pages = (_memory_span_size / _memory_page_size) * 2;
  જો (limit_pages < (original_pages * 2))
    limit_pages = original_pages * 2;

  size_t mapped_size, align_offset;
  span_t *span;

retry:
  align_offset = 0;
  mapped_size = અંક_pages * _memory_page_size;

  span = (span_t *)_rpmalloc_mmap(mapped_size, &align_offset);
  જો (!span) {
    errno = ENOMEM;
    પાછા 0;
  }
  ptr = pointer_offset(span, SPAN_HEADER_SIZE);

  જો ((uintptr_t)ptr & align_mask)
    ptr = (શૂન્ય *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);

  જો (((size_t)pointer_diff(ptr, span) >= _memory_span_size) ||
      (pointer_offset(ptr, size) > pointer_offset(span, mapped_size)) ||
      (((uintptr_t)ptr & _memory_span_mask) != (uintptr_t)span)) {
    _rpmalloc_unmap(span, mapped_size, align_offset, mapped_size);
    ++અંક_pages;
    જો (અંક_pages > limit_pages) {
      errno = EINVAL;
      પાછા 0;
    }
    જાઓ retry;
  }

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_ગણતરી = (uint32_t)અંક_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, અંક_pages, _huge_pages_peak);

#જો RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_ગણતરી;

  _rpmalloc_stat_add64(&_allocation_counter, 1);

  પાછા ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
સ્થિર શૂન્ય _rpmalloc_deallocate_direct_small_or_medium(span_t *span,
                                                        શૂન્ય *block) {
  heap_t *heap = span->heap;
  rpmalloc_assert(heap->owner_thread == get_thread_id() ||
                      !heap->owner_thread || heap->finalize,
                  "Internal failure");
  // Add block to free list
  જો (UNEXPECTED(_rpmalloc_span_is_fully_utilized(span))) {
    span->used_ગણતરી = span->block_ગણતરી;
#જો RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                           span);
#endif
    _rpmalloc_span_double_link_list_add(
        &heap->size_class[span->size_class].partial_span, span);
    --heap->full_span_ગણતરી;
  }
  *((શૂન્ય **)block) = span->free_list;
  --span->used_ગણતરી;
  span->free_list = block;
  જો (UNEXPECTED(span->used_ગણતરી == span->list_size)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    જો (span->used_ગણતરી) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      શૂન્ય *free_list;
      કરો {
        free_list = atomic_exchange_ptr_acquire(&span->free_list_deferred,
                                                INVALID_POINTER);
      } જ્યારે (free_list == INVALID_POINTER);
      atomic_store_ptr_release(&span->free_list_deferred, free_list);
    }
    _rpmalloc_span_double_link_list_remove(
        &heap->size_class[span->size_class].partial_span, span);
    _rpmalloc_span_release_to_cache(heap, span);
  }
}

સ્થિર શૂન્ય _rpmalloc_deallocate_defer_free_span(heap_t *heap, span_t *span) {
  જો (span->size_class != SIZE_CLASS_HUGE)
    _rpmalloc_stat_inc(&heap->span_use[span->span_ગણતરી - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  કરો {
    span->free_list = (શૂન્ય *)atomic_load_ptr(&heap->span_free_deferred);
  } જ્યારે (!atomic_cas_ptr(&heap->span_free_deferred, span, span->free_list));
}

//! Put the block in the deferred free list of the owning span
સ્થિર શૂન્ય _rpmalloc_deallocate_defer_small_or_medium(span_t *span,
                                                       શૂન્ય *block) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  શૂન્ય *free_list;
  કરો {
    free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } જ્યારે (free_list == INVALID_POINTER);
  *((શૂન્ય **)block) = free_list;
  uint32_t free_ગણતરી = ++span->list_size;
  પૂર્ણાંક all_deferred_free = (free_ગણતરી == span->block_ગણતરી);
  atomic_store_ptr_release(&span->free_list_deferred, block);
  જો (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
  }
}

સ્થિર શૂન્ય _rpmalloc_deallocate_small_or_medium(span_t *span, શૂન્ય *p) {
  _rpmalloc_stat_inc_free(span->heap, span->size_class);
  જો (span->flags & SPAN_FLAG_ALIGNED_BLOCKS) {
    // Realign pointer to block start
    શૂન્ય *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
    p = pointer_offset(p, -(int32_t)(block_offset % span->block_size));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#જો RPMALLOC_FIRST_CLASS_HEAPS
  પૂર્ણાંક defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#નહીંતર
  પૂર્ણાંક defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  જો (!defer)
    _rpmalloc_deallocate_direct_small_or_medium(span, p);
  નહીંતર
    _rpmalloc_deallocate_defer_small_or_medium(span, p);
}

//! Deallocate the given large memory block to the current heap
સ્થિર શૂન્ય _rpmalloc_deallocate_large(span_t *span) {
  rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#જો RPMALLOC_FIRST_CLASS_HEAPS
  પૂર્ણાંક defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#નહીંતર
  પૂર્ણાંક defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  જો (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    પાછા;
  }
  rpmalloc_assert(span->heap->full_span_ગણતરી, "Heap span counter corrupted");
  --span->heap->full_span_ગણતરી;
#જો RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif
#જો ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = span->span_ગણતરી - 1;
  atomic_decr32(&span->heap->span_use[idx].current);
#endif
  heap_t *heap = span->heap;
  rpmalloc_assert(heap, "No thread heap");
#જો ENABLE_THREAD_CACHE
  અચલ પૂર્ણાંક set_as_reserved =
      ((span->span_ગણતરી > 1) && (heap->span_cache.ગણતરી == 0) &&
       !heap->finalize && !heap->spans_reserved);
#નહીંતર
  અચલ પૂર્ણાંક set_as_reserved =
      ((span->span_ગણતરી > 1) && !heap->finalize && !heap->spans_reserved);
#endif
  જો (set_as_reserved) {
    heap->span_reserve = span;
    heap->spans_reserved = span->span_ગણતરી;
    જો (span->flags & SPAN_FLAG_MASTER) {
      heap->span_reserve_master = span;
    } નહીંતર { // SPAN_FLAG_SUBSPAN
      span_t *master = (span_t *)pointer_offset(
          span,
          -(intptr_t)((size_t)span->offset_from_master * _memory_span_size));
      heap->span_reserve_master = master;
      rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->remaining_spans) >=
                          (int32_t)span->span_ગણતરી,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&heap->span_use[idx].spans_to_reserved);
  } નહીંતર {
    // Insert into cache list
    rpmalloc_heap_cache_ઉમેરો(heap, span);
  }
}

//! Deallocate the given huge span
સ્થિર શૂન્ય _rpmalloc_deallocate_huge(span_t *span) {
  rpmalloc_assert(span->heap, "No span heap");
#જો RPMALLOC_FIRST_CLASS_HEAPS
  પૂર્ણાંક defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#નહીંતર
  પૂર્ણાંક defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  જો (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    પાછા;
  }
  rpmalloc_assert(span->heap->full_span_ગણતરી, "Heap span counter corrupted");
  --span->heap->full_span_ગણતરી;
#જો RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif

  // Oversized allocation, page count is stored in span_count
  size_t અંક_pages = span->span_ગણતરી;
  _rpmalloc_unmap(span, અંક_pages * _memory_page_size, span->align_offset,
                  અંક_pages * _memory_page_size);
  _rpmalloc_stat_sub(&_huge_pages_current, અંક_pages);
}

//! Deallocate the given block
સ્થિર શૂન્ય _rpmalloc_deallocate(શૂન્ય *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  જો (UNEXPECTED(!span))
    પાછા;
  જો (EXPECTED(span->size_class < SIZE_CLASS_ગણતરી))
    _rpmalloc_deallocate_small_or_medium(span, p);
  નહીંતર જો (span->size_class == SIZE_CLASS_LARGE)
    _rpmalloc_deallocate_large(span);
  નહીંતર
    _rpmalloc_deallocate_huge(span);
}

////////////
///
/// Reallocation entry points
///
//////

સ્થિર size_t _rpmalloc_usable_size(શૂન્ય *p);

//! Reallocate the given block to the given size
સ્થિર શૂન્ય *_rpmalloc_reallocate(heap_t *heap, શૂન્ય *p, size_t size,
                                  size_t oldsize, unsigned પૂર્ણાંક flags) {
  જો (p) {
    // Grab the span using guaranteed span alignment
    span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
    જો (EXPECTED(span->size_class < SIZE_CLASS_ગણતરી)) {
      // Small/medium sized block
      rpmalloc_assert(span->span_ગણતરી == 1, "Span counter corrupted");
      શૂન્ય *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
      uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
      uint32_t block_idx = block_offset / span->block_size;
      શૂન્ય *block =
          pointer_offset(blocks_start, (size_t)block_idx * span->block_size);
      જો (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)span->block_size - pointer_diff(p, block));
      જો ((size_t)span->block_size >= size) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        જો ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        પાછા block;
      }
    } નહીંતર જો (span->size_class == SIZE_CLASS_LARGE) {
      // Large block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t અંક_spans = total_size >> _memory_span_size_shift;
      જો (total_size & (_memory_span_mask - 1))
        ++અંક_spans;
      size_t current_spans = span->span_ગણતરી;
      શૂન્ય *block = pointer_offset(span, SPAN_HEADER_SIZE);
      જો (!oldsize)
        oldsize = (current_spans * _memory_span_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      જો ((current_spans >= અંક_spans) && (total_size >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        જો ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        પાછા block;
      }
    } નહીંતર {
      // Oversized block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t અંક_pages = total_size >> _memory_page_size_shift;
      જો (total_size & (_memory_page_size - 1))
        ++અંક_pages;
      // Page count is stored in span_count
      size_t current_pages = span->span_ગણતરી;
      શૂન્ય *block = pointer_offset(span, SPAN_HEADER_SIZE);
      જો (!oldsize)
        oldsize = (current_pages * _memory_page_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      જો ((current_pages >= અંક_pages) && (અંક_pages >= (current_pages / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        જો ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        પાછા block;
      }
    }
  } નહીંતર {
    oldsize = 0;
  }

  જો (!!(flags & RPMALLOC_GROW_OR_FAIL))
    પાછા 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t lower_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_size =
      (size > lower_bound) ? size : ((size > oldsize) ? lower_bound : size);
  શૂન્ય *block = _rpmalloc_allocate(heap, new_size);
  જો (p && block) {
    જો (!(flags & RPMALLOC_NO_PRESERVE))
      memcpy(block, p, oldsize < new_size ? oldsize : new_size);
    _rpmalloc_deallocate(p);
  }

  પાછા block;
}

સ્થિર શૂન્ય *_rpmalloc_aligned_reallocate(heap_t *heap, શૂન્ય *ptr,
                                          size_t alignment, size_t size,
                                          size_t oldsize, unsigned પૂર્ણાંક flags) {
  જો (alignment <= SMALL_GRANULARITY)
    પાછા _rpmalloc_reallocate(heap, ptr, size, oldsize, flags);

  પૂર્ણાંક no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? _rpmalloc_usable_size(ptr) : 0);
  જો ((usablesize >= size) && !((uintptr_t)ptr & (alignment - 1))) {
    જો (no_alloc || (size >= (usablesize / 2)))
      પાછા ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  શૂન્ય *block =
      (!no_alloc ? _rpmalloc_aligned_allocate(heap, alignment, size) : 0);
  જો (EXPECTED(block != 0)) {
    જો (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      જો (!oldsize)
        oldsize = usablesize;
      memcpy(block, ptr, oldsize < size ? oldsize : size);
    }
    _rpmalloc_deallocate(ptr);
  }
  પાછા block;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
સ્થિર size_t _rpmalloc_usable_size(શૂન્ય *p) {
  // Grab the span using guaranteed span alignment
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  જો (span->size_class < SIZE_CLASS_ગણતરી) {
    // Small/medium block
    શૂન્ય *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    પાછા span->block_size -
           ((size_t)pointer_diff(p, blocks_start) % span->block_size);
  }
  જો (span->size_class == SIZE_CLASS_LARGE) {
    // Large block
    size_t current_spans = span->span_ગણતરી;
    પાછા (current_spans * _memory_span_size) - (size_t)pointer_diff(p, span);
  }
  // Oversized block, page count is stored in span_count
  size_t current_pages = span->span_ગણતરી;
  પાછા (current_pages * _memory_page_size) - (size_t)pointer_diff(p, span);
}

//! Adjust and optimize the size class properties for the given class
સ્થિર શૂન્ય _rpmalloc_adjust_size_class(size_t iclass) {
  size_t block_size = _memory_size_class[iclass].block_size;
  size_t block_ગણતરી = (_memory_span_size - SPAN_HEADER_SIZE) / block_size;

  _memory_size_class[iclass].block_ગણતરી = (uint16_t)block_ગણતરી;
  _memory_size_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  જો (iclass >= SMALL_CLASS_ગણતરી) {
    size_t prevclass = iclass;
    જ્યારે (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      જો (_memory_size_class[prevclass].block_ગણતરી ==
          _memory_size_class[iclass].block_ગણતરી)
        _rpmalloc_memcpy_const(_memory_size_class + prevclass,
                               _memory_size_class + iclass,
                               કદ(_memory_size_class[iclass]));
      નહીંતર
        તોડો;
    }
  }
}

//! Initialize the allocator and setup global data
બાહ્ય અંતઃરેખ પૂર્ણાંક rpmalloc_initialize(શૂન્ય) {
  જો (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    પાછા 0;
  }
  પાછા rpmalloc_initialize_config(0);
}

પૂર્ણાંક rpmalloc_initialize_config(અચલ rpmalloc_config_t *config) {
  જો (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    પાછા 0;
  }
  _rpmalloc_initialized = 1;

  જો (config)
    memcpy(&_memory_config, config, કદ(rpmalloc_config_t));
  નહીંતર
    _rpmalloc_memset_const(&_memory_config, 0, કદ(rpmalloc_config_t));

  જો (!_memory_config.memory_map || !_memory_config.memory_unmap) {
    _memory_config.memory_map = _rpmalloc_mmap_os;
    _memory_config.memory_unmap = _rpmalloc_unmap_os;
  }

#જો PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, કદ(system_info));
  GetSystemInfo(&system_info);
  _memory_map_granularity = system_info.dwAllocationGranularity;
#નહીંતર
  _memory_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#endif

#જો RPMALLOC_CONFIGURABLE
  _memory_page_size = _memory_config.page_size;
#નહીંતર
  _memory_page_size = 0;
#endif
  _memory_huge_pages = 0;
  જો (!_memory_page_size) {
#જો PLATFORM_WINDOWS
    _memory_page_size = system_info.dwPageSize;
#નહીંતર
    _memory_page_size = _memory_map_granularity;
    જો (_memory_config.enable_huge_pages) {
#જો defined(__linux__)
      size_t huge_page_size = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      જો (meminfo) {
        અક્ષર લીટી_2[128];
        જ્યારે (!huge_page_size && fgets(લીટી_2, કદ(લીટી_2) - 1, meminfo)) {
          લીટી_2[કદ(લીટી_2) - 1] = 0;
          જો (strstr(લીટી_2, "Hugepagesize:"))
            huge_page_size = (size_t)strtol(લીટી_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      જો (huge_page_size) {
        _memory_huge_pages = 1;
        _memory_page_size = huge_page_size;
        _memory_map_granularity = huge_page_size;
      }
#અથવાજો defined(__FreeBSD__)
      પૂર્ણાંક rc;
      size_t sz = કદ(rc);

      જો (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, ખાલી, 0) == 0 &&
          rc == 1) {
        સ્થિર size_t defsize = 2 * 1024 * 1024;
        પૂર્ણાંક nsize = 0;
        size_t sizes[4] = {0};
        _memory_huge_pages = 1;
        _memory_page_size = defsize;
        જો ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          માટે (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            જો (defsize < csize) {
              _memory_page_size = csize;
              તોડો;
            }
          }
        }
        _memory_map_granularity = _memory_page_size;
      }
#અથવાજો defined(__APPLE__) || defined(__NetBSD__)
      _memory_huge_pages = 1;
      _memory_page_size = 2 * 1024 * 1024;
      _memory_map_granularity = _memory_page_size;
#endif
    }
#endif
  } નહીંતર {
    જો (_memory_config.enable_huge_pages)
      _memory_huge_pages = 1;
  }

#જો PLATFORM_WINDOWS
  જો (_memory_config.enable_huge_pages) {
    HANDLE token = 0;
    size_t large_page_minimum = GetLargePageMinimum();
    જો (large_page_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    જો (token) {
      LUID luid;
      જો (Lookup_Privilege_કિંમત(0, SE_LOCK_MEMORY_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, કદ(token_privileges));
        token_privileges.Privilege_ગણતરી = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        જો (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          જો (GetLastError() == ERROR_SUCCESS)
            _memory_huge_pages = 1;
        }
      }
      CloseHandle(token);
    }
    જો (_memory_huge_pages) {
      જો (large_page_minimum > _memory_page_size)
        _memory_page_size = large_page_minimum;
      જો (large_page_minimum > _memory_map_granularity)
        _memory_map_granularity = large_page_minimum;
    }
  }
#endif

  size_t min_span_size = 256;
  size_t max_page_size;
#જો UINTPTR_MAX > 0xFFFFFFFF
  max_page_size = 4096ULL * 1024ULL * 1024ULL;
#નહીંતર
  max_page_size = 4 * 1024 * 1024;
#endif
  જો (_memory_page_size < min_span_size)
    _memory_page_size = min_span_size;
  જો (_memory_page_size > max_page_size)
    _memory_page_size = max_page_size;
  _memory_page_size_shift = 0;
  size_t page_size_bit = _memory_page_size;
  જ્યારે (page_size_bit != 1) {
    ++_memory_page_size_shift;
    page_size_bit >>= 1;
  }
  _memory_page_size = ((size_t)1 << _memory_page_size_shift);

#જો RPMALLOC_CONFIGURABLE
  જો (!_memory_config.span_size) {
    _memory_span_size = _memory_default_span_size;
    _memory_span_size_shift = _memory_default_span_size_shift;
    _memory_span_mask = _memory_default_span_mask;
  } નહીંતર {
    size_t span_size = _memory_config.span_size;
    જો (span_size > (256 * 1024))
      span_size = (256 * 1024);
    _memory_span_size = 4096;
    _memory_span_size_shift = 12;
    જ્યારે (_memory_span_size < span_size) {
      _memory_span_size <<= 1;
      ++_memory_span_size_shift;
    }
    _memory_span_mask = ~(uintptr_t)(_memory_span_size - 1);
  }
#endif

  memory_span_map_ગણતરી =
      (_memory_config.span_map_ગણતરી ? _memory_config.span_map_ગણતરી
                                     : DEFAULT_SPAN_MAP_ગણતરી);
  જો ((_memory_span_size * memory_span_map_ગણતરી) < _memory_page_size)
    memory_span_map_ગણતરી = (_memory_page_size / _memory_span_size);
  જો ((_memory_page_size >= _memory_span_size) &&
      ((memory_span_map_ગણતરી * _memory_span_size) % _memory_page_size))
    memory_span_map_ગણતરી = (_memory_page_size / _memory_span_size);
  memory_heap_reserve_ગણતરી = (memory_span_map_ગણતરી > DEFAULT_SPAN_MAP_ગણતરી)
                                   ? DEFAULT_SPAN_MAP_ગણતરી
                                   : memory_span_map_ગણતરી;

  _memory_config.page_size = _memory_page_size;
  _memory_config.span_size = _memory_span_size;
  _memory_config.span_map_ગણતરી = memory_span_map_ગણતરી;
  _memory_config.enable_huge_pages = _memory_huge_pages;

#જો ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  જો (pthread_ચાવી_create(&_memory_thread_heap, _rpmalloc_heap_release_raw_fc))
    પાછા -1;
#endif
#જો defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_ચાવી = FlsAlloc(&_rpmalloc_thread_destructor);
#endif

  // Setup all small and medium size classes
  size_t iclass = 0;
  _memory_size_class[iclass].block_size = SMALL_GRANULARITY;
  _rpmalloc_adjust_size_class(iclass);
  માટે (iclass = 1; iclass < SMALL_CLASS_ગણતરી; ++iclass) {
    size_t size = iclass * SMALL_GRANULARITY;
    _memory_size_class[iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  _memory_medium_size_limit = (_memory_span_size - SPAN_HEADER_SIZE) >> 1;
  જો (_memory_medium_size_limit > MEDIUM_SIZE_LIMIT)
    _memory_medium_size_limit = MEDIUM_SIZE_LIMIT;
  માટે (iclass = 0; iclass < MEDIUM_CLASS_ગણતરી; ++iclass) {
    size_t size = SMALL_SIZE_LIMIT + ((iclass + 1) * MEDIUM_GRANULARITY);
    જો (size > _memory_medium_size_limit) {
      _memory_medium_size_limit =
          SMALL_SIZE_LIMIT + (iclass * MEDIUM_GRANULARITY);
      તોડો;
    }
    _memory_size_class[SMALL_CLASS_ગણતરી + iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(SMALL_CLASS_ગણતરી + iclass);
  }

  _memory_orphan_heaps = 0;
#જો RPMALLOC_FIRST_CLASS_HEAPS
  _memory_first_class_orphan_heaps = 0;
#endif
#જો ENABLE_STATISTICS
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
  memset(_memory_heaps, 0, કદ(_memory_heaps));
  atomic_store32_release(&_memory_global_lock, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  પાછા 0;
}

//! Finalize the allocator
શૂન્ય rpmalloc_finalize(શૂન્ય) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  જો (_memory_global_reserve) {
    atomic_add32(&_memory_global_reserve_master->remaining_spans,
                 -(int32_t)memory_global_reserve_ગણતરી);
    _memory_global_reserve_master = 0;
    memory_global_reserve_ગણતરી = 0;
    _memory_global_reserve = 0;
  }
  atomic_store32_release(&_memory_global_lock, 0);

  // Free all thread caches and fully free spans
  માટે (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    જ્યારે (heap) {
      heap_t *next_heap = heap->next_heap;
      heap->finalize = 1;
      _rpmalloc_heap_global_finalize(heap);
      heap = next_heap;
    }
  }

#જો ENABLE_GLOBAL_CACHE
  // Free global caches
  માટે (size_t iclass = 0; iclass < LARGE_CLASS_ગણતરી; ++iclass)
    _rpmalloc_global_cache_finalize(&_memory_span_cache[iclass]);
#endif

#જો (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_ચાવી_delete(_memory_thread_heap);
#endif
#જો defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_ચાવી);
  fls_ચાવી = 0;
#endif
#જો ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&_mapped_pages) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&_mapped_pages_os) == 0,
                  "Memory leak detected");
#endif

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
બાહ્ય અંતઃરેખ શૂન્ય rpmalloc_thread_initialize(શૂન્ય) {
  જો (!get_thread_heap_raw()) {
    heap_t *heap = _rpmalloc_heap_allocate(0);
    જો (heap) {
      _rpmalloc_stat_inc(&_memory_active_heaps);
      set_thread_heap(heap);
#જો defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_Set_કિંમત(fls_ચાવી, heap);
#endif
    }
  }
}

//! Finalize thread, orphan heap
શૂન્ય rpmalloc_thread_finalize(પૂર્ણાંક release_caches) {
  heap_t *heap = get_thread_heap_raw();
  જો (heap)
    _rpmalloc_heap_release_raw(heap, release_caches);
  set_thread_heap(0);
#જો defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_Set_કિંમત(fls_ચાવી, 0);
#endif
}

પૂર્ણાંક rpmalloc_is_thread_initialized(શૂન્ય) {
  પાછા (get_thread_heap_raw() != 0) ? 1 : 0;
}

અચલ rpmalloc_config_t *rpmalloc_config(શૂન્ય) { પાછા &_memory_config; }

// Extern interface

બાહ્ય અંતઃરેખ RPMALLOC_ALLOCATOR શૂન્ય *rpmalloc(size_t size) {
#જો ENABLE_VALIDATE_ARGS
  જો (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    પાછા 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  પાછા _rpmalloc_allocate(heap, size);
}

બાહ્ય અંતઃરેખ શૂન્ય rpfree(શૂન્ય *ptr) { _rpmalloc_deallocate(ptr); }

બાહ્ય અંતઃરેખ RPMALLOC_ALLOCATOR શૂન્ય *rpcalloc(size_t અંક_2, size_t size) {
  size_t total;
#જો ENABLE_VALIDATE_ARGS
#જો PLATFORM_WINDOWS
  પૂર્ણાંક err = SizeTMult(અંક_2, size, &total);
  જો ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    પાછા 0;
  }
#નહીંતર
  પૂર્ણાંક err = __builtin_umull_overflow(અંક_2, size, &total);
  જો (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    પાછા 0;
  }
#endif
#નહીંતર
  total = અંક_2 * size;
#endif
  heap_t *heap = get_thread_heap();
  શૂન્ય *block = _rpmalloc_allocate(heap, total);
  જો (block)
    memset(block, 0, total);
  પાછા block;
}

બાહ્ય અંતઃરેખ RPMALLOC_ALLOCATOR શૂન્ય *rprealloc(શૂન્ય *ptr, size_t size) {
#જો ENABLE_VALIDATE_ARGS
  જો (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    પાછા ptr;
  }
#endif
  heap_t *heap = get_thread_heap();
  પાછા _rpmalloc_reallocate(heap, ptr, size, 0, 0);
}

બાહ્ય RPMALLOC_ALLOCATOR શૂન્ય *rpaligned_realloc(શૂન્ય *ptr, size_t alignment,
                                                  size_t size, size_t oldsize,
                                                  unsigned પૂર્ણાંક flags) {
#જો ENABLE_VALIDATE_ARGS
  જો ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    પાછા 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  પાછા _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, oldsize,
                                      flags);
}

બાહ્ય RPMALLOC_ALLOCATOR શૂન્ય *rpaligned_alloc(size_t alignment, size_t size) {
  heap_t *heap = get_thread_heap();
  પાછા _rpmalloc_aligned_allocate(heap, alignment, size);
}

બાહ્ય અંતઃરેખ RPMALLOC_ALLOCATOR શૂન્ય *
rpaligned_calloc(size_t alignment, size_t અંક_2, size_t size) {
  size_t total;
#જો ENABLE_VALIDATE_ARGS
#જો PLATFORM_WINDOWS
  પૂર્ણાંક err = SizeTMult(અંક_2, size, &total);
  જો ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    પાછા 0;
  }
#નહીંતર
  પૂર્ણાંક err = __builtin_umull_overflow(અંક_2, size, &total);
  જો (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    પાછા 0;
  }
#endif
#નહીંતર
  total = અંક_2 * size;
#endif
  શૂન્ય *block = rpaligned_alloc(alignment, total);
  જો (block)
    memset(block, 0, total);
  પાછા block;
}

બાહ્ય અંતઃરેખ RPMALLOC_ALLOCATOR શૂન્ય *rpmemalign(size_t alignment,
                                                  size_t size) {
  પાછા rpaligned_alloc(alignment, size);
}

બાહ્ય અંતઃરેખ પૂર્ણાંક rpposix_memalign(શૂન્ય **memptr, size_t alignment,
                                   size_t size) {
  જો (memptr)
    *memptr = rpaligned_alloc(alignment, size);
  નહીંતર
    પાછા EINVAL;
  પાછા *memptr ? 0 : ENOMEM;
}

બાહ્ય અંતઃરેખ size_t rpmalloc_usable_size(શૂન્ય *ptr) {
  પાછા (ptr ? _rpmalloc_usable_size(ptr) : 0);
}

બાહ્ય અંતઃરેખ શૂન્ય rpmalloc_thread_collect(શૂન્ય) {}

શૂન્ય rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, કદ(rpmalloc_thread_statistics_t));
  heap_t *heap = get_thread_heap_raw();
  જો (!heap)
    પાછા;

  માટે (size_t iclass = 0; iclass < SIZE_CLASS_ગણતરી; ++iclass) {
    size_class_t *size_class = _memory_size_class + iclass;
    span_t *span = heap->size_class[iclass].partial_span;
    જ્યારે (span) {
      size_t free_ગણતરી = span->list_size;
      size_t block_ગણતરી = size_class->block_ગણતરી;
      જો (span->free_list_limit < block_ગણતરી)
        block_ગણતરી = span->free_list_limit;
      free_ગણતરી += (block_ગણતરી - span->used_ગણતરી);
      stats->sizecache += free_ગણતરી * size_class->block_size;
      span = span->next;
    }
  }

#જો ENABLE_THREAD_CACHE
  માટે (size_t iclass = 0; iclass < LARGE_CLASS_ગણતરી; ++iclass) {
    span_cache_t *span_cache;
    જો (!iclass)
      span_cache = &heap->span_cache;
    નહીંતર
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    stats->spancache += span_cache->ગણતરી * (iclass + 1) * _memory_span_size;
  }
#endif

  span_t *deferred = (span_t *)atomic_load_ptr(&heap->span_free_deferred);
  જ્યારે (deferred) {
    જો (deferred->size_class != SIZE_CLASS_HUGE)
      stats->spancache += (size_t)deferred->span_ગણતરી * _memory_span_size;
    deferred = (span_t *)deferred->free_list;
  }

#જો ENABLE_STATISTICS
  stats->thread_to_global = (size_t)atomic_load64(&heap->thread_to_global);
  stats->global_to_thread = (size_t)atomic_load64(&heap->global_to_thread);

  માટે (size_t iclass = 0; iclass < LARGE_CLASS_ગણતરી; ++iclass) {
    stats->span_use[iclass].current =
        (size_t)atomic_load32(&heap->span_use[iclass].current);
    stats->span_use[iclass].peak =
        (size_t)atomic_load32(&heap->span_use[iclass].ઉપલું);
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
  માટે (size_t iclass = 0; iclass < SIZE_CLASS_ગણતરી; ++iclass) {
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

શૂન્ય rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, કદ(rpmalloc_global_statistics_t));
#જો ENABLE_STATISTICS
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
#જો ENABLE_GLOBAL_CACHE
  માટે (size_t iclass = 0; iclass < LARGE_CLASS_ગણતરી; ++iclass) {
    global_cache_t *cache = &_memory_span_cache[iclass];
    જ્યારે (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();
    uint32_t ગણતરી = cache->ગણતરી;
#જો ENABLE_UNLIMITED_CACHE
    span_t *current_span = cache->overflow;
    જ્યારે (current_span) {
      ++ગણતરી;
      current_span = current_span->next;
    }
#endif
    atomic_store32_release(&cache->lock, 0);
    stats->cached += ગણતરી * (iclass + 1) * _memory_span_size;
  }
#endif
}

#જો ENABLE_STATISTICS

સ્થિર શૂન્ય _memory_heap_dump_statistics(heap_t *heap, શૂન્ય *file) {
  fprintf(file, "Heap %d stats:\n", heap->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  માટે (size_t iclass = 0; iclass < SIZE_CLASS_ગણતરી; ++iclass) {
    જો (!atomic_load32(&heap->size_class_use[iclass].alloc_total))
      ચાલુ;
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
        _memory_size_class[iclass].block_ગણતરી,
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
  માટે (size_t iclass = 0; iclass < LARGE_CLASS_ગણતરી; ++iclass) {
    જો (!atomic_load32(&heap->span_use[iclass].ઉપલું) &&
        !atomic_load32(&heap->span_use[iclass].spans_map_calls))
      ચાલુ;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&heap->span_use[iclass].current),
        atomic_load32(&heap->span_use[iclass].ઉપલું),
        atomic_load32(&heap->span_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&heap->span_use[iclass].ઉપલું) *
         (size_t)_memory_span_size * (iclass + 1)) /
            (size_t)(1024 * 1024),
#જો ENABLE_THREAD_CACHE
        (unsigned પૂર્ણાંક)(!iclass ? heap->span_cache.ગણતરી
                               : heap->span_large_cache[iclass - 1].ગણતરી),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_to_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_from_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
#નહીંતર
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
  fprintf(file, "Full spans: %zu\n", heap->full_span_ગણતરી);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&heap->thread_to_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&heap->global_to_thread) / (size_t)(1024 * 1024));
}

#endif

શૂન્ય rpmalloc_dump_statistics(શૂન્ય *file) {
#જો ENABLE_STATISTICS
  માટે (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    જ્યારે (heap) {
      પૂર્ણાંક need_dump = 0;
      માટે (size_t iclass = 0; !need_dump && (iclass < SIZE_CLASS_ગણતરી);
           ++iclass) {
        જો (!atomic_load32(&heap->size_class_use[iclass].alloc_total)) {
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].free_total),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          ચાલુ;
        }
        need_dump = 1;
      }
      માટે (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_ગણતરી);
           ++iclass) {
        જો (!atomic_load32(&heap->span_use[iclass].ઉપલું) &&
            !atomic_load32(&heap->span_use[iclass].spans_map_calls))
          ચાલુ;
        need_dump = 1;
      }
      જો (need_dump)
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

#જો ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  માટે (size_t iclass = 0; iclass < LARGE_CLASS_ગણતરી; ++iclass) {
    global_cache_t *cache = _memory_span_cache + iclass;
    size_t global_cache = (size_t)cache->ગણતરી * iclass * _memory_span_size;

    size_t global_overflow_cache = 0;
    span_t *span = cache->overflow;
    જ્યારે (span) {
      global_overflow_cache += iclass * _memory_span_size;
      span = span->next;
    }
    જો (global_cache || global_overflow_cache || cache->ઉમેરો_ગણતરી ||
        cache->extract_ગણતરી)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_overflow_cache / (size_t)(1024 * 1024),
              cache->ઉમેરો_ગણતરી, cache->extract_ગણતરી);
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
#જો 0
	int64_t allocated = atomic_load64(&_allocation_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#endif
#endif
  (શૂન્ય)કદ(file);
}

#જો RPMALLOC_FIRST_CLASS_HEAPS

બાહ્ય અંતઃરેખ rpmalloc_heap_t *rpmalloc_heap_acquire(શૂન્ય) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  heap_t *heap = _rpmalloc_heap_allocate(1);
  rpmalloc_assume(heap != ખાલી);
  heap->owner_thread = 0;
  _rpmalloc_stat_inc(&_memory_active_heaps);
  પાછા heap;
}

બાહ્ય અંતઃરેખ શૂન્ય rpmalloc_heap_release(rpmalloc_heap_t *heap) {
  જો (heap)
    _rpmalloc_heap_release(heap, 1, 1);
}

બાહ્ય અંતઃરેખ RPMALLOC_ALLOCATOR શૂન્ય *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, size_t size) {
#જો ENABLE_VALIDATE_ARGS
  જો (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    પાછા 0;
  }
#endif
  પાછા _rpmalloc_allocate(heap, size);
}

બાહ્ય અંતઃરેખ RPMALLOC_ALLOCATOR શૂન્ય *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, size_t alignment,
                            size_t size) {
#જો ENABLE_VALIDATE_ARGS
  જો (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    પાછા 0;
  }
#endif
  પાછા _rpmalloc_aligned_allocate(heap, alignment, size);
}

બાહ્ય અંતઃરેખ RPMALLOC_ALLOCATOR શૂન્ય *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, size_t અંક_2, size_t size) {
  પાછા rpmalloc_heap_aligned_calloc(heap, 0, અંક_2, size);
}

બાહ્ય અંતઃરેખ RPMALLOC_ALLOCATOR શૂન્ય *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, size_t alignment,
                             size_t અંક_2, size_t size) {
  size_t total;
#જો ENABLE_VALIDATE_ARGS
#જો PLATFORM_WINDOWS
  પૂર્ણાંક err = SizeTMult(અંક_2, size, &total);
  જો ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    પાછા 0;
  }
#નહીંતર
  પૂર્ણાંક err = __builtin_umull_overflow(અંક_2, size, &total);
  જો (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    પાછા 0;
  }
#endif
#નહીંતર
  total = અંક_2 * size;
#endif
  શૂન્ય *block = _rpmalloc_aligned_allocate(heap, alignment, total);
  જો (block)
    memset(block, 0, total);
  પાછા block;
}

બાહ્ય અંતઃરેખ RPMALLOC_ALLOCATOR શૂન્ય *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, શૂન્ય *ptr, size_t size,
                      unsigned પૂર્ણાંક flags) {
#જો ENABLE_VALIDATE_ARGS
  જો (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    પાછા ptr;
  }
#endif
  પાછા _rpmalloc_reallocate(heap, ptr, size, 0, flags);
}

બાહ્ય અંતઃરેખ RPMALLOC_ALLOCATOR શૂન્ય *
rpmalloc_heap_aligned_realloc(rpmalloc_heap_t *heap, શૂન્ય *ptr,
                              size_t alignment, size_t size,
                              unsigned પૂર્ણાંક flags) {
#જો ENABLE_VALIDATE_ARGS
  જો ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    પાછા 0;
  }
#endif
  પાછા _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, 0, flags);
}

બાહ્ય અંતઃરેખ શૂન્ય rpmalloc_heap_free(rpmalloc_heap_t *heap, શૂન્ય *ptr) {
  (શૂન્ય)કદ(heap);
  _rpmalloc_deallocate(ptr);
}

બાહ્ય અંતઃરેખ શૂન્ય rpmalloc_heap_free_all(rpmalloc_heap_t *heap) {
  span_t *span;
  span_t *next_span;

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  માટે (size_t iclass = 0; iclass < SIZE_CLASS_ગણતરી; ++iclass) {
    span = heap->size_class[iclass].partial_span;
    જ્યારે (span) {
      next_span = span->next;
      rpmalloc_heap_cache_ઉમેરો(heap, span);
      span = next_span;
    }
    heap->size_class[iclass].partial_span = 0;
    span = heap->full_span[iclass];
    જ્યારે (span) {
      next_span = span->next;
      rpmalloc_heap_cache_ઉમેરો(heap, span);
      span = next_span;
    }

    span = heap->size_class[iclass].cache;
    જો (span)
      rpmalloc_heap_cache_ઉમેરો(heap, span);
    heap->size_class[iclass].cache = 0;
  }
  memset(heap->size_class, 0, કદ(heap->size_class));
  memset(heap->full_span, 0, કદ(heap->full_span));

  span = heap->large_huge_span;
  જ્યારે (span) {
    next_span = span->next;
    જો (UNEXPECTED(span->size_class == SIZE_CLASS_HUGE))
      _rpmalloc_deallocate_huge(span);
    નહીંતર
      rpmalloc_heap_cache_ઉમેરો(heap, span);
    span = next_span;
  }
  heap->large_huge_span = 0;
  heap->full_span_ગણતરી = 0;

#જો ENABLE_THREAD_CACHE
  માટે (size_t iclass = 0; iclass < LARGE_CLASS_ગણતરી; ++iclass) {
    span_cache_t *span_cache;
    જો (!iclass)
      span_cache = &heap->span_cache;
    નહીંતર
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    જો (!span_cache->ગણતરી)
      ચાલુ;
#જો ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&heap->thread_to_global,
                         span_cache->ગણતરી * (iclass + 1) * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                       span_cache->ગણતરી);
    rpmalloc_global_cache_ઉમેરો_spans(span_cache->span, iclass + 1,
                                        span_cache->ગણતરી);
#નહીંતર
    માટે (size_t ispan = 0; ispan < span_cache->ગણતરી; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
    span_cache->ગણતરી = 0;
  }
#endif

#જો ENABLE_STATISTICS
  માટે (size_t iclass = 0; iclass < SIZE_CLASS_ગણતરી; ++iclass) {
    atomic_store32(&heap->size_class_use[iclass].alloc_current, 0);
    atomic_store32(&heap->size_class_use[iclass].spans_current, 0);
  }
  માટે (size_t iclass = 0; iclass < LARGE_CLASS_ગણતરી; ++iclass) {
    atomic_store32(&heap->span_use[iclass].current, 0);
  }
#endif
}

બાહ્ય અંતઃરેખ શૂન્ય rpmalloc_heap_thread_set_current(rpmalloc_heap_t *heap) {
  heap_t *prev_heap = get_thread_heap_raw();
  જો (prev_heap != heap) {
    set_thread_heap(heap);
    જો (prev_heap)
      rpmalloc_heap_release(prev_heap);
  }
}

બાહ્ય અંતઃરેખ rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(શૂન્ય *ptr) {
  // Grab the span, and then the heap from the span
  span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
  જો (span) {
    પાછા span->heap;
  }
  પાછા 0;
}

#endif

#જો ENABLE_PRELOAD || ENABLE_OVERRIDE

#અન્તર્ભાવ "malloc.c"

#endif

શૂન્ય rpmalloc_linker_reference(શૂન્ય) { (શૂન્ય)કદ(_rpmalloc_initialized); }
