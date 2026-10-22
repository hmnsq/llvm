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

#ضمن "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#إذا defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wunused-function"
#إذا __has_warning("-Wreserved-identifier")
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
#إذا __has_warning("-Wstatic-in-inline")
#pragma clang diagnostic ignored "-Wstatic-in-inline"
#endif
#وإلاإذا defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#إذا !defined(__has_builtin)
#define __has_builtin(b) 0
#endif

#إذا defined(__GNUC__) || defined(__clang__)

#إذا __has_builtin(__builtin_memcpy_inline)
#define _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#وإلا
#define _rpmalloc_memcpy_const(x, y, s)                                        \
  افعل {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memcpy(x, y, s);                                                           \
  } بينما (0)
#endif

#إذا __has_builtin(__builtin_memset_inline)
#define _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#وإلا
#define _rpmalloc_memset_const(x, y, s)                                        \
  افعل {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } بينما (0)
#endif
#وإلا
#define _rpmalloc_memcpy_const(x, y, s) memcpy(x, y, s)
#define _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#endif

#إذا __has_builtin(__builtin_assume)
#define rpmalloc_assume(cond) __builtin_assume(cond)
#وإلاإذا defined(__GNUC__)
#define rpmalloc_assume(cond)                                                  \
  افعل {                                                                         \
    إذا (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } بينما (0)
#وإلاإذا defined(_MSC_VER)
#define rpmalloc_assume(cond) __assume(cond)
#وإلا
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
#ifndef DEFAULT_SPAN_MAP_تعداد
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#define DEFAULT_SPAN_MAP_تعداد 64
#endif
#ifndef GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#define GLOBAL_CACHE_MULTIPLIER 8
#endif

#إذا DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#error Must use global cache إذا unmap is disabled
#endif

#إذا DISABLE_UNMAP
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 1
#endif

#إذا !ENABLE_GLOBAL_CACHE
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 0
#endif

#إذا !ENABLE_THREAD_CACHE
#undef ENABLE_ADAPTIVE_THREAD_CACHE
#define ENABLE_ADAPTIVE_THREAD_CACHE 0
#endif

#إذا defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#define PLATFORM_POSIX 0
#وإلا
#define PLATFORM_WINDOWS 0
#define PLATFORM_POSIX 1
#endif

/// Platform and arch specifics
#إذا defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 5105)
#ifndef FORCEINLINE
#define FORCEINLINE مضمن __forceinline
#endif
#define _Static_assert static_assert
#وإلا
#ifndef FORCEINLINE
#define FORCEINLINE مضمن __attribute__((__always_inline__))
#endif
#endif
#إذا PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ضمن <windows.h>
#إذا ENABLE_VALIDATE_ARGS
#ضمن <intsafe.h>
#endif
#وإلا
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <time.h>
#ضمن <unistd.h>
#إذا defined(__linux__) || defined(__ANDROID__)
#ضمن <sys/prctl.h>
#إذا !defined(PR_SET_VMA)
#define PR_SET_VMA 0x53564d41
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif
#إذا defined(__APPLE__)
#ضمن <TargetConditionals.h>
#إذا !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#ضمن <mach/mach_vm.h>
#ضمن <mach/vm_statistics.h>
#endif
#ضمن <pthread.h>
#endif
#إذا defined(__HAIKU__) || defined(__TINYC__)
#ضمن <pthread.h>
#endif
#endif

#ضمن <errno.h>
#ضمن <stdint.h>
#ضمن <string.h>

#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#ضمن <fibersapi.h>
ساكن DWORD fls_مفتاح;
#endif

#إذا PLATFORM_POSIX
#ضمن <sched.h>
#ضمن <sys/mman.h>
#ifdef __FreeBSD__
#ضمن <sys/sysctl.h>
#define MAP_HUGETLB MAP_ALIGNED_SUPER
#ifndef PROT_MAX
#define PROT_MAX(f) 0
#endif
#وإلا
#define PROT_MAX(f) 0
#endif
#ifdef __sun
خارجي صحيح madvise(caddr_t, size_t, صحيح);
#endif
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif
#endif
#ضمن <errno.h>

#إذا ENABLE_ASSERTS
#undef NDEBUG
#إذا defined(_MSC_VER) && !defined(_DEBUG)
#define _DEBUG
#endif
#ضمن <assert.h>
#define RPMALLOC_TOSTRING_M(x) #x
#define RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#define rpmalloc_assert(truth, message)                                        \
  افعل {                                                                         \
    إذا (!(truth)) {                                                            \
      إذا (_memory_config.error_callback) {                                     \
        _memory_config.error_callback(message " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } وإلا {                                                                 \
        أكد((truth) && message);                                            \
      }                                                                        \
    }                                                                          \
  } بينما (0)
#وإلا
#define rpmalloc_assert(truth, message)                                        \
  افعل {                                                                         \
  } بينما (0)
#endif
#إذا ENABLE_STATISTICS
#ضمن <stdio.h>
#endif

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#إذا defined(_MSC_VER) && !defined(__clang__)

تعريفنوع متطاير long atomic32_t;
تعريفنوع متطاير long long atomic64_t;
تعريفنوع متطاير فراغ *atomicptr_t;

ساكن FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  إرجاع (int32_t)InterlockedOr(src, 0);
}
ساكن FORCEINLINE فراغ atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ساكن FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  إرجاع (int32_t)InterlockedIncrement(val);
}
ساكن FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  إرجاع (int32_t)InterlockedDecrement(val);
}
ساكن FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  إرجاع (int32_t)InterlockedExchangeAdd(val, add) + add;
}
ساكن FORCEINLINE صحيح atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  إرجاع (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
ساكن FORCEINLINE فراغ atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ساكن FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  إرجاع (int64_t)InterlockedOr64(src, 0);
}
ساكن FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  إرجاع (int64_t)InterlockedExchangeAdd64(val, add) + add;
}
ساكن FORCEINLINE فراغ *atomic_load_ptr(atomicptr_t *src) {
  إرجاع InterlockedCompareExchangePointer(src, 0, 0);
}
ساكن FORCEINLINE فراغ atomic_store_ptr(atomicptr_t *dst, فراغ *val) {
  InterlockedExchangePointer(dst, val);
}
ساكن FORCEINLINE فراغ atomic_store_ptr_release(atomicptr_t *dst, فراغ *val) {
  InterlockedExchangePointer(dst, val);
}
ساكن FORCEINLINE فراغ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     فراغ *val) {
  إرجاع (فراغ *)InterlockedExchangePointer((فراغ *متطاير *)dst, val);
}
ساكن FORCEINLINE صحيح atomic_cas_ptr(atomicptr_t *dst, فراغ *val, فراغ *ref) {
  إرجاع (InterlockedCompareExchangePointer((فراغ *متطاير *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#define EXPECTED(x) (x)
#define UNEXPECTED(x) (x)

#وإلا

#ضمن <stdatomic.h>

تعريفنوع متطاير _Atomic(int32_t) atomic32_t;
تعريفنوع متطاير _Atomic(int64_t) atomic64_t;
تعريفنوع متطاير _Atomic(فراغ *) atomicptr_t;

ساكن FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  إرجاع atomic_load_explicit(src, memory_order_relaxed);
}
ساكن FORCEINLINE فراغ atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ساكن FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  إرجاع atomic_fetch_add_explicit(val, 1, memory_order_relaxed) + 1;
}
ساكن FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  إرجاع atomic_fetch_add_explicit(val, -1, memory_order_relaxed) - 1;
}
ساكن FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  إرجاع atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
ساكن FORCEINLINE صحيح atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  إرجاع atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
ساكن FORCEINLINE فراغ atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ساكن FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  إرجاع atomic_load_explicit(val, memory_order_relaxed);
}
ساكن FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  إرجاع atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
ساكن FORCEINLINE فراغ *atomic_load_ptr(atomicptr_t *src) {
  إرجاع atomic_load_explicit(src, memory_order_relaxed);
}
ساكن FORCEINLINE فراغ atomic_store_ptr(atomicptr_t *dst, فراغ *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ساكن FORCEINLINE فراغ atomic_store_ptr_release(atomicptr_t *dst, فراغ *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ساكن FORCEINLINE فراغ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     فراغ *val) {
  إرجاع atomic_exchange_explicit(dst, val, memory_order_acquire);
}
ساكن FORCEINLINE صحيح atomic_cas_ptr(atomicptr_t *dst, فراغ *val, فراغ *ref) {
  إرجاع atomic_compare_exchange_weak_explicit(
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

#إذا ENABLE_STATISTICS
#define _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#define _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#define _rpmalloc_stat_add(counter, قيمة_2)                                     \
  atomic_add32(counter, (int32_t)(قيمة_2))
#define _rpmalloc_stat_add64(counter, قيمة_2)                                   \
  atomic_add64(counter, (int64_t)(قيمة_2))
#define _rpmalloc_stat_add_peak(counter, قيمة_2, peak)                          \
  افعل {                                                                         \
    int32_t cur_تعداد = atomic_add32(counter, (int32_t)(قيمة_2));              \
    إذا (cur_تعداد > (peak))                                                   \
      peak = cur_تعداد;                                                       \
  } بينما (0)
#define _rpmalloc_stat_sub(counter, قيمة_2)                                     \
  atomic_add32(counter, -(int32_t)(قيمة_2))
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  افعل {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&heap->size_class_use[class_idx].alloc_current);         \
    إذا (alloc_current > heap->size_class_use[class_idx].alloc_peak)            \
      heap->size_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&heap->size_class_use[class_idx].alloc_total);               \
  } بينما (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  افعل {                                                                         \
    atomic_decr32(&heap->size_class_use[class_idx].alloc_current);             \
    atomic_incr32(&heap->size_class_use[class_idx].free_total);                \
  } بينما (0)
#وإلا
#define _rpmalloc_stat_inc(counter)                                            \
  افعل {                                                                         \
  } بينما (0)
#define _rpmalloc_stat_dec(counter)                                            \
  افعل {                                                                         \
  } بينما (0)
#define _rpmalloc_stat_add(counter, قيمة_2)                                     \
  افعل {                                                                         \
  } بينما (0)
#define _rpmalloc_stat_add64(counter, قيمة_2)                                   \
  افعل {                                                                         \
  } بينما (0)
#define _rpmalloc_stat_add_peak(counter, قيمة_2, peak)                          \
  افعل {                                                                         \
  } بينما (0)
#define _rpmalloc_stat_sub(counter, قيمة_2)                                     \
  افعل {                                                                         \
  } بينما (0)
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  افعل {                                                                         \
  } بينما (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  افعل {                                                                         \
  } بينما (0)
#endif

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#define SMALL_GRANULARITY 16
//! Small granularity shift count
#define SMALL_GRANULARITY_SHIFT 4
//! Number of small block size classes
#define SMALL_CLASS_تعداد 65
//! Maximum size of a small block
#define SMALL_SIZE_LIMIT (SMALL_GRANULARITY * (SMALL_CLASS_تعداد - 1))
//! Granularity of a medium allocation block
#define MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#define MEDIUM_GRANULARITY_SHIFT 9
//! Number of medium block size classes
#define MEDIUM_CLASS_تعداد 61
//! Total number of small + medium size classes
#define SIZE_CLASS_تعداد (SMALL_CLASS_تعداد + MEDIUM_CLASS_تعداد)
//! Number of large block size classes
#define LARGE_CLASS_تعداد 63
//! Maximum size of a medium block
#define MEDIUM_SIZE_LIMIT                                                      \
  (SMALL_SIZE_LIMIT + (MEDIUM_GRANULARITY * MEDIUM_CLASS_تعداد))
//! Maximum size of a large block
#define LARGE_SIZE_LIMIT                                                       \
  ((LARGE_CLASS_تعداد * _memory_span_size) - SPAN_HEADER_SIZE)
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

#إذا ENABLE_VALIDATE_ARGS
//! Maximum allocation size to avoid integer overflow
#undef MAX_ALLOC_SIZE
#define MAX_ALLOC_SIZE (((size_t) - 1) - _memory_span_size)
#endif

#define pointer_offset(ptr, ofs) (فراغ *)((حرف *)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff(first, second)                                            \
  (ptrdiff_t)((ثابت حرف *)(first) - (ثابت حرف *)(second))

#define INVALID_POINTER ((فراغ *)((uintptr_t) - 1))

#define SIZE_CLASS_LARGE SIZE_CLASS_تعداد
#define SIZE_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
تعريفنوع بنية heap_t heap_t;
//! Span of memory pages
تعريفنوع بنية span_t span_t;
//! Span list
تعريفنوع بنية span_list_t span_list_t;
//! Span active data
تعريفنوع بنية span_active_t span_active_t;
//! Size class definition
تعريفنوع بنية size_class_t size_class_t;
//! Global cache
تعريفنوع بنية global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#define SPAN_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#define SPAN_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#define SPAN_FLAG_ALIGNED_BLOCKS 4U
//! Flag indicating an unmapped master span
#define SPAN_FLAG_UNMAPPED_MASTER 8U

#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
بنية span_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t أعلى;
#إذا ENABLE_STATISTICS
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
تعريفنوع بنية span_use_t span_use_t;
#endif

#إذا ENABLE_STATISTICS
بنية size_class_use_t {
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
تعريفنوع بنية size_class_use_t size_class_use_t;
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
بنية span_t {
  //! Free list
  فراغ *free_list;
  //! Total block count of size class
  uint32_t block_تعداد;
  //! Size class
  uint32_t size_class;
  //! Index of last block initialized in free list
  uint32_t free_list_limit;
  //! Number of used blocks remaining when in partial state
  uint32_t used_تعداد;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_size;
  //! Size of a block
  uint32_t block_size;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t span_تعداد;
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
_Static_assert(حجم(span_t) <= SPAN_HEADER_SIZE, "span size mismatch");

بنية span_cache_t {
  size_t تعداد_2;
  span_t *span[MAX_THREAD_SPAN_CACHE];
};
تعريفنوع بنية span_cache_t span_cache_t;

بنية span_large_cache_t {
  size_t تعداد_2;
  span_t *span[MAX_THREAD_SPAN_LARGE_CACHE];
};
تعريفنوع بنية span_large_cache_t span_large_cache_t;

بنية heap_size_class_t {
  //! Free list of active span
  فراغ *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  span_t *partial_span;
  //! Early level cache of fully free spans
  span_t *cache;
};
تعريفنوع بنية heap_size_class_t heap_size_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
بنية heap_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  heap_size_class_t size_class[SIZE_CLASS_تعداد];
#إذا ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  span_cache_t span_cache;
#endif
  //! List of deferred free spans (single linked list)
  atomicptr_t span_free_deferred;
  //! Number of full spans
  size_t full_span_تعداد;
  //! Mapped but unused spans
  span_t *span_reserve;
  //! Master span for mapped but unused spans
  span_t *span_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t child_تعداد;
  //! Next heap in id list
  heap_t *next_heap;
  //! Next heap in orphan list
  heap_t *next_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  صحيح finalize;
  //! Master heap owning the memory pages
  heap_t *master_heap;
#إذا ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  span_large_cache_t span_large_cache[LARGE_CLASS_تعداد - 1];
#endif
#إذا RPMALLOC_FIRST_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  span_t *full_span[SIZE_CLASS_تعداد];
  //! Double linked list of large and huge spans allocated by this heap
  span_t *large_huge_span;
#endif
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  span_use_t span_use[LARGE_CLASS_تعداد];
#endif
#إذا ENABLE_STATISTICS
  //! Allocation stats per size class
  size_class_use_t size_class_use[SIZE_CLASS_تعداد + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_to_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_to_thread;
#endif
};

// Size class for defining a block size bucket
بنية size_class_t {
  //! Size of blocks in this class
  uint32_t block_size;
  //! Number of blocks in each chunk
  uint16_t block_تعداد;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(حجم(size_class_t) == 8, "Size class size mismatch");

بنية global_cache_t {
  //! Cache lock
  atomic32_t lock;
  //! Cache count
  uint32_t تعداد_2;
#إذا ENABLE_STATISTICS
  //! Insert count
  size_t أدرج_تعداد;
  //! Extract count
  size_t extract_تعداد;
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
ساكن صحيح _rpmalloc_initialized;
//! Main thread ID
ساكن uintptr_t _rpmalloc_main_thread_id;
//! Configuration
ساكن rpmalloc_config_t _memory_config;
//! Memory page size
ساكن size_t _memory_page_size;
//! Shift to divide by page size
ساكن size_t _memory_page_size_shift;
//! Granularity at which memory pages are mapped by OS
ساكن size_t _memory_map_granularity;
#إذا RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
ساكن size_t _memory_span_size;
//! Shift to divide by span size
ساكن size_t _memory_span_size_shift;
//! Mask to get to start of a memory span
ساكن uintptr_t _memory_span_mask;
#وإلا
//! Hardwired span size
#define _memory_span_size _memory_default_span_size
#define _memory_span_size_shift _memory_default_span_size_shift
#define _memory_span_mask _memory_default_span_mask
#endif
//! Number of spans to map in each map call
ساكن size_t memory_span_map_تعداد;
//! Number of spans to keep reserved in each heap
ساكن size_t memory_heap_reserve_تعداد;
//! Global size classes
ساكن size_class_t _memory_size_class[SIZE_CLASS_تعداد];
//! Run-time size limit of medium blocks
ساكن size_t _memory_medium_size_limit;
//! Heap ID counter
ساكن atomic32_t _memory_heap_id;
//! Huge page support
ساكن صحيح _memory_huge_pages;
#إذا ENABLE_GLOBAL_CACHE
//! Global span cache
ساكن global_cache_t _memory_span_cache[LARGE_CLASS_تعداد];
#endif
//! Global reserved spans
ساكن span_t *_memory_global_reserve;
//! Global reserved count
ساكن size_t memory_global_reserve_تعداد;
//! Global reserved master
ساكن span_t *_memory_global_reserve_master;
//! All heaps
ساكن heap_t *_memory_heaps[HEAP_ARRAY_SIZE];
//! Used to restrict access to mapping memory for huge pages
ساكن atomic32_t _memory_global_lock;
//! Orphaned heaps
ساكن heap_t *_memory_orphan_heaps;
#إذا RPMALLOC_FIRST_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
ساكن heap_t *_memory_first_class_orphan_heaps;
#endif
#إذا ENABLE_STATISTICS
//! Allocations counter
ساكن atomic64_t _allocation_counter;
//! Deallocations counter
ساكن atomic64_t _deallocation_counter;
//! Active heap count
ساكن atomic32_t _memory_active_heaps;
//! Number of currently mapped memory pages
ساكن atomic32_t _mapped_pages;
//! Peak number of concurrently mapped memory pages
ساكن int32_t _mapped_pages_peak;
//! Number of mapped master spans
ساكن atomic32_t _master_spans;
//! Number of unmapped dangling master spans
ساكن atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
ساكن atomic32_t _mapped_total;
//! Running counter of total number of unmapped memory pages since start
ساكن atomic32_t _unmapped_total;
//! Number of currently mapped memory pages in OS calls
ساكن atomic32_t _mapped_pages_os;
//! Number of currently allocated pages in huge allocations
ساكن atomic32_t _huge_pages_current;
//! Peak number of currently allocated pages in huge allocations
ساكن int32_t _huge_pages_peak;
#endif

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
ساكن pthread_key_t _memory_thread_heap;
#وإلا
#ifdef _MSC_VER
#define _Thread_local __declspec(thread)
#define TLS_MODEL
#وإلا
#ifndef __HAIKU__
#define TLS_MODEL __attribute__((tls_model("initial-exec")))
#وإلا
#define TLS_MODEL
#endif
#إذا !defined(__clang__) && defined(__GNUC__)
#define _Thread_local __thread
#endif
#endif
ساكن _Thread_local heap_t *_memory_thread_heap TLS_MODEL;
#endif

ساكن مضمن heap_t *get_thread_heap_raw(فراغ) {
#إذا (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  إرجاع pthread_getspecific(_memory_thread_heap);
#وإلا
  إرجاع _memory_thread_heap;
#endif
}

//! Get the current thread heap
ساكن مضمن heap_t *get_thread_heap(فراغ) {
  heap_t *heap = get_thread_heap_raw();
#إذا ENABLE_PRELOAD
  إذا (EXPECTED(heap != 0))
    إرجاع heap;
  rpmalloc_initialize();
  إرجاع get_thread_heap_raw();
#وإلا
  إرجاع heap;
#endif
}

//! Fast thread ID
ساكن مضمن uintptr_t get_thread_id(فراغ) {
#إذا defined(_WIN32)
  إرجاع (uintptr_t)((فراغ *)NtCurrentTeb());
#وإلاإذا (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#إذا defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#وإلاإذا defined(__x86_64__)
#إذا defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#وإلا
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#endif
#وإلاإذا defined(__arm__)
  __asm__ متطاير("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#وإلاإذا defined(__aarch64__)
#إذا defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ متطاير("mrs %0, tpidrro_el0" : "=r"(tid));
#وإلا
  __asm__ متطاير("mrs %0, tpidr_el0" : "=r"(tid));
#endif
#وإلا
#error This platform needs implementation of get_thread_id()
#endif
  إرجاع tid;
#وإلا
#error This platform needs implementation of get_thread_id()
#endif
}

//! Set the current thread heap
ساكن فراغ set_thread_heap(heap_t *heap) {
#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(_memory_thread_heap, heap);
#وإلا
  _memory_thread_heap = heap;
#endif
  إذا (heap)
    heap->owner_thread = get_thread_id();
}

//! Set main thread ID
خارجي فراغ rpmalloc_set_main_thread(فراغ);

فراغ rpmalloc_set_main_thread(فراغ) {
  _rpmalloc_main_thread_id = get_thread_id();
}

ساكن فراغ _rpmalloc_spin(فراغ) {
#إذا defined(_MSC_VER)
#إذا defined(_M_ARM64)
  __yield();
#وإلا
  _mm_pause();
#endif
#وإلاإذا defined(__x86_64__) || defined(__i386__)
  __asm__ متطاير("pause" ::: "memory");
#وإلاإذا defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ متطاير("yield" ::: "memory");
#وإلاإذا defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ متطاير("or 27,27,27");
#وإلاإذا defined(__sparc__)
  __asm__ متطاير("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#وإلا
  بنية timespec ts = {0};
  nanosleep(&ts, 0);
#endif
}

#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

ساكن فراغ NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  بدل (reason) {
  حالة DLL_PROCESS_ATTACH:
    اكسر;
  حالة DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    اكسر;
  حالة DLL_THREAD_ATTACH:
    اكسر;
  حالة DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    اكسر;
  }
}

#ifdef _WIN64
#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#pragma const_seg(".CRT$XLY")

خارجي ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#pragma const_seg()
#وإلا // _WIN64
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#pragma data_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#pragma data_seg()
#endif // _WIN64

ساكن فراغ NTAPI _rpmalloc_thread_destructor(فراغ *قيمة_2) {
#إذا ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  إذا (get_thread_id() == _rpmalloc_main_thread_id)
    إرجاع;
#endif
  إذا (قيمة_2)
    rpmalloc_thread_finalize(1);
}
#endif

////////////
///
/// Low level memory map/unmap
///
//////

ساكن فراغ _rpmalloc_set_name(فراغ *address, size_t size) {
#إذا defined(__linux__) || defined(__ANDROID__)
  ثابت حرف *name = _memory_huge_pages ? _memory_config.huge_page_name
                                        : _memory_config.page_name;
  إذا (address == MAP_FAILED || !name)
    إرجاع;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (فراغ)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (uintptr_t)address, size,
              (uintptr_t)name);
#وإلا
  (فراغ)حجم(size);
  (فراغ)حجم(address);
#endif
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
ساكن فراغ *_rpmalloc_mmap(size_t size, size_t *offset) {
  rpmalloc_assert(!(size % _memory_page_size), "Invalid mmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
  فراغ *address = _memory_config.memory_map(size, offset);
  إذا (EXPECTED(address != 0)) {
    _rpmalloc_stat_add_peak(&_mapped_pages, (size >> _memory_page_size_shift),
                            _mapped_pages_peak);
    _rpmalloc_stat_add(&_mapped_total, (size >> _memory_page_size_shift));
  }
  إرجاع address;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
ساكن فراغ _rpmalloc_unmap(فراغ *address, size_t size, size_t offset,
                            size_t release) {
  rpmalloc_assert(!release || (release >= size), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  إذا (release) {
    rpmalloc_assert(!(release % _memory_page_size), "Invalid unmap size");
    _rpmalloc_stat_sub(&_mapped_pages, (release >> _memory_page_size_shift));
    _rpmalloc_stat_add(&_unmapped_total, (release >> _memory_page_size_shift));
  }
  _memory_config.memory_unmap(address, size, offset, release);
}

//! Default implementation to map new pages to virtual memory
ساكن فراغ *_rpmalloc_mmap_os(size_t size, size_t *offset) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((size >= _memory_span_size) &&
                    (_memory_span_size > _memory_map_granularity))
                       ? _memory_span_size
                       : 0;
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
#إذا PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  فراغ *ptr = VirtualAlloc(0, size + padding,
                           (_memory_huge_pages ? MEM_LARGE_PAGES : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
  إذا (!ptr) {
    إذا (_memory_config.map_fail_callback) {
      إذا (_memory_config.map_fail_callback(size + padding))
        إرجاع _rpmalloc_mmap_os(size, offset);
    } وإلا {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    إرجاع 0;
  }
#وإلا
  صحيح flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#إذا defined(__APPLE__) && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
  صحيح fd = (صحيح)VM_MAKE_TAG(240U);
  إذا (_memory_huge_pages)
    fd |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
  فراغ *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#وإلاإذا defined(MAP_HUGETLB)
  فراغ *ptr = mmap(0, size + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (_memory_huge_pages ? MAP_HUGETLB : 0) | flags, -1, 0);
#إذا defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  إذا ((ptr == MAP_FAILED || !ptr) && _memory_huge_pages) {
    ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    إذا (ptr && ptr != MAP_FAILED) {
      صحيح prm = madvise(ptr, size + padding, MADV_HUGEPAGE);
      (فراغ)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#endif
  _rpmalloc_set_name(ptr, size + padding);
#وإلاإذا defined(MAP_ALIGNED)
  ثابت size_t align =
      (حجم(size_t) * 8) - (size_t)(__builtin_clzl(size - 1));
  فراغ *ptr =
      mmap(0, size + padding, PROT_READ | PROT_WRITE,
           (_memory_huge_pages ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#وإلاإذا defined(MAP_ALIGN)
  caddr_t base = (_memory_huge_pages ? (caddr_t)(4 << 20) : 0);
  فراغ *ptr = mmap(base, size + padding, PROT_READ | PROT_WRITE,
                   (_memory_huge_pages ? MAP_ALIGN : 0) | flags, -1, 0);
#وإلا
  فراغ *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#endif
  إذا ((ptr == MAP_FAILED) || !ptr) {
    إذا (_memory_config.map_fail_callback) {
      إذا (_memory_config.map_fail_callback(size + padding))
        إرجاع _rpmalloc_mmap_os(size, offset);
    } وإلا إذا (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    إرجاع 0;
  }
#endif
  _rpmalloc_stat_add(&_mapped_pages_os,
                     (int32_t)((size + padding) >> _memory_page_size_shift));
  إذا (padding) {
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
  إرجاع ptr;
}

//! Default implementation to unmap pages from virtual memory
ساكن فراغ _rpmalloc_unmap_os(فراغ *address, size_t size, size_t offset,
                               size_t release) {
  rpmalloc_assert(release || (offset == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid unmap size");
  إذا (release && offset) {
    offset <<= 3;
    address = pointer_offset(address, -(int32_t)offset);
    إذا ((release >= _memory_span_size) &&
        (_memory_span_size > _memory_map_granularity)) {
      // Padding is always one span size
      release += _memory_span_size;
    }
  }
#إذا !DISABLE_UNMAP
#إذا PLATFORM_WINDOWS
  إذا (!VirtualFree(address, release ? 0 : size,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#وإلا
  إذا (release) {
    إذا (munmap(address, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } وإلا {
#إذا defined(MADV_FREE_REUSABLE)
    صحيح ret;
    بينما ((ret = madvise(address, size, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    إذا ((ret == -1) && (errno != 0)) {
#وإلاإذا defined(MADV_DONTNEED)
    إذا (madvise(address, size, MADV_DONTNEED)) {
#وإلاإذا defined(MADV_PAGEOUT)
    إذا (madvise(address, size, MADV_PAGEOUT)) {
#وإلاإذا defined(MADV_FREE)
    إذا (madvise(address, size, MADV_FREE)) {
#وإلا
    إذا (posix_madvise(address, size, POSIX_MADV_DONTNEED)) {
#endif
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#endif
#endif
  إذا (release)
    _rpmalloc_stat_sub(&_mapped_pages_os, release >> _memory_page_size_shift);
}

ساكن فراغ _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_تعداد);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
ساكن span_t *_rpmalloc_global_get_reserved_spans(size_t span_تعداد) {
  span_t *span = _memory_global_reserve;
  _rpmalloc_span_mark_as_subspan_unless_master(_memory_global_reserve_master,
                                               span, span_تعداد);
  memory_global_reserve_تعداد -= span_تعداد;
  إذا (memory_global_reserve_تعداد)
    _memory_global_reserve =
        (span_t *)pointer_offset(span, span_تعداد << _memory_span_size_shift);
  وإلا
    _memory_global_reserve = 0;
  إرجاع span;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
ساكن فراغ _rpmalloc_global_set_reserved_spans(span_t *master, span_t *reserve,
                                                size_t reserve_span_تعداد) {
  _memory_global_reserve_master = master;
  memory_global_reserve_تعداد = reserve_span_تعداد;
  _memory_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
ساكن فراغ _rpmalloc_span_double_link_list_add(span_t **head, span_t *span) {
  إذا (*head)
    (*head)->prev = span;
  span->next = *head;
  *head = span;
}

//! Pop head span from double linked list
ساكن فراغ rpmalloc_span_double_link_list_اسحب_head(span_t **head,
                                                     span_t *span) {
  rpmalloc_assert(*head == span, "Linked list corrupted");
  span = *head;
  *head = span->next;
}

//! Remove a span from double linked list
ساكن فراغ _rpmalloc_span_double_link_list_remove(span_t **head,
                                                   span_t *span) {
  rpmalloc_assert(*head, "Linked list corrupted");
  إذا (*head == span) {
    *head = span->next;
  } وإلا {
    span_t *next_span = span->next;
    span_t *prev_span = span->prev;
    prev_span->next = next_span;
    إذا (EXPECTED(next_span != 0))
      next_span->prev = prev_span;
  }
}

////////////
///
/// Span control
///
//////

ساكن فراغ rpmalloc_heap_cache_أدرج(heap_t *heap, span_t *span);

ساكن فراغ _rpmalloc_heap_finalize(heap_t *heap);

ساكن فراغ _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_تعداد);

//! Declare the span to be a subspan and store distance from master span and
//! span count
ساكن فراغ _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_تعداد) {
  rpmalloc_assert((subspan != master) || (subspan->flags & SPAN_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  إذا (subspan != master) {
    subspan->flags = SPAN_FLAG_SUBSPAN;
    subspan->offset_from_master =
        (uint32_t)((uintptr_t)pointer_diff(subspan, master) >>
                   _memory_span_size_shift);
    subspan->align_offset = 0;
  }
  subspan->span_تعداد = (uint32_t)span_تعداد;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
ساكن span_t *_rpmalloc_span_map_from_reserve(heap_t *heap,
                                               size_t span_تعداد) {
  // Update the heap span reserve
  span_t *span = heap->span_reserve;
  heap->span_reserve =
      (span_t *)pointer_offset(span, span_تعداد * _memory_span_size);
  heap->spans_reserved -= (uint32_t)span_تعداد;

  _rpmalloc_span_mark_as_subspan_unless_master(heap->span_reserve_master, span,
                                               span_تعداد);
  إذا (span_تعداد <= LARGE_CLASS_تعداد)
    _rpmalloc_stat_inc(&heap->span_use[span_تعداد - 1].spans_from_reserved);

  إرجاع span;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
ساكن size_t rpmalloc_span_align_تعداد(size_t span_تعداد) {
  size_t request_تعداد = (span_تعداد > memory_span_map_تعداد)
                             ? span_تعداد
                             : memory_span_map_تعداد;
  إذا ((_memory_page_size > _memory_span_size) &&
      ((request_تعداد * _memory_span_size) % _memory_page_size))
    request_تعداد +=
        memory_span_map_تعداد - (request_تعداد % memory_span_map_تعداد);
  إرجاع request_تعداد;
}

//! Setup a newly mapped span
ساكن فراغ _rpmalloc_span_initialize(span_t *span, size_t total_span_تعداد,
                                      size_t span_تعداد, size_t align_offset) {
  span->total_spans = (uint32_t)total_span_تعداد;
  span->span_تعداد = (uint32_t)span_تعداد;
  span->align_offset = (uint32_t)align_offset;
  span->flags = SPAN_FLAG_MASTER;
  atomic_store32(&span->remaining_spans, (int32_t)total_span_تعداد);
}

ساكن فراغ _rpmalloc_span_unmap(span_t *span);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
ساكن span_t *rpmalloc_span_map_aligned_تعداد(heap_t *heap,
                                                size_t span_تعداد) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_span_تعداد = rpmalloc_span_align_تعداد(span_تعداد);
  size_t align_offset = 0;
  span_t *span = (span_t *)_rpmalloc_mmap(
      aligned_span_تعداد * _memory_span_size, &align_offset);
  إذا (!span)
    إرجاع 0;
  _rpmalloc_span_initialize(span, aligned_span_تعداد, span_تعداد, align_offset);
  _rpmalloc_stat_inc(&_master_spans);
  إذا (span_تعداد <= LARGE_CLASS_تعداد)
    _rpmalloc_stat_inc(&heap->span_use[span_تعداد - 1].spans_map_calls);
  إذا (aligned_span_تعداد > span_تعداد) {
    span_t *reserved_spans =
        (span_t *)pointer_offset(span, span_تعداد * _memory_span_size);
    size_t reserved_تعداد = aligned_span_تعداد - span_تعداد;
    إذا (heap->spans_reserved) {
      _rpmalloc_span_mark_as_subspan_unless_master(
          heap->span_reserve_master, heap->span_reserve, heap->spans_reserved);
      rpmalloc_heap_cache_أدرج(heap, heap->span_reserve);
    }
    إذا (reserved_تعداد > memory_heap_reserve_تعداد) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&_memory_global_lock) == 1,
                      "Global spin lock not held as expected");
      size_t remain_تعداد = reserved_تعداد - memory_heap_reserve_تعداد;
      reserved_تعداد = memory_heap_reserve_تعداد;
      span_t *remain_span = (span_t *)pointer_offset(
          reserved_spans, reserved_تعداد * _memory_span_size);
      إذا (_memory_global_reserve) {
        _rpmalloc_span_mark_as_subspan_unless_master(
            _memory_global_reserve_master, _memory_global_reserve,
            memory_global_reserve_تعداد);
        _rpmalloc_span_unmap(_memory_global_reserve);
      }
      _rpmalloc_global_set_reserved_spans(span, remain_span, remain_تعداد);
    }
    _rpmalloc_heap_set_reserved_spans(heap, span, reserved_spans,
                                      reserved_تعداد);
  }
  إرجاع span;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
ساكن span_t *_rpmalloc_span_map(heap_t *heap, size_t span_تعداد) {
  إذا (span_تعداد <= heap->spans_reserved)
    إرجاع _rpmalloc_span_map_from_reserve(heap, span_تعداد);
  span_t *span = 0;
  صحيح use_global_reserve =
      (_memory_page_size > _memory_span_size) ||
      (memory_span_map_تعداد > memory_heap_reserve_تعداد);
  إذا (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    بينما (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
    إذا (memory_global_reserve_تعداد >= span_تعداد) {
      size_t reserve_تعداد =
          (!heap->spans_reserved ? memory_heap_reserve_تعداد : span_تعداد);
      إذا (memory_global_reserve_تعداد < reserve_تعداد)
        reserve_تعداد = memory_global_reserve_تعداد;
      span = _rpmalloc_global_get_reserved_spans(reserve_تعداد);
      إذا (span) {
        إذا (reserve_تعداد > span_تعداد) {
          span_t *reserved_span = (span_t *)pointer_offset(
              span, span_تعداد << _memory_span_size_shift);
          _rpmalloc_heap_set_reserved_spans(heap, _memory_global_reserve_master,
                                            reserved_span,
                                            reserve_تعداد - span_تعداد);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        span->span_تعداد = (uint32_t)span_تعداد;
      }
    }
  }
  إذا (!span)
    span = rpmalloc_span_map_aligned_تعداد(heap, span_تعداد);
  إذا (use_global_reserve)
    atomic_store32_release(&_memory_global_lock, 0);
  إرجاع span;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
ساكن فراغ _rpmalloc_span_unmap(span_t *span) {
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");

  صحيح is_master = !!(span->flags & SPAN_FLAG_MASTER);
  span_t *master =
      is_master ? span
                : ((span_t *)pointer_offset(
                      span, -(intptr_t)((uintptr_t)span->offset_from_master *
                                        _memory_span_size)));
  rpmalloc_assert(is_master || (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");

  size_t span_تعداد = span->span_تعداد;
  إذا (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(span->align_offset == 0, "Span align offset corrupted");
    إذا (_memory_span_size >= _memory_page_size)
      _rpmalloc_unmap(span, span_تعداد * _memory_span_size, 0, 0);
  } وإلا {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    span->flags |=
        SPAN_FLAG_MASTER | SPAN_FLAG_SUBSPAN | SPAN_FLAG_UNMAPPED_MASTER;
    _rpmalloc_stat_add(&_unmapped_master_spans, 1);
  }

  إذا (atomic_add32(&master->remaining_spans, -(int32_t)span_تعداد) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & SPAN_FLAG_MASTER) &&
                        !!(master->flags & SPAN_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_تعداد = master->span_تعداد;
    إذا (_memory_span_size < _memory_page_size)
      unmap_تعداد = master->total_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_تعداد * _memory_span_size,
                    master->align_offset,
                    (size_t)master->total_spans * _memory_span_size);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
ساكن فراغ _rpmalloc_span_release_to_cache(heap_t *heap, span_t *span) {
  rpmalloc_assert(heap == span->heap, "Span heap pointer corrupted");
  rpmalloc_assert(span->size_class < SIZE_CLASS_تعداد,
                  "Invalid span size class");
  rpmalloc_assert(span->span_تعداد == 1, "Invalid span count");
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&heap->span_use[0].current);
#endif
  _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
  إذا (!heap->finalize) {
    _rpmalloc_stat_inc(&heap->span_use[0].spans_to_cache);
    _rpmalloc_stat_inc(&heap->size_class_use[span->size_class].spans_to_cache);
    إذا (heap->size_class[span->size_class].cache)
      rpmalloc_heap_cache_أدرج(heap,
                                  heap->size_class[span->size_class].cache);
    heap->size_class[span->size_class].cache = span;
  } وإلا {
    _rpmalloc_span_unmap(span);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
ساكن uint32_t free_list_partial_init(فراغ **list, فراغ **first_block,
                                       فراغ *page_start, فراغ *block_start,
                                       uint32_t block_تعداد,
                                       uint32_t block_size) {
  rpmalloc_assert(block_تعداد, "Internal failure");
  *first_block = block_start;
  إذا (block_تعداد > 1) {
    فراغ *free_block = pointer_offset(block_start, block_size);
    فراغ *block_end =
        pointer_offset(block_start, (size_t)block_size * block_تعداد);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    إذا (block_size < (_memory_page_size >> 1)) {
      فراغ *page_end = pointer_offset(page_start, _memory_page_size);
      إذا (page_end < block_end)
        block_end = page_end;
    }
    *list = free_block;
    block_تعداد = 2;
    فراغ *next_block = pointer_offset(free_block, block_size);
    بينما (next_block < block_end) {
      *((فراغ **)free_block) = next_block;
      free_block = next_block;
      ++block_تعداد;
      next_block = pointer_offset(next_block, block_size);
    }
    *((فراغ **)free_block) = 0;
  } وإلا {
    *list = 0;
  }
  إرجاع block_تعداد;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
ساكن فراغ *_rpmalloc_span_initialize_new(heap_t *heap,
                                           heap_size_class_t *heap_size_class,
                                           span_t *span, uint32_t class_idx) {
  rpmalloc_assert(span->span_تعداد == 1, "Internal failure");
  size_class_t *size_class = _memory_size_class + class_idx;
  span->size_class = class_idx;
  span->heap = heap;
  span->flags &= ~SPAN_FLAG_ALIGNED_BLOCKS;
  span->block_size = size_class->block_size;
  span->block_تعداد = size_class->block_تعداد;
  span->free_list = 0;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  فراغ *block;
  span->free_list_limit =
      free_list_partial_init(&heap_size_class->free_list, &block, span,
                             pointer_offset(span, SPAN_HEADER_SIZE),
                             size_class->block_تعداد, size_class->block_size);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  إذا (span->free_list_limit < span->block_تعداد) {
    _rpmalloc_span_double_link_list_add(&heap_size_class->partial_span, span);
    span->used_تعداد = span->free_list_limit;
  } وإلا {
#إذا RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_تعداد;
    span->used_تعداد = span->block_تعداد;
  }
  إرجاع block;
}

ساكن فراغ _rpmalloc_span_extract_free_list_deferred(span_t *span) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  افعل {
    span->free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } بينما (span->free_list == INVALID_POINTER);
  span->used_تعداد -= span->list_size;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);
}

ساكن صحيح _rpmalloc_span_is_fully_utilized(span_t *span) {
  rpmalloc_assert(span->free_list_limit <= span->block_تعداد,
                  "Span free list corrupted");
  إرجاع !span->free_list && (span->free_list_limit >= span->block_تعداد);
}

ساكن صحيح _rpmalloc_span_finalize(heap_t *heap, size_t iclass, span_t *span,
                                   span_t **list_head) {
  فراغ *free_list = heap->size_class[iclass].free_list;
  span_t *class_span = (span_t *)((uintptr_t)free_list & _memory_span_mask);
  إذا (span == class_span) {
    // Adopt the heap class free list back into the span free list
    فراغ *block = span->free_list;
    فراغ *last_block = 0;
    بينما (block) {
      last_block = block;
      block = *((فراغ **)block);
    }
    uint32_t free_تعداد = 0;
    block = free_list;
    بينما (block) {
      ++free_تعداد;
      block = *((فراغ **)block);
    }
    إذا (last_block) {
      *((فراغ **)last_block) = free_list;
    } وإلا {
      span->free_list = free_list;
    }
    heap->size_class[iclass].free_list = 0;
    span->used_تعداد -= free_تعداد;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(span->list_size == span->used_تعداد, "Memory leak detected");
  إذا (span->list_size == span->used_تعداد) {
    _rpmalloc_stat_dec(&heap->span_use[0].current);
    _rpmalloc_stat_dec(&heap->size_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    إذا (list_head)
      _rpmalloc_span_double_link_list_remove(list_head, span);
    _rpmalloc_span_unmap(span);
    إرجاع 1;
  }
  إرجاع 0;
}

////////////
///
/// Global cache
///
//////

#إذا ENABLE_GLOBAL_CACHE

//! Finalize a global cache
ساكن فراغ _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  بينما (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

  لكل (size_t ispan = 0; ispan < cache->تعداد_2; ++ispan)
    _rpmalloc_span_unmap(cache->span[ispan]);
  cache->تعداد_2 = 0;

  بينما (cache->overflow) {
    span_t *span = cache->overflow;
    cache->overflow = span->next;
    _rpmalloc_span_unmap(span);
  }

  atomic_store32_release(&cache->lock, 0);
}

ساكن فراغ rpmalloc_global_cache_أدرج_spans(span_t **span,
                                                size_t span_تعداد,
                                                size_t تعداد_2) {
  ثابت size_t cache_limit =
      (span_تعداد == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_SPAN_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_SPAN_LARGE_CACHE - (span_تعداد >> 1));

  global_cache_t *cache = &_memory_span_cache[span_تعداد - 1];

  size_t أدرج_تعداد = تعداد_2;
  بينما (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#إذا ENABLE_STATISTICS
  cache->أدرج_تعداد += تعداد_2;
#endif
  إذا ((cache->تعداد_2 + أدرج_تعداد) > cache_limit)
    أدرج_تعداد = cache_limit - cache->تعداد_2;

  memcpy(cache->span + cache->تعداد_2, span, حجم(span_t *) * أدرج_تعداد);
  cache->تعداد_2 += (uint32_t)أدرج_تعداد;

#إذا ENABLE_UNLIMITED_CACHE
  بينما (أدرج_تعداد < تعداد_2) {
#وإلا
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  بينما ((_memory_page_size > _memory_span_size) && (أدرج_تعداد < تعداد_2)) {
#endif
    span_t *current_span = span[أدرج_تعداد++];
    current_span->next = cache->overflow;
    cache->overflow = current_span;
  }
  atomic_store32_release(&cache->lock, 0);

  span_t *keep = 0;
  لكل (size_t ispan = أدرج_تعداد; ispan < تعداد_2; ++ispan) {
    span_t *current_span = span[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    إذا ((current_span->flags & SPAN_FLAG_MASTER) &&
        (atomic_load32(&current_span->remaining_spans) >
         (int32_t)current_span->span_تعداد)) {
      current_span->next = keep;
      keep = current_span;
    } وإلا {
      _rpmalloc_span_unmap(current_span);
    }
  }

  إذا (keep) {
    بينما (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    بينما (keep) {
      لكل (; islot < cache->تعداد_2; ++islot) {
        span_t *current_span = cache->span[islot];
        إذا (!(current_span->flags & SPAN_FLAG_MASTER) ||
            ((current_span->flags & SPAN_FLAG_MASTER) &&
             (atomic_load32(&current_span->remaining_spans) <=
              (int32_t)current_span->span_تعداد))) {
          _rpmalloc_span_unmap(current_span);
          cache->span[islot] = keep;
          اكسر;
        }
      }
      إذا (islot == cache->تعداد_2)
        اكسر;
      keep = keep->next;
    }

    إذا (keep) {
      span_t *tail = keep;
      بينما (tail->next)
        tail = tail->next;
      tail->next = cache->overflow;
      cache->overflow = keep;
    }

    atomic_store32_release(&cache->lock, 0);
  }
}

ساكن size_t _rpmalloc_global_cache_extract_spans(span_t **span,
                                                   size_t span_تعداد,
                                                   size_t تعداد_2) {
  global_cache_t *cache = &_memory_span_cache[span_تعداد - 1];

  size_t extract_تعداد = 0;
  بينما (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#إذا ENABLE_STATISTICS
  cache->extract_تعداد += تعداد_2;
#endif
  size_t want = تعداد_2 - extract_تعداد;
  إذا (want > cache->تعداد_2)
    want = cache->تعداد_2;

  memcpy(span + extract_تعداد, cache->span + (cache->تعداد_2 - want),
         حجم(span_t *) * want);
  cache->تعداد_2 -= (uint32_t)want;
  extract_تعداد += want;

  بينما ((extract_تعداد < تعداد_2) && cache->overflow) {
    span_t *current_span = cache->overflow;
    span[extract_تعداد++] = current_span;
    cache->overflow = current_span->next;
  }

#إذا ENABLE_ASSERTS
  لكل (size_t ispan = 0; ispan < extract_تعداد; ++ispan) {
    rpmalloc_assert(span[ispan]->span_تعداد == span_تعداد,
                    "Global cache span count mismatch");
  }
#endif

  atomic_store32_release(&cache->lock, 0);

  إرجاع extract_تعداد;
}

#endif

////////////
///
/// Heap control
///
//////

ساكن فراغ _rpmalloc_deallocate_huge(span_t *);

//! Store the given spans as reserve in the given heap
ساكن فراغ _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_تعداد) {
  heap->span_reserve_master = master;
  heap->span_reserve = reserve;
  heap->spans_reserved = (uint32_t)reserve_span_تعداد;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
ساكن فراغ _rpmalloc_heap_cache_adopt_deferred(heap_t *heap,
                                                span_t **single_span) {
  span_t *span = (span_t *)((فراغ *)atomic_exchange_ptr_acquire(
      &heap->span_free_deferred, 0));
  بينما (span) {
    span_t *next_span = (span_t *)span->free_list;
    rpmalloc_assert(span->heap == heap, "Span heap pointer corrupted");
    إذا (EXPECTED(span->size_class < SIZE_CLASS_تعداد)) {
      rpmalloc_assert(heap->full_span_تعداد, "Heap span counter corrupted");
      --heap->full_span_تعداد;
      _rpmalloc_stat_dec(&heap->span_use[0].spans_deferred);
#إذا RPMALLOC_FIRST_CLASS_HEAPS
      _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                             span);
#endif
      _rpmalloc_stat_dec(&heap->span_use[0].current);
      _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
      إذا (single_span && !*single_span)
        *single_span = span;
      وإلا
        rpmalloc_heap_cache_أدرج(heap, span);
    } وإلا {
      إذا (span->size_class == SIZE_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(span);
      } وإلا {
        rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(heap->full_span_تعداد, "Heap span counter corrupted");
        --heap->full_span_تعداد;
#إذا RPMALLOC_FIRST_CLASS_HEAPS
        _rpmalloc_span_double_link_list_remove(&heap->large_huge_span, span);
#endif
        uint32_t idx = span->span_تعداد - 1;
        _rpmalloc_stat_dec(&heap->span_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&heap->span_use[idx].current);
        إذا (!idx && single_span && !*single_span)
          *single_span = span;
        وإلا
          rpmalloc_heap_cache_أدرج(heap, span);
      }
    }
    span = next_span;
  }
}

ساكن فراغ _rpmalloc_heap_unmap(heap_t *heap) {
  إذا (!heap->master_heap) {
    إذا ((heap->finalize > 1) && !atomic_load32(&heap->child_تعداد)) {
      span_t *span = (span_t *)((uintptr_t)heap & _memory_span_mask);
      _rpmalloc_span_unmap(span);
    }
  } وإلا {
    إذا (atomic_decr32(&heap->master_heap->child_تعداد) == 0) {
      _rpmalloc_heap_unmap(heap->master_heap);
    }
  }
}

ساكن فراغ _rpmalloc_heap_global_finalize(heap_t *heap) {
  إذا (heap->finalize++ > 1) {
    --heap->finalize;
    إرجاع;
  }

  _rpmalloc_heap_finalize(heap);

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_تعداد; ++iclass) {
    span_cache_t *span_cache;
    إذا (!iclass)
      span_cache = &heap->span_cache;
    وإلا
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    لكل (size_t ispan = 0; ispan < span_cache->تعداد_2; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->تعداد_2 = 0;
  }
#endif

  إذا (heap->full_span_تعداد) {
    --heap->finalize;
    إرجاع;
  }

  لكل (size_t iclass = 0; iclass < SIZE_CLASS_تعداد; ++iclass) {
    إذا (heap->size_class[iclass].free_list ||
        heap->size_class[iclass].partial_span) {
      --heap->finalize;
      إرجاع;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap_t *list_heap = _memory_heaps[list_idx];
  إذا (list_heap == heap) {
    _memory_heaps[list_idx] = heap->next_heap;
  } وإلا {
    بينما (list_heap->next_heap != heap)
      list_heap = list_heap->next_heap;
    list_heap->next_heap = heap->next_heap;
  }

  _rpmalloc_heap_unmap(heap);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
ساكن فراغ rpmalloc_heap_cache_أدرج(heap_t *heap, span_t *span) {
  إذا (UNEXPECTED(heap->finalize != 0)) {
    _rpmalloc_span_unmap(span);
    _rpmalloc_heap_global_finalize(heap);
    إرجاع;
  }
#إذا ENABLE_THREAD_CACHE
  size_t span_تعداد = span->span_تعداد;
  _rpmalloc_stat_inc(&heap->span_use[span_تعداد - 1].spans_to_cache);
  إذا (span_تعداد == 1) {
    span_cache_t *span_cache = &heap->span_cache;
    span_cache->span[span_cache->تعداد_2++] = span;
    إذا (span_cache->تعداد_2 == MAX_THREAD_SPAN_CACHE) {
      ثابت size_t remain_تعداد =
          MAX_THREAD_SPAN_CACHE - THREAD_SPAN_CACHE_TRANSFER;
#إذا ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           THREAD_SPAN_CACHE_TRANSFER * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_تعداد - 1].spans_to_global,
                         THREAD_SPAN_CACHE_TRANSFER);
      rpmalloc_global_cache_أدرج_spans(span_cache->span + remain_تعداد,
                                          span_تعداد,
                                          THREAD_SPAN_CACHE_TRANSFER);
#وإلا
      لكل (size_t ispan = 0; ispan < THREAD_SPAN_CACHE_TRANSFER; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_تعداد + ispan]);
#endif
      span_cache->تعداد_2 = remain_تعداد;
    }
  } وإلا {
    size_t cache_idx = span_تعداد - 2;
    span_large_cache_t *span_cache = heap->span_large_cache + cache_idx;
    span_cache->span[span_cache->تعداد_2++] = span;
    ثابت size_t cache_limit =
        (MAX_THREAD_SPAN_LARGE_CACHE - (span_تعداد >> 1));
    إذا (span_cache->تعداد_2 == cache_limit) {
      ثابت size_t transfer_limit = 2 + (cache_limit >> 2);
      ثابت size_t transfer_تعداد =
          (THREAD_SPAN_LARGE_CACHE_TRANSFER <= transfer_limit
               ? THREAD_SPAN_LARGE_CACHE_TRANSFER
               : transfer_limit);
      ثابت size_t remain_تعداد = cache_limit - transfer_تعداد;
#إذا ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           transfer_تعداد * span_تعداد * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_تعداد - 1].spans_to_global,
                         transfer_تعداد);
      rpmalloc_global_cache_أدرج_spans(span_cache->span + remain_تعداد,
                                          span_تعداد, transfer_تعداد);
#وإلا
      لكل (size_t ispan = 0; ispan < transfer_تعداد; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_تعداد + ispan]);
#endif
      span_cache->تعداد_2 = remain_تعداد;
    }
  }
#وإلا
  (فراغ)حجم(heap);
  _rpmalloc_span_unmap(span);
#endif
}

//! Extract the given number of spans from the different cache levels
ساكن span_t *_rpmalloc_heap_thread_cache_extract(heap_t *heap,
                                                   size_t span_تعداد) {
  span_t *span = 0;
#إذا ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  إذا (span_تعداد == 1)
    span_cache = &heap->span_cache;
  وإلا
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_تعداد - 2));
  إذا (span_cache->تعداد_2) {
    _rpmalloc_stat_inc(&heap->span_use[span_تعداد - 1].spans_from_cache);
    إرجاع span_cache->span[--span_cache->تعداد_2];
  }
#endif
  إرجاع span;
}

ساكن span_t *_rpmalloc_heap_thread_cache_deferred_extract(heap_t *heap,
                                                            size_t span_تعداد) {
  span_t *span = 0;
  إذا (span_تعداد == 1) {
    _rpmalloc_heap_cache_adopt_deferred(heap, &span);
  } وإلا {
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
    span = _rpmalloc_heap_thread_cache_extract(heap, span_تعداد);
  }
  إرجاع span;
}

ساكن span_t *_rpmalloc_heap_reserved_extract(heap_t *heap,
                                               size_t span_تعداد) {
  إذا (heap->spans_reserved >= span_تعداد)
    إرجاع _rpmalloc_span_map(heap, span_تعداد);
  إرجاع 0;
}

//! Extract a span from the global cache
ساكن span_t *_rpmalloc_heap_global_cache_extract(heap_t *heap,
                                                   size_t span_تعداد) {
#إذا ENABLE_GLOBAL_CACHE
#إذا ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  size_t wanted_تعداد;
  إذا (span_تعداد == 1) {
    span_cache = &heap->span_cache;
    wanted_تعداد = THREAD_SPAN_CACHE_TRANSFER;
  } وإلا {
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_تعداد - 2));
    wanted_تعداد = THREAD_SPAN_LARGE_CACHE_TRANSFER;
  }
  span_cache->تعداد_2 = _rpmalloc_global_cache_extract_spans(
      span_cache->span, span_تعداد, wanted_تعداد);
  إذا (span_cache->تعداد_2) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_تعداد * span_cache->تعداد_2 * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_تعداد - 1].spans_from_global,
                       span_cache->تعداد_2);
    إرجاع span_cache->span[--span_cache->تعداد_2];
  }
#وإلا
  span_t *span = 0;
  size_t تعداد_2 = _rpmalloc_global_cache_extract_spans(&span, span_تعداد, 1);
  إذا (تعداد_2) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_تعداد * تعداد_2 * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_تعداد - 1].spans_from_global,
                       تعداد_2);
    إرجاع span;
  }
#endif
#endif
  (فراغ)حجم(heap);
  (فراغ)حجم(span_تعداد);
  إرجاع 0;
}

ساكن فراغ _rpmalloc_inc_span_statistics(heap_t *heap, size_t span_تعداد,
                                          uint32_t class_idx) {
  (فراغ)حجم(heap);
  (فراغ)حجم(span_تعداد);
  (فراغ)حجم(class_idx);
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)span_تعداد - 1;
  uint32_t current_تعداد =
      (uint32_t)atomic_incr32(&heap->span_use[idx].current);
  إذا (current_تعداد > (uint32_t)atomic_load32(&heap->span_use[idx].أعلى))
    atomic_store32(&heap->span_use[idx].أعلى, (int32_t)current_تعداد);
  _rpmalloc_stat_add_peak(&heap->size_class_use[class_idx].spans_current, 1,
                          heap->size_class_use[class_idx].spans_peak);
#endif
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
ساكن span_t *
_rpmalloc_heap_extract_new_span(heap_t *heap,
                                heap_size_class_t *heap_size_class,
                                size_t span_تعداد, uint32_t class_idx) {
  span_t *span;
#إذا ENABLE_THREAD_CACHE
  إذا (heap_size_class && heap_size_class->cache) {
    span = heap_size_class->cache;
    heap_size_class->cache =
        (heap->span_cache.تعداد_2
             ? heap->span_cache.span[--heap->span_cache.تعداد_2]
             : 0);
    _rpmalloc_inc_span_statistics(heap, span_تعداد, class_idx);
    إرجاع span;
  }
#endif
  (فراغ)حجم(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t base_span_تعداد = span_تعداد;
  size_t limit_span_تعداد =
      (span_تعداد > 2) ? (span_تعداد + (span_تعداد >> 1)) : span_تعداد;
  إذا (limit_span_تعداد > LARGE_CLASS_تعداد)
    limit_span_تعداد = LARGE_CLASS_تعداد;
  افعل {
    span = _rpmalloc_heap_thread_cache_extract(heap, span_تعداد);
    إذا (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_تعداد, class_idx);
      إرجاع span;
    }
    span = _rpmalloc_heap_thread_cache_deferred_extract(heap, span_تعداد);
    إذا (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_تعداد, class_idx);
      إرجاع span;
    }
    span = _rpmalloc_heap_global_cache_extract(heap, span_تعداد);
    إذا (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_تعداد, class_idx);
      إرجاع span;
    }
    span = _rpmalloc_heap_reserved_extract(heap, span_تعداد);
    إذا (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_reserved);
      _rpmalloc_inc_span_statistics(heap, span_تعداد, class_idx);
      إرجاع span;
    }
    ++span_تعداد;
  } بينما (span_تعداد <= limit_span_تعداد);
  // Final fallback, map in more virtual memory
  span = _rpmalloc_span_map(heap, base_span_تعداد);
  _rpmalloc_inc_span_statistics(heap, base_span_تعداد, class_idx);
  _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_map_calls);
  إرجاع span;
}

ساكن فراغ _rpmalloc_heap_initialize(heap_t *heap) {
  _rpmalloc_memset_const(heap, 0, حجم(heap_t));
  // Get a new heap ID
  heap->id = 1 + atomic_incr32(&_memory_heap_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap->next_heap = _memory_heaps[list_idx];
  _memory_heaps[list_idx] = heap;
}

ساكن فراغ _rpmalloc_heap_orphan(heap_t *heap, صحيح first_class) {
  heap->owner_thread = (uintptr_t)-1;
#إذا RPMALLOC_FIRST_CLASS_HEAPS
  heap_t **heap_list =
      (first_class ? &_memory_first_class_orphan_heaps : &_memory_orphan_heaps);
#وإلا
  (فراغ)حجم(first_class);
  heap_t **heap_list = &_memory_orphan_heaps;
#endif
  heap->next_orphan = *heap_list;
  *heap_list = heap;
}

//! Allocate a new heap from newly mapped memory pages
ساكن heap_t *_rpmalloc_heap_allocate_new(فراغ) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t heap_size = حجم(heap_t);
  size_t aligned_heap_size = 16 * ((heap_size + 15) / 16);
  size_t request_heap_تعداد = 16;
  size_t heap_span_تعداد = ((aligned_heap_size * request_heap_تعداد) +
                            حجم(span_t) + _memory_span_size - 1) /
                           _memory_span_size;
  size_t block_size = _memory_span_size * heap_span_تعداد;
  size_t span_تعداد = heap_span_تعداد;
  span_t *span = 0;
  // If there are global reserved spans, use these first
  إذا (memory_global_reserve_تعداد >= heap_span_تعداد) {
    span = _rpmalloc_global_get_reserved_spans(heap_span_تعداد);
  }
  إذا (!span) {
    إذا (_memory_page_size > block_size) {
      span_تعداد = _memory_page_size / _memory_span_size;
      block_size = _memory_page_size;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_heap_تعداد =
          (block_size - حجم(span_t)) / aligned_heap_size;
      إذا (possible_heap_تعداد >= (request_heap_تعداد * 16))
        request_heap_تعداد *= 16;
      وإلا إذا (possible_heap_تعداد < request_heap_تعداد)
        request_heap_تعداد = possible_heap_تعداد;
      heap_span_تعداد = ((aligned_heap_size * request_heap_تعداد) +
                         حجم(span_t) + _memory_span_size - 1) /
                        _memory_span_size;
    }

    size_t align_offset = 0;
    span = (span_t *)_rpmalloc_mmap(block_size, &align_offset);
    إذا (!span)
      إرجاع 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    _rpmalloc_span_initialize(span, span_تعداد, heap_span_تعداد, align_offset);
  }

  size_t remain_size = _memory_span_size - حجم(span_t);
  heap_t *heap = (heap_t *)pointer_offset(span, حجم(span_t));
  _rpmalloc_heap_initialize(heap);

  // Put extra heaps as orphans
  size_t رقم_heaps = remain_size / aligned_heap_size;
  إذا (رقم_heaps < request_heap_تعداد)
    رقم_heaps = request_heap_تعداد;
  atomic_store32(&heap->child_تعداد, (int32_t)رقم_heaps - 1);
  heap_t *extra_heap = (heap_t *)pointer_offset(heap, aligned_heap_size);
  بينما (رقم_heaps > 1) {
    _rpmalloc_heap_initialize(extra_heap);
    extra_heap->master_heap = heap;
    _rpmalloc_heap_orphan(extra_heap, 1);
    extra_heap = (heap_t *)pointer_offset(extra_heap, aligned_heap_size);
    --رقم_heaps;
  }

  إذا (span_تعداد > heap_span_تعداد) {
    // Cap reserved spans
    size_t remain_تعداد = span_تعداد - heap_span_تعداد;
    size_t reserve_تعداد =
        (remain_تعداد > memory_heap_reserve_تعداد ? memory_heap_reserve_تعداد
                                                   : remain_تعداد);
    span_t *remain_span =
        (span_t *)pointer_offset(span, heap_span_تعداد * _memory_span_size);
    _rpmalloc_heap_set_reserved_spans(heap, span, remain_span, reserve_تعداد);

    إذا (remain_تعداد > reserve_تعداد) {
      // Set to global reserved spans
      remain_span = (span_t *)pointer_offset(remain_span,
                                             reserve_تعداد * _memory_span_size);
      reserve_تعداد = remain_تعداد - reserve_تعداد;
      _rpmalloc_global_set_reserved_spans(span, remain_span, reserve_تعداد);
    }
  }

  إرجاع heap;
}

ساكن heap_t *_rpmalloc_heap_extract_orphan(heap_t **heap_list) {
  heap_t *heap = *heap_list;
  *heap_list = (heap ? heap->next_orphan : 0);
  إرجاع heap;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
ساكن heap_t *_rpmalloc_heap_allocate(صحيح first_class) {
  heap_t *heap = 0;
  بينما (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
    _rpmalloc_spin();
  إذا (first_class == 0)
    heap = _rpmalloc_heap_extract_orphan(&_memory_orphan_heaps);
#إذا RPMALLOC_FIRST_CLASS_HEAPS
  إذا (!heap)
    heap = _rpmalloc_heap_extract_orphan(&_memory_first_class_orphan_heaps);
#endif
  إذا (!heap)
    heap = _rpmalloc_heap_allocate_new();
  atomic_store32_release(&_memory_global_lock, 0);
  إذا (heap)
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  إرجاع heap;
}

ساكن فراغ _rpmalloc_heap_release(فراغ *heapptr, صحيح first_class,
                                   صحيح release_cache) {
  heap_t *heap = (heap_t *)heapptr;
  إذا (!heap)
    إرجاع;
  // Release thread cache spans back to global cache
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  إذا (release_cache || heap->finalize) {
#إذا ENABLE_THREAD_CACHE
    لكل (size_t iclass = 0; iclass < LARGE_CLASS_تعداد; ++iclass) {
      span_cache_t *span_cache;
      إذا (!iclass)
        span_cache = &heap->span_cache;
      وإلا
        span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
      إذا (!span_cache->تعداد_2)
        استمر;
#إذا ENABLE_GLOBAL_CACHE
      إذا (heap->finalize) {
        لكل (size_t ispan = 0; ispan < span_cache->تعداد_2; ++ispan)
          _rpmalloc_span_unmap(span_cache->span[ispan]);
      } وإلا {
        _rpmalloc_stat_add64(&heap->thread_to_global, span_cache->تعداد_2 *
                                                          (iclass + 1) *
                                                          _memory_span_size);
        _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                           span_cache->تعداد_2);
        rpmalloc_global_cache_أدرج_spans(span_cache->span, iclass + 1,
                                            span_cache->تعداد_2);
      }
#وإلا
      لكل (size_t ispan = 0; ispan < span_cache->تعداد_2; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
      span_cache->تعداد_2 = 0;
    }
#endif
  }

  إذا (get_thread_heap_raw() == heap)
    set_thread_heap(0);

#إذا ENABLE_STATISTICS
  atomic_decr32(&_memory_active_heaps);
  rpmalloc_assert(atomic_load32(&_memory_active_heaps) >= 0,
                  "Still active heaps during finalization");
#endif

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  إذا (get_thread_id() != _rpmalloc_main_thread_id) {
    بينما (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
  }
  _rpmalloc_heap_orphan(heap, first_class);
  atomic_store32_release(&_memory_global_lock, 0);
}

ساكن فراغ _rpmalloc_heap_release_raw(فراغ *heapptr, صحيح release_cache) {
  _rpmalloc_heap_release(heapptr, 0, release_cache);
}

ساكن فراغ _rpmalloc_heap_release_raw_fc(فراغ *heapptr) {
  _rpmalloc_heap_release_raw(heapptr, 1);
}

ساكن فراغ _rpmalloc_heap_finalize(heap_t *heap) {
  إذا (heap->spans_reserved) {
    span_t *span = _rpmalloc_span_map(heap, heap->spans_reserved);
    _rpmalloc_span_unmap(span);
    heap->spans_reserved = 0;
  }

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  لكل (size_t iclass = 0; iclass < SIZE_CLASS_تعداد; ++iclass) {
    إذا (heap->size_class[iclass].cache)
      _rpmalloc_span_unmap(heap->size_class[iclass].cache);
    heap->size_class[iclass].cache = 0;
    span_t *span = heap->size_class[iclass].partial_span;
    بينما (span) {
      span_t *next = span->next;
      _rpmalloc_span_finalize(heap, iclass, span,
                              &heap->size_class[iclass].partial_span);
      span = next;
    }
    // If class still has a free list it must be a full span
    إذا (heap->size_class[iclass].free_list) {
      span_t *class_span =
          (span_t *)((uintptr_t)heap->size_class[iclass].free_list &
                     _memory_span_mask);
      span_t **list = 0;
#إذا RPMALLOC_FIRST_CLASS_HEAPS
      list = &heap->full_span[iclass];
#endif
      --heap->full_span_تعداد;
      إذا (!_rpmalloc_span_finalize(heap, iclass, class_span, list)) {
        إذا (list)
          _rpmalloc_span_double_link_list_remove(list, class_span);
        _rpmalloc_span_double_link_list_add(
            &heap->size_class[iclass].partial_span, class_span);
      }
    }
  }

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_تعداد; ++iclass) {
    span_cache_t *span_cache;
    إذا (!iclass)
      span_cache = &heap->span_cache;
    وإلا
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    لكل (size_t ispan = 0; ispan < span_cache->تعداد_2; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->تعداد_2 = 0;
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
ساكن فراغ *free_list_اسحب(فراغ **list) {
  فراغ *block = *list;
  *list = *((فراغ **)block);
  إرجاع block;
}

//! Allocate a small/medium sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_from_heap_fallback(
    heap_t *heap, heap_size_class_t *heap_size_class, uint32_t class_idx) {
  span_t *span = heap_size_class->partial_span;
  rpmalloc_assume(heap != 0);
  إذا (EXPECTED(span != 0)) {
    rpmalloc_assert(span->block_تعداد ==
                        _memory_size_class[span->size_class].block_تعداد,
                    "Span block count corrupted");
    rpmalloc_assert(!_rpmalloc_span_is_fully_utilized(span),
                    "Internal failure");
    فراغ *block;
    إذا (span->free_list) {
      // Span local free list is not empty, swap to size class free list
      block = free_list_اسحب(&span->free_list);
      heap_size_class->free_list = span->free_list;
      span->free_list = 0;
    } وإلا {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      فراغ *block_start = pointer_offset(
          span, SPAN_HEADER_SIZE +
                    ((size_t)span->free_list_limit * span->block_size));
      span->free_list_limit += free_list_partial_init(
          &heap_size_class->free_list, &block,
          (فراغ *)((uintptr_t)block_start & ~(_memory_page_size - 1)),
          block_start, span->block_تعداد - span->free_list_limit,
          span->block_size);
    }
    rpmalloc_assert(span->free_list_limit <= span->block_تعداد,
                    "Span block count corrupted");
    span->used_تعداد = span->free_list_limit;

    // Swap in deferred free list if present
    إذا (atomic_load_ptr(&span->free_list_deferred))
      _rpmalloc_span_extract_free_list_deferred(span);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    إذا (!_rpmalloc_span_is_fully_utilized(span))
      إرجاع block;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_span_double_link_list_اسحب_head(&heap_size_class->partial_span,
                                             span);
#إذا RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_تعداد;
    إرجاع block;
  }

  // Find a span in one of the cache levels
  span = _rpmalloc_heap_extract_new_span(heap, heap_size_class, 1, class_idx);
  إذا (EXPECTED(span != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    إرجاع _rpmalloc_span_initialize_new(heap, heap_size_class, span,
                                         class_idx);
  }

  إرجاع 0;
}

//! Allocate a small sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_small(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Small sizes have unique size classes
  ثابت uint32_t class_idx =
      (uint32_t)((size + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_SHIFT);
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  إذا (EXPECTED(heap_size_class->free_list != 0))
    إرجاع free_list_اسحب(&heap_size_class->free_list);
  إرجاع _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_medium(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ثابت uint32_t base_idx =
      (uint32_t)(SMALL_CLASS_تعداد +
                 ((size - (SMALL_SIZE_LIMIT + 1)) >> MEDIUM_GRANULARITY_SHIFT));
  ثابت uint32_t class_idx = _memory_size_class[base_idx].class_idx;
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  إذا (EXPECTED(heap_size_class->free_list != 0))
    إرجاع free_list_اسحب(&heap_size_class->free_list);
  إرجاع _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_large(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  size += SPAN_HEADER_SIZE;
  size_t span_تعداد = size >> _memory_span_size_shift;
  إذا (size & (_memory_span_size - 1))
    ++span_تعداد;

  // Find a span in one of the cache levels
  span_t *span =
      _rpmalloc_heap_extract_new_span(heap, 0, span_تعداد, SIZE_CLASS_LARGE);
  إذا (!span)
    إرجاع span;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(span->span_تعداد >= span_تعداد, "Internal failure");
  span->size_class = SIZE_CLASS_LARGE;
  span->heap = heap;

#إذا RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_تعداد;

  إرجاع pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a huge block by mapping memory pages directly
ساكن فراغ *_rpmalloc_allocate_huge(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  size += SPAN_HEADER_SIZE;
  size_t رقم_pages = size >> _memory_page_size_shift;
  إذا (size & (_memory_page_size - 1))
    ++رقم_pages;
  size_t align_offset = 0;
  span_t *span =
      (span_t *)_rpmalloc_mmap(رقم_pages * _memory_page_size, &align_offset);
  إذا (!span)
    إرجاع span;

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_تعداد = (uint32_t)رقم_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, رقم_pages, _huge_pages_peak);

#إذا RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_تعداد;

  إرجاع pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a block of the given size
ساكن فراغ *_rpmalloc_allocate(heap_t *heap, size_t size) {
  _rpmalloc_stat_add64(&_allocation_counter, 1);
  إذا (EXPECTED(size <= SMALL_SIZE_LIMIT))
    إرجاع _rpmalloc_allocate_small(heap, size);
  وإلا إذا (size <= _memory_medium_size_limit)
    إرجاع _rpmalloc_allocate_medium(heap, size);
  وإلا إذا (size <= LARGE_SIZE_LIMIT)
    إرجاع _rpmalloc_allocate_large(heap, size);
  إرجاع _rpmalloc_allocate_huge(heap, size);
}

ساكن فراغ *_rpmalloc_aligned_allocate(heap_t *heap, size_t alignment,
                                        size_t size) {
  إذا (alignment <= SMALL_GRANULARITY)
    إرجاع _rpmalloc_allocate(heap, size);

#إذا ENABLE_VALIDATE_ARGS
  إذا ((size + alignment) < size) {
    errno = EINVAL;
    إرجاع 0;
  }
  إذا (alignment & (alignment - 1)) {
    errno = EINVAL;
    إرجاع 0;
  }
#endif

  إذا ((alignment <= SPAN_HEADER_SIZE) &&
      ((size + SPAN_HEADER_SIZE) < _memory_medium_size_limit)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t multiple_size = size ? (size + (SPAN_HEADER_SIZE - 1)) &
                                      ~(uintptr_t)(SPAN_HEADER_SIZE - 1)
                                : SPAN_HEADER_SIZE;
    rpmalloc_assert(!(multiple_size % SPAN_HEADER_SIZE),
                    "Failed alignment calculation");
    إذا (multiple_size <= (size + alignment))
      إرجاع _rpmalloc_allocate(heap, multiple_size);
  }

  فراغ *ptr = 0;
  size_t align_mask = alignment - 1;
  إذا (alignment <= _memory_page_size) {
    ptr = _rpmalloc_allocate(heap, size + alignment);
    إذا ((uintptr_t)ptr & align_mask) {
      ptr = (فراغ *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);
      // Mark as having aligned blocks
      span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
      span->flags |= SPAN_FLAG_ALIGNED_BLOCKS;
    }
    إرجاع ptr;
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
  إذا (alignment & align_mask) {
    errno = EINVAL;
    إرجاع 0;
  }
  إذا (alignment >= _memory_span_size) {
    errno = EINVAL;
    إرجاع 0;
  }

  size_t extra_pages = alignment / _memory_page_size;

  // Since each span has a header, we will at least need one extra memory page
  size_t رقم_pages = 1 + (size / _memory_page_size);
  إذا (size & (_memory_page_size - 1))
    ++رقم_pages;

  إذا (extra_pages > رقم_pages)
    رقم_pages = 1 + extra_pages;

  size_t original_pages = رقم_pages;
  size_t limit_pages = (_memory_span_size / _memory_page_size) * 2;
  إذا (limit_pages < (original_pages * 2))
    limit_pages = original_pages * 2;

  size_t mapped_size, align_offset;
  span_t *span;

retry:
  align_offset = 0;
  mapped_size = رقم_pages * _memory_page_size;

  span = (span_t *)_rpmalloc_mmap(mapped_size, &align_offset);
  إذا (!span) {
    errno = ENOMEM;
    إرجاع 0;
  }
  ptr = pointer_offset(span, SPAN_HEADER_SIZE);

  إذا ((uintptr_t)ptr & align_mask)
    ptr = (فراغ *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);

  إذا (((size_t)pointer_diff(ptr, span) >= _memory_span_size) ||
      (pointer_offset(ptr, size) > pointer_offset(span, mapped_size)) ||
      (((uintptr_t)ptr & _memory_span_mask) != (uintptr_t)span)) {
    _rpmalloc_unmap(span, mapped_size, align_offset, mapped_size);
    ++رقم_pages;
    إذا (رقم_pages > limit_pages) {
      errno = EINVAL;
      إرجاع 0;
    }
    اذهب retry;
  }

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_تعداد = (uint32_t)رقم_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, رقم_pages, _huge_pages_peak);

#إذا RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_تعداد;

  _rpmalloc_stat_add64(&_allocation_counter, 1);

  إرجاع ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
ساكن فراغ _rpmalloc_deallocate_direct_small_or_medium(span_t *span,
                                                        فراغ *block) {
  heap_t *heap = span->heap;
  rpmalloc_assert(heap->owner_thread == get_thread_id() ||
                      !heap->owner_thread || heap->finalize,
                  "Internal failure");
  // Add block to free list
  إذا (UNEXPECTED(_rpmalloc_span_is_fully_utilized(span))) {
    span->used_تعداد = span->block_تعداد;
#إذا RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                           span);
#endif
    _rpmalloc_span_double_link_list_add(
        &heap->size_class[span->size_class].partial_span, span);
    --heap->full_span_تعداد;
  }
  *((فراغ **)block) = span->free_list;
  --span->used_تعداد;
  span->free_list = block;
  إذا (UNEXPECTED(span->used_تعداد == span->list_size)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    إذا (span->used_تعداد) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      فراغ *free_list;
      افعل {
        free_list = atomic_exchange_ptr_acquire(&span->free_list_deferred,
                                                INVALID_POINTER);
      } بينما (free_list == INVALID_POINTER);
      atomic_store_ptr_release(&span->free_list_deferred, free_list);
    }
    _rpmalloc_span_double_link_list_remove(
        &heap->size_class[span->size_class].partial_span, span);
    _rpmalloc_span_release_to_cache(heap, span);
  }
}

ساكن فراغ _rpmalloc_deallocate_defer_free_span(heap_t *heap, span_t *span) {
  إذا (span->size_class != SIZE_CLASS_HUGE)
    _rpmalloc_stat_inc(&heap->span_use[span->span_تعداد - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  افعل {
    span->free_list = (فراغ *)atomic_load_ptr(&heap->span_free_deferred);
  } بينما (!atomic_cas_ptr(&heap->span_free_deferred, span, span->free_list));
}

//! Put the block in the deferred free list of the owning span
ساكن فراغ _rpmalloc_deallocate_defer_small_or_medium(span_t *span,
                                                       فراغ *block) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  فراغ *free_list;
  افعل {
    free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } بينما (free_list == INVALID_POINTER);
  *((فراغ **)block) = free_list;
  uint32_t free_تعداد = ++span->list_size;
  صحيح all_deferred_free = (free_تعداد == span->block_تعداد);
  atomic_store_ptr_release(&span->free_list_deferred, block);
  إذا (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
  }
}

ساكن فراغ _rpmalloc_deallocate_small_or_medium(span_t *span, فراغ *p) {
  _rpmalloc_stat_inc_free(span->heap, span->size_class);
  إذا (span->flags & SPAN_FLAG_ALIGNED_BLOCKS) {
    // Realign pointer to block start
    فراغ *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
    p = pointer_offset(p, -(int32_t)(block_offset % span->block_size));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#إذا RPMALLOC_FIRST_CLASS_HEAPS
  صحيح defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#وإلا
  صحيح defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  إذا (!defer)
    _rpmalloc_deallocate_direct_small_or_medium(span, p);
  وإلا
    _rpmalloc_deallocate_defer_small_or_medium(span, p);
}

//! Deallocate the given large memory block to the current heap
ساكن فراغ _rpmalloc_deallocate_large(span_t *span) {
  rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#إذا RPMALLOC_FIRST_CLASS_HEAPS
  صحيح defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#وإلا
  صحيح defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  إذا (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    إرجاع;
  }
  rpmalloc_assert(span->heap->full_span_تعداد, "Heap span counter corrupted");
  --span->heap->full_span_تعداد;
#إذا RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = span->span_تعداد - 1;
  atomic_decr32(&span->heap->span_use[idx].current);
#endif
  heap_t *heap = span->heap;
  rpmalloc_assert(heap, "No thread heap");
#إذا ENABLE_THREAD_CACHE
  ثابت صحيح set_as_reserved =
      ((span->span_تعداد > 1) && (heap->span_cache.تعداد_2 == 0) &&
       !heap->finalize && !heap->spans_reserved);
#وإلا
  ثابت صحيح set_as_reserved =
      ((span->span_تعداد > 1) && !heap->finalize && !heap->spans_reserved);
#endif
  إذا (set_as_reserved) {
    heap->span_reserve = span;
    heap->spans_reserved = span->span_تعداد;
    إذا (span->flags & SPAN_FLAG_MASTER) {
      heap->span_reserve_master = span;
    } وإلا { // SPAN_FLAG_SUBSPAN
      span_t *master = (span_t *)pointer_offset(
          span,
          -(intptr_t)((size_t)span->offset_from_master * _memory_span_size));
      heap->span_reserve_master = master;
      rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->remaining_spans) >=
                          (int32_t)span->span_تعداد,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&heap->span_use[idx].spans_to_reserved);
  } وإلا {
    // Insert into cache list
    rpmalloc_heap_cache_أدرج(heap, span);
  }
}

//! Deallocate the given huge span
ساكن فراغ _rpmalloc_deallocate_huge(span_t *span) {
  rpmalloc_assert(span->heap, "No span heap");
#إذا RPMALLOC_FIRST_CLASS_HEAPS
  صحيح defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#وإلا
  صحيح defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  إذا (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    إرجاع;
  }
  rpmalloc_assert(span->heap->full_span_تعداد, "Heap span counter corrupted");
  --span->heap->full_span_تعداد;
#إذا RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif

  // Oversized allocation, page count is stored in span_count
  size_t رقم_pages = span->span_تعداد;
  _rpmalloc_unmap(span, رقم_pages * _memory_page_size, span->align_offset,
                  رقم_pages * _memory_page_size);
  _rpmalloc_stat_sub(&_huge_pages_current, رقم_pages);
}

//! Deallocate the given block
ساكن فراغ _rpmalloc_deallocate(فراغ *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  إذا (UNEXPECTED(!span))
    إرجاع;
  إذا (EXPECTED(span->size_class < SIZE_CLASS_تعداد))
    _rpmalloc_deallocate_small_or_medium(span, p);
  وإلا إذا (span->size_class == SIZE_CLASS_LARGE)
    _rpmalloc_deallocate_large(span);
  وإلا
    _rpmalloc_deallocate_huge(span);
}

////////////
///
/// Reallocation entry points
///
//////

ساكن size_t _rpmalloc_usable_size(فراغ *p);

//! Reallocate the given block to the given size
ساكن فراغ *_rpmalloc_reallocate(heap_t *heap, فراغ *p, size_t size,
                                  size_t oldsize, unsigned صحيح flags) {
  إذا (p) {
    // Grab the span using guaranteed span alignment
    span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
    إذا (EXPECTED(span->size_class < SIZE_CLASS_تعداد)) {
      // Small/medium sized block
      rpmalloc_assert(span->span_تعداد == 1, "Span counter corrupted");
      فراغ *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
      uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
      uint32_t block_idx = block_offset / span->block_size;
      فراغ *block =
          pointer_offset(blocks_start, (size_t)block_idx * span->block_size);
      إذا (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)span->block_size - pointer_diff(p, block));
      إذا ((size_t)span->block_size >= size) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        إرجاع block;
      }
    } وإلا إذا (span->size_class == SIZE_CLASS_LARGE) {
      // Large block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t رقم_spans = total_size >> _memory_span_size_shift;
      إذا (total_size & (_memory_span_mask - 1))
        ++رقم_spans;
      size_t current_spans = span->span_تعداد;
      فراغ *block = pointer_offset(span, SPAN_HEADER_SIZE);
      إذا (!oldsize)
        oldsize = (current_spans * _memory_span_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      إذا ((current_spans >= رقم_spans) && (total_size >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        إرجاع block;
      }
    } وإلا {
      // Oversized block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t رقم_pages = total_size >> _memory_page_size_shift;
      إذا (total_size & (_memory_page_size - 1))
        ++رقم_pages;
      // Page count is stored in span_count
      size_t current_pages = span->span_تعداد;
      فراغ *block = pointer_offset(span, SPAN_HEADER_SIZE);
      إذا (!oldsize)
        oldsize = (current_pages * _memory_page_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      إذا ((current_pages >= رقم_pages) && (رقم_pages >= (current_pages / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        إرجاع block;
      }
    }
  } وإلا {
    oldsize = 0;
  }

  إذا (!!(flags & RPMALLOC_GROW_OR_FAIL))
    إرجاع 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t lower_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_size =
      (size > lower_bound) ? size : ((size > oldsize) ? lower_bound : size);
  فراغ *block = _rpmalloc_allocate(heap, new_size);
  إذا (p && block) {
    إذا (!(flags & RPMALLOC_NO_PRESERVE))
      memcpy(block, p, oldsize < new_size ? oldsize : new_size);
    _rpmalloc_deallocate(p);
  }

  إرجاع block;
}

ساكن فراغ *_rpmalloc_aligned_reallocate(heap_t *heap, فراغ *ptr,
                                          size_t alignment, size_t size,
                                          size_t oldsize, unsigned صحيح flags) {
  إذا (alignment <= SMALL_GRANULARITY)
    إرجاع _rpmalloc_reallocate(heap, ptr, size, oldsize, flags);

  صحيح no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? _rpmalloc_usable_size(ptr) : 0);
  إذا ((usablesize >= size) && !((uintptr_t)ptr & (alignment - 1))) {
    إذا (no_alloc || (size >= (usablesize / 2)))
      إرجاع ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  فراغ *block =
      (!no_alloc ? _rpmalloc_aligned_allocate(heap, alignment, size) : 0);
  إذا (EXPECTED(block != 0)) {
    إذا (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      إذا (!oldsize)
        oldsize = usablesize;
      memcpy(block, ptr, oldsize < size ? oldsize : size);
    }
    _rpmalloc_deallocate(ptr);
  }
  إرجاع block;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
ساكن size_t _rpmalloc_usable_size(فراغ *p) {
  // Grab the span using guaranteed span alignment
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  إذا (span->size_class < SIZE_CLASS_تعداد) {
    // Small/medium block
    فراغ *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    إرجاع span->block_size -
           ((size_t)pointer_diff(p, blocks_start) % span->block_size);
  }
  إذا (span->size_class == SIZE_CLASS_LARGE) {
    // Large block
    size_t current_spans = span->span_تعداد;
    إرجاع (current_spans * _memory_span_size) - (size_t)pointer_diff(p, span);
  }
  // Oversized block, page count is stored in span_count
  size_t current_pages = span->span_تعداد;
  إرجاع (current_pages * _memory_page_size) - (size_t)pointer_diff(p, span);
}

//! Adjust and optimize the size class properties for the given class
ساكن فراغ _rpmalloc_adjust_size_class(size_t iclass) {
  size_t block_size = _memory_size_class[iclass].block_size;
  size_t block_تعداد = (_memory_span_size - SPAN_HEADER_SIZE) / block_size;

  _memory_size_class[iclass].block_تعداد = (uint16_t)block_تعداد;
  _memory_size_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  إذا (iclass >= SMALL_CLASS_تعداد) {
    size_t prevclass = iclass;
    بينما (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      إذا (_memory_size_class[prevclass].block_تعداد ==
          _memory_size_class[iclass].block_تعداد)
        _rpmalloc_memcpy_const(_memory_size_class + prevclass,
                               _memory_size_class + iclass,
                               حجم(_memory_size_class[iclass]));
      وإلا
        اكسر;
    }
  }
}

//! Initialize the allocator and setup global data
خارجي مضمن صحيح rpmalloc_initialize(فراغ) {
  إذا (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    إرجاع 0;
  }
  إرجاع rpmalloc_initialize_config(0);
}

صحيح rpmalloc_initialize_config(ثابت rpmalloc_config_t *config) {
  إذا (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    إرجاع 0;
  }
  _rpmalloc_initialized = 1;

  إذا (config)
    memcpy(&_memory_config, config, حجم(rpmalloc_config_t));
  وإلا
    _rpmalloc_memset_const(&_memory_config, 0, حجم(rpmalloc_config_t));

  إذا (!_memory_config.memory_map || !_memory_config.memory_unmap) {
    _memory_config.memory_map = _rpmalloc_mmap_os;
    _memory_config.memory_unmap = _rpmalloc_unmap_os;
  }

#إذا PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, حجم(system_info));
  GetSystemInfo(&system_info);
  _memory_map_granularity = system_info.dwAllocationGranularity;
#وإلا
  _memory_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#endif

#إذا RPMALLOC_CONFIGURABLE
  _memory_page_size = _memory_config.page_size;
#وإلا
  _memory_page_size = 0;
#endif
  _memory_huge_pages = 0;
  إذا (!_memory_page_size) {
#إذا PLATFORM_WINDOWS
    _memory_page_size = system_info.dwPageSize;
#وإلا
    _memory_page_size = _memory_map_granularity;
    إذا (_memory_config.enable_huge_pages) {
#إذا defined(__linux__)
      size_t huge_page_size = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      إذا (meminfo) {
        حرف سطر_2[128];
        بينما (!huge_page_size && fgets(سطر_2, حجم(سطر_2) - 1, meminfo)) {
          سطر_2[حجم(سطر_2) - 1] = 0;
          إذا (strstr(سطر_2, "Hugepagesize:"))
            huge_page_size = (size_t)strtol(سطر_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      إذا (huge_page_size) {
        _memory_huge_pages = 1;
        _memory_page_size = huge_page_size;
        _memory_map_granularity = huge_page_size;
      }
#وإلاإذا defined(__FreeBSD__)
      صحيح rc;
      size_t sz = حجم(rc);

      إذا (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, عدم, 0) == 0 &&
          rc == 1) {
        ساكن size_t defsize = 2 * 1024 * 1024;
        صحيح nsize = 0;
        size_t sizes[4] = {0};
        _memory_huge_pages = 1;
        _memory_page_size = defsize;
        إذا ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          لكل (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            إذا (defsize < csize) {
              _memory_page_size = csize;
              اكسر;
            }
          }
        }
        _memory_map_granularity = _memory_page_size;
      }
#وإلاإذا defined(__APPLE__) || defined(__NetBSD__)
      _memory_huge_pages = 1;
      _memory_page_size = 2 * 1024 * 1024;
      _memory_map_granularity = _memory_page_size;
#endif
    }
#endif
  } وإلا {
    إذا (_memory_config.enable_huge_pages)
      _memory_huge_pages = 1;
  }

#إذا PLATFORM_WINDOWS
  إذا (_memory_config.enable_huge_pages) {
    HANDLE token = 0;
    size_t large_page_minimum = GetLargePageMinimum();
    إذا (large_page_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    إذا (token) {
      LUID luid;
      إذا (Lookup_Privilege_قيمة(0, SE_LOCK_MEMORY_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, حجم(token_privileges));
        token_privileges.Privilege_تعداد = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        إذا (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          إذا (GetLastError() == ERROR_SUCCESS)
            _memory_huge_pages = 1;
        }
      }
      CloseHandle(token);
    }
    إذا (_memory_huge_pages) {
      إذا (large_page_minimum > _memory_page_size)
        _memory_page_size = large_page_minimum;
      إذا (large_page_minimum > _memory_map_granularity)
        _memory_map_granularity = large_page_minimum;
    }
  }
#endif

  size_t min_span_size = 256;
  size_t max_page_size;
#إذا UINTPTR_MAX > 0xFFFFFFFF
  max_page_size = 4096ULL * 1024ULL * 1024ULL;
#وإلا
  max_page_size = 4 * 1024 * 1024;
#endif
  إذا (_memory_page_size < min_span_size)
    _memory_page_size = min_span_size;
  إذا (_memory_page_size > max_page_size)
    _memory_page_size = max_page_size;
  _memory_page_size_shift = 0;
  size_t page_size_bit = _memory_page_size;
  بينما (page_size_bit != 1) {
    ++_memory_page_size_shift;
    page_size_bit >>= 1;
  }
  _memory_page_size = ((size_t)1 << _memory_page_size_shift);

#إذا RPMALLOC_CONFIGURABLE
  إذا (!_memory_config.span_size) {
    _memory_span_size = _memory_default_span_size;
    _memory_span_size_shift = _memory_default_span_size_shift;
    _memory_span_mask = _memory_default_span_mask;
  } وإلا {
    size_t span_size = _memory_config.span_size;
    إذا (span_size > (256 * 1024))
      span_size = (256 * 1024);
    _memory_span_size = 4096;
    _memory_span_size_shift = 12;
    بينما (_memory_span_size < span_size) {
      _memory_span_size <<= 1;
      ++_memory_span_size_shift;
    }
    _memory_span_mask = ~(uintptr_t)(_memory_span_size - 1);
  }
#endif

  memory_span_map_تعداد =
      (_memory_config.span_map_تعداد ? _memory_config.span_map_تعداد
                                     : DEFAULT_SPAN_MAP_تعداد);
  إذا ((_memory_span_size * memory_span_map_تعداد) < _memory_page_size)
    memory_span_map_تعداد = (_memory_page_size / _memory_span_size);
  إذا ((_memory_page_size >= _memory_span_size) &&
      ((memory_span_map_تعداد * _memory_span_size) % _memory_page_size))
    memory_span_map_تعداد = (_memory_page_size / _memory_span_size);
  memory_heap_reserve_تعداد = (memory_span_map_تعداد > DEFAULT_SPAN_MAP_تعداد)
                                   ? DEFAULT_SPAN_MAP_تعداد
                                   : memory_span_map_تعداد;

  _memory_config.page_size = _memory_page_size;
  _memory_config.span_size = _memory_span_size;
  _memory_config.span_map_تعداد = memory_span_map_تعداد;
  _memory_config.enable_huge_pages = _memory_huge_pages;

#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  إذا (pthread_مفتاح_create(&_memory_thread_heap, _rpmalloc_heap_release_raw_fc))
    إرجاع -1;
#endif
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_مفتاح = FlsAlloc(&_rpmalloc_thread_destructor);
#endif

  // Setup all small and medium size classes
  size_t iclass = 0;
  _memory_size_class[iclass].block_size = SMALL_GRANULARITY;
  _rpmalloc_adjust_size_class(iclass);
  لكل (iclass = 1; iclass < SMALL_CLASS_تعداد; ++iclass) {
    size_t size = iclass * SMALL_GRANULARITY;
    _memory_size_class[iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  _memory_medium_size_limit = (_memory_span_size - SPAN_HEADER_SIZE) >> 1;
  إذا (_memory_medium_size_limit > MEDIUM_SIZE_LIMIT)
    _memory_medium_size_limit = MEDIUM_SIZE_LIMIT;
  لكل (iclass = 0; iclass < MEDIUM_CLASS_تعداد; ++iclass) {
    size_t size = SMALL_SIZE_LIMIT + ((iclass + 1) * MEDIUM_GRANULARITY);
    إذا (size > _memory_medium_size_limit) {
      _memory_medium_size_limit =
          SMALL_SIZE_LIMIT + (iclass * MEDIUM_GRANULARITY);
      اكسر;
    }
    _memory_size_class[SMALL_CLASS_تعداد + iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(SMALL_CLASS_تعداد + iclass);
  }

  _memory_orphan_heaps = 0;
#إذا RPMALLOC_FIRST_CLASS_HEAPS
  _memory_first_class_orphan_heaps = 0;
#endif
#إذا ENABLE_STATISTICS
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
  memset(_memory_heaps, 0, حجم(_memory_heaps));
  atomic_store32_release(&_memory_global_lock, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  إرجاع 0;
}

//! Finalize the allocator
فراغ rpmalloc_finalize(فراغ) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  إذا (_memory_global_reserve) {
    atomic_add32(&_memory_global_reserve_master->remaining_spans,
                 -(int32_t)memory_global_reserve_تعداد);
    _memory_global_reserve_master = 0;
    memory_global_reserve_تعداد = 0;
    _memory_global_reserve = 0;
  }
  atomic_store32_release(&_memory_global_lock, 0);

  // Free all thread caches and fully free spans
  لكل (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    بينما (heap) {
      heap_t *next_heap = heap->next_heap;
      heap->finalize = 1;
      _rpmalloc_heap_global_finalize(heap);
      heap = next_heap;
    }
  }

#إذا ENABLE_GLOBAL_CACHE
  // Free global caches
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_تعداد; ++iclass)
    _rpmalloc_global_cache_finalize(&_memory_span_cache[iclass]);
#endif

#إذا (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_مفتاح_delete(_memory_thread_heap);
#endif
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_مفتاح);
  fls_مفتاح = 0;
#endif
#إذا ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&_mapped_pages) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&_mapped_pages_os) == 0,
                  "Memory leak detected");
#endif

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
خارجي مضمن فراغ rpmalloc_thread_initialize(فراغ) {
  إذا (!get_thread_heap_raw()) {
    heap_t *heap = _rpmalloc_heap_allocate(0);
    إذا (heap) {
      _rpmalloc_stat_inc(&_memory_active_heaps);
      set_thread_heap(heap);
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_Set_قيمة(fls_مفتاح, heap);
#endif
    }
  }
}

//! Finalize thread, orphan heap
فراغ rpmalloc_thread_finalize(صحيح release_caches) {
  heap_t *heap = get_thread_heap_raw();
  إذا (heap)
    _rpmalloc_heap_release_raw(heap, release_caches);
  set_thread_heap(0);
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_Set_قيمة(fls_مفتاح, 0);
#endif
}

صحيح rpmalloc_is_thread_initialized(فراغ) {
  إرجاع (get_thread_heap_raw() != 0) ? 1 : 0;
}

ثابت rpmalloc_config_t *rpmalloc_config(فراغ) { إرجاع &_memory_config; }

// Extern interface

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpmalloc(size_t size) {
#إذا ENABLE_VALIDATE_ARGS
  إذا (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    إرجاع 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  إرجاع _rpmalloc_allocate(heap, size);
}

خارجي مضمن فراغ rpfree(فراغ *ptr) { _rpmalloc_deallocate(ptr); }

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpcalloc(size_t رقم_2, size_t size) {
  size_t total;
#إذا ENABLE_VALIDATE_ARGS
#إذا PLATFORM_WINDOWS
  صحيح err = SizeTMult(رقم_2, size, &total);
  إذا ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    إرجاع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(رقم_2, size, &total);
  إذا (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    إرجاع 0;
  }
#endif
#وإلا
  total = رقم_2 * size;
#endif
  heap_t *heap = get_thread_heap();
  فراغ *block = _rpmalloc_allocate(heap, total);
  إذا (block)
    memset(block, 0, total);
  إرجاع block;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rprealloc(فراغ *ptr, size_t size) {
#إذا ENABLE_VALIDATE_ARGS
  إذا (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    إرجاع ptr;
  }
#endif
  heap_t *heap = get_thread_heap();
  إرجاع _rpmalloc_reallocate(heap, ptr, size, 0, 0);
}

خارجي RPMALLOC_ALLOCATOR فراغ *rpaligned_realloc(فراغ *ptr, size_t alignment,
                                                  size_t size, size_t oldsize,
                                                  unsigned صحيح flags) {
#إذا ENABLE_VALIDATE_ARGS
  إذا ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    إرجاع 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  إرجاع _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, oldsize,
                                      flags);
}

خارجي RPMALLOC_ALLOCATOR فراغ *rpaligned_alloc(size_t alignment, size_t size) {
  heap_t *heap = get_thread_heap();
  إرجاع _rpmalloc_aligned_allocate(heap, alignment, size);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpaligned_calloc(size_t alignment, size_t رقم_2, size_t size) {
  size_t total;
#إذا ENABLE_VALIDATE_ARGS
#إذا PLATFORM_WINDOWS
  صحيح err = SizeTMult(رقم_2, size, &total);
  إذا ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    إرجاع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(رقم_2, size, &total);
  إذا (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    إرجاع 0;
  }
#endif
#وإلا
  total = رقم_2 * size;
#endif
  فراغ *block = rpaligned_alloc(alignment, total);
  إذا (block)
    memset(block, 0, total);
  إرجاع block;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpmemalign(size_t alignment,
                                                  size_t size) {
  إرجاع rpaligned_alloc(alignment, size);
}

خارجي مضمن صحيح rpposix_memalign(فراغ **memptr, size_t alignment,
                                   size_t size) {
  إذا (memptr)
    *memptr = rpaligned_alloc(alignment, size);
  وإلا
    إرجاع EINVAL;
  إرجاع *memptr ? 0 : ENOMEM;
}

خارجي مضمن size_t rpmalloc_usable_size(فراغ *ptr) {
  إرجاع (ptr ? _rpmalloc_usable_size(ptr) : 0);
}

خارجي مضمن فراغ rpmalloc_thread_collect(فراغ) {}

فراغ rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, حجم(rpmalloc_thread_statistics_t));
  heap_t *heap = get_thread_heap_raw();
  إذا (!heap)
    إرجاع;

  لكل (size_t iclass = 0; iclass < SIZE_CLASS_تعداد; ++iclass) {
    size_class_t *size_class = _memory_size_class + iclass;
    span_t *span = heap->size_class[iclass].partial_span;
    بينما (span) {
      size_t free_تعداد = span->list_size;
      size_t block_تعداد = size_class->block_تعداد;
      إذا (span->free_list_limit < block_تعداد)
        block_تعداد = span->free_list_limit;
      free_تعداد += (block_تعداد - span->used_تعداد);
      stats->sizecache += free_تعداد * size_class->block_size;
      span = span->next;
    }
  }

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_تعداد; ++iclass) {
    span_cache_t *span_cache;
    إذا (!iclass)
      span_cache = &heap->span_cache;
    وإلا
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    stats->spancache += span_cache->تعداد_2 * (iclass + 1) * _memory_span_size;
  }
#endif

  span_t *deferred = (span_t *)atomic_load_ptr(&heap->span_free_deferred);
  بينما (deferred) {
    إذا (deferred->size_class != SIZE_CLASS_HUGE)
      stats->spancache += (size_t)deferred->span_تعداد * _memory_span_size;
    deferred = (span_t *)deferred->free_list;
  }

#إذا ENABLE_STATISTICS
  stats->thread_to_global = (size_t)atomic_load64(&heap->thread_to_global);
  stats->global_to_thread = (size_t)atomic_load64(&heap->global_to_thread);

  لكل (size_t iclass = 0; iclass < LARGE_CLASS_تعداد; ++iclass) {
    stats->span_use[iclass].current =
        (size_t)atomic_load32(&heap->span_use[iclass].current);
    stats->span_use[iclass].peak =
        (size_t)atomic_load32(&heap->span_use[iclass].أعلى);
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
  لكل (size_t iclass = 0; iclass < SIZE_CLASS_تعداد; ++iclass) {
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

فراغ rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, حجم(rpmalloc_global_statistics_t));
#إذا ENABLE_STATISTICS
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
#إذا ENABLE_GLOBAL_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_تعداد; ++iclass) {
    global_cache_t *cache = &_memory_span_cache[iclass];
    بينما (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();
    uint32_t تعداد_2 = cache->تعداد_2;
#إذا ENABLE_UNLIMITED_CACHE
    span_t *current_span = cache->overflow;
    بينما (current_span) {
      ++تعداد_2;
      current_span = current_span->next;
    }
#endif
    atomic_store32_release(&cache->lock, 0);
    stats->cached += تعداد_2 * (iclass + 1) * _memory_span_size;
  }
#endif
}

#إذا ENABLE_STATISTICS

ساكن فراغ _memory_heap_dump_statistics(heap_t *heap, فراغ *file) {
  fprintf(file, "Heap %d stats:\n", heap->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  لكل (size_t iclass = 0; iclass < SIZE_CLASS_تعداد; ++iclass) {
    إذا (!atomic_load32(&heap->size_class_use[iclass].alloc_total))
      استمر;
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
        _memory_size_class[iclass].block_تعداد,
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
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_تعداد; ++iclass) {
    إذا (!atomic_load32(&heap->span_use[iclass].أعلى) &&
        !atomic_load32(&heap->span_use[iclass].spans_map_calls))
      استمر;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&heap->span_use[iclass].current),
        atomic_load32(&heap->span_use[iclass].أعلى),
        atomic_load32(&heap->span_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&heap->span_use[iclass].أعلى) *
         (size_t)_memory_span_size * (iclass + 1)) /
            (size_t)(1024 * 1024),
#إذا ENABLE_THREAD_CACHE
        (unsigned صحيح)(!iclass ? heap->span_cache.تعداد_2
                               : heap->span_large_cache[iclass - 1].تعداد_2),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_to_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_from_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
#وإلا
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
  fprintf(file, "Full spans: %zu\n", heap->full_span_تعداد);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&heap->thread_to_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&heap->global_to_thread) / (size_t)(1024 * 1024));
}

#endif

فراغ rpmalloc_dump_statistics(فراغ *file) {
#إذا ENABLE_STATISTICS
  لكل (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    بينما (heap) {
      صحيح need_dump = 0;
      لكل (size_t iclass = 0; !need_dump && (iclass < SIZE_CLASS_تعداد);
           ++iclass) {
        إذا (!atomic_load32(&heap->size_class_use[iclass].alloc_total)) {
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].free_total),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          استمر;
        }
        need_dump = 1;
      }
      لكل (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_تعداد);
           ++iclass) {
        إذا (!atomic_load32(&heap->span_use[iclass].أعلى) &&
            !atomic_load32(&heap->span_use[iclass].spans_map_calls))
          استمر;
        need_dump = 1;
      }
      إذا (need_dump)
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

#إذا ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_تعداد; ++iclass) {
    global_cache_t *cache = _memory_span_cache + iclass;
    size_t global_cache = (size_t)cache->تعداد_2 * iclass * _memory_span_size;

    size_t global_overflow_cache = 0;
    span_t *span = cache->overflow;
    بينما (span) {
      global_overflow_cache += iclass * _memory_span_size;
      span = span->next;
    }
    إذا (global_cache || global_overflow_cache || cache->أدرج_تعداد ||
        cache->extract_تعداد)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_overflow_cache / (size_t)(1024 * 1024),
              cache->أدرج_تعداد, cache->extract_تعداد);
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
#إذا 0
	int64_t allocated = atomic_load64(&_allocation_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#endif
#endif
  (فراغ)حجم(file);
}

#إذا RPMALLOC_FIRST_CLASS_HEAPS

خارجي مضمن rpmalloc_heap_t *rpmalloc_heap_acquire(فراغ) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  heap_t *heap = _rpmalloc_heap_allocate(1);
  rpmalloc_assume(heap != عدم);
  heap->owner_thread = 0;
  _rpmalloc_stat_inc(&_memory_active_heaps);
  إرجاع heap;
}

خارجي مضمن فراغ rpmalloc_heap_release(rpmalloc_heap_t *heap) {
  إذا (heap)
    _rpmalloc_heap_release(heap, 1, 1);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, size_t size) {
#إذا ENABLE_VALIDATE_ARGS
  إذا (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    إرجاع 0;
  }
#endif
  إرجاع _rpmalloc_allocate(heap, size);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, size_t alignment,
                            size_t size) {
#إذا ENABLE_VALIDATE_ARGS
  إذا (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    إرجاع 0;
  }
#endif
  إرجاع _rpmalloc_aligned_allocate(heap, alignment, size);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, size_t رقم_2, size_t size) {
  إرجاع rpmalloc_heap_aligned_calloc(heap, 0, رقم_2, size);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, size_t alignment,
                             size_t رقم_2, size_t size) {
  size_t total;
#إذا ENABLE_VALIDATE_ARGS
#إذا PLATFORM_WINDOWS
  صحيح err = SizeTMult(رقم_2, size, &total);
  إذا ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    إرجاع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(رقم_2, size, &total);
  إذا (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    إرجاع 0;
  }
#endif
#وإلا
  total = رقم_2 * size;
#endif
  فراغ *block = _rpmalloc_aligned_allocate(heap, alignment, total);
  إذا (block)
    memset(block, 0, total);
  إرجاع block;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, فراغ *ptr, size_t size,
                      unsigned صحيح flags) {
#إذا ENABLE_VALIDATE_ARGS
  إذا (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    إرجاع ptr;
  }
#endif
  إرجاع _rpmalloc_reallocate(heap, ptr, size, 0, flags);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_heap_aligned_realloc(rpmalloc_heap_t *heap, فراغ *ptr,
                              size_t alignment, size_t size,
                              unsigned صحيح flags) {
#إذا ENABLE_VALIDATE_ARGS
  إذا ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    إرجاع 0;
  }
#endif
  إرجاع _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, 0, flags);
}

خارجي مضمن فراغ rpmalloc_heap_free(rpmalloc_heap_t *heap, فراغ *ptr) {
  (فراغ)حجم(heap);
  _rpmalloc_deallocate(ptr);
}

خارجي مضمن فراغ rpmalloc_heap_free_all(rpmalloc_heap_t *heap) {
  span_t *span;
  span_t *next_span;

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  لكل (size_t iclass = 0; iclass < SIZE_CLASS_تعداد; ++iclass) {
    span = heap->size_class[iclass].partial_span;
    بينما (span) {
      next_span = span->next;
      rpmalloc_heap_cache_أدرج(heap, span);
      span = next_span;
    }
    heap->size_class[iclass].partial_span = 0;
    span = heap->full_span[iclass];
    بينما (span) {
      next_span = span->next;
      rpmalloc_heap_cache_أدرج(heap, span);
      span = next_span;
    }

    span = heap->size_class[iclass].cache;
    إذا (span)
      rpmalloc_heap_cache_أدرج(heap, span);
    heap->size_class[iclass].cache = 0;
  }
  memset(heap->size_class, 0, حجم(heap->size_class));
  memset(heap->full_span, 0, حجم(heap->full_span));

  span = heap->large_huge_span;
  بينما (span) {
    next_span = span->next;
    إذا (UNEXPECTED(span->size_class == SIZE_CLASS_HUGE))
      _rpmalloc_deallocate_huge(span);
    وإلا
      rpmalloc_heap_cache_أدرج(heap, span);
    span = next_span;
  }
  heap->large_huge_span = 0;
  heap->full_span_تعداد = 0;

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_تعداد; ++iclass) {
    span_cache_t *span_cache;
    إذا (!iclass)
      span_cache = &heap->span_cache;
    وإلا
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    إذا (!span_cache->تعداد_2)
      استمر;
#إذا ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&heap->thread_to_global,
                         span_cache->تعداد_2 * (iclass + 1) * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                       span_cache->تعداد_2);
    rpmalloc_global_cache_أدرج_spans(span_cache->span, iclass + 1,
                                        span_cache->تعداد_2);
#وإلا
    لكل (size_t ispan = 0; ispan < span_cache->تعداد_2; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
    span_cache->تعداد_2 = 0;
  }
#endif

#إذا ENABLE_STATISTICS
  لكل (size_t iclass = 0; iclass < SIZE_CLASS_تعداد; ++iclass) {
    atomic_store32(&heap->size_class_use[iclass].alloc_current, 0);
    atomic_store32(&heap->size_class_use[iclass].spans_current, 0);
  }
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_تعداد; ++iclass) {
    atomic_store32(&heap->span_use[iclass].current, 0);
  }
#endif
}

خارجي مضمن فراغ rpmalloc_heap_thread_set_current(rpmalloc_heap_t *heap) {
  heap_t *prev_heap = get_thread_heap_raw();
  إذا (prev_heap != heap) {
    set_thread_heap(heap);
    إذا (prev_heap)
      rpmalloc_heap_release(prev_heap);
  }
}

خارجي مضمن rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(فراغ *ptr) {
  // Grab the span, and then the heap from the span
  span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
  إذا (span) {
    إرجاع span->heap;
  }
  إرجاع 0;
}

#endif

#إذا ENABLE_PRELOAD || ENABLE_OVERRIDE

#ضمن "malloc.c"

#endif

فراغ rpmalloc_linker_reference(فراغ) { (فراغ)حجم(_rpmalloc_initialized); }
