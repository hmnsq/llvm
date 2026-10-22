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

#अन्तर्भाव "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#जइ defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wunused-function"
#जइ __has_warning("-Wreserved-identifier")
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
#जइ __has_warning("-Wstatic-in-inline")
#pragma clang diagnostic ignored "-Wstatic-in-inline"
#endif
#नोचेद्यदि defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#जइ !defined(__has_builtin)
#define __has_builtin(b) 0
#endif

#जइ defined(__GNUC__) || defined(__clang__)

#जइ __has_builtin(__builtin_memcpy_inline)
#define _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#अण्णहा
#define _rpmalloc_memcpy_const(x, y, s)                                        \
  करेहि {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memcpy(x, y, s);                                                           \
  } जाव (0)
#endif

#जइ __has_builtin(__builtin_memset_inline)
#define _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#अण्णहा
#define _rpmalloc_memset_const(x, y, s)                                        \
  करेहि {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } जाव (0)
#endif
#अण्णहा
#define _rpmalloc_memcpy_const(x, y, s) memcpy(x, y, s)
#define _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#endif

#जइ __has_builtin(__builtin_assume)
#define rpmalloc_assume(cond) __builtin_assume(cond)
#नोचेद्यदि defined(__GNUC__)
#define rpmalloc_assume(cond)                                                  \
  करेहि {                                                                         \
    जइ (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } जाव (0)
#नोचेद्यदि defined(_MSC_VER)
#define rpmalloc_assume(cond) __assume(cond)
#अण्णहा
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
#ifndef DEFAULT_SPAN_MAP_गणनम्
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#define DEFAULT_SPAN_MAP_गणनम् 64
#endif
#ifndef GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#define GLOBAL_CACHE_MULTIPLIER 8
#endif

#जइ DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#error Must use global cache जइ unmap is disabled
#endif

#जइ DISABLE_UNMAP
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 1
#endif

#जइ !ENABLE_GLOBAL_CACHE
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 0
#endif

#जइ !ENABLE_THREAD_CACHE
#undef ENABLE_ADAPTIVE_THREAD_CACHE
#define ENABLE_ADAPTIVE_THREAD_CACHE 0
#endif

#जइ defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#define PLATFORM_POSIX 0
#अण्णहा
#define PLATFORM_WINDOWS 0
#define PLATFORM_POSIX 1
#endif

/// Platform and arch specifics
#जइ defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 5105)
#ifndef FORCEINLINE
#define FORCEINLINE अन्तःस्थ __forceinline
#endif
#define _Static_assert static_assert
#अण्णहा
#ifndef FORCEINLINE
#define FORCEINLINE अन्तःस्थ __attribute__((__always_inline__))
#endif
#endif
#जइ PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#अन्तर्भाव <windows.h>
#जइ ENABLE_VALIDATE_ARGS
#अन्तर्भाव <intsafe.h>
#endif
#अण्णहा
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>
#जइ defined(__linux__) || defined(__ANDROID__)
#अन्तर्भाव <sys/prctl.h>
#जइ !defined(PR_SET_VMA)
#define PR_SET_VMA 0x53564d41
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif
#जइ defined(__APPLE__)
#अन्तर्भाव <TargetConditionals.h>
#जइ !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#अन्तर्भाव <mach/mach_vm.h>
#अन्तर्भाव <mach/vm_statistics.h>
#endif
#अन्तर्भाव <pthread.h>
#endif
#जइ defined(__HAIKU__) || defined(__TINYC__)
#अन्तर्भाव <pthread.h>
#endif
#endif

#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <string.h>

#जइ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#अन्तर्भाव <fibersapi.h>
स्थिर DWORD fls_कुञ्चिका;
#endif

#जइ PLATFORM_POSIX
#अन्तर्भाव <sched.h>
#अन्तर्भाव <sys/mman.h>
#ifdef __FreeBSD__
#अन्तर्भाव <sys/sysctl.h>
#define MAP_HUGETLB MAP_ALIGNED_SUPER
#ifndef PROT_MAX
#define PROT_MAX(f) 0
#endif
#अण्णहा
#define PROT_MAX(f) 0
#endif
#ifdef __sun
बाह्य पूर्णाङ्क madvise(caddr_t, size_t, पूर्णाङ्क);
#endif
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif
#endif
#अन्तर्भाव <errno.h>

#जइ ENABLE_ASSERTS
#undef NDEBUG
#जइ defined(_MSC_VER) && !defined(_DEBUG)
#define _DEBUG
#endif
#अन्तर्भाव <assert.h>
#define RPMALLOC_TOSTRING_M(x) #x
#define RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#define rpmalloc_assert(truth, message)                                        \
  करेहि {                                                                         \
    जइ (!(truth)) {                                                            \
      जइ (_memory_config.error_callback) {                                     \
        _memory_config.error_callback(message " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } अण्णहा {                                                                 \
        प्रतिज्ञा((truth) && message);                                            \
      }                                                                        \
    }                                                                          \
  } जाव (0)
#अण्णहा
#define rpmalloc_assert(truth, message)                                        \
  करेहि {                                                                         \
  } जाव (0)
#endif
#जइ ENABLE_STATISTICS
#अन्तर्भाव <stdio.h>
#endif

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#जइ defined(_MSC_VER) && !defined(__clang__)

प्रकारनिर्देश चञ्चल long atomic32_t;
प्रकारनिर्देश चञ्चल long long atomic64_t;
प्रकारनिर्देश चञ्चल शून्य *atomicptr_t;

स्थिर FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  प्रत्यागम (int32_t)InterlockedOr(src, 0);
}
स्थिर FORCEINLINE शून्य atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
स्थिर FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  प्रत्यागम (int32_t)InterlockedIncrement(val);
}
स्थिर FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  प्रत्यागम (int32_t)InterlockedDecrement(val);
}
स्थिर FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  प्रत्यागम (int32_t)InterlockedExchangeAdd(val, add) + add;
}
स्थिर FORCEINLINE पूर्णाङ्क atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  प्रत्यागम (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
स्थिर FORCEINLINE शून्य atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
स्थिर FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  प्रत्यागम (int64_t)InterlockedOr64(src, 0);
}
स्थिर FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  प्रत्यागम (int64_t)InterlockedExchangeAdd64(val, add) + add;
}
स्थिर FORCEINLINE शून्य *atomic_load_ptr(atomicptr_t *src) {
  प्रत्यागम InterlockedCompareExchangePointer(src, 0, 0);
}
स्थिर FORCEINLINE शून्य atomic_store_ptr(atomicptr_t *dst, शून्य *val) {
  InterlockedExchangePointer(dst, val);
}
स्थिर FORCEINLINE शून्य atomic_store_ptr_release(atomicptr_t *dst, शून्य *val) {
  InterlockedExchangePointer(dst, val);
}
स्थिर FORCEINLINE शून्य *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     शून्य *val) {
  प्रत्यागम (शून्य *)InterlockedExchangePointer((शून्य *चञ्चल *)dst, val);
}
स्थिर FORCEINLINE पूर्णाङ्क atomic_cas_ptr(atomicptr_t *dst, शून्य *val, शून्य *ref) {
  प्रत्यागम (InterlockedCompareExchangePointer((शून्य *चञ्चल *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#define EXPECTED(x) (x)
#define UNEXPECTED(x) (x)

#अण्णहा

#अन्तर्भाव <stdatomic.h>

प्रकारनिर्देश चञ्चल _Atomic(int32_t) atomic32_t;
प्रकारनिर्देश चञ्चल _Atomic(int64_t) atomic64_t;
प्रकारनिर्देश चञ्चल _Atomic(शून्य *) atomicptr_t;

स्थिर FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  प्रत्यागम atomic_load_explicit(src, memory_order_relaxed);
}
स्थिर FORCEINLINE शून्य atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
स्थिर FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  प्रत्यागम atomic_fetch_add_explicit(val, 1, memory_order_relaxed) + 1;
}
स्थिर FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  प्रत्यागम atomic_fetch_add_explicit(val, -1, memory_order_relaxed) - 1;
}
स्थिर FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  प्रत्यागम atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
स्थिर FORCEINLINE पूर्णाङ्क atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  प्रत्यागम atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
स्थिर FORCEINLINE शून्य atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
स्थिर FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  प्रत्यागम atomic_load_explicit(val, memory_order_relaxed);
}
स्थिर FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  प्रत्यागम atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
स्थिर FORCEINLINE शून्य *atomic_load_ptr(atomicptr_t *src) {
  प्रत्यागम atomic_load_explicit(src, memory_order_relaxed);
}
स्थिर FORCEINLINE शून्य atomic_store_ptr(atomicptr_t *dst, शून्य *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
स्थिर FORCEINLINE शून्य atomic_store_ptr_release(atomicptr_t *dst, शून्य *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
स्थिर FORCEINLINE शून्य *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     शून्य *val) {
  प्रत्यागम atomic_exchange_explicit(dst, val, memory_order_acquire);
}
स्थिर FORCEINLINE पूर्णाङ्क atomic_cas_ptr(atomicptr_t *dst, शून्य *val, शून्य *ref) {
  प्रत्यागम atomic_compare_exchange_weak_explicit(
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

#जइ ENABLE_STATISTICS
#define _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#define _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#define _rpmalloc_stat_add(counter, मानम्_2)                                     \
  atomic_add32(counter, (int32_t)(मानम्_2))
#define _rpmalloc_stat_add64(counter, मानम्_2)                                   \
  atomic_add64(counter, (int64_t)(मानम्_2))
#define _rpmalloc_stat_add_peak(counter, मानम्_2, peak)                          \
  करेहि {                                                                         \
    int32_t cur_गणनम् = atomic_add32(counter, (int32_t)(मानम्_2));              \
    जइ (cur_गणनम् > (peak))                                                   \
      peak = cur_गणनम्;                                                       \
  } जाव (0)
#define _rpmalloc_stat_sub(counter, मानम्_2)                                     \
  atomic_add32(counter, -(int32_t)(मानम्_2))
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  करेहि {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&heap->size_class_use[class_idx].alloc_current);         \
    जइ (alloc_current > heap->size_class_use[class_idx].alloc_peak)            \
      heap->size_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&heap->size_class_use[class_idx].alloc_total);               \
  } जाव (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  करेहि {                                                                         \
    atomic_decr32(&heap->size_class_use[class_idx].alloc_current);             \
    atomic_incr32(&heap->size_class_use[class_idx].free_total);                \
  } जाव (0)
#अण्णहा
#define _rpmalloc_stat_inc(counter)                                            \
  करेहि {                                                                         \
  } जाव (0)
#define _rpmalloc_stat_dec(counter)                                            \
  करेहि {                                                                         \
  } जाव (0)
#define _rpmalloc_stat_add(counter, मानम्_2)                                     \
  करेहि {                                                                         \
  } जाव (0)
#define _rpmalloc_stat_add64(counter, मानम्_2)                                   \
  करेहि {                                                                         \
  } जाव (0)
#define _rpmalloc_stat_add_peak(counter, मानम्_2, peak)                          \
  करेहि {                                                                         \
  } जाव (0)
#define _rpmalloc_stat_sub(counter, मानम्_2)                                     \
  करेहि {                                                                         \
  } जाव (0)
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  करेहि {                                                                         \
  } जाव (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  करेहि {                                                                         \
  } जाव (0)
#endif

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#define SMALL_GRANULARITY 16
//! Small granularity shift count
#define SMALL_GRANULARITY_SHIFT 4
//! Number of small block size classes
#define SMALL_CLASS_गणनम् 65
//! Maximum size of a small block
#define SMALL_SIZE_LIMIT (SMALL_GRANULARITY * (SMALL_CLASS_गणनम् - 1))
//! Granularity of a medium allocation block
#define MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#define MEDIUM_GRANULARITY_SHIFT 9
//! Number of medium block size classes
#define MEDIUM_CLASS_गणनम् 61
//! Total number of small + medium size classes
#define SIZE_CLASS_गणनम् (SMALL_CLASS_गणनम् + MEDIUM_CLASS_गणनम्)
//! Number of large block size classes
#define LARGE_CLASS_गणनम् 63
//! Maximum size of a medium block
#define MEDIUM_SIZE_LIMIT                                                      \
  (SMALL_SIZE_LIMIT + (MEDIUM_GRANULARITY * MEDIUM_CLASS_गणनम्))
//! Maximum size of a large block
#define LARGE_SIZE_LIMIT                                                       \
  ((LARGE_CLASS_गणनम् * _memory_span_size) - SPAN_HEADER_SIZE)
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

#जइ ENABLE_VALIDATE_ARGS
//! Maximum allocation size to avoid integer overflow
#undef MAX_ALLOC_SIZE
#define MAX_ALLOC_SIZE (((size_t) - 1) - _memory_span_size)
#endif

#define pointer_offset(ptr, ofs) (शून्य *)((वर्ण *)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff(first, second)                                            \
  (ptrdiff_t)((ध्रुव वर्ण *)(first) - (ध्रुव वर्ण *)(second))

#define INVALID_POINTER ((शून्य *)((uintptr_t) - 1))

#define SIZE_CLASS_LARGE SIZE_CLASS_गणनम्
#define SIZE_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
प्रकारनिर्देश संरचना heap_t heap_t;
//! Span of memory pages
प्रकारनिर्देश संरचना span_t span_t;
//! Span list
प्रकारनिर्देश संरचना span_list_t span_list_t;
//! Span active data
प्रकारनिर्देश संरचना span_active_t span_active_t;
//! Size class definition
प्रकारनिर्देश संरचना size_class_t size_class_t;
//! Global cache
प्रकारनिर्देश संरचना global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#define SPAN_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#define SPAN_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#define SPAN_FLAG_ALIGNED_BLOCKS 4U
//! Flag indicating an unmapped master span
#define SPAN_FLAG_UNMAPPED_MASTER 8U

#जइ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
संरचना span_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t उच्चः;
#जइ ENABLE_STATISTICS
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
प्रकारनिर्देश संरचना span_use_t span_use_t;
#endif

#जइ ENABLE_STATISTICS
संरचना size_class_use_t {
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
प्रकारनिर्देश संरचना size_class_use_t size_class_use_t;
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
संरचना span_t {
  //! Free list
  शून्य *free_list;
  //! Total block count of size class
  uint32_t block_गणनम्;
  //! Size class
  uint32_t size_class;
  //! Index of last block initialized in free list
  uint32_t free_list_limit;
  //! Number of used blocks remaining when in partial state
  uint32_t used_गणनम्;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_size;
  //! Size of a block
  uint32_t block_size;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t span_गणनम्;
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
_Static_assert(परिमाण(span_t) <= SPAN_HEADER_SIZE, "span size mismatch");

संरचना span_cache_t {
  size_t गणनम्;
  span_t *span[MAX_THREAD_SPAN_CACHE];
};
प्रकारनिर्देश संरचना span_cache_t span_cache_t;

संरचना span_large_cache_t {
  size_t गणनम्;
  span_t *span[MAX_THREAD_SPAN_LARGE_CACHE];
};
प्रकारनिर्देश संरचना span_large_cache_t span_large_cache_t;

संरचना heap_size_class_t {
  //! Free list of active span
  शून्य *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  span_t *partial_span;
  //! Early level cache of fully free spans
  span_t *cache;
};
प्रकारनिर्देश संरचना heap_size_class_t heap_size_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
संरचना heap_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  heap_size_class_t size_class[SIZE_CLASS_गणनम्];
#जइ ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  span_cache_t span_cache;
#endif
  //! List of deferred free spans (single linked list)
  atomicptr_t span_free_deferred;
  //! Number of full spans
  size_t full_span_गणनम्;
  //! Mapped but unused spans
  span_t *span_reserve;
  //! Master span for mapped but unused spans
  span_t *span_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t child_गणनम्;
  //! Next heap in id list
  heap_t *next_heap;
  //! Next heap in orphan list
  heap_t *next_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  पूर्णाङ्क finalize;
  //! Master heap owning the memory pages
  heap_t *master_heap;
#जइ ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  span_large_cache_t span_large_cache[LARGE_CLASS_गणनम् - 1];
#endif
#जइ RPMALLOC_FIRST_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  span_t *full_span[SIZE_CLASS_गणनम्];
  //! Double linked list of large and huge spans allocated by this heap
  span_t *large_huge_span;
#endif
#जइ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  span_use_t span_use[LARGE_CLASS_गणनम्];
#endif
#जइ ENABLE_STATISTICS
  //! Allocation stats per size class
  size_class_use_t size_class_use[SIZE_CLASS_गणनम् + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_to_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_to_thread;
#endif
};

// Size class for defining a block size bucket
संरचना size_class_t {
  //! Size of blocks in this class
  uint32_t block_size;
  //! Number of blocks in each chunk
  uint16_t block_गणनम्;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(परिमाण(size_class_t) == 8, "Size class size mismatch");

संरचना global_cache_t {
  //! Cache lock
  atomic32_t lock;
  //! Cache count
  uint32_t गणनम्;
#जइ ENABLE_STATISTICS
  //! Insert count
  size_t निवेशय_गणनम्;
  //! Extract count
  size_t extract_गणनम्;
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
स्थिर पूर्णाङ्क _rpmalloc_initialized;
//! Main thread ID
स्थिर uintptr_t _rpmalloc_main_thread_id;
//! Configuration
स्थिर rpmalloc_config_t _memory_config;
//! Memory page size
स्थिर size_t _memory_page_size;
//! Shift to divide by page size
स्थिर size_t _memory_page_size_shift;
//! Granularity at which memory pages are mapped by OS
स्थिर size_t _memory_map_granularity;
#जइ RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
स्थिर size_t _memory_span_size;
//! Shift to divide by span size
स्थिर size_t _memory_span_size_shift;
//! Mask to get to start of a memory span
स्थिर uintptr_t _memory_span_mask;
#अण्णहा
//! Hardwired span size
#define _memory_span_size _memory_default_span_size
#define _memory_span_size_shift _memory_default_span_size_shift
#define _memory_span_mask _memory_default_span_mask
#endif
//! Number of spans to map in each map call
स्थिर size_t memory_span_map_गणनम्;
//! Number of spans to keep reserved in each heap
स्थिर size_t memory_heap_reserve_गणनम्;
//! Global size classes
स्थिर size_class_t _memory_size_class[SIZE_CLASS_गणनम्];
//! Run-time size limit of medium blocks
स्थिर size_t _memory_medium_size_limit;
//! Heap ID counter
स्थिर atomic32_t _memory_heap_id;
//! Huge page support
स्थिर पूर्णाङ्क _memory_huge_pages;
#जइ ENABLE_GLOBAL_CACHE
//! Global span cache
स्थिर global_cache_t _memory_span_cache[LARGE_CLASS_गणनम्];
#endif
//! Global reserved spans
स्थिर span_t *_memory_global_reserve;
//! Global reserved count
स्थिर size_t memory_global_reserve_गणनम्;
//! Global reserved master
स्थिर span_t *_memory_global_reserve_master;
//! All heaps
स्थिर heap_t *_memory_heaps[HEAP_ARRAY_SIZE];
//! Used to restrict access to mapping memory for huge pages
स्थिर atomic32_t _memory_global_lock;
//! Orphaned heaps
स्थिर heap_t *_memory_orphan_heaps;
#जइ RPMALLOC_FIRST_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
स्थिर heap_t *_memory_first_class_orphan_heaps;
#endif
#जइ ENABLE_STATISTICS
//! Allocations counter
स्थिर atomic64_t _allocation_counter;
//! Deallocations counter
स्थिर atomic64_t _deallocation_counter;
//! Active heap count
स्थिर atomic32_t _memory_active_heaps;
//! Number of currently mapped memory pages
स्थिर atomic32_t _mapped_pages;
//! Peak number of concurrently mapped memory pages
स्थिर int32_t _mapped_pages_peak;
//! Number of mapped master spans
स्थिर atomic32_t _master_spans;
//! Number of unmapped dangling master spans
स्थिर atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
स्थिर atomic32_t _mapped_total;
//! Running counter of total number of unmapped memory pages since start
स्थिर atomic32_t _unmapped_total;
//! Number of currently mapped memory pages in OS calls
स्थिर atomic32_t _mapped_pages_os;
//! Number of currently allocated pages in huge allocations
स्थिर atomic32_t _huge_pages_current;
//! Peak number of currently allocated pages in huge allocations
स्थिर int32_t _huge_pages_peak;
#endif

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#जइ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
स्थिर pthread_key_t _memory_thread_heap;
#अण्णहा
#ifdef _MSC_VER
#define _Thread_local __declspec(thread)
#define TLS_MODEL
#अण्णहा
#ifndef __HAIKU__
#define TLS_MODEL __attribute__((tls_model("initial-exec")))
#अण्णहा
#define TLS_MODEL
#endif
#जइ !defined(__clang__) && defined(__GNUC__)
#define _Thread_local __thread
#endif
#endif
स्थिर _Thread_local heap_t *_memory_thread_heap TLS_MODEL;
#endif

स्थिर अन्तःस्थ heap_t *get_thread_heap_raw(शून्य) {
#जइ (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  प्रत्यागम pthread_getspecific(_memory_thread_heap);
#अण्णहा
  प्रत्यागम _memory_thread_heap;
#endif
}

//! Get the current thread heap
स्थिर अन्तःस्थ heap_t *get_thread_heap(शून्य) {
  heap_t *heap = get_thread_heap_raw();
#जइ ENABLE_PRELOAD
  जइ (EXPECTED(heap != 0))
    प्रत्यागम heap;
  rpmalloc_initialize();
  प्रत्यागम get_thread_heap_raw();
#अण्णहा
  प्रत्यागम heap;
#endif
}

//! Fast thread ID
स्थिर अन्तःस्थ uintptr_t get_thread_id(शून्य) {
#जइ defined(_WIN32)
  प्रत्यागम (uintptr_t)((शून्य *)NtCurrentTeb());
#नोचेद्यदि (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#जइ defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#नोचेद्यदि defined(__x86_64__)
#जइ defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#अण्णहा
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#endif
#नोचेद्यदि defined(__arm__)
  __asm__ चञ्चल("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#नोचेद्यदि defined(__aarch64__)
#जइ defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ चञ्चल("mrs %0, tpidrro_el0" : "=r"(tid));
#अण्णहा
  __asm__ चञ्चल("mrs %0, tpidr_el0" : "=r"(tid));
#endif
#अण्णहा
#error This platform needs implementation of get_thread_id()
#endif
  प्रत्यागम tid;
#अण्णहा
#error This platform needs implementation of get_thread_id()
#endif
}

//! Set the current thread heap
स्थिर शून्य set_thread_heap(heap_t *heap) {
#जइ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(_memory_thread_heap, heap);
#अण्णहा
  _memory_thread_heap = heap;
#endif
  जइ (heap)
    heap->owner_thread = get_thread_id();
}

//! Set main thread ID
बाह्य शून्य rpmalloc_set_main_thread(शून्य);

शून्य rpmalloc_set_main_thread(शून्य) {
  _rpmalloc_main_thread_id = get_thread_id();
}

स्थिर शून्य _rpmalloc_spin(शून्य) {
#जइ defined(_MSC_VER)
#जइ defined(_M_ARM64)
  __yield();
#अण्णहा
  _mm_pause();
#endif
#नोचेद्यदि defined(__x86_64__) || defined(__i386__)
  __asm__ चञ्चल("pause" ::: "memory");
#नोचेद्यदि defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ चञ्चल("yield" ::: "memory");
#नोचेद्यदि defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ चञ्चल("or 27,27,27");
#नोचेद्यदि defined(__sparc__)
  __asm__ चञ्चल("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#अण्णहा
  संरचना timespec ts = {0};
  nanosleep(&ts, 0);
#endif
}

#जइ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

स्थिर शून्य NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  विकल्प (reason) {
  पक्ष DLL_PROCESS_ATTACH:
    भङ्ग;
  पक्ष DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    भङ्ग;
  पक्ष DLL_THREAD_ATTACH:
    भङ्ग;
  पक्ष DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    भङ्ग;
  }
}

#ifdef _WIN64
#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#pragma const_seg(".CRT$XLY")

बाह्य ध्रुव PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ध्रुव PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#pragma const_seg()
#अण्णहा // _WIN64
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#pragma data_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#pragma data_seg()
#endif // _WIN64

स्थिर शून्य NTAPI _rpmalloc_thread_destructor(शून्य *मानम्_2) {
#जइ ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  जइ (get_thread_id() == _rpmalloc_main_thread_id)
    प्रत्यागम;
#endif
  जइ (मानम्_2)
    rpmalloc_thread_finalize(1);
}
#endif

////////////
///
/// Low level memory map/unmap
///
//////

स्थिर शून्य _rpmalloc_set_name(शून्य *address, size_t size) {
#जइ defined(__linux__) || defined(__ANDROID__)
  ध्रुव वर्ण *name = _memory_huge_pages ? _memory_config.huge_page_name
                                        : _memory_config.page_name;
  जइ (address == MAP_FAILED || !name)
    प्रत्यागम;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (शून्य)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (uintptr_t)address, size,
              (uintptr_t)name);
#अण्णहा
  (शून्य)परिमाण(size);
  (शून्य)परिमाण(address);
#endif
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
स्थिर शून्य *_rpmalloc_mmap(size_t size, size_t *offset) {
  rpmalloc_assert(!(size % _memory_page_size), "Invalid mmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
  शून्य *address = _memory_config.memory_map(size, offset);
  जइ (EXPECTED(address != 0)) {
    _rpmalloc_stat_add_peak(&_mapped_pages, (size >> _memory_page_size_shift),
                            _mapped_pages_peak);
    _rpmalloc_stat_add(&_mapped_total, (size >> _memory_page_size_shift));
  }
  प्रत्यागम address;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
स्थिर शून्य _rpmalloc_unmap(शून्य *address, size_t size, size_t offset,
                            size_t release) {
  rpmalloc_assert(!release || (release >= size), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  जइ (release) {
    rpmalloc_assert(!(release % _memory_page_size), "Invalid unmap size");
    _rpmalloc_stat_sub(&_mapped_pages, (release >> _memory_page_size_shift));
    _rpmalloc_stat_add(&_unmapped_total, (release >> _memory_page_size_shift));
  }
  _memory_config.memory_unmap(address, size, offset, release);
}

//! Default implementation to map new pages to virtual memory
स्थिर शून्य *_rpmalloc_mmap_os(size_t size, size_t *offset) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((size >= _memory_span_size) &&
                    (_memory_span_size > _memory_map_granularity))
                       ? _memory_span_size
                       : 0;
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
#जइ PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  शून्य *ptr = VirtualAlloc(0, size + padding,
                           (_memory_huge_pages ? MEM_LARGE_PAGES : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
  जइ (!ptr) {
    जइ (_memory_config.map_fail_callback) {
      जइ (_memory_config.map_fail_callback(size + padding))
        प्रत्यागम _rpmalloc_mmap_os(size, offset);
    } अण्णहा {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    प्रत्यागम 0;
  }
#अण्णहा
  पूर्णाङ्क flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#जइ defined(__APPLE__) && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
  पूर्णाङ्क fd = (पूर्णाङ्क)VM_MAKE_TAG(240U);
  जइ (_memory_huge_pages)
    fd |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
  शून्य *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#नोचेद्यदि defined(MAP_HUGETLB)
  शून्य *ptr = mmap(0, size + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (_memory_huge_pages ? MAP_HUGETLB : 0) | flags, -1, 0);
#जइ defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  जइ ((ptr == MAP_FAILED || !ptr) && _memory_huge_pages) {
    ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    जइ (ptr && ptr != MAP_FAILED) {
      पूर्णाङ्क prm = madvise(ptr, size + padding, MADV_HUGEPAGE);
      (शून्य)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#endif
  _rpmalloc_set_name(ptr, size + padding);
#नोचेद्यदि defined(MAP_ALIGNED)
  ध्रुव size_t align =
      (परिमाण(size_t) * 8) - (size_t)(__builtin_clzl(size - 1));
  शून्य *ptr =
      mmap(0, size + padding, PROT_READ | PROT_WRITE,
           (_memory_huge_pages ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#नोचेद्यदि defined(MAP_ALIGN)
  caddr_t base = (_memory_huge_pages ? (caddr_t)(4 << 20) : 0);
  शून्य *ptr = mmap(base, size + padding, PROT_READ | PROT_WRITE,
                   (_memory_huge_pages ? MAP_ALIGN : 0) | flags, -1, 0);
#अण्णहा
  शून्य *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#endif
  जइ ((ptr == MAP_FAILED) || !ptr) {
    जइ (_memory_config.map_fail_callback) {
      जइ (_memory_config.map_fail_callback(size + padding))
        प्रत्यागम _rpmalloc_mmap_os(size, offset);
    } अण्णहा जइ (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    प्रत्यागम 0;
  }
#endif
  _rpmalloc_stat_add(&_mapped_pages_os,
                     (int32_t)((size + padding) >> _memory_page_size_shift));
  जइ (padding) {
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
  प्रत्यागम ptr;
}

//! Default implementation to unmap pages from virtual memory
स्थिर शून्य _rpmalloc_unmap_os(शून्य *address, size_t size, size_t offset,
                               size_t release) {
  rpmalloc_assert(release || (offset == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid unmap size");
  जइ (release && offset) {
    offset <<= 3;
    address = pointer_offset(address, -(int32_t)offset);
    जइ ((release >= _memory_span_size) &&
        (_memory_span_size > _memory_map_granularity)) {
      // Padding is always one span size
      release += _memory_span_size;
    }
  }
#जइ !DISABLE_UNMAP
#जइ PLATFORM_WINDOWS
  जइ (!VirtualFree(address, release ? 0 : size,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#अण्णहा
  जइ (release) {
    जइ (munmap(address, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } अण्णहा {
#जइ defined(MADV_FREE_REUSABLE)
    पूर्णाङ्क ret;
    जाव ((ret = madvise(address, size, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    जइ ((ret == -1) && (errno != 0)) {
#नोचेद्यदि defined(MADV_DONTNEED)
    जइ (madvise(address, size, MADV_DONTNEED)) {
#नोचेद्यदि defined(MADV_PAGEOUT)
    जइ (madvise(address, size, MADV_PAGEOUT)) {
#नोचेद्यदि defined(MADV_FREE)
    जइ (madvise(address, size, MADV_FREE)) {
#अण्णहा
    जइ (posix_madvise(address, size, POSIX_MADV_DONTNEED)) {
#endif
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#endif
#endif
  जइ (release)
    _rpmalloc_stat_sub(&_mapped_pages_os, release >> _memory_page_size_shift);
}

स्थिर शून्य _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_गणनम्);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
स्थिर span_t *_rpmalloc_global_get_reserved_spans(size_t span_गणनम्) {
  span_t *span = _memory_global_reserve;
  _rpmalloc_span_mark_as_subspan_unless_master(_memory_global_reserve_master,
                                               span, span_गणनम्);
  memory_global_reserve_गणनम् -= span_गणनम्;
  जइ (memory_global_reserve_गणनम्)
    _memory_global_reserve =
        (span_t *)pointer_offset(span, span_गणनम् << _memory_span_size_shift);
  अण्णहा
    _memory_global_reserve = 0;
  प्रत्यागम span;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
स्थिर शून्य _rpmalloc_global_set_reserved_spans(span_t *master, span_t *reserve,
                                                size_t reserve_span_गणनम्) {
  _memory_global_reserve_master = master;
  memory_global_reserve_गणनम् = reserve_span_गणनम्;
  _memory_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
स्थिर शून्य _rpmalloc_span_double_link_list_add(span_t **head, span_t *span) {
  जइ (*head)
    (*head)->prev = span;
  span->next = *head;
  *head = span;
}

//! Pop head span from double linked list
स्थिर शून्य rpmalloc_span_double_link_list_निष्कासय_head(span_t **head,
                                                     span_t *span) {
  rpmalloc_assert(*head == span, "Linked list corrupted");
  span = *head;
  *head = span->next;
}

//! Remove a span from double linked list
स्थिर शून्य _rpmalloc_span_double_link_list_remove(span_t **head,
                                                   span_t *span) {
  rpmalloc_assert(*head, "Linked list corrupted");
  जइ (*head == span) {
    *head = span->next;
  } अण्णहा {
    span_t *next_span = span->next;
    span_t *prev_span = span->prev;
    prev_span->next = next_span;
    जइ (EXPECTED(next_span != 0))
      next_span->prev = prev_span;
  }
}

////////////
///
/// Span control
///
//////

स्थिर शून्य rpmalloc_heap_cache_निवेशय(heap_t *heap, span_t *span);

स्थिर शून्य _rpmalloc_heap_finalize(heap_t *heap);

स्थिर शून्य _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_गणनम्);

//! Declare the span to be a subspan and store distance from master span and
//! span count
स्थिर शून्य _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_गणनम्) {
  rpmalloc_assert((subspan != master) || (subspan->flags & SPAN_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  जइ (subspan != master) {
    subspan->flags = SPAN_FLAG_SUBSPAN;
    subspan->offset_from_master =
        (uint32_t)((uintptr_t)pointer_diff(subspan, master) >>
                   _memory_span_size_shift);
    subspan->align_offset = 0;
  }
  subspan->span_गणनम् = (uint32_t)span_गणनम्;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
स्थिर span_t *_rpmalloc_span_map_from_reserve(heap_t *heap,
                                               size_t span_गणनम्) {
  // Update the heap span reserve
  span_t *span = heap->span_reserve;
  heap->span_reserve =
      (span_t *)pointer_offset(span, span_गणनम् * _memory_span_size);
  heap->spans_reserved -= (uint32_t)span_गणनम्;

  _rpmalloc_span_mark_as_subspan_unless_master(heap->span_reserve_master, span,
                                               span_गणनम्);
  जइ (span_गणनम् <= LARGE_CLASS_गणनम्)
    _rpmalloc_stat_inc(&heap->span_use[span_गणनम् - 1].spans_from_reserved);

  प्रत्यागम span;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
स्थिर size_t rpmalloc_span_align_गणनम्(size_t span_गणनम्) {
  size_t request_गणनम् = (span_गणनम् > memory_span_map_गणनम्)
                             ? span_गणनम्
                             : memory_span_map_गणनम्;
  जइ ((_memory_page_size > _memory_span_size) &&
      ((request_गणनम् * _memory_span_size) % _memory_page_size))
    request_गणनम् +=
        memory_span_map_गणनम् - (request_गणनम् % memory_span_map_गणनम्);
  प्रत्यागम request_गणनम्;
}

//! Setup a newly mapped span
स्थिर शून्य _rpmalloc_span_initialize(span_t *span, size_t total_span_गणनम्,
                                      size_t span_गणनम्, size_t align_offset) {
  span->total_spans = (uint32_t)total_span_गणनम्;
  span->span_गणनम् = (uint32_t)span_गणनम्;
  span->align_offset = (uint32_t)align_offset;
  span->flags = SPAN_FLAG_MASTER;
  atomic_store32(&span->remaining_spans, (int32_t)total_span_गणनम्);
}

स्थिर शून्य _rpmalloc_span_unmap(span_t *span);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
स्थिर span_t *rpmalloc_span_map_aligned_गणनम्(heap_t *heap,
                                                size_t span_गणनम्) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_span_गणनम् = rpmalloc_span_align_गणनम्(span_गणनम्);
  size_t align_offset = 0;
  span_t *span = (span_t *)_rpmalloc_mmap(
      aligned_span_गणनम् * _memory_span_size, &align_offset);
  जइ (!span)
    प्रत्यागम 0;
  _rpmalloc_span_initialize(span, aligned_span_गणनम्, span_गणनम्, align_offset);
  _rpmalloc_stat_inc(&_master_spans);
  जइ (span_गणनम् <= LARGE_CLASS_गणनम्)
    _rpmalloc_stat_inc(&heap->span_use[span_गणनम् - 1].spans_map_calls);
  जइ (aligned_span_गणनम् > span_गणनम्) {
    span_t *reserved_spans =
        (span_t *)pointer_offset(span, span_गणनम् * _memory_span_size);
    size_t reserved_गणनम् = aligned_span_गणनम् - span_गणनम्;
    जइ (heap->spans_reserved) {
      _rpmalloc_span_mark_as_subspan_unless_master(
          heap->span_reserve_master, heap->span_reserve, heap->spans_reserved);
      rpmalloc_heap_cache_निवेशय(heap, heap->span_reserve);
    }
    जइ (reserved_गणनम् > memory_heap_reserve_गणनम्) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&_memory_global_lock) == 1,
                      "Global spin lock not held as expected");
      size_t remain_गणनम् = reserved_गणनम् - memory_heap_reserve_गणनम्;
      reserved_गणनम् = memory_heap_reserve_गणनम्;
      span_t *remain_span = (span_t *)pointer_offset(
          reserved_spans, reserved_गणनम् * _memory_span_size);
      जइ (_memory_global_reserve) {
        _rpmalloc_span_mark_as_subspan_unless_master(
            _memory_global_reserve_master, _memory_global_reserve,
            memory_global_reserve_गणनम्);
        _rpmalloc_span_unmap(_memory_global_reserve);
      }
      _rpmalloc_global_set_reserved_spans(span, remain_span, remain_गणनम्);
    }
    _rpmalloc_heap_set_reserved_spans(heap, span, reserved_spans,
                                      reserved_गणनम्);
  }
  प्रत्यागम span;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
स्थिर span_t *_rpmalloc_span_map(heap_t *heap, size_t span_गणनम्) {
  जइ (span_गणनम् <= heap->spans_reserved)
    प्रत्यागम _rpmalloc_span_map_from_reserve(heap, span_गणनम्);
  span_t *span = 0;
  पूर्णाङ्क use_global_reserve =
      (_memory_page_size > _memory_span_size) ||
      (memory_span_map_गणनम् > memory_heap_reserve_गणनम्);
  जइ (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    जाव (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
    जइ (memory_global_reserve_गणनम् >= span_गणनम्) {
      size_t reserve_गणनम् =
          (!heap->spans_reserved ? memory_heap_reserve_गणनम् : span_गणनम्);
      जइ (memory_global_reserve_गणनम् < reserve_गणनम्)
        reserve_गणनम् = memory_global_reserve_गणनम्;
      span = _rpmalloc_global_get_reserved_spans(reserve_गणनम्);
      जइ (span) {
        जइ (reserve_गणनम् > span_गणनम्) {
          span_t *reserved_span = (span_t *)pointer_offset(
              span, span_गणनम् << _memory_span_size_shift);
          _rpmalloc_heap_set_reserved_spans(heap, _memory_global_reserve_master,
                                            reserved_span,
                                            reserve_गणनम् - span_गणनम्);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        span->span_गणनम् = (uint32_t)span_गणनम्;
      }
    }
  }
  जइ (!span)
    span = rpmalloc_span_map_aligned_गणनम्(heap, span_गणनम्);
  जइ (use_global_reserve)
    atomic_store32_release(&_memory_global_lock, 0);
  प्रत्यागम span;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
स्थिर शून्य _rpmalloc_span_unmap(span_t *span) {
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");

  पूर्णाङ्क is_master = !!(span->flags & SPAN_FLAG_MASTER);
  span_t *master =
      is_master ? span
                : ((span_t *)pointer_offset(
                      span, -(intptr_t)((uintptr_t)span->offset_from_master *
                                        _memory_span_size)));
  rpmalloc_assert(is_master || (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");

  size_t span_गणनम् = span->span_गणनम्;
  जइ (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(span->align_offset == 0, "Span align offset corrupted");
    जइ (_memory_span_size >= _memory_page_size)
      _rpmalloc_unmap(span, span_गणनम् * _memory_span_size, 0, 0);
  } अण्णहा {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    span->flags |=
        SPAN_FLAG_MASTER | SPAN_FLAG_SUBSPAN | SPAN_FLAG_UNMAPPED_MASTER;
    _rpmalloc_stat_add(&_unmapped_master_spans, 1);
  }

  जइ (atomic_add32(&master->remaining_spans, -(int32_t)span_गणनम्) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & SPAN_FLAG_MASTER) &&
                        !!(master->flags & SPAN_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_गणनम् = master->span_गणनम्;
    जइ (_memory_span_size < _memory_page_size)
      unmap_गणनम् = master->total_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_गणनम् * _memory_span_size,
                    master->align_offset,
                    (size_t)master->total_spans * _memory_span_size);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
स्थिर शून्य _rpmalloc_span_release_to_cache(heap_t *heap, span_t *span) {
  rpmalloc_assert(heap == span->heap, "Span heap pointer corrupted");
  rpmalloc_assert(span->size_class < SIZE_CLASS_गणनम्,
                  "Invalid span size class");
  rpmalloc_assert(span->span_गणनम् == 1, "Invalid span count");
#जइ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&heap->span_use[0].current);
#endif
  _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
  जइ (!heap->finalize) {
    _rpmalloc_stat_inc(&heap->span_use[0].spans_to_cache);
    _rpmalloc_stat_inc(&heap->size_class_use[span->size_class].spans_to_cache);
    जइ (heap->size_class[span->size_class].cache)
      rpmalloc_heap_cache_निवेशय(heap,
                                  heap->size_class[span->size_class].cache);
    heap->size_class[span->size_class].cache = span;
  } अण्णहा {
    _rpmalloc_span_unmap(span);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
स्थिर uint32_t free_list_partial_init(शून्य **list, शून्य **first_block,
                                       शून्य *page_start, शून्य *block_start,
                                       uint32_t block_गणनम्,
                                       uint32_t block_size) {
  rpmalloc_assert(block_गणनम्, "Internal failure");
  *first_block = block_start;
  जइ (block_गणनम् > 1) {
    शून्य *free_block = pointer_offset(block_start, block_size);
    शून्य *block_end =
        pointer_offset(block_start, (size_t)block_size * block_गणनम्);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    जइ (block_size < (_memory_page_size >> 1)) {
      शून्य *page_end = pointer_offset(page_start, _memory_page_size);
      जइ (page_end < block_end)
        block_end = page_end;
    }
    *list = free_block;
    block_गणनम् = 2;
    शून्य *next_block = pointer_offset(free_block, block_size);
    जाव (next_block < block_end) {
      *((शून्य **)free_block) = next_block;
      free_block = next_block;
      ++block_गणनम्;
      next_block = pointer_offset(next_block, block_size);
    }
    *((शून्य **)free_block) = 0;
  } अण्णहा {
    *list = 0;
  }
  प्रत्यागम block_गणनम्;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
स्थिर शून्य *_rpmalloc_span_initialize_new(heap_t *heap,
                                           heap_size_class_t *heap_size_class,
                                           span_t *span, uint32_t class_idx) {
  rpmalloc_assert(span->span_गणनम् == 1, "Internal failure");
  size_class_t *size_class = _memory_size_class + class_idx;
  span->size_class = class_idx;
  span->heap = heap;
  span->flags &= ~SPAN_FLAG_ALIGNED_BLOCKS;
  span->block_size = size_class->block_size;
  span->block_गणनम् = size_class->block_गणनम्;
  span->free_list = 0;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  शून्य *block;
  span->free_list_limit =
      free_list_partial_init(&heap_size_class->free_list, &block, span,
                             pointer_offset(span, SPAN_HEADER_SIZE),
                             size_class->block_गणनम्, size_class->block_size);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  जइ (span->free_list_limit < span->block_गणनम्) {
    _rpmalloc_span_double_link_list_add(&heap_size_class->partial_span, span);
    span->used_गणनम् = span->free_list_limit;
  } अण्णहा {
#जइ RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_गणनम्;
    span->used_गणनम् = span->block_गणनम्;
  }
  प्रत्यागम block;
}

स्थिर शून्य _rpmalloc_span_extract_free_list_deferred(span_t *span) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  करेहि {
    span->free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } जाव (span->free_list == INVALID_POINTER);
  span->used_गणनम् -= span->list_size;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);
}

स्थिर पूर्णाङ्क _rpmalloc_span_is_fully_utilized(span_t *span) {
  rpmalloc_assert(span->free_list_limit <= span->block_गणनम्,
                  "Span free list corrupted");
  प्रत्यागम !span->free_list && (span->free_list_limit >= span->block_गणनम्);
}

स्थिर पूर्णाङ्क _rpmalloc_span_finalize(heap_t *heap, size_t iclass, span_t *span,
                                   span_t **list_head) {
  शून्य *free_list = heap->size_class[iclass].free_list;
  span_t *class_span = (span_t *)((uintptr_t)free_list & _memory_span_mask);
  जइ (span == class_span) {
    // Adopt the heap class free list back into the span free list
    शून्य *block = span->free_list;
    शून्य *last_block = 0;
    जाव (block) {
      last_block = block;
      block = *((शून्य **)block);
    }
    uint32_t free_गणनम् = 0;
    block = free_list;
    जाव (block) {
      ++free_गणनम्;
      block = *((शून्य **)block);
    }
    जइ (last_block) {
      *((शून्य **)last_block) = free_list;
    } अण्णहा {
      span->free_list = free_list;
    }
    heap->size_class[iclass].free_list = 0;
    span->used_गणनम् -= free_गणनम्;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(span->list_size == span->used_गणनम्, "Memory leak detected");
  जइ (span->list_size == span->used_गणनम्) {
    _rpmalloc_stat_dec(&heap->span_use[0].current);
    _rpmalloc_stat_dec(&heap->size_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    जइ (list_head)
      _rpmalloc_span_double_link_list_remove(list_head, span);
    _rpmalloc_span_unmap(span);
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
}

////////////
///
/// Global cache
///
//////

#जइ ENABLE_GLOBAL_CACHE

//! Finalize a global cache
स्थिर शून्य _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  जाव (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

  प्रति (size_t ispan = 0; ispan < cache->गणनम्; ++ispan)
    _rpmalloc_span_unmap(cache->span[ispan]);
  cache->गणनम् = 0;

  जाव (cache->overflow) {
    span_t *span = cache->overflow;
    cache->overflow = span->next;
    _rpmalloc_span_unmap(span);
  }

  atomic_store32_release(&cache->lock, 0);
}

स्थिर शून्य rpmalloc_global_cache_निवेशय_spans(span_t **span,
                                                size_t span_गणनम्,
                                                size_t गणनम्) {
  ध्रुव size_t cache_limit =
      (span_गणनम् == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_SPAN_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_SPAN_LARGE_CACHE - (span_गणनम् >> 1));

  global_cache_t *cache = &_memory_span_cache[span_गणनम् - 1];

  size_t निवेशय_गणनम् = गणनम्;
  जाव (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#जइ ENABLE_STATISTICS
  cache->निवेशय_गणनम् += गणनम्;
#endif
  जइ ((cache->गणनम् + निवेशय_गणनम्) > cache_limit)
    निवेशय_गणनम् = cache_limit - cache->गणनम्;

  memcpy(cache->span + cache->गणनम्, span, परिमाण(span_t *) * निवेशय_गणनम्);
  cache->गणनम् += (uint32_t)निवेशय_गणनम्;

#जइ ENABLE_UNLIMITED_CACHE
  जाव (निवेशय_गणनम् < गणनम्) {
#अण्णहा
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  जाव ((_memory_page_size > _memory_span_size) && (निवेशय_गणनम् < गणनम्)) {
#endif
    span_t *current_span = span[निवेशय_गणनम्++];
    current_span->next = cache->overflow;
    cache->overflow = current_span;
  }
  atomic_store32_release(&cache->lock, 0);

  span_t *keep = 0;
  प्रति (size_t ispan = निवेशय_गणनम्; ispan < गणनम्; ++ispan) {
    span_t *current_span = span[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    जइ ((current_span->flags & SPAN_FLAG_MASTER) &&
        (atomic_load32(&current_span->remaining_spans) >
         (int32_t)current_span->span_गणनम्)) {
      current_span->next = keep;
      keep = current_span;
    } अण्णहा {
      _rpmalloc_span_unmap(current_span);
    }
  }

  जइ (keep) {
    जाव (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    जाव (keep) {
      प्रति (; islot < cache->गणनम्; ++islot) {
        span_t *current_span = cache->span[islot];
        जइ (!(current_span->flags & SPAN_FLAG_MASTER) ||
            ((current_span->flags & SPAN_FLAG_MASTER) &&
             (atomic_load32(&current_span->remaining_spans) <=
              (int32_t)current_span->span_गणनम्))) {
          _rpmalloc_span_unmap(current_span);
          cache->span[islot] = keep;
          भङ्ग;
        }
      }
      जइ (islot == cache->गणनम्)
        भङ्ग;
      keep = keep->next;
    }

    जइ (keep) {
      span_t *tail = keep;
      जाव (tail->next)
        tail = tail->next;
      tail->next = cache->overflow;
      cache->overflow = keep;
    }

    atomic_store32_release(&cache->lock, 0);
  }
}

स्थिर size_t _rpmalloc_global_cache_extract_spans(span_t **span,
                                                   size_t span_गणनम्,
                                                   size_t गणनम्) {
  global_cache_t *cache = &_memory_span_cache[span_गणनम् - 1];

  size_t extract_गणनम् = 0;
  जाव (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#जइ ENABLE_STATISTICS
  cache->extract_गणनम् += गणनम्;
#endif
  size_t want = गणनम् - extract_गणनम्;
  जइ (want > cache->गणनम्)
    want = cache->गणनम्;

  memcpy(span + extract_गणनम्, cache->span + (cache->गणनम् - want),
         परिमाण(span_t *) * want);
  cache->गणनम् -= (uint32_t)want;
  extract_गणनम् += want;

  जाव ((extract_गणनम् < गणनम्) && cache->overflow) {
    span_t *current_span = cache->overflow;
    span[extract_गणनम्++] = current_span;
    cache->overflow = current_span->next;
  }

#जइ ENABLE_ASSERTS
  प्रति (size_t ispan = 0; ispan < extract_गणनम्; ++ispan) {
    rpmalloc_assert(span[ispan]->span_गणनम् == span_गणनम्,
                    "Global cache span count mismatch");
  }
#endif

  atomic_store32_release(&cache->lock, 0);

  प्रत्यागम extract_गणनम्;
}

#endif

////////////
///
/// Heap control
///
//////

स्थिर शून्य _rpmalloc_deallocate_huge(span_t *);

//! Store the given spans as reserve in the given heap
स्थिर शून्य _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_गणनम्) {
  heap->span_reserve_master = master;
  heap->span_reserve = reserve;
  heap->spans_reserved = (uint32_t)reserve_span_गणनम्;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
स्थिर शून्य _rpmalloc_heap_cache_adopt_deferred(heap_t *heap,
                                                span_t **single_span) {
  span_t *span = (span_t *)((शून्य *)atomic_exchange_ptr_acquire(
      &heap->span_free_deferred, 0));
  जाव (span) {
    span_t *next_span = (span_t *)span->free_list;
    rpmalloc_assert(span->heap == heap, "Span heap pointer corrupted");
    जइ (EXPECTED(span->size_class < SIZE_CLASS_गणनम्)) {
      rpmalloc_assert(heap->full_span_गणनम्, "Heap span counter corrupted");
      --heap->full_span_गणनम्;
      _rpmalloc_stat_dec(&heap->span_use[0].spans_deferred);
#जइ RPMALLOC_FIRST_CLASS_HEAPS
      _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                             span);
#endif
      _rpmalloc_stat_dec(&heap->span_use[0].current);
      _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
      जइ (single_span && !*single_span)
        *single_span = span;
      अण्णहा
        rpmalloc_heap_cache_निवेशय(heap, span);
    } अण्णहा {
      जइ (span->size_class == SIZE_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(span);
      } अण्णहा {
        rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(heap->full_span_गणनम्, "Heap span counter corrupted");
        --heap->full_span_गणनम्;
#जइ RPMALLOC_FIRST_CLASS_HEAPS
        _rpmalloc_span_double_link_list_remove(&heap->large_huge_span, span);
#endif
        uint32_t idx = span->span_गणनम् - 1;
        _rpmalloc_stat_dec(&heap->span_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&heap->span_use[idx].current);
        जइ (!idx && single_span && !*single_span)
          *single_span = span;
        अण्णहा
          rpmalloc_heap_cache_निवेशय(heap, span);
      }
    }
    span = next_span;
  }
}

स्थिर शून्य _rpmalloc_heap_unmap(heap_t *heap) {
  जइ (!heap->master_heap) {
    जइ ((heap->finalize > 1) && !atomic_load32(&heap->child_गणनम्)) {
      span_t *span = (span_t *)((uintptr_t)heap & _memory_span_mask);
      _rpmalloc_span_unmap(span);
    }
  } अण्णहा {
    जइ (atomic_decr32(&heap->master_heap->child_गणनम्) == 0) {
      _rpmalloc_heap_unmap(heap->master_heap);
    }
  }
}

स्थिर शून्य _rpmalloc_heap_global_finalize(heap_t *heap) {
  जइ (heap->finalize++ > 1) {
    --heap->finalize;
    प्रत्यागम;
  }

  _rpmalloc_heap_finalize(heap);

#जइ ENABLE_THREAD_CACHE
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    span_cache_t *span_cache;
    जइ (!iclass)
      span_cache = &heap->span_cache;
    अण्णहा
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    प्रति (size_t ispan = 0; ispan < span_cache->गणनम्; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->गणनम् = 0;
  }
#endif

  जइ (heap->full_span_गणनम्) {
    --heap->finalize;
    प्रत्यागम;
  }

  प्रति (size_t iclass = 0; iclass < SIZE_CLASS_गणनम्; ++iclass) {
    जइ (heap->size_class[iclass].free_list ||
        heap->size_class[iclass].partial_span) {
      --heap->finalize;
      प्रत्यागम;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap_t *list_heap = _memory_heaps[list_idx];
  जइ (list_heap == heap) {
    _memory_heaps[list_idx] = heap->next_heap;
  } अण्णहा {
    जाव (list_heap->next_heap != heap)
      list_heap = list_heap->next_heap;
    list_heap->next_heap = heap->next_heap;
  }

  _rpmalloc_heap_unmap(heap);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
स्थिर शून्य rpmalloc_heap_cache_निवेशय(heap_t *heap, span_t *span) {
  जइ (UNEXPECTED(heap->finalize != 0)) {
    _rpmalloc_span_unmap(span);
    _rpmalloc_heap_global_finalize(heap);
    प्रत्यागम;
  }
#जइ ENABLE_THREAD_CACHE
  size_t span_गणनम् = span->span_गणनम्;
  _rpmalloc_stat_inc(&heap->span_use[span_गणनम् - 1].spans_to_cache);
  जइ (span_गणनम् == 1) {
    span_cache_t *span_cache = &heap->span_cache;
    span_cache->span[span_cache->गणनम्++] = span;
    जइ (span_cache->गणनम् == MAX_THREAD_SPAN_CACHE) {
      ध्रुव size_t remain_गणनम् =
          MAX_THREAD_SPAN_CACHE - THREAD_SPAN_CACHE_TRANSFER;
#जइ ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           THREAD_SPAN_CACHE_TRANSFER * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_गणनम् - 1].spans_to_global,
                         THREAD_SPAN_CACHE_TRANSFER);
      rpmalloc_global_cache_निवेशय_spans(span_cache->span + remain_गणनम्,
                                          span_गणनम्,
                                          THREAD_SPAN_CACHE_TRANSFER);
#अण्णहा
      प्रति (size_t ispan = 0; ispan < THREAD_SPAN_CACHE_TRANSFER; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_गणनम् + ispan]);
#endif
      span_cache->गणनम् = remain_गणनम्;
    }
  } अण्णहा {
    size_t cache_idx = span_गणनम् - 2;
    span_large_cache_t *span_cache = heap->span_large_cache + cache_idx;
    span_cache->span[span_cache->गणनम्++] = span;
    ध्रुव size_t cache_limit =
        (MAX_THREAD_SPAN_LARGE_CACHE - (span_गणनम् >> 1));
    जइ (span_cache->गणनम् == cache_limit) {
      ध्रुव size_t transfer_limit = 2 + (cache_limit >> 2);
      ध्रुव size_t transfer_गणनम् =
          (THREAD_SPAN_LARGE_CACHE_TRANSFER <= transfer_limit
               ? THREAD_SPAN_LARGE_CACHE_TRANSFER
               : transfer_limit);
      ध्रुव size_t remain_गणनम् = cache_limit - transfer_गणनम्;
#जइ ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           transfer_गणनम् * span_गणनम् * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_गणनम् - 1].spans_to_global,
                         transfer_गणनम्);
      rpmalloc_global_cache_निवेशय_spans(span_cache->span + remain_गणनम्,
                                          span_गणनम्, transfer_गणनम्);
#अण्णहा
      प्रति (size_t ispan = 0; ispan < transfer_गणनम्; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_गणनम् + ispan]);
#endif
      span_cache->गणनम् = remain_गणनम्;
    }
  }
#अण्णहा
  (शून्य)परिमाण(heap);
  _rpmalloc_span_unmap(span);
#endif
}

//! Extract the given number of spans from the different cache levels
स्थिर span_t *_rpmalloc_heap_thread_cache_extract(heap_t *heap,
                                                   size_t span_गणनम्) {
  span_t *span = 0;
#जइ ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  जइ (span_गणनम् == 1)
    span_cache = &heap->span_cache;
  अण्णहा
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_गणनम् - 2));
  जइ (span_cache->गणनम्) {
    _rpmalloc_stat_inc(&heap->span_use[span_गणनम् - 1].spans_from_cache);
    प्रत्यागम span_cache->span[--span_cache->गणनम्];
  }
#endif
  प्रत्यागम span;
}

स्थिर span_t *_rpmalloc_heap_thread_cache_deferred_extract(heap_t *heap,
                                                            size_t span_गणनम्) {
  span_t *span = 0;
  जइ (span_गणनम् == 1) {
    _rpmalloc_heap_cache_adopt_deferred(heap, &span);
  } अण्णहा {
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
    span = _rpmalloc_heap_thread_cache_extract(heap, span_गणनम्);
  }
  प्रत्यागम span;
}

स्थिर span_t *_rpmalloc_heap_reserved_extract(heap_t *heap,
                                               size_t span_गणनम्) {
  जइ (heap->spans_reserved >= span_गणनम्)
    प्रत्यागम _rpmalloc_span_map(heap, span_गणनम्);
  प्रत्यागम 0;
}

//! Extract a span from the global cache
स्थिर span_t *_rpmalloc_heap_global_cache_extract(heap_t *heap,
                                                   size_t span_गणनम्) {
#जइ ENABLE_GLOBAL_CACHE
#जइ ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  size_t wanted_गणनम्;
  जइ (span_गणनम् == 1) {
    span_cache = &heap->span_cache;
    wanted_गणनम् = THREAD_SPAN_CACHE_TRANSFER;
  } अण्णहा {
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_गणनम् - 2));
    wanted_गणनम् = THREAD_SPAN_LARGE_CACHE_TRANSFER;
  }
  span_cache->गणनम् = _rpmalloc_global_cache_extract_spans(
      span_cache->span, span_गणनम्, wanted_गणनम्);
  जइ (span_cache->गणनम्) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_गणनम् * span_cache->गणनम् * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_गणनम् - 1].spans_from_global,
                       span_cache->गणनम्);
    प्रत्यागम span_cache->span[--span_cache->गणनम्];
  }
#अण्णहा
  span_t *span = 0;
  size_t गणनम् = _rpmalloc_global_cache_extract_spans(&span, span_गणनम्, 1);
  जइ (गणनम्) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_गणनम् * गणनम् * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_गणनम् - 1].spans_from_global,
                       गणनम्);
    प्रत्यागम span;
  }
#endif
#endif
  (शून्य)परिमाण(heap);
  (शून्य)परिमाण(span_गणनम्);
  प्रत्यागम 0;
}

स्थिर शून्य _rpmalloc_inc_span_statistics(heap_t *heap, size_t span_गणनम्,
                                          uint32_t class_idx) {
  (शून्य)परिमाण(heap);
  (शून्य)परिमाण(span_गणनम्);
  (शून्य)परिमाण(class_idx);
#जइ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)span_गणनम् - 1;
  uint32_t current_गणनम् =
      (uint32_t)atomic_incr32(&heap->span_use[idx].current);
  जइ (current_गणनम् > (uint32_t)atomic_load32(&heap->span_use[idx].उच्चः))
    atomic_store32(&heap->span_use[idx].उच्चः, (int32_t)current_गणनम्);
  _rpmalloc_stat_add_peak(&heap->size_class_use[class_idx].spans_current, 1,
                          heap->size_class_use[class_idx].spans_peak);
#endif
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
स्थिर span_t *
_rpmalloc_heap_extract_new_span(heap_t *heap,
                                heap_size_class_t *heap_size_class,
                                size_t span_गणनम्, uint32_t class_idx) {
  span_t *span;
#जइ ENABLE_THREAD_CACHE
  जइ (heap_size_class && heap_size_class->cache) {
    span = heap_size_class->cache;
    heap_size_class->cache =
        (heap->span_cache.गणनम्
             ? heap->span_cache.span[--heap->span_cache.गणनम्]
             : 0);
    _rpmalloc_inc_span_statistics(heap, span_गणनम्, class_idx);
    प्रत्यागम span;
  }
#endif
  (शून्य)परिमाण(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t base_span_गणनम् = span_गणनम्;
  size_t limit_span_गणनम् =
      (span_गणनम् > 2) ? (span_गणनम् + (span_गणनम् >> 1)) : span_गणनम्;
  जइ (limit_span_गणनम् > LARGE_CLASS_गणनम्)
    limit_span_गणनम् = LARGE_CLASS_गणनम्;
  करेहि {
    span = _rpmalloc_heap_thread_cache_extract(heap, span_गणनम्);
    जइ (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_गणनम्, class_idx);
      प्रत्यागम span;
    }
    span = _rpmalloc_heap_thread_cache_deferred_extract(heap, span_गणनम्);
    जइ (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_गणनम्, class_idx);
      प्रत्यागम span;
    }
    span = _rpmalloc_heap_global_cache_extract(heap, span_गणनम्);
    जइ (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_गणनम्, class_idx);
      प्रत्यागम span;
    }
    span = _rpmalloc_heap_reserved_extract(heap, span_गणनम्);
    जइ (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_reserved);
      _rpmalloc_inc_span_statistics(heap, span_गणनम्, class_idx);
      प्रत्यागम span;
    }
    ++span_गणनम्;
  } जाव (span_गणनम् <= limit_span_गणनम्);
  // Final fallback, map in more virtual memory
  span = _rpmalloc_span_map(heap, base_span_गणनम्);
  _rpmalloc_inc_span_statistics(heap, base_span_गणनम्, class_idx);
  _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_map_calls);
  प्रत्यागम span;
}

स्थिर शून्य _rpmalloc_heap_initialize(heap_t *heap) {
  _rpmalloc_memset_const(heap, 0, परिमाण(heap_t));
  // Get a new heap ID
  heap->id = 1 + atomic_incr32(&_memory_heap_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap->next_heap = _memory_heaps[list_idx];
  _memory_heaps[list_idx] = heap;
}

स्थिर शून्य _rpmalloc_heap_orphan(heap_t *heap, पूर्णाङ्क first_class) {
  heap->owner_thread = (uintptr_t)-1;
#जइ RPMALLOC_FIRST_CLASS_HEAPS
  heap_t **heap_list =
      (first_class ? &_memory_first_class_orphan_heaps : &_memory_orphan_heaps);
#अण्णहा
  (शून्य)परिमाण(first_class);
  heap_t **heap_list = &_memory_orphan_heaps;
#endif
  heap->next_orphan = *heap_list;
  *heap_list = heap;
}

//! Allocate a new heap from newly mapped memory pages
स्थिर heap_t *_rpmalloc_heap_allocate_new(शून्य) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t heap_size = परिमाण(heap_t);
  size_t aligned_heap_size = 16 * ((heap_size + 15) / 16);
  size_t request_heap_गणनम् = 16;
  size_t heap_span_गणनम् = ((aligned_heap_size * request_heap_गणनम्) +
                            परिमाण(span_t) + _memory_span_size - 1) /
                           _memory_span_size;
  size_t block_size = _memory_span_size * heap_span_गणनम्;
  size_t span_गणनम् = heap_span_गणनम्;
  span_t *span = 0;
  // If there are global reserved spans, use these first
  जइ (memory_global_reserve_गणनम् >= heap_span_गणनम्) {
    span = _rpmalloc_global_get_reserved_spans(heap_span_गणनम्);
  }
  जइ (!span) {
    जइ (_memory_page_size > block_size) {
      span_गणनम् = _memory_page_size / _memory_span_size;
      block_size = _memory_page_size;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_heap_गणनम् =
          (block_size - परिमाण(span_t)) / aligned_heap_size;
      जइ (possible_heap_गणनम् >= (request_heap_गणनम् * 16))
        request_heap_गणनम् *= 16;
      अण्णहा जइ (possible_heap_गणनम् < request_heap_गणनम्)
        request_heap_गणनम् = possible_heap_गणनम्;
      heap_span_गणनम् = ((aligned_heap_size * request_heap_गणनम्) +
                         परिमाण(span_t) + _memory_span_size - 1) /
                        _memory_span_size;
    }

    size_t align_offset = 0;
    span = (span_t *)_rpmalloc_mmap(block_size, &align_offset);
    जइ (!span)
      प्रत्यागम 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    _rpmalloc_span_initialize(span, span_गणनम्, heap_span_गणनम्, align_offset);
  }

  size_t remain_size = _memory_span_size - परिमाण(span_t);
  heap_t *heap = (heap_t *)pointer_offset(span, परिमाण(span_t));
  _rpmalloc_heap_initialize(heap);

  // Put extra heaps as orphans
  size_t अङ्कः_heaps = remain_size / aligned_heap_size;
  जइ (अङ्कः_heaps < request_heap_गणनम्)
    अङ्कः_heaps = request_heap_गणनम्;
  atomic_store32(&heap->child_गणनम्, (int32_t)अङ्कः_heaps - 1);
  heap_t *extra_heap = (heap_t *)pointer_offset(heap, aligned_heap_size);
  जाव (अङ्कः_heaps > 1) {
    _rpmalloc_heap_initialize(extra_heap);
    extra_heap->master_heap = heap;
    _rpmalloc_heap_orphan(extra_heap, 1);
    extra_heap = (heap_t *)pointer_offset(extra_heap, aligned_heap_size);
    --अङ्कः_heaps;
  }

  जइ (span_गणनम् > heap_span_गणनम्) {
    // Cap reserved spans
    size_t remain_गणनम् = span_गणनम् - heap_span_गणनम्;
    size_t reserve_गणनम् =
        (remain_गणनम् > memory_heap_reserve_गणनम् ? memory_heap_reserve_गणनम्
                                                   : remain_गणनम्);
    span_t *remain_span =
        (span_t *)pointer_offset(span, heap_span_गणनम् * _memory_span_size);
    _rpmalloc_heap_set_reserved_spans(heap, span, remain_span, reserve_गणनम्);

    जइ (remain_गणनम् > reserve_गणनम्) {
      // Set to global reserved spans
      remain_span = (span_t *)pointer_offset(remain_span,
                                             reserve_गणनम् * _memory_span_size);
      reserve_गणनम् = remain_गणनम् - reserve_गणनम्;
      _rpmalloc_global_set_reserved_spans(span, remain_span, reserve_गणनम्);
    }
  }

  प्रत्यागम heap;
}

स्थिर heap_t *_rpmalloc_heap_extract_orphan(heap_t **heap_list) {
  heap_t *heap = *heap_list;
  *heap_list = (heap ? heap->next_orphan : 0);
  प्रत्यागम heap;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
स्थिर heap_t *_rpmalloc_heap_allocate(पूर्णाङ्क first_class) {
  heap_t *heap = 0;
  जाव (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
    _rpmalloc_spin();
  जइ (first_class == 0)
    heap = _rpmalloc_heap_extract_orphan(&_memory_orphan_heaps);
#जइ RPMALLOC_FIRST_CLASS_HEAPS
  जइ (!heap)
    heap = _rpmalloc_heap_extract_orphan(&_memory_first_class_orphan_heaps);
#endif
  जइ (!heap)
    heap = _rpmalloc_heap_allocate_new();
  atomic_store32_release(&_memory_global_lock, 0);
  जइ (heap)
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  प्रत्यागम heap;
}

स्थिर शून्य _rpmalloc_heap_release(शून्य *heapptr, पूर्णाङ्क first_class,
                                   पूर्णाङ्क release_cache) {
  heap_t *heap = (heap_t *)heapptr;
  जइ (!heap)
    प्रत्यागम;
  // Release thread cache spans back to global cache
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  जइ (release_cache || heap->finalize) {
#जइ ENABLE_THREAD_CACHE
    प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
      span_cache_t *span_cache;
      जइ (!iclass)
        span_cache = &heap->span_cache;
      अण्णहा
        span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
      जइ (!span_cache->गणनम्)
        अनुवर्त;
#जइ ENABLE_GLOBAL_CACHE
      जइ (heap->finalize) {
        प्रति (size_t ispan = 0; ispan < span_cache->गणनम्; ++ispan)
          _rpmalloc_span_unmap(span_cache->span[ispan]);
      } अण्णहा {
        _rpmalloc_stat_add64(&heap->thread_to_global, span_cache->गणनम् *
                                                          (iclass + 1) *
                                                          _memory_span_size);
        _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                           span_cache->गणनम्);
        rpmalloc_global_cache_निवेशय_spans(span_cache->span, iclass + 1,
                                            span_cache->गणनम्);
      }
#अण्णहा
      प्रति (size_t ispan = 0; ispan < span_cache->गणनम्; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
      span_cache->गणनम् = 0;
    }
#endif
  }

  जइ (get_thread_heap_raw() == heap)
    set_thread_heap(0);

#जइ ENABLE_STATISTICS
  atomic_decr32(&_memory_active_heaps);
  rpmalloc_assert(atomic_load32(&_memory_active_heaps) >= 0,
                  "Still active heaps during finalization");
#endif

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  जइ (get_thread_id() != _rpmalloc_main_thread_id) {
    जाव (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
  }
  _rpmalloc_heap_orphan(heap, first_class);
  atomic_store32_release(&_memory_global_lock, 0);
}

स्थिर शून्य _rpmalloc_heap_release_raw(शून्य *heapptr, पूर्णाङ्क release_cache) {
  _rpmalloc_heap_release(heapptr, 0, release_cache);
}

स्थिर शून्य _rpmalloc_heap_release_raw_fc(शून्य *heapptr) {
  _rpmalloc_heap_release_raw(heapptr, 1);
}

स्थिर शून्य _rpmalloc_heap_finalize(heap_t *heap) {
  जइ (heap->spans_reserved) {
    span_t *span = _rpmalloc_span_map(heap, heap->spans_reserved);
    _rpmalloc_span_unmap(span);
    heap->spans_reserved = 0;
  }

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  प्रति (size_t iclass = 0; iclass < SIZE_CLASS_गणनम्; ++iclass) {
    जइ (heap->size_class[iclass].cache)
      _rpmalloc_span_unmap(heap->size_class[iclass].cache);
    heap->size_class[iclass].cache = 0;
    span_t *span = heap->size_class[iclass].partial_span;
    जाव (span) {
      span_t *next = span->next;
      _rpmalloc_span_finalize(heap, iclass, span,
                              &heap->size_class[iclass].partial_span);
      span = next;
    }
    // If class still has a free list it must be a full span
    जइ (heap->size_class[iclass].free_list) {
      span_t *class_span =
          (span_t *)((uintptr_t)heap->size_class[iclass].free_list &
                     _memory_span_mask);
      span_t **list = 0;
#जइ RPMALLOC_FIRST_CLASS_HEAPS
      list = &heap->full_span[iclass];
#endif
      --heap->full_span_गणनम्;
      जइ (!_rpmalloc_span_finalize(heap, iclass, class_span, list)) {
        जइ (list)
          _rpmalloc_span_double_link_list_remove(list, class_span);
        _rpmalloc_span_double_link_list_add(
            &heap->size_class[iclass].partial_span, class_span);
      }
    }
  }

#जइ ENABLE_THREAD_CACHE
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    span_cache_t *span_cache;
    जइ (!iclass)
      span_cache = &heap->span_cache;
    अण्णहा
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    प्रति (size_t ispan = 0; ispan < span_cache->गणनम्; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->गणनम् = 0;
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
स्थिर शून्य *free_list_निष्कासय(शून्य **list) {
  शून्य *block = *list;
  *list = *((शून्य **)block);
  प्रत्यागम block;
}

//! Allocate a small/medium sized memory block from the given heap
स्थिर शून्य *_rpmalloc_allocate_from_heap_fallback(
    heap_t *heap, heap_size_class_t *heap_size_class, uint32_t class_idx) {
  span_t *span = heap_size_class->partial_span;
  rpmalloc_assume(heap != 0);
  जइ (EXPECTED(span != 0)) {
    rpmalloc_assert(span->block_गणनम् ==
                        _memory_size_class[span->size_class].block_गणनम्,
                    "Span block count corrupted");
    rpmalloc_assert(!_rpmalloc_span_is_fully_utilized(span),
                    "Internal failure");
    शून्य *block;
    जइ (span->free_list) {
      // Span local free list is not empty, swap to size class free list
      block = free_list_निष्कासय(&span->free_list);
      heap_size_class->free_list = span->free_list;
      span->free_list = 0;
    } अण्णहा {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      शून्य *block_start = pointer_offset(
          span, SPAN_HEADER_SIZE +
                    ((size_t)span->free_list_limit * span->block_size));
      span->free_list_limit += free_list_partial_init(
          &heap_size_class->free_list, &block,
          (शून्य *)((uintptr_t)block_start & ~(_memory_page_size - 1)),
          block_start, span->block_गणनम् - span->free_list_limit,
          span->block_size);
    }
    rpmalloc_assert(span->free_list_limit <= span->block_गणनम्,
                    "Span block count corrupted");
    span->used_गणनम् = span->free_list_limit;

    // Swap in deferred free list if present
    जइ (atomic_load_ptr(&span->free_list_deferred))
      _rpmalloc_span_extract_free_list_deferred(span);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    जइ (!_rpmalloc_span_is_fully_utilized(span))
      प्रत्यागम block;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_span_double_link_list_निष्कासय_head(&heap_size_class->partial_span,
                                             span);
#जइ RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_गणनम्;
    प्रत्यागम block;
  }

  // Find a span in one of the cache levels
  span = _rpmalloc_heap_extract_new_span(heap, heap_size_class, 1, class_idx);
  जइ (EXPECTED(span != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    प्रत्यागम _rpmalloc_span_initialize_new(heap, heap_size_class, span,
                                         class_idx);
  }

  प्रत्यागम 0;
}

//! Allocate a small sized memory block from the given heap
स्थिर शून्य *_rpmalloc_allocate_small(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Small sizes have unique size classes
  ध्रुव uint32_t class_idx =
      (uint32_t)((size + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_SHIFT);
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  जइ (EXPECTED(heap_size_class->free_list != 0))
    प्रत्यागम free_list_निष्कासय(&heap_size_class->free_list);
  प्रत्यागम _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
स्थिर शून्य *_rpmalloc_allocate_medium(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ध्रुव uint32_t base_idx =
      (uint32_t)(SMALL_CLASS_गणनम् +
                 ((size - (SMALL_SIZE_LIMIT + 1)) >> MEDIUM_GRANULARITY_SHIFT));
  ध्रुव uint32_t class_idx = _memory_size_class[base_idx].class_idx;
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  जइ (EXPECTED(heap_size_class->free_list != 0))
    प्रत्यागम free_list_निष्कासय(&heap_size_class->free_list);
  प्रत्यागम _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
स्थिर शून्य *_rpmalloc_allocate_large(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  size += SPAN_HEADER_SIZE;
  size_t span_गणनम् = size >> _memory_span_size_shift;
  जइ (size & (_memory_span_size - 1))
    ++span_गणनम्;

  // Find a span in one of the cache levels
  span_t *span =
      _rpmalloc_heap_extract_new_span(heap, 0, span_गणनम्, SIZE_CLASS_LARGE);
  जइ (!span)
    प्रत्यागम span;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(span->span_गणनम् >= span_गणनम्, "Internal failure");
  span->size_class = SIZE_CLASS_LARGE;
  span->heap = heap;

#जइ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_गणनम्;

  प्रत्यागम pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a huge block by mapping memory pages directly
स्थिर शून्य *_rpmalloc_allocate_huge(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  size += SPAN_HEADER_SIZE;
  size_t अङ्कः_pages = size >> _memory_page_size_shift;
  जइ (size & (_memory_page_size - 1))
    ++अङ्कः_pages;
  size_t align_offset = 0;
  span_t *span =
      (span_t *)_rpmalloc_mmap(अङ्कः_pages * _memory_page_size, &align_offset);
  जइ (!span)
    प्रत्यागम span;

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_गणनम् = (uint32_t)अङ्कः_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, अङ्कः_pages, _huge_pages_peak);

#जइ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_गणनम्;

  प्रत्यागम pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a block of the given size
स्थिर शून्य *_rpmalloc_allocate(heap_t *heap, size_t size) {
  _rpmalloc_stat_add64(&_allocation_counter, 1);
  जइ (EXPECTED(size <= SMALL_SIZE_LIMIT))
    प्रत्यागम _rpmalloc_allocate_small(heap, size);
  अण्णहा जइ (size <= _memory_medium_size_limit)
    प्रत्यागम _rpmalloc_allocate_medium(heap, size);
  अण्णहा जइ (size <= LARGE_SIZE_LIMIT)
    प्रत्यागम _rpmalloc_allocate_large(heap, size);
  प्रत्यागम _rpmalloc_allocate_huge(heap, size);
}

स्थिर शून्य *_rpmalloc_aligned_allocate(heap_t *heap, size_t alignment,
                                        size_t size) {
  जइ (alignment <= SMALL_GRANULARITY)
    प्रत्यागम _rpmalloc_allocate(heap, size);

#जइ ENABLE_VALIDATE_ARGS
  जइ ((size + alignment) < size) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
  जइ (alignment & (alignment - 1)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#endif

  जइ ((alignment <= SPAN_HEADER_SIZE) &&
      ((size + SPAN_HEADER_SIZE) < _memory_medium_size_limit)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t multiple_size = size ? (size + (SPAN_HEADER_SIZE - 1)) &
                                      ~(uintptr_t)(SPAN_HEADER_SIZE - 1)
                                : SPAN_HEADER_SIZE;
    rpmalloc_assert(!(multiple_size % SPAN_HEADER_SIZE),
                    "Failed alignment calculation");
    जइ (multiple_size <= (size + alignment))
      प्रत्यागम _rpmalloc_allocate(heap, multiple_size);
  }

  शून्य *ptr = 0;
  size_t align_mask = alignment - 1;
  जइ (alignment <= _memory_page_size) {
    ptr = _rpmalloc_allocate(heap, size + alignment);
    जइ ((uintptr_t)ptr & align_mask) {
      ptr = (शून्य *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);
      // Mark as having aligned blocks
      span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
      span->flags |= SPAN_FLAG_ALIGNED_BLOCKS;
    }
    प्रत्यागम ptr;
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
  जइ (alignment & align_mask) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
  जइ (alignment >= _memory_span_size) {
    errno = EINVAL;
    प्रत्यागम 0;
  }

  size_t extra_pages = alignment / _memory_page_size;

  // Since each span has a header, we will at least need one extra memory page
  size_t अङ्कः_pages = 1 + (size / _memory_page_size);
  जइ (size & (_memory_page_size - 1))
    ++अङ्कः_pages;

  जइ (extra_pages > अङ्कः_pages)
    अङ्कः_pages = 1 + extra_pages;

  size_t original_pages = अङ्कः_pages;
  size_t limit_pages = (_memory_span_size / _memory_page_size) * 2;
  जइ (limit_pages < (original_pages * 2))
    limit_pages = original_pages * 2;

  size_t mapped_size, align_offset;
  span_t *span;

retry:
  align_offset = 0;
  mapped_size = अङ्कः_pages * _memory_page_size;

  span = (span_t *)_rpmalloc_mmap(mapped_size, &align_offset);
  जइ (!span) {
    errno = ENOMEM;
    प्रत्यागम 0;
  }
  ptr = pointer_offset(span, SPAN_HEADER_SIZE);

  जइ ((uintptr_t)ptr & align_mask)
    ptr = (शून्य *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);

  जइ (((size_t)pointer_diff(ptr, span) >= _memory_span_size) ||
      (pointer_offset(ptr, size) > pointer_offset(span, mapped_size)) ||
      (((uintptr_t)ptr & _memory_span_mask) != (uintptr_t)span)) {
    _rpmalloc_unmap(span, mapped_size, align_offset, mapped_size);
    ++अङ्कः_pages;
    जइ (अङ्कः_pages > limit_pages) {
      errno = EINVAL;
      प्रत्यागम 0;
    }
    गच्छ retry;
  }

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_गणनम् = (uint32_t)अङ्कः_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, अङ्कः_pages, _huge_pages_peak);

#जइ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_गणनम्;

  _rpmalloc_stat_add64(&_allocation_counter, 1);

  प्रत्यागम ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
स्थिर शून्य _rpmalloc_deallocate_direct_small_or_medium(span_t *span,
                                                        शून्य *block) {
  heap_t *heap = span->heap;
  rpmalloc_assert(heap->owner_thread == get_thread_id() ||
                      !heap->owner_thread || heap->finalize,
                  "Internal failure");
  // Add block to free list
  जइ (UNEXPECTED(_rpmalloc_span_is_fully_utilized(span))) {
    span->used_गणनम् = span->block_गणनम्;
#जइ RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                           span);
#endif
    _rpmalloc_span_double_link_list_add(
        &heap->size_class[span->size_class].partial_span, span);
    --heap->full_span_गणनम्;
  }
  *((शून्य **)block) = span->free_list;
  --span->used_गणनम्;
  span->free_list = block;
  जइ (UNEXPECTED(span->used_गणनम् == span->list_size)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    जइ (span->used_गणनम्) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      शून्य *free_list;
      करेहि {
        free_list = atomic_exchange_ptr_acquire(&span->free_list_deferred,
                                                INVALID_POINTER);
      } जाव (free_list == INVALID_POINTER);
      atomic_store_ptr_release(&span->free_list_deferred, free_list);
    }
    _rpmalloc_span_double_link_list_remove(
        &heap->size_class[span->size_class].partial_span, span);
    _rpmalloc_span_release_to_cache(heap, span);
  }
}

स्थिर शून्य _rpmalloc_deallocate_defer_free_span(heap_t *heap, span_t *span) {
  जइ (span->size_class != SIZE_CLASS_HUGE)
    _rpmalloc_stat_inc(&heap->span_use[span->span_गणनम् - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  करेहि {
    span->free_list = (शून्य *)atomic_load_ptr(&heap->span_free_deferred);
  } जाव (!atomic_cas_ptr(&heap->span_free_deferred, span, span->free_list));
}

//! Put the block in the deferred free list of the owning span
स्थिर शून्य _rpmalloc_deallocate_defer_small_or_medium(span_t *span,
                                                       शून्य *block) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  शून्य *free_list;
  करेहि {
    free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } जाव (free_list == INVALID_POINTER);
  *((शून्य **)block) = free_list;
  uint32_t free_गणनम् = ++span->list_size;
  पूर्णाङ्क all_deferred_free = (free_गणनम् == span->block_गणनम्);
  atomic_store_ptr_release(&span->free_list_deferred, block);
  जइ (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
  }
}

स्थिर शून्य _rpmalloc_deallocate_small_or_medium(span_t *span, शून्य *p) {
  _rpmalloc_stat_inc_free(span->heap, span->size_class);
  जइ (span->flags & SPAN_FLAG_ALIGNED_BLOCKS) {
    // Realign pointer to block start
    शून्य *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
    p = pointer_offset(p, -(int32_t)(block_offset % span->block_size));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#जइ RPMALLOC_FIRST_CLASS_HEAPS
  पूर्णाङ्क defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#अण्णहा
  पूर्णाङ्क defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  जइ (!defer)
    _rpmalloc_deallocate_direct_small_or_medium(span, p);
  अण्णहा
    _rpmalloc_deallocate_defer_small_or_medium(span, p);
}

//! Deallocate the given large memory block to the current heap
स्थिर शून्य _rpmalloc_deallocate_large(span_t *span) {
  rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#जइ RPMALLOC_FIRST_CLASS_HEAPS
  पूर्णाङ्क defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#अण्णहा
  पूर्णाङ्क defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  जइ (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    प्रत्यागम;
  }
  rpmalloc_assert(span->heap->full_span_गणनम्, "Heap span counter corrupted");
  --span->heap->full_span_गणनम्;
#जइ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif
#जइ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = span->span_गणनम् - 1;
  atomic_decr32(&span->heap->span_use[idx].current);
#endif
  heap_t *heap = span->heap;
  rpmalloc_assert(heap, "No thread heap");
#जइ ENABLE_THREAD_CACHE
  ध्रुव पूर्णाङ्क set_as_reserved =
      ((span->span_गणनम् > 1) && (heap->span_cache.गणनम् == 0) &&
       !heap->finalize && !heap->spans_reserved);
#अण्णहा
  ध्रुव पूर्णाङ्क set_as_reserved =
      ((span->span_गणनम् > 1) && !heap->finalize && !heap->spans_reserved);
#endif
  जइ (set_as_reserved) {
    heap->span_reserve = span;
    heap->spans_reserved = span->span_गणनम्;
    जइ (span->flags & SPAN_FLAG_MASTER) {
      heap->span_reserve_master = span;
    } अण्णहा { // SPAN_FLAG_SUBSPAN
      span_t *master = (span_t *)pointer_offset(
          span,
          -(intptr_t)((size_t)span->offset_from_master * _memory_span_size));
      heap->span_reserve_master = master;
      rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->remaining_spans) >=
                          (int32_t)span->span_गणनम्,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&heap->span_use[idx].spans_to_reserved);
  } अण्णहा {
    // Insert into cache list
    rpmalloc_heap_cache_निवेशय(heap, span);
  }
}

//! Deallocate the given huge span
स्थिर शून्य _rpmalloc_deallocate_huge(span_t *span) {
  rpmalloc_assert(span->heap, "No span heap");
#जइ RPMALLOC_FIRST_CLASS_HEAPS
  पूर्णाङ्क defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#अण्णहा
  पूर्णाङ्क defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  जइ (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    प्रत्यागम;
  }
  rpmalloc_assert(span->heap->full_span_गणनम्, "Heap span counter corrupted");
  --span->heap->full_span_गणनम्;
#जइ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif

  // Oversized allocation, page count is stored in span_count
  size_t अङ्कः_pages = span->span_गणनम्;
  _rpmalloc_unmap(span, अङ्कः_pages * _memory_page_size, span->align_offset,
                  अङ्कः_pages * _memory_page_size);
  _rpmalloc_stat_sub(&_huge_pages_current, अङ्कः_pages);
}

//! Deallocate the given block
स्थिर शून्य _rpmalloc_deallocate(शून्य *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  जइ (UNEXPECTED(!span))
    प्रत्यागम;
  जइ (EXPECTED(span->size_class < SIZE_CLASS_गणनम्))
    _rpmalloc_deallocate_small_or_medium(span, p);
  अण्णहा जइ (span->size_class == SIZE_CLASS_LARGE)
    _rpmalloc_deallocate_large(span);
  अण्णहा
    _rpmalloc_deallocate_huge(span);
}

////////////
///
/// Reallocation entry points
///
//////

स्थिर size_t _rpmalloc_usable_size(शून्य *p);

//! Reallocate the given block to the given size
स्थिर शून्य *_rpmalloc_reallocate(heap_t *heap, शून्य *p, size_t size,
                                  size_t oldsize, unsigned पूर्णाङ्क flags) {
  जइ (p) {
    // Grab the span using guaranteed span alignment
    span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
    जइ (EXPECTED(span->size_class < SIZE_CLASS_गणनम्)) {
      // Small/medium sized block
      rpmalloc_assert(span->span_गणनम् == 1, "Span counter corrupted");
      शून्य *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
      uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
      uint32_t block_idx = block_offset / span->block_size;
      शून्य *block =
          pointer_offset(blocks_start, (size_t)block_idx * span->block_size);
      जइ (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)span->block_size - pointer_diff(p, block));
      जइ ((size_t)span->block_size >= size) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        जइ ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        प्रत्यागम block;
      }
    } अण्णहा जइ (span->size_class == SIZE_CLASS_LARGE) {
      // Large block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t अङ्कः_spans = total_size >> _memory_span_size_shift;
      जइ (total_size & (_memory_span_mask - 1))
        ++अङ्कः_spans;
      size_t current_spans = span->span_गणनम्;
      शून्य *block = pointer_offset(span, SPAN_HEADER_SIZE);
      जइ (!oldsize)
        oldsize = (current_spans * _memory_span_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      जइ ((current_spans >= अङ्कः_spans) && (total_size >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        जइ ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        प्रत्यागम block;
      }
    } अण्णहा {
      // Oversized block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t अङ्कः_pages = total_size >> _memory_page_size_shift;
      जइ (total_size & (_memory_page_size - 1))
        ++अङ्कः_pages;
      // Page count is stored in span_count
      size_t current_pages = span->span_गणनम्;
      शून्य *block = pointer_offset(span, SPAN_HEADER_SIZE);
      जइ (!oldsize)
        oldsize = (current_pages * _memory_page_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      जइ ((current_pages >= अङ्कः_pages) && (अङ्कः_pages >= (current_pages / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        जइ ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        प्रत्यागम block;
      }
    }
  } अण्णहा {
    oldsize = 0;
  }

  जइ (!!(flags & RPMALLOC_GROW_OR_FAIL))
    प्रत्यागम 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t lower_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_size =
      (size > lower_bound) ? size : ((size > oldsize) ? lower_bound : size);
  शून्य *block = _rpmalloc_allocate(heap, new_size);
  जइ (p && block) {
    जइ (!(flags & RPMALLOC_NO_PRESERVE))
      memcpy(block, p, oldsize < new_size ? oldsize : new_size);
    _rpmalloc_deallocate(p);
  }

  प्रत्यागम block;
}

स्थिर शून्य *_rpmalloc_aligned_reallocate(heap_t *heap, शून्य *ptr,
                                          size_t alignment, size_t size,
                                          size_t oldsize, unsigned पूर्णाङ्क flags) {
  जइ (alignment <= SMALL_GRANULARITY)
    प्रत्यागम _rpmalloc_reallocate(heap, ptr, size, oldsize, flags);

  पूर्णाङ्क no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? _rpmalloc_usable_size(ptr) : 0);
  जइ ((usablesize >= size) && !((uintptr_t)ptr & (alignment - 1))) {
    जइ (no_alloc || (size >= (usablesize / 2)))
      प्रत्यागम ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  शून्य *block =
      (!no_alloc ? _rpmalloc_aligned_allocate(heap, alignment, size) : 0);
  जइ (EXPECTED(block != 0)) {
    जइ (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      जइ (!oldsize)
        oldsize = usablesize;
      memcpy(block, ptr, oldsize < size ? oldsize : size);
    }
    _rpmalloc_deallocate(ptr);
  }
  प्रत्यागम block;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
स्थिर size_t _rpmalloc_usable_size(शून्य *p) {
  // Grab the span using guaranteed span alignment
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  जइ (span->size_class < SIZE_CLASS_गणनम्) {
    // Small/medium block
    शून्य *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    प्रत्यागम span->block_size -
           ((size_t)pointer_diff(p, blocks_start) % span->block_size);
  }
  जइ (span->size_class == SIZE_CLASS_LARGE) {
    // Large block
    size_t current_spans = span->span_गणनम्;
    प्रत्यागम (current_spans * _memory_span_size) - (size_t)pointer_diff(p, span);
  }
  // Oversized block, page count is stored in span_count
  size_t current_pages = span->span_गणनम्;
  प्रत्यागम (current_pages * _memory_page_size) - (size_t)pointer_diff(p, span);
}

//! Adjust and optimize the size class properties for the given class
स्थिर शून्य _rpmalloc_adjust_size_class(size_t iclass) {
  size_t block_size = _memory_size_class[iclass].block_size;
  size_t block_गणनम् = (_memory_span_size - SPAN_HEADER_SIZE) / block_size;

  _memory_size_class[iclass].block_गणनम् = (uint16_t)block_गणनम्;
  _memory_size_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  जइ (iclass >= SMALL_CLASS_गणनम्) {
    size_t prevclass = iclass;
    जाव (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      जइ (_memory_size_class[prevclass].block_गणनम् ==
          _memory_size_class[iclass].block_गणनम्)
        _rpmalloc_memcpy_const(_memory_size_class + prevclass,
                               _memory_size_class + iclass,
                               परिमाण(_memory_size_class[iclass]));
      अण्णहा
        भङ्ग;
    }
  }
}

//! Initialize the allocator and setup global data
बाह्य अन्तःस्थ पूर्णाङ्क rpmalloc_initialize(शून्य) {
  जइ (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    प्रत्यागम 0;
  }
  प्रत्यागम rpmalloc_initialize_config(0);
}

पूर्णाङ्क rpmalloc_initialize_config(ध्रुव rpmalloc_config_t *config) {
  जइ (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    प्रत्यागम 0;
  }
  _rpmalloc_initialized = 1;

  जइ (config)
    memcpy(&_memory_config, config, परिमाण(rpmalloc_config_t));
  अण्णहा
    _rpmalloc_memset_const(&_memory_config, 0, परिमाण(rpmalloc_config_t));

  जइ (!_memory_config.memory_map || !_memory_config.memory_unmap) {
    _memory_config.memory_map = _rpmalloc_mmap_os;
    _memory_config.memory_unmap = _rpmalloc_unmap_os;
  }

#जइ PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, परिमाण(system_info));
  GetSystemInfo(&system_info);
  _memory_map_granularity = system_info.dwAllocationGranularity;
#अण्णहा
  _memory_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#endif

#जइ RPMALLOC_CONFIGURABLE
  _memory_page_size = _memory_config.page_size;
#अण्णहा
  _memory_page_size = 0;
#endif
  _memory_huge_pages = 0;
  जइ (!_memory_page_size) {
#जइ PLATFORM_WINDOWS
    _memory_page_size = system_info.dwPageSize;
#अण्णहा
    _memory_page_size = _memory_map_granularity;
    जइ (_memory_config.enable_huge_pages) {
#जइ defined(__linux__)
      size_t huge_page_size = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      जइ (meminfo) {
        वर्ण पङ्क्तिः_2[128];
        जाव (!huge_page_size && fgets(पङ्क्तिः_2, परिमाण(पङ्क्तिः_2) - 1, meminfo)) {
          पङ्क्तिः_2[परिमाण(पङ्क्तिः_2) - 1] = 0;
          जइ (strstr(पङ्क्तिः_2, "Hugepagesize:"))
            huge_page_size = (size_t)strtol(पङ्क्तिः_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      जइ (huge_page_size) {
        _memory_huge_pages = 1;
        _memory_page_size = huge_page_size;
        _memory_map_granularity = huge_page_size;
      }
#नोचेद्यदि defined(__FreeBSD__)
      पूर्णाङ्क rc;
      size_t sz = परिमाण(rc);

      जइ (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, रिक्त, 0) == 0 &&
          rc == 1) {
        स्थिर size_t defsize = 2 * 1024 * 1024;
        पूर्णाङ्क nsize = 0;
        size_t sizes[4] = {0};
        _memory_huge_pages = 1;
        _memory_page_size = defsize;
        जइ ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          प्रति (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            जइ (defsize < csize) {
              _memory_page_size = csize;
              भङ्ग;
            }
          }
        }
        _memory_map_granularity = _memory_page_size;
      }
#नोचेद्यदि defined(__APPLE__) || defined(__NetBSD__)
      _memory_huge_pages = 1;
      _memory_page_size = 2 * 1024 * 1024;
      _memory_map_granularity = _memory_page_size;
#endif
    }
#endif
  } अण्णहा {
    जइ (_memory_config.enable_huge_pages)
      _memory_huge_pages = 1;
  }

#जइ PLATFORM_WINDOWS
  जइ (_memory_config.enable_huge_pages) {
    HANDLE token = 0;
    size_t large_page_minimum = GetLargePageMinimum();
    जइ (large_page_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    जइ (token) {
      LUID luid;
      जइ (Lookup_Privilege_मानम्(0, SE_LOCK_MEMORY_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, परिमाण(token_privileges));
        token_privileges.Privilege_गणनम् = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        जइ (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          जइ (GetLastError() == ERROR_SUCCESS)
            _memory_huge_pages = 1;
        }
      }
      CloseHandle(token);
    }
    जइ (_memory_huge_pages) {
      जइ (large_page_minimum > _memory_page_size)
        _memory_page_size = large_page_minimum;
      जइ (large_page_minimum > _memory_map_granularity)
        _memory_map_granularity = large_page_minimum;
    }
  }
#endif

  size_t min_span_size = 256;
  size_t max_page_size;
#जइ UINTPTR_MAX > 0xFFFFFFFF
  max_page_size = 4096ULL * 1024ULL * 1024ULL;
#अण्णहा
  max_page_size = 4 * 1024 * 1024;
#endif
  जइ (_memory_page_size < min_span_size)
    _memory_page_size = min_span_size;
  जइ (_memory_page_size > max_page_size)
    _memory_page_size = max_page_size;
  _memory_page_size_shift = 0;
  size_t page_size_bit = _memory_page_size;
  जाव (page_size_bit != 1) {
    ++_memory_page_size_shift;
    page_size_bit >>= 1;
  }
  _memory_page_size = ((size_t)1 << _memory_page_size_shift);

#जइ RPMALLOC_CONFIGURABLE
  जइ (!_memory_config.span_size) {
    _memory_span_size = _memory_default_span_size;
    _memory_span_size_shift = _memory_default_span_size_shift;
    _memory_span_mask = _memory_default_span_mask;
  } अण्णहा {
    size_t span_size = _memory_config.span_size;
    जइ (span_size > (256 * 1024))
      span_size = (256 * 1024);
    _memory_span_size = 4096;
    _memory_span_size_shift = 12;
    जाव (_memory_span_size < span_size) {
      _memory_span_size <<= 1;
      ++_memory_span_size_shift;
    }
    _memory_span_mask = ~(uintptr_t)(_memory_span_size - 1);
  }
#endif

  memory_span_map_गणनम् =
      (_memory_config.span_map_गणनम् ? _memory_config.span_map_गणनम्
                                     : DEFAULT_SPAN_MAP_गणनम्);
  जइ ((_memory_span_size * memory_span_map_गणनम्) < _memory_page_size)
    memory_span_map_गणनम् = (_memory_page_size / _memory_span_size);
  जइ ((_memory_page_size >= _memory_span_size) &&
      ((memory_span_map_गणनम् * _memory_span_size) % _memory_page_size))
    memory_span_map_गणनम् = (_memory_page_size / _memory_span_size);
  memory_heap_reserve_गणनम् = (memory_span_map_गणनम् > DEFAULT_SPAN_MAP_गणनम्)
                                   ? DEFAULT_SPAN_MAP_गणनम्
                                   : memory_span_map_गणनम्;

  _memory_config.page_size = _memory_page_size;
  _memory_config.span_size = _memory_span_size;
  _memory_config.span_map_गणनम् = memory_span_map_गणनम्;
  _memory_config.enable_huge_pages = _memory_huge_pages;

#जइ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  जइ (pthread_कुञ्चिका_create(&_memory_thread_heap, _rpmalloc_heap_release_raw_fc))
    प्रत्यागम -1;
#endif
#जइ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_कुञ्चिका = FlsAlloc(&_rpmalloc_thread_destructor);
#endif

  // Setup all small and medium size classes
  size_t iclass = 0;
  _memory_size_class[iclass].block_size = SMALL_GRANULARITY;
  _rpmalloc_adjust_size_class(iclass);
  प्रति (iclass = 1; iclass < SMALL_CLASS_गणनम्; ++iclass) {
    size_t size = iclass * SMALL_GRANULARITY;
    _memory_size_class[iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  _memory_medium_size_limit = (_memory_span_size - SPAN_HEADER_SIZE) >> 1;
  जइ (_memory_medium_size_limit > MEDIUM_SIZE_LIMIT)
    _memory_medium_size_limit = MEDIUM_SIZE_LIMIT;
  प्रति (iclass = 0; iclass < MEDIUM_CLASS_गणनम्; ++iclass) {
    size_t size = SMALL_SIZE_LIMIT + ((iclass + 1) * MEDIUM_GRANULARITY);
    जइ (size > _memory_medium_size_limit) {
      _memory_medium_size_limit =
          SMALL_SIZE_LIMIT + (iclass * MEDIUM_GRANULARITY);
      भङ्ग;
    }
    _memory_size_class[SMALL_CLASS_गणनम् + iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(SMALL_CLASS_गणनम् + iclass);
  }

  _memory_orphan_heaps = 0;
#जइ RPMALLOC_FIRST_CLASS_HEAPS
  _memory_first_class_orphan_heaps = 0;
#endif
#जइ ENABLE_STATISTICS
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
  memset(_memory_heaps, 0, परिमाण(_memory_heaps));
  atomic_store32_release(&_memory_global_lock, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  प्रत्यागम 0;
}

//! Finalize the allocator
शून्य rpmalloc_finalize(शून्य) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  जइ (_memory_global_reserve) {
    atomic_add32(&_memory_global_reserve_master->remaining_spans,
                 -(int32_t)memory_global_reserve_गणनम्);
    _memory_global_reserve_master = 0;
    memory_global_reserve_गणनम् = 0;
    _memory_global_reserve = 0;
  }
  atomic_store32_release(&_memory_global_lock, 0);

  // Free all thread caches and fully free spans
  प्रति (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    जाव (heap) {
      heap_t *next_heap = heap->next_heap;
      heap->finalize = 1;
      _rpmalloc_heap_global_finalize(heap);
      heap = next_heap;
    }
  }

#जइ ENABLE_GLOBAL_CACHE
  // Free global caches
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass)
    _rpmalloc_global_cache_finalize(&_memory_span_cache[iclass]);
#endif

#जइ (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_कुञ्चिका_delete(_memory_thread_heap);
#endif
#जइ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_कुञ्चिका);
  fls_कुञ्चिका = 0;
#endif
#जइ ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&_mapped_pages) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&_mapped_pages_os) == 0,
                  "Memory leak detected");
#endif

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
बाह्य अन्तःस्थ शून्य rpmalloc_thread_initialize(शून्य) {
  जइ (!get_thread_heap_raw()) {
    heap_t *heap = _rpmalloc_heap_allocate(0);
    जइ (heap) {
      _rpmalloc_stat_inc(&_memory_active_heaps);
      set_thread_heap(heap);
#जइ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_Set_मानम्(fls_कुञ्चिका, heap);
#endif
    }
  }
}

//! Finalize thread, orphan heap
शून्य rpmalloc_thread_finalize(पूर्णाङ्क release_caches) {
  heap_t *heap = get_thread_heap_raw();
  जइ (heap)
    _rpmalloc_heap_release_raw(heap, release_caches);
  set_thread_heap(0);
#जइ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_Set_मानम्(fls_कुञ्चिका, 0);
#endif
}

पूर्णाङ्क rpmalloc_is_thread_initialized(शून्य) {
  प्रत्यागम (get_thread_heap_raw() != 0) ? 1 : 0;
}

ध्रुव rpmalloc_config_t *rpmalloc_config(शून्य) { प्रत्यागम &_memory_config; }

// Extern interface

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *rpmalloc(size_t size) {
#जइ ENABLE_VALIDATE_ARGS
  जइ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  प्रत्यागम _rpmalloc_allocate(heap, size);
}

बाह्य अन्तःस्थ शून्य rpfree(शून्य *ptr) { _rpmalloc_deallocate(ptr); }

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *rpcalloc(size_t अङ्कः_2, size_t size) {
  size_t total;
#जइ ENABLE_VALIDATE_ARGS
#जइ PLATFORM_WINDOWS
  पूर्णाङ्क err = SizeTMult(अङ्कः_2, size, &total);
  जइ ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#अण्णहा
  पूर्णाङ्क err = __builtin_umull_overflow(अङ्कः_2, size, &total);
  जइ (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#endif
#अण्णहा
  total = अङ्कः_2 * size;
#endif
  heap_t *heap = get_thread_heap();
  शून्य *block = _rpmalloc_allocate(heap, total);
  जइ (block)
    memset(block, 0, total);
  प्रत्यागम block;
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *rprealloc(शून्य *ptr, size_t size) {
#जइ ENABLE_VALIDATE_ARGS
  जइ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    प्रत्यागम ptr;
  }
#endif
  heap_t *heap = get_thread_heap();
  प्रत्यागम _rpmalloc_reallocate(heap, ptr, size, 0, 0);
}

बाह्य RPMALLOC_ALLOCATOR शून्य *rpaligned_realloc(शून्य *ptr, size_t alignment,
                                                  size_t size, size_t oldsize,
                                                  unsigned पूर्णाङ्क flags) {
#जइ ENABLE_VALIDATE_ARGS
  जइ ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  प्रत्यागम _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, oldsize,
                                      flags);
}

बाह्य RPMALLOC_ALLOCATOR शून्य *rpaligned_alloc(size_t alignment, size_t size) {
  heap_t *heap = get_thread_heap();
  प्रत्यागम _rpmalloc_aligned_allocate(heap, alignment, size);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpaligned_calloc(size_t alignment, size_t अङ्कः_2, size_t size) {
  size_t total;
#जइ ENABLE_VALIDATE_ARGS
#जइ PLATFORM_WINDOWS
  पूर्णाङ्क err = SizeTMult(अङ्कः_2, size, &total);
  जइ ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#अण्णहा
  पूर्णाङ्क err = __builtin_umull_overflow(अङ्कः_2, size, &total);
  जइ (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#endif
#अण्णहा
  total = अङ्कः_2 * size;
#endif
  शून्य *block = rpaligned_alloc(alignment, total);
  जइ (block)
    memset(block, 0, total);
  प्रत्यागम block;
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *rpmemalign(size_t alignment,
                                                  size_t size) {
  प्रत्यागम rpaligned_alloc(alignment, size);
}

बाह्य अन्तःस्थ पूर्णाङ्क rpposix_memalign(शून्य **memptr, size_t alignment,
                                   size_t size) {
  जइ (memptr)
    *memptr = rpaligned_alloc(alignment, size);
  अण्णहा
    प्रत्यागम EINVAL;
  प्रत्यागम *memptr ? 0 : ENOMEM;
}

बाह्य अन्तःस्थ size_t rpmalloc_usable_size(शून्य *ptr) {
  प्रत्यागम (ptr ? _rpmalloc_usable_size(ptr) : 0);
}

बाह्य अन्तःस्थ शून्य rpmalloc_thread_collect(शून्य) {}

शून्य rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, परिमाण(rpmalloc_thread_statistics_t));
  heap_t *heap = get_thread_heap_raw();
  जइ (!heap)
    प्रत्यागम;

  प्रति (size_t iclass = 0; iclass < SIZE_CLASS_गणनम्; ++iclass) {
    size_class_t *size_class = _memory_size_class + iclass;
    span_t *span = heap->size_class[iclass].partial_span;
    जाव (span) {
      size_t free_गणनम् = span->list_size;
      size_t block_गणनम् = size_class->block_गणनम्;
      जइ (span->free_list_limit < block_गणनम्)
        block_गणनम् = span->free_list_limit;
      free_गणनम् += (block_गणनम् - span->used_गणनम्);
      stats->sizecache += free_गणनम् * size_class->block_size;
      span = span->next;
    }
  }

#जइ ENABLE_THREAD_CACHE
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    span_cache_t *span_cache;
    जइ (!iclass)
      span_cache = &heap->span_cache;
    अण्णहा
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    stats->spancache += span_cache->गणनम् * (iclass + 1) * _memory_span_size;
  }
#endif

  span_t *deferred = (span_t *)atomic_load_ptr(&heap->span_free_deferred);
  जाव (deferred) {
    जइ (deferred->size_class != SIZE_CLASS_HUGE)
      stats->spancache += (size_t)deferred->span_गणनम् * _memory_span_size;
    deferred = (span_t *)deferred->free_list;
  }

#जइ ENABLE_STATISTICS
  stats->thread_to_global = (size_t)atomic_load64(&heap->thread_to_global);
  stats->global_to_thread = (size_t)atomic_load64(&heap->global_to_thread);

  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    stats->span_use[iclass].current =
        (size_t)atomic_load32(&heap->span_use[iclass].current);
    stats->span_use[iclass].peak =
        (size_t)atomic_load32(&heap->span_use[iclass].उच्चः);
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
  प्रति (size_t iclass = 0; iclass < SIZE_CLASS_गणनम्; ++iclass) {
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

शून्य rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, परिमाण(rpmalloc_global_statistics_t));
#जइ ENABLE_STATISTICS
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
#जइ ENABLE_GLOBAL_CACHE
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    global_cache_t *cache = &_memory_span_cache[iclass];
    जाव (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();
    uint32_t गणनम् = cache->गणनम्;
#जइ ENABLE_UNLIMITED_CACHE
    span_t *current_span = cache->overflow;
    जाव (current_span) {
      ++गणनम्;
      current_span = current_span->next;
    }
#endif
    atomic_store32_release(&cache->lock, 0);
    stats->cached += गणनम् * (iclass + 1) * _memory_span_size;
  }
#endif
}

#जइ ENABLE_STATISTICS

स्थिर शून्य _memory_heap_dump_statistics(heap_t *heap, शून्य *file) {
  fprintf(file, "Heap %d stats:\n", heap->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  प्रति (size_t iclass = 0; iclass < SIZE_CLASS_गणनम्; ++iclass) {
    जइ (!atomic_load32(&heap->size_class_use[iclass].alloc_total))
      अनुवर्त;
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
        _memory_size_class[iclass].block_गणनम्,
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
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    जइ (!atomic_load32(&heap->span_use[iclass].उच्चः) &&
        !atomic_load32(&heap->span_use[iclass].spans_map_calls))
      अनुवर्त;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&heap->span_use[iclass].current),
        atomic_load32(&heap->span_use[iclass].उच्चः),
        atomic_load32(&heap->span_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&heap->span_use[iclass].उच्चः) *
         (size_t)_memory_span_size * (iclass + 1)) /
            (size_t)(1024 * 1024),
#जइ ENABLE_THREAD_CACHE
        (unsigned पूर्णाङ्क)(!iclass ? heap->span_cache.गणनम्
                               : heap->span_large_cache[iclass - 1].गणनम्),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_to_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_from_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
#अण्णहा
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
  fprintf(file, "Full spans: %zu\n", heap->full_span_गणनम्);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&heap->thread_to_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&heap->global_to_thread) / (size_t)(1024 * 1024));
}

#endif

शून्य rpmalloc_dump_statistics(शून्य *file) {
#जइ ENABLE_STATISTICS
  प्रति (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    जाव (heap) {
      पूर्णाङ्क need_dump = 0;
      प्रति (size_t iclass = 0; !need_dump && (iclass < SIZE_CLASS_गणनम्);
           ++iclass) {
        जइ (!atomic_load32(&heap->size_class_use[iclass].alloc_total)) {
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].free_total),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          अनुवर्त;
        }
        need_dump = 1;
      }
      प्रति (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_गणनम्);
           ++iclass) {
        जइ (!atomic_load32(&heap->span_use[iclass].उच्चः) &&
            !atomic_load32(&heap->span_use[iclass].spans_map_calls))
          अनुवर्त;
        need_dump = 1;
      }
      जइ (need_dump)
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

#जइ ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    global_cache_t *cache = _memory_span_cache + iclass;
    size_t global_cache = (size_t)cache->गणनम् * iclass * _memory_span_size;

    size_t global_overflow_cache = 0;
    span_t *span = cache->overflow;
    जाव (span) {
      global_overflow_cache += iclass * _memory_span_size;
      span = span->next;
    }
    जइ (global_cache || global_overflow_cache || cache->निवेशय_गणनम् ||
        cache->extract_गणनम्)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_overflow_cache / (size_t)(1024 * 1024),
              cache->निवेशय_गणनम्, cache->extract_गणनम्);
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
#जइ 0
	int64_t allocated = atomic_load64(&_allocation_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#endif
#endif
  (शून्य)परिमाण(file);
}

#जइ RPMALLOC_FIRST_CLASS_HEAPS

बाह्य अन्तःस्थ rpmalloc_heap_t *rpmalloc_heap_acquire(शून्य) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  heap_t *heap = _rpmalloc_heap_allocate(1);
  rpmalloc_assume(heap != रिक्त);
  heap->owner_thread = 0;
  _rpmalloc_stat_inc(&_memory_active_heaps);
  प्रत्यागम heap;
}

बाह्य अन्तःस्थ शून्य rpmalloc_heap_release(rpmalloc_heap_t *heap) {
  जइ (heap)
    _rpmalloc_heap_release(heap, 1, 1);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, size_t size) {
#जइ ENABLE_VALIDATE_ARGS
  जइ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#endif
  प्रत्यागम _rpmalloc_allocate(heap, size);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, size_t alignment,
                            size_t size) {
#जइ ENABLE_VALIDATE_ARGS
  जइ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#endif
  प्रत्यागम _rpmalloc_aligned_allocate(heap, alignment, size);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, size_t अङ्कः_2, size_t size) {
  प्रत्यागम rpmalloc_heap_aligned_calloc(heap, 0, अङ्कः_2, size);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, size_t alignment,
                             size_t अङ्कः_2, size_t size) {
  size_t total;
#जइ ENABLE_VALIDATE_ARGS
#जइ PLATFORM_WINDOWS
  पूर्णाङ्क err = SizeTMult(अङ्कः_2, size, &total);
  जइ ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#अण्णहा
  पूर्णाङ्क err = __builtin_umull_overflow(अङ्कः_2, size, &total);
  जइ (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#endif
#अण्णहा
  total = अङ्कः_2 * size;
#endif
  शून्य *block = _rpmalloc_aligned_allocate(heap, alignment, total);
  जइ (block)
    memset(block, 0, total);
  प्रत्यागम block;
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, शून्य *ptr, size_t size,
                      unsigned पूर्णाङ्क flags) {
#जइ ENABLE_VALIDATE_ARGS
  जइ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    प्रत्यागम ptr;
  }
#endif
  प्रत्यागम _rpmalloc_reallocate(heap, ptr, size, 0, flags);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_aligned_realloc(rpmalloc_heap_t *heap, शून्य *ptr,
                              size_t alignment, size_t size,
                              unsigned पूर्णाङ्क flags) {
#जइ ENABLE_VALIDATE_ARGS
  जइ ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#endif
  प्रत्यागम _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, 0, flags);
}

बाह्य अन्तःस्थ शून्य rpmalloc_heap_free(rpmalloc_heap_t *heap, शून्य *ptr) {
  (शून्य)परिमाण(heap);
  _rpmalloc_deallocate(ptr);
}

बाह्य अन्तःस्थ शून्य rpmalloc_heap_free_all(rpmalloc_heap_t *heap) {
  span_t *span;
  span_t *next_span;

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  प्रति (size_t iclass = 0; iclass < SIZE_CLASS_गणनम्; ++iclass) {
    span = heap->size_class[iclass].partial_span;
    जाव (span) {
      next_span = span->next;
      rpmalloc_heap_cache_निवेशय(heap, span);
      span = next_span;
    }
    heap->size_class[iclass].partial_span = 0;
    span = heap->full_span[iclass];
    जाव (span) {
      next_span = span->next;
      rpmalloc_heap_cache_निवेशय(heap, span);
      span = next_span;
    }

    span = heap->size_class[iclass].cache;
    जइ (span)
      rpmalloc_heap_cache_निवेशय(heap, span);
    heap->size_class[iclass].cache = 0;
  }
  memset(heap->size_class, 0, परिमाण(heap->size_class));
  memset(heap->full_span, 0, परिमाण(heap->full_span));

  span = heap->large_huge_span;
  जाव (span) {
    next_span = span->next;
    जइ (UNEXPECTED(span->size_class == SIZE_CLASS_HUGE))
      _rpmalloc_deallocate_huge(span);
    अण्णहा
      rpmalloc_heap_cache_निवेशय(heap, span);
    span = next_span;
  }
  heap->large_huge_span = 0;
  heap->full_span_गणनम् = 0;

#जइ ENABLE_THREAD_CACHE
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    span_cache_t *span_cache;
    जइ (!iclass)
      span_cache = &heap->span_cache;
    अण्णहा
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    जइ (!span_cache->गणनम्)
      अनुवर्त;
#जइ ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&heap->thread_to_global,
                         span_cache->गणनम् * (iclass + 1) * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                       span_cache->गणनम्);
    rpmalloc_global_cache_निवेशय_spans(span_cache->span, iclass + 1,
                                        span_cache->गणनम्);
#अण्णहा
    प्रति (size_t ispan = 0; ispan < span_cache->गणनम्; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
    span_cache->गणनम् = 0;
  }
#endif

#जइ ENABLE_STATISTICS
  प्रति (size_t iclass = 0; iclass < SIZE_CLASS_गणनम्; ++iclass) {
    atomic_store32(&heap->size_class_use[iclass].alloc_current, 0);
    atomic_store32(&heap->size_class_use[iclass].spans_current, 0);
  }
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    atomic_store32(&heap->span_use[iclass].current, 0);
  }
#endif
}

बाह्य अन्तःस्थ शून्य rpmalloc_heap_thread_set_current(rpmalloc_heap_t *heap) {
  heap_t *prev_heap = get_thread_heap_raw();
  जइ (prev_heap != heap) {
    set_thread_heap(heap);
    जइ (prev_heap)
      rpmalloc_heap_release(prev_heap);
  }
}

बाह्य अन्तःस्थ rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(शून्य *ptr) {
  // Grab the span, and then the heap from the span
  span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
  जइ (span) {
    प्रत्यागम span->heap;
  }
  प्रत्यागम 0;
}

#endif

#जइ ENABLE_PRELOAD || ENABLE_OVERRIDE

#अन्तर्भाव "malloc.c"

#endif

शून्य rpmalloc_linker_reference(शून्य) { (शून्य)परिमाण(_rpmalloc_initialized); }
