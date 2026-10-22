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

#അന്തര്ഭാവ "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#എങ്കിൽ defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wunused-function"
#എങ്കിൽ __has_warning("-Wreserved-identifier")
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
#എങ്കിൽ __has_warning("-Wstatic-in-inline")
#pragma clang diagnostic ignored "-Wstatic-in-inline"
#endif
#അല്ലെങ്കിലെങ്കിൽ defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#എങ്കിൽ !defined(__has_builtin)
#define __has_builtin(b) 0
#endif

#എങ്കിൽ defined(__GNUC__) || defined(__clang__)

#എങ്കിൽ __has_builtin(__builtin_memcpy_inline)
#define _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#അല്ലെങ്കിൽ
#define _rpmalloc_memcpy_const(x, y, s)                                        \
  ചെയ്യുക {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memcpy(x, y, s);                                                           \
  } ആകുമ്പോൾ (0)
#endif

#എങ്കിൽ __has_builtin(__builtin_memset_inline)
#define _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#അല്ലെങ്കിൽ
#define _rpmalloc_memset_const(x, y, s)                                        \
  ചെയ്യുക {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } ആകുമ്പോൾ (0)
#endif
#അല്ലെങ്കിൽ
#define _rpmalloc_memcpy_const(x, y, s) memcpy(x, y, s)
#define _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#endif

#എങ്കിൽ __has_builtin(__builtin_assume)
#define rpmalloc_assume(cond) __builtin_assume(cond)
#അല്ലെങ്കിലെങ്കിൽ defined(__GNUC__)
#define rpmalloc_assume(cond)                                                  \
  ചെയ്യുക {                                                                         \
    എങ്കിൽ (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } ആകുമ്പോൾ (0)
#അല്ലെങ്കിലെങ്കിൽ defined(_MSC_VER)
#define rpmalloc_assume(cond) __assume(cond)
#അല്ലെങ്കിൽ
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
#ifndef DEFAULT_SPAN_MAP_എണ്ണം
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#define DEFAULT_SPAN_MAP_എണ്ണം 64
#endif
#ifndef GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#define GLOBAL_CACHE_MULTIPLIER 8
#endif

#എങ്കിൽ DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#error Must use global cache എങ്കിൽ unmap is disabled
#endif

#എങ്കിൽ DISABLE_UNMAP
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 1
#endif

#എങ്കിൽ !ENABLE_GLOBAL_CACHE
#undef ENABLE_UNLIMITED_CACHE
#define ENABLE_UNLIMITED_CACHE 0
#endif

#എങ്കിൽ !ENABLE_THREAD_CACHE
#undef ENABLE_ADAPTIVE_THREAD_CACHE
#define ENABLE_ADAPTIVE_THREAD_CACHE 0
#endif

#എങ്കിൽ defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#define PLATFORM_POSIX 0
#അല്ലെങ്കിൽ
#define PLATFORM_WINDOWS 0
#define PLATFORM_POSIX 1
#endif

/// Platform and arch specifics
#എങ്കിൽ defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable : 5105)
#ifndef FORCEINLINE
#define FORCEINLINE വരിയിൽ __forceinline
#endif
#define _Static_assert static_assert
#അല്ലെങ്കിൽ
#ifndef FORCEINLINE
#define FORCEINLINE വരിയിൽ __attribute__((__always_inline__))
#endif
#endif
#എങ്കിൽ PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#അന്തര്ഭാവ <windows.h>
#എങ്കിൽ ENABLE_VALIDATE_ARGS
#അന്തര്ഭാവ <intsafe.h>
#endif
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <unistd.h>
#എങ്കിൽ defined(__linux__) || defined(__ANDROID__)
#അന്തര്ഭാവ <sys/prctl.h>
#എങ്കിൽ !defined(PR_SET_VMA)
#define PR_SET_VMA 0x53564d41
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif
#എങ്കിൽ defined(__APPLE__)
#അന്തര്ഭാവ <TargetConditionals.h>
#എങ്കിൽ !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#അന്തര്ഭാവ <mach/mach_vm.h>
#അന്തര്ഭാവ <mach/vm_statistics.h>
#endif
#അന്തര്ഭാവ <pthread.h>
#endif
#എങ്കിൽ defined(__HAIKU__) || defined(__TINYC__)
#അന്തര്ഭാവ <pthread.h>
#endif
#endif

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <string.h>

#എങ്കിൽ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#അന്തര്ഭാവ <fibersapi.h>
സ്ഥിതം DWORD fls_താക്കോൽ;
#endif

#എങ്കിൽ PLATFORM_POSIX
#അന്തര്ഭാവ <sched.h>
#അന്തര്ഭാവ <sys/mman.h>
#ifdef __FreeBSD__
#അന്തര്ഭാവ <sys/sysctl.h>
#define MAP_HUGETLB MAP_ALIGNED_SUPER
#ifndef PROT_MAX
#define PROT_MAX(f) 0
#endif
#അല്ലെങ്കിൽ
#define PROT_MAX(f) 0
#endif
#ifdef __sun
ബാഹ്യ പൂർണ്ണസംഖ്യ madvise(caddr_t, size_t, പൂർണ്ണസംഖ്യ);
#endif
#ifndef MAP_UNINITIALIZED
#define MAP_UNINITIALIZED 0
#endif
#endif
#അന്തര്ഭാവ <errno.h>

#എങ്കിൽ ENABLE_ASSERTS
#undef NDEBUG
#എങ്കിൽ defined(_MSC_VER) && !defined(_DEBUG)
#define _DEBUG
#endif
#അന്തര്ഭാവ <assert.h>
#define RPMALLOC_TOSTRING_M(x) #x
#define RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#define rpmalloc_assert(truth, message)                                        \
  ചെയ്യുക {                                                                         \
    എങ്കിൽ (!(truth)) {                                                            \
      എങ്കിൽ (_memory_config.error_callback) {                                     \
        _memory_config.error_callback(message " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } അല്ലെങ്കിൽ {                                                                 \
        പ്രതിജ്ഞാ((truth) && message);                                            \
      }                                                                        \
    }                                                                          \
  } ആകുമ്പോൾ (0)
#അല്ലെങ്കിൽ
#define rpmalloc_assert(truth, message)                                        \
  ചെയ്യുക {                                                                         \
  } ആകുമ്പോൾ (0)
#endif
#എങ്കിൽ ENABLE_STATISTICS
#അന്തര്ഭാവ <stdio.h>
#endif

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#എങ്കിൽ defined(_MSC_VER) && !defined(__clang__)

തരനിർവചനം അസ്ഥിര long atomic32_t;
തരനിർവചനം അസ്ഥിര long long atomic64_t;
തരനിർവചനം അസ്ഥിര ശൂന്യം *atomicptr_t;

സ്ഥിതം FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  തിരികെ (int32_t)InterlockedOr(src, 0);
}
സ്ഥിതം FORCEINLINE ശൂന്യം atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
സ്ഥിതം FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  തിരികെ (int32_t)InterlockedIncrement(val);
}
സ്ഥിതം FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  തിരികെ (int32_t)InterlockedDecrement(val);
}
സ്ഥിതം FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  തിരികെ (int32_t)InterlockedExchangeAdd(val, add) + add;
}
സ്ഥിതം FORCEINLINE പൂർണ്ണസംഖ്യ atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  തിരികെ (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
സ്ഥിതം FORCEINLINE ശൂന്യം atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
സ്ഥിതം FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  തിരികെ (int64_t)InterlockedOr64(src, 0);
}
സ്ഥിതം FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  തിരികെ (int64_t)InterlockedExchangeAdd64(val, add) + add;
}
സ്ഥിതം FORCEINLINE ശൂന്യം *atomic_load_ptr(atomicptr_t *src) {
  തിരികെ InterlockedCompareExchangePointer(src, 0, 0);
}
സ്ഥിതം FORCEINLINE ശൂന്യം atomic_store_ptr(atomicptr_t *dst, ശൂന്യം *val) {
  InterlockedExchangePointer(dst, val);
}
സ്ഥിതം FORCEINLINE ശൂന്യം atomic_store_ptr_release(atomicptr_t *dst, ശൂന്യം *val) {
  InterlockedExchangePointer(dst, val);
}
സ്ഥിതം FORCEINLINE ശൂന്യം *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     ശൂന്യം *val) {
  തിരികെ (ശൂന്യം *)InterlockedExchangePointer((ശൂന്യം *അസ്ഥിര *)dst, val);
}
സ്ഥിതം FORCEINLINE പൂർണ്ണസംഖ്യ atomic_cas_ptr(atomicptr_t *dst, ശൂന്യം *val, ശൂന്യം *ref) {
  തിരികെ (InterlockedCompareExchangePointer((ശൂന്യം *അസ്ഥിര *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#define EXPECTED(x) (x)
#define UNEXPECTED(x) (x)

#അല്ലെങ്കിൽ

#അന്തര്ഭാവ <stdatomic.h>

തരനിർവചനം അസ്ഥിര _Atomic(int32_t) atomic32_t;
തരനിർവചനം അസ്ഥിര _Atomic(int64_t) atomic64_t;
തരനിർവചനം അസ്ഥിര _Atomic(ശൂന്യം *) atomicptr_t;

സ്ഥിതം FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  തിരികെ atomic_load_explicit(src, memory_order_relaxed);
}
സ്ഥിതം FORCEINLINE ശൂന്യം atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
സ്ഥിതം FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  തിരികെ atomic_fetch_add_explicit(val, 1, memory_order_relaxed) + 1;
}
സ്ഥിതം FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  തിരികെ atomic_fetch_add_explicit(val, -1, memory_order_relaxed) - 1;
}
സ്ഥിതം FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  തിരികെ atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
സ്ഥിതം FORCEINLINE പൂർണ്ണസംഖ്യ atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  തിരികെ atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
സ്ഥിതം FORCEINLINE ശൂന്യം atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
സ്ഥിതം FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  തിരികെ atomic_load_explicit(val, memory_order_relaxed);
}
സ്ഥിതം FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  തിരികെ atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
സ്ഥിതം FORCEINLINE ശൂന്യം *atomic_load_ptr(atomicptr_t *src) {
  തിരികെ atomic_load_explicit(src, memory_order_relaxed);
}
സ്ഥിതം FORCEINLINE ശൂന്യം atomic_store_ptr(atomicptr_t *dst, ശൂന്യം *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
സ്ഥിതം FORCEINLINE ശൂന്യം atomic_store_ptr_release(atomicptr_t *dst, ശൂന്യം *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
സ്ഥിതം FORCEINLINE ശൂന്യം *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     ശൂന്യം *val) {
  തിരികെ atomic_exchange_explicit(dst, val, memory_order_acquire);
}
സ്ഥിതം FORCEINLINE പൂർണ്ണസംഖ്യ atomic_cas_ptr(atomicptr_t *dst, ശൂന്യം *val, ശൂന്യം *ref) {
  തിരികെ atomic_compare_exchange_weak_explicit(
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

#എങ്കിൽ ENABLE_STATISTICS
#define _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#define _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#define _rpmalloc_stat_add(counter, മൂല്യം_2)                                     \
  atomic_add32(counter, (int32_t)(മൂല്യം_2))
#define _rpmalloc_stat_add64(counter, മൂല്യം_2)                                   \
  atomic_add64(counter, (int64_t)(മൂല്യം_2))
#define _rpmalloc_stat_add_peak(counter, മൂല്യം_2, peak)                          \
  ചെയ്യുക {                                                                         \
    int32_t cur_എണ്ണം = atomic_add32(counter, (int32_t)(മൂല്യം_2));              \
    എങ്കിൽ (cur_എണ്ണം > (peak))                                                   \
      peak = cur_എണ്ണം;                                                       \
  } ആകുമ്പോൾ (0)
#define _rpmalloc_stat_sub(counter, മൂല്യം_2)                                     \
  atomic_add32(counter, -(int32_t)(മൂല്യം_2))
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  ചെയ്യുക {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&heap->size_class_use[class_idx].alloc_current);         \
    എങ്കിൽ (alloc_current > heap->size_class_use[class_idx].alloc_peak)            \
      heap->size_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&heap->size_class_use[class_idx].alloc_total);               \
  } ആകുമ്പോൾ (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  ചെയ്യുക {                                                                         \
    atomic_decr32(&heap->size_class_use[class_idx].alloc_current);             \
    atomic_incr32(&heap->size_class_use[class_idx].free_total);                \
  } ആകുമ്പോൾ (0)
#അല്ലെങ്കിൽ
#define _rpmalloc_stat_inc(counter)                                            \
  ചെയ്യുക {                                                                         \
  } ആകുമ്പോൾ (0)
#define _rpmalloc_stat_dec(counter)                                            \
  ചെയ്യുക {                                                                         \
  } ആകുമ്പോൾ (0)
#define _rpmalloc_stat_add(counter, മൂല്യം_2)                                     \
  ചെയ്യുക {                                                                         \
  } ആകുമ്പോൾ (0)
#define _rpmalloc_stat_add64(counter, മൂല്യം_2)                                   \
  ചെയ്യുക {                                                                         \
  } ആകുമ്പോൾ (0)
#define _rpmalloc_stat_add_peak(counter, മൂല്യം_2, peak)                          \
  ചെയ്യുക {                                                                         \
  } ആകുമ്പോൾ (0)
#define _rpmalloc_stat_sub(counter, മൂല്യം_2)                                     \
  ചെയ്യുക {                                                                         \
  } ആകുമ്പോൾ (0)
#define _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  ചെയ്യുക {                                                                         \
  } ആകുമ്പോൾ (0)
#define _rpmalloc_stat_inc_free(heap, class_idx)                               \
  ചെയ്യുക {                                                                         \
  } ആകുമ്പോൾ (0)
#endif

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#define SMALL_GRANULARITY 16
//! Small granularity shift count
#define SMALL_GRANULARITY_SHIFT 4
//! Number of small block size classes
#define SMALL_CLASS_എണ്ണം 65
//! Maximum size of a small block
#define SMALL_SIZE_LIMIT (SMALL_GRANULARITY * (SMALL_CLASS_എണ്ണം - 1))
//! Granularity of a medium allocation block
#define MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#define MEDIUM_GRANULARITY_SHIFT 9
//! Number of medium block size classes
#define MEDIUM_CLASS_എണ്ണം 61
//! Total number of small + medium size classes
#define SIZE_CLASS_എണ്ണം (SMALL_CLASS_എണ്ണം + MEDIUM_CLASS_എണ്ണം)
//! Number of large block size classes
#define LARGE_CLASS_എണ്ണം 63
//! Maximum size of a medium block
#define MEDIUM_SIZE_LIMIT                                                      \
  (SMALL_SIZE_LIMIT + (MEDIUM_GRANULARITY * MEDIUM_CLASS_എണ്ണം))
//! Maximum size of a large block
#define LARGE_SIZE_LIMIT                                                       \
  ((LARGE_CLASS_എണ്ണം * _memory_span_size) - SPAN_HEADER_SIZE)
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

#എങ്കിൽ ENABLE_VALIDATE_ARGS
//! Maximum allocation size to avoid integer overflow
#undef MAX_ALLOC_SIZE
#define MAX_ALLOC_SIZE (((size_t) - 1) - _memory_span_size)
#endif

#define pointer_offset(ptr, ofs) (ശൂന്യം *)((അക്ഷരം *)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff(first, second)                                            \
  (ptrdiff_t)((സ്ഥിരം അക്ഷരം *)(first) - (സ്ഥിരം അക്ഷരം *)(second))

#define INVALID_POINTER ((ശൂന്യം *)((uintptr_t) - 1))

#define SIZE_CLASS_LARGE SIZE_CLASS_എണ്ണം
#define SIZE_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
തരനിർവചനം ഘടന heap_t heap_t;
//! Span of memory pages
തരനിർവചനം ഘടന span_t span_t;
//! Span list
തരനിർവചനം ഘടന span_list_t span_list_t;
//! Span active data
തരനിർവചനം ഘടന span_active_t span_active_t;
//! Size class definition
തരനിർവചനം ഘടന size_class_t size_class_t;
//! Global cache
തരനിർവചനം ഘടന global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#define SPAN_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#define SPAN_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#define SPAN_FLAG_ALIGNED_BLOCKS 4U
//! Flag indicating an unmapped master span
#define SPAN_FLAG_UNMAPPED_MASTER 8U

#എങ്കിൽ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
ഘടന span_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t മുകളിലെ;
#എങ്കിൽ ENABLE_STATISTICS
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
തരനിർവചനം ഘടന span_use_t span_use_t;
#endif

#എങ്കിൽ ENABLE_STATISTICS
ഘടന size_class_use_t {
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
തരനിർവചനം ഘടന size_class_use_t size_class_use_t;
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
ഘടന span_t {
  //! Free list
  ശൂന്യം *free_list;
  //! Total block count of size class
  uint32_t block_എണ്ണം;
  //! Size class
  uint32_t size_class;
  //! Index of last block initialized in free list
  uint32_t free_list_limit;
  //! Number of used blocks remaining when in partial state
  uint32_t used_എണ്ണം;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_size;
  //! Size of a block
  uint32_t block_size;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t span_എണ്ണം;
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
_Static_assert(വലുപ്പം(span_t) <= SPAN_HEADER_SIZE, "span size mismatch");

ഘടന span_cache_t {
  size_t എണ്ണം;
  span_t *span[MAX_THREAD_SPAN_CACHE];
};
തരനിർവചനം ഘടന span_cache_t span_cache_t;

ഘടന span_large_cache_t {
  size_t എണ്ണം;
  span_t *span[MAX_THREAD_SPAN_LARGE_CACHE];
};
തരനിർവചനം ഘടന span_large_cache_t span_large_cache_t;

ഘടന heap_size_class_t {
  //! Free list of active span
  ശൂന്യം *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  span_t *partial_span;
  //! Early level cache of fully free spans
  span_t *cache;
};
തരനിർവചനം ഘടന heap_size_class_t heap_size_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
ഘടന heap_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  heap_size_class_t size_class[SIZE_CLASS_എണ്ണം];
#എങ്കിൽ ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  span_cache_t span_cache;
#endif
  //! List of deferred free spans (single linked list)
  atomicptr_t span_free_deferred;
  //! Number of full spans
  size_t full_span_എണ്ണം;
  //! Mapped but unused spans
  span_t *span_reserve;
  //! Master span for mapped but unused spans
  span_t *span_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t child_എണ്ണം;
  //! Next heap in id list
  heap_t *next_heap;
  //! Next heap in orphan list
  heap_t *next_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  പൂർണ്ണസംഖ്യ finalize;
  //! Master heap owning the memory pages
  heap_t *master_heap;
#എങ്കിൽ ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  span_large_cache_t span_large_cache[LARGE_CLASS_എണ്ണം - 1];
#endif
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  span_t *full_span[SIZE_CLASS_എണ്ണം];
  //! Double linked list of large and huge spans allocated by this heap
  span_t *large_huge_span;
#endif
#എങ്കിൽ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  span_use_t span_use[LARGE_CLASS_എണ്ണം];
#endif
#എങ്കിൽ ENABLE_STATISTICS
  //! Allocation stats per size class
  size_class_use_t size_class_use[SIZE_CLASS_എണ്ണം + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_to_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_to_thread;
#endif
};

// Size class for defining a block size bucket
ഘടന size_class_t {
  //! Size of blocks in this class
  uint32_t block_size;
  //! Number of blocks in each chunk
  uint16_t block_എണ്ണം;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(വലുപ്പം(size_class_t) == 8, "Size class size mismatch");

ഘടന global_cache_t {
  //! Cache lock
  atomic32_t lock;
  //! Cache count
  uint32_t എണ്ണം;
#എങ്കിൽ ENABLE_STATISTICS
  //! Insert count
  size_t തിരുകുക_എണ്ണം;
  //! Extract count
  size_t extract_എണ്ണം;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ _rpmalloc_initialized;
//! Main thread ID
സ്ഥിതം uintptr_t _rpmalloc_main_thread_id;
//! Configuration
സ്ഥിതം rpmalloc_config_t _memory_config;
//! Memory page size
സ്ഥിതം size_t _memory_page_size;
//! Shift to divide by page size
സ്ഥിതം size_t _memory_page_size_shift;
//! Granularity at which memory pages are mapped by OS
സ്ഥിതം size_t _memory_map_granularity;
#എങ്കിൽ RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
സ്ഥിതം size_t _memory_span_size;
//! Shift to divide by span size
സ്ഥിതം size_t _memory_span_size_shift;
//! Mask to get to start of a memory span
സ്ഥിതം uintptr_t _memory_span_mask;
#അല്ലെങ്കിൽ
//! Hardwired span size
#define _memory_span_size _memory_default_span_size
#define _memory_span_size_shift _memory_default_span_size_shift
#define _memory_span_mask _memory_default_span_mask
#endif
//! Number of spans to map in each map call
സ്ഥിതം size_t memory_span_map_എണ്ണം;
//! Number of spans to keep reserved in each heap
സ്ഥിതം size_t memory_heap_reserve_എണ്ണം;
//! Global size classes
സ്ഥിതം size_class_t _memory_size_class[SIZE_CLASS_എണ്ണം];
//! Run-time size limit of medium blocks
സ്ഥിതം size_t _memory_medium_size_limit;
//! Heap ID counter
സ്ഥിതം atomic32_t _memory_heap_id;
//! Huge page support
സ്ഥിതം പൂർണ്ണസംഖ്യ _memory_huge_pages;
#എങ്കിൽ ENABLE_GLOBAL_CACHE
//! Global span cache
സ്ഥിതം global_cache_t _memory_span_cache[LARGE_CLASS_എണ്ണം];
#endif
//! Global reserved spans
സ്ഥിതം span_t *_memory_global_reserve;
//! Global reserved count
സ്ഥിതം size_t memory_global_reserve_എണ്ണം;
//! Global reserved master
സ്ഥിതം span_t *_memory_global_reserve_master;
//! All heaps
സ്ഥിതം heap_t *_memory_heaps[HEAP_ARRAY_SIZE];
//! Used to restrict access to mapping memory for huge pages
സ്ഥിതം atomic32_t _memory_global_lock;
//! Orphaned heaps
സ്ഥിതം heap_t *_memory_orphan_heaps;
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
സ്ഥിതം heap_t *_memory_first_class_orphan_heaps;
#endif
#എങ്കിൽ ENABLE_STATISTICS
//! Allocations counter
സ്ഥിതം atomic64_t _allocation_counter;
//! Deallocations counter
സ്ഥിതം atomic64_t _deallocation_counter;
//! Active heap count
സ്ഥിതം atomic32_t _memory_active_heaps;
//! Number of currently mapped memory pages
സ്ഥിതം atomic32_t _mapped_pages;
//! Peak number of concurrently mapped memory pages
സ്ഥിതം int32_t _mapped_pages_peak;
//! Number of mapped master spans
സ്ഥിതം atomic32_t _master_spans;
//! Number of unmapped dangling master spans
സ്ഥിതം atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
സ്ഥിതം atomic32_t _mapped_total;
//! Running counter of total number of unmapped memory pages since start
സ്ഥിതം atomic32_t _unmapped_total;
//! Number of currently mapped memory pages in OS calls
സ്ഥിതം atomic32_t _mapped_pages_os;
//! Number of currently allocated pages in huge allocations
സ്ഥിതം atomic32_t _huge_pages_current;
//! Peak number of currently allocated pages in huge allocations
സ്ഥിതം int32_t _huge_pages_peak;
#endif

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#എങ്കിൽ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
സ്ഥിതം pthread_key_t _memory_thread_heap;
#അല്ലെങ്കിൽ
#ifdef _MSC_VER
#define _Thread_local __declspec(thread)
#define TLS_MODEL
#അല്ലെങ്കിൽ
#ifndef __HAIKU__
#define TLS_MODEL __attribute__((tls_model("initial-exec")))
#അല്ലെങ്കിൽ
#define TLS_MODEL
#endif
#എങ്കിൽ !defined(__clang__) && defined(__GNUC__)
#define _Thread_local __thread
#endif
#endif
സ്ഥിതം _Thread_local heap_t *_memory_thread_heap TLS_MODEL;
#endif

സ്ഥിതം വരിയിൽ heap_t *get_thread_heap_raw(ശൂന്യം) {
#എങ്കിൽ (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  തിരികെ pthread_getspecific(_memory_thread_heap);
#അല്ലെങ്കിൽ
  തിരികെ _memory_thread_heap;
#endif
}

//! Get the current thread heap
സ്ഥിതം വരിയിൽ heap_t *get_thread_heap(ശൂന്യം) {
  heap_t *heap = get_thread_heap_raw();
#എങ്കിൽ ENABLE_PRELOAD
  എങ്കിൽ (EXPECTED(heap != 0))
    തിരികെ heap;
  rpmalloc_initialize();
  തിരികെ get_thread_heap_raw();
#അല്ലെങ്കിൽ
  തിരികെ heap;
#endif
}

//! Fast thread ID
സ്ഥിതം വരിയിൽ uintptr_t get_thread_id(ശൂന്യം) {
#എങ്കിൽ defined(_WIN32)
  തിരികെ (uintptr_t)((ശൂന്യം *)NtCurrentTeb());
#അല്ലെങ്കിലെങ്കിൽ (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#എങ്കിൽ defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#അല്ലെങ്കിലെങ്കിൽ defined(__x86_64__)
#എങ്കിൽ defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#അല്ലെങ്കിൽ
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#endif
#അല്ലെങ്കിലെങ്കിൽ defined(__arm__)
  __asm__ അസ്ഥിര("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#അല്ലെങ്കിലെങ്കിൽ defined(__aarch64__)
#എങ്കിൽ defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ അസ്ഥിര("mrs %0, tpidrro_el0" : "=r"(tid));
#അല്ലെങ്കിൽ
  __asm__ അസ്ഥിര("mrs %0, tpidr_el0" : "=r"(tid));
#endif
#അല്ലെങ്കിൽ
#error This platform needs implementation of get_thread_id()
#endif
  തിരികെ tid;
#അല്ലെങ്കിൽ
#error This platform needs implementation of get_thread_id()
#endif
}

//! Set the current thread heap
സ്ഥിതം ശൂന്യം set_thread_heap(heap_t *heap) {
#എങ്കിൽ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(_memory_thread_heap, heap);
#അല്ലെങ്കിൽ
  _memory_thread_heap = heap;
#endif
  എങ്കിൽ (heap)
    heap->owner_thread = get_thread_id();
}

//! Set main thread ID
ബാഹ്യ ശൂന്യം rpmalloc_set_main_thread(ശൂന്യം);

ശൂന്യം rpmalloc_set_main_thread(ശൂന്യം) {
  _rpmalloc_main_thread_id = get_thread_id();
}

സ്ഥിതം ശൂന്യം _rpmalloc_spin(ശൂന്യം) {
#എങ്കിൽ defined(_MSC_VER)
#എങ്കിൽ defined(_M_ARM64)
  __yield();
#അല്ലെങ്കിൽ
  _mm_pause();
#endif
#അല്ലെങ്കിലെങ്കിൽ defined(__x86_64__) || defined(__i386__)
  __asm__ അസ്ഥിര("pause" ::: "memory");
#അല്ലെങ്കിലെങ്കിൽ defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ അസ്ഥിര("yield" ::: "memory");
#അല്ലെങ്കിലെങ്കിൽ defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ അസ്ഥിര("or 27,27,27");
#അല്ലെങ്കിലെങ്കിൽ defined(__sparc__)
  __asm__ അസ്ഥിര("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#അല്ലെങ്കിൽ
  ഘടന timespec ts = {0};
  nanosleep(&ts, 0);
#endif
}

#എങ്കിൽ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

സ്ഥിതം ശൂന്യം NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  തിരഞ്ഞെടുപ്പ് (reason) {
  അവസ്ഥ DLL_PROCESS_ATTACH:
    നിർത്തുക;
  അവസ്ഥ DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    നിർത്തുക;
  അവസ്ഥ DLL_THREAD_ATTACH:
    നിർത്തുക;
  അവസ്ഥ DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    നിർത്തുക;
  }
}

#ifdef _WIN64
#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#pragma const_seg(".CRT$XLY")

ബാഹ്യ സ്ഥിരം PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
സ്ഥിരം PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#pragma const_seg()
#അല്ലെങ്കിൽ // _WIN64
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#pragma data_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#pragma data_seg()
#endif // _WIN64

സ്ഥിതം ശൂന്യം NTAPI _rpmalloc_thread_destructor(ശൂന്യം *മൂല്യം_2) {
#എങ്കിൽ ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  എങ്കിൽ (get_thread_id() == _rpmalloc_main_thread_id)
    തിരികെ;
#endif
  എങ്കിൽ (മൂല്യം_2)
    rpmalloc_thread_finalize(1);
}
#endif

////////////
///
/// Low level memory map/unmap
///
//////

സ്ഥിതം ശൂന്യം _rpmalloc_set_name(ശൂന്യം *address, size_t size) {
#എങ്കിൽ defined(__linux__) || defined(__ANDROID__)
  സ്ഥിരം അക്ഷരം *name = _memory_huge_pages ? _memory_config.huge_page_name
                                        : _memory_config.page_name;
  എങ്കിൽ (address == MAP_FAILED || !name)
    തിരികെ;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (ശൂന്യം)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (uintptr_t)address, size,
              (uintptr_t)name);
#അല്ലെങ്കിൽ
  (ശൂന്യം)വലുപ്പം(size);
  (ശൂന്യം)വലുപ്പം(address);
#endif
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
സ്ഥിതം ശൂന്യം *_rpmalloc_mmap(size_t size, size_t *offset) {
  rpmalloc_assert(!(size % _memory_page_size), "Invalid mmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
  ശൂന്യം *address = _memory_config.memory_map(size, offset);
  എങ്കിൽ (EXPECTED(address != 0)) {
    _rpmalloc_stat_add_peak(&_mapped_pages, (size >> _memory_page_size_shift),
                            _mapped_pages_peak);
    _rpmalloc_stat_add(&_mapped_total, (size >> _memory_page_size_shift));
  }
  തിരികെ address;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
സ്ഥിതം ശൂന്യം _rpmalloc_unmap(ശൂന്യം *address, size_t size, size_t offset,
                            size_t release) {
  rpmalloc_assert(!release || (release >= size), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  എങ്കിൽ (release) {
    rpmalloc_assert(!(release % _memory_page_size), "Invalid unmap size");
    _rpmalloc_stat_sub(&_mapped_pages, (release >> _memory_page_size_shift));
    _rpmalloc_stat_add(&_unmapped_total, (release >> _memory_page_size_shift));
  }
  _memory_config.memory_unmap(address, size, offset, release);
}

//! Default implementation to map new pages to virtual memory
സ്ഥിതം ശൂന്യം *_rpmalloc_mmap_os(size_t size, size_t *offset) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((size >= _memory_span_size) &&
                    (_memory_span_size > _memory_map_granularity))
                       ? _memory_span_size
                       : 0;
  rpmalloc_assert(size >= _memory_page_size, "Invalid mmap size");
#എങ്കിൽ PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  ശൂന്യം *ptr = VirtualAlloc(0, size + padding,
                           (_memory_huge_pages ? MEM_LARGE_PAGES : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
  എങ്കിൽ (!ptr) {
    എങ്കിൽ (_memory_config.map_fail_callback) {
      എങ്കിൽ (_memory_config.map_fail_callback(size + padding))
        തിരികെ _rpmalloc_mmap_os(size, offset);
    } അല്ലെങ്കിൽ {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    തിരികെ 0;
  }
#അല്ലെങ്കിൽ
  പൂർണ്ണസംഖ്യ flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#എങ്കിൽ defined(__APPLE__) && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
  പൂർണ്ണസംഖ്യ fd = (പൂർണ്ണസംഖ്യ)VM_MAKE_TAG(240U);
  എങ്കിൽ (_memory_huge_pages)
    fd |= VM_FLAGS_SUPERPAGE_SIZE_2MB;
  ശൂന്യം *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#അല്ലെങ്കിലെങ്കിൽ defined(MAP_HUGETLB)
  ശൂന്യം *ptr = mmap(0, size + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (_memory_huge_pages ? MAP_HUGETLB : 0) | flags, -1, 0);
#എങ്കിൽ defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  എങ്കിൽ ((ptr == MAP_FAILED || !ptr) && _memory_huge_pages) {
    ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    എങ്കിൽ (ptr && ptr != MAP_FAILED) {
      പൂർണ്ണസംഖ്യ prm = madvise(ptr, size + padding, MADV_HUGEPAGE);
      (ശൂന്യം)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#endif
  _rpmalloc_set_name(ptr, size + padding);
#അല്ലെങ്കിലെങ്കിൽ defined(MAP_ALIGNED)
  സ്ഥിരം size_t align =
      (വലുപ്പം(size_t) * 8) - (size_t)(__builtin_clzl(size - 1));
  ശൂന്യം *ptr =
      mmap(0, size + padding, PROT_READ | PROT_WRITE,
           (_memory_huge_pages ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#അല്ലെങ്കിലെങ്കിൽ defined(MAP_ALIGN)
  caddr_t base = (_memory_huge_pages ? (caddr_t)(4 << 20) : 0);
  ശൂന്യം *ptr = mmap(base, size + padding, PROT_READ | PROT_WRITE,
                   (_memory_huge_pages ? MAP_ALIGN : 0) | flags, -1, 0);
#അല്ലെങ്കിൽ
  ശൂന്യം *ptr = mmap(0, size + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#endif
  എങ്കിൽ ((ptr == MAP_FAILED) || !ptr) {
    എങ്കിൽ (_memory_config.map_fail_callback) {
      എങ്കിൽ (_memory_config.map_fail_callback(size + padding))
        തിരികെ _rpmalloc_mmap_os(size, offset);
    } അല്ലെങ്കിൽ എങ്കിൽ (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    തിരികെ 0;
  }
#endif
  _rpmalloc_stat_add(&_mapped_pages_os,
                     (int32_t)((size + padding) >> _memory_page_size_shift));
  എങ്കിൽ (padding) {
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
  തിരികെ ptr;
}

//! Default implementation to unmap pages from virtual memory
സ്ഥിതം ശൂന്യം _rpmalloc_unmap_os(ശൂന്യം *address, size_t size, size_t offset,
                               size_t release) {
  rpmalloc_assert(release || (offset == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= _memory_page_size),
                  "Invalid unmap size");
  rpmalloc_assert(size >= _memory_page_size, "Invalid unmap size");
  എങ്കിൽ (release && offset) {
    offset <<= 3;
    address = pointer_offset(address, -(int32_t)offset);
    എങ്കിൽ ((release >= _memory_span_size) &&
        (_memory_span_size > _memory_map_granularity)) {
      // Padding is always one span size
      release += _memory_span_size;
    }
  }
#എങ്കിൽ !DISABLE_UNMAP
#എങ്കിൽ PLATFORM_WINDOWS
  എങ്കിൽ (!VirtualFree(address, release ? 0 : size,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#അല്ലെങ്കിൽ
  എങ്കിൽ (release) {
    എങ്കിൽ (munmap(address, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } അല്ലെങ്കിൽ {
#എങ്കിൽ defined(MADV_FREE_REUSABLE)
    പൂർണ്ണസംഖ്യ ret;
    ആകുമ്പോൾ ((ret = madvise(address, size, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    എങ്കിൽ ((ret == -1) && (errno != 0)) {
#അല്ലെങ്കിലെങ്കിൽ defined(MADV_DONTNEED)
    എങ്കിൽ (madvise(address, size, MADV_DONTNEED)) {
#അല്ലെങ്കിലെങ്കിൽ defined(MADV_PAGEOUT)
    എങ്കിൽ (madvise(address, size, MADV_PAGEOUT)) {
#അല്ലെങ്കിലെങ്കിൽ defined(MADV_FREE)
    എങ്കിൽ (madvise(address, size, MADV_FREE)) {
#അല്ലെങ്കിൽ
    എങ്കിൽ (posix_madvise(address, size, POSIX_MADV_DONTNEED)) {
#endif
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#endif
#endif
  എങ്കിൽ (release)
    _rpmalloc_stat_sub(&_mapped_pages_os, release >> _memory_page_size_shift);
}

സ്ഥിതം ശൂന്യം _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_എണ്ണം);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
സ്ഥിതം span_t *_rpmalloc_global_get_reserved_spans(size_t span_എണ്ണം) {
  span_t *span = _memory_global_reserve;
  _rpmalloc_span_mark_as_subspan_unless_master(_memory_global_reserve_master,
                                               span, span_എണ്ണം);
  memory_global_reserve_എണ്ണം -= span_എണ്ണം;
  എങ്കിൽ (memory_global_reserve_എണ്ണം)
    _memory_global_reserve =
        (span_t *)pointer_offset(span, span_എണ്ണം << _memory_span_size_shift);
  അല്ലെങ്കിൽ
    _memory_global_reserve = 0;
  തിരികെ span;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
സ്ഥിതം ശൂന്യം _rpmalloc_global_set_reserved_spans(span_t *master, span_t *reserve,
                                                size_t reserve_span_എണ്ണം) {
  _memory_global_reserve_master = master;
  memory_global_reserve_എണ്ണം = reserve_span_എണ്ണം;
  _memory_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
സ്ഥിതം ശൂന്യം _rpmalloc_span_double_link_list_add(span_t **head, span_t *span) {
  എങ്കിൽ (*head)
    (*head)->prev = span;
  span->next = *head;
  *head = span;
}

//! Pop head span from double linked list
സ്ഥിതം ശൂന്യം rpmalloc_span_double_link_list_എടുക്കുക_head(span_t **head,
                                                     span_t *span) {
  rpmalloc_assert(*head == span, "Linked list corrupted");
  span = *head;
  *head = span->next;
}

//! Remove a span from double linked list
സ്ഥിതം ശൂന്യം _rpmalloc_span_double_link_list_remove(span_t **head,
                                                   span_t *span) {
  rpmalloc_assert(*head, "Linked list corrupted");
  എങ്കിൽ (*head == span) {
    *head = span->next;
  } അല്ലെങ്കിൽ {
    span_t *next_span = span->next;
    span_t *prev_span = span->prev;
    prev_span->next = next_span;
    എങ്കിൽ (EXPECTED(next_span != 0))
      next_span->prev = prev_span;
  }
}

////////////
///
/// Span control
///
//////

സ്ഥിതം ശൂന്യം rpmalloc_heap_cache_തിരുകുക(heap_t *heap, span_t *span);

സ്ഥിതം ശൂന്യം _rpmalloc_heap_finalize(heap_t *heap);

സ്ഥിതം ശൂന്യം _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_എണ്ണം);

//! Declare the span to be a subspan and store distance from master span and
//! span count
സ്ഥിതം ശൂന്യം _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         size_t span_എണ്ണം) {
  rpmalloc_assert((subspan != master) || (subspan->flags & SPAN_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  എങ്കിൽ (subspan != master) {
    subspan->flags = SPAN_FLAG_SUBSPAN;
    subspan->offset_from_master =
        (uint32_t)((uintptr_t)pointer_diff(subspan, master) >>
                   _memory_span_size_shift);
    subspan->align_offset = 0;
  }
  subspan->span_എണ്ണം = (uint32_t)span_എണ്ണം;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
സ്ഥിതം span_t *_rpmalloc_span_map_from_reserve(heap_t *heap,
                                               size_t span_എണ്ണം) {
  // Update the heap span reserve
  span_t *span = heap->span_reserve;
  heap->span_reserve =
      (span_t *)pointer_offset(span, span_എണ്ണം * _memory_span_size);
  heap->spans_reserved -= (uint32_t)span_എണ്ണം;

  _rpmalloc_span_mark_as_subspan_unless_master(heap->span_reserve_master, span,
                                               span_എണ്ണം);
  എങ്കിൽ (span_എണ്ണം <= LARGE_CLASS_എണ്ണം)
    _rpmalloc_stat_inc(&heap->span_use[span_എണ്ണം - 1].spans_from_reserved);

  തിരികെ span;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
സ്ഥിതം size_t rpmalloc_span_align_എണ്ണം(size_t span_എണ്ണം) {
  size_t request_എണ്ണം = (span_എണ്ണം > memory_span_map_എണ്ണം)
                             ? span_എണ്ണം
                             : memory_span_map_എണ്ണം;
  എങ്കിൽ ((_memory_page_size > _memory_span_size) &&
      ((request_എണ്ണം * _memory_span_size) % _memory_page_size))
    request_എണ്ണം +=
        memory_span_map_എണ്ണം - (request_എണ്ണം % memory_span_map_എണ്ണം);
  തിരികെ request_എണ്ണം;
}

//! Setup a newly mapped span
സ്ഥിതം ശൂന്യം _rpmalloc_span_initialize(span_t *span, size_t total_span_എണ്ണം,
                                      size_t span_എണ്ണം, size_t align_offset) {
  span->total_spans = (uint32_t)total_span_എണ്ണം;
  span->span_എണ്ണം = (uint32_t)span_എണ്ണം;
  span->align_offset = (uint32_t)align_offset;
  span->flags = SPAN_FLAG_MASTER;
  atomic_store32(&span->remaining_spans, (int32_t)total_span_എണ്ണം);
}

സ്ഥിതം ശൂന്യം _rpmalloc_span_unmap(span_t *span);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
സ്ഥിതം span_t *rpmalloc_span_map_aligned_എണ്ണം(heap_t *heap,
                                                size_t span_എണ്ണം) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_span_എണ്ണം = rpmalloc_span_align_എണ്ണം(span_എണ്ണം);
  size_t align_offset = 0;
  span_t *span = (span_t *)_rpmalloc_mmap(
      aligned_span_എണ്ണം * _memory_span_size, &align_offset);
  എങ്കിൽ (!span)
    തിരികെ 0;
  _rpmalloc_span_initialize(span, aligned_span_എണ്ണം, span_എണ്ണം, align_offset);
  _rpmalloc_stat_inc(&_master_spans);
  എങ്കിൽ (span_എണ്ണം <= LARGE_CLASS_എണ്ണം)
    _rpmalloc_stat_inc(&heap->span_use[span_എണ്ണം - 1].spans_map_calls);
  എങ്കിൽ (aligned_span_എണ്ണം > span_എണ്ണം) {
    span_t *reserved_spans =
        (span_t *)pointer_offset(span, span_എണ്ണം * _memory_span_size);
    size_t reserved_എണ്ണം = aligned_span_എണ്ണം - span_എണ്ണം;
    എങ്കിൽ (heap->spans_reserved) {
      _rpmalloc_span_mark_as_subspan_unless_master(
          heap->span_reserve_master, heap->span_reserve, heap->spans_reserved);
      rpmalloc_heap_cache_തിരുകുക(heap, heap->span_reserve);
    }
    എങ്കിൽ (reserved_എണ്ണം > memory_heap_reserve_എണ്ണം) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&_memory_global_lock) == 1,
                      "Global spin lock not held as expected");
      size_t remain_എണ്ണം = reserved_എണ്ണം - memory_heap_reserve_എണ്ണം;
      reserved_എണ്ണം = memory_heap_reserve_എണ്ണം;
      span_t *remain_span = (span_t *)pointer_offset(
          reserved_spans, reserved_എണ്ണം * _memory_span_size);
      എങ്കിൽ (_memory_global_reserve) {
        _rpmalloc_span_mark_as_subspan_unless_master(
            _memory_global_reserve_master, _memory_global_reserve,
            memory_global_reserve_എണ്ണം);
        _rpmalloc_span_unmap(_memory_global_reserve);
      }
      _rpmalloc_global_set_reserved_spans(span, remain_span, remain_എണ്ണം);
    }
    _rpmalloc_heap_set_reserved_spans(heap, span, reserved_spans,
                                      reserved_എണ്ണം);
  }
  തിരികെ span;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
സ്ഥിതം span_t *_rpmalloc_span_map(heap_t *heap, size_t span_എണ്ണം) {
  എങ്കിൽ (span_എണ്ണം <= heap->spans_reserved)
    തിരികെ _rpmalloc_span_map_from_reserve(heap, span_എണ്ണം);
  span_t *span = 0;
  പൂർണ്ണസംഖ്യ use_global_reserve =
      (_memory_page_size > _memory_span_size) ||
      (memory_span_map_എണ്ണം > memory_heap_reserve_എണ്ണം);
  എങ്കിൽ (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    ആകുമ്പോൾ (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
    എങ്കിൽ (memory_global_reserve_എണ്ണം >= span_എണ്ണം) {
      size_t reserve_എണ്ണം =
          (!heap->spans_reserved ? memory_heap_reserve_എണ്ണം : span_എണ്ണം);
      എങ്കിൽ (memory_global_reserve_എണ്ണം < reserve_എണ്ണം)
        reserve_എണ്ണം = memory_global_reserve_എണ്ണം;
      span = _rpmalloc_global_get_reserved_spans(reserve_എണ്ണം);
      എങ്കിൽ (span) {
        എങ്കിൽ (reserve_എണ്ണം > span_എണ്ണം) {
          span_t *reserved_span = (span_t *)pointer_offset(
              span, span_എണ്ണം << _memory_span_size_shift);
          _rpmalloc_heap_set_reserved_spans(heap, _memory_global_reserve_master,
                                            reserved_span,
                                            reserve_എണ്ണം - span_എണ്ണം);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        span->span_എണ്ണം = (uint32_t)span_എണ്ണം;
      }
    }
  }
  എങ്കിൽ (!span)
    span = rpmalloc_span_map_aligned_എണ്ണം(heap, span_എണ്ണം);
  എങ്കിൽ (use_global_reserve)
    atomic_store32_release(&_memory_global_lock, 0);
  തിരികെ span;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
സ്ഥിതം ശൂന്യം _rpmalloc_span_unmap(span_t *span) {
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");

  പൂർണ്ണസംഖ്യ is_master = !!(span->flags & SPAN_FLAG_MASTER);
  span_t *master =
      is_master ? span
                : ((span_t *)pointer_offset(
                      span, -(intptr_t)((uintptr_t)span->offset_from_master *
                                        _memory_span_size)));
  rpmalloc_assert(is_master || (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");

  size_t span_എണ്ണം = span->span_എണ്ണം;
  എങ്കിൽ (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(span->align_offset == 0, "Span align offset corrupted");
    എങ്കിൽ (_memory_span_size >= _memory_page_size)
      _rpmalloc_unmap(span, span_എണ്ണം * _memory_span_size, 0, 0);
  } അല്ലെങ്കിൽ {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    span->flags |=
        SPAN_FLAG_MASTER | SPAN_FLAG_SUBSPAN | SPAN_FLAG_UNMAPPED_MASTER;
    _rpmalloc_stat_add(&_unmapped_master_spans, 1);
  }

  എങ്കിൽ (atomic_add32(&master->remaining_spans, -(int32_t)span_എണ്ണം) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & SPAN_FLAG_MASTER) &&
                        !!(master->flags & SPAN_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_എണ്ണം = master->span_എണ്ണം;
    എങ്കിൽ (_memory_span_size < _memory_page_size)
      unmap_എണ്ണം = master->total_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_എണ്ണം * _memory_span_size,
                    master->align_offset,
                    (size_t)master->total_spans * _memory_span_size);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
സ്ഥിതം ശൂന്യം _rpmalloc_span_release_to_cache(heap_t *heap, span_t *span) {
  rpmalloc_assert(heap == span->heap, "Span heap pointer corrupted");
  rpmalloc_assert(span->size_class < SIZE_CLASS_എണ്ണം,
                  "Invalid span size class");
  rpmalloc_assert(span->span_എണ്ണം == 1, "Invalid span count");
#എങ്കിൽ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&heap->span_use[0].current);
#endif
  _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
  എങ്കിൽ (!heap->finalize) {
    _rpmalloc_stat_inc(&heap->span_use[0].spans_to_cache);
    _rpmalloc_stat_inc(&heap->size_class_use[span->size_class].spans_to_cache);
    എങ്കിൽ (heap->size_class[span->size_class].cache)
      rpmalloc_heap_cache_തിരുകുക(heap,
                                  heap->size_class[span->size_class].cache);
    heap->size_class[span->size_class].cache = span;
  } അല്ലെങ്കിൽ {
    _rpmalloc_span_unmap(span);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
സ്ഥിതം uint32_t free_list_partial_init(ശൂന്യം **list, ശൂന്യം **first_block,
                                       ശൂന്യം *page_start, ശൂന്യം *block_start,
                                       uint32_t block_എണ്ണം,
                                       uint32_t block_size) {
  rpmalloc_assert(block_എണ്ണം, "Internal failure");
  *first_block = block_start;
  എങ്കിൽ (block_എണ്ണം > 1) {
    ശൂന്യം *free_block = pointer_offset(block_start, block_size);
    ശൂന്യം *block_end =
        pointer_offset(block_start, (size_t)block_size * block_എണ്ണം);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    എങ്കിൽ (block_size < (_memory_page_size >> 1)) {
      ശൂന്യം *page_end = pointer_offset(page_start, _memory_page_size);
      എങ്കിൽ (page_end < block_end)
        block_end = page_end;
    }
    *list = free_block;
    block_എണ്ണം = 2;
    ശൂന്യം *next_block = pointer_offset(free_block, block_size);
    ആകുമ്പോൾ (next_block < block_end) {
      *((ശൂന്യം **)free_block) = next_block;
      free_block = next_block;
      ++block_എണ്ണം;
      next_block = pointer_offset(next_block, block_size);
    }
    *((ശൂന്യം **)free_block) = 0;
  } അല്ലെങ്കിൽ {
    *list = 0;
  }
  തിരികെ block_എണ്ണം;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
സ്ഥിതം ശൂന്യം *_rpmalloc_span_initialize_new(heap_t *heap,
                                           heap_size_class_t *heap_size_class,
                                           span_t *span, uint32_t class_idx) {
  rpmalloc_assert(span->span_എണ്ണം == 1, "Internal failure");
  size_class_t *size_class = _memory_size_class + class_idx;
  span->size_class = class_idx;
  span->heap = heap;
  span->flags &= ~SPAN_FLAG_ALIGNED_BLOCKS;
  span->block_size = size_class->block_size;
  span->block_എണ്ണം = size_class->block_എണ്ണം;
  span->free_list = 0;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  ശൂന്യം *block;
  span->free_list_limit =
      free_list_partial_init(&heap_size_class->free_list, &block, span,
                             pointer_offset(span, SPAN_HEADER_SIZE),
                             size_class->block_എണ്ണം, size_class->block_size);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  എങ്കിൽ (span->free_list_limit < span->block_എണ്ണം) {
    _rpmalloc_span_double_link_list_add(&heap_size_class->partial_span, span);
    span->used_എണ്ണം = span->free_list_limit;
  } അല്ലെങ്കിൽ {
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_എണ്ണം;
    span->used_എണ്ണം = span->block_എണ്ണം;
  }
  തിരികെ block;
}

സ്ഥിതം ശൂന്യം _rpmalloc_span_extract_free_list_deferred(span_t *span) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  ചെയ്യുക {
    span->free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } ആകുമ്പോൾ (span->free_list == INVALID_POINTER);
  span->used_എണ്ണം -= span->list_size;
  span->list_size = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ _rpmalloc_span_is_fully_utilized(span_t *span) {
  rpmalloc_assert(span->free_list_limit <= span->block_എണ്ണം,
                  "Span free list corrupted");
  തിരികെ !span->free_list && (span->free_list_limit >= span->block_എണ്ണം);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ _rpmalloc_span_finalize(heap_t *heap, size_t iclass, span_t *span,
                                   span_t **list_head) {
  ശൂന്യം *free_list = heap->size_class[iclass].free_list;
  span_t *class_span = (span_t *)((uintptr_t)free_list & _memory_span_mask);
  എങ്കിൽ (span == class_span) {
    // Adopt the heap class free list back into the span free list
    ശൂന്യം *block = span->free_list;
    ശൂന്യം *last_block = 0;
    ആകുമ്പോൾ (block) {
      last_block = block;
      block = *((ശൂന്യം **)block);
    }
    uint32_t free_എണ്ണം = 0;
    block = free_list;
    ആകുമ്പോൾ (block) {
      ++free_എണ്ണം;
      block = *((ശൂന്യം **)block);
    }
    എങ്കിൽ (last_block) {
      *((ശൂന്യം **)last_block) = free_list;
    } അല്ലെങ്കിൽ {
      span->free_list = free_list;
    }
    heap->size_class[iclass].free_list = 0;
    span->used_എണ്ണം -= free_എണ്ണം;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(span->list_size == span->used_എണ്ണം, "Memory leak detected");
  എങ്കിൽ (span->list_size == span->used_എണ്ണം) {
    _rpmalloc_stat_dec(&heap->span_use[0].current);
    _rpmalloc_stat_dec(&heap->size_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    എങ്കിൽ (list_head)
      _rpmalloc_span_double_link_list_remove(list_head, span);
    _rpmalloc_span_unmap(span);
    തിരികെ 1;
  }
  തിരികെ 0;
}

////////////
///
/// Global cache
///
//////

#എങ്കിൽ ENABLE_GLOBAL_CACHE

//! Finalize a global cache
സ്ഥിതം ശൂന്യം _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  ആകുമ്പോൾ (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

  വേണ്ടി (size_t ispan = 0; ispan < cache->എണ്ണം; ++ispan)
    _rpmalloc_span_unmap(cache->span[ispan]);
  cache->എണ്ണം = 0;

  ആകുമ്പോൾ (cache->overflow) {
    span_t *span = cache->overflow;
    cache->overflow = span->next;
    _rpmalloc_span_unmap(span);
  }

  atomic_store32_release(&cache->lock, 0);
}

സ്ഥിതം ശൂന്യം rpmalloc_global_cache_തിരുകുക_spans(span_t **span,
                                                size_t span_എണ്ണം,
                                                size_t എണ്ണം) {
  സ്ഥിരം size_t cache_limit =
      (span_എണ്ണം == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_SPAN_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_SPAN_LARGE_CACHE - (span_എണ്ണം >> 1));

  global_cache_t *cache = &_memory_span_cache[span_എണ്ണം - 1];

  size_t തിരുകുക_എണ്ണം = എണ്ണം;
  ആകുമ്പോൾ (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#എങ്കിൽ ENABLE_STATISTICS
  cache->തിരുകുക_എണ്ണം += എണ്ണം;
#endif
  എങ്കിൽ ((cache->എണ്ണം + തിരുകുക_എണ്ണം) > cache_limit)
    തിരുകുക_എണ്ണം = cache_limit - cache->എണ്ണം;

  memcpy(cache->span + cache->എണ്ണം, span, വലുപ്പം(span_t *) * തിരുകുക_എണ്ണം);
  cache->എണ്ണം += (uint32_t)തിരുകുക_എണ്ണം;

#എങ്കിൽ ENABLE_UNLIMITED_CACHE
  ആകുമ്പോൾ (തിരുകുക_എണ്ണം < എണ്ണം) {
#അല്ലെങ്കിൽ
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  ആകുമ്പോൾ ((_memory_page_size > _memory_span_size) && (തിരുകുക_എണ്ണം < എണ്ണം)) {
#endif
    span_t *current_span = span[തിരുകുക_എണ്ണം++];
    current_span->next = cache->overflow;
    cache->overflow = current_span;
  }
  atomic_store32_release(&cache->lock, 0);

  span_t *keep = 0;
  വേണ്ടി (size_t ispan = തിരുകുക_എണ്ണം; ispan < എണ്ണം; ++ispan) {
    span_t *current_span = span[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    എങ്കിൽ ((current_span->flags & SPAN_FLAG_MASTER) &&
        (atomic_load32(&current_span->remaining_spans) >
         (int32_t)current_span->span_എണ്ണം)) {
      current_span->next = keep;
      keep = current_span;
    } അല്ലെങ്കിൽ {
      _rpmalloc_span_unmap(current_span);
    }
  }

  എങ്കിൽ (keep) {
    ആകുമ്പോൾ (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    ആകുമ്പോൾ (keep) {
      വേണ്ടി (; islot < cache->എണ്ണം; ++islot) {
        span_t *current_span = cache->span[islot];
        എങ്കിൽ (!(current_span->flags & SPAN_FLAG_MASTER) ||
            ((current_span->flags & SPAN_FLAG_MASTER) &&
             (atomic_load32(&current_span->remaining_spans) <=
              (int32_t)current_span->span_എണ്ണം))) {
          _rpmalloc_span_unmap(current_span);
          cache->span[islot] = keep;
          നിർത്തുക;
        }
      }
      എങ്കിൽ (islot == cache->എണ്ണം)
        നിർത്തുക;
      keep = keep->next;
    }

    എങ്കിൽ (keep) {
      span_t *tail = keep;
      ആകുമ്പോൾ (tail->next)
        tail = tail->next;
      tail->next = cache->overflow;
      cache->overflow = keep;
    }

    atomic_store32_release(&cache->lock, 0);
  }
}

സ്ഥിതം size_t _rpmalloc_global_cache_extract_spans(span_t **span,
                                                   size_t span_എണ്ണം,
                                                   size_t എണ്ണം) {
  global_cache_t *cache = &_memory_span_cache[span_എണ്ണം - 1];

  size_t extract_എണ്ണം = 0;
  ആകുമ്പോൾ (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#എങ്കിൽ ENABLE_STATISTICS
  cache->extract_എണ്ണം += എണ്ണം;
#endif
  size_t want = എണ്ണം - extract_എണ്ണം;
  എങ്കിൽ (want > cache->എണ്ണം)
    want = cache->എണ്ണം;

  memcpy(span + extract_എണ്ണം, cache->span + (cache->എണ്ണം - want),
         വലുപ്പം(span_t *) * want);
  cache->എണ്ണം -= (uint32_t)want;
  extract_എണ്ണം += want;

  ആകുമ്പോൾ ((extract_എണ്ണം < എണ്ണം) && cache->overflow) {
    span_t *current_span = cache->overflow;
    span[extract_എണ്ണം++] = current_span;
    cache->overflow = current_span->next;
  }

#എങ്കിൽ ENABLE_ASSERTS
  വേണ്ടി (size_t ispan = 0; ispan < extract_എണ്ണം; ++ispan) {
    rpmalloc_assert(span[ispan]->span_എണ്ണം == span_എണ്ണം,
                    "Global cache span count mismatch");
  }
#endif

  atomic_store32_release(&cache->lock, 0);

  തിരികെ extract_എണ്ണം;
}

#endif

////////////
///
/// Heap control
///
//////

സ്ഥിതം ശൂന്യം _rpmalloc_deallocate_huge(span_t *);

//! Store the given spans as reserve in the given heap
സ്ഥിതം ശൂന്യം _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              size_t reserve_span_എണ്ണം) {
  heap->span_reserve_master = master;
  heap->span_reserve = reserve;
  heap->spans_reserved = (uint32_t)reserve_span_എണ്ണം;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
സ്ഥിതം ശൂന്യം _rpmalloc_heap_cache_adopt_deferred(heap_t *heap,
                                                span_t **single_span) {
  span_t *span = (span_t *)((ശൂന്യം *)atomic_exchange_ptr_acquire(
      &heap->span_free_deferred, 0));
  ആകുമ്പോൾ (span) {
    span_t *next_span = (span_t *)span->free_list;
    rpmalloc_assert(span->heap == heap, "Span heap pointer corrupted");
    എങ്കിൽ (EXPECTED(span->size_class < SIZE_CLASS_എണ്ണം)) {
      rpmalloc_assert(heap->full_span_എണ്ണം, "Heap span counter corrupted");
      --heap->full_span_എണ്ണം;
      _rpmalloc_stat_dec(&heap->span_use[0].spans_deferred);
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
      _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                             span);
#endif
      _rpmalloc_stat_dec(&heap->span_use[0].current);
      _rpmalloc_stat_dec(&heap->size_class_use[span->size_class].spans_current);
      എങ്കിൽ (single_span && !*single_span)
        *single_span = span;
      അല്ലെങ്കിൽ
        rpmalloc_heap_cache_തിരുകുക(heap, span);
    } അല്ലെങ്കിൽ {
      എങ്കിൽ (span->size_class == SIZE_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(span);
      } അല്ലെങ്കിൽ {
        rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(heap->full_span_എണ്ണം, "Heap span counter corrupted");
        --heap->full_span_എണ്ണം;
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
        _rpmalloc_span_double_link_list_remove(&heap->large_huge_span, span);
#endif
        uint32_t idx = span->span_എണ്ണം - 1;
        _rpmalloc_stat_dec(&heap->span_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&heap->span_use[idx].current);
        എങ്കിൽ (!idx && single_span && !*single_span)
          *single_span = span;
        അല്ലെങ്കിൽ
          rpmalloc_heap_cache_തിരുകുക(heap, span);
      }
    }
    span = next_span;
  }
}

സ്ഥിതം ശൂന്യം _rpmalloc_heap_unmap(heap_t *heap) {
  എങ്കിൽ (!heap->master_heap) {
    എങ്കിൽ ((heap->finalize > 1) && !atomic_load32(&heap->child_എണ്ണം)) {
      span_t *span = (span_t *)((uintptr_t)heap & _memory_span_mask);
      _rpmalloc_span_unmap(span);
    }
  } അല്ലെങ്കിൽ {
    എങ്കിൽ (atomic_decr32(&heap->master_heap->child_എണ്ണം) == 0) {
      _rpmalloc_heap_unmap(heap->master_heap);
    }
  }
}

സ്ഥിതം ശൂന്യം _rpmalloc_heap_global_finalize(heap_t *heap) {
  എങ്കിൽ (heap->finalize++ > 1) {
    --heap->finalize;
    തിരികെ;
  }

  _rpmalloc_heap_finalize(heap);

#എങ്കിൽ ENABLE_THREAD_CACHE
  വേണ്ടി (size_t iclass = 0; iclass < LARGE_CLASS_എണ്ണം; ++iclass) {
    span_cache_t *span_cache;
    എങ്കിൽ (!iclass)
      span_cache = &heap->span_cache;
    അല്ലെങ്കിൽ
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    വേണ്ടി (size_t ispan = 0; ispan < span_cache->എണ്ണം; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->എണ്ണം = 0;
  }
#endif

  എങ്കിൽ (heap->full_span_എണ്ണം) {
    --heap->finalize;
    തിരികെ;
  }

  വേണ്ടി (size_t iclass = 0; iclass < SIZE_CLASS_എണ്ണം; ++iclass) {
    എങ്കിൽ (heap->size_class[iclass].free_list ||
        heap->size_class[iclass].partial_span) {
      --heap->finalize;
      തിരികെ;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap_t *list_heap = _memory_heaps[list_idx];
  എങ്കിൽ (list_heap == heap) {
    _memory_heaps[list_idx] = heap->next_heap;
  } അല്ലെങ്കിൽ {
    ആകുമ്പോൾ (list_heap->next_heap != heap)
      list_heap = list_heap->next_heap;
    list_heap->next_heap = heap->next_heap;
  }

  _rpmalloc_heap_unmap(heap);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
സ്ഥിതം ശൂന്യം rpmalloc_heap_cache_തിരുകുക(heap_t *heap, span_t *span) {
  എങ്കിൽ (UNEXPECTED(heap->finalize != 0)) {
    _rpmalloc_span_unmap(span);
    _rpmalloc_heap_global_finalize(heap);
    തിരികെ;
  }
#എങ്കിൽ ENABLE_THREAD_CACHE
  size_t span_എണ്ണം = span->span_എണ്ണം;
  _rpmalloc_stat_inc(&heap->span_use[span_എണ്ണം - 1].spans_to_cache);
  എങ്കിൽ (span_എണ്ണം == 1) {
    span_cache_t *span_cache = &heap->span_cache;
    span_cache->span[span_cache->എണ്ണം++] = span;
    എങ്കിൽ (span_cache->എണ്ണം == MAX_THREAD_SPAN_CACHE) {
      സ്ഥിരം size_t remain_എണ്ണം =
          MAX_THREAD_SPAN_CACHE - THREAD_SPAN_CACHE_TRANSFER;
#എങ്കിൽ ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           THREAD_SPAN_CACHE_TRANSFER * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_എണ്ണം - 1].spans_to_global,
                         THREAD_SPAN_CACHE_TRANSFER);
      rpmalloc_global_cache_തിരുകുക_spans(span_cache->span + remain_എണ്ണം,
                                          span_എണ്ണം,
                                          THREAD_SPAN_CACHE_TRANSFER);
#അല്ലെങ്കിൽ
      വേണ്ടി (size_t ispan = 0; ispan < THREAD_SPAN_CACHE_TRANSFER; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_എണ്ണം + ispan]);
#endif
      span_cache->എണ്ണം = remain_എണ്ണം;
    }
  } അല്ലെങ്കിൽ {
    size_t cache_idx = span_എണ്ണം - 2;
    span_large_cache_t *span_cache = heap->span_large_cache + cache_idx;
    span_cache->span[span_cache->എണ്ണം++] = span;
    സ്ഥിരം size_t cache_limit =
        (MAX_THREAD_SPAN_LARGE_CACHE - (span_എണ്ണം >> 1));
    എങ്കിൽ (span_cache->എണ്ണം == cache_limit) {
      സ്ഥിരം size_t transfer_limit = 2 + (cache_limit >> 2);
      സ്ഥിരം size_t transfer_എണ്ണം =
          (THREAD_SPAN_LARGE_CACHE_TRANSFER <= transfer_limit
               ? THREAD_SPAN_LARGE_CACHE_TRANSFER
               : transfer_limit);
      സ്ഥിരം size_t remain_എണ്ണം = cache_limit - transfer_എണ്ണം;
#എങ്കിൽ ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           transfer_എണ്ണം * span_എണ്ണം * _memory_span_size);
      _rpmalloc_stat_add(&heap->span_use[span_എണ്ണം - 1].spans_to_global,
                         transfer_എണ്ണം);
      rpmalloc_global_cache_തിരുകുക_spans(span_cache->span + remain_എണ്ണം,
                                          span_എണ്ണം, transfer_എണ്ണം);
#അല്ലെങ്കിൽ
      വേണ്ടി (size_t ispan = 0; ispan < transfer_എണ്ണം; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_എണ്ണം + ispan]);
#endif
      span_cache->എണ്ണം = remain_എണ്ണം;
    }
  }
#അല്ലെങ്കിൽ
  (ശൂന്യം)വലുപ്പം(heap);
  _rpmalloc_span_unmap(span);
#endif
}

//! Extract the given number of spans from the different cache levels
സ്ഥിതം span_t *_rpmalloc_heap_thread_cache_extract(heap_t *heap,
                                                   size_t span_എണ്ണം) {
  span_t *span = 0;
#എങ്കിൽ ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  എങ്കിൽ (span_എണ്ണം == 1)
    span_cache = &heap->span_cache;
  അല്ലെങ്കിൽ
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_എണ്ണം - 2));
  എങ്കിൽ (span_cache->എണ്ണം) {
    _rpmalloc_stat_inc(&heap->span_use[span_എണ്ണം - 1].spans_from_cache);
    തിരികെ span_cache->span[--span_cache->എണ്ണം];
  }
#endif
  തിരികെ span;
}

സ്ഥിതം span_t *_rpmalloc_heap_thread_cache_deferred_extract(heap_t *heap,
                                                            size_t span_എണ്ണം) {
  span_t *span = 0;
  എങ്കിൽ (span_എണ്ണം == 1) {
    _rpmalloc_heap_cache_adopt_deferred(heap, &span);
  } അല്ലെങ്കിൽ {
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
    span = _rpmalloc_heap_thread_cache_extract(heap, span_എണ്ണം);
  }
  തിരികെ span;
}

സ്ഥിതം span_t *_rpmalloc_heap_reserved_extract(heap_t *heap,
                                               size_t span_എണ്ണം) {
  എങ്കിൽ (heap->spans_reserved >= span_എണ്ണം)
    തിരികെ _rpmalloc_span_map(heap, span_എണ്ണം);
  തിരികെ 0;
}

//! Extract a span from the global cache
സ്ഥിതം span_t *_rpmalloc_heap_global_cache_extract(heap_t *heap,
                                                   size_t span_എണ്ണം) {
#എങ്കിൽ ENABLE_GLOBAL_CACHE
#എങ്കിൽ ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  size_t wanted_എണ്ണം;
  എങ്കിൽ (span_എണ്ണം == 1) {
    span_cache = &heap->span_cache;
    wanted_എണ്ണം = THREAD_SPAN_CACHE_TRANSFER;
  } അല്ലെങ്കിൽ {
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_എണ്ണം - 2));
    wanted_എണ്ണം = THREAD_SPAN_LARGE_CACHE_TRANSFER;
  }
  span_cache->എണ്ണം = _rpmalloc_global_cache_extract_spans(
      span_cache->span, span_എണ്ണം, wanted_എണ്ണം);
  എങ്കിൽ (span_cache->എണ്ണം) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_എണ്ണം * span_cache->എണ്ണം * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_എണ്ണം - 1].spans_from_global,
                       span_cache->എണ്ണം);
    തിരികെ span_cache->span[--span_cache->എണ്ണം];
  }
#അല്ലെങ്കിൽ
  span_t *span = 0;
  size_t എണ്ണം = _rpmalloc_global_cache_extract_spans(&span, span_എണ്ണം, 1);
  എങ്കിൽ (എണ്ണം) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_എണ്ണം * എണ്ണം * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[span_എണ്ണം - 1].spans_from_global,
                       എണ്ണം);
    തിരികെ span;
  }
#endif
#endif
  (ശൂന്യം)വലുപ്പം(heap);
  (ശൂന്യം)വലുപ്പം(span_എണ്ണം);
  തിരികെ 0;
}

സ്ഥിതം ശൂന്യം _rpmalloc_inc_span_statistics(heap_t *heap, size_t span_എണ്ണം,
                                          uint32_t class_idx) {
  (ശൂന്യം)വലുപ്പം(heap);
  (ശൂന്യം)വലുപ്പം(span_എണ്ണം);
  (ശൂന്യം)വലുപ്പം(class_idx);
#എങ്കിൽ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)span_എണ്ണം - 1;
  uint32_t current_എണ്ണം =
      (uint32_t)atomic_incr32(&heap->span_use[idx].current);
  എങ്കിൽ (current_എണ്ണം > (uint32_t)atomic_load32(&heap->span_use[idx].മുകളിലെ))
    atomic_store32(&heap->span_use[idx].മുകളിലെ, (int32_t)current_എണ്ണം);
  _rpmalloc_stat_add_peak(&heap->size_class_use[class_idx].spans_current, 1,
                          heap->size_class_use[class_idx].spans_peak);
#endif
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
സ്ഥിതം span_t *
_rpmalloc_heap_extract_new_span(heap_t *heap,
                                heap_size_class_t *heap_size_class,
                                size_t span_എണ്ണം, uint32_t class_idx) {
  span_t *span;
#എങ്കിൽ ENABLE_THREAD_CACHE
  എങ്കിൽ (heap_size_class && heap_size_class->cache) {
    span = heap_size_class->cache;
    heap_size_class->cache =
        (heap->span_cache.എണ്ണം
             ? heap->span_cache.span[--heap->span_cache.എണ്ണം]
             : 0);
    _rpmalloc_inc_span_statistics(heap, span_എണ്ണം, class_idx);
    തിരികെ span;
  }
#endif
  (ശൂന്യം)വലുപ്പം(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t base_span_എണ്ണം = span_എണ്ണം;
  size_t limit_span_എണ്ണം =
      (span_എണ്ണം > 2) ? (span_എണ്ണം + (span_എണ്ണം >> 1)) : span_എണ്ണം;
  എങ്കിൽ (limit_span_എണ്ണം > LARGE_CLASS_എണ്ണം)
    limit_span_എണ്ണം = LARGE_CLASS_എണ്ണം;
  ചെയ്യുക {
    span = _rpmalloc_heap_thread_cache_extract(heap, span_എണ്ണം);
    എങ്കിൽ (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_എണ്ണം, class_idx);
      തിരികെ span;
    }
    span = _rpmalloc_heap_thread_cache_deferred_extract(heap, span_എണ്ണം);
    എങ്കിൽ (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_എണ്ണം, class_idx);
      തിരികെ span;
    }
    span = _rpmalloc_heap_global_cache_extract(heap, span_എണ്ണം);
    എങ്കിൽ (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_എണ്ണം, class_idx);
      തിരികെ span;
    }
    span = _rpmalloc_heap_reserved_extract(heap, span_എണ്ണം);
    എങ്കിൽ (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_from_reserved);
      _rpmalloc_inc_span_statistics(heap, span_എണ്ണം, class_idx);
      തിരികെ span;
    }
    ++span_എണ്ണം;
  } ആകുമ്പോൾ (span_എണ്ണം <= limit_span_എണ്ണം);
  // Final fallback, map in more virtual memory
  span = _rpmalloc_span_map(heap, base_span_എണ്ണം);
  _rpmalloc_inc_span_statistics(heap, base_span_എണ്ണം, class_idx);
  _rpmalloc_stat_inc(&heap->size_class_use[class_idx].spans_map_calls);
  തിരികെ span;
}

സ്ഥിതം ശൂന്യം _rpmalloc_heap_initialize(heap_t *heap) {
  _rpmalloc_memset_const(heap, 0, വലുപ്പം(heap_t));
  // Get a new heap ID
  heap->id = 1 + atomic_incr32(&_memory_heap_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_SIZE;
  heap->next_heap = _memory_heaps[list_idx];
  _memory_heaps[list_idx] = heap;
}

സ്ഥിതം ശൂന്യം _rpmalloc_heap_orphan(heap_t *heap, പൂർണ്ണസംഖ്യ first_class) {
  heap->owner_thread = (uintptr_t)-1;
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
  heap_t **heap_list =
      (first_class ? &_memory_first_class_orphan_heaps : &_memory_orphan_heaps);
#അല്ലെങ്കിൽ
  (ശൂന്യം)വലുപ്പം(first_class);
  heap_t **heap_list = &_memory_orphan_heaps;
#endif
  heap->next_orphan = *heap_list;
  *heap_list = heap;
}

//! Allocate a new heap from newly mapped memory pages
സ്ഥിതം heap_t *_rpmalloc_heap_allocate_new(ശൂന്യം) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t heap_size = വലുപ്പം(heap_t);
  size_t aligned_heap_size = 16 * ((heap_size + 15) / 16);
  size_t request_heap_എണ്ണം = 16;
  size_t heap_span_എണ്ണം = ((aligned_heap_size * request_heap_എണ്ണം) +
                            വലുപ്പം(span_t) + _memory_span_size - 1) /
                           _memory_span_size;
  size_t block_size = _memory_span_size * heap_span_എണ്ണം;
  size_t span_എണ്ണം = heap_span_എണ്ണം;
  span_t *span = 0;
  // If there are global reserved spans, use these first
  എങ്കിൽ (memory_global_reserve_എണ്ണം >= heap_span_എണ്ണം) {
    span = _rpmalloc_global_get_reserved_spans(heap_span_എണ്ണം);
  }
  എങ്കിൽ (!span) {
    എങ്കിൽ (_memory_page_size > block_size) {
      span_എണ്ണം = _memory_page_size / _memory_span_size;
      block_size = _memory_page_size;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_heap_എണ്ണം =
          (block_size - വലുപ്പം(span_t)) / aligned_heap_size;
      എങ്കിൽ (possible_heap_എണ്ണം >= (request_heap_എണ്ണം * 16))
        request_heap_എണ്ണം *= 16;
      അല്ലെങ്കിൽ എങ്കിൽ (possible_heap_എണ്ണം < request_heap_എണ്ണം)
        request_heap_എണ്ണം = possible_heap_എണ്ണം;
      heap_span_എണ്ണം = ((aligned_heap_size * request_heap_എണ്ണം) +
                         വലുപ്പം(span_t) + _memory_span_size - 1) /
                        _memory_span_size;
    }

    size_t align_offset = 0;
    span = (span_t *)_rpmalloc_mmap(block_size, &align_offset);
    എങ്കിൽ (!span)
      തിരികെ 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    _rpmalloc_span_initialize(span, span_എണ്ണം, heap_span_എണ്ണം, align_offset);
  }

  size_t remain_size = _memory_span_size - വലുപ്പം(span_t);
  heap_t *heap = (heap_t *)pointer_offset(span, വലുപ്പം(span_t));
  _rpmalloc_heap_initialize(heap);

  // Put extra heaps as orphans
  size_t അക്കം_heaps = remain_size / aligned_heap_size;
  എങ്കിൽ (അക്കം_heaps < request_heap_എണ്ണം)
    അക്കം_heaps = request_heap_എണ്ണം;
  atomic_store32(&heap->child_എണ്ണം, (int32_t)അക്കം_heaps - 1);
  heap_t *extra_heap = (heap_t *)pointer_offset(heap, aligned_heap_size);
  ആകുമ്പോൾ (അക്കം_heaps > 1) {
    _rpmalloc_heap_initialize(extra_heap);
    extra_heap->master_heap = heap;
    _rpmalloc_heap_orphan(extra_heap, 1);
    extra_heap = (heap_t *)pointer_offset(extra_heap, aligned_heap_size);
    --അക്കം_heaps;
  }

  എങ്കിൽ (span_എണ്ണം > heap_span_എണ്ണം) {
    // Cap reserved spans
    size_t remain_എണ്ണം = span_എണ്ണം - heap_span_എണ്ണം;
    size_t reserve_എണ്ണം =
        (remain_എണ്ണം > memory_heap_reserve_എണ്ണം ? memory_heap_reserve_എണ്ണം
                                                   : remain_എണ്ണം);
    span_t *remain_span =
        (span_t *)pointer_offset(span, heap_span_എണ്ണം * _memory_span_size);
    _rpmalloc_heap_set_reserved_spans(heap, span, remain_span, reserve_എണ്ണം);

    എങ്കിൽ (remain_എണ്ണം > reserve_എണ്ണം) {
      // Set to global reserved spans
      remain_span = (span_t *)pointer_offset(remain_span,
                                             reserve_എണ്ണം * _memory_span_size);
      reserve_എണ്ണം = remain_എണ്ണം - reserve_എണ്ണം;
      _rpmalloc_global_set_reserved_spans(span, remain_span, reserve_എണ്ണം);
    }
  }

  തിരികെ heap;
}

സ്ഥിതം heap_t *_rpmalloc_heap_extract_orphan(heap_t **heap_list) {
  heap_t *heap = *heap_list;
  *heap_list = (heap ? heap->next_orphan : 0);
  തിരികെ heap;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
സ്ഥിതം heap_t *_rpmalloc_heap_allocate(പൂർണ്ണസംഖ്യ first_class) {
  heap_t *heap = 0;
  ആകുമ്പോൾ (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
    _rpmalloc_spin();
  എങ്കിൽ (first_class == 0)
    heap = _rpmalloc_heap_extract_orphan(&_memory_orphan_heaps);
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
  എങ്കിൽ (!heap)
    heap = _rpmalloc_heap_extract_orphan(&_memory_first_class_orphan_heaps);
#endif
  എങ്കിൽ (!heap)
    heap = _rpmalloc_heap_allocate_new();
  atomic_store32_release(&_memory_global_lock, 0);
  എങ്കിൽ (heap)
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  തിരികെ heap;
}

സ്ഥിതം ശൂന്യം _rpmalloc_heap_release(ശൂന്യം *heapptr, പൂർണ്ണസംഖ്യ first_class,
                                   പൂർണ്ണസംഖ്യ release_cache) {
  heap_t *heap = (heap_t *)heapptr;
  എങ്കിൽ (!heap)
    തിരികെ;
  // Release thread cache spans back to global cache
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  എങ്കിൽ (release_cache || heap->finalize) {
#എങ്കിൽ ENABLE_THREAD_CACHE
    വേണ്ടി (size_t iclass = 0; iclass < LARGE_CLASS_എണ്ണം; ++iclass) {
      span_cache_t *span_cache;
      എങ്കിൽ (!iclass)
        span_cache = &heap->span_cache;
      അല്ലെങ്കിൽ
        span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
      എങ്കിൽ (!span_cache->എണ്ണം)
        തുടരുക;
#എങ്കിൽ ENABLE_GLOBAL_CACHE
      എങ്കിൽ (heap->finalize) {
        വേണ്ടി (size_t ispan = 0; ispan < span_cache->എണ്ണം; ++ispan)
          _rpmalloc_span_unmap(span_cache->span[ispan]);
      } അല്ലെങ്കിൽ {
        _rpmalloc_stat_add64(&heap->thread_to_global, span_cache->എണ്ണം *
                                                          (iclass + 1) *
                                                          _memory_span_size);
        _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                           span_cache->എണ്ണം);
        rpmalloc_global_cache_തിരുകുക_spans(span_cache->span, iclass + 1,
                                            span_cache->എണ്ണം);
      }
#അല്ലെങ്കിൽ
      വേണ്ടി (size_t ispan = 0; ispan < span_cache->എണ്ണം; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
      span_cache->എണ്ണം = 0;
    }
#endif
  }

  എങ്കിൽ (get_thread_heap_raw() == heap)
    set_thread_heap(0);

#എങ്കിൽ ENABLE_STATISTICS
  atomic_decr32(&_memory_active_heaps);
  rpmalloc_assert(atomic_load32(&_memory_active_heaps) >= 0,
                  "Still active heaps during finalization");
#endif

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  എങ്കിൽ (get_thread_id() != _rpmalloc_main_thread_id) {
    ആകുമ്പോൾ (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
  }
  _rpmalloc_heap_orphan(heap, first_class);
  atomic_store32_release(&_memory_global_lock, 0);
}

സ്ഥിതം ശൂന്യം _rpmalloc_heap_release_raw(ശൂന്യം *heapptr, പൂർണ്ണസംഖ്യ release_cache) {
  _rpmalloc_heap_release(heapptr, 0, release_cache);
}

സ്ഥിതം ശൂന്യം _rpmalloc_heap_release_raw_fc(ശൂന്യം *heapptr) {
  _rpmalloc_heap_release_raw(heapptr, 1);
}

സ്ഥിതം ശൂന്യം _rpmalloc_heap_finalize(heap_t *heap) {
  എങ്കിൽ (heap->spans_reserved) {
    span_t *span = _rpmalloc_span_map(heap, heap->spans_reserved);
    _rpmalloc_span_unmap(span);
    heap->spans_reserved = 0;
  }

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  വേണ്ടി (size_t iclass = 0; iclass < SIZE_CLASS_എണ്ണം; ++iclass) {
    എങ്കിൽ (heap->size_class[iclass].cache)
      _rpmalloc_span_unmap(heap->size_class[iclass].cache);
    heap->size_class[iclass].cache = 0;
    span_t *span = heap->size_class[iclass].partial_span;
    ആകുമ്പോൾ (span) {
      span_t *next = span->next;
      _rpmalloc_span_finalize(heap, iclass, span,
                              &heap->size_class[iclass].partial_span);
      span = next;
    }
    // If class still has a free list it must be a full span
    എങ്കിൽ (heap->size_class[iclass].free_list) {
      span_t *class_span =
          (span_t *)((uintptr_t)heap->size_class[iclass].free_list &
                     _memory_span_mask);
      span_t **list = 0;
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
      list = &heap->full_span[iclass];
#endif
      --heap->full_span_എണ്ണം;
      എങ്കിൽ (!_rpmalloc_span_finalize(heap, iclass, class_span, list)) {
        എങ്കിൽ (list)
          _rpmalloc_span_double_link_list_remove(list, class_span);
        _rpmalloc_span_double_link_list_add(
            &heap->size_class[iclass].partial_span, class_span);
      }
    }
  }

#എങ്കിൽ ENABLE_THREAD_CACHE
  വേണ്ടി (size_t iclass = 0; iclass < LARGE_CLASS_എണ്ണം; ++iclass) {
    span_cache_t *span_cache;
    എങ്കിൽ (!iclass)
      span_cache = &heap->span_cache;
    അല്ലെങ്കിൽ
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    വേണ്ടി (size_t ispan = 0; ispan < span_cache->എണ്ണം; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->എണ്ണം = 0;
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
സ്ഥിതം ശൂന്യം *free_list_എടുക്കുക(ശൂന്യം **list) {
  ശൂന്യം *block = *list;
  *list = *((ശൂന്യം **)block);
  തിരികെ block;
}

//! Allocate a small/medium sized memory block from the given heap
സ്ഥിതം ശൂന്യം *_rpmalloc_allocate_from_heap_fallback(
    heap_t *heap, heap_size_class_t *heap_size_class, uint32_t class_idx) {
  span_t *span = heap_size_class->partial_span;
  rpmalloc_assume(heap != 0);
  എങ്കിൽ (EXPECTED(span != 0)) {
    rpmalloc_assert(span->block_എണ്ണം ==
                        _memory_size_class[span->size_class].block_എണ്ണം,
                    "Span block count corrupted");
    rpmalloc_assert(!_rpmalloc_span_is_fully_utilized(span),
                    "Internal failure");
    ശൂന്യം *block;
    എങ്കിൽ (span->free_list) {
      // Span local free list is not empty, swap to size class free list
      block = free_list_എടുക്കുക(&span->free_list);
      heap_size_class->free_list = span->free_list;
      span->free_list = 0;
    } അല്ലെങ്കിൽ {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      ശൂന്യം *block_start = pointer_offset(
          span, SPAN_HEADER_SIZE +
                    ((size_t)span->free_list_limit * span->block_size));
      span->free_list_limit += free_list_partial_init(
          &heap_size_class->free_list, &block,
          (ശൂന്യം *)((uintptr_t)block_start & ~(_memory_page_size - 1)),
          block_start, span->block_എണ്ണം - span->free_list_limit,
          span->block_size);
    }
    rpmalloc_assert(span->free_list_limit <= span->block_എണ്ണം,
                    "Span block count corrupted");
    span->used_എണ്ണം = span->free_list_limit;

    // Swap in deferred free list if present
    എങ്കിൽ (atomic_load_ptr(&span->free_list_deferred))
      _rpmalloc_span_extract_free_list_deferred(span);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    എങ്കിൽ (!_rpmalloc_span_is_fully_utilized(span))
      തിരികെ block;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_span_double_link_list_എടുക്കുക_head(&heap_size_class->partial_span,
                                             span);
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#endif
    ++heap->full_span_എണ്ണം;
    തിരികെ block;
  }

  // Find a span in one of the cache levels
  span = _rpmalloc_heap_extract_new_span(heap, heap_size_class, 1, class_idx);
  എങ്കിൽ (EXPECTED(span != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    തിരികെ _rpmalloc_span_initialize_new(heap, heap_size_class, span,
                                         class_idx);
  }

  തിരികെ 0;
}

//! Allocate a small sized memory block from the given heap
സ്ഥിതം ശൂന്യം *_rpmalloc_allocate_small(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Small sizes have unique size classes
  സ്ഥിരം uint32_t class_idx =
      (uint32_t)((size + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_SHIFT);
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  എങ്കിൽ (EXPECTED(heap_size_class->free_list != 0))
    തിരികെ free_list_എടുക്കുക(&heap_size_class->free_list);
  തിരികെ _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
സ്ഥിതം ശൂന്യം *_rpmalloc_allocate_medium(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  സ്ഥിരം uint32_t base_idx =
      (uint32_t)(SMALL_CLASS_എണ്ണം +
                 ((size - (SMALL_SIZE_LIMIT + 1)) >> MEDIUM_GRANULARITY_SHIFT));
  സ്ഥിരം uint32_t class_idx = _memory_size_class[base_idx].class_idx;
  heap_size_class_t *heap_size_class = heap->size_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  എങ്കിൽ (EXPECTED(heap_size_class->free_list != 0))
    തിരികെ free_list_എടുക്കുക(&heap_size_class->free_list);
  തിരികെ _rpmalloc_allocate_from_heap_fallback(heap, heap_size_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
സ്ഥിതം ശൂന്യം *_rpmalloc_allocate_large(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  size += SPAN_HEADER_SIZE;
  size_t span_എണ്ണം = size >> _memory_span_size_shift;
  എങ്കിൽ (size & (_memory_span_size - 1))
    ++span_എണ്ണം;

  // Find a span in one of the cache levels
  span_t *span =
      _rpmalloc_heap_extract_new_span(heap, 0, span_എണ്ണം, SIZE_CLASS_LARGE);
  എങ്കിൽ (!span)
    തിരികെ span;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(span->span_എണ്ണം >= span_എണ്ണം, "Internal failure");
  span->size_class = SIZE_CLASS_LARGE;
  span->heap = heap;

#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_എണ്ണം;

  തിരികെ pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a huge block by mapping memory pages directly
സ്ഥിതം ശൂന്യം *_rpmalloc_allocate_huge(heap_t *heap, size_t size) {
  rpmalloc_assert(heap, "No thread heap");
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  size += SPAN_HEADER_SIZE;
  size_t അക്കം_pages = size >> _memory_page_size_shift;
  എങ്കിൽ (size & (_memory_page_size - 1))
    ++അക്കം_pages;
  size_t align_offset = 0;
  span_t *span =
      (span_t *)_rpmalloc_mmap(അക്കം_pages * _memory_page_size, &align_offset);
  എങ്കിൽ (!span)
    തിരികെ span;

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_എണ്ണം = (uint32_t)അക്കം_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, അക്കം_pages, _huge_pages_peak);

#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_എണ്ണം;

  തിരികെ pointer_offset(span, SPAN_HEADER_SIZE);
}

//! Allocate a block of the given size
സ്ഥിതം ശൂന്യം *_rpmalloc_allocate(heap_t *heap, size_t size) {
  _rpmalloc_stat_add64(&_allocation_counter, 1);
  എങ്കിൽ (EXPECTED(size <= SMALL_SIZE_LIMIT))
    തിരികെ _rpmalloc_allocate_small(heap, size);
  അല്ലെങ്കിൽ എങ്കിൽ (size <= _memory_medium_size_limit)
    തിരികെ _rpmalloc_allocate_medium(heap, size);
  അല്ലെങ്കിൽ എങ്കിൽ (size <= LARGE_SIZE_LIMIT)
    തിരികെ _rpmalloc_allocate_large(heap, size);
  തിരികെ _rpmalloc_allocate_huge(heap, size);
}

സ്ഥിതം ശൂന്യം *_rpmalloc_aligned_allocate(heap_t *heap, size_t alignment,
                                        size_t size) {
  എങ്കിൽ (alignment <= SMALL_GRANULARITY)
    തിരികെ _rpmalloc_allocate(heap, size);

#എങ്കിൽ ENABLE_VALIDATE_ARGS
  എങ്കിൽ ((size + alignment) < size) {
    errno = EINVAL;
    തിരികെ 0;
  }
  എങ്കിൽ (alignment & (alignment - 1)) {
    errno = EINVAL;
    തിരികെ 0;
  }
#endif

  എങ്കിൽ ((alignment <= SPAN_HEADER_SIZE) &&
      ((size + SPAN_HEADER_SIZE) < _memory_medium_size_limit)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t multiple_size = size ? (size + (SPAN_HEADER_SIZE - 1)) &
                                      ~(uintptr_t)(SPAN_HEADER_SIZE - 1)
                                : SPAN_HEADER_SIZE;
    rpmalloc_assert(!(multiple_size % SPAN_HEADER_SIZE),
                    "Failed alignment calculation");
    എങ്കിൽ (multiple_size <= (size + alignment))
      തിരികെ _rpmalloc_allocate(heap, multiple_size);
  }

  ശൂന്യം *ptr = 0;
  size_t align_mask = alignment - 1;
  എങ്കിൽ (alignment <= _memory_page_size) {
    ptr = _rpmalloc_allocate(heap, size + alignment);
    എങ്കിൽ ((uintptr_t)ptr & align_mask) {
      ptr = (ശൂന്യം *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);
      // Mark as having aligned blocks
      span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
      span->flags |= SPAN_FLAG_ALIGNED_BLOCKS;
    }
    തിരികെ ptr;
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
  എങ്കിൽ (alignment & align_mask) {
    errno = EINVAL;
    തിരികെ 0;
  }
  എങ്കിൽ (alignment >= _memory_span_size) {
    errno = EINVAL;
    തിരികെ 0;
  }

  size_t extra_pages = alignment / _memory_page_size;

  // Since each span has a header, we will at least need one extra memory page
  size_t അക്കം_pages = 1 + (size / _memory_page_size);
  എങ്കിൽ (size & (_memory_page_size - 1))
    ++അക്കം_pages;

  എങ്കിൽ (extra_pages > അക്കം_pages)
    അക്കം_pages = 1 + extra_pages;

  size_t original_pages = അക്കം_pages;
  size_t limit_pages = (_memory_span_size / _memory_page_size) * 2;
  എങ്കിൽ (limit_pages < (original_pages * 2))
    limit_pages = original_pages * 2;

  size_t mapped_size, align_offset;
  span_t *span;

retry:
  align_offset = 0;
  mapped_size = അക്കം_pages * _memory_page_size;

  span = (span_t *)_rpmalloc_mmap(mapped_size, &align_offset);
  എങ്കിൽ (!span) {
    errno = ENOMEM;
    തിരികെ 0;
  }
  ptr = pointer_offset(span, SPAN_HEADER_SIZE);

  എങ്കിൽ ((uintptr_t)ptr & align_mask)
    ptr = (ശൂന്യം *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);

  എങ്കിൽ (((size_t)pointer_diff(ptr, span) >= _memory_span_size) ||
      (pointer_offset(ptr, size) > pointer_offset(span, mapped_size)) ||
      (((uintptr_t)ptr & _memory_span_mask) != (uintptr_t)span)) {
    _rpmalloc_unmap(span, mapped_size, align_offset, mapped_size);
    ++അക്കം_pages;
    എങ്കിൽ (അക്കം_pages > limit_pages) {
      errno = EINVAL;
      തിരികെ 0;
    }
    പോകുക retry;
  }

  // Store page count in span_count
  span->size_class = SIZE_CLASS_HUGE;
  span->span_എണ്ണം = (uint32_t)അക്കം_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, അക്കം_pages, _huge_pages_peak);

#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#endif
  ++heap->full_span_എണ്ണം;

  _rpmalloc_stat_add64(&_allocation_counter, 1);

  തിരികെ ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
സ്ഥിതം ശൂന്യം _rpmalloc_deallocate_direct_small_or_medium(span_t *span,
                                                        ശൂന്യം *block) {
  heap_t *heap = span->heap;
  rpmalloc_assert(heap->owner_thread == get_thread_id() ||
                      !heap->owner_thread || heap->finalize,
                  "Internal failure");
  // Add block to free list
  എങ്കിൽ (UNEXPECTED(_rpmalloc_span_is_fully_utilized(span))) {
    span->used_എണ്ണം = span->block_എണ്ണം;
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_remove(&heap->full_span[span->size_class],
                                           span);
#endif
    _rpmalloc_span_double_link_list_add(
        &heap->size_class[span->size_class].partial_span, span);
    --heap->full_span_എണ്ണം;
  }
  *((ശൂന്യം **)block) = span->free_list;
  --span->used_എണ്ണം;
  span->free_list = block;
  എങ്കിൽ (UNEXPECTED(span->used_എണ്ണം == span->list_size)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    എങ്കിൽ (span->used_എണ്ണം) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      ശൂന്യം *free_list;
      ചെയ്യുക {
        free_list = atomic_exchange_ptr_acquire(&span->free_list_deferred,
                                                INVALID_POINTER);
      } ആകുമ്പോൾ (free_list == INVALID_POINTER);
      atomic_store_ptr_release(&span->free_list_deferred, free_list);
    }
    _rpmalloc_span_double_link_list_remove(
        &heap->size_class[span->size_class].partial_span, span);
    _rpmalloc_span_release_to_cache(heap, span);
  }
}

സ്ഥിതം ശൂന്യം _rpmalloc_deallocate_defer_free_span(heap_t *heap, span_t *span) {
  എങ്കിൽ (span->size_class != SIZE_CLASS_HUGE)
    _rpmalloc_stat_inc(&heap->span_use[span->span_എണ്ണം - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  ചെയ്യുക {
    span->free_list = (ശൂന്യം *)atomic_load_ptr(&heap->span_free_deferred);
  } ആകുമ്പോൾ (!atomic_cas_ptr(&heap->span_free_deferred, span, span->free_list));
}

//! Put the block in the deferred free list of the owning span
സ്ഥിതം ശൂന്യം _rpmalloc_deallocate_defer_small_or_medium(span_t *span,
                                                       ശൂന്യം *block) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  ശൂന്യം *free_list;
  ചെയ്യുക {
    free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } ആകുമ്പോൾ (free_list == INVALID_POINTER);
  *((ശൂന്യം **)block) = free_list;
  uint32_t free_എണ്ണം = ++span->list_size;
  പൂർണ്ണസംഖ്യ all_deferred_free = (free_എണ്ണം == span->block_എണ്ണം);
  atomic_store_ptr_release(&span->free_list_deferred, block);
  എങ്കിൽ (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
  }
}

സ്ഥിതം ശൂന്യം _rpmalloc_deallocate_small_or_medium(span_t *span, ശൂന്യം *p) {
  _rpmalloc_stat_inc_free(span->heap, span->size_class);
  എങ്കിൽ (span->flags & SPAN_FLAG_ALIGNED_BLOCKS) {
    // Realign pointer to block start
    ശൂന്യം *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
    p = pointer_offset(p, -(int32_t)(block_offset % span->block_size));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
  പൂർണ്ണസംഖ്യ defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#അല്ലെങ്കിൽ
  പൂർണ്ണസംഖ്യ defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  എങ്കിൽ (!defer)
    _rpmalloc_deallocate_direct_small_or_medium(span, p);
  അല്ലെങ്കിൽ
    _rpmalloc_deallocate_defer_small_or_medium(span, p);
}

//! Deallocate the given large memory block to the current heap
സ്ഥിതം ശൂന്യം _rpmalloc_deallocate_large(span_t *span) {
  rpmalloc_assert(span->size_class == SIZE_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
  പൂർണ്ണസംഖ്യ defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#അല്ലെങ്കിൽ
  പൂർണ്ണസംഖ്യ defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  എങ്കിൽ (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    തിരികെ;
  }
  rpmalloc_assert(span->heap->full_span_എണ്ണം, "Heap span counter corrupted");
  --span->heap->full_span_എണ്ണം;
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif
#എങ്കിൽ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = span->span_എണ്ണം - 1;
  atomic_decr32(&span->heap->span_use[idx].current);
#endif
  heap_t *heap = span->heap;
  rpmalloc_assert(heap, "No thread heap");
#എങ്കിൽ ENABLE_THREAD_CACHE
  സ്ഥിരം പൂർണ്ണസംഖ്യ set_as_reserved =
      ((span->span_എണ്ണം > 1) && (heap->span_cache.എണ്ണം == 0) &&
       !heap->finalize && !heap->spans_reserved);
#അല്ലെങ്കിൽ
  സ്ഥിരം പൂർണ്ണസംഖ്യ set_as_reserved =
      ((span->span_എണ്ണം > 1) && !heap->finalize && !heap->spans_reserved);
#endif
  എങ്കിൽ (set_as_reserved) {
    heap->span_reserve = span;
    heap->spans_reserved = span->span_എണ്ണം;
    എങ്കിൽ (span->flags & SPAN_FLAG_MASTER) {
      heap->span_reserve_master = span;
    } അല്ലെങ്കിൽ { // SPAN_FLAG_SUBSPAN
      span_t *master = (span_t *)pointer_offset(
          span,
          -(intptr_t)((size_t)span->offset_from_master * _memory_span_size));
      heap->span_reserve_master = master;
      rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->remaining_spans) >=
                          (int32_t)span->span_എണ്ണം,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&heap->span_use[idx].spans_to_reserved);
  } അല്ലെങ്കിൽ {
    // Insert into cache list
    rpmalloc_heap_cache_തിരുകുക(heap, span);
  }
}

//! Deallocate the given huge span
സ്ഥിതം ശൂന്യം _rpmalloc_deallocate_huge(span_t *span) {
  rpmalloc_assert(span->heap, "No span heap");
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
  പൂർണ്ണസംഖ്യ defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#അല്ലെങ്കിൽ
  പൂർണ്ണസംഖ്യ defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#endif
  എങ്കിൽ (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    തിരികെ;
  }
  rpmalloc_assert(span->heap->full_span_എണ്ണം, "Heap span counter corrupted");
  --span->heap->full_span_എണ്ണം;
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#endif

  // Oversized allocation, page count is stored in span_count
  size_t അക്കം_pages = span->span_എണ്ണം;
  _rpmalloc_unmap(span, അക്കം_pages * _memory_page_size, span->align_offset,
                  അക്കം_pages * _memory_page_size);
  _rpmalloc_stat_sub(&_huge_pages_current, അക്കം_pages);
}

//! Deallocate the given block
സ്ഥിതം ശൂന്യം _rpmalloc_deallocate(ശൂന്യം *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  എങ്കിൽ (UNEXPECTED(!span))
    തിരികെ;
  എങ്കിൽ (EXPECTED(span->size_class < SIZE_CLASS_എണ്ണം))
    _rpmalloc_deallocate_small_or_medium(span, p);
  അല്ലെങ്കിൽ എങ്കിൽ (span->size_class == SIZE_CLASS_LARGE)
    _rpmalloc_deallocate_large(span);
  അല്ലെങ്കിൽ
    _rpmalloc_deallocate_huge(span);
}

////////////
///
/// Reallocation entry points
///
//////

സ്ഥിതം size_t _rpmalloc_usable_size(ശൂന്യം *p);

//! Reallocate the given block to the given size
സ്ഥിതം ശൂന്യം *_rpmalloc_reallocate(heap_t *heap, ശൂന്യം *p, size_t size,
                                  size_t oldsize, unsigned പൂർണ്ണസംഖ്യ flags) {
  എങ്കിൽ (p) {
    // Grab the span using guaranteed span alignment
    span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
    എങ്കിൽ (EXPECTED(span->size_class < SIZE_CLASS_എണ്ണം)) {
      // Small/medium sized block
      rpmalloc_assert(span->span_എണ്ണം == 1, "Span counter corrupted");
      ശൂന്യം *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
      uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
      uint32_t block_idx = block_offset / span->block_size;
      ശൂന്യം *block =
          pointer_offset(blocks_start, (size_t)block_idx * span->block_size);
      എങ്കിൽ (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)span->block_size - pointer_diff(p, block));
      എങ്കിൽ ((size_t)span->block_size >= size) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        എങ്കിൽ ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        തിരികെ block;
      }
    } അല്ലെങ്കിൽ എങ്കിൽ (span->size_class == SIZE_CLASS_LARGE) {
      // Large block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t അക്കം_spans = total_size >> _memory_span_size_shift;
      എങ്കിൽ (total_size & (_memory_span_mask - 1))
        ++അക്കം_spans;
      size_t current_spans = span->span_എണ്ണം;
      ശൂന്യം *block = pointer_offset(span, SPAN_HEADER_SIZE);
      എങ്കിൽ (!oldsize)
        oldsize = (current_spans * _memory_span_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      എങ്കിൽ ((current_spans >= അക്കം_spans) && (total_size >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        എങ്കിൽ ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        തിരികെ block;
      }
    } അല്ലെങ്കിൽ {
      // Oversized block
      size_t total_size = size + SPAN_HEADER_SIZE;
      size_t അക്കം_pages = total_size >> _memory_page_size_shift;
      എങ്കിൽ (total_size & (_memory_page_size - 1))
        ++അക്കം_pages;
      // Page count is stored in span_count
      size_t current_pages = span->span_എണ്ണം;
      ശൂന്യം *block = pointer_offset(span, SPAN_HEADER_SIZE);
      എങ്കിൽ (!oldsize)
        oldsize = (current_pages * _memory_page_size) -
                  (size_t)pointer_diff(p, block) - SPAN_HEADER_SIZE;
      എങ്കിൽ ((current_pages >= അക്കം_pages) && (അക്കം_pages >= (current_pages / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        എങ്കിൽ ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(block, p, oldsize);
        തിരികെ block;
      }
    }
  } അല്ലെങ്കിൽ {
    oldsize = 0;
  }

  എങ്കിൽ (!!(flags & RPMALLOC_GROW_OR_FAIL))
    തിരികെ 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t lower_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_size =
      (size > lower_bound) ? size : ((size > oldsize) ? lower_bound : size);
  ശൂന്യം *block = _rpmalloc_allocate(heap, new_size);
  എങ്കിൽ (p && block) {
    എങ്കിൽ (!(flags & RPMALLOC_NO_PRESERVE))
      memcpy(block, p, oldsize < new_size ? oldsize : new_size);
    _rpmalloc_deallocate(p);
  }

  തിരികെ block;
}

സ്ഥിതം ശൂന്യം *_rpmalloc_aligned_reallocate(heap_t *heap, ശൂന്യം *ptr,
                                          size_t alignment, size_t size,
                                          size_t oldsize, unsigned പൂർണ്ണസംഖ്യ flags) {
  എങ്കിൽ (alignment <= SMALL_GRANULARITY)
    തിരികെ _rpmalloc_reallocate(heap, ptr, size, oldsize, flags);

  പൂർണ്ണസംഖ്യ no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? _rpmalloc_usable_size(ptr) : 0);
  എങ്കിൽ ((usablesize >= size) && !((uintptr_t)ptr & (alignment - 1))) {
    എങ്കിൽ (no_alloc || (size >= (usablesize / 2)))
      തിരികെ ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  ശൂന്യം *block =
      (!no_alloc ? _rpmalloc_aligned_allocate(heap, alignment, size) : 0);
  എങ്കിൽ (EXPECTED(block != 0)) {
    എങ്കിൽ (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      എങ്കിൽ (!oldsize)
        oldsize = usablesize;
      memcpy(block, ptr, oldsize < size ? oldsize : size);
    }
    _rpmalloc_deallocate(ptr);
  }
  തിരികെ block;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
സ്ഥിതം size_t _rpmalloc_usable_size(ശൂന്യം *p) {
  // Grab the span using guaranteed span alignment
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  എങ്കിൽ (span->size_class < SIZE_CLASS_എണ്ണം) {
    // Small/medium block
    ശൂന്യം *blocks_start = pointer_offset(span, SPAN_HEADER_SIZE);
    തിരികെ span->block_size -
           ((size_t)pointer_diff(p, blocks_start) % span->block_size);
  }
  എങ്കിൽ (span->size_class == SIZE_CLASS_LARGE) {
    // Large block
    size_t current_spans = span->span_എണ്ണം;
    തിരികെ (current_spans * _memory_span_size) - (size_t)pointer_diff(p, span);
  }
  // Oversized block, page count is stored in span_count
  size_t current_pages = span->span_എണ്ണം;
  തിരികെ (current_pages * _memory_page_size) - (size_t)pointer_diff(p, span);
}

//! Adjust and optimize the size class properties for the given class
സ്ഥിതം ശൂന്യം _rpmalloc_adjust_size_class(size_t iclass) {
  size_t block_size = _memory_size_class[iclass].block_size;
  size_t block_എണ്ണം = (_memory_span_size - SPAN_HEADER_SIZE) / block_size;

  _memory_size_class[iclass].block_എണ്ണം = (uint16_t)block_എണ്ണം;
  _memory_size_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  എങ്കിൽ (iclass >= SMALL_CLASS_എണ്ണം) {
    size_t prevclass = iclass;
    ആകുമ്പോൾ (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      എങ്കിൽ (_memory_size_class[prevclass].block_എണ്ണം ==
          _memory_size_class[iclass].block_എണ്ണം)
        _rpmalloc_memcpy_const(_memory_size_class + prevclass,
                               _memory_size_class + iclass,
                               വലുപ്പം(_memory_size_class[iclass]));
      അല്ലെങ്കിൽ
        നിർത്തുക;
    }
  }
}

//! Initialize the allocator and setup global data
ബാഹ്യ വരിയിൽ പൂർണ്ണസംഖ്യ rpmalloc_initialize(ശൂന്യം) {
  എങ്കിൽ (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    തിരികെ 0;
  }
  തിരികെ rpmalloc_initialize_config(0);
}

പൂർണ്ണസംഖ്യ rpmalloc_initialize_config(സ്ഥിരം rpmalloc_config_t *config) {
  എങ്കിൽ (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    തിരികെ 0;
  }
  _rpmalloc_initialized = 1;

  എങ്കിൽ (config)
    memcpy(&_memory_config, config, വലുപ്പം(rpmalloc_config_t));
  അല്ലെങ്കിൽ
    _rpmalloc_memset_const(&_memory_config, 0, വലുപ്പം(rpmalloc_config_t));

  എങ്കിൽ (!_memory_config.memory_map || !_memory_config.memory_unmap) {
    _memory_config.memory_map = _rpmalloc_mmap_os;
    _memory_config.memory_unmap = _rpmalloc_unmap_os;
  }

#എങ്കിൽ PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, വലുപ്പം(system_info));
  GetSystemInfo(&system_info);
  _memory_map_granularity = system_info.dwAllocationGranularity;
#അല്ലെങ്കിൽ
  _memory_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#endif

#എങ്കിൽ RPMALLOC_CONFIGURABLE
  _memory_page_size = _memory_config.page_size;
#അല്ലെങ്കിൽ
  _memory_page_size = 0;
#endif
  _memory_huge_pages = 0;
  എങ്കിൽ (!_memory_page_size) {
#എങ്കിൽ PLATFORM_WINDOWS
    _memory_page_size = system_info.dwPageSize;
#അല്ലെങ്കിൽ
    _memory_page_size = _memory_map_granularity;
    എങ്കിൽ (_memory_config.enable_huge_pages) {
#എങ്കിൽ defined(__linux__)
      size_t huge_page_size = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      എങ്കിൽ (meminfo) {
        അക്ഷരം വരി_2[128];
        ആകുമ്പോൾ (!huge_page_size && fgets(വരി_2, വലുപ്പം(വരി_2) - 1, meminfo)) {
          വരി_2[വലുപ്പം(വരി_2) - 1] = 0;
          എങ്കിൽ (strstr(വരി_2, "Hugepagesize:"))
            huge_page_size = (size_t)strtol(വരി_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      എങ്കിൽ (huge_page_size) {
        _memory_huge_pages = 1;
        _memory_page_size = huge_page_size;
        _memory_map_granularity = huge_page_size;
      }
#അല്ലെങ്കിലെങ്കിൽ defined(__FreeBSD__)
      പൂർണ്ണസംഖ്യ rc;
      size_t sz = വലുപ്പം(rc);

      എങ്കിൽ (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, ശൂന്യ, 0) == 0 &&
          rc == 1) {
        സ്ഥിതം size_t defsize = 2 * 1024 * 1024;
        പൂർണ്ണസംഖ്യ nsize = 0;
        size_t sizes[4] = {0};
        _memory_huge_pages = 1;
        _memory_page_size = defsize;
        എങ്കിൽ ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          വേണ്ടി (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            എങ്കിൽ (defsize < csize) {
              _memory_page_size = csize;
              നിർത്തുക;
            }
          }
        }
        _memory_map_granularity = _memory_page_size;
      }
#അല്ലെങ്കിലെങ്കിൽ defined(__APPLE__) || defined(__NetBSD__)
      _memory_huge_pages = 1;
      _memory_page_size = 2 * 1024 * 1024;
      _memory_map_granularity = _memory_page_size;
#endif
    }
#endif
  } അല്ലെങ്കിൽ {
    എങ്കിൽ (_memory_config.enable_huge_pages)
      _memory_huge_pages = 1;
  }

#എങ്കിൽ PLATFORM_WINDOWS
  എങ്കിൽ (_memory_config.enable_huge_pages) {
    HANDLE token = 0;
    size_t large_page_minimum = GetLargePageMinimum();
    എങ്കിൽ (large_page_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    എങ്കിൽ (token) {
      LUID luid;
      എങ്കിൽ (Lookup_Privilege_മൂല്യം(0, SE_LOCK_MEMORY_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, വലുപ്പം(token_privileges));
        token_privileges.Privilege_എണ്ണം = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        എങ്കിൽ (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          എങ്കിൽ (GetLastError() == ERROR_SUCCESS)
            _memory_huge_pages = 1;
        }
      }
      CloseHandle(token);
    }
    എങ്കിൽ (_memory_huge_pages) {
      എങ്കിൽ (large_page_minimum > _memory_page_size)
        _memory_page_size = large_page_minimum;
      എങ്കിൽ (large_page_minimum > _memory_map_granularity)
        _memory_map_granularity = large_page_minimum;
    }
  }
#endif

  size_t min_span_size = 256;
  size_t max_page_size;
#എങ്കിൽ UINTPTR_MAX > 0xFFFFFFFF
  max_page_size = 4096ULL * 1024ULL * 1024ULL;
#അല്ലെങ്കിൽ
  max_page_size = 4 * 1024 * 1024;
#endif
  എങ്കിൽ (_memory_page_size < min_span_size)
    _memory_page_size = min_span_size;
  എങ്കിൽ (_memory_page_size > max_page_size)
    _memory_page_size = max_page_size;
  _memory_page_size_shift = 0;
  size_t page_size_bit = _memory_page_size;
  ആകുമ്പോൾ (page_size_bit != 1) {
    ++_memory_page_size_shift;
    page_size_bit >>= 1;
  }
  _memory_page_size = ((size_t)1 << _memory_page_size_shift);

#എങ്കിൽ RPMALLOC_CONFIGURABLE
  എങ്കിൽ (!_memory_config.span_size) {
    _memory_span_size = _memory_default_span_size;
    _memory_span_size_shift = _memory_default_span_size_shift;
    _memory_span_mask = _memory_default_span_mask;
  } അല്ലെങ്കിൽ {
    size_t span_size = _memory_config.span_size;
    എങ്കിൽ (span_size > (256 * 1024))
      span_size = (256 * 1024);
    _memory_span_size = 4096;
    _memory_span_size_shift = 12;
    ആകുമ്പോൾ (_memory_span_size < span_size) {
      _memory_span_size <<= 1;
      ++_memory_span_size_shift;
    }
    _memory_span_mask = ~(uintptr_t)(_memory_span_size - 1);
  }
#endif

  memory_span_map_എണ്ണം =
      (_memory_config.span_map_എണ്ണം ? _memory_config.span_map_എണ്ണം
                                     : DEFAULT_SPAN_MAP_എണ്ണം);
  എങ്കിൽ ((_memory_span_size * memory_span_map_എണ്ണം) < _memory_page_size)
    memory_span_map_എണ്ണം = (_memory_page_size / _memory_span_size);
  എങ്കിൽ ((_memory_page_size >= _memory_span_size) &&
      ((memory_span_map_എണ്ണം * _memory_span_size) % _memory_page_size))
    memory_span_map_എണ്ണം = (_memory_page_size / _memory_span_size);
  memory_heap_reserve_എണ്ണം = (memory_span_map_എണ്ണം > DEFAULT_SPAN_MAP_എണ്ണം)
                                   ? DEFAULT_SPAN_MAP_എണ്ണം
                                   : memory_span_map_എണ്ണം;

  _memory_config.page_size = _memory_page_size;
  _memory_config.span_size = _memory_span_size;
  _memory_config.span_map_എണ്ണം = memory_span_map_എണ്ണം;
  _memory_config.enable_huge_pages = _memory_huge_pages;

#എങ്കിൽ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  എങ്കിൽ (pthread_താക്കോൽ_create(&_memory_thread_heap, _rpmalloc_heap_release_raw_fc))
    തിരികെ -1;
#endif
#എങ്കിൽ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_താക്കോൽ = FlsAlloc(&_rpmalloc_thread_destructor);
#endif

  // Setup all small and medium size classes
  size_t iclass = 0;
  _memory_size_class[iclass].block_size = SMALL_GRANULARITY;
  _rpmalloc_adjust_size_class(iclass);
  വേണ്ടി (iclass = 1; iclass < SMALL_CLASS_എണ്ണം; ++iclass) {
    size_t size = iclass * SMALL_GRANULARITY;
    _memory_size_class[iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  _memory_medium_size_limit = (_memory_span_size - SPAN_HEADER_SIZE) >> 1;
  എങ്കിൽ (_memory_medium_size_limit > MEDIUM_SIZE_LIMIT)
    _memory_medium_size_limit = MEDIUM_SIZE_LIMIT;
  വേണ്ടി (iclass = 0; iclass < MEDIUM_CLASS_എണ്ണം; ++iclass) {
    size_t size = SMALL_SIZE_LIMIT + ((iclass + 1) * MEDIUM_GRANULARITY);
    എങ്കിൽ (size > _memory_medium_size_limit) {
      _memory_medium_size_limit =
          SMALL_SIZE_LIMIT + (iclass * MEDIUM_GRANULARITY);
      നിർത്തുക;
    }
    _memory_size_class[SMALL_CLASS_എണ്ണം + iclass].block_size = (uint32_t)size;
    _rpmalloc_adjust_size_class(SMALL_CLASS_എണ്ണം + iclass);
  }

  _memory_orphan_heaps = 0;
#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS
  _memory_first_class_orphan_heaps = 0;
#endif
#എങ്കിൽ ENABLE_STATISTICS
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
  memset(_memory_heaps, 0, വലുപ്പം(_memory_heaps));
  atomic_store32_release(&_memory_global_lock, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  തിരികെ 0;
}

//! Finalize the allocator
ശൂന്യം rpmalloc_finalize(ശൂന്യം) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  എങ്കിൽ (_memory_global_reserve) {
    atomic_add32(&_memory_global_reserve_master->remaining_spans,
                 -(int32_t)memory_global_reserve_എണ്ണം);
    _memory_global_reserve_master = 0;
    memory_global_reserve_എണ്ണം = 0;
    _memory_global_reserve = 0;
  }
  atomic_store32_release(&_memory_global_lock, 0);

  // Free all thread caches and fully free spans
  വേണ്ടി (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    ആകുമ്പോൾ (heap) {
      heap_t *next_heap = heap->next_heap;
      heap->finalize = 1;
      _rpmalloc_heap_global_finalize(heap);
      heap = next_heap;
    }
  }

#എങ്കിൽ ENABLE_GLOBAL_CACHE
  // Free global caches
  വേണ്ടി (size_t iclass = 0; iclass < LARGE_CLASS_എണ്ണം; ++iclass)
    _rpmalloc_global_cache_finalize(&_memory_span_cache[iclass]);
#endif

#എങ്കിൽ (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_താക്കോൽ_delete(_memory_thread_heap);
#endif
#എങ്കിൽ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_താക്കോൽ);
  fls_താക്കോൽ = 0;
#endif
#എങ്കിൽ ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&_mapped_pages) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&_mapped_pages_os) == 0,
                  "Memory leak detected");
#endif

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
ബാഹ്യ വരിയിൽ ശൂന്യം rpmalloc_thread_initialize(ശൂന്യം) {
  എങ്കിൽ (!get_thread_heap_raw()) {
    heap_t *heap = _rpmalloc_heap_allocate(0);
    എങ്കിൽ (heap) {
      _rpmalloc_stat_inc(&_memory_active_heaps);
      set_thread_heap(heap);
#എങ്കിൽ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_Set_മൂല്യം(fls_താക്കോൽ, heap);
#endif
    }
  }
}

//! Finalize thread, orphan heap
ശൂന്യം rpmalloc_thread_finalize(പൂർണ്ണസംഖ്യ release_caches) {
  heap_t *heap = get_thread_heap_raw();
  എങ്കിൽ (heap)
    _rpmalloc_heap_release_raw(heap, release_caches);
  set_thread_heap(0);
#എങ്കിൽ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_Set_മൂല്യം(fls_താക്കോൽ, 0);
#endif
}

പൂർണ്ണസംഖ്യ rpmalloc_is_thread_initialized(ശൂന്യം) {
  തിരികെ (get_thread_heap_raw() != 0) ? 1 : 0;
}

സ്ഥിരം rpmalloc_config_t *rpmalloc_config(ശൂന്യം) { തിരികെ &_memory_config; }

// Extern interface

ബാഹ്യ വരിയിൽ RPMALLOC_ALLOCATOR ശൂന്യം *rpmalloc(size_t size) {
#എങ്കിൽ ENABLE_VALIDATE_ARGS
  എങ്കിൽ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    തിരികെ 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  തിരികെ _rpmalloc_allocate(heap, size);
}

ബാഹ്യ വരിയിൽ ശൂന്യം rpfree(ശൂന്യം *ptr) { _rpmalloc_deallocate(ptr); }

ബാഹ്യ വരിയിൽ RPMALLOC_ALLOCATOR ശൂന്യം *rpcalloc(size_t അക്കം_2, size_t size) {
  size_t total;
#എങ്കിൽ ENABLE_VALIDATE_ARGS
#എങ്കിൽ PLATFORM_WINDOWS
  പൂർണ്ണസംഖ്യ err = SizeTMult(അക്കം_2, size, &total);
  എങ്കിൽ ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    തിരികെ 0;
  }
#അല്ലെങ്കിൽ
  പൂർണ്ണസംഖ്യ err = __builtin_umull_overflow(അക്കം_2, size, &total);
  എങ്കിൽ (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    തിരികെ 0;
  }
#endif
#അല്ലെങ്കിൽ
  total = അക്കം_2 * size;
#endif
  heap_t *heap = get_thread_heap();
  ശൂന്യം *block = _rpmalloc_allocate(heap, total);
  എങ്കിൽ (block)
    memset(block, 0, total);
  തിരികെ block;
}

ബാഹ്യ വരിയിൽ RPMALLOC_ALLOCATOR ശൂന്യം *rprealloc(ശൂന്യം *ptr, size_t size) {
#എങ്കിൽ ENABLE_VALIDATE_ARGS
  എങ്കിൽ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    തിരികെ ptr;
  }
#endif
  heap_t *heap = get_thread_heap();
  തിരികെ _rpmalloc_reallocate(heap, ptr, size, 0, 0);
}

ബാഹ്യ RPMALLOC_ALLOCATOR ശൂന്യം *rpaligned_realloc(ശൂന്യം *ptr, size_t alignment,
                                                  size_t size, size_t oldsize,
                                                  unsigned പൂർണ്ണസംഖ്യ flags) {
#എങ്കിൽ ENABLE_VALIDATE_ARGS
  എങ്കിൽ ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    തിരികെ 0;
  }
#endif
  heap_t *heap = get_thread_heap();
  തിരികെ _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, oldsize,
                                      flags);
}

ബാഹ്യ RPMALLOC_ALLOCATOR ശൂന്യം *rpaligned_alloc(size_t alignment, size_t size) {
  heap_t *heap = get_thread_heap();
  തിരികെ _rpmalloc_aligned_allocate(heap, alignment, size);
}

ബാഹ്യ വരിയിൽ RPMALLOC_ALLOCATOR ശൂന്യം *
rpaligned_calloc(size_t alignment, size_t അക്കം_2, size_t size) {
  size_t total;
#എങ്കിൽ ENABLE_VALIDATE_ARGS
#എങ്കിൽ PLATFORM_WINDOWS
  പൂർണ്ണസംഖ്യ err = SizeTMult(അക്കം_2, size, &total);
  എങ്കിൽ ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    തിരികെ 0;
  }
#അല്ലെങ്കിൽ
  പൂർണ്ണസംഖ്യ err = __builtin_umull_overflow(അക്കം_2, size, &total);
  എങ്കിൽ (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    തിരികെ 0;
  }
#endif
#അല്ലെങ്കിൽ
  total = അക്കം_2 * size;
#endif
  ശൂന്യം *block = rpaligned_alloc(alignment, total);
  എങ്കിൽ (block)
    memset(block, 0, total);
  തിരികെ block;
}

ബാഹ്യ വരിയിൽ RPMALLOC_ALLOCATOR ശൂന്യം *rpmemalign(size_t alignment,
                                                  size_t size) {
  തിരികെ rpaligned_alloc(alignment, size);
}

ബാഹ്യ വരിയിൽ പൂർണ്ണസംഖ്യ rpposix_memalign(ശൂന്യം **memptr, size_t alignment,
                                   size_t size) {
  എങ്കിൽ (memptr)
    *memptr = rpaligned_alloc(alignment, size);
  അല്ലെങ്കിൽ
    തിരികെ EINVAL;
  തിരികെ *memptr ? 0 : ENOMEM;
}

ബാഹ്യ വരിയിൽ size_t rpmalloc_usable_size(ശൂന്യം *ptr) {
  തിരികെ (ptr ? _rpmalloc_usable_size(ptr) : 0);
}

ബാഹ്യ വരിയിൽ ശൂന്യം rpmalloc_thread_collect(ശൂന്യം) {}

ശൂന്യം rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, വലുപ്പം(rpmalloc_thread_statistics_t));
  heap_t *heap = get_thread_heap_raw();
  എങ്കിൽ (!heap)
    തിരികെ;

  വേണ്ടി (size_t iclass = 0; iclass < SIZE_CLASS_എണ്ണം; ++iclass) {
    size_class_t *size_class = _memory_size_class + iclass;
    span_t *span = heap->size_class[iclass].partial_span;
    ആകുമ്പോൾ (span) {
      size_t free_എണ്ണം = span->list_size;
      size_t block_എണ്ണം = size_class->block_എണ്ണം;
      എങ്കിൽ (span->free_list_limit < block_എണ്ണം)
        block_എണ്ണം = span->free_list_limit;
      free_എണ്ണം += (block_എണ്ണം - span->used_എണ്ണം);
      stats->sizecache += free_എണ്ണം * size_class->block_size;
      span = span->next;
    }
  }

#എങ്കിൽ ENABLE_THREAD_CACHE
  വേണ്ടി (size_t iclass = 0; iclass < LARGE_CLASS_എണ്ണം; ++iclass) {
    span_cache_t *span_cache;
    എങ്കിൽ (!iclass)
      span_cache = &heap->span_cache;
    അല്ലെങ്കിൽ
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    stats->spancache += span_cache->എണ്ണം * (iclass + 1) * _memory_span_size;
  }
#endif

  span_t *deferred = (span_t *)atomic_load_ptr(&heap->span_free_deferred);
  ആകുമ്പോൾ (deferred) {
    എങ്കിൽ (deferred->size_class != SIZE_CLASS_HUGE)
      stats->spancache += (size_t)deferred->span_എണ്ണം * _memory_span_size;
    deferred = (span_t *)deferred->free_list;
  }

#എങ്കിൽ ENABLE_STATISTICS
  stats->thread_to_global = (size_t)atomic_load64(&heap->thread_to_global);
  stats->global_to_thread = (size_t)atomic_load64(&heap->global_to_thread);

  വേണ്ടി (size_t iclass = 0; iclass < LARGE_CLASS_എണ്ണം; ++iclass) {
    stats->span_use[iclass].current =
        (size_t)atomic_load32(&heap->span_use[iclass].current);
    stats->span_use[iclass].peak =
        (size_t)atomic_load32(&heap->span_use[iclass].മുകളിലെ);
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
  വേണ്ടി (size_t iclass = 0; iclass < SIZE_CLASS_എണ്ണം; ++iclass) {
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

ശൂന്യം rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, വലുപ്പം(rpmalloc_global_statistics_t));
#എങ്കിൽ ENABLE_STATISTICS
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
#എങ്കിൽ ENABLE_GLOBAL_CACHE
  വേണ്ടി (size_t iclass = 0; iclass < LARGE_CLASS_എണ്ണം; ++iclass) {
    global_cache_t *cache = &_memory_span_cache[iclass];
    ആകുമ്പോൾ (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();
    uint32_t എണ്ണം = cache->എണ്ണം;
#എങ്കിൽ ENABLE_UNLIMITED_CACHE
    span_t *current_span = cache->overflow;
    ആകുമ്പോൾ (current_span) {
      ++എണ്ണം;
      current_span = current_span->next;
    }
#endif
    atomic_store32_release(&cache->lock, 0);
    stats->cached += എണ്ണം * (iclass + 1) * _memory_span_size;
  }
#endif
}

#എങ്കിൽ ENABLE_STATISTICS

സ്ഥിതം ശൂന്യം _memory_heap_dump_statistics(heap_t *heap, ശൂന്യം *file) {
  fprintf(file, "Heap %d stats:\n", heap->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  വേണ്ടി (size_t iclass = 0; iclass < SIZE_CLASS_എണ്ണം; ++iclass) {
    എങ്കിൽ (!atomic_load32(&heap->size_class_use[iclass].alloc_total))
      തുടരുക;
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
        _memory_size_class[iclass].block_എണ്ണം,
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
  വേണ്ടി (size_t iclass = 0; iclass < LARGE_CLASS_എണ്ണം; ++iclass) {
    എങ്കിൽ (!atomic_load32(&heap->span_use[iclass].മുകളിലെ) &&
        !atomic_load32(&heap->span_use[iclass].spans_map_calls))
      തുടരുക;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&heap->span_use[iclass].current),
        atomic_load32(&heap->span_use[iclass].മുകളിലെ),
        atomic_load32(&heap->span_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&heap->span_use[iclass].മുകളിലെ) *
         (size_t)_memory_span_size * (iclass + 1)) /
            (size_t)(1024 * 1024),
#എങ്കിൽ ENABLE_THREAD_CACHE
        (unsigned പൂർണ്ണസംഖ്യ)(!iclass ? heap->span_cache.എണ്ണം
                               : heap->span_large_cache[iclass - 1].എണ്ണം),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_to_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->span_use[iclass].spans_from_cache) *
         (iclass + 1) * _memory_span_size) /
            (size_t)(1024 * 1024),
#അല്ലെങ്കിൽ
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
  fprintf(file, "Full spans: %zu\n", heap->full_span_എണ്ണം);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&heap->thread_to_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&heap->global_to_thread) / (size_t)(1024 * 1024));
}

#endif

ശൂന്യം rpmalloc_dump_statistics(ശൂന്യം *file) {
#എങ്കിൽ ENABLE_STATISTICS
  വേണ്ടി (size_t list_idx = 0; list_idx < HEAP_ARRAY_SIZE; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    ആകുമ്പോൾ (heap) {
      പൂർണ്ണസംഖ്യ need_dump = 0;
      വേണ്ടി (size_t iclass = 0; !need_dump && (iclass < SIZE_CLASS_എണ്ണം);
           ++iclass) {
        എങ്കിൽ (!atomic_load32(&heap->size_class_use[iclass].alloc_total)) {
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].free_total),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&heap->size_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          തുടരുക;
        }
        need_dump = 1;
      }
      വേണ്ടി (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_എണ്ണം);
           ++iclass) {
        എങ്കിൽ (!atomic_load32(&heap->span_use[iclass].മുകളിലെ) &&
            !atomic_load32(&heap->span_use[iclass].spans_map_calls))
          തുടരുക;
        need_dump = 1;
      }
      എങ്കിൽ (need_dump)
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

#എങ്കിൽ ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  വേണ്ടി (size_t iclass = 0; iclass < LARGE_CLASS_എണ്ണം; ++iclass) {
    global_cache_t *cache = _memory_span_cache + iclass;
    size_t global_cache = (size_t)cache->എണ്ണം * iclass * _memory_span_size;

    size_t global_overflow_cache = 0;
    span_t *span = cache->overflow;
    ആകുമ്പോൾ (span) {
      global_overflow_cache += iclass * _memory_span_size;
      span = span->next;
    }
    എങ്കിൽ (global_cache || global_overflow_cache || cache->തിരുകുക_എണ്ണം ||
        cache->extract_എണ്ണം)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_overflow_cache / (size_t)(1024 * 1024),
              cache->തിരുകുക_എണ്ണം, cache->extract_എണ്ണം);
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
#എങ്കിൽ 0
	int64_t allocated = atomic_load64(&_allocation_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#endif
#endif
  (ശൂന്യം)വലുപ്പം(file);
}

#എങ്കിൽ RPMALLOC_FIRST_CLASS_HEAPS

ബാഹ്യ വരിയിൽ rpmalloc_heap_t *rpmalloc_heap_acquire(ശൂന്യം) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  heap_t *heap = _rpmalloc_heap_allocate(1);
  rpmalloc_assume(heap != ശൂന്യ);
  heap->owner_thread = 0;
  _rpmalloc_stat_inc(&_memory_active_heaps);
  തിരികെ heap;
}

ബാഹ്യ വരിയിൽ ശൂന്യം rpmalloc_heap_release(rpmalloc_heap_t *heap) {
  എങ്കിൽ (heap)
    _rpmalloc_heap_release(heap, 1, 1);
}

ബാഹ്യ വരിയിൽ RPMALLOC_ALLOCATOR ശൂന്യം *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, size_t size) {
#എങ്കിൽ ENABLE_VALIDATE_ARGS
  എങ്കിൽ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    തിരികെ 0;
  }
#endif
  തിരികെ _rpmalloc_allocate(heap, size);
}

ബാഹ്യ വരിയിൽ RPMALLOC_ALLOCATOR ശൂന്യം *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, size_t alignment,
                            size_t size) {
#എങ്കിൽ ENABLE_VALIDATE_ARGS
  എങ്കിൽ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    തിരികെ 0;
  }
#endif
  തിരികെ _rpmalloc_aligned_allocate(heap, alignment, size);
}

ബാഹ്യ വരിയിൽ RPMALLOC_ALLOCATOR ശൂന്യം *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, size_t അക്കം_2, size_t size) {
  തിരികെ rpmalloc_heap_aligned_calloc(heap, 0, അക്കം_2, size);
}

ബാഹ്യ വരിയിൽ RPMALLOC_ALLOCATOR ശൂന്യം *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, size_t alignment,
                             size_t അക്കം_2, size_t size) {
  size_t total;
#എങ്കിൽ ENABLE_VALIDATE_ARGS
#എങ്കിൽ PLATFORM_WINDOWS
  പൂർണ്ണസംഖ്യ err = SizeTMult(അക്കം_2, size, &total);
  എങ്കിൽ ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    തിരികെ 0;
  }
#അല്ലെങ്കിൽ
  പൂർണ്ണസംഖ്യ err = __builtin_umull_overflow(അക്കം_2, size, &total);
  എങ്കിൽ (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    തിരികെ 0;
  }
#endif
#അല്ലെങ്കിൽ
  total = അക്കം_2 * size;
#endif
  ശൂന്യം *block = _rpmalloc_aligned_allocate(heap, alignment, total);
  എങ്കിൽ (block)
    memset(block, 0, total);
  തിരികെ block;
}

ബാഹ്യ വരിയിൽ RPMALLOC_ALLOCATOR ശൂന്യം *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, ശൂന്യം *ptr, size_t size,
                      unsigned പൂർണ്ണസംഖ്യ flags) {
#എങ്കിൽ ENABLE_VALIDATE_ARGS
  എങ്കിൽ (size >= MAX_ALLOC_SIZE) {
    errno = EINVAL;
    തിരികെ ptr;
  }
#endif
  തിരികെ _rpmalloc_reallocate(heap, ptr, size, 0, flags);
}

ബാഹ്യ വരിയിൽ RPMALLOC_ALLOCATOR ശൂന്യം *
rpmalloc_heap_aligned_realloc(rpmalloc_heap_t *heap, ശൂന്യം *ptr,
                              size_t alignment, size_t size,
                              unsigned പൂർണ്ണസംഖ്യ flags) {
#എങ്കിൽ ENABLE_VALIDATE_ARGS
  എങ്കിൽ ((size + alignment < size) || (alignment > _memory_page_size)) {
    errno = EINVAL;
    തിരികെ 0;
  }
#endif
  തിരികെ _rpmalloc_aligned_reallocate(heap, ptr, alignment, size, 0, flags);
}

ബാഹ്യ വരിയിൽ ശൂന്യം rpmalloc_heap_free(rpmalloc_heap_t *heap, ശൂന്യം *ptr) {
  (ശൂന്യം)വലുപ്പം(heap);
  _rpmalloc_deallocate(ptr);
}

ബാഹ്യ വരിയിൽ ശൂന്യം rpmalloc_heap_free_all(rpmalloc_heap_t *heap) {
  span_t *span;
  span_t *next_span;

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  വേണ്ടി (size_t iclass = 0; iclass < SIZE_CLASS_എണ്ണം; ++iclass) {
    span = heap->size_class[iclass].partial_span;
    ആകുമ്പോൾ (span) {
      next_span = span->next;
      rpmalloc_heap_cache_തിരുകുക(heap, span);
      span = next_span;
    }
    heap->size_class[iclass].partial_span = 0;
    span = heap->full_span[iclass];
    ആകുമ്പോൾ (span) {
      next_span = span->next;
      rpmalloc_heap_cache_തിരുകുക(heap, span);
      span = next_span;
    }

    span = heap->size_class[iclass].cache;
    എങ്കിൽ (span)
      rpmalloc_heap_cache_തിരുകുക(heap, span);
    heap->size_class[iclass].cache = 0;
  }
  memset(heap->size_class, 0, വലുപ്പം(heap->size_class));
  memset(heap->full_span, 0, വലുപ്പം(heap->full_span));

  span = heap->large_huge_span;
  ആകുമ്പോൾ (span) {
    next_span = span->next;
    എങ്കിൽ (UNEXPECTED(span->size_class == SIZE_CLASS_HUGE))
      _rpmalloc_deallocate_huge(span);
    അല്ലെങ്കിൽ
      rpmalloc_heap_cache_തിരുകുക(heap, span);
    span = next_span;
  }
  heap->large_huge_span = 0;
  heap->full_span_എണ്ണം = 0;

#എങ്കിൽ ENABLE_THREAD_CACHE
  വേണ്ടി (size_t iclass = 0; iclass < LARGE_CLASS_എണ്ണം; ++iclass) {
    span_cache_t *span_cache;
    എങ്കിൽ (!iclass)
      span_cache = &heap->span_cache;
    അല്ലെങ്കിൽ
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    എങ്കിൽ (!span_cache->എണ്ണം)
      തുടരുക;
#എങ്കിൽ ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&heap->thread_to_global,
                         span_cache->എണ്ണം * (iclass + 1) * _memory_span_size);
    _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                       span_cache->എണ്ണം);
    rpmalloc_global_cache_തിരുകുക_spans(span_cache->span, iclass + 1,
                                        span_cache->എണ്ണം);
#അല്ലെങ്കിൽ
    വേണ്ടി (size_t ispan = 0; ispan < span_cache->എണ്ണം; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
#endif
    span_cache->എണ്ണം = 0;
  }
#endif

#എങ്കിൽ ENABLE_STATISTICS
  വേണ്ടി (size_t iclass = 0; iclass < SIZE_CLASS_എണ്ണം; ++iclass) {
    atomic_store32(&heap->size_class_use[iclass].alloc_current, 0);
    atomic_store32(&heap->size_class_use[iclass].spans_current, 0);
  }
  വേണ്ടി (size_t iclass = 0; iclass < LARGE_CLASS_എണ്ണം; ++iclass) {
    atomic_store32(&heap->span_use[iclass].current, 0);
  }
#endif
}

ബാഹ്യ വരിയിൽ ശൂന്യം rpmalloc_heap_thread_set_current(rpmalloc_heap_t *heap) {
  heap_t *prev_heap = get_thread_heap_raw();
  എങ്കിൽ (prev_heap != heap) {
    set_thread_heap(heap);
    എങ്കിൽ (prev_heap)
      rpmalloc_heap_release(prev_heap);
  }
}

ബാഹ്യ വരിയിൽ rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(ശൂന്യം *ptr) {
  // Grab the span, and then the heap from the span
  span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
  എങ്കിൽ (span) {
    തിരികെ span->heap;
  }
  തിരികെ 0;
}

#endif

#എങ്കിൽ ENABLE_PRELOAD || ENABLE_OVERRIDE

#അന്തര്ഭാവ "malloc.c"

#endif

ശൂന്യം rpmalloc_linker_reference(ശൂന്യം) { (ശൂന്യം)വലുപ്പം(_rpmalloc_initialized); }
