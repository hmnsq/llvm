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

#यदि defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wunused-function"
#यदि __has_warning("-Wreserved-identifier")
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
#यदि __has_warning("-Wstatic-in-inline")
#pragma clang diagnostic ignored "-Wstatic-in-inline"
#endif
#नोचेद्यदि defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#यदि !defined(__has_builtin)
#define __has_builtin(b) 0
#endif

#यदि defined(__GNUC__) || defined(__clang__)

#यदि __has_builtin(__builtin_memcpy_inline)
#define _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#अन्यथा
#define _rpmalloc_memcpy_const(x, y, s)                                        \
  कुरु {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    स_नकल(x, y, s);                                                           \
  } यावत् (0)
#endif

#यदि __has_builtin(__builtin_memset_inline)
#define _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#अन्यथा
#define _rpmalloc_memset_const(x, y, s)                                        \
  कुरु {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    स_रखो(x, y, s);                                                           \
  } यावत् (0)
#endif
#अन्यथा
#define _rpmalloc_memcpy_const(x, y, s) स_नकल(x, y, s)
#define _rpmalloc_memset_const(x, y, s) स_रखो(x, y, s)
#endif

#यदि __has_builtin(__builtin_assume)
#define rpmalloc_assume(cond) __builtin_assume(cond)
#नोचेद्यदि defined(__GNUC__)
#define rpmalloc_assume(cond)                                                  \
  कुरु {                                                                         \
    यदि (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } यावत् (0)
#नोचेद्यदि defined(_MSC_VER)
#define rpmalloc_assume(cond) __assume(cond)
#अन्यथा
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
#ifndef DEFAULT_SPAN_MAP_गिनती
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#define DEFAULT_SPAN_MAP_गिनती 64
#endif
#ifndef GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#define GLOBAL_CACHE_MULTIPLIER 8
#endif

#यदि DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#error Must use global cache यदि unmap is disabled
#endif

#यदि DISABLE_UNMAP
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 1
#endif

#यदि !ENABLE_GLOBAL_CACHE
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 0
#endif

#यदि !ENABLE_THREAD_CACHE
#undef ENABLE_ADAPTIVE_THREAD_CACHE
#define ENABLE_ADAPTIVE_THREAD_CACHE 0
#endif

#यदि defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#define PLATFORM_POSIX 0
#अन्यथा
#define PLATFORM_WINDOWS 0
#define PLATFORM_POSIX 1
#endif

/// Platform and arch specifics
#यदि defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 5105)
#ifndef FORCEINLINE
#define FORCEINLINE अन्तःस्थ __forceinline
#endif
#define _Static_assert static_assert
#अन्यथा
#ifndef FORCEINLINE
#define FORCEINLINE अन्तःस्थ __attribute__((__always_inline__))
#endif
#endif
#यदि PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#अन्तर्भाव <windows.h>
#यदि ENABLE_VALIDATE_ARGS
#अन्तर्भाव <intsafe.h>
#endif
#अन्यथा
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>
#यदि defined(__linux__) || defined(__ANDROID__)
#अन्तर्भाव <sys/prctl.h>
#यदि !defined(PR_SET_VMA)
#define PR_SET_VMA 0x53564d41
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif
#यदि defined(__APPLE__)
#अन्तर्भाव <TargetConditionals.h>
#यदि !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#अन्तर्भाव <mach/mach_vm.h>
#अन्तर्भाव <mach/vm_statistics.h>
#endif
#अन्तर्भाव <pthread.h>
#endif
#यदि defined(__HAIKU__) || defined(__TINYC__)
#अन्तर्भाव <pthread.h>
#endif
#endif

#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <string.h>

#यदि defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#अन्तर्भाव <fibersapi.h>
स्थिर DWORD fls_कुंजी;
#endif

#यदि PLATFORM_POSIX
#अन्तर्भाव <sched.h>
#अन्तर्भाव <sys/mman.h>
#ifdef __FreeBSD__
#अन्तर्भाव <sys/sysctl.h>
#define MAP_HUGETLB MAP_ALIGNED_SUPER
#ifndef PROT_MAX
#define PROT_MAX(f) 0
#endif
#अन्यथा
#define PROT_MAX(f) 0
#endif
#ifdef __sun
बाह्य पूर्णाङ्क madvise(caddr_t, माप_प्रकार, पूर्णाङ्क);
#endif
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif
#endif
#अन्तर्भाव <errno.h>

#यदि ENABLE_ASSERTS
#undef न_संशोधन
#यदि defined(_MSC_VER) && !defined(_DEBUG)
#define _DEBUG
#endif
#अन्तर्भाव <assert.h>
#define RPMALLOC_TOSTRING_M(x) #x
#define RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#define rpmalloc_assert(truth, message)                                        \
  कुरु {                                                                         \
    यदि (!(truth)) {                                                            \
      यदि (_memory_config.error_callback) {                                     \
        _memory_config.error_callback(message " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } अन्यथा {                                                                 \
        प्रतिज्ञा((truth) && message);                                            \
      }                                                                        \
    }                                                                          \
  } यावत् (0)
#अन्यथा
#define rpmalloc_assert(truth, message)                                        \
  कुरु {                                                                         \
  } यावत् (0)
#endif
#यदि ENABLE_STATISTICS
#अन्तर्भाव <stdio.h>
#endif

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#यदि defined(_MSC_VER) && !defined(__clang__)

प्रकारनिर्देश चञ्चल दीर्घ atomic32_t;
प्रकारनिर्देश चञ्चल दीर्घ दीर्घ atomic64_t;
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

#अन्यथा

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

#यदि ENABLE_STATISTICS
#define _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#define _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#define _rpmalloc_stat_add(counter, मूल्य_2)                                     \
  atomic_add32(counter, (int32_t)(मूल्य_2))
#define _rpmalloc_stat_add64(counter, मूल्य_2)                                   \
  atomic_add64(counter, (int64_t)(मूल्य_2))
#define _rpmalloc_stat_add_peak(counter, मूल्य_2, peak)                          \
  कुरु {                                                                         \
    int32_t cur_गिनती = atomic_add32(counter, (int32_t)(मूल्य_2));              \
    यदि (cur_गिनती > (peak))                                                   \
      peak = cur_गिनती;                                                       \
  } यावत् (0)
#define _rpmalloc_stat_sub(counter, मूल्य_2)                                     \
  atomic_add32(counter, -(int32_t)(मूल्य_2))
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  कुरु {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&heap->size_class_use[class_idx].alloc_current);         \
    यदि (alloc_current > heap->size_class_use[class_idx].alloc_peak)            \
      heap->size_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&heap->size_class_use[class_idx].alloc_total);               \
  } यावत् (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  कुरु {                                                                         \
    atomic_decr32(&heap->size_class_use[class_idx].alloc_current);             \
    atomic_incr32(&heap->size_class_use[class_idx].free_total);                \
  } यावत् (0)
#अन्यथा
#define _rpmalloc_stat_inc(counter)                                            \
  कुरु {                                                                         \
  } यावत् (0)
#define _rpmalloc_stat_dec(counter)                                            \
  कुरु {                                                                         \
  } यावत् (0)
#define _rpmalloc_stat_add(counter, मूल्य_2)                                     \
  कुरु {                                                                         \
  } यावत् (0)
#define _rpmalloc_stat_add64(counter, मूल्य_2)                                   \
  कुरु {                                                                         \
  } यावत् (0)
#define _rpmalloc_stat_add_peak(counter, मूल्य_2, peak)                          \
  कुरु {                                                                         \
  } यावत् (0)
#define _rpmalloc_stat_sub(counter, मूल्य_2)                                     \
  कुरु {                                                                         \
  } यावत् (0)
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  कुरु {                                                                         \
  } यावत् (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  कुरु {                                                                         \
  } यावत् (0)
#endif

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#define SMALL_GRANULARITY 16
//! Small granularity shift count
#define SMALL_GRANULARITY_SHIFT 4
//! Number of small block size classes
#define SMALL_CLASS_गिनती 65
//! Maximum size of a small block
#define SMALL_SIZE_LIMIT (SMALL_GRANULARITY * (SMALL_CLASS_गिनती - 1))
//! Granularity of a medium allocation block
#define MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#define MEDIUM_GRANULARITY_SHIFT 9
//! Number of medium block size classes
#define MEDIUM_CLASS_गिनती 61
//! Total number of small + medium size classes
#define SIZE_CLASS_गिनती (SMALL_CLASS_गिनती + MEDIUM_CLASS_गिनती)
//! Number of large block size classes
#define LARGE_CLASS_गिनती 63
//! Maximum size of a medium block
#define MEDIUM_SIZE_LIMIT                                                      \
  (SMALL_SIZE_LIMIT + (MEDIUM_GRANULARITY * MEDIUM_CLASS_गिनती))
//! Maximum size of a large block
#define LARGE_SIZE_LIMIT                                                       \
  ((LARGE_CLASS_गिनती * _memory_span_size) - SPAN_HEADER_SIZE)
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

#यदि ENABLE_VALIDATE_ARGS
//! Maximum allocation size to avoid integer overflow
#undef MAX_ALLOC_SIZE
#define MAX_ALLOC_SIZE (((माप_प्रकार) - 1) - _memory_span_size)
#endif

#define pointer_offset(ptr, ofs) (शून्य *)((वर्ण *)(ptr) + (सूचक_भेद_प्रकार)(ofs))
#define pointer_diff(first, second)                                            \
  (सूचक_भेद_प्रकार)((ध्रुव वर्ण *)(first) - (ध्रुव वर्ण *)(second))

#define INVALID_POINTER ((शून्य *)((uintptr_t) - 1))

#define SIZE_CLASS_LARGE SIZE_CLASS_गिनती
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

#यदि ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
संरचना span_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t ऊपरी;
#यदि ENABLE_STATISTICS
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

#यदि ENABLE_STATISTICS
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
  uint32_t block_गिनती;
  //! Size class
  uint32_t size_class;
  //! Index of last block initialized in free list
  uint32_t free_list_limit;
  //! Number of used blocks remaining when in partial state
  uint32_t used_गिनती;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_size;
  //! Size of a block
  uint32_t block_size;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t span_गिनती;
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
  माप_प्रकार गिनती;
  span_t *span[MAX_THREAD_SPAN_CACHE];
};
प्रकारनिर्देश संरचना span_cache_t span_cache_t;

संरचना span_large_cache_t {
  माप_प्रकार गिनती;
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
  heap_size_class_t size_class[SIZE_CLASS_गिनती];
#यदि ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  span_cache_t span_cache;
#endif
  //! List of deferred free spans (single linked list)
  atomicptr_t span_free_deferred;
  //! Number of full spans
  माप_प्रकार full_span_गिनती;
  //! Mapped but unused spans
  span_t *span_reserve;
  //! Master span for mapped but unused spans
  span_t *span_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t child_गिनती;
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
#यदि ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  span_large_cache_t span_large_cache[LARGE_CLASS_गिनती - 1];
#endif
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  span_t *full_span[SIZE_CLASS_गिनती];
  //! Double linked list of large and huge spans allocated by this heap
  span_t *large_huge_span;
#endif
#यदि ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  span_use_t span_use[LARGE_CLASS_गिनती];
#endif
#यदि ENABLE_STATISTICS
  //! Allocation stats per size class
  size_class_use_t size_class_use[SIZE_CLASS_गिनती + 1];
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
  uint16_t block_गिनती;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(परिमाण(size_class_t) == 8, "Size class size mismatch");

संरचना global_cache_t {
  //! Cache lock
  atomic32_t lock;
  //! Cache count
  uint32_t गिनती;
#यदि ENABLE_STATISTICS
  //! Insert count
  माप_प्रकार जोड़ो_गिनती;
  //! Extract count
  माप_प्रकार extract_गिनती;
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
स्थिर माप_प्रकार _memory_page_size;
//! Shift to divide by page size
स्थिर माप_प्रकार _memory_page_size_shift;
//! Granularity at which memory pages are mapped by OS
स्थिर माप_प्रकार _memory_map_granularity;
#यदि RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
स्थिर माप_प्रकार _memory_span_size;
//! Shift to divide by span size
स्थिर माप_प्रकार _memory_span_size_shift;
//! Mask to get to start of a memory span
स्थिर uintptr_t _memory_span_mask;
#अन्यथा
//! Hardwired span size
#define _memory_span_size _memory_default_span_size
#define _memory_span_size_shift _memory_default_span_size_shift
#define _memory_span_mask _memory_default_span_mask
#endif
//! Number of spans to map in each map call
स्थिर माप_प्रकार memory_span_map_गिनती;
//! Number of spans to keep reserved in each heap
स्थिर माप_प्रकार memory_heap_reserve_गिनती;
//! Global size classes
स्थिर size_class_t _memory_size_class[SIZE_CLASS_गिनती];
//! Run-time size limit of medium blocks
स्थिर माप_प्रकार _memory_medium_size_limit;
//! Heap ID counter
स्थिर atomic32_t _memory_heap_id;
//! Huge page support
स्थिर पूर्णाङ्क _memory_huge_pages;
#यदि ENABLE_GLOBAL_CACHE
//! Global span cache
स्थिर global_cache_t _memory_span_cache[LARGE_CLASS_गिनती];
#endif
//! Global reserved spans
स्थिर span_t *_memory_global_reserve;
//! Global reserved count
स्थिर माप_प्रकार memory_global_reserve_गिनती;
//! Global reserved master
स्थिर span_t *_memory_global_reserve_master;
//! All heaps
स्थिर heap_t *_memory_heaps[HEAP_ARRAY_SIZE];
//! Used to restrict access to mapping memory for huge pages
स्थिर atomic32_t _memory_global_lock;
//! Orphaned heaps
स्थिर heap_t *_memory_orphan_heaps;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
स्थिर heap_t *_memory_first_class_orphan_heaps;
#endif
#यदि ENABLE_STATISTICS
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
#यदि ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
स्थिर pthread_key_t _memory_thread_heap;
#अन्यथा
#ifdef _MSC_VER
#define _Thread_local __declspec(thread)
#define TLS_MODEL
#अन्यथा
#ifndef __HAIKU__
#define TLS_MODEL __attribute__((tls_model("initial-exec")))
#अन्यथा
#define TLS_MODEL
#endif
#यदि !defined(__clang__) && defined(__GNUC__)
#define _Thread_local __thread
#endif
#endif
स्थिर _Thread_local heap_t *_memory_thread_heap TLS_MODEL;
#endif

स्थिर अन्तःस्थ heap_t *get_thread_heap_raw(शून्य) {
#यदि (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  प्रत्यागम pthread_getspecific(_memory_thread_heap);
#अन्यथा
  प्रत्यागम _memory_thread_heap;
#endif
}

//! Get the current thread heap
स्थिर अन्तःस्थ heap_t *get_thread_heap(शून्य) {
  heap_t *heap = get_thread_heap_raw();
#यदि ENABLE_PRELOAD
  यदि (EXPECTED(heap != 0))
    प्रत्यागम heap;
  rpmalloc_initialize();
  प्रत्यागम get_thread_heap_raw();
#अन्यथा
  प्रत्यागम heap;
#endif
}

//! Fast thread ID
स्थिर अन्तःस्थ uintptr_t get_thread_id(शून्य) {
#यदि defined(_WIN32)
  प्रत्यागम (uintptr_t)((शून्य *)NtCurrentTeb());
#नोचेद्यदि (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#यदि defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#नोचेद्यदि defined(__x86_64__)
#यदि defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#अन्यथा
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#endif
#नोचेद्यदि defined(__arm__)
  __asm__ चञ्चल("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#नोचेद्यदि defined(__aarch64__)
#यदि defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ चञ्चल("mrs %0, tpidrro_el0" : "=r"(tid));
#अन्यथा
  __asm__ चञ्चल("mrs %0, tpidr_el0" : "=r"(tid));
#endif
#अन्यथा
#error This platform needs implementation of get_thread_id()
#endif
  प्रत्यागम tid;
#अन्यथा
#error This platform needs implementation of get_thread_id()
#endif
}

//! Set the current thread heap
स्थिर शून्य set_thread_heap(heap_t *heap) {
#यदि ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(_memory_thread_heap, heap);
#अन्यथा
  _memory_thread_heap = heap;
#endif
  यदि (heap)
    heap->owner_thread = get_thread_id();
}

//! Set main thread ID
बाह्य शून्य rpmalloc_set_main_thread(शून्य);

शून्य rpmalloc_set_main_thread(शून्य) {
  _rpmalloc_main_thread_id = get_thread_id();
}

स्थिर शून्य _rpmalloc_spin(शून्य) {
#यदि defined(_MSC_VER)
#यदि defined(_M_ARM64)
  __yield();
#अन्यथा
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
#अन्यथा
  संरचना timespec ts = {0};
  nanosleep(&ts, 0);
#endif
}

#यदि defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

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
#अन्यथा // _WIN64
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#pragma data_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#pragma data_seg()
#endif // _WIN64

स्थिर शून्य NTAPI _rpmalloc_thread_destructor(शून्य *मूल्य_2) {
#यदि ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  यदि (get_thread_id() == _rpmalloc_main_thread_id)
    प्रत्यागम;
#endif
  यदि (मूल्य_2)
    rpmalloc_thread_finalize(1);
}
#endif

////////////
///
/// Low level memory map/unmap
///
//////

स्थिर शून्य _rpmalloc_set_name(शून्य *address, माप_प्रकार size) {
#यदि defined(__linux__) || defined(__ANDROID__)
  ध्रुव वर्ण *name = _memory_huge_pages ? _memory_config.huge_page_name
                                        : _memory_config.page_name;
  यदि (address == MAP_FAILED || !name)
    प्रत्यागम;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (शून्य)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (uintptr_t)address, size,
              (uintptr_t)name);
#अन्यथा
  (शून्य)परिमाण(size);
  (शून्य)परिमाण(address);
#endif
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
स्थिर शून्य *_rpmalloc_mmap(माप_प्रकार size, माप_प्रकार *offset) {
  rpmalloc_assert(!(size % _memory_page_size), "Invalid mmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
  शून्य *address = _memory_config.memory_map(size, offset);
  यदि (EXPECTED(address != 0)) {
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
स्थिर शून्य _rpmalloc_unmap(शून्य *address, माप_प्रकार size, माप_प्रकार offset,
                            माप_प्रकार release) {
  rpmalloc_assert(!release || (release >= size), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  यदि (release) {
    rpmalloc_assert(!(release % _memory_page_size), "Invalid unmap size");
    _rpmalloc_stat_sub(&_mapped_pages, (release >> _memory_page_size_shift));
    _rpmalloc_stat_add(&_unmapped_total, (release >> _memory_page_size_shift));
  }
  _memory_config.memory_unmap(address, size, offset, release);
}

//! Default implementation to map new pages to virtual memory
स्थिर शून्य *_rpmalloc_mmap_os(माप_प्रकार size, माप_प्रकार *offset) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  माप_प्रकार padding = ((size >= _memory_span_size) &&
                    (_memory_span_size > _memory_map_granularity))
                       ? _memory_span_size
                       : 0;
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
#यदि PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  शून्य *ptr = VirtualAlloc(0, size + padding,
                           (_memory_huge_pages ? MEM_LARGE_PAGES : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
  यदि (!ptr) {
    यदि (_memory_config.map_fail_callback) {
      यदि (_memory_config.map_fail_callback(size + padding))
        प्रत्यागम _rpmalloc_mmap_os(size, offset);
    } अन्यथा {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    प्रत्यागम 0;
  }
#अन्यथा
  पूर्णाङ्क flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#यदि defined(__APPLE__) && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
  पूर्णाङ्क fd = (पूर्णाङ्क)VM_MAKE_TAG(240U);
  यदि (_memory_huge_pages)
    fd |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
  शून्य *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#नोचेद्यदि defined(MAP_HUGETLB)
  शून्य *ptr = mmap(0, size + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (_memory_huge_pages ? MAP_HUGETLB : 0) | flags, -1, 0);
#यदि defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  यदि ((ptr == MAP_FAILED || !ptr) && _memory_huge_pages) {
    ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    यदि (ptr && ptr != MAP_FAILED) {
      पूर्णाङ्क prm = madvise(ptr, size + padding, MADV_HUGEPAGE);
      (शून्य)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#endif
  _rpmalloc_set_name(ptr, size + padding);
#नोचेद्यदि defined(MAP_ALIGNED)
  ध्रुव माप_प्रकार align =
      (परिमाण(माप_प्रकार) * 8) - (माप_प्रकार)(__builtin_clzl(size - 1));
  शून्य *ptr =
      mmap(0, size + padding, PROT_READ | PROT_WRITE,
           (_memory_huge_pages ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#नोचेद्यदि defined(MAP_ALIGN)
  caddr_t base = (_memory_huge_pages ? (caddr_t)(4 << 20) : 0);
  शून्य *ptr = mmap(base, size + padding, PROT_READ | PROT_WRITE,
                   (_memory_huge_pages ? MAP_ALIGN : 0) | flags, -1, 0);
#अन्यथा
  शून्य *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#endif
  यदि ((ptr == MAP_FAILED) || !ptr) {
    यदि (_memory_config.map_fail_callback) {
      यदि (_memory_config.map_fail_callback(size + padding))
        प्रत्यागम _rpmalloc_mmap_os(size, offset);
    } अन्यथा यदि (त्रुटि_सं != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    प्रत्यागम 0;
  }
#endif
  _rpmalloc_stat_add(&_mapped_pages_os,
                     (int32_t)((size + padding) >> _memory_page_size_shift));
  यदि (padding) {
    माप_प्रकार final_padding = padding - ((uintptr_t)ptr & ~_memory_span_mask);
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
स्थिर शून्य _rpmalloc_unmap_os(शून्य *address, माप_प्रकार size, माप_प्रकार offset,
                               माप_प्रकार release) {
  rpmalloc_assert(release || (offset == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid unmap size");
  यदि (release && offset) {
    offset <<= 3;
    address = pointer_offset(address, -(int32_t)offset);
    यदि ((release >= _memory_span_size) &&
        (_memory_span_size > _memory_map_granularity)) {
      // Padding is always one span size
      release += _memory_span_size;
    }
  }
#यदि !DISABLE_UNMAP
#यदि PLATFORM_WINDOWS
  यदि (!VirtualFree(address, release ? 0 : size,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#अन्यथा
  यदि (release) {
    यदि (munmap(address, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } अन्यथा {
#यदि defined(MADV_FREE_REUSABLE)
    पूर्णाङ्क ret;
    यावत् ((ret = madvise(address, size, MADV_FREE_REUSABLE)) == -1 &&
           (त्रुटि_सं == EAGAIN))
      त्रुटि_सं = 0;
    यदि ((ret == -1) && (त्रुटि_सं != 0)) {
#नोचेद्यदि defined(MADV_DONTNEED)
    यदि (madvise(address, size, MADV_DONTNEED)) {
#नोचेद्यदि defined(MADV_PAGEOUT)
    यदि (madvise(address, size, MADV_PAGEOUT)) {
#नोचेद्यदि defined(MADV_FREE)
    यदि (madvise(address, size, MADV_FREE)) {
#अन्यथा
    यदि (posix_madvise(address, size, POSIX_MADV_DONTNEED)) {
#endif
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#endif
#endif
  यदि (release)
    _rpmalloc_stat_sub(&_mapped_pages_os, release >> _memory_page_size_shift);
}

स्थिर शून्य _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         माप_प्रकार span_गिनती);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
स्थिर span_t *_rpmalloc_global_get_reserved_spans(माप_प्रकार span_गिनती) {
  span_t *span = _memory_global_reserve;
  _rpmalloc_span_mark_as_subspan_unless_master(_memory_global_reserve_master,
                                               span, span_गिनती);
  memory_global_reserve_गिनती -= span_गिनती;
  यदि (memory_global_reserve_गिनती)
    _memory_global_reserve =
        (span_t *)pointer_offset(span, span_गिनती << _memory_span_size_shift);
  अन्यथा
    _memory_global_reserve = 0;
  प्रत्यागम span;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
स्थिर शून्य _rpmalloc_global_set_reserved_spans(span_t *master, span_t *reserve,
                                                माप_प्रकार reserve_span_गिनती) {
  _memory_global_reserve_master = master;
  memory_global_reserve_गिनती = reserve_span_गिनती;
  _memory_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
स्थिर शून्य _rpmalloc_span_double_link_list_add(span_t **head, span_t *span) {
  यदि (*head)
    (*head)->prev = span;
  span->next = *head;
  *head = span;
}

//! Pop head span from double linked list
स्थिर शून्य rpmalloc_span_double_link_list_निकालो_head(span_t **head,
                                                     span_t *span) {
  rpmalloc_assert(*head == span, "Linked list corrupted");
  span = *head;
  *head = span->next;
}

//! Remove a span from double linked list
स्थिर शून्य _rpmalloc_span_double_link_list_remove(span_t **head,
                                                   span_t *span) {
  rpmalloc_assert(*head, "Linked list corrupted");
  यदि (*head == span) {
    *head = span->next;
  } अन्यथा {
    span_t *next_span = span->next;
    span_t *prev_span = span->prev;
    prev_span->next = next_span;
    यदि (EXPECTED(next_span != 0))
      next_span->prev = prev_span;
  }
}

////////////
///
/// Span control
///
//////

स्थिर शून्य rpmalloc_heap_cache_जोड़ो(heap_t *heap, span_t *span);

स्थिर शून्य _rpmalloc_heap_finalize(heap_t *heap);

स्थिर शून्य _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              माप_प्रकार reserve_span_गिनती);

//! Declare the span to be a subspan and store distance from master span and
//! span count
स्थिर शून्य _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         माप_प्रकार span_गिनती) {
  rpmalloc_assert((subspan != master) || (subspan->flags & SPAN_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  यदि (subspan != master) {
    subspan->flags = SPAN_FLAG_SUBSPAN;
    subspan->offset_from_master =
        (uint32_t)((uintptr_t)pointer_diff(subspan, master) >>
                   _memory_span_size_shift);
    subspan->align_offset = 0;
  }
  subspan->span_गिनती = (uint32_t)span_गिनती;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
स्थिर span_t *_rpmalloc_span_map_from_reserve(heap_t *heap,
                                               माप_प्रकार span_गिनती) {
  // Update the heap span reserve
  span_t *span = heap->span_reserve;
  heap->span_reserve =
      (span_t *)pointer_offset(span, span_गिनती * _memory_span_size);
  heap->spans_reserved -= (uint32_t)span_गिनती;

  _rpmalloc_span_mark_as_subspan_unless_master(heap->span_reserve_master, span,
                                               span_गिनती);
  यदि (span_गिनती <= LARGE_CLASS_गिनती)
    _rpmalloc_stat_inc(&heap->span_use[span_गिनती - 1].spans_from_reserved);

  प्रत्यागम span;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
स्थिर माप_प्रकार rpmalloc_span_align_गिनती(माप_प्रकार span_गिनती) {
  माप_प्रकार request_गिनती = (span_गिनती > memory_span_map_गिनती)
                             ? span_गिनती
                             : memory_span_map_गिनती;
  यदि ((_memory_page_size > _memory_span_size) &&
      ((request_गिनती * _memory_span_size) % _memory_page_size))
    request_गिनती +=
        memory_span_map_गिनती - (request_गिनती % memory_span_map_गिनती);
  प्रत्यागम request_गिनती;
}

//! Setup a newly mapped span
स्थिर शून्य _rpmalloc_span_initialize(span_t *span, माप_प्रकार total_span_गिनती,
                                      माप_प्रकार span_गिनती, माप_प्रकार align_offset) {
  span->total_spans = (uint32_t)total_span_गिनती;
  span->span_गिनती = (uint32_t)span_गिनती;
  span->align_offset = (uint32_t)align_offset;
  span->flags = SPAN_FLAG_MASTER;
  atomic_store32(&span->remaining_spans, (int32_t)total_span_गिनती);
}

स्थिर शून्य _rpmalloc_span_unmap(span_t *span);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
स्थिर span_t *rpmalloc_span_map_aligned_गिनती(heap_t *heap,
                                                माप_प्रकार span_गिनती) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  माप_प्रकार aligned_span_गिनती = rpmalloc_span_align_गिनती(span_गिनती);
  माप_प्रकार align_offset = 0;
  span_t *span = (span_t *)_rpmalloc_mmap(
      aligned_span_गिनती * _memory_span_size, &align_offset);
  यदि (!span)
    प्रत्यागम 0;
  _rpmalloc_span_initialize(span, aligned_span_गिनती, span_गिनती, align_offset);
  _rpmalloc_stat_inc(&_master_spans);
  यदि (span_गिनती <= LARGE_CLASS_गिनती)
    _rpmalloc_stat_inc(&heap->span_use[span_गिनती - 1].spans_map_calls);
  यदि (aligned_span_गिनती > span_गिनती) {
    span_t *reserved_spans =
        (span_t *)pointer_offset(span, span_गिनती * _memory_span_size);
    माप_प्रकार reserved_गिनती = aligned_span_गिनती - span_गिनती;
    यदि (heap->spans_reserved) {
      _rpmalloc_span_mark_as_subspan_unless_master(
          heap->span_reserve_master, heap->span_reserve, heap->spans_reserved);
      rpmalloc_heap_cache_जोड़ो(heap, heap->span_reserve);
    }
    यदि (reserved_गिनती > memory_heap_reserve_गिनती) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&_memory_global_lock) == 1,
                      "Global spin lock not held as expected");
      माप_प्रकार remain_गिनती = reserved_गिनती - memory_heap_reserve_गिनती;
      reserved_गिनती = memory_heap_reserve_गिनती;
      span_t *remain_span = (span_t *)pointer_offset(
          reserved_spans, reserved_गिनती * _memory_span_size);
      यदि (_memory_global_reserve) {
        _rpmalloc_span_mark_as_subspan_unless_master(
            _memory_global_reserve_master, _memory_global_reserve,
            memory_global_reserve_गिनती);
        _rpmalloc_span_unmap(_memory_global_reserve);
      }
      _rpmalloc_global_set_reserved_spans(span, remain_span, remain_गिनती);
    }
    _rpmalloc_heap_set_reserved_spans(heap, span, reserved_spans,
                                      reserved_गिनती);
  }
  प्रत्यागम span;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
स्थिर span_t *_rpmalloc_span_map(heap_t *heap, माप_प्रकार span_गिनती) {
  यदि (span_गिनती <= heap->spans_reserved)
    प्रत्यागम _rpmalloc_span_map_from_reserve(heap, span_गिनती);
  span_t *span = 0;
  पूर्णाङ्क use_global_reserve =
      (_memory_page_size > _memory_span_size) ||
      (memory_span_map_गिनती > memory_heap_reserve_गिनती);
  यदि (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    यावत् (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
    यदि (memory_global_reserve_गिनती >= span_गिनती) {
      माप_प्रकार reserve_गिनती =
          (!heap->spans_reserved ? memory_heap_reserve_गिनती : span_गिनती);
      यदि (memory_global_reserve_गिनती < reserve_गिनती)
        reserve_गिनती = memory_global_reserve_गिनती;
      span = _rpmalloc_global_get_reserved_spans(reserve_गिनती);
      यदि (span) {
        यदि (reserve_गिनती > span_गिनती) {
          span_t *reserved_span = (span_t *)pointer_offset(
              span, span_गिनती << _memory_span_size_shift);
          _rpmalloc_heap_set_reserved_spans(heap, _memory_global_reserve_master,
                                            reserved_span,
                                            reserve_गिनती - span_गिनती);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        span->span_गिनती = (uint32_t)span_गिनती;
      }
    }
  }
  यदि (!span)
    span = rpmalloc_span_map_aligned_गिनती(heap, span_गिनती);
  यदि (use_global_reserve)
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

  माप_प्रकार span_गिनती = span->span_गिनती;
  यदि (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(span->align_offset == 0, "Span align offset corrupted");
    यदि (_memory_span_size >= _memory_page_size)
      _rpmalloc_unmap(span, span_गिनती * _memory_span_size, 0, 0);
  } अन्यथा {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    span->flags |=
        SPAN_FLAG_MASTER | SPAN_FLAG_SUBSPAN | SPAN_FLAG_UNMAPPED_MASTER;
    _rpmalloc_stat_add(&_unmapped_master_spans, 1);
  }

  यदि (atomic_add32(&master->remaining_spans, -(int32_t)span_गिनती) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & SPAN_FLAG_MASTER) &&
                        !!(master->flags & SPAN_FLAG_SUBSPAN),
                    "Span flag corrupted");
    माप_प्रकार unmap_गिनती = master->span_गिनती;
    यदि (_memory_span_size < _memory_page_size)
      unmap_गिनती = master->total_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_गिनती * _memory_span_size,
                    master->align_offset,
                    (माप_प्रकार)master->total_spans * _memory_span_size);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
स्थिर शून्य _rpmalloc_span_release_to_cache(heap_t *heap, span_t *span) {
  rpmalloc_assert(heap == span->heap, "Span heap pointer corrupted");
  rpmalloc_assert(span->size_class < SIZE_CLASS_गिनती,
                  "Invalid span size class");
  rpmalloc_assert(span->span_गिनती == 1, "Invalid span count");
#यदि ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&heap->span_use[0].current);
#endif
  _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
  यदि (!heap->finalize) {
    _rpmalloc_stat_inc(&heap->span_use[0].spans_to_cache);
    _rpmalloc_stat_inc(&heap->size_class_use[span->size_class].spans_to_cache);
    यदि (heap->size_class[span->size_class].cache)
      rpmalloc_heap_cache_जोड़ो(heap,
                                  heap->size_class[span->size_class].cache);
    heap->size_class[span->size_class].cache = span;
  } अन्यथा {
    _rpmalloc_span_unmap(span);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
स्थिर uint32_t free_list_partial_init(शून्य **list, शून्य **first_block,
                                       शून्य *page_start, शून्य *block_start,
                                       uint32_t block_गिनती,
                                       uint32_t block_size) {
  rpmalloc_assert(block_गिनती, "Internal failure");
  *first_block = block_start;
  यदि (block_गिनती > 1) {
    शून्य *free_block = pointer_offset(block_start, block_size);
    शून्य *block_end =
        pointer_offset(block_start, (माप_प्रकार)block_size * block_गिनती);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    यदि (block_size < (_memory_page_size >> 1)) {
      शून्य *page_end = pointer_offset(page_start, _memory_page_size);
      यदि (page_end < block_end)
        block_end = page_end;
    }
    *list = free_block;
    block_गिनती = 2;
    शून्य *next_block = pointer_offset(free_block, block_size);
    यावत् (next_block < block_end) {
      *((शून्य **)free_block) = next_block;
      free_block = next_block;
      ++block_गिनती;
      next_block = pointer_offset(next_block, block_size);
    }
    *((शून्य **)free_block) = 0;
  } अन्यथा {
    *list = 0;
  }
  प्रत्यागम block_गिनती;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
स्थिर शून्य *_rpmalloc_span_initialize_new(heap_t *heap,
                                           heap_size_class_t *heap_size_class,
                                           span_t *span, uint32_t class_idx) {
  rpmalloc_assert(span->span_गिनती == 1, "Internal failure");
  size_class_t *size_class = _memory_size_class + class_idx;
  span->size_class = class_idx;
  span->heap = heap;
  span->flags &= ~SPAN_FLAG_ALIGNED_BLOCKS;
  span->block_size = size_class->block_size;
  span->block_गिनती = size_class->block_गिनती;
  span->free_list = 0;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  शून्य *block;
  span->free_list_limit =
      free_list_partial_init(&heap_size_class->free_list, &block, span,
                             pointer_offset(span, SPAN_HEADER_SIZE),
                             size_class->block_गिनती, size_class->block_size);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  यदि (span->free_list_limit < span->block_गिनती) {
    _rpmalloc_span_double_link_list_add(&heap_size_class->partial_span, span);
    span->used_गिनती = span->free_list_limit;
  } अन्यथा {
#यदि RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_गिनती;
    span->used_गिनती = span->block_गिनती;
  }
  प्रत्यागम block;
}

स्थिर शून्य _rpmalloc_span_extract_free_list_deferred(span_t *span) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  कुरु {
    span->free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } यावत् (span->free_list == INVALID_POINTER);
  span->used_गिनती -= span->list_size;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);
}

स्थिर पूर्णाङ्क _rpmalloc_span_is_fully_utilized(span_t *span) {
  rpmalloc_assert(span->free_list_limit <= span->block_गिनती,
                  "Span free list corrupted");
  प्रत्यागम !span->free_list && (span->free_list_limit >= span->block_गिनती);
}

स्थिर पूर्णाङ्क _rpmalloc_span_finalize(heap_t *heap, माप_प्रकार iclass, span_t *span,
                                   span_t **list_head) {
  शून्य *free_list = heap->size_class[iclass].free_list;
  span_t *class_span = (span_t *)((uintptr_t)free_list & _memory_span_mask);
  यदि (span == class_span) {
    // Adopt the heap class free list back into the span free list
    शून्य *block = span->free_list;
    शून्य *last_block = 0;
    यावत् (block) {
      last_block = block;
      block = *((शून्य **)block);
    }
    uint32_t free_गिनती = 0;
    block = free_list;
    यावत् (block) {
      ++free_गिनती;
      block = *((शून्य **)block);
    }
    यदि (last_block) {
      *((शून्य **)last_block) = free_list;
    } अन्यथा {
      span->free_list = free_list;
    }
    heap->size_class[iclass].free_list = 0;
    span->used_गिनती -= free_गिनती;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(span->list_size == span->used_गिनती, "Memory leak detected");
  यदि (span->list_size == span->used_गिनती) {
    _rpmalloc_stat_dec(&heap->span_use[0].current);
    _rpmalloc_stat_dec(&heap->size_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    यदि (list_head)
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

#यदि ENABLE_GLOBAL_CACHE

//! Finalize a global cache
स्थिर शून्य _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  यावत् (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

  प्रति (माप_प्रकार ispan = 0; ispan < cache->गिनती; ++ispan)
    _rpmalloc_span_unmap(cache->span[ispan]);
  cache->गिनती = 0;

  यावत् (cache->overflow) {
    span_t *span = cache->overflow;
    cache->overflow = span->next;
    _rpmalloc_span_unmap(span);
  }

  atomic_store32_release(&cache->lock, 0);
}

स्थिर शून्य rpmalloc_global_cache_जोड़ो_spans(span_t **span,
                                                माप_प्रकार span_गिनती,
                                                माप_प्रकार गिनती) {
  ध्रुव माप_प्रकार cache_limit =
      (span_गिनती == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_SPAN_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_SPAN_LARGE_CACHE - (span_गिनती >> 1));

  global_cache_t *cache = &_memory_span_cache[span_गिनती - 1];

  माप_प्रकार जोड़ो_गिनती = गिनती;
  यावत् (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#यदि ENABLE_STATISTICS
  cache->जोड़ो_गिनती += गिनती;
#endif
  यदि ((cache->गिनती + जोड़ो_गिनती) > cache_limit)
    जोड़ो_गिनती = cache_limit - cache->गिनती;

  स_नकल(cache->span + cache->गिनती, span, परिमाण(span_t *) * जोड़ो_गिनती);
  cache->गिनती += (uint32_t)जोड़ो_गिनती;

#यदि ENABLE_UNLIMITED_CACHE
  यावत् (जोड़ो_गिनती < गिनती) {
#अन्यथा
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  यावत् ((_memory_page_size > _memory_span_size) && (जोड़ो_गिनती < गिनती)) {
#endif
    span_t *current_span = span[जोड़ो_गिनती++];
    current_span->next = cache->overflow;
    cache->overflow = current_span;
  }
  atomic_store32_release(&cache->lock, 0);

  span_t *keep = 0;
  प्रति (माप_प्रकार ispan = जोड़ो_गिनती; ispan < गिनती; ++ispan) {
    span_t *current_span = span[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    यदि ((current_span->flags & SPAN_FLAG_MASTER) &&
        (atomic_load32(&current_span->remaining_spans) >
         (int32_t)current_span->span_गिनती)) {
      current_span->next = keep;
      keep = current_span;
    } अन्यथा {
      _rpmalloc_span_unmap(current_span);
    }
  }

  यदि (keep) {
    यावत् (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();

    माप_प्रकार islot = 0;
    यावत् (keep) {
      प्रति (; islot < cache->गिनती; ++islot) {
        span_t *current_span = cache->span[islot];
        यदि (!(current_span->flags & SPAN_FLAG_MASTER) ||
            ((current_span->flags & SPAN_FLAG_MASTER) &&
             (atomic_load32(&current_span->remaining_spans) <=
              (int32_t)current_span->span_गिनती))) {
          _rpmalloc_span_unmap(current_span);
          cache->span[islot] = keep;
          भङ्ग;
        }
      }
      यदि (islot == cache->गिनती)
        भङ्ग;
      keep = keep->next;
    }

    यदि (keep) {
      span_t *tail = keep;
      यावत् (tail->next)
        tail = tail->next;
      tail->next = cache->overflow;
      cache->overflow = keep;
    }

    atomic_store32_release(&cache->lock, 0);
  }
}

स्थिर माप_प्रकार _rpmalloc_global_cache_extract_spans(span_t **span,
                                                   माप_प्रकार span_गिनती,
                                                   माप_प्रकार गिनती) {
  global_cache_t *cache = &_memory_span_cache[span_गिनती - 1];

  माप_प्रकार extract_गिनती = 0;
  यावत् (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#यदि ENABLE_STATISTICS
  cache->extract_गिनती += गिनती;
#endif
  माप_प्रकार want = गिनती - extract_गिनती;
  यदि (want > cache->गिनती)
    want = cache->गिनती;

  स_नकल(span + extract_गिनती, cache->span + (cache->गिनती - want),
         परिमाण(span_t *) * want);
  cache->गिनती -= (uint32_t)want;
  extract_गिनती += want;

  यावत् ((extract_गिनती < गिनती) && cache->overflow) {
    span_t *current_span = cache->overflow;
    span[extract_गिनती++] = current_span;
    cache->overflow = current_span->next;
  }

#यदि ENABLE_ASSERTS
  प्रति (माप_प्रकार ispan = 0; ispan < extract_गिनती; ++ispan) {
    rpmalloc_assert(span[ispan]->span_गिनती == span_गिनती,
                    "Global cache span count mismatch");
  }
#endif

  atomic_store32_release(&cache->lock, 0);

  प्रत्यागम extract_गिनती;
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
                                              माप_प्रकार reserve_span_गिनती) {
  heap->span_reserve_master = master;
  heap->span_reserve = reserve;
  heap->spans_reserved = (uint32_t)reserve_span_गिनती;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
स्थिर शून्य _rpmalloc_heap_cache_adopt_deferred(heap_t *heap,
                                                span_t **single_span) {
  span_t *span = (span_t *)((शून्य *)atomic_exchange_ptr_acquire(
      &heap->span_free_deferred, 0));
  यावत् (span) {
    span_t *next_span = (span_t *)span->free_list;
    rpmalloc_assert(span->heap == heap, "Span heap pointer corrupted");
    यदि (EXPECTED(span->size_class < SIZE_CLASS_गिनती)) {
      rpmalloc_assert(heap->full_span_गिनती, "Heap span counter corrupted");
      --heap->full_span_गिनती;
      _rpmalloc_stat_dec(&heap->span_use[0].spans_deferred);
#यदि RPMALLOC_FIRST_CLASS_HEAPS
      _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                             span);
#endif
      _rpmalloc_stat_dec(&heap->span_use[0].current);
      _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
      यदि (single_span && !*single_span)
        *single_span = span;
      अन्यथा
        rpmalloc_heap_cache_जोड़ो(heap, span);
    } अन्यथा {
      यदि (span->size_class == SIZE_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(span);
      } अन्यथा {
        rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(heap->full_span_गिनती, "Heap span counter corrupted");
        --heap->full_span_गिनती;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
        _rpmalloc_span_double_link_list_remove(&heap->large_huge_span, span);
#endif
        uint32_t idx = span->span_गिनती - 1;
        _rpmalloc_stat_dec(&heap->span_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&heap->span_use[idx].current);
        यदि (!idx && single_span && !*single_span)
          *single_span = span;
        अन्यथा
          rpmalloc_heap_cache_जोड़ो(heap, span);
      }
    }
    span = next_span;
  }
}

स्थिर शून्य _rpmalloc_heap_unmap(heap_t *heap) {
  यदि (!heap->master_heap) {
    यदि ((heap->finalize > 1) && !atomic_load32(&heap->child_गिनती)) {
      span_t *span = (span_t *)((uintptr_t)heap & _memory_span_mask);
      _rpmalloc_span_unmap(span);
    }
  } अन्यथा {
    यदि (atomic_decr32(&heap->master_heap->child_गिनती) == 0) {
      _rpmalloc_heap_unmap(heap->master_heap);
    }
  }
}

स्थिर शून्य _rpmalloc_heap_global_finalize(heap_t *heap) {
  यदि (heap->finalize++ > 1) {
    --heap->finalize;
    प्रत्यागम;
  }

  _rpmalloc_heap_finalize(heap);

#यदि ENABLE_THREAD_CACHE
  प्रति (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    span_cache_t *span_cache;
    यदि (!iclass)
      span_cache = &heap->span_cache;
    अन्यथा
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    प्रति (माप_प्रकार ispan = 0; ispan < span_cache->गिनती; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->गिनती = 0;
  }
#endif

  यदि (heap->full_span_गिनती) {
    --heap->finalize;
    प्रत्यागम;
  }

  प्रति (माप_प्रकार iclass = 0; iclass < SIZE_CLASS_गिनती; ++iclass) {
    यदि (heap->size_class[iclass].free_list ||
        heap->size_class[iclass].partial_span) {
      --heap->finalize;
      प्रत्यागम;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  माप_प्रकार list_idx = (माप_प्रकार)heap->id % HEAP_ARRAY_SIZE;
  heap_t *list_heap = _memory_heaps[list_idx];
  यदि (list_heap == heap) {
    _memory_heaps[list_idx] = heap->next_heap;
  } अन्यथा {
    यावत् (list_heap->next_heap != heap)
      list_heap = list_heap->next_heap;
    list_heap->next_heap = heap->next_heap;
  }

  _rpmalloc_heap_unmap(heap);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
स्थिर शून्य rpmalloc_heap_cache_जोड़ो(heap_t *heap, span_t *span) {
  यदि (UNEXPECTED(heap->finalize != 0)) {
    _rpmalloc_span_unmap(span);
    _rpmalloc_heap_global_finalize(heap);
    प्रत्यागम;
  }
#यदि ENABLE_THREAD_CACHE
  माप_प्रकार span_गिनती = span->span_गिनती;
  _rpmalloc_stat_inc(&heap->span_use[span_गिनती - 1].spans_to_cache);
  यदि (span_गिनती == 1) {
    span_cache_t *span_cache = &heap->span_cache;
    span_cache->span[span_cache->गिनती++] = span;
    यदि (span_cache->गिनती == MAX_THREAD_SPAN_CACHE) {
      ध्रुव माप_प्रकार remain_गिनती =
          MAX_THREAD_SPAN_CACHE - THREAD_SPAN_CACHE_TRANSFER;
#यदि ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           THREAD_SPAN_CACHE_TRANSFER * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_गिनती - 1].spans_to_global,
                         THREAD_SPAN_CACHE_TRANSFER);
      rpmalloc_global_cache_जोड़ो_spans(span_cache->span + remain_गिनती,
                                          span_गिनती,
                                          THREAD_SPAN_CACHE_TRANSFER);
#अन्यथा
      प्रति (माप_प्रकार ispan = 0; ispan < THREAD_SPAN_CACHE_TRANSFER; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_गिनती + ispan]);
#endif
      span_cache->गिनती = remain_गिनती;
    }
  } अन्यथा {
    माप_प्रकार cache_idx = span_गिनती - 2;
    span_large_cache_t *span_cache = heap->span_large_cache + cache_idx;
    span_cache->span[span_cache->गिनती++] = span;
    ध्रुव माप_प्रकार cache_limit =
        (MAX_THREAD_SPAN_LARGE_CACHE - (span_गिनती >> 1));
    यदि (span_cache->गिनती == cache_limit) {
      ध्रुव माप_प्रकार transfer_limit = 2 + (cache_limit >> 2);
      ध्रुव माप_प्रकार transfer_गिनती =
          (THREAD_SPAN_LARGE_CACHE_TRANSFER <= transfer_limit
               ? THREAD_SPAN_LARGE_CACHE_TRANSFER
               : transfer_limit);
      ध्रुव माप_प्रकार remain_गिनती = cache_limit - transfer_गिनती;
#यदि ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           transfer_गिनती * span_गिनती * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_गिनती - 1].spans_to_global,
                         transfer_गिनती);
      rpmalloc_global_cache_जोड़ो_spans(span_cache->span + remain_गिनती,
                                          span_गिनती, transfer_गिनती);
#अन्यथा
      प्रति (माप_प्रकार ispan = 0; ispan < transfer_गिनती; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_गिनती + ispan]);
#endif
      span_cache->गिनती = remain_गिनती;
    }
  }
#अन्यथा
  (शून्य)परिमाण(heap);
  _rpmalloc_span_unmap(span);
#endif
}

//! Extract the given number of spans from the different cache levels
स्थिर span_t *_rpmalloc_heap_thread_cache_extract(heap_t *heap,
                                                   माप_प्रकार span_गिनती) {
  span_t *span = 0;
#यदि ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  यदि (span_गिनती == 1)
    span_cache = &heap->span_cache;
  अन्यथा
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_गिनती - 2));
  यदि (span_cache->गिनती) {
    _rpmalloc_stat_inc(&heap->span_use[span_गिनती - 1].spans_from_cache);
    प्रत्यागम span_cache->span[--span_cache->गिनती];
  }
#endif
  प्रत्यागम span;
}

स्थिर span_t *_rpmalloc_heap_thread_cache_deferred_extract(heap_t *heap,
                                                            माप_प्रकार span_गिनती) {
  span_t *span = 0;
  यदि (span_गिनती == 1) {
    _rpmalloc_heap_cache_adopt_deferred(heap, &span);
  } अन्यथा {
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
    span = _rpmalloc_heap_thread_cache_extract(heap, span_गिनती);
  }
  प्रत्यागम span;
}

स्थिर span_t *_rpmalloc_heap_reserved_extract(heap_t *heap,
                                               माप_प्रकार span_गिनती) {
  यदि (heap->spans_reserved >= span_गिनती)
    प्रत्यागम _rpmalloc_span_map(heap, span_गिनती);
  प्रत्यागम 0;
}

//! Extract a span from the global cache
स्थिर span_t *_rpmalloc_heap_global_cache_extract(heap_t *heap,
                                                   माप_प्रकार span_गिनती) {
#यदि ENABLE_GLOBAL_CACHE
#यदि ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  माप_प्रकार wanted_गिनती;
  यदि (span_गिनती == 1) {
    span_cache = &heap->span_cache;
    wanted_गिनती = THREAD_SPAN_CACHE_TRANSFER;
  } अन्यथा {
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_गिनती - 2));
    wanted_गिनती = THREAD_SPAN_LARGE_CACHE_TRANSFER;
  }
  span_cache->गिनती = _rpmalloc_global_cache_extract_spans(
      span_cache->span, span_गिनती, wanted_गिनती);
  यदि (span_cache->गिनती) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_गिनती * span_cache->गिनती * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_गिनती - 1].spans_from_global,
                       span_cache->गिनती);
    प्रत्यागम span_cache->span[--span_cache->गिनती];
  }
#अन्यथा
  span_t *span = 0;
  माप_प्रकार गिनती = _rpmalloc_global_cache_extract_spans(&span, span_गिनती, 1);
  यदि (गिनती) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_गिनती * गिनती * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_गिनती - 1].spans_from_global,
                       गिनती);
    प्रत्यागम span;
  }
#endif
#endif
  (शून्य)परिमाण(heap);
  (शून्य)परिमाण(span_गिनती);
  प्रत्यागम 0;
}

स्थिर शून्य _rpmalloc_inc_span_statistics(heap_t *heap, माप_प्रकार span_गिनती,
                                          uint32_t class_idx) {
  (शून्य)परिमाण(heap);
  (शून्य)परिमाण(span_गिनती);
  (शून्य)परिमाण(class_idx);
#यदि ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)span_गिनती - 1;
  uint32_t current_गिनती =
      (uint32_t)atomic_incr32(&heap->span_use[idx].current);
  यदि (current_गिनती > (uint32_t)atomic_load32(&heap->span_use[idx].ऊपरी))
    atomic_store32(&heap->span_use[idx].ऊपरी, (int32_t)current_गिनती);
  _rpmalloc_stat_add_peak(&heap->size_class_use[class_idx].spans_current, 1,
                          heap->size_class_use[class_idx].spans_peak);
#endif
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
स्थिर span_t *
_rpmalloc_heap_extract_new_span(heap_t *heap,
                                heap_size_class_t *heap_size_class,
                                माप_प्रकार span_गिनती, uint32_t class_idx) {
  span_t *span;
#यदि ENABLE_THREAD_CACHE
  यदि (heap_size_class && heap_size_class->cache) {
    span = heap_size_class->cache;
    heap_size_class->cache =
        (heap->span_cache.गिनती
             ? heap->span_cache.span[--heap->span_cache.गिनती]
             : 0);
    _rpmalloc_inc_span_statistics(heap, span_गिनती, class_idx);
    प्रत्यागम span;
  }
#endif
  (शून्य)परिमाण(class_idx);
  // Allow 50% overhead to increase cache hits
  माप_प्रकार base_span_गिनती = span_गिनती;
  माप_प्रकार limit_span_गिनती =
      (span_गिनती > 2) ? (span_गिनती + (span_गिनती >> 1)) : span_गिनती;
  यदि (limit_span_गिनती > LARGE_CLASS_गिनती)
    limit_span_गिनती = LARGE_CLASS_गिनती;
  कुरु {
    span = _rpmalloc_heap_thread_cache_extract(heap, span_गिनती);
    यदि (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_गिनती, class_idx);
      प्रत्यागम span;
    }
    span = _rpmalloc_heap_thread_cache_deferred_extract(heap, span_गिनती);
    यदि (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_गिनती, class_idx);
      प्रत्यागम span;
    }
    span = _rpmalloc_heap_global_cache_extract(heap, span_गिनती);
    यदि (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_गिनती, class_idx);
      प्रत्यागम span;
    }
    span = _rpmalloc_heap_reserved_extract(heap, span_गिनती);
    यदि (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_reserved);
      _rpmalloc_inc_span_statistics(heap, span_गिनती, class_idx);
      प्रत्यागम span;
    }
    ++span_गिनती;
  } यावत् (span_गिनती <= limit_span_गिनती);
  // Final fallback, map in more virtual memory
  span = _rpmalloc_span_map(heap, base_span_गिनती);
  _rpmalloc_inc_span_statistics(heap, base_span_गिनती, class_idx);
  _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_map_calls);
  प्रत्यागम span;
}

स्थिर शून्य _rpmalloc_heap_initialize(heap_t *heap) {
  _rpmalloc_memset_const(heap, 0, परिमाण(heap_t));
  // Get a new heap ID
  heap->id = 1 + atomic_incr32(&_memory_heap_id);

  // Link in heap in heap ID map
  माप_प्रकार list_idx = (माप_प्रकार)heap->id % HEAP_ARRAY_SIZE;
  heap->next_heap = _memory_heaps[list_idx];
  _memory_heaps[list_idx] = heap;
}

स्थिर शून्य _rpmalloc_heap_orphan(heap_t *heap, पूर्णाङ्क first_class) {
  heap->owner_thread = (uintptr_t)-1;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  heap_t **heap_list =
      (first_class ? &_memory_first_class_orphan_heaps : &_memory_orphan_heaps);
#अन्यथा
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
  माप_प्रकार heap_size = परिमाण(heap_t);
  माप_प्रकार aligned_heap_size = 16 * ((heap_size + 15) / 16);
  माप_प्रकार request_heap_गिनती = 16;
  माप_प्रकार heap_span_गिनती = ((aligned_heap_size * request_heap_गिनती) +
                            परिमाण(span_t) + _memory_span_size - 1) /
                           _memory_span_size;
  माप_प्रकार block_size = _memory_span_size * heap_span_गिनती;
  माप_प्रकार span_गिनती = heap_span_गिनती;
  span_t *span = 0;
  // If there are global reserved spans, use these first
  यदि (memory_global_reserve_गिनती >= heap_span_गिनती) {
    span = _rpmalloc_global_get_reserved_spans(heap_span_गिनती);
  }
  यदि (!span) {
    यदि (_memory_page_size > block_size) {
      span_गिनती = _memory_page_size / _memory_span_size;
      block_size = _memory_page_size;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      माप_प्रकार possible_heap_गिनती =
          (block_size - परिमाण(span_t)) / aligned_heap_size;
      यदि (possible_heap_गिनती >= (request_heap_गिनती * 16))
        request_heap_गिनती *= 16;
      अन्यथा यदि (possible_heap_गिनती < request_heap_गिनती)
        request_heap_गिनती = possible_heap_गिनती;
      heap_span_गिनती = ((aligned_heap_size * request_heap_गिनती) +
                         परिमाण(span_t) + _memory_span_size - 1) /
                        _memory_span_size;
    }

    माप_प्रकार align_offset = 0;
    span = (span_t *)_rpmalloc_mmap(block_size, &align_offset);
    यदि (!span)
      प्रत्यागम 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    _rpmalloc_span_initialize(span, span_गिनती, heap_span_गिनती, align_offset);
  }

  माप_प्रकार remain_size = _memory_span_size - परिमाण(span_t);
  heap_t *heap = (heap_t *)pointer_offset(span, परिमाण(span_t));
  _rpmalloc_heap_initialize(heap);

  // Put extra heaps as orphans
  माप_प्रकार अंक_heaps = remain_size / aligned_heap_size;
  यदि (अंक_heaps < request_heap_गिनती)
    अंक_heaps = request_heap_गिनती;
  atomic_store32(&heap->child_गिनती, (int32_t)अंक_heaps - 1);
  heap_t *extra_heap = (heap_t *)pointer_offset(heap, aligned_heap_size);
  यावत् (अंक_heaps > 1) {
    _rpmalloc_heap_initialize(extra_heap);
    extra_heap->master_heap = heap;
    _rpmalloc_heap_orphan(extra_heap, 1);
    extra_heap = (heap_t *)pointer_offset(extra_heap, aligned_heap_size);
    --अंक_heaps;
  }

  यदि (span_गिनती > heap_span_गिनती) {
    // Cap reserved spans
    माप_प्रकार remain_गिनती = span_गिनती - heap_span_गिनती;
    माप_प्रकार reserve_गिनती =
        (remain_गिनती > memory_heap_reserve_गिनती ? memory_heap_reserve_गिनती
                                                   : remain_गिनती);
    span_t *remain_span =
        (span_t *)pointer_offset(span, heap_span_गिनती * _memory_span_size);
    _rpmalloc_heap_set_reserved_spans(heap, span, remain_span, reserve_गिनती);

    यदि (remain_गिनती > reserve_गिनती) {
      // Set to global reserved spans
      remain_span = (span_t *)pointer_offset(remain_span,
                                             reserve_गिनती * _memory_span_size);
      reserve_गिनती = remain_गिनती - reserve_गिनती;
      _rpmalloc_global_set_reserved_spans(span, remain_span, reserve_गिनती);
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
  यावत् (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
    _rpmalloc_spin();
  यदि (first_class == 0)
    heap = _rpmalloc_heap_extract_orphan(&_memory_orphan_heaps);
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  यदि (!heap)
    heap = _rpmalloc_heap_extract_orphan(&_memory_first_class_orphan_heaps);
#endif
  यदि (!heap)
    heap = _rpmalloc_heap_allocate_new();
  atomic_store32_release(&_memory_global_lock, 0);
  यदि (heap)
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  प्रत्यागम heap;
}

स्थिर शून्य _rpmalloc_heap_release(शून्य *heapptr, पूर्णाङ्क first_class,
                                   पूर्णाङ्क release_cache) {
  heap_t *heap = (heap_t *)heapptr;
  यदि (!heap)
    प्रत्यागम;
  // Release thread cache spans back to global cache
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  यदि (release_cache || heap->finalize) {
#यदि ENABLE_THREAD_CACHE
    प्रति (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
      span_cache_t *span_cache;
      यदि (!iclass)
        span_cache = &heap->span_cache;
      अन्यथा
        span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
      यदि (!span_cache->गिनती)
        अनुवर्त;
#यदि ENABLE_GLOBAL_CACHE
      यदि (heap->finalize) {
        प्रति (माप_प्रकार ispan = 0; ispan < span_cache->गिनती; ++ispan)
          _rpmalloc_span_unmap(span_cache->span[ispan]);
      } अन्यथा {
        _rpmalloc_stat_add64(&heap->thread_to_global, span_cache->गिनती *
                                                          (iclass + 1) *
                                                          _memory_span_size);
        _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                           span_cache->गिनती);
        rpmalloc_global_cache_जोड़ो_spans(span_cache->span, iclass + 1,
                                            span_cache->गिनती);
      }
#अन्यथा
      प्रति (माप_प्रकार ispan = 0; ispan < span_cache->गिनती; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
      span_cache->गिनती = 0;
    }
#endif
  }

  यदि (get_thread_heap_raw() == heap)
    set_thread_heap(0);

#यदि ENABLE_STATISTICS
  atomic_decr32(&_memory_active_heaps);
  rpmalloc_assert(atomic_load32(&_memory_active_heaps) >= 0,
                  "Still active heaps during finalization");
#endif

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  यदि (get_thread_id() != _rpmalloc_main_thread_id) {
    यावत् (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
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
  यदि (heap->spans_reserved) {
    span_t *span = _rpmalloc_span_map(heap, heap->spans_reserved);
    _rpmalloc_span_unmap(span);
    heap->spans_reserved = 0;
  }

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  प्रति (माप_प्रकार iclass = 0; iclass < SIZE_CLASS_गिनती; ++iclass) {
    यदि (heap->size_class[iclass].cache)
      _rpmalloc_span_unmap(heap->size_class[iclass].cache);
    heap->size_class[iclass].cache = 0;
    span_t *span = heap->size_class[iclass].partial_span;
    यावत् (span) {
      span_t *next = span->next;
      _rpmalloc_span_finalize(heap, iclass, span,
                              &heap->size_class[iclass].partial_span);
      span = next;
    }
    // If class still has a free list it must be a full span
    यदि (heap->size_class[iclass].free_list) {
      span_t *class_span =
          (span_t *)((uintptr_t)heap->size_class[iclass].free_list &
                     _memory_span_mask);
      span_t **list = 0;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
      list = &heap->full_span[iclass];
#endif
      --heap->full_span_गिनती;
      यदि (!_rpmalloc_span_finalize(heap, iclass, class_span, list)) {
        यदि (list)
          _rpmalloc_span_double_link_list_remove(list, class_span);
        _rpmalloc_span_double_link_list_add(
            &heap->size_class[iclass].partial_span, class_span);
      }
    }
  }

#यदि ENABLE_THREAD_CACHE
  प्रति (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    span_cache_t *span_cache;
    यदि (!iclass)
      span_cache = &heap->span_cache;
    अन्यथा
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    प्रति (माप_प्रकार ispan = 0; ispan < span_cache->गिनती; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->गिनती = 0;
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
स्थिर शून्य *free_list_निकालो(शून्य **list) {
  शून्य *block = *list;
  *list = *((शून्य **)block);
  प्रत्यागम block;
}

//! Allocate a small/medium sized memory block from the given heap
स्थिर शून्य *_rpmalloc_allocate_from_heap_fallback(
    heap_t *heap, heap_size_class_t *heap_size_class, uint32_t class_idx) {
  span_t *span = heap_size_class->partial_span;
  rpmalloc_assume(heap != 0);
  यदि (EXPECTED(span != 0)) {
    rpmalloc_assert(span->block_गिनती ==
                        _memory_size_class[span->size_class].block_गिनती,
                    "Span block count corrupted");
    rpmalloc_assert(!_rpmalloc_span_is_fully_utilized(span),
                    "Internal failure");
    शून्य *block;
    यदि (span->free_list) {
      // Span local free list is not empty, swap to size class free list
      block = free_list_निकालो(&span->free_list);
      heap_size_class->free_list = span->free_list;
      span->free_list = 0;
    } अन्यथा {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      शून्य *block_start = pointer_offset(
          span, SPAN_HEADER_SIZE +
                    ((माप_प्रकार)span->free_list_limit * span->block_size));
      span->free_list_limit += free_list_partial_init(
          &heap_size_class->free_list, &block,
          (शून्य *)((uintptr_t)block_start & ~(_memory_page_size - 1)),
          block_start, span->block_गिनती - span->free_list_limit,
          span->block_size);
    }
    rpmalloc_assert(span->free_list_limit <= span->block_गिनती,
                    "Span block count corrupted");
    span->used_गिनती = span->free_list_limit;

    // Swap in deferred free list if present
    यदि (atomic_load_ptr(&span->free_list_deferred))
      _rpmalloc_span_extract_free_list_deferred(span);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    यदि (!_rpmalloc_span_is_fully_utilized(span))
      प्रत्यागम block;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_span_double_link_list_निकालो_head(&heap_size_class->partial_span,
                                             span);
#यदि RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_गिनती;
    प्रत्यागम block;
  }

  // Find a span in one of the cache levels
  span = _rpmalloc_heap_extract_new_span(heap, heap_size_class, 1, class_idx);
  यदि (EXPECTED(span != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    प्रत्यागम _rpmalloc_span_initialize_new(heap, heap_size_class, span,
                                         class_idx);
  }

  प्रत्यागम 0;
}

//! Allocate a small sized memory block from the given heap
स्थिर शून्य *_rpmalloc_allocate_small(heap_t *heap, माप_प्रकार size) {
  rpmalloc_assert(heap, "No thread heap");
  // Small sizes have unique size classes
  ध्रुव uint32_t class_idx =
      (uint32_t)((size + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_SHIFT);
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  यदि (EXPECTED(heap_size_class->free_list != 0))
    प्रत्यागम free_list_निकालो(&heap_size_class->free_list);
  प्रत्यागम _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
स्थिर शून्य *_rpmalloc_allocate_medium(heap_t *heap, माप_प्रकार size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ध्रुव uint32_t base_idx =
      (uint32_t)(SMALL_CLASS_गिनती +
                 ((size - (SMALL_SIZE_LIMIT + 1)) >> MEDIUM_GRANULARITY_SHIFT));
  ध्रुव uint32_t class_idx = _memory_size_class[base_idx].class_idx;
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  यदि (EXPECTED(heap_size_class->free_list != 0))
    प्रत्यागम free_list_निकालो(&heap_size_class->free_list);
  प्रत्यागम _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
स्थिर शून्य *_rpmalloc_allocate_large(heap_t *heap, माप_प्रकार size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  size += SPAN_HEADER_SIZE;
  माप_प्रकार span_गिनती = size >> _memory_span_size_shift;
  यदि (size & (_memory_span_size - 1))
    ++span_गिनती;

  // Find a span in one of the cache levels
  span_t *span =
      _rpmalloc_heap_extract_new_span(heap, 0, span_गिनती, SIZE_CLASS_LARGE);
  यदि (!span)
    प्रत्यागम span;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(span->span_गिनती >= span_गिनती, "Internal failure");
  span->size_class = SIZE_CLASS_LARGE;
  span->heap = heap;

#यदि RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_गिनती;

  प्रत्यागम pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a huge block by mapping memory pages directly
स्थिर शून्य *_rpmalloc_allocate_huge(heap_t *heap, माप_प्रकार size) {
  rpmalloc_assert(heap, "No thread heap");
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  size += SPAN_HEADER_SIZE;
  माप_प्रकार अंक_pages = size >> _memory_page_size_shift;
  यदि (size & (_memory_page_size - 1))
    ++अंक_pages;
  माप_प्रकार align_offset = 0;
  span_t *span =
      (span_t *)_rpmalloc_mmap(अंक_pages * _memory_page_size, &align_offset);
  यदि (!span)
    प्रत्यागम span;

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_गिनती = (uint32_t)अंक_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, अंक_pages, _huge_pages_peak);

#यदि RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_गिनती;

  प्रत्यागम pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a block of the given size
स्थिर शून्य *_rpmalloc_allocate(heap_t *heap, माप_प्रकार size) {
  _rpmalloc_stat_add64(&_allocation_counter, 1);
  यदि (EXPECTED(size <= SMALL_SIZE_LIMIT))
    प्रत्यागम _rpmalloc_allocate_small(heap, size);
  अन्यथा यदि (size <= _memory_medium_size_limit)
    प्रत्यागम _rpmalloc_allocate_medium(heap, size);
  अन्यथा यदि (size <= LARGE_SIZE_LIMIT)
    प्रत्यागम _rpmalloc_allocate_large(heap, size);
  प्रत्यागम _rpmalloc_allocate_huge(heap, size);
}

स्थिर शून्य *_rpmalloc_aligned_allocate(heap_t *heap, माप_प्रकार alignment,
                                        माप_प्रकार size) {
  यदि (alignment <= SMALL_GRANULARITY)
    प्रत्यागम _rpmalloc_allocate(heap, size);

#यदि ENABLE_VALIDATE_ARGS
  यदि ((size + alignment) < size) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
  यदि (alignment & (alignment - 1)) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
#endif

  यदि ((alignment <= SPAN_HEADER_SIZE) &&
      ((size + SPAN_HEADER_SIZE) < _memory_medium_size_limit)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    माप_प्रकार multiple_size = size ? (size + (SPAN_HEADER_SIZE - 1)) &
                                      ~(uintptr_t)(SPAN_HEADER_SIZE - 1)
                                : SPAN_HEADER_SIZE;
    rpmalloc_assert(!(multiple_size % SPAN_HEADER_SIZE),
                    "Failed alignment calculation");
    यदि (multiple_size <= (size + alignment))
      प्रत्यागम _rpmalloc_allocate(heap, multiple_size);
  }

  शून्य *ptr = 0;
  माप_प्रकार align_mask = alignment - 1;
  यदि (alignment <= _memory_page_size) {
    ptr = _rpmalloc_allocate(heap, size + alignment);
    यदि ((uintptr_t)ptr & align_mask) {
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
  यदि (alignment & align_mask) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
  यदि (alignment >= _memory_span_size) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }

  माप_प्रकार extra_pages = alignment / _memory_page_size;

  // Since each span has a header, we will at least need one extra memory page
  माप_प्रकार अंक_pages = 1 + (size / _memory_page_size);
  यदि (size & (_memory_page_size - 1))
    ++अंक_pages;

  यदि (extra_pages > अंक_pages)
    अंक_pages = 1 + extra_pages;

  माप_प्रकार original_pages = अंक_pages;
  माप_प्रकार limit_pages = (_memory_span_size / _memory_page_size) * 2;
  यदि (limit_pages < (original_pages * 2))
    limit_pages = original_pages * 2;

  माप_प्रकार mapped_size, align_offset;
  span_t *span;

retry:
  align_offset = 0;
  mapped_size = अंक_pages * _memory_page_size;

  span = (span_t *)_rpmalloc_mmap(mapped_size, &align_offset);
  यदि (!span) {
    त्रुटि_सं = ENOMEM;
    प्रत्यागम 0;
  }
  ptr = pointer_offset(span, SPAN_HEADER_SIZE);

  यदि ((uintptr_t)ptr & align_mask)
    ptr = (शून्य *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);

  यदि (((माप_प्रकार)pointer_diff(ptr, span) >= _memory_span_size) ||
      (pointer_offset(ptr, size) > pointer_offset(span, mapped_size)) ||
      (((uintptr_t)ptr & _memory_span_mask) != (uintptr_t)span)) {
    _rpmalloc_unmap(span, mapped_size, align_offset, mapped_size);
    ++अंक_pages;
    यदि (अंक_pages > limit_pages) {
      त्रुटि_सं = EINVAL;
      प्रत्यागम 0;
    }
    गच्छ retry;
  }

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_गिनती = (uint32_t)अंक_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, अंक_pages, _huge_pages_peak);

#यदि RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_गिनती;

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
  यदि (UNEXPECTED(_rpmalloc_span_is_fully_utilized(span))) {
    span->used_गिनती = span->block_गिनती;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                           span);
#endif
    _rpmalloc_span_double_link_list_add(
        &heap->size_class[span->size_class].partial_span, span);
    --heap->full_span_गिनती;
  }
  *((शून्य **)block) = span->free_list;
  --span->used_गिनती;
  span->free_list = block;
  यदि (UNEXPECTED(span->used_गिनती == span->list_size)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    यदि (span->used_गिनती) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      शून्य *free_list;
      कुरु {
        free_list = atomic_exchange_ptr_acquire(&span->free_list_deferred,
                                                INVALID_POINTER);
      } यावत् (free_list == INVALID_POINTER);
      atomic_store_ptr_release(&span->free_list_deferred, free_list);
    }
    _rpmalloc_span_double_link_list_remove(
        &heap->size_class[span->size_class].partial_span, span);
    _rpmalloc_span_release_to_cache(heap, span);
  }
}

स्थिर शून्य _rpmalloc_deallocate_defer_free_span(heap_t *heap, span_t *span) {
  यदि (span->size_class != SIZE_CLASS_HUGE)
    _rpmalloc_stat_inc(&heap->span_use[span->span_गिनती - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  कुरु {
    span->free_list = (शून्य *)atomic_load_ptr(&heap->span_free_deferred);
  } यावत् (!atomic_cas_ptr(&heap->span_free_deferred, span, span->free_list));
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
  कुरु {
    free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } यावत् (free_list == INVALID_POINTER);
  *((शून्य **)block) = free_list;
  uint32_t free_गिनती = ++span->list_size;
  पूर्णाङ्क all_deferred_free = (free_गिनती == span->block_गिनती);
  atomic_store_ptr_release(&span->free_list_deferred, block);
  यदि (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
  }
}

स्थिर शून्य _rpmalloc_deallocate_small_or_medium(span_t *span, शून्य *p) {
  _rpmalloc_stat_inc_free(span->heap, span->size_class);
  यदि (span->flags & SPAN_FLAG_ALIGNED_BLOCKS) {
    // Realign pointer to block start
    शून्य *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
    p = pointer_offset(p, -(int32_t)(block_offset % span->block_size));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  पूर्णाङ्क defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#अन्यथा
  पूर्णाङ्क defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  यदि (!defer)
    _rpmalloc_deallocate_direct_small_or_medium(span, p);
  अन्यथा
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
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  पूर्णाङ्क defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#अन्यथा
  पूर्णाङ्क defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  यदि (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    प्रत्यागम;
  }
  rpmalloc_assert(span->heap->full_span_गिनती, "Heap span counter corrupted");
  --span->heap->full_span_गिनती;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif
#यदि ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  माप_प्रकार idx = span->span_गिनती - 1;
  atomic_decr32(&span->heap->span_use[idx].current);
#endif
  heap_t *heap = span->heap;
  rpmalloc_assert(heap, "No thread heap");
#यदि ENABLE_THREAD_CACHE
  ध्रुव पूर्णाङ्क set_as_reserved =
      ((span->span_गिनती > 1) && (heap->span_cache.गिनती == 0) &&
       !heap->finalize && !heap->spans_reserved);
#अन्यथा
  ध्रुव पूर्णाङ्क set_as_reserved =
      ((span->span_गिनती > 1) && !heap->finalize && !heap->spans_reserved);
#endif
  यदि (set_as_reserved) {
    heap->span_reserve = span;
    heap->spans_reserved = span->span_गिनती;
    यदि (span->flags & SPAN_FLAG_MASTER) {
      heap->span_reserve_master = span;
    } अन्यथा { // SPAN_FLAG_SUBSPAN
      span_t *master = (span_t *)pointer_offset(
          span,
          -(intptr_t)((माप_प्रकार)span->offset_from_master * _memory_span_size));
      heap->span_reserve_master = master;
      rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->remaining_spans) >=
                          (int32_t)span->span_गिनती,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&heap->span_use[idx].spans_to_reserved);
  } अन्यथा {
    // Insert into cache list
    rpmalloc_heap_cache_जोड़ो(heap, span);
  }
}

//! Deallocate the given huge span
स्थिर शून्य _rpmalloc_deallocate_huge(span_t *span) {
  rpmalloc_assert(span->heap, "No span heap");
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  पूर्णाङ्क defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#अन्यथा
  पूर्णाङ्क defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  यदि (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    प्रत्यागम;
  }
  rpmalloc_assert(span->heap->full_span_गिनती, "Heap span counter corrupted");
  --span->heap->full_span_गिनती;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif

  // Oversized allocation, page count is stored in span_count
  माप_प्रकार अंक_pages = span->span_गिनती;
  _rpmalloc_unmap(span, अंक_pages * _memory_page_size, span->align_offset,
                  अंक_pages * _memory_page_size);
  _rpmalloc_stat_sub(&_huge_pages_current, अंक_pages);
}

//! Deallocate the given block
स्थिर शून्य _rpmalloc_deallocate(शून्य *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  यदि (UNEXPECTED(!span))
    प्रत्यागम;
  यदि (EXPECTED(span->size_class < SIZE_CLASS_गिनती))
    _rpmalloc_deallocate_small_or_medium(span, p);
  अन्यथा यदि (span->size_class == SIZE_CLASS_LARGE)
    _rpmalloc_deallocate_large(span);
  अन्यथा
    _rpmalloc_deallocate_huge(span);
}

////////////
///
/// Reallocation entry points
///
//////

स्थिर माप_प्रकार _rpmalloc_usable_size(शून्य *p);

//! Reallocate the given block to the given size
स्थिर शून्य *_rpmalloc_reallocate(heap_t *heap, शून्य *p, माप_प्रकार size,
                                  माप_प्रकार oldsize, अचिन्हित पूर्णाङ्क flags) {
  यदि (p) {
    // Grab the span using guaranteed span alignment
    span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
    यदि (EXPECTED(span->size_class < SIZE_CLASS_गिनती)) {
      // Small/medium sized block
      rpmalloc_assert(span->span_गिनती == 1, "Span counter corrupted");
      शून्य *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
      uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
      uint32_t block_idx = block_offset / span->block_size;
      शून्य *block =
          pointer_offset(blocks_start, (माप_प्रकार)block_idx * span->block_size);
      यदि (!oldsize)
        oldsize =
            (माप_प्रकार)((सूचक_भेद_प्रकार)span->block_size - pointer_diff(p, block));
      यदि ((माप_प्रकार)span->block_size >= size) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        यदि ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          स_हटाओ(block, p, oldsize);
        प्रत्यागम block;
      }
    } अन्यथा यदि (span->size_class == SIZE_CLASS_LARGE) {
      // Large block
      माप_प्रकार total_size = size + SPAN_HEADER_SIZE;
      माप_प्रकार अंक_spans = total_size >> _memory_span_size_shift;
      यदि (total_size & (_memory_span_mask - 1))
        ++अंक_spans;
      माप_प्रकार current_spans = span->span_गिनती;
      शून्य *block = pointer_offset(span, SPAN_HEADER_SIZE);
      यदि (!oldsize)
        oldsize = (current_spans * _memory_span_size) -
                  (माप_प्रकार)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      यदि ((current_spans >= अंक_spans) && (total_size >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        यदि ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          स_हटाओ(block, p, oldsize);
        प्रत्यागम block;
      }
    } अन्यथा {
      // Oversized block
      माप_प्रकार total_size = size + SPAN_HEADER_SIZE;
      माप_प्रकार अंक_pages = total_size >> _memory_page_size_shift;
      यदि (total_size & (_memory_page_size - 1))
        ++अंक_pages;
      // Page count is stored in span_count
      माप_प्रकार current_pages = span->span_गिनती;
      शून्य *block = pointer_offset(span, SPAN_HEADER_SIZE);
      यदि (!oldsize)
        oldsize = (current_pages * _memory_page_size) -
                  (माप_प्रकार)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      यदि ((current_pages >= अंक_pages) && (अंक_pages >= (current_pages / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        यदि ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          स_हटाओ(block, p, oldsize);
        प्रत्यागम block;
      }
    }
  } अन्यथा {
    oldsize = 0;
  }

  यदि (!!(flags & RPMALLOC_GROW_OR_FAIL))
    प्रत्यागम 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  माप_प्रकार lower_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  माप_प्रकार new_size =
      (size > lower_bound) ? size : ((size > oldsize) ? lower_bound : size);
  शून्य *block = _rpmalloc_allocate(heap, new_size);
  यदि (p && block) {
    यदि (!(flags & RPMALLOC_NO_PRESERVE))
      स_नकल(block, p, oldsize < new_size ? oldsize : new_size);
    _rpmalloc_deallocate(p);
  }

  प्रत्यागम block;
}

स्थिर शून्य *_rpmalloc_aligned_reallocate(heap_t *heap, शून्य *ptr,
                                          माप_प्रकार alignment, माप_प्रकार size,
                                          माप_प्रकार oldsize, अचिन्हित पूर्णाङ्क flags) {
  यदि (alignment <= SMALL_GRANULARITY)
    प्रत्यागम _rpmalloc_reallocate(heap, ptr, size, oldsize, flags);

  पूर्णाङ्क no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  माप_प्रकार usablesize = (ptr ? _rpmalloc_usable_size(ptr) : 0);
  यदि ((usablesize >= size) && !((uintptr_t)ptr & (alignment - 1))) {
    यदि (no_alloc || (size >= (usablesize / 2)))
      प्रत्यागम ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  शून्य *block =
      (!no_alloc ? _rpmalloc_aligned_allocate(heap, alignment, size) : 0);
  यदि (EXPECTED(block != 0)) {
    यदि (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      यदि (!oldsize)
        oldsize = usablesize;
      स_नकल(block, ptr, oldsize < size ? oldsize : size);
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
स्थिर माप_प्रकार _rpmalloc_usable_size(शून्य *p) {
  // Grab the span using guaranteed span alignment
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  यदि (span->size_class < SIZE_CLASS_गिनती) {
    // Small/medium block
    शून्य *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    प्रत्यागम span->block_size -
           ((माप_प्रकार)pointer_diff(p, blocks_start) % span->block_size);
  }
  यदि (span->size_class == SIZE_CLASS_LARGE) {
    // Large block
    माप_प्रकार current_spans = span->span_गिनती;
    प्रत्यागम (current_spans * _memory_span_size) - (माप_प्रकार)pointer_diff(p, span);
  }
  // Oversized block, page count is stored in span_count
  माप_प्रकार current_pages = span->span_गिनती;
  प्रत्यागम (current_pages * _memory_page_size) - (माप_प्रकार)pointer_diff(p, span);
}

//! Adjust and optimize the size class properties for the given class
स्थिर शून्य _rpmalloc_adjust_size_class(माप_प्रकार iclass) {
  माप_प्रकार block_size = _memory_size_class[iclass].block_size;
  माप_प्रकार block_गिनती = (_memory_span_size - SPAN_HEADER_SIZE) / block_size;

  _memory_size_class[iclass].block_गिनती = (uint16_t)block_गिनती;
  _memory_size_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  यदि (iclass >= SMALL_CLASS_गिनती) {
    माप_प्रकार prevclass = iclass;
    यावत् (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      यदि (_memory_size_class[prevclass].block_गिनती ==
          _memory_size_class[iclass].block_गिनती)
        _rpmalloc_memcpy_const(_memory_size_class + prevclass,
                               _memory_size_class + iclass,
                               परिमाण(_memory_size_class[iclass]));
      अन्यथा
        भङ्ग;
    }
  }
}

//! Initialize the allocator and setup global data
बाह्य अन्तःस्थ पूर्णाङ्क rpmalloc_initialize(शून्य) {
  यदि (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    प्रत्यागम 0;
  }
  प्रत्यागम rpmalloc_initialize_config(0);
}

पूर्णाङ्क rpmalloc_initialize_config(ध्रुव rpmalloc_config_t *config) {
  यदि (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    प्रत्यागम 0;
  }
  _rpmalloc_initialized = 1;

  यदि (config)
    स_नकल(&_memory_config, config, परिमाण(rpmalloc_config_t));
  अन्यथा
    _rpmalloc_memset_const(&_memory_config, 0, परिमाण(rpmalloc_config_t));

  यदि (!_memory_config.memory_map || !_memory_config.memory_unmap) {
    _memory_config.memory_map = _rpmalloc_mmap_os;
    _memory_config.memory_unmap = _rpmalloc_unmap_os;
  }

#यदि PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  स_रखो(&system_info, 0, परिमाण(system_info));
  GetSystemInfo(&system_info);
  _memory_map_granularity = system_info.dwAllocationGranularity;
#अन्यथा
  _memory_map_granularity = (माप_प्रकार)sysconf(_SC_PAGESIZE);
#endif

#यदि RPMALLOC_CONFIGURABLE
  _memory_page_size = _memory_config.page_size;
#अन्यथा
  _memory_page_size = 0;
#endif
  _memory_huge_pages = 0;
  यदि (!_memory_page_size) {
#यदि PLATFORM_WINDOWS
    _memory_page_size = system_info.dwPageSize;
#अन्यथा
    _memory_page_size = _memory_map_granularity;
    यदि (_memory_config.enable_huge_pages) {
#यदि defined(__linux__)
      माप_प्रकार huge_page_size = 0;
      खाता *meminfo = ख_खोलो("/proc/meminfo", "r");
      यदि (meminfo) {
        वर्ण पंक्ति_2[128];
        यावत् (!huge_page_size && ख_माला_लो(पंक्ति_2, परिमाण(पंक्ति_2) - 1, meminfo)) {
          पंक्ति_2[परिमाण(पंक्ति_2) - 1] = 0;
          यदि (म_माला(पंक्ति_2, "Hugepagesize:"))
            huge_page_size = (माप_प्रकार)म_से_दीर्घ(पंक्ति_2 + 13, 0, 10) * 1024;
        }
        ख_बंद(meminfo);
      }
      यदि (huge_page_size) {
        _memory_huge_pages = 1;
        _memory_page_size = huge_page_size;
        _memory_map_granularity = huge_page_size;
      }
#नोचेद्यदि defined(__FreeBSD__)
      पूर्णाङ्क rc;
      माप_प्रकार sz = परिमाण(rc);

      यदि (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, रिक्त, 0) == 0 &&
          rc == 1) {
        स्थिर माप_प्रकार defsize = 2 * 1024 * 1024;
        पूर्णाङ्क nsize = 0;
        माप_प्रकार sizes[4] = {0};
        _memory_huge_pages = 1;
        _memory_page_size = defsize;
        यदि ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          प्रति (माप_प्रकार csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            यदि (defsize < csize) {
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
  } अन्यथा {
    यदि (_memory_config.enable_huge_pages)
      _memory_huge_pages = 1;
  }

#यदि PLATFORM_WINDOWS
  यदि (_memory_config.enable_huge_pages) {
    HANDLE token = 0;
    माप_प्रकार large_page_minimum = GetLargePageMinimum();
    यदि (large_page_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    यदि (token) {
      LUID luid;
      यदि (Lookup_Privilege_मूल्य(0, SE_LOCK_MEMORY_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        स_रखो(&token_privileges, 0, परिमाण(token_privileges));
        token_privileges.Privilege_गिनती = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        यदि (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          यदि (GetLastError() == ERROR_SUCCESS)
            _memory_huge_pages = 1;
        }
      }
      CloseHandle(token);
    }
    यदि (_memory_huge_pages) {
      यदि (large_page_minimum > _memory_page_size)
        _memory_page_size = large_page_minimum;
      यदि (large_page_minimum > _memory_map_granularity)
        _memory_map_granularity = large_page_minimum;
    }
  }
#endif

  माप_प्रकार min_span_size = 256;
  माप_प्रकार max_page_size;
#यदि UINTPTR_MAX > 0xFFFFFFFF
  max_page_size = 4096ULL * 1024ULL * 1024ULL;
#अन्यथा
  max_page_size = 4 * 1024 * 1024;
#endif
  यदि (_memory_page_size < min_span_size)
    _memory_page_size = min_span_size;
  यदि (_memory_page_size > max_page_size)
    _memory_page_size = max_page_size;
  _memory_page_size_shift = 0;
  माप_प्रकार page_size_bit = _memory_page_size;
  यावत् (page_size_bit != 1) {
    ++_memory_page_size_shift;
    page_size_bit >>= 1;
  }
  _memory_page_size = ((माप_प्रकार)1 << _memory_page_size_shift);

#यदि RPMALLOC_CONFIGURABLE
  यदि (!_memory_config.span_size) {
    _memory_span_size = _memory_default_span_size;
    _memory_span_size_shift = _memory_default_span_size_shift;
    _memory_span_mask = _memory_default_span_mask;
  } अन्यथा {
    माप_प्रकार span_size = _memory_config.span_size;
    यदि (span_size > (256 * 1024))
      span_size = (256 * 1024);
    _memory_span_size = 4096;
    _memory_span_size_shift = 12;
    यावत् (_memory_span_size < span_size) {
      _memory_span_size <<= 1;
      ++_memory_span_size_shift;
    }
    _memory_span_mask = ~(uintptr_t)(_memory_span_size - 1);
  }
#endif

  memory_span_map_गिनती =
      (_memory_config.span_map_गिनती ? _memory_config.span_map_गिनती
                                     : DEFAULT_SPAN_MAP_गिनती);
  यदि ((_memory_span_size * memory_span_map_गिनती) < _memory_page_size)
    memory_span_map_गिनती = (_memory_page_size / _memory_span_size);
  यदि ((_memory_page_size >= _memory_span_size) &&
      ((memory_span_map_गिनती * _memory_span_size) % _memory_page_size))
    memory_span_map_गिनती = (_memory_page_size / _memory_span_size);
  memory_heap_reserve_गिनती = (memory_span_map_गिनती > DEFAULT_SPAN_MAP_गिनती)
                                   ? DEFAULT_SPAN_MAP_गिनती
                                   : memory_span_map_गिनती;

  _memory_config.page_size = _memory_page_size;
  _memory_config.span_size = _memory_span_size;
  _memory_config.span_map_गिनती = memory_span_map_गिनती;
  _memory_config.enable_huge_pages = _memory_huge_pages;

#यदि ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  यदि (pthread_कुंजी_create(&_memory_thread_heap, _rpmalloc_heap_release_raw_fc))
    प्रत्यागम -1;
#endif
#यदि defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_कुंजी = FlsAlloc(&_rpmalloc_thread_destructor);
#endif

  // Setup all small and medium size classes
  माप_प्रकार iclass = 0;
  _memory_size_class[iclass].block_size = SMALL_GRANULARITY;
  _rpmalloc_adjust_size_class(iclass);
  प्रति (iclass = 1; iclass < SMALL_CLASS_गिनती; ++iclass) {
    माप_प्रकार size = iclass * SMALL_GRANULARITY;
    _memory_size_class[iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  _memory_medium_size_limit = (_memory_span_size - SPAN_HEADER_SIZE) >> 1;
  यदि (_memory_medium_size_limit > MEDIUM_SIZE_LIMIT)
    _memory_medium_size_limit = MEDIUM_SIZE_LIMIT;
  प्रति (iclass = 0; iclass < MEDIUM_CLASS_गिनती; ++iclass) {
    माप_प्रकार size = SMALL_SIZE_LIMIT + ((iclass + 1) * MEDIUM_GRANULARITY);
    यदि (size > _memory_medium_size_limit) {
      _memory_medium_size_limit =
          SMALL_SIZE_LIMIT + (iclass * MEDIUM_GRANULARITY);
      भङ्ग;
    }
    _memory_size_class[SMALL_CLASS_गिनती + iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(SMALL_CLASS_गिनती + iclass);
  }

  _memory_orphan_heaps = 0;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  _memory_first_class_orphan_heaps = 0;
#endif
#यदि ENABLE_STATISTICS
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
  स_रखो(_memory_heaps, 0, परिमाण(_memory_heaps));
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

  यदि (_memory_global_reserve) {
    atomic_add32(&_memory_global_reserve_master->remaining_spans,
                 -(int32_t)memory_global_reserve_गिनती);
    _memory_global_reserve_master = 0;
    memory_global_reserve_गिनती = 0;
    _memory_global_reserve = 0;
  }
  atomic_store32_release(&_memory_global_lock, 0);

  // Free all thread caches and fully free spans
  प्रति (माप_प्रकार list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    यावत् (heap) {
      heap_t *next_heap = heap->next_heap;
      heap->finalize = 1;
      _rpmalloc_heap_global_finalize(heap);
      heap = next_heap;
    }
  }

#यदि ENABLE_GLOBAL_CACHE
  // Free global caches
  प्रति (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass)
    _rpmalloc_global_cache_finalize(&_memory_span_cache[iclass]);
#endif

#यदि (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_कुंजी_delete(_memory_thread_heap);
#endif
#यदि defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_कुंजी);
  fls_कुंजी = 0;
#endif
#यदि ENABLE_STATISTICS
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
  यदि (!get_thread_heap_raw()) {
    heap_t *heap = _rpmalloc_heap_allocate(0);
    यदि (heap) {
      _rpmalloc_stat_inc(&_memory_active_heaps);
      set_thread_heap(heap);
#यदि defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_Set_मूल्य(fls_कुंजी, heap);
#endif
    }
  }
}

//! Finalize thread, orphan heap
शून्य rpmalloc_thread_finalize(पूर्णाङ्क release_caches) {
  heap_t *heap = get_thread_heap_raw();
  यदि (heap)
    _rpmalloc_heap_release_raw(heap, release_caches);
  set_thread_heap(0);
#यदि defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_Set_मूल्य(fls_कुंजी, 0);
#endif
}

पूर्णाङ्क rpmalloc_is_thread_initialized(शून्य) {
  प्रत्यागम (get_thread_heap_raw() != 0) ? 1 : 0;
}

ध्रुव rpmalloc_config_t *rpmalloc_config(शून्य) { प्रत्यागम &_memory_config; }

// Extern interface

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *rpmalloc(माप_प्रकार size) {
#यदि ENABLE_VALIDATE_ARGS
  यदि (size >= MAX_ALLOC_SIZE) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  प्रत्यागम _rpmalloc_allocate(heap, size);
}

बाह्य अन्तःस्थ शून्य rpfree(शून्य *ptr) { _rpmalloc_deallocate(ptr); }

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *rpcalloc(माप_प्रकार अंक_2, माप_प्रकार size) {
  माप_प्रकार total;
#यदि ENABLE_VALIDATE_ARGS
#यदि PLATFORM_WINDOWS
  पूर्णाङ्क err = SizeTMult(अंक_2, size, &total);
  यदि ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
#अन्यथा
  पूर्णाङ्क err = __builtin_umull_overflow(अंक_2, size, &total);
  यदि (err || (total >= MAX_ALLOC_SIZE)) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
#endif
#अन्यथा
  total = अंक_2 * size;
#endif
  heap_t *heap = get_thread_heap();
  शून्य *block = _rpmalloc_allocate(heap, total);
  यदि (block)
    स_रखो(block, 0, total);
  प्रत्यागम block;
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *rprealloc(शून्य *ptr, माप_प्रकार size) {
#यदि ENABLE_VALIDATE_ARGS
  यदि (size >= MAX_ALLOC_SIZE) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम ptr;
  }
#endif
  heap_t *heap = get_thread_heap();
  प्रत्यागम _rpmalloc_reallocate(heap, ptr, size, 0, 0);
}

बाह्य RPMALLOC_ALLOCATOR शून्य *rpaligned_realloc(शून्य *ptr, माप_प्रकार alignment,
                                                  माप_प्रकार size, माप_प्रकार oldsize,
                                                  अचिन्हित पूर्णाङ्क flags) {
#यदि ENABLE_VALIDATE_ARGS
  यदि ((size + alignment < size) || (alignment > _memory_page_size)) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  प्रत्यागम _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, oldsize,
                                      flags);
}

बाह्य RPMALLOC_ALLOCATOR शून्य *rpaligned_alloc(माप_प्रकार alignment, माप_प्रकार size) {
  heap_t *heap = get_thread_heap();
  प्रत्यागम _rpmalloc_aligned_allocate(heap, alignment, size);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpaligned_calloc(माप_प्रकार alignment, माप_प्रकार अंक_2, माप_प्रकार size) {
  माप_प्रकार total;
#यदि ENABLE_VALIDATE_ARGS
#यदि PLATFORM_WINDOWS
  पूर्णाङ्क err = SizeTMult(अंक_2, size, &total);
  यदि ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
#अन्यथा
  पूर्णाङ्क err = __builtin_umull_overflow(अंक_2, size, &total);
  यदि (err || (total >= MAX_ALLOC_SIZE)) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
#endif
#अन्यथा
  total = अंक_2 * size;
#endif
  शून्य *block = rpaligned_alloc(alignment, total);
  यदि (block)
    स_रखो(block, 0, total);
  प्रत्यागम block;
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *rpmemalign(माप_प्रकार alignment,
                                                  माप_प्रकार size) {
  प्रत्यागम rpaligned_alloc(alignment, size);
}

बाह्य अन्तःस्थ पूर्णाङ्क rpposix_memalign(शून्य **memptr, माप_प्रकार alignment,
                                   माप_प्रकार size) {
  यदि (memptr)
    *memptr = rpaligned_alloc(alignment, size);
  अन्यथा
    प्रत्यागम EINVAL;
  प्रत्यागम *memptr ? 0 : ENOMEM;
}

बाह्य अन्तःस्थ माप_प्रकार rpmalloc_usable_size(शून्य *ptr) {
  प्रत्यागम (ptr ? _rpmalloc_usable_size(ptr) : 0);
}

बाह्य अन्तःस्थ शून्य rpmalloc_thread_collect(शून्य) {}

शून्य rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  स_रखो(stats, 0, परिमाण(rpmalloc_thread_statistics_t));
  heap_t *heap = get_thread_heap_raw();
  यदि (!heap)
    प्रत्यागम;

  प्रति (माप_प्रकार iclass = 0; iclass < SIZE_CLASS_गिनती; ++iclass) {
    size_class_t *size_class = _memory_size_class + iclass;
    span_t *span = heap->size_class[iclass].partial_span;
    यावत् (span) {
      माप_प्रकार free_गिनती = span->list_size;
      माप_प्रकार block_गिनती = size_class->block_गिनती;
      यदि (span->free_list_limit < block_गिनती)
        block_गिनती = span->free_list_limit;
      free_गिनती += (block_गिनती - span->used_गिनती);
      stats->sizecache += free_गिनती * size_class->block_size;
      span = span->next;
    }
  }

#यदि ENABLE_THREAD_CACHE
  प्रति (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    span_cache_t *span_cache;
    यदि (!iclass)
      span_cache = &heap->span_cache;
    अन्यथा
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    stats->spancache += span_cache->गिनती * (iclass + 1) * _memory_span_size;
  }
#endif

  span_t *deferred = (span_t *)atomic_load_ptr(&heap->span_free_deferred);
  यावत् (deferred) {
    यदि (deferred->size_class != SIZE_CLASS_HUGE)
      stats->spancache += (माप_प्रकार)deferred->span_गिनती * _memory_span_size;
    deferred = (span_t *)deferred->free_list;
  }

#यदि ENABLE_STATISTICS
  stats->thread_to_global = (माप_प्रकार)atomic_load64(&heap->thread_to_global);
  stats->global_to_thread = (माप_प्रकार)atomic_load64(&heap->global_to_thread);

  प्रति (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    stats->span_use[iclass].current =
        (माप_प्रकार)atomic_load32(&heap->span_use[iclass].current);
    stats->span_use[iclass].peak =
        (माप_प्रकार)atomic_load32(&heap->span_use[iclass].ऊपरी);
    stats->span_use[iclass].to_global =
        (माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_to_global);
    stats->span_use[iclass].from_global =
        (माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_from_global);
    stats->span_use[iclass].to_cache =
        (माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_to_cache);
    stats->span_use[iclass].from_cache =
        (माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_from_cache);
    stats->span_use[iclass].to_reserved =
        (माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_to_reserved);
    stats->span_use[iclass].from_reserved =
        (माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_from_reserved);
    stats->span_use[iclass].map_calls =
        (माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_map_calls);
  }
  प्रति (माप_प्रकार iclass = 0; iclass < SIZE_CLASS_गिनती; ++iclass) {
    stats->size_use[iclass].alloc_current =
        (माप_प्रकार)atomic_load32(&heap->size_class_use[iclass].alloc_current);
    stats->size_use[iclass].alloc_peak =
        (माप_प्रकार)heap->size_class_use[iclass].alloc_peak;
    stats->size_use[iclass].alloc_total =
        (माप_प्रकार)atomic_load32(&heap->size_class_use[iclass].alloc_total);
    stats->size_use[iclass].free_total =
        (माप_प्रकार)atomic_load32(&heap->size_class_use[iclass].free_total);
    stats->size_use[iclass].spans_to_cache =
        (माप_प्रकार)atomic_load32(&heap->size_class_use[iclass].spans_to_cache);
    stats->size_use[iclass].spans_from_cache =
        (माप_प्रकार)atomic_load32(&heap->size_class_use[iclass].spans_from_cache);
    stats->size_use[iclass].spans_from_reserved = (माप_प्रकार)atomic_load32(
        &heap->size_class_use[iclass].spans_from_reserved);
    stats->size_use[iclass].map_calls =
        (माप_प्रकार)atomic_load32(&heap->size_class_use[iclass].spans_map_calls);
  }
#endif
}

शून्य rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  स_रखो(stats, 0, परिमाण(rpmalloc_global_statistics_t));
#यदि ENABLE_STATISTICS
  stats->mapped = (माप_प्रकार)atomic_load32(&_mapped_pages) * _memory_page_size;
  stats->mapped_peak = (माप_प्रकार)_mapped_pages_peak * _memory_page_size;
  stats->mapped_total =
      (माप_प्रकार)atomic_load32(&_mapped_total) * _memory_page_size;
  stats->unmapped_total =
      (माप_प्रकार)atomic_load32(&_unmapped_total) * _memory_page_size;
  stats->huge_alloc =
      (माप_प्रकार)atomic_load32(&_huge_pages_current) * _memory_page_size;
  stats->huge_alloc_peak = (माप_प्रकार)_huge_pages_peak * _memory_page_size;
#endif
#यदि ENABLE_GLOBAL_CACHE
  प्रति (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    global_cache_t *cache = &_memory_span_cache[iclass];
    यावत् (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();
    uint32_t गिनती = cache->गिनती;
#यदि ENABLE_UNLIMITED_CACHE
    span_t *current_span = cache->overflow;
    यावत् (current_span) {
      ++गिनती;
      current_span = current_span->next;
    }
#endif
    atomic_store32_release(&cache->lock, 0);
    stats->cached += गिनती * (iclass + 1) * _memory_span_size;
  }
#endif
}

#यदि ENABLE_STATISTICS

स्थिर शून्य _memory_heap_dump_statistics(heap_t *heap, शून्य *file) {
  ख_लिखो(file, "Heap %d stats:\n", heap->id);
  ख_लिखो(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  प्रति (माप_प्रकार iclass = 0; iclass < SIZE_CLASS_गिनती; ++iclass) {
    यदि (!atomic_load32(&heap->size_class_use[iclass].alloc_total))
      अनुवर्त;
    ख_लिखो(
        file,
        "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu "
        "%9u\n",
        (uint32_t)iclass,
        atomic_load32(&heap->size_class_use[iclass].alloc_current),
        heap->size_class_use[iclass].alloc_peak,
        atomic_load32(&heap->size_class_use[iclass].alloc_total),
        atomic_load32(&heap->size_class_use[iclass].free_total),
        _memory_size_class[iclass].block_size,
        _memory_size_class[iclass].block_गिनती,
        atomic_load32(&heap->size_class_use[iclass].spans_current),
        heap->size_class_use[iclass].spans_peak,
        ((माप_प्रकार)heap->size_class_use[iclass].alloc_peak *
         (माप_प्रकार)_memory_size_class[iclass].block_size) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(&heap->size_class_use[iclass].spans_to_cache) *
         _memory_span_size) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(&heap->size_class_use[iclass].spans_from_cache) *
         _memory_span_size) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(
             &heap->size_class_use[iclass].spans_from_reserved) *
         _memory_span_size) /
            (माप_प्रकार)(1024 * 1024),
        atomic_load32(&heap->size_class_use[iclass].spans_map_calls));
  }
  ख_लिखो(file, "Spans  Current     Peak Deferred  PeakMiB  Cached  ToCacheMiB "
                "FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB "
                "FromGlobalMiB  MmapCalls\n");
  प्रति (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    यदि (!atomic_load32(&heap->span_use[iclass].ऊपरी) &&
        !atomic_load32(&heap->span_use[iclass].spans_map_calls))
      अनुवर्त;
    ख_लिखो(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&heap->span_use[iclass].current),
        atomic_load32(&heap->span_use[iclass].ऊपरी),
        atomic_load32(&heap->span_use[iclass].spans_deferred),
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].ऊपरी) *
         (माप_प्रकार)_memory_span_size * (iclass + 1)) /
            (माप_प्रकार)(1024 * 1024),
#यदि ENABLE_THREAD_CACHE
        (अचिन्हित पूर्णाङ्क)(!iclass ? heap->span_cache.गिनती
                               : heap->span_large_cache[iclass - 1].गिनती),
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_to_cache) *
         (iclass + 1) * _memory_span_size) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_from_cache) *
         (iclass + 1) * _memory_span_size) /
            (माप_प्रकार)(1024 * 1024),
#अन्यथा
        0, (माप_प्रकार)0, (माप_प्रकार)0,
#endif
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_to_reserved) *
         (iclass + 1) * _memory_span_size) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_from_reserved) *
         (iclass + 1) * _memory_span_size) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_to_global) *
         (माप_प्रकार)_memory_span_size * (iclass + 1)) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_from_global) *
         (माप_प्रकार)_memory_span_size * (iclass + 1)) /
            (माप_प्रकार)(1024 * 1024),
        atomic_load32(&heap->span_use[iclass].spans_map_calls));
  }
  ख_लिखो(file, "Full spans: %zu\n", heap->full_span_गिनती);
  ख_लिखो(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  ख_लिखो(
      file, "%17zu %17zu\n",
      (माप_प्रकार)atomic_load64(&heap->thread_to_global) / (माप_प्रकार)(1024 * 1024),
      (माप_प्रकार)atomic_load64(&heap->global_to_thread) / (माप_प्रकार)(1024 * 1024));
}

#endif

शून्य rpmalloc_dump_statistics(शून्य *file) {
#यदि ENABLE_STATISTICS
  प्रति (माप_प्रकार list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    यावत् (heap) {
      पूर्णाङ्क need_dump = 0;
      प्रति (माप_प्रकार iclass = 0; !need_dump && (iclass < SIZE_CLASS_गिनती);
           ++iclass) {
        यदि (!atomic_load32(&heap->size_class_use[iclass].alloc_total)) {
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
      प्रति (माप_प्रकार iclass = 0; !need_dump && (iclass < LARGE_CLASS_गिनती);
           ++iclass) {
        यदि (!atomic_load32(&heap->span_use[iclass].ऊपरी) &&
            !atomic_load32(&heap->span_use[iclass].spans_map_calls))
          अनुवर्त;
        need_dump = 1;
      }
      यदि (need_dump)
        _memory_heap_dump_statistics(heap, file);
      heap = heap->next_heap;
    }
  }
  ख_लिखो(file, "Global stats:\n");
  माप_प्रकार huge_current =
      (माप_प्रकार)atomic_load32(&_huge_pages_current) * _memory_page_size;
  माप_प्रकार huge_peak = (माप_प्रकार)_huge_pages_peak * _memory_page_size;
  ख_लिखो(file, "HugeCurrentMiB HugePeakMiB\n");
  ख_लिखो(file, "%14zu %11zu\n", huge_current / (माप_प्रकार)(1024 * 1024),
          huge_peak / (माप_प्रकार)(1024 * 1024));

#यदि ENABLE_GLOBAL_CACHE
  ख_लिखो(file, "GlobalCacheMiB\n");
  प्रति (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    global_cache_t *cache = _memory_span_cache + iclass;
    माप_प्रकार global_cache = (माप_प्रकार)cache->गिनती * iclass * _memory_span_size;

    माप_प्रकार global_overflow_cache = 0;
    span_t *span = cache->overflow;
    यावत् (span) {
      global_overflow_cache += iclass * _memory_span_size;
      span = span->next;
    }
    यदि (global_cache || global_overflow_cache || cache->जोड़ो_गिनती ||
        cache->extract_गिनती)
      ख_लिखो(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (माप_प्रकार)(1024 * 1024),
              global_overflow_cache / (माप_प्रकार)(1024 * 1024),
              cache->जोड़ो_गिनती, cache->extract_गिनती);
  }
#endif

  माप_प्रकार mapped = (माप_प्रकार)atomic_load32(&_mapped_pages) * _memory_page_size;
  माप_प्रकार mapped_os =
      (माप_प्रकार)atomic_load32(&_mapped_pages_os) * _memory_page_size;
  माप_प्रकार mapped_peak = (माप_प्रकार)_mapped_pages_peak * _memory_page_size;
  माप_प्रकार mapped_total =
      (माप_प्रकार)atomic_load32(&_mapped_total) * _memory_page_size;
  माप_प्रकार unmapped_total =
      (माप_प्रकार)atomic_load32(&_unmapped_total) * _memory_page_size;
  ख_लिखो(
      file,
      "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB\n");
  ख_लिखो(file, "%9zu %11zu %13zu %14zu %16zu\n",
          mapped / (माप_प्रकार)(1024 * 1024), mapped_os / (माप_प्रकार)(1024 * 1024),
          mapped_peak / (माप_प्रकार)(1024 * 1024),
          mapped_total / (माप_प्रकार)(1024 * 1024),
          unmapped_total / (माप_प्रकार)(1024 * 1024));

  ख_लिखो(file, "\n");
#यदि 0
	int64_t allocated = atomic_load64(&_allocation_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	ख_लिखो(file, "Allocation count: %lli\n", allocated);
	ख_लिखो(file, "Deallocation count: %lli\n", deallocated);
	ख_लिखो(file, "Current allocations: %lli\n", (allocated - deallocated));
	ख_लिखो(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	ख_लिखो(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#endif
#endif
  (शून्य)परिमाण(file);
}

#यदि RPMALLOC_FIRST_CLASS_HEAPS

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
  यदि (heap)
    _rpmalloc_heap_release(heap, 1, 1);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, माप_प्रकार size) {
#यदि ENABLE_VALIDATE_ARGS
  यदि (size >= MAX_ALLOC_SIZE) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
#endif
  प्रत्यागम _rpmalloc_allocate(heap, size);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, माप_प्रकार alignment,
                            माप_प्रकार size) {
#यदि ENABLE_VALIDATE_ARGS
  यदि (size >= MAX_ALLOC_SIZE) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
#endif
  प्रत्यागम _rpmalloc_aligned_allocate(heap, alignment, size);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, माप_प्रकार अंक_2, माप_प्रकार size) {
  प्रत्यागम rpmalloc_heap_aligned_calloc(heap, 0, अंक_2, size);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, माप_प्रकार alignment,
                             माप_प्रकार अंक_2, माप_प्रकार size) {
  माप_प्रकार total;
#यदि ENABLE_VALIDATE_ARGS
#यदि PLATFORM_WINDOWS
  पूर्णाङ्क err = SizeTMult(अंक_2, size, &total);
  यदि ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
#अन्यथा
  पूर्णाङ्क err = __builtin_umull_overflow(अंक_2, size, &total);
  यदि (err || (total >= MAX_ALLOC_SIZE)) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम 0;
  }
#endif
#अन्यथा
  total = अंक_2 * size;
#endif
  शून्य *block = _rpmalloc_aligned_allocate(heap, alignment, total);
  यदि (block)
    स_रखो(block, 0, total);
  प्रत्यागम block;
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, शून्य *ptr, माप_प्रकार size,
                      अचिन्हित पूर्णाङ्क flags) {
#यदि ENABLE_VALIDATE_ARGS
  यदि (size >= MAX_ALLOC_SIZE) {
    त्रुटि_सं = EINVAL;
    प्रत्यागम ptr;
  }
#endif
  प्रत्यागम _rpmalloc_reallocate(heap, ptr, size, 0, flags);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_aligned_realloc(rpmalloc_heap_t *heap, शून्य *ptr,
                              माप_प्रकार alignment, माप_प्रकार size,
                              अचिन्हित पूर्णाङ्क flags) {
#यदि ENABLE_VALIDATE_ARGS
  यदि ((size + alignment < size) || (alignment > _memory_page_size)) {
    त्रुटि_सं = EINVAL;
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

  प्रति (माप_प्रकार iclass = 0; iclass < SIZE_CLASS_गिनती; ++iclass) {
    span = heap->size_class[iclass].partial_span;
    यावत् (span) {
      next_span = span->next;
      rpmalloc_heap_cache_जोड़ो(heap, span);
      span = next_span;
    }
    heap->size_class[iclass].partial_span = 0;
    span = heap->full_span[iclass];
    यावत् (span) {
      next_span = span->next;
      rpmalloc_heap_cache_जोड़ो(heap, span);
      span = next_span;
    }

    span = heap->size_class[iclass].cache;
    यदि (span)
      rpmalloc_heap_cache_जोड़ो(heap, span);
    heap->size_class[iclass].cache = 0;
  }
  स_रखो(heap->size_class, 0, परिमाण(heap->size_class));
  स_रखो(heap->full_span, 0, परिमाण(heap->full_span));

  span = heap->large_huge_span;
  यावत् (span) {
    next_span = span->next;
    यदि (UNEXPECTED(span->size_class == SIZE_CLASS_HUGE))
      _rpmalloc_deallocate_huge(span);
    अन्यथा
      rpmalloc_heap_cache_जोड़ो(heap, span);
    span = next_span;
  }
  heap->large_huge_span = 0;
  heap->full_span_गिनती = 0;

#यदि ENABLE_THREAD_CACHE
  प्रति (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    span_cache_t *span_cache;
    यदि (!iclass)
      span_cache = &heap->span_cache;
    अन्यथा
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    यदि (!span_cache->गिनती)
      अनुवर्त;
#यदि ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&heap->thread_to_global,
                         span_cache->गिनती * (iclass + 1) * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                       span_cache->गिनती);
    rpmalloc_global_cache_जोड़ो_spans(span_cache->span, iclass + 1,
                                        span_cache->गिनती);
#अन्यथा
    प्रति (माप_प्रकार ispan = 0; ispan < span_cache->गिनती; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
    span_cache->गिनती = 0;
  }
#endif

#यदि ENABLE_STATISTICS
  प्रति (माप_प्रकार iclass = 0; iclass < SIZE_CLASS_गिनती; ++iclass) {
    atomic_store32(&heap->size_class_use[iclass].alloc_current, 0);
    atomic_store32(&heap->size_class_use[iclass].spans_current, 0);
  }
  प्रति (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    atomic_store32(&heap->span_use[iclass].current, 0);
  }
#endif
}

बाह्य अन्तःस्थ शून्य rpmalloc_heap_thread_set_current(rpmalloc_heap_t *heap) {
  heap_t *prev_heap = get_thread_heap_raw();
  यदि (prev_heap != heap) {
    set_thread_heap(heap);
    यदि (prev_heap)
      rpmalloc_heap_release(prev_heap);
  }
}

बाह्य अन्तःस्थ rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(शून्य *ptr) {
  // Grab the span, and then the heap from the span
  span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
  यदि (span) {
    प्रत्यागम span->heap;
  }
  प्रत्यागम 0;
}

#endif

#यदि ENABLE_PRELOAD || ENABLE_OVERRIDE

#अन्तर्भाव "malloc.c"

#endif

शून्य rpmalloc_linker_reference(शून्य) { (शून्य)परिमाण(_rpmalloc_initialized); }
