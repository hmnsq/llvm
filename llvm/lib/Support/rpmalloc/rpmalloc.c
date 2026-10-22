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

#অন্তর্ভাব "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#যদি defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wunused-function"
#যদি __has_warning("-Wreserved-identifier")
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
#যদি __has_warning("-Wstatic-in-inline")
#pragma clang diagnostic ignored "-Wstatic-in-inline"
#endif
#নোচেদ্যদি defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#যদি !defined(__has_builtin)
#define __has_builtin(b) 0
#endif

#যদি defined(__GNUC__) || defined(__clang__)

#যদি __has_builtin(__builtin_memcpy_inline)
#define _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#অন্যথা
#define _rpmalloc_memcpy_const(x, y, s)                                        \
  কুরু {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memcpy(x, y, s);                                                           \
  } যাবত্ (0)
#endif

#যদি __has_builtin(__builtin_memset_inline)
#define _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#অন্যথা
#define _rpmalloc_memset_const(x, y, s)                                        \
  কুরু {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } যাবত্ (0)
#endif
#অন্যথা
#define _rpmalloc_memcpy_const(x, y, s) memcpy(x, y, s)
#define _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#endif

#যদি __has_builtin(__builtin_assume)
#define rpmalloc_assume(cond) __builtin_assume(cond)
#নোচেদ্যদি defined(__GNUC__)
#define rpmalloc_assume(cond)                                                  \
  কুরু {                                                                         \
    যদি (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } যাবত্ (0)
#নোচেদ্যদি defined(_MSC_VER)
#define rpmalloc_assume(cond) __assume(cond)
#অন্যথা
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
#ifndef DEFAULT_SPAN_MAP_গুনতি
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#define DEFAULT_SPAN_MAP_গুনতি 64
#endif
#ifndef GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#define GLOBAL_CACHE_MULTIPLIER 8
#endif

#যদি DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#error Must use global cache যদি unmap is disabled
#endif

#যদি DISABLE_UNMAP
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 1
#endif

#যদি !ENABLE_GLOBAL_CACHE
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 0
#endif

#যদি !ENABLE_THREAD_CACHE
#undef ENABLE_ADAPTIVE_THREAD_CACHE
#define ENABLE_ADAPTIVE_THREAD_CACHE 0
#endif

#যদি defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#define PLATFORM_POSIX 0
#অন্যথা
#define PLATFORM_WINDOWS 0
#define PLATFORM_POSIX 1
#endif

/// Platform and arch specifics
#যদি defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 5105)
#ifndef FORCEINLINE
#define FORCEINLINE অন্তঃস্থ __forceinline
#endif
#define _Static_assert static_assert
#অন্যথা
#ifndef FORCEINLINE
#define FORCEINLINE অন্তঃস্থ __attribute__((__always_inline__))
#endif
#endif
#যদি PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#অন্তর্ভাব <windows.h>
#যদি ENABLE_VALIDATE_ARGS
#অন্তর্ভাব <intsafe.h>
#endif
#অন্যথা
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <unistd.h>
#যদি defined(__linux__) || defined(__ANDROID__)
#অন্তর্ভাব <sys/prctl.h>
#যদি !defined(PR_SET_VMA)
#define PR_SET_VMA 0x53564d41
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif
#যদি defined(__APPLE__)
#অন্তর্ভাব <TargetConditionals.h>
#যদি !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#অন্তর্ভাব <mach/mach_vm.h>
#অন্তর্ভাব <mach/vm_statistics.h>
#endif
#অন্তর্ভাব <pthread.h>
#endif
#যদি defined(__HAIKU__) || defined(__TINYC__)
#অন্তর্ভাব <pthread.h>
#endif
#endif

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <string.h>

#যদি defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#অন্তর্ভাব <fibersapi.h>
স্থির DWORD fls_চাবি;
#endif

#যদি PLATFORM_POSIX
#অন্তর্ভাব <sched.h>
#অন্তর্ভাব <sys/mman.h>
#ifdef __FreeBSD__
#অন্তর্ভাব <sys/sysctl.h>
#define MAP_HUGETLB MAP_ALIGNED_SUPER
#ifndef PROT_MAX
#define PROT_MAX(f) 0
#endif
#অন্যথা
#define PROT_MAX(f) 0
#endif
#ifdef __sun
বাহ্য পূর্ণাঙ্ক madvise(caddr_t, size_t, পূর্ণাঙ্ক);
#endif
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif
#endif
#অন্তর্ভাব <errno.h>

#যদি ENABLE_ASSERTS
#undef NDEBUG
#যদি defined(_MSC_VER) && !defined(_DEBUG)
#define _DEBUG
#endif
#অন্তর্ভাব <assert.h>
#define RPMALLOC_TOSTRING_M(x) #x
#define RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#define rpmalloc_assert(truth, message)                                        \
  কুরু {                                                                         \
    যদি (!(truth)) {                                                            \
      যদি (_memory_config.error_callback) {                                     \
        _memory_config.error_callback(message " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } অন্যথা {                                                                 \
        প্রতিজ্ঞা((truth) && message);                                            \
      }                                                                        \
    }                                                                          \
  } যাবত্ (0)
#অন্যথা
#define rpmalloc_assert(truth, message)                                        \
  কুরু {                                                                         \
  } যাবত্ (0)
#endif
#যদি ENABLE_STATISTICS
#অন্তর্ভাব <stdio.h>
#endif

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#যদি defined(_MSC_VER) && !defined(__clang__)

প্রকারনির্দেশ চঞ্চল long atomic32_t;
প্রকারনির্দেশ চঞ্চল long long atomic64_t;
প্রকারনির্দেশ চঞ্চল শূন্য *atomicptr_t;

স্থির FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  প্রত্যাগম (int32_t)InterlockedOr(src, 0);
}
স্থির FORCEINLINE শূন্য atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
স্থির FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  প্রত্যাগম (int32_t)InterlockedIncrement(val);
}
স্থির FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  প্রত্যাগম (int32_t)InterlockedDecrement(val);
}
স্থির FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  প্রত্যাগম (int32_t)InterlockedExchangeAdd(val, add) + add;
}
স্থির FORCEINLINE পূর্ণাঙ্ক atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  প্রত্যাগম (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
স্থির FORCEINLINE শূন্য atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
স্থির FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  প্রত্যাগম (int64_t)InterlockedOr64(src, 0);
}
স্থির FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  প্রত্যাগম (int64_t)InterlockedExchangeAdd64(val, add) + add;
}
স্থির FORCEINLINE শূন্য *atomic_load_ptr(atomicptr_t *src) {
  প্রত্যাগম InterlockedCompareExchangePointer(src, 0, 0);
}
স্থির FORCEINLINE শূন্য atomic_store_ptr(atomicptr_t *dst, শূন্য *val) {
  InterlockedExchangePointer(dst, val);
}
স্থির FORCEINLINE শূন্য atomic_store_ptr_release(atomicptr_t *dst, শূন্য *val) {
  InterlockedExchangePointer(dst, val);
}
স্থির FORCEINLINE শূন্য *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     শূন্য *val) {
  প্রত্যাগম (শূন্য *)InterlockedExchangePointer((শূন্য *চঞ্চল *)dst, val);
}
স্থির FORCEINLINE পূর্ণাঙ্ক atomic_cas_ptr(atomicptr_t *dst, শূন্য *val, শূন্য *ref) {
  প্রত্যাগম (InterlockedCompareExchangePointer((শূন্য *চঞ্চল *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#define EXPECTED(x) (x)
#define UNEXPECTED(x) (x)

#অন্যথা

#অন্তর্ভাব <stdatomic.h>

প্রকারনির্দেশ চঞ্চল _Atomic(int32_t) atomic32_t;
প্রকারনির্দেশ চঞ্চল _Atomic(int64_t) atomic64_t;
প্রকারনির্দেশ চঞ্চল _Atomic(শূন্য *) atomicptr_t;

স্থির FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  প্রত্যাগম atomic_load_explicit(src, memory_order_relaxed);
}
স্থির FORCEINLINE শূন্য atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
স্থির FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  প্রত্যাগম atomic_fetch_add_explicit(val, 1, memory_order_relaxed) + 1;
}
স্থির FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  প্রত্যাগম atomic_fetch_add_explicit(val, -1, memory_order_relaxed) - 1;
}
স্থির FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  প্রত্যাগম atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
স্থির FORCEINLINE পূর্ণাঙ্ক atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  প্রত্যাগম atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
স্থির FORCEINLINE শূন্য atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
স্থির FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  প্রত্যাগম atomic_load_explicit(val, memory_order_relaxed);
}
স্থির FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  প্রত্যাগম atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
স্থির FORCEINLINE শূন্য *atomic_load_ptr(atomicptr_t *src) {
  প্রত্যাগম atomic_load_explicit(src, memory_order_relaxed);
}
স্থির FORCEINLINE শূন্য atomic_store_ptr(atomicptr_t *dst, শূন্য *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
স্থির FORCEINLINE শূন্য atomic_store_ptr_release(atomicptr_t *dst, শূন্য *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
স্থির FORCEINLINE শূন্য *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     শূন্য *val) {
  প্রত্যাগম atomic_exchange_explicit(dst, val, memory_order_acquire);
}
স্থির FORCEINLINE পূর্ণাঙ্ক atomic_cas_ptr(atomicptr_t *dst, শূন্য *val, শূন্য *ref) {
  প্রত্যাগম atomic_compare_exchange_weak_explicit(
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

#যদি ENABLE_STATISTICS
#define _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#define _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#define _rpmalloc_stat_add(counter, মান_2)                                     \
  atomic_add32(counter, (int32_t)(মান_2))
#define _rpmalloc_stat_add64(counter, মান_2)                                   \
  atomic_add64(counter, (int64_t)(মান_2))
#define _rpmalloc_stat_add_peak(counter, মান_2, peak)                          \
  কুরু {                                                                         \
    int32_t cur_গুনতি = atomic_add32(counter, (int32_t)(মান_2));              \
    যদি (cur_গুনতি > (peak))                                                   \
      peak = cur_গুনতি;                                                       \
  } যাবত্ (0)
#define _rpmalloc_stat_sub(counter, মান_2)                                     \
  atomic_add32(counter, -(int32_t)(মান_2))
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  কুরু {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&heap->size_class_use[class_idx].alloc_current);         \
    যদি (alloc_current > heap->size_class_use[class_idx].alloc_peak)            \
      heap->size_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&heap->size_class_use[class_idx].alloc_total);               \
  } যাবত্ (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  কুরু {                                                                         \
    atomic_decr32(&heap->size_class_use[class_idx].alloc_current);             \
    atomic_incr32(&heap->size_class_use[class_idx].free_total);                \
  } যাবত্ (0)
#অন্যথা
#define _rpmalloc_stat_inc(counter)                                            \
  কুরু {                                                                         \
  } যাবত্ (0)
#define _rpmalloc_stat_dec(counter)                                            \
  কুরু {                                                                         \
  } যাবত্ (0)
#define _rpmalloc_stat_add(counter, মান_2)                                     \
  কুরু {                                                                         \
  } যাবত্ (0)
#define _rpmalloc_stat_add64(counter, মান_2)                                   \
  কুরু {                                                                         \
  } যাবত্ (0)
#define _rpmalloc_stat_add_peak(counter, মান_2, peak)                          \
  কুরু {                                                                         \
  } যাবত্ (0)
#define _rpmalloc_stat_sub(counter, মান_2)                                     \
  কুরু {                                                                         \
  } যাবত্ (0)
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  কুরু {                                                                         \
  } যাবত্ (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  কুরু {                                                                         \
  } যাবত্ (0)
#endif

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#define SMALL_GRANULARITY 16
//! Small granularity shift count
#define SMALL_GRANULARITY_SHIFT 4
//! Number of small block size classes
#define SMALL_CLASS_গুনতি 65
//! Maximum size of a small block
#define SMALL_SIZE_LIMIT (SMALL_GRANULARITY * (SMALL_CLASS_গুনতি - 1))
//! Granularity of a medium allocation block
#define MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#define MEDIUM_GRANULARITY_SHIFT 9
//! Number of medium block size classes
#define MEDIUM_CLASS_গুনতি 61
//! Total number of small + medium size classes
#define SIZE_CLASS_গুনতি (SMALL_CLASS_গুনতি + MEDIUM_CLASS_গুনতি)
//! Number of large block size classes
#define LARGE_CLASS_গুনতি 63
//! Maximum size of a medium block
#define MEDIUM_SIZE_LIMIT                                                      \
  (SMALL_SIZE_LIMIT + (MEDIUM_GRANULARITY * MEDIUM_CLASS_গুনতি))
//! Maximum size of a large block
#define LARGE_SIZE_LIMIT                                                       \
  ((LARGE_CLASS_গুনতি * _memory_span_size) - SPAN_HEADER_SIZE)
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

#যদি ENABLE_VALIDATE_ARGS
//! Maximum allocation size to avoid integer overflow
#undef MAX_ALLOC_SIZE
#define MAX_ALLOC_SIZE (((size_t) - 1) - _memory_span_size)
#endif

#define pointer_offset(ptr, ofs) (শূন্য *)((বর্ণ *)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff(first, second)                                            \
  (ptrdiff_t)((ধ্রুব বর্ণ *)(first) - (ধ্রুব বর্ণ *)(second))

#define INVALID_POINTER ((শূন্য *)((uintptr_t) - 1))

#define SIZE_CLASS_LARGE SIZE_CLASS_গুনতি
#define SIZE_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
প্রকারনির্দেশ সংরচনা heap_t heap_t;
//! Span of memory pages
প্রকারনির্দেশ সংরচনা span_t span_t;
//! Span list
প্রকারনির্দেশ সংরচনা span_list_t span_list_t;
//! Span active data
প্রকারনির্দেশ সংরচনা span_active_t span_active_t;
//! Size class definition
প্রকারনির্দেশ সংরচনা size_class_t size_class_t;
//! Global cache
প্রকারনির্দেশ সংরচনা global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#define SPAN_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#define SPAN_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#define SPAN_FLAG_ALIGNED_BLOCKS 4U
//! Flag indicating an unmapped master span
#define SPAN_FLAG_UNMAPPED_MASTER 8U

#যদি ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
সংরচনা span_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t উপরের;
#যদি ENABLE_STATISTICS
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
প্রকারনির্দেশ সংরচনা span_use_t span_use_t;
#endif

#যদি ENABLE_STATISTICS
সংরচনা size_class_use_t {
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
প্রকারনির্দেশ সংরচনা size_class_use_t size_class_use_t;
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
সংরচনা span_t {
  //! Free list
  শূন্য *free_list;
  //! Total block count of size class
  uint32_t block_গুনতি;
  //! Size class
  uint32_t size_class;
  //! Index of last block initialized in free list
  uint32_t free_list_limit;
  //! Number of used blocks remaining when in partial state
  uint32_t used_গুনতি;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_size;
  //! Size of a block
  uint32_t block_size;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t span_গুনতি;
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
_Static_assert(পরিমাণ(span_t) <= SPAN_HEADER_SIZE, "span size mismatch");

সংরচনা span_cache_t {
  size_t গুনতি;
  span_t *span[MAX_THREAD_SPAN_CACHE];
};
প্রকারনির্দেশ সংরচনা span_cache_t span_cache_t;

সংরচনা span_large_cache_t {
  size_t গুনতি;
  span_t *span[MAX_THREAD_SPAN_LARGE_CACHE];
};
প্রকারনির্দেশ সংরচনা span_large_cache_t span_large_cache_t;

সংরচনা heap_size_class_t {
  //! Free list of active span
  শূন্য *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  span_t *partial_span;
  //! Early level cache of fully free spans
  span_t *cache;
};
প্রকারনির্দেশ সংরচনা heap_size_class_t heap_size_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
সংরচনা heap_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  heap_size_class_t size_class[SIZE_CLASS_গুনতি];
#যদি ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  span_cache_t span_cache;
#endif
  //! List of deferred free spans (single linked list)
  atomicptr_t span_free_deferred;
  //! Number of full spans
  size_t full_span_গুনতি;
  //! Mapped but unused spans
  span_t *span_reserve;
  //! Master span for mapped but unused spans
  span_t *span_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t child_গুনতি;
  //! Next heap in id list
  heap_t *next_heap;
  //! Next heap in orphan list
  heap_t *next_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  পূর্ণাঙ্ক finalize;
  //! Master heap owning the memory pages
  heap_t *master_heap;
#যদি ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  span_large_cache_t span_large_cache[LARGE_CLASS_গুনতি - 1];
#endif
#যদি RPMALLOC_FIRST_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  span_t *full_span[SIZE_CLASS_গুনতি];
  //! Double linked list of large and huge spans allocated by this heap
  span_t *large_huge_span;
#endif
#যদি ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  span_use_t span_use[LARGE_CLASS_গুনতি];
#endif
#যদি ENABLE_STATISTICS
  //! Allocation stats per size class
  size_class_use_t size_class_use[SIZE_CLASS_গুনতি + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_to_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_to_thread;
#endif
};

// Size class for defining a block size bucket
সংরচনা size_class_t {
  //! Size of blocks in this class
  uint32_t block_size;
  //! Number of blocks in each chunk
  uint16_t block_গুনতি;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(পরিমাণ(size_class_t) == 8, "Size class size mismatch");

সংরচনা global_cache_t {
  //! Cache lock
  atomic32_t lock;
  //! Cache count
  uint32_t গুনতি;
#যদি ENABLE_STATISTICS
  //! Insert count
  size_t ঢোকাও_গুনতি;
  //! Extract count
  size_t extract_গুনতি;
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
স্থির পূর্ণাঙ্ক _rpmalloc_initialized;
//! Main thread ID
স্থির uintptr_t _rpmalloc_main_thread_id;
//! Configuration
স্থির rpmalloc_config_t _memory_config;
//! Memory page size
স্থির size_t _memory_page_size;
//! Shift to divide by page size
স্থির size_t _memory_page_size_shift;
//! Granularity at which memory pages are mapped by OS
স্থির size_t _memory_map_granularity;
#যদি RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
স্থির size_t _memory_span_size;
//! Shift to divide by span size
স্থির size_t _memory_span_size_shift;
//! Mask to get to start of a memory span
স্থির uintptr_t _memory_span_mask;
#অন্যথা
//! Hardwired span size
#define _memory_span_size _memory_default_span_size
#define _memory_span_size_shift _memory_default_span_size_shift
#define _memory_span_mask _memory_default_span_mask
#endif
//! Number of spans to map in each map call
স্থির size_t memory_span_map_গুনতি;
//! Number of spans to keep reserved in each heap
স্থির size_t memory_heap_reserve_গুনতি;
//! Global size classes
স্থির size_class_t _memory_size_class[SIZE_CLASS_গুনতি];
//! Run-time size limit of medium blocks
স্থির size_t _memory_medium_size_limit;
//! Heap ID counter
স্থির atomic32_t _memory_heap_id;
//! Huge page support
স্থির পূর্ণাঙ্ক _memory_huge_pages;
#যদি ENABLE_GLOBAL_CACHE
//! Global span cache
স্থির global_cache_t _memory_span_cache[LARGE_CLASS_গুনতি];
#endif
//! Global reserved spans
স্থির span_t *_memory_global_reserve;
//! Global reserved count
স্থির size_t memory_global_reserve_গুনতি;
//! Global reserved master
স্থির span_t *_memory_global_reserve_master;
//! All heaps
স্থির heap_t *_memory_heaps[HEAP_ARRAY_SIZE];
//! Used to restrict access to mapping memory for huge pages
স্থির atomic32_t _memory_global_lock;
//! Orphaned heaps
স্থির heap_t *_memory_orphan_heaps;
#যদি RPMALLOC_FIRST_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
স্থির heap_t *_memory_first_class_orphan_heaps;
#endif
#যদি ENABLE_STATISTICS
//! Allocations counter
স্থির atomic64_t _allocation_counter;
//! Deallocations counter
স্থির atomic64_t _deallocation_counter;
//! Active heap count
স্থির atomic32_t _memory_active_heaps;
//! Number of currently mapped memory pages
স্থির atomic32_t _mapped_pages;
//! Peak number of concurrently mapped memory pages
স্থির int32_t _mapped_pages_peak;
//! Number of mapped master spans
স্থির atomic32_t _master_spans;
//! Number of unmapped dangling master spans
স্থির atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
স্থির atomic32_t _mapped_total;
//! Running counter of total number of unmapped memory pages since start
স্থির atomic32_t _unmapped_total;
//! Number of currently mapped memory pages in OS calls
স্থির atomic32_t _mapped_pages_os;
//! Number of currently allocated pages in huge allocations
স্থির atomic32_t _huge_pages_current;
//! Peak number of currently allocated pages in huge allocations
স্থির int32_t _huge_pages_peak;
#endif

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#যদি ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
স্থির pthread_key_t _memory_thread_heap;
#অন্যথা
#ifdef _MSC_VER
#define _Thread_local __declspec(thread)
#define TLS_MODEL
#অন্যথা
#ifndef __HAIKU__
#define TLS_MODEL __attribute__((tls_model("initial-exec")))
#অন্যথা
#define TLS_MODEL
#endif
#যদি !defined(__clang__) && defined(__GNUC__)
#define _Thread_local __thread
#endif
#endif
স্থির _Thread_local heap_t *_memory_thread_heap TLS_MODEL;
#endif

স্থির অন্তঃস্থ heap_t *get_thread_heap_raw(শূন্য) {
#যদি (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  প্রত্যাগম pthread_getspecific(_memory_thread_heap);
#অন্যথা
  প্রত্যাগম _memory_thread_heap;
#endif
}

//! Get the current thread heap
স্থির অন্তঃস্থ heap_t *get_thread_heap(শূন্য) {
  heap_t *heap = get_thread_heap_raw();
#যদি ENABLE_PRELOAD
  যদি (EXPECTED(heap != 0))
    প্রত্যাগম heap;
  rpmalloc_initialize();
  প্রত্যাগম get_thread_heap_raw();
#অন্যথা
  প্রত্যাগম heap;
#endif
}

//! Fast thread ID
স্থির অন্তঃস্থ uintptr_t get_thread_id(শূন্য) {
#যদি defined(_WIN32)
  প্রত্যাগম (uintptr_t)((শূন্য *)NtCurrentTeb());
#নোচেদ্যদি (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#যদি defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#নোচেদ্যদি defined(__x86_64__)
#যদি defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#অন্যথা
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#endif
#নোচেদ্যদি defined(__arm__)
  __asm__ চঞ্চল("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#নোচেদ্যদি defined(__aarch64__)
#যদি defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ চঞ্চল("mrs %0, tpidrro_el0" : "=r"(tid));
#অন্যথা
  __asm__ চঞ্চল("mrs %0, tpidr_el0" : "=r"(tid));
#endif
#অন্যথা
#error This platform needs implementation of get_thread_id()
#endif
  প্রত্যাগম tid;
#অন্যথা
#error This platform needs implementation of get_thread_id()
#endif
}

//! Set the current thread heap
স্থির শূন্য set_thread_heap(heap_t *heap) {
#যদি ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(_memory_thread_heap, heap);
#অন্যথা
  _memory_thread_heap = heap;
#endif
  যদি (heap)
    heap->owner_thread = get_thread_id();
}

//! Set main thread ID
বাহ্য শূন্য rpmalloc_set_main_thread(শূন্য);

শূন্য rpmalloc_set_main_thread(শূন্য) {
  _rpmalloc_main_thread_id = get_thread_id();
}

স্থির শূন্য _rpmalloc_spin(শূন্য) {
#যদি defined(_MSC_VER)
#যদি defined(_M_ARM64)
  __yield();
#অন্যথা
  _mm_pause();
#endif
#নোচেদ্যদি defined(__x86_64__) || defined(__i386__)
  __asm__ চঞ্চল("pause" ::: "memory");
#নোচেদ্যদি defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ চঞ্চল("yield" ::: "memory");
#নোচেদ্যদি defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ চঞ্চল("or 27,27,27");
#নোচেদ্যদি defined(__sparc__)
  __asm__ চঞ্চল("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#অন্যথা
  সংরচনা timespec ts = {0};
  nanosleep(&ts, 0);
#endif
}

#যদি defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

স্থির শূন্য NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  বিকল্প (reason) {
  পক্ষ DLL_PROCESS_ATTACH:
    ভঙ্গ;
  পক্ষ DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    ভঙ্গ;
  পক্ষ DLL_THREAD_ATTACH:
    ভঙ্গ;
  পক্ষ DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    ভঙ্গ;
  }
}

#ifdef _WIN64
#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#pragma const_seg(".CRT$XLY")

বাহ্য ধ্রুব PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ধ্রুব PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#pragma const_seg()
#অন্যথা // _WIN64
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#pragma data_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#pragma data_seg()
#endif // _WIN64

স্থির শূন্য NTAPI _rpmalloc_thread_destructor(শূন্য *মান_2) {
#যদি ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  যদি (get_thread_id() == _rpmalloc_main_thread_id)
    প্রত্যাগম;
#endif
  যদি (মান_2)
    rpmalloc_thread_finalize(1);
}
#endif

////////////
///
/// Low level memory map/unmap
///
//////

স্থির শূন্য _rpmalloc_set_name(শূন্য *address, size_t size) {
#যদি defined(__linux__) || defined(__ANDROID__)
  ধ্রুব বর্ণ *name = _memory_huge_pages ? _memory_config.huge_page_name
                                        : _memory_config.page_name;
  যদি (address == MAP_FAILED || !name)
    প্রত্যাগম;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (শূন্য)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (uintptr_t)address, size,
              (uintptr_t)name);
#অন্যথা
  (শূন্য)পরিমাণ(size);
  (শূন্য)পরিমাণ(address);
#endif
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
স্থির শূন্য *_rpmalloc_mmap(size_t size, size_t *offset) {
  rpmalloc_assert(!(size % _memory_page_size), "Invalid mmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
  শূন্য *address = _memory_config.memory_map(size, offset);
  যদি (EXPECTED(address != 0)) {
    _rpmalloc_stat_add_peak(&_mapped_pages, (size >> _memory_page_size_shift),
                            _mapped_pages_peak);
    _rpmalloc_stat_add(&_mapped_total, (size >> _memory_page_size_shift));
  }
  প্রত্যাগম address;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
স্থির শূন্য _rpmalloc_unmap(শূন্য *address, size_t size, size_t offset,
                            size_t release) {
  rpmalloc_assert(!release || (release >= size), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  যদি (release) {
    rpmalloc_assert(!(release % _memory_page_size), "Invalid unmap size");
    _rpmalloc_stat_sub(&_mapped_pages, (release >> _memory_page_size_shift));
    _rpmalloc_stat_add(&_unmapped_total, (release >> _memory_page_size_shift));
  }
  _memory_config.memory_unmap(address, size, offset, release);
}

//! Default implementation to map new pages to virtual memory
স্থির শূন্য *_rpmalloc_mmap_os(size_t size, size_t *offset) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((size >= _memory_span_size) &&
                    (_memory_span_size > _memory_map_granularity))
                       ? _memory_span_size
                       : 0;
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
#যদি PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  শূন্য *ptr = VirtualAlloc(0, size + padding,
                           (_memory_huge_pages ? MEM_LARGE_PAGES : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
  যদি (!ptr) {
    যদি (_memory_config.map_fail_callback) {
      যদি (_memory_config.map_fail_callback(size + padding))
        প্রত্যাগম _rpmalloc_mmap_os(size, offset);
    } অন্যথা {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    প্রত্যাগম 0;
  }
#অন্যথা
  পূর্ণাঙ্ক flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#যদি defined(__APPLE__) && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
  পূর্ণাঙ্ক fd = (পূর্ণাঙ্ক)VM_MAKE_TAG(240U);
  যদি (_memory_huge_pages)
    fd |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
  শূন্য *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#নোচেদ্যদি defined(MAP_HUGETLB)
  শূন্য *ptr = mmap(0, size + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (_memory_huge_pages ? MAP_HUGETLB : 0) | flags, -1, 0);
#যদি defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  যদি ((ptr == MAP_FAILED || !ptr) && _memory_huge_pages) {
    ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    যদি (ptr && ptr != MAP_FAILED) {
      পূর্ণাঙ্ক prm = madvise(ptr, size + padding, MADV_HUGEPAGE);
      (শূন্য)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#endif
  _rpmalloc_set_name(ptr, size + padding);
#নোচেদ্যদি defined(MAP_ALIGNED)
  ধ্রুব size_t align =
      (পরিমাণ(size_t) * 8) - (size_t)(__builtin_clzl(size - 1));
  শূন্য *ptr =
      mmap(0, size + padding, PROT_READ | PROT_WRITE,
           (_memory_huge_pages ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#নোচেদ্যদি defined(MAP_ALIGN)
  caddr_t base = (_memory_huge_pages ? (caddr_t)(4 << 20) : 0);
  শূন্য *ptr = mmap(base, size + padding, PROT_READ | PROT_WRITE,
                   (_memory_huge_pages ? MAP_ALIGN : 0) | flags, -1, 0);
#অন্যথা
  শূন্য *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#endif
  যদি ((ptr == MAP_FAILED) || !ptr) {
    যদি (_memory_config.map_fail_callback) {
      যদি (_memory_config.map_fail_callback(size + padding))
        প্রত্যাগম _rpmalloc_mmap_os(size, offset);
    } অন্যথা যদি (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    প্রত্যাগম 0;
  }
#endif
  _rpmalloc_stat_add(&_mapped_pages_os,
                     (int32_t)((size + padding) >> _memory_page_size_shift));
  যদি (padding) {
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
  প্রত্যাগম ptr;
}

//! Default implementation to unmap pages from virtual memory
স্থির শূন্য _rpmalloc_unmap_os(শূন্য *address, size_t size, size_t offset,
                               size_t release) {
  rpmalloc_assert(release || (offset == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid unmap size");
  যদি (release && offset) {
    offset <<= 3;
    address = pointer_offset(address, -(int32_t)offset);
    যদি ((release >= _memory_span_size) &&
        (_memory_span_size > _memory_map_granularity)) {
      // Padding is always one span size
      release += _memory_span_size;
    }
  }
#যদি !DISABLE_UNMAP
#যদি PLATFORM_WINDOWS
  যদি (!VirtualFree(address, release ? 0 : size,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#অন্যথা
  যদি (release) {
    যদি (munmap(address, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } অন্যথা {
#যদি defined(MADV_FREE_REUSABLE)
    পূর্ণাঙ্ক ret;
    যাবত্ ((ret = madvise(address, size, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    যদি ((ret == -1) && (errno != 0)) {
#নোচেদ্যদি defined(MADV_DONTNEED)
    যদি (madvise(address, size, MADV_DONTNEED)) {
#নোচেদ্যদি defined(MADV_PAGEOUT)
    যদি (madvise(address, size, MADV_PAGEOUT)) {
#নোচেদ্যদি defined(MADV_FREE)
    যদি (madvise(address, size, MADV_FREE)) {
#অন্যথা
    যদি (posix_madvise(address, size, POSIX_MADV_DONTNEED)) {
#endif
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#endif
#endif
  যদি (release)
    _rpmalloc_stat_sub(&_mapped_pages_os, release >> _memory_page_size_shift);
}

স্থির শূন্য _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_গুনতি);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
স্থির span_t *_rpmalloc_global_get_reserved_spans(size_t span_গুনতি) {
  span_t *span = _memory_global_reserve;
  _rpmalloc_span_mark_as_subspan_unless_master(_memory_global_reserve_master,
                                               span, span_গুনতি);
  memory_global_reserve_গুনতি -= span_গুনতি;
  যদি (memory_global_reserve_গুনতি)
    _memory_global_reserve =
        (span_t *)pointer_offset(span, span_গুনতি << _memory_span_size_shift);
  অন্যথা
    _memory_global_reserve = 0;
  প্রত্যাগম span;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
স্থির শূন্য _rpmalloc_global_set_reserved_spans(span_t *master, span_t *reserve,
                                                size_t reserve_span_গুনতি) {
  _memory_global_reserve_master = master;
  memory_global_reserve_গুনতি = reserve_span_গুনতি;
  _memory_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
স্থির শূন্য _rpmalloc_span_double_link_list_add(span_t **head, span_t *span) {
  যদি (*head)
    (*head)->prev = span;
  span->next = *head;
  *head = span;
}

//! Pop head span from double linked list
স্থির শূন্য rpmalloc_span_double_link_list_বের_করো_head(span_t **head,
                                                     span_t *span) {
  rpmalloc_assert(*head == span, "Linked list corrupted");
  span = *head;
  *head = span->next;
}

//! Remove a span from double linked list
স্থির শূন্য _rpmalloc_span_double_link_list_remove(span_t **head,
                                                   span_t *span) {
  rpmalloc_assert(*head, "Linked list corrupted");
  যদি (*head == span) {
    *head = span->next;
  } অন্যথা {
    span_t *next_span = span->next;
    span_t *prev_span = span->prev;
    prev_span->next = next_span;
    যদি (EXPECTED(next_span != 0))
      next_span->prev = prev_span;
  }
}

////////////
///
/// Span control
///
//////

স্থির শূন্য rpmalloc_heap_cache_ঢোকাও(heap_t *heap, span_t *span);

স্থির শূন্য _rpmalloc_heap_finalize(heap_t *heap);

স্থির শূন্য _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_গুনতি);

//! Declare the span to be a subspan and store distance from master span and
//! span count
স্থির শূন্য _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_গুনতি) {
  rpmalloc_assert((subspan != master) || (subspan->flags & SPAN_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  যদি (subspan != master) {
    subspan->flags = SPAN_FLAG_SUBSPAN;
    subspan->offset_from_master =
        (uint32_t)((uintptr_t)pointer_diff(subspan, master) >>
                   _memory_span_size_shift);
    subspan->align_offset = 0;
  }
  subspan->span_গুনতি = (uint32_t)span_গুনতি;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
স্থির span_t *_rpmalloc_span_map_from_reserve(heap_t *heap,
                                               size_t span_গুনতি) {
  // Update the heap span reserve
  span_t *span = heap->span_reserve;
  heap->span_reserve =
      (span_t *)pointer_offset(span, span_গুনতি * _memory_span_size);
  heap->spans_reserved -= (uint32_t)span_গুনতি;

  _rpmalloc_span_mark_as_subspan_unless_master(heap->span_reserve_master, span,
                                               span_গুনতি);
  যদি (span_গুনতি <= LARGE_CLASS_গুনতি)
    _rpmalloc_stat_inc(&heap->span_use[span_গুনতি - 1].spans_from_reserved);

  প্রত্যাগম span;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
স্থির size_t rpmalloc_span_align_গুনতি(size_t span_গুনতি) {
  size_t request_গুনতি = (span_গুনতি > memory_span_map_গুনতি)
                             ? span_গুনতি
                             : memory_span_map_গুনতি;
  যদি ((_memory_page_size > _memory_span_size) &&
      ((request_গুনতি * _memory_span_size) % _memory_page_size))
    request_গুনতি +=
        memory_span_map_গুনতি - (request_গুনতি % memory_span_map_গুনতি);
  প্রত্যাগম request_গুনতি;
}

//! Setup a newly mapped span
স্থির শূন্য _rpmalloc_span_initialize(span_t *span, size_t total_span_গুনতি,
                                      size_t span_গুনতি, size_t align_offset) {
  span->total_spans = (uint32_t)total_span_গুনতি;
  span->span_গুনতি = (uint32_t)span_গুনতি;
  span->align_offset = (uint32_t)align_offset;
  span->flags = SPAN_FLAG_MASTER;
  atomic_store32(&span->remaining_spans, (int32_t)total_span_গুনতি);
}

স্থির শূন্য _rpmalloc_span_unmap(span_t *span);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
স্থির span_t *rpmalloc_span_map_aligned_গুনতি(heap_t *heap,
                                                size_t span_গুনতি) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_span_গুনতি = rpmalloc_span_align_গুনতি(span_গুনতি);
  size_t align_offset = 0;
  span_t *span = (span_t *)_rpmalloc_mmap(
      aligned_span_গুনতি * _memory_span_size, &align_offset);
  যদি (!span)
    প্রত্যাগম 0;
  _rpmalloc_span_initialize(span, aligned_span_গুনতি, span_গুনতি, align_offset);
  _rpmalloc_stat_inc(&_master_spans);
  যদি (span_গুনতি <= LARGE_CLASS_গুনতি)
    _rpmalloc_stat_inc(&heap->span_use[span_গুনতি - 1].spans_map_calls);
  যদি (aligned_span_গুনতি > span_গুনতি) {
    span_t *reserved_spans =
        (span_t *)pointer_offset(span, span_গুনতি * _memory_span_size);
    size_t reserved_গুনতি = aligned_span_গুনতি - span_গুনতি;
    যদি (heap->spans_reserved) {
      _rpmalloc_span_mark_as_subspan_unless_master(
          heap->span_reserve_master, heap->span_reserve, heap->spans_reserved);
      rpmalloc_heap_cache_ঢোকাও(heap, heap->span_reserve);
    }
    যদি (reserved_গুনতি > memory_heap_reserve_গুনতি) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&_memory_global_lock) == 1,
                      "Global spin lock not held as expected");
      size_t remain_গুনতি = reserved_গুনতি - memory_heap_reserve_গুনতি;
      reserved_গুনতি = memory_heap_reserve_গুনতি;
      span_t *remain_span = (span_t *)pointer_offset(
          reserved_spans, reserved_গুনতি * _memory_span_size);
      যদি (_memory_global_reserve) {
        _rpmalloc_span_mark_as_subspan_unless_master(
            _memory_global_reserve_master, _memory_global_reserve,
            memory_global_reserve_গুনতি);
        _rpmalloc_span_unmap(_memory_global_reserve);
      }
      _rpmalloc_global_set_reserved_spans(span, remain_span, remain_গুনতি);
    }
    _rpmalloc_heap_set_reserved_spans(heap, span, reserved_spans,
                                      reserved_গুনতি);
  }
  প্রত্যাগম span;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
স্থির span_t *_rpmalloc_span_map(heap_t *heap, size_t span_গুনতি) {
  যদি (span_গুনতি <= heap->spans_reserved)
    প্রত্যাগম _rpmalloc_span_map_from_reserve(heap, span_গুনতি);
  span_t *span = 0;
  পূর্ণাঙ্ক use_global_reserve =
      (_memory_page_size > _memory_span_size) ||
      (memory_span_map_গুনতি > memory_heap_reserve_গুনতি);
  যদি (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    যাবত্ (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
    যদি (memory_global_reserve_গুনতি >= span_গুনতি) {
      size_t reserve_গুনতি =
          (!heap->spans_reserved ? memory_heap_reserve_গুনতি : span_গুনতি);
      যদি (memory_global_reserve_গুনতি < reserve_গুনতি)
        reserve_গুনতি = memory_global_reserve_গুনতি;
      span = _rpmalloc_global_get_reserved_spans(reserve_গুনতি);
      যদি (span) {
        যদি (reserve_গুনতি > span_গুনতি) {
          span_t *reserved_span = (span_t *)pointer_offset(
              span, span_গুনতি << _memory_span_size_shift);
          _rpmalloc_heap_set_reserved_spans(heap, _memory_global_reserve_master,
                                            reserved_span,
                                            reserve_গুনতি - span_গুনতি);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        span->span_গুনতি = (uint32_t)span_গুনতি;
      }
    }
  }
  যদি (!span)
    span = rpmalloc_span_map_aligned_গুনতি(heap, span_গুনতি);
  যদি (use_global_reserve)
    atomic_store32_release(&_memory_global_lock, 0);
  প্রত্যাগম span;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
স্থির শূন্য _rpmalloc_span_unmap(span_t *span) {
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");

  পূর্ণাঙ্ক is_master = !!(span->flags & SPAN_FLAG_MASTER);
  span_t *master =
      is_master ? span
                : ((span_t *)pointer_offset(
                      span, -(intptr_t)((uintptr_t)span->offset_from_master *
                                        _memory_span_size)));
  rpmalloc_assert(is_master || (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");

  size_t span_গুনতি = span->span_গুনতি;
  যদি (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(span->align_offset == 0, "Span align offset corrupted");
    যদি (_memory_span_size >= _memory_page_size)
      _rpmalloc_unmap(span, span_গুনতি * _memory_span_size, 0, 0);
  } অন্যথা {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    span->flags |=
        SPAN_FLAG_MASTER | SPAN_FLAG_SUBSPAN | SPAN_FLAG_UNMAPPED_MASTER;
    _rpmalloc_stat_add(&_unmapped_master_spans, 1);
  }

  যদি (atomic_add32(&master->remaining_spans, -(int32_t)span_গুনতি) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & SPAN_FLAG_MASTER) &&
                        !!(master->flags & SPAN_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_গুনতি = master->span_গুনতি;
    যদি (_memory_span_size < _memory_page_size)
      unmap_গুনতি = master->total_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_গুনতি * _memory_span_size,
                    master->align_offset,
                    (size_t)master->total_spans * _memory_span_size);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
স্থির শূন্য _rpmalloc_span_release_to_cache(heap_t *heap, span_t *span) {
  rpmalloc_assert(heap == span->heap, "Span heap pointer corrupted");
  rpmalloc_assert(span->size_class < SIZE_CLASS_গুনতি,
                  "Invalid span size class");
  rpmalloc_assert(span->span_গুনতি == 1, "Invalid span count");
#যদি ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&heap->span_use[0].current);
#endif
  _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
  যদি (!heap->finalize) {
    _rpmalloc_stat_inc(&heap->span_use[0].spans_to_cache);
    _rpmalloc_stat_inc(&heap->size_class_use[span->size_class].spans_to_cache);
    যদি (heap->size_class[span->size_class].cache)
      rpmalloc_heap_cache_ঢোকাও(heap,
                                  heap->size_class[span->size_class].cache);
    heap->size_class[span->size_class].cache = span;
  } অন্যথা {
    _rpmalloc_span_unmap(span);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
স্থির uint32_t free_list_partial_init(শূন্য **list, শূন্য **first_block,
                                       শূন্য *page_start, শূন্য *block_start,
                                       uint32_t block_গুনতি,
                                       uint32_t block_size) {
  rpmalloc_assert(block_গুনতি, "Internal failure");
  *first_block = block_start;
  যদি (block_গুনতি > 1) {
    শূন্য *free_block = pointer_offset(block_start, block_size);
    শূন্য *block_end =
        pointer_offset(block_start, (size_t)block_size * block_গুনতি);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    যদি (block_size < (_memory_page_size >> 1)) {
      শূন্য *page_end = pointer_offset(page_start, _memory_page_size);
      যদি (page_end < block_end)
        block_end = page_end;
    }
    *list = free_block;
    block_গুনতি = 2;
    শূন্য *next_block = pointer_offset(free_block, block_size);
    যাবত্ (next_block < block_end) {
      *((শূন্য **)free_block) = next_block;
      free_block = next_block;
      ++block_গুনতি;
      next_block = pointer_offset(next_block, block_size);
    }
    *((শূন্য **)free_block) = 0;
  } অন্যথা {
    *list = 0;
  }
  প্রত্যাগম block_গুনতি;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
স্থির শূন্য *_rpmalloc_span_initialize_new(heap_t *heap,
                                           heap_size_class_t *heap_size_class,
                                           span_t *span, uint32_t class_idx) {
  rpmalloc_assert(span->span_গুনতি == 1, "Internal failure");
  size_class_t *size_class = _memory_size_class + class_idx;
  span->size_class = class_idx;
  span->heap = heap;
  span->flags &= ~SPAN_FLAG_ALIGNED_BLOCKS;
  span->block_size = size_class->block_size;
  span->block_গুনতি = size_class->block_গুনতি;
  span->free_list = 0;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  শূন্য *block;
  span->free_list_limit =
      free_list_partial_init(&heap_size_class->free_list, &block, span,
                             pointer_offset(span, SPAN_HEADER_SIZE),
                             size_class->block_গুনতি, size_class->block_size);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  যদি (span->free_list_limit < span->block_গুনতি) {
    _rpmalloc_span_double_link_list_add(&heap_size_class->partial_span, span);
    span->used_গুনতি = span->free_list_limit;
  } অন্যথা {
#যদি RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_গুনতি;
    span->used_গুনতি = span->block_গুনতি;
  }
  প্রত্যাগম block;
}

স্থির শূন্য _rpmalloc_span_extract_free_list_deferred(span_t *span) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  কুরু {
    span->free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } যাবত্ (span->free_list == INVALID_POINTER);
  span->used_গুনতি -= span->list_size;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);
}

স্থির পূর্ণাঙ্ক _rpmalloc_span_is_fully_utilized(span_t *span) {
  rpmalloc_assert(span->free_list_limit <= span->block_গুনতি,
                  "Span free list corrupted");
  প্রত্যাগম !span->free_list && (span->free_list_limit >= span->block_গুনতি);
}

স্থির পূর্ণাঙ্ক _rpmalloc_span_finalize(heap_t *heap, size_t iclass, span_t *span,
                                   span_t **list_head) {
  শূন্য *free_list = heap->size_class[iclass].free_list;
  span_t *class_span = (span_t *)((uintptr_t)free_list & _memory_span_mask);
  যদি (span == class_span) {
    // Adopt the heap class free list back into the span free list
    শূন্য *block = span->free_list;
    শূন্য *last_block = 0;
    যাবত্ (block) {
      last_block = block;
      block = *((শূন্য **)block);
    }
    uint32_t free_গুনতি = 0;
    block = free_list;
    যাবত্ (block) {
      ++free_গুনতি;
      block = *((শূন্য **)block);
    }
    যদি (last_block) {
      *((শূন্য **)last_block) = free_list;
    } অন্যথা {
      span->free_list = free_list;
    }
    heap->size_class[iclass].free_list = 0;
    span->used_গুনতি -= free_গুনতি;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(span->list_size == span->used_গুনতি, "Memory leak detected");
  যদি (span->list_size == span->used_গুনতি) {
    _rpmalloc_stat_dec(&heap->span_use[0].current);
    _rpmalloc_stat_dec(&heap->size_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    যদি (list_head)
      _rpmalloc_span_double_link_list_remove(list_head, span);
    _rpmalloc_span_unmap(span);
    প্রত্যাগম 1;
  }
  প্রত্যাগম 0;
}

////////////
///
/// Global cache
///
//////

#যদি ENABLE_GLOBAL_CACHE

//! Finalize a global cache
স্থির শূন্য _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  যাবত্ (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

  প্রতি (size_t ispan = 0; ispan < cache->গুনতি; ++ispan)
    _rpmalloc_span_unmap(cache->span[ispan]);
  cache->গুনতি = 0;

  যাবত্ (cache->overflow) {
    span_t *span = cache->overflow;
    cache->overflow = span->next;
    _rpmalloc_span_unmap(span);
  }

  atomic_store32_release(&cache->lock, 0);
}

স্থির শূন্য rpmalloc_global_cache_ঢোকাও_spans(span_t **span,
                                                size_t span_গুনতি,
                                                size_t গুনতি) {
  ধ্রুব size_t cache_limit =
      (span_গুনতি == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_SPAN_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_SPAN_LARGE_CACHE - (span_গুনতি >> 1));

  global_cache_t *cache = &_memory_span_cache[span_গুনতি - 1];

  size_t ঢোকাও_গুনতি = গুনতি;
  যাবত্ (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#যদি ENABLE_STATISTICS
  cache->ঢোকাও_গুনতি += গুনতি;
#endif
  যদি ((cache->গুনতি + ঢোকাও_গুনতি) > cache_limit)
    ঢোকাও_গুনতি = cache_limit - cache->গুনতি;

  memcpy(cache->span + cache->গুনতি, span, পরিমাণ(span_t *) * ঢোকাও_গুনতি);
  cache->গুনতি += (uint32_t)ঢোকাও_গুনতি;

#যদি ENABLE_UNLIMITED_CACHE
  যাবত্ (ঢোকাও_গুনতি < গুনতি) {
#অন্যথা
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  যাবত্ ((_memory_page_size > _memory_span_size) && (ঢোকাও_গুনতি < গুনতি)) {
#endif
    span_t *current_span = span[ঢোকাও_গুনতি++];
    current_span->next = cache->overflow;
    cache->overflow = current_span;
  }
  atomic_store32_release(&cache->lock, 0);

  span_t *keep = 0;
  প্রতি (size_t ispan = ঢোকাও_গুনতি; ispan < গুনতি; ++ispan) {
    span_t *current_span = span[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    যদি ((current_span->flags & SPAN_FLAG_MASTER) &&
        (atomic_load32(&current_span->remaining_spans) >
         (int32_t)current_span->span_গুনতি)) {
      current_span->next = keep;
      keep = current_span;
    } অন্যথা {
      _rpmalloc_span_unmap(current_span);
    }
  }

  যদি (keep) {
    যাবত্ (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    যাবত্ (keep) {
      প্রতি (; islot < cache->গুনতি; ++islot) {
        span_t *current_span = cache->span[islot];
        যদি (!(current_span->flags & SPAN_FLAG_MASTER) ||
            ((current_span->flags & SPAN_FLAG_MASTER) &&
             (atomic_load32(&current_span->remaining_spans) <=
              (int32_t)current_span->span_গুনতি))) {
          _rpmalloc_span_unmap(current_span);
          cache->span[islot] = keep;
          ভঙ্গ;
        }
      }
      যদি (islot == cache->গুনতি)
        ভঙ্গ;
      keep = keep->next;
    }

    যদি (keep) {
      span_t *tail = keep;
      যাবত্ (tail->next)
        tail = tail->next;
      tail->next = cache->overflow;
      cache->overflow = keep;
    }

    atomic_store32_release(&cache->lock, 0);
  }
}

স্থির size_t _rpmalloc_global_cache_extract_spans(span_t **span,
                                                   size_t span_গুনতি,
                                                   size_t গুনতি) {
  global_cache_t *cache = &_memory_span_cache[span_গুনতি - 1];

  size_t extract_গুনতি = 0;
  যাবত্ (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#যদি ENABLE_STATISTICS
  cache->extract_গুনতি += গুনতি;
#endif
  size_t want = গুনতি - extract_গুনতি;
  যদি (want > cache->গুনতি)
    want = cache->গুনতি;

  memcpy(span + extract_গুনতি, cache->span + (cache->গুনতি - want),
         পরিমাণ(span_t *) * want);
  cache->গুনতি -= (uint32_t)want;
  extract_গুনতি += want;

  যাবত্ ((extract_গুনতি < গুনতি) && cache->overflow) {
    span_t *current_span = cache->overflow;
    span[extract_গুনতি++] = current_span;
    cache->overflow = current_span->next;
  }

#যদি ENABLE_ASSERTS
  প্রতি (size_t ispan = 0; ispan < extract_গুনতি; ++ispan) {
    rpmalloc_assert(span[ispan]->span_গুনতি == span_গুনতি,
                    "Global cache span count mismatch");
  }
#endif

  atomic_store32_release(&cache->lock, 0);

  প্রত্যাগম extract_গুনতি;
}

#endif

////////////
///
/// Heap control
///
//////

স্থির শূন্য _rpmalloc_deallocate_huge(span_t *);

//! Store the given spans as reserve in the given heap
স্থির শূন্য _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_গুনতি) {
  heap->span_reserve_master = master;
  heap->span_reserve = reserve;
  heap->spans_reserved = (uint32_t)reserve_span_গুনতি;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
স্থির শূন্য _rpmalloc_heap_cache_adopt_deferred(heap_t *heap,
                                                span_t **single_span) {
  span_t *span = (span_t *)((শূন্য *)atomic_exchange_ptr_acquire(
      &heap->span_free_deferred, 0));
  যাবত্ (span) {
    span_t *next_span = (span_t *)span->free_list;
    rpmalloc_assert(span->heap == heap, "Span heap pointer corrupted");
    যদি (EXPECTED(span->size_class < SIZE_CLASS_গুনতি)) {
      rpmalloc_assert(heap->full_span_গুনতি, "Heap span counter corrupted");
      --heap->full_span_গুনতি;
      _rpmalloc_stat_dec(&heap->span_use[0].spans_deferred);
#যদি RPMALLOC_FIRST_CLASS_HEAPS
      _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                             span);
#endif
      _rpmalloc_stat_dec(&heap->span_use[0].current);
      _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
      যদি (single_span && !*single_span)
        *single_span = span;
      অন্যথা
        rpmalloc_heap_cache_ঢোকাও(heap, span);
    } অন্যথা {
      যদি (span->size_class == SIZE_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(span);
      } অন্যথা {
        rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(heap->full_span_গুনতি, "Heap span counter corrupted");
        --heap->full_span_গুনতি;
#যদি RPMALLOC_FIRST_CLASS_HEAPS
        _rpmalloc_span_double_link_list_remove(&heap->large_huge_span, span);
#endif
        uint32_t idx = span->span_গুনতি - 1;
        _rpmalloc_stat_dec(&heap->span_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&heap->span_use[idx].current);
        যদি (!idx && single_span && !*single_span)
          *single_span = span;
        অন্যথা
          rpmalloc_heap_cache_ঢোকাও(heap, span);
      }
    }
    span = next_span;
  }
}

স্থির শূন্য _rpmalloc_heap_unmap(heap_t *heap) {
  যদি (!heap->master_heap) {
    যদি ((heap->finalize > 1) && !atomic_load32(&heap->child_গুনতি)) {
      span_t *span = (span_t *)((uintptr_t)heap & _memory_span_mask);
      _rpmalloc_span_unmap(span);
    }
  } অন্যথা {
    যদি (atomic_decr32(&heap->master_heap->child_গুনতি) == 0) {
      _rpmalloc_heap_unmap(heap->master_heap);
    }
  }
}

স্থির শূন্য _rpmalloc_heap_global_finalize(heap_t *heap) {
  যদি (heap->finalize++ > 1) {
    --heap->finalize;
    প্রত্যাগম;
  }

  _rpmalloc_heap_finalize(heap);

#যদি ENABLE_THREAD_CACHE
  প্রতি (size_t iclass = 0; iclass < LARGE_CLASS_গুনতি; ++iclass) {
    span_cache_t *span_cache;
    যদি (!iclass)
      span_cache = &heap->span_cache;
    অন্যথা
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    প্রতি (size_t ispan = 0; ispan < span_cache->গুনতি; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->গুনতি = 0;
  }
#endif

  যদি (heap->full_span_গুনতি) {
    --heap->finalize;
    প্রত্যাগম;
  }

  প্রতি (size_t iclass = 0; iclass < SIZE_CLASS_গুনতি; ++iclass) {
    যদি (heap->size_class[iclass].free_list ||
        heap->size_class[iclass].partial_span) {
      --heap->finalize;
      প্রত্যাগম;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap_t *list_heap = _memory_heaps[list_idx];
  যদি (list_heap == heap) {
    _memory_heaps[list_idx] = heap->next_heap;
  } অন্যথা {
    যাবত্ (list_heap->next_heap != heap)
      list_heap = list_heap->next_heap;
    list_heap->next_heap = heap->next_heap;
  }

  _rpmalloc_heap_unmap(heap);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
স্থির শূন্য rpmalloc_heap_cache_ঢোকাও(heap_t *heap, span_t *span) {
  যদি (UNEXPECTED(heap->finalize != 0)) {
    _rpmalloc_span_unmap(span);
    _rpmalloc_heap_global_finalize(heap);
    প্রত্যাগম;
  }
#যদি ENABLE_THREAD_CACHE
  size_t span_গুনতি = span->span_গুনতি;
  _rpmalloc_stat_inc(&heap->span_use[span_গুনতি - 1].spans_to_cache);
  যদি (span_গুনতি == 1) {
    span_cache_t *span_cache = &heap->span_cache;
    span_cache->span[span_cache->গুনতি++] = span;
    যদি (span_cache->গুনতি == MAX_THREAD_SPAN_CACHE) {
      ধ্রুব size_t remain_গুনতি =
          MAX_THREAD_SPAN_CACHE - THREAD_SPAN_CACHE_TRANSFER;
#যদি ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           THREAD_SPAN_CACHE_TRANSFER * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_গুনতি - 1].spans_to_global,
                         THREAD_SPAN_CACHE_TRANSFER);
      rpmalloc_global_cache_ঢোকাও_spans(span_cache->span + remain_গুনতি,
                                          span_গুনতি,
                                          THREAD_SPAN_CACHE_TRANSFER);
#অন্যথা
      প্রতি (size_t ispan = 0; ispan < THREAD_SPAN_CACHE_TRANSFER; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_গুনতি + ispan]);
#endif
      span_cache->গুনতি = remain_গুনতি;
    }
  } অন্যথা {
    size_t cache_idx = span_গুনতি - 2;
    span_large_cache_t *span_cache = heap->span_large_cache + cache_idx;
    span_cache->span[span_cache->গুনতি++] = span;
    ধ্রুব size_t cache_limit =
        (MAX_THREAD_SPAN_LARGE_CACHE - (span_গুনতি >> 1));
    যদি (span_cache->গুনতি == cache_limit) {
      ধ্রুব size_t transfer_limit = 2 + (cache_limit >> 2);
      ধ্রুব size_t transfer_গুনতি =
          (THREAD_SPAN_LARGE_CACHE_TRANSFER <= transfer_limit
               ? THREAD_SPAN_LARGE_CACHE_TRANSFER
               : transfer_limit);
      ধ্রুব size_t remain_গুনতি = cache_limit - transfer_গুনতি;
#যদি ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           transfer_গুনতি * span_গুনতি * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_গুনতি - 1].spans_to_global,
                         transfer_গুনতি);
      rpmalloc_global_cache_ঢোকাও_spans(span_cache->span + remain_গুনতি,
                                          span_গুনতি, transfer_গুনতি);
#অন্যথা
      প্রতি (size_t ispan = 0; ispan < transfer_গুনতি; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_গুনতি + ispan]);
#endif
      span_cache->গুনতি = remain_গুনতি;
    }
  }
#অন্যথা
  (শূন্য)পরিমাণ(heap);
  _rpmalloc_span_unmap(span);
#endif
}

//! Extract the given number of spans from the different cache levels
স্থির span_t *_rpmalloc_heap_thread_cache_extract(heap_t *heap,
                                                   size_t span_গুনতি) {
  span_t *span = 0;
#যদি ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  যদি (span_গুনতি == 1)
    span_cache = &heap->span_cache;
  অন্যথা
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_গুনতি - 2));
  যদি (span_cache->গুনতি) {
    _rpmalloc_stat_inc(&heap->span_use[span_গুনতি - 1].spans_from_cache);
    প্রত্যাগম span_cache->span[--span_cache->গুনতি];
  }
#endif
  প্রত্যাগম span;
}

স্থির span_t *_rpmalloc_heap_thread_cache_deferred_extract(heap_t *heap,
                                                            size_t span_গুনতি) {
  span_t *span = 0;
  যদি (span_গুনতি == 1) {
    _rpmalloc_heap_cache_adopt_deferred(heap, &span);
  } অন্যথা {
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
    span = _rpmalloc_heap_thread_cache_extract(heap, span_গুনতি);
  }
  প্রত্যাগম span;
}

স্থির span_t *_rpmalloc_heap_reserved_extract(heap_t *heap,
                                               size_t span_গুনতি) {
  যদি (heap->spans_reserved >= span_গুনতি)
    প্রত্যাগম _rpmalloc_span_map(heap, span_গুনতি);
  প্রত্যাগম 0;
}

//! Extract a span from the global cache
স্থির span_t *_rpmalloc_heap_global_cache_extract(heap_t *heap,
                                                   size_t span_গুনতি) {
#যদি ENABLE_GLOBAL_CACHE
#যদি ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  size_t wanted_গুনতি;
  যদি (span_গুনতি == 1) {
    span_cache = &heap->span_cache;
    wanted_গুনতি = THREAD_SPAN_CACHE_TRANSFER;
  } অন্যথা {
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_গুনতি - 2));
    wanted_গুনতি = THREAD_SPAN_LARGE_CACHE_TRANSFER;
  }
  span_cache->গুনতি = _rpmalloc_global_cache_extract_spans(
      span_cache->span, span_গুনতি, wanted_গুনতি);
  যদি (span_cache->গুনতি) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_গুনতি * span_cache->গুনতি * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_গুনতি - 1].spans_from_global,
                       span_cache->গুনতি);
    প্রত্যাগম span_cache->span[--span_cache->গুনতি];
  }
#অন্যথা
  span_t *span = 0;
  size_t গুনতি = _rpmalloc_global_cache_extract_spans(&span, span_গুনতি, 1);
  যদি (গুনতি) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_গুনতি * গুনতি * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_গুনতি - 1].spans_from_global,
                       গুনতি);
    প্রত্যাগম span;
  }
#endif
#endif
  (শূন্য)পরিমাণ(heap);
  (শূন্য)পরিমাণ(span_গুনতি);
  প্রত্যাগম 0;
}

স্থির শূন্য _rpmalloc_inc_span_statistics(heap_t *heap, size_t span_গুনতি,
                                          uint32_t class_idx) {
  (শূন্য)পরিমাণ(heap);
  (শূন্য)পরিমাণ(span_গুনতি);
  (শূন্য)পরিমাণ(class_idx);
#যদি ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)span_গুনতি - 1;
  uint32_t current_গুনতি =
      (uint32_t)atomic_incr32(&heap->span_use[idx].current);
  যদি (current_গুনতি > (uint32_t)atomic_load32(&heap->span_use[idx].উপরের))
    atomic_store32(&heap->span_use[idx].উপরের, (int32_t)current_গুনতি);
  _rpmalloc_stat_add_peak(&heap->size_class_use[class_idx].spans_current, 1,
                          heap->size_class_use[class_idx].spans_peak);
#endif
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
স্থির span_t *
_rpmalloc_heap_extract_new_span(heap_t *heap,
                                heap_size_class_t *heap_size_class,
                                size_t span_গুনতি, uint32_t class_idx) {
  span_t *span;
#যদি ENABLE_THREAD_CACHE
  যদি (heap_size_class && heap_size_class->cache) {
    span = heap_size_class->cache;
    heap_size_class->cache =
        (heap->span_cache.গুনতি
             ? heap->span_cache.span[--heap->span_cache.গুনতি]
             : 0);
    _rpmalloc_inc_span_statistics(heap, span_গুনতি, class_idx);
    প্রত্যাগম span;
  }
#endif
  (শূন্য)পরিমাণ(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t base_span_গুনতি = span_গুনতি;
  size_t limit_span_গুনতি =
      (span_গুনতি > 2) ? (span_গুনতি + (span_গুনতি >> 1)) : span_গুনতি;
  যদি (limit_span_গুনতি > LARGE_CLASS_গুনতি)
    limit_span_গুনতি = LARGE_CLASS_গুনতি;
  কুরু {
    span = _rpmalloc_heap_thread_cache_extract(heap, span_গুনতি);
    যদি (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_গুনতি, class_idx);
      প্রত্যাগম span;
    }
    span = _rpmalloc_heap_thread_cache_deferred_extract(heap, span_গুনতি);
    যদি (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_গুনতি, class_idx);
      প্রত্যাগম span;
    }
    span = _rpmalloc_heap_global_cache_extract(heap, span_গুনতি);
    যদি (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_গুনতি, class_idx);
      প্রত্যাগম span;
    }
    span = _rpmalloc_heap_reserved_extract(heap, span_গুনতি);
    যদি (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_reserved);
      _rpmalloc_inc_span_statistics(heap, span_গুনতি, class_idx);
      প্রত্যাগম span;
    }
    ++span_গুনতি;
  } যাবত্ (span_গুনতি <= limit_span_গুনতি);
  // Final fallback, map in more virtual memory
  span = _rpmalloc_span_map(heap, base_span_গুনতি);
  _rpmalloc_inc_span_statistics(heap, base_span_গুনতি, class_idx);
  _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_map_calls);
  প্রত্যাগম span;
}

স্থির শূন্য _rpmalloc_heap_initialize(heap_t *heap) {
  _rpmalloc_memset_const(heap, 0, পরিমাণ(heap_t));
  // Get a new heap ID
  heap->id = 1 + atomic_incr32(&_memory_heap_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap->next_heap = _memory_heaps[list_idx];
  _memory_heaps[list_idx] = heap;
}

স্থির শূন্য _rpmalloc_heap_orphan(heap_t *heap, পূর্ণাঙ্ক first_class) {
  heap->owner_thread = (uintptr_t)-1;
#যদি RPMALLOC_FIRST_CLASS_HEAPS
  heap_t **heap_list =
      (first_class ? &_memory_first_class_orphan_heaps : &_memory_orphan_heaps);
#অন্যথা
  (শূন্য)পরিমাণ(first_class);
  heap_t **heap_list = &_memory_orphan_heaps;
#endif
  heap->next_orphan = *heap_list;
  *heap_list = heap;
}

//! Allocate a new heap from newly mapped memory pages
স্থির heap_t *_rpmalloc_heap_allocate_new(শূন্য) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t heap_size = পরিমাণ(heap_t);
  size_t aligned_heap_size = 16 * ((heap_size + 15) / 16);
  size_t request_heap_গুনতি = 16;
  size_t heap_span_গুনতি = ((aligned_heap_size * request_heap_গুনতি) +
                            পরিমাণ(span_t) + _memory_span_size - 1) /
                           _memory_span_size;
  size_t block_size = _memory_span_size * heap_span_গুনতি;
  size_t span_গুনতি = heap_span_গুনতি;
  span_t *span = 0;
  // If there are global reserved spans, use these first
  যদি (memory_global_reserve_গুনতি >= heap_span_গুনতি) {
    span = _rpmalloc_global_get_reserved_spans(heap_span_গুনতি);
  }
  যদি (!span) {
    যদি (_memory_page_size > block_size) {
      span_গুনতি = _memory_page_size / _memory_span_size;
      block_size = _memory_page_size;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_heap_গুনতি =
          (block_size - পরিমাণ(span_t)) / aligned_heap_size;
      যদি (possible_heap_গুনতি >= (request_heap_গুনতি * 16))
        request_heap_গুনতি *= 16;
      অন্যথা যদি (possible_heap_গুনতি < request_heap_গুনতি)
        request_heap_গুনতি = possible_heap_গুনতি;
      heap_span_গুনতি = ((aligned_heap_size * request_heap_গুনতি) +
                         পরিমাণ(span_t) + _memory_span_size - 1) /
                        _memory_span_size;
    }

    size_t align_offset = 0;
    span = (span_t *)_rpmalloc_mmap(block_size, &align_offset);
    যদি (!span)
      প্রত্যাগম 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    _rpmalloc_span_initialize(span, span_গুনতি, heap_span_গুনতি, align_offset);
  }

  size_t remain_size = _memory_span_size - পরিমাণ(span_t);
  heap_t *heap = (heap_t *)pointer_offset(span, পরিমাণ(span_t));
  _rpmalloc_heap_initialize(heap);

  // Put extra heaps as orphans
  size_t অঙ্ক_heaps = remain_size / aligned_heap_size;
  যদি (অঙ্ক_heaps < request_heap_গুনতি)
    অঙ্ক_heaps = request_heap_গুনতি;
  atomic_store32(&heap->child_গুনতি, (int32_t)অঙ্ক_heaps - 1);
  heap_t *extra_heap = (heap_t *)pointer_offset(heap, aligned_heap_size);
  যাবত্ (অঙ্ক_heaps > 1) {
    _rpmalloc_heap_initialize(extra_heap);
    extra_heap->master_heap = heap;
    _rpmalloc_heap_orphan(extra_heap, 1);
    extra_heap = (heap_t *)pointer_offset(extra_heap, aligned_heap_size);
    --অঙ্ক_heaps;
  }

  যদি (span_গুনতি > heap_span_গুনতি) {
    // Cap reserved spans
    size_t remain_গুনতি = span_গুনতি - heap_span_গুনতি;
    size_t reserve_গুনতি =
        (remain_গুনতি > memory_heap_reserve_গুনতি ? memory_heap_reserve_গুনতি
                                                   : remain_গুনতি);
    span_t *remain_span =
        (span_t *)pointer_offset(span, heap_span_গুনতি * _memory_span_size);
    _rpmalloc_heap_set_reserved_spans(heap, span, remain_span, reserve_গুনতি);

    যদি (remain_গুনতি > reserve_গুনতি) {
      // Set to global reserved spans
      remain_span = (span_t *)pointer_offset(remain_span,
                                             reserve_গুনতি * _memory_span_size);
      reserve_গুনতি = remain_গুনতি - reserve_গুনতি;
      _rpmalloc_global_set_reserved_spans(span, remain_span, reserve_গুনতি);
    }
  }

  প্রত্যাগম heap;
}

স্থির heap_t *_rpmalloc_heap_extract_orphan(heap_t **heap_list) {
  heap_t *heap = *heap_list;
  *heap_list = (heap ? heap->next_orphan : 0);
  প্রত্যাগম heap;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
স্থির heap_t *_rpmalloc_heap_allocate(পূর্ণাঙ্ক first_class) {
  heap_t *heap = 0;
  যাবত্ (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
    _rpmalloc_spin();
  যদি (first_class == 0)
    heap = _rpmalloc_heap_extract_orphan(&_memory_orphan_heaps);
#যদি RPMALLOC_FIRST_CLASS_HEAPS
  যদি (!heap)
    heap = _rpmalloc_heap_extract_orphan(&_memory_first_class_orphan_heaps);
#endif
  যদি (!heap)
    heap = _rpmalloc_heap_allocate_new();
  atomic_store32_release(&_memory_global_lock, 0);
  যদি (heap)
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  প্রত্যাগম heap;
}

স্থির শূন্য _rpmalloc_heap_release(শূন্য *heapptr, পূর্ণাঙ্ক first_class,
                                   পূর্ণাঙ্ক release_cache) {
  heap_t *heap = (heap_t *)heapptr;
  যদি (!heap)
    প্রত্যাগম;
  // Release thread cache spans back to global cache
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  যদি (release_cache || heap->finalize) {
#যদি ENABLE_THREAD_CACHE
    প্রতি (size_t iclass = 0; iclass < LARGE_CLASS_গুনতি; ++iclass) {
      span_cache_t *span_cache;
      যদি (!iclass)
        span_cache = &heap->span_cache;
      অন্যথা
        span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
      যদি (!span_cache->গুনতি)
        অনুবর্ত;
#যদি ENABLE_GLOBAL_CACHE
      যদি (heap->finalize) {
        প্রতি (size_t ispan = 0; ispan < span_cache->গুনতি; ++ispan)
          _rpmalloc_span_unmap(span_cache->span[ispan]);
      } অন্যথা {
        _rpmalloc_stat_add64(&heap->thread_to_global, span_cache->গুনতি *
                                                          (iclass + 1) *
                                                          _memory_span_size);
        _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                           span_cache->গুনতি);
        rpmalloc_global_cache_ঢোকাও_spans(span_cache->span, iclass + 1,
                                            span_cache->গুনতি);
      }
#অন্যথা
      প্রতি (size_t ispan = 0; ispan < span_cache->গুনতি; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
      span_cache->গুনতি = 0;
    }
#endif
  }

  যদি (get_thread_heap_raw() == heap)
    set_thread_heap(0);

#যদি ENABLE_STATISTICS
  atomic_decr32(&_memory_active_heaps);
  rpmalloc_assert(atomic_load32(&_memory_active_heaps) >= 0,
                  "Still active heaps during finalization");
#endif

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  যদি (get_thread_id() != _rpmalloc_main_thread_id) {
    যাবত্ (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
  }
  _rpmalloc_heap_orphan(heap, first_class);
  atomic_store32_release(&_memory_global_lock, 0);
}

স্থির শূন্য _rpmalloc_heap_release_raw(শূন্য *heapptr, পূর্ণাঙ্ক release_cache) {
  _rpmalloc_heap_release(heapptr, 0, release_cache);
}

স্থির শূন্য _rpmalloc_heap_release_raw_fc(শূন্য *heapptr) {
  _rpmalloc_heap_release_raw(heapptr, 1);
}

স্থির শূন্য _rpmalloc_heap_finalize(heap_t *heap) {
  যদি (heap->spans_reserved) {
    span_t *span = _rpmalloc_span_map(heap, heap->spans_reserved);
    _rpmalloc_span_unmap(span);
    heap->spans_reserved = 0;
  }

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  প্রতি (size_t iclass = 0; iclass < SIZE_CLASS_গুনতি; ++iclass) {
    যদি (heap->size_class[iclass].cache)
      _rpmalloc_span_unmap(heap->size_class[iclass].cache);
    heap->size_class[iclass].cache = 0;
    span_t *span = heap->size_class[iclass].partial_span;
    যাবত্ (span) {
      span_t *next = span->next;
      _rpmalloc_span_finalize(heap, iclass, span,
                              &heap->size_class[iclass].partial_span);
      span = next;
    }
    // If class still has a free list it must be a full span
    যদি (heap->size_class[iclass].free_list) {
      span_t *class_span =
          (span_t *)((uintptr_t)heap->size_class[iclass].free_list &
                     _memory_span_mask);
      span_t **list = 0;
#যদি RPMALLOC_FIRST_CLASS_HEAPS
      list = &heap->full_span[iclass];
#endif
      --heap->full_span_গুনতি;
      যদি (!_rpmalloc_span_finalize(heap, iclass, class_span, list)) {
        যদি (list)
          _rpmalloc_span_double_link_list_remove(list, class_span);
        _rpmalloc_span_double_link_list_add(
            &heap->size_class[iclass].partial_span, class_span);
      }
    }
  }

#যদি ENABLE_THREAD_CACHE
  প্রতি (size_t iclass = 0; iclass < LARGE_CLASS_গুনতি; ++iclass) {
    span_cache_t *span_cache;
    যদি (!iclass)
      span_cache = &heap->span_cache;
    অন্যথা
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    প্রতি (size_t ispan = 0; ispan < span_cache->গুনতি; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->গুনতি = 0;
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
স্থির শূন্য *free_list_বের_করো(শূন্য **list) {
  শূন্য *block = *list;
  *list = *((শূন্য **)block);
  প্রত্যাগম block;
}

//! Allocate a small/medium sized memory block from the given heap
স্থির শূন্য *_rpmalloc_allocate_from_heap_fallback(
    heap_t *heap, heap_size_class_t *heap_size_class, uint32_t class_idx) {
  span_t *span = heap_size_class->partial_span;
  rpmalloc_assume(heap != 0);
  যদি (EXPECTED(span != 0)) {
    rpmalloc_assert(span->block_গুনতি ==
                        _memory_size_class[span->size_class].block_গুনতি,
                    "Span block count corrupted");
    rpmalloc_assert(!_rpmalloc_span_is_fully_utilized(span),
                    "Internal failure");
    শূন্য *block;
    যদি (span->free_list) {
      // Span local free list is not empty, swap to size class free list
      block = free_list_বের_করো(&span->free_list);
      heap_size_class->free_list = span->free_list;
      span->free_list = 0;
    } অন্যথা {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      শূন্য *block_start = pointer_offset(
          span, SPAN_HEADER_SIZE +
                    ((size_t)span->free_list_limit * span->block_size));
      span->free_list_limit += free_list_partial_init(
          &heap_size_class->free_list, &block,
          (শূন্য *)((uintptr_t)block_start & ~(_memory_page_size - 1)),
          block_start, span->block_গুনতি - span->free_list_limit,
          span->block_size);
    }
    rpmalloc_assert(span->free_list_limit <= span->block_গুনতি,
                    "Span block count corrupted");
    span->used_গুনতি = span->free_list_limit;

    // Swap in deferred free list if present
    যদি (atomic_load_ptr(&span->free_list_deferred))
      _rpmalloc_span_extract_free_list_deferred(span);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    যদি (!_rpmalloc_span_is_fully_utilized(span))
      প্রত্যাগম block;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_span_double_link_list_বের_করো_head(&heap_size_class->partial_span,
                                             span);
#যদি RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_গুনতি;
    প্রত্যাগম block;
  }

  // Find a span in one of the cache levels
  span = _rpmalloc_heap_extract_new_span(heap, heap_size_class, 1, class_idx);
  যদি (EXPECTED(span != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    প্রত্যাগম _rpmalloc_span_initialize_new(heap, heap_size_class, span,
                                         class_idx);
  }

  প্রত্যাগম 0;
}

//! Allocate a small sized memory block from the given heap
স্থির শূন্য *_rpmalloc_allocate_small(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Small sizes have unique size classes
  ধ্রুব uint32_t class_idx =
      (uint32_t)((size + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_SHIFT);
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  যদি (EXPECTED(heap_size_class->free_list != 0))
    প্রত্যাগম free_list_বের_করো(&heap_size_class->free_list);
  প্রত্যাগম _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
স্থির শূন্য *_rpmalloc_allocate_medium(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ধ্রুব uint32_t base_idx =
      (uint32_t)(SMALL_CLASS_গুনতি +
                 ((size - (SMALL_SIZE_LIMIT + 1)) >> MEDIUM_GRANULARITY_SHIFT));
  ধ্রুব uint32_t class_idx = _memory_size_class[base_idx].class_idx;
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  যদি (EXPECTED(heap_size_class->free_list != 0))
    প্রত্যাগম free_list_বের_করো(&heap_size_class->free_list);
  প্রত্যাগম _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
স্থির শূন্য *_rpmalloc_allocate_large(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  size += SPAN_HEADER_SIZE;
  size_t span_গুনতি = size >> _memory_span_size_shift;
  যদি (size & (_memory_span_size - 1))
    ++span_গুনতি;

  // Find a span in one of the cache levels
  span_t *span =
      _rpmalloc_heap_extract_new_span(heap, 0, span_গুনতি, SIZE_CLASS_LARGE);
  যদি (!span)
    প্রত্যাগম span;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(span->span_গুনতি >= span_গুনতি, "Internal failure");
  span->size_class = SIZE_CLASS_LARGE;
  span->heap = heap;

#যদি RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_গুনতি;

  প্রত্যাগম pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a huge block by mapping memory pages directly
স্থির শূন্য *_rpmalloc_allocate_huge(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  size += SPAN_HEADER_SIZE;
  size_t অঙ্ক_pages = size >> _memory_page_size_shift;
  যদি (size & (_memory_page_size - 1))
    ++অঙ্ক_pages;
  size_t align_offset = 0;
  span_t *span =
      (span_t *)_rpmalloc_mmap(অঙ্ক_pages * _memory_page_size, &align_offset);
  যদি (!span)
    প্রত্যাগম span;

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_গুনতি = (uint32_t)অঙ্ক_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, অঙ্ক_pages, _huge_pages_peak);

#যদি RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_গুনতি;

  প্রত্যাগম pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a block of the given size
স্থির শূন্য *_rpmalloc_allocate(heap_t *heap, size_t size) {
  _rpmalloc_stat_add64(&_allocation_counter, 1);
  যদি (EXPECTED(size <= SMALL_SIZE_LIMIT))
    প্রত্যাগম _rpmalloc_allocate_small(heap, size);
  অন্যথা যদি (size <= _memory_medium_size_limit)
    প্রত্যাগম _rpmalloc_allocate_medium(heap, size);
  অন্যথা যদি (size <= LARGE_SIZE_LIMIT)
    প্রত্যাগম _rpmalloc_allocate_large(heap, size);
  প্রত্যাগম _rpmalloc_allocate_huge(heap, size);
}

স্থির শূন্য *_rpmalloc_aligned_allocate(heap_t *heap, size_t alignment,
                                        size_t size) {
  যদি (alignment <= SMALL_GRANULARITY)
    প্রত্যাগম _rpmalloc_allocate(heap, size);

#যদি ENABLE_VALIDATE_ARGS
  যদি ((size + alignment) < size) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
  যদি (alignment & (alignment - 1)) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#endif

  যদি ((alignment <= SPAN_HEADER_SIZE) &&
      ((size + SPAN_HEADER_SIZE) < _memory_medium_size_limit)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t multiple_size = size ? (size + (SPAN_HEADER_SIZE - 1)) &
                                      ~(uintptr_t)(SPAN_HEADER_SIZE - 1)
                                : SPAN_HEADER_SIZE;
    rpmalloc_assert(!(multiple_size % SPAN_HEADER_SIZE),
                    "Failed alignment calculation");
    যদি (multiple_size <= (size + alignment))
      প্রত্যাগম _rpmalloc_allocate(heap, multiple_size);
  }

  শূন্য *ptr = 0;
  size_t align_mask = alignment - 1;
  যদি (alignment <= _memory_page_size) {
    ptr = _rpmalloc_allocate(heap, size + alignment);
    যদি ((uintptr_t)ptr & align_mask) {
      ptr = (শূন্য *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);
      // Mark as having aligned blocks
      span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
      span->flags |= SPAN_FLAG_ALIGNED_BLOCKS;
    }
    প্রত্যাগম ptr;
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
  যদি (alignment & align_mask) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
  যদি (alignment >= _memory_span_size) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }

  size_t extra_pages = alignment / _memory_page_size;

  // Since each span has a header, we will at least need one extra memory page
  size_t অঙ্ক_pages = 1 + (size / _memory_page_size);
  যদি (size & (_memory_page_size - 1))
    ++অঙ্ক_pages;

  যদি (extra_pages > অঙ্ক_pages)
    অঙ্ক_pages = 1 + extra_pages;

  size_t original_pages = অঙ্ক_pages;
  size_t limit_pages = (_memory_span_size / _memory_page_size) * 2;
  যদি (limit_pages < (original_pages * 2))
    limit_pages = original_pages * 2;

  size_t mapped_size, align_offset;
  span_t *span;

retry:
  align_offset = 0;
  mapped_size = অঙ্ক_pages * _memory_page_size;

  span = (span_t *)_rpmalloc_mmap(mapped_size, &align_offset);
  যদি (!span) {
    errno = ENOMEM;
    প্রত্যাগম 0;
  }
  ptr = pointer_offset(span, SPAN_HEADER_SIZE);

  যদি ((uintptr_t)ptr & align_mask)
    ptr = (শূন্য *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);

  যদি (((size_t)pointer_diff(ptr, span) >= _memory_span_size) ||
      (pointer_offset(ptr, size) > pointer_offset(span, mapped_size)) ||
      (((uintptr_t)ptr & _memory_span_mask) != (uintptr_t)span)) {
    _rpmalloc_unmap(span, mapped_size, align_offset, mapped_size);
    ++অঙ্ক_pages;
    যদি (অঙ্ক_pages > limit_pages) {
      errno = EINVAL;
      প্রত্যাগম 0;
    }
    গচ্ছ retry;
  }

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_গুনতি = (uint32_t)অঙ্ক_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, অঙ্ক_pages, _huge_pages_peak);

#যদি RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_গুনতি;

  _rpmalloc_stat_add64(&_allocation_counter, 1);

  প্রত্যাগম ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
স্থির শূন্য _rpmalloc_deallocate_direct_small_or_medium(span_t *span,
                                                        শূন্য *block) {
  heap_t *heap = span->heap;
  rpmalloc_assert(heap->owner_thread == get_thread_id() ||
                      !heap->owner_thread || heap->finalize,
                  "Internal failure");
  // Add block to free list
  যদি (UNEXPECTED(_rpmalloc_span_is_fully_utilized(span))) {
    span->used_গুনতি = span->block_গুনতি;
#যদি RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                           span);
#endif
    _rpmalloc_span_double_link_list_add(
        &heap->size_class[span->size_class].partial_span, span);
    --heap->full_span_গুনতি;
  }
  *((শূন্য **)block) = span->free_list;
  --span->used_গুনতি;
  span->free_list = block;
  যদি (UNEXPECTED(span->used_গুনতি == span->list_size)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    যদি (span->used_গুনতি) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      শূন্য *free_list;
      কুরু {
        free_list = atomic_exchange_ptr_acquire(&span->free_list_deferred,
                                                INVALID_POINTER);
      } যাবত্ (free_list == INVALID_POINTER);
      atomic_store_ptr_release(&span->free_list_deferred, free_list);
    }
    _rpmalloc_span_double_link_list_remove(
        &heap->size_class[span->size_class].partial_span, span);
    _rpmalloc_span_release_to_cache(heap, span);
  }
}

স্থির শূন্য _rpmalloc_deallocate_defer_free_span(heap_t *heap, span_t *span) {
  যদি (span->size_class != SIZE_CLASS_HUGE)
    _rpmalloc_stat_inc(&heap->span_use[span->span_গুনতি - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  কুরু {
    span->free_list = (শূন্য *)atomic_load_ptr(&heap->span_free_deferred);
  } যাবত্ (!atomic_cas_ptr(&heap->span_free_deferred, span, span->free_list));
}

//! Put the block in the deferred free list of the owning span
স্থির শূন্য _rpmalloc_deallocate_defer_small_or_medium(span_t *span,
                                                       শূন্য *block) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  শূন্য *free_list;
  কুরু {
    free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } যাবত্ (free_list == INVALID_POINTER);
  *((শূন্য **)block) = free_list;
  uint32_t free_গুনতি = ++span->list_size;
  পূর্ণাঙ্ক all_deferred_free = (free_গুনতি == span->block_গুনতি);
  atomic_store_ptr_release(&span->free_list_deferred, block);
  যদি (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
  }
}

স্থির শূন্য _rpmalloc_deallocate_small_or_medium(span_t *span, শূন্য *p) {
  _rpmalloc_stat_inc_free(span->heap, span->size_class);
  যদি (span->flags & SPAN_FLAG_ALIGNED_BLOCKS) {
    // Realign pointer to block start
    শূন্য *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
    p = pointer_offset(p, -(int32_t)(block_offset % span->block_size));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#যদি RPMALLOC_FIRST_CLASS_HEAPS
  পূর্ণাঙ্ক defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#অন্যথা
  পূর্ণাঙ্ক defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  যদি (!defer)
    _rpmalloc_deallocate_direct_small_or_medium(span, p);
  অন্যথা
    _rpmalloc_deallocate_defer_small_or_medium(span, p);
}

//! Deallocate the given large memory block to the current heap
স্থির শূন্য _rpmalloc_deallocate_large(span_t *span) {
  rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#যদি RPMALLOC_FIRST_CLASS_HEAPS
  পূর্ণাঙ্ক defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#অন্যথা
  পূর্ণাঙ্ক defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  যদি (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    প্রত্যাগম;
  }
  rpmalloc_assert(span->heap->full_span_গুনতি, "Heap span counter corrupted");
  --span->heap->full_span_গুনতি;
#যদি RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif
#যদি ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = span->span_গুনতি - 1;
  atomic_decr32(&span->heap->span_use[idx].current);
#endif
  heap_t *heap = span->heap;
  rpmalloc_assert(heap, "No thread heap");
#যদি ENABLE_THREAD_CACHE
  ধ্রুব পূর্ণাঙ্ক set_as_reserved =
      ((span->span_গুনতি > 1) && (heap->span_cache.গুনতি == 0) &&
       !heap->finalize && !heap->spans_reserved);
#অন্যথা
  ধ্রুব পূর্ণাঙ্ক set_as_reserved =
      ((span->span_গুনতি > 1) && !heap->finalize && !heap->spans_reserved);
#endif
  যদি (set_as_reserved) {
    heap->span_reserve = span;
    heap->spans_reserved = span->span_গুনতি;
    যদি (span->flags & SPAN_FLAG_MASTER) {
      heap->span_reserve_master = span;
    } অন্যথা { // SPAN_FLAG_SUBSPAN
      span_t *master = (span_t *)pointer_offset(
          span,
          -(intptr_t)((size_t)span->offset_from_master * _memory_span_size));
      heap->span_reserve_master = master;
      rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->remaining_spans) >=
                          (int32_t)span->span_গুনতি,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&heap->span_use[idx].spans_to_reserved);
  } অন্যথা {
    // Insert into cache list
    rpmalloc_heap_cache_ঢোকাও(heap, span);
  }
}

//! Deallocate the given huge span
স্থির শূন্য _rpmalloc_deallocate_huge(span_t *span) {
  rpmalloc_assert(span->heap, "No span heap");
#যদি RPMALLOC_FIRST_CLASS_HEAPS
  পূর্ণাঙ্ক defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#অন্যথা
  পূর্ণাঙ্ক defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  যদি (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    প্রত্যাগম;
  }
  rpmalloc_assert(span->heap->full_span_গুনতি, "Heap span counter corrupted");
  --span->heap->full_span_গুনতি;
#যদি RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif

  // Oversized allocation, page count is stored in span_count
  size_t অঙ্ক_pages = span->span_গুনতি;
  _rpmalloc_unmap(span, অঙ্ক_pages * _memory_page_size, span->align_offset,
                  অঙ্ক_pages * _memory_page_size);
  _rpmalloc_stat_sub(&_huge_pages_current, অঙ্ক_pages);
}

//! Deallocate the given block
স্থির শূন্য _rpmalloc_deallocate(শূন্য *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  যদি (UNEXPECTED(!span))
    প্রত্যাগম;
  যদি (EXPECTED(span->size_class < SIZE_CLASS_গুনতি))
    _rpmalloc_deallocate_small_or_medium(span, p);
  অন্যথা যদি (span->size_class == SIZE_CLASS_LARGE)
    _rpmalloc_deallocate_large(span);
  অন্যথা
    _rpmalloc_deallocate_huge(span);
}

////////////
///
/// Reallocation entry points
///
//////

স্থির size_t _rpmalloc_usable_size(শূন্য *p);

//! Reallocate the given block to the given size
স্থির শূন্য *_rpmalloc_reallocate(heap_t *heap, শূন্য *p, size_t size,
                                  size_t oldsize, unsigned পূর্ণাঙ্ক flags) {
  যদি (p) {
    // Grab the span using guaranteed span alignment
    span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
    যদি (EXPECTED(span->size_class < SIZE_CLASS_গুনতি)) {
      // Small/medium sized block
      rpmalloc_assert(span->span_গুনতি == 1, "Span counter corrupted");
      শূন্য *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
      uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
      uint32_t block_idx = block_offset / span->block_size;
      শূন্য *block =
          pointer_offset(blocks_start, (size_t)block_idx * span->block_size);
      যদি (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)span->block_size - pointer_diff(p, block));
      যদি ((size_t)span->block_size >= size) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        যদি ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        প্রত্যাগম block;
      }
    } অন্যথা যদি (span->size_class == SIZE_CLASS_LARGE) {
      // Large block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t অঙ্ক_spans = total_size >> _memory_span_size_shift;
      যদি (total_size & (_memory_span_mask - 1))
        ++অঙ্ক_spans;
      size_t current_spans = span->span_গুনতি;
      শূন্য *block = pointer_offset(span, SPAN_HEADER_SIZE);
      যদি (!oldsize)
        oldsize = (current_spans * _memory_span_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      যদি ((current_spans >= অঙ্ক_spans) && (total_size >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        যদি ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        প্রত্যাগম block;
      }
    } অন্যথা {
      // Oversized block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t অঙ্ক_pages = total_size >> _memory_page_size_shift;
      যদি (total_size & (_memory_page_size - 1))
        ++অঙ্ক_pages;
      // Page count is stored in span_count
      size_t current_pages = span->span_গুনতি;
      শূন্য *block = pointer_offset(span, SPAN_HEADER_SIZE);
      যদি (!oldsize)
        oldsize = (current_pages * _memory_page_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      যদি ((current_pages >= অঙ্ক_pages) && (অঙ্ক_pages >= (current_pages / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        যদি ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        প্রত্যাগম block;
      }
    }
  } অন্যথা {
    oldsize = 0;
  }

  যদি (!!(flags & RPMALLOC_GROW_OR_FAIL))
    প্রত্যাগম 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t lower_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_size =
      (size > lower_bound) ? size : ((size > oldsize) ? lower_bound : size);
  শূন্য *block = _rpmalloc_allocate(heap, new_size);
  যদি (p && block) {
    যদি (!(flags & RPMALLOC_NO_PRESERVE))
      memcpy(block, p, oldsize < new_size ? oldsize : new_size);
    _rpmalloc_deallocate(p);
  }

  প্রত্যাগম block;
}

স্থির শূন্য *_rpmalloc_aligned_reallocate(heap_t *heap, শূন্য *ptr,
                                          size_t alignment, size_t size,
                                          size_t oldsize, unsigned পূর্ণাঙ্ক flags) {
  যদি (alignment <= SMALL_GRANULARITY)
    প্রত্যাগম _rpmalloc_reallocate(heap, ptr, size, oldsize, flags);

  পূর্ণাঙ্ক no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? _rpmalloc_usable_size(ptr) : 0);
  যদি ((usablesize >= size) && !((uintptr_t)ptr & (alignment - 1))) {
    যদি (no_alloc || (size >= (usablesize / 2)))
      প্রত্যাগম ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  শূন্য *block =
      (!no_alloc ? _rpmalloc_aligned_allocate(heap, alignment, size) : 0);
  যদি (EXPECTED(block != 0)) {
    যদি (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      যদি (!oldsize)
        oldsize = usablesize;
      memcpy(block, ptr, oldsize < size ? oldsize : size);
    }
    _rpmalloc_deallocate(ptr);
  }
  প্রত্যাগম block;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
স্থির size_t _rpmalloc_usable_size(শূন্য *p) {
  // Grab the span using guaranteed span alignment
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  যদি (span->size_class < SIZE_CLASS_গুনতি) {
    // Small/medium block
    শূন্য *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    প্রত্যাগম span->block_size -
           ((size_t)pointer_diff(p, blocks_start) % span->block_size);
  }
  যদি (span->size_class == SIZE_CLASS_LARGE) {
    // Large block
    size_t current_spans = span->span_গুনতি;
    প্রত্যাগম (current_spans * _memory_span_size) - (size_t)pointer_diff(p, span);
  }
  // Oversized block, page count is stored in span_count
  size_t current_pages = span->span_গুনতি;
  প্রত্যাগম (current_pages * _memory_page_size) - (size_t)pointer_diff(p, span);
}

//! Adjust and optimize the size class properties for the given class
স্থির শূন্য _rpmalloc_adjust_size_class(size_t iclass) {
  size_t block_size = _memory_size_class[iclass].block_size;
  size_t block_গুনতি = (_memory_span_size - SPAN_HEADER_SIZE) / block_size;

  _memory_size_class[iclass].block_গুনতি = (uint16_t)block_গুনতি;
  _memory_size_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  যদি (iclass >= SMALL_CLASS_গুনতি) {
    size_t prevclass = iclass;
    যাবত্ (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      যদি (_memory_size_class[prevclass].block_গুনতি ==
          _memory_size_class[iclass].block_গুনতি)
        _rpmalloc_memcpy_const(_memory_size_class + prevclass,
                               _memory_size_class + iclass,
                               পরিমাণ(_memory_size_class[iclass]));
      অন্যথা
        ভঙ্গ;
    }
  }
}

//! Initialize the allocator and setup global data
বাহ্য অন্তঃস্থ পূর্ণাঙ্ক rpmalloc_initialize(শূন্য) {
  যদি (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    প্রত্যাগম 0;
  }
  প্রত্যাগম rpmalloc_initialize_config(0);
}

পূর্ণাঙ্ক rpmalloc_initialize_config(ধ্রুব rpmalloc_config_t *config) {
  যদি (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    প্রত্যাগম 0;
  }
  _rpmalloc_initialized = 1;

  যদি (config)
    memcpy(&_memory_config, config, পরিমাণ(rpmalloc_config_t));
  অন্যথা
    _rpmalloc_memset_const(&_memory_config, 0, পরিমাণ(rpmalloc_config_t));

  যদি (!_memory_config.memory_map || !_memory_config.memory_unmap) {
    _memory_config.memory_map = _rpmalloc_mmap_os;
    _memory_config.memory_unmap = _rpmalloc_unmap_os;
  }

#যদি PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, পরিমাণ(system_info));
  GetSystemInfo(&system_info);
  _memory_map_granularity = system_info.dwAllocationGranularity;
#অন্যথা
  _memory_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#endif

#যদি RPMALLOC_CONFIGURABLE
  _memory_page_size = _memory_config.page_size;
#অন্যথা
  _memory_page_size = 0;
#endif
  _memory_huge_pages = 0;
  যদি (!_memory_page_size) {
#যদি PLATFORM_WINDOWS
    _memory_page_size = system_info.dwPageSize;
#অন্যথা
    _memory_page_size = _memory_map_granularity;
    যদি (_memory_config.enable_huge_pages) {
#যদি defined(__linux__)
      size_t huge_page_size = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      যদি (meminfo) {
        বর্ণ সারি_2[128];
        যাবত্ (!huge_page_size && fgets(সারি_2, পরিমাণ(সারি_2) - 1, meminfo)) {
          সারি_2[পরিমাণ(সারি_2) - 1] = 0;
          যদি (strstr(সারি_2, "Hugepagesize:"))
            huge_page_size = (size_t)strtol(সারি_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      যদি (huge_page_size) {
        _memory_huge_pages = 1;
        _memory_page_size = huge_page_size;
        _memory_map_granularity = huge_page_size;
      }
#নোচেদ্যদি defined(__FreeBSD__)
      পূর্ণাঙ্ক rc;
      size_t sz = পরিমাণ(rc);

      যদি (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, রিক্ত, 0) == 0 &&
          rc == 1) {
        স্থির size_t defsize = 2 * 1024 * 1024;
        পূর্ণাঙ্ক nsize = 0;
        size_t sizes[4] = {0};
        _memory_huge_pages = 1;
        _memory_page_size = defsize;
        যদি ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          প্রতি (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            যদি (defsize < csize) {
              _memory_page_size = csize;
              ভঙ্গ;
            }
          }
        }
        _memory_map_granularity = _memory_page_size;
      }
#নোচেদ্যদি defined(__APPLE__) || defined(__NetBSD__)
      _memory_huge_pages = 1;
      _memory_page_size = 2 * 1024 * 1024;
      _memory_map_granularity = _memory_page_size;
#endif
    }
#endif
  } অন্যথা {
    যদি (_memory_config.enable_huge_pages)
      _memory_huge_pages = 1;
  }

#যদি PLATFORM_WINDOWS
  যদি (_memory_config.enable_huge_pages) {
    HANDLE token = 0;
    size_t large_page_minimum = GetLargePageMinimum();
    যদি (large_page_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    যদি (token) {
      LUID luid;
      যদি (Lookup_Privilege_মান(0, SE_LOCK_MEMORY_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, পরিমাণ(token_privileges));
        token_privileges.Privilege_গুনতি = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        যদি (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          যদি (GetLastError() == ERROR_SUCCESS)
            _memory_huge_pages = 1;
        }
      }
      CloseHandle(token);
    }
    যদি (_memory_huge_pages) {
      যদি (large_page_minimum > _memory_page_size)
        _memory_page_size = large_page_minimum;
      যদি (large_page_minimum > _memory_map_granularity)
        _memory_map_granularity = large_page_minimum;
    }
  }
#endif

  size_t min_span_size = 256;
  size_t max_page_size;
#যদি UINTPTR_MAX > 0xFFFFFFFF
  max_page_size = 4096ULL * 1024ULL * 1024ULL;
#অন্যথা
  max_page_size = 4 * 1024 * 1024;
#endif
  যদি (_memory_page_size < min_span_size)
    _memory_page_size = min_span_size;
  যদি (_memory_page_size > max_page_size)
    _memory_page_size = max_page_size;
  _memory_page_size_shift = 0;
  size_t page_size_bit = _memory_page_size;
  যাবত্ (page_size_bit != 1) {
    ++_memory_page_size_shift;
    page_size_bit >>= 1;
  }
  _memory_page_size = ((size_t)1 << _memory_page_size_shift);

#যদি RPMALLOC_CONFIGURABLE
  যদি (!_memory_config.span_size) {
    _memory_span_size = _memory_default_span_size;
    _memory_span_size_shift = _memory_default_span_size_shift;
    _memory_span_mask = _memory_default_span_mask;
  } অন্যথা {
    size_t span_size = _memory_config.span_size;
    যদি (span_size > (256 * 1024))
      span_size = (256 * 1024);
    _memory_span_size = 4096;
    _memory_span_size_shift = 12;
    যাবত্ (_memory_span_size < span_size) {
      _memory_span_size <<= 1;
      ++_memory_span_size_shift;
    }
    _memory_span_mask = ~(uintptr_t)(_memory_span_size - 1);
  }
#endif

  memory_span_map_গুনতি =
      (_memory_config.span_map_গুনতি ? _memory_config.span_map_গুনতি
                                     : DEFAULT_SPAN_MAP_গুনতি);
  যদি ((_memory_span_size * memory_span_map_গুনতি) < _memory_page_size)
    memory_span_map_গুনতি = (_memory_page_size / _memory_span_size);
  যদি ((_memory_page_size >= _memory_span_size) &&
      ((memory_span_map_গুনতি * _memory_span_size) % _memory_page_size))
    memory_span_map_গুনতি = (_memory_page_size / _memory_span_size);
  memory_heap_reserve_গুনতি = (memory_span_map_গুনতি > DEFAULT_SPAN_MAP_গুনতি)
                                   ? DEFAULT_SPAN_MAP_গুনতি
                                   : memory_span_map_গুনতি;

  _memory_config.page_size = _memory_page_size;
  _memory_config.span_size = _memory_span_size;
  _memory_config.span_map_গুনতি = memory_span_map_গুনতি;
  _memory_config.enable_huge_pages = _memory_huge_pages;

#যদি ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  যদি (pthread_চাবি_create(&_memory_thread_heap, _rpmalloc_heap_release_raw_fc))
    প্রত্যাগম -1;
#endif
#যদি defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_চাবি = FlsAlloc(&_rpmalloc_thread_destructor);
#endif

  // Setup all small and medium size classes
  size_t iclass = 0;
  _memory_size_class[iclass].block_size = SMALL_GRANULARITY;
  _rpmalloc_adjust_size_class(iclass);
  প্রতি (iclass = 1; iclass < SMALL_CLASS_গুনতি; ++iclass) {
    size_t size = iclass * SMALL_GRANULARITY;
    _memory_size_class[iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  _memory_medium_size_limit = (_memory_span_size - SPAN_HEADER_SIZE) >> 1;
  যদি (_memory_medium_size_limit > MEDIUM_SIZE_LIMIT)
    _memory_medium_size_limit = MEDIUM_SIZE_LIMIT;
  প্রতি (iclass = 0; iclass < MEDIUM_CLASS_গুনতি; ++iclass) {
    size_t size = SMALL_SIZE_LIMIT + ((iclass + 1) * MEDIUM_GRANULARITY);
    যদি (size > _memory_medium_size_limit) {
      _memory_medium_size_limit =
          SMALL_SIZE_LIMIT + (iclass * MEDIUM_GRANULARITY);
      ভঙ্গ;
    }
    _memory_size_class[SMALL_CLASS_গুনতি + iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(SMALL_CLASS_গুনতি + iclass);
  }

  _memory_orphan_heaps = 0;
#যদি RPMALLOC_FIRST_CLASS_HEAPS
  _memory_first_class_orphan_heaps = 0;
#endif
#যদি ENABLE_STATISTICS
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
  memset(_memory_heaps, 0, পরিমাণ(_memory_heaps));
  atomic_store32_release(&_memory_global_lock, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  প্রত্যাগম 0;
}

//! Finalize the allocator
শূন্য rpmalloc_finalize(শূন্য) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  যদি (_memory_global_reserve) {
    atomic_add32(&_memory_global_reserve_master->remaining_spans,
                 -(int32_t)memory_global_reserve_গুনতি);
    _memory_global_reserve_master = 0;
    memory_global_reserve_গুনতি = 0;
    _memory_global_reserve = 0;
  }
  atomic_store32_release(&_memory_global_lock, 0);

  // Free all thread caches and fully free spans
  প্রতি (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    যাবত্ (heap) {
      heap_t *next_heap = heap->next_heap;
      heap->finalize = 1;
      _rpmalloc_heap_global_finalize(heap);
      heap = next_heap;
    }
  }

#যদি ENABLE_GLOBAL_CACHE
  // Free global caches
  প্রতি (size_t iclass = 0; iclass < LARGE_CLASS_গুনতি; ++iclass)
    _rpmalloc_global_cache_finalize(&_memory_span_cache[iclass]);
#endif

#যদি (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_চাবি_delete(_memory_thread_heap);
#endif
#যদি defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_চাবি);
  fls_চাবি = 0;
#endif
#যদি ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&_mapped_pages) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&_mapped_pages_os) == 0,
                  "Memory leak detected");
#endif

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
বাহ্য অন্তঃস্থ শূন্য rpmalloc_thread_initialize(শূন্য) {
  যদি (!get_thread_heap_raw()) {
    heap_t *heap = _rpmalloc_heap_allocate(0);
    যদি (heap) {
      _rpmalloc_stat_inc(&_memory_active_heaps);
      set_thread_heap(heap);
#যদি defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_Set_মান(fls_চাবি, heap);
#endif
    }
  }
}

//! Finalize thread, orphan heap
শূন্য rpmalloc_thread_finalize(পূর্ণাঙ্ক release_caches) {
  heap_t *heap = get_thread_heap_raw();
  যদি (heap)
    _rpmalloc_heap_release_raw(heap, release_caches);
  set_thread_heap(0);
#যদি defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_Set_মান(fls_চাবি, 0);
#endif
}

পূর্ণাঙ্ক rpmalloc_is_thread_initialized(শূন্য) {
  প্রত্যাগম (get_thread_heap_raw() != 0) ? 1 : 0;
}

ধ্রুব rpmalloc_config_t *rpmalloc_config(শূন্য) { প্রত্যাগম &_memory_config; }

// Extern interface

বাহ্য অন্তঃস্থ RPMALLOC_ALLOCATOR শূন্য *rpmalloc(size_t size) {
#যদি ENABLE_VALIDATE_ARGS
  যদি (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  প্রত্যাগম _rpmalloc_allocate(heap, size);
}

বাহ্য অন্তঃস্থ শূন্য rpfree(শূন্য *ptr) { _rpmalloc_deallocate(ptr); }

বাহ্য অন্তঃস্থ RPMALLOC_ALLOCATOR শূন্য *rpcalloc(size_t অঙ্ক_2, size_t size) {
  size_t total;
#যদি ENABLE_VALIDATE_ARGS
#যদি PLATFORM_WINDOWS
  পূর্ণাঙ্ক err = SizeTMult(অঙ্ক_2, size, &total);
  যদি ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#অন্যথা
  পূর্ণাঙ্ক err = __builtin_umull_overflow(অঙ্ক_2, size, &total);
  যদি (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#endif
#অন্যথা
  total = অঙ্ক_2 * size;
#endif
  heap_t *heap = get_thread_heap();
  শূন্য *block = _rpmalloc_allocate(heap, total);
  যদি (block)
    memset(block, 0, total);
  প্রত্যাগম block;
}

বাহ্য অন্তঃস্থ RPMALLOC_ALLOCATOR শূন্য *rprealloc(শূন্য *ptr, size_t size) {
#যদি ENABLE_VALIDATE_ARGS
  যদি (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    প্রত্যাগম ptr;
  }
#endif
  heap_t *heap = get_thread_heap();
  প্রত্যাগম _rpmalloc_reallocate(heap, ptr, size, 0, 0);
}

বাহ্য RPMALLOC_ALLOCATOR শূন্য *rpaligned_realloc(শূন্য *ptr, size_t alignment,
                                                  size_t size, size_t oldsize,
                                                  unsigned পূর্ণাঙ্ক flags) {
#যদি ENABLE_VALIDATE_ARGS
  যদি ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  প্রত্যাগম _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, oldsize,
                                      flags);
}

বাহ্য RPMALLOC_ALLOCATOR শূন্য *rpaligned_alloc(size_t alignment, size_t size) {
  heap_t *heap = get_thread_heap();
  প্রত্যাগম _rpmalloc_aligned_allocate(heap, alignment, size);
}

বাহ্য অন্তঃস্থ RPMALLOC_ALLOCATOR শূন্য *
rpaligned_calloc(size_t alignment, size_t অঙ্ক_2, size_t size) {
  size_t total;
#যদি ENABLE_VALIDATE_ARGS
#যদি PLATFORM_WINDOWS
  পূর্ণাঙ্ক err = SizeTMult(অঙ্ক_2, size, &total);
  যদি ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#অন্যথা
  পূর্ণাঙ্ক err = __builtin_umull_overflow(অঙ্ক_2, size, &total);
  যদি (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#endif
#অন্যথা
  total = অঙ্ক_2 * size;
#endif
  শূন্য *block = rpaligned_alloc(alignment, total);
  যদি (block)
    memset(block, 0, total);
  প্রত্যাগম block;
}

বাহ্য অন্তঃস্থ RPMALLOC_ALLOCATOR শূন্য *rpmemalign(size_t alignment,
                                                  size_t size) {
  প্রত্যাগম rpaligned_alloc(alignment, size);
}

বাহ্য অন্তঃস্থ পূর্ণাঙ্ক rpposix_memalign(শূন্য **memptr, size_t alignment,
                                   size_t size) {
  যদি (memptr)
    *memptr = rpaligned_alloc(alignment, size);
  অন্যথা
    প্রত্যাগম EINVAL;
  প্রত্যাগম *memptr ? 0 : ENOMEM;
}

বাহ্য অন্তঃস্থ size_t rpmalloc_usable_size(শূন্য *ptr) {
  প্রত্যাগম (ptr ? _rpmalloc_usable_size(ptr) : 0);
}

বাহ্য অন্তঃস্থ শূন্য rpmalloc_thread_collect(শূন্য) {}

শূন্য rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, পরিমাণ(rpmalloc_thread_statistics_t));
  heap_t *heap = get_thread_heap_raw();
  যদি (!heap)
    প্রত্যাগম;

  প্রতি (size_t iclass = 0; iclass < SIZE_CLASS_গুনতি; ++iclass) {
    size_class_t *size_class = _memory_size_class + iclass;
    span_t *span = heap->size_class[iclass].partial_span;
    যাবত্ (span) {
      size_t free_গুনতি = span->list_size;
      size_t block_গুনতি = size_class->block_গুনতি;
      যদি (span->free_list_limit < block_গুনতি)
        block_গুনতি = span->free_list_limit;
      free_গুনতি += (block_গুনতি - span->used_গুনতি);
      stats->sizecache += free_গুনতি * size_class->block_size;
      span = span->next;
    }
  }

#যদি ENABLE_THREAD_CACHE
  প্রতি (size_t iclass = 0; iclass < LARGE_CLASS_গুনতি; ++iclass) {
    span_cache_t *span_cache;
    যদি (!iclass)
      span_cache = &heap->span_cache;
    অন্যথা
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    stats->spancache += span_cache->গুনতি * (iclass + 1) * _memory_span_size;
  }
#endif

  span_t *deferred = (span_t *)atomic_load_ptr(&heap->span_free_deferred);
  যাবত্ (deferred) {
    যদি (deferred->size_class != SIZE_CLASS_HUGE)
      stats->spancache += (size_t)deferred->span_গুনতি * _memory_span_size;
    deferred = (span_t *)deferred->free_list;
  }

#যদি ENABLE_STATISTICS
  stats->thread_to_global = (size_t)atomic_load64(&heap->thread_to_global);
  stats->global_to_thread = (size_t)atomic_load64(&heap->global_to_thread);

  প্রতি (size_t iclass = 0; iclass < LARGE_CLASS_গুনতি; ++iclass) {
    stats->span_use[iclass].current =
        (size_t)atomic_load32(&heap->span_use[iclass].current);
    stats->span_use[iclass].peak =
        (size_t)atomic_load32(&heap->span_use[iclass].উপরের);
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
  প্রতি (size_t iclass = 0; iclass < SIZE_CLASS_গুনতি; ++iclass) {
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

শূন্য rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, পরিমাণ(rpmalloc_global_statistics_t));
#যদি ENABLE_STATISTICS
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
#যদি ENABLE_GLOBAL_CACHE
  প্রতি (size_t iclass = 0; iclass < LARGE_CLASS_গুনতি; ++iclass) {
    global_cache_t *cache = &_memory_span_cache[iclass];
    যাবত্ (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();
    uint32_t গুনতি = cache->গুনতি;
#যদি ENABLE_UNLIMITED_CACHE
    span_t *current_span = cache->overflow;
    যাবত্ (current_span) {
      ++গুনতি;
      current_span = current_span->next;
    }
#endif
    atomic_store32_release(&cache->lock, 0);
    stats->cached += গুনতি * (iclass + 1) * _memory_span_size;
  }
#endif
}

#যদি ENABLE_STATISTICS

স্থির শূন্য _memory_heap_dump_statistics(heap_t *heap, শূন্য *file) {
  fprintf(file, "Heap %d stats:\n", heap->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  প্রতি (size_t iclass = 0; iclass < SIZE_CLASS_গুনতি; ++iclass) {
    যদি (!atomic_load32(&heap->size_class_use[iclass].alloc_total))
      অনুবর্ত;
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
        _memory_size_class[iclass].block_গুনতি,
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
  প্রতি (size_t iclass = 0; iclass < LARGE_CLASS_গুনতি; ++iclass) {
    যদি (!atomic_load32(&heap->span_use[iclass].উপরের) &&
        !atomic_load32(&heap->span_use[iclass].spans_map_calls))
      অনুবর্ত;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&heap->span_use[iclass].current),
        atomic_load32(&heap->span_use[iclass].উপরের),
        atomic_load32(&heap->span_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&heap->span_use[iclass].উপরের) *
         (size_t)_memory_span_size * (iclass + 1)) /
            (size_t)(1024 * 1024),
#যদি ENABLE_THREAD_CACHE
        (unsigned পূর্ণাঙ্ক)(!iclass ? heap->span_cache.গুনতি
                               : heap->span_large_cache[iclass - 1].গুনতি),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_to_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_from_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
#অন্যথা
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
  fprintf(file, "Full spans: %zu\n", heap->full_span_গুনতি);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&heap->thread_to_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&heap->global_to_thread) / (size_t)(1024 * 1024));
}

#endif

শূন্য rpmalloc_dump_statistics(শূন্য *file) {
#যদি ENABLE_STATISTICS
  প্রতি (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    যাবত্ (heap) {
      পূর্ণাঙ্ক need_dump = 0;
      প্রতি (size_t iclass = 0; !need_dump && (iclass < SIZE_CLASS_গুনতি);
           ++iclass) {
        যদি (!atomic_load32(&heap->size_class_use[iclass].alloc_total)) {
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].free_total),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          অনুবর্ত;
        }
        need_dump = 1;
      }
      প্রতি (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_গুনতি);
           ++iclass) {
        যদি (!atomic_load32(&heap->span_use[iclass].উপরের) &&
            !atomic_load32(&heap->span_use[iclass].spans_map_calls))
          অনুবর্ত;
        need_dump = 1;
      }
      যদি (need_dump)
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

#যদি ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  প্রতি (size_t iclass = 0; iclass < LARGE_CLASS_গুনতি; ++iclass) {
    global_cache_t *cache = _memory_span_cache + iclass;
    size_t global_cache = (size_t)cache->গুনতি * iclass * _memory_span_size;

    size_t global_overflow_cache = 0;
    span_t *span = cache->overflow;
    যাবত্ (span) {
      global_overflow_cache += iclass * _memory_span_size;
      span = span->next;
    }
    যদি (global_cache || global_overflow_cache || cache->ঢোকাও_গুনতি ||
        cache->extract_গুনতি)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_overflow_cache / (size_t)(1024 * 1024),
              cache->ঢোকাও_গুনতি, cache->extract_গুনতি);
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
#যদি 0
	int64_t allocated = atomic_load64(&_allocation_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#endif
#endif
  (শূন্য)পরিমাণ(file);
}

#যদি RPMALLOC_FIRST_CLASS_HEAPS

বাহ্য অন্তঃস্থ rpmalloc_heap_t *rpmalloc_heap_acquire(শূন্য) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  heap_t *heap = _rpmalloc_heap_allocate(1);
  rpmalloc_assume(heap != রিক্ত);
  heap->owner_thread = 0;
  _rpmalloc_stat_inc(&_memory_active_heaps);
  প্রত্যাগম heap;
}

বাহ্য অন্তঃস্থ শূন্য rpmalloc_heap_release(rpmalloc_heap_t *heap) {
  যদি (heap)
    _rpmalloc_heap_release(heap, 1, 1);
}

বাহ্য অন্তঃস্থ RPMALLOC_ALLOCATOR শূন্য *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, size_t size) {
#যদি ENABLE_VALIDATE_ARGS
  যদি (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#endif
  প্রত্যাগম _rpmalloc_allocate(heap, size);
}

বাহ্য অন্তঃস্থ RPMALLOC_ALLOCATOR শূন্য *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, size_t alignment,
                            size_t size) {
#যদি ENABLE_VALIDATE_ARGS
  যদি (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#endif
  প্রত্যাগম _rpmalloc_aligned_allocate(heap, alignment, size);
}

বাহ্য অন্তঃস্থ RPMALLOC_ALLOCATOR শূন্য *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, size_t অঙ্ক_2, size_t size) {
  প্রত্যাগম rpmalloc_heap_aligned_calloc(heap, 0, অঙ্ক_2, size);
}

বাহ্য অন্তঃস্থ RPMALLOC_ALLOCATOR শূন্য *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, size_t alignment,
                             size_t অঙ্ক_2, size_t size) {
  size_t total;
#যদি ENABLE_VALIDATE_ARGS
#যদি PLATFORM_WINDOWS
  পূর্ণাঙ্ক err = SizeTMult(অঙ্ক_2, size, &total);
  যদি ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#অন্যথা
  পূর্ণাঙ্ক err = __builtin_umull_overflow(অঙ্ক_2, size, &total);
  যদি (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#endif
#অন্যথা
  total = অঙ্ক_2 * size;
#endif
  শূন্য *block = _rpmalloc_aligned_allocate(heap, alignment, total);
  যদি (block)
    memset(block, 0, total);
  প্রত্যাগম block;
}

বাহ্য অন্তঃস্থ RPMALLOC_ALLOCATOR শূন্য *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, শূন্য *ptr, size_t size,
                      unsigned পূর্ণাঙ্ক flags) {
#যদি ENABLE_VALIDATE_ARGS
  যদি (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    প্রত্যাগম ptr;
  }
#endif
  প্রত্যাগম _rpmalloc_reallocate(heap, ptr, size, 0, flags);
}

বাহ্য অন্তঃস্থ RPMALLOC_ALLOCATOR শূন্য *
rpmalloc_heap_aligned_realloc(rpmalloc_heap_t *heap, শূন্য *ptr,
                              size_t alignment, size_t size,
                              unsigned পূর্ণাঙ্ক flags) {
#যদি ENABLE_VALIDATE_ARGS
  যদি ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#endif
  প্রত্যাগম _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, 0, flags);
}

বাহ্য অন্তঃস্থ শূন্য rpmalloc_heap_free(rpmalloc_heap_t *heap, শূন্য *ptr) {
  (শূন্য)পরিমাণ(heap);
  _rpmalloc_deallocate(ptr);
}

বাহ্য অন্তঃস্থ শূন্য rpmalloc_heap_free_all(rpmalloc_heap_t *heap) {
  span_t *span;
  span_t *next_span;

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  প্রতি (size_t iclass = 0; iclass < SIZE_CLASS_গুনতি; ++iclass) {
    span = heap->size_class[iclass].partial_span;
    যাবত্ (span) {
      next_span = span->next;
      rpmalloc_heap_cache_ঢোকাও(heap, span);
      span = next_span;
    }
    heap->size_class[iclass].partial_span = 0;
    span = heap->full_span[iclass];
    যাবত্ (span) {
      next_span = span->next;
      rpmalloc_heap_cache_ঢোকাও(heap, span);
      span = next_span;
    }

    span = heap->size_class[iclass].cache;
    যদি (span)
      rpmalloc_heap_cache_ঢোকাও(heap, span);
    heap->size_class[iclass].cache = 0;
  }
  memset(heap->size_class, 0, পরিমাণ(heap->size_class));
  memset(heap->full_span, 0, পরিমাণ(heap->full_span));

  span = heap->large_huge_span;
  যাবত্ (span) {
    next_span = span->next;
    যদি (UNEXPECTED(span->size_class == SIZE_CLASS_HUGE))
      _rpmalloc_deallocate_huge(span);
    অন্যথা
      rpmalloc_heap_cache_ঢোকাও(heap, span);
    span = next_span;
  }
  heap->large_huge_span = 0;
  heap->full_span_গুনতি = 0;

#যদি ENABLE_THREAD_CACHE
  প্রতি (size_t iclass = 0; iclass < LARGE_CLASS_গুনতি; ++iclass) {
    span_cache_t *span_cache;
    যদি (!iclass)
      span_cache = &heap->span_cache;
    অন্যথা
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    যদি (!span_cache->গুনতি)
      অনুবর্ত;
#যদি ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&heap->thread_to_global,
                         span_cache->গুনতি * (iclass + 1) * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                       span_cache->গুনতি);
    rpmalloc_global_cache_ঢোকাও_spans(span_cache->span, iclass + 1,
                                        span_cache->গুনতি);
#অন্যথা
    প্রতি (size_t ispan = 0; ispan < span_cache->গুনতি; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
    span_cache->গুনতি = 0;
  }
#endif

#যদি ENABLE_STATISTICS
  প্রতি (size_t iclass = 0; iclass < SIZE_CLASS_গুনতি; ++iclass) {
    atomic_store32(&heap->size_class_use[iclass].alloc_current, 0);
    atomic_store32(&heap->size_class_use[iclass].spans_current, 0);
  }
  প্রতি (size_t iclass = 0; iclass < LARGE_CLASS_গুনতি; ++iclass) {
    atomic_store32(&heap->span_use[iclass].current, 0);
  }
#endif
}

বাহ্য অন্তঃস্থ শূন্য rpmalloc_heap_thread_set_current(rpmalloc_heap_t *heap) {
  heap_t *prev_heap = get_thread_heap_raw();
  যদি (prev_heap != heap) {
    set_thread_heap(heap);
    যদি (prev_heap)
      rpmalloc_heap_release(prev_heap);
  }
}

বাহ্য অন্তঃস্থ rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(শূন্য *ptr) {
  // Grab the span, and then the heap from the span
  span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
  যদি (span) {
    প্রত্যাগম span->heap;
  }
  প্রত্যাগম 0;
}

#endif

#যদি ENABLE_PRELOAD || ENABLE_OVERRIDE

#অন্তর্ভাব "malloc.c"

#endif

শূন্য rpmalloc_linker_reference(শূন্য) { (শূন্য)পরিমাণ(_rpmalloc_initialized); }
