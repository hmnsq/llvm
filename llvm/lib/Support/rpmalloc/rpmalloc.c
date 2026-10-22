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

#شامل "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#اگر defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wunused-function"
#اگر __has_warning("-Wreserved-identifier")
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
#اگر __has_warning("-Wstatic-in-inline")
#pragma clang diagnostic ignored "-Wstatic-in-inline"
#endif
#وگرنهاگر defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#اگر !defined(__has_builtin)
#define __has_builtin(b) 0
#endif

#اگر defined(__GNUC__) || defined(__clang__)

#اگر __has_builtin(__builtin_memcpy_inline)
#define _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#وگرنه
#define _rpmalloc_memcpy_const(x, y, s)                                        \
  انجامبده {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memcpy(x, y, s);                                                           \
  } وقتی (0)
#endif

#اگر __has_builtin(__builtin_memset_inline)
#define _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#وگرنه
#define _rpmalloc_memset_const(x, y, s)                                        \
  انجامبده {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } وقتی (0)
#endif
#وگرنه
#define _rpmalloc_memcpy_const(x, y, s) memcpy(x, y, s)
#define _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#endif

#اگر __has_builtin(__builtin_assume)
#define rpmalloc_assume(cond) __builtin_assume(cond)
#وگرنهاگر defined(__GNUC__)
#define rpmalloc_assume(cond)                                                  \
  انجامبده {                                                                         \
    اگر (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } وقتی (0)
#وگرنهاگر defined(_MSC_VER)
#define rpmalloc_assume(cond) __assume(cond)
#وگرنه
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
#ifndef DEFAULT_SPAN_MAP_شمار
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#define DEFAULT_SPAN_MAP_شمار 64
#endif
#ifndef GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#define GLOBAL_CACHE_MULTIPLIER 8
#endif

#اگر DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#error Must use global cache اگر unmap is disabled
#endif

#اگر DISABLE_UNMAP
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 1
#endif

#اگر !ENABLE_GLOBAL_CACHE
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 0
#endif

#اگر !ENABLE_THREAD_CACHE
#undef ENABLE_ADAPTIVE_THREAD_CACHE
#define ENABLE_ADAPTIVE_THREAD_CACHE 0
#endif

#اگر defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#define PLATFORM_POSIX 0
#وگرنه
#define PLATFORM_WINDOWS 0
#define PLATFORM_POSIX 1
#endif

/// Platform and arch specifics
#اگر defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 5105)
#ifndef FORCEINLINE
#define FORCEINLINE درونخطی __forceinline
#endif
#define _Static_assert static_assert
#وگرنه
#ifndef FORCEINLINE
#define FORCEINLINE درونخطی __attribute__((__always_inline__))
#endif
#endif
#اگر PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#شامل <windows.h>
#اگر ENABLE_VALIDATE_ARGS
#شامل <intsafe.h>
#endif
#وگرنه
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <time.h>
#شامل <unistd.h>
#اگر defined(__linux__) || defined(__ANDROID__)
#شامل <sys/prctl.h>
#اگر !defined(PR_SET_VMA)
#define PR_SET_VMA 0x53564d41
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif
#اگر defined(__APPLE__)
#شامل <TargetConditionals.h>
#اگر !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#شامل <mach/mach_vm.h>
#شامل <mach/vm_statistics.h>
#endif
#شامل <pthread.h>
#endif
#اگر defined(__HAIKU__) || defined(__TINYC__)
#شامل <pthread.h>
#endif
#endif

#شامل <errno.h>
#شامل <stdint.h>
#شامل <string.h>

#اگر defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#شامل <fibersapi.h>
ایستا DWORD fls_کلید;
#endif

#اگر PLATFORM_POSIX
#شامل <sched.h>
#شامل <sys/mman.h>
#ifdef __FreeBSD__
#شامل <sys/sysctl.h>
#define MAP_HUGETLB MAP_ALIGNED_SUPER
#ifndef PROT_MAX
#define PROT_MAX(f) 0
#endif
#وگرنه
#define PROT_MAX(f) 0
#endif
#ifdef __sun
بیرونی صحیح madvise(caddr_t, size_t, صحیح);
#endif
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif
#endif
#شامل <errno.h>

#اگر ENABLE_ASSERTS
#undef NDEBUG
#اگر defined(_MSC_VER) && !defined(_DEBUG)
#define _DEBUG
#endif
#شامل <assert.h>
#define RPMALLOC_TOSTRING_M(x) #x
#define RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#define rpmalloc_assert(truth, message)                                        \
  انجامبده {                                                                         \
    اگر (!(truth)) {                                                            \
      اگر (_memory_config.error_callback) {                                     \
        _memory_config.error_callback(message " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } وگرنه {                                                                 \
        أكد((truth) && message);                                            \
      }                                                                        \
    }                                                                          \
  } وقتی (0)
#وگرنه
#define rpmalloc_assert(truth, message)                                        \
  انجامبده {                                                                         \
  } وقتی (0)
#endif
#اگر ENABLE_STATISTICS
#شامل <stdio.h>
#endif

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#اگر defined(_MSC_VER) && !defined(__clang__)

تعریفنوع فرار long atomic32_t;
تعریفنوع فرار long long atomic64_t;
تعریفنوع فرار تهی *atomicptr_t;

ایستا FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  بازگشت (int32_t)InterlockedOr(src, 0);
}
ایستا FORCEINLINE تهی atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ایستا FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  بازگشت (int32_t)InterlockedIncrement(val);
}
ایستا FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  بازگشت (int32_t)InterlockedDecrement(val);
}
ایستا FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  بازگشت (int32_t)InterlockedExchangeAdd(val, add) + add;
}
ایستا FORCEINLINE صحیح atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  بازگشت (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
ایستا FORCEINLINE تهی atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ایستا FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  بازگشت (int64_t)InterlockedOr64(src, 0);
}
ایستا FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  بازگشت (int64_t)InterlockedExchangeAdd64(val, add) + add;
}
ایستا FORCEINLINE تهی *atomic_load_ptr(atomicptr_t *src) {
  بازگشت InterlockedCompareExchangePointer(src, 0, 0);
}
ایستا FORCEINLINE تهی atomic_store_ptr(atomicptr_t *dst, تهی *val) {
  InterlockedExchangePointer(dst, val);
}
ایستا FORCEINLINE تهی atomic_store_ptr_release(atomicptr_t *dst, تهی *val) {
  InterlockedExchangePointer(dst, val);
}
ایستا FORCEINLINE تهی *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     تهی *val) {
  بازگشت (تهی *)InterlockedExchangePointer((تهی *فرار *)dst, val);
}
ایستا FORCEINLINE صحیح atomic_cas_ptr(atomicptr_t *dst, تهی *val, تهی *ref) {
  بازگشت (InterlockedCompareExchangePointer((تهی *فرار *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#define EXPECTED(x) (x)
#define UNEXPECTED(x) (x)

#وگرنه

#شامل <stdatomic.h>

تعریفنوع فرار _Atomic(int32_t) atomic32_t;
تعریفنوع فرار _Atomic(int64_t) atomic64_t;
تعریفنوع فرار _Atomic(تهی *) atomicptr_t;

ایستا FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  بازگشت atomic_load_explicit(src, memory_order_relaxed);
}
ایستا FORCEINLINE تهی atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ایستا FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  بازگشت atomic_fetch_add_explicit(val, 1, memory_order_relaxed) + 1;
}
ایستا FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  بازگشت atomic_fetch_add_explicit(val, -1, memory_order_relaxed) - 1;
}
ایستا FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  بازگشت atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
ایستا FORCEINLINE صحیح atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  بازگشت atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
ایستا FORCEINLINE تهی atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ایستا FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  بازگشت atomic_load_explicit(val, memory_order_relaxed);
}
ایستا FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  بازگشت atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
ایستا FORCEINLINE تهی *atomic_load_ptr(atomicptr_t *src) {
  بازگشت atomic_load_explicit(src, memory_order_relaxed);
}
ایستا FORCEINLINE تهی atomic_store_ptr(atomicptr_t *dst, تهی *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ایستا FORCEINLINE تهی atomic_store_ptr_release(atomicptr_t *dst, تهی *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ایستا FORCEINLINE تهی *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     تهی *val) {
  بازگشت atomic_exchange_explicit(dst, val, memory_order_acquire);
}
ایستا FORCEINLINE صحیح atomic_cas_ptr(atomicptr_t *dst, تهی *val, تهی *ref) {
  بازگشت atomic_compare_exchange_weak_explicit(
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

#اگر ENABLE_STATISTICS
#define _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#define _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#define _rpmalloc_stat_add(counter, مقدار_2)                                     \
  atomic_add32(counter, (int32_t)(مقدار_2))
#define _rpmalloc_stat_add64(counter, مقدار_2)                                   \
  atomic_add64(counter, (int64_t)(مقدار_2))
#define _rpmalloc_stat_add_peak(counter, مقدار_2, peak)                          \
  انجامبده {                                                                         \
    int32_t cur_شمار = atomic_add32(counter, (int32_t)(مقدار_2));              \
    اگر (cur_شمار > (peak))                                                   \
      peak = cur_شمار;                                                       \
  } وقتی (0)
#define _rpmalloc_stat_sub(counter, مقدار_2)                                     \
  atomic_add32(counter, -(int32_t)(مقدار_2))
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  انجامبده {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&heap->size_class_use[class_idx].alloc_current);         \
    اگر (alloc_current > heap->size_class_use[class_idx].alloc_peak)            \
      heap->size_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&heap->size_class_use[class_idx].alloc_total);               \
  } وقتی (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  انجامبده {                                                                         \
    atomic_decr32(&heap->size_class_use[class_idx].alloc_current);             \
    atomic_incr32(&heap->size_class_use[class_idx].free_total);                \
  } وقتی (0)
#وگرنه
#define _rpmalloc_stat_inc(counter)                                            \
  انجامبده {                                                                         \
  } وقتی (0)
#define _rpmalloc_stat_dec(counter)                                            \
  انجامبده {                                                                         \
  } وقتی (0)
#define _rpmalloc_stat_add(counter, مقدار_2)                                     \
  انجامبده {                                                                         \
  } وقتی (0)
#define _rpmalloc_stat_add64(counter, مقدار_2)                                   \
  انجامبده {                                                                         \
  } وقتی (0)
#define _rpmalloc_stat_add_peak(counter, مقدار_2, peak)                          \
  انجامبده {                                                                         \
  } وقتی (0)
#define _rpmalloc_stat_sub(counter, مقدار_2)                                     \
  انجامبده {                                                                         \
  } وقتی (0)
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  انجامبده {                                                                         \
  } وقتی (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  انجامبده {                                                                         \
  } وقتی (0)
#endif

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#define SMALL_GRANULARITY 16
//! Small granularity shift count
#define SMALL_GRANULARITY_SHIFT 4
//! Number of small block size classes
#define SMALL_CLASS_شمار 65
//! Maximum size of a small block
#define SMALL_SIZE_LIMIT (SMALL_GRANULARITY * (SMALL_CLASS_شمار - 1))
//! Granularity of a medium allocation block
#define MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#define MEDIUM_GRANULARITY_SHIFT 9
//! Number of medium block size classes
#define MEDIUM_CLASS_شمار 61
//! Total number of small + medium size classes
#define SIZE_CLASS_شمار (SMALL_CLASS_شمار + MEDIUM_CLASS_شمار)
//! Number of large block size classes
#define LARGE_CLASS_شمار 63
//! Maximum size of a medium block
#define MEDIUM_SIZE_LIMIT                                                      \
  (SMALL_SIZE_LIMIT + (MEDIUM_GRANULARITY * MEDIUM_CLASS_شمار))
//! Maximum size of a large block
#define LARGE_SIZE_LIMIT                                                       \
  ((LARGE_CLASS_شمار * _memory_span_size) - SPAN_HEADER_SIZE)
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

#اگر ENABLE_VALIDATE_ARGS
//! Maximum allocation size to avoid integer overflow
#undef MAX_ALLOC_SIZE
#define MAX_ALLOC_SIZE (((size_t) - 1) - _memory_span_size)
#endif

#define pointer_offset(ptr, ofs) (تهی *)((نویسه *)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff(first, second)                                            \
  (ptrdiff_t)((ثابت نویسه *)(first) - (ثابت نویسه *)(second))

#define INVALID_POINTER ((تهی *)((uintptr_t) - 1))

#define SIZE_CLASS_LARGE SIZE_CLASS_شمار
#define SIZE_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
تعریفنوع ساختار heap_t heap_t;
//! Span of memory pages
تعریفنوع ساختار span_t span_t;
//! Span list
تعریفنوع ساختار span_list_t span_list_t;
//! Span active data
تعریفنوع ساختار span_active_t span_active_t;
//! Size class definition
تعریفنوع ساختار size_class_t size_class_t;
//! Global cache
تعریفنوع ساختار global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#define SPAN_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#define SPAN_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#define SPAN_FLAG_ALIGNED_BLOCKS 4U
//! Flag indicating an unmapped master span
#define SPAN_FLAG_UNMAPPED_MASTER 8U

#اگر ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
ساختار span_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t بالا;
#اگر ENABLE_STATISTICS
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
تعریفنوع ساختار span_use_t span_use_t;
#endif

#اگر ENABLE_STATISTICS
ساختار size_class_use_t {
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
تعریفنوع ساختار size_class_use_t size_class_use_t;
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
ساختار span_t {
  //! Free list
  تهی *free_list;
  //! Total block count of size class
  uint32_t block_شمار;
  //! Size class
  uint32_t size_class;
  //! Index of last block initialized in free list
  uint32_t free_list_limit;
  //! Number of used blocks remaining when in partial state
  uint32_t used_شمار;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_size;
  //! Size of a block
  uint32_t block_size;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t span_شمار;
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
_Static_assert(اندازه(span_t) <= SPAN_HEADER_SIZE, "span size mismatch");

ساختار span_cache_t {
  size_t شمار;
  span_t *span[MAX_THREAD_SPAN_CACHE];
};
تعریفنوع ساختار span_cache_t span_cache_t;

ساختار span_large_cache_t {
  size_t شمار;
  span_t *span[MAX_THREAD_SPAN_LARGE_CACHE];
};
تعریفنوع ساختار span_large_cache_t span_large_cache_t;

ساختار heap_size_class_t {
  //! Free list of active span
  تهی *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  span_t *partial_span;
  //! Early level cache of fully free spans
  span_t *cache;
};
تعریفنوع ساختار heap_size_class_t heap_size_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
ساختار heap_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  heap_size_class_t size_class[SIZE_CLASS_شمار];
#اگر ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  span_cache_t span_cache;
#endif
  //! List of deferred free spans (single linked list)
  atomicptr_t span_free_deferred;
  //! Number of full spans
  size_t full_span_شمار;
  //! Mapped but unused spans
  span_t *span_reserve;
  //! Master span for mapped but unused spans
  span_t *span_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t child_شمار;
  //! Next heap in id list
  heap_t *next_heap;
  //! Next heap in orphan list
  heap_t *next_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  صحیح finalize;
  //! Master heap owning the memory pages
  heap_t *master_heap;
#اگر ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  span_large_cache_t span_large_cache[LARGE_CLASS_شمار - 1];
#endif
#اگر RPMALLOC_FIRST_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  span_t *full_span[SIZE_CLASS_شمار];
  //! Double linked list of large and huge spans allocated by this heap
  span_t *large_huge_span;
#endif
#اگر ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  span_use_t span_use[LARGE_CLASS_شمار];
#endif
#اگر ENABLE_STATISTICS
  //! Allocation stats per size class
  size_class_use_t size_class_use[SIZE_CLASS_شمار + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_to_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_to_thread;
#endif
};

// Size class for defining a block size bucket
ساختار size_class_t {
  //! Size of blocks in this class
  uint32_t block_size;
  //! Number of blocks in each chunk
  uint16_t block_شمار;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(اندازه(size_class_t) == 8, "Size class size mismatch");

ساختار global_cache_t {
  //! Cache lock
  atomic32_t lock;
  //! Cache count
  uint32_t شمار;
#اگر ENABLE_STATISTICS
  //! Insert count
  size_t درج_شمار;
  //! Extract count
  size_t extract_شمار;
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
ایستا صحیح _rpmalloc_initialized;
//! Main thread ID
ایستا uintptr_t _rpmalloc_main_thread_id;
//! Configuration
ایستا rpmalloc_config_t _memory_config;
//! Memory page size
ایستا size_t _memory_page_size;
//! Shift to divide by page size
ایستا size_t _memory_page_size_shift;
//! Granularity at which memory pages are mapped by OS
ایستا size_t _memory_map_granularity;
#اگر RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
ایستا size_t _memory_span_size;
//! Shift to divide by span size
ایستا size_t _memory_span_size_shift;
//! Mask to get to start of a memory span
ایستا uintptr_t _memory_span_mask;
#وگرنه
//! Hardwired span size
#define _memory_span_size _memory_default_span_size
#define _memory_span_size_shift _memory_default_span_size_shift
#define _memory_span_mask _memory_default_span_mask
#endif
//! Number of spans to map in each map call
ایستا size_t memory_span_map_شمار;
//! Number of spans to keep reserved in each heap
ایستا size_t memory_heap_reserve_شمار;
//! Global size classes
ایستا size_class_t _memory_size_class[SIZE_CLASS_شمار];
//! Run-time size limit of medium blocks
ایستا size_t _memory_medium_size_limit;
//! Heap ID counter
ایستا atomic32_t _memory_heap_id;
//! Huge page support
ایستا صحیح _memory_huge_pages;
#اگر ENABLE_GLOBAL_CACHE
//! Global span cache
ایستا global_cache_t _memory_span_cache[LARGE_CLASS_شمار];
#endif
//! Global reserved spans
ایستا span_t *_memory_global_reserve;
//! Global reserved count
ایستا size_t memory_global_reserve_شمار;
//! Global reserved master
ایستا span_t *_memory_global_reserve_master;
//! All heaps
ایستا heap_t *_memory_heaps[HEAP_ARRAY_SIZE];
//! Used to restrict access to mapping memory for huge pages
ایستا atomic32_t _memory_global_lock;
//! Orphaned heaps
ایستا heap_t *_memory_orphan_heaps;
#اگر RPMALLOC_FIRST_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
ایستا heap_t *_memory_first_class_orphan_heaps;
#endif
#اگر ENABLE_STATISTICS
//! Allocations counter
ایستا atomic64_t _allocation_counter;
//! Deallocations counter
ایستا atomic64_t _deallocation_counter;
//! Active heap count
ایستا atomic32_t _memory_active_heaps;
//! Number of currently mapped memory pages
ایستا atomic32_t _mapped_pages;
//! Peak number of concurrently mapped memory pages
ایستا int32_t _mapped_pages_peak;
//! Number of mapped master spans
ایستا atomic32_t _master_spans;
//! Number of unmapped dangling master spans
ایستا atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
ایستا atomic32_t _mapped_total;
//! Running counter of total number of unmapped memory pages since start
ایستا atomic32_t _unmapped_total;
//! Number of currently mapped memory pages in OS calls
ایستا atomic32_t _mapped_pages_os;
//! Number of currently allocated pages in huge allocations
ایستا atomic32_t _huge_pages_current;
//! Peak number of currently allocated pages in huge allocations
ایستا int32_t _huge_pages_peak;
#endif

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#اگر ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
ایستا pthread_key_t _memory_thread_heap;
#وگرنه
#ifdef _MSC_VER
#define _Thread_local __declspec(thread)
#define TLS_MODEL
#وگرنه
#ifndef __HAIKU__
#define TLS_MODEL __attribute__((tls_model("initial-exec")))
#وگرنه
#define TLS_MODEL
#endif
#اگر !defined(__clang__) && defined(__GNUC__)
#define _Thread_local __thread
#endif
#endif
ایستا _Thread_local heap_t *_memory_thread_heap TLS_MODEL;
#endif

ایستا درونخطی heap_t *get_thread_heap_raw(تهی) {
#اگر (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  بازگشت pthread_getspecific(_memory_thread_heap);
#وگرنه
  بازگشت _memory_thread_heap;
#endif
}

//! Get the current thread heap
ایستا درونخطی heap_t *get_thread_heap(تهی) {
  heap_t *heap = get_thread_heap_raw();
#اگر ENABLE_PRELOAD
  اگر (EXPECTED(heap != 0))
    بازگشت heap;
  rpmalloc_initialize();
  بازگشت get_thread_heap_raw();
#وگرنه
  بازگشت heap;
#endif
}

//! Fast thread ID
ایستا درونخطی uintptr_t get_thread_id(تهی) {
#اگر defined(_WIN32)
  بازگشت (uintptr_t)((تهی *)NtCurrentTeb());
#وگرنهاگر (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#اگر defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#وگرنهاگر defined(__x86_64__)
#اگر defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#وگرنه
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#endif
#وگرنهاگر defined(__arm__)
  __asm__ فرار("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#وگرنهاگر defined(__aarch64__)
#اگر defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ فرار("mrs %0, tpidrro_el0" : "=r"(tid));
#وگرنه
  __asm__ فرار("mrs %0, tpidr_el0" : "=r"(tid));
#endif
#وگرنه
#error This platform needs implementation of get_thread_id()
#endif
  بازگشت tid;
#وگرنه
#error This platform needs implementation of get_thread_id()
#endif
}

//! Set the current thread heap
ایستا تهی set_thread_heap(heap_t *heap) {
#اگر ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(_memory_thread_heap, heap);
#وگرنه
  _memory_thread_heap = heap;
#endif
  اگر (heap)
    heap->owner_thread = get_thread_id();
}

//! Set main thread ID
بیرونی تهی rpmalloc_set_main_thread(تهی);

تهی rpmalloc_set_main_thread(تهی) {
  _rpmalloc_main_thread_id = get_thread_id();
}

ایستا تهی _rpmalloc_spin(تهی) {
#اگر defined(_MSC_VER)
#اگر defined(_M_ARM64)
  __yield();
#وگرنه
  _mm_pause();
#endif
#وگرنهاگر defined(__x86_64__) || defined(__i386__)
  __asm__ فرار("pause" ::: "memory");
#وگرنهاگر defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ فرار("yield" ::: "memory");
#وگرنهاگر defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ فرار("or 27,27,27");
#وگرنهاگر defined(__sparc__)
  __asm__ فرار("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#وگرنه
  ساختار timespec ts = {0};
  nanosleep(&ts, 0);
#endif
}

#اگر defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

ایستا تهی NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  انتخاب (reason) {
  حالت DLL_PROCESS_ATTACH:
    بشکن;
  حالت DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    بشکن;
  حالت DLL_THREAD_ATTACH:
    بشکن;
  حالت DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    بشکن;
  }
}

#ifdef _WIN64
#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#pragma const_seg(".CRT$XLY")

بیرونی ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#pragma const_seg()
#وگرنه // _WIN64
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#pragma data_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#pragma data_seg()
#endif // _WIN64

ایستا تهی NTAPI _rpmalloc_thread_destructor(تهی *مقدار_2) {
#اگر ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  اگر (get_thread_id() == _rpmalloc_main_thread_id)
    بازگشت;
#endif
  اگر (مقدار_2)
    rpmalloc_thread_finalize(1);
}
#endif

////////////
///
/// Low level memory map/unmap
///
//////

ایستا تهی _rpmalloc_set_name(تهی *address, size_t size) {
#اگر defined(__linux__) || defined(__ANDROID__)
  ثابت نویسه *name = _memory_huge_pages ? _memory_config.huge_page_name
                                        : _memory_config.page_name;
  اگر (address == MAP_FAILED || !name)
    بازگشت;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (تهی)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (uintptr_t)address, size,
              (uintptr_t)name);
#وگرنه
  (تهی)اندازه(size);
  (تهی)اندازه(address);
#endif
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
ایستا تهی *_rpmalloc_mmap(size_t size, size_t *offset) {
  rpmalloc_assert(!(size % _memory_page_size), "Invalid mmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
  تهی *address = _memory_config.memory_map(size, offset);
  اگر (EXPECTED(address != 0)) {
    _rpmalloc_stat_add_peak(&_mapped_pages, (size >> _memory_page_size_shift),
                            _mapped_pages_peak);
    _rpmalloc_stat_add(&_mapped_total, (size >> _memory_page_size_shift));
  }
  بازگشت address;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
ایستا تهی _rpmalloc_unmap(تهی *address, size_t size, size_t offset,
                            size_t release) {
  rpmalloc_assert(!release || (release >= size), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  اگر (release) {
    rpmalloc_assert(!(release % _memory_page_size), "Invalid unmap size");
    _rpmalloc_stat_sub(&_mapped_pages, (release >> _memory_page_size_shift));
    _rpmalloc_stat_add(&_unmapped_total, (release >> _memory_page_size_shift));
  }
  _memory_config.memory_unmap(address, size, offset, release);
}

//! Default implementation to map new pages to virtual memory
ایستا تهی *_rpmalloc_mmap_os(size_t size, size_t *offset) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((size >= _memory_span_size) &&
                    (_memory_span_size > _memory_map_granularity))
                       ? _memory_span_size
                       : 0;
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
#اگر PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  تهی *ptr = VirtualAlloc(0, size + padding,
                           (_memory_huge_pages ? MEM_LARGE_PAGES : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
  اگر (!ptr) {
    اگر (_memory_config.map_fail_callback) {
      اگر (_memory_config.map_fail_callback(size + padding))
        بازگشت _rpmalloc_mmap_os(size, offset);
    } وگرنه {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    بازگشت 0;
  }
#وگرنه
  صحیح flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#اگر defined(__APPLE__) && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
  صحیح fd = (صحیح)VM_MAKE_TAG(240U);
  اگر (_memory_huge_pages)
    fd |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
  تهی *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#وگرنهاگر defined(MAP_HUGETLB)
  تهی *ptr = mmap(0, size + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (_memory_huge_pages ? MAP_HUGETLB : 0) | flags, -1, 0);
#اگر defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  اگر ((ptr == MAP_FAILED || !ptr) && _memory_huge_pages) {
    ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    اگر (ptr && ptr != MAP_FAILED) {
      صحیح prm = madvise(ptr, size + padding, MADV_HUGEPAGE);
      (تهی)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#endif
  _rpmalloc_set_name(ptr, size + padding);
#وگرنهاگر defined(MAP_ALIGNED)
  ثابت size_t align =
      (اندازه(size_t) * 8) - (size_t)(__builtin_clzl(size - 1));
  تهی *ptr =
      mmap(0, size + padding, PROT_READ | PROT_WRITE,
           (_memory_huge_pages ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#وگرنهاگر defined(MAP_ALIGN)
  caddr_t base = (_memory_huge_pages ? (caddr_t)(4 << 20) : 0);
  تهی *ptr = mmap(base, size + padding, PROT_READ | PROT_WRITE,
                   (_memory_huge_pages ? MAP_ALIGN : 0) | flags, -1, 0);
#وگرنه
  تهی *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#endif
  اگر ((ptr == MAP_FAILED) || !ptr) {
    اگر (_memory_config.map_fail_callback) {
      اگر (_memory_config.map_fail_callback(size + padding))
        بازگشت _rpmalloc_mmap_os(size, offset);
    } وگرنه اگر (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    بازگشت 0;
  }
#endif
  _rpmalloc_stat_add(&_mapped_pages_os,
                     (int32_t)((size + padding) >> _memory_page_size_shift));
  اگر (padding) {
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
  بازگشت ptr;
}

//! Default implementation to unmap pages from virtual memory
ایستا تهی _rpmalloc_unmap_os(تهی *address, size_t size, size_t offset,
                               size_t release) {
  rpmalloc_assert(release || (offset == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid unmap size");
  اگر (release && offset) {
    offset <<= 3;
    address = pointer_offset(address, -(int32_t)offset);
    اگر ((release >= _memory_span_size) &&
        (_memory_span_size > _memory_map_granularity)) {
      // Padding is always one span size
      release += _memory_span_size;
    }
  }
#اگر !DISABLE_UNMAP
#اگر PLATFORM_WINDOWS
  اگر (!VirtualFree(address, release ? 0 : size,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#وگرنه
  اگر (release) {
    اگر (munmap(address, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } وگرنه {
#اگر defined(MADV_FREE_REUSABLE)
    صحیح ret;
    وقتی ((ret = madvise(address, size, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    اگر ((ret == -1) && (errno != 0)) {
#وگرنهاگر defined(MADV_DONTNEED)
    اگر (madvise(address, size, MADV_DONTNEED)) {
#وگرنهاگر defined(MADV_PAGEOUT)
    اگر (madvise(address, size, MADV_PAGEOUT)) {
#وگرنهاگر defined(MADV_FREE)
    اگر (madvise(address, size, MADV_FREE)) {
#وگرنه
    اگر (posix_madvise(address, size, POSIX_MADV_DONTNEED)) {
#endif
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#endif
#endif
  اگر (release)
    _rpmalloc_stat_sub(&_mapped_pages_os, release >> _memory_page_size_shift);
}

ایستا تهی _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_شمار);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
ایستا span_t *_rpmalloc_global_get_reserved_spans(size_t span_شمار) {
  span_t *span = _memory_global_reserve;
  _rpmalloc_span_mark_as_subspan_unless_master(_memory_global_reserve_master,
                                               span, span_شمار);
  memory_global_reserve_شمار -= span_شمار;
  اگر (memory_global_reserve_شمار)
    _memory_global_reserve =
        (span_t *)pointer_offset(span, span_شمار << _memory_span_size_shift);
  وگرنه
    _memory_global_reserve = 0;
  بازگشت span;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
ایستا تهی _rpmalloc_global_set_reserved_spans(span_t *master, span_t *reserve,
                                                size_t reserve_span_شمار) {
  _memory_global_reserve_master = master;
  memory_global_reserve_شمار = reserve_span_شمار;
  _memory_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
ایستا تهی _rpmalloc_span_double_link_list_add(span_t **head, span_t *span) {
  اگر (*head)
    (*head)->prev = span;
  span->next = *head;
  *head = span;
}

//! Pop head span from double linked list
ایستا تهی rpmalloc_span_double_link_list_برداشت_head(span_t **head,
                                                     span_t *span) {
  rpmalloc_assert(*head == span, "Linked list corrupted");
  span = *head;
  *head = span->next;
}

//! Remove a span from double linked list
ایستا تهی _rpmalloc_span_double_link_list_remove(span_t **head,
                                                   span_t *span) {
  rpmalloc_assert(*head, "Linked list corrupted");
  اگر (*head == span) {
    *head = span->next;
  } وگرنه {
    span_t *next_span = span->next;
    span_t *prev_span = span->prev;
    prev_span->next = next_span;
    اگر (EXPECTED(next_span != 0))
      next_span->prev = prev_span;
  }
}

////////////
///
/// Span control
///
//////

ایستا تهی rpmalloc_heap_cache_درج(heap_t *heap, span_t *span);

ایستا تهی _rpmalloc_heap_finalize(heap_t *heap);

ایستا تهی _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_شمار);

//! Declare the span to be a subspan and store distance from master span and
//! span count
ایستا تهی _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_شمار) {
  rpmalloc_assert((subspan != master) || (subspan->flags & SPAN_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  اگر (subspan != master) {
    subspan->flags = SPAN_FLAG_SUBSPAN;
    subspan->offset_from_master =
        (uint32_t)((uintptr_t)pointer_diff(subspan, master) >>
                   _memory_span_size_shift);
    subspan->align_offset = 0;
  }
  subspan->span_شمار = (uint32_t)span_شمار;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
ایستا span_t *_rpmalloc_span_map_from_reserve(heap_t *heap,
                                               size_t span_شمار) {
  // Update the heap span reserve
  span_t *span = heap->span_reserve;
  heap->span_reserve =
      (span_t *)pointer_offset(span, span_شمار * _memory_span_size);
  heap->spans_reserved -= (uint32_t)span_شمار;

  _rpmalloc_span_mark_as_subspan_unless_master(heap->span_reserve_master, span,
                                               span_شمار);
  اگر (span_شمار <= LARGE_CLASS_شمار)
    _rpmalloc_stat_inc(&heap->span_use[span_شمار - 1].spans_from_reserved);

  بازگشت span;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
ایستا size_t rpmalloc_span_align_شمار(size_t span_شمار) {
  size_t request_شمار = (span_شمار > memory_span_map_شمار)
                             ? span_شمار
                             : memory_span_map_شمار;
  اگر ((_memory_page_size > _memory_span_size) &&
      ((request_شمار * _memory_span_size) % _memory_page_size))
    request_شمار +=
        memory_span_map_شمار - (request_شمار % memory_span_map_شمار);
  بازگشت request_شمار;
}

//! Setup a newly mapped span
ایستا تهی _rpmalloc_span_initialize(span_t *span, size_t total_span_شمار,
                                      size_t span_شمار, size_t align_offset) {
  span->total_spans = (uint32_t)total_span_شمار;
  span->span_شمار = (uint32_t)span_شمار;
  span->align_offset = (uint32_t)align_offset;
  span->flags = SPAN_FLAG_MASTER;
  atomic_store32(&span->remaining_spans, (int32_t)total_span_شمار);
}

ایستا تهی _rpmalloc_span_unmap(span_t *span);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
ایستا span_t *rpmalloc_span_map_aligned_شمار(heap_t *heap,
                                                size_t span_شمار) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_span_شمار = rpmalloc_span_align_شمار(span_شمار);
  size_t align_offset = 0;
  span_t *span = (span_t *)_rpmalloc_mmap(
      aligned_span_شمار * _memory_span_size, &align_offset);
  اگر (!span)
    بازگشت 0;
  _rpmalloc_span_initialize(span, aligned_span_شمار, span_شمار, align_offset);
  _rpmalloc_stat_inc(&_master_spans);
  اگر (span_شمار <= LARGE_CLASS_شمار)
    _rpmalloc_stat_inc(&heap->span_use[span_شمار - 1].spans_map_calls);
  اگر (aligned_span_شمار > span_شمار) {
    span_t *reserved_spans =
        (span_t *)pointer_offset(span, span_شمار * _memory_span_size);
    size_t reserved_شمار = aligned_span_شمار - span_شمار;
    اگر (heap->spans_reserved) {
      _rpmalloc_span_mark_as_subspan_unless_master(
          heap->span_reserve_master, heap->span_reserve, heap->spans_reserved);
      rpmalloc_heap_cache_درج(heap, heap->span_reserve);
    }
    اگر (reserved_شمار > memory_heap_reserve_شمار) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&_memory_global_lock) == 1,
                      "Global spin lock not held as expected");
      size_t remain_شمار = reserved_شمار - memory_heap_reserve_شمار;
      reserved_شمار = memory_heap_reserve_شمار;
      span_t *remain_span = (span_t *)pointer_offset(
          reserved_spans, reserved_شمار * _memory_span_size);
      اگر (_memory_global_reserve) {
        _rpmalloc_span_mark_as_subspan_unless_master(
            _memory_global_reserve_master, _memory_global_reserve,
            memory_global_reserve_شمار);
        _rpmalloc_span_unmap(_memory_global_reserve);
      }
      _rpmalloc_global_set_reserved_spans(span, remain_span, remain_شمار);
    }
    _rpmalloc_heap_set_reserved_spans(heap, span, reserved_spans,
                                      reserved_شمار);
  }
  بازگشت span;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
ایستا span_t *_rpmalloc_span_map(heap_t *heap, size_t span_شمار) {
  اگر (span_شمار <= heap->spans_reserved)
    بازگشت _rpmalloc_span_map_from_reserve(heap, span_شمار);
  span_t *span = 0;
  صحیح use_global_reserve =
      (_memory_page_size > _memory_span_size) ||
      (memory_span_map_شمار > memory_heap_reserve_شمار);
  اگر (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    وقتی (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
    اگر (memory_global_reserve_شمار >= span_شمار) {
      size_t reserve_شمار =
          (!heap->spans_reserved ? memory_heap_reserve_شمار : span_شمار);
      اگر (memory_global_reserve_شمار < reserve_شمار)
        reserve_شمار = memory_global_reserve_شمار;
      span = _rpmalloc_global_get_reserved_spans(reserve_شمار);
      اگر (span) {
        اگر (reserve_شمار > span_شمار) {
          span_t *reserved_span = (span_t *)pointer_offset(
              span, span_شمار << _memory_span_size_shift);
          _rpmalloc_heap_set_reserved_spans(heap, _memory_global_reserve_master,
                                            reserved_span,
                                            reserve_شمار - span_شمار);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        span->span_شمار = (uint32_t)span_شمار;
      }
    }
  }
  اگر (!span)
    span = rpmalloc_span_map_aligned_شمار(heap, span_شمار);
  اگر (use_global_reserve)
    atomic_store32_release(&_memory_global_lock, 0);
  بازگشت span;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
ایستا تهی _rpmalloc_span_unmap(span_t *span) {
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");

  صحیح is_master = !!(span->flags & SPAN_FLAG_MASTER);
  span_t *master =
      is_master ? span
                : ((span_t *)pointer_offset(
                      span, -(intptr_t)((uintptr_t)span->offset_from_master *
                                        _memory_span_size)));
  rpmalloc_assert(is_master || (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");

  size_t span_شمار = span->span_شمار;
  اگر (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(span->align_offset == 0, "Span align offset corrupted");
    اگر (_memory_span_size >= _memory_page_size)
      _rpmalloc_unmap(span, span_شمار * _memory_span_size, 0, 0);
  } وگرنه {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    span->flags |=
        SPAN_FLAG_MASTER | SPAN_FLAG_SUBSPAN | SPAN_FLAG_UNMAPPED_MASTER;
    _rpmalloc_stat_add(&_unmapped_master_spans, 1);
  }

  اگر (atomic_add32(&master->remaining_spans, -(int32_t)span_شمار) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & SPAN_FLAG_MASTER) &&
                        !!(master->flags & SPAN_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_شمار = master->span_شمار;
    اگر (_memory_span_size < _memory_page_size)
      unmap_شمار = master->total_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_شمار * _memory_span_size,
                    master->align_offset,
                    (size_t)master->total_spans * _memory_span_size);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
ایستا تهی _rpmalloc_span_release_to_cache(heap_t *heap, span_t *span) {
  rpmalloc_assert(heap == span->heap, "Span heap pointer corrupted");
  rpmalloc_assert(span->size_class < SIZE_CLASS_شمار,
                  "Invalid span size class");
  rpmalloc_assert(span->span_شمار == 1, "Invalid span count");
#اگر ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&heap->span_use[0].current);
#endif
  _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
  اگر (!heap->finalize) {
    _rpmalloc_stat_inc(&heap->span_use[0].spans_to_cache);
    _rpmalloc_stat_inc(&heap->size_class_use[span->size_class].spans_to_cache);
    اگر (heap->size_class[span->size_class].cache)
      rpmalloc_heap_cache_درج(heap,
                                  heap->size_class[span->size_class].cache);
    heap->size_class[span->size_class].cache = span;
  } وگرنه {
    _rpmalloc_span_unmap(span);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
ایستا uint32_t free_list_partial_init(تهی **list, تهی **first_block,
                                       تهی *page_start, تهی *block_start,
                                       uint32_t block_شمار,
                                       uint32_t block_size) {
  rpmalloc_assert(block_شمار, "Internal failure");
  *first_block = block_start;
  اگر (block_شمار > 1) {
    تهی *free_block = pointer_offset(block_start, block_size);
    تهی *block_end =
        pointer_offset(block_start, (size_t)block_size * block_شمار);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    اگر (block_size < (_memory_page_size >> 1)) {
      تهی *page_end = pointer_offset(page_start, _memory_page_size);
      اگر (page_end < block_end)
        block_end = page_end;
    }
    *list = free_block;
    block_شمار = 2;
    تهی *next_block = pointer_offset(free_block, block_size);
    وقتی (next_block < block_end) {
      *((تهی **)free_block) = next_block;
      free_block = next_block;
      ++block_شمار;
      next_block = pointer_offset(next_block, block_size);
    }
    *((تهی **)free_block) = 0;
  } وگرنه {
    *list = 0;
  }
  بازگشت block_شمار;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
ایستا تهی *_rpmalloc_span_initialize_new(heap_t *heap,
                                           heap_size_class_t *heap_size_class,
                                           span_t *span, uint32_t class_idx) {
  rpmalloc_assert(span->span_شمار == 1, "Internal failure");
  size_class_t *size_class = _memory_size_class + class_idx;
  span->size_class = class_idx;
  span->heap = heap;
  span->flags &= ~SPAN_FLAG_ALIGNED_BLOCKS;
  span->block_size = size_class->block_size;
  span->block_شمار = size_class->block_شمار;
  span->free_list = 0;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  تهی *block;
  span->free_list_limit =
      free_list_partial_init(&heap_size_class->free_list, &block, span,
                             pointer_offset(span, SPAN_HEADER_SIZE),
                             size_class->block_شمار, size_class->block_size);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  اگر (span->free_list_limit < span->block_شمار) {
    _rpmalloc_span_double_link_list_add(&heap_size_class->partial_span, span);
    span->used_شمار = span->free_list_limit;
  } وگرنه {
#اگر RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_شمار;
    span->used_شمار = span->block_شمار;
  }
  بازگشت block;
}

ایستا تهی _rpmalloc_span_extract_free_list_deferred(span_t *span) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  انجامبده {
    span->free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } وقتی (span->free_list == INVALID_POINTER);
  span->used_شمار -= span->list_size;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);
}

ایستا صحیح _rpmalloc_span_is_fully_utilized(span_t *span) {
  rpmalloc_assert(span->free_list_limit <= span->block_شمار,
                  "Span free list corrupted");
  بازگشت !span->free_list && (span->free_list_limit >= span->block_شمار);
}

ایستا صحیح _rpmalloc_span_finalize(heap_t *heap, size_t iclass, span_t *span,
                                   span_t **list_head) {
  تهی *free_list = heap->size_class[iclass].free_list;
  span_t *class_span = (span_t *)((uintptr_t)free_list & _memory_span_mask);
  اگر (span == class_span) {
    // Adopt the heap class free list back into the span free list
    تهی *block = span->free_list;
    تهی *last_block = 0;
    وقتی (block) {
      last_block = block;
      block = *((تهی **)block);
    }
    uint32_t free_شمار = 0;
    block = free_list;
    وقتی (block) {
      ++free_شمار;
      block = *((تهی **)block);
    }
    اگر (last_block) {
      *((تهی **)last_block) = free_list;
    } وگرنه {
      span->free_list = free_list;
    }
    heap->size_class[iclass].free_list = 0;
    span->used_شمار -= free_شمار;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(span->list_size == span->used_شمار, "Memory leak detected");
  اگر (span->list_size == span->used_شمار) {
    _rpmalloc_stat_dec(&heap->span_use[0].current);
    _rpmalloc_stat_dec(&heap->size_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    اگر (list_head)
      _rpmalloc_span_double_link_list_remove(list_head, span);
    _rpmalloc_span_unmap(span);
    بازگشت 1;
  }
  بازگشت 0;
}

////////////
///
/// Global cache
///
//////

#اگر ENABLE_GLOBAL_CACHE

//! Finalize a global cache
ایستا تهی _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  وقتی (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

  برای (size_t ispan = 0; ispan < cache->شمار; ++ispan)
    _rpmalloc_span_unmap(cache->span[ispan]);
  cache->شمار = 0;

  وقتی (cache->overflow) {
    span_t *span = cache->overflow;
    cache->overflow = span->next;
    _rpmalloc_span_unmap(span);
  }

  atomic_store32_release(&cache->lock, 0);
}

ایستا تهی rpmalloc_global_cache_درج_spans(span_t **span,
                                                size_t span_شمار,
                                                size_t شمار) {
  ثابت size_t cache_limit =
      (span_شمار == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_SPAN_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_SPAN_LARGE_CACHE - (span_شمار >> 1));

  global_cache_t *cache = &_memory_span_cache[span_شمار - 1];

  size_t درج_شمار = شمار;
  وقتی (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#اگر ENABLE_STATISTICS
  cache->درج_شمار += شمار;
#endif
  اگر ((cache->شمار + درج_شمار) > cache_limit)
    درج_شمار = cache_limit - cache->شمار;

  memcpy(cache->span + cache->شمار, span, اندازه(span_t *) * درج_شمار);
  cache->شمار += (uint32_t)درج_شمار;

#اگر ENABLE_UNLIMITED_CACHE
  وقتی (درج_شمار < شمار) {
#وگرنه
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  وقتی ((_memory_page_size > _memory_span_size) && (درج_شمار < شمار)) {
#endif
    span_t *current_span = span[درج_شمار++];
    current_span->next = cache->overflow;
    cache->overflow = current_span;
  }
  atomic_store32_release(&cache->lock, 0);

  span_t *keep = 0;
  برای (size_t ispan = درج_شمار; ispan < شمار; ++ispan) {
    span_t *current_span = span[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    اگر ((current_span->flags & SPAN_FLAG_MASTER) &&
        (atomic_load32(&current_span->remaining_spans) >
         (int32_t)current_span->span_شمار)) {
      current_span->next = keep;
      keep = current_span;
    } وگرنه {
      _rpmalloc_span_unmap(current_span);
    }
  }

  اگر (keep) {
    وقتی (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    وقتی (keep) {
      برای (; islot < cache->شمار; ++islot) {
        span_t *current_span = cache->span[islot];
        اگر (!(current_span->flags & SPAN_FLAG_MASTER) ||
            ((current_span->flags & SPAN_FLAG_MASTER) &&
             (atomic_load32(&current_span->remaining_spans) <=
              (int32_t)current_span->span_شمار))) {
          _rpmalloc_span_unmap(current_span);
          cache->span[islot] = keep;
          بشکن;
        }
      }
      اگر (islot == cache->شمار)
        بشکن;
      keep = keep->next;
    }

    اگر (keep) {
      span_t *tail = keep;
      وقتی (tail->next)
        tail = tail->next;
      tail->next = cache->overflow;
      cache->overflow = keep;
    }

    atomic_store32_release(&cache->lock, 0);
  }
}

ایستا size_t _rpmalloc_global_cache_extract_spans(span_t **span,
                                                   size_t span_شمار,
                                                   size_t شمار) {
  global_cache_t *cache = &_memory_span_cache[span_شمار - 1];

  size_t extract_شمار = 0;
  وقتی (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#اگر ENABLE_STATISTICS
  cache->extract_شمار += شمار;
#endif
  size_t want = شمار - extract_شمار;
  اگر (want > cache->شمار)
    want = cache->شمار;

  memcpy(span + extract_شمار, cache->span + (cache->شمار - want),
         اندازه(span_t *) * want);
  cache->شمار -= (uint32_t)want;
  extract_شمار += want;

  وقتی ((extract_شمار < شمار) && cache->overflow) {
    span_t *current_span = cache->overflow;
    span[extract_شمار++] = current_span;
    cache->overflow = current_span->next;
  }

#اگر ENABLE_ASSERTS
  برای (size_t ispan = 0; ispan < extract_شمار; ++ispan) {
    rpmalloc_assert(span[ispan]->span_شمار == span_شمار,
                    "Global cache span count mismatch");
  }
#endif

  atomic_store32_release(&cache->lock, 0);

  بازگشت extract_شمار;
}

#endif

////////////
///
/// Heap control
///
//////

ایستا تهی _rpmalloc_deallocate_huge(span_t *);

//! Store the given spans as reserve in the given heap
ایستا تهی _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_شمار) {
  heap->span_reserve_master = master;
  heap->span_reserve = reserve;
  heap->spans_reserved = (uint32_t)reserve_span_شمار;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
ایستا تهی _rpmalloc_heap_cache_adopt_deferred(heap_t *heap,
                                                span_t **single_span) {
  span_t *span = (span_t *)((تهی *)atomic_exchange_ptr_acquire(
      &heap->span_free_deferred, 0));
  وقتی (span) {
    span_t *next_span = (span_t *)span->free_list;
    rpmalloc_assert(span->heap == heap, "Span heap pointer corrupted");
    اگر (EXPECTED(span->size_class < SIZE_CLASS_شمار)) {
      rpmalloc_assert(heap->full_span_شمار, "Heap span counter corrupted");
      --heap->full_span_شمار;
      _rpmalloc_stat_dec(&heap->span_use[0].spans_deferred);
#اگر RPMALLOC_FIRST_CLASS_HEAPS
      _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                             span);
#endif
      _rpmalloc_stat_dec(&heap->span_use[0].current);
      _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
      اگر (single_span && !*single_span)
        *single_span = span;
      وگرنه
        rpmalloc_heap_cache_درج(heap, span);
    } وگرنه {
      اگر (span->size_class == SIZE_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(span);
      } وگرنه {
        rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(heap->full_span_شمار, "Heap span counter corrupted");
        --heap->full_span_شمار;
#اگر RPMALLOC_FIRST_CLASS_HEAPS
        _rpmalloc_span_double_link_list_remove(&heap->large_huge_span, span);
#endif
        uint32_t idx = span->span_شمار - 1;
        _rpmalloc_stat_dec(&heap->span_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&heap->span_use[idx].current);
        اگر (!idx && single_span && !*single_span)
          *single_span = span;
        وگرنه
          rpmalloc_heap_cache_درج(heap, span);
      }
    }
    span = next_span;
  }
}

ایستا تهی _rpmalloc_heap_unmap(heap_t *heap) {
  اگر (!heap->master_heap) {
    اگر ((heap->finalize > 1) && !atomic_load32(&heap->child_شمار)) {
      span_t *span = (span_t *)((uintptr_t)heap & _memory_span_mask);
      _rpmalloc_span_unmap(span);
    }
  } وگرنه {
    اگر (atomic_decr32(&heap->master_heap->child_شمار) == 0) {
      _rpmalloc_heap_unmap(heap->master_heap);
    }
  }
}

ایستا تهی _rpmalloc_heap_global_finalize(heap_t *heap) {
  اگر (heap->finalize++ > 1) {
    --heap->finalize;
    بازگشت;
  }

  _rpmalloc_heap_finalize(heap);

#اگر ENABLE_THREAD_CACHE
  برای (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    span_cache_t *span_cache;
    اگر (!iclass)
      span_cache = &heap->span_cache;
    وگرنه
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    برای (size_t ispan = 0; ispan < span_cache->شمار; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->شمار = 0;
  }
#endif

  اگر (heap->full_span_شمار) {
    --heap->finalize;
    بازگشت;
  }

  برای (size_t iclass = 0; iclass < SIZE_CLASS_شمار; ++iclass) {
    اگر (heap->size_class[iclass].free_list ||
        heap->size_class[iclass].partial_span) {
      --heap->finalize;
      بازگشت;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap_t *list_heap = _memory_heaps[list_idx];
  اگر (list_heap == heap) {
    _memory_heaps[list_idx] = heap->next_heap;
  } وگرنه {
    وقتی (list_heap->next_heap != heap)
      list_heap = list_heap->next_heap;
    list_heap->next_heap = heap->next_heap;
  }

  _rpmalloc_heap_unmap(heap);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
ایستا تهی rpmalloc_heap_cache_درج(heap_t *heap, span_t *span) {
  اگر (UNEXPECTED(heap->finalize != 0)) {
    _rpmalloc_span_unmap(span);
    _rpmalloc_heap_global_finalize(heap);
    بازگشت;
  }
#اگر ENABLE_THREAD_CACHE
  size_t span_شمار = span->span_شمار;
  _rpmalloc_stat_inc(&heap->span_use[span_شمار - 1].spans_to_cache);
  اگر (span_شمار == 1) {
    span_cache_t *span_cache = &heap->span_cache;
    span_cache->span[span_cache->شمار++] = span;
    اگر (span_cache->شمار == MAX_THREAD_SPAN_CACHE) {
      ثابت size_t remain_شمار =
          MAX_THREAD_SPAN_CACHE - THREAD_SPAN_CACHE_TRANSFER;
#اگر ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           THREAD_SPAN_CACHE_TRANSFER * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_شمار - 1].spans_to_global,
                         THREAD_SPAN_CACHE_TRANSFER);
      rpmalloc_global_cache_درج_spans(span_cache->span + remain_شمار,
                                          span_شمار,
                                          THREAD_SPAN_CACHE_TRANSFER);
#وگرنه
      برای (size_t ispan = 0; ispan < THREAD_SPAN_CACHE_TRANSFER; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_شمار + ispan]);
#endif
      span_cache->شمار = remain_شمار;
    }
  } وگرنه {
    size_t cache_idx = span_شمار - 2;
    span_large_cache_t *span_cache = heap->span_large_cache + cache_idx;
    span_cache->span[span_cache->شمار++] = span;
    ثابت size_t cache_limit =
        (MAX_THREAD_SPAN_LARGE_CACHE - (span_شمار >> 1));
    اگر (span_cache->شمار == cache_limit) {
      ثابت size_t transfer_limit = 2 + (cache_limit >> 2);
      ثابت size_t transfer_شمار =
          (THREAD_SPAN_LARGE_CACHE_TRANSFER <= transfer_limit
               ? THREAD_SPAN_LARGE_CACHE_TRANSFER
               : transfer_limit);
      ثابت size_t remain_شمار = cache_limit - transfer_شمار;
#اگر ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           transfer_شمار * span_شمار * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_شمار - 1].spans_to_global,
                         transfer_شمار);
      rpmalloc_global_cache_درج_spans(span_cache->span + remain_شمار,
                                          span_شمار, transfer_شمار);
#وگرنه
      برای (size_t ispan = 0; ispan < transfer_شمار; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_شمار + ispan]);
#endif
      span_cache->شمار = remain_شمار;
    }
  }
#وگرنه
  (تهی)اندازه(heap);
  _rpmalloc_span_unmap(span);
#endif
}

//! Extract the given number of spans from the different cache levels
ایستا span_t *_rpmalloc_heap_thread_cache_extract(heap_t *heap,
                                                   size_t span_شمار) {
  span_t *span = 0;
#اگر ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  اگر (span_شمار == 1)
    span_cache = &heap->span_cache;
  وگرنه
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_شمار - 2));
  اگر (span_cache->شمار) {
    _rpmalloc_stat_inc(&heap->span_use[span_شمار - 1].spans_from_cache);
    بازگشت span_cache->span[--span_cache->شمار];
  }
#endif
  بازگشت span;
}

ایستا span_t *_rpmalloc_heap_thread_cache_deferred_extract(heap_t *heap,
                                                            size_t span_شمار) {
  span_t *span = 0;
  اگر (span_شمار == 1) {
    _rpmalloc_heap_cache_adopt_deferred(heap, &span);
  } وگرنه {
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
    span = _rpmalloc_heap_thread_cache_extract(heap, span_شمار);
  }
  بازگشت span;
}

ایستا span_t *_rpmalloc_heap_reserved_extract(heap_t *heap,
                                               size_t span_شمار) {
  اگر (heap->spans_reserved >= span_شمار)
    بازگشت _rpmalloc_span_map(heap, span_شمار);
  بازگشت 0;
}

//! Extract a span from the global cache
ایستا span_t *_rpmalloc_heap_global_cache_extract(heap_t *heap,
                                                   size_t span_شمار) {
#اگر ENABLE_GLOBAL_CACHE
#اگر ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  size_t wanted_شمار;
  اگر (span_شمار == 1) {
    span_cache = &heap->span_cache;
    wanted_شمار = THREAD_SPAN_CACHE_TRANSFER;
  } وگرنه {
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_شمار - 2));
    wanted_شمار = THREAD_SPAN_LARGE_CACHE_TRANSFER;
  }
  span_cache->شمار = _rpmalloc_global_cache_extract_spans(
      span_cache->span, span_شمار, wanted_شمار);
  اگر (span_cache->شمار) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_شمار * span_cache->شمار * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_شمار - 1].spans_from_global,
                       span_cache->شمار);
    بازگشت span_cache->span[--span_cache->شمار];
  }
#وگرنه
  span_t *span = 0;
  size_t شمار = _rpmalloc_global_cache_extract_spans(&span, span_شمار, 1);
  اگر (شمار) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_شمار * شمار * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_شمار - 1].spans_from_global,
                       شمار);
    بازگشت span;
  }
#endif
#endif
  (تهی)اندازه(heap);
  (تهی)اندازه(span_شمار);
  بازگشت 0;
}

ایستا تهی _rpmalloc_inc_span_statistics(heap_t *heap, size_t span_شمار,
                                          uint32_t class_idx) {
  (تهی)اندازه(heap);
  (تهی)اندازه(span_شمار);
  (تهی)اندازه(class_idx);
#اگر ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)span_شمار - 1;
  uint32_t current_شمار =
      (uint32_t)atomic_incr32(&heap->span_use[idx].current);
  اگر (current_شمار > (uint32_t)atomic_load32(&heap->span_use[idx].بالا))
    atomic_store32(&heap->span_use[idx].بالا, (int32_t)current_شمار);
  _rpmalloc_stat_add_peak(&heap->size_class_use[class_idx].spans_current, 1,
                          heap->size_class_use[class_idx].spans_peak);
#endif
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
ایستا span_t *
_rpmalloc_heap_extract_new_span(heap_t *heap,
                                heap_size_class_t *heap_size_class,
                                size_t span_شمار, uint32_t class_idx) {
  span_t *span;
#اگر ENABLE_THREAD_CACHE
  اگر (heap_size_class && heap_size_class->cache) {
    span = heap_size_class->cache;
    heap_size_class->cache =
        (heap->span_cache.شمار
             ? heap->span_cache.span[--heap->span_cache.شمار]
             : 0);
    _rpmalloc_inc_span_statistics(heap, span_شمار, class_idx);
    بازگشت span;
  }
#endif
  (تهی)اندازه(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t base_span_شمار = span_شمار;
  size_t limit_span_شمار =
      (span_شمار > 2) ? (span_شمار + (span_شمار >> 1)) : span_شمار;
  اگر (limit_span_شمار > LARGE_CLASS_شمار)
    limit_span_شمار = LARGE_CLASS_شمار;
  انجامبده {
    span = _rpmalloc_heap_thread_cache_extract(heap, span_شمار);
    اگر (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_شمار, class_idx);
      بازگشت span;
    }
    span = _rpmalloc_heap_thread_cache_deferred_extract(heap, span_شمار);
    اگر (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_شمار, class_idx);
      بازگشت span;
    }
    span = _rpmalloc_heap_global_cache_extract(heap, span_شمار);
    اگر (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_شمار, class_idx);
      بازگشت span;
    }
    span = _rpmalloc_heap_reserved_extract(heap, span_شمار);
    اگر (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_reserved);
      _rpmalloc_inc_span_statistics(heap, span_شمار, class_idx);
      بازگشت span;
    }
    ++span_شمار;
  } وقتی (span_شمار <= limit_span_شمار);
  // Final fallback, map in more virtual memory
  span = _rpmalloc_span_map(heap, base_span_شمار);
  _rpmalloc_inc_span_statistics(heap, base_span_شمار, class_idx);
  _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_map_calls);
  بازگشت span;
}

ایستا تهی _rpmalloc_heap_initialize(heap_t *heap) {
  _rpmalloc_memset_const(heap, 0, اندازه(heap_t));
  // Get a new heap ID
  heap->id = 1 + atomic_incr32(&_memory_heap_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap->next_heap = _memory_heaps[list_idx];
  _memory_heaps[list_idx] = heap;
}

ایستا تهی _rpmalloc_heap_orphan(heap_t *heap, صحیح first_class) {
  heap->owner_thread = (uintptr_t)-1;
#اگر RPMALLOC_FIRST_CLASS_HEAPS
  heap_t **heap_list =
      (first_class ? &_memory_first_class_orphan_heaps : &_memory_orphan_heaps);
#وگرنه
  (تهی)اندازه(first_class);
  heap_t **heap_list = &_memory_orphan_heaps;
#endif
  heap->next_orphan = *heap_list;
  *heap_list = heap;
}

//! Allocate a new heap from newly mapped memory pages
ایستا heap_t *_rpmalloc_heap_allocate_new(تهی) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t heap_size = اندازه(heap_t);
  size_t aligned_heap_size = 16 * ((heap_size + 15) / 16);
  size_t request_heap_شمار = 16;
  size_t heap_span_شمار = ((aligned_heap_size * request_heap_شمار) +
                            اندازه(span_t) + _memory_span_size - 1) /
                           _memory_span_size;
  size_t block_size = _memory_span_size * heap_span_شمار;
  size_t span_شمار = heap_span_شمار;
  span_t *span = 0;
  // If there are global reserved spans, use these first
  اگر (memory_global_reserve_شمار >= heap_span_شمار) {
    span = _rpmalloc_global_get_reserved_spans(heap_span_شمار);
  }
  اگر (!span) {
    اگر (_memory_page_size > block_size) {
      span_شمار = _memory_page_size / _memory_span_size;
      block_size = _memory_page_size;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_heap_شمار =
          (block_size - اندازه(span_t)) / aligned_heap_size;
      اگر (possible_heap_شمار >= (request_heap_شمار * 16))
        request_heap_شمار *= 16;
      وگرنه اگر (possible_heap_شمار < request_heap_شمار)
        request_heap_شمار = possible_heap_شمار;
      heap_span_شمار = ((aligned_heap_size * request_heap_شمار) +
                         اندازه(span_t) + _memory_span_size - 1) /
                        _memory_span_size;
    }

    size_t align_offset = 0;
    span = (span_t *)_rpmalloc_mmap(block_size, &align_offset);
    اگر (!span)
      بازگشت 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    _rpmalloc_span_initialize(span, span_شمار, heap_span_شمار, align_offset);
  }

  size_t remain_size = _memory_span_size - اندازه(span_t);
  heap_t *heap = (heap_t *)pointer_offset(span, اندازه(span_t));
  _rpmalloc_heap_initialize(heap);

  // Put extra heaps as orphans
  size_t رقم_heaps = remain_size / aligned_heap_size;
  اگر (رقم_heaps < request_heap_شمار)
    رقم_heaps = request_heap_شمار;
  atomic_store32(&heap->child_شمار, (int32_t)رقم_heaps - 1);
  heap_t *extra_heap = (heap_t *)pointer_offset(heap, aligned_heap_size);
  وقتی (رقم_heaps > 1) {
    _rpmalloc_heap_initialize(extra_heap);
    extra_heap->master_heap = heap;
    _rpmalloc_heap_orphan(extra_heap, 1);
    extra_heap = (heap_t *)pointer_offset(extra_heap, aligned_heap_size);
    --رقم_heaps;
  }

  اگر (span_شمار > heap_span_شمار) {
    // Cap reserved spans
    size_t remain_شمار = span_شمار - heap_span_شمار;
    size_t reserve_شمار =
        (remain_شمار > memory_heap_reserve_شمار ? memory_heap_reserve_شمار
                                                   : remain_شمار);
    span_t *remain_span =
        (span_t *)pointer_offset(span, heap_span_شمار * _memory_span_size);
    _rpmalloc_heap_set_reserved_spans(heap, span, remain_span, reserve_شمار);

    اگر (remain_شمار > reserve_شمار) {
      // Set to global reserved spans
      remain_span = (span_t *)pointer_offset(remain_span,
                                             reserve_شمار * _memory_span_size);
      reserve_شمار = remain_شمار - reserve_شمار;
      _rpmalloc_global_set_reserved_spans(span, remain_span, reserve_شمار);
    }
  }

  بازگشت heap;
}

ایستا heap_t *_rpmalloc_heap_extract_orphan(heap_t **heap_list) {
  heap_t *heap = *heap_list;
  *heap_list = (heap ? heap->next_orphan : 0);
  بازگشت heap;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
ایستا heap_t *_rpmalloc_heap_allocate(صحیح first_class) {
  heap_t *heap = 0;
  وقتی (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
    _rpmalloc_spin();
  اگر (first_class == 0)
    heap = _rpmalloc_heap_extract_orphan(&_memory_orphan_heaps);
#اگر RPMALLOC_FIRST_CLASS_HEAPS
  اگر (!heap)
    heap = _rpmalloc_heap_extract_orphan(&_memory_first_class_orphan_heaps);
#endif
  اگر (!heap)
    heap = _rpmalloc_heap_allocate_new();
  atomic_store32_release(&_memory_global_lock, 0);
  اگر (heap)
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  بازگشت heap;
}

ایستا تهی _rpmalloc_heap_release(تهی *heapptr, صحیح first_class,
                                   صحیح release_cache) {
  heap_t *heap = (heap_t *)heapptr;
  اگر (!heap)
    بازگشت;
  // Release thread cache spans back to global cache
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  اگر (release_cache || heap->finalize) {
#اگر ENABLE_THREAD_CACHE
    برای (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
      span_cache_t *span_cache;
      اگر (!iclass)
        span_cache = &heap->span_cache;
      وگرنه
        span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
      اگر (!span_cache->شمار)
        ادامه;
#اگر ENABLE_GLOBAL_CACHE
      اگر (heap->finalize) {
        برای (size_t ispan = 0; ispan < span_cache->شمار; ++ispan)
          _rpmalloc_span_unmap(span_cache->span[ispan]);
      } وگرنه {
        _rpmalloc_stat_add64(&heap->thread_to_global, span_cache->شمار *
                                                          (iclass + 1) *
                                                          _memory_span_size);
        _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                           span_cache->شمار);
        rpmalloc_global_cache_درج_spans(span_cache->span, iclass + 1,
                                            span_cache->شمار);
      }
#وگرنه
      برای (size_t ispan = 0; ispan < span_cache->شمار; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
      span_cache->شمار = 0;
    }
#endif
  }

  اگر (get_thread_heap_raw() == heap)
    set_thread_heap(0);

#اگر ENABLE_STATISTICS
  atomic_decr32(&_memory_active_heaps);
  rpmalloc_assert(atomic_load32(&_memory_active_heaps) >= 0,
                  "Still active heaps during finalization");
#endif

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  اگر (get_thread_id() != _rpmalloc_main_thread_id) {
    وقتی (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
  }
  _rpmalloc_heap_orphan(heap, first_class);
  atomic_store32_release(&_memory_global_lock, 0);
}

ایستا تهی _rpmalloc_heap_release_raw(تهی *heapptr, صحیح release_cache) {
  _rpmalloc_heap_release(heapptr, 0, release_cache);
}

ایستا تهی _rpmalloc_heap_release_raw_fc(تهی *heapptr) {
  _rpmalloc_heap_release_raw(heapptr, 1);
}

ایستا تهی _rpmalloc_heap_finalize(heap_t *heap) {
  اگر (heap->spans_reserved) {
    span_t *span = _rpmalloc_span_map(heap, heap->spans_reserved);
    _rpmalloc_span_unmap(span);
    heap->spans_reserved = 0;
  }

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  برای (size_t iclass = 0; iclass < SIZE_CLASS_شمار; ++iclass) {
    اگر (heap->size_class[iclass].cache)
      _rpmalloc_span_unmap(heap->size_class[iclass].cache);
    heap->size_class[iclass].cache = 0;
    span_t *span = heap->size_class[iclass].partial_span;
    وقتی (span) {
      span_t *next = span->next;
      _rpmalloc_span_finalize(heap, iclass, span,
                              &heap->size_class[iclass].partial_span);
      span = next;
    }
    // If class still has a free list it must be a full span
    اگر (heap->size_class[iclass].free_list) {
      span_t *class_span =
          (span_t *)((uintptr_t)heap->size_class[iclass].free_list &
                     _memory_span_mask);
      span_t **list = 0;
#اگر RPMALLOC_FIRST_CLASS_HEAPS
      list = &heap->full_span[iclass];
#endif
      --heap->full_span_شمار;
      اگر (!_rpmalloc_span_finalize(heap, iclass, class_span, list)) {
        اگر (list)
          _rpmalloc_span_double_link_list_remove(list, class_span);
        _rpmalloc_span_double_link_list_add(
            &heap->size_class[iclass].partial_span, class_span);
      }
    }
  }

#اگر ENABLE_THREAD_CACHE
  برای (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    span_cache_t *span_cache;
    اگر (!iclass)
      span_cache = &heap->span_cache;
    وگرنه
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    برای (size_t ispan = 0; ispan < span_cache->شمار; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->شمار = 0;
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
ایستا تهی *free_list_برداشت(تهی **list) {
  تهی *block = *list;
  *list = *((تهی **)block);
  بازگشت block;
}

//! Allocate a small/medium sized memory block from the given heap
ایستا تهی *_rpmalloc_allocate_from_heap_fallback(
    heap_t *heap, heap_size_class_t *heap_size_class, uint32_t class_idx) {
  span_t *span = heap_size_class->partial_span;
  rpmalloc_assume(heap != 0);
  اگر (EXPECTED(span != 0)) {
    rpmalloc_assert(span->block_شمار ==
                        _memory_size_class[span->size_class].block_شمار,
                    "Span block count corrupted");
    rpmalloc_assert(!_rpmalloc_span_is_fully_utilized(span),
                    "Internal failure");
    تهی *block;
    اگر (span->free_list) {
      // Span local free list is not empty, swap to size class free list
      block = free_list_برداشت(&span->free_list);
      heap_size_class->free_list = span->free_list;
      span->free_list = 0;
    } وگرنه {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      تهی *block_start = pointer_offset(
          span, SPAN_HEADER_SIZE +
                    ((size_t)span->free_list_limit * span->block_size));
      span->free_list_limit += free_list_partial_init(
          &heap_size_class->free_list, &block,
          (تهی *)((uintptr_t)block_start & ~(_memory_page_size - 1)),
          block_start, span->block_شمار - span->free_list_limit,
          span->block_size);
    }
    rpmalloc_assert(span->free_list_limit <= span->block_شمار,
                    "Span block count corrupted");
    span->used_شمار = span->free_list_limit;

    // Swap in deferred free list if present
    اگر (atomic_load_ptr(&span->free_list_deferred))
      _rpmalloc_span_extract_free_list_deferred(span);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    اگر (!_rpmalloc_span_is_fully_utilized(span))
      بازگشت block;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_span_double_link_list_برداشت_head(&heap_size_class->partial_span,
                                             span);
#اگر RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_شمار;
    بازگشت block;
  }

  // Find a span in one of the cache levels
  span = _rpmalloc_heap_extract_new_span(heap, heap_size_class, 1, class_idx);
  اگر (EXPECTED(span != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    بازگشت _rpmalloc_span_initialize_new(heap, heap_size_class, span,
                                         class_idx);
  }

  بازگشت 0;
}

//! Allocate a small sized memory block from the given heap
ایستا تهی *_rpmalloc_allocate_small(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Small sizes have unique size classes
  ثابت uint32_t class_idx =
      (uint32_t)((size + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_SHIFT);
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  اگر (EXPECTED(heap_size_class->free_list != 0))
    بازگشت free_list_برداشت(&heap_size_class->free_list);
  بازگشت _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
ایستا تهی *_rpmalloc_allocate_medium(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ثابت uint32_t base_idx =
      (uint32_t)(SMALL_CLASS_شمار +
                 ((size - (SMALL_SIZE_LIMIT + 1)) >> MEDIUM_GRANULARITY_SHIFT));
  ثابت uint32_t class_idx = _memory_size_class[base_idx].class_idx;
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  اگر (EXPECTED(heap_size_class->free_list != 0))
    بازگشت free_list_برداشت(&heap_size_class->free_list);
  بازگشت _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
ایستا تهی *_rpmalloc_allocate_large(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  size += SPAN_HEADER_SIZE;
  size_t span_شمار = size >> _memory_span_size_shift;
  اگر (size & (_memory_span_size - 1))
    ++span_شمار;

  // Find a span in one of the cache levels
  span_t *span =
      _rpmalloc_heap_extract_new_span(heap, 0, span_شمار, SIZE_CLASS_LARGE);
  اگر (!span)
    بازگشت span;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(span->span_شمار >= span_شمار, "Internal failure");
  span->size_class = SIZE_CLASS_LARGE;
  span->heap = heap;

#اگر RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_شمار;

  بازگشت pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a huge block by mapping memory pages directly
ایستا تهی *_rpmalloc_allocate_huge(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  size += SPAN_HEADER_SIZE;
  size_t رقم_pages = size >> _memory_page_size_shift;
  اگر (size & (_memory_page_size - 1))
    ++رقم_pages;
  size_t align_offset = 0;
  span_t *span =
      (span_t *)_rpmalloc_mmap(رقم_pages * _memory_page_size, &align_offset);
  اگر (!span)
    بازگشت span;

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_شمار = (uint32_t)رقم_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, رقم_pages, _huge_pages_peak);

#اگر RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_شمار;

  بازگشت pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a block of the given size
ایستا تهی *_rpmalloc_allocate(heap_t *heap, size_t size) {
  _rpmalloc_stat_add64(&_allocation_counter, 1);
  اگر (EXPECTED(size <= SMALL_SIZE_LIMIT))
    بازگشت _rpmalloc_allocate_small(heap, size);
  وگرنه اگر (size <= _memory_medium_size_limit)
    بازگشت _rpmalloc_allocate_medium(heap, size);
  وگرنه اگر (size <= LARGE_SIZE_LIMIT)
    بازگشت _rpmalloc_allocate_large(heap, size);
  بازگشت _rpmalloc_allocate_huge(heap, size);
}

ایستا تهی *_rpmalloc_aligned_allocate(heap_t *heap, size_t alignment,
                                        size_t size) {
  اگر (alignment <= SMALL_GRANULARITY)
    بازگشت _rpmalloc_allocate(heap, size);

#اگر ENABLE_VALIDATE_ARGS
  اگر ((size + alignment) < size) {
    errno = EINVAL;
    بازگشت 0;
  }
  اگر (alignment & (alignment - 1)) {
    errno = EINVAL;
    بازگشت 0;
  }
#endif

  اگر ((alignment <= SPAN_HEADER_SIZE) &&
      ((size + SPAN_HEADER_SIZE) < _memory_medium_size_limit)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t multiple_size = size ? (size + (SPAN_HEADER_SIZE - 1)) &
                                      ~(uintptr_t)(SPAN_HEADER_SIZE - 1)
                                : SPAN_HEADER_SIZE;
    rpmalloc_assert(!(multiple_size % SPAN_HEADER_SIZE),
                    "Failed alignment calculation");
    اگر (multiple_size <= (size + alignment))
      بازگشت _rpmalloc_allocate(heap, multiple_size);
  }

  تهی *ptr = 0;
  size_t align_mask = alignment - 1;
  اگر (alignment <= _memory_page_size) {
    ptr = _rpmalloc_allocate(heap, size + alignment);
    اگر ((uintptr_t)ptr & align_mask) {
      ptr = (تهی *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);
      // Mark as having aligned blocks
      span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
      span->flags |= SPAN_FLAG_ALIGNED_BLOCKS;
    }
    بازگشت ptr;
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
  اگر (alignment & align_mask) {
    errno = EINVAL;
    بازگشت 0;
  }
  اگر (alignment >= _memory_span_size) {
    errno = EINVAL;
    بازگشت 0;
  }

  size_t extra_pages = alignment / _memory_page_size;

  // Since each span has a header, we will at least need one extra memory page
  size_t رقم_pages = 1 + (size / _memory_page_size);
  اگر (size & (_memory_page_size - 1))
    ++رقم_pages;

  اگر (extra_pages > رقم_pages)
    رقم_pages = 1 + extra_pages;

  size_t original_pages = رقم_pages;
  size_t limit_pages = (_memory_span_size / _memory_page_size) * 2;
  اگر (limit_pages < (original_pages * 2))
    limit_pages = original_pages * 2;

  size_t mapped_size, align_offset;
  span_t *span;

retry:
  align_offset = 0;
  mapped_size = رقم_pages * _memory_page_size;

  span = (span_t *)_rpmalloc_mmap(mapped_size, &align_offset);
  اگر (!span) {
    errno = ENOMEM;
    بازگشت 0;
  }
  ptr = pointer_offset(span, SPAN_HEADER_SIZE);

  اگر ((uintptr_t)ptr & align_mask)
    ptr = (تهی *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);

  اگر (((size_t)pointer_diff(ptr, span) >= _memory_span_size) ||
      (pointer_offset(ptr, size) > pointer_offset(span, mapped_size)) ||
      (((uintptr_t)ptr & _memory_span_mask) != (uintptr_t)span)) {
    _rpmalloc_unmap(span, mapped_size, align_offset, mapped_size);
    ++رقم_pages;
    اگر (رقم_pages > limit_pages) {
      errno = EINVAL;
      بازگشت 0;
    }
    برو retry;
  }

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_شمار = (uint32_t)رقم_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, رقم_pages, _huge_pages_peak);

#اگر RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_شمار;

  _rpmalloc_stat_add64(&_allocation_counter, 1);

  بازگشت ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
ایستا تهی _rpmalloc_deallocate_direct_small_or_medium(span_t *span,
                                                        تهی *block) {
  heap_t *heap = span->heap;
  rpmalloc_assert(heap->owner_thread == get_thread_id() ||
                      !heap->owner_thread || heap->finalize,
                  "Internal failure");
  // Add block to free list
  اگر (UNEXPECTED(_rpmalloc_span_is_fully_utilized(span))) {
    span->used_شمار = span->block_شمار;
#اگر RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                           span);
#endif
    _rpmalloc_span_double_link_list_add(
        &heap->size_class[span->size_class].partial_span, span);
    --heap->full_span_شمار;
  }
  *((تهی **)block) = span->free_list;
  --span->used_شمار;
  span->free_list = block;
  اگر (UNEXPECTED(span->used_شمار == span->list_size)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    اگر (span->used_شمار) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      تهی *free_list;
      انجامبده {
        free_list = atomic_exchange_ptr_acquire(&span->free_list_deferred,
                                                INVALID_POINTER);
      } وقتی (free_list == INVALID_POINTER);
      atomic_store_ptr_release(&span->free_list_deferred, free_list);
    }
    _rpmalloc_span_double_link_list_remove(
        &heap->size_class[span->size_class].partial_span, span);
    _rpmalloc_span_release_to_cache(heap, span);
  }
}

ایستا تهی _rpmalloc_deallocate_defer_free_span(heap_t *heap, span_t *span) {
  اگر (span->size_class != SIZE_CLASS_HUGE)
    _rpmalloc_stat_inc(&heap->span_use[span->span_شمار - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  انجامبده {
    span->free_list = (تهی *)atomic_load_ptr(&heap->span_free_deferred);
  } وقتی (!atomic_cas_ptr(&heap->span_free_deferred, span, span->free_list));
}

//! Put the block in the deferred free list of the owning span
ایستا تهی _rpmalloc_deallocate_defer_small_or_medium(span_t *span,
                                                       تهی *block) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  تهی *free_list;
  انجامبده {
    free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } وقتی (free_list == INVALID_POINTER);
  *((تهی **)block) = free_list;
  uint32_t free_شمار = ++span->list_size;
  صحیح all_deferred_free = (free_شمار == span->block_شمار);
  atomic_store_ptr_release(&span->free_list_deferred, block);
  اگر (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
  }
}

ایستا تهی _rpmalloc_deallocate_small_or_medium(span_t *span, تهی *p) {
  _rpmalloc_stat_inc_free(span->heap, span->size_class);
  اگر (span->flags & SPAN_FLAG_ALIGNED_BLOCKS) {
    // Realign pointer to block start
    تهی *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
    p = pointer_offset(p, -(int32_t)(block_offset % span->block_size));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#اگر RPMALLOC_FIRST_CLASS_HEAPS
  صحیح defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#وگرنه
  صحیح defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  اگر (!defer)
    _rpmalloc_deallocate_direct_small_or_medium(span, p);
  وگرنه
    _rpmalloc_deallocate_defer_small_or_medium(span, p);
}

//! Deallocate the given large memory block to the current heap
ایستا تهی _rpmalloc_deallocate_large(span_t *span) {
  rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#اگر RPMALLOC_FIRST_CLASS_HEAPS
  صحیح defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#وگرنه
  صحیح defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  اگر (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    بازگشت;
  }
  rpmalloc_assert(span->heap->full_span_شمار, "Heap span counter corrupted");
  --span->heap->full_span_شمار;
#اگر RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif
#اگر ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = span->span_شمار - 1;
  atomic_decr32(&span->heap->span_use[idx].current);
#endif
  heap_t *heap = span->heap;
  rpmalloc_assert(heap, "No thread heap");
#اگر ENABLE_THREAD_CACHE
  ثابت صحیح set_as_reserved =
      ((span->span_شمار > 1) && (heap->span_cache.شمار == 0) &&
       !heap->finalize && !heap->spans_reserved);
#وگرنه
  ثابت صحیح set_as_reserved =
      ((span->span_شمار > 1) && !heap->finalize && !heap->spans_reserved);
#endif
  اگر (set_as_reserved) {
    heap->span_reserve = span;
    heap->spans_reserved = span->span_شمار;
    اگر (span->flags & SPAN_FLAG_MASTER) {
      heap->span_reserve_master = span;
    } وگرنه { // SPAN_FLAG_SUBSPAN
      span_t *master = (span_t *)pointer_offset(
          span,
          -(intptr_t)((size_t)span->offset_from_master * _memory_span_size));
      heap->span_reserve_master = master;
      rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->remaining_spans) >=
                          (int32_t)span->span_شمار,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&heap->span_use[idx].spans_to_reserved);
  } وگرنه {
    // Insert into cache list
    rpmalloc_heap_cache_درج(heap, span);
  }
}

//! Deallocate the given huge span
ایستا تهی _rpmalloc_deallocate_huge(span_t *span) {
  rpmalloc_assert(span->heap, "No span heap");
#اگر RPMALLOC_FIRST_CLASS_HEAPS
  صحیح defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#وگرنه
  صحیح defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  اگر (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    بازگشت;
  }
  rpmalloc_assert(span->heap->full_span_شمار, "Heap span counter corrupted");
  --span->heap->full_span_شمار;
#اگر RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif

  // Oversized allocation, page count is stored in span_count
  size_t رقم_pages = span->span_شمار;
  _rpmalloc_unmap(span, رقم_pages * _memory_page_size, span->align_offset,
                  رقم_pages * _memory_page_size);
  _rpmalloc_stat_sub(&_huge_pages_current, رقم_pages);
}

//! Deallocate the given block
ایستا تهی _rpmalloc_deallocate(تهی *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  اگر (UNEXPECTED(!span))
    بازگشت;
  اگر (EXPECTED(span->size_class < SIZE_CLASS_شمار))
    _rpmalloc_deallocate_small_or_medium(span, p);
  وگرنه اگر (span->size_class == SIZE_CLASS_LARGE)
    _rpmalloc_deallocate_large(span);
  وگرنه
    _rpmalloc_deallocate_huge(span);
}

////////////
///
/// Reallocation entry points
///
//////

ایستا size_t _rpmalloc_usable_size(تهی *p);

//! Reallocate the given block to the given size
ایستا تهی *_rpmalloc_reallocate(heap_t *heap, تهی *p, size_t size,
                                  size_t oldsize, unsigned صحیح flags) {
  اگر (p) {
    // Grab the span using guaranteed span alignment
    span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
    اگر (EXPECTED(span->size_class < SIZE_CLASS_شمار)) {
      // Small/medium sized block
      rpmalloc_assert(span->span_شمار == 1, "Span counter corrupted");
      تهی *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
      uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
      uint32_t block_idx = block_offset / span->block_size;
      تهی *block =
          pointer_offset(blocks_start, (size_t)block_idx * span->block_size);
      اگر (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)span->block_size - pointer_diff(p, block));
      اگر ((size_t)span->block_size >= size) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        اگر ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        بازگشت block;
      }
    } وگرنه اگر (span->size_class == SIZE_CLASS_LARGE) {
      // Large block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t رقم_spans = total_size >> _memory_span_size_shift;
      اگر (total_size & (_memory_span_mask - 1))
        ++رقم_spans;
      size_t current_spans = span->span_شمار;
      تهی *block = pointer_offset(span, SPAN_HEADER_SIZE);
      اگر (!oldsize)
        oldsize = (current_spans * _memory_span_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      اگر ((current_spans >= رقم_spans) && (total_size >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        اگر ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        بازگشت block;
      }
    } وگرنه {
      // Oversized block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t رقم_pages = total_size >> _memory_page_size_shift;
      اگر (total_size & (_memory_page_size - 1))
        ++رقم_pages;
      // Page count is stored in span_count
      size_t current_pages = span->span_شمار;
      تهی *block = pointer_offset(span, SPAN_HEADER_SIZE);
      اگر (!oldsize)
        oldsize = (current_pages * _memory_page_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      اگر ((current_pages >= رقم_pages) && (رقم_pages >= (current_pages / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        اگر ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        بازگشت block;
      }
    }
  } وگرنه {
    oldsize = 0;
  }

  اگر (!!(flags & RPMALLOC_GROW_OR_FAIL))
    بازگشت 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t lower_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_size =
      (size > lower_bound) ? size : ((size > oldsize) ? lower_bound : size);
  تهی *block = _rpmalloc_allocate(heap, new_size);
  اگر (p && block) {
    اگر (!(flags & RPMALLOC_NO_PRESERVE))
      memcpy(block, p, oldsize < new_size ? oldsize : new_size);
    _rpmalloc_deallocate(p);
  }

  بازگشت block;
}

ایستا تهی *_rpmalloc_aligned_reallocate(heap_t *heap, تهی *ptr,
                                          size_t alignment, size_t size,
                                          size_t oldsize, unsigned صحیح flags) {
  اگر (alignment <= SMALL_GRANULARITY)
    بازگشت _rpmalloc_reallocate(heap, ptr, size, oldsize, flags);

  صحیح no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? _rpmalloc_usable_size(ptr) : 0);
  اگر ((usablesize >= size) && !((uintptr_t)ptr & (alignment - 1))) {
    اگر (no_alloc || (size >= (usablesize / 2)))
      بازگشت ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  تهی *block =
      (!no_alloc ? _rpmalloc_aligned_allocate(heap, alignment, size) : 0);
  اگر (EXPECTED(block != 0)) {
    اگر (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      اگر (!oldsize)
        oldsize = usablesize;
      memcpy(block, ptr, oldsize < size ? oldsize : size);
    }
    _rpmalloc_deallocate(ptr);
  }
  بازگشت block;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
ایستا size_t _rpmalloc_usable_size(تهی *p) {
  // Grab the span using guaranteed span alignment
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  اگر (span->size_class < SIZE_CLASS_شمار) {
    // Small/medium block
    تهی *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    بازگشت span->block_size -
           ((size_t)pointer_diff(p, blocks_start) % span->block_size);
  }
  اگر (span->size_class == SIZE_CLASS_LARGE) {
    // Large block
    size_t current_spans = span->span_شمار;
    بازگشت (current_spans * _memory_span_size) - (size_t)pointer_diff(p, span);
  }
  // Oversized block, page count is stored in span_count
  size_t current_pages = span->span_شمار;
  بازگشت (current_pages * _memory_page_size) - (size_t)pointer_diff(p, span);
}

//! Adjust and optimize the size class properties for the given class
ایستا تهی _rpmalloc_adjust_size_class(size_t iclass) {
  size_t block_size = _memory_size_class[iclass].block_size;
  size_t block_شمار = (_memory_span_size - SPAN_HEADER_SIZE) / block_size;

  _memory_size_class[iclass].block_شمار = (uint16_t)block_شمار;
  _memory_size_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  اگر (iclass >= SMALL_CLASS_شمار) {
    size_t prevclass = iclass;
    وقتی (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      اگر (_memory_size_class[prevclass].block_شمار ==
          _memory_size_class[iclass].block_شمار)
        _rpmalloc_memcpy_const(_memory_size_class + prevclass,
                               _memory_size_class + iclass,
                               اندازه(_memory_size_class[iclass]));
      وگرنه
        بشکن;
    }
  }
}

//! Initialize the allocator and setup global data
بیرونی درونخطی صحیح rpmalloc_initialize(تهی) {
  اگر (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    بازگشت 0;
  }
  بازگشت rpmalloc_initialize_config(0);
}

صحیح rpmalloc_initialize_config(ثابت rpmalloc_config_t *config) {
  اگر (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    بازگشت 0;
  }
  _rpmalloc_initialized = 1;

  اگر (config)
    memcpy(&_memory_config, config, اندازه(rpmalloc_config_t));
  وگرنه
    _rpmalloc_memset_const(&_memory_config, 0, اندازه(rpmalloc_config_t));

  اگر (!_memory_config.memory_map || !_memory_config.memory_unmap) {
    _memory_config.memory_map = _rpmalloc_mmap_os;
    _memory_config.memory_unmap = _rpmalloc_unmap_os;
  }

#اگر PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, اندازه(system_info));
  GetSystemInfo(&system_info);
  _memory_map_granularity = system_info.dwAllocationGranularity;
#وگرنه
  _memory_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#endif

#اگر RPMALLOC_CONFIGURABLE
  _memory_page_size = _memory_config.page_size;
#وگرنه
  _memory_page_size = 0;
#endif
  _memory_huge_pages = 0;
  اگر (!_memory_page_size) {
#اگر PLATFORM_WINDOWS
    _memory_page_size = system_info.dwPageSize;
#وگرنه
    _memory_page_size = _memory_map_granularity;
    اگر (_memory_config.enable_huge_pages) {
#اگر defined(__linux__)
      size_t huge_page_size = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      اگر (meminfo) {
        نویسه سطر_2[128];
        وقتی (!huge_page_size && fgets(سطر_2, اندازه(سطر_2) - 1, meminfo)) {
          سطر_2[اندازه(سطر_2) - 1] = 0;
          اگر (strstr(سطر_2, "Hugepagesize:"))
            huge_page_size = (size_t)strtol(سطر_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      اگر (huge_page_size) {
        _memory_huge_pages = 1;
        _memory_page_size = huge_page_size;
        _memory_map_granularity = huge_page_size;
      }
#وگرنهاگر defined(__FreeBSD__)
      صحیح rc;
      size_t sz = اندازه(rc);

      اگر (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, پوچ, 0) == 0 &&
          rc == 1) {
        ایستا size_t defsize = 2 * 1024 * 1024;
        صحیح nsize = 0;
        size_t sizes[4] = {0};
        _memory_huge_pages = 1;
        _memory_page_size = defsize;
        اگر ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          برای (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            اگر (defsize < csize) {
              _memory_page_size = csize;
              بشکن;
            }
          }
        }
        _memory_map_granularity = _memory_page_size;
      }
#وگرنهاگر defined(__APPLE__) || defined(__NetBSD__)
      _memory_huge_pages = 1;
      _memory_page_size = 2 * 1024 * 1024;
      _memory_map_granularity = _memory_page_size;
#endif
    }
#endif
  } وگرنه {
    اگر (_memory_config.enable_huge_pages)
      _memory_huge_pages = 1;
  }

#اگر PLATFORM_WINDOWS
  اگر (_memory_config.enable_huge_pages) {
    HANDLE token = 0;
    size_t large_page_minimum = GetLargePageMinimum();
    اگر (large_page_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    اگر (token) {
      LUID luid;
      اگر (Lookup_Privilege_مقدار(0, SE_LOCK_MEMORY_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, اندازه(token_privileges));
        token_privileges.Privilege_شمار = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        اگر (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          اگر (GetLastError() == ERROR_SUCCESS)
            _memory_huge_pages = 1;
        }
      }
      CloseHandle(token);
    }
    اگر (_memory_huge_pages) {
      اگر (large_page_minimum > _memory_page_size)
        _memory_page_size = large_page_minimum;
      اگر (large_page_minimum > _memory_map_granularity)
        _memory_map_granularity = large_page_minimum;
    }
  }
#endif

  size_t min_span_size = 256;
  size_t max_page_size;
#اگر UINTPTR_MAX > 0xFFFFFFFF
  max_page_size = 4096ULL * 1024ULL * 1024ULL;
#وگرنه
  max_page_size = 4 * 1024 * 1024;
#endif
  اگر (_memory_page_size < min_span_size)
    _memory_page_size = min_span_size;
  اگر (_memory_page_size > max_page_size)
    _memory_page_size = max_page_size;
  _memory_page_size_shift = 0;
  size_t page_size_bit = _memory_page_size;
  وقتی (page_size_bit != 1) {
    ++_memory_page_size_shift;
    page_size_bit >>= 1;
  }
  _memory_page_size = ((size_t)1 << _memory_page_size_shift);

#اگر RPMALLOC_CONFIGURABLE
  اگر (!_memory_config.span_size) {
    _memory_span_size = _memory_default_span_size;
    _memory_span_size_shift = _memory_default_span_size_shift;
    _memory_span_mask = _memory_default_span_mask;
  } وگرنه {
    size_t span_size = _memory_config.span_size;
    اگر (span_size > (256 * 1024))
      span_size = (256 * 1024);
    _memory_span_size = 4096;
    _memory_span_size_shift = 12;
    وقتی (_memory_span_size < span_size) {
      _memory_span_size <<= 1;
      ++_memory_span_size_shift;
    }
    _memory_span_mask = ~(uintptr_t)(_memory_span_size - 1);
  }
#endif

  memory_span_map_شمار =
      (_memory_config.span_map_شمار ? _memory_config.span_map_شمار
                                     : DEFAULT_SPAN_MAP_شمار);
  اگر ((_memory_span_size * memory_span_map_شمار) < _memory_page_size)
    memory_span_map_شمار = (_memory_page_size / _memory_span_size);
  اگر ((_memory_page_size >= _memory_span_size) &&
      ((memory_span_map_شمار * _memory_span_size) % _memory_page_size))
    memory_span_map_شمار = (_memory_page_size / _memory_span_size);
  memory_heap_reserve_شمار = (memory_span_map_شمار > DEFAULT_SPAN_MAP_شمار)
                                   ? DEFAULT_SPAN_MAP_شمار
                                   : memory_span_map_شمار;

  _memory_config.page_size = _memory_page_size;
  _memory_config.span_size = _memory_span_size;
  _memory_config.span_map_شمار = memory_span_map_شمار;
  _memory_config.enable_huge_pages = _memory_huge_pages;

#اگر ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  اگر (pthread_کلید_create(&_memory_thread_heap, _rpmalloc_heap_release_raw_fc))
    بازگشت -1;
#endif
#اگر defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_کلید = FlsAlloc(&_rpmalloc_thread_destructor);
#endif

  // Setup all small and medium size classes
  size_t iclass = 0;
  _memory_size_class[iclass].block_size = SMALL_GRANULARITY;
  _rpmalloc_adjust_size_class(iclass);
  برای (iclass = 1; iclass < SMALL_CLASS_شمار; ++iclass) {
    size_t size = iclass * SMALL_GRANULARITY;
    _memory_size_class[iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  _memory_medium_size_limit = (_memory_span_size - SPAN_HEADER_SIZE) >> 1;
  اگر (_memory_medium_size_limit > MEDIUM_SIZE_LIMIT)
    _memory_medium_size_limit = MEDIUM_SIZE_LIMIT;
  برای (iclass = 0; iclass < MEDIUM_CLASS_شمار; ++iclass) {
    size_t size = SMALL_SIZE_LIMIT + ((iclass + 1) * MEDIUM_GRANULARITY);
    اگر (size > _memory_medium_size_limit) {
      _memory_medium_size_limit =
          SMALL_SIZE_LIMIT + (iclass * MEDIUM_GRANULARITY);
      بشکن;
    }
    _memory_size_class[SMALL_CLASS_شمار + iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(SMALL_CLASS_شمار + iclass);
  }

  _memory_orphan_heaps = 0;
#اگر RPMALLOC_FIRST_CLASS_HEAPS
  _memory_first_class_orphan_heaps = 0;
#endif
#اگر ENABLE_STATISTICS
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
  memset(_memory_heaps, 0, اندازه(_memory_heaps));
  atomic_store32_release(&_memory_global_lock, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  بازگشت 0;
}

//! Finalize the allocator
تهی rpmalloc_finalize(تهی) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  اگر (_memory_global_reserve) {
    atomic_add32(&_memory_global_reserve_master->remaining_spans,
                 -(int32_t)memory_global_reserve_شمار);
    _memory_global_reserve_master = 0;
    memory_global_reserve_شمار = 0;
    _memory_global_reserve = 0;
  }
  atomic_store32_release(&_memory_global_lock, 0);

  // Free all thread caches and fully free spans
  برای (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    وقتی (heap) {
      heap_t *next_heap = heap->next_heap;
      heap->finalize = 1;
      _rpmalloc_heap_global_finalize(heap);
      heap = next_heap;
    }
  }

#اگر ENABLE_GLOBAL_CACHE
  // Free global caches
  برای (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass)
    _rpmalloc_global_cache_finalize(&_memory_span_cache[iclass]);
#endif

#اگر (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_کلید_delete(_memory_thread_heap);
#endif
#اگر defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_کلید);
  fls_کلید = 0;
#endif
#اگر ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&_mapped_pages) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&_mapped_pages_os) == 0,
                  "Memory leak detected");
#endif

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
بیرونی درونخطی تهی rpmalloc_thread_initialize(تهی) {
  اگر (!get_thread_heap_raw()) {
    heap_t *heap = _rpmalloc_heap_allocate(0);
    اگر (heap) {
      _rpmalloc_stat_inc(&_memory_active_heaps);
      set_thread_heap(heap);
#اگر defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_Set_مقدار(fls_کلید, heap);
#endif
    }
  }
}

//! Finalize thread, orphan heap
تهی rpmalloc_thread_finalize(صحیح release_caches) {
  heap_t *heap = get_thread_heap_raw();
  اگر (heap)
    _rpmalloc_heap_release_raw(heap, release_caches);
  set_thread_heap(0);
#اگر defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_Set_مقدار(fls_کلید, 0);
#endif
}

صحیح rpmalloc_is_thread_initialized(تهی) {
  بازگشت (get_thread_heap_raw() != 0) ? 1 : 0;
}

ثابت rpmalloc_config_t *rpmalloc_config(تهی) { بازگشت &_memory_config; }

// Extern interface

بیرونی درونخطی RPMALLOC_ALLOCATOR تهی *rpmalloc(size_t size) {
#اگر ENABLE_VALIDATE_ARGS
  اگر (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    بازگشت 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  بازگشت _rpmalloc_allocate(heap, size);
}

بیرونی درونخطی تهی rpfree(تهی *ptr) { _rpmalloc_deallocate(ptr); }

بیرونی درونخطی RPMALLOC_ALLOCATOR تهی *rpcalloc(size_t رقم_2, size_t size) {
  size_t total;
#اگر ENABLE_VALIDATE_ARGS
#اگر PLATFORM_WINDOWS
  صحیح err = SizeTMult(رقم_2, size, &total);
  اگر ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    بازگشت 0;
  }
#وگرنه
  صحیح err = __builtin_umull_overflow(رقم_2, size, &total);
  اگر (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    بازگشت 0;
  }
#endif
#وگرنه
  total = رقم_2 * size;
#endif
  heap_t *heap = get_thread_heap();
  تهی *block = _rpmalloc_allocate(heap, total);
  اگر (block)
    memset(block, 0, total);
  بازگشت block;
}

بیرونی درونخطی RPMALLOC_ALLOCATOR تهی *rprealloc(تهی *ptr, size_t size) {
#اگر ENABLE_VALIDATE_ARGS
  اگر (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    بازگشت ptr;
  }
#endif
  heap_t *heap = get_thread_heap();
  بازگشت _rpmalloc_reallocate(heap, ptr, size, 0, 0);
}

بیرونی RPMALLOC_ALLOCATOR تهی *rpaligned_realloc(تهی *ptr, size_t alignment,
                                                  size_t size, size_t oldsize,
                                                  unsigned صحیح flags) {
#اگر ENABLE_VALIDATE_ARGS
  اگر ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    بازگشت 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  بازگشت _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, oldsize,
                                      flags);
}

بیرونی RPMALLOC_ALLOCATOR تهی *rpaligned_alloc(size_t alignment, size_t size) {
  heap_t *heap = get_thread_heap();
  بازگشت _rpmalloc_aligned_allocate(heap, alignment, size);
}

بیرونی درونخطی RPMALLOC_ALLOCATOR تهی *
rpaligned_calloc(size_t alignment, size_t رقم_2, size_t size) {
  size_t total;
#اگر ENABLE_VALIDATE_ARGS
#اگر PLATFORM_WINDOWS
  صحیح err = SizeTMult(رقم_2, size, &total);
  اگر ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    بازگشت 0;
  }
#وگرنه
  صحیح err = __builtin_umull_overflow(رقم_2, size, &total);
  اگر (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    بازگشت 0;
  }
#endif
#وگرنه
  total = رقم_2 * size;
#endif
  تهی *block = rpaligned_alloc(alignment, total);
  اگر (block)
    memset(block, 0, total);
  بازگشت block;
}

بیرونی درونخطی RPMALLOC_ALLOCATOR تهی *rpmemalign(size_t alignment,
                                                  size_t size) {
  بازگشت rpaligned_alloc(alignment, size);
}

بیرونی درونخطی صحیح rpposix_memalign(تهی **memptr, size_t alignment,
                                   size_t size) {
  اگر (memptr)
    *memptr = rpaligned_alloc(alignment, size);
  وگرنه
    بازگشت EINVAL;
  بازگشت *memptr ? 0 : ENOMEM;
}

بیرونی درونخطی size_t rpmalloc_usable_size(تهی *ptr) {
  بازگشت (ptr ? _rpmalloc_usable_size(ptr) : 0);
}

بیرونی درونخطی تهی rpmalloc_thread_collect(تهی) {}

تهی rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, اندازه(rpmalloc_thread_statistics_t));
  heap_t *heap = get_thread_heap_raw();
  اگر (!heap)
    بازگشت;

  برای (size_t iclass = 0; iclass < SIZE_CLASS_شمار; ++iclass) {
    size_class_t *size_class = _memory_size_class + iclass;
    span_t *span = heap->size_class[iclass].partial_span;
    وقتی (span) {
      size_t free_شمار = span->list_size;
      size_t block_شمار = size_class->block_شمار;
      اگر (span->free_list_limit < block_شمار)
        block_شمار = span->free_list_limit;
      free_شمار += (block_شمار - span->used_شمار);
      stats->sizecache += free_شمار * size_class->block_size;
      span = span->next;
    }
  }

#اگر ENABLE_THREAD_CACHE
  برای (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    span_cache_t *span_cache;
    اگر (!iclass)
      span_cache = &heap->span_cache;
    وگرنه
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    stats->spancache += span_cache->شمار * (iclass + 1) * _memory_span_size;
  }
#endif

  span_t *deferred = (span_t *)atomic_load_ptr(&heap->span_free_deferred);
  وقتی (deferred) {
    اگر (deferred->size_class != SIZE_CLASS_HUGE)
      stats->spancache += (size_t)deferred->span_شمار * _memory_span_size;
    deferred = (span_t *)deferred->free_list;
  }

#اگر ENABLE_STATISTICS
  stats->thread_to_global = (size_t)atomic_load64(&heap->thread_to_global);
  stats->global_to_thread = (size_t)atomic_load64(&heap->global_to_thread);

  برای (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    stats->span_use[iclass].current =
        (size_t)atomic_load32(&heap->span_use[iclass].current);
    stats->span_use[iclass].peak =
        (size_t)atomic_load32(&heap->span_use[iclass].بالا);
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
  برای (size_t iclass = 0; iclass < SIZE_CLASS_شمار; ++iclass) {
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

تهی rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, اندازه(rpmalloc_global_statistics_t));
#اگر ENABLE_STATISTICS
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
#اگر ENABLE_GLOBAL_CACHE
  برای (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    global_cache_t *cache = &_memory_span_cache[iclass];
    وقتی (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();
    uint32_t شمار = cache->شمار;
#اگر ENABLE_UNLIMITED_CACHE
    span_t *current_span = cache->overflow;
    وقتی (current_span) {
      ++شمار;
      current_span = current_span->next;
    }
#endif
    atomic_store32_release(&cache->lock, 0);
    stats->cached += شمار * (iclass + 1) * _memory_span_size;
  }
#endif
}

#اگر ENABLE_STATISTICS

ایستا تهی _memory_heap_dump_statistics(heap_t *heap, تهی *file) {
  fprintf(file, "Heap %d stats:\n", heap->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  برای (size_t iclass = 0; iclass < SIZE_CLASS_شمار; ++iclass) {
    اگر (!atomic_load32(&heap->size_class_use[iclass].alloc_total))
      ادامه;
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
        _memory_size_class[iclass].block_شمار,
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
  برای (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    اگر (!atomic_load32(&heap->span_use[iclass].بالا) &&
        !atomic_load32(&heap->span_use[iclass].spans_map_calls))
      ادامه;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&heap->span_use[iclass].current),
        atomic_load32(&heap->span_use[iclass].بالا),
        atomic_load32(&heap->span_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&heap->span_use[iclass].بالا) *
         (size_t)_memory_span_size * (iclass + 1)) /
            (size_t)(1024 * 1024),
#اگر ENABLE_THREAD_CACHE
        (unsigned صحیح)(!iclass ? heap->span_cache.شمار
                               : heap->span_large_cache[iclass - 1].شمار),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_to_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_from_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
#وگرنه
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
  fprintf(file, "Full spans: %zu\n", heap->full_span_شمار);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&heap->thread_to_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&heap->global_to_thread) / (size_t)(1024 * 1024));
}

#endif

تهی rpmalloc_dump_statistics(تهی *file) {
#اگر ENABLE_STATISTICS
  برای (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    وقتی (heap) {
      صحیح need_dump = 0;
      برای (size_t iclass = 0; !need_dump && (iclass < SIZE_CLASS_شمار);
           ++iclass) {
        اگر (!atomic_load32(&heap->size_class_use[iclass].alloc_total)) {
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].free_total),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          ادامه;
        }
        need_dump = 1;
      }
      برای (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_شمار);
           ++iclass) {
        اگر (!atomic_load32(&heap->span_use[iclass].بالا) &&
            !atomic_load32(&heap->span_use[iclass].spans_map_calls))
          ادامه;
        need_dump = 1;
      }
      اگر (need_dump)
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

#اگر ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  برای (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    global_cache_t *cache = _memory_span_cache + iclass;
    size_t global_cache = (size_t)cache->شمار * iclass * _memory_span_size;

    size_t global_overflow_cache = 0;
    span_t *span = cache->overflow;
    وقتی (span) {
      global_overflow_cache += iclass * _memory_span_size;
      span = span->next;
    }
    اگر (global_cache || global_overflow_cache || cache->درج_شمار ||
        cache->extract_شمار)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_overflow_cache / (size_t)(1024 * 1024),
              cache->درج_شمار, cache->extract_شمار);
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
#اگر 0
	int64_t allocated = atomic_load64(&_allocation_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#endif
#endif
  (تهی)اندازه(file);
}

#اگر RPMALLOC_FIRST_CLASS_HEAPS

بیرونی درونخطی rpmalloc_heap_t *rpmalloc_heap_acquire(تهی) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  heap_t *heap = _rpmalloc_heap_allocate(1);
  rpmalloc_assume(heap != پوچ);
  heap->owner_thread = 0;
  _rpmalloc_stat_inc(&_memory_active_heaps);
  بازگشت heap;
}

بیرونی درونخطی تهی rpmalloc_heap_release(rpmalloc_heap_t *heap) {
  اگر (heap)
    _rpmalloc_heap_release(heap, 1, 1);
}

بیرونی درونخطی RPMALLOC_ALLOCATOR تهی *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, size_t size) {
#اگر ENABLE_VALIDATE_ARGS
  اگر (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    بازگشت 0;
  }
#endif
  بازگشت _rpmalloc_allocate(heap, size);
}

بیرونی درونخطی RPMALLOC_ALLOCATOR تهی *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, size_t alignment,
                            size_t size) {
#اگر ENABLE_VALIDATE_ARGS
  اگر (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    بازگشت 0;
  }
#endif
  بازگشت _rpmalloc_aligned_allocate(heap, alignment, size);
}

بیرونی درونخطی RPMALLOC_ALLOCATOR تهی *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, size_t رقم_2, size_t size) {
  بازگشت rpmalloc_heap_aligned_calloc(heap, 0, رقم_2, size);
}

بیرونی درونخطی RPMALLOC_ALLOCATOR تهی *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, size_t alignment,
                             size_t رقم_2, size_t size) {
  size_t total;
#اگر ENABLE_VALIDATE_ARGS
#اگر PLATFORM_WINDOWS
  صحیح err = SizeTMult(رقم_2, size, &total);
  اگر ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    بازگشت 0;
  }
#وگرنه
  صحیح err = __builtin_umull_overflow(رقم_2, size, &total);
  اگر (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    بازگشت 0;
  }
#endif
#وگرنه
  total = رقم_2 * size;
#endif
  تهی *block = _rpmalloc_aligned_allocate(heap, alignment, total);
  اگر (block)
    memset(block, 0, total);
  بازگشت block;
}

بیرونی درونخطی RPMALLOC_ALLOCATOR تهی *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, تهی *ptr, size_t size,
                      unsigned صحیح flags) {
#اگر ENABLE_VALIDATE_ARGS
  اگر (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    بازگشت ptr;
  }
#endif
  بازگشت _rpmalloc_reallocate(heap, ptr, size, 0, flags);
}

بیرونی درونخطی RPMALLOC_ALLOCATOR تهی *
rpmalloc_heap_aligned_realloc(rpmalloc_heap_t *heap, تهی *ptr,
                              size_t alignment, size_t size,
                              unsigned صحیح flags) {
#اگر ENABLE_VALIDATE_ARGS
  اگر ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    بازگشت 0;
  }
#endif
  بازگشت _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, 0, flags);
}

بیرونی درونخطی تهی rpmalloc_heap_free(rpmalloc_heap_t *heap, تهی *ptr) {
  (تهی)اندازه(heap);
  _rpmalloc_deallocate(ptr);
}

بیرونی درونخطی تهی rpmalloc_heap_free_all(rpmalloc_heap_t *heap) {
  span_t *span;
  span_t *next_span;

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  برای (size_t iclass = 0; iclass < SIZE_CLASS_شمار; ++iclass) {
    span = heap->size_class[iclass].partial_span;
    وقتی (span) {
      next_span = span->next;
      rpmalloc_heap_cache_درج(heap, span);
      span = next_span;
    }
    heap->size_class[iclass].partial_span = 0;
    span = heap->full_span[iclass];
    وقتی (span) {
      next_span = span->next;
      rpmalloc_heap_cache_درج(heap, span);
      span = next_span;
    }

    span = heap->size_class[iclass].cache;
    اگر (span)
      rpmalloc_heap_cache_درج(heap, span);
    heap->size_class[iclass].cache = 0;
  }
  memset(heap->size_class, 0, اندازه(heap->size_class));
  memset(heap->full_span, 0, اندازه(heap->full_span));

  span = heap->large_huge_span;
  وقتی (span) {
    next_span = span->next;
    اگر (UNEXPECTED(span->size_class == SIZE_CLASS_HUGE))
      _rpmalloc_deallocate_huge(span);
    وگرنه
      rpmalloc_heap_cache_درج(heap, span);
    span = next_span;
  }
  heap->large_huge_span = 0;
  heap->full_span_شمار = 0;

#اگر ENABLE_THREAD_CACHE
  برای (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    span_cache_t *span_cache;
    اگر (!iclass)
      span_cache = &heap->span_cache;
    وگرنه
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    اگر (!span_cache->شمار)
      ادامه;
#اگر ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&heap->thread_to_global,
                         span_cache->شمار * (iclass + 1) * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                       span_cache->شمار);
    rpmalloc_global_cache_درج_spans(span_cache->span, iclass + 1,
                                        span_cache->شمار);
#وگرنه
    برای (size_t ispan = 0; ispan < span_cache->شمار; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
    span_cache->شمار = 0;
  }
#endif

#اگر ENABLE_STATISTICS
  برای (size_t iclass = 0; iclass < SIZE_CLASS_شمار; ++iclass) {
    atomic_store32(&heap->size_class_use[iclass].alloc_current, 0);
    atomic_store32(&heap->size_class_use[iclass].spans_current, 0);
  }
  برای (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    atomic_store32(&heap->span_use[iclass].current, 0);
  }
#endif
}

بیرونی درونخطی تهی rpmalloc_heap_thread_set_current(rpmalloc_heap_t *heap) {
  heap_t *prev_heap = get_thread_heap_raw();
  اگر (prev_heap != heap) {
    set_thread_heap(heap);
    اگر (prev_heap)
      rpmalloc_heap_release(prev_heap);
  }
}

بیرونی درونخطی rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(تهی *ptr) {
  // Grab the span, and then the heap from the span
  span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
  اگر (span) {
    بازگشت span->heap;
  }
  بازگشت 0;
}

#endif

#اگر ENABLE_PRELOAD || ENABLE_OVERRIDE

#شامل "malloc.c"

#endif

تهی rpmalloc_linker_reference(تهی) { (تهی)اندازه(_rpmalloc_initialized); }
