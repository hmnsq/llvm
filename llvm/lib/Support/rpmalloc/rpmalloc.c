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

#அந்தர்பாவ "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#எனில் defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wunused-function"
#எனில் __has_warning("-Wreserved-identifier")
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
#எனில் __has_warning("-Wstatic-in-inline")
#pragma clang diagnostic ignored "-Wstatic-in-inline"
#endif
#அல்லதெனில் defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#எனில் !defined(__has_builtin)
#define __has_builtin(b) 0
#endif

#எனில் defined(__GNUC__) || defined(__clang__)

#எனில் __has_builtin(__builtin_memcpy_inline)
#define _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#இல்லையெனில்
#define _rpmalloc_memcpy_const(x, y, s)                                        \
  செய் {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memcpy(x, y, s);                                                           \
  } போது (0)
#endif

#எனில் __has_builtin(__builtin_memset_inline)
#define _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#இல்லையெனில்
#define _rpmalloc_memset_const(x, y, s)                                        \
  செய் {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } போது (0)
#endif
#இல்லையெனில்
#define _rpmalloc_memcpy_const(x, y, s) memcpy(x, y, s)
#define _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#endif

#எனில் __has_builtin(__builtin_assume)
#define rpmalloc_assume(cond) __builtin_assume(cond)
#அல்லதெனில் defined(__GNUC__)
#define rpmalloc_assume(cond)                                                  \
  செய் {                                                                         \
    எனில் (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } போது (0)
#அல்லதெனில் defined(_MSC_VER)
#define rpmalloc_assume(cond) __assume(cond)
#இல்லையெனில்
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
#ifndef DEFAULT_SPAN_MAP_மொத்தம்
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#define DEFAULT_SPAN_MAP_மொத்தம் 64
#endif
#ifndef GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#define GLOBAL_CACHE_MULTIPLIER 8
#endif

#எனில் DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#error Must use global cache எனில் unmap is disabled
#endif

#எனில் DISABLE_UNMAP
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 1
#endif

#எனில் !ENABLE_GLOBAL_CACHE
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 0
#endif

#எனில் !ENABLE_THREAD_CACHE
#undef ENABLE_ADAPTIVE_THREAD_CACHE
#define ENABLE_ADAPTIVE_THREAD_CACHE 0
#endif

#எனில் defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#define PLATFORM_POSIX 0
#இல்லையெனில்
#define PLATFORM_WINDOWS 0
#define PLATFORM_POSIX 1
#endif

/// Platform and arch specifics
#எனில் defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 5105)
#ifndef FORCEINLINE
#define FORCEINLINE உள்வரி __forceinline
#endif
#define _Static_assert static_assert
#இல்லையெனில்
#ifndef FORCEINLINE
#define FORCEINLINE உள்வரி __attribute__((__always_inline__))
#endif
#endif
#எனில் PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#அந்தர்பாவ <windows.h>
#எனில் ENABLE_VALIDATE_ARGS
#அந்தர்பாவ <intsafe.h>
#endif
#இல்லையெனில்
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <unistd.h>
#எனில் defined(__linux__) || defined(__ANDROID__)
#அந்தர்பாவ <sys/prctl.h>
#எனில் !defined(PR_SET_VMA)
#define PR_SET_VMA 0x53564d41
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif
#எனில் defined(__APPLE__)
#அந்தர்பாவ <TargetConditionals.h>
#எனில் !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#அந்தர்பாவ <mach/mach_vm.h>
#அந்தர்பாவ <mach/vm_statistics.h>
#endif
#அந்தர்பாவ <pthread.h>
#endif
#எனில் defined(__HAIKU__) || defined(__TINYC__)
#அந்தர்பாவ <pthread.h>
#endif
#endif

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <string.h>

#எனில் defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#அந்தர்பாவ <fibersapi.h>
நிலையான DWORD fls_சாவி;
#endif

#எனில் PLATFORM_POSIX
#அந்தர்பாவ <sched.h>
#அந்தர்பாவ <sys/mman.h>
#ifdef __FreeBSD__
#அந்தர்பாவ <sys/sysctl.h>
#define MAP_HUGETLB MAP_ALIGNED_SUPER
#ifndef PROT_MAX
#define PROT_MAX(f) 0
#endif
#இல்லையெனில்
#define PROT_MAX(f) 0
#endif
#ifdef __sun
வெளி முழு madvise(caddr_t, size_t, முழு);
#endif
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif
#endif
#அந்தர்பாவ <errno.h>

#எனில் ENABLE_ASSERTS
#undef NDEBUG
#எனில் defined(_MSC_VER) && !defined(_DEBUG)
#define _DEBUG
#endif
#அந்தர்பாவ <assert.h>
#define RPMALLOC_TOSTRING_M(x) #x
#define RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#define rpmalloc_assert(truth, message)                                        \
  செய் {                                                                         \
    எனில் (!(truth)) {                                                            \
      எனில் (_memory_config.error_callback) {                                     \
        _memory_config.error_callback(message " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } இல்லையெனில் {                                                                 \
        ப்ரதிஜ்ஞா((truth) && message);                                            \
      }                                                                        \
    }                                                                          \
  } போது (0)
#இல்லையெனில்
#define rpmalloc_assert(truth, message)                                        \
  செய் {                                                                         \
  } போது (0)
#endif
#எனில் ENABLE_STATISTICS
#அந்தர்பாவ <stdio.h>
#endif

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#எனில் defined(_MSC_VER) && !defined(__clang__)

வகைவரை நிலையற்ற long atomic32_t;
வகைவரை நிலையற்ற long long atomic64_t;
வகைவரை நிலையற்ற வெறுமை *atomicptr_t;

நிலையான FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  திரும்பு (int32_t)InterlockedOr(src, 0);
}
நிலையான FORCEINLINE வெறுமை atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
நிலையான FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  திரும்பு (int32_t)InterlockedIncrement(val);
}
நிலையான FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  திரும்பு (int32_t)InterlockedDecrement(val);
}
நிலையான FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  திரும்பு (int32_t)InterlockedExchangeAdd(val, add) + add;
}
நிலையான FORCEINLINE முழு atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  திரும்பு (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
நிலையான FORCEINLINE வெறுமை atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
நிலையான FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  திரும்பு (int64_t)InterlockedOr64(src, 0);
}
நிலையான FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  திரும்பு (int64_t)InterlockedExchangeAdd64(val, add) + add;
}
நிலையான FORCEINLINE வெறுமை *atomic_load_ptr(atomicptr_t *src) {
  திரும்பு InterlockedCompareExchangePointer(src, 0, 0);
}
நிலையான FORCEINLINE வெறுமை atomic_store_ptr(atomicptr_t *dst, வெறுமை *val) {
  InterlockedExchangePointer(dst, val);
}
நிலையான FORCEINLINE வெறுமை atomic_store_ptr_release(atomicptr_t *dst, வெறுமை *val) {
  InterlockedExchangePointer(dst, val);
}
நிலையான FORCEINLINE வெறுமை *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     வெறுமை *val) {
  திரும்பு (வெறுமை *)InterlockedExchangePointer((வெறுமை *நிலையற்ற *)dst, val);
}
நிலையான FORCEINLINE முழு atomic_cas_ptr(atomicptr_t *dst, வெறுமை *val, வெறுமை *ref) {
  திரும்பு (InterlockedCompareExchangePointer((வெறுமை *நிலையற்ற *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#define EXPECTED(x) (x)
#define UNEXPECTED(x) (x)

#இல்லையெனில்

#அந்தர்பாவ <stdatomic.h>

வகைவரை நிலையற்ற _Atomic(int32_t) atomic32_t;
வகைவரை நிலையற்ற _Atomic(int64_t) atomic64_t;
வகைவரை நிலையற்ற _Atomic(வெறுமை *) atomicptr_t;

நிலையான FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  திரும்பு atomic_load_explicit(src, memory_order_relaxed);
}
நிலையான FORCEINLINE வெறுமை atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
நிலையான FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  திரும்பு atomic_fetch_add_explicit(val, 1, memory_order_relaxed) + 1;
}
நிலையான FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  திரும்பு atomic_fetch_add_explicit(val, -1, memory_order_relaxed) - 1;
}
நிலையான FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  திரும்பு atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
நிலையான FORCEINLINE முழு atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  திரும்பு atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
நிலையான FORCEINLINE வெறுமை atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
நிலையான FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  திரும்பு atomic_load_explicit(val, memory_order_relaxed);
}
நிலையான FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  திரும்பு atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
நிலையான FORCEINLINE வெறுமை *atomic_load_ptr(atomicptr_t *src) {
  திரும்பு atomic_load_explicit(src, memory_order_relaxed);
}
நிலையான FORCEINLINE வெறுமை atomic_store_ptr(atomicptr_t *dst, வெறுமை *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
நிலையான FORCEINLINE வெறுமை atomic_store_ptr_release(atomicptr_t *dst, வெறுமை *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
நிலையான FORCEINLINE வெறுமை *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     வெறுமை *val) {
  திரும்பு atomic_exchange_explicit(dst, val, memory_order_acquire);
}
நிலையான FORCEINLINE முழு atomic_cas_ptr(atomicptr_t *dst, வெறுமை *val, வெறுமை *ref) {
  திரும்பு atomic_compare_exchange_weak_explicit(
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

#எனில் ENABLE_STATISTICS
#define _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#define _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#define _rpmalloc_stat_add(counter, மதிப்பு_2)                                     \
  atomic_add32(counter, (int32_t)(மதிப்பு_2))
#define _rpmalloc_stat_add64(counter, மதிப்பு_2)                                   \
  atomic_add64(counter, (int64_t)(மதிப்பு_2))
#define _rpmalloc_stat_add_peak(counter, மதிப்பு_2, peak)                          \
  செய் {                                                                         \
    int32_t cur_மொத்தம் = atomic_add32(counter, (int32_t)(மதிப்பு_2));              \
    எனில் (cur_மொத்தம் > (peak))                                                   \
      peak = cur_மொத்தம்;                                                       \
  } போது (0)
#define _rpmalloc_stat_sub(counter, மதிப்பு_2)                                     \
  atomic_add32(counter, -(int32_t)(மதிப்பு_2))
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  செய் {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&heap->size_class_use[class_idx].alloc_current);         \
    எனில் (alloc_current > heap->size_class_use[class_idx].alloc_peak)            \
      heap->size_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&heap->size_class_use[class_idx].alloc_total);               \
  } போது (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  செய் {                                                                         \
    atomic_decr32(&heap->size_class_use[class_idx].alloc_current);             \
    atomic_incr32(&heap->size_class_use[class_idx].free_total);                \
  } போது (0)
#இல்லையெனில்
#define _rpmalloc_stat_inc(counter)                                            \
  செய் {                                                                         \
  } போது (0)
#define _rpmalloc_stat_dec(counter)                                            \
  செய் {                                                                         \
  } போது (0)
#define _rpmalloc_stat_add(counter, மதிப்பு_2)                                     \
  செய் {                                                                         \
  } போது (0)
#define _rpmalloc_stat_add64(counter, மதிப்பு_2)                                   \
  செய் {                                                                         \
  } போது (0)
#define _rpmalloc_stat_add_peak(counter, மதிப்பு_2, peak)                          \
  செய் {                                                                         \
  } போது (0)
#define _rpmalloc_stat_sub(counter, மதிப்பு_2)                                     \
  செய் {                                                                         \
  } போது (0)
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  செய் {                                                                         \
  } போது (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  செய் {                                                                         \
  } போது (0)
#endif

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#define SMALL_GRANULARITY 16
//! Small granularity shift count
#define SMALL_GRANULARITY_SHIFT 4
//! Number of small block size classes
#define SMALL_CLASS_மொத்தம் 65
//! Maximum size of a small block
#define SMALL_SIZE_LIMIT (SMALL_GRANULARITY * (SMALL_CLASS_மொத்தம் - 1))
//! Granularity of a medium allocation block
#define MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#define MEDIUM_GRANULARITY_SHIFT 9
//! Number of medium block size classes
#define MEDIUM_CLASS_மொத்தம் 61
//! Total number of small + medium size classes
#define SIZE_CLASS_மொத்தம் (SMALL_CLASS_மொத்தம் + MEDIUM_CLASS_மொத்தம்)
//! Number of large block size classes
#define LARGE_CLASS_மொத்தம் 63
//! Maximum size of a medium block
#define MEDIUM_SIZE_LIMIT                                                      \
  (SMALL_SIZE_LIMIT + (MEDIUM_GRANULARITY * MEDIUM_CLASS_மொத்தம்))
//! Maximum size of a large block
#define LARGE_SIZE_LIMIT                                                       \
  ((LARGE_CLASS_மொத்தம் * _memory_span_size) - SPAN_HEADER_SIZE)
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

#எனில் ENABLE_VALIDATE_ARGS
//! Maximum allocation size to avoid integer overflow
#undef MAX_ALLOC_SIZE
#define MAX_ALLOC_SIZE (((size_t) - 1) - _memory_span_size)
#endif

#define pointer_offset(ptr, ofs) (வெறுமை *)((எழுத்து *)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff(first, second)                                            \
  (ptrdiff_t)((மாறிலி எழுத்து *)(first) - (மாறிலி எழுத்து *)(second))

#define INVALID_POINTER ((வெறுமை *)((uintptr_t) - 1))

#define SIZE_CLASS_LARGE SIZE_CLASS_மொத்தம்
#define SIZE_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
வகைவரை கட்டமைப்பு heap_t heap_t;
//! Span of memory pages
வகைவரை கட்டமைப்பு span_t span_t;
//! Span list
வகைவரை கட்டமைப்பு span_list_t span_list_t;
//! Span active data
வகைவரை கட்டமைப்பு span_active_t span_active_t;
//! Size class definition
வகைவரை கட்டமைப்பு size_class_t size_class_t;
//! Global cache
வகைவரை கட்டமைப்பு global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#define SPAN_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#define SPAN_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#define SPAN_FLAG_ALIGNED_BLOCKS 4U
//! Flag indicating an unmapped master span
#define SPAN_FLAG_UNMAPPED_MASTER 8U

#எனில் ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
கட்டமைப்பு span_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t உயர்வு;
#எனில் ENABLE_STATISTICS
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
வகைவரை கட்டமைப்பு span_use_t span_use_t;
#endif

#எனில் ENABLE_STATISTICS
கட்டமைப்பு size_class_use_t {
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
வகைவரை கட்டமைப்பு size_class_use_t size_class_use_t;
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
கட்டமைப்பு span_t {
  //! Free list
  வெறுமை *free_list;
  //! Total block count of size class
  uint32_t block_மொத்தம்;
  //! Size class
  uint32_t size_class;
  //! Index of last block initialized in free list
  uint32_t free_list_limit;
  //! Number of used blocks remaining when in partial state
  uint32_t used_மொத்தம்;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_size;
  //! Size of a block
  uint32_t block_size;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t span_மொத்தம்;
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
_Static_assert(அளவு(span_t) <= SPAN_HEADER_SIZE, "span size mismatch");

கட்டமைப்பு span_cache_t {
  size_t மொத்தம்;
  span_t *span[MAX_THREAD_SPAN_CACHE];
};
வகைவரை கட்டமைப்பு span_cache_t span_cache_t;

கட்டமைப்பு span_large_cache_t {
  size_t மொத்தம்;
  span_t *span[MAX_THREAD_SPAN_LARGE_CACHE];
};
வகைவரை கட்டமைப்பு span_large_cache_t span_large_cache_t;

கட்டமைப்பு heap_size_class_t {
  //! Free list of active span
  வெறுமை *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  span_t *partial_span;
  //! Early level cache of fully free spans
  span_t *cache;
};
வகைவரை கட்டமைப்பு heap_size_class_t heap_size_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
கட்டமைப்பு heap_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  heap_size_class_t size_class[SIZE_CLASS_மொத்தம்];
#எனில் ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  span_cache_t span_cache;
#endif
  //! List of deferred free spans (single linked list)
  atomicptr_t span_free_deferred;
  //! Number of full spans
  size_t full_span_மொத்தம்;
  //! Mapped but unused spans
  span_t *span_reserve;
  //! Master span for mapped but unused spans
  span_t *span_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t child_மொத்தம்;
  //! Next heap in id list
  heap_t *next_heap;
  //! Next heap in orphan list
  heap_t *next_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  முழு finalize;
  //! Master heap owning the memory pages
  heap_t *master_heap;
#எனில் ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  span_large_cache_t span_large_cache[LARGE_CLASS_மொத்தம் - 1];
#endif
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  span_t *full_span[SIZE_CLASS_மொத்தம்];
  //! Double linked list of large and huge spans allocated by this heap
  span_t *large_huge_span;
#endif
#எனில் ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  span_use_t span_use[LARGE_CLASS_மொத்தம்];
#endif
#எனில் ENABLE_STATISTICS
  //! Allocation stats per size class
  size_class_use_t size_class_use[SIZE_CLASS_மொத்தம் + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_to_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_to_thread;
#endif
};

// Size class for defining a block size bucket
கட்டமைப்பு size_class_t {
  //! Size of blocks in this class
  uint32_t block_size;
  //! Number of blocks in each chunk
  uint16_t block_மொத்தம்;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(அளவு(size_class_t) == 8, "Size class size mismatch");

கட்டமைப்பு global_cache_t {
  //! Cache lock
  atomic32_t lock;
  //! Cache count
  uint32_t மொத்தம்;
#எனில் ENABLE_STATISTICS
  //! Insert count
  size_t செருகு_மொத்தம்;
  //! Extract count
  size_t extract_மொத்தம்;
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
நிலையான முழு _rpmalloc_initialized;
//! Main thread ID
நிலையான uintptr_t _rpmalloc_main_thread_id;
//! Configuration
நிலையான rpmalloc_config_t _memory_config;
//! Memory page size
நிலையான size_t _memory_page_size;
//! Shift to divide by page size
நிலையான size_t _memory_page_size_shift;
//! Granularity at which memory pages are mapped by OS
நிலையான size_t _memory_map_granularity;
#எனில் RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
நிலையான size_t _memory_span_size;
//! Shift to divide by span size
நிலையான size_t _memory_span_size_shift;
//! Mask to get to start of a memory span
நிலையான uintptr_t _memory_span_mask;
#இல்லையெனில்
//! Hardwired span size
#define _memory_span_size _memory_default_span_size
#define _memory_span_size_shift _memory_default_span_size_shift
#define _memory_span_mask _memory_default_span_mask
#endif
//! Number of spans to map in each map call
நிலையான size_t memory_span_map_மொத்தம்;
//! Number of spans to keep reserved in each heap
நிலையான size_t memory_heap_reserve_மொத்தம்;
//! Global size classes
நிலையான size_class_t _memory_size_class[SIZE_CLASS_மொத்தம்];
//! Run-time size limit of medium blocks
நிலையான size_t _memory_medium_size_limit;
//! Heap ID counter
நிலையான atomic32_t _memory_heap_id;
//! Huge page support
நிலையான முழு _memory_huge_pages;
#எனில் ENABLE_GLOBAL_CACHE
//! Global span cache
நிலையான global_cache_t _memory_span_cache[LARGE_CLASS_மொத்தம்];
#endif
//! Global reserved spans
நிலையான span_t *_memory_global_reserve;
//! Global reserved count
நிலையான size_t memory_global_reserve_மொத்தம்;
//! Global reserved master
நிலையான span_t *_memory_global_reserve_master;
//! All heaps
நிலையான heap_t *_memory_heaps[HEAP_ARRAY_SIZE];
//! Used to restrict access to mapping memory for huge pages
நிலையான atomic32_t _memory_global_lock;
//! Orphaned heaps
நிலையான heap_t *_memory_orphan_heaps;
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
நிலையான heap_t *_memory_first_class_orphan_heaps;
#endif
#எனில் ENABLE_STATISTICS
//! Allocations counter
நிலையான atomic64_t _allocation_counter;
//! Deallocations counter
நிலையான atomic64_t _deallocation_counter;
//! Active heap count
நிலையான atomic32_t _memory_active_heaps;
//! Number of currently mapped memory pages
நிலையான atomic32_t _mapped_pages;
//! Peak number of concurrently mapped memory pages
நிலையான int32_t _mapped_pages_peak;
//! Number of mapped master spans
நிலையான atomic32_t _master_spans;
//! Number of unmapped dangling master spans
நிலையான atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
நிலையான atomic32_t _mapped_total;
//! Running counter of total number of unmapped memory pages since start
நிலையான atomic32_t _unmapped_total;
//! Number of currently mapped memory pages in OS calls
நிலையான atomic32_t _mapped_pages_os;
//! Number of currently allocated pages in huge allocations
நிலையான atomic32_t _huge_pages_current;
//! Peak number of currently allocated pages in huge allocations
நிலையான int32_t _huge_pages_peak;
#endif

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#எனில் ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
நிலையான pthread_key_t _memory_thread_heap;
#இல்லையெனில்
#ifdef _MSC_VER
#define _Thread_local __declspec(thread)
#define TLS_MODEL
#இல்லையெனில்
#ifndef __HAIKU__
#define TLS_MODEL __attribute__((tls_model("initial-exec")))
#இல்லையெனில்
#define TLS_MODEL
#endif
#எனில் !defined(__clang__) && defined(__GNUC__)
#define _Thread_local __thread
#endif
#endif
நிலையான _Thread_local heap_t *_memory_thread_heap TLS_MODEL;
#endif

நிலையான உள்வரி heap_t *get_thread_heap_raw(வெறுமை) {
#எனில் (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  திரும்பு pthread_getspecific(_memory_thread_heap);
#இல்லையெனில்
  திரும்பு _memory_thread_heap;
#endif
}

//! Get the current thread heap
நிலையான உள்வரி heap_t *get_thread_heap(வெறுமை) {
  heap_t *heap = get_thread_heap_raw();
#எனில் ENABLE_PRELOAD
  எனில் (EXPECTED(heap != 0))
    திரும்பு heap;
  rpmalloc_initialize();
  திரும்பு get_thread_heap_raw();
#இல்லையெனில்
  திரும்பு heap;
#endif
}

//! Fast thread ID
நிலையான உள்வரி uintptr_t get_thread_id(வெறுமை) {
#எனில் defined(_WIN32)
  திரும்பு (uintptr_t)((வெறுமை *)NtCurrentTeb());
#அல்லதெனில் (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#எனில் defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#அல்லதெனில் defined(__x86_64__)
#எனில் defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#இல்லையெனில்
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#endif
#அல்லதெனில் defined(__arm__)
  __asm__ நிலையற்ற("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#அல்லதெனில் defined(__aarch64__)
#எனில் defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ நிலையற்ற("mrs %0, tpidrro_el0" : "=r"(tid));
#இல்லையெனில்
  __asm__ நிலையற்ற("mrs %0, tpidr_el0" : "=r"(tid));
#endif
#இல்லையெனில்
#error This platform needs implementation of get_thread_id()
#endif
  திரும்பு tid;
#இல்லையெனில்
#error This platform needs implementation of get_thread_id()
#endif
}

//! Set the current thread heap
நிலையான வெறுமை set_thread_heap(heap_t *heap) {
#எனில் ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(_memory_thread_heap, heap);
#இல்லையெனில்
  _memory_thread_heap = heap;
#endif
  எனில் (heap)
    heap->owner_thread = get_thread_id();
}

//! Set main thread ID
வெளி வெறுமை rpmalloc_set_main_thread(வெறுமை);

வெறுமை rpmalloc_set_main_thread(வெறுமை) {
  _rpmalloc_main_thread_id = get_thread_id();
}

நிலையான வெறுமை _rpmalloc_spin(வெறுமை) {
#எனில் defined(_MSC_VER)
#எனில் defined(_M_ARM64)
  __yield();
#இல்லையெனில்
  _mm_pause();
#endif
#அல்லதெனில் defined(__x86_64__) || defined(__i386__)
  __asm__ நிலையற்ற("pause" ::: "memory");
#அல்லதெனில் defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ நிலையற்ற("yield" ::: "memory");
#அல்லதெனில் defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ நிலையற்ற("or 27,27,27");
#அல்லதெனில் defined(__sparc__)
  __asm__ நிலையற்ற("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#இல்லையெனில்
  கட்டமைப்பு timespec ts = {0};
  nanosleep(&ts, 0);
#endif
}

#எனில் defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

நிலையான வெறுமை NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  தேர்வு (reason) {
  நிலை DLL_PROCESS_ATTACH:
    நிறுத்து;
  நிலை DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    நிறுத்து;
  நிலை DLL_THREAD_ATTACH:
    நிறுத்து;
  நிலை DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    நிறுத்து;
  }
}

#ifdef _WIN64
#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#pragma const_seg(".CRT$XLY")

வெளி மாறிலி PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
மாறிலி PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#pragma const_seg()
#இல்லையெனில் // _WIN64
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#pragma data_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#pragma data_seg()
#endif // _WIN64

நிலையான வெறுமை NTAPI _rpmalloc_thread_destructor(வெறுமை *மதிப்பு_2) {
#எனில் ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  எனில் (get_thread_id() == _rpmalloc_main_thread_id)
    திரும்பு;
#endif
  எனில் (மதிப்பு_2)
    rpmalloc_thread_finalize(1);
}
#endif

////////////
///
/// Low level memory map/unmap
///
//////

நிலையான வெறுமை _rpmalloc_set_name(வெறுமை *address, size_t size) {
#எனில் defined(__linux__) || defined(__ANDROID__)
  மாறிலி எழுத்து *name = _memory_huge_pages ? _memory_config.huge_page_name
                                        : _memory_config.page_name;
  எனில் (address == MAP_FAILED || !name)
    திரும்பு;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (வெறுமை)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (uintptr_t)address, size,
              (uintptr_t)name);
#இல்லையெனில்
  (வெறுமை)அளவு(size);
  (வெறுமை)அளவு(address);
#endif
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
நிலையான வெறுமை *_rpmalloc_mmap(size_t size, size_t *offset) {
  rpmalloc_assert(!(size % _memory_page_size), "Invalid mmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
  வெறுமை *address = _memory_config.memory_map(size, offset);
  எனில் (EXPECTED(address != 0)) {
    _rpmalloc_stat_add_peak(&_mapped_pages, (size >> _memory_page_size_shift),
                            _mapped_pages_peak);
    _rpmalloc_stat_add(&_mapped_total, (size >> _memory_page_size_shift));
  }
  திரும்பு address;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
நிலையான வெறுமை _rpmalloc_unmap(வெறுமை *address, size_t size, size_t offset,
                            size_t release) {
  rpmalloc_assert(!release || (release >= size), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  எனில் (release) {
    rpmalloc_assert(!(release % _memory_page_size), "Invalid unmap size");
    _rpmalloc_stat_sub(&_mapped_pages, (release >> _memory_page_size_shift));
    _rpmalloc_stat_add(&_unmapped_total, (release >> _memory_page_size_shift));
  }
  _memory_config.memory_unmap(address, size, offset, release);
}

//! Default implementation to map new pages to virtual memory
நிலையான வெறுமை *_rpmalloc_mmap_os(size_t size, size_t *offset) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((size >= _memory_span_size) &&
                    (_memory_span_size > _memory_map_granularity))
                       ? _memory_span_size
                       : 0;
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
#எனில் PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  வெறுமை *ptr = VirtualAlloc(0, size + padding,
                           (_memory_huge_pages ? MEM_LARGE_PAGES : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
  எனில் (!ptr) {
    எனில் (_memory_config.map_fail_callback) {
      எனில் (_memory_config.map_fail_callback(size + padding))
        திரும்பு _rpmalloc_mmap_os(size, offset);
    } இல்லையெனில் {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    திரும்பு 0;
  }
#இல்லையெனில்
  முழு flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#எனில் defined(__APPLE__) && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
  முழு fd = (முழு)VM_MAKE_TAG(240U);
  எனில் (_memory_huge_pages)
    fd |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
  வெறுமை *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#அல்லதெனில் defined(MAP_HUGETLB)
  வெறுமை *ptr = mmap(0, size + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (_memory_huge_pages ? MAP_HUGETLB : 0) | flags, -1, 0);
#எனில் defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  எனில் ((ptr == MAP_FAILED || !ptr) && _memory_huge_pages) {
    ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    எனில் (ptr && ptr != MAP_FAILED) {
      முழு prm = madvise(ptr, size + padding, MADV_HUGEPAGE);
      (வெறுமை)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#endif
  _rpmalloc_set_name(ptr, size + padding);
#அல்லதெனில் defined(MAP_ALIGNED)
  மாறிலி size_t align =
      (அளவு(size_t) * 8) - (size_t)(__builtin_clzl(size - 1));
  வெறுமை *ptr =
      mmap(0, size + padding, PROT_READ | PROT_WRITE,
           (_memory_huge_pages ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#அல்லதெனில் defined(MAP_ALIGN)
  caddr_t base = (_memory_huge_pages ? (caddr_t)(4 << 20) : 0);
  வெறுமை *ptr = mmap(base, size + padding, PROT_READ | PROT_WRITE,
                   (_memory_huge_pages ? MAP_ALIGN : 0) | flags, -1, 0);
#இல்லையெனில்
  வெறுமை *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#endif
  எனில் ((ptr == MAP_FAILED) || !ptr) {
    எனில் (_memory_config.map_fail_callback) {
      எனில் (_memory_config.map_fail_callback(size + padding))
        திரும்பு _rpmalloc_mmap_os(size, offset);
    } இல்லையெனில் எனில் (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    திரும்பு 0;
  }
#endif
  _rpmalloc_stat_add(&_mapped_pages_os,
                     (int32_t)((size + padding) >> _memory_page_size_shift));
  எனில் (padding) {
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
  திரும்பு ptr;
}

//! Default implementation to unmap pages from virtual memory
நிலையான வெறுமை _rpmalloc_unmap_os(வெறுமை *address, size_t size, size_t offset,
                               size_t release) {
  rpmalloc_assert(release || (offset == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid unmap size");
  எனில் (release && offset) {
    offset <<= 3;
    address = pointer_offset(address, -(int32_t)offset);
    எனில் ((release >= _memory_span_size) &&
        (_memory_span_size > _memory_map_granularity)) {
      // Padding is always one span size
      release += _memory_span_size;
    }
  }
#எனில் !DISABLE_UNMAP
#எனில் PLATFORM_WINDOWS
  எனில் (!VirtualFree(address, release ? 0 : size,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#இல்லையெனில்
  எனில் (release) {
    எனில் (munmap(address, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } இல்லையெனில் {
#எனில் defined(MADV_FREE_REUSABLE)
    முழு ret;
    போது ((ret = madvise(address, size, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    எனில் ((ret == -1) && (errno != 0)) {
#அல்லதெனில் defined(MADV_DONTNEED)
    எனில் (madvise(address, size, MADV_DONTNEED)) {
#அல்லதெனில் defined(MADV_PAGEOUT)
    எனில் (madvise(address, size, MADV_PAGEOUT)) {
#அல்லதெனில் defined(MADV_FREE)
    எனில் (madvise(address, size, MADV_FREE)) {
#இல்லையெனில்
    எனில் (posix_madvise(address, size, POSIX_MADV_DONTNEED)) {
#endif
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#endif
#endif
  எனில் (release)
    _rpmalloc_stat_sub(&_mapped_pages_os, release >> _memory_page_size_shift);
}

நிலையான வெறுமை _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_மொத்தம்);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
நிலையான span_t *_rpmalloc_global_get_reserved_spans(size_t span_மொத்தம்) {
  span_t *span = _memory_global_reserve;
  _rpmalloc_span_mark_as_subspan_unless_master(_memory_global_reserve_master,
                                               span, span_மொத்தம்);
  memory_global_reserve_மொத்தம் -= span_மொத்தம்;
  எனில் (memory_global_reserve_மொத்தம்)
    _memory_global_reserve =
        (span_t *)pointer_offset(span, span_மொத்தம் << _memory_span_size_shift);
  இல்லையெனில்
    _memory_global_reserve = 0;
  திரும்பு span;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
நிலையான வெறுமை _rpmalloc_global_set_reserved_spans(span_t *master, span_t *reserve,
                                                size_t reserve_span_மொத்தம்) {
  _memory_global_reserve_master = master;
  memory_global_reserve_மொத்தம் = reserve_span_மொத்தம்;
  _memory_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
நிலையான வெறுமை _rpmalloc_span_double_link_list_add(span_t **head, span_t *span) {
  எனில் (*head)
    (*head)->prev = span;
  span->next = *head;
  *head = span;
}

//! Pop head span from double linked list
நிலையான வெறுமை rpmalloc_span_double_link_list_எடு_head(span_t **head,
                                                     span_t *span) {
  rpmalloc_assert(*head == span, "Linked list corrupted");
  span = *head;
  *head = span->next;
}

//! Remove a span from double linked list
நிலையான வெறுமை _rpmalloc_span_double_link_list_remove(span_t **head,
                                                   span_t *span) {
  rpmalloc_assert(*head, "Linked list corrupted");
  எனில் (*head == span) {
    *head = span->next;
  } இல்லையெனில் {
    span_t *next_span = span->next;
    span_t *prev_span = span->prev;
    prev_span->next = next_span;
    எனில் (EXPECTED(next_span != 0))
      next_span->prev = prev_span;
  }
}

////////////
///
/// Span control
///
//////

நிலையான வெறுமை rpmalloc_heap_cache_செருகு(heap_t *heap, span_t *span);

நிலையான வெறுமை _rpmalloc_heap_finalize(heap_t *heap);

நிலையான வெறுமை _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_மொத்தம்);

//! Declare the span to be a subspan and store distance from master span and
//! span count
நிலையான வெறுமை _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_மொத்தம்) {
  rpmalloc_assert((subspan != master) || (subspan->flags & SPAN_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  எனில் (subspan != master) {
    subspan->flags = SPAN_FLAG_SUBSPAN;
    subspan->offset_from_master =
        (uint32_t)((uintptr_t)pointer_diff(subspan, master) >>
                   _memory_span_size_shift);
    subspan->align_offset = 0;
  }
  subspan->span_மொத்தம் = (uint32_t)span_மொத்தம்;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
நிலையான span_t *_rpmalloc_span_map_from_reserve(heap_t *heap,
                                               size_t span_மொத்தம்) {
  // Update the heap span reserve
  span_t *span = heap->span_reserve;
  heap->span_reserve =
      (span_t *)pointer_offset(span, span_மொத்தம் * _memory_span_size);
  heap->spans_reserved -= (uint32_t)span_மொத்தம்;

  _rpmalloc_span_mark_as_subspan_unless_master(heap->span_reserve_master, span,
                                               span_மொத்தம்);
  எனில் (span_மொத்தம் <= LARGE_CLASS_மொத்தம்)
    _rpmalloc_stat_inc(&heap->span_use[span_மொத்தம் - 1].spans_from_reserved);

  திரும்பு span;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
நிலையான size_t rpmalloc_span_align_மொத்தம்(size_t span_மொத்தம்) {
  size_t request_மொத்தம் = (span_மொத்தம் > memory_span_map_மொத்தம்)
                             ? span_மொத்தம்
                             : memory_span_map_மொத்தம்;
  எனில் ((_memory_page_size > _memory_span_size) &&
      ((request_மொத்தம் * _memory_span_size) % _memory_page_size))
    request_மொத்தம் +=
        memory_span_map_மொத்தம் - (request_மொத்தம் % memory_span_map_மொத்தம்);
  திரும்பு request_மொத்தம்;
}

//! Setup a newly mapped span
நிலையான வெறுமை _rpmalloc_span_initialize(span_t *span, size_t total_span_மொத்தம்,
                                      size_t span_மொத்தம், size_t align_offset) {
  span->total_spans = (uint32_t)total_span_மொத்தம்;
  span->span_மொத்தம் = (uint32_t)span_மொத்தம்;
  span->align_offset = (uint32_t)align_offset;
  span->flags = SPAN_FLAG_MASTER;
  atomic_store32(&span->remaining_spans, (int32_t)total_span_மொத்தம்);
}

நிலையான வெறுமை _rpmalloc_span_unmap(span_t *span);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
நிலையான span_t *rpmalloc_span_map_aligned_மொத்தம்(heap_t *heap,
                                                size_t span_மொத்தம்) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_span_மொத்தம் = rpmalloc_span_align_மொத்தம்(span_மொத்தம்);
  size_t align_offset = 0;
  span_t *span = (span_t *)_rpmalloc_mmap(
      aligned_span_மொத்தம் * _memory_span_size, &align_offset);
  எனில் (!span)
    திரும்பு 0;
  _rpmalloc_span_initialize(span, aligned_span_மொத்தம், span_மொத்தம், align_offset);
  _rpmalloc_stat_inc(&_master_spans);
  எனில் (span_மொத்தம் <= LARGE_CLASS_மொத்தம்)
    _rpmalloc_stat_inc(&heap->span_use[span_மொத்தம் - 1].spans_map_calls);
  எனில் (aligned_span_மொத்தம் > span_மொத்தம்) {
    span_t *reserved_spans =
        (span_t *)pointer_offset(span, span_மொத்தம் * _memory_span_size);
    size_t reserved_மொத்தம் = aligned_span_மொத்தம் - span_மொத்தம்;
    எனில் (heap->spans_reserved) {
      _rpmalloc_span_mark_as_subspan_unless_master(
          heap->span_reserve_master, heap->span_reserve, heap->spans_reserved);
      rpmalloc_heap_cache_செருகு(heap, heap->span_reserve);
    }
    எனில் (reserved_மொத்தம் > memory_heap_reserve_மொத்தம்) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&_memory_global_lock) == 1,
                      "Global spin lock not held as expected");
      size_t remain_மொத்தம் = reserved_மொத்தம் - memory_heap_reserve_மொத்தம்;
      reserved_மொத்தம் = memory_heap_reserve_மொத்தம்;
      span_t *remain_span = (span_t *)pointer_offset(
          reserved_spans, reserved_மொத்தம் * _memory_span_size);
      எனில் (_memory_global_reserve) {
        _rpmalloc_span_mark_as_subspan_unless_master(
            _memory_global_reserve_master, _memory_global_reserve,
            memory_global_reserve_மொத்தம்);
        _rpmalloc_span_unmap(_memory_global_reserve);
      }
      _rpmalloc_global_set_reserved_spans(span, remain_span, remain_மொத்தம்);
    }
    _rpmalloc_heap_set_reserved_spans(heap, span, reserved_spans,
                                      reserved_மொத்தம்);
  }
  திரும்பு span;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
நிலையான span_t *_rpmalloc_span_map(heap_t *heap, size_t span_மொத்தம்) {
  எனில் (span_மொத்தம் <= heap->spans_reserved)
    திரும்பு _rpmalloc_span_map_from_reserve(heap, span_மொத்தம்);
  span_t *span = 0;
  முழு use_global_reserve =
      (_memory_page_size > _memory_span_size) ||
      (memory_span_map_மொத்தம் > memory_heap_reserve_மொத்தம்);
  எனில் (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    போது (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
    எனில் (memory_global_reserve_மொத்தம் >= span_மொத்தம்) {
      size_t reserve_மொத்தம் =
          (!heap->spans_reserved ? memory_heap_reserve_மொத்தம் : span_மொத்தம்);
      எனில் (memory_global_reserve_மொத்தம் < reserve_மொத்தம்)
        reserve_மொத்தம் = memory_global_reserve_மொத்தம்;
      span = _rpmalloc_global_get_reserved_spans(reserve_மொத்தம்);
      எனில் (span) {
        எனில் (reserve_மொத்தம் > span_மொத்தம்) {
          span_t *reserved_span = (span_t *)pointer_offset(
              span, span_மொத்தம் << _memory_span_size_shift);
          _rpmalloc_heap_set_reserved_spans(heap, _memory_global_reserve_master,
                                            reserved_span,
                                            reserve_மொத்தம் - span_மொத்தம்);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        span->span_மொத்தம் = (uint32_t)span_மொத்தம்;
      }
    }
  }
  எனில் (!span)
    span = rpmalloc_span_map_aligned_மொத்தம்(heap, span_மொத்தம்);
  எனில் (use_global_reserve)
    atomic_store32_release(&_memory_global_lock, 0);
  திரும்பு span;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
நிலையான வெறுமை _rpmalloc_span_unmap(span_t *span) {
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");

  முழு is_master = !!(span->flags & SPAN_FLAG_MASTER);
  span_t *master =
      is_master ? span
                : ((span_t *)pointer_offset(
                      span, -(intptr_t)((uintptr_t)span->offset_from_master *
                                        _memory_span_size)));
  rpmalloc_assert(is_master || (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");

  size_t span_மொத்தம் = span->span_மொத்தம்;
  எனில் (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(span->align_offset == 0, "Span align offset corrupted");
    எனில் (_memory_span_size >= _memory_page_size)
      _rpmalloc_unmap(span, span_மொத்தம் * _memory_span_size, 0, 0);
  } இல்லையெனில் {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    span->flags |=
        SPAN_FLAG_MASTER | SPAN_FLAG_SUBSPAN | SPAN_FLAG_UNMAPPED_MASTER;
    _rpmalloc_stat_add(&_unmapped_master_spans, 1);
  }

  எனில் (atomic_add32(&master->remaining_spans, -(int32_t)span_மொத்தம்) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & SPAN_FLAG_MASTER) &&
                        !!(master->flags & SPAN_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_மொத்தம் = master->span_மொத்தம்;
    எனில் (_memory_span_size < _memory_page_size)
      unmap_மொத்தம் = master->total_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_மொத்தம் * _memory_span_size,
                    master->align_offset,
                    (size_t)master->total_spans * _memory_span_size);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
நிலையான வெறுமை _rpmalloc_span_release_to_cache(heap_t *heap, span_t *span) {
  rpmalloc_assert(heap == span->heap, "Span heap pointer corrupted");
  rpmalloc_assert(span->size_class < SIZE_CLASS_மொத்தம்,
                  "Invalid span size class");
  rpmalloc_assert(span->span_மொத்தம் == 1, "Invalid span count");
#எனில் ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&heap->span_use[0].current);
#endif
  _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
  எனில் (!heap->finalize) {
    _rpmalloc_stat_inc(&heap->span_use[0].spans_to_cache);
    _rpmalloc_stat_inc(&heap->size_class_use[span->size_class].spans_to_cache);
    எனில் (heap->size_class[span->size_class].cache)
      rpmalloc_heap_cache_செருகு(heap,
                                  heap->size_class[span->size_class].cache);
    heap->size_class[span->size_class].cache = span;
  } இல்லையெனில் {
    _rpmalloc_span_unmap(span);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
நிலையான uint32_t free_list_partial_init(வெறுமை **list, வெறுமை **first_block,
                                       வெறுமை *page_start, வெறுமை *block_start,
                                       uint32_t block_மொத்தம்,
                                       uint32_t block_size) {
  rpmalloc_assert(block_மொத்தம், "Internal failure");
  *first_block = block_start;
  எனில் (block_மொத்தம் > 1) {
    வெறுமை *free_block = pointer_offset(block_start, block_size);
    வெறுமை *block_end =
        pointer_offset(block_start, (size_t)block_size * block_மொத்தம்);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    எனில் (block_size < (_memory_page_size >> 1)) {
      வெறுமை *page_end = pointer_offset(page_start, _memory_page_size);
      எனில் (page_end < block_end)
        block_end = page_end;
    }
    *list = free_block;
    block_மொத்தம் = 2;
    வெறுமை *next_block = pointer_offset(free_block, block_size);
    போது (next_block < block_end) {
      *((வெறுமை **)free_block) = next_block;
      free_block = next_block;
      ++block_மொத்தம்;
      next_block = pointer_offset(next_block, block_size);
    }
    *((வெறுமை **)free_block) = 0;
  } இல்லையெனில் {
    *list = 0;
  }
  திரும்பு block_மொத்தம்;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
நிலையான வெறுமை *_rpmalloc_span_initialize_new(heap_t *heap,
                                           heap_size_class_t *heap_size_class,
                                           span_t *span, uint32_t class_idx) {
  rpmalloc_assert(span->span_மொத்தம் == 1, "Internal failure");
  size_class_t *size_class = _memory_size_class + class_idx;
  span->size_class = class_idx;
  span->heap = heap;
  span->flags &= ~SPAN_FLAG_ALIGNED_BLOCKS;
  span->block_size = size_class->block_size;
  span->block_மொத்தம் = size_class->block_மொத்தம்;
  span->free_list = 0;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  வெறுமை *block;
  span->free_list_limit =
      free_list_partial_init(&heap_size_class->free_list, &block, span,
                             pointer_offset(span, SPAN_HEADER_SIZE),
                             size_class->block_மொத்தம், size_class->block_size);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  எனில் (span->free_list_limit < span->block_மொத்தம்) {
    _rpmalloc_span_double_link_list_add(&heap_size_class->partial_span, span);
    span->used_மொத்தம் = span->free_list_limit;
  } இல்லையெனில் {
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_மொத்தம்;
    span->used_மொத்தம் = span->block_மொத்தம்;
  }
  திரும்பு block;
}

நிலையான வெறுமை _rpmalloc_span_extract_free_list_deferred(span_t *span) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  செய் {
    span->free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } போது (span->free_list == INVALID_POINTER);
  span->used_மொத்தம் -= span->list_size;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);
}

நிலையான முழு _rpmalloc_span_is_fully_utilized(span_t *span) {
  rpmalloc_assert(span->free_list_limit <= span->block_மொத்தம்,
                  "Span free list corrupted");
  திரும்பு !span->free_list && (span->free_list_limit >= span->block_மொத்தம்);
}

நிலையான முழு _rpmalloc_span_finalize(heap_t *heap, size_t iclass, span_t *span,
                                   span_t **list_head) {
  வெறுமை *free_list = heap->size_class[iclass].free_list;
  span_t *class_span = (span_t *)((uintptr_t)free_list & _memory_span_mask);
  எனில் (span == class_span) {
    // Adopt the heap class free list back into the span free list
    வெறுமை *block = span->free_list;
    வெறுமை *last_block = 0;
    போது (block) {
      last_block = block;
      block = *((வெறுமை **)block);
    }
    uint32_t free_மொத்தம் = 0;
    block = free_list;
    போது (block) {
      ++free_மொத்தம்;
      block = *((வெறுமை **)block);
    }
    எனில் (last_block) {
      *((வெறுமை **)last_block) = free_list;
    } இல்லையெனில் {
      span->free_list = free_list;
    }
    heap->size_class[iclass].free_list = 0;
    span->used_மொத்தம் -= free_மொத்தம்;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(span->list_size == span->used_மொத்தம், "Memory leak detected");
  எனில் (span->list_size == span->used_மொத்தம்) {
    _rpmalloc_stat_dec(&heap->span_use[0].current);
    _rpmalloc_stat_dec(&heap->size_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    எனில் (list_head)
      _rpmalloc_span_double_link_list_remove(list_head, span);
    _rpmalloc_span_unmap(span);
    திரும்பு 1;
  }
  திரும்பு 0;
}

////////////
///
/// Global cache
///
//////

#எனில் ENABLE_GLOBAL_CACHE

//! Finalize a global cache
நிலையான வெறுமை _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  போது (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

  க்கு (size_t ispan = 0; ispan < cache->மொத்தம்; ++ispan)
    _rpmalloc_span_unmap(cache->span[ispan]);
  cache->மொத்தம் = 0;

  போது (cache->overflow) {
    span_t *span = cache->overflow;
    cache->overflow = span->next;
    _rpmalloc_span_unmap(span);
  }

  atomic_store32_release(&cache->lock, 0);
}

நிலையான வெறுமை rpmalloc_global_cache_செருகு_spans(span_t **span,
                                                size_t span_மொத்தம்,
                                                size_t மொத்தம்) {
  மாறிலி size_t cache_limit =
      (span_மொத்தம் == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_SPAN_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_SPAN_LARGE_CACHE - (span_மொத்தம் >> 1));

  global_cache_t *cache = &_memory_span_cache[span_மொத்தம் - 1];

  size_t செருகு_மொத்தம் = மொத்தம்;
  போது (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#எனில் ENABLE_STATISTICS
  cache->செருகு_மொத்தம் += மொத்தம்;
#endif
  எனில் ((cache->மொத்தம் + செருகு_மொத்தம்) > cache_limit)
    செருகு_மொத்தம் = cache_limit - cache->மொத்தம்;

  memcpy(cache->span + cache->மொத்தம், span, அளவு(span_t *) * செருகு_மொத்தம்);
  cache->மொத்தம் += (uint32_t)செருகு_மொத்தம்;

#எனில் ENABLE_UNLIMITED_CACHE
  போது (செருகு_மொத்தம் < மொத்தம்) {
#இல்லையெனில்
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  போது ((_memory_page_size > _memory_span_size) && (செருகு_மொத்தம் < மொத்தம்)) {
#endif
    span_t *current_span = span[செருகு_மொத்தம்++];
    current_span->next = cache->overflow;
    cache->overflow = current_span;
  }
  atomic_store32_release(&cache->lock, 0);

  span_t *keep = 0;
  க்கு (size_t ispan = செருகு_மொத்தம்; ispan < மொத்தம்; ++ispan) {
    span_t *current_span = span[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    எனில் ((current_span->flags & SPAN_FLAG_MASTER) &&
        (atomic_load32(&current_span->remaining_spans) >
         (int32_t)current_span->span_மொத்தம்)) {
      current_span->next = keep;
      keep = current_span;
    } இல்லையெனில் {
      _rpmalloc_span_unmap(current_span);
    }
  }

  எனில் (keep) {
    போது (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    போது (keep) {
      க்கு (; islot < cache->மொத்தம்; ++islot) {
        span_t *current_span = cache->span[islot];
        எனில் (!(current_span->flags & SPAN_FLAG_MASTER) ||
            ((current_span->flags & SPAN_FLAG_MASTER) &&
             (atomic_load32(&current_span->remaining_spans) <=
              (int32_t)current_span->span_மொத்தம்))) {
          _rpmalloc_span_unmap(current_span);
          cache->span[islot] = keep;
          நிறுத்து;
        }
      }
      எனில் (islot == cache->மொத்தம்)
        நிறுத்து;
      keep = keep->next;
    }

    எனில் (keep) {
      span_t *tail = keep;
      போது (tail->next)
        tail = tail->next;
      tail->next = cache->overflow;
      cache->overflow = keep;
    }

    atomic_store32_release(&cache->lock, 0);
  }
}

நிலையான size_t _rpmalloc_global_cache_extract_spans(span_t **span,
                                                   size_t span_மொத்தம்,
                                                   size_t மொத்தம்) {
  global_cache_t *cache = &_memory_span_cache[span_மொத்தம் - 1];

  size_t extract_மொத்தம் = 0;
  போது (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#எனில் ENABLE_STATISTICS
  cache->extract_மொத்தம் += மொத்தம்;
#endif
  size_t want = மொத்தம் - extract_மொத்தம்;
  எனில் (want > cache->மொத்தம்)
    want = cache->மொத்தம்;

  memcpy(span + extract_மொத்தம், cache->span + (cache->மொத்தம் - want),
         அளவு(span_t *) * want);
  cache->மொத்தம் -= (uint32_t)want;
  extract_மொத்தம் += want;

  போது ((extract_மொத்தம் < மொத்தம்) && cache->overflow) {
    span_t *current_span = cache->overflow;
    span[extract_மொத்தம்++] = current_span;
    cache->overflow = current_span->next;
  }

#எனில் ENABLE_ASSERTS
  க்கு (size_t ispan = 0; ispan < extract_மொத்தம்; ++ispan) {
    rpmalloc_assert(span[ispan]->span_மொத்தம் == span_மொத்தம்,
                    "Global cache span count mismatch");
  }
#endif

  atomic_store32_release(&cache->lock, 0);

  திரும்பு extract_மொத்தம்;
}

#endif

////////////
///
/// Heap control
///
//////

நிலையான வெறுமை _rpmalloc_deallocate_huge(span_t *);

//! Store the given spans as reserve in the given heap
நிலையான வெறுமை _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_மொத்தம்) {
  heap->span_reserve_master = master;
  heap->span_reserve = reserve;
  heap->spans_reserved = (uint32_t)reserve_span_மொத்தம்;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
நிலையான வெறுமை _rpmalloc_heap_cache_adopt_deferred(heap_t *heap,
                                                span_t **single_span) {
  span_t *span = (span_t *)((வெறுமை *)atomic_exchange_ptr_acquire(
      &heap->span_free_deferred, 0));
  போது (span) {
    span_t *next_span = (span_t *)span->free_list;
    rpmalloc_assert(span->heap == heap, "Span heap pointer corrupted");
    எனில் (EXPECTED(span->size_class < SIZE_CLASS_மொத்தம்)) {
      rpmalloc_assert(heap->full_span_மொத்தம், "Heap span counter corrupted");
      --heap->full_span_மொத்தம்;
      _rpmalloc_stat_dec(&heap->span_use[0].spans_deferred);
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
      _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                             span);
#endif
      _rpmalloc_stat_dec(&heap->span_use[0].current);
      _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
      எனில் (single_span && !*single_span)
        *single_span = span;
      இல்லையெனில்
        rpmalloc_heap_cache_செருகு(heap, span);
    } இல்லையெனில் {
      எனில் (span->size_class == SIZE_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(span);
      } இல்லையெனில் {
        rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(heap->full_span_மொத்தம், "Heap span counter corrupted");
        --heap->full_span_மொத்தம்;
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
        _rpmalloc_span_double_link_list_remove(&heap->large_huge_span, span);
#endif
        uint32_t idx = span->span_மொத்தம் - 1;
        _rpmalloc_stat_dec(&heap->span_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&heap->span_use[idx].current);
        எனில் (!idx && single_span && !*single_span)
          *single_span = span;
        இல்லையெனில்
          rpmalloc_heap_cache_செருகு(heap, span);
      }
    }
    span = next_span;
  }
}

நிலையான வெறுமை _rpmalloc_heap_unmap(heap_t *heap) {
  எனில் (!heap->master_heap) {
    எனில் ((heap->finalize > 1) && !atomic_load32(&heap->child_மொத்தம்)) {
      span_t *span = (span_t *)((uintptr_t)heap & _memory_span_mask);
      _rpmalloc_span_unmap(span);
    }
  } இல்லையெனில் {
    எனில் (atomic_decr32(&heap->master_heap->child_மொத்தம்) == 0) {
      _rpmalloc_heap_unmap(heap->master_heap);
    }
  }
}

நிலையான வெறுமை _rpmalloc_heap_global_finalize(heap_t *heap) {
  எனில் (heap->finalize++ > 1) {
    --heap->finalize;
    திரும்பு;
  }

  _rpmalloc_heap_finalize(heap);

#எனில் ENABLE_THREAD_CACHE
  க்கு (size_t iclass = 0; iclass < LARGE_CLASS_மொத்தம்; ++iclass) {
    span_cache_t *span_cache;
    எனில் (!iclass)
      span_cache = &heap->span_cache;
    இல்லையெனில்
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    க்கு (size_t ispan = 0; ispan < span_cache->மொத்தம்; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->மொத்தம் = 0;
  }
#endif

  எனில் (heap->full_span_மொத்தம்) {
    --heap->finalize;
    திரும்பு;
  }

  க்கு (size_t iclass = 0; iclass < SIZE_CLASS_மொத்தம்; ++iclass) {
    எனில் (heap->size_class[iclass].free_list ||
        heap->size_class[iclass].partial_span) {
      --heap->finalize;
      திரும்பு;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap_t *list_heap = _memory_heaps[list_idx];
  எனில் (list_heap == heap) {
    _memory_heaps[list_idx] = heap->next_heap;
  } இல்லையெனில் {
    போது (list_heap->next_heap != heap)
      list_heap = list_heap->next_heap;
    list_heap->next_heap = heap->next_heap;
  }

  _rpmalloc_heap_unmap(heap);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
நிலையான வெறுமை rpmalloc_heap_cache_செருகு(heap_t *heap, span_t *span) {
  எனில் (UNEXPECTED(heap->finalize != 0)) {
    _rpmalloc_span_unmap(span);
    _rpmalloc_heap_global_finalize(heap);
    திரும்பு;
  }
#எனில் ENABLE_THREAD_CACHE
  size_t span_மொத்தம் = span->span_மொத்தம்;
  _rpmalloc_stat_inc(&heap->span_use[span_மொத்தம் - 1].spans_to_cache);
  எனில் (span_மொத்தம் == 1) {
    span_cache_t *span_cache = &heap->span_cache;
    span_cache->span[span_cache->மொத்தம்++] = span;
    எனில் (span_cache->மொத்தம் == MAX_THREAD_SPAN_CACHE) {
      மாறிலி size_t remain_மொத்தம் =
          MAX_THREAD_SPAN_CACHE - THREAD_SPAN_CACHE_TRANSFER;
#எனில் ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           THREAD_SPAN_CACHE_TRANSFER * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_மொத்தம் - 1].spans_to_global,
                         THREAD_SPAN_CACHE_TRANSFER);
      rpmalloc_global_cache_செருகு_spans(span_cache->span + remain_மொத்தம்,
                                          span_மொத்தம்,
                                          THREAD_SPAN_CACHE_TRANSFER);
#இல்லையெனில்
      க்கு (size_t ispan = 0; ispan < THREAD_SPAN_CACHE_TRANSFER; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_மொத்தம் + ispan]);
#endif
      span_cache->மொத்தம் = remain_மொத்தம்;
    }
  } இல்லையெனில் {
    size_t cache_idx = span_மொத்தம் - 2;
    span_large_cache_t *span_cache = heap->span_large_cache + cache_idx;
    span_cache->span[span_cache->மொத்தம்++] = span;
    மாறிலி size_t cache_limit =
        (MAX_THREAD_SPAN_LARGE_CACHE - (span_மொத்தம் >> 1));
    எனில் (span_cache->மொத்தம் == cache_limit) {
      மாறிலி size_t transfer_limit = 2 + (cache_limit >> 2);
      மாறிலி size_t transfer_மொத்தம் =
          (THREAD_SPAN_LARGE_CACHE_TRANSFER <= transfer_limit
               ? THREAD_SPAN_LARGE_CACHE_TRANSFER
               : transfer_limit);
      மாறிலி size_t remain_மொத்தம் = cache_limit - transfer_மொத்தம்;
#எனில் ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           transfer_மொத்தம் * span_மொத்தம் * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_மொத்தம் - 1].spans_to_global,
                         transfer_மொத்தம்);
      rpmalloc_global_cache_செருகு_spans(span_cache->span + remain_மொத்தம்,
                                          span_மொத்தம், transfer_மொத்தம்);
#இல்லையெனில்
      க்கு (size_t ispan = 0; ispan < transfer_மொத்தம்; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_மொத்தம் + ispan]);
#endif
      span_cache->மொத்தம் = remain_மொத்தம்;
    }
  }
#இல்லையெனில்
  (வெறுமை)அளவு(heap);
  _rpmalloc_span_unmap(span);
#endif
}

//! Extract the given number of spans from the different cache levels
நிலையான span_t *_rpmalloc_heap_thread_cache_extract(heap_t *heap,
                                                   size_t span_மொத்தம்) {
  span_t *span = 0;
#எனில் ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  எனில் (span_மொத்தம் == 1)
    span_cache = &heap->span_cache;
  இல்லையெனில்
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_மொத்தம் - 2));
  எனில் (span_cache->மொத்தம்) {
    _rpmalloc_stat_inc(&heap->span_use[span_மொத்தம் - 1].spans_from_cache);
    திரும்பு span_cache->span[--span_cache->மொத்தம்];
  }
#endif
  திரும்பு span;
}

நிலையான span_t *_rpmalloc_heap_thread_cache_deferred_extract(heap_t *heap,
                                                            size_t span_மொத்தம்) {
  span_t *span = 0;
  எனில் (span_மொத்தம் == 1) {
    _rpmalloc_heap_cache_adopt_deferred(heap, &span);
  } இல்லையெனில் {
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
    span = _rpmalloc_heap_thread_cache_extract(heap, span_மொத்தம்);
  }
  திரும்பு span;
}

நிலையான span_t *_rpmalloc_heap_reserved_extract(heap_t *heap,
                                               size_t span_மொத்தம்) {
  எனில் (heap->spans_reserved >= span_மொத்தம்)
    திரும்பு _rpmalloc_span_map(heap, span_மொத்தம்);
  திரும்பு 0;
}

//! Extract a span from the global cache
நிலையான span_t *_rpmalloc_heap_global_cache_extract(heap_t *heap,
                                                   size_t span_மொத்தம்) {
#எனில் ENABLE_GLOBAL_CACHE
#எனில் ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  size_t wanted_மொத்தம்;
  எனில் (span_மொத்தம் == 1) {
    span_cache = &heap->span_cache;
    wanted_மொத்தம் = THREAD_SPAN_CACHE_TRANSFER;
  } இல்லையெனில் {
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_மொத்தம் - 2));
    wanted_மொத்தம் = THREAD_SPAN_LARGE_CACHE_TRANSFER;
  }
  span_cache->மொத்தம் = _rpmalloc_global_cache_extract_spans(
      span_cache->span, span_மொத்தம், wanted_மொத்தம்);
  எனில் (span_cache->மொத்தம்) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_மொத்தம் * span_cache->மொத்தம் * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_மொத்தம் - 1].spans_from_global,
                       span_cache->மொத்தம்);
    திரும்பு span_cache->span[--span_cache->மொத்தம்];
  }
#இல்லையெனில்
  span_t *span = 0;
  size_t மொத்தம் = _rpmalloc_global_cache_extract_spans(&span, span_மொத்தம், 1);
  எனில் (மொத்தம்) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_மொத்தம் * மொத்தம் * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_மொத்தம் - 1].spans_from_global,
                       மொத்தம்);
    திரும்பு span;
  }
#endif
#endif
  (வெறுமை)அளவு(heap);
  (வெறுமை)அளவு(span_மொத்தம்);
  திரும்பு 0;
}

நிலையான வெறுமை _rpmalloc_inc_span_statistics(heap_t *heap, size_t span_மொத்தம்,
                                          uint32_t class_idx) {
  (வெறுமை)அளவு(heap);
  (வெறுமை)அளவு(span_மொத்தம்);
  (வெறுமை)அளவு(class_idx);
#எனில் ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)span_மொத்தம் - 1;
  uint32_t current_மொத்தம் =
      (uint32_t)atomic_incr32(&heap->span_use[idx].current);
  எனில் (current_மொத்தம் > (uint32_t)atomic_load32(&heap->span_use[idx].உயர்வு))
    atomic_store32(&heap->span_use[idx].உயர்வு, (int32_t)current_மொத்தம்);
  _rpmalloc_stat_add_peak(&heap->size_class_use[class_idx].spans_current, 1,
                          heap->size_class_use[class_idx].spans_peak);
#endif
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
நிலையான span_t *
_rpmalloc_heap_extract_new_span(heap_t *heap,
                                heap_size_class_t *heap_size_class,
                                size_t span_மொத்தம், uint32_t class_idx) {
  span_t *span;
#எனில் ENABLE_THREAD_CACHE
  எனில் (heap_size_class && heap_size_class->cache) {
    span = heap_size_class->cache;
    heap_size_class->cache =
        (heap->span_cache.மொத்தம்
             ? heap->span_cache.span[--heap->span_cache.மொத்தம்]
             : 0);
    _rpmalloc_inc_span_statistics(heap, span_மொத்தம், class_idx);
    திரும்பு span;
  }
#endif
  (வெறுமை)அளவு(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t base_span_மொத்தம் = span_மொத்தம்;
  size_t limit_span_மொத்தம் =
      (span_மொத்தம் > 2) ? (span_மொத்தம் + (span_மொத்தம் >> 1)) : span_மொத்தம்;
  எனில் (limit_span_மொத்தம் > LARGE_CLASS_மொத்தம்)
    limit_span_மொத்தம் = LARGE_CLASS_மொத்தம்;
  செய் {
    span = _rpmalloc_heap_thread_cache_extract(heap, span_மொத்தம்);
    எனில் (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_மொத்தம், class_idx);
      திரும்பு span;
    }
    span = _rpmalloc_heap_thread_cache_deferred_extract(heap, span_மொத்தம்);
    எனில் (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_மொத்தம், class_idx);
      திரும்பு span;
    }
    span = _rpmalloc_heap_global_cache_extract(heap, span_மொத்தம்);
    எனில் (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_மொத்தம், class_idx);
      திரும்பு span;
    }
    span = _rpmalloc_heap_reserved_extract(heap, span_மொத்தம்);
    எனில் (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_reserved);
      _rpmalloc_inc_span_statistics(heap, span_மொத்தம், class_idx);
      திரும்பு span;
    }
    ++span_மொத்தம்;
  } போது (span_மொத்தம் <= limit_span_மொத்தம்);
  // Final fallback, map in more virtual memory
  span = _rpmalloc_span_map(heap, base_span_மொத்தம்);
  _rpmalloc_inc_span_statistics(heap, base_span_மொத்தம், class_idx);
  _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_map_calls);
  திரும்பு span;
}

நிலையான வெறுமை _rpmalloc_heap_initialize(heap_t *heap) {
  _rpmalloc_memset_const(heap, 0, அளவு(heap_t));
  // Get a new heap ID
  heap->id = 1 + atomic_incr32(&_memory_heap_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap->next_heap = _memory_heaps[list_idx];
  _memory_heaps[list_idx] = heap;
}

நிலையான வெறுமை _rpmalloc_heap_orphan(heap_t *heap, முழு first_class) {
  heap->owner_thread = (uintptr_t)-1;
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
  heap_t **heap_list =
      (first_class ? &_memory_first_class_orphan_heaps : &_memory_orphan_heaps);
#இல்லையெனில்
  (வெறுமை)அளவு(first_class);
  heap_t **heap_list = &_memory_orphan_heaps;
#endif
  heap->next_orphan = *heap_list;
  *heap_list = heap;
}

//! Allocate a new heap from newly mapped memory pages
நிலையான heap_t *_rpmalloc_heap_allocate_new(வெறுமை) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t heap_size = அளவு(heap_t);
  size_t aligned_heap_size = 16 * ((heap_size + 15) / 16);
  size_t request_heap_மொத்தம் = 16;
  size_t heap_span_மொத்தம் = ((aligned_heap_size * request_heap_மொத்தம்) +
                            அளவு(span_t) + _memory_span_size - 1) /
                           _memory_span_size;
  size_t block_size = _memory_span_size * heap_span_மொத்தம்;
  size_t span_மொத்தம் = heap_span_மொத்தம்;
  span_t *span = 0;
  // If there are global reserved spans, use these first
  எனில் (memory_global_reserve_மொத்தம் >= heap_span_மொத்தம்) {
    span = _rpmalloc_global_get_reserved_spans(heap_span_மொத்தம்);
  }
  எனில் (!span) {
    எனில் (_memory_page_size > block_size) {
      span_மொத்தம் = _memory_page_size / _memory_span_size;
      block_size = _memory_page_size;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_heap_மொத்தம் =
          (block_size - அளவு(span_t)) / aligned_heap_size;
      எனில் (possible_heap_மொத்தம் >= (request_heap_மொத்தம் * 16))
        request_heap_மொத்தம் *= 16;
      இல்லையெனில் எனில் (possible_heap_மொத்தம் < request_heap_மொத்தம்)
        request_heap_மொத்தம் = possible_heap_மொத்தம்;
      heap_span_மொத்தம் = ((aligned_heap_size * request_heap_மொத்தம்) +
                         அளவு(span_t) + _memory_span_size - 1) /
                        _memory_span_size;
    }

    size_t align_offset = 0;
    span = (span_t *)_rpmalloc_mmap(block_size, &align_offset);
    எனில் (!span)
      திரும்பு 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    _rpmalloc_span_initialize(span, span_மொத்தம், heap_span_மொத்தம், align_offset);
  }

  size_t remain_size = _memory_span_size - அளவு(span_t);
  heap_t *heap = (heap_t *)pointer_offset(span, அளவு(span_t));
  _rpmalloc_heap_initialize(heap);

  // Put extra heaps as orphans
  size_t இலக்கம்_heaps = remain_size / aligned_heap_size;
  எனில் (இலக்கம்_heaps < request_heap_மொத்தம்)
    இலக்கம்_heaps = request_heap_மொத்தம்;
  atomic_store32(&heap->child_மொத்தம், (int32_t)இலக்கம்_heaps - 1);
  heap_t *extra_heap = (heap_t *)pointer_offset(heap, aligned_heap_size);
  போது (இலக்கம்_heaps > 1) {
    _rpmalloc_heap_initialize(extra_heap);
    extra_heap->master_heap = heap;
    _rpmalloc_heap_orphan(extra_heap, 1);
    extra_heap = (heap_t *)pointer_offset(extra_heap, aligned_heap_size);
    --இலக்கம்_heaps;
  }

  எனில் (span_மொத்தம் > heap_span_மொத்தம்) {
    // Cap reserved spans
    size_t remain_மொத்தம் = span_மொத்தம் - heap_span_மொத்தம்;
    size_t reserve_மொத்தம் =
        (remain_மொத்தம் > memory_heap_reserve_மொத்தம் ? memory_heap_reserve_மொத்தம்
                                                   : remain_மொத்தம்);
    span_t *remain_span =
        (span_t *)pointer_offset(span, heap_span_மொத்தம் * _memory_span_size);
    _rpmalloc_heap_set_reserved_spans(heap, span, remain_span, reserve_மொத்தம்);

    எனில் (remain_மொத்தம் > reserve_மொத்தம்) {
      // Set to global reserved spans
      remain_span = (span_t *)pointer_offset(remain_span,
                                             reserve_மொத்தம் * _memory_span_size);
      reserve_மொத்தம் = remain_மொத்தம் - reserve_மொத்தம்;
      _rpmalloc_global_set_reserved_spans(span, remain_span, reserve_மொத்தம்);
    }
  }

  திரும்பு heap;
}

நிலையான heap_t *_rpmalloc_heap_extract_orphan(heap_t **heap_list) {
  heap_t *heap = *heap_list;
  *heap_list = (heap ? heap->next_orphan : 0);
  திரும்பு heap;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
நிலையான heap_t *_rpmalloc_heap_allocate(முழு first_class) {
  heap_t *heap = 0;
  போது (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
    _rpmalloc_spin();
  எனில் (first_class == 0)
    heap = _rpmalloc_heap_extract_orphan(&_memory_orphan_heaps);
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
  எனில் (!heap)
    heap = _rpmalloc_heap_extract_orphan(&_memory_first_class_orphan_heaps);
#endif
  எனில் (!heap)
    heap = _rpmalloc_heap_allocate_new();
  atomic_store32_release(&_memory_global_lock, 0);
  எனில் (heap)
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  திரும்பு heap;
}

நிலையான வெறுமை _rpmalloc_heap_release(வெறுமை *heapptr, முழு first_class,
                                   முழு release_cache) {
  heap_t *heap = (heap_t *)heapptr;
  எனில் (!heap)
    திரும்பு;
  // Release thread cache spans back to global cache
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  எனில் (release_cache || heap->finalize) {
#எனில் ENABLE_THREAD_CACHE
    க்கு (size_t iclass = 0; iclass < LARGE_CLASS_மொத்தம்; ++iclass) {
      span_cache_t *span_cache;
      எனில் (!iclass)
        span_cache = &heap->span_cache;
      இல்லையெனில்
        span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
      எனில் (!span_cache->மொத்தம்)
        தொடர்;
#எனில் ENABLE_GLOBAL_CACHE
      எனில் (heap->finalize) {
        க்கு (size_t ispan = 0; ispan < span_cache->மொத்தம்; ++ispan)
          _rpmalloc_span_unmap(span_cache->span[ispan]);
      } இல்லையெனில் {
        _rpmalloc_stat_add64(&heap->thread_to_global, span_cache->மொத்தம் *
                                                          (iclass + 1) *
                                                          _memory_span_size);
        _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                           span_cache->மொத்தம்);
        rpmalloc_global_cache_செருகு_spans(span_cache->span, iclass + 1,
                                            span_cache->மொத்தம்);
      }
#இல்லையெனில்
      க்கு (size_t ispan = 0; ispan < span_cache->மொத்தம்; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
      span_cache->மொத்தம் = 0;
    }
#endif
  }

  எனில் (get_thread_heap_raw() == heap)
    set_thread_heap(0);

#எனில் ENABLE_STATISTICS
  atomic_decr32(&_memory_active_heaps);
  rpmalloc_assert(atomic_load32(&_memory_active_heaps) >= 0,
                  "Still active heaps during finalization");
#endif

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  எனில் (get_thread_id() != _rpmalloc_main_thread_id) {
    போது (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
  }
  _rpmalloc_heap_orphan(heap, first_class);
  atomic_store32_release(&_memory_global_lock, 0);
}

நிலையான வெறுமை _rpmalloc_heap_release_raw(வெறுமை *heapptr, முழு release_cache) {
  _rpmalloc_heap_release(heapptr, 0, release_cache);
}

நிலையான வெறுமை _rpmalloc_heap_release_raw_fc(வெறுமை *heapptr) {
  _rpmalloc_heap_release_raw(heapptr, 1);
}

நிலையான வெறுமை _rpmalloc_heap_finalize(heap_t *heap) {
  எனில் (heap->spans_reserved) {
    span_t *span = _rpmalloc_span_map(heap, heap->spans_reserved);
    _rpmalloc_span_unmap(span);
    heap->spans_reserved = 0;
  }

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  க்கு (size_t iclass = 0; iclass < SIZE_CLASS_மொத்தம்; ++iclass) {
    எனில் (heap->size_class[iclass].cache)
      _rpmalloc_span_unmap(heap->size_class[iclass].cache);
    heap->size_class[iclass].cache = 0;
    span_t *span = heap->size_class[iclass].partial_span;
    போது (span) {
      span_t *next = span->next;
      _rpmalloc_span_finalize(heap, iclass, span,
                              &heap->size_class[iclass].partial_span);
      span = next;
    }
    // If class still has a free list it must be a full span
    எனில் (heap->size_class[iclass].free_list) {
      span_t *class_span =
          (span_t *)((uintptr_t)heap->size_class[iclass].free_list &
                     _memory_span_mask);
      span_t **list = 0;
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
      list = &heap->full_span[iclass];
#endif
      --heap->full_span_மொத்தம்;
      எனில் (!_rpmalloc_span_finalize(heap, iclass, class_span, list)) {
        எனில் (list)
          _rpmalloc_span_double_link_list_remove(list, class_span);
        _rpmalloc_span_double_link_list_add(
            &heap->size_class[iclass].partial_span, class_span);
      }
    }
  }

#எனில் ENABLE_THREAD_CACHE
  க்கு (size_t iclass = 0; iclass < LARGE_CLASS_மொத்தம்; ++iclass) {
    span_cache_t *span_cache;
    எனில் (!iclass)
      span_cache = &heap->span_cache;
    இல்லையெனில்
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    க்கு (size_t ispan = 0; ispan < span_cache->மொத்தம்; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->மொத்தம் = 0;
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
நிலையான வெறுமை *free_list_எடு(வெறுமை **list) {
  வெறுமை *block = *list;
  *list = *((வெறுமை **)block);
  திரும்பு block;
}

//! Allocate a small/medium sized memory block from the given heap
நிலையான வெறுமை *_rpmalloc_allocate_from_heap_fallback(
    heap_t *heap, heap_size_class_t *heap_size_class, uint32_t class_idx) {
  span_t *span = heap_size_class->partial_span;
  rpmalloc_assume(heap != 0);
  எனில் (EXPECTED(span != 0)) {
    rpmalloc_assert(span->block_மொத்தம் ==
                        _memory_size_class[span->size_class].block_மொத்தம்,
                    "Span block count corrupted");
    rpmalloc_assert(!_rpmalloc_span_is_fully_utilized(span),
                    "Internal failure");
    வெறுமை *block;
    எனில் (span->free_list) {
      // Span local free list is not empty, swap to size class free list
      block = free_list_எடு(&span->free_list);
      heap_size_class->free_list = span->free_list;
      span->free_list = 0;
    } இல்லையெனில் {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      வெறுமை *block_start = pointer_offset(
          span, SPAN_HEADER_SIZE +
                    ((size_t)span->free_list_limit * span->block_size));
      span->free_list_limit += free_list_partial_init(
          &heap_size_class->free_list, &block,
          (வெறுமை *)((uintptr_t)block_start & ~(_memory_page_size - 1)),
          block_start, span->block_மொத்தம் - span->free_list_limit,
          span->block_size);
    }
    rpmalloc_assert(span->free_list_limit <= span->block_மொத்தம்,
                    "Span block count corrupted");
    span->used_மொத்தம் = span->free_list_limit;

    // Swap in deferred free list if present
    எனில் (atomic_load_ptr(&span->free_list_deferred))
      _rpmalloc_span_extract_free_list_deferred(span);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    எனில் (!_rpmalloc_span_is_fully_utilized(span))
      திரும்பு block;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_span_double_link_list_எடு_head(&heap_size_class->partial_span,
                                             span);
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_மொத்தம்;
    திரும்பு block;
  }

  // Find a span in one of the cache levels
  span = _rpmalloc_heap_extract_new_span(heap, heap_size_class, 1, class_idx);
  எனில் (EXPECTED(span != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    திரும்பு _rpmalloc_span_initialize_new(heap, heap_size_class, span,
                                         class_idx);
  }

  திரும்பு 0;
}

//! Allocate a small sized memory block from the given heap
நிலையான வெறுமை *_rpmalloc_allocate_small(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Small sizes have unique size classes
  மாறிலி uint32_t class_idx =
      (uint32_t)((size + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_SHIFT);
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  எனில் (EXPECTED(heap_size_class->free_list != 0))
    திரும்பு free_list_எடு(&heap_size_class->free_list);
  திரும்பு _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
நிலையான வெறுமை *_rpmalloc_allocate_medium(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  மாறிலி uint32_t base_idx =
      (uint32_t)(SMALL_CLASS_மொத்தம் +
                 ((size - (SMALL_SIZE_LIMIT + 1)) >> MEDIUM_GRANULARITY_SHIFT));
  மாறிலி uint32_t class_idx = _memory_size_class[base_idx].class_idx;
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  எனில் (EXPECTED(heap_size_class->free_list != 0))
    திரும்பு free_list_எடு(&heap_size_class->free_list);
  திரும்பு _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
நிலையான வெறுமை *_rpmalloc_allocate_large(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  size += SPAN_HEADER_SIZE;
  size_t span_மொத்தம் = size >> _memory_span_size_shift;
  எனில் (size & (_memory_span_size - 1))
    ++span_மொத்தம்;

  // Find a span in one of the cache levels
  span_t *span =
      _rpmalloc_heap_extract_new_span(heap, 0, span_மொத்தம், SIZE_CLASS_LARGE);
  எனில் (!span)
    திரும்பு span;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(span->span_மொத்தம் >= span_மொத்தம், "Internal failure");
  span->size_class = SIZE_CLASS_LARGE;
  span->heap = heap;

#எனில் RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_மொத்தம்;

  திரும்பு pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a huge block by mapping memory pages directly
நிலையான வெறுமை *_rpmalloc_allocate_huge(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  size += SPAN_HEADER_SIZE;
  size_t இலக்கம்_pages = size >> _memory_page_size_shift;
  எனில் (size & (_memory_page_size - 1))
    ++இலக்கம்_pages;
  size_t align_offset = 0;
  span_t *span =
      (span_t *)_rpmalloc_mmap(இலக்கம்_pages * _memory_page_size, &align_offset);
  எனில் (!span)
    திரும்பு span;

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_மொத்தம் = (uint32_t)இலக்கம்_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, இலக்கம்_pages, _huge_pages_peak);

#எனில் RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_மொத்தம்;

  திரும்பு pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a block of the given size
நிலையான வெறுமை *_rpmalloc_allocate(heap_t *heap, size_t size) {
  _rpmalloc_stat_add64(&_allocation_counter, 1);
  எனில் (EXPECTED(size <= SMALL_SIZE_LIMIT))
    திரும்பு _rpmalloc_allocate_small(heap, size);
  இல்லையெனில் எனில் (size <= _memory_medium_size_limit)
    திரும்பு _rpmalloc_allocate_medium(heap, size);
  இல்லையெனில் எனில் (size <= LARGE_SIZE_LIMIT)
    திரும்பு _rpmalloc_allocate_large(heap, size);
  திரும்பு _rpmalloc_allocate_huge(heap, size);
}

நிலையான வெறுமை *_rpmalloc_aligned_allocate(heap_t *heap, size_t alignment,
                                        size_t size) {
  எனில் (alignment <= SMALL_GRANULARITY)
    திரும்பு _rpmalloc_allocate(heap, size);

#எனில் ENABLE_VALIDATE_ARGS
  எனில் ((size + alignment) < size) {
    errno = EINVAL;
    திரும்பு 0;
  }
  எனில் (alignment & (alignment - 1)) {
    errno = EINVAL;
    திரும்பு 0;
  }
#endif

  எனில் ((alignment <= SPAN_HEADER_SIZE) &&
      ((size + SPAN_HEADER_SIZE) < _memory_medium_size_limit)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t multiple_size = size ? (size + (SPAN_HEADER_SIZE - 1)) &
                                      ~(uintptr_t)(SPAN_HEADER_SIZE - 1)
                                : SPAN_HEADER_SIZE;
    rpmalloc_assert(!(multiple_size % SPAN_HEADER_SIZE),
                    "Failed alignment calculation");
    எனில் (multiple_size <= (size + alignment))
      திரும்பு _rpmalloc_allocate(heap, multiple_size);
  }

  வெறுமை *ptr = 0;
  size_t align_mask = alignment - 1;
  எனில் (alignment <= _memory_page_size) {
    ptr = _rpmalloc_allocate(heap, size + alignment);
    எனில் ((uintptr_t)ptr & align_mask) {
      ptr = (வெறுமை *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);
      // Mark as having aligned blocks
      span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
      span->flags |= SPAN_FLAG_ALIGNED_BLOCKS;
    }
    திரும்பு ptr;
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
  எனில் (alignment & align_mask) {
    errno = EINVAL;
    திரும்பு 0;
  }
  எனில் (alignment >= _memory_span_size) {
    errno = EINVAL;
    திரும்பு 0;
  }

  size_t extra_pages = alignment / _memory_page_size;

  // Since each span has a header, we will at least need one extra memory page
  size_t இலக்கம்_pages = 1 + (size / _memory_page_size);
  எனில் (size & (_memory_page_size - 1))
    ++இலக்கம்_pages;

  எனில் (extra_pages > இலக்கம்_pages)
    இலக்கம்_pages = 1 + extra_pages;

  size_t original_pages = இலக்கம்_pages;
  size_t limit_pages = (_memory_span_size / _memory_page_size) * 2;
  எனில் (limit_pages < (original_pages * 2))
    limit_pages = original_pages * 2;

  size_t mapped_size, align_offset;
  span_t *span;

retry:
  align_offset = 0;
  mapped_size = இலக்கம்_pages * _memory_page_size;

  span = (span_t *)_rpmalloc_mmap(mapped_size, &align_offset);
  எனில் (!span) {
    errno = ENOMEM;
    திரும்பு 0;
  }
  ptr = pointer_offset(span, SPAN_HEADER_SIZE);

  எனில் ((uintptr_t)ptr & align_mask)
    ptr = (வெறுமை *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);

  எனில் (((size_t)pointer_diff(ptr, span) >= _memory_span_size) ||
      (pointer_offset(ptr, size) > pointer_offset(span, mapped_size)) ||
      (((uintptr_t)ptr & _memory_span_mask) != (uintptr_t)span)) {
    _rpmalloc_unmap(span, mapped_size, align_offset, mapped_size);
    ++இலக்கம்_pages;
    எனில் (இலக்கம்_pages > limit_pages) {
      errno = EINVAL;
      திரும்பு 0;
    }
    செல் retry;
  }

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_மொத்தம் = (uint32_t)இலக்கம்_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, இலக்கம்_pages, _huge_pages_peak);

#எனில் RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_மொத்தம்;

  _rpmalloc_stat_add64(&_allocation_counter, 1);

  திரும்பு ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
நிலையான வெறுமை _rpmalloc_deallocate_direct_small_or_medium(span_t *span,
                                                        வெறுமை *block) {
  heap_t *heap = span->heap;
  rpmalloc_assert(heap->owner_thread == get_thread_id() ||
                      !heap->owner_thread || heap->finalize,
                  "Internal failure");
  // Add block to free list
  எனில் (UNEXPECTED(_rpmalloc_span_is_fully_utilized(span))) {
    span->used_மொத்தம் = span->block_மொத்தம்;
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                           span);
#endif
    _rpmalloc_span_double_link_list_add(
        &heap->size_class[span->size_class].partial_span, span);
    --heap->full_span_மொத்தம்;
  }
  *((வெறுமை **)block) = span->free_list;
  --span->used_மொத்தம்;
  span->free_list = block;
  எனில் (UNEXPECTED(span->used_மொத்தம் == span->list_size)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    எனில் (span->used_மொத்தம்) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      வெறுமை *free_list;
      செய் {
        free_list = atomic_exchange_ptr_acquire(&span->free_list_deferred,
                                                INVALID_POINTER);
      } போது (free_list == INVALID_POINTER);
      atomic_store_ptr_release(&span->free_list_deferred, free_list);
    }
    _rpmalloc_span_double_link_list_remove(
        &heap->size_class[span->size_class].partial_span, span);
    _rpmalloc_span_release_to_cache(heap, span);
  }
}

நிலையான வெறுமை _rpmalloc_deallocate_defer_free_span(heap_t *heap, span_t *span) {
  எனில் (span->size_class != SIZE_CLASS_HUGE)
    _rpmalloc_stat_inc(&heap->span_use[span->span_மொத்தம் - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  செய் {
    span->free_list = (வெறுமை *)atomic_load_ptr(&heap->span_free_deferred);
  } போது (!atomic_cas_ptr(&heap->span_free_deferred, span, span->free_list));
}

//! Put the block in the deferred free list of the owning span
நிலையான வெறுமை _rpmalloc_deallocate_defer_small_or_medium(span_t *span,
                                                       வெறுமை *block) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  வெறுமை *free_list;
  செய் {
    free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } போது (free_list == INVALID_POINTER);
  *((வெறுமை **)block) = free_list;
  uint32_t free_மொத்தம் = ++span->list_size;
  முழு all_deferred_free = (free_மொத்தம் == span->block_மொத்தம்);
  atomic_store_ptr_release(&span->free_list_deferred, block);
  எனில் (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
  }
}

நிலையான வெறுமை _rpmalloc_deallocate_small_or_medium(span_t *span, வெறுமை *p) {
  _rpmalloc_stat_inc_free(span->heap, span->size_class);
  எனில் (span->flags & SPAN_FLAG_ALIGNED_BLOCKS) {
    // Realign pointer to block start
    வெறுமை *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
    p = pointer_offset(p, -(int32_t)(block_offset % span->block_size));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
  முழு defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#இல்லையெனில்
  முழு defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  எனில் (!defer)
    _rpmalloc_deallocate_direct_small_or_medium(span, p);
  இல்லையெனில்
    _rpmalloc_deallocate_defer_small_or_medium(span, p);
}

//! Deallocate the given large memory block to the current heap
நிலையான வெறுமை _rpmalloc_deallocate_large(span_t *span) {
  rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
  முழு defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#இல்லையெனில்
  முழு defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  எனில் (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    திரும்பு;
  }
  rpmalloc_assert(span->heap->full_span_மொத்தம், "Heap span counter corrupted");
  --span->heap->full_span_மொத்தம்;
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif
#எனில் ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = span->span_மொத்தம் - 1;
  atomic_decr32(&span->heap->span_use[idx].current);
#endif
  heap_t *heap = span->heap;
  rpmalloc_assert(heap, "No thread heap");
#எனில் ENABLE_THREAD_CACHE
  மாறிலி முழு set_as_reserved =
      ((span->span_மொத்தம் > 1) && (heap->span_cache.மொத்தம் == 0) &&
       !heap->finalize && !heap->spans_reserved);
#இல்லையெனில்
  மாறிலி முழு set_as_reserved =
      ((span->span_மொத்தம் > 1) && !heap->finalize && !heap->spans_reserved);
#endif
  எனில் (set_as_reserved) {
    heap->span_reserve = span;
    heap->spans_reserved = span->span_மொத்தம்;
    எனில் (span->flags & SPAN_FLAG_MASTER) {
      heap->span_reserve_master = span;
    } இல்லையெனில் { // SPAN_FLAG_SUBSPAN
      span_t *master = (span_t *)pointer_offset(
          span,
          -(intptr_t)((size_t)span->offset_from_master * _memory_span_size));
      heap->span_reserve_master = master;
      rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->remaining_spans) >=
                          (int32_t)span->span_மொத்தம்,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&heap->span_use[idx].spans_to_reserved);
  } இல்லையெனில் {
    // Insert into cache list
    rpmalloc_heap_cache_செருகு(heap, span);
  }
}

//! Deallocate the given huge span
நிலையான வெறுமை _rpmalloc_deallocate_huge(span_t *span) {
  rpmalloc_assert(span->heap, "No span heap");
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
  முழு defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#இல்லையெனில்
  முழு defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  எனில் (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    திரும்பு;
  }
  rpmalloc_assert(span->heap->full_span_மொத்தம், "Heap span counter corrupted");
  --span->heap->full_span_மொத்தம்;
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif

  // Oversized allocation, page count is stored in span_count
  size_t இலக்கம்_pages = span->span_மொத்தம்;
  _rpmalloc_unmap(span, இலக்கம்_pages * _memory_page_size, span->align_offset,
                  இலக்கம்_pages * _memory_page_size);
  _rpmalloc_stat_sub(&_huge_pages_current, இலக்கம்_pages);
}

//! Deallocate the given block
நிலையான வெறுமை _rpmalloc_deallocate(வெறுமை *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  எனில் (UNEXPECTED(!span))
    திரும்பு;
  எனில் (EXPECTED(span->size_class < SIZE_CLASS_மொத்தம்))
    _rpmalloc_deallocate_small_or_medium(span, p);
  இல்லையெனில் எனில் (span->size_class == SIZE_CLASS_LARGE)
    _rpmalloc_deallocate_large(span);
  இல்லையெனில்
    _rpmalloc_deallocate_huge(span);
}

////////////
///
/// Reallocation entry points
///
//////

நிலையான size_t _rpmalloc_usable_size(வெறுமை *p);

//! Reallocate the given block to the given size
நிலையான வெறுமை *_rpmalloc_reallocate(heap_t *heap, வெறுமை *p, size_t size,
                                  size_t oldsize, unsigned முழு flags) {
  எனில் (p) {
    // Grab the span using guaranteed span alignment
    span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
    எனில் (EXPECTED(span->size_class < SIZE_CLASS_மொத்தம்)) {
      // Small/medium sized block
      rpmalloc_assert(span->span_மொத்தம் == 1, "Span counter corrupted");
      வெறுமை *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
      uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
      uint32_t block_idx = block_offset / span->block_size;
      வெறுமை *block =
          pointer_offset(blocks_start, (size_t)block_idx * span->block_size);
      எனில் (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)span->block_size - pointer_diff(p, block));
      எனில் ((size_t)span->block_size >= size) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        எனில் ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        திரும்பு block;
      }
    } இல்லையெனில் எனில் (span->size_class == SIZE_CLASS_LARGE) {
      // Large block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t இலக்கம்_spans = total_size >> _memory_span_size_shift;
      எனில் (total_size & (_memory_span_mask - 1))
        ++இலக்கம்_spans;
      size_t current_spans = span->span_மொத்தம்;
      வெறுமை *block = pointer_offset(span, SPAN_HEADER_SIZE);
      எனில் (!oldsize)
        oldsize = (current_spans * _memory_span_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      எனில் ((current_spans >= இலக்கம்_spans) && (total_size >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        எனில் ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        திரும்பு block;
      }
    } இல்லையெனில் {
      // Oversized block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t இலக்கம்_pages = total_size >> _memory_page_size_shift;
      எனில் (total_size & (_memory_page_size - 1))
        ++இலக்கம்_pages;
      // Page count is stored in span_count
      size_t current_pages = span->span_மொத்தம்;
      வெறுமை *block = pointer_offset(span, SPAN_HEADER_SIZE);
      எனில் (!oldsize)
        oldsize = (current_pages * _memory_page_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      எனில் ((current_pages >= இலக்கம்_pages) && (இலக்கம்_pages >= (current_pages / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        எனில் ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        திரும்பு block;
      }
    }
  } இல்லையெனில் {
    oldsize = 0;
  }

  எனில் (!!(flags & RPMALLOC_GROW_OR_FAIL))
    திரும்பு 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t lower_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_size =
      (size > lower_bound) ? size : ((size > oldsize) ? lower_bound : size);
  வெறுமை *block = _rpmalloc_allocate(heap, new_size);
  எனில் (p && block) {
    எனில் (!(flags & RPMALLOC_NO_PRESERVE))
      memcpy(block, p, oldsize < new_size ? oldsize : new_size);
    _rpmalloc_deallocate(p);
  }

  திரும்பு block;
}

நிலையான வெறுமை *_rpmalloc_aligned_reallocate(heap_t *heap, வெறுமை *ptr,
                                          size_t alignment, size_t size,
                                          size_t oldsize, unsigned முழு flags) {
  எனில் (alignment <= SMALL_GRANULARITY)
    திரும்பு _rpmalloc_reallocate(heap, ptr, size, oldsize, flags);

  முழு no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? _rpmalloc_usable_size(ptr) : 0);
  எனில் ((usablesize >= size) && !((uintptr_t)ptr & (alignment - 1))) {
    எனில் (no_alloc || (size >= (usablesize / 2)))
      திரும்பு ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  வெறுமை *block =
      (!no_alloc ? _rpmalloc_aligned_allocate(heap, alignment, size) : 0);
  எனில் (EXPECTED(block != 0)) {
    எனில் (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      எனில் (!oldsize)
        oldsize = usablesize;
      memcpy(block, ptr, oldsize < size ? oldsize : size);
    }
    _rpmalloc_deallocate(ptr);
  }
  திரும்பு block;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
நிலையான size_t _rpmalloc_usable_size(வெறுமை *p) {
  // Grab the span using guaranteed span alignment
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  எனில் (span->size_class < SIZE_CLASS_மொத்தம்) {
    // Small/medium block
    வெறுமை *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    திரும்பு span->block_size -
           ((size_t)pointer_diff(p, blocks_start) % span->block_size);
  }
  எனில் (span->size_class == SIZE_CLASS_LARGE) {
    // Large block
    size_t current_spans = span->span_மொத்தம்;
    திரும்பு (current_spans * _memory_span_size) - (size_t)pointer_diff(p, span);
  }
  // Oversized block, page count is stored in span_count
  size_t current_pages = span->span_மொத்தம்;
  திரும்பு (current_pages * _memory_page_size) - (size_t)pointer_diff(p, span);
}

//! Adjust and optimize the size class properties for the given class
நிலையான வெறுமை _rpmalloc_adjust_size_class(size_t iclass) {
  size_t block_size = _memory_size_class[iclass].block_size;
  size_t block_மொத்தம் = (_memory_span_size - SPAN_HEADER_SIZE) / block_size;

  _memory_size_class[iclass].block_மொத்தம் = (uint16_t)block_மொத்தம்;
  _memory_size_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  எனில் (iclass >= SMALL_CLASS_மொத்தம்) {
    size_t prevclass = iclass;
    போது (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      எனில் (_memory_size_class[prevclass].block_மொத்தம் ==
          _memory_size_class[iclass].block_மொத்தம்)
        _rpmalloc_memcpy_const(_memory_size_class + prevclass,
                               _memory_size_class + iclass,
                               அளவு(_memory_size_class[iclass]));
      இல்லையெனில்
        நிறுத்து;
    }
  }
}

//! Initialize the allocator and setup global data
வெளி உள்வரி முழு rpmalloc_initialize(வெறுமை) {
  எனில் (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    திரும்பு 0;
  }
  திரும்பு rpmalloc_initialize_config(0);
}

முழு rpmalloc_initialize_config(மாறிலி rpmalloc_config_t *config) {
  எனில் (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    திரும்பு 0;
  }
  _rpmalloc_initialized = 1;

  எனில் (config)
    memcpy(&_memory_config, config, அளவு(rpmalloc_config_t));
  இல்லையெனில்
    _rpmalloc_memset_const(&_memory_config, 0, அளவு(rpmalloc_config_t));

  எனில் (!_memory_config.memory_map || !_memory_config.memory_unmap) {
    _memory_config.memory_map = _rpmalloc_mmap_os;
    _memory_config.memory_unmap = _rpmalloc_unmap_os;
  }

#எனில் PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, அளவு(system_info));
  GetSystemInfo(&system_info);
  _memory_map_granularity = system_info.dwAllocationGranularity;
#இல்லையெனில்
  _memory_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#endif

#எனில் RPMALLOC_CONFIGURABLE
  _memory_page_size = _memory_config.page_size;
#இல்லையெனில்
  _memory_page_size = 0;
#endif
  _memory_huge_pages = 0;
  எனில் (!_memory_page_size) {
#எனில் PLATFORM_WINDOWS
    _memory_page_size = system_info.dwPageSize;
#இல்லையெனில்
    _memory_page_size = _memory_map_granularity;
    எனில் (_memory_config.enable_huge_pages) {
#எனில் defined(__linux__)
      size_t huge_page_size = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      எனில் (meminfo) {
        எழுத்து வரி_2[128];
        போது (!huge_page_size && fgets(வரி_2, அளவு(வரி_2) - 1, meminfo)) {
          வரி_2[அளவு(வரி_2) - 1] = 0;
          எனில் (strstr(வரி_2, "Hugepagesize:"))
            huge_page_size = (size_t)strtol(வரி_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      எனில் (huge_page_size) {
        _memory_huge_pages = 1;
        _memory_page_size = huge_page_size;
        _memory_map_granularity = huge_page_size;
      }
#அல்லதெனில் defined(__FreeBSD__)
      முழு rc;
      size_t sz = அளவு(rc);

      எனில் (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, வெற்று, 0) == 0 &&
          rc == 1) {
        நிலையான size_t defsize = 2 * 1024 * 1024;
        முழு nsize = 0;
        size_t sizes[4] = {0};
        _memory_huge_pages = 1;
        _memory_page_size = defsize;
        எனில் ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          க்கு (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            எனில் (defsize < csize) {
              _memory_page_size = csize;
              நிறுத்து;
            }
          }
        }
        _memory_map_granularity = _memory_page_size;
      }
#அல்லதெனில் defined(__APPLE__) || defined(__NetBSD__)
      _memory_huge_pages = 1;
      _memory_page_size = 2 * 1024 * 1024;
      _memory_map_granularity = _memory_page_size;
#endif
    }
#endif
  } இல்லையெனில் {
    எனில் (_memory_config.enable_huge_pages)
      _memory_huge_pages = 1;
  }

#எனில் PLATFORM_WINDOWS
  எனில் (_memory_config.enable_huge_pages) {
    HANDLE token = 0;
    size_t large_page_minimum = GetLargePageMinimum();
    எனில் (large_page_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    எனில் (token) {
      LUID luid;
      எனில் (Lookup_Privilege_மதிப்பு(0, SE_LOCK_MEMORY_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, அளவு(token_privileges));
        token_privileges.Privilege_மொத்தம் = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        எனில் (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          எனில் (GetLastError() == ERROR_SUCCESS)
            _memory_huge_pages = 1;
        }
      }
      CloseHandle(token);
    }
    எனில் (_memory_huge_pages) {
      எனில் (large_page_minimum > _memory_page_size)
        _memory_page_size = large_page_minimum;
      எனில் (large_page_minimum > _memory_map_granularity)
        _memory_map_granularity = large_page_minimum;
    }
  }
#endif

  size_t min_span_size = 256;
  size_t max_page_size;
#எனில் UINTPTR_MAX > 0xFFFFFFFF
  max_page_size = 4096ULL * 1024ULL * 1024ULL;
#இல்லையெனில்
  max_page_size = 4 * 1024 * 1024;
#endif
  எனில் (_memory_page_size < min_span_size)
    _memory_page_size = min_span_size;
  எனில் (_memory_page_size > max_page_size)
    _memory_page_size = max_page_size;
  _memory_page_size_shift = 0;
  size_t page_size_bit = _memory_page_size;
  போது (page_size_bit != 1) {
    ++_memory_page_size_shift;
    page_size_bit >>= 1;
  }
  _memory_page_size = ((size_t)1 << _memory_page_size_shift);

#எனில் RPMALLOC_CONFIGURABLE
  எனில் (!_memory_config.span_size) {
    _memory_span_size = _memory_default_span_size;
    _memory_span_size_shift = _memory_default_span_size_shift;
    _memory_span_mask = _memory_default_span_mask;
  } இல்லையெனில் {
    size_t span_size = _memory_config.span_size;
    எனில் (span_size > (256 * 1024))
      span_size = (256 * 1024);
    _memory_span_size = 4096;
    _memory_span_size_shift = 12;
    போது (_memory_span_size < span_size) {
      _memory_span_size <<= 1;
      ++_memory_span_size_shift;
    }
    _memory_span_mask = ~(uintptr_t)(_memory_span_size - 1);
  }
#endif

  memory_span_map_மொத்தம் =
      (_memory_config.span_map_மொத்தம் ? _memory_config.span_map_மொத்தம்
                                     : DEFAULT_SPAN_MAP_மொத்தம்);
  எனில் ((_memory_span_size * memory_span_map_மொத்தம்) < _memory_page_size)
    memory_span_map_மொத்தம் = (_memory_page_size / _memory_span_size);
  எனில் ((_memory_page_size >= _memory_span_size) &&
      ((memory_span_map_மொத்தம் * _memory_span_size) % _memory_page_size))
    memory_span_map_மொத்தம் = (_memory_page_size / _memory_span_size);
  memory_heap_reserve_மொத்தம் = (memory_span_map_மொத்தம் > DEFAULT_SPAN_MAP_மொத்தம்)
                                   ? DEFAULT_SPAN_MAP_மொத்தம்
                                   : memory_span_map_மொத்தம்;

  _memory_config.page_size = _memory_page_size;
  _memory_config.span_size = _memory_span_size;
  _memory_config.span_map_மொத்தம் = memory_span_map_மொத்தம்;
  _memory_config.enable_huge_pages = _memory_huge_pages;

#எனில் ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  எனில் (pthread_சாவி_create(&_memory_thread_heap, _rpmalloc_heap_release_raw_fc))
    திரும்பு -1;
#endif
#எனில் defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_சாவி = FlsAlloc(&_rpmalloc_thread_destructor);
#endif

  // Setup all small and medium size classes
  size_t iclass = 0;
  _memory_size_class[iclass].block_size = SMALL_GRANULARITY;
  _rpmalloc_adjust_size_class(iclass);
  க்கு (iclass = 1; iclass < SMALL_CLASS_மொத்தம்; ++iclass) {
    size_t size = iclass * SMALL_GRANULARITY;
    _memory_size_class[iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  _memory_medium_size_limit = (_memory_span_size - SPAN_HEADER_SIZE) >> 1;
  எனில் (_memory_medium_size_limit > MEDIUM_SIZE_LIMIT)
    _memory_medium_size_limit = MEDIUM_SIZE_LIMIT;
  க்கு (iclass = 0; iclass < MEDIUM_CLASS_மொத்தம்; ++iclass) {
    size_t size = SMALL_SIZE_LIMIT + ((iclass + 1) * MEDIUM_GRANULARITY);
    எனில் (size > _memory_medium_size_limit) {
      _memory_medium_size_limit =
          SMALL_SIZE_LIMIT + (iclass * MEDIUM_GRANULARITY);
      நிறுத்து;
    }
    _memory_size_class[SMALL_CLASS_மொத்தம் + iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(SMALL_CLASS_மொத்தம் + iclass);
  }

  _memory_orphan_heaps = 0;
#எனில் RPMALLOC_FIRST_CLASS_HEAPS
  _memory_first_class_orphan_heaps = 0;
#endif
#எனில் ENABLE_STATISTICS
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
  memset(_memory_heaps, 0, அளவு(_memory_heaps));
  atomic_store32_release(&_memory_global_lock, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  திரும்பு 0;
}

//! Finalize the allocator
வெறுமை rpmalloc_finalize(வெறுமை) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  எனில் (_memory_global_reserve) {
    atomic_add32(&_memory_global_reserve_master->remaining_spans,
                 -(int32_t)memory_global_reserve_மொத்தம்);
    _memory_global_reserve_master = 0;
    memory_global_reserve_மொத்தம் = 0;
    _memory_global_reserve = 0;
  }
  atomic_store32_release(&_memory_global_lock, 0);

  // Free all thread caches and fully free spans
  க்கு (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    போது (heap) {
      heap_t *next_heap = heap->next_heap;
      heap->finalize = 1;
      _rpmalloc_heap_global_finalize(heap);
      heap = next_heap;
    }
  }

#எனில் ENABLE_GLOBAL_CACHE
  // Free global caches
  க்கு (size_t iclass = 0; iclass < LARGE_CLASS_மொத்தம்; ++iclass)
    _rpmalloc_global_cache_finalize(&_memory_span_cache[iclass]);
#endif

#எனில் (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_சாவி_delete(_memory_thread_heap);
#endif
#எனில் defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_சாவி);
  fls_சாவி = 0;
#endif
#எனில் ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&_mapped_pages) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&_mapped_pages_os) == 0,
                  "Memory leak detected");
#endif

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
வெளி உள்வரி வெறுமை rpmalloc_thread_initialize(வெறுமை) {
  எனில் (!get_thread_heap_raw()) {
    heap_t *heap = _rpmalloc_heap_allocate(0);
    எனில் (heap) {
      _rpmalloc_stat_inc(&_memory_active_heaps);
      set_thread_heap(heap);
#எனில் defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_Set_மதிப்பு(fls_சாவி, heap);
#endif
    }
  }
}

//! Finalize thread, orphan heap
வெறுமை rpmalloc_thread_finalize(முழு release_caches) {
  heap_t *heap = get_thread_heap_raw();
  எனில் (heap)
    _rpmalloc_heap_release_raw(heap, release_caches);
  set_thread_heap(0);
#எனில் defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_Set_மதிப்பு(fls_சாவி, 0);
#endif
}

முழு rpmalloc_is_thread_initialized(வெறுமை) {
  திரும்பு (get_thread_heap_raw() != 0) ? 1 : 0;
}

மாறிலி rpmalloc_config_t *rpmalloc_config(வெறுமை) { திரும்பு &_memory_config; }

// Extern interface

வெளி உள்வரி RPMALLOC_ALLOCATOR வெறுமை *rpmalloc(size_t size) {
#எனில் ENABLE_VALIDATE_ARGS
  எனில் (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    திரும்பு 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  திரும்பு _rpmalloc_allocate(heap, size);
}

வெளி உள்வரி வெறுமை rpfree(வெறுமை *ptr) { _rpmalloc_deallocate(ptr); }

வெளி உள்வரி RPMALLOC_ALLOCATOR வெறுமை *rpcalloc(size_t இலக்கம்_2, size_t size) {
  size_t total;
#எனில் ENABLE_VALIDATE_ARGS
#எனில் PLATFORM_WINDOWS
  முழு err = SizeTMult(இலக்கம்_2, size, &total);
  எனில் ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    திரும்பு 0;
  }
#இல்லையெனில்
  முழு err = __builtin_umull_overflow(இலக்கம்_2, size, &total);
  எனில் (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    திரும்பு 0;
  }
#endif
#இல்லையெனில்
  total = இலக்கம்_2 * size;
#endif
  heap_t *heap = get_thread_heap();
  வெறுமை *block = _rpmalloc_allocate(heap, total);
  எனில் (block)
    memset(block, 0, total);
  திரும்பு block;
}

வெளி உள்வரி RPMALLOC_ALLOCATOR வெறுமை *rprealloc(வெறுமை *ptr, size_t size) {
#எனில் ENABLE_VALIDATE_ARGS
  எனில் (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    திரும்பு ptr;
  }
#endif
  heap_t *heap = get_thread_heap();
  திரும்பு _rpmalloc_reallocate(heap, ptr, size, 0, 0);
}

வெளி RPMALLOC_ALLOCATOR வெறுமை *rpaligned_realloc(வெறுமை *ptr, size_t alignment,
                                                  size_t size, size_t oldsize,
                                                  unsigned முழு flags) {
#எனில் ENABLE_VALIDATE_ARGS
  எனில் ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    திரும்பு 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  திரும்பு _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, oldsize,
                                      flags);
}

வெளி RPMALLOC_ALLOCATOR வெறுமை *rpaligned_alloc(size_t alignment, size_t size) {
  heap_t *heap = get_thread_heap();
  திரும்பு _rpmalloc_aligned_allocate(heap, alignment, size);
}

வெளி உள்வரி RPMALLOC_ALLOCATOR வெறுமை *
rpaligned_calloc(size_t alignment, size_t இலக்கம்_2, size_t size) {
  size_t total;
#எனில் ENABLE_VALIDATE_ARGS
#எனில் PLATFORM_WINDOWS
  முழு err = SizeTMult(இலக்கம்_2, size, &total);
  எனில் ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    திரும்பு 0;
  }
#இல்லையெனில்
  முழு err = __builtin_umull_overflow(இலக்கம்_2, size, &total);
  எனில் (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    திரும்பு 0;
  }
#endif
#இல்லையெனில்
  total = இலக்கம்_2 * size;
#endif
  வெறுமை *block = rpaligned_alloc(alignment, total);
  எனில் (block)
    memset(block, 0, total);
  திரும்பு block;
}

வெளி உள்வரி RPMALLOC_ALLOCATOR வெறுமை *rpmemalign(size_t alignment,
                                                  size_t size) {
  திரும்பு rpaligned_alloc(alignment, size);
}

வெளி உள்வரி முழு rpposix_memalign(வெறுமை **memptr, size_t alignment,
                                   size_t size) {
  எனில் (memptr)
    *memptr = rpaligned_alloc(alignment, size);
  இல்லையெனில்
    திரும்பு EINVAL;
  திரும்பு *memptr ? 0 : ENOMEM;
}

வெளி உள்வரி size_t rpmalloc_usable_size(வெறுமை *ptr) {
  திரும்பு (ptr ? _rpmalloc_usable_size(ptr) : 0);
}

வெளி உள்வரி வெறுமை rpmalloc_thread_collect(வெறுமை) {}

வெறுமை rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, அளவு(rpmalloc_thread_statistics_t));
  heap_t *heap = get_thread_heap_raw();
  எனில் (!heap)
    திரும்பு;

  க்கு (size_t iclass = 0; iclass < SIZE_CLASS_மொத்தம்; ++iclass) {
    size_class_t *size_class = _memory_size_class + iclass;
    span_t *span = heap->size_class[iclass].partial_span;
    போது (span) {
      size_t free_மொத்தம் = span->list_size;
      size_t block_மொத்தம் = size_class->block_மொத்தம்;
      எனில் (span->free_list_limit < block_மொத்தம்)
        block_மொத்தம் = span->free_list_limit;
      free_மொத்தம் += (block_மொத்தம் - span->used_மொத்தம்);
      stats->sizecache += free_மொத்தம் * size_class->block_size;
      span = span->next;
    }
  }

#எனில் ENABLE_THREAD_CACHE
  க்கு (size_t iclass = 0; iclass < LARGE_CLASS_மொத்தம்; ++iclass) {
    span_cache_t *span_cache;
    எனில் (!iclass)
      span_cache = &heap->span_cache;
    இல்லையெனில்
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    stats->spancache += span_cache->மொத்தம் * (iclass + 1) * _memory_span_size;
  }
#endif

  span_t *deferred = (span_t *)atomic_load_ptr(&heap->span_free_deferred);
  போது (deferred) {
    எனில் (deferred->size_class != SIZE_CLASS_HUGE)
      stats->spancache += (size_t)deferred->span_மொத்தம் * _memory_span_size;
    deferred = (span_t *)deferred->free_list;
  }

#எனில் ENABLE_STATISTICS
  stats->thread_to_global = (size_t)atomic_load64(&heap->thread_to_global);
  stats->global_to_thread = (size_t)atomic_load64(&heap->global_to_thread);

  க்கு (size_t iclass = 0; iclass < LARGE_CLASS_மொத்தம்; ++iclass) {
    stats->span_use[iclass].current =
        (size_t)atomic_load32(&heap->span_use[iclass].current);
    stats->span_use[iclass].peak =
        (size_t)atomic_load32(&heap->span_use[iclass].உயர்வு);
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
  க்கு (size_t iclass = 0; iclass < SIZE_CLASS_மொத்தம்; ++iclass) {
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

வெறுமை rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, அளவு(rpmalloc_global_statistics_t));
#எனில் ENABLE_STATISTICS
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
#எனில் ENABLE_GLOBAL_CACHE
  க்கு (size_t iclass = 0; iclass < LARGE_CLASS_மொத்தம்; ++iclass) {
    global_cache_t *cache = &_memory_span_cache[iclass];
    போது (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();
    uint32_t மொத்தம் = cache->மொத்தம்;
#எனில் ENABLE_UNLIMITED_CACHE
    span_t *current_span = cache->overflow;
    போது (current_span) {
      ++மொத்தம்;
      current_span = current_span->next;
    }
#endif
    atomic_store32_release(&cache->lock, 0);
    stats->cached += மொத்தம் * (iclass + 1) * _memory_span_size;
  }
#endif
}

#எனில் ENABLE_STATISTICS

நிலையான வெறுமை _memory_heap_dump_statistics(heap_t *heap, வெறுமை *file) {
  fprintf(file, "Heap %d stats:\n", heap->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  க்கு (size_t iclass = 0; iclass < SIZE_CLASS_மொத்தம்; ++iclass) {
    எனில் (!atomic_load32(&heap->size_class_use[iclass].alloc_total))
      தொடர்;
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
        _memory_size_class[iclass].block_மொத்தம்,
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
  க்கு (size_t iclass = 0; iclass < LARGE_CLASS_மொத்தம்; ++iclass) {
    எனில் (!atomic_load32(&heap->span_use[iclass].உயர்வு) &&
        !atomic_load32(&heap->span_use[iclass].spans_map_calls))
      தொடர்;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&heap->span_use[iclass].current),
        atomic_load32(&heap->span_use[iclass].உயர்வு),
        atomic_load32(&heap->span_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&heap->span_use[iclass].உயர்வு) *
         (size_t)_memory_span_size * (iclass + 1)) /
            (size_t)(1024 * 1024),
#எனில் ENABLE_THREAD_CACHE
        (unsigned முழு)(!iclass ? heap->span_cache.மொத்தம்
                               : heap->span_large_cache[iclass - 1].மொத்தம்),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_to_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_from_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
#இல்லையெனில்
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
  fprintf(file, "Full spans: %zu\n", heap->full_span_மொத்தம்);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&heap->thread_to_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&heap->global_to_thread) / (size_t)(1024 * 1024));
}

#endif

வெறுமை rpmalloc_dump_statistics(வெறுமை *file) {
#எனில் ENABLE_STATISTICS
  க்கு (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    போது (heap) {
      முழு need_dump = 0;
      க்கு (size_t iclass = 0; !need_dump && (iclass < SIZE_CLASS_மொத்தம்);
           ++iclass) {
        எனில் (!atomic_load32(&heap->size_class_use[iclass].alloc_total)) {
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].free_total),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          தொடர்;
        }
        need_dump = 1;
      }
      க்கு (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_மொத்தம்);
           ++iclass) {
        எனில் (!atomic_load32(&heap->span_use[iclass].உயர்வு) &&
            !atomic_load32(&heap->span_use[iclass].spans_map_calls))
          தொடர்;
        need_dump = 1;
      }
      எனில் (need_dump)
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

#எனில் ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  க்கு (size_t iclass = 0; iclass < LARGE_CLASS_மொத்தம்; ++iclass) {
    global_cache_t *cache = _memory_span_cache + iclass;
    size_t global_cache = (size_t)cache->மொத்தம் * iclass * _memory_span_size;

    size_t global_overflow_cache = 0;
    span_t *span = cache->overflow;
    போது (span) {
      global_overflow_cache += iclass * _memory_span_size;
      span = span->next;
    }
    எனில் (global_cache || global_overflow_cache || cache->செருகு_மொத்தம் ||
        cache->extract_மொத்தம்)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_overflow_cache / (size_t)(1024 * 1024),
              cache->செருகு_மொத்தம், cache->extract_மொத்தம்);
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
#எனில் 0
	int64_t allocated = atomic_load64(&_allocation_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#endif
#endif
  (வெறுமை)அளவு(file);
}

#எனில் RPMALLOC_FIRST_CLASS_HEAPS

வெளி உள்வரி rpmalloc_heap_t *rpmalloc_heap_acquire(வெறுமை) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  heap_t *heap = _rpmalloc_heap_allocate(1);
  rpmalloc_assume(heap != வெற்று);
  heap->owner_thread = 0;
  _rpmalloc_stat_inc(&_memory_active_heaps);
  திரும்பு heap;
}

வெளி உள்வரி வெறுமை rpmalloc_heap_release(rpmalloc_heap_t *heap) {
  எனில் (heap)
    _rpmalloc_heap_release(heap, 1, 1);
}

வெளி உள்வரி RPMALLOC_ALLOCATOR வெறுமை *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, size_t size) {
#எனில் ENABLE_VALIDATE_ARGS
  எனில் (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    திரும்பு 0;
  }
#endif
  திரும்பு _rpmalloc_allocate(heap, size);
}

வெளி உள்வரி RPMALLOC_ALLOCATOR வெறுமை *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, size_t alignment,
                            size_t size) {
#எனில் ENABLE_VALIDATE_ARGS
  எனில் (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    திரும்பு 0;
  }
#endif
  திரும்பு _rpmalloc_aligned_allocate(heap, alignment, size);
}

வெளி உள்வரி RPMALLOC_ALLOCATOR வெறுமை *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, size_t இலக்கம்_2, size_t size) {
  திரும்பு rpmalloc_heap_aligned_calloc(heap, 0, இலக்கம்_2, size);
}

வெளி உள்வரி RPMALLOC_ALLOCATOR வெறுமை *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, size_t alignment,
                             size_t இலக்கம்_2, size_t size) {
  size_t total;
#எனில் ENABLE_VALIDATE_ARGS
#எனில் PLATFORM_WINDOWS
  முழு err = SizeTMult(இலக்கம்_2, size, &total);
  எனில் ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    திரும்பு 0;
  }
#இல்லையெனில்
  முழு err = __builtin_umull_overflow(இலக்கம்_2, size, &total);
  எனில் (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    திரும்பு 0;
  }
#endif
#இல்லையெனில்
  total = இலக்கம்_2 * size;
#endif
  வெறுமை *block = _rpmalloc_aligned_allocate(heap, alignment, total);
  எனில் (block)
    memset(block, 0, total);
  திரும்பு block;
}

வெளி உள்வரி RPMALLOC_ALLOCATOR வெறுமை *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, வெறுமை *ptr, size_t size,
                      unsigned முழு flags) {
#எனில் ENABLE_VALIDATE_ARGS
  எனில் (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    திரும்பு ptr;
  }
#endif
  திரும்பு _rpmalloc_reallocate(heap, ptr, size, 0, flags);
}

வெளி உள்வரி RPMALLOC_ALLOCATOR வெறுமை *
rpmalloc_heap_aligned_realloc(rpmalloc_heap_t *heap, வெறுமை *ptr,
                              size_t alignment, size_t size,
                              unsigned முழு flags) {
#எனில் ENABLE_VALIDATE_ARGS
  எனில் ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    திரும்பு 0;
  }
#endif
  திரும்பு _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, 0, flags);
}

வெளி உள்வரி வெறுமை rpmalloc_heap_free(rpmalloc_heap_t *heap, வெறுமை *ptr) {
  (வெறுமை)அளவு(heap);
  _rpmalloc_deallocate(ptr);
}

வெளி உள்வரி வெறுமை rpmalloc_heap_free_all(rpmalloc_heap_t *heap) {
  span_t *span;
  span_t *next_span;

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  க்கு (size_t iclass = 0; iclass < SIZE_CLASS_மொத்தம்; ++iclass) {
    span = heap->size_class[iclass].partial_span;
    போது (span) {
      next_span = span->next;
      rpmalloc_heap_cache_செருகு(heap, span);
      span = next_span;
    }
    heap->size_class[iclass].partial_span = 0;
    span = heap->full_span[iclass];
    போது (span) {
      next_span = span->next;
      rpmalloc_heap_cache_செருகு(heap, span);
      span = next_span;
    }

    span = heap->size_class[iclass].cache;
    எனில் (span)
      rpmalloc_heap_cache_செருகு(heap, span);
    heap->size_class[iclass].cache = 0;
  }
  memset(heap->size_class, 0, அளவு(heap->size_class));
  memset(heap->full_span, 0, அளவு(heap->full_span));

  span = heap->large_huge_span;
  போது (span) {
    next_span = span->next;
    எனில் (UNEXPECTED(span->size_class == SIZE_CLASS_HUGE))
      _rpmalloc_deallocate_huge(span);
    இல்லையெனில்
      rpmalloc_heap_cache_செருகு(heap, span);
    span = next_span;
  }
  heap->large_huge_span = 0;
  heap->full_span_மொத்தம் = 0;

#எனில் ENABLE_THREAD_CACHE
  க்கு (size_t iclass = 0; iclass < LARGE_CLASS_மொத்தம்; ++iclass) {
    span_cache_t *span_cache;
    எனில் (!iclass)
      span_cache = &heap->span_cache;
    இல்லையெனில்
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    எனில் (!span_cache->மொத்தம்)
      தொடர்;
#எனில் ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&heap->thread_to_global,
                         span_cache->மொத்தம் * (iclass + 1) * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                       span_cache->மொத்தம்);
    rpmalloc_global_cache_செருகு_spans(span_cache->span, iclass + 1,
                                        span_cache->மொத்தம்);
#இல்லையெனில்
    க்கு (size_t ispan = 0; ispan < span_cache->மொத்தம்; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
    span_cache->மொத்தம் = 0;
  }
#endif

#எனில் ENABLE_STATISTICS
  க்கு (size_t iclass = 0; iclass < SIZE_CLASS_மொத்தம்; ++iclass) {
    atomic_store32(&heap->size_class_use[iclass].alloc_current, 0);
    atomic_store32(&heap->size_class_use[iclass].spans_current, 0);
  }
  க்கு (size_t iclass = 0; iclass < LARGE_CLASS_மொத்தம்; ++iclass) {
    atomic_store32(&heap->span_use[iclass].current, 0);
  }
#endif
}

வெளி உள்வரி வெறுமை rpmalloc_heap_thread_set_current(rpmalloc_heap_t *heap) {
  heap_t *prev_heap = get_thread_heap_raw();
  எனில் (prev_heap != heap) {
    set_thread_heap(heap);
    எனில் (prev_heap)
      rpmalloc_heap_release(prev_heap);
  }
}

வெளி உள்வரி rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(வெறுமை *ptr) {
  // Grab the span, and then the heap from the span
  span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
  எனில் (span) {
    திரும்பு span->heap;
  }
  திரும்பு 0;
}

#endif

#எனில் ENABLE_PRELOAD || ENABLE_OVERRIDE

#அந்தர்பாவ "malloc.c"

#endif

வெறுமை rpmalloc_linker_reference(வெறுமை) { (வெறுமை)அளவு(_rpmalloc_initialized); }
