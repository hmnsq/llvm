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
#निर्देशः clang diagnostic ignored "-Wunused-macros"
#निर्देशः clang diagnostic ignored "-Wunused-function"
#यदि __has_warning("-Wreserved-identifier")
#निर्देशः clang diagnostic ignored "-Wreserved-identifier"
#यदि_अन्तः
#यदि __has_warning("-Wstatic-in-inline")
#निर्देशः clang diagnostic ignored "-Wstatic-in-inline"
#यदि_अन्तः
#नोचेद्यदि defined(__GNUC__)
#निर्देशः GCC diagnostic ignored "-Wunused-macros"
#निर्देशः GCC diagnostic ignored "-Wunused-function"
#यदि_अन्तः

#यदि !defined(__has_builtin)
#परिभाषय __has_builtin(b) 0
#यदि_अन्तः

#यदि defined(__GNUC__) || defined(__clang__)

#यदि __has_builtin(__builtin_memcpy_inline)
#परिभाषय _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#अन्यथा
#परिभाषय _rpmalloc_memcpy_const(x, y, s)                                        \
  कुरु {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    स्मृतिं_प्रतिलिख(x, y, s);                                                           \
  } यावत् (0)
#यदि_अन्तः

#यदि __has_builtin(__builtin_memset_inline)
#परिभाषय _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#अन्यथा
#परिभाषय _rpmalloc_memset_const(x, y, s)                                        \
  कुरु {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    स्मृतिं_पूरय(x, y, s);                                                           \
  } यावत् (0)
#यदि_अन्तः
#अन्यथा
#परिभाषय _rpmalloc_memcpy_const(x, y, s) स्मृतिं_प्रतिलिख(x, y, s)
#परिभाषय _rpmalloc_memset_const(x, y, s) स्मृतिं_पूरय(x, y, s)
#यदि_अन्तः

#यदि __has_builtin(__builtin_assume)
#परिभाषय rpmalloc_assume(cond) __builtin_assume(cond)
#नोचेद्यदि defined(__GNUC__)
#परिभाषय rpmalloc_assume(cond)                                                  \
  कुरु {                                                                         \
    यदि (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } यावत् (0)
#नोचेद्यदि defined(_MSC_VER)
#परिभाषय rpmalloc_assume(cond) __assume(cond)
#अन्यथा
#परिभाषय rpmalloc_assume(cond) 0
#यदि_अन्तः

#यदि_अपरिभाषितम् HEAP_ARRAY_आकार
//! Size of heap hashmap
#परिभाषय HEAP_ARRAY_आकार 47
#यदि_अन्तः
#यदि_अपरिभाषितम् ENABLE_THREAD_CACHE
//! Enable per-thread cache
#परिभाषय ENABLE_THREAD_CACHE 1
#यदि_अन्तः
#यदि_अपरिभाषितम् ENABLE_GLOBAL_CACHE
//! Enable global cache shared between all threads, requires thread cache
#परिभाषय ENABLE_GLOBAL_CACHE 1
#यदि_अन्तः
#यदि_अपरिभाषितम् ENABLE_VALIDATE_तर्काः
//! Enable validation of args to public entry points
#परिभाषय ENABLE_VALIDATE_तर्काः 0
#यदि_अन्तः
#यदि_अपरिभाषितम् ENABLE_STATISTICS
//! Enable statistics collection
#परिभाषय ENABLE_STATISTICS 0
#यदि_अन्तः
#यदि_अपरिभाषितम् ENABLE_ASSERTS
//! Enable asserts
#परिभाषय ENABLE_ASSERTS 0
#यदि_अन्तः
#यदि_अपरिभाषितम् ENABLE_OVERRIDE
//! Override standard library malloc/free and new/delete entry points
#परिभाषय ENABLE_OVERRIDE 0
#यदि_अन्तः
#यदि_अपरिभाषितम् ENABLE_PRELOAD
//! Support preloading
#परिभाषय ENABLE_PRELOAD 0
#यदि_अन्तः
#यदि_अपरिभाषितम् DISABLE_UNMAP
//! Disable unmapping memory pages (also enables unlimited cache)
#परिभाषय DISABLE_UNMAP 0
#यदि_अन्तः
#यदि_अपरिभाषितम् ENABLE_UNLIMITED_CACHE
//! Enable unlimited global cache (no unmapping until finalization)
#परिभाषय ENABLE_UNLIMITED_CACHE 0
#यदि_अन्तः
#यदि_अपरिभाषितम् ENABLE_ADAPTIVE_THREAD_CACHE
//! Enable adaptive thread cache size based on use heuristics
#परिभाषय ENABLE_ADAPTIVE_THREAD_CACHE 0
#यदि_अन्तः
#यदि_अपरिभाषितम् DEFAULT_विस्तृतिः_MAP_गणनम्
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#परिभाषय DEFAULT_विस्तृतिः_MAP_गणनम् 64
#यदि_अन्तः
#यदि_अपरिभाषितम् GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#परिभाषय GLOBAL_CACHE_MULTIPLIER 8
#यदि_अन्तः

#यदि DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#दोषः Must use global cache यदि unmap is disabled
#यदि_अन्तः

#यदि DISABLE_UNMAP
#अपरिभाषय ENABLE_UNLIMITED_CACHE
#परिभाषय ENABLE_UNLIMITED_CACHE 1
#यदि_अन्तः

#यदि !ENABLE_GLOBAL_CACHE
#अपरिभाषय ENABLE_UNLIMITED_CACHE
#परिभाषय ENABLE_UNLIMITED_CACHE 0
#यदि_अन्तः

#यदि !ENABLE_THREAD_CACHE
#अपरिभाषय ENABLE_ADAPTIVE_THREAD_CACHE
#परिभाषय ENABLE_ADAPTIVE_THREAD_CACHE 0
#यदि_अन्तः

#यदि defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#परिभाषय PLATFORM_WINDOWS 1
#परिभाषय PLATFORM_POSIX 0
#अन्यथा
#परिभाषय PLATFORM_WINDOWS 0
#परिभाषय PLATFORM_POSIX 1
#यदि_अन्तः

/// Platform and arch specifics
#यदि defined(_MSC_VER) && !defined(__clang__)
#निर्देशः warning(disable : 5105)
#यदि_अपरिभाषितम् FORCEINLINE
#परिभाषय FORCEINLINE अन्तःस्थ __forceinline
#यदि_अन्तः
#परिभाषय _Static_assert static_assert
#अन्यथा
#यदि_अपरिभाषितम् FORCEINLINE
#परिभाषय FORCEINLINE अन्तःस्थ __attribute__((__always_inline__))
#यदि_अन्तः
#यदि_अन्तः
#यदि PLATFORM_WINDOWS
#यदि_अपरिभाषितम् WIN32_LEAN_AND_माध्यम्
#परिभाषय WIN32_LEAN_AND_माध्यम्
#यदि_अन्तः
#अन्तर्भाव <windows.h>
#यदि ENABLE_VALIDATE_तर्काः
#अन्तर्भाव <intsafe.h>
#यदि_अन्तः
#अन्यथा
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>
#यदि defined(__linux__) || defined(__ANDROID__)
#अन्तर्भाव <sys/prctl.h>
#यदि !defined(PR_समुच्चयः_VMA)
#परिभाषय PR_समुच्चयः_VMA 0x53564d41
#परिभाषय PR_समुच्चयः_VMA_ANON_NAME 0
#यदि_अन्तः
#यदि_अन्तः
#यदि defined(__APPLE__)
#अन्तर्भाव <TargetConditionals.h>
#यदि !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#अन्तर्भाव <mach/mach_vm.h>
#अन्तर्भाव <mach/vm_statistics.h>
#यदि_अन्तः
#अन्तर्भाव <pthread.h>
#यदि_अन्तः
#यदि defined(__HAIKU__) || defined(__TINYC__)
#अन्तर्भाव <pthread.h>
#यदि_अन्तः
#यदि_अन्तः

#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <string.h>

#यदि defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#अन्तर्भाव <fibersapi.h>
स्थिर DWORD fls_कुञ्चिका;
#यदि_अन्तः

#यदि PLATFORM_POSIX
#अन्तर्भाव <sched.h>
#अन्तर्भाव <sys/mman.h>
#यदि_परिभाषितम् __FreeBSD__
#अन्तर्भाव <sys/sysctl.h>
#परिभाषय MAP_HUGETLB MAP_ALIGNED_SUPER
#यदि_अपरिभाषितम् PROT_MAX
#परिभाषय PROT_MAX(f) 0
#यदि_अन्तः
#अन्यथा
#परिभाषय PROT_MAX(f) 0
#यदि_अन्तः
#यदि_परिभाषितम् __sun
बाह्य पूर्णाङ्क madvise(caddr_t, size_t, पूर्णाङ्क);
#यदि_अन्तः
#यदि_अपरिभाषितम् MAP_UNINITIALIZED
#परिभाषय MAP_UNINITIALIZED 0
#यदि_अन्तः
#यदि_अन्तः
#अन्तर्भाव <errno.h>

#यदि ENABLE_ASSERTS
#अपरिभाषय NDEBUG
#यदि defined(_MSC_VER) && !defined(_DEBUG)
#परिभाषय _DEBUG
#यदि_अन्तः
#अन्तर्भाव <assert.h>
#परिभाषय RPMALLOC_TOSTRING_M(x) #x
#परिभाषय RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#परिभाषय rpmalloc_assert(truth, सन्देशः_2)                                        \
  कुरु {                                                                         \
    यदि (!(truth)) {                                                            \
      यदि (स्मृतिः_config.error_callback) {                                     \
        स्मृतिः_config.error_callback(सन्देशः_2 " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } अन्यथा {                                                                 \
        प्रतिज्ञा((truth) && सन्देशः_2);                                            \
      }                                                                        \
    }                                                                          \
  } यावत् (0)
#अन्यथा
#परिभाषय rpmalloc_assert(truth, सन्देशः_2)                                        \
  कुरु {                                                                         \
  } यावत् (0)
#यदि_अन्तः
#यदि ENABLE_STATISTICS
#अन्तर्भाव <stdio.h>
#यदि_अन्तः

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
स्थिर FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t सङ्कलय) {
  प्रत्यागम (int32_t)Interlocked_Exchange_सङ्कलय(val, सङ्कलय) + सङ्कलय;
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
स्थिर FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t सङ्कलय) {
  प्रत्यागम (int64_t)InterlockedExchangeAdd64(val, सङ्कलय) + सङ्कलय;
}
स्थिर FORCEINLINE शून्य *atomic_आरोपय_ptr(atomicptr_t *src) {
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

#परिभाषय EXPECTED(x) (x)
#परिभाषय UNEXPECTED(x) (x)

#अन्यथा

#अन्तर्भाव <stdatomic.h>

प्रकारनिर्देश चञ्चल _Atomic(int32_t) atomic32_t;
प्रकारनिर्देश चञ्चल _Atomic(int64_t) atomic64_t;
प्रकारनिर्देश चञ्चल _Atomic(शून्य *) atomicptr_t;

स्थिर FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  प्रत्यागम atomic_आरोपय_explicit(src, memory_order_relaxed);
}
स्थिर FORCEINLINE शून्य atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
स्थिर FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  प्रत्यागम atomic_fetch_सङ्कलय_explicit(val, 1, memory_order_relaxed) + 1;
}
स्थिर FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  प्रत्यागम atomic_fetch_सङ्कलय_explicit(val, -1, memory_order_relaxed) - 1;
}
स्थिर FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t सङ्कलय) {
  प्रत्यागम atomic_fetch_सङ्कलय_explicit(val, सङ्कलय, memory_order_relaxed) + सङ्कलय;
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
  प्रत्यागम atomic_आरोपय_explicit(val, memory_order_relaxed);
}
स्थिर FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t सङ्कलय) {
  प्रत्यागम atomic_fetch_सङ्कलय_explicit(val, सङ्कलय, memory_order_relaxed) + सङ्कलय;
}
स्थिर FORCEINLINE शून्य *atomic_आरोपय_ptr(atomicptr_t *src) {
  प्रत्यागम atomic_आरोपय_explicit(src, memory_order_relaxed);
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

#परिभाषय EXPECTED(x) __builtin_expect((x), 1)
#परिभाषय UNEXPECTED(x) __builtin_expect((x), 0)

#यदि_अन्तः

////////////
///
/// Statistics related functions (evaluate to nothing when statistics not
/// enabled)
///
//////

#यदि ENABLE_STATISTICS
#परिभाषय _rpmalloc_stat_inc(गणक_2) atomic_incr32(गणक_2)
#परिभाषय _rpmalloc_stat_dec(गणक_2) atomic_decr32(गणक_2)
#परिभाषय rpmalloc_stat_सङ्कलय(गणक_2, मानम्_2)                                     \
  atomic_add32(गणक_2, (int32_t)(मानम्_2))
#परिभाषय _rpmalloc_stat_add64(गणक_2, मानम्_2)                                   \
  atomic_add64(गणक_2, (int64_t)(मानम्_2))
#परिभाषय rpmalloc_stat_सङ्कलय_peak(गणक_2, मानम्_2, peak)                          \
  कुरु {                                                                         \
    int32_t cur_गणनम् = atomic_add32(गणक_2, (int32_t)(मानम्_2));              \
    यदि (cur_गणनम् > (peak))                                                   \
      peak = cur_गणनम्;                                                       \
  } यावत् (0)
#परिभाषय _rpmalloc_stat_sub(गणक_2, मानम्_2)                                     \
  atomic_add32(गणक_2, -(int32_t)(मानम्_2))
#परिभाषय _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  कुरु {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&heap->आकार_class_use[class_idx].alloc_current);         \
    यदि (alloc_current > heap->आकार_class_use[class_idx].alloc_peak)            \
      heap->आकार_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&heap->आकार_class_use[class_idx].alloc_योग);               \
  } यावत् (0)
#परिभाषय _rpmalloc_stat_inc_free(heap, class_idx)                               \
  कुरु {                                                                         \
    atomic_decr32(&heap->आकार_class_use[class_idx].alloc_current);             \
    atomic_incr32(&heap->आकार_class_use[class_idx].free_योग);                \
  } यावत् (0)
#अन्यथा
#परिभाषय _rpmalloc_stat_inc(गणक_2)                                            \
  कुरु {                                                                         \
  } यावत् (0)
#परिभाषय _rpmalloc_stat_dec(गणक_2)                                            \
  कुरु {                                                                         \
  } यावत् (0)
#परिभाषय rpmalloc_stat_सङ्कलय(गणक_2, मानम्_2)                                     \
  कुरु {                                                                         \
  } यावत् (0)
#परिभाषय _rpmalloc_stat_add64(गणक_2, मानम्_2)                                   \
  कुरु {                                                                         \
  } यावत् (0)
#परिभाषय rpmalloc_stat_सङ्कलय_peak(गणक_2, मानम्_2, peak)                          \
  कुरु {                                                                         \
  } यावत् (0)
#परिभाषय _rpmalloc_stat_sub(गणक_2, मानम्_2)                                     \
  कुरु {                                                                         \
  } यावत् (0)
#परिभाषय _rpmalloc_stat_inc_alloc(heap, class_idx)                              \
  कुरु {                                                                         \
  } यावत् (0)
#परिभाषय _rpmalloc_stat_inc_free(heap, class_idx)                               \
  कुरु {                                                                         \
  } यावत् (0)
#यदि_अन्तः

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#परिभाषय SMALL_GRANULARITY 16
//! Small granularity shift count
#परिभाषय SMALL_GRANULARITY_सरणम् 4
//! Number of small block size classes
#परिभाषय SMALL_CLASS_गणनम् 65
//! Maximum size of a small block
#परिभाषय SMALL_आकार_सीमा (SMALL_GRANULARITY * (SMALL_CLASS_गणनम् - 1))
//! Granularity of a medium allocation block
#परिभाषय MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#परिभाषय MEDIUM_GRANULARITY_सरणम् 9
//! Number of medium block size classes
#परिभाषय MEDIUM_CLASS_गणनम् 61
//! Total number of small + medium size classes
#परिभाषय आकार_CLASS_गणनम् (SMALL_CLASS_गणनम् + MEDIUM_CLASS_गणनम्)
//! Number of large block size classes
#परिभाषय LARGE_CLASS_गणनम् 63
//! Maximum size of a medium block
#परिभाषय MEDIUM_आकार_सीमा                                                      \
  (SMALL_आकार_सीमा + (MEDIUM_GRANULARITY * MEDIUM_CLASS_गणनम्))
//! Maximum size of a large block
#परिभाषय LARGE_आकार_सीमा                                                       \
  ((LARGE_CLASS_गणनम् * स्मृतिः_विस्तृतिः_आकार) - विस्तृतिः_शीर्षकम्_आकार)
//! Size of a span header (must be a multiple of SMALL_GRANULARITY and a power
//! of two)
#परिभाषय विस्तृतिः_शीर्षकम्_आकार 128
//! Number of spans in thread cache
#परिभाषय MAX_THREAD_विस्तृतिः_CACHE 400
//! Number of spans to transfer between thread and global cache
#परिभाषय THREAD_विस्तृतिः_CACHE_TRANSFER 64
//! Number of spans in thread cache for large spans (must be greater than
//! LARGE_CLASS_COUNT / 2)
#परिभाषय MAX_THREAD_विस्तृतिः_LARGE_CACHE 100
//! Number of spans to transfer between thread and global cache for large spans
#परिभाषय THREAD_विस्तृतिः_LARGE_CACHE_TRANSFER 6

_Static_assert((SMALL_GRANULARITY & (SMALL_GRANULARITY - 1)) == 0,
               "Small granularity must be power of two");
_Static_assert((विस्तृतिः_शीर्षकम्_आकार & (विस्तृतिः_शीर्षकम्_आकार - 1)) == 0,
               "Span header size must be power of two");

#यदि ENABLE_VALIDATE_तर्काः
//! Maximum allocation size to avoid integer overflow
#अपरिभाषय MAX_ALLOC_आकार
#परिभाषय MAX_ALLOC_आकार (((size_t) - 1) - स्मृतिः_विस्तृतिः_आकार)
#यदि_अन्तः

#परिभाषय pointer_विस्थापनम्(ptr, ofs) (शून्य *)((वर्ण *)(ptr) + (ptrdiff_t)(ofs))
#परिभाषय pointer_diff(first, second)                                            \
  (ptrdiff_t)((ध्रुव वर्ण *)(first) - (ध्रुव वर्ण *)(second))

#परिभाषय INVALID_POINTER ((शून्य *)((uintptr_t) - 1))

#परिभाषय आकार_CLASS_LARGE आकार_CLASS_गणनम्
#परिभाषय आकार_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
प्रकारनिर्देश संरचना heap_t heap_t;
//! Span of memory pages
प्रकारनिर्देश संरचना विस्तृतिः_t विस्तृतिः_t;
//! Span list
प्रकारनिर्देश संरचना विस्तृतिः_list_t विस्तृतिः_list_t;
//! Span active data
प्रकारनिर्देश संरचना विस्तृतिः_सक्रिय_t विस्तृतिः_सक्रिय_t;
//! Size class definition
प्रकारनिर्देश संरचना आकार_class_t आकार_class_t;
//! Global cache
प्रकारनिर्देश संरचना global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#परिभाषय विस्तृतिः_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#परिभाषय विस्तृतिः_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#परिभाषय विस्तृतिः_FLAG_ALIGNED_खण्डानि 4U
//! Flag indicating an unmapped master span
#परिभाषय विस्तृतिः_FLAG_UNMAPPED_MASTER 8U

#यदि ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
संरचना विस्तृतिः_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t उच्चः;
#यदि ENABLE_STATISTICS
  //! Number of spans in deferred list
  atomic32_t spans_deferred;
  //! Number of spans transitioned to global cache
  atomic32_t spans_गन्तव्यम्_global;
  //! Number of spans transitioned from global cache
  atomic32_t spans_from_global;
  //! Number of spans transitioned to thread cache
  atomic32_t spans_गन्तव्यम्_cache;
  //! Number of spans transitioned from thread cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned to reserved state
  atomic32_t spans_गन्तव्यम्_reserved;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of raw memory map calls
  atomic32_t spans_map_calls;
#यदि_अन्तः
};
प्रकारनिर्देश संरचना विस्तृतिः_use_t विस्तृतिः_use_t;
#यदि_अन्तः

#यदि ENABLE_STATISTICS
संरचना आकार_class_use_t {
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
  atomic32_t spans_गन्तव्यम्_cache;
  //! Number of spans transitioned from cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of spans mapped
  atomic32_t spans_map_calls;
  int32_t अप्रयुक्तम्_2;
};
प्रकारनिर्देश संरचना आकार_class_use_t आकार_class_use_t;
#यदि_अन्तः

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
संरचना विस्तृतिः_t {
  //! Free list
  शून्य *free_list;
  //! Total block count of size class
  uint32_t खण्डम्_गणनम्;
  //! Size class
  uint32_t आकार_class;
  //! Index of last block initialized in free list
  uint32_t free_list_सीमा;
  //! Number of used blocks remaining when in partial state
  uint32_t प्रयुक्तम्_गणनम्;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_आकार;
  //! Size of a block
  uint32_t खण्डम्_आकार_2;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t विस्तृतिः_गणनम्;
  //! Total span counter for master spans
  uint32_t योग_spans;
  //! Offset from master span for subspans
  uint32_t विस्थापनम्_from_master;
  //! Remaining span counter, for master spans
  atomic32_t remaining_spans;
  //! Alignment offset
  uint32_t align_विस्थापनम्;
  //! Owning heap
  heap_t *heap;
  //! Next span
  विस्तृतिः_t *अग्र;
  //! Previous span
  विस्तृतिः_t *पूर्व;
};
_Static_assert(परिमाण(विस्तृतिः_t) <= विस्तृतिः_शीर्षकम्_आकार, "span size mismatch");

संरचना विस्तृतिः_cache_t {
  size_t गणनम्;
  विस्तृतिः_t *विस्तृतिः[MAX_THREAD_विस्तृतिः_CACHE];
};
प्रकारनिर्देश संरचना विस्तृतिः_cache_t विस्तृतिः_cache_t;

संरचना विस्तृतिः_large_cache_t {
  size_t गणनम्;
  विस्तृतिः_t *विस्तृतिः[MAX_THREAD_विस्तृतिः_LARGE_CACHE];
};
प्रकारनिर्देश संरचना विस्तृतिः_large_cache_t विस्तृतिः_large_cache_t;

संरचना heap_आकार_class_t {
  //! Free list of active span
  शून्य *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  विस्तृतिः_t *partial_विस्तृतिः;
  //! Early level cache of fully free spans
  विस्तृतिः_t *cache;
};
प्रकारनिर्देश संरचना heap_आकार_class_t heap_आकार_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
संरचना heap_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  heap_आकार_class_t आकार_class[आकार_CLASS_गणनम्];
#यदि ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  विस्तृतिः_cache_t विस्तृतिः_cache;
#यदि_अन्तः
  //! List of deferred free spans (single linked list)
  atomicptr_t विस्तृतिः_free_deferred;
  //! Number of full spans
  size_t पूर्णम्_विस्तृतिः_गणनम्;
  //! Mapped but unused spans
  विस्तृतिः_t *विस्तृतिः_reserve;
  //! Master span for mapped but unused spans
  विस्तृतिः_t *विस्तृतिः_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t child_गणनम्;
  //! Next heap in id list
  heap_t *अग्र_heap;
  //! Next heap in orphan list
  heap_t *अग्र_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  पूर्णाङ्क finalize;
  //! Master heap owning the memory pages
  heap_t *master_heap;
#यदि ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  विस्तृतिः_large_cache_t विस्तृतिः_large_cache[LARGE_CLASS_गणनम् - 1];
#यदि_अन्तः
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  विस्तृतिः_t *पूर्णम्_विस्तृतिः[आकार_CLASS_गणनम्];
  //! Double linked list of large and huge spans allocated by this heap
  विस्तृतिः_t *large_huge_विस्तृतिः;
#यदि_अन्तः
#यदि ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  विस्तृतिः_use_t विस्तृतिः_use[LARGE_CLASS_गणनम्];
#यदि_अन्तः
#यदि ENABLE_STATISTICS
  //! Allocation stats per size class
  आकार_class_use_t आकार_class_use[आकार_CLASS_गणनम् + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_गन्तव्यम्_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_गन्तव्यम्_thread;
#यदि_अन्तः
};

// Size class for defining a block size bucket
संरचना आकार_class_t {
  //! Size of blocks in this class
  uint32_t खण्डम्_आकार_2;
  //! Number of blocks in each chunk
  uint16_t खण्डम्_गणनम्;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(परिमाण(आकार_class_t) == 8, "Size class size mismatch");

संरचना global_cache_t {
  //! Cache lock
  atomic32_t तालकम्;
  //! Cache count
  uint32_t गणनम्;
#यदि ENABLE_STATISTICS
  //! Insert count
  size_t निवेशय_गणनम्;
  //! Extract count
  size_t extract_गणनम्;
#यदि_अन्तः
  //! Cached spans
  विस्तृतिः_t *विस्तृतिः[GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_विस्तृतिः_CACHE];
  //! Unlimited cache overflow
  विस्तृतिः_t *अतिप्रवाहः_2;
};

////////////
///
/// Global data
///
//////

//! Default span size (64KiB)
#परिभाषय स्मृतिः_default_विस्तृतिः_आकार (64 * 1024)
#परिभाषय स्मृतिः_default_विस्तृतिः_आकार_सरणम् 16
#परिभाषय स्मृतिः_default_विस्तृतिः_आवरणम् (~((uintptr_t)(स्मृतिः_विस्तृतिः_आकार - 1)))

//! Initialized flag
स्थिर पूर्णाङ्क _rpmalloc_initialized;
//! Main thread ID
स्थिर uintptr_t _rpmalloc_main_thread_id;
//! Configuration
स्थिर rpmalloc_config_t स्मृतिः_config;
//! Memory page size
स्थिर size_t स्मृतिः_page_आकार;
//! Shift to divide by page size
स्थिर size_t स्मृतिः_page_आकार_सरणम्;
//! Granularity at which memory pages are mapped by OS
स्थिर size_t स्मृतिः_map_granularity;
#यदि RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
स्थिर size_t स्मृतिः_विस्तृतिः_आकार;
//! Shift to divide by span size
स्थिर size_t स्मृतिः_विस्तृतिः_आकार_सरणम्;
//! Mask to get to start of a memory span
स्थिर uintptr_t स्मृतिः_विस्तृतिः_आवरणम्;
#अन्यथा
//! Hardwired span size
#परिभाषय स्मृतिः_विस्तृतिः_आकार स्मृतिः_default_विस्तृतिः_आकार
#परिभाषय स्मृतिः_विस्तृतिः_आकार_सरणम् स्मृतिः_default_विस्तृतिः_आकार_सरणम्
#परिभाषय स्मृतिः_विस्तृतिः_आवरणम् स्मृतिः_default_विस्तृतिः_आवरणम्
#यदि_अन्तः
//! Number of spans to map in each map call
स्थिर size_t स्मृतिः_विस्तृतिः_map_गणनम्;
//! Number of spans to keep reserved in each heap
स्थिर size_t स्मृतिः_heap_reserve_गणनम्;
//! Global size classes
स्थिर आकार_class_t स्मृतिः_आकार_class[आकार_CLASS_गणनम्];
//! Run-time size limit of medium blocks
स्थिर size_t स्मृतिः_medium_आकार_सीमा;
//! Heap ID counter
स्थिर atomic32_t स्मृतिः_heap_id;
//! Huge page support
स्थिर पूर्णाङ्क स्मृतिः_huge_पृष्ठानि;
#यदि ENABLE_GLOBAL_CACHE
//! Global span cache
स्थिर global_cache_t स्मृतिः_विस्तृतिः_cache[LARGE_CLASS_गणनम्];
#यदि_अन्तः
//! Global reserved spans
स्थिर विस्तृतिः_t *स्मृतिः_global_reserve;
//! Global reserved count
स्थिर size_t स्मृतिः_global_reserve_गणनम्;
//! Global reserved master
स्थिर विस्तृतिः_t *स्मृतिः_global_reserve_master;
//! All heaps
स्थिर heap_t *स्मृतिः_heaps[HEAP_ARRAY_आकार];
//! Used to restrict access to mapping memory for huge pages
स्थिर atomic32_t स्मृतिः_global_तालकम्;
//! Orphaned heaps
स्थिर heap_t *स्मृतिः_orphan_heaps;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
स्थिर heap_t *स्मृतिः_first_class_orphan_heaps;
#यदि_अन्तः
#यदि ENABLE_STATISTICS
//! Allocations counter
स्थिर atomic64_t आवण्टनम्_गणक;
//! Deallocations counter
स्थिर atomic64_t deallocation_गणक;
//! Active heap count
स्थिर atomic32_t स्मृतिः_सक्रिय_heaps;
//! Number of currently mapped memory pages
स्थिर atomic32_t mapped_पृष्ठानि;
//! Peak number of concurrently mapped memory pages
स्थिर int32_t mapped_पृष्ठानि_peak;
//! Number of mapped master spans
स्थिर atomic32_t _master_spans;
//! Number of unmapped dangling master spans
स्थिर atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
स्थिर atomic32_t mapped_योग;
//! Running counter of total number of unmapped memory pages since start
स्थिर atomic32_t unmapped_योग;
//! Number of currently mapped memory pages in OS calls
स्थिर atomic32_t mapped_पृष्ठानि_os;
//! Number of currently allocated pages in huge allocations
स्थिर atomic32_t huge_पृष्ठानि_current;
//! Peak number of currently allocated pages in huge allocations
स्थिर int32_t huge_पृष्ठानि_peak;
#यदि_अन्तः

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#यदि ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
स्थिर pthread_key_t स्मृतिः_thread_heap;
#अन्यथा
#यदि_परिभाषितम् _MSC_VER
#परिभाषय _Thread_local __declspec(thread)
#परिभाषय TLS_MODEL
#अन्यथा
#यदि_अपरिभाषितम् __HAIKU__
#परिभाषय TLS_MODEL __attribute__((tls_model("initial-exec")))
#अन्यथा
#परिभाषय TLS_MODEL
#यदि_अन्तः
#यदि !defined(__clang__) && defined(__GNUC__)
#परिभाषय _Thread_local __thread
#यदि_अन्तः
#यदि_अन्तः
स्थिर _Thread_local heap_t *स्मृतिः_thread_heap TLS_MODEL;
#यदि_अन्तः

स्थिर अन्तःस्थ heap_t *get_thread_heap_raw(शून्य) {
#यदि (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  प्रत्यागम pthread_getspecific(स्मृतिः_thread_heap);
#अन्यथा
  प्रत्यागम स्मृतिः_thread_heap;
#यदि_अन्तः
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
#यदि_अन्तः
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
#यदि_अन्तः
#नोचेद्यदि defined(__arm__)
  __asm__ चञ्चल("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#नोचेद्यदि defined(__aarch64__)
#यदि defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ चञ्चल("mrs %0, tpidrro_el0" : "=r"(tid));
#अन्यथा
  __asm__ चञ्चल("mrs %0, tpidr_el0" : "=r"(tid));
#यदि_अन्तः
#अन्यथा
#दोषः This platform needs implementation सम्बन्धि_3 get_thread_id()
#यदि_अन्तः
  प्रत्यागम tid;
#अन्यथा
#दोषः This platform needs implementation सम्बन्धि_3 get_thread_id()
#यदि_अन्तः
}

//! Set the current thread heap
स्थिर शून्य समुच्चयः_thread_heap(heap_t *heap) {
#यदि ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(स्मृतिः_thread_heap, heap);
#अन्यथा
  स्मृतिः_thread_heap = heap;
#यदि_अन्तः
  यदि (heap)
    heap->owner_thread = get_thread_id();
}

//! Set main thread ID
बाह्य शून्य rpmalloc_समुच्चयः_main_thread(शून्य);

शून्य rpmalloc_समुच्चयः_main_thread(शून्य) {
  _rpmalloc_main_thread_id = get_thread_id();
}

स्थिर शून्य _rpmalloc_spin(शून्य) {
#यदि defined(_MSC_VER)
#यदि defined(_M_ARM64)
  __yield();
#अन्यथा
  _mm_pause();
#यदि_अन्तः
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
#यदि_अन्तः
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

#यदि_परिभाषितम् _WIN64
#निर्देशः comment(linker, "/INCLUDE:_tls_used")
#निर्देशः comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#निर्देशः const_seg(".CRT$XLY")

बाह्य ध्रुव PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ध्रुव PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#निर्देशः const_seg()
#अन्यथा // _WIN64
#निर्देशः comment(linker, "/INCLUDE:__tls_used")
#निर्देशः comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#निर्देशः दत्त_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#निर्देशः दत्त_seg()
#यदि_अन्तः // _WIN64

स्थिर शून्य NTAPI _rpmalloc_thread_destructor(शून्य *मानम्_2) {
#यदि ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  यदि (get_thread_id() == _rpmalloc_main_thread_id)
    प्रत्यागम;
#यदि_अन्तः
  यदि (मानम्_2)
    rpmalloc_thread_finalize(1);
}
#यदि_अन्तः

////////////
///
/// Low level memory map/unmap
///
//////

स्थिर शून्य rpmalloc_समुच्चयः_name(शून्य *स्थानाङ्कः_2, size_t आकार_2) {
#यदि defined(__linux__) || defined(__ANDROID__)
  ध्रुव वर्ण *name = स्मृतिः_huge_पृष्ठानि ? स्मृतिः_config.huge_page_name
                                        : स्मृतिः_config.page_name;
  यदि (स्थानाङ्कः_2 == MAP_FAILED || !name)
    प्रत्यागम;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (शून्य)prctl(PR_समुच्चयः_VMA, PR_समुच्चयः_VMA_ANON_NAME, (uintptr_t)स्थानाङ्कः_2, आकार_2,
              (uintptr_t)name);
#अन्यथा
  (शून्य)परिमाण(आकार_2);
  (शून्य)परिमाण(स्थानाङ्कः_2);
#यदि_अन्तः
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
स्थिर शून्य *_rpmalloc_mmap(size_t आकार_2, size_t *विस्थापनम्_2) {
  rpmalloc_assert(!(आकार_2 % स्मृतिः_page_आकार), "Invalid mmap size");
  rpmalloc_assert(आकार_2 >= स्मृतिः_page_आकार, "Invalid mmap size");
  शून्य *स्थानाङ्कः_2 = स्मृतिः_config.स्मृतिः_map(आकार_2, विस्थापनम्_2);
  यदि (EXPECTED(स्थानाङ्कः_2 != 0)) {
    rpmalloc_stat_सङ्कलय_peak(&mapped_पृष्ठानि, (आकार_2 >> स्मृतिः_page_आकार_सरणम्),
                            mapped_पृष्ठानि_peak);
    rpmalloc_stat_सङ्कलय(&mapped_योग, (आकार_2 >> स्मृतिः_page_आकार_सरणम्));
  }
  प्रत्यागम स्थानाङ्कः_2;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
स्थिर शून्य _rpmalloc_unmap(शून्य *स्थानाङ्कः_2, size_t आकार_2, size_t विस्थापनम्_2,
                            size_t release) {
  rpmalloc_assert(!release || (release >= आकार_2), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= स्मृतिः_page_आकार),
                  "Invalid unmap size");
  यदि (release) {
    rpmalloc_assert(!(release % स्मृतिः_page_आकार), "Invalid unmap size");
    _rpmalloc_stat_sub(&mapped_पृष्ठानि, (release >> स्मृतिः_page_आकार_सरणम्));
    rpmalloc_stat_सङ्कलय(&unmapped_योग, (release >> स्मृतिः_page_आकार_सरणम्));
  }
  स्मृतिः_config.स्मृतिः_unmap(स्थानाङ्कः_2, आकार_2, विस्थापनम्_2, release);
}

//! Default implementation to map new pages to virtual memory
स्थिर शून्य *_rpmalloc_mmap_os(size_t आकार_2, size_t *विस्थापनम्_2) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((आकार_2 >= स्मृतिः_विस्तृतिः_आकार) &&
                    (स्मृतिः_विस्तृतिः_आकार > स्मृतिः_map_granularity))
                       ? स्मृतिः_विस्तृतिः_आकार
                       : 0;
  rpmalloc_assert(आकार_2 >= स्मृतिः_page_आकार, "Invalid mmap size");
#यदि PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  शून्य *ptr = VirtualAlloc(0, आकार_2 + padding,
                           (स्मृतिः_huge_पृष्ठानि ? MEM_LARGE_पृष्ठानि : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
  यदि (!ptr) {
    यदि (स्मृतिः_config.map_fail_callback) {
      यदि (स्मृतिः_config.map_fail_callback(आकार_2 + padding))
        प्रत्यागम _rpmalloc_mmap_os(आकार_2, विस्थापनम्_2);
    } अन्यथा {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    प्रत्यागम 0;
  }
#अन्यथा
  पूर्णाङ्क flags = MAP_वैयक्तिकम् | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#यदि defined(__APPLE__) && !TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
  पूर्णाङ्क fd = (पूर्णाङ्क)VM_MAKE_चिह्नकम्(240U);
  यदि (स्मृतिः_huge_पृष्ठानि)
    fd |= VM_FLAGS_SUPERPAGE_आकार_2_MB;
  शून्य *ptr = mmap(0, आकार_2 + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#नोचेद्यदि defined(MAP_HUGETLB)
  शून्य *ptr = mmap(0, आकार_2 + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (स्मृतिः_huge_पृष्ठानि ? MAP_HUGETLB : 0) | flags, -1, 0);
#यदि defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  यदि ((ptr == MAP_FAILED || !ptr) && स्मृतिः_huge_पृष्ठानि) {
    ptr = mmap(0, आकार_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    यदि (ptr && ptr != MAP_FAILED) {
      पूर्णाङ्क prm = madvise(ptr, आकार_2 + padding, MADV_HUGEPAGE);
      (शून्य)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#यदि_अन्तः
  rpmalloc_समुच्चयः_name(ptr, आकार_2 + padding);
#नोचेद्यदि defined(MAP_ALIGNED)
  ध्रुव size_t align =
      (परिमाण(size_t) * 8) - (size_t)(__builtin_clzl(आकार_2 - 1));
  शून्य *ptr =
      mmap(0, आकार_2 + padding, PROT_READ | PROT_WRITE,
           (स्मृतिः_huge_पृष्ठानि ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#नोचेद्यदि defined(MAP_ALIGN)
  caddr_t आधारः_2 = (स्मृतिः_huge_पृष्ठानि ? (caddr_t)(4 << 20) : 0);
  शून्य *ptr = mmap(आधारः_2, आकार_2 + padding, PROT_READ | PROT_WRITE,
                   (स्मृतिः_huge_पृष्ठानि ? MAP_ALIGN : 0) | flags, -1, 0);
#अन्यथा
  शून्य *ptr = mmap(0, आकार_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#यदि_अन्तः
  यदि ((ptr == MAP_FAILED) || !ptr) {
    यदि (स्मृतिः_config.map_fail_callback) {
      यदि (स्मृतिः_config.map_fail_callback(आकार_2 + padding))
        प्रत्यागम _rpmalloc_mmap_os(आकार_2, विस्थापनम्_2);
    } अन्यथा यदि (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    प्रत्यागम 0;
  }
#यदि_अन्तः
  rpmalloc_stat_सङ्कलय(&mapped_पृष्ठानि_os,
                     (int32_t)((आकार_2 + padding) >> स्मृतिः_page_आकार_सरणम्));
  यदि (padding) {
    size_t final_padding = padding - ((uintptr_t)ptr & ~स्मृतिः_विस्तृतिः_आवरणम्);
    rpmalloc_assert(final_padding <= स्मृतिः_विस्तृतिः_आकार,
                    "Internal failure in padding");
    rpmalloc_assert(final_padding <= padding, "Internal failure in padding");
    rpmalloc_assert(!(final_padding % 8), "Internal failure in padding");
    ptr = pointer_विस्थापनम्(ptr, final_padding);
    *विस्थापनम्_2 = final_padding >> 3;
  }
  rpmalloc_assert((आकार_2 < स्मृतिः_विस्तृतिः_आकार) ||
                      !((uintptr_t)ptr & ~स्मृतिः_विस्तृतिः_आवरणम्),
                  "Internal failure in padding");
  प्रत्यागम ptr;
}

//! Default implementation to unmap pages from virtual memory
स्थिर शून्य _rpmalloc_unmap_os(शून्य *स्थानाङ्कः_2, size_t आकार_2, size_t विस्थापनम्_2,
                               size_t release) {
  rpmalloc_assert(release || (विस्थापनम्_2 == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= स्मृतिः_page_आकार),
                  "Invalid unmap size");
  rpmalloc_assert(आकार_2 >= स्मृतिः_page_आकार, "Invalid unmap size");
  यदि (release && विस्थापनम्_2) {
    विस्थापनम्_2 <<= 3;
    स्थानाङ्कः_2 = pointer_विस्थापनम्(स्थानाङ्कः_2, -(int32_t)विस्थापनम्_2);
    यदि ((release >= स्मृतिः_विस्तृतिः_आकार) &&
        (स्मृतिः_विस्तृतिः_आकार > स्मृतिः_map_granularity)) {
      // Padding is always one span size
      release += स्मृतिः_विस्तृतिः_आकार;
    }
  }
#यदि !DISABLE_UNMAP
#यदि PLATFORM_WINDOWS
  यदि (!VirtualFree(स्थानाङ्कः_2, release ? 0 : आकार_2,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#अन्यथा
  यदि (release) {
    यदि (munmap(स्थानाङ्कः_2, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } अन्यथा {
#यदि defined(MADV_FREE_REUSABLE)
    पूर्णाङ्क ret;
    यावत् ((ret = madvise(स्थानाङ्कः_2, आकार_2, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    यदि ((ret == -1) && (errno != 0)) {
#नोचेद्यदि defined(MADV_DONTNEED)
    यदि (madvise(स्थानाङ्कः_2, आकार_2, MADV_DONTNEED)) {
#नोचेद्यदि defined(MADV_PAGEOUT)
    यदि (madvise(स्थानाङ्कः_2, आकार_2, MADV_PAGEOUT)) {
#नोचेद्यदि defined(MADV_FREE)
    यदि (madvise(स्थानाङ्कः_2, आकार_2, MADV_FREE)) {
#अन्यथा
    यदि (posix_madvise(स्थानाङ्कः_2, आकार_2, POSIX_MADV_DONTNEED)) {
#यदि_अन्तः
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#यदि_अन्तः
#यदि_अन्तः
  यदि (release)
    _rpmalloc_stat_sub(&mapped_पृष्ठानि_os, release >> स्मृतिः_page_आकार_सरणम्);
}

स्थिर शून्य rpmalloc_विस्तृतिः_mark_as_subspan_unless_master(विस्तृतिः_t *master,
                                                         विस्तृतिः_t *subspan,
                                                         size_t विस्तृतिः_गणनम्);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
स्थिर विस्तृतिः_t *_rpmalloc_global_get_reserved_spans(size_t विस्तृतिः_गणनम्) {
  विस्तृतिः_t *विस्तृतिः = स्मृतिः_global_reserve;
  rpmalloc_विस्तृतिः_mark_as_subspan_unless_master(स्मृतिः_global_reserve_master,
                                               विस्तृतिः, विस्तृतिः_गणनम्);
  स्मृतिः_global_reserve_गणनम् -= विस्तृतिः_गणनम्;
  यदि (स्मृतिः_global_reserve_गणनम्)
    स्मृतिः_global_reserve =
        (विस्तृतिः_t *)pointer_विस्थापनम्(विस्तृतिः, विस्तृतिः_गणनम् << स्मृतिः_विस्तृतिः_आकार_सरणम्);
  अन्यथा
    स्मृतिः_global_reserve = 0;
  प्रत्यागम विस्तृतिः;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
स्थिर शून्य rpmalloc_global_समुच्चयः_reserved_spans(विस्तृतिः_t *master, विस्तृतिः_t *reserve,
                                                size_t reserve_विस्तृतिः_गणनम्) {
  स्मृतिः_global_reserve_master = master;
  स्मृतिः_global_reserve_गणनम् = reserve_विस्तृतिः_गणनम्;
  स्मृतिः_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
स्थिर शून्य rpmalloc_विस्तृतिः_double_link_list_सङ्कलय(विस्तृतिः_t **शीर्ष, विस्तृतिः_t *विस्तृतिः) {
  यदि (*शीर्ष)
    (*शीर्ष)->पूर्व = विस्तृतिः;
  विस्तृतिः->अग्र = *शीर्ष;
  *शीर्ष = विस्तृतिः;
}

//! Pop head span from double linked list
स्थिर शून्य rpmalloc_विस्तृतिः_double_link_list_निष्कासय_शीर्ष(विस्तृतिः_t **शीर्ष,
                                                     विस्तृतिः_t *विस्तृतिः) {
  rpmalloc_assert(*शीर्ष == विस्तृतिः, "Linked list corrupted");
  विस्तृतिः = *शीर्ष;
  *शीर्ष = विस्तृतिः->अग्र;
}

//! Remove a span from double linked list
स्थिर शून्य rpmalloc_विस्तृतिः_double_link_list_remove(विस्तृतिः_t **शीर्ष,
                                                   विस्तृतिः_t *विस्तृतिः) {
  rpmalloc_assert(*शीर्ष, "Linked list corrupted");
  यदि (*शीर्ष == विस्तृतिः) {
    *शीर्ष = विस्तृतिः->अग्र;
  } अन्यथा {
    विस्तृतिः_t *अग्र_विस्तृतिः = विस्तृतिः->अग्र;
    विस्तृतिः_t *पूर्व_विस्तृतिः = विस्तृतिः->पूर्व;
    पूर्व_विस्तृतिः->अग्र = अग्र_विस्तृतिः;
    यदि (EXPECTED(अग्र_विस्तृतिः != 0))
      अग्र_विस्तृतिः->पूर्व = पूर्व_विस्तृतिः;
  }
}

////////////
///
/// Span control
///
//////

स्थिर शून्य rpmalloc_heap_cache_निवेशय(heap_t *heap, विस्तृतिः_t *विस्तृतिः);

स्थिर शून्य _rpmalloc_heap_finalize(heap_t *heap);

स्थिर शून्य rpmalloc_heap_समुच्चयः_reserved_spans(heap_t *heap, विस्तृतिः_t *master,
                                              विस्तृतिः_t *reserve,
                                              size_t reserve_विस्तृतिः_गणनम्);

//! Declare the span to be a subspan and store distance from master span and
//! span count
स्थिर शून्य rpmalloc_विस्तृतिः_mark_as_subspan_unless_master(विस्तृतिः_t *master,
                                                         विस्तृतिः_t *subspan,
                                                         size_t विस्तृतिः_गणनम्) {
  rpmalloc_assert((subspan != master) || (subspan->flags & विस्तृतिः_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  यदि (subspan != master) {
    subspan->flags = विस्तृतिः_FLAG_SUBSPAN;
    subspan->विस्थापनम्_from_master =
        (uint32_t)((uintptr_t)pointer_diff(subspan, master) >>
                   स्मृतिः_विस्तृतिः_आकार_सरणम्);
    subspan->align_विस्थापनम् = 0;
  }
  subspan->विस्तृतिः_गणनम् = (uint32_t)विस्तृतिः_गणनम्;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
स्थिर विस्तृतिः_t *rpmalloc_विस्तृतिः_map_from_reserve(heap_t *heap,
                                               size_t विस्तृतिः_गणनम्) {
  // Update the heap span reserve
  विस्तृतिः_t *विस्तृतिः = heap->विस्तृतिः_reserve;
  heap->विस्तृतिः_reserve =
      (विस्तृतिः_t *)pointer_विस्थापनम्(विस्तृतिः, विस्तृतिः_गणनम् * स्मृतिः_विस्तृतिः_आकार);
  heap->spans_reserved -= (uint32_t)विस्तृतिः_गणनम्;

  rpmalloc_विस्तृतिः_mark_as_subspan_unless_master(heap->विस्तृतिः_reserve_master, विस्तृतिः,
                                               विस्तृतिः_गणनम्);
  यदि (विस्तृतिः_गणनम् <= LARGE_CLASS_गणनम्)
    _rpmalloc_stat_inc(&heap->विस्तृतिः_use[विस्तृतिः_गणनम् - 1].spans_from_reserved);

  प्रत्यागम विस्तृतिः;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
स्थिर size_t rpmalloc_विस्तृतिः_align_गणनम्(size_t विस्तृतिः_गणनम्) {
  size_t request_गणनम् = (विस्तृतिः_गणनम् > स्मृतिः_विस्तृतिः_map_गणनम्)
                             ? विस्तृतिः_गणनम्
                             : स्मृतिः_विस्तृतिः_map_गणनम्;
  यदि ((स्मृतिः_page_आकार > स्मृतिः_विस्तृतिः_आकार) &&
      ((request_गणनम् * स्मृतिः_विस्तृतिः_आकार) % स्मृतिः_page_आकार))
    request_गणनम् +=
        स्मृतिः_विस्तृतिः_map_गणनम् - (request_गणनम् % स्मृतिः_विस्तृतिः_map_गणनम्);
  प्रत्यागम request_गणनम्;
}

//! Setup a newly mapped span
स्थिर शून्य rpmalloc_विस्तृतिः_initialize(विस्तृतिः_t *विस्तृतिः, size_t योग_विस्तृतिः_गणनम्,
                                      size_t विस्तृतिः_गणनम्, size_t align_विस्थापनम्) {
  विस्तृतिः->योग_spans = (uint32_t)योग_विस्तृतिः_गणनम्;
  विस्तृतिः->विस्तृतिः_गणनम् = (uint32_t)विस्तृतिः_गणनम्;
  विस्तृतिः->align_विस्थापनम् = (uint32_t)align_विस्थापनम्;
  विस्तृतिः->flags = विस्तृतिः_FLAG_MASTER;
  atomic_store32(&विस्तृतिः->remaining_spans, (int32_t)योग_विस्तृतिः_गणनम्);
}

स्थिर शून्य rpmalloc_विस्तृतिः_unmap(विस्तृतिः_t *विस्तृतिः);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
स्थिर विस्तृतिः_t *rpmalloc_विस्तृतिः_map_aligned_गणनम्(heap_t *heap,
                                                size_t विस्तृतिः_गणनम्) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_विस्तृतिः_गणनम् = rpmalloc_विस्तृतिः_align_गणनम्(विस्तृतिः_गणनम्);
  size_t align_विस्थापनम् = 0;
  विस्तृतिः_t *विस्तृतिः = (विस्तृतिः_t *)_rpmalloc_mmap(
      aligned_विस्तृतिः_गणनम् * स्मृतिः_विस्तृतिः_आकार, &align_विस्थापनम्);
  यदि (!विस्तृतिः)
    प्रत्यागम 0;
  rpmalloc_विस्तृतिः_initialize(विस्तृतिः, aligned_विस्तृतिः_गणनम्, विस्तृतिः_गणनम्, align_विस्थापनम्);
  _rpmalloc_stat_inc(&_master_spans);
  यदि (विस्तृतिः_गणनम् <= LARGE_CLASS_गणनम्)
    _rpmalloc_stat_inc(&heap->विस्तृतिः_use[विस्तृतिः_गणनम् - 1].spans_map_calls);
  यदि (aligned_विस्तृतिः_गणनम् > विस्तृतिः_गणनम्) {
    विस्तृतिः_t *reserved_spans =
        (विस्तृतिः_t *)pointer_विस्थापनम्(विस्तृतिः, विस्तृतिः_गणनम् * स्मृतिः_विस्तृतिः_आकार);
    size_t reserved_गणनम् = aligned_विस्तृतिः_गणनम् - विस्तृतिः_गणनम्;
    यदि (heap->spans_reserved) {
      rpmalloc_विस्तृतिः_mark_as_subspan_unless_master(
          heap->विस्तृतिः_reserve_master, heap->विस्तृतिः_reserve, heap->spans_reserved);
      rpmalloc_heap_cache_निवेशय(heap, heap->विस्तृतिः_reserve);
    }
    यदि (reserved_गणनम् > स्मृतिः_heap_reserve_गणनम्) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&स्मृतिः_global_तालकम्) == 1,
                      "Global spin lock not held as expected");
      size_t remain_गणनम् = reserved_गणनम् - स्मृतिः_heap_reserve_गणनम्;
      reserved_गणनम् = स्मृतिः_heap_reserve_गणनम्;
      विस्तृतिः_t *remain_विस्तृतिः = (विस्तृतिः_t *)pointer_विस्थापनम्(
          reserved_spans, reserved_गणनम् * स्मृतिः_विस्तृतिः_आकार);
      यदि (स्मृतिः_global_reserve) {
        rpmalloc_विस्तृतिः_mark_as_subspan_unless_master(
            स्मृतिः_global_reserve_master, स्मृतिः_global_reserve,
            स्मृतिः_global_reserve_गणनम्);
        rpmalloc_विस्तृतिः_unmap(स्मृतिः_global_reserve);
      }
      rpmalloc_global_समुच्चयः_reserved_spans(विस्तृतिः, remain_विस्तृतिः, remain_गणनम्);
    }
    rpmalloc_heap_समुच्चयः_reserved_spans(heap, विस्तृतिः, reserved_spans,
                                      reserved_गणनम्);
  }
  प्रत्यागम विस्तृतिः;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
स्थिर विस्तृतिः_t *rpmalloc_विस्तृतिः_map(heap_t *heap, size_t विस्तृतिः_गणनम्) {
  यदि (विस्तृतिः_गणनम् <= heap->spans_reserved)
    प्रत्यागम rpmalloc_विस्तृतिः_map_from_reserve(heap, विस्तृतिः_गणनम्);
  विस्तृतिः_t *विस्तृतिः = 0;
  पूर्णाङ्क use_global_reserve =
      (स्मृतिः_page_आकार > स्मृतिः_विस्तृतिः_आकार) ||
      (स्मृतिः_विस्तृतिः_map_गणनम् > स्मृतिः_heap_reserve_गणनम्);
  यदि (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    यावत् (!atomic_cas32_acquire(&स्मृतिः_global_तालकम्, 1, 0))
      _rpmalloc_spin();
    यदि (स्मृतिः_global_reserve_गणनम् >= विस्तृतिः_गणनम्) {
      size_t reserve_गणनम् =
          (!heap->spans_reserved ? स्मृतिः_heap_reserve_गणनम् : विस्तृतिः_गणनम्);
      यदि (स्मृतिः_global_reserve_गणनम् < reserve_गणनम्)
        reserve_गणनम् = स्मृतिः_global_reserve_गणनम्;
      विस्तृतिः = _rpmalloc_global_get_reserved_spans(reserve_गणनम्);
      यदि (विस्तृतिः) {
        यदि (reserve_गणनम् > विस्तृतिः_गणनम्) {
          विस्तृतिः_t *reserved_विस्तृतिः = (विस्तृतिः_t *)pointer_विस्थापनम्(
              विस्तृतिः, विस्तृतिः_गणनम् << स्मृतिः_विस्तृतिः_आकार_सरणम्);
          rpmalloc_heap_समुच्चयः_reserved_spans(heap, स्मृतिः_global_reserve_master,
                                            reserved_विस्तृतिः,
                                            reserve_गणनम् - विस्तृतिः_गणनम्);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        विस्तृतिः->विस्तृतिः_गणनम् = (uint32_t)विस्तृतिः_गणनम्;
      }
    }
  }
  यदि (!विस्तृतिः)
    विस्तृतिः = rpmalloc_विस्तृतिः_map_aligned_गणनम्(heap, विस्तृतिः_गणनम्);
  यदि (use_global_reserve)
    atomic_store32_release(&स्मृतिः_global_तालकम्, 0);
  प्रत्यागम विस्तृतिः;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
स्थिर शून्य rpmalloc_विस्तृतिः_unmap(विस्तृतिः_t *विस्तृतिः) {
  rpmalloc_assert((विस्तृतिः->flags & विस्तृतिः_FLAG_MASTER) ||
                      (विस्तृतिः->flags & विस्तृतिः_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(विस्तृतिः->flags & विस्तृतिः_FLAG_MASTER) ||
                      !(विस्तृतिः->flags & विस्तृतिः_FLAG_SUBSPAN),
                  "Span flag corrupted");

  पूर्णाङ्क is_master = !!(विस्तृतिः->flags & विस्तृतिः_FLAG_MASTER);
  विस्तृतिः_t *master =
      is_master ? विस्तृतिः
                : ((विस्तृतिः_t *)pointer_विस्थापनम्(
                      विस्तृतिः, -(intptr_t)((uintptr_t)विस्तृतिः->विस्थापनम्_from_master *
                                        स्मृतिः_विस्तृतिः_आकार)));
  rpmalloc_assert(is_master || (विस्तृतिः->flags & विस्तृतिः_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & विस्तृतिः_FLAG_MASTER, "Span flag corrupted");

  size_t विस्तृतिः_गणनम् = विस्तृतिः->विस्तृतिः_गणनम्;
  यदि (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(विस्तृतिः->align_विस्थापनम् == 0, "Span align offset corrupted");
    यदि (स्मृतिः_विस्तृतिः_आकार >= स्मृतिः_page_आकार)
      _rpmalloc_unmap(विस्तृतिः, विस्तृतिः_गणनम् * स्मृतिः_विस्तृतिः_आकार, 0, 0);
  } अन्यथा {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    विस्तृतिः->flags |=
        विस्तृतिः_FLAG_MASTER | विस्तृतिः_FLAG_SUBSPAN | विस्तृतिः_FLAG_UNMAPPED_MASTER;
    rpmalloc_stat_सङ्कलय(&_unmapped_master_spans, 1);
  }

  यदि (atomic_add32(&master->remaining_spans, -(int32_t)विस्तृतिः_गणनम्) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & विस्तृतिः_FLAG_MASTER) &&
                        !!(master->flags & विस्तृतिः_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_गणनम् = master->विस्तृतिः_गणनम्;
    यदि (स्मृतिः_विस्तृतिः_आकार < स्मृतिः_page_आकार)
      unmap_गणनम् = master->योग_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_गणनम् * स्मृतिः_विस्तृतिः_आकार,
                    master->align_विस्थापनम्,
                    (size_t)master->योग_spans * स्मृतिः_विस्तृतिः_आकार);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
स्थिर शून्य rpmalloc_विस्तृतिः_release_गन्तव्यम्_cache(heap_t *heap, विस्तृतिः_t *विस्तृतिः) {
  rpmalloc_assert(heap == विस्तृतिः->heap, "Span heap pointer corrupted");
  rpmalloc_assert(विस्तृतिः->आकार_class < आकार_CLASS_गणनम्,
                  "Invalid span size class");
  rpmalloc_assert(विस्तृतिः->विस्तृतिः_गणनम् == 1, "Invalid span count");
#यदि ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&heap->विस्तृतिः_use[0].current);
#यदि_अन्तः
  _rpmalloc_stat_dec(&heap->आकार_class_use[विस्तृतिः->आकार_class].spans_current);
  यदि (!heap->finalize) {
    _rpmalloc_stat_inc(&heap->विस्तृतिः_use[0].spans_गन्तव्यम्_cache);
    _rpmalloc_stat_inc(&heap->आकार_class_use[विस्तृतिः->आकार_class].spans_गन्तव्यम्_cache);
    यदि (heap->आकार_class[विस्तृतिः->आकार_class].cache)
      rpmalloc_heap_cache_निवेशय(heap,
                                  heap->आकार_class[विस्तृतिः->आकार_class].cache);
    heap->आकार_class[विस्तृतिः->आकार_class].cache = विस्तृतिः;
  } अन्यथा {
    rpmalloc_विस्तृतिः_unmap(विस्तृतिः);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
स्थिर uint32_t free_list_partial_init(शून्य **list, शून्य **first_खण्डम्,
                                       शून्य *page_start, शून्य *खण्डम्_start,
                                       uint32_t खण्डम्_गणनम्,
                                       uint32_t खण्डम्_आकार_2) {
  rpmalloc_assert(खण्डम्_गणनम्, "Internal failure");
  *first_खण्डम् = खण्डम्_start;
  यदि (खण्डम्_गणनम् > 1) {
    शून्य *free_खण्डम् = pointer_विस्थापनम्(खण्डम्_start, खण्डम्_आकार_2);
    शून्य *खण्डम्_अन्तः =
        pointer_विस्थापनम्(खण्डम्_start, (size_t)खण्डम्_आकार_2 * खण्डम्_गणनम्);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    यदि (खण्डम्_आकार_2 < (स्मृतिः_page_आकार >> 1)) {
      शून्य *page_अन्तः = pointer_विस्थापनम्(page_start, स्मृतिः_page_आकार);
      यदि (page_अन्तः < खण्डम्_अन्तः)
        खण्डम्_अन्तः = page_अन्तः;
    }
    *list = free_खण्डम्;
    खण्डम्_गणनम् = 2;
    शून्य *अग्र_खण्डम् = pointer_विस्थापनम्(free_खण्डम्, खण्डम्_आकार_2);
    यावत् (अग्र_खण्डम् < खण्डम्_अन्तः) {
      *((शून्य **)free_खण्डम्) = अग्र_खण्डम्;
      free_खण्डम् = अग्र_खण्डम्;
      ++खण्डम्_गणनम्;
      अग्र_खण्डम् = pointer_विस्थापनम्(अग्र_खण्डम्, खण्डम्_आकार_2);
    }
    *((शून्य **)free_खण्डम्) = 0;
  } अन्यथा {
    *list = 0;
  }
  प्रत्यागम खण्डम्_गणनम्;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
स्थिर शून्य *rpmalloc_विस्तृतिः_initialize_new(heap_t *heap,
                                           heap_आकार_class_t *heap_आकार_class,
                                           विस्तृतिः_t *विस्तृतिः, uint32_t class_idx) {
  rpmalloc_assert(विस्तृतिः->विस्तृतिः_गणनम् == 1, "Internal failure");
  आकार_class_t *आकार_class = स्मृतिः_आकार_class + class_idx;
  विस्तृतिः->आकार_class = class_idx;
  विस्तृतिः->heap = heap;
  विस्तृतिः->flags &= ~विस्तृतिः_FLAG_ALIGNED_खण्डानि;
  विस्तृतिः->खण्डम्_आकार_2 = आकार_class->खण्डम्_आकार_2;
  विस्तृतिः->खण्डम्_गणनम् = आकार_class->खण्डम्_गणनम्;
  विस्तृतिः->free_list = 0;
  विस्तृतिः->list_आकार = 0;
  atomic_store_ptr_release(&विस्तृतिः->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  शून्य *खण्डम्;
  विस्तृतिः->free_list_सीमा =
      free_list_partial_init(&heap_आकार_class->free_list, &खण्डम्, विस्तृतिः,
                             pointer_विस्थापनम्(विस्तृतिः, विस्तृतिः_शीर्षकम्_आकार),
                             आकार_class->खण्डम्_गणनम्, आकार_class->खण्डम्_आकार_2);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  यदि (विस्तृतिः->free_list_सीमा < विस्तृतिः->खण्डम्_गणनम्) {
    rpmalloc_विस्तृतिः_double_link_list_सङ्कलय(&heap_आकार_class->partial_विस्तृतिः, विस्तृतिः);
    विस्तृतिः->प्रयुक्तम्_गणनम् = विस्तृतिः->free_list_सीमा;
  } अन्यथा {
#यदि RPMALLOC_FIRST_CLASS_HEAPS
    rpmalloc_विस्तृतिः_double_link_list_सङ्कलय(&heap->पूर्णम्_विस्तृतिः[class_idx], विस्तृतिः);
#यदि_अन्तः
    ++heap->पूर्णम्_विस्तृतिः_गणनम्;
    विस्तृतिः->प्रयुक्तम्_गणनम् = विस्तृतिः->खण्डम्_गणनम्;
  }
  प्रत्यागम खण्डम्;
}

स्थिर शून्य rpmalloc_विस्तृतिः_extract_free_list_deferred(विस्तृतिः_t *विस्तृतिः) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  कुरु {
    विस्तृतिः->free_list =
        atomic_exchange_ptr_acquire(&विस्तृतिः->free_list_deferred, INVALID_POINTER);
  } यावत् (विस्तृतिः->free_list == INVALID_POINTER);
  विस्तृतिः->प्रयुक्तम्_गणनम् -= विस्तृतिः->list_आकार;
  विस्तृतिः->list_आकार = 0;
  atomic_store_ptr_release(&विस्तृतिः->free_list_deferred, 0);
}

स्थिर पूर्णाङ्क rpmalloc_विस्तृतिः_is_fully_utilized(विस्तृतिः_t *विस्तृतिः) {
  rpmalloc_assert(विस्तृतिः->free_list_सीमा <= विस्तृतिः->खण्डम्_गणनम्,
                  "Span free list corrupted");
  प्रत्यागम !विस्तृतिः->free_list && (विस्तृतिः->free_list_सीमा >= विस्तृतिः->खण्डम्_गणनम्);
}

स्थिर पूर्णाङ्क rpmalloc_विस्तृतिः_finalize(heap_t *heap, size_t iclass, विस्तृतिः_t *विस्तृतिः,
                                   विस्तृतिः_t **list_शीर्ष) {
  शून्य *free_list = heap->आकार_class[iclass].free_list;
  विस्तृतिः_t *class_विस्तृतिः = (विस्तृतिः_t *)((uintptr_t)free_list & स्मृतिः_विस्तृतिः_आवरणम्);
  यदि (विस्तृतिः == class_विस्तृतिः) {
    // Adopt the heap class free list back into the span free list
    शून्य *खण्डम् = विस्तृतिः->free_list;
    शून्य *चरमः_खण्डम् = 0;
    यावत् (खण्डम्) {
      चरमः_खण्डम् = खण्डम्;
      खण्डम् = *((शून्य **)खण्डम्);
    }
    uint32_t free_गणनम् = 0;
    खण्डम् = free_list;
    यावत् (खण्डम्) {
      ++free_गणनम्;
      खण्डम् = *((शून्य **)खण्डम्);
    }
    यदि (चरमः_खण्डम्) {
      *((शून्य **)चरमः_खण्डम्) = free_list;
    } अन्यथा {
      विस्तृतिः->free_list = free_list;
    }
    heap->आकार_class[iclass].free_list = 0;
    विस्तृतिः->प्रयुक्तम्_गणनम् -= free_गणनम्;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(विस्तृतिः->list_आकार == विस्तृतिः->प्रयुक्तम्_गणनम्, "Memory leak detected");
  यदि (विस्तृतिः->list_आकार == विस्तृतिः->प्रयुक्तम्_गणनम्) {
    _rpmalloc_stat_dec(&heap->विस्तृतिः_use[0].current);
    _rpmalloc_stat_dec(&heap->आकार_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    यदि (list_शीर्ष)
      rpmalloc_विस्तृतिः_double_link_list_remove(list_शीर्ष, विस्तृतिः);
    rpmalloc_विस्तृतिः_unmap(विस्तृतिः);
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
  यावत् (!atomic_cas32_acquire(&cache->तालकम्, 1, 0))
    _rpmalloc_spin();

  प्रति (size_t ispan = 0; ispan < cache->गणनम्; ++ispan)
    rpmalloc_विस्तृतिः_unmap(cache->विस्तृतिः[ispan]);
  cache->गणनम् = 0;

  यावत् (cache->अतिप्रवाहः_2) {
    विस्तृतिः_t *विस्तृतिः = cache->अतिप्रवाहः_2;
    cache->अतिप्रवाहः_2 = विस्तृतिः->अग्र;
    rpmalloc_विस्तृतिः_unmap(विस्तृतिः);
  }

  atomic_store32_release(&cache->तालकम्, 0);
}

स्थिर शून्य rpmalloc_global_cache_निवेशय_spans(विस्तृतिः_t **विस्तृतिः,
                                                size_t विस्तृतिः_गणनम्,
                                                size_t गणनम्) {
  ध्रुव size_t cache_सीमा =
      (विस्तृतिः_गणनम् == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_विस्तृतिः_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_विस्तृतिः_LARGE_CACHE - (विस्तृतिः_गणनम् >> 1));

  global_cache_t *cache = &स्मृतिः_विस्तृतिः_cache[विस्तृतिः_गणनम् - 1];

  size_t निवेशय_गणनम् = गणनम्;
  यावत् (!atomic_cas32_acquire(&cache->तालकम्, 1, 0))
    _rpmalloc_spin();

#यदि ENABLE_STATISTICS
  cache->निवेशय_गणनम् += गणनम्;
#यदि_अन्तः
  यदि ((cache->गणनम् + निवेशय_गणनम्) > cache_सीमा)
    निवेशय_गणनम् = cache_सीमा - cache->गणनम्;

  स्मृतिं_प्रतिलिख(cache->विस्तृतिः + cache->गणनम्, विस्तृतिः, परिमाण(विस्तृतिः_t *) * निवेशय_गणनम्);
  cache->गणनम् += (uint32_t)निवेशय_गणनम्;

#यदि ENABLE_UNLIMITED_CACHE
  यावत् (निवेशय_गणनम् < गणनम्) {
#अन्यथा
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  यावत् ((स्मृतिः_page_आकार > स्मृतिः_विस्तृतिः_आकार) && (निवेशय_गणनम् < गणनम्)) {
#यदि_अन्तः
    विस्तृतिः_t *current_विस्तृतिः = विस्तृतिः[निवेशय_गणनम्++];
    current_विस्तृतिः->अग्र = cache->अतिप्रवाहः_2;
    cache->अतिप्रवाहः_2 = current_विस्तृतिः;
  }
  atomic_store32_release(&cache->तालकम्, 0);

  विस्तृतिः_t *keep = 0;
  प्रति (size_t ispan = निवेशय_गणनम्; ispan < गणनम्; ++ispan) {
    विस्तृतिः_t *current_विस्तृतिः = विस्तृतिः[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    यदि ((current_विस्तृतिः->flags & विस्तृतिः_FLAG_MASTER) &&
        (atomic_load32(&current_विस्तृतिः->remaining_spans) >
         (int32_t)current_विस्तृतिः->विस्तृतिः_गणनम्)) {
      current_विस्तृतिः->अग्र = keep;
      keep = current_विस्तृतिः;
    } अन्यथा {
      rpmalloc_विस्तृतिः_unmap(current_विस्तृतिः);
    }
  }

  यदि (keep) {
    यावत् (!atomic_cas32_acquire(&cache->तालकम्, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    यावत् (keep) {
      प्रति (; islot < cache->गणनम्; ++islot) {
        विस्तृतिः_t *current_विस्तृतिः = cache->विस्तृतिः[islot];
        यदि (!(current_विस्तृतिः->flags & विस्तृतिः_FLAG_MASTER) ||
            ((current_विस्तृतिः->flags & विस्तृतिः_FLAG_MASTER) &&
             (atomic_load32(&current_विस्तृतिः->remaining_spans) <=
              (int32_t)current_विस्तृतिः->विस्तृतिः_गणनम्))) {
          rpmalloc_विस्तृतिः_unmap(current_विस्तृतिः);
          cache->विस्तृतिः[islot] = keep;
          भङ्ग;
        }
      }
      यदि (islot == cache->गणनम्)
        भङ्ग;
      keep = keep->अग्र;
    }

    यदि (keep) {
      विस्तृतिः_t *पुच्छ_2 = keep;
      यावत् (पुच्छ_2->अग्र)
        पुच्छ_2 = पुच्छ_2->अग्र;
      पुच्छ_2->अग्र = cache->अतिप्रवाहः_2;
      cache->अतिप्रवाहः_2 = keep;
    }

    atomic_store32_release(&cache->तालकम्, 0);
  }
}

स्थिर size_t _rpmalloc_global_cache_extract_spans(विस्तृतिः_t **विस्तृतिः,
                                                   size_t विस्तृतिः_गणनम्,
                                                   size_t गणनम्) {
  global_cache_t *cache = &स्मृतिः_विस्तृतिः_cache[विस्तृतिः_गणनम् - 1];

  size_t extract_गणनम् = 0;
  यावत् (!atomic_cas32_acquire(&cache->तालकम्, 1, 0))
    _rpmalloc_spin();

#यदि ENABLE_STATISTICS
  cache->extract_गणनम् += गणनम्;
#यदि_अन्तः
  size_t want = गणनम् - extract_गणनम्;
  यदि (want > cache->गणनम्)
    want = cache->गणनम्;

  स्मृतिं_प्रतिलिख(विस्तृतिः + extract_गणनम्, cache->विस्तृतिः + (cache->गणनम् - want),
         परिमाण(विस्तृतिः_t *) * want);
  cache->गणनम् -= (uint32_t)want;
  extract_गणनम् += want;

  यावत् ((extract_गणनम् < गणनम्) && cache->अतिप्रवाहः_2) {
    विस्तृतिः_t *current_विस्तृतिः = cache->अतिप्रवाहः_2;
    विस्तृतिः[extract_गणनम्++] = current_विस्तृतिः;
    cache->अतिप्रवाहः_2 = current_विस्तृतिः->अग्र;
  }

#यदि ENABLE_ASSERTS
  प्रति (size_t ispan = 0; ispan < extract_गणनम्; ++ispan) {
    rpmalloc_assert(विस्तृतिः[ispan]->विस्तृतिः_गणनम् == विस्तृतिः_गणनम्,
                    "Global cache span count mismatch");
  }
#यदि_अन्तः

  atomic_store32_release(&cache->तालकम्, 0);

  प्रत्यागम extract_गणनम्;
}

#यदि_अन्तः

////////////
///
/// Heap control
///
//////

स्थिर शून्य _rpmalloc_deallocate_huge(विस्तृतिः_t *);

//! Store the given spans as reserve in the given heap
स्थिर शून्य rpmalloc_heap_समुच्चयः_reserved_spans(heap_t *heap, विस्तृतिः_t *master,
                                              विस्तृतिः_t *reserve,
                                              size_t reserve_विस्तृतिः_गणनम्) {
  heap->विस्तृतिः_reserve_master = master;
  heap->विस्तृतिः_reserve = reserve;
  heap->spans_reserved = (uint32_t)reserve_विस्तृतिः_गणनम्;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
स्थिर शून्य _rpmalloc_heap_cache_adopt_deferred(heap_t *heap,
                                                विस्तृतिः_t **single_विस्तृतिः) {
  विस्तृतिः_t *विस्तृतिः = (विस्तृतिः_t *)((शून्य *)atomic_exchange_ptr_acquire(
      &heap->विस्तृतिः_free_deferred, 0));
  यावत् (विस्तृतिः) {
    विस्तृतिः_t *अग्र_विस्तृतिः = (विस्तृतिः_t *)विस्तृतिः->free_list;
    rpmalloc_assert(विस्तृतिः->heap == heap, "Span heap pointer corrupted");
    यदि (EXPECTED(विस्तृतिः->आकार_class < आकार_CLASS_गणनम्)) {
      rpmalloc_assert(heap->पूर्णम्_विस्तृतिः_गणनम्, "Heap span counter corrupted");
      --heap->पूर्णम्_विस्तृतिः_गणनम्;
      _rpmalloc_stat_dec(&heap->विस्तृतिः_use[0].spans_deferred);
#यदि RPMALLOC_FIRST_CLASS_HEAPS
      rpmalloc_विस्तृतिः_double_link_list_remove(&heap->पूर्णम्_विस्तृतिः[विस्तृतिः->आकार_class],
                                             विस्तृतिः);
#यदि_अन्तः
      _rpmalloc_stat_dec(&heap->विस्तृतिः_use[0].current);
      _rpmalloc_stat_dec(&heap->आकार_class_use[विस्तृतिः->आकार_class].spans_current);
      यदि (single_विस्तृतिः && !*single_विस्तृतिः)
        *single_विस्तृतिः = विस्तृतिः;
      अन्यथा
        rpmalloc_heap_cache_निवेशय(heap, विस्तृतिः);
    } अन्यथा {
      यदि (विस्तृतिः->आकार_class == आकार_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(विस्तृतिः);
      } अन्यथा {
        rpmalloc_assert(विस्तृतिः->आकार_class == आकार_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(heap->पूर्णम्_विस्तृतिः_गणनम्, "Heap span counter corrupted");
        --heap->पूर्णम्_विस्तृतिः_गणनम्;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
        rpmalloc_विस्तृतिः_double_link_list_remove(&heap->large_huge_विस्तृतिः, विस्तृतिः);
#यदि_अन्तः
        uint32_t idx = विस्तृतिः->विस्तृतिः_गणनम् - 1;
        _rpmalloc_stat_dec(&heap->विस्तृतिः_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&heap->विस्तृतिः_use[idx].current);
        यदि (!idx && single_विस्तृतिः && !*single_विस्तृतिः)
          *single_विस्तृतिः = विस्तृतिः;
        अन्यथा
          rpmalloc_heap_cache_निवेशय(heap, विस्तृतिः);
      }
    }
    विस्तृतिः = अग्र_विस्तृतिः;
  }
}

स्थिर शून्य _rpmalloc_heap_unmap(heap_t *heap) {
  यदि (!heap->master_heap) {
    यदि ((heap->finalize > 1) && !atomic_load32(&heap->child_गणनम्)) {
      विस्तृतिः_t *विस्तृतिः = (विस्तृतिः_t *)((uintptr_t)heap & स्मृतिः_विस्तृतिः_आवरणम्);
      rpmalloc_विस्तृतिः_unmap(विस्तृतिः);
    }
  } अन्यथा {
    यदि (atomic_decr32(&heap->master_heap->child_गणनम्) == 0) {
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
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    विस्तृतिः_cache_t *विस्तृतिः_cache;
    यदि (!iclass)
      विस्तृतिः_cache = &heap->विस्तृतिः_cache;
    अन्यथा
      विस्तृतिः_cache = (विस्तृतिः_cache_t *)(heap->विस्तृतिः_large_cache + (iclass - 1));
    प्रति (size_t ispan = 0; ispan < विस्तृतिः_cache->गणनम्; ++ispan)
      rpmalloc_विस्तृतिः_unmap(विस्तृतिः_cache->विस्तृतिः[ispan]);
    विस्तृतिः_cache->गणनम् = 0;
  }
#यदि_अन्तः

  यदि (heap->पूर्णम्_विस्तृतिः_गणनम्) {
    --heap->finalize;
    प्रत्यागम;
  }

  प्रति (size_t iclass = 0; iclass < आकार_CLASS_गणनम्; ++iclass) {
    यदि (heap->आकार_class[iclass].free_list ||
        heap->आकार_class[iclass].partial_विस्तृतिः) {
      --heap->finalize;
      प्रत्यागम;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_आकार;
  heap_t *list_heap = स्मृतिः_heaps[list_idx];
  यदि (list_heap == heap) {
    स्मृतिः_heaps[list_idx] = heap->अग्र_heap;
  } अन्यथा {
    यावत् (list_heap->अग्र_heap != heap)
      list_heap = list_heap->अग्र_heap;
    list_heap->अग्र_heap = heap->अग्र_heap;
  }

  _rpmalloc_heap_unmap(heap);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
स्थिर शून्य rpmalloc_heap_cache_निवेशय(heap_t *heap, विस्तृतिः_t *विस्तृतिः) {
  यदि (UNEXPECTED(heap->finalize != 0)) {
    rpmalloc_विस्तृतिः_unmap(विस्तृतिः);
    _rpmalloc_heap_global_finalize(heap);
    प्रत्यागम;
  }
#यदि ENABLE_THREAD_CACHE
  size_t विस्तृतिः_गणनम् = विस्तृतिः->विस्तृतिः_गणनम्;
  _rpmalloc_stat_inc(&heap->विस्तृतिः_use[विस्तृतिः_गणनम् - 1].spans_गन्तव्यम्_cache);
  यदि (विस्तृतिः_गणनम् == 1) {
    विस्तृतिः_cache_t *विस्तृतिः_cache = &heap->विस्तृतिः_cache;
    विस्तृतिः_cache->विस्तृतिः[विस्तृतिः_cache->गणनम्++] = विस्तृतिः;
    यदि (विस्तृतिः_cache->गणनम् == MAX_THREAD_विस्तृतिः_CACHE) {
      ध्रुव size_t remain_गणनम् =
          MAX_THREAD_विस्तृतिः_CACHE - THREAD_विस्तृतिः_CACHE_TRANSFER;
#यदि ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_गन्तव्यम्_global,
                           THREAD_विस्तृतिः_CACHE_TRANSFER * स्मृतिः_विस्तृतिः_आकार);
      rpmalloc_stat_सङ्कलय(&heap->विस्तृतिः_use[विस्तृतिः_गणनम् - 1].spans_गन्तव्यम्_global,
                         THREAD_विस्तृतिः_CACHE_TRANSFER);
      rpmalloc_global_cache_निवेशय_spans(विस्तृतिः_cache->विस्तृतिः + remain_गणनम्,
                                          विस्तृतिः_गणनम्,
                                          THREAD_विस्तृतिः_CACHE_TRANSFER);
#अन्यथा
      प्रति (size_t ispan = 0; ispan < THREAD_विस्तृतिः_CACHE_TRANSFER; ++ispan)
        rpmalloc_विस्तृतिः_unmap(विस्तृतिः_cache->विस्तृतिः[remain_गणनम् + ispan]);
#यदि_अन्तः
      विस्तृतिः_cache->गणनम् = remain_गणनम्;
    }
  } अन्यथा {
    size_t cache_idx = विस्तृतिः_गणनम् - 2;
    विस्तृतिः_large_cache_t *विस्तृतिः_cache = heap->विस्तृतिः_large_cache + cache_idx;
    विस्तृतिः_cache->विस्तृतिः[विस्तृतिः_cache->गणनम्++] = विस्तृतिः;
    ध्रुव size_t cache_सीमा =
        (MAX_THREAD_विस्तृतिः_LARGE_CACHE - (विस्तृतिः_गणनम् >> 1));
    यदि (विस्तृतिः_cache->गणनम् == cache_सीमा) {
      ध्रुव size_t transfer_सीमा = 2 + (cache_सीमा >> 2);
      ध्रुव size_t transfer_गणनम् =
          (THREAD_विस्तृतिः_LARGE_CACHE_TRANSFER <= transfer_सीमा
               ? THREAD_विस्तृतिः_LARGE_CACHE_TRANSFER
               : transfer_सीमा);
      ध्रुव size_t remain_गणनम् = cache_सीमा - transfer_गणनम्;
#यदि ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&heap->thread_गन्तव्यम्_global,
                           transfer_गणनम् * विस्तृतिः_गणनम् * स्मृतिः_विस्तृतिः_आकार);
      rpmalloc_stat_सङ्कलय(&heap->विस्तृतिः_use[विस्तृतिः_गणनम् - 1].spans_गन्तव्यम्_global,
                         transfer_गणनम्);
      rpmalloc_global_cache_निवेशय_spans(विस्तृतिः_cache->विस्तृतिः + remain_गणनम्,
                                          विस्तृतिः_गणनम्, transfer_गणनम्);
#अन्यथा
      प्रति (size_t ispan = 0; ispan < transfer_गणनम्; ++ispan)
        rpmalloc_विस्तृतिः_unmap(विस्तृतिः_cache->विस्तृतिः[remain_गणनम् + ispan]);
#यदि_अन्तः
      विस्तृतिः_cache->गणनम् = remain_गणनम्;
    }
  }
#अन्यथा
  (शून्य)परिमाण(heap);
  rpmalloc_विस्तृतिः_unmap(विस्तृतिः);
#यदि_अन्तः
}

//! Extract the given number of spans from the different cache levels
स्थिर विस्तृतिः_t *_rpmalloc_heap_thread_cache_extract(heap_t *heap,
                                                   size_t विस्तृतिः_गणनम्) {
  विस्तृतिः_t *विस्तृतिः = 0;
#यदि ENABLE_THREAD_CACHE
  विस्तृतिः_cache_t *विस्तृतिः_cache;
  यदि (विस्तृतिः_गणनम् == 1)
    विस्तृतिः_cache = &heap->विस्तृतिः_cache;
  अन्यथा
    विस्तृतिः_cache = (विस्तृतिः_cache_t *)(heap->विस्तृतिः_large_cache + (विस्तृतिः_गणनम् - 2));
  यदि (विस्तृतिः_cache->गणनम्) {
    _rpmalloc_stat_inc(&heap->विस्तृतिः_use[विस्तृतिः_गणनम् - 1].spans_from_cache);
    प्रत्यागम विस्तृतिः_cache->विस्तृतिः[--विस्तृतिः_cache->गणनम्];
  }
#यदि_अन्तः
  प्रत्यागम विस्तृतिः;
}

स्थिर विस्तृतिः_t *_rpmalloc_heap_thread_cache_deferred_extract(heap_t *heap,
                                                            size_t विस्तृतिः_गणनम्) {
  विस्तृतिः_t *विस्तृतिः = 0;
  यदि (विस्तृतिः_गणनम् == 1) {
    _rpmalloc_heap_cache_adopt_deferred(heap, &विस्तृतिः);
  } अन्यथा {
    _rpmalloc_heap_cache_adopt_deferred(heap, 0);
    विस्तृतिः = _rpmalloc_heap_thread_cache_extract(heap, विस्तृतिः_गणनम्);
  }
  प्रत्यागम विस्तृतिः;
}

स्थिर विस्तृतिः_t *_rpmalloc_heap_reserved_extract(heap_t *heap,
                                               size_t विस्तृतिः_गणनम्) {
  यदि (heap->spans_reserved >= विस्तृतिः_गणनम्)
    प्रत्यागम rpmalloc_विस्तृतिः_map(heap, विस्तृतिः_गणनम्);
  प्रत्यागम 0;
}

//! Extract a span from the global cache
स्थिर विस्तृतिः_t *_rpmalloc_heap_global_cache_extract(heap_t *heap,
                                                   size_t विस्तृतिः_गणनम्) {
#यदि ENABLE_GLOBAL_CACHE
#यदि ENABLE_THREAD_CACHE
  विस्तृतिः_cache_t *विस्तृतिः_cache;
  size_t wanted_गणनम्;
  यदि (विस्तृतिः_गणनम् == 1) {
    विस्तृतिः_cache = &heap->विस्तृतिः_cache;
    wanted_गणनम् = THREAD_विस्तृतिः_CACHE_TRANSFER;
  } अन्यथा {
    विस्तृतिः_cache = (विस्तृतिः_cache_t *)(heap->विस्तृतिः_large_cache + (विस्तृतिः_गणनम् - 2));
    wanted_गणनम् = THREAD_विस्तृतिः_LARGE_CACHE_TRANSFER;
  }
  विस्तृतिः_cache->गणनम् = _rpmalloc_global_cache_extract_spans(
      विस्तृतिः_cache->विस्तृतिः, विस्तृतिः_गणनम्, wanted_गणनम्);
  यदि (विस्तृतिः_cache->गणनम्) {
    _rpmalloc_stat_add64(&heap->global_गन्तव्यम्_thread,
                         विस्तृतिः_गणनम् * विस्तृतिः_cache->गणनम् * स्मृतिः_विस्तृतिः_आकार);
    rpmalloc_stat_सङ्कलय(&heap->विस्तृतिः_use[विस्तृतिः_गणनम् - 1].spans_from_global,
                       विस्तृतिः_cache->गणनम्);
    प्रत्यागम विस्तृतिः_cache->विस्तृतिः[--विस्तृतिः_cache->गणनम्];
  }
#अन्यथा
  विस्तृतिः_t *विस्तृतिः = 0;
  size_t गणनम् = _rpmalloc_global_cache_extract_spans(&विस्तृतिः, विस्तृतिः_गणनम्, 1);
  यदि (गणनम्) {
    _rpmalloc_stat_add64(&heap->global_गन्तव्यम्_thread,
                         विस्तृतिः_गणनम् * गणनम् * स्मृतिः_विस्तृतिः_आकार);
    rpmalloc_stat_सङ्कलय(&heap->विस्तृतिः_use[विस्तृतिः_गणनम् - 1].spans_from_global,
                       गणनम्);
    प्रत्यागम विस्तृतिः;
  }
#यदि_अन्तः
#यदि_अन्तः
  (शून्य)परिमाण(heap);
  (शून्य)परिमाण(विस्तृतिः_गणनम्);
  प्रत्यागम 0;
}

स्थिर शून्य rpmalloc_inc_विस्तृतिः_statistics(heap_t *heap, size_t विस्तृतिः_गणनम्,
                                          uint32_t class_idx) {
  (शून्य)परिमाण(heap);
  (शून्य)परिमाण(विस्तृतिः_गणनम्);
  (शून्य)परिमाण(class_idx);
#यदि ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)विस्तृतिः_गणनम् - 1;
  uint32_t current_गणनम् =
      (uint32_t)atomic_incr32(&heap->विस्तृतिः_use[idx].current);
  यदि (current_गणनम् > (uint32_t)atomic_load32(&heap->विस्तृतिः_use[idx].उच्चः))
    atomic_store32(&heap->विस्तृतिः_use[idx].उच्चः, (int32_t)current_गणनम्);
  rpmalloc_stat_सङ्कलय_peak(&heap->आकार_class_use[class_idx].spans_current, 1,
                          heap->आकार_class_use[class_idx].spans_peak);
#यदि_अन्तः
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
स्थिर विस्तृतिः_t *
rpmalloc_heap_extract_new_विस्तृतिः(heap_t *heap,
                                heap_आकार_class_t *heap_आकार_class,
                                size_t विस्तृतिः_गणनम्, uint32_t class_idx) {
  विस्तृतिः_t *विस्तृतिः;
#यदि ENABLE_THREAD_CACHE
  यदि (heap_आकार_class && heap_आकार_class->cache) {
    विस्तृतिः = heap_आकार_class->cache;
    heap_आकार_class->cache =
        (heap->विस्तृतिः_cache.गणनम्
             ? heap->विस्तृतिः_cache.विस्तृतिः[--heap->विस्तृतिः_cache.गणनम्]
             : 0);
    rpmalloc_inc_विस्तृतिः_statistics(heap, विस्तृतिः_गणनम्, class_idx);
    प्रत्यागम विस्तृतिः;
  }
#यदि_अन्तः
  (शून्य)परिमाण(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t आधारः_विस्तृतिः_गणनम् = विस्तृतिः_गणनम्;
  size_t सीमा_विस्तृतिः_गणनम् =
      (विस्तृतिः_गणनम् > 2) ? (विस्तृतिः_गणनम् + (विस्तृतिः_गणनम् >> 1)) : विस्तृतिः_गणनम्;
  यदि (सीमा_विस्तृतिः_गणनम् > LARGE_CLASS_गणनम्)
    सीमा_विस्तृतिः_गणनम् = LARGE_CLASS_गणनम्;
  कुरु {
    विस्तृतिः = _rpmalloc_heap_thread_cache_extract(heap, विस्तृतिः_गणनम्);
    यदि (EXPECTED(विस्तृतिः != 0)) {
      _rpmalloc_stat_inc(&heap->आकार_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_विस्तृतिः_statistics(heap, विस्तृतिः_गणनम्, class_idx);
      प्रत्यागम विस्तृतिः;
    }
    विस्तृतिः = _rpmalloc_heap_thread_cache_deferred_extract(heap, विस्तृतिः_गणनम्);
    यदि (EXPECTED(विस्तृतिः != 0)) {
      _rpmalloc_stat_inc(&heap->आकार_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_विस्तृतिः_statistics(heap, विस्तृतिः_गणनम्, class_idx);
      प्रत्यागम विस्तृतिः;
    }
    विस्तृतिः = _rpmalloc_heap_global_cache_extract(heap, विस्तृतिः_गणनम्);
    यदि (EXPECTED(विस्तृतिः != 0)) {
      _rpmalloc_stat_inc(&heap->आकार_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_विस्तृतिः_statistics(heap, विस्तृतिः_गणनम्, class_idx);
      प्रत्यागम विस्तृतिः;
    }
    विस्तृतिः = _rpmalloc_heap_reserved_extract(heap, विस्तृतिः_गणनम्);
    यदि (EXPECTED(विस्तृतिः != 0)) {
      _rpmalloc_stat_inc(&heap->आकार_class_use[class_idx].spans_from_reserved);
      rpmalloc_inc_विस्तृतिः_statistics(heap, विस्तृतिः_गणनम्, class_idx);
      प्रत्यागम विस्तृतिः;
    }
    ++विस्तृतिः_गणनम्;
  } यावत् (विस्तृतिः_गणनम् <= सीमा_विस्तृतिः_गणनम्);
  // Final fallback, map in more virtual memory
  विस्तृतिः = rpmalloc_विस्तृतिः_map(heap, आधारः_विस्तृतिः_गणनम्);
  rpmalloc_inc_विस्तृतिः_statistics(heap, आधारः_विस्तृतिः_गणनम्, class_idx);
  _rpmalloc_stat_inc(&heap->आकार_class_use[class_idx].spans_map_calls);
  प्रत्यागम विस्तृतिः;
}

स्थिर शून्य _rpmalloc_heap_initialize(heap_t *heap) {
  _rpmalloc_memset_const(heap, 0, परिमाण(heap_t));
  // Get a new heap ID
  heap->id = 1 + atomic_incr32(&स्मृतिः_heap_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)heap->id % HEAP_ARRAY_आकार;
  heap->अग्र_heap = स्मृतिः_heaps[list_idx];
  स्मृतिः_heaps[list_idx] = heap;
}

स्थिर शून्य _rpmalloc_heap_orphan(heap_t *heap, पूर्णाङ्क first_class) {
  heap->owner_thread = (uintptr_t)-1;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  heap_t **heap_list =
      (first_class ? &स्मृतिः_first_class_orphan_heaps : &स्मृतिः_orphan_heaps);
#अन्यथा
  (शून्य)परिमाण(first_class);
  heap_t **heap_list = &स्मृतिः_orphan_heaps;
#यदि_अन्तः
  heap->अग्र_orphan = *heap_list;
  *heap_list = heap;
}

//! Allocate a new heap from newly mapped memory pages
स्थिर heap_t *_rpmalloc_heap_allocate_new(शून्य) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t heap_आकार = परिमाण(heap_t);
  size_t aligned_heap_आकार = 16 * ((heap_आकार + 15) / 16);
  size_t request_heap_गणनम् = 16;
  size_t heap_विस्तृतिः_गणनम् = ((aligned_heap_आकार * request_heap_गणनम्) +
                            परिमाण(विस्तृतिः_t) + स्मृतिः_विस्तृतिः_आकार - 1) /
                           स्मृतिः_विस्तृतिः_आकार;
  size_t खण्डम्_आकार_2 = स्मृतिः_विस्तृतिः_आकार * heap_विस्तृतिः_गणनम्;
  size_t विस्तृतिः_गणनम् = heap_विस्तृतिः_गणनम्;
  विस्तृतिः_t *विस्तृतिः = 0;
  // If there are global reserved spans, use these first
  यदि (स्मृतिः_global_reserve_गणनम् >= heap_विस्तृतिः_गणनम्) {
    विस्तृतिः = _rpmalloc_global_get_reserved_spans(heap_विस्तृतिः_गणनम्);
  }
  यदि (!विस्तृतिः) {
    यदि (स्मृतिः_page_आकार > खण्डम्_आकार_2) {
      विस्तृतिः_गणनम् = स्मृतिः_page_आकार / स्मृतिः_विस्तृतिः_आकार;
      खण्डम्_आकार_2 = स्मृतिः_page_आकार;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_heap_गणनम् =
          (खण्डम्_आकार_2 - परिमाण(विस्तृतिः_t)) / aligned_heap_आकार;
      यदि (possible_heap_गणनम् >= (request_heap_गणनम् * 16))
        request_heap_गणनम् *= 16;
      अन्यथा यदि (possible_heap_गणनम् < request_heap_गणनम्)
        request_heap_गणनम् = possible_heap_गणनम्;
      heap_विस्तृतिः_गणनम् = ((aligned_heap_आकार * request_heap_गणनम्) +
                         परिमाण(विस्तृतिः_t) + स्मृतिः_विस्तृतिः_आकार - 1) /
                        स्मृतिः_विस्तृतिः_आकार;
    }

    size_t align_विस्थापनम् = 0;
    विस्तृतिः = (विस्तृतिः_t *)_rpmalloc_mmap(खण्डम्_आकार_2, &align_विस्थापनम्);
    यदि (!विस्तृतिः)
      प्रत्यागम 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    rpmalloc_विस्तृतिः_initialize(विस्तृतिः, विस्तृतिः_गणनम्, heap_विस्तृतिः_गणनम्, align_विस्थापनम्);
  }

  size_t remain_आकार = स्मृतिः_विस्तृतिः_आकार - परिमाण(विस्तृतिः_t);
  heap_t *heap = (heap_t *)pointer_विस्थापनम्(विस्तृतिः, परिमाण(विस्तृतिः_t));
  _rpmalloc_heap_initialize(heap);

  // Put extra heaps as orphans
  size_t अङ्कः_heaps = remain_आकार / aligned_heap_आकार;
  यदि (अङ्कः_heaps < request_heap_गणनम्)
    अङ्कः_heaps = request_heap_गणनम्;
  atomic_store32(&heap->child_गणनम्, (int32_t)अङ्कः_heaps - 1);
  heap_t *extra_heap = (heap_t *)pointer_विस्थापनम्(heap, aligned_heap_आकार);
  यावत् (अङ्कः_heaps > 1) {
    _rpmalloc_heap_initialize(extra_heap);
    extra_heap->master_heap = heap;
    _rpmalloc_heap_orphan(extra_heap, 1);
    extra_heap = (heap_t *)pointer_विस्थापनम्(extra_heap, aligned_heap_आकार);
    --अङ्कः_heaps;
  }

  यदि (विस्तृतिः_गणनम् > heap_विस्तृतिः_गणनम्) {
    // Cap reserved spans
    size_t remain_गणनम् = विस्तृतिः_गणनम् - heap_विस्तृतिः_गणनम्;
    size_t reserve_गणनम् =
        (remain_गणनम् > स्मृतिः_heap_reserve_गणनम् ? स्मृतिः_heap_reserve_गणनम्
                                                   : remain_गणनम्);
    विस्तृतिः_t *remain_विस्तृतिः =
        (विस्तृतिः_t *)pointer_विस्थापनम्(विस्तृतिः, heap_विस्तृतिः_गणनम् * स्मृतिः_विस्तृतिः_आकार);
    rpmalloc_heap_समुच्चयः_reserved_spans(heap, विस्तृतिः, remain_विस्तृतिः, reserve_गणनम्);

    यदि (remain_गणनम् > reserve_गणनम्) {
      // Set to global reserved spans
      remain_विस्तृतिः = (विस्तृतिः_t *)pointer_विस्थापनम्(remain_विस्तृतिः,
                                             reserve_गणनम् * स्मृतिः_विस्तृतिः_आकार);
      reserve_गणनम् = remain_गणनम् - reserve_गणनम्;
      rpmalloc_global_समुच्चयः_reserved_spans(विस्तृतिः, remain_विस्तृतिः, reserve_गणनम्);
    }
  }

  प्रत्यागम heap;
}

स्थिर heap_t *_rpmalloc_heap_extract_orphan(heap_t **heap_list) {
  heap_t *heap = *heap_list;
  *heap_list = (heap ? heap->अग्र_orphan : 0);
  प्रत्यागम heap;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
स्थिर heap_t *_rpmalloc_heap_allocate(पूर्णाङ्क first_class) {
  heap_t *heap = 0;
  यावत् (!atomic_cas32_acquire(&स्मृतिः_global_तालकम्, 1, 0))
    _rpmalloc_spin();
  यदि (first_class == 0)
    heap = _rpmalloc_heap_extract_orphan(&स्मृतिः_orphan_heaps);
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  यदि (!heap)
    heap = _rpmalloc_heap_extract_orphan(&स्मृतिः_first_class_orphan_heaps);
#यदि_अन्तः
  यदि (!heap)
    heap = _rpmalloc_heap_allocate_new();
  atomic_store32_release(&स्मृतिः_global_तालकम्, 0);
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
    प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
      विस्तृतिः_cache_t *विस्तृतिः_cache;
      यदि (!iclass)
        विस्तृतिः_cache = &heap->विस्तृतिः_cache;
      अन्यथा
        विस्तृतिः_cache = (विस्तृतिः_cache_t *)(heap->विस्तृतिः_large_cache + (iclass - 1));
      यदि (!विस्तृतिः_cache->गणनम्)
        अनुवर्त;
#यदि ENABLE_GLOBAL_CACHE
      यदि (heap->finalize) {
        प्रति (size_t ispan = 0; ispan < विस्तृतिः_cache->गणनम्; ++ispan)
          rpmalloc_विस्तृतिः_unmap(विस्तृतिः_cache->विस्तृतिः[ispan]);
      } अन्यथा {
        _rpmalloc_stat_add64(&heap->thread_गन्तव्यम्_global, विस्तृतिः_cache->गणनम् *
                                                          (iclass + 1) *
                                                          स्मृतिः_विस्तृतिः_आकार);
        rpmalloc_stat_सङ्कलय(&heap->विस्तृतिः_use[iclass].spans_गन्तव्यम्_global,
                           विस्तृतिः_cache->गणनम्);
        rpmalloc_global_cache_निवेशय_spans(विस्तृतिः_cache->विस्तृतिः, iclass + 1,
                                            विस्तृतिः_cache->गणनम्);
      }
#अन्यथा
      प्रति (size_t ispan = 0; ispan < विस्तृतिः_cache->गणनम्; ++ispan)
        rpmalloc_विस्तृतिः_unmap(विस्तृतिः_cache->विस्तृतिः[ispan]);
#यदि_अन्तः
      विस्तृतिः_cache->गणनम् = 0;
    }
#यदि_अन्तः
  }

  यदि (get_thread_heap_raw() == heap)
    समुच्चयः_thread_heap(0);

#यदि ENABLE_STATISTICS
  atomic_decr32(&स्मृतिः_सक्रिय_heaps);
  rpmalloc_assert(atomic_load32(&स्मृतिः_सक्रिय_heaps) >= 0,
                  "Still active heaps during finalization");
#यदि_अन्तः

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  यदि (get_thread_id() != _rpmalloc_main_thread_id) {
    यावत् (!atomic_cas32_acquire(&स्मृतिः_global_तालकम्, 1, 0))
      _rpmalloc_spin();
  }
  _rpmalloc_heap_orphan(heap, first_class);
  atomic_store32_release(&स्मृतिः_global_तालकम्, 0);
}

स्थिर शून्य _rpmalloc_heap_release_raw(शून्य *heapptr, पूर्णाङ्क release_cache) {
  _rpmalloc_heap_release(heapptr, 0, release_cache);
}

स्थिर शून्य _rpmalloc_heap_release_raw_fc(शून्य *heapptr) {
  _rpmalloc_heap_release_raw(heapptr, 1);
}

स्थिर शून्य _rpmalloc_heap_finalize(heap_t *heap) {
  यदि (heap->spans_reserved) {
    विस्तृतिः_t *विस्तृतिः = rpmalloc_विस्तृतिः_map(heap, heap->spans_reserved);
    rpmalloc_विस्तृतिः_unmap(विस्तृतिः);
    heap->spans_reserved = 0;
  }

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  प्रति (size_t iclass = 0; iclass < आकार_CLASS_गणनम्; ++iclass) {
    यदि (heap->आकार_class[iclass].cache)
      rpmalloc_विस्तृतिः_unmap(heap->आकार_class[iclass].cache);
    heap->आकार_class[iclass].cache = 0;
    विस्तृतिः_t *विस्तृतिः = heap->आकार_class[iclass].partial_विस्तृतिः;
    यावत् (विस्तृतिः) {
      विस्तृतिः_t *अग्र = विस्तृतिः->अग्र;
      rpmalloc_विस्तृतिः_finalize(heap, iclass, विस्तृतिः,
                              &heap->आकार_class[iclass].partial_विस्तृतिः);
      विस्तृतिः = अग्र;
    }
    // If class still has a free list it must be a full span
    यदि (heap->आकार_class[iclass].free_list) {
      विस्तृतिः_t *class_विस्तृतिः =
          (विस्तृतिः_t *)((uintptr_t)heap->आकार_class[iclass].free_list &
                     स्मृतिः_विस्तृतिः_आवरणम्);
      विस्तृतिः_t **list = 0;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
      list = &heap->पूर्णम्_विस्तृतिः[iclass];
#यदि_अन्तः
      --heap->पूर्णम्_विस्तृतिः_गणनम्;
      यदि (!rpmalloc_विस्तृतिः_finalize(heap, iclass, class_विस्तृतिः, list)) {
        यदि (list)
          rpmalloc_विस्तृतिः_double_link_list_remove(list, class_विस्तृतिः);
        rpmalloc_विस्तृतिः_double_link_list_सङ्कलय(
            &heap->आकार_class[iclass].partial_विस्तृतिः, class_विस्तृतिः);
      }
    }
  }

#यदि ENABLE_THREAD_CACHE
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    विस्तृतिः_cache_t *विस्तृतिः_cache;
    यदि (!iclass)
      विस्तृतिः_cache = &heap->विस्तृतिः_cache;
    अन्यथा
      विस्तृतिः_cache = (विस्तृतिः_cache_t *)(heap->विस्तृतिः_large_cache + (iclass - 1));
    प्रति (size_t ispan = 0; ispan < विस्तृतिः_cache->गणनम्; ++ispan)
      rpmalloc_विस्तृतिः_unmap(विस्तृतिः_cache->विस्तृतिः[ispan]);
    विस्तृतिः_cache->गणनम् = 0;
  }
#यदि_अन्तः
  rpmalloc_assert(!atomic_आरोपय_ptr(&heap->विस्तृतिः_free_deferred),
                  "Heaps still active during finalization");
}

////////////
///
/// Allocation entry points
///
//////

//! Pop first block from a free list
स्थिर शून्य *free_list_निष्कासय(शून्य **list) {
  शून्य *खण्डम् = *list;
  *list = *((शून्य **)खण्डम्);
  प्रत्यागम खण्डम्;
}

//! Allocate a small/medium sized memory block from the given heap
स्थिर शून्य *_rpmalloc_allocate_from_heap_fallback(
    heap_t *heap, heap_आकार_class_t *heap_आकार_class, uint32_t class_idx) {
  विस्तृतिः_t *विस्तृतिः = heap_आकार_class->partial_विस्तृतिः;
  rpmalloc_assume(heap != 0);
  यदि (EXPECTED(विस्तृतिः != 0)) {
    rpmalloc_assert(विस्तृतिः->खण्डम्_गणनम् ==
                        स्मृतिः_आकार_class[विस्तृतिः->आकार_class].खण्डम्_गणनम्,
                    "Span block count corrupted");
    rpmalloc_assert(!rpmalloc_विस्तृतिः_is_fully_utilized(विस्तृतिः),
                    "Internal failure");
    शून्य *खण्डम्;
    यदि (विस्तृतिः->free_list) {
      // Span local free list is not empty, swap to size class free list
      खण्डम् = free_list_निष्कासय(&विस्तृतिः->free_list);
      heap_आकार_class->free_list = विस्तृतिः->free_list;
      विस्तृतिः->free_list = 0;
    } अन्यथा {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      शून्य *खण्डम्_start = pointer_विस्थापनम्(
          विस्तृतिः, विस्तृतिः_शीर्षकम्_आकार +
                    ((size_t)विस्तृतिः->free_list_सीमा * विस्तृतिः->खण्डम्_आकार_2));
      विस्तृतिः->free_list_सीमा += free_list_partial_init(
          &heap_आकार_class->free_list, &खण्डम्,
          (शून्य *)((uintptr_t)खण्डम्_start & ~(स्मृतिः_page_आकार - 1)),
          खण्डम्_start, विस्तृतिः->खण्डम्_गणनम् - विस्तृतिः->free_list_सीमा,
          विस्तृतिः->खण्डम्_आकार_2);
    }
    rpmalloc_assert(विस्तृतिः->free_list_सीमा <= विस्तृतिः->खण्डम्_गणनम्,
                    "Span block count corrupted");
    विस्तृतिः->प्रयुक्तम्_गणनम् = विस्तृतिः->free_list_सीमा;

    // Swap in deferred free list if present
    यदि (atomic_आरोपय_ptr(&विस्तृतिः->free_list_deferred))
      rpmalloc_विस्तृतिः_extract_free_list_deferred(विस्तृतिः);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    यदि (!rpmalloc_विस्तृतिः_is_fully_utilized(विस्तृतिः))
      प्रत्यागम खण्डम्;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_विस्तृतिः_double_link_list_निष्कासय_शीर्ष(&heap_आकार_class->partial_विस्तृतिः,
                                             विस्तृतिः);
#यदि RPMALLOC_FIRST_CLASS_HEAPS
    rpmalloc_विस्तृतिः_double_link_list_सङ्कलय(&heap->पूर्णम्_विस्तृतिः[class_idx], विस्तृतिः);
#यदि_अन्तः
    ++heap->पूर्णम्_विस्तृतिः_गणनम्;
    प्रत्यागम खण्डम्;
  }

  // Find a span in one of the cache levels
  विस्तृतिः = rpmalloc_heap_extract_new_विस्तृतिः(heap, heap_आकार_class, 1, class_idx);
  यदि (EXPECTED(विस्तृतिः != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    प्रत्यागम rpmalloc_विस्तृतिः_initialize_new(heap, heap_आकार_class, विस्तृतिः,
                                         class_idx);
  }

  प्रत्यागम 0;
}

//! Allocate a small sized memory block from the given heap
स्थिर शून्य *_rpmalloc_allocate_small(heap_t *heap, size_t आकार_2) {
  rpmalloc_assert(heap, "No thread heap");
  // Small sizes have unique size classes
  ध्रुव uint32_t class_idx =
      (uint32_t)((आकार_2 + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_सरणम्);
  heap_आकार_class_t *heap_आकार_class = heap->आकार_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  यदि (EXPECTED(heap_आकार_class->free_list != 0))
    प्रत्यागम free_list_निष्कासय(&heap_आकार_class->free_list);
  प्रत्यागम _rpmalloc_allocate_from_heap_fallback(heap, heap_आकार_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
स्थिर शून्य *_rpmalloc_allocate_medium(heap_t *heap, size_t आकार_2) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ध्रुव uint32_t आधारः_idx =
      (uint32_t)(SMALL_CLASS_गणनम् +
                 ((आकार_2 - (SMALL_आकार_सीमा + 1)) >> MEDIUM_GRANULARITY_सरणम्));
  ध्रुव uint32_t class_idx = स्मृतिः_आकार_class[आधारः_idx].class_idx;
  heap_आकार_class_t *heap_आकार_class = heap->आकार_class + class_idx;
  _rpmalloc_stat_inc_alloc(heap, class_idx);
  यदि (EXPECTED(heap_आकार_class->free_list != 0))
    प्रत्यागम free_list_निष्कासय(&heap_आकार_class->free_list);
  प्रत्यागम _rpmalloc_allocate_from_heap_fallback(heap, heap_आकार_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
स्थिर शून्य *_rpmalloc_allocate_large(heap_t *heap, size_t आकार_2) {
  rpmalloc_assert(heap, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  आकार_2 += विस्तृतिः_शीर्षकम्_आकार;
  size_t विस्तृतिः_गणनम् = आकार_2 >> स्मृतिः_विस्तृतिः_आकार_सरणम्;
  यदि (आकार_2 & (स्मृतिः_विस्तृतिः_आकार - 1))
    ++विस्तृतिः_गणनम्;

  // Find a span in one of the cache levels
  विस्तृतिः_t *विस्तृतिः =
      rpmalloc_heap_extract_new_विस्तृतिः(heap, 0, विस्तृतिः_गणनम्, आकार_CLASS_LARGE);
  यदि (!विस्तृतिः)
    प्रत्यागम विस्तृतिः;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(विस्तृतिः->विस्तृतिः_गणनम् >= विस्तृतिः_गणनम्, "Internal failure");
  विस्तृतिः->आकार_class = आकार_CLASS_LARGE;
  विस्तृतिः->heap = heap;

#यदि RPMALLOC_FIRST_CLASS_HEAPS
  rpmalloc_विस्तृतिः_double_link_list_सङ्कलय(&heap->large_huge_विस्तृतिः, विस्तृतिः);
#यदि_अन्तः
  ++heap->पूर्णम्_विस्तृतिः_गणनम्;

  प्रत्यागम pointer_विस्थापनम्(विस्तृतिः, विस्तृतिः_शीर्षकम्_आकार);
}

//! Allocate a huge block by mapping memory pages directly
स्थिर शून्य *_rpmalloc_allocate_huge(heap_t *heap, size_t आकार_2) {
  rpmalloc_assert(heap, "No thread heap");
  _rpmalloc_heap_cache_adopt_deferred(heap, 0);
  आकार_2 += विस्तृतिः_शीर्षकम्_आकार;
  size_t अङ्कः_पृष्ठानि = आकार_2 >> स्मृतिः_page_आकार_सरणम्;
  यदि (आकार_2 & (स्मृतिः_page_आकार - 1))
    ++अङ्कः_पृष्ठानि;
  size_t align_विस्थापनम् = 0;
  विस्तृतिः_t *विस्तृतिः =
      (विस्तृतिः_t *)_rpmalloc_mmap(अङ्कः_पृष्ठानि * स्मृतिः_page_आकार, &align_विस्थापनम्);
  यदि (!विस्तृतिः)
    प्रत्यागम विस्तृतिः;

  // Store page count in span_count
  विस्तृतिः->आकार_class = आकार_CLASS_HUGE;
  विस्तृतिः->विस्तृतिः_गणनम् = (uint32_t)अङ्कः_पृष्ठानि;
  विस्तृतिः->align_विस्थापनम् = (uint32_t)align_विस्थापनम्;
  विस्तृतिः->heap = heap;
  rpmalloc_stat_सङ्कलय_peak(&huge_पृष्ठानि_current, अङ्कः_पृष्ठानि, huge_पृष्ठानि_peak);

#यदि RPMALLOC_FIRST_CLASS_HEAPS
  rpmalloc_विस्तृतिः_double_link_list_सङ्कलय(&heap->large_huge_विस्तृतिः, विस्तृतिः);
#यदि_अन्तः
  ++heap->पूर्णम्_विस्तृतिः_गणनम्;

  प्रत्यागम pointer_विस्थापनम्(विस्तृतिः, विस्तृतिः_शीर्षकम्_आकार);
}

//! Allocate a block of the given size
स्थिर शून्य *_rpmalloc_allocate(heap_t *heap, size_t आकार_2) {
  _rpmalloc_stat_add64(&आवण्टनम्_गणक, 1);
  यदि (EXPECTED(आकार_2 <= SMALL_आकार_सीमा))
    प्रत्यागम _rpmalloc_allocate_small(heap, आकार_2);
  अन्यथा यदि (आकार_2 <= स्मृतिः_medium_आकार_सीमा)
    प्रत्यागम _rpmalloc_allocate_medium(heap, आकार_2);
  अन्यथा यदि (आकार_2 <= LARGE_आकार_सीमा)
    प्रत्यागम _rpmalloc_allocate_large(heap, आकार_2);
  प्रत्यागम _rpmalloc_allocate_huge(heap, आकार_2);
}

स्थिर शून्य *_rpmalloc_aligned_allocate(heap_t *heap, size_t alignment,
                                        size_t आकार_2) {
  यदि (alignment <= SMALL_GRANULARITY)
    प्रत्यागम _rpmalloc_allocate(heap, आकार_2);

#यदि ENABLE_VALIDATE_तर्काः
  यदि ((आकार_2 + alignment) < आकार_2) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
  यदि (alignment & (alignment - 1)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#यदि_अन्तः

  यदि ((alignment <= विस्तृतिः_शीर्षकम्_आकार) &&
      ((आकार_2 + विस्तृतिः_शीर्षकम्_आकार) < स्मृतिः_medium_आकार_सीमा)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t गुणजः_आकार = आकार_2 ? (आकार_2 + (विस्तृतिः_शीर्षकम्_आकार - 1)) &
                                      ~(uintptr_t)(विस्तृतिः_शीर्षकम्_आकार - 1)
                                : विस्तृतिः_शीर्षकम्_आकार;
    rpmalloc_assert(!(गुणजः_आकार % विस्तृतिः_शीर्षकम्_आकार),
                    "Failed alignment calculation");
    यदि (गुणजः_आकार <= (आकार_2 + alignment))
      प्रत्यागम _rpmalloc_allocate(heap, गुणजः_आकार);
  }

  शून्य *ptr = 0;
  size_t align_आवरणम् = alignment - 1;
  यदि (alignment <= स्मृतिः_page_आकार) {
    ptr = _rpmalloc_allocate(heap, आकार_2 + alignment);
    यदि ((uintptr_t)ptr & align_आवरणम्) {
      ptr = (शून्य *)(((uintptr_t)ptr & ~(uintptr_t)align_आवरणम्) + alignment);
      // Mark as having aligned blocks
      विस्तृतिः_t *विस्तृतिः = (विस्तृतिः_t *)((uintptr_t)ptr & स्मृतिः_विस्तृतिः_आवरणम्);
      विस्तृतिः->flags |= विस्तृतिः_FLAG_ALIGNED_खण्डानि;
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
  यदि (alignment & align_आवरणम्) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
  यदि (alignment >= स्मृतिः_विस्तृतिः_आकार) {
    errno = EINVAL;
    प्रत्यागम 0;
  }

  size_t extra_पृष्ठानि = alignment / स्मृतिः_page_आकार;

  // Since each span has a header, we will at least need one extra memory page
  size_t अङ्कः_पृष्ठानि = 1 + (आकार_2 / स्मृतिः_page_आकार);
  यदि (आकार_2 & (स्मृतिः_page_आकार - 1))
    ++अङ्कः_पृष्ठानि;

  यदि (extra_पृष्ठानि > अङ्कः_पृष्ठानि)
    अङ्कः_पृष्ठानि = 1 + extra_पृष्ठानि;

  size_t original_पृष्ठानि = अङ्कः_पृष्ठानि;
  size_t सीमा_पृष्ठानि = (स्मृतिः_विस्तृतिः_आकार / स्मृतिः_page_आकार) * 2;
  यदि (सीमा_पृष्ठानि < (original_पृष्ठानि * 2))
    सीमा_पृष्ठानि = original_पृष्ठानि * 2;

  size_t mapped_आकार, align_विस्थापनम्;
  विस्तृतिः_t *विस्तृतिः;

retry:
  align_विस्थापनम् = 0;
  mapped_आकार = अङ्कः_पृष्ठानि * स्मृतिः_page_आकार;

  विस्तृतिः = (विस्तृतिः_t *)_rpmalloc_mmap(mapped_आकार, &align_विस्थापनम्);
  यदि (!विस्तृतिः) {
    errno = ENOMEM;
    प्रत्यागम 0;
  }
  ptr = pointer_विस्थापनम्(विस्तृतिः, विस्तृतिः_शीर्षकम्_आकार);

  यदि ((uintptr_t)ptr & align_आवरणम्)
    ptr = (शून्य *)(((uintptr_t)ptr & ~(uintptr_t)align_आवरणम्) + alignment);

  यदि (((size_t)pointer_diff(ptr, विस्तृतिः) >= स्मृतिः_विस्तृतिः_आकार) ||
      (pointer_विस्थापनम्(ptr, आकार_2) > pointer_विस्थापनम्(विस्तृतिः, mapped_आकार)) ||
      (((uintptr_t)ptr & स्मृतिः_विस्तृतिः_आवरणम्) != (uintptr_t)विस्तृतिः)) {
    _rpmalloc_unmap(विस्तृतिः, mapped_आकार, align_विस्थापनम्, mapped_आकार);
    ++अङ्कः_पृष्ठानि;
    यदि (अङ्कः_पृष्ठानि > सीमा_पृष्ठानि) {
      errno = EINVAL;
      प्रत्यागम 0;
    }
    गच्छ retry;
  }

  // Store page count in span_count
  विस्तृतिः->आकार_class = आकार_CLASS_HUGE;
  विस्तृतिः->विस्तृतिः_गणनम् = (uint32_t)अङ्कः_पृष्ठानि;
  विस्तृतिः->align_विस्थापनम् = (uint32_t)align_विस्थापनम्;
  विस्तृतिः->heap = heap;
  rpmalloc_stat_सङ्कलय_peak(&huge_पृष्ठानि_current, अङ्कः_पृष्ठानि, huge_पृष्ठानि_peak);

#यदि RPMALLOC_FIRST_CLASS_HEAPS
  rpmalloc_विस्तृतिः_double_link_list_सङ्कलय(&heap->large_huge_विस्तृतिः, विस्तृतिः);
#यदि_अन्तः
  ++heap->पूर्णम्_विस्तृतिः_गणनम्;

  _rpmalloc_stat_add64(&आवण्टनम्_गणक, 1);

  प्रत्यागम ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
स्थिर शून्य rpmalloc_deallocate_प्रत्यक्षम्_small_or_medium(विस्तृतिः_t *विस्तृतिः,
                                                        शून्य *खण्डम्) {
  heap_t *heap = विस्तृतिः->heap;
  rpmalloc_assert(heap->owner_thread == get_thread_id() ||
                      !heap->owner_thread || heap->finalize,
                  "Internal failure");
  // Add block to free list
  यदि (UNEXPECTED(rpmalloc_विस्तृतिः_is_fully_utilized(विस्तृतिः))) {
    विस्तृतिः->प्रयुक्तम्_गणनम् = विस्तृतिः->खण्डम्_गणनम्;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
    rpmalloc_विस्तृतिः_double_link_list_remove(&heap->पूर्णम्_विस्तृतिः[विस्तृतिः->आकार_class],
                                           विस्तृतिः);
#यदि_अन्तः
    rpmalloc_विस्तृतिः_double_link_list_सङ्कलय(
        &heap->आकार_class[विस्तृतिः->आकार_class].partial_विस्तृतिः, विस्तृतिः);
    --heap->पूर्णम्_विस्तृतिः_गणनम्;
  }
  *((शून्य **)खण्डम्) = विस्तृतिः->free_list;
  --विस्तृतिः->प्रयुक्तम्_गणनम्;
  विस्तृतिः->free_list = खण्डम्;
  यदि (UNEXPECTED(विस्तृतिः->प्रयुक्तम्_गणनम् == विस्तृतिः->list_आकार)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    यदि (विस्तृतिः->प्रयुक्तम्_गणनम्) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      शून्य *free_list;
      कुरु {
        free_list = atomic_exchange_ptr_acquire(&विस्तृतिः->free_list_deferred,
                                                INVALID_POINTER);
      } यावत् (free_list == INVALID_POINTER);
      atomic_store_ptr_release(&विस्तृतिः->free_list_deferred, free_list);
    }
    rpmalloc_विस्तृतिः_double_link_list_remove(
        &heap->आकार_class[विस्तृतिः->आकार_class].partial_विस्तृतिः, विस्तृतिः);
    rpmalloc_विस्तृतिः_release_गन्तव्यम्_cache(heap, विस्तृतिः);
  }
}

स्थिर शून्य rpmalloc_deallocate_defer_free_विस्तृतिः(heap_t *heap, विस्तृतिः_t *विस्तृतिः) {
  यदि (विस्तृतिः->आकार_class != आकार_CLASS_HUGE)
    _rpmalloc_stat_inc(&heap->विस्तृतिः_use[विस्तृतिः->विस्तृतिः_गणनम् - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  कुरु {
    विस्तृतिः->free_list = (शून्य *)atomic_आरोपय_ptr(&heap->विस्तृतिः_free_deferred);
  } यावत् (!atomic_cas_ptr(&heap->विस्तृतिः_free_deferred, विस्तृतिः, विस्तृतिः->free_list));
}

//! Put the block in the deferred free list of the owning span
स्थिर शून्य _rpmalloc_deallocate_defer_small_or_medium(विस्तृतिः_t *विस्तृतिः,
                                                       शून्य *खण्डम्) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  शून्य *free_list;
  कुरु {
    free_list =
        atomic_exchange_ptr_acquire(&विस्तृतिः->free_list_deferred, INVALID_POINTER);
  } यावत् (free_list == INVALID_POINTER);
  *((शून्य **)खण्डम्) = free_list;
  uint32_t free_गणनम् = ++विस्तृतिः->list_आकार;
  पूर्णाङ्क all_deferred_free = (free_गणनम् == विस्तृतिः->खण्डम्_गणनम्);
  atomic_store_ptr_release(&विस्तृतिः->free_list_deferred, खण्डम्);
  यदि (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    rpmalloc_deallocate_defer_free_विस्तृतिः(विस्तृतिः->heap, विस्तृतिः);
  }
}

स्थिर शून्य _rpmalloc_deallocate_small_or_medium(विस्तृतिः_t *विस्तृतिः, शून्य *p) {
  _rpmalloc_stat_inc_free(विस्तृतिः->heap, विस्तृतिः->आकार_class);
  यदि (विस्तृतिः->flags & विस्तृतिः_FLAG_ALIGNED_खण्डानि) {
    // Realign pointer to block start
    शून्य *खण्डानि_start = pointer_विस्थापनम्(विस्तृतिः, विस्तृतिः_शीर्षकम्_आकार);
    uint32_t खण्डम्_विस्थापनम् = (uint32_t)pointer_diff(p, खण्डानि_start);
    p = pointer_विस्थापनम्(p, -(int32_t)(खण्डम्_विस्थापनम् % विस्तृतिः->खण्डम्_आकार_2));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  पूर्णाङ्क defer =
      (विस्तृतिः->heap->owner_thread &&
       (विस्तृतिः->heap->owner_thread != get_thread_id()) && !विस्तृतिः->heap->finalize);
#अन्यथा
  पूर्णाङ्क defer =
      ((विस्तृतिः->heap->owner_thread != get_thread_id()) && !विस्तृतिः->heap->finalize);
#यदि_अन्तः
  यदि (!defer)
    rpmalloc_deallocate_प्रत्यक्षम्_small_or_medium(विस्तृतिः, p);
  अन्यथा
    _rpmalloc_deallocate_defer_small_or_medium(विस्तृतिः, p);
}

//! Deallocate the given large memory block to the current heap
स्थिर शून्य _rpmalloc_deallocate_large(विस्तृतिः_t *विस्तृतिः) {
  rpmalloc_assert(विस्तृतिः->आकार_class == आकार_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(विस्तृतिः->flags & विस्तृतिः_FLAG_MASTER) ||
                      !(विस्तृतिः->flags & विस्तृतिः_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((विस्तृतिः->flags & विस्तृतिः_FLAG_MASTER) ||
                      (विस्तृतिः->flags & विस्तृतिः_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  पूर्णाङ्क defer =
      (विस्तृतिः->heap->owner_thread &&
       (विस्तृतिः->heap->owner_thread != get_thread_id()) && !विस्तृतिः->heap->finalize);
#अन्यथा
  पूर्णाङ्क defer =
      ((विस्तृतिः->heap->owner_thread != get_thread_id()) && !विस्तृतिः->heap->finalize);
#यदि_अन्तः
  यदि (defer) {
    rpmalloc_deallocate_defer_free_विस्तृतिः(विस्तृतिः->heap, विस्तृतिः);
    प्रत्यागम;
  }
  rpmalloc_assert(विस्तृतिः->heap->पूर्णम्_विस्तृतिः_गणनम्, "Heap span counter corrupted");
  --विस्तृतिः->heap->पूर्णम्_विस्तृतिः_गणनम्;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  rpmalloc_विस्तृतिः_double_link_list_remove(&विस्तृतिः->heap->large_huge_विस्तृतिः, विस्तृतिः);
#यदि_अन्तः
#यदि ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = विस्तृतिः->विस्तृतिः_गणनम् - 1;
  atomic_decr32(&विस्तृतिः->heap->विस्तृतिः_use[idx].current);
#यदि_अन्तः
  heap_t *heap = विस्तृतिः->heap;
  rpmalloc_assert(heap, "No thread heap");
#यदि ENABLE_THREAD_CACHE
  ध्रुव पूर्णाङ्क समुच्चयः_as_reserved =
      ((विस्तृतिः->विस्तृतिः_गणनम् > 1) && (heap->विस्तृतिः_cache.गणनम् == 0) &&
       !heap->finalize && !heap->spans_reserved);
#अन्यथा
  ध्रुव पूर्णाङ्क समुच्चयः_as_reserved =
      ((विस्तृतिः->विस्तृतिः_गणनम् > 1) && !heap->finalize && !heap->spans_reserved);
#यदि_अन्तः
  यदि (समुच्चयः_as_reserved) {
    heap->विस्तृतिः_reserve = विस्तृतिः;
    heap->spans_reserved = विस्तृतिः->विस्तृतिः_गणनम्;
    यदि (विस्तृतिः->flags & विस्तृतिः_FLAG_MASTER) {
      heap->विस्तृतिः_reserve_master = विस्तृतिः;
    } अन्यथा { // SPAN_FLAG_SUBSPAN
      विस्तृतिः_t *master = (विस्तृतिः_t *)pointer_विस्थापनम्(
          विस्तृतिः,
          -(intptr_t)((size_t)विस्तृतिः->विस्थापनम्_from_master * स्मृतिः_विस्तृतिः_आकार));
      heap->विस्तृतिः_reserve_master = master;
      rpmalloc_assert(master->flags & विस्तृतिः_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->remaining_spans) >=
                          (int32_t)विस्तृतिः->विस्तृतिः_गणनम्,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&heap->विस्तृतिः_use[idx].spans_गन्तव्यम्_reserved);
  } अन्यथा {
    // Insert into cache list
    rpmalloc_heap_cache_निवेशय(heap, विस्तृतिः);
  }
}

//! Deallocate the given huge span
स्थिर शून्य _rpmalloc_deallocate_huge(विस्तृतिः_t *विस्तृतिः) {
  rpmalloc_assert(विस्तृतिः->heap, "No span heap");
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  पूर्णाङ्क defer =
      (विस्तृतिः->heap->owner_thread &&
       (विस्तृतिः->heap->owner_thread != get_thread_id()) && !विस्तृतिः->heap->finalize);
#अन्यथा
  पूर्णाङ्क defer =
      ((विस्तृतिः->heap->owner_thread != get_thread_id()) && !विस्तृतिः->heap->finalize);
#यदि_अन्तः
  यदि (defer) {
    rpmalloc_deallocate_defer_free_विस्तृतिः(विस्तृतिः->heap, विस्तृतिः);
    प्रत्यागम;
  }
  rpmalloc_assert(विस्तृतिः->heap->पूर्णम्_विस्तृतिः_गणनम्, "Heap span counter corrupted");
  --विस्तृतिः->heap->पूर्णम्_विस्तृतिः_गणनम्;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  rpmalloc_विस्तृतिः_double_link_list_remove(&विस्तृतिः->heap->large_huge_विस्तृतिः, विस्तृतिः);
#यदि_अन्तः

  // Oversized allocation, page count is stored in span_count
  size_t अङ्कः_पृष्ठानि = विस्तृतिः->विस्तृतिः_गणनम्;
  _rpmalloc_unmap(विस्तृतिः, अङ्कः_पृष्ठानि * स्मृतिः_page_आकार, विस्तृतिः->align_विस्थापनम्,
                  अङ्कः_पृष्ठानि * स्मृतिः_page_आकार);
  _rpmalloc_stat_sub(&huge_पृष्ठानि_current, अङ्कः_पृष्ठानि);
}

//! Deallocate the given block
स्थिर शून्य _rpmalloc_deallocate(शून्य *p) {
  _rpmalloc_stat_add64(&deallocation_गणक, 1);
  // Grab the span (always at start of span, using span alignment)
  विस्तृतिः_t *विस्तृतिः = (विस्तृतिः_t *)((uintptr_t)p & स्मृतिः_विस्तृतिः_आवरणम्);
  यदि (UNEXPECTED(!विस्तृतिः))
    प्रत्यागम;
  यदि (EXPECTED(विस्तृतिः->आकार_class < आकार_CLASS_गणनम्))
    _rpmalloc_deallocate_small_or_medium(विस्तृतिः, p);
  अन्यथा यदि (विस्तृतिः->आकार_class == आकार_CLASS_LARGE)
    _rpmalloc_deallocate_large(विस्तृतिः);
  अन्यथा
    _rpmalloc_deallocate_huge(विस्तृतिः);
}

////////////
///
/// Reallocation entry points
///
//////

स्थिर size_t rpmalloc_usable_आकार_2(शून्य *p);

//! Reallocate the given block to the given size
स्थिर शून्य *_rpmalloc_reallocate(heap_t *heap, शून्य *p, size_t आकार_2,
                                  size_t oldsize, अचिह्नित पूर्णाङ्क flags) {
  यदि (p) {
    // Grab the span using guaranteed span alignment
    विस्तृतिः_t *विस्तृतिः = (विस्तृतिः_t *)((uintptr_t)p & स्मृतिः_विस्तृतिः_आवरणम्);
    यदि (EXPECTED(विस्तृतिः->आकार_class < आकार_CLASS_गणनम्)) {
      // Small/medium sized block
      rpmalloc_assert(विस्तृतिः->विस्तृतिः_गणनम् == 1, "Span counter corrupted");
      शून्य *खण्डानि_start = pointer_विस्थापनम्(विस्तृतिः, विस्तृतिः_शीर्षकम्_आकार);
      uint32_t खण्डम्_विस्थापनम् = (uint32_t)pointer_diff(p, खण्डानि_start);
      uint32_t खण्डम्_idx = खण्डम्_विस्थापनम् / विस्तृतिः->खण्डम्_आकार_2;
      शून्य *खण्डम् =
          pointer_विस्थापनम्(खण्डानि_start, (size_t)खण्डम्_idx * विस्तृतिः->खण्डम्_आकार_2);
      यदि (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)विस्तृतिः->खण्डम्_आकार_2 - pointer_diff(p, खण्डम्));
      यदि ((size_t)विस्तृतिः->खण्डम्_आकार_2 >= आकार_2) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        यदि ((p != खण्डम्) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(खण्डम्, p, oldsize);
        प्रत्यागम खण्डम्;
      }
    } अन्यथा यदि (विस्तृतिः->आकार_class == आकार_CLASS_LARGE) {
      // Large block
      size_t योग_आकार_2 = आकार_2 + विस्तृतिः_शीर्षकम्_आकार;
      size_t अङ्कः_spans = योग_आकार_2 >> स्मृतिः_विस्तृतिः_आकार_सरणम्;
      यदि (योग_आकार_2 & (स्मृतिः_विस्तृतिः_आवरणम् - 1))
        ++अङ्कः_spans;
      size_t current_spans = विस्तृतिः->विस्तृतिः_गणनम्;
      शून्य *खण्डम् = pointer_विस्थापनम्(विस्तृतिः, विस्तृतिः_शीर्षकम्_आकार);
      यदि (!oldsize)
        oldsize = (current_spans * स्मृतिः_विस्तृतिः_आकार) -
                  (size_t)pointer_diff(p, खण्डम्) - विस्तृतिः_शीर्षकम्_आकार;
      यदि ((current_spans >= अङ्कः_spans) && (योग_आकार_2 >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        यदि ((p != खण्डम्) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(खण्डम्, p, oldsize);
        प्रत्यागम खण्डम्;
      }
    } अन्यथा {
      // Oversized block
      size_t योग_आकार_2 = आकार_2 + विस्तृतिः_शीर्षकम्_आकार;
      size_t अङ्कः_पृष्ठानि = योग_आकार_2 >> स्मृतिः_page_आकार_सरणम्;
      यदि (योग_आकार_2 & (स्मृतिः_page_आकार - 1))
        ++अङ्कः_पृष्ठानि;
      // Page count is stored in span_count
      size_t current_पृष्ठानि = विस्तृतिः->विस्तृतिः_गणनम्;
      शून्य *खण्डम् = pointer_विस्थापनम्(विस्तृतिः, विस्तृतिः_शीर्षकम्_आकार);
      यदि (!oldsize)
        oldsize = (current_पृष्ठानि * स्मृतिः_page_आकार) -
                  (size_t)pointer_diff(p, खण्डम्) - विस्तृतिः_शीर्षकम्_आकार;
      यदि ((current_पृष्ठानि >= अङ्कः_पृष्ठानि) && (अङ्कः_पृष्ठानि >= (current_पृष्ठानि / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        यदि ((p != खण्डम्) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(खण्डम्, p, oldsize);
        प्रत्यागम खण्डम्;
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
  size_t अधस्तनम्_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_आकार =
      (आकार_2 > अधस्तनम्_bound) ? आकार_2 : ((आकार_2 > oldsize) ? अधस्तनम्_bound : आकार_2);
  शून्य *खण्डम् = _rpmalloc_allocate(heap, new_आकार);
  यदि (p && खण्डम्) {
    यदि (!(flags & RPMALLOC_NO_PRESERVE))
      स्मृतिं_प्रतिलिख(खण्डम्, p, oldsize < new_आकार ? oldsize : new_आकार);
    _rpmalloc_deallocate(p);
  }

  प्रत्यागम खण्डम्;
}

स्थिर शून्य *_rpmalloc_aligned_reallocate(heap_t *heap, शून्य *ptr,
                                          size_t alignment, size_t आकार_2,
                                          size_t oldsize, अचिह्नित पूर्णाङ्क flags) {
  यदि (alignment <= SMALL_GRANULARITY)
    प्रत्यागम _rpmalloc_reallocate(heap, ptr, आकार_2, oldsize, flags);

  पूर्णाङ्क no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? rpmalloc_usable_आकार_2(ptr) : 0);
  यदि ((usablesize >= आकार_2) && !((uintptr_t)ptr & (alignment - 1))) {
    यदि (no_alloc || (आकार_2 >= (usablesize / 2)))
      प्रत्यागम ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  शून्य *खण्डम् =
      (!no_alloc ? _rpmalloc_aligned_allocate(heap, alignment, आकार_2) : 0);
  यदि (EXPECTED(खण्डम् != 0)) {
    यदि (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      यदि (!oldsize)
        oldsize = usablesize;
      स्मृतिं_प्रतिलिख(खण्डम्, ptr, oldsize < आकार_2 ? oldsize : आकार_2);
    }
    _rpmalloc_deallocate(ptr);
  }
  प्रत्यागम खण्डम्;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
स्थिर size_t rpmalloc_usable_आकार_2(शून्य *p) {
  // Grab the span using guaranteed span alignment
  विस्तृतिः_t *विस्तृतिः = (विस्तृतिः_t *)((uintptr_t)p & स्मृतिः_विस्तृतिः_आवरणम्);
  यदि (विस्तृतिः->आकार_class < आकार_CLASS_गणनम्) {
    // Small/medium block
    शून्य *खण्डानि_start = pointer_विस्थापनम्(विस्तृतिः, विस्तृतिः_शीर्षकम्_आकार);
    प्रत्यागम विस्तृतिः->खण्डम्_आकार_2 -
           ((size_t)pointer_diff(p, खण्डानि_start) % विस्तृतिः->खण्डम्_आकार_2);
  }
  यदि (विस्तृतिः->आकार_class == आकार_CLASS_LARGE) {
    // Large block
    size_t current_spans = विस्तृतिः->विस्तृतिः_गणनम्;
    प्रत्यागम (current_spans * स्मृतिः_विस्तृतिः_आकार) - (size_t)pointer_diff(p, विस्तृतिः);
  }
  // Oversized block, page count is stored in span_count
  size_t current_पृष्ठानि = विस्तृतिः->विस्तृतिः_गणनम्;
  प्रत्यागम (current_पृष्ठानि * स्मृतिः_page_आकार) - (size_t)pointer_diff(p, विस्तृतिः);
}

//! Adjust and optimize the size class properties for the given class
स्थिर शून्य rpmalloc_adjust_आकार_class(size_t iclass) {
  size_t खण्डम्_आकार_2 = स्मृतिः_आकार_class[iclass].खण्डम्_आकार_2;
  size_t खण्डम्_गणनम् = (स्मृतिः_विस्तृतिः_आकार - विस्तृतिः_शीर्षकम्_आकार) / खण्डम्_आकार_2;

  स्मृतिः_आकार_class[iclass].खण्डम्_गणनम् = (uint16_t)खण्डम्_गणनम्;
  स्मृतिः_आकार_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  यदि (iclass >= SMALL_CLASS_गणनम्) {
    size_t prevclass = iclass;
    यावत् (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      यदि (स्मृतिः_आकार_class[prevclass].खण्डम्_गणनम् ==
          स्मृतिः_आकार_class[iclass].खण्डम्_गणनम्)
        _rpmalloc_memcpy_const(स्मृतिः_आकार_class + prevclass,
                               स्मृतिः_आकार_class + iclass,
                               परिमाण(स्मृतिः_आकार_class[iclass]));
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
    स्मृतिं_प्रतिलिख(&स्मृतिः_config, config, परिमाण(rpmalloc_config_t));
  अन्यथा
    _rpmalloc_memset_const(&स्मृतिः_config, 0, परिमाण(rpmalloc_config_t));

  यदि (!स्मृतिः_config.स्मृतिः_map || !स्मृतिः_config.स्मृतिः_unmap) {
    स्मृतिः_config.स्मृतिः_map = _rpmalloc_mmap_os;
    स्मृतिः_config.स्मृतिः_unmap = _rpmalloc_unmap_os;
  }

#यदि PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  स्मृतिं_पूरय(&system_info, 0, परिमाण(system_info));
  GetSystemInfo(&system_info);
  स्मृतिः_map_granularity = system_info.dw_आवण्टनम्_Granularity;
#अन्यथा
  स्मृतिः_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#यदि_अन्तः

#यदि RPMALLOC_CONFIGURABLE
  स्मृतिः_page_आकार = स्मृतिः_config.page_आकार;
#अन्यथा
  स्मृतिः_page_आकार = 0;
#यदि_अन्तः
  स्मृतिः_huge_पृष्ठानि = 0;
  यदि (!स्मृतिः_page_आकार) {
#यदि PLATFORM_WINDOWS
    स्मृतिः_page_आकार = system_info.dw_Page_आकार;
#अन्यथा
    स्मृतिः_page_आकार = स्मृतिः_map_granularity;
    यदि (स्मृतिः_config.enable_huge_पृष्ठानि) {
#यदि defined(__linux__)
      size_t huge_page_आकार = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      यदि (meminfo) {
        वर्ण पङ्क्तिः_2[128];
        यावत् (!huge_page_आकार && fgets(पङ्क्तिः_2, परिमाण(पङ्क्तिः_2) - 1, meminfo)) {
          पङ्क्तिः_2[परिमाण(पङ्क्तिः_2) - 1] = 0;
          यदि (वाक्ये_अन्विष(पङ्क्तिः_2, "Hugepagesize:"))
            huge_page_आकार = (size_t)strtol(पङ्क्तिः_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      यदि (huge_page_आकार) {
        स्मृतिः_huge_पृष्ठानि = 1;
        स्मृतिः_page_आकार = huge_page_आकार;
        स्मृतिः_map_granularity = huge_page_आकार;
      }
#नोचेद्यदि defined(__FreeBSD__)
      पूर्णाङ्क rc;
      size_t sz = परिमाण(rc);

      यदि (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, रिक्त, 0) == 0 &&
          rc == 1) {
        स्थिर size_t defsize = 2 * 1024 * 1024;
        पूर्णाङ्क nsize = 0;
        size_t sizes[4] = {0};
        स्मृतिः_huge_पृष्ठानि = 1;
        स्मृतिः_page_आकार = defsize;
        यदि ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          प्रति (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            यदि (defsize < csize) {
              स्मृतिः_page_आकार = csize;
              भङ्ग;
            }
          }
        }
        स्मृतिः_map_granularity = स्मृतिः_page_आकार;
      }
#नोचेद्यदि defined(__APPLE__) || defined(__NetBSD__)
      स्मृतिः_huge_पृष्ठानि = 1;
      स्मृतिः_page_आकार = 2 * 1024 * 1024;
      स्मृतिः_map_granularity = स्मृतिः_page_आकार;
#यदि_अन्तः
    }
#यदि_अन्तः
  } अन्यथा {
    यदि (स्मृतिः_config.enable_huge_पृष्ठानि)
      स्मृतिः_huge_पृष्ठानि = 1;
  }

#यदि PLATFORM_WINDOWS
  यदि (स्मृतिः_config.enable_huge_पृष्ठानि) {
    HANDLE token = 0;
    size_t large_page_minimum = GetLargePageMinimum();
    यदि (large_page_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    यदि (token) {
      LUID luid;
      यदि (Lookup_Privilege_मानम्(0, SE_तालकम्_स्मृतिः_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        स्मृतिं_पूरय(&token_privileges, 0, परिमाण(token_privileges));
        token_privileges.Privilege_गणनम् = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].गुणाः = SE_PRIVILEGE_ENABLED;
        यदि (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          यदि (Get_चरमः_Error() == ERROR_SUCCESS)
            स्मृतिः_huge_पृष्ठानि = 1;
        }
      }
      CloseHandle(token);
    }
    यदि (स्मृतिः_huge_पृष्ठानि) {
      यदि (large_page_minimum > स्मृतिः_page_आकार)
        स्मृतिः_page_आकार = large_page_minimum;
      यदि (large_page_minimum > स्मृतिः_map_granularity)
        स्मृतिः_map_granularity = large_page_minimum;
    }
  }
#यदि_अन्तः

  size_t min_विस्तृतिः_आकार = 256;
  size_t max_page_आकार;
#यदि UINTPTR_MAX > 0xFFFFFFFF
  max_page_आकार = 4096ULL * 1024ULL * 1024ULL;
#अन्यथा
  max_page_आकार = 4 * 1024 * 1024;
#यदि_अन्तः
  यदि (स्मृतिः_page_आकार < min_विस्तृतिः_आकार)
    स्मृतिः_page_आकार = min_विस्तृतिः_आकार;
  यदि (स्मृतिः_page_आकार > max_page_आकार)
    स्मृतिः_page_आकार = max_page_आकार;
  स्मृतिः_page_आकार_सरणम् = 0;
  size_t page_आकार_bit = स्मृतिः_page_आकार;
  यावत् (page_आकार_bit != 1) {
    ++स्मृतिः_page_आकार_सरणम्;
    page_आकार_bit >>= 1;
  }
  स्मृतिः_page_आकार = ((size_t)1 << स्मृतिः_page_आकार_सरणम्);

#यदि RPMALLOC_CONFIGURABLE
  यदि (!स्मृतिः_config.विस्तृतिः_आकार) {
    स्मृतिः_विस्तृतिः_आकार = स्मृतिः_default_विस्तृतिः_आकार;
    स्मृतिः_विस्तृतिः_आकार_सरणम् = स्मृतिः_default_विस्तृतिः_आकार_सरणम्;
    स्मृतिः_विस्तृतिः_आवरणम् = स्मृतिः_default_विस्तृतिः_आवरणम्;
  } अन्यथा {
    size_t विस्तृतिः_आकार = स्मृतिः_config.विस्तृतिः_आकार;
    यदि (विस्तृतिः_आकार > (256 * 1024))
      विस्तृतिः_आकार = (256 * 1024);
    स्मृतिः_विस्तृतिः_आकार = 4096;
    स्मृतिः_विस्तृतिः_आकार_सरणम् = 12;
    यावत् (स्मृतिः_विस्तृतिः_आकार < विस्तृतिः_आकार) {
      स्मृतिः_विस्तृतिः_आकार <<= 1;
      ++स्मृतिः_विस्तृतिः_आकार_सरणम्;
    }
    स्मृतिः_विस्तृतिः_आवरणम् = ~(uintptr_t)(स्मृतिः_विस्तृतिः_आकार - 1);
  }
#यदि_अन्तः

  स्मृतिः_विस्तृतिः_map_गणनम् =
      (स्मृतिः_config.विस्तृतिः_map_गणनम् ? स्मृतिः_config.विस्तृतिः_map_गणनम्
                                     : DEFAULT_विस्तृतिः_MAP_गणनम्);
  यदि ((स्मृतिः_विस्तृतिः_आकार * स्मृतिः_विस्तृतिः_map_गणनम्) < स्मृतिः_page_आकार)
    स्मृतिः_विस्तृतिः_map_गणनम् = (स्मृतिः_page_आकार / स्मृतिः_विस्तृतिः_आकार);
  यदि ((स्मृतिः_page_आकार >= स्मृतिः_विस्तृतिः_आकार) &&
      ((स्मृतिः_विस्तृतिः_map_गणनम् * स्मृतिः_विस्तृतिः_आकार) % स्मृतिः_page_आकार))
    स्मृतिः_विस्तृतिः_map_गणनम् = (स्मृतिः_page_आकार / स्मृतिः_विस्तृतिः_आकार);
  स्मृतिः_heap_reserve_गणनम् = (स्मृतिः_विस्तृतिः_map_गणनम् > DEFAULT_विस्तृतिः_MAP_गणनम्)
                                   ? DEFAULT_विस्तृतिः_MAP_गणनम्
                                   : स्मृतिः_विस्तृतिः_map_गणनम्;

  स्मृतिः_config.page_आकार = स्मृतिः_page_आकार;
  स्मृतिः_config.विस्तृतिः_आकार = स्मृतिः_विस्तृतिः_आकार;
  स्मृतिः_config.विस्तृतिः_map_गणनम् = स्मृतिः_विस्तृतिः_map_गणनम्;
  स्मृतिः_config.enable_huge_पृष्ठानि = स्मृतिः_huge_पृष्ठानि;

#यदि ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  यदि (pthread_कुञ्चिका_create(&स्मृतिः_thread_heap, _rpmalloc_heap_release_raw_fc))
    प्रत्यागम -1;
#यदि_अन्तः
#यदि defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_कुञ्चिका = FlsAlloc(&_rpmalloc_thread_destructor);
#यदि_अन्तः

  // Setup all small and medium size classes
  size_t iclass = 0;
  स्मृतिः_आकार_class[iclass].खण्डम्_आकार_2 = SMALL_GRANULARITY;
  rpmalloc_adjust_आकार_class(iclass);
  प्रति (iclass = 1; iclass < SMALL_CLASS_गणनम्; ++iclass) {
    size_t आकार_2 = iclass * SMALL_GRANULARITY;
    स्मृतिः_आकार_class[iclass].खण्डम्_आकार_2 = (uint32_t)आकार_2;
    rpmalloc_adjust_आकार_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  स्मृतिः_medium_आकार_सीमा = (स्मृतिः_विस्तृतिः_आकार - विस्तृतिः_शीर्षकम्_आकार) >> 1;
  यदि (स्मृतिः_medium_आकार_सीमा > MEDIUM_आकार_सीमा)
    स्मृतिः_medium_आकार_सीमा = MEDIUM_आकार_सीमा;
  प्रति (iclass = 0; iclass < MEDIUM_CLASS_गणनम्; ++iclass) {
    size_t आकार_2 = SMALL_आकार_सीमा + ((iclass + 1) * MEDIUM_GRANULARITY);
    यदि (आकार_2 > स्मृतिः_medium_आकार_सीमा) {
      स्मृतिः_medium_आकार_सीमा =
          SMALL_आकार_सीमा + (iclass * MEDIUM_GRANULARITY);
      भङ्ग;
    }
    स्मृतिः_आकार_class[SMALL_CLASS_गणनम् + iclass].खण्डम्_आकार_2 = (uint32_t)आकार_2;
    rpmalloc_adjust_आकार_class(SMALL_CLASS_गणनम् + iclass);
  }

  स्मृतिः_orphan_heaps = 0;
#यदि RPMALLOC_FIRST_CLASS_HEAPS
  स्मृतिः_first_class_orphan_heaps = 0;
#यदि_अन्तः
#यदि ENABLE_STATISTICS
  atomic_store32(&स्मृतिः_सक्रिय_heaps, 0);
  atomic_store32(&mapped_पृष्ठानि, 0);
  mapped_पृष्ठानि_peak = 0;
  atomic_store32(&_master_spans, 0);
  atomic_store32(&mapped_योग, 0);
  atomic_store32(&unmapped_योग, 0);
  atomic_store32(&mapped_पृष्ठानि_os, 0);
  atomic_store32(&huge_पृष्ठानि_current, 0);
  huge_पृष्ठानि_peak = 0;
#यदि_अन्तः
  स्मृतिं_पूरय(स्मृतिः_heaps, 0, परिमाण(स्मृतिः_heaps));
  atomic_store32_release(&स्मृतिः_global_तालकम्, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  प्रत्यागम 0;
}

//! Finalize the allocator
शून्य rpmalloc_finalize(शून्य) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  यदि (स्मृतिः_global_reserve) {
    atomic_add32(&स्मृतिः_global_reserve_master->remaining_spans,
                 -(int32_t)स्मृतिः_global_reserve_गणनम्);
    स्मृतिः_global_reserve_master = 0;
    स्मृतिः_global_reserve_गणनम् = 0;
    स्मृतिः_global_reserve = 0;
  }
  atomic_store32_release(&स्मृतिः_global_तालकम्, 0);

  // Free all thread caches and fully free spans
  प्रति (size_t list_idx = 0; list_idx < HEAP_ARRAY_आकार; ++list_idx) {
    heap_t *heap = स्मृतिः_heaps[list_idx];
    यावत् (heap) {
      heap_t *अग्र_heap = heap->अग्र_heap;
      heap->finalize = 1;
      _rpmalloc_heap_global_finalize(heap);
      heap = अग्र_heap;
    }
  }

#यदि ENABLE_GLOBAL_CACHE
  // Free global caches
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass)
    _rpmalloc_global_cache_finalize(&स्मृतिः_विस्तृतिः_cache[iclass]);
#यदि_अन्तः

#यदि (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_कुञ्चिका_delete(स्मृतिः_thread_heap);
#यदि_अन्तः
#यदि defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_कुञ्चिका);
  fls_कुञ्चिका = 0;
#यदि_अन्तः
#यदि ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&mapped_पृष्ठानि) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&mapped_पृष्ठानि_os) == 0,
                  "Memory leak detected");
#यदि_अन्तः

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
बाह्य अन्तःस्थ शून्य rpmalloc_thread_initialize(शून्य) {
  यदि (!get_thread_heap_raw()) {
    heap_t *heap = _rpmalloc_heap_allocate(0);
    यदि (heap) {
      _rpmalloc_stat_inc(&स्मृतिः_सक्रिय_heaps);
      समुच्चयः_thread_heap(heap);
#यदि defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_समुच्चयः_मानम्(fls_कुञ्चिका, heap);
#यदि_अन्तः
    }
  }
}

//! Finalize thread, orphan heap
शून्य rpmalloc_thread_finalize(पूर्णाङ्क release_caches) {
  heap_t *heap = get_thread_heap_raw();
  यदि (heap)
    _rpmalloc_heap_release_raw(heap, release_caches);
  समुच्चयः_thread_heap(0);
#यदि defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_समुच्चयः_मानम्(fls_कुञ्चिका, 0);
#यदि_अन्तः
}

पूर्णाङ्क rpmalloc_is_thread_initialized(शून्य) {
  प्रत्यागम (get_thread_heap_raw() != 0) ? 1 : 0;
}

ध्रुव rpmalloc_config_t *rpmalloc_config(शून्य) { प्रत्यागम &स्मृतिः_config; }

// Extern interface

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *rpmalloc(size_t आकार_2) {
#यदि ENABLE_VALIDATE_तर्काः
  यदि (आकार_2 >= MAX_ALLOC_आकार) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#यदि_अन्तः
  heap_t *heap = get_thread_heap();
  प्रत्यागम _rpmalloc_allocate(heap, आकार_2);
}

बाह्य अन्तःस्थ शून्य rpfree(शून्य *ptr) { _rpmalloc_deallocate(ptr); }

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *rpcalloc(size_t अङ्कः_2, size_t आकार_2) {
  size_t योग_3;
#यदि ENABLE_VALIDATE_तर्काः
#यदि PLATFORM_WINDOWS
  पूर्णाङ्क err = आकार_TMult(अङ्कः_2, आकार_2, &योग_3);
  यदि ((err != S_OK) || (योग_3 >= MAX_ALLOC_आकार)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#अन्यथा
  पूर्णाङ्क err = __builtin_umull_overflow(अङ्कः_2, आकार_2, &योग_3);
  यदि (err || (योग_3 >= MAX_ALLOC_आकार)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#यदि_अन्तः
#अन्यथा
  योग_3 = अङ्कः_2 * आकार_2;
#यदि_अन्तः
  heap_t *heap = get_thread_heap();
  शून्य *खण्डम् = _rpmalloc_allocate(heap, योग_3);
  यदि (खण्डम्)
    स्मृतिं_पूरय(खण्डम्, 0, योग_3);
  प्रत्यागम खण्डम्;
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *rprealloc(शून्य *ptr, size_t आकार_2) {
#यदि ENABLE_VALIDATE_तर्काः
  यदि (आकार_2 >= MAX_ALLOC_आकार) {
    errno = EINVAL;
    प्रत्यागम ptr;
  }
#यदि_अन्तः
  heap_t *heap = get_thread_heap();
  प्रत्यागम _rpmalloc_reallocate(heap, ptr, आकार_2, 0, 0);
}

बाह्य RPMALLOC_ALLOCATOR शून्य *rpaligned_realloc(शून्य *ptr, size_t alignment,
                                                  size_t आकार_2, size_t oldsize,
                                                  अचिह्नित पूर्णाङ्क flags) {
#यदि ENABLE_VALIDATE_तर्काः
  यदि ((आकार_2 + alignment < आकार_2) || (alignment > स्मृतिः_page_आकार)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#यदि_अन्तः
  heap_t *heap = get_thread_heap();
  प्रत्यागम _rpmalloc_aligned_reallocate(heap, ptr, alignment, आकार_2, oldsize,
                                      flags);
}

बाह्य RPMALLOC_ALLOCATOR शून्य *rpaligned_alloc(size_t alignment, size_t आकार_2) {
  heap_t *heap = get_thread_heap();
  प्रत्यागम _rpmalloc_aligned_allocate(heap, alignment, आकार_2);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpaligned_calloc(size_t alignment, size_t अङ्कः_2, size_t आकार_2) {
  size_t योग_3;
#यदि ENABLE_VALIDATE_तर्काः
#यदि PLATFORM_WINDOWS
  पूर्णाङ्क err = आकार_TMult(अङ्कः_2, आकार_2, &योग_3);
  यदि ((err != S_OK) || (योग_3 >= MAX_ALLOC_आकार)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#अन्यथा
  पूर्णाङ्क err = __builtin_umull_overflow(अङ्कः_2, आकार_2, &योग_3);
  यदि (err || (योग_3 >= MAX_ALLOC_आकार)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#यदि_अन्तः
#अन्यथा
  योग_3 = अङ्कः_2 * आकार_2;
#यदि_अन्तः
  शून्य *खण्डम् = rpaligned_alloc(alignment, योग_3);
  यदि (खण्डम्)
    स्मृतिं_पूरय(खण्डम्, 0, योग_3);
  प्रत्यागम खण्डम्;
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *rpmemalign(size_t alignment,
                                                  size_t आकार_2) {
  प्रत्यागम rpaligned_alloc(alignment, आकार_2);
}

बाह्य अन्तःस्थ पूर्णाङ्क rpposix_memalign(शून्य **memptr, size_t alignment,
                                   size_t आकार_2) {
  यदि (memptr)
    *memptr = rpaligned_alloc(alignment, आकार_2);
  अन्यथा
    प्रत्यागम EINVAL;
  प्रत्यागम *memptr ? 0 : ENOMEM;
}

बाह्य अन्तःस्थ size_t rpmalloc_usable_आकार(शून्य *ptr) {
  प्रत्यागम (ptr ? rpmalloc_usable_आकार_2(ptr) : 0);
}

बाह्य अन्तःस्थ शून्य rpmalloc_thread_collect(शून्य) {}

शून्य rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  स्मृतिं_पूरय(stats, 0, परिमाण(rpmalloc_thread_statistics_t));
  heap_t *heap = get_thread_heap_raw();
  यदि (!heap)
    प्रत्यागम;

  प्रति (size_t iclass = 0; iclass < आकार_CLASS_गणनम्; ++iclass) {
    आकार_class_t *आकार_class = स्मृतिः_आकार_class + iclass;
    विस्तृतिः_t *विस्तृतिः = heap->आकार_class[iclass].partial_विस्तृतिः;
    यावत् (विस्तृतिः) {
      size_t free_गणनम् = विस्तृतिः->list_आकार;
      size_t खण्डम्_गणनम् = आकार_class->खण्डम्_गणनम्;
      यदि (विस्तृतिः->free_list_सीमा < खण्डम्_गणनम्)
        खण्डम्_गणनम् = विस्तृतिः->free_list_सीमा;
      free_गणनम् += (खण्डम्_गणनम् - विस्तृतिः->प्रयुक्तम्_गणनम्);
      stats->sizecache += free_गणनम् * आकार_class->खण्डम्_आकार_2;
      विस्तृतिः = विस्तृतिः->अग्र;
    }
  }

#यदि ENABLE_THREAD_CACHE
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    विस्तृतिः_cache_t *विस्तृतिः_cache;
    यदि (!iclass)
      विस्तृतिः_cache = &heap->विस्तृतिः_cache;
    अन्यथा
      विस्तृतिः_cache = (विस्तृतिः_cache_t *)(heap->विस्तृतिः_large_cache + (iclass - 1));
    stats->spancache += विस्तृतिः_cache->गणनम् * (iclass + 1) * स्मृतिः_विस्तृतिः_आकार;
  }
#यदि_अन्तः

  विस्तृतिः_t *deferred = (विस्तृतिः_t *)atomic_आरोपय_ptr(&heap->विस्तृतिः_free_deferred);
  यावत् (deferred) {
    यदि (deferred->आकार_class != आकार_CLASS_HUGE)
      stats->spancache += (size_t)deferred->विस्तृतिः_गणनम् * स्मृतिः_विस्तृतिः_आकार;
    deferred = (विस्तृतिः_t *)deferred->free_list;
  }

#यदि ENABLE_STATISTICS
  stats->thread_गन्तव्यम्_global = (size_t)atomic_load64(&heap->thread_गन्तव्यम्_global);
  stats->global_गन्तव्यम्_thread = (size_t)atomic_load64(&heap->global_गन्तव्यम्_thread);

  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    stats->विस्तृतिः_use[iclass].current =
        (size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].current);
    stats->विस्तृतिः_use[iclass].peak =
        (size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].उच्चः);
    stats->विस्तृतिः_use[iclass].गन्तव्यम्_global =
        (size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_गन्तव्यम्_global);
    stats->विस्तृतिः_use[iclass].from_global =
        (size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_from_global);
    stats->विस्तृतिः_use[iclass].गन्तव्यम्_cache =
        (size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_गन्तव्यम्_cache);
    stats->विस्तृतिः_use[iclass].from_cache =
        (size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_from_cache);
    stats->विस्तृतिः_use[iclass].गन्तव्यम्_reserved =
        (size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_गन्तव्यम्_reserved);
    stats->विस्तृतिः_use[iclass].from_reserved =
        (size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_from_reserved);
    stats->विस्तृतिः_use[iclass].map_calls =
        (size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_map_calls);
  }
  प्रति (size_t iclass = 0; iclass < आकार_CLASS_गणनम्; ++iclass) {
    stats->आकार_use[iclass].alloc_current =
        (size_t)atomic_load32(&heap->आकार_class_use[iclass].alloc_current);
    stats->आकार_use[iclass].alloc_peak =
        (size_t)heap->आकार_class_use[iclass].alloc_peak;
    stats->आकार_use[iclass].alloc_योग =
        (size_t)atomic_load32(&heap->आकार_class_use[iclass].alloc_योग);
    stats->आकार_use[iclass].free_योग =
        (size_t)atomic_load32(&heap->आकार_class_use[iclass].free_योग);
    stats->आकार_use[iclass].spans_गन्तव्यम्_cache =
        (size_t)atomic_load32(&heap->आकार_class_use[iclass].spans_गन्तव्यम्_cache);
    stats->आकार_use[iclass].spans_from_cache =
        (size_t)atomic_load32(&heap->आकार_class_use[iclass].spans_from_cache);
    stats->आकार_use[iclass].spans_from_reserved = (size_t)atomic_load32(
        &heap->आकार_class_use[iclass].spans_from_reserved);
    stats->आकार_use[iclass].map_calls =
        (size_t)atomic_load32(&heap->आकार_class_use[iclass].spans_map_calls);
  }
#यदि_अन्तः
}

शून्य rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  स्मृतिं_पूरय(stats, 0, परिमाण(rpmalloc_global_statistics_t));
#यदि ENABLE_STATISTICS
  stats->mapped = (size_t)atomic_load32(&mapped_पृष्ठानि) * स्मृतिः_page_आकार;
  stats->mapped_peak = (size_t)mapped_पृष्ठानि_peak * स्मृतिः_page_आकार;
  stats->mapped_योग_2 =
      (size_t)atomic_load32(&mapped_योग) * स्मृतिः_page_आकार;
  stats->unmapped_योग_2 =
      (size_t)atomic_load32(&unmapped_योग) * स्मृतिः_page_आकार;
  stats->huge_alloc =
      (size_t)atomic_load32(&huge_पृष्ठानि_current) * स्मृतिः_page_आकार;
  stats->huge_alloc_peak = (size_t)huge_पृष्ठानि_peak * स्मृतिः_page_आकार;
#यदि_अन्तः
#यदि ENABLE_GLOBAL_CACHE
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    global_cache_t *cache = &स्मृतिः_विस्तृतिः_cache[iclass];
    यावत् (!atomic_cas32_acquire(&cache->तालकम्, 1, 0))
      _rpmalloc_spin();
    uint32_t गणनम् = cache->गणनम्;
#यदि ENABLE_UNLIMITED_CACHE
    विस्तृतिः_t *current_विस्तृतिः = cache->अतिप्रवाहः_2;
    यावत् (current_विस्तृतिः) {
      ++गणनम्;
      current_विस्तृतिः = current_विस्तृतिः->अग्र;
    }
#यदि_अन्तः
    atomic_store32_release(&cache->तालकम्, 0);
    stats->cached += गणनम् * (iclass + 1) * स्मृतिः_विस्तृतिः_आकार;
  }
#यदि_अन्तः
}

#यदि ENABLE_STATISTICS

स्थिर शून्य स्मृतिः_heap_dump_statistics(heap_t *heap, शून्य *file) {
  fprintf(file, "Heap %d stats:\n", heap->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  प्रति (size_t iclass = 0; iclass < आकार_CLASS_गणनम्; ++iclass) {
    यदि (!atomic_load32(&heap->आकार_class_use[iclass].alloc_योग))
      अनुवर्त;
    fprintf(
        file,
        "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu "
        "%9u\n",
        (uint32_t)iclass,
        atomic_load32(&heap->आकार_class_use[iclass].alloc_current),
        heap->आकार_class_use[iclass].alloc_peak,
        atomic_load32(&heap->आकार_class_use[iclass].alloc_योग),
        atomic_load32(&heap->आकार_class_use[iclass].free_योग),
        स्मृतिः_आकार_class[iclass].खण्डम्_आकार_2,
        स्मृतिः_आकार_class[iclass].खण्डम्_गणनम्,
        atomic_load32(&heap->आकार_class_use[iclass].spans_current),
        heap->आकार_class_use[iclass].spans_peak,
        ((size_t)heap->आकार_class_use[iclass].alloc_peak *
         (size_t)स्मृतिः_आकार_class[iclass].खण्डम्_आकार_2) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->आकार_class_use[iclass].spans_गन्तव्यम्_cache) *
         स्मृतिः_विस्तृतिः_आकार) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->आकार_class_use[iclass].spans_from_cache) *
         स्मृतिः_विस्तृतिः_आकार) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(
             &heap->आकार_class_use[iclass].spans_from_reserved) *
         स्मृतिः_विस्तृतिः_आकार) /
            (size_t)(1024 * 1024),
        atomic_load32(&heap->आकार_class_use[iclass].spans_map_calls));
  }
  fprintf(file, "Spans  Current     Peak Deferred  PeakMiB  Cached  ToCacheMiB "
                "FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB "
                "FromGlobalMiB  MmapCalls\n");
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    यदि (!atomic_load32(&heap->विस्तृतिः_use[iclass].उच्चः) &&
        !atomic_load32(&heap->विस्तृतिः_use[iclass].spans_map_calls))
      अनुवर्त;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&heap->विस्तृतिः_use[iclass].current),
        atomic_load32(&heap->विस्तृतिः_use[iclass].उच्चः),
        atomic_load32(&heap->विस्तृतिः_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].उच्चः) *
         (size_t)स्मृतिः_विस्तृतिः_आकार * (iclass + 1)) /
            (size_t)(1024 * 1024),
#यदि ENABLE_THREAD_CACHE
        (अचिह्नित पूर्णाङ्क)(!iclass ? heap->विस्तृतिः_cache.गणनम्
                               : heap->विस्तृतिः_large_cache[iclass - 1].गणनम्),
        ((size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_गन्तव्यम्_cache) *
         (iclass + 1) * स्मृतिः_विस्तृतिः_आकार) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_from_cache) *
         (iclass + 1) * स्मृतिः_विस्तृतिः_आकार) /
            (size_t)(1024 * 1024),
#अन्यथा
        0, (size_t)0, (size_t)0,
#यदि_अन्तः
        ((size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_गन्तव्यम्_reserved) *
         (iclass + 1) * स्मृतिः_विस्तृतिः_आकार) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_from_reserved) *
         (iclass + 1) * स्मृतिः_विस्तृतिः_आकार) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_गन्तव्यम्_global) *
         (size_t)स्मृतिः_विस्तृतिः_आकार * (iclass + 1)) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&heap->विस्तृतिः_use[iclass].spans_from_global) *
         (size_t)स्मृतिः_विस्तृतिः_आकार * (iclass + 1)) /
            (size_t)(1024 * 1024),
        atomic_load32(&heap->विस्तृतिः_use[iclass].spans_map_calls));
  }
  fprintf(file, "Full spans: %zu\n", heap->पूर्णम्_विस्तृतिः_गणनम्);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&heap->thread_गन्तव्यम्_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&heap->global_गन्तव्यम्_thread) / (size_t)(1024 * 1024));
}

#यदि_अन्तः

शून्य rpmalloc_dump_statistics(शून्य *file) {
#यदि ENABLE_STATISTICS
  प्रति (size_t list_idx = 0; list_idx < HEAP_ARRAY_आकार; ++list_idx) {
    heap_t *heap = स्मृतिः_heaps[list_idx];
    यावत् (heap) {
      पूर्णाङ्क need_dump = 0;
      प्रति (size_t iclass = 0; !need_dump && (iclass < आकार_CLASS_गणनम्);
           ++iclass) {
        यदि (!atomic_load32(&heap->आकार_class_use[iclass].alloc_योग)) {
          rpmalloc_assert(
              !atomic_load32(&heap->आकार_class_use[iclass].free_योग),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&heap->आकार_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          अनुवर्त;
        }
        need_dump = 1;
      }
      प्रति (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_गणनम्);
           ++iclass) {
        यदि (!atomic_load32(&heap->विस्तृतिः_use[iclass].उच्चः) &&
            !atomic_load32(&heap->विस्तृतिः_use[iclass].spans_map_calls))
          अनुवर्त;
        need_dump = 1;
      }
      यदि (need_dump)
        स्मृतिः_heap_dump_statistics(heap, file);
      heap = heap->अग्र_heap;
    }
  }
  fprintf(file, "Global stats:\n");
  size_t huge_current =
      (size_t)atomic_load32(&huge_पृष्ठानि_current) * स्मृतिः_page_आकार;
  size_t huge_peak = (size_t)huge_पृष्ठानि_peak * स्मृतिः_page_आकार;
  fprintf(file, "HugeCurrentMiB HugePeakMiB\n");
  fprintf(file, "%14zu %11zu\n", huge_current / (size_t)(1024 * 1024),
          huge_peak / (size_t)(1024 * 1024));

#यदि ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    global_cache_t *cache = स्मृतिः_विस्तृतिः_cache + iclass;
    size_t global_cache = (size_t)cache->गणनम् * iclass * स्मृतिः_विस्तृतिः_आकार;

    size_t global_अतिप्रवाहः_cache = 0;
    विस्तृतिः_t *विस्तृतिः = cache->अतिप्रवाहः_2;
    यावत् (विस्तृतिः) {
      global_अतिप्रवाहः_cache += iclass * स्मृतिः_विस्तृतिः_आकार;
      विस्तृतिः = विस्तृतिः->अग्र;
    }
    यदि (global_cache || global_अतिप्रवाहः_cache || cache->निवेशय_गणनम् ||
        cache->extract_गणनम्)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_अतिप्रवाहः_cache / (size_t)(1024 * 1024),
              cache->निवेशय_गणनम्, cache->extract_गणनम्);
  }
#यदि_अन्तः

  size_t mapped = (size_t)atomic_load32(&mapped_पृष्ठानि) * स्मृतिः_page_आकार;
  size_t mapped_os =
      (size_t)atomic_load32(&mapped_पृष्ठानि_os) * स्मृतिः_page_आकार;
  size_t mapped_peak = (size_t)mapped_पृष्ठानि_peak * स्मृतिः_page_आकार;
  size_t mapped_योग_2 =
      (size_t)atomic_load32(&mapped_योग) * स्मृतिः_page_आकार;
  size_t unmapped_योग_2 =
      (size_t)atomic_load32(&unmapped_योग) * स्मृतिः_page_आकार;
  fprintf(
      file,
      "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB\n");
  fprintf(file, "%9zu %11zu %13zu %14zu %16zu\n",
          mapped / (size_t)(1024 * 1024), mapped_os / (size_t)(1024 * 1024),
          mapped_peak / (size_t)(1024 * 1024),
          mapped_योग_2 / (size_t)(1024 * 1024),
          unmapped_योग_2 / (size_t)(1024 * 1024));

  fprintf(file, "\n");
#यदि 0
	int64_t allocated = atomic_load64(&आवण्टनम्_गणक);
	int64_t deallocated = atomic_load64(&deallocation_गणक);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#यदि_अन्तः
#यदि_अन्तः
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
  _rpmalloc_stat_inc(&स्मृतिः_सक्रिय_heaps);
  प्रत्यागम heap;
}

बाह्य अन्तःस्थ शून्य rpmalloc_heap_release(rpmalloc_heap_t *heap) {
  यदि (heap)
    _rpmalloc_heap_release(heap, 1, 1);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, size_t आकार_2) {
#यदि ENABLE_VALIDATE_तर्काः
  यदि (आकार_2 >= MAX_ALLOC_आकार) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#यदि_अन्तः
  प्रत्यागम _rpmalloc_allocate(heap, आकार_2);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, size_t alignment,
                            size_t आकार_2) {
#यदि ENABLE_VALIDATE_तर्काः
  यदि (आकार_2 >= MAX_ALLOC_आकार) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#यदि_अन्तः
  प्रत्यागम _rpmalloc_aligned_allocate(heap, alignment, आकार_2);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, size_t अङ्कः_2, size_t आकार_2) {
  प्रत्यागम rpmalloc_heap_aligned_calloc(heap, 0, अङ्कः_2, आकार_2);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, size_t alignment,
                             size_t अङ्कः_2, size_t आकार_2) {
  size_t योग_3;
#यदि ENABLE_VALIDATE_तर्काः
#यदि PLATFORM_WINDOWS
  पूर्णाङ्क err = आकार_TMult(अङ्कः_2, आकार_2, &योग_3);
  यदि ((err != S_OK) || (योग_3 >= MAX_ALLOC_आकार)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#अन्यथा
  पूर्णाङ्क err = __builtin_umull_overflow(अङ्कः_2, आकार_2, &योग_3);
  यदि (err || (योग_3 >= MAX_ALLOC_आकार)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#यदि_अन्तः
#अन्यथा
  योग_3 = अङ्कः_2 * आकार_2;
#यदि_अन्तः
  शून्य *खण्डम् = _rpmalloc_aligned_allocate(heap, alignment, योग_3);
  यदि (खण्डम्)
    स्मृतिं_पूरय(खण्डम्, 0, योग_3);
  प्रत्यागम खण्डम्;
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, शून्य *ptr, size_t आकार_2,
                      अचिह्नित पूर्णाङ्क flags) {
#यदि ENABLE_VALIDATE_तर्काः
  यदि (आकार_2 >= MAX_ALLOC_आकार) {
    errno = EINVAL;
    प्रत्यागम ptr;
  }
#यदि_अन्तः
  प्रत्यागम _rpmalloc_reallocate(heap, ptr, आकार_2, 0, flags);
}

बाह्य अन्तःस्थ RPMALLOC_ALLOCATOR शून्य *
rpmalloc_heap_aligned_realloc(rpmalloc_heap_t *heap, शून्य *ptr,
                              size_t alignment, size_t आकार_2,
                              अचिह्नित पूर्णाङ्क flags) {
#यदि ENABLE_VALIDATE_तर्काः
  यदि ((आकार_2 + alignment < आकार_2) || (alignment > स्मृतिः_page_आकार)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#यदि_अन्तः
  प्रत्यागम _rpmalloc_aligned_reallocate(heap, ptr, alignment, आकार_2, 0, flags);
}

बाह्य अन्तःस्थ शून्य rpmalloc_heap_free(rpmalloc_heap_t *heap, शून्य *ptr) {
  (शून्य)परिमाण(heap);
  _rpmalloc_deallocate(ptr);
}

बाह्य अन्तःस्थ शून्य rpmalloc_heap_free_all(rpmalloc_heap_t *heap) {
  विस्तृतिः_t *विस्तृतिः;
  विस्तृतिः_t *अग्र_विस्तृतिः;

  _rpmalloc_heap_cache_adopt_deferred(heap, 0);

  प्रति (size_t iclass = 0; iclass < आकार_CLASS_गणनम्; ++iclass) {
    विस्तृतिः = heap->आकार_class[iclass].partial_विस्तृतिः;
    यावत् (विस्तृतिः) {
      अग्र_विस्तृतिः = विस्तृतिः->अग्र;
      rpmalloc_heap_cache_निवेशय(heap, विस्तृतिः);
      विस्तृतिः = अग्र_विस्तृतिः;
    }
    heap->आकार_class[iclass].partial_विस्तृतिः = 0;
    विस्तृतिः = heap->पूर्णम्_विस्तृतिः[iclass];
    यावत् (विस्तृतिः) {
      अग्र_विस्तृतिः = विस्तृतिः->अग्र;
      rpmalloc_heap_cache_निवेशय(heap, विस्तृतिः);
      विस्तृतिः = अग्र_विस्तृतिः;
    }

    विस्तृतिः = heap->आकार_class[iclass].cache;
    यदि (विस्तृतिः)
      rpmalloc_heap_cache_निवेशय(heap, विस्तृतिः);
    heap->आकार_class[iclass].cache = 0;
  }
  स्मृतिं_पूरय(heap->आकार_class, 0, परिमाण(heap->आकार_class));
  स्मृतिं_पूरय(heap->पूर्णम्_विस्तृतिः, 0, परिमाण(heap->पूर्णम्_विस्तृतिः));

  विस्तृतिः = heap->large_huge_विस्तृतिः;
  यावत् (विस्तृतिः) {
    अग्र_विस्तृतिः = विस्तृतिः->अग्र;
    यदि (UNEXPECTED(विस्तृतिः->आकार_class == आकार_CLASS_HUGE))
      _rpmalloc_deallocate_huge(विस्तृतिः);
    अन्यथा
      rpmalloc_heap_cache_निवेशय(heap, विस्तृतिः);
    विस्तृतिः = अग्र_विस्तृतिः;
  }
  heap->large_huge_विस्तृतिः = 0;
  heap->पूर्णम्_विस्तृतिः_गणनम् = 0;

#यदि ENABLE_THREAD_CACHE
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    विस्तृतिः_cache_t *विस्तृतिः_cache;
    यदि (!iclass)
      विस्तृतिः_cache = &heap->विस्तृतिः_cache;
    अन्यथा
      विस्तृतिः_cache = (विस्तृतिः_cache_t *)(heap->विस्तृतिः_large_cache + (iclass - 1));
    यदि (!विस्तृतिः_cache->गणनम्)
      अनुवर्त;
#यदि ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&heap->thread_गन्तव्यम्_global,
                         विस्तृतिः_cache->गणनम् * (iclass + 1) * स्मृतिः_विस्तृतिः_आकार);
    rpmalloc_stat_सङ्कलय(&heap->विस्तृतिः_use[iclass].spans_गन्तव्यम्_global,
                       विस्तृतिः_cache->गणनम्);
    rpmalloc_global_cache_निवेशय_spans(विस्तृतिः_cache->विस्तृतिः, iclass + 1,
                                        विस्तृतिः_cache->गणनम्);
#अन्यथा
    प्रति (size_t ispan = 0; ispan < विस्तृतिः_cache->गणनम्; ++ispan)
      rpmalloc_विस्तृतिः_unmap(विस्तृतिः_cache->विस्तृतिः[ispan]);
#यदि_अन्तः
    विस्तृतिः_cache->गणनम् = 0;
  }
#यदि_अन्तः

#यदि ENABLE_STATISTICS
  प्रति (size_t iclass = 0; iclass < आकार_CLASS_गणनम्; ++iclass) {
    atomic_store32(&heap->आकार_class_use[iclass].alloc_current, 0);
    atomic_store32(&heap->आकार_class_use[iclass].spans_current, 0);
  }
  प्रति (size_t iclass = 0; iclass < LARGE_CLASS_गणनम्; ++iclass) {
    atomic_store32(&heap->विस्तृतिः_use[iclass].current, 0);
  }
#यदि_अन्तः
}

बाह्य अन्तःस्थ शून्य rpmalloc_heap_thread_समुच्चयः_current(rpmalloc_heap_t *heap) {
  heap_t *पूर्व_heap = get_thread_heap_raw();
  यदि (पूर्व_heap != heap) {
    समुच्चयः_thread_heap(heap);
    यदि (पूर्व_heap)
      rpmalloc_heap_release(पूर्व_heap);
  }
}

बाह्य अन्तःस्थ rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(शून्य *ptr) {
  // Grab the span, and then the heap from the span
  विस्तृतिः_t *विस्तृतिः = (विस्तृतिः_t *)((uintptr_t)ptr & स्मृतिः_विस्तृतिः_आवरणम्);
  यदि (विस्तृतिः) {
    प्रत्यागम विस्तृतिः->heap;
  }
  प्रत्यागम 0;
}

#यदि_अन्तः

#यदि ENABLE_PRELOAD || ENABLE_OVERRIDE

#अन्तर्भाव "malloc.c"

#यदि_अन्तः

शून्य rpmalloc_linker_reference(शून्य) { (शून्य)परिमाण(_rpmalloc_initialized); }
