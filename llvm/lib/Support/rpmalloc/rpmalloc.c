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

#समावेश "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#अगर defined(__clang__)
#आशय clang diagnostic ignored "-Wunused-macros"
#आशय clang diagnostic ignored "-Wunused-function"
#अगर __has_warning("-Wreserved-identifier")
#आशय clang diagnostic ignored "-Wreserved-identifier"
#पूर्ण_अगर
#अगर __has_warning("-Wstatic-in-inline")
#आशय clang diagnostic ignored "-Wstatic-in-inline"
#पूर्ण_अगर
#या_अगर defined(__GNUC__)
#आशय GCC diagnostic ignored "-Wunused-macros"
#आशय GCC diagnostic ignored "-Wunused-function"
#पूर्ण_अगर

#अगर !defined(__has_builtin)
#घोषणा __has_builtin(b) 0
#पूर्ण_अगर

#अगर defined(__GNUC__) || defined(__clang__)

#अगर __has_builtin(__builtin_memcpy_inline)
#घोषणा _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#अन्यथा
#घोषणा _rpmalloc_memcpy_const(x, y, s)                                        \
  करो {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    स_नकल(x, y, s);                                                           \
  } जबतक (0)
#पूर्ण_अगर

#अगर __has_builtin(__builtin_memset_inline)
#घोषणा _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#अन्यथा
#घोषणा _rpmalloc_memset_const(x, y, s)                                        \
  करो {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    स_रखो(x, y, s);                                                           \
  } जबतक (0)
#पूर्ण_अगर
#अन्यथा
#घोषणा _rpmalloc_memcpy_const(x, y, s) स_नकल(x, y, s)
#घोषणा _rpmalloc_memset_const(x, y, s) स_रखो(x, y, s)
#पूर्ण_अगर

#अगर __has_builtin(__builtin_assume)
#घोषणा rpmalloc_assume(cond) __builtin_assume(cond)
#या_अगर defined(__GNUC__)
#घोषणा rpmalloc_assume(cond)                                                  \
  करो {                                                                         \
    अगर (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } जबतक (0)
#या_अगर defined(_MSC_VER)
#घोषणा rpmalloc_assume(cond) __assume(cond)
#अन्यथा
#घोषणा rpmalloc_assume(cond) 0
#पूर्ण_अगर

#अगर_अघोषित HEAP_ARRAY_आकार
//! Size of heap hashmap
#घोषणा HEAP_ARRAY_आकार 47
#पूर्ण_अगर
#अगर_अघोषित ENABLE_THREAD_CACHE
//! Enable per-thread cache
#घोषणा ENABLE_THREAD_CACHE 1
#पूर्ण_अगर
#अगर_अघोषित ENABLE_GLOBAL_CACHE
//! Enable global cache shared between all threads, requires thread cache
#घोषणा ENABLE_GLOBAL_CACHE 1
#पूर्ण_अगर
#अगर_अघोषित ENABLE_VALIDATE_ARGS
//! Enable validation of args to public entry points
#घोषणा ENABLE_VALIDATE_ARGS 0
#पूर्ण_अगर
#अगर_अघोषित ENABLE_STATISTICS
//! Enable statistics collection
#घोषणा ENABLE_STATISTICS 0
#पूर्ण_अगर
#अगर_अघोषित ENABLE_ASSERTS
//! Enable asserts
#घोषणा ENABLE_ASSERTS 0
#पूर्ण_अगर
#अगर_अघोषित ENABLE_OVERRIDE
//! Override standard library malloc/free and new/delete entry points
#घोषणा ENABLE_OVERRIDE 0
#पूर्ण_अगर
#अगर_अघोषित ENABLE_PRELOAD
//! Support preloading
#घोषणा ENABLE_PRELOAD 0
#पूर्ण_अगर
#अगर_अघोषित DISABLE_UNMAP
//! Disable unmapping memory pages (also enables unlimited cache)
#घोषणा DISABLE_UNMAP 0
#पूर्ण_अगर
#अगर_अघोषित ENABLE_UNLIMITED_CACHE
//! Enable unlimited global cache (no unmapping until finalization)
#घोषणा ENABLE_UNLIMITED_CACHE 0
#पूर्ण_अगर
#अगर_अघोषित ENABLE_ADAPTIVE_THREAD_CACHE
//! Enable adaptive thread cache size based on use heuristics
#घोषणा ENABLE_ADAPTIVE_THREAD_CACHE 0
#पूर्ण_अगर
#अगर_अघोषित DEFAULT_SPAN_MAP_गिनती
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#घोषणा DEFAULT_SPAN_MAP_गिनती 64
#पूर्ण_अगर
#अगर_अघोषित GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#घोषणा GLOBAL_CACHE_MULTIPLIER 8
#पूर्ण_अगर

#अगर DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#त्रुटि Must use global cache अगर unmap is disabled
#पूर्ण_अगर

#अगर DISABLE_UNMAP
#अघोषित ENABLE_UNLIMITED_CACHE
#घोषणा ENABLE_UNLIMITED_CACHE 1
#पूर्ण_अगर

#अगर !ENABLE_GLOBAL_CACHE
#अघोषित ENABLE_UNLIMITED_CACHE
#घोषणा ENABLE_UNLIMITED_CACHE 0
#पूर्ण_अगर

#अगर !ENABLE_THREAD_CACHE
#अघोषित ENABLE_ADAPTIVE_THREAD_CACHE
#घोषणा ENABLE_ADAPTIVE_THREAD_CACHE 0
#पूर्ण_अगर

#अगर defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#घोषणा PLATFORM_WINDOWS 1
#घोषणा PLATFORM_POSIX 0
#अन्यथा
#घोषणा PLATFORM_WINDOWS 0
#घोषणा PLATFORM_POSIX 1
#पूर्ण_अगर

/// Platform and arch specifics
#अगर defined(_MSC_VER) && !defined(__clang__)
#आशय warning(disable : 5105)
#अगर_अघोषित FORCEINLINE
#घोषणा FORCEINLINE अंतरभूत __forceinline
#पूर्ण_अगर
#घोषणा _Static_assert static_assert
#अन्यथा
#अगर_अघोषित FORCEINLINE
#घोषणा FORCEINLINE अंतरभूत __attribute__((__always_inline__))
#पूर्ण_अगर
#पूर्ण_अगर
#अगर PLATFORM_WINDOWS
#अगर_अघोषित WIN32_LEAN_AND_MEAN
#घोषणा WIN32_LEAN_AND_MEAN
#पूर्ण_अगर
#समावेश <windows.h>
#अगर ENABLE_VALIDATE_ARGS
#समावेश <intsafe.h>
#पूर्ण_अगर
#अन्यथा
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <time.h>
#समावेश <unistd.h>
#अगर defined(__linux__) || defined(__ANDROID__)
#समावेश <sys/prctl.h>
#अगर !defined(PR_SET_VMA)
#घोषणा PR_SET_VMA 0x53564d41
#घोषणा PR_SET_VMA_ANON_NAME 0
#पूर्ण_अगर
#पूर्ण_अगर
#अगर defined(__APPLE__)
#समावेश <TargetConditionals.h>
#अगर !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#समावेश <mach/mach_vm.h>
#समावेश <mach/vm_statistics.h>
#पूर्ण_अगर
#समावेश <pthread.h>
#पूर्ण_अगर
#अगर defined(__HAIKU__) || defined(__TINYC__)
#समावेश <pthread.h>
#पूर्ण_अगर
#पूर्ण_अगर

#समावेश <errno.h>
#समावेश <stdint.h>
#समावेश <string.h>

#अगर defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#समावेश <fibersapi.h>
अटल DWORD fls_कुंजी;
#पूर्ण_अगर

#अगर PLATFORM_POSIX
#समावेश <sched.h>
#समावेश <sys/mman.h>
#अगर_घोषित __FreeBSD__
#समावेश <sys/sysctl.h>
#घोषणा MAP_HUGETLB MAP_ALIGNED_SUPER
#अगर_अघोषित PROT_MAX
#घोषणा PROT_MAX(f) 0
#पूर्ण_अगर
#अन्यथा
#घोषणा PROT_MAX(f) 0
#पूर्ण_अगर
#अगर_घोषित __sun
बाह्य पूर्णांक madvise(caddr_t, माप_प्रकार, पूर्णांक);
#पूर्ण_अगर
#अगर_अघोषित MAP_UNINITIALIZED
#घोषणा MAP_UNINITIALIZED 0
#पूर्ण_अगर
#पूर्ण_अगर
#समावेश <errno.h>

#अगर ENABLE_ASSERTS
#अघोषित न_संशोधन
#अगर defined(_MSC_VER) && !defined(_DEBUG)
#घोषणा _DEBUG
#पूर्ण_अगर
#समावेश <assert.h>
#घोषणा RPMALLOC_TOSTRING_M(x) #x
#घोषणा RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#घोषणा rpmalloc_assert(truth, message)                                        \
  करो {                                                                         \
    अगर (!(truth)) {                                                            \
      अगर (_memory_config.error_callback) {                                     \
        _memory_config.error_callback(message " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __खाता__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } अन्यथा {                                                                 \
        निश्चित((truth) && message);                                            \
      }                                                                        \
    }                                                                          \
  } जबतक (0)
#अन्यथा
#घोषणा rpmalloc_assert(truth, message)                                        \
  करो {                                                                         \
  } जबतक (0)
#पूर्ण_अगर
#अगर ENABLE_STATISTICS
#समावेश <stdio.h>
#पूर्ण_अगर

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#अगर defined(_MSC_VER) && !defined(__clang__)

प्रकार अस्थिर दीर्घ atomic32_t;
प्रकार अस्थिर दीर्घ दीर्घ atomic64_t;
प्रकार अस्थिर व्योम *atomicptr_t;

अटल FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  वापस (int32_t)InterlockedOr(src, 0);
}
अटल FORCEINLINE व्योम atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
अटल FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  वापस (int32_t)InterlockedIncrement(val);
}
अटल FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  वापस (int32_t)InterlockedDecrement(val);
}
अटल FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  वापस (int32_t)InterlockedExchangeAdd(val, add) + add;
}
अटल FORCEINLINE पूर्णांक atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  वापस (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
अटल FORCEINLINE व्योम atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
अटल FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  वापस (int64_t)InterlockedOr64(src, 0);
}
अटल FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  वापस (int64_t)InterlockedExchangeAdd64(val, add) + add;
}
अटल FORCEINLINE व्योम *atomic_load_ptr(atomicptr_t *src) {
  वापस InterlockedCompareExchangePointer(src, 0, 0);
}
अटल FORCEINLINE व्योम atomic_store_ptr(atomicptr_t *dst, व्योम *val) {
  InterlockedExchangePointer(dst, val);
}
अटल FORCEINLINE व्योम atomic_store_ptr_release(atomicptr_t *dst, व्योम *val) {
  InterlockedExchangePointer(dst, val);
}
अटल FORCEINLINE व्योम *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     व्योम *val) {
  वापस (व्योम *)InterlockedExchangePointer((व्योम *अस्थिर *)dst, val);
}
अटल FORCEINLINE पूर्णांक atomic_cas_ptr(atomicptr_t *dst, व्योम *val, व्योम *ref) {
  वापस (InterlockedCompareExchangePointer((व्योम *अस्थिर *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#घोषणा EXPECTED(x) (x)
#घोषणा UNEXPECTED(x) (x)

#अन्यथा

#समावेश <stdatomic.h>

प्रकार अस्थिर _Atomic(int32_t) atomic32_t;
प्रकार अस्थिर _Atomic(int64_t) atomic64_t;
प्रकार अस्थिर _Atomic(व्योम *) atomicptr_t;

अटल FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  वापस atomic_load_explicit(src, memory_order_relaxed);
}
अटल FORCEINLINE व्योम atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
अटल FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  वापस atomic_fetch_add_explicit(val, 1, memory_order_relaxed) + 1;
}
अटल FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  वापस atomic_fetch_add_explicit(val, -1, memory_order_relaxed) - 1;
}
अटल FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t add) {
  वापस atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
अटल FORCEINLINE पूर्णांक atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  वापस atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
अटल FORCEINLINE व्योम atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
अटल FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  वापस atomic_load_explicit(val, memory_order_relaxed);
}
अटल FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t add) {
  वापस atomic_fetch_add_explicit(val, add, memory_order_relaxed) + add;
}
अटल FORCEINLINE व्योम *atomic_load_ptr(atomicptr_t *src) {
  वापस atomic_load_explicit(src, memory_order_relaxed);
}
अटल FORCEINLINE व्योम atomic_store_ptr(atomicptr_t *dst, व्योम *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
अटल FORCEINLINE व्योम atomic_store_ptr_release(atomicptr_t *dst, व्योम *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
अटल FORCEINLINE व्योम *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     व्योम *val) {
  वापस atomic_exchange_explicit(dst, val, memory_order_acquire);
}
अटल FORCEINLINE पूर्णांक atomic_cas_ptr(atomicptr_t *dst, व्योम *val, व्योम *ref) {
  वापस atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_relaxed, memory_order_relaxed);
}

#घोषणा EXPECTED(x) __builtin_expect((x), 1)
#घोषणा UNEXPECTED(x) __builtin_expect((x), 0)

#पूर्ण_अगर

////////////
///
/// Statistics related functions (evaluate to nothing when statistics not
/// enabled)
///
//////

#अगर ENABLE_STATISTICS
#घोषणा _rpmalloc_stat_inc(गणक_2) atomic_incr32(गणक_2)
#घोषणा _rpmalloc_stat_dec(गणक_2) atomic_decr32(गणक_2)
#घोषणा _rpmalloc_stat_add(गणक_2, मूल्य_2)                                     \
  atomic_add32(गणक_2, (int32_t)(मूल्य_2))
#घोषणा _rpmalloc_stat_add64(गणक_2, मूल्य_2)                                   \
  atomic_add64(गणक_2, (int64_t)(मूल्य_2))
#घोषणा _rpmalloc_stat_add_peak(गणक_2, मूल्य_2, peak)                          \
  करो {                                                                         \
    int32_t cur_गिनती = atomic_add32(गणक_2, (int32_t)(मूल्य_2));              \
    अगर (cur_गिनती > (peak))                                                   \
      peak = cur_गिनती;                                                       \
  } जबतक (0)
#घोषणा _rpmalloc_stat_sub(गणक_2, मूल्य_2)                                     \
  atomic_add32(गणक_2, -(int32_t)(मूल्य_2))
#घोषणा _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  करो {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&heap->आकार_class_use[class_idx].alloc_current);         \
    अगर (alloc_current > heap->आकार_class_use[class_idx].alloc_peak)            \
      heap->आकार_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&heap->आकार_class_use[class_idx].alloc_योग);               \
  } जबतक (0)
#घोषणा _rpmalloc_stat_inc_free(heap, class_idx)                               \
  करो {                                                                         \
    atomic_decr32(&heap->आकार_class_use[class_idx].alloc_current);             \
    atomic_incr32(&heap->आकार_class_use[class_idx].free_योग);                \
  } जबतक (0)
#अन्यथा
#घोषणा _rpmalloc_stat_inc(गणक_2)                                            \
  करो {                                                                         \
  } जबतक (0)
#घोषणा _rpmalloc_stat_dec(गणक_2)                                            \
  करो {                                                                         \
  } जबतक (0)
#घोषणा _rpmalloc_stat_add(गणक_2, मूल्य_2)                                     \
  करो {                                                                         \
  } जबतक (0)
#घोषणा _rpmalloc_stat_add64(गणक_2, मूल्य_2)                                   \
  करो {                                                                         \
  } जबतक (0)
#घोषणा _rpmalloc_stat_add_peak(गणक_2, मूल्य_2, peak)                          \
  करो {                                                                         \
  } जबतक (0)
#घोषणा _rpmalloc_stat_sub(गणक_2, मूल्य_2)                                     \
  करो {                                                                         \
  } जबतक (0)
#घोषणा _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  करो {                                                                         \
  } जबतक (0)
#घोषणा _rpmalloc_stat_inc_free(heap, class_idx)                               \
  करो {                                                                         \
  } जबतक (0)
#पूर्ण_अगर

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#घोषणा SMALL_GRANULARITY 16
//! Small granularity shift count
#घोषणा SMALL_GRANULARITY_SHIFT 4
//! Number of small block size classes
#घोषणा SMALL_CLASS_गिनती 65
//! Maximum size of a small block
#घोषणा SMALL_आकार_LIMIT (SMALL_GRANULARITY * (SMALL_CLASS_गिनती - 1))
//! Granularity of a medium allocation block
#घोषणा MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#घोषणा MEDIUM_GRANULARITY_SHIFT 9
//! Number of medium block size classes
#घोषणा MEDIUM_CLASS_गिनती 61
//! Total number of small + medium size classes
#घोषणा आकार_CLASS_गिनती (SMALL_CLASS_गिनती + MEDIUM_CLASS_गिनती)
//! Number of large block size classes
#घोषणा LARGE_CLASS_गिनती 63
//! Maximum size of a medium block
#घोषणा MEDIUM_आकार_LIMIT                                                      \
  (SMALL_आकार_LIMIT + (MEDIUM_GRANULARITY * MEDIUM_CLASS_गिनती))
//! Maximum size of a large block
#घोषणा LARGE_आकार_LIMIT                                                       \
  ((LARGE_CLASS_गिनती * memory_span_आकार) - SPAN_HEADER_आकार)
//! Size of a span header (must be a multiple of SMALL_GRANULARITY and a power
//! of two)
#घोषणा SPAN_HEADER_आकार 128
//! Number of spans in thread cache
#घोषणा MAX_THREAD_SPAN_CACHE 400
//! Number of spans to transfer between thread and global cache
#घोषणा THREAD_SPAN_CACHE_TRANSFER 64
//! Number of spans in thread cache for large spans (must be greater than
//! LARGE_CLASS_COUNT / 2)
#घोषणा MAX_THREAD_SPAN_LARGE_CACHE 100
//! Number of spans to transfer between thread and global cache for large spans
#घोषणा THREAD_SPAN_LARGE_CACHE_TRANSFER 6

_Static_assert((SMALL_GRANULARITY & (SMALL_GRANULARITY - 1)) == 0,
               "Small granularity must be power of two");
_Static_assert((SPAN_HEADER_आकार & (SPAN_HEADER_आकार - 1)) == 0,
               "Span header size must be power of two");

#अगर ENABLE_VALIDATE_ARGS
//! Maximum allocation size to avoid integer overflow
#अघोषित MAX_ALLOC_आकार
#घोषणा MAX_ALLOC_आकार (((माप_प्रकार) - 1) - memory_span_आकार)
#पूर्ण_अगर

#घोषणा pointer_offset(ptr, ofs) (व्योम *)((अक्षर *)(ptr) + (सूचक_भेद_प्रकार)(ofs))
#घोषणा pointer_diff(first, second)                                            \
  (सूचक_भेद_प्रकार)((स्थिर अक्षर *)(first) - (स्थिर अक्षर *)(second))

#घोषणा INVALID_POINTER ((व्योम *)((uintptr_t) - 1))

#घोषणा आकार_CLASS_LARGE आकार_CLASS_गिनती
#घोषणा आकार_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
प्रकार काष्ठा heap_t heap_t;
//! Span of memory pages
प्रकार काष्ठा span_t span_t;
//! Span list
प्रकार काष्ठा span_list_t span_list_t;
//! Span active data
प्रकार काष्ठा span_active_t span_active_t;
//! Size class definition
प्रकार काष्ठा आकार_class_t आकार_class_t;
//! Global cache
प्रकार काष्ठा global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#घोषणा SPAN_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#घोषणा SPAN_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#घोषणा SPAN_FLAG_ALIGNED_BLOCKS 4U
//! Flag indicating an unmapped master span
#घोषणा SPAN_FLAG_UNMAPPED_MASTER 8U

#अगर ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
काष्ठा span_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t ऊपरी;
#अगर ENABLE_STATISTICS
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
#पूर्ण_अगर
};
प्रकार काष्ठा span_use_t span_use_t;
#पूर्ण_अगर

#अगर ENABLE_STATISTICS
काष्ठा आकार_class_use_t {
  //! Current number of allocations
  atomic32_t alloc_current;
  //! Peak number of allocations
  int32_t alloc_peak;
  //! Total number of allocations
  atomic32_t alloc_योग;
  //! Total number of frees
  atomic32_t free_योग;
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
प्रकार काष्ठा आकार_class_use_t आकार_class_use_t;
#पूर्ण_अगर

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
काष्ठा span_t {
  //! Free list
  व्योम *free_list;
  //! Total block count of size class
  uint32_t block_गिनती;
  //! Size class
  uint32_t आकार_class;
  //! Index of last block initialized in free list
  uint32_t free_list_limit;
  //! Number of used blocks remaining when in partial state
  uint32_t used_गिनती;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_आकार;
  //! Size of a block
  uint32_t block_आकार;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t span_गिनती;
  //! Total span counter for master spans
  uint32_t योग_spans;
  //! Offset from master span for subspans
  uint32_t offset_from_master;
  //! Remaining span counter, for master spans
  atomic32_t remaining_spans;
  //! Alignment offset
  uint32_t align_offset;
  //! Owning heap
  heap_t *heap;
  //! Next span
  span_t *अग्र;
  //! Previous span
  span_t *पूर्व;
};
_Static_assert(माप(span_t) <= SPAN_HEADER_आकार, "span size mismatch");

काष्ठा span_cache_t {
  माप_प्रकार गिनती;
  span_t *span[MAX_THREAD_SPAN_CACHE];
};
प्रकार काष्ठा span_cache_t span_cache_t;

काष्ठा span_large_cache_t {
  माप_प्रकार गिनती;
  span_t *span[MAX_THREAD_SPAN_LARGE_CACHE];
};
प्रकार काष्ठा span_large_cache_t span_large_cache_t;

काष्ठा heap_आकार_class_t {
  //! Free list of active span
  व्योम *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  span_t *partial_span;
  //! Early level cache of fully free spans
  span_t *cache;
};
प्रकार काष्ठा heap_आकार_class_t heap_आकार_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
काष्ठा heap_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  heap_आकार_class_t आकार_class[आकार_CLASS_गिनती];
#अगर ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  span_cache_t span_cache;
#पूर्ण_अगर
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
  heap_t *अग्र_heap;
  //! Next heap in orphan list
  heap_t *अग्र_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  पूर्णांक finalize;
  //! Master heap owning the memory pages
  heap_t *master_heap;
#अगर ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  span_large_cache_t span_large_cache[LARGE_CLASS_गिनती - 1];
#पूर्ण_अगर
#अगर RPMALLOC_FIRST_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  span_t *full_span[आकार_CLASS_गिनती];
  //! Double linked list of large and huge spans allocated by this heap
  span_t *large_huge_span;
#पूर्ण_अगर
#अगर ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  span_use_t span_use[LARGE_CLASS_गिनती];
#पूर्ण_अगर
#अगर ENABLE_STATISTICS
  //! Allocation stats per size class
  आकार_class_use_t आकार_class_use[आकार_CLASS_गिनती + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_to_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_to_thread;
#पूर्ण_अगर
};

// Size class for defining a block size bucket
काष्ठा आकार_class_t {
  //! Size of blocks in this class
  uint32_t block_आकार;
  //! Number of blocks in each chunk
  uint16_t block_गिनती;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(माप(आकार_class_t) == 8, "Size class size mismatch");

काष्ठा global_cache_t {
  //! Cache lock
  atomic32_t lock;
  //! Cache count
  uint32_t गिनती;
#अगर ENABLE_STATISTICS
  //! Insert count
  माप_प्रकार जोड़ो_गिनती;
  //! Extract count
  माप_प्रकार extract_गिनती;
#पूर्ण_अगर
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
#घोषणा memory_default_span_आकार (64 * 1024)
#घोषणा memory_default_span_आकार_shift 16
#घोषणा _memory_default_span_mask (~((uintptr_t)(memory_span_आकार - 1)))

//! Initialized flag
अटल पूर्णांक _rpmalloc_initialized;
//! Main thread ID
अटल uintptr_t _rpmalloc_main_thread_id;
//! Configuration
अटल rpmalloc_config_t _memory_config;
//! Memory page size
अटल माप_प्रकार memory_page_आकार;
//! Shift to divide by page size
अटल माप_प्रकार memory_page_आकार_shift;
//! Granularity at which memory pages are mapped by OS
अटल माप_प्रकार _memory_map_granularity;
#अगर RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
अटल माप_प्रकार memory_span_आकार;
//! Shift to divide by span size
अटल माप_प्रकार memory_span_आकार_shift;
//! Mask to get to start of a memory span
अटल uintptr_t _memory_span_mask;
#अन्यथा
//! Hardwired span size
#घोषणा memory_span_आकार memory_default_span_आकार
#घोषणा memory_span_आकार_shift memory_default_span_आकार_shift
#घोषणा _memory_span_mask _memory_default_span_mask
#पूर्ण_अगर
//! Number of spans to map in each map call
अटल माप_प्रकार memory_span_map_गिनती;
//! Number of spans to keep reserved in each heap
अटल माप_प्रकार memory_heap_reserve_गिनती;
//! Global size classes
अटल आकार_class_t memory_आकार_class[आकार_CLASS_गिनती];
//! Run-time size limit of medium blocks
अटल माप_प्रकार memory_medium_आकार_limit;
//! Heap ID counter
अटल atomic32_t _memory_heap_id;
//! Huge page support
अटल पूर्णांक _memory_huge_pages;
#अगर ENABLE_GLOBAL_CACHE
//! Global span cache
अटल global_cache_t _memory_span_cache[LARGE_CLASS_गिनती];
#पूर्ण_अगर
//! Global reserved spans
अटल span_t *_memory_global_reserve;
//! Global reserved count
अटल माप_प्रकार memory_global_reserve_गिनती;
//! Global reserved master
अटल span_t *_memory_global_reserve_master;
//! All heaps
अटल heap_t *_memory_heaps[HEAP_ARRAY_आकार];
//! Used to restrict access to mapping memory for huge pages
अटल atomic32_t _memory_global_lock;
//! Orphaned heaps
अटल heap_t *_memory_orphan_heaps;
#अगर RPMALLOC_FIRST_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
अटल heap_t *_memory_first_class_orphan_heaps;
#पूर्ण_अगर
#अगर ENABLE_STATISTICS
//! Allocations counter
अटल atomic64_t allocation_गणक;
//! Deallocations counter
अटल atomic64_t deallocation_गणक;
//! Active heap count
अटल atomic32_t _memory_active_heaps;
//! Number of currently mapped memory pages
अटल atomic32_t _mapped_pages;
//! Peak number of concurrently mapped memory pages
अटल int32_t _mapped_pages_peak;
//! Number of mapped master spans
अटल atomic32_t _master_spans;
//! Number of unmapped dangling master spans
अटल atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
अटल atomic32_t mapped_योग;
//! Running counter of total number of unmapped memory pages since start
अटल atomic32_t unmapped_योग;
//! Number of currently mapped memory pages in OS calls
अटल atomic32_t _mapped_pages_os;
//! Number of currently allocated pages in huge allocations
अटल atomic32_t _huge_pages_current;
//! Peak number of currently allocated pages in huge allocations
अटल int32_t _huge_pages_peak;
#पूर्ण_अगर

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#अगर ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
अटल pthread_key_t _memory_thread_heap;
#अन्यथा
#अगर_घोषित _MSC_VER
#घोषणा _Thread_local __declspec(thread)
#घोषणा TLS_MODEL
#अन्यथा
#अगर_अघोषित __HAIKU__
#घोषणा TLS_MODEL __attribute__((tls_model("initial-exec")))
#अन्यथा
#घोषणा TLS_MODEL
#पूर्ण_अगर
#अगर !defined(__clang__) && defined(__GNUC__)
#घोषणा _Thread_local __thread
#पूर्ण_अगर
#पूर्ण_अगर
अटल _Thread_local heap_t *_memory_thread_heap TLS_MODEL;
#पूर्ण_अगर

अटल अंतरभूत heap_t *get_thread_heap_raw(व्योम) {
#अगर (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  वापस pthread_getspecific(_memory_thread_heap);
#अन्यथा
  वापस _memory_thread_heap;
#पूर्ण_अगर
}

//! Get the current thread heap
अटल अंतरभूत heap_t *get_thread_heap(व्योम) {
  heap_t *heap = get_thread_heap_raw();
#अगर ENABLE_PRELOAD
  अगर (EXPECTED(heap != 0))
    वापस heap;
  rpmalloc_initialize();
  वापस get_thread_heap_raw();
#अन्यथा
  वापस heap;
#पूर्ण_अगर
}

//! Fast thread ID
अटल अंतरभूत uintptr_t get_thread_id(व्योम) {
#अगर defined(_WIN32)
  वापस (uintptr_t)((व्योम *)NtCurrentTeb());
#या_अगर (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#अगर defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#या_अगर defined(__x86_64__)
#अगर defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#अन्यथा
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#पूर्ण_अगर
#या_अगर defined(__arm__)
  __asm__ अस्थिर("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#या_अगर defined(__aarch64__)
#अगर defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ अस्थिर("mrs %0, tpidrro_el0" : "=r"(tid));
#अन्यथा
  __asm__ अस्थिर("mrs %0, tpidr_el0" : "=r"(tid));
#पूर्ण_अगर
#अन्यथा
#त्रुटि This platform needs implementation of get_thread_id()
#पूर्ण_अगर
  वापस tid;
#अन्यथा
#त्रुटि This platform needs implementation of get_thread_id()
#पूर्ण_अगर
}

//! Set the current thread heap
अटल व्योम set_thread_heap(heap_t *heap) {
#अगर ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(_memory_thread_heap, heap);
#अन्यथा
  _memory_thread_heap = heap;
#पूर्ण_अगर
  अगर (heap)
    heap->owner_thread = get_thread_id();
}

//! Set main thread ID
बाह्य व्योम rpmalloc_set_main_thread(व्योम);

व्योम rpmalloc_set_main_thread(व्योम) {
  _rpmalloc_main_thread_id = get_thread_id();
}

अटल व्योम _rpmalloc_spin(व्योम) {
#अगर defined(_MSC_VER)
#अगर defined(_M_ARM64)
  __yield();
#अन्यथा
  _mm_pause();
#पूर्ण_अगर
#या_अगर defined(__x86_64__) || defined(__i386__)
  __asm__ अस्थिर("pause" ::: "memory");
#या_अगर defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ अस्थिर("yield" ::: "memory");
#या_अगर defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ अस्थिर("or 27,27,27");
#या_अगर defined(__sparc__)
  __asm__ अस्थिर("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#अन्यथा
  काष्ठा timespec ts = {0};
  nanosleep(&ts, 0);
#पूर्ण_अगर
}

#अगर defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

अटल व्योम NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  चयन (reason) {
  हाल DLL_PROCESS_ATTACH:
    अवरोध;
  हाल DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    अवरोध;
  हाल DLL_THREAD_ATTACH:
    अवरोध;
  हाल DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    अवरोध;
  }
}

#अगर_घोषित _WIN64
#आशय comment(linker, "/INCLUDE:_tls_used")
#आशय comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#आशय const_seg(".CRT$XLY")

बाह्य स्थिर PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
स्थिर PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#आशय const_seg()
#अन्यथा // _WIN64
#आशय comment(linker, "/INCLUDE:__tls_used")
#आशय comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#आशय तथ्य_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#आशय तथ्य_seg()
#पूर्ण_अगर // _WIN64

अटल व्योम NTAPI _rpmalloc_thread_destructor(व्योम *मूल्य_2) {
#अगर ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  अगर (get_thread_id() == _rpmalloc_main_thread_id)
    वापस;
#पूर्ण_अगर
  अगर (मूल्य_2)
    rpmalloc_thread_finalize(1);
}
#पूर्ण_अगर

////////////
///
/// Low level memory map/unmap
///
//////

अटल व्योम _rpmalloc_set_name(व्योम *address, माप_प्रकार आकार_2) {
#अगर defined(__linux__) || defined(__ANDROID__)
  स्थिर अक्षर *name = _memory_huge_pages ? _memory_config.huge_page_name
                                        : _memory_config.page_name;
  अगर (address == MAP_FAILED || !name)
    वापस;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (व्योम)prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (uintptr_t)address, आकार_2,
              (uintptr_t)name);
#अन्यथा
  (व्योम)माप(आकार_2);
  (व्योम)माप(address);
#पूर्ण_अगर
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
अटल व्योम *_rpmalloc_mmap(माप_प्रकार आकार_2, माप_प्रकार *offset) {
  rpmalloc_assert(!(आकार_2 % memory_page_आकार), "Invalid mmap size");
  rpmalloc_assert(आकार_2 >= memory_page_आकार, "Invalid mmap size");
  व्योम *address = _memory_config.memory_map(आकार_2, offset);
  अगर (EXPECTED(address != 0)) {
    _rpmalloc_stat_add_peak(&_mapped_pages, (आकार_2 >> memory_page_आकार_shift),
                            _mapped_pages_peak);
    _rpmalloc_stat_add(&mapped_योग, (आकार_2 >> memory_page_आकार_shift));
  }
  वापस address;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
अटल व्योम _rpmalloc_unmap(व्योम *address, माप_प्रकार आकार_2, माप_प्रकार offset,
                            माप_प्रकार release) {
  rpmalloc_assert(!release || (release >= आकार_2), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= memory_page_आकार),
                  "Invalid unmap size");
  अगर (release) {
    rpmalloc_assert(!(release % memory_page_आकार), "Invalid unmap size");
    _rpmalloc_stat_sub(&_mapped_pages, (release >> memory_page_आकार_shift));
    _rpmalloc_stat_add(&unmapped_योग, (release >> memory_page_आकार_shift));
  }
  _memory_config.memory_unmap(address, आकार_2, offset, release);
}

//! Default implementation to map new pages to virtual memory
अटल व्योम *_rpmalloc_mmap_os(माप_प्रकार आकार_2, माप_प्रकार *offset) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  माप_प्रकार padding = ((आकार_2 >= memory_span_आकार) &&
                    (memory_span_आकार > _memory_map_granularity))
                       ? memory_span_आकार
                       : 0;
  rpmalloc_assert(आकार_2 >= memory_page_आकार, "Invalid mmap size");
#अगर PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  व्योम *ptr = VirtualAlloc(0, आकार_2 + padding,
                           (_memory_huge_pages ? MEM_LARGE_PAGES : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
  अगर (!ptr) {
    अगर (_memory_config.map_fail_callback) {
      अगर (_memory_config.map_fail_callback(आकार_2 + padding))
        वापस _rpmalloc_mmap_os(आकार_2, offset);
    } अन्यथा {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    वापस 0;
  }
#अन्यथा
  पूर्णांक flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#अगर defined(__APPLE__) && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
  पूर्णांक fd = (पूर्णांक)VM_MAKE_TAG(240U);
  अगर (_memory_huge_pages)
    fd |= VM_FLAGS_SUPERPAGE_आकार_2_MB;
  व्योम *ptr = mmap(0, आकार_2 + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#या_अगर defined(MAP_HUGETLB)
  व्योम *ptr = mmap(0, आकार_2 + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (_memory_huge_pages ? MAP_HUGETLB : 0) | flags, -1, 0);
#अगर defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  अगर ((ptr == MAP_FAILED || !ptr) && _memory_huge_pages) {
    ptr = mmap(0, आकार_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    अगर (ptr && ptr != MAP_FAILED) {
      पूर्णांक prm = madvise(ptr, आकार_2 + padding, MADV_HUGEPAGE);
      (व्योम)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#पूर्ण_अगर
  _rpmalloc_set_name(ptr, आकार_2 + padding);
#या_अगर defined(MAP_ALIGNED)
  स्थिर माप_प्रकार align =
      (माप(माप_प्रकार) * 8) - (माप_प्रकार)(__builtin_clzl(आकार_2 - 1));
  व्योम *ptr =
      mmap(0, आकार_2 + padding, PROT_READ | PROT_WRITE,
           (_memory_huge_pages ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#या_अगर defined(MAP_ALIGN)
  caddr_t base = (_memory_huge_pages ? (caddr_t)(4 << 20) : 0);
  व्योम *ptr = mmap(base, आकार_2 + padding, PROT_READ | PROT_WRITE,
                   (_memory_huge_pages ? MAP_ALIGN : 0) | flags, -1, 0);
#अन्यथा
  व्योम *ptr = mmap(0, आकार_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#पूर्ण_अगर
  अगर ((ptr == MAP_FAILED) || !ptr) {
    अगर (_memory_config.map_fail_callback) {
      अगर (_memory_config.map_fail_callback(आकार_2 + padding))
        वापस _rpmalloc_mmap_os(आकार_2, offset);
    } अन्यथा अगर (त्रुटि_सं != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    वापस 0;
  }
#पूर्ण_अगर
  _rpmalloc_stat_add(&_mapped_pages_os,
                     (int32_t)((आकार_2 + padding) >> memory_page_आकार_shift));
  अगर (padding) {
    माप_प्रकार final_padding = padding - ((uintptr_t)ptr & ~_memory_span_mask);
    rpmalloc_assert(final_padding <= memory_span_आकार,
                    "Internal failure in padding");
    rpmalloc_assert(final_padding <= padding, "Internal failure in padding");
    rpmalloc_assert(!(final_padding % 8), "Internal failure in padding");
    ptr = pointer_offset(ptr, final_padding);
    *offset = final_padding >> 3;
  }
  rpmalloc_assert((आकार_2 < memory_span_आकार) ||
                      !((uintptr_t)ptr & ~_memory_span_mask),
                  "Internal failure in padding");
  वापस ptr;
}

//! Default implementation to unmap pages from virtual memory
अटल व्योम _rpmalloc_unmap_os(व्योम *address, माप_प्रकार आकार_2, माप_प्रकार offset,
                               माप_प्रकार release) {
  rpmalloc_assert(release || (offset == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= memory_page_आकार),
                  "Invalid unmap size");
  rpmalloc_assert(आकार_2 >= memory_page_आकार, "Invalid unmap size");
  अगर (release && offset) {
    offset <<= 3;
    address = pointer_offset(address, -(int32_t)offset);
    अगर ((release >= memory_span_आकार) &&
        (memory_span_आकार > _memory_map_granularity)) {
      // Padding is always one span size
      release += memory_span_आकार;
    }
  }
#अगर !DISABLE_UNMAP
#अगर PLATFORM_WINDOWS
  अगर (!VirtualFree(address, release ? 0 : आकार_2,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#अन्यथा
  अगर (release) {
    अगर (munmap(address, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } अन्यथा {
#अगर defined(MADV_FREE_REUSABLE)
    पूर्णांक ret;
    जबतक ((ret = madvise(address, आकार_2, MADV_FREE_REUSABLE)) == -1 &&
           (त्रुटि_सं == EAGAIN))
      त्रुटि_सं = 0;
    अगर ((ret == -1) && (त्रुटि_सं != 0)) {
#या_अगर defined(MADV_DONTNEED)
    अगर (madvise(address, आकार_2, MADV_DONTNEED)) {
#या_अगर defined(MADV_PAGEOUT)
    अगर (madvise(address, आकार_2, MADV_PAGEOUT)) {
#या_अगर defined(MADV_FREE)
    अगर (madvise(address, आकार_2, MADV_FREE)) {
#अन्यथा
    अगर (posix_madvise(address, आकार_2, POSIX_MADV_DONTNEED)) {
#पूर्ण_अगर
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#पूर्ण_अगर
#पूर्ण_अगर
  अगर (release)
    _rpmalloc_stat_sub(&_mapped_pages_os, release >> memory_page_आकार_shift);
}

अटल व्योम _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         माप_प्रकार span_गिनती);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
अटल span_t *_rpmalloc_global_get_reserved_spans(माप_प्रकार span_गिनती) {
  span_t *span = _memory_global_reserve;
  _rpmalloc_span_mark_as_subspan_unless_master(_memory_global_reserve_master,
                                               span, span_गिनती);
  memory_global_reserve_गिनती -= span_गिनती;
  अगर (memory_global_reserve_गिनती)
    _memory_global_reserve =
        (span_t *)pointer_offset(span, span_गिनती << memory_span_आकार_shift);
  अन्यथा
    _memory_global_reserve = 0;
  वापस span;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
अटल व्योम _rpmalloc_global_set_reserved_spans(span_t *master, span_t *reserve,
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
अटल व्योम _rpmalloc_span_double_link_list_add(span_t **शीर्ष, span_t *span) {
  अगर (*शीर्ष)
    (*शीर्ष)->पूर्व = span;
  span->अग्र = *शीर्ष;
  *शीर्ष = span;
}

//! Pop head span from double linked list
अटल व्योम rpmalloc_span_double_link_list_निकालो_शीर्ष(span_t **शीर्ष,
                                                     span_t *span) {
  rpmalloc_assert(*शीर्ष == span, "Linked list corrupted");
  span = *शीर्ष;
  *शीर्ष = span->अग्र;
}

//! Remove a span from double linked list
अटल व्योम _rpmalloc_span_double_link_list_remove(span_t **शीर्ष,
                                                   span_t *span) {
  rpmalloc_assert(*शीर्ष, "Linked list corrupted");
  अगर (*शीर्ष == span) {
    *शीर्ष = span->अग्र;
  } अन्यथा {
    span_t *अग्र_span = span->अग्र;
    span_t *पूर्व_span = span->पूर्व;
    पूर्व_span->अग्र = अग्र_span;
    अगर (EXPECTED(अग्र_span != 0))
      अग्र_span->पूर्व = पूर्व_span;
  }
}

////////////
///
/// Span control
///
//////

अटल व्योम rpmalloc_heap_cache_जोड़ो(heap_t *heap, span_t *span);

अटल व्योम _rpmalloc_heap_finalize(heap_t *heap);

अटल व्योम _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              माप_प्रकार reserve_span_गिनती);

//! Declare the span to be a subspan and store distance from master span and
//! span count
अटल व्योम _rpmalloc_span_mark_as_subspan_unless_master(span_t *master,
                                                         span_t *subspan,
                                                         माप_प्रकार span_गिनती) {
  rpmalloc_assert((subspan != master) || (subspan->flags & SPAN_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  अगर (subspan != master) {
    subspan->flags = SPAN_FLAG_SUBSPAN;
    subspan->offset_from_master =
        (uint32_t)((uintptr_t)pointer_diff(subspan, master) >>
                   memory_span_आकार_shift);
    subspan->align_offset = 0;
  }
  subspan->span_गिनती = (uint32_t)span_गिनती;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
अटल span_t *_rpmalloc_span_map_from_reserve(heap_t *heap,
                                               माप_प्रकार span_गिनती) {
  // Update the heap span reserve
  span_t *span = heap->span_reserve;
  heap->span_reserve =
      (span_t *)pointer_offset(span, span_गिनती * memory_span_आकार);
  heap->spans_reserved -= (uint32_t)span_गिनती;

  _rpmalloc_span_mark_as_subspan_unless_master(heap->span_reserve_master, span,
                                               span_गिनती);
  अगर (span_गिनती <= LARGE_CLASS_गिनती)
    _rpmalloc_stat_inc(&heap->span_use[span_गिनती - 1].spans_from_reserved);

  वापस span;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
अटल माप_प्रकार rpmalloc_span_align_गिनती(माप_प्रकार span_गिनती) {
  माप_प्रकार request_गिनती = (span_गिनती > memory_span_map_गिनती)
                             ? span_गिनती
                             : memory_span_map_गिनती;
  अगर ((memory_page_आकार > memory_span_आकार) &&
      ((request_गिनती * memory_span_आकार) % memory_page_आकार))
    request_गिनती +=
        memory_span_map_गिनती - (request_गिनती % memory_span_map_गिनती);
  वापस request_गिनती;
}

//! Setup a newly mapped span
अटल व्योम _rpmalloc_span_initialize(span_t *span, माप_प्रकार योग_span_गिनती,
                                      माप_प्रकार span_गिनती, माप_प्रकार align_offset) {
  span->योग_spans = (uint32_t)योग_span_गिनती;
  span->span_गिनती = (uint32_t)span_गिनती;
  span->align_offset = (uint32_t)align_offset;
  span->flags = SPAN_FLAG_MASTER;
  atomic_store32(&span->remaining_spans, (int32_t)योग_span_गिनती);
}

अटल व्योम _rpmalloc_span_unmap(span_t *span);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
अटल span_t *rpmalloc_span_map_aligned_गिनती(heap_t *heap,
                                                माप_प्रकार span_गिनती) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  माप_प्रकार aligned_span_गिनती = rpmalloc_span_align_गिनती(span_गिनती);
  माप_प्रकार align_offset = 0;
  span_t *span = (span_t *)_rpmalloc_mmap(
      aligned_span_गिनती * memory_span_आकार, &align_offset);
  अगर (!span)
    वापस 0;
  _rpmalloc_span_initialize(span, aligned_span_गिनती, span_गिनती, align_offset);
  _rpmalloc_stat_inc(&_master_spans);
  अगर (span_गिनती <= LARGE_CLASS_गिनती)
    _rpmalloc_stat_inc(&heap->span_use[span_गिनती - 1].spans_map_calls);
  अगर (aligned_span_गिनती > span_गिनती) {
    span_t *reserved_spans =
        (span_t *)pointer_offset(span, span_गिनती * memory_span_आकार);
    माप_प्रकार reserved_गिनती = aligned_span_गिनती - span_गिनती;
    अगर (heap->spans_reserved) {
      _rpmalloc_span_mark_as_subspan_unless_master(
          heap->span_reserve_master, heap->span_reserve, heap->spans_reserved);
      rpmalloc_heap_cache_जोड़ो(heap, heap->span_reserve);
    }
    अगर (reserved_गिनती > memory_heap_reserve_गिनती) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&_memory_global_lock) == 1,
                      "Global spin lock not held as expected");
      माप_प्रकार remain_गिनती = reserved_गिनती - memory_heap_reserve_गिनती;
      reserved_गिनती = memory_heap_reserve_गिनती;
      span_t *remain_span = (span_t *)pointer_offset(
          reserved_spans, reserved_गिनती * memory_span_आकार);
      अगर (_memory_global_reserve) {
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
  वापस span;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
अटल span_t *_rpmalloc_span_map(heap_t *heap, माप_प्रकार span_गिनती) {
  अगर (span_गिनती <= heap->spans_reserved)
    वापस _rpmalloc_span_map_from_reserve(heap, span_गिनती);
  span_t *span = 0;
  पूर्णांक use_global_reserve =
      (memory_page_आकार > memory_span_आकार) ||
      (memory_span_map_गिनती > memory_heap_reserve_गिनती);
  अगर (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    जबतक (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
    अगर (memory_global_reserve_गिनती >= span_गिनती) {
      माप_प्रकार reserve_गिनती =
          (!heap->spans_reserved ? memory_heap_reserve_गिनती : span_गिनती);
      अगर (memory_global_reserve_गिनती < reserve_गिनती)
        reserve_गिनती = memory_global_reserve_गिनती;
      span = _rpmalloc_global_get_reserved_spans(reserve_गिनती);
      अगर (span) {
        अगर (reserve_गिनती > span_गिनती) {
          span_t *reserved_span = (span_t *)pointer_offset(
              span, span_गिनती << memory_span_आकार_shift);
          _rpmalloc_heap_set_reserved_spans(heap, _memory_global_reserve_master,
                                            reserved_span,
                                            reserve_गिनती - span_गिनती);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        span->span_गिनती = (uint32_t)span_गिनती;
      }
    }
  }
  अगर (!span)
    span = rpmalloc_span_map_aligned_गिनती(heap, span_गिनती);
  अगर (use_global_reserve)
    atomic_store32_release(&_memory_global_lock, 0);
  वापस span;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
अटल व्योम _rpmalloc_span_unmap(span_t *span) {
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");

  पूर्णांक is_master = !!(span->flags & SPAN_FLAG_MASTER);
  span_t *master =
      is_master ? span
                : ((span_t *)pointer_offset(
                      span, -(intptr_t)((uintptr_t)span->offset_from_master *
                                        memory_span_आकार)));
  rpmalloc_assert(is_master || (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & SPAN_FLAG_MASTER, "Span flag corrupted");

  माप_प्रकार span_गिनती = span->span_गिनती;
  अगर (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(span->align_offset == 0, "Span align offset corrupted");
    अगर (memory_span_आकार >= memory_page_आकार)
      _rpmalloc_unmap(span, span_गिनती * memory_span_आकार, 0, 0);
  } अन्यथा {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    span->flags |=
        SPAN_FLAG_MASTER | SPAN_FLAG_SUBSPAN | SPAN_FLAG_UNMAPPED_MASTER;
    _rpmalloc_stat_add(&_unmapped_master_spans, 1);
  }

  अगर (atomic_add32(&master->remaining_spans, -(int32_t)span_गिनती) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & SPAN_FLAG_MASTER) &&
                        !!(master->flags & SPAN_FLAG_SUBSPAN),
                    "Span flag corrupted");
    माप_प्रकार unmap_गिनती = master->span_गिनती;
    अगर (memory_span_आकार < memory_page_आकार)
      unmap_गिनती = master->योग_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_गिनती * memory_span_आकार,
                    master->align_offset,
                    (माप_प्रकार)master->योग_spans * memory_span_आकार);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
अटल व्योम _rpmalloc_span_release_to_cache(heap_t *heap, span_t *span) {
  rpmalloc_assert(heap == span->heap, "Span heap pointer corrupted");
  rpmalloc_assert(span->आकार_class < आकार_CLASS_गिनती,
                  "Invalid span size class");
  rpmalloc_assert(span->span_गिनती == 1, "Invalid span count");
#अगर ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&heap->span_use[0].current);
#पूर्ण_अगर
  _rpmalloc_stat_dec(&heap->आकार_class_use[span->आकार_class].spans_current);
  अगर (!heap->finalize) {
    _rpmalloc_stat_inc(&heap->span_use[0].spans_to_cache);
    _rpmalloc_stat_inc(&heap->आकार_class_use[span->आकार_class].spans_to_cache);
    अगर (heap->आकार_class[span->आकार_class].cache)
      rpmalloc_heap_cache_जोड़ो(heap,
                                  heap->आकार_class[span->आकार_class].cache);
    heap->आकार_class[span->आकार_class].cache = span;
  } अन्यथा {
    _rpmalloc_span_unmap(span);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
अटल uint32_t free_list_partial_init(व्योम **list, व्योम **first_block,
                                       व्योम *page_start, व्योम *block_start,
                                       uint32_t block_गिनती,
                                       uint32_t block_आकार) {
  rpmalloc_assert(block_गिनती, "Internal failure");
  *first_block = block_start;
  अगर (block_गिनती > 1) {
    व्योम *free_block = pointer_offset(block_start, block_आकार);
    व्योम *block_end =
        pointer_offset(block_start, (माप_प्रकार)block_आकार * block_गिनती);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    अगर (block_आकार < (memory_page_आकार >> 1)) {
      व्योम *page_end = pointer_offset(page_start, memory_page_आकार);
      अगर (page_end < block_end)
        block_end = page_end;
    }
    *list = free_block;
    block_गिनती = 2;
    व्योम *अग्र_block = pointer_offset(free_block, block_आकार);
    जबतक (अग्र_block < block_end) {
      *((व्योम **)free_block) = अग्र_block;
      free_block = अग्र_block;
      ++block_गिनती;
      अग्र_block = pointer_offset(अग्र_block, block_आकार);
    }
    *((व्योम **)free_block) = 0;
  } अन्यथा {
    *list = 0;
  }
  वापस block_गिनती;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
अटल व्योम *_rpmalloc_span_initialize_new(heap_t *heap,
                                           heap_आकार_class_t *heap_आकार_class,
                                           span_t *span, uint32_t class_idx) {
  rpmalloc_assert(span->span_गिनती == 1, "Internal failure");
  आकार_class_t *आकार_class = memory_आकार_class + class_idx;
  span->आकार_class = class_idx;
  span->heap = heap;
  span->flags &= ~SPAN_FLAG_ALIGNED_BLOCKS;
  span->block_आकार = आकार_class->block_आकार;
  span->block_गिनती = आकार_class->block_गिनती;
  span->free_list = 0;
  span->list_आकार = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  व्योम *block;
  span->free_list_limit =
      free_list_partial_init(&heap_आकार_class->free_list, &block, span,
                             pointer_offset(span, SPAN_HEADER_आकार),
                             आकार_class->block_गिनती, आकार_class->block_आकार);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  अगर (span->free_list_limit < span->block_गिनती) {
    _rpmalloc_span_double_link_list_add(&heap_आकार_class->partial_span, span);
    span->used_गिनती = span->free_list_limit;
  } अन्यथा {
#अगर RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#पूर्ण_अगर
    ++heap->full_span_गिनती;
    span->used_गिनती = span->block_गिनती;
  }
  वापस block;
}

अटल व्योम _rpmalloc_span_extract_free_list_deferred(span_t *span) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  करो {
    span->free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } जबतक (span->free_list == INVALID_POINTER);
  span->used_गिनती -= span->list_आकार;
  span->list_आकार = 0;
  atomic_store_ptr_release(&span->free_list_deferred, 0);
}

अटल पूर्णांक _rpmalloc_span_is_fully_utilized(span_t *span) {
  rpmalloc_assert(span->free_list_limit <= span->block_गिनती,
                  "Span free list corrupted");
  वापस !span->free_list && (span->free_list_limit >= span->block_गिनती);
}

अटल पूर्णांक _rpmalloc_span_finalize(heap_t *heap, माप_प्रकार iclass, span_t *span,
                                   span_t **list_शीर्ष) {
  व्योम *free_list = heap->आकार_class[iclass].free_list;
  span_t *class_span = (span_t *)((uintptr_t)free_list & _memory_span_mask);
  अगर (span == class_span) {
    // Adopt the heap class free list back into the span free list
    व्योम *block = span->free_list;
    व्योम *last_block = 0;
    जबतक (block) {
      last_block = block;
      block = *((व्योम **)block);
    }
    uint32_t free_गिनती = 0;
    block = free_list;
    जबतक (block) {
      ++free_गिनती;
      block = *((व्योम **)block);
    }
    अगर (last_block) {
      *((व्योम **)last_block) = free_list;
    } अन्यथा {
      span->free_list = free_list;
    }
    heap->आकार_class[iclass].free_list = 0;
    span->used_गिनती -= free_गिनती;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(span->list_आकार == span->used_गिनती, "Memory leak detected");
  अगर (span->list_आकार == span->used_गिनती) {
    _rpmalloc_stat_dec(&heap->span_use[0].current);
    _rpmalloc_stat_dec(&heap->आकार_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    अगर (list_शीर्ष)
      _rpmalloc_span_double_link_list_remove(list_शीर्ष, span);
    _rpmalloc_span_unmap(span);
    वापस 1;
  }
  वापस 0;
}

////////////
///
/// Global cache
///
//////

#अगर ENABLE_GLOBAL_CACHE

//! Finalize a global cache
अटल व्योम _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  जबतक (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

  क्रम (माप_प्रकार ispan = 0; ispan < cache->गिनती; ++ispan)
    _rpmalloc_span_unmap(cache->span[ispan]);
  cache->गिनती = 0;

  जबतक (cache->overflow) {
    span_t *span = cache->overflow;
    cache->overflow = span->अग्र;
    _rpmalloc_span_unmap(span);
  }

  atomic_store32_release(&cache->lock, 0);
}

अटल व्योम rpmalloc_global_cache_जोड़ो_spans(span_t **span,
                                                माप_प्रकार span_गिनती,
                                                माप_प्रकार गिनती) {
  स्थिर माप_प्रकार cache_limit =
      (span_गिनती == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_SPAN_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_SPAN_LARGE_CACHE - (span_गिनती >> 1));

  global_cache_t *cache = &_memory_span_cache[span_गिनती - 1];

  माप_प्रकार जोड़ो_गिनती = गिनती;
  जबतक (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#अगर ENABLE_STATISTICS
  cache->जोड़ो_गिनती += गिनती;
#पूर्ण_अगर
  अगर ((cache->गिनती + जोड़ो_गिनती) > cache_limit)
    जोड़ो_गिनती = cache_limit - cache->गिनती;

  स_नकल(cache->span + cache->गिनती, span, माप(span_t *) * जोड़ो_गिनती);
  cache->गिनती += (uint32_t)जोड़ो_गिनती;

#अगर ENABLE_UNLIMITED_CACHE
  जबतक (जोड़ो_गिनती < गिनती) {
#अन्यथा
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  जबतक ((memory_page_आकार > memory_span_आकार) && (जोड़ो_गिनती < गिनती)) {
#पूर्ण_अगर
    span_t *current_span = span[जोड़ो_गिनती++];
    current_span->अग्र = cache->overflow;
    cache->overflow = current_span;
  }
  atomic_store32_release(&cache->lock, 0);

  span_t *keep = 0;
  क्रम (माप_प्रकार ispan = जोड़ो_गिनती; ispan < गिनती; ++ispan) {
    span_t *current_span = span[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    अगर ((current_span->flags & SPAN_FLAG_MASTER) &&
        (atomic_load32(&current_span->remaining_spans) >
         (int32_t)current_span->span_गिनती)) {
      current_span->अग्र = keep;
      keep = current_span;
    } अन्यथा {
      _rpmalloc_span_unmap(current_span);
    }
  }

  अगर (keep) {
    जबतक (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();

    माप_प्रकार islot = 0;
    जबतक (keep) {
      क्रम (; islot < cache->गिनती; ++islot) {
        span_t *current_span = cache->span[islot];
        अगर (!(current_span->flags & SPAN_FLAG_MASTER) ||
            ((current_span->flags & SPAN_FLAG_MASTER) &&
             (atomic_load32(&current_span->remaining_spans) <=
              (int32_t)current_span->span_गिनती))) {
          _rpmalloc_span_unmap(current_span);
          cache->span[islot] = keep;
          अवरोध;
        }
      }
      अगर (islot == cache->गिनती)
        अवरोध;
      keep = keep->अग्र;
    }

    अगर (keep) {
      span_t *पुच्छ_2 = keep;
      जबतक (पुच्छ_2->अग्र)
        पुच्छ_2 = पुच्छ_2->अग्र;
      पुच्छ_2->अग्र = cache->overflow;
      cache->overflow = keep;
    }

    atomic_store32_release(&cache->lock, 0);
  }
}

अटल माप_प्रकार _rpmalloc_global_cache_extract_spans(span_t **span,
                                                   माप_प्रकार span_गिनती,
                                                   माप_प्रकार गिनती) {
  global_cache_t *cache = &_memory_span_cache[span_गिनती - 1];

  माप_प्रकार extract_गिनती = 0;
  जबतक (!atomic_cas32_acquire(&cache->lock, 1, 0))
    _rpmalloc_spin();

#अगर ENABLE_STATISTICS
  cache->extract_गिनती += गिनती;
#पूर्ण_अगर
  माप_प्रकार want = गिनती - extract_गिनती;
  अगर (want > cache->गिनती)
    want = cache->गिनती;

  स_नकल(span + extract_गिनती, cache->span + (cache->गिनती - want),
         माप(span_t *) * want);
  cache->गिनती -= (uint32_t)want;
  extract_गिनती += want;

  जबतक ((extract_गिनती < गिनती) && cache->overflow) {
    span_t *current_span = cache->overflow;
    span[extract_गिनती++] = current_span;
    cache->overflow = current_span->अग्र;
  }

#अगर ENABLE_ASSERTS
  क्रम (माप_प्रकार ispan = 0; ispan < extract_गिनती; ++ispan) {
    rpmalloc_assert(span[ispan]->span_गिनती == span_गिनती,
                    "Global cache span count mismatch");
  }
#पूर्ण_अगर

  atomic_store32_release(&cache->lock, 0);

  वापस extract_गिनती;
}

#पूर्ण_अगर

////////////
///
/// Heap control
///
//////

अटल व्योम _rpmalloc_deallocate_huge(span_t *);

//! Store the given spans as reserve in the given heap
अटल व्योम _rpmalloc_heap_set_reserved_spans(heap_t *heap, span_t *master,
                                              span_t *reserve,
                                              माप_प्रकार reserve_span_गिनती) {
  heap->span_reserve_master = master;
  heap->span_reserve = reserve;
  heap->spans_reserved = (uint32_t)reserve_span_गिनती;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
अटल व्योम _rpmalloc_heap_cache_adopt_deferred(heap_t *heap,
                                                span_t **single_span) {
  span_t *span = (span_t *)((व्योम *)atomic_exchange_ptr_acquire(
      &heap->span_free_deferred, 0));
  जबतक (span) {
    span_t *अग्र_span = (span_t *)span->free_list;
    rpmalloc_assert(span->heap == heap, "Span heap pointer corrupted");
    अगर (EXPECTED(span->आकार_class < आकार_CLASS_गिनती)) {
      rpmalloc_assert(heap->full_span_गिनती, "Heap span counter corrupted");
      --heap->full_span_गिनती;
      _rpmalloc_stat_dec(&heap->span_use[0].spans_deferred);
#अगर RPMALLOC_FIRST_CLASS_HEAPS
      _rpmalloc_span_double_link_list_remove(&heap->full_span[span->आकार_class],
                                             span);
#पूर्ण_अगर
      _rpmalloc_stat_dec(&heap->span_use[0].current);
      _rpmalloc_stat_dec(&heap->आकार_class_use[span->आकार_class].spans_current);
      अगर (single_span && !*single_span)
        *single_span = span;
      अन्यथा
        rpmalloc_heap_cache_जोड़ो(heap, span);
    } अन्यथा {
      अगर (span->आकार_class == आकार_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(span);
      } अन्यथा {
        rpmalloc_assert(span->आकार_class == आकार_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(heap->full_span_गिनती, "Heap span counter corrupted");
        --heap->full_span_गिनती;
#अगर RPMALLOC_FIRST_CLASS_HEAPS
        _rpmalloc_span_double_link_list_remove(&heap->large_huge_span, span);
#पूर्ण_अगर
        uint32_t idx = span->span_गिनती - 1;
        _rpmalloc_stat_dec(&heap->span_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&heap->span_use[idx].current);
        अगर (!idx && single_span && !*single_span)
          *single_span = span;
        अन्यथा
          rpmalloc_heap_cache_जोड़ो(heap, span);
      }
    }
    span = अग्र_span;
  }
}

अटल व्योम _rpmalloc_heap_unmap(heap_t *heap) {
  अगर (!heap->master_heap) {
    अगर ((heap->finalize > 1) && !atomic_load32(&heap->child_गिनती)) {
      span_t *span = (span_t *)((uintptr_t)heap & _memory_span_mask);
      _rpmalloc_span_unmap(span);
    }
  } अन्यथा {
    अगर (atomic_decr32(&heap->master_heap->child_गिनती) == 0) {
      _rpmalloc_heap_unmap(heap->master_heap);
    }
  }
}

अटल व्योम _rpmalloc_heap_global_finalize(heap_t *heap) {
  अगर (heap->finalize++ > 1) {
    --heap->finalize;
    वापस;
  }

  _rpmalloc_heap_finalize(heap);

#अगर ENABLE_THREAD_CACHE
  क्रम (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    span_cache_t *span_cache;
    अगर (!iclass)
      span_cache = &heap->span_cache;
    अन्यथा
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    क्रम (माप_प्रकार ispan = 0; ispan < span_cache->गिनती; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->गिनती = 0;
  }
#पूर्ण_अगर

  अगर (heap->full_span_गिनती) {
    --heap->finalize;
    वापस;
  }

  क्रम (माप_प्रकार iclass = 0; iclass < आकार_CLASS_गिनती; ++iclass) {
    अगर (heap->आकार_class[iclass].free_list ||
        heap->आकार_class[iclass].partial_span) {
      --heap->finalize;
      वापस;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  माप_प्रकार list_idx = (माप_प्रकार)heap->id % HEAP_ARRAY_आकार;
  heap_t *list_heap = _memory_heaps[list_idx];
  अगर (list_heap == heap) {
    _memory_heaps[list_idx] = heap->अग्र_heap;
  } अन्यथा {
    जबतक (list_heap->अग्र_heap != heap)
      list_heap = list_heap->अग्र_heap;
    list_heap->अग्र_heap = heap->अग्र_heap;
  }

  _rpmalloc_heap_unmap(heap);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
अटल व्योम rpmalloc_heap_cache_जोड़ो(heap_t *heap, span_t *span) {
  अगर (UNEXPECTED(heap->finalize != 0)) {
    _rpmalloc_span_unmap(span);
    _rpmalloc_heap_global_finalize(heap);
    वापस;
  }
#अगर ENABLE_THREAD_CACHE
  माप_प्रकार span_गिनती = span->span_गिनती;
  _rpmalloc_stat_inc(&heap->span_use[span_गिनती - 1].spans_to_cache);
  अगर (span_गिनती == 1) {
    span_cache_t *span_cache = &heap->span_cache;
    span_cache->span[span_cache->गिनती++] = span;
    अगर (span_cache->गिनती == MAX_THREAD_SPAN_CACHE) {
      स्थिर माप_प्रकार remain_गिनती =
          MAX_THREAD_SPAN_CACHE - THREAD_SPAN_CACHE_TRANSFER;
#अगर ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           THREAD_SPAN_CACHE_TRANSFER * memory_span_आकार);
      _rpmalloc_stat_add(&heap->span_use[span_गिनती - 1].spans_to_global,
                         THREAD_SPAN_CACHE_TRANSFER);
      rpmalloc_global_cache_जोड़ो_spans(span_cache->span + remain_गिनती,
                                          span_गिनती,
                                          THREAD_SPAN_CACHE_TRANSFER);
#अन्यथा
      क्रम (माप_प्रकार ispan = 0; ispan < THREAD_SPAN_CACHE_TRANSFER; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_गिनती + ispan]);
#पूर्ण_अगर
      span_cache->गिनती = remain_गिनती;
    }
  } अन्यथा {
    माप_प्रकार cache_idx = span_गिनती - 2;
    span_large_cache_t *span_cache = heap->span_large_cache + cache_idx;
    span_cache->span[span_cache->गिनती++] = span;
    स्थिर माप_प्रकार cache_limit =
        (MAX_THREAD_SPAN_LARGE_CACHE - (span_गिनती >> 1));
    अगर (span_cache->गिनती == cache_limit) {
      स्थिर माप_प्रकार transfer_limit = 2 + (cache_limit >> 2);
      स्थिर माप_प्रकार transfer_गिनती =
          (THREAD_SPAN_LARGE_CACHE_TRANSFER <= transfer_limit
               ? THREAD_SPAN_LARGE_CACHE_TRANSFER
               : transfer_limit);
      स्थिर माप_प्रकार remain_गिनती = cache_limit - transfer_गिनती;
#अगर ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_to_global,
                           transfer_गिनती * span_गिनती * memory_span_आकार);
      _rpmalloc_stat_add(&heap->span_use[span_गिनती - 1].spans_to_global,
                         transfer_गिनती);
      rpmalloc_global_cache_जोड़ो_spans(span_cache->span + remain_गिनती,
                                          span_गिनती, transfer_गिनती);
#अन्यथा
      क्रम (माप_प्रकार ispan = 0; ispan < transfer_गिनती; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[remain_गिनती + ispan]);
#पूर्ण_अगर
      span_cache->गिनती = remain_गिनती;
    }
  }
#अन्यथा
  (व्योम)माप(heap);
  _rpmalloc_span_unmap(span);
#पूर्ण_अगर
}

//! Extract the given number of spans from the different cache levels
अटल span_t *_rpmalloc_heap_thread_cache_extract(heap_t *heap,
                                                   माप_प्रकार span_गिनती) {
  span_t *span = 0;
#अगर ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  अगर (span_गिनती == 1)
    span_cache = &heap->span_cache;
  अन्यथा
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_गिनती - 2));
  अगर (span_cache->गिनती) {
    _rpmalloc_stat_inc(&heap->span_use[span_गिनती - 1].spans_from_cache);
    वापस span_cache->span[--span_cache->गिनती];
  }
#पूर्ण_अगर
  वापस span;
}

अटल span_t *_rpmalloc_heap_thread_cache_deferred_extract(heap_t *heap,
                                                            माप_प्रकार span_गिनती) {
  span_t *span = 0;
  अगर (span_गिनती == 1) {
    _rpmalloc_heap_cache_adopt_deferred(heap, &span);
  } अन्यथा {
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
    span = _rpmalloc_heap_thread_cache_extract(heap, span_गिनती);
  }
  वापस span;
}

अटल span_t *_rpmalloc_heap_reserved_extract(heap_t *heap,
                                               माप_प्रकार span_गिनती) {
  अगर (heap->spans_reserved >= span_गिनती)
    वापस _rpmalloc_span_map(heap, span_गिनती);
  वापस 0;
}

//! Extract a span from the global cache
अटल span_t *_rpmalloc_heap_global_cache_extract(heap_t *heap,
                                                   माप_प्रकार span_गिनती) {
#अगर ENABLE_GLOBAL_CACHE
#अगर ENABLE_THREAD_CACHE
  span_cache_t *span_cache;
  माप_प्रकार wanted_गिनती;
  अगर (span_गिनती == 1) {
    span_cache = &heap->span_cache;
    wanted_गिनती = THREAD_SPAN_CACHE_TRANSFER;
  } अन्यथा {
    span_cache = (span_cache_t *)(heap->span_large_cache + (span_गिनती - 2));
    wanted_गिनती = THREAD_SPAN_LARGE_CACHE_TRANSFER;
  }
  span_cache->गिनती = _rpmalloc_global_cache_extract_spans(
      span_cache->span, span_गिनती, wanted_गिनती);
  अगर (span_cache->गिनती) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_गिनती * span_cache->गिनती * memory_span_आकार);
    _rpmalloc_stat_add(&heap->span_use[span_गिनती - 1].spans_from_global,
                       span_cache->गिनती);
    वापस span_cache->span[--span_cache->गिनती];
  }
#अन्यथा
  span_t *span = 0;
  माप_प्रकार गिनती = _rpmalloc_global_cache_extract_spans(&span, span_गिनती, 1);
  अगर (गिनती) {
    _rpmalloc_stat_add64(&heap->global_to_thread,
                         span_गिनती * गिनती * memory_span_आकार);
    _rpmalloc_stat_add(&heap->span_use[span_गिनती - 1].spans_from_global,
                       गिनती);
    वापस span;
  }
#पूर्ण_अगर
#पूर्ण_अगर
  (व्योम)माप(heap);
  (व्योम)माप(span_गिनती);
  वापस 0;
}

अटल व्योम _rpmalloc_inc_span_statistics(heap_t *heap, माप_प्रकार span_गिनती,
                                          uint32_t class_idx) {
  (व्योम)माप(heap);
  (व्योम)माप(span_गिनती);
  (व्योम)माप(class_idx);
#अगर ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)span_गिनती - 1;
  uint32_t current_गिनती =
      (uint32_t)atomic_incr32(&heap->span_use[idx].current);
  अगर (current_गिनती > (uint32_t)atomic_load32(&heap->span_use[idx].ऊपरी))
    atomic_store32(&heap->span_use[idx].ऊपरी, (int32_t)current_गिनती);
  _rpmalloc_stat_add_peak(&heap->आकार_class_use[class_idx].spans_current, 1,
                          heap->आकार_class_use[class_idx].spans_peak);
#पूर्ण_अगर
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
अटल span_t *
_rpmalloc_heap_extract_new_span(heap_t *heap,
                                heap_आकार_class_t *heap_आकार_class,
                                माप_प्रकार span_गिनती, uint32_t class_idx) {
  span_t *span;
#अगर ENABLE_THREAD_CACHE
  अगर (heap_आकार_class && heap_आकार_class->cache) {
    span = heap_आकार_class->cache;
    heap_आकार_class->cache =
        (heap->span_cache.गिनती
             ? heap->span_cache.span[--heap->span_cache.गिनती]
             : 0);
    _rpmalloc_inc_span_statistics(heap, span_गिनती, class_idx);
    वापस span;
  }
#पूर्ण_अगर
  (व्योम)माप(class_idx);
  // Allow 50% overhead to increase cache hits
  माप_प्रकार base_span_गिनती = span_गिनती;
  माप_प्रकार limit_span_गिनती =
      (span_गिनती > 2) ? (span_गिनती + (span_गिनती >> 1)) : span_गिनती;
  अगर (limit_span_गिनती > LARGE_CLASS_गिनती)
    limit_span_गिनती = LARGE_CLASS_गिनती;
  करो {
    span = _rpmalloc_heap_thread_cache_extract(heap, span_गिनती);
    अगर (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->आकार_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_गिनती, class_idx);
      वापस span;
    }
    span = _rpmalloc_heap_thread_cache_deferred_extract(heap, span_गिनती);
    अगर (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->आकार_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_गिनती, class_idx);
      वापस span;
    }
    span = _rpmalloc_heap_global_cache_extract(heap, span_गिनती);
    अगर (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->आकार_class_use[class_idx].spans_from_cache);
      _rpmalloc_inc_span_statistics(heap, span_गिनती, class_idx);
      वापस span;
    }
    span = _rpmalloc_heap_reserved_extract(heap, span_गिनती);
    अगर (EXPECTED(span != 0)) {
      _rpmalloc_stat_inc(&heap->आकार_class_use[class_idx].spans_from_reserved);
      _rpmalloc_inc_span_statistics(heap, span_गिनती, class_idx);
      वापस span;
    }
    ++span_गिनती;
  } जबतक (span_गिनती <= limit_span_गिनती);
  // Final fallback, map in more virtual memory
  span = _rpmalloc_span_map(heap, base_span_गिनती);
  _rpmalloc_inc_span_statistics(heap, base_span_गिनती, class_idx);
  _rpmalloc_stat_inc(&heap->आकार_class_use[class_idx].spans_map_calls);
  वापस span;
}

अटल व्योम _rpmalloc_heap_initialize(heap_t *heap) {
  _rpmalloc_memset_const(heap, 0, माप(heap_t));
  // Get a new heap ID
  heap->id = 1 + atomic_incr32(&_memory_heap_id);

  // Link in heap in heap ID map
  माप_प्रकार list_idx = (माप_प्रकार)heap->id % HEAP_ARRAY_आकार;
  heap->अग्र_heap = _memory_heaps[list_idx];
  _memory_heaps[list_idx] = heap;
}

अटल व्योम _rpmalloc_heap_orphan(heap_t *heap, पूर्णांक first_class) {
  heap->owner_thread = (uintptr_t)-1;
#अगर RPMALLOC_FIRST_CLASS_HEAPS
  heap_t **heap_list =
      (first_class ? &_memory_first_class_orphan_heaps : &_memory_orphan_heaps);
#अन्यथा
  (व्योम)माप(first_class);
  heap_t **heap_list = &_memory_orphan_heaps;
#पूर्ण_अगर
  heap->अग्र_orphan = *heap_list;
  *heap_list = heap;
}

//! Allocate a new heap from newly mapped memory pages
अटल heap_t *_rpmalloc_heap_allocate_new(व्योम) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  माप_प्रकार heap_आकार = माप(heap_t);
  माप_प्रकार aligned_heap_आकार = 16 * ((heap_आकार + 15) / 16);
  माप_प्रकार request_heap_गिनती = 16;
  माप_प्रकार heap_span_गिनती = ((aligned_heap_आकार * request_heap_गिनती) +
                            माप(span_t) + memory_span_आकार - 1) /
                           memory_span_आकार;
  माप_प्रकार block_आकार = memory_span_आकार * heap_span_गिनती;
  माप_प्रकार span_गिनती = heap_span_गिनती;
  span_t *span = 0;
  // If there are global reserved spans, use these first
  अगर (memory_global_reserve_गिनती >= heap_span_गिनती) {
    span = _rpmalloc_global_get_reserved_spans(heap_span_गिनती);
  }
  अगर (!span) {
    अगर (memory_page_आकार > block_आकार) {
      span_गिनती = memory_page_आकार / memory_span_आकार;
      block_आकार = memory_page_आकार;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      माप_प्रकार possible_heap_गिनती =
          (block_आकार - माप(span_t)) / aligned_heap_आकार;
      अगर (possible_heap_गिनती >= (request_heap_गिनती * 16))
        request_heap_गिनती *= 16;
      अन्यथा अगर (possible_heap_गिनती < request_heap_गिनती)
        request_heap_गिनती = possible_heap_गिनती;
      heap_span_गिनती = ((aligned_heap_आकार * request_heap_गिनती) +
                         माप(span_t) + memory_span_आकार - 1) /
                        memory_span_आकार;
    }

    माप_प्रकार align_offset = 0;
    span = (span_t *)_rpmalloc_mmap(block_आकार, &align_offset);
    अगर (!span)
      वापस 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    _rpmalloc_span_initialize(span, span_गिनती, heap_span_गिनती, align_offset);
  }

  माप_प्रकार remain_आकार = memory_span_आकार - माप(span_t);
  heap_t *heap = (heap_t *)pointer_offset(span, माप(span_t));
  _rpmalloc_heap_initialize(heap);

  // Put extra heaps as orphans
  माप_प्रकार अंक_heaps = remain_आकार / aligned_heap_आकार;
  अगर (अंक_heaps < request_heap_गिनती)
    अंक_heaps = request_heap_गिनती;
  atomic_store32(&heap->child_गिनती, (int32_t)अंक_heaps - 1);
  heap_t *extra_heap = (heap_t *)pointer_offset(heap, aligned_heap_आकार);
  जबतक (अंक_heaps > 1) {
    _rpmalloc_heap_initialize(extra_heap);
    extra_heap->master_heap = heap;
    _rpmalloc_heap_orphan(extra_heap, 1);
    extra_heap = (heap_t *)pointer_offset(extra_heap, aligned_heap_आकार);
    --अंक_heaps;
  }

  अगर (span_गिनती > heap_span_गिनती) {
    // Cap reserved spans
    माप_प्रकार remain_गिनती = span_गिनती - heap_span_गिनती;
    माप_प्रकार reserve_गिनती =
        (remain_गिनती > memory_heap_reserve_गिनती ? memory_heap_reserve_गिनती
                                                   : remain_गिनती);
    span_t *remain_span =
        (span_t *)pointer_offset(span, heap_span_गिनती * memory_span_आकार);
    _rpmalloc_heap_set_reserved_spans(heap, span, remain_span, reserve_गिनती);

    अगर (remain_गिनती > reserve_गिनती) {
      // Set to global reserved spans
      remain_span = (span_t *)pointer_offset(remain_span,
                                             reserve_गिनती * memory_span_आकार);
      reserve_गिनती = remain_गिनती - reserve_गिनती;
      _rpmalloc_global_set_reserved_spans(span, remain_span, reserve_गिनती);
    }
  }

  वापस heap;
}

अटल heap_t *_rpmalloc_heap_extract_orphan(heap_t **heap_list) {
  heap_t *heap = *heap_list;
  *heap_list = (heap ? heap->अग्र_orphan : 0);
  वापस heap;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
अटल heap_t *_rpmalloc_heap_allocate(पूर्णांक first_class) {
  heap_t *heap = 0;
  जबतक (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
    _rpmalloc_spin();
  अगर (first_class == 0)
    heap = _rpmalloc_heap_extract_orphan(&_memory_orphan_heaps);
#अगर RPMALLOC_FIRST_CLASS_HEAPS
  अगर (!heap)
    heap = _rpmalloc_heap_extract_orphan(&_memory_first_class_orphan_heaps);
#पूर्ण_अगर
  अगर (!heap)
    heap = _rpmalloc_heap_allocate_new();
  atomic_store32_release(&_memory_global_lock, 0);
  अगर (heap)
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  वापस heap;
}

अटल व्योम _rpmalloc_heap_release(व्योम *heapptr, पूर्णांक first_class,
                                   पूर्णांक release_cache) {
  heap_t *heap = (heap_t *)heapptr;
  अगर (!heap)
    वापस;
  // Release thread cache spans back to global cache
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  अगर (release_cache || heap->finalize) {
#अगर ENABLE_THREAD_CACHE
    क्रम (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
      span_cache_t *span_cache;
      अगर (!iclass)
        span_cache = &heap->span_cache;
      अन्यथा
        span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
      अगर (!span_cache->गिनती)
        जारी;
#अगर ENABLE_GLOBAL_CACHE
      अगर (heap->finalize) {
        क्रम (माप_प्रकार ispan = 0; ispan < span_cache->गिनती; ++ispan)
          _rpmalloc_span_unmap(span_cache->span[ispan]);
      } अन्यथा {
        _rpmalloc_stat_add64(&heap->thread_to_global, span_cache->गिनती *
                                                          (iclass + 1) *
                                                          memory_span_आकार);
        _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                           span_cache->गिनती);
        rpmalloc_global_cache_जोड़ो_spans(span_cache->span, iclass + 1,
                                            span_cache->गिनती);
      }
#अन्यथा
      क्रम (माप_प्रकार ispan = 0; ispan < span_cache->गिनती; ++ispan)
        _rpmalloc_span_unmap(span_cache->span[ispan]);
#पूर्ण_अगर
      span_cache->गिनती = 0;
    }
#पूर्ण_अगर
  }

  अगर (get_thread_heap_raw() == heap)
    set_thread_heap(0);

#अगर ENABLE_STATISTICS
  atomic_decr32(&_memory_active_heaps);
  rpmalloc_assert(atomic_load32(&_memory_active_heaps) >= 0,
                  "Still active heaps during finalization");
#पूर्ण_अगर

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  अगर (get_thread_id() != _rpmalloc_main_thread_id) {
    जबतक (!atomic_cas32_acquire(&_memory_global_lock, 1, 0))
      _rpmalloc_spin();
  }
  _rpmalloc_heap_orphan(heap, first_class);
  atomic_store32_release(&_memory_global_lock, 0);
}

अटल व्योम _rpmalloc_heap_release_raw(व्योम *heapptr, पूर्णांक release_cache) {
  _rpmalloc_heap_release(heapptr, 0, release_cache);
}

अटल व्योम _rpmalloc_heap_release_raw_fc(व्योम *heapptr) {
  _rpmalloc_heap_release_raw(heapptr, 1);
}

अटल व्योम _rpmalloc_heap_finalize(heap_t *heap) {
  अगर (heap->spans_reserved) {
    span_t *span = _rpmalloc_span_map(heap, heap->spans_reserved);
    _rpmalloc_span_unmap(span);
    heap->spans_reserved = 0;
  }

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  क्रम (माप_प्रकार iclass = 0; iclass < आकार_CLASS_गिनती; ++iclass) {
    अगर (heap->आकार_class[iclass].cache)
      _rpmalloc_span_unmap(heap->आकार_class[iclass].cache);
    heap->आकार_class[iclass].cache = 0;
    span_t *span = heap->आकार_class[iclass].partial_span;
    जबतक (span) {
      span_t *अग्र = span->अग्र;
      _rpmalloc_span_finalize(heap, iclass, span,
                              &heap->आकार_class[iclass].partial_span);
      span = अग्र;
    }
    // If class still has a free list it must be a full span
    अगर (heap->आकार_class[iclass].free_list) {
      span_t *class_span =
          (span_t *)((uintptr_t)heap->आकार_class[iclass].free_list &
                     _memory_span_mask);
      span_t **list = 0;
#अगर RPMALLOC_FIRST_CLASS_HEAPS
      list = &heap->full_span[iclass];
#पूर्ण_अगर
      --heap->full_span_गिनती;
      अगर (!_rpmalloc_span_finalize(heap, iclass, class_span, list)) {
        अगर (list)
          _rpmalloc_span_double_link_list_remove(list, class_span);
        _rpmalloc_span_double_link_list_add(
            &heap->आकार_class[iclass].partial_span, class_span);
      }
    }
  }

#अगर ENABLE_THREAD_CACHE
  क्रम (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    span_cache_t *span_cache;
    अगर (!iclass)
      span_cache = &heap->span_cache;
    अन्यथा
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    क्रम (माप_प्रकार ispan = 0; ispan < span_cache->गिनती; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
    span_cache->गिनती = 0;
  }
#पूर्ण_अगर
  rpmalloc_assert(!atomic_load_ptr(&heap->span_free_deferred),
                  "Heaps still active during finalization");
}

////////////
///
/// Allocation entry points
///
//////

//! Pop first block from a free list
अटल व्योम *free_list_निकालो(व्योम **list) {
  व्योम *block = *list;
  *list = *((व्योम **)block);
  वापस block;
}

//! Allocate a small/medium sized memory block from the given heap
अटल व्योम *_rpmalloc_allocate_from_heap_fallback(
    heap_t *heap, heap_आकार_class_t *heap_आकार_class, uint32_t class_idx) {
  span_t *span = heap_आकार_class->partial_span;
  rpmalloc_assume(heap != 0);
  अगर (EXPECTED(span != 0)) {
    rpmalloc_assert(span->block_गिनती ==
                        memory_आकार_class[span->आकार_class].block_गिनती,
                    "Span block count corrupted");
    rpmalloc_assert(!_rpmalloc_span_is_fully_utilized(span),
                    "Internal failure");
    व्योम *block;
    अगर (span->free_list) {
      // Span local free list is not empty, swap to size class free list
      block = free_list_निकालो(&span->free_list);
      heap_आकार_class->free_list = span->free_list;
      span->free_list = 0;
    } अन्यथा {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      व्योम *block_start = pointer_offset(
          span, SPAN_HEADER_आकार +
                    ((माप_प्रकार)span->free_list_limit * span->block_आकार));
      span->free_list_limit += free_list_partial_init(
          &heap_आकार_class->free_list, &block,
          (व्योम *)((uintptr_t)block_start & ~(memory_page_आकार - 1)),
          block_start, span->block_गिनती - span->free_list_limit,
          span->block_आकार);
    }
    rpmalloc_assert(span->free_list_limit <= span->block_गिनती,
                    "Span block count corrupted");
    span->used_गिनती = span->free_list_limit;

    // Swap in deferred free list if present
    अगर (atomic_load_ptr(&span->free_list_deferred))
      _rpmalloc_span_extract_free_list_deferred(span);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    अगर (!_rpmalloc_span_is_fully_utilized(span))
      वापस block;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_span_double_link_list_निकालो_शीर्ष(&heap_आकार_class->partial_span,
                                             span);
#अगर RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_add(&heap->full_span[class_idx], span);
#पूर्ण_अगर
    ++heap->full_span_गिनती;
    वापस block;
  }

  // Find a span in one of the cache levels
  span = _rpmalloc_heap_extract_new_span(heap, heap_आकार_class, 1, class_idx);
  अगर (EXPECTED(span != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    वापस _rpmalloc_span_initialize_new(heap, heap_आकार_class, span,
                                         class_idx);
  }

  वापस 0;
}

//! Allocate a small sized memory block from the given heap
अटल व्योम *_rpmalloc_allocate_small(heap_t *heap, माप_प्रकार आकार_2) {
  rpmalloc_assert(heap, "No thread heap");
  // Small sizes have unique size classes
  स्थिर uint32_t class_idx =
      (uint32_t)((आकार_2 + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_SHIFT);
  heap_आकार_class_t *heap_आकार_class = heap->आकार_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  अगर (EXPECTED(heap_आकार_class->free_list != 0))
    वापस free_list_निकालो(&heap_आकार_class->free_list);
  वापस _rpmalloc_allocate_from_heap_fallback(heap, heap_आकार_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
अटल व्योम *_rpmalloc_allocate_medium(heap_t *heap, माप_प्रकार आकार_2) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  स्थिर uint32_t base_idx =
      (uint32_t)(SMALL_CLASS_गिनती +
                 ((आकार_2 - (SMALL_आकार_LIMIT + 1)) >> MEDIUM_GRANULARITY_SHIFT));
  स्थिर uint32_t class_idx = memory_आकार_class[base_idx].class_idx;
  heap_आकार_class_t *heap_आकार_class = heap->आकार_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  अगर (EXPECTED(heap_आकार_class->free_list != 0))
    वापस free_list_निकालो(&heap_आकार_class->free_list);
  वापस _rpmalloc_allocate_from_heap_fallback(heap, heap_आकार_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
अटल व्योम *_rpmalloc_allocate_large(heap_t *heap, माप_प्रकार आकार_2) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  आकार_2 += SPAN_HEADER_आकार;
  माप_प्रकार span_गिनती = आकार_2 >> memory_span_आकार_shift;
  अगर (आकार_2 & (memory_span_आकार - 1))
    ++span_गिनती;

  // Find a span in one of the cache levels
  span_t *span =
      _rpmalloc_heap_extract_new_span(heap, 0, span_गिनती, आकार_CLASS_LARGE);
  अगर (!span)
    वापस span;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(span->span_गिनती >= span_गिनती, "Internal failure");
  span->आकार_class = आकार_CLASS_LARGE;
  span->heap = heap;

#अगर RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#पूर्ण_अगर
  ++heap->full_span_गिनती;

  वापस pointer_offset(span, SPAN_HEADER_आकार);
}

//! Allocate a huge block by mapping memory pages directly
अटल व्योम *_rpmalloc_allocate_huge(heap_t *heap, माप_प्रकार आकार_2) {
  rpmalloc_assert(heap, "No thread heap");
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  आकार_2 += SPAN_HEADER_आकार;
  माप_प्रकार अंक_pages = आकार_2 >> memory_page_आकार_shift;
  अगर (आकार_2 & (memory_page_आकार - 1))
    ++अंक_pages;
  माप_प्रकार align_offset = 0;
  span_t *span =
      (span_t *)_rpmalloc_mmap(अंक_pages * memory_page_आकार, &align_offset);
  अगर (!span)
    वापस span;

  // Store page count in span_count
  span->आकार_class = आकार_CLASS_HUGE;
  span->span_गिनती = (uint32_t)अंक_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, अंक_pages, _huge_pages_peak);

#अगर RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#पूर्ण_अगर
  ++heap->full_span_गिनती;

  वापस pointer_offset(span, SPAN_HEADER_आकार);
}

//! Allocate a block of the given size
अटल व्योम *_rpmalloc_allocate(heap_t *heap, माप_प्रकार आकार_2) {
  _rpmalloc_stat_add64(&allocation_गणक, 1);
  अगर (EXPECTED(आकार_2 <= SMALL_आकार_LIMIT))
    वापस _rpmalloc_allocate_small(heap, आकार_2);
  अन्यथा अगर (आकार_2 <= memory_medium_आकार_limit)
    वापस _rpmalloc_allocate_medium(heap, आकार_2);
  अन्यथा अगर (आकार_2 <= LARGE_आकार_LIMIT)
    वापस _rpmalloc_allocate_large(heap, आकार_2);
  वापस _rpmalloc_allocate_huge(heap, आकार_2);
}

अटल व्योम *_rpmalloc_aligned_allocate(heap_t *heap, माप_प्रकार alignment,
                                        माप_प्रकार आकार_2) {
  अगर (alignment <= SMALL_GRANULARITY)
    वापस _rpmalloc_allocate(heap, आकार_2);

#अगर ENABLE_VALIDATE_ARGS
  अगर ((आकार_2 + alignment) < आकार_2) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
  अगर (alignment & (alignment - 1)) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#पूर्ण_अगर

  अगर ((alignment <= SPAN_HEADER_आकार) &&
      ((आकार_2 + SPAN_HEADER_आकार) < memory_medium_आकार_limit)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    माप_प्रकार multiple_आकार = आकार_2 ? (आकार_2 + (SPAN_HEADER_आकार - 1)) &
                                      ~(uintptr_t)(SPAN_HEADER_आकार - 1)
                                : SPAN_HEADER_आकार;
    rpmalloc_assert(!(multiple_आकार % SPAN_HEADER_आकार),
                    "Failed alignment calculation");
    अगर (multiple_आकार <= (आकार_2 + alignment))
      वापस _rpmalloc_allocate(heap, multiple_आकार);
  }

  व्योम *ptr = 0;
  माप_प्रकार align_mask = alignment - 1;
  अगर (alignment <= memory_page_आकार) {
    ptr = _rpmalloc_allocate(heap, आकार_2 + alignment);
    अगर ((uintptr_t)ptr & align_mask) {
      ptr = (व्योम *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);
      // Mark as having aligned blocks
      span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
      span->flags |= SPAN_FLAG_ALIGNED_BLOCKS;
    }
    वापस ptr;
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
  अगर (alignment & align_mask) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
  अगर (alignment >= memory_span_आकार) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }

  माप_प्रकार extra_pages = alignment / memory_page_आकार;

  // Since each span has a header, we will at least need one extra memory page
  माप_प्रकार अंक_pages = 1 + (आकार_2 / memory_page_आकार);
  अगर (आकार_2 & (memory_page_आकार - 1))
    ++अंक_pages;

  अगर (extra_pages > अंक_pages)
    अंक_pages = 1 + extra_pages;

  माप_प्रकार original_pages = अंक_pages;
  माप_प्रकार limit_pages = (memory_span_आकार / memory_page_आकार) * 2;
  अगर (limit_pages < (original_pages * 2))
    limit_pages = original_pages * 2;

  माप_प्रकार mapped_आकार, align_offset;
  span_t *span;

retry:
  align_offset = 0;
  mapped_आकार = अंक_pages * memory_page_आकार;

  span = (span_t *)_rpmalloc_mmap(mapped_आकार, &align_offset);
  अगर (!span) {
    त्रुटि_सं = ENOMEM;
    वापस 0;
  }
  ptr = pointer_offset(span, SPAN_HEADER_आकार);

  अगर ((uintptr_t)ptr & align_mask)
    ptr = (व्योम *)(((uintptr_t)ptr & ~(uintptr_t)align_mask) + alignment);

  अगर (((माप_प्रकार)pointer_diff(ptr, span) >= memory_span_आकार) ||
      (pointer_offset(ptr, आकार_2) > pointer_offset(span, mapped_आकार)) ||
      (((uintptr_t)ptr & _memory_span_mask) != (uintptr_t)span)) {
    _rpmalloc_unmap(span, mapped_आकार, align_offset, mapped_आकार);
    ++अंक_pages;
    अगर (अंक_pages > limit_pages) {
      त्रुटि_सं = EINVAL;
      वापस 0;
    }
    जाओ retry;
  }

  // Store page count in span_count
  span->आकार_class = आकार_CLASS_HUGE;
  span->span_गिनती = (uint32_t)अंक_pages;
  span->align_offset = (uint32_t)align_offset;
  span->heap = heap;
  _rpmalloc_stat_add_peak(&_huge_pages_current, अंक_pages, _huge_pages_peak);

#अगर RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_add(&heap->large_huge_span, span);
#पूर्ण_अगर
  ++heap->full_span_गिनती;

  _rpmalloc_stat_add64(&allocation_गणक, 1);

  वापस ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
अटल व्योम _rpmalloc_deallocate_direct_small_or_medium(span_t *span,
                                                        व्योम *block) {
  heap_t *heap = span->heap;
  rpmalloc_assert(heap->owner_thread == get_thread_id() ||
                      !heap->owner_thread || heap->finalize,
                  "Internal failure");
  // Add block to free list
  अगर (UNEXPECTED(_rpmalloc_span_is_fully_utilized(span))) {
    span->used_गिनती = span->block_गिनती;
#अगर RPMALLOC_FIRST_CLASS_HEAPS
    _rpmalloc_span_double_link_list_remove(&heap->full_span[span->आकार_class],
                                           span);
#पूर्ण_अगर
    _rpmalloc_span_double_link_list_add(
        &heap->आकार_class[span->आकार_class].partial_span, span);
    --heap->full_span_गिनती;
  }
  *((व्योम **)block) = span->free_list;
  --span->used_गिनती;
  span->free_list = block;
  अगर (UNEXPECTED(span->used_गिनती == span->list_आकार)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    अगर (span->used_गिनती) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      व्योम *free_list;
      करो {
        free_list = atomic_exchange_ptr_acquire(&span->free_list_deferred,
                                                INVALID_POINTER);
      } जबतक (free_list == INVALID_POINTER);
      atomic_store_ptr_release(&span->free_list_deferred, free_list);
    }
    _rpmalloc_span_double_link_list_remove(
        &heap->आकार_class[span->आकार_class].partial_span, span);
    _rpmalloc_span_release_to_cache(heap, span);
  }
}

अटल व्योम _rpmalloc_deallocate_defer_free_span(heap_t *heap, span_t *span) {
  अगर (span->आकार_class != आकार_CLASS_HUGE)
    _rpmalloc_stat_inc(&heap->span_use[span->span_गिनती - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  करो {
    span->free_list = (व्योम *)atomic_load_ptr(&heap->span_free_deferred);
  } जबतक (!atomic_cas_ptr(&heap->span_free_deferred, span, span->free_list));
}

//! Put the block in the deferred free list of the owning span
अटल व्योम _rpmalloc_deallocate_defer_small_or_medium(span_t *span,
                                                       व्योम *block) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  व्योम *free_list;
  करो {
    free_list =
        atomic_exchange_ptr_acquire(&span->free_list_deferred, INVALID_POINTER);
  } जबतक (free_list == INVALID_POINTER);
  *((व्योम **)block) = free_list;
  uint32_t free_गिनती = ++span->list_आकार;
  पूर्णांक all_deferred_free = (free_गिनती == span->block_गिनती);
  atomic_store_ptr_release(&span->free_list_deferred, block);
  अगर (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
  }
}

अटल व्योम _rpmalloc_deallocate_small_or_medium(span_t *span, व्योम *p) {
  _rpmalloc_stat_inc_free(span->heap, span->आकार_class);
  अगर (span->flags & SPAN_FLAG_ALIGNED_BLOCKS) {
    // Realign pointer to block start
    व्योम *blocks_start = pointer_offset(span, SPAN_HEADER_आकार);
    uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
    p = pointer_offset(p, -(int32_t)(block_offset % span->block_आकार));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#अगर RPMALLOC_FIRST_CLASS_HEAPS
  पूर्णांक defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#अन्यथा
  पूर्णांक defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#पूर्ण_अगर
  अगर (!defer)
    _rpmalloc_deallocate_direct_small_or_medium(span, p);
  अन्यथा
    _rpmalloc_deallocate_defer_small_or_medium(span, p);
}

//! Deallocate the given large memory block to the current heap
अटल व्योम _rpmalloc_deallocate_large(span_t *span) {
  rpmalloc_assert(span->आकार_class == आकार_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(span->flags & SPAN_FLAG_MASTER) ||
                      !(span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((span->flags & SPAN_FLAG_MASTER) ||
                      (span->flags & SPAN_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#अगर RPMALLOC_FIRST_CLASS_HEAPS
  पूर्णांक defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#अन्यथा
  पूर्णांक defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#पूर्ण_अगर
  अगर (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    वापस;
  }
  rpmalloc_assert(span->heap->full_span_गिनती, "Heap span counter corrupted");
  --span->heap->full_span_गिनती;
#अगर RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#पूर्ण_अगर
#अगर ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  माप_प्रकार idx = span->span_गिनती - 1;
  atomic_decr32(&span->heap->span_use[idx].current);
#पूर्ण_अगर
  heap_t *heap = span->heap;
  rpmalloc_assert(heap, "No thread heap");
#अगर ENABLE_THREAD_CACHE
  स्थिर पूर्णांक set_as_reserved =
      ((span->span_गिनती > 1) && (heap->span_cache.गिनती == 0) &&
       !heap->finalize && !heap->spans_reserved);
#अन्यथा
  स्थिर पूर्णांक set_as_reserved =
      ((span->span_गिनती > 1) && !heap->finalize && !heap->spans_reserved);
#पूर्ण_अगर
  अगर (set_as_reserved) {
    heap->span_reserve = span;
    heap->spans_reserved = span->span_गिनती;
    अगर (span->flags & SPAN_FLAG_MASTER) {
      heap->span_reserve_master = span;
    } अन्यथा { // SPAN_FLAG_SUBSPAN
      span_t *master = (span_t *)pointer_offset(
          span,
          -(intptr_t)((माप_प्रकार)span->offset_from_master * memory_span_आकार));
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
अटल व्योम _rpmalloc_deallocate_huge(span_t *span) {
  rpmalloc_assert(span->heap, "No span heap");
#अगर RPMALLOC_FIRST_CLASS_HEAPS
  पूर्णांक defer =
      (span->heap->owner_thread &&
       (span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#अन्यथा
  पूर्णांक defer =
      ((span->heap->owner_thread != get_thread_id()) && !span->heap->finalize);
#पूर्ण_अगर
  अगर (defer) {
    _rpmalloc_deallocate_defer_free_span(span->heap, span);
    वापस;
  }
  rpmalloc_assert(span->heap->full_span_गिनती, "Heap span counter corrupted");
  --span->heap->full_span_गिनती;
#अगर RPMALLOC_FIRST_CLASS_HEAPS
  _rpmalloc_span_double_link_list_remove(&span->heap->large_huge_span, span);
#पूर्ण_अगर

  // Oversized allocation, page count is stored in span_count
  माप_प्रकार अंक_pages = span->span_गिनती;
  _rpmalloc_unmap(span, अंक_pages * memory_page_आकार, span->align_offset,
                  अंक_pages * memory_page_आकार);
  _rpmalloc_stat_sub(&_huge_pages_current, अंक_pages);
}

//! Deallocate the given block
अटल व्योम _rpmalloc_deallocate(व्योम *p) {
  _rpmalloc_stat_add64(&deallocation_गणक, 1);
  // Grab the span (always at start of span, using span alignment)
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  अगर (UNEXPECTED(!span))
    वापस;
  अगर (EXPECTED(span->आकार_class < आकार_CLASS_गिनती))
    _rpmalloc_deallocate_small_or_medium(span, p);
  अन्यथा अगर (span->आकार_class == आकार_CLASS_LARGE)
    _rpmalloc_deallocate_large(span);
  अन्यथा
    _rpmalloc_deallocate_huge(span);
}

////////////
///
/// Reallocation entry points
///
//////

अटल माप_प्रकार rpmalloc_usable_आकार_2(व्योम *p);

//! Reallocate the given block to the given size
अटल व्योम *_rpmalloc_reallocate(heap_t *heap, व्योम *p, माप_प्रकार आकार_2,
                                  माप_प्रकार oldsize, अचिन्हित पूर्णांक flags) {
  अगर (p) {
    // Grab the span using guaranteed span alignment
    span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
    अगर (EXPECTED(span->आकार_class < आकार_CLASS_गिनती)) {
      // Small/medium sized block
      rpmalloc_assert(span->span_गिनती == 1, "Span counter corrupted");
      व्योम *blocks_start = pointer_offset(span, SPAN_HEADER_आकार);
      uint32_t block_offset = (uint32_t)pointer_diff(p, blocks_start);
      uint32_t block_idx = block_offset / span->block_आकार;
      व्योम *block =
          pointer_offset(blocks_start, (माप_प्रकार)block_idx * span->block_आकार);
      अगर (!oldsize)
        oldsize =
            (माप_प्रकार)((सूचक_भेद_प्रकार)span->block_आकार - pointer_diff(p, block));
      अगर ((माप_प्रकार)span->block_आकार >= आकार_2) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        अगर ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          स_हटाओ(block, p, oldsize);
        वापस block;
      }
    } अन्यथा अगर (span->आकार_class == आकार_CLASS_LARGE) {
      // Large block
      माप_प्रकार योग_आकार_2 = आकार_2 + SPAN_HEADER_आकार;
      माप_प्रकार अंक_spans = योग_आकार_2 >> memory_span_आकार_shift;
      अगर (योग_आकार_2 & (_memory_span_mask - 1))
        ++अंक_spans;
      माप_प्रकार current_spans = span->span_गिनती;
      व्योम *block = pointer_offset(span, SPAN_HEADER_आकार);
      अगर (!oldsize)
        oldsize = (current_spans * memory_span_आकार) -
                  (माप_प्रकार)pointer_diff(p, block) - SPAN_HEADER_आकार;
      अगर ((current_spans >= अंक_spans) && (योग_आकार_2 >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        अगर ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          स_हटाओ(block, p, oldsize);
        वापस block;
      }
    } अन्यथा {
      // Oversized block
      माप_प्रकार योग_आकार_2 = आकार_2 + SPAN_HEADER_आकार;
      माप_प्रकार अंक_pages = योग_आकार_2 >> memory_page_आकार_shift;
      अगर (योग_आकार_2 & (memory_page_आकार - 1))
        ++अंक_pages;
      // Page count is stored in span_count
      माप_प्रकार current_pages = span->span_गिनती;
      व्योम *block = pointer_offset(span, SPAN_HEADER_आकार);
      अगर (!oldsize)
        oldsize = (current_pages * memory_page_आकार) -
                  (माप_प्रकार)pointer_diff(p, block) - SPAN_HEADER_आकार;
      अगर ((current_pages >= अंक_pages) && (अंक_pages >= (current_pages / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        अगर ((p != block) && !(flags & RPMALLOC_NO_PRESERVE))
          स_हटाओ(block, p, oldsize);
        वापस block;
      }
    }
  } अन्यथा {
    oldsize = 0;
  }

  अगर (!!(flags & RPMALLOC_GROW_OR_FAIL))
    वापस 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  माप_प्रकार lower_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  माप_प्रकार new_आकार =
      (आकार_2 > lower_bound) ? आकार_2 : ((आकार_2 > oldsize) ? lower_bound : आकार_2);
  व्योम *block = _rpmalloc_allocate(heap, new_आकार);
  अगर (p && block) {
    अगर (!(flags & RPMALLOC_NO_PRESERVE))
      स_नकल(block, p, oldsize < new_आकार ? oldsize : new_आकार);
    _rpmalloc_deallocate(p);
  }

  वापस block;
}

अटल व्योम *_rpmalloc_aligned_reallocate(heap_t *heap, व्योम *ptr,
                                          माप_प्रकार alignment, माप_प्रकार आकार_2,
                                          माप_प्रकार oldsize, अचिन्हित पूर्णांक flags) {
  अगर (alignment <= SMALL_GRANULARITY)
    वापस _rpmalloc_reallocate(heap, ptr, आकार_2, oldsize, flags);

  पूर्णांक no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  माप_प्रकार usablesize = (ptr ? rpmalloc_usable_आकार_2(ptr) : 0);
  अगर ((usablesize >= आकार_2) && !((uintptr_t)ptr & (alignment - 1))) {
    अगर (no_alloc || (आकार_2 >= (usablesize / 2)))
      वापस ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  व्योम *block =
      (!no_alloc ? _rpmalloc_aligned_allocate(heap, alignment, आकार_2) : 0);
  अगर (EXPECTED(block != 0)) {
    अगर (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      अगर (!oldsize)
        oldsize = usablesize;
      स_नकल(block, ptr, oldsize < आकार_2 ? oldsize : आकार_2);
    }
    _rpmalloc_deallocate(ptr);
  }
  वापस block;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
अटल माप_प्रकार rpmalloc_usable_आकार_2(व्योम *p) {
  // Grab the span using guaranteed span alignment
  span_t *span = (span_t *)((uintptr_t)p & _memory_span_mask);
  अगर (span->आकार_class < आकार_CLASS_गिनती) {
    // Small/medium block
    व्योम *blocks_start = pointer_offset(span, SPAN_HEADER_आकार);
    वापस span->block_आकार -
           ((माप_प्रकार)pointer_diff(p, blocks_start) % span->block_आकार);
  }
  अगर (span->आकार_class == आकार_CLASS_LARGE) {
    // Large block
    माप_प्रकार current_spans = span->span_गिनती;
    वापस (current_spans * memory_span_आकार) - (माप_प्रकार)pointer_diff(p, span);
  }
  // Oversized block, page count is stored in span_count
  माप_प्रकार current_pages = span->span_गिनती;
  वापस (current_pages * memory_page_आकार) - (माप_प्रकार)pointer_diff(p, span);
}

//! Adjust and optimize the size class properties for the given class
अटल व्योम rpmalloc_adjust_आकार_class(माप_प्रकार iclass) {
  माप_प्रकार block_आकार = memory_आकार_class[iclass].block_आकार;
  माप_प्रकार block_गिनती = (memory_span_आकार - SPAN_HEADER_आकार) / block_आकार;

  memory_आकार_class[iclass].block_गिनती = (uint16_t)block_गिनती;
  memory_आकार_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  अगर (iclass >= SMALL_CLASS_गिनती) {
    माप_प्रकार prevclass = iclass;
    जबतक (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      अगर (memory_आकार_class[prevclass].block_गिनती ==
          memory_आकार_class[iclass].block_गिनती)
        _rpmalloc_memcpy_const(memory_आकार_class + prevclass,
                               memory_आकार_class + iclass,
                               माप(memory_आकार_class[iclass]));
      अन्यथा
        अवरोध;
    }
  }
}

//! Initialize the allocator and setup global data
बाह्य अंतरभूत पूर्णांक rpmalloc_initialize(व्योम) {
  अगर (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    वापस 0;
  }
  वापस rpmalloc_initialize_config(0);
}

पूर्णांक rpmalloc_initialize_config(स्थिर rpmalloc_config_t *config) {
  अगर (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    वापस 0;
  }
  _rpmalloc_initialized = 1;

  अगर (config)
    स_नकल(&_memory_config, config, माप(rpmalloc_config_t));
  अन्यथा
    _rpmalloc_memset_const(&_memory_config, 0, माप(rpmalloc_config_t));

  अगर (!_memory_config.memory_map || !_memory_config.memory_unmap) {
    _memory_config.memory_map = _rpmalloc_mmap_os;
    _memory_config.memory_unmap = _rpmalloc_unmap_os;
  }

#अगर PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  स_रखो(&system_info, 0, माप(system_info));
  GetSystemInfo(&system_info);
  _memory_map_granularity = system_info.dwAllocationGranularity;
#अन्यथा
  _memory_map_granularity = (माप_प्रकार)sysconf(_SC_PAGESIZE);
#पूर्ण_अगर

#अगर RPMALLOC_CONFIGURABLE
  memory_page_आकार = _memory_config.page_आकार;
#अन्यथा
  memory_page_आकार = 0;
#पूर्ण_अगर
  _memory_huge_pages = 0;
  अगर (!memory_page_आकार) {
#अगर PLATFORM_WINDOWS
    memory_page_आकार = system_info.dw_Page_आकार;
#अन्यथा
    memory_page_आकार = _memory_map_granularity;
    अगर (_memory_config.enable_huge_pages) {
#अगर defined(__linux__)
      माप_प्रकार huge_page_आकार = 0;
      खाता *meminfo = ख_खोलो("/proc/meminfo", "r");
      अगर (meminfo) {
        अक्षर पंक्ति_3[128];
        जबतक (!huge_page_आकार && ख_माला_लो(पंक्ति_3, माप(पंक्ति_3) - 1, meminfo)) {
          पंक्ति_3[माप(पंक्ति_3) - 1] = 0;
          अगर (म_माला(पंक्ति_3, "Hugepagesize:"))
            huge_page_आकार = (माप_प्रकार)म_से_दीर्घ(पंक्ति_3 + 13, 0, 10) * 1024;
        }
        ख_बंद(meminfo);
      }
      अगर (huge_page_आकार) {
        _memory_huge_pages = 1;
        memory_page_आकार = huge_page_आकार;
        _memory_map_granularity = huge_page_आकार;
      }
#या_अगर defined(__FreeBSD__)
      पूर्णांक rc;
      माप_प्रकार sz = माप(rc);

      अगर (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, शून्य, 0) == 0 &&
          rc == 1) {
        अटल माप_प्रकार defsize = 2 * 1024 * 1024;
        पूर्णांक nsize = 0;
        माप_प्रकार sizes[4] = {0};
        _memory_huge_pages = 1;
        memory_page_आकार = defsize;
        अगर ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          क्रम (माप_प्रकार csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            अगर (defsize < csize) {
              memory_page_आकार = csize;
              अवरोध;
            }
          }
        }
        _memory_map_granularity = memory_page_आकार;
      }
#या_अगर defined(__APPLE__) || defined(__NetBSD__)
      _memory_huge_pages = 1;
      memory_page_आकार = 2 * 1024 * 1024;
      _memory_map_granularity = memory_page_आकार;
#पूर्ण_अगर
    }
#पूर्ण_अगर
  } अन्यथा {
    अगर (_memory_config.enable_huge_pages)
      _memory_huge_pages = 1;
  }

#अगर PLATFORM_WINDOWS
  अगर (_memory_config.enable_huge_pages) {
    HANDLE token = 0;
    माप_प्रकार large_page_minimum = GetLargePageMinimum();
    अगर (large_page_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    अगर (token) {
      LUID luid;
      अगर (Lookup_Privilege_मूल्य(0, SE_LOCK_MEMORY_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        स_रखो(&token_privileges, 0, माप(token_privileges));
        token_privileges.Privilege_गिनती = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        अगर (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          अगर (GetLastError() == ERROR_SUCCESS)
            _memory_huge_pages = 1;
        }
      }
      CloseHandle(token);
    }
    अगर (_memory_huge_pages) {
      अगर (large_page_minimum > memory_page_आकार)
        memory_page_आकार = large_page_minimum;
      अगर (large_page_minimum > _memory_map_granularity)
        _memory_map_granularity = large_page_minimum;
    }
  }
#पूर्ण_अगर

  माप_प्रकार min_span_आकार = 256;
  माप_प्रकार max_page_आकार;
#अगर UINTPTR_MAX > 0xFFFFFFFF
  max_page_आकार = 4096ULL * 1024ULL * 1024ULL;
#अन्यथा
  max_page_आकार = 4 * 1024 * 1024;
#पूर्ण_अगर
  अगर (memory_page_आकार < min_span_आकार)
    memory_page_आकार = min_span_आकार;
  अगर (memory_page_आकार > max_page_आकार)
    memory_page_आकार = max_page_आकार;
  memory_page_आकार_shift = 0;
  माप_प्रकार page_आकार_bit = memory_page_आकार;
  जबतक (page_आकार_bit != 1) {
    ++memory_page_आकार_shift;
    page_आकार_bit >>= 1;
  }
  memory_page_आकार = ((माप_प्रकार)1 << memory_page_आकार_shift);

#अगर RPMALLOC_CONFIGURABLE
  अगर (!_memory_config.span_आकार) {
    memory_span_आकार = memory_default_span_आकार;
    memory_span_आकार_shift = memory_default_span_आकार_shift;
    _memory_span_mask = _memory_default_span_mask;
  } अन्यथा {
    माप_प्रकार span_आकार = _memory_config.span_आकार;
    अगर (span_आकार > (256 * 1024))
      span_आकार = (256 * 1024);
    memory_span_आकार = 4096;
    memory_span_आकार_shift = 12;
    जबतक (memory_span_आकार < span_आकार) {
      memory_span_आकार <<= 1;
      ++memory_span_आकार_shift;
    }
    _memory_span_mask = ~(uintptr_t)(memory_span_आकार - 1);
  }
#पूर्ण_अगर

  memory_span_map_गिनती =
      (_memory_config.span_map_गिनती ? _memory_config.span_map_गिनती
                                     : DEFAULT_SPAN_MAP_गिनती);
  अगर ((memory_span_आकार * memory_span_map_गिनती) < memory_page_आकार)
    memory_span_map_गिनती = (memory_page_आकार / memory_span_आकार);
  अगर ((memory_page_आकार >= memory_span_आकार) &&
      ((memory_span_map_गिनती * memory_span_आकार) % memory_page_आकार))
    memory_span_map_गिनती = (memory_page_आकार / memory_span_आकार);
  memory_heap_reserve_गिनती = (memory_span_map_गिनती > DEFAULT_SPAN_MAP_गिनती)
                                   ? DEFAULT_SPAN_MAP_गिनती
                                   : memory_span_map_गिनती;

  _memory_config.page_आकार = memory_page_आकार;
  _memory_config.span_आकार = memory_span_आकार;
  _memory_config.span_map_गिनती = memory_span_map_गिनती;
  _memory_config.enable_huge_pages = _memory_huge_pages;

#अगर ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  अगर (pthread_कुंजी_create(&_memory_thread_heap, _rpmalloc_heap_release_raw_fc))
    वापस -1;
#पूर्ण_अगर
#अगर defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_कुंजी = FlsAlloc(&_rpmalloc_thread_destructor);
#पूर्ण_अगर

  // Setup all small and medium size classes
  माप_प्रकार iclass = 0;
  memory_आकार_class[iclass].block_आकार = SMALL_GRANULARITY;
  rpmalloc_adjust_आकार_class(iclass);
  क्रम (iclass = 1; iclass < SMALL_CLASS_गिनती; ++iclass) {
    माप_प्रकार आकार_2 = iclass * SMALL_GRANULARITY;
    memory_आकार_class[iclass].block_आकार = (uint32_t)आकार_2;
    rpmalloc_adjust_आकार_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  memory_medium_आकार_limit = (memory_span_आकार - SPAN_HEADER_आकार) >> 1;
  अगर (memory_medium_आकार_limit > MEDIUM_आकार_LIMIT)
    memory_medium_आकार_limit = MEDIUM_आकार_LIMIT;
  क्रम (iclass = 0; iclass < MEDIUM_CLASS_गिनती; ++iclass) {
    माप_प्रकार आकार_2 = SMALL_आकार_LIMIT + ((iclass + 1) * MEDIUM_GRANULARITY);
    अगर (आकार_2 > memory_medium_आकार_limit) {
      memory_medium_आकार_limit =
          SMALL_आकार_LIMIT + (iclass * MEDIUM_GRANULARITY);
      अवरोध;
    }
    memory_आकार_class[SMALL_CLASS_गिनती + iclass].block_आकार = (uint32_t)आकार_2;
    rpmalloc_adjust_आकार_class(SMALL_CLASS_गिनती + iclass);
  }

  _memory_orphan_heaps = 0;
#अगर RPMALLOC_FIRST_CLASS_HEAPS
  _memory_first_class_orphan_heaps = 0;
#पूर्ण_अगर
#अगर ENABLE_STATISTICS
  atomic_store32(&_memory_active_heaps, 0);
  atomic_store32(&_mapped_pages, 0);
  _mapped_pages_peak = 0;
  atomic_store32(&_master_spans, 0);
  atomic_store32(&mapped_योग, 0);
  atomic_store32(&unmapped_योग, 0);
  atomic_store32(&_mapped_pages_os, 0);
  atomic_store32(&_huge_pages_current, 0);
  _huge_pages_peak = 0;
#पूर्ण_अगर
  स_रखो(_memory_heaps, 0, माप(_memory_heaps));
  atomic_store32_release(&_memory_global_lock, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  वापस 0;
}

//! Finalize the allocator
व्योम rpmalloc_finalize(व्योम) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  अगर (_memory_global_reserve) {
    atomic_add32(&_memory_global_reserve_master->remaining_spans,
                 -(int32_t)memory_global_reserve_गिनती);
    _memory_global_reserve_master = 0;
    memory_global_reserve_गिनती = 0;
    _memory_global_reserve = 0;
  }
  atomic_store32_release(&_memory_global_lock, 0);

  // Free all thread caches and fully free spans
  क्रम (माप_प्रकार list_idx = 0; list_idx < HEAP_ARRAY_आकार; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    जबतक (heap) {
      heap_t *अग्र_heap = heap->अग्र_heap;
      heap->finalize = 1;
      _rpmalloc_heap_global_finalize(heap);
      heap = अग्र_heap;
    }
  }

#अगर ENABLE_GLOBAL_CACHE
  // Free global caches
  क्रम (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass)
    _rpmalloc_global_cache_finalize(&_memory_span_cache[iclass]);
#पूर्ण_अगर

#अगर (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_कुंजी_delete(_memory_thread_heap);
#पूर्ण_अगर
#अगर defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_कुंजी);
  fls_कुंजी = 0;
#पूर्ण_अगर
#अगर ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&_mapped_pages) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&_mapped_pages_os) == 0,
                  "Memory leak detected");
#पूर्ण_अगर

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
बाह्य अंतरभूत व्योम rpmalloc_thread_initialize(व्योम) {
  अगर (!get_thread_heap_raw()) {
    heap_t *heap = _rpmalloc_heap_allocate(0);
    अगर (heap) {
      _rpmalloc_stat_inc(&_memory_active_heaps);
      set_thread_heap(heap);
#अगर defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_Set_मूल्य(fls_कुंजी, heap);
#पूर्ण_अगर
    }
  }
}

//! Finalize thread, orphan heap
व्योम rpmalloc_thread_finalize(पूर्णांक release_caches) {
  heap_t *heap = get_thread_heap_raw();
  अगर (heap)
    _rpmalloc_heap_release_raw(heap, release_caches);
  set_thread_heap(0);
#अगर defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_Set_मूल्य(fls_कुंजी, 0);
#पूर्ण_अगर
}

पूर्णांक rpmalloc_is_thread_initialized(व्योम) {
  वापस (get_thread_heap_raw() != 0) ? 1 : 0;
}

स्थिर rpmalloc_config_t *rpmalloc_config(व्योम) { वापस &_memory_config; }

// Extern interface

बाह्य अंतरभूत RPMALLOC_ALLOCATOR व्योम *rpmalloc(माप_प्रकार आकार_2) {
#अगर ENABLE_VALIDATE_ARGS
  अगर (आकार_2 >= MAX_ALLOC_आकार) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#पूर्ण_अगर
  heap_t *heap = get_thread_heap();
  वापस _rpmalloc_allocate(heap, आकार_2);
}

बाह्य अंतरभूत व्योम rpfree(व्योम *ptr) { _rpmalloc_deallocate(ptr); }

बाह्य अंतरभूत RPMALLOC_ALLOCATOR व्योम *rpcalloc(माप_प्रकार अंक_2, माप_प्रकार आकार_2) {
  माप_प्रकार योग_3;
#अगर ENABLE_VALIDATE_ARGS
#अगर PLATFORM_WINDOWS
  पूर्णांक err = आकार_TMult(अंक_2, आकार_2, &योग_3);
  अगर ((err != S_OK) || (योग_3 >= MAX_ALLOC_आकार)) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#अन्यथा
  पूर्णांक err = __builtin_umull_overflow(अंक_2, आकार_2, &योग_3);
  अगर (err || (योग_3 >= MAX_ALLOC_आकार)) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#पूर्ण_अगर
#अन्यथा
  योग_3 = अंक_2 * आकार_2;
#पूर्ण_अगर
  heap_t *heap = get_thread_heap();
  व्योम *block = _rpmalloc_allocate(heap, योग_3);
  अगर (block)
    स_रखो(block, 0, योग_3);
  वापस block;
}

बाह्य अंतरभूत RPMALLOC_ALLOCATOR व्योम *rprealloc(व्योम *ptr, माप_प्रकार आकार_2) {
#अगर ENABLE_VALIDATE_ARGS
  अगर (आकार_2 >= MAX_ALLOC_आकार) {
    त्रुटि_सं = EINVAL;
    वापस ptr;
  }
#पूर्ण_अगर
  heap_t *heap = get_thread_heap();
  वापस _rpmalloc_reallocate(heap, ptr, आकार_2, 0, 0);
}

बाह्य RPMALLOC_ALLOCATOR व्योम *rpaligned_realloc(व्योम *ptr, माप_प्रकार alignment,
                                                  माप_प्रकार आकार_2, माप_प्रकार oldsize,
                                                  अचिन्हित पूर्णांक flags) {
#अगर ENABLE_VALIDATE_ARGS
  अगर ((आकार_2 + alignment < आकार_2) || (alignment > memory_page_आकार)) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#पूर्ण_अगर
  heap_t *heap = get_thread_heap();
  वापस _rpmalloc_aligned_reallocate(heap, ptr, alignment, आकार_2, oldsize,
                                      flags);
}

बाह्य RPMALLOC_ALLOCATOR व्योम *rpaligned_alloc(माप_प्रकार alignment, माप_प्रकार आकार_2) {
  heap_t *heap = get_thread_heap();
  वापस _rpmalloc_aligned_allocate(heap, alignment, आकार_2);
}

बाह्य अंतरभूत RPMALLOC_ALLOCATOR व्योम *
rpaligned_calloc(माप_प्रकार alignment, माप_प्रकार अंक_2, माप_प्रकार आकार_2) {
  माप_प्रकार योग_3;
#अगर ENABLE_VALIDATE_ARGS
#अगर PLATFORM_WINDOWS
  पूर्णांक err = आकार_TMult(अंक_2, आकार_2, &योग_3);
  अगर ((err != S_OK) || (योग_3 >= MAX_ALLOC_आकार)) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#अन्यथा
  पूर्णांक err = __builtin_umull_overflow(अंक_2, आकार_2, &योग_3);
  अगर (err || (योग_3 >= MAX_ALLOC_आकार)) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#पूर्ण_अगर
#अन्यथा
  योग_3 = अंक_2 * आकार_2;
#पूर्ण_अगर
  व्योम *block = rpaligned_alloc(alignment, योग_3);
  अगर (block)
    स_रखो(block, 0, योग_3);
  वापस block;
}

बाह्य अंतरभूत RPMALLOC_ALLOCATOR व्योम *rpmemalign(माप_प्रकार alignment,
                                                  माप_प्रकार आकार_2) {
  वापस rpaligned_alloc(alignment, आकार_2);
}

बाह्य अंतरभूत पूर्णांक rpposix_memalign(व्योम **memptr, माप_प्रकार alignment,
                                   माप_प्रकार आकार_2) {
  अगर (memptr)
    *memptr = rpaligned_alloc(alignment, आकार_2);
  अन्यथा
    वापस EINVAL;
  वापस *memptr ? 0 : ENOMEM;
}

बाह्य अंतरभूत माप_प्रकार rpmalloc_usable_आकार(व्योम *ptr) {
  वापस (ptr ? rpmalloc_usable_आकार_2(ptr) : 0);
}

बाह्य अंतरभूत व्योम rpmalloc_thread_collect(व्योम) {}

व्योम rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  स_रखो(stats, 0, माप(rpmalloc_thread_statistics_t));
  heap_t *heap = get_thread_heap_raw();
  अगर (!heap)
    वापस;

  क्रम (माप_प्रकार iclass = 0; iclass < आकार_CLASS_गिनती; ++iclass) {
    आकार_class_t *आकार_class = memory_आकार_class + iclass;
    span_t *span = heap->आकार_class[iclass].partial_span;
    जबतक (span) {
      माप_प्रकार free_गिनती = span->list_आकार;
      माप_प्रकार block_गिनती = आकार_class->block_गिनती;
      अगर (span->free_list_limit < block_गिनती)
        block_गिनती = span->free_list_limit;
      free_गिनती += (block_गिनती - span->used_गिनती);
      stats->sizecache += free_गिनती * आकार_class->block_आकार;
      span = span->अग्र;
    }
  }

#अगर ENABLE_THREAD_CACHE
  क्रम (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    span_cache_t *span_cache;
    अगर (!iclass)
      span_cache = &heap->span_cache;
    अन्यथा
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    stats->spancache += span_cache->गिनती * (iclass + 1) * memory_span_आकार;
  }
#पूर्ण_अगर

  span_t *deferred = (span_t *)atomic_load_ptr(&heap->span_free_deferred);
  जबतक (deferred) {
    अगर (deferred->आकार_class != आकार_CLASS_HUGE)
      stats->spancache += (माप_प्रकार)deferred->span_गिनती * memory_span_आकार;
    deferred = (span_t *)deferred->free_list;
  }

#अगर ENABLE_STATISTICS
  stats->thread_to_global = (माप_प्रकार)atomic_load64(&heap->thread_to_global);
  stats->global_to_thread = (माप_प्रकार)atomic_load64(&heap->global_to_thread);

  क्रम (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
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
  क्रम (माप_प्रकार iclass = 0; iclass < आकार_CLASS_गिनती; ++iclass) {
    stats->आकार_use[iclass].alloc_current =
        (माप_प्रकार)atomic_load32(&heap->आकार_class_use[iclass].alloc_current);
    stats->आकार_use[iclass].alloc_peak =
        (माप_प्रकार)heap->आकार_class_use[iclass].alloc_peak;
    stats->आकार_use[iclass].alloc_योग =
        (माप_प्रकार)atomic_load32(&heap->आकार_class_use[iclass].alloc_योग);
    stats->आकार_use[iclass].free_योग =
        (माप_प्रकार)atomic_load32(&heap->आकार_class_use[iclass].free_योग);
    stats->आकार_use[iclass].spans_to_cache =
        (माप_प्रकार)atomic_load32(&heap->आकार_class_use[iclass].spans_to_cache);
    stats->आकार_use[iclass].spans_from_cache =
        (माप_प्रकार)atomic_load32(&heap->आकार_class_use[iclass].spans_from_cache);
    stats->आकार_use[iclass].spans_from_reserved = (माप_प्रकार)atomic_load32(
        &heap->आकार_class_use[iclass].spans_from_reserved);
    stats->आकार_use[iclass].map_calls =
        (माप_प्रकार)atomic_load32(&heap->आकार_class_use[iclass].spans_map_calls);
  }
#पूर्ण_अगर
}

व्योम rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  स_रखो(stats, 0, माप(rpmalloc_global_statistics_t));
#अगर ENABLE_STATISTICS
  stats->mapped = (माप_प्रकार)atomic_load32(&_mapped_pages) * memory_page_आकार;
  stats->mapped_peak = (माप_प्रकार)_mapped_pages_peak * memory_page_आकार;
  stats->mapped_योग_2 =
      (माप_प्रकार)atomic_load32(&mapped_योग) * memory_page_आकार;
  stats->unmapped_योग_2 =
      (माप_प्रकार)atomic_load32(&unmapped_योग) * memory_page_आकार;
  stats->huge_alloc =
      (माप_प्रकार)atomic_load32(&_huge_pages_current) * memory_page_आकार;
  stats->huge_alloc_peak = (माप_प्रकार)_huge_pages_peak * memory_page_आकार;
#पूर्ण_अगर
#अगर ENABLE_GLOBAL_CACHE
  क्रम (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    global_cache_t *cache = &_memory_span_cache[iclass];
    जबतक (!atomic_cas32_acquire(&cache->lock, 1, 0))
      _rpmalloc_spin();
    uint32_t गिनती = cache->गिनती;
#अगर ENABLE_UNLIMITED_CACHE
    span_t *current_span = cache->overflow;
    जबतक (current_span) {
      ++गिनती;
      current_span = current_span->अग्र;
    }
#पूर्ण_अगर
    atomic_store32_release(&cache->lock, 0);
    stats->cached += गिनती * (iclass + 1) * memory_span_आकार;
  }
#पूर्ण_अगर
}

#अगर ENABLE_STATISTICS

अटल व्योम _memory_heap_dump_statistics(heap_t *heap, व्योम *file) {
  ख_लिखो(file, "Heap %d stats:\n", heap->id);
  ख_लिखो(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  क्रम (माप_प्रकार iclass = 0; iclass < आकार_CLASS_गिनती; ++iclass) {
    अगर (!atomic_load32(&heap->आकार_class_use[iclass].alloc_योग))
      जारी;
    ख_लिखो(
        file,
        "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu "
        "%9u\n",
        (uint32_t)iclass,
        atomic_load32(&heap->आकार_class_use[iclass].alloc_current),
        heap->आकार_class_use[iclass].alloc_peak,
        atomic_load32(&heap->आकार_class_use[iclass].alloc_योग),
        atomic_load32(&heap->आकार_class_use[iclass].free_योग),
        memory_आकार_class[iclass].block_आकार,
        memory_आकार_class[iclass].block_गिनती,
        atomic_load32(&heap->आकार_class_use[iclass].spans_current),
        heap->आकार_class_use[iclass].spans_peak,
        ((माप_प्रकार)heap->आकार_class_use[iclass].alloc_peak *
         (माप_प्रकार)memory_आकार_class[iclass].block_आकार) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(&heap->आकार_class_use[iclass].spans_to_cache) *
         memory_span_आकार) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(&heap->आकार_class_use[iclass].spans_from_cache) *
         memory_span_आकार) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(
             &heap->आकार_class_use[iclass].spans_from_reserved) *
         memory_span_आकार) /
            (माप_प्रकार)(1024 * 1024),
        atomic_load32(&heap->आकार_class_use[iclass].spans_map_calls));
  }
  ख_लिखो(file, "Spans  Current     Peak Deferred  PeakMiB  Cached  ToCacheMiB "
                "FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB "
                "FromGlobalMiB  MmapCalls\n");
  क्रम (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    अगर (!atomic_load32(&heap->span_use[iclass].ऊपरी) &&
        !atomic_load32(&heap->span_use[iclass].spans_map_calls))
      जारी;
    ख_लिखो(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&heap->span_use[iclass].current),
        atomic_load32(&heap->span_use[iclass].ऊपरी),
        atomic_load32(&heap->span_use[iclass].spans_deferred),
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].ऊपरी) *
         (माप_प्रकार)memory_span_आकार * (iclass + 1)) /
            (माप_प्रकार)(1024 * 1024),
#अगर ENABLE_THREAD_CACHE
        (अचिन्हित पूर्णांक)(!iclass ? heap->span_cache.गिनती
                               : heap->span_large_cache[iclass - 1].गिनती),
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_to_cache) *
         (iclass + 1) * memory_span_आकार) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_from_cache) *
         (iclass + 1) * memory_span_आकार) /
            (माप_प्रकार)(1024 * 1024),
#अन्यथा
        0, (माप_प्रकार)0, (माप_प्रकार)0,
#पूर्ण_अगर
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_to_reserved) *
         (iclass + 1) * memory_span_आकार) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_from_reserved) *
         (iclass + 1) * memory_span_आकार) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_to_global) *
         (माप_प्रकार)memory_span_आकार * (iclass + 1)) /
            (माप_प्रकार)(1024 * 1024),
        ((माप_प्रकार)atomic_load32(&heap->span_use[iclass].spans_from_global) *
         (माप_प्रकार)memory_span_आकार * (iclass + 1)) /
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

#पूर्ण_अगर

व्योम rpmalloc_dump_statistics(व्योम *file) {
#अगर ENABLE_STATISTICS
  क्रम (माप_प्रकार list_idx = 0; list_idx < HEAP_ARRAY_आकार; ++list_idx) {
    heap_t *heap = _memory_heaps[list_idx];
    जबतक (heap) {
      पूर्णांक need_dump = 0;
      क्रम (माप_प्रकार iclass = 0; !need_dump && (iclass < आकार_CLASS_गिनती);
           ++iclass) {
        अगर (!atomic_load32(&heap->आकार_class_use[iclass].alloc_योग)) {
          rpmalloc_assert(
              !atomic_load32(&heap->आकार_class_use[iclass].free_योग),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&heap->आकार_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          जारी;
        }
        need_dump = 1;
      }
      क्रम (माप_प्रकार iclass = 0; !need_dump && (iclass < LARGE_CLASS_गिनती);
           ++iclass) {
        अगर (!atomic_load32(&heap->span_use[iclass].ऊपरी) &&
            !atomic_load32(&heap->span_use[iclass].spans_map_calls))
          जारी;
        need_dump = 1;
      }
      अगर (need_dump)
        _memory_heap_dump_statistics(heap, file);
      heap = heap->अग्र_heap;
    }
  }
  ख_लिखो(file, "Global stats:\n");
  माप_प्रकार huge_current =
      (माप_प्रकार)atomic_load32(&_huge_pages_current) * memory_page_आकार;
  माप_प्रकार huge_peak = (माप_प्रकार)_huge_pages_peak * memory_page_आकार;
  ख_लिखो(file, "HugeCurrentMiB HugePeakMiB\n");
  ख_लिखो(file, "%14zu %11zu\n", huge_current / (माप_प्रकार)(1024 * 1024),
          huge_peak / (माप_प्रकार)(1024 * 1024));

#अगर ENABLE_GLOBAL_CACHE
  ख_लिखो(file, "GlobalCacheMiB\n");
  क्रम (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    global_cache_t *cache = _memory_span_cache + iclass;
    माप_प्रकार global_cache = (माप_प्रकार)cache->गिनती * iclass * memory_span_आकार;

    माप_प्रकार global_overflow_cache = 0;
    span_t *span = cache->overflow;
    जबतक (span) {
      global_overflow_cache += iclass * memory_span_आकार;
      span = span->अग्र;
    }
    अगर (global_cache || global_overflow_cache || cache->जोड़ो_गिनती ||
        cache->extract_गिनती)
      ख_लिखो(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (माप_प्रकार)(1024 * 1024),
              global_overflow_cache / (माप_प्रकार)(1024 * 1024),
              cache->जोड़ो_गिनती, cache->extract_गिनती);
  }
#पूर्ण_अगर

  माप_प्रकार mapped = (माप_प्रकार)atomic_load32(&_mapped_pages) * memory_page_आकार;
  माप_प्रकार mapped_os =
      (माप_प्रकार)atomic_load32(&_mapped_pages_os) * memory_page_आकार;
  माप_प्रकार mapped_peak = (माप_प्रकार)_mapped_pages_peak * memory_page_आकार;
  माप_प्रकार mapped_योग_2 =
      (माप_प्रकार)atomic_load32(&mapped_योग) * memory_page_आकार;
  माप_प्रकार unmapped_योग_2 =
      (माप_प्रकार)atomic_load32(&unmapped_योग) * memory_page_आकार;
  ख_लिखो(
      file,
      "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB\n");
  ख_लिखो(file, "%9zu %11zu %13zu %14zu %16zu\n",
          mapped / (माप_प्रकार)(1024 * 1024), mapped_os / (माप_प्रकार)(1024 * 1024),
          mapped_peak / (माप_प्रकार)(1024 * 1024),
          mapped_योग_2 / (माप_प्रकार)(1024 * 1024),
          unmapped_योग_2 / (माप_प्रकार)(1024 * 1024));

  ख_लिखो(file, "\n");
#अगर 0
	int64_t allocated = atomic_load64(&allocation_गणक);
	int64_t deallocated = atomic_load64(&deallocation_गणक);
	ख_लिखो(file, "Allocation count: %lli\n", allocated);
	ख_लिखो(file, "Deallocation count: %lli\n", deallocated);
	ख_लिखो(file, "Current allocations: %lli\n", (allocated - deallocated));
	ख_लिखो(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	ख_लिखो(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#पूर्ण_अगर
#पूर्ण_अगर
  (व्योम)माप(file);
}

#अगर RPMALLOC_FIRST_CLASS_HEAPS

बाह्य अंतरभूत rpmalloc_heap_t *rpmalloc_heap_acquire(व्योम) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  heap_t *heap = _rpmalloc_heap_allocate(1);
  rpmalloc_assume(heap != शून्य);
  heap->owner_thread = 0;
  _rpmalloc_stat_inc(&_memory_active_heaps);
  वापस heap;
}

बाह्य अंतरभूत व्योम rpmalloc_heap_release(rpmalloc_heap_t *heap) {
  अगर (heap)
    _rpmalloc_heap_release(heap, 1, 1);
}

बाह्य अंतरभूत RPMALLOC_ALLOCATOR व्योम *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, माप_प्रकार आकार_2) {
#अगर ENABLE_VALIDATE_ARGS
  अगर (आकार_2 >= MAX_ALLOC_आकार) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#पूर्ण_अगर
  वापस _rpmalloc_allocate(heap, आकार_2);
}

बाह्य अंतरभूत RPMALLOC_ALLOCATOR व्योम *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, माप_प्रकार alignment,
                            माप_प्रकार आकार_2) {
#अगर ENABLE_VALIDATE_ARGS
  अगर (आकार_2 >= MAX_ALLOC_आकार) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#पूर्ण_अगर
  वापस _rpmalloc_aligned_allocate(heap, alignment, आकार_2);
}

बाह्य अंतरभूत RPMALLOC_ALLOCATOR व्योम *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, माप_प्रकार अंक_2, माप_प्रकार आकार_2) {
  वापस rpmalloc_heap_aligned_calloc(heap, 0, अंक_2, आकार_2);
}

बाह्य अंतरभूत RPMALLOC_ALLOCATOR व्योम *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, माप_प्रकार alignment,
                             माप_प्रकार अंक_2, माप_प्रकार आकार_2) {
  माप_प्रकार योग_3;
#अगर ENABLE_VALIDATE_ARGS
#अगर PLATFORM_WINDOWS
  पूर्णांक err = आकार_TMult(अंक_2, आकार_2, &योग_3);
  अगर ((err != S_OK) || (योग_3 >= MAX_ALLOC_आकार)) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#अन्यथा
  पूर्णांक err = __builtin_umull_overflow(अंक_2, आकार_2, &योग_3);
  अगर (err || (योग_3 >= MAX_ALLOC_आकार)) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#पूर्ण_अगर
#अन्यथा
  योग_3 = अंक_2 * आकार_2;
#पूर्ण_अगर
  व्योम *block = _rpmalloc_aligned_allocate(heap, alignment, योग_3);
  अगर (block)
    स_रखो(block, 0, योग_3);
  वापस block;
}

बाह्य अंतरभूत RPMALLOC_ALLOCATOR व्योम *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, व्योम *ptr, माप_प्रकार आकार_2,
                      अचिन्हित पूर्णांक flags) {
#अगर ENABLE_VALIDATE_ARGS
  अगर (आकार_2 >= MAX_ALLOC_आकार) {
    त्रुटि_सं = EINVAL;
    वापस ptr;
  }
#पूर्ण_अगर
  वापस _rpmalloc_reallocate(heap, ptr, आकार_2, 0, flags);
}

बाह्य अंतरभूत RPMALLOC_ALLOCATOR व्योम *
rpmalloc_heap_aligned_realloc(rpmalloc_heap_t *heap, व्योम *ptr,
                              माप_प्रकार alignment, माप_प्रकार आकार_2,
                              अचिन्हित पूर्णांक flags) {
#अगर ENABLE_VALIDATE_ARGS
  अगर ((आकार_2 + alignment < आकार_2) || (alignment > memory_page_आकार)) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#पूर्ण_अगर
  वापस _rpmalloc_aligned_reallocate(heap, ptr, alignment, आकार_2, 0, flags);
}

बाह्य अंतरभूत व्योम rpmalloc_heap_free(rpmalloc_heap_t *heap, व्योम *ptr) {
  (व्योम)माप(heap);
  _rpmalloc_deallocate(ptr);
}

बाह्य अंतरभूत व्योम rpmalloc_heap_free_all(rpmalloc_heap_t *heap) {
  span_t *span;
  span_t *अग्र_span;

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  क्रम (माप_प्रकार iclass = 0; iclass < आकार_CLASS_गिनती; ++iclass) {
    span = heap->आकार_class[iclass].partial_span;
    जबतक (span) {
      अग्र_span = span->अग्र;
      rpmalloc_heap_cache_जोड़ो(heap, span);
      span = अग्र_span;
    }
    heap->आकार_class[iclass].partial_span = 0;
    span = heap->full_span[iclass];
    जबतक (span) {
      अग्र_span = span->अग्र;
      rpmalloc_heap_cache_जोड़ो(heap, span);
      span = अग्र_span;
    }

    span = heap->आकार_class[iclass].cache;
    अगर (span)
      rpmalloc_heap_cache_जोड़ो(heap, span);
    heap->आकार_class[iclass].cache = 0;
  }
  स_रखो(heap->आकार_class, 0, माप(heap->आकार_class));
  स_रखो(heap->full_span, 0, माप(heap->full_span));

  span = heap->large_huge_span;
  जबतक (span) {
    अग्र_span = span->अग्र;
    अगर (UNEXPECTED(span->आकार_class == आकार_CLASS_HUGE))
      _rpmalloc_deallocate_huge(span);
    अन्यथा
      rpmalloc_heap_cache_जोड़ो(heap, span);
    span = अग्र_span;
  }
  heap->large_huge_span = 0;
  heap->full_span_गिनती = 0;

#अगर ENABLE_THREAD_CACHE
  क्रम (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    span_cache_t *span_cache;
    अगर (!iclass)
      span_cache = &heap->span_cache;
    अन्यथा
      span_cache = (span_cache_t *)(heap->span_large_cache + (iclass - 1));
    अगर (!span_cache->गिनती)
      जारी;
#अगर ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&heap->thread_to_global,
                         span_cache->गिनती * (iclass + 1) * memory_span_आकार);
    _rpmalloc_stat_add(&heap->span_use[iclass].spans_to_global,
                       span_cache->गिनती);
    rpmalloc_global_cache_जोड़ो_spans(span_cache->span, iclass + 1,
                                        span_cache->गिनती);
#अन्यथा
    क्रम (माप_प्रकार ispan = 0; ispan < span_cache->गिनती; ++ispan)
      _rpmalloc_span_unmap(span_cache->span[ispan]);
#पूर्ण_अगर
    span_cache->गिनती = 0;
  }
#पूर्ण_अगर

#अगर ENABLE_STATISTICS
  क्रम (माप_प्रकार iclass = 0; iclass < आकार_CLASS_गिनती; ++iclass) {
    atomic_store32(&heap->आकार_class_use[iclass].alloc_current, 0);
    atomic_store32(&heap->आकार_class_use[iclass].spans_current, 0);
  }
  क्रम (माप_प्रकार iclass = 0; iclass < LARGE_CLASS_गिनती; ++iclass) {
    atomic_store32(&heap->span_use[iclass].current, 0);
  }
#पूर्ण_अगर
}

बाह्य अंतरभूत व्योम rpmalloc_heap_thread_set_current(rpmalloc_heap_t *heap) {
  heap_t *पूर्व_heap = get_thread_heap_raw();
  अगर (पूर्व_heap != heap) {
    set_thread_heap(heap);
    अगर (पूर्व_heap)
      rpmalloc_heap_release(पूर्व_heap);
  }
}

बाह्य अंतरभूत rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(व्योम *ptr) {
  // Grab the span, and then the heap from the span
  span_t *span = (span_t *)((uintptr_t)ptr & _memory_span_mask);
  अगर (span) {
    वापस span->heap;
  }
  वापस 0;
}

#पूर्ण_अगर

#अगर ENABLE_PRELOAD || ENABLE_OVERRIDE

#समावेश "malloc.c"

#पूर्ण_अगर

व्योम rpmalloc_linker_reference(व्योम) { (व्योम)माप(_rpmalloc_initialized); }
