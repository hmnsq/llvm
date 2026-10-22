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
#رهنمود clang diagnostic ignored "-Wunused-macros"
#رهنمود clang diagnostic ignored "-Wunused-function"
#إذا __has_warning("-Wreserved-identifier")
#رهنمود clang diagnostic ignored "-Wreserved-identifier"
#پایان_اگر
#إذا __has_warning("-Wstatic-in-inline")
#رهنمود clang diagnostic ignored "-Wstatic-in-inline"
#پایان_اگر
#وإلاإذا defined(__GNUC__)
#رهنمود GCC diagnostic ignored "-Wunused-macros"
#رهنمود GCC diagnostic ignored "-Wunused-function"
#پایان_اگر

#إذا !defined(__has_builtin)
#تعریف_کن __has_builtin(b) 0
#پایان_اگر

#إذا defined(__GNUC__) || defined(__clang__)

#إذا __has_builtin(__builtin_memcpy_inline)
#تعریف_کن _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#وإلا
#تعریف_کن _rpmalloc_memcpy_const(x, y, s)                                        \
  افعل {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    کپی_حافظه(x, y, s);                                                           \
  } طالما (0)
#پایان_اگر

#إذا __has_builtin(__builtin_memset_inline)
#تعریف_کن _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#وإلا
#تعریف_کن _rpmalloc_memset_const(x, y, s)                                        \
  افعل {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } طالما (0)
#پایان_اگر
#وإلا
#تعریف_کن _rpmalloc_memcpy_const(x, y, s) کپی_حافظه(x, y, s)
#تعریف_کن _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#پایان_اگر

#إذا __has_builtin(__builtin_assume)
#تعریف_کن rpmalloc_assume(cond) __builtin_assume(cond)
#وإلاإذا defined(__GNUC__)
#تعریف_کن rpmalloc_assume(cond)                                                  \
  افعل {                                                                         \
    إذا (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } طالما (0)
#وإلاإذا defined(_MSC_VER)
#تعریف_کن rpmalloc_assume(cond) __assume(cond)
#وإلا
#تعریف_کن rpmalloc_assume(cond) 0
#پایان_اگر

#اگر_تعریف_نشده انباشته_ARRAY_اندازه
//! Size of heap hashmap
#تعریف_کن انباشته_ARRAY_اندازه 47
#پایان_اگر
#اگر_تعریف_نشده ENABLE_THREAD_CACHE
//! Enable per-thread cache
#تعریف_کن ENABLE_THREAD_CACHE 1
#پایان_اگر
#اگر_تعریف_نشده ENABLE_GLOBAL_CACHE
//! Enable global cache shared between all threads, requires thread cache
#تعریف_کن ENABLE_GLOBAL_CACHE 1
#پایان_اگر
#اگر_تعریف_نشده ENABLE_VALIDATE_آرگومانها
//! Enable validation of args to public entry points
#تعریف_کن ENABLE_VALIDATE_آرگومانها 0
#پایان_اگر
#اگر_تعریف_نشده ENABLE_STATISTICS
//! Enable statistics collection
#تعریف_کن ENABLE_STATISTICS 0
#پایان_اگر
#اگر_تعریف_نشده ENABLE_ASSERTS
//! Enable asserts
#تعریف_کن ENABLE_ASSERTS 0
#پایان_اگر
#اگر_تعریف_نشده ENABLE_OVERRIDE
//! Override standard library malloc/free and new/delete entry points
#تعریف_کن ENABLE_OVERRIDE 0
#پایان_اگر
#اگر_تعریف_نشده ENABLE_PRELOAD
//! Support preloading
#تعریف_کن ENABLE_PRELOAD 0
#پایان_اگر
#اگر_تعریف_نشده DISABLE_UNMAP
//! Disable unmapping memory pages (also enables unlimited cache)
#تعریف_کن DISABLE_UNMAP 0
#پایان_اگر
#اگر_تعریف_نشده ENABLE_UNLIMITED_CACHE
//! Enable unlimited global cache (no unmapping until finalization)
#تعریف_کن ENABLE_UNLIMITED_CACHE 0
#پایان_اگر
#اگر_تعریف_نشده ENABLE_ADAPTIVE_THREAD_CACHE
//! Enable adaptive thread cache size based on use heuristics
#تعریف_کن ENABLE_ADAPTIVE_THREAD_CACHE 0
#پایان_اگر
#اگر_تعریف_نشده DEFAULT_بازه_MAP_شمار
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#تعریف_کن DEFAULT_بازه_MAP_شمار 64
#پایان_اگر
#اگر_تعریف_نشده GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#تعریف_کن GLOBAL_CACHE_MULTIPLIER 8
#پایان_اگر

#إذا DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#خطا Must use global cache إذا unmap is disabled
#پایان_اگر

#إذا DISABLE_UNMAP
#لغو_تعریف ENABLE_UNLIMITED_CACHE
#تعریف_کن ENABLE_UNLIMITED_CACHE 1
#پایان_اگر

#إذا !ENABLE_GLOBAL_CACHE
#لغو_تعریف ENABLE_UNLIMITED_CACHE
#تعریف_کن ENABLE_UNLIMITED_CACHE 0
#پایان_اگر

#إذا !ENABLE_THREAD_CACHE
#لغو_تعریف ENABLE_ADAPTIVE_THREAD_CACHE
#تعریف_کن ENABLE_ADAPTIVE_THREAD_CACHE 0
#پایان_اگر

#إذا defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#تعریف_کن PLATFORM_WINDOWS 1
#تعریف_کن PLATFORM_POSIX 0
#وإلا
#تعریف_کن PLATFORM_WINDOWS 0
#تعریف_کن PLATFORM_POSIX 1
#پایان_اگر

/// Platform and arch specifics
#إذا defined(_MSC_VER) && !defined(__clang__)
#رهنمود warning(disable : 5105)
#اگر_تعریف_نشده FORCEINLINE
#تعریف_کن FORCEINLINE مضمن __forceinline
#پایان_اگر
#تعریف_کن _Static_assert static_assert
#وإلا
#اگر_تعریف_نشده FORCEINLINE
#تعریف_کن FORCEINLINE مضمن __attribute__((__always_inline__))
#پایان_اگر
#پایان_اگر
#إذا PLATFORM_WINDOWS
#اگر_تعریف_نشده WIN32_LEAN_AND_میانگین
#تعریف_کن WIN32_LEAN_AND_میانگین
#پایان_اگر
#ضمن <windows.h>
#إذا ENABLE_VALIDATE_آرگومانها
#ضمن <intsafe.h>
#پایان_اگر
#وإلا
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <time.h>
#ضمن <unistd.h>
#إذا defined(__linux__) || defined(__ANDROID__)
#ضمن <sys/prctl.h>
#إذا !defined(PR_مجموعه_VMA)
#تعریف_کن PR_مجموعه_VMA 0x53564d41
#تعریف_کن PR_مجموعه_VMA_ANON_NAME 0
#پایان_اگر
#پایان_اگر
#إذا defined(__APPLE__)
#ضمن <TargetConditionals.h>
#إذا !هدف_OS_IPHONE && !هدف_OS_SIMULATOR
#ضمن <mach/mach_vm.h>
#ضمن <mach/vm_statistics.h>
#پایان_اگر
#ضمن <pthread.h>
#پایان_اگر
#إذا defined(__HAIKU__) || defined(__TINYC__)
#ضمن <pthread.h>
#پایان_اگر
#پایان_اگر

#ضمن <errno.h>
#ضمن <stdint.h>
#ضمن <string.h>

#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#ضمن <fibersapi.h>
ساكن DWORD fls_کلید;
#پایان_اگر

#إذا PLATFORM_POSIX
#ضمن <sched.h>
#ضمن <sys/mman.h>
#اگر_تعریف_شده __FreeBSD__
#ضمن <sys/sysctl.h>
#تعریف_کن MAP_HUGETLB MAP_ALIGNED_SUPER
#اگر_تعریف_نشده PROT_MAX
#تعریف_کن PROT_MAX(f) 0
#پایان_اگر
#وإلا
#تعریف_کن PROT_MAX(f) 0
#پایان_اگر
#اگر_تعریف_شده __sun
خارجي صحيح madvise(caddr_t, size_t, صحيح);
#پایان_اگر
#اگر_تعریف_نشده MAP_UNINITIALIZED
#تعریف_کن MAP_UNINITIALIZED 0
#پایان_اگر
#پایان_اگر
#ضمن <errno.h>

#إذا ENABLE_ASSERTS
#لغو_تعریف NDEBUG
#إذا defined(_MSC_VER) && !defined(_DEBUG)
#تعریف_کن _DEBUG
#پایان_اگر
#ضمن <assert.h>
#تعریف_کن RPMALLOC_TOSTRING_M(x) #x
#تعریف_کن RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#تعریف_کن rpmalloc_assert(truth, پیام_2)                                        \
  افعل {                                                                         \
    إذا (!(truth)) {                                                            \
      إذا (حافظه_config.error_callback) {                                     \
        حافظه_config.error_callback(پیام_2 " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } وإلا {                                                                 \
        أكد((truth) && پیام_2);                                            \
      }                                                                        \
    }                                                                          \
  } طالما (0)
#وإلا
#تعریف_کن rpmalloc_assert(truth, پیام_2)                                        \
  افعل {                                                                         \
  } طالما (0)
#پایان_اگر
#إذا ENABLE_STATISTICS
#ضمن <stdio.h>
#پایان_اگر

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#إذا defined(_MSC_VER) && !defined(__clang__)

تعريفنوع متطاير دراز atomic32_t;
تعريفنوع متطاير دراز دراز atomic64_t;
تعريفنوع متطاير فراغ *atomicptr_t;

ساكن FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  أرجع (int32_t)InterlockedOr(src, 0);
}
ساكن FORCEINLINE فراغ atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ساكن FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  أرجع (int32_t)InterlockedIncrement(val);
}
ساكن FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  أرجع (int32_t)InterlockedDecrement(val);
}
ساكن FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t جمع_کن) {
  أرجع (int32_t)Interlocked_Exchange_جمع_کن(val, جمع_کن) + جمع_کن;
}
ساكن FORCEINLINE صحيح atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  أرجع (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
ساكن FORCEINLINE فراغ atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ساكن FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  أرجع (int64_t)InterlockedOr64(src, 0);
}
ساكن FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t جمع_کن) {
  أرجع (int64_t)InterlockedExchangeAdd64(val, جمع_کن) + جمع_کن;
}
ساكن FORCEINLINE فراغ *atomic_بارگیری_ptr(atomicptr_t *src) {
  أرجع Interlocked_Compare_Exchange_اشاره_گر(src, 0, 0);
}
ساكن FORCEINLINE فراغ atomic_store_ptr(atomicptr_t *dst, فراغ *val) {
  Interlocked_Exchange_اشاره_گر(dst, val);
}
ساكن FORCEINLINE فراغ atomic_store_ptr_release(atomicptr_t *dst, فراغ *val) {
  Interlocked_Exchange_اشاره_گر(dst, val);
}
ساكن FORCEINLINE فراغ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     فراغ *val) {
  أرجع (فراغ *)Interlocked_Exchange_اشاره_گر((فراغ *متطاير *)dst, val);
}
ساكن FORCEINLINE صحيح atomic_cas_ptr(atomicptr_t *dst, فراغ *val, فراغ *ref) {
  أرجع (Interlocked_Compare_Exchange_اشاره_گر((فراغ *متطاير *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#تعریف_کن EXPECTED(x) (x)
#تعریف_کن UNEXPECTED(x) (x)

#وإلا

#ضمن <stdatomic.h>

تعريفنوع متطاير _Atomic(int32_t) atomic32_t;
تعريفنوع متطاير _Atomic(int64_t) atomic64_t;
تعريفنوع متطاير _Atomic(فراغ *) atomicptr_t;

ساكن FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  أرجع atomic_بارگیری_explicit(src, memory_order_relaxed);
}
ساكن FORCEINLINE فراغ atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ساكن FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  أرجع atomic_fetch_جمع_کن_explicit(val, 1, memory_order_relaxed) + 1;
}
ساكن FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  أرجع atomic_fetch_جمع_کن_explicit(val, -1, memory_order_relaxed) - 1;
}
ساكن FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t جمع_کن) {
  أرجع atomic_fetch_جمع_کن_explicit(val, جمع_کن, memory_order_relaxed) + جمع_کن;
}
ساكن FORCEINLINE صحيح atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  أرجع atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
ساكن FORCEINLINE فراغ atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ساكن FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  أرجع atomic_بارگیری_explicit(val, memory_order_relaxed);
}
ساكن FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t جمع_کن) {
  أرجع atomic_fetch_جمع_کن_explicit(val, جمع_کن, memory_order_relaxed) + جمع_کن;
}
ساكن FORCEINLINE فراغ *atomic_بارگیری_ptr(atomicptr_t *src) {
  أرجع atomic_بارگیری_explicit(src, memory_order_relaxed);
}
ساكن FORCEINLINE فراغ atomic_store_ptr(atomicptr_t *dst, فراغ *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ساكن FORCEINLINE فراغ atomic_store_ptr_release(atomicptr_t *dst, فراغ *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ساكن FORCEINLINE فراغ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     فراغ *val) {
  أرجع atomic_exchange_explicit(dst, val, memory_order_acquire);
}
ساكن FORCEINLINE صحيح atomic_cas_ptr(atomicptr_t *dst, فراغ *val, فراغ *ref) {
  أرجع atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_relaxed, memory_order_relaxed);
}

#تعریف_کن EXPECTED(x) __builtin_expect((x), 1)
#تعریف_کن UNEXPECTED(x) __builtin_expect((x), 0)

#پایان_اگر

////////////
///
/// Statistics related functions (evaluate to nothing when statistics not
/// enabled)
///
//////

#إذا ENABLE_STATISTICS
#تعریف_کن _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#تعریف_کن _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#تعریف_کن rpmalloc_stat_جمع_کن(counter, مقدار_2)                                     \
  atomic_add32(counter, (int32_t)(مقدار_2))
#تعریف_کن _rpmalloc_stat_add64(counter, مقدار_2)                                   \
  atomic_add64(counter, (int64_t)(مقدار_2))
#تعریف_کن rpmalloc_stat_جمع_کن_peak(counter, مقدار_2, peak)                          \
  افعل {                                                                         \
    int32_t cur_شمار = atomic_add32(counter, (int32_t)(مقدار_2));              \
    إذا (cur_شمار > (peak))                                                   \
      peak = cur_شمار;                                                       \
  } طالما (0)
#تعریف_کن _rpmalloc_stat_sub(counter, مقدار_2)                                     \
  atomic_add32(counter, -(int32_t)(مقدار_2))
#تعریف_کن _rpmalloc_stat_inc_alloc(انباشته, class_idx)                              \
  افعل {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&انباشته->اندازه_class_use[class_idx].alloc_current);         \
    إذا (alloc_current > انباشته->اندازه_class_use[class_idx].alloc_peak)            \
      انباشته->اندازه_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&انباشته->اندازه_class_use[class_idx].alloc_مجموع);               \
  } طالما (0)
#تعریف_کن _rpmalloc_stat_inc_free(انباشته, class_idx)                               \
  افعل {                                                                         \
    atomic_decr32(&انباشته->اندازه_class_use[class_idx].alloc_current);             \
    atomic_incr32(&انباشته->اندازه_class_use[class_idx].free_مجموع);                \
  } طالما (0)
#وإلا
#تعریف_کن _rpmalloc_stat_inc(counter)                                            \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کن _rpmalloc_stat_dec(counter)                                            \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کن rpmalloc_stat_جمع_کن(counter, مقدار_2)                                     \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کن _rpmalloc_stat_add64(counter, مقدار_2)                                   \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کن rpmalloc_stat_جمع_کن_peak(counter, مقدار_2, peak)                          \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کن _rpmalloc_stat_sub(counter, مقدار_2)                                     \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کن _rpmalloc_stat_inc_alloc(انباشته, class_idx)                              \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کن _rpmalloc_stat_inc_free(انباشته, class_idx)                               \
  افعل {                                                                         \
  } طالما (0)
#پایان_اگر

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#تعریف_کن SMALL_GRANULARITY 16
//! Small granularity shift count
#تعریف_کن SMALL_GRANULARITY_لغزش 4
//! Number of small block size classes
#تعریف_کن SMALL_CLASS_شمار 65
//! Maximum size of a small block
#تعریف_کن SMALL_اندازه_حد (SMALL_GRANULARITY * (SMALL_CLASS_شمار - 1))
//! Granularity of a medium allocation block
#تعریف_کن MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#تعریف_کن MEDIUM_GRANULARITY_لغزش 9
//! Number of medium block size classes
#تعریف_کن MEDIUM_CLASS_شمار 61
//! Total number of small + medium size classes
#تعریف_کن اندازه_CLASS_شمار (SMALL_CLASS_شمار + MEDIUM_CLASS_شمار)
//! Number of large block size classes
#تعریف_کن LARGE_CLASS_شمار 63
//! Maximum size of a medium block
#تعریف_کن MEDIUM_اندازه_حد                                                      \
  (SMALL_اندازه_حد + (MEDIUM_GRANULARITY * MEDIUM_CLASS_شمار))
//! Maximum size of a large block
#تعریف_کن LARGE_اندازه_حد                                                       \
  ((LARGE_CLASS_شمار * حافظه_بازه_اندازه) - بازه_سرآیند_اندازه)
//! Size of a span header (must be a multiple of SMALL_GRANULARITY and a power
//! of two)
#تعریف_کن بازه_سرآیند_اندازه 128
//! Number of spans in thread cache
#تعریف_کن MAX_THREAD_بازه_CACHE 400
//! Number of spans to transfer between thread and global cache
#تعریف_کن THREAD_بازه_CACHE_TRANSFER 64
//! Number of spans in thread cache for large spans (must be greater than
//! LARGE_CLASS_COUNT / 2)
#تعریف_کن MAX_THREAD_بازه_LARGE_CACHE 100
//! Number of spans to transfer between thread and global cache for large spans
#تعریف_کن THREAD_بازه_LARGE_CACHE_TRANSFER 6

_Static_assert((SMALL_GRANULARITY & (SMALL_GRANULARITY - 1)) == 0,
               "Small granularity must be power of two");
_Static_assert((بازه_سرآیند_اندازه & (بازه_سرآیند_اندازه - 1)) == 0,
               "Span header size must be power of two");

#إذا ENABLE_VALIDATE_آرگومانها
//! Maximum allocation size to avoid integer overflow
#لغو_تعریف MAX_ALLOC_اندازه
#تعریف_کن MAX_ALLOC_اندازه (((size_t) - 1) - حافظه_بازه_اندازه)
#پایان_اگر

#تعریف_کن اشاره_گر_جابه_جایی(ptr, ofs) (فراغ *)((حرف *)(ptr) + (ptrdiff_t)(ofs))
#تعریف_کن اشاره_گر_diff(اولی_2, دومی_2)                                            \
  (ptrdiff_t)((ثابت حرف *)(اولی_2) - (ثابت حرف *)(دومی_2))

#تعریف_کن INVALID_اشاره_گر ((فراغ *)((uintptr_t) - 1))

#تعریف_کن اندازه_CLASS_LARGE اندازه_CLASS_شمار
#تعریف_کن اندازه_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
تعريفنوع بنية انباشته_t انباشته_t;
//! Span of memory pages
تعريفنوع بنية بازه_t بازه_t;
//! Span list
تعريفنوع بنية بازه_list_t بازه_list_t;
//! Span active data
تعريفنوع بنية بازه_فعال_t بازه_فعال_t;
//! Size class definition
تعريفنوع بنية اندازه_class_t اندازه_class_t;
//! Global cache
تعريفنوع بنية global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#تعریف_کن بازه_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#تعریف_کن بازه_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#تعریف_کن بازه_FLAG_ALIGNED_بلوکها 4U
//! Flag indicating an unmapped master span
#تعریف_کن بازه_FLAG_UNMAPPED_MASTER 8U

#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
بنية بازه_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t بالا;
#إذا ENABLE_STATISTICS
  //! Number of spans in deferred list
  atomic32_t spans_deferred;
  //! Number of spans transitioned to global cache
  atomic32_t spans_مقصد_global;
  //! Number of spans transitioned from global cache
  atomic32_t spans_from_global;
  //! Number of spans transitioned to thread cache
  atomic32_t spans_مقصد_cache;
  //! Number of spans transitioned from thread cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned to reserved state
  atomic32_t spans_مقصد_reserved;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of raw memory map calls
  atomic32_t spans_map_calls;
#پایان_اگر
};
تعريفنوع بنية بازه_use_t بازه_use_t;
#پایان_اگر

#إذا ENABLE_STATISTICS
بنية اندازه_class_use_t {
  //! Current number of allocations
  atomic32_t alloc_current;
  //! Peak number of allocations
  int32_t alloc_peak;
  //! Total number of allocations
  atomic32_t alloc_مجموع;
  //! Total number of frees
  atomic32_t free_مجموع;
  //! Number of spans in use
  atomic32_t spans_current;
  //! Number of spans transitioned to cache
  int32_t spans_peak;
  //! Number of spans transitioned to cache
  atomic32_t spans_مقصد_cache;
  //! Number of spans transitioned from cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of spans mapped
  atomic32_t spans_map_calls;
  int32_t استفادهنشده_2;
};
تعريفنوع بنية اندازه_class_use_t اندازه_class_use_t;
#پایان_اگر

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
بنية بازه_t {
  //! Free list
  فراغ *free_list;
  //! Total block count of size class
  uint32_t بلوک_شمار;
  //! Size class
  uint32_t اندازه_class;
  //! Index of last block initialized in free list
  uint32_t free_list_حد;
  //! Number of used blocks remaining when in partial state
  uint32_t استفاده_شده_شمار;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_اندازه;
  //! Size of a block
  uint32_t بلوک_اندازه_2;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t بازه_شمار;
  //! Total span counter for master spans
  uint32_t مجموع_spans;
  //! Offset from master span for subspans
  uint32_t جابه_جایی_from_master;
  //! Remaining span counter, for master spans
  atomic32_t مانده_spans;
  //! Alignment offset
  uint32_t align_جابه_جایی;
  //! Owning heap
  انباشته_t *انباشته;
  //! Next span
  بازه_t *بعدی;
  //! Previous span
  بازه_t *prev;
};
_Static_assert(حجم(بازه_t) <= بازه_سرآیند_اندازه, "span size mismatch");

بنية بازه_cache_t {
  size_t شمار;
  بازه_t *بازه[MAX_THREAD_بازه_CACHE];
};
تعريفنوع بنية بازه_cache_t بازه_cache_t;

بنية بازه_large_cache_t {
  size_t شمار;
  بازه_t *بازه[MAX_THREAD_بازه_LARGE_CACHE];
};
تعريفنوع بنية بازه_large_cache_t بازه_large_cache_t;

بنية انباشته_اندازه_class_t {
  //! Free list of active span
  فراغ *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  بازه_t *partial_بازه;
  //! Early level cache of fully free spans
  بازه_t *cache;
};
تعريفنوع بنية انباشته_اندازه_class_t انباشته_اندازه_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
بنية انباشته_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  انباشته_اندازه_class_t اندازه_class[اندازه_CLASS_شمار];
#إذا ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  بازه_cache_t بازه_cache;
#پایان_اگر
  //! List of deferred free spans (single linked list)
  atomicptr_t بازه_free_deferred;
  //! Number of full spans
  size_t پر_بازه_شمار;
  //! Mapped but unused spans
  بازه_t *بازه_reserve;
  //! Master span for mapped but unused spans
  بازه_t *بازه_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t فرزند_شمار;
  //! Next heap in id list
  انباشته_t *بعدی_انباشته;
  //! Next heap in orphan list
  انباشته_t *بعدی_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  صحيح finalize;
  //! Master heap owning the memory pages
  انباشته_t *master_انباشته;
#إذا ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  بازه_large_cache_t بازه_large_cache[LARGE_CLASS_شمار - 1];
#پایان_اگر
#إذا RPMALLOC_اولی_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  بازه_t *پر_بازه[اندازه_CLASS_شمار];
  //! Double linked list of large and huge spans allocated by this heap
  بازه_t *large_huge_بازه;
#پایان_اگر
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  بازه_use_t بازه_use[LARGE_CLASS_شمار];
#پایان_اگر
#إذا ENABLE_STATISTICS
  //! Allocation stats per size class
  اندازه_class_use_t اندازه_class_use[اندازه_CLASS_شمار + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_مقصد_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_مقصد_thread;
#پایان_اگر
};

// Size class for defining a block size bucket
بنية اندازه_class_t {
  //! Size of blocks in this class
  uint32_t بلوک_اندازه_2;
  //! Number of blocks in each chunk
  uint16_t بلوک_شمار;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(حجم(اندازه_class_t) == 8, "Size class size mismatch");

بنية global_cache_t {
  //! Cache lock
  atomic32_t قفل_2;
  //! Cache count
  uint32_t شمار;
#إذا ENABLE_STATISTICS
  //! Insert count
  size_t درج_شمار;
  //! Extract count
  size_t استخراج_شمار;
#پایان_اگر
  //! Cached spans
  بازه_t *بازه[GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_بازه_CACHE];
  //! Unlimited cache overflow
  بازه_t *سرریز_2;
};

////////////
///
/// Global data
///
//////

//! Default span size (64KiB)
#تعریف_کن حافظه_default_بازه_اندازه (64 * 1024)
#تعریف_کن حافظه_default_بازه_اندازه_لغزش 16
#تعریف_کن حافظه_default_بازه_ماسک (~((uintptr_t)(حافظه_بازه_اندازه - 1)))

//! Initialized flag
ساكن صحيح _rpmalloc_initialized;
//! Main thread ID
ساكن uintptr_t _rpmalloc_main_thread_id;
//! Configuration
ساكن rpmalloc_config_t حافظه_config;
//! Memory page size
ساكن size_t حافظه_صفحه_اندازه;
//! Shift to divide by page size
ساكن size_t حافظه_صفحه_اندازه_لغزش;
//! Granularity at which memory pages are mapped by OS
ساكن size_t حافظه_map_granularity;
#إذا RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
ساكن size_t حافظه_بازه_اندازه;
//! Shift to divide by span size
ساكن size_t حافظه_بازه_اندازه_لغزش;
//! Mask to get to start of a memory span
ساكن uintptr_t حافظه_بازه_ماسک;
#وإلا
//! Hardwired span size
#تعریف_کن حافظه_بازه_اندازه حافظه_default_بازه_اندازه
#تعریف_کن حافظه_بازه_اندازه_لغزش حافظه_default_بازه_اندازه_لغزش
#تعریف_کن حافظه_بازه_ماسک حافظه_default_بازه_ماسک
#پایان_اگر
//! Number of spans to map in each map call
ساكن size_t حافظه_بازه_map_شمار;
//! Number of spans to keep reserved in each heap
ساكن size_t حافظه_انباشته_reserve_شمار;
//! Global size classes
ساكن اندازه_class_t حافظه_اندازه_class[اندازه_CLASS_شمار];
//! Run-time size limit of medium blocks
ساكن size_t حافظه_medium_اندازه_حد;
//! Heap ID counter
ساكن atomic32_t حافظه_انباشته_id;
//! Huge page support
ساكن صحيح حافظه_huge_صفحات;
#إذا ENABLE_GLOBAL_CACHE
//! Global span cache
ساكن global_cache_t حافظه_بازه_cache[LARGE_CLASS_شمار];
#پایان_اگر
//! Global reserved spans
ساكن بازه_t *حافظه_global_reserve;
//! Global reserved count
ساكن size_t حافظه_global_reserve_شمار;
//! Global reserved master
ساكن بازه_t *حافظه_global_reserve_master;
//! All heaps
ساكن انباشته_t *حافظه_heaps[انباشته_ARRAY_اندازه];
//! Used to restrict access to mapping memory for huge pages
ساكن atomic32_t حافظه_global_قفل;
//! Orphaned heaps
ساكن انباشته_t *حافظه_orphan_heaps;
#إذا RPMALLOC_اولی_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
ساكن انباشته_t *حافظه_اولی_class_orphan_heaps;
#پایان_اگر
#إذا ENABLE_STATISTICS
//! Allocations counter
ساكن atomic64_t تخصیص_counter;
//! Deallocations counter
ساكن atomic64_t _deallocation_counter;
//! Active heap count
ساكن atomic32_t حافظه_فعال_heaps;
//! Number of currently mapped memory pages
ساكن atomic32_t mapped_صفحات;
//! Peak number of concurrently mapped memory pages
ساكن int32_t mapped_صفحات_peak;
//! Number of mapped master spans
ساكن atomic32_t _master_spans;
//! Number of unmapped dangling master spans
ساكن atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
ساكن atomic32_t mapped_مجموع;
//! Running counter of total number of unmapped memory pages since start
ساكن atomic32_t unmapped_مجموع;
//! Number of currently mapped memory pages in OS calls
ساكن atomic32_t mapped_صفحات_os;
//! Number of currently allocated pages in huge allocations
ساكن atomic32_t huge_صفحات_current;
//! Peak number of currently allocated pages in huge allocations
ساكن int32_t huge_صفحات_peak;
#پایان_اگر

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
ساكن pthread_key_t حافظه_thread_انباشته;
#وإلا
#اگر_تعریف_شده _MSC_VER
#تعریف_کن _Thread_local __declspec(thread)
#تعریف_کن TLS_MODEL
#وإلا
#اگر_تعریف_نشده __HAIKU__
#تعریف_کن TLS_MODEL __attribute__((tls_model("initial-exec")))
#وإلا
#تعریف_کن TLS_MODEL
#پایان_اگر
#إذا !defined(__clang__) && defined(__GNUC__)
#تعریف_کن _Thread_local __thread
#پایان_اگر
#پایان_اگر
ساكن _Thread_local انباشته_t *حافظه_thread_انباشته TLS_MODEL;
#پایان_اگر

ساكن مضمن انباشته_t *get_thread_انباشته_raw(فراغ) {
#إذا (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  أرجع pthread_getspecific(حافظه_thread_انباشته);
#وإلا
  أرجع حافظه_thread_انباشته;
#پایان_اگر
}

//! Get the current thread heap
ساكن مضمن انباشته_t *get_thread_انباشته(فراغ) {
  انباشته_t *انباشته = get_thread_انباشته_raw();
#إذا ENABLE_PRELOAD
  إذا (EXPECTED(انباشته != 0))
    أرجع انباشته;
  rpmalloc_initialize();
  أرجع get_thread_انباشته_raw();
#وإلا
  أرجع انباشته;
#پایان_اگر
}

//! Fast thread ID
ساكن مضمن uintptr_t get_thread_id(فراغ) {
#إذا defined(_WIN32)
  أرجع (uintptr_t)((فراغ *)NtCurrentTeb());
#وإلاإذا (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#إذا defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#وإلاإذا defined(__x86_64__)
#إذا defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#وإلا
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#پایان_اگر
#وإلاإذا defined(__arm__)
  __asm__ متطاير("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#وإلاإذا defined(__aarch64__)
#إذا defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ متطاير("mrs %0, tpidrro_el0" : "=r"(tid));
#وإلا
  __asm__ متطاير("mrs %0, tpidr_el0" : "=r"(tid));
#پایان_اگر
#وإلا
#خطا This platform needs implementation از_3 get_thread_id()
#پایان_اگر
  أرجع tid;
#وإلا
#خطا This platform needs implementation از_3 get_thread_id()
#پایان_اگر
}

//! Set the current thread heap
ساكن فراغ مجموعه_thread_انباشته(انباشته_t *انباشته) {
#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(حافظه_thread_انباشته, انباشته);
#وإلا
  حافظه_thread_انباشته = انباشته;
#پایان_اگر
  إذا (انباشته)
    انباشته->owner_thread = get_thread_id();
}

//! Set main thread ID
خارجي فراغ rpmalloc_مجموعه_main_thread(فراغ);

فراغ rpmalloc_مجموعه_main_thread(فراغ) {
  _rpmalloc_main_thread_id = get_thread_id();
}

ساكن فراغ _rpmalloc_spin(فراغ) {
#إذا defined(_MSC_VER)
#إذا defined(_M_ARM64)
  __yield();
#وإلا
  _mm_pause();
#پایان_اگر
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
#پایان_اگر
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

#اگر_تعریف_شده _WIN64
#رهنمود comment(linker, "/INCLUDE:_tls_used")
#رهنمود comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#رهنمود const_seg(".CRT$XLY")

خارجي ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#رهنمود const_seg()
#وإلا // _WIN64
#رهنمود comment(linker, "/INCLUDE:__tls_used")
#رهنمود comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#رهنمود داده_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#رهنمود داده_seg()
#پایان_اگر // _WIN64

ساكن فراغ NTAPI _rpmalloc_thread_destructor(فراغ *مقدار_2) {
#إذا ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  إذا (get_thread_id() == _rpmalloc_main_thread_id)
    أرجع;
#پایان_اگر
  إذا (مقدار_2)
    rpmalloc_thread_finalize(1);
}
#پایان_اگر

////////////
///
/// Low level memory map/unmap
///
//////

ساكن فراغ rpmalloc_مجموعه_name(فراغ *آدرس_2, size_t اندازه_2) {
#إذا defined(__linux__) || defined(__ANDROID__)
  ثابت حرف *name = حافظه_huge_صفحات ? حافظه_config.huge_صفحه_name
                                        : حافظه_config.صفحه_name;
  إذا (آدرس_2 == MAP_FAILED || !name)
    أرجع;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (فراغ)prctl(PR_مجموعه_VMA, PR_مجموعه_VMA_ANON_NAME, (uintptr_t)آدرس_2, اندازه_2,
              (uintptr_t)name);
#وإلا
  (فراغ)حجم(اندازه_2);
  (فراغ)حجم(آدرس_2);
#پایان_اگر
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
ساكن فراغ *_rpmalloc_mmap(size_t اندازه_2, size_t *جابه_جایی_2) {
  rpmalloc_assert(!(اندازه_2 % حافظه_صفحه_اندازه), "Invalid mmap size");
  rpmalloc_assert(اندازه_2 >= حافظه_صفحه_اندازه, "Invalid mmap size");
  فراغ *آدرس_2 = حافظه_config.حافظه_map(اندازه_2, جابه_جایی_2);
  إذا (EXPECTED(آدرس_2 != 0)) {
    rpmalloc_stat_جمع_کن_peak(&mapped_صفحات, (اندازه_2 >> حافظه_صفحه_اندازه_لغزش),
                            mapped_صفحات_peak);
    rpmalloc_stat_جمع_کن(&mapped_مجموع, (اندازه_2 >> حافظه_صفحه_اندازه_لغزش));
  }
  أرجع آدرس_2;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
ساكن فراغ _rpmalloc_unmap(فراغ *آدرس_2, size_t اندازه_2, size_t جابه_جایی_2,
                            size_t release) {
  rpmalloc_assert(!release || (release >= اندازه_2), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= حافظه_صفحه_اندازه),
                  "Invalid unmap size");
  إذا (release) {
    rpmalloc_assert(!(release % حافظه_صفحه_اندازه), "Invalid unmap size");
    _rpmalloc_stat_sub(&mapped_صفحات, (release >> حافظه_صفحه_اندازه_لغزش));
    rpmalloc_stat_جمع_کن(&unmapped_مجموع, (release >> حافظه_صفحه_اندازه_لغزش));
  }
  حافظه_config.حافظه_unmap(آدرس_2, اندازه_2, جابه_جایی_2, release);
}

//! Default implementation to map new pages to virtual memory
ساكن فراغ *_rpmalloc_mmap_os(size_t اندازه_2, size_t *جابه_جایی_2) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((اندازه_2 >= حافظه_بازه_اندازه) &&
                    (حافظه_بازه_اندازه > حافظه_map_granularity))
                       ? حافظه_بازه_اندازه
                       : 0;
  rpmalloc_assert(اندازه_2 >= حافظه_صفحه_اندازه, "Invalid mmap size");
#إذا PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  فراغ *ptr = VirtualAlloc(0, اندازه_2 + padding,
                           (حافظه_huge_صفحات ? MEM_LARGE_صفحات : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           صفحه_READWRITE);
  إذا (!ptr) {
    إذا (حافظه_config.map_fail_callback) {
      إذا (حافظه_config.map_fail_callback(اندازه_2 + padding))
        أرجع _rpmalloc_mmap_os(اندازه_2, جابه_جایی_2);
    } وإلا {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    أرجع 0;
  }
#وإلا
  صحيح flags = MAP_خصوصی | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#إذا defined(__APPLE__) && !هدف_OS_IPHONE && !هدف_OS_SIMULATOR
  صحيح fd = (صحيح)VM_MAKE_برچسب(240U);
  إذا (حافظه_huge_صفحات)
    fd |= VM_FLAGS_SUPERPAGE_اندازه_2_MB;
  فراغ *ptr = mmap(0, اندازه_2 + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#وإلاإذا defined(MAP_HUGETLB)
  فراغ *ptr = mmap(0, اندازه_2 + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (حافظه_huge_صفحات ? MAP_HUGETLB : 0) | flags, -1, 0);
#إذا defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  إذا ((ptr == MAP_FAILED || !ptr) && حافظه_huge_صفحات) {
    ptr = mmap(0, اندازه_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    إذا (ptr && ptr != MAP_FAILED) {
      صحيح prm = madvise(ptr, اندازه_2 + padding, MADV_HUGEPAGE);
      (فراغ)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#پایان_اگر
  rpmalloc_مجموعه_name(ptr, اندازه_2 + padding);
#وإلاإذا defined(MAP_ALIGNED)
  ثابت size_t align =
      (حجم(size_t) * 8) - (size_t)(__builtin_clzl(اندازه_2 - 1));
  فراغ *ptr =
      mmap(0, اندازه_2 + padding, PROT_READ | PROT_WRITE,
           (حافظه_huge_صفحات ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#وإلاإذا defined(MAP_ALIGN)
  caddr_t پایه_2 = (حافظه_huge_صفحات ? (caddr_t)(4 << 20) : 0);
  فراغ *ptr = mmap(پایه_2, اندازه_2 + padding, PROT_READ | PROT_WRITE,
                   (حافظه_huge_صفحات ? MAP_ALIGN : 0) | flags, -1, 0);
#وإلا
  فراغ *ptr = mmap(0, اندازه_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#پایان_اگر
  إذا ((ptr == MAP_FAILED) || !ptr) {
    إذا (حافظه_config.map_fail_callback) {
      إذا (حافظه_config.map_fail_callback(اندازه_2 + padding))
        أرجع _rpmalloc_mmap_os(اندازه_2, جابه_جایی_2);
    } وإلا إذا (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    أرجع 0;
  }
#پایان_اگر
  rpmalloc_stat_جمع_کن(&mapped_صفحات_os,
                     (int32_t)((اندازه_2 + padding) >> حافظه_صفحه_اندازه_لغزش));
  إذا (padding) {
    size_t final_padding = padding - ((uintptr_t)ptr & ~حافظه_بازه_ماسک);
    rpmalloc_assert(final_padding <= حافظه_بازه_اندازه,
                    "Internal failure in padding");
    rpmalloc_assert(final_padding <= padding, "Internal failure in padding");
    rpmalloc_assert(!(final_padding % 8), "Internal failure in padding");
    ptr = اشاره_گر_جابه_جایی(ptr, final_padding);
    *جابه_جایی_2 = final_padding >> 3;
  }
  rpmalloc_assert((اندازه_2 < حافظه_بازه_اندازه) ||
                      !((uintptr_t)ptr & ~حافظه_بازه_ماسک),
                  "Internal failure in padding");
  أرجع ptr;
}

//! Default implementation to unmap pages from virtual memory
ساكن فراغ _rpmalloc_unmap_os(فراغ *آدرس_2, size_t اندازه_2, size_t جابه_جایی_2,
                               size_t release) {
  rpmalloc_assert(release || (جابه_جایی_2 == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= حافظه_صفحه_اندازه),
                  "Invalid unmap size");
  rpmalloc_assert(اندازه_2 >= حافظه_صفحه_اندازه, "Invalid unmap size");
  إذا (release && جابه_جایی_2) {
    جابه_جایی_2 <<= 3;
    آدرس_2 = اشاره_گر_جابه_جایی(آدرس_2, -(int32_t)جابه_جایی_2);
    إذا ((release >= حافظه_بازه_اندازه) &&
        (حافظه_بازه_اندازه > حافظه_map_granularity)) {
      // Padding is always one span size
      release += حافظه_بازه_اندازه;
    }
  }
#إذا !DISABLE_UNMAP
#إذا PLATFORM_WINDOWS
  إذا (!VirtualFree(آدرس_2, release ? 0 : اندازه_2,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#وإلا
  إذا (release) {
    إذا (munmap(آدرس_2, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } وإلا {
#إذا defined(MADV_FREE_REUSABLE)
    صحيح ret;
    طالما ((ret = madvise(آدرس_2, اندازه_2, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    إذا ((ret == -1) && (errno != 0)) {
#وإلاإذا defined(MADV_DONTNEED)
    إذا (madvise(آدرس_2, اندازه_2, MADV_DONTNEED)) {
#وإلاإذا defined(MADV_PAGEOUT)
    إذا (madvise(آدرس_2, اندازه_2, MADV_PAGEOUT)) {
#وإلاإذا defined(MADV_FREE)
    إذا (madvise(آدرس_2, اندازه_2, MADV_FREE)) {
#وإلا
    إذا (posix_madvise(آدرس_2, اندازه_2, POSIX_MADV_DONTNEED)) {
#پایان_اگر
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#پایان_اگر
#پایان_اگر
  إذا (release)
    _rpmalloc_stat_sub(&mapped_صفحات_os, release >> حافظه_صفحه_اندازه_لغزش);
}

ساكن فراغ rpmalloc_بازه_mark_as_subspan_unless_master(بازه_t *master,
                                                         بازه_t *subspan,
                                                         size_t بازه_شمار);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
ساكن بازه_t *_rpmalloc_global_get_reserved_spans(size_t بازه_شمار) {
  بازه_t *بازه = حافظه_global_reserve;
  rpmalloc_بازه_mark_as_subspan_unless_master(حافظه_global_reserve_master,
                                               بازه, بازه_شمار);
  حافظه_global_reserve_شمار -= بازه_شمار;
  إذا (حافظه_global_reserve_شمار)
    حافظه_global_reserve =
        (بازه_t *)اشاره_گر_جابه_جایی(بازه, بازه_شمار << حافظه_بازه_اندازه_لغزش);
  وإلا
    حافظه_global_reserve = 0;
  أرجع بازه;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
ساكن فراغ rpmalloc_global_مجموعه_reserved_spans(بازه_t *master, بازه_t *reserve,
                                                size_t reserve_بازه_شمار) {
  حافظه_global_reserve_master = master;
  حافظه_global_reserve_شمار = reserve_بازه_شمار;
  حافظه_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
ساكن فراغ rpmalloc_بازه_double_link_list_جمع_کن(بازه_t **سر, بازه_t *بازه) {
  إذا (*سر)
    (*سر)->prev = بازه;
  بازه->بعدی = *سر;
  *سر = بازه;
}

//! Pop head span from double linked list
ساكن فراغ rpmalloc_بازه_double_link_list_برداشت_سر(بازه_t **سر,
                                                     بازه_t *بازه) {
  rpmalloc_assert(*سر == بازه, "Linked list corrupted");
  بازه = *سر;
  *سر = بازه->بعدی;
}

//! Remove a span from double linked list
ساكن فراغ rpmalloc_بازه_double_link_list_remove(بازه_t **سر,
                                                   بازه_t *بازه) {
  rpmalloc_assert(*سر, "Linked list corrupted");
  إذا (*سر == بازه) {
    *سر = بازه->بعدی;
  } وإلا {
    بازه_t *بعدی_بازه = بازه->بعدی;
    بازه_t *prev_بازه = بازه->prev;
    prev_بازه->بعدی = بعدی_بازه;
    إذا (EXPECTED(بعدی_بازه != 0))
      بعدی_بازه->prev = prev_بازه;
  }
}

////////////
///
/// Span control
///
//////

ساكن فراغ rpmalloc_انباشته_cache_درج(انباشته_t *انباشته, بازه_t *بازه);

ساكن فراغ rpmalloc_انباشته_finalize(انباشته_t *انباشته);

ساكن فراغ rpmalloc_انباشته_مجموعه_reserved_spans(انباشته_t *انباشته, بازه_t *master,
                                              بازه_t *reserve,
                                              size_t reserve_بازه_شمار);

//! Declare the span to be a subspan and store distance from master span and
//! span count
ساكن فراغ rpmalloc_بازه_mark_as_subspan_unless_master(بازه_t *master,
                                                         بازه_t *subspan,
                                                         size_t بازه_شمار) {
  rpmalloc_assert((subspan != master) || (subspan->flags & بازه_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  إذا (subspan != master) {
    subspan->flags = بازه_FLAG_SUBSPAN;
    subspan->جابه_جایی_from_master =
        (uint32_t)((uintptr_t)اشاره_گر_diff(subspan, master) >>
                   حافظه_بازه_اندازه_لغزش);
    subspan->align_جابه_جایی = 0;
  }
  subspan->بازه_شمار = (uint32_t)بازه_شمار;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
ساكن بازه_t *rpmalloc_بازه_map_from_reserve(انباشته_t *انباشته,
                                               size_t بازه_شمار) {
  // Update the heap span reserve
  بازه_t *بازه = انباشته->بازه_reserve;
  انباشته->بازه_reserve =
      (بازه_t *)اشاره_گر_جابه_جایی(بازه, بازه_شمار * حافظه_بازه_اندازه);
  انباشته->spans_reserved -= (uint32_t)بازه_شمار;

  rpmalloc_بازه_mark_as_subspan_unless_master(انباشته->بازه_reserve_master, بازه,
                                               بازه_شمار);
  إذا (بازه_شمار <= LARGE_CLASS_شمار)
    _rpmalloc_stat_inc(&انباشته->بازه_use[بازه_شمار - 1].spans_from_reserved);

  أرجع بازه;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
ساكن size_t rpmalloc_بازه_align_شمار(size_t بازه_شمار) {
  size_t request_شمار = (بازه_شمار > حافظه_بازه_map_شمار)
                             ? بازه_شمار
                             : حافظه_بازه_map_شمار;
  إذا ((حافظه_صفحه_اندازه > حافظه_بازه_اندازه) &&
      ((request_شمار * حافظه_بازه_اندازه) % حافظه_صفحه_اندازه))
    request_شمار +=
        حافظه_بازه_map_شمار - (request_شمار % حافظه_بازه_map_شمار);
  أرجع request_شمار;
}

//! Setup a newly mapped span
ساكن فراغ rpmalloc_بازه_initialize(بازه_t *بازه, size_t مجموع_بازه_شمار,
                                      size_t بازه_شمار, size_t align_جابه_جایی) {
  بازه->مجموع_spans = (uint32_t)مجموع_بازه_شمار;
  بازه->بازه_شمار = (uint32_t)بازه_شمار;
  بازه->align_جابه_جایی = (uint32_t)align_جابه_جایی;
  بازه->flags = بازه_FLAG_MASTER;
  atomic_store32(&بازه->مانده_spans, (int32_t)مجموع_بازه_شمار);
}

ساكن فراغ rpmalloc_بازه_unmap(بازه_t *بازه);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
ساكن بازه_t *rpmalloc_بازه_map_aligned_شمار(انباشته_t *انباشته,
                                                size_t بازه_شمار) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_بازه_شمار = rpmalloc_بازه_align_شمار(بازه_شمار);
  size_t align_جابه_جایی = 0;
  بازه_t *بازه = (بازه_t *)_rpmalloc_mmap(
      aligned_بازه_شمار * حافظه_بازه_اندازه, &align_جابه_جایی);
  إذا (!بازه)
    أرجع 0;
  rpmalloc_بازه_initialize(بازه, aligned_بازه_شمار, بازه_شمار, align_جابه_جایی);
  _rpmalloc_stat_inc(&_master_spans);
  إذا (بازه_شمار <= LARGE_CLASS_شمار)
    _rpmalloc_stat_inc(&انباشته->بازه_use[بازه_شمار - 1].spans_map_calls);
  إذا (aligned_بازه_شمار > بازه_شمار) {
    بازه_t *reserved_spans =
        (بازه_t *)اشاره_گر_جابه_جایی(بازه, بازه_شمار * حافظه_بازه_اندازه);
    size_t reserved_شمار = aligned_بازه_شمار - بازه_شمار;
    إذا (انباشته->spans_reserved) {
      rpmalloc_بازه_mark_as_subspan_unless_master(
          انباشته->بازه_reserve_master, انباشته->بازه_reserve, انباشته->spans_reserved);
      rpmalloc_انباشته_cache_درج(انباشته, انباشته->بازه_reserve);
    }
    إذا (reserved_شمار > حافظه_انباشته_reserve_شمار) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&حافظه_global_قفل) == 1,
                      "Global spin lock not held as expected");
      size_t remain_شمار = reserved_شمار - حافظه_انباشته_reserve_شمار;
      reserved_شمار = حافظه_انباشته_reserve_شمار;
      بازه_t *remain_بازه = (بازه_t *)اشاره_گر_جابه_جایی(
          reserved_spans, reserved_شمار * حافظه_بازه_اندازه);
      إذا (حافظه_global_reserve) {
        rpmalloc_بازه_mark_as_subspan_unless_master(
            حافظه_global_reserve_master, حافظه_global_reserve,
            حافظه_global_reserve_شمار);
        rpmalloc_بازه_unmap(حافظه_global_reserve);
      }
      rpmalloc_global_مجموعه_reserved_spans(بازه, remain_بازه, remain_شمار);
    }
    rpmalloc_انباشته_مجموعه_reserved_spans(انباشته, بازه, reserved_spans,
                                      reserved_شمار);
  }
  أرجع بازه;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
ساكن بازه_t *rpmalloc_بازه_map(انباشته_t *انباشته, size_t بازه_شمار) {
  إذا (بازه_شمار <= انباشته->spans_reserved)
    أرجع rpmalloc_بازه_map_from_reserve(انباشته, بازه_شمار);
  بازه_t *بازه = 0;
  صحيح use_global_reserve =
      (حافظه_صفحه_اندازه > حافظه_بازه_اندازه) ||
      (حافظه_بازه_map_شمار > حافظه_انباشته_reserve_شمار);
  إذا (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    طالما (!atomic_cas32_acquire(&حافظه_global_قفل, 1, 0))
      _rpmalloc_spin();
    إذا (حافظه_global_reserve_شمار >= بازه_شمار) {
      size_t reserve_شمار =
          (!انباشته->spans_reserved ? حافظه_انباشته_reserve_شمار : بازه_شمار);
      إذا (حافظه_global_reserve_شمار < reserve_شمار)
        reserve_شمار = حافظه_global_reserve_شمار;
      بازه = _rpmalloc_global_get_reserved_spans(reserve_شمار);
      إذا (بازه) {
        إذا (reserve_شمار > بازه_شمار) {
          بازه_t *reserved_بازه = (بازه_t *)اشاره_گر_جابه_جایی(
              بازه, بازه_شمار << حافظه_بازه_اندازه_لغزش);
          rpmalloc_انباشته_مجموعه_reserved_spans(انباشته, حافظه_global_reserve_master,
                                            reserved_بازه,
                                            reserve_شمار - بازه_شمار);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        بازه->بازه_شمار = (uint32_t)بازه_شمار;
      }
    }
  }
  إذا (!بازه)
    بازه = rpmalloc_بازه_map_aligned_شمار(انباشته, بازه_شمار);
  إذا (use_global_reserve)
    atomic_store32_release(&حافظه_global_قفل, 0);
  أرجع بازه;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
ساكن فراغ rpmalloc_بازه_unmap(بازه_t *بازه) {
  rpmalloc_assert((بازه->flags & بازه_FLAG_MASTER) ||
                      (بازه->flags & بازه_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(بازه->flags & بازه_FLAG_MASTER) ||
                      !(بازه->flags & بازه_FLAG_SUBSPAN),
                  "Span flag corrupted");

  صحيح is_master = !!(بازه->flags & بازه_FLAG_MASTER);
  بازه_t *master =
      is_master ? بازه
                : ((بازه_t *)اشاره_گر_جابه_جایی(
                      بازه, -(intptr_t)((uintptr_t)بازه->جابه_جایی_from_master *
                                        حافظه_بازه_اندازه)));
  rpmalloc_assert(is_master || (بازه->flags & بازه_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & بازه_FLAG_MASTER, "Span flag corrupted");

  size_t بازه_شمار = بازه->بازه_شمار;
  إذا (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(بازه->align_جابه_جایی == 0, "Span align offset corrupted");
    إذا (حافظه_بازه_اندازه >= حافظه_صفحه_اندازه)
      _rpmalloc_unmap(بازه, بازه_شمار * حافظه_بازه_اندازه, 0, 0);
  } وإلا {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    بازه->flags |=
        بازه_FLAG_MASTER | بازه_FLAG_SUBSPAN | بازه_FLAG_UNMAPPED_MASTER;
    rpmalloc_stat_جمع_کن(&_unmapped_master_spans, 1);
  }

  إذا (atomic_add32(&master->مانده_spans, -(int32_t)بازه_شمار) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & بازه_FLAG_MASTER) &&
                        !!(master->flags & بازه_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_شمار = master->بازه_شمار;
    إذا (حافظه_بازه_اندازه < حافظه_صفحه_اندازه)
      unmap_شمار = master->مجموع_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_شمار * حافظه_بازه_اندازه,
                    master->align_جابه_جایی,
                    (size_t)master->مجموع_spans * حافظه_بازه_اندازه);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
ساكن فراغ rpmalloc_بازه_release_مقصد_cache(انباشته_t *انباشته, بازه_t *بازه) {
  rpmalloc_assert(انباشته == بازه->انباشته, "Span heap pointer corrupted");
  rpmalloc_assert(بازه->اندازه_class < اندازه_CLASS_شمار,
                  "Invalid span size class");
  rpmalloc_assert(بازه->بازه_شمار == 1, "Invalid span count");
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&انباشته->بازه_use[0].current);
#پایان_اگر
  _rpmalloc_stat_dec(&انباشته->اندازه_class_use[بازه->اندازه_class].spans_current);
  إذا (!انباشته->finalize) {
    _rpmalloc_stat_inc(&انباشته->بازه_use[0].spans_مقصد_cache);
    _rpmalloc_stat_inc(&انباشته->اندازه_class_use[بازه->اندازه_class].spans_مقصد_cache);
    إذا (انباشته->اندازه_class[بازه->اندازه_class].cache)
      rpmalloc_انباشته_cache_درج(انباشته,
                                  انباشته->اندازه_class[بازه->اندازه_class].cache);
    انباشته->اندازه_class[بازه->اندازه_class].cache = بازه;
  } وإلا {
    rpmalloc_بازه_unmap(بازه);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
ساكن uint32_t free_list_partial_init(فراغ **list, فراغ **اولی_بلوک,
                                       فراغ *صفحه_شروع, فراغ *بلوک_شروع,
                                       uint32_t بلوک_شمار,
                                       uint32_t بلوک_اندازه_2) {
  rpmalloc_assert(بلوک_شمار, "Internal failure");
  *اولی_بلوک = بلوک_شروع;
  إذا (بلوک_شمار > 1) {
    فراغ *free_بلوک = اشاره_گر_جابه_جایی(بلوک_شروع, بلوک_اندازه_2);
    فراغ *بلوک_انتها =
        اشاره_گر_جابه_جایی(بلوک_شروع, (size_t)بلوک_اندازه_2 * بلوک_شمار);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    إذا (بلوک_اندازه_2 < (حافظه_صفحه_اندازه >> 1)) {
      فراغ *صفحه_انتها = اشاره_گر_جابه_جایی(صفحه_شروع, حافظه_صفحه_اندازه);
      إذا (صفحه_انتها < بلوک_انتها)
        بلوک_انتها = صفحه_انتها;
    }
    *list = free_بلوک;
    بلوک_شمار = 2;
    فراغ *بعدی_بلوک = اشاره_گر_جابه_جایی(free_بلوک, بلوک_اندازه_2);
    طالما (بعدی_بلوک < بلوک_انتها) {
      *((فراغ **)free_بلوک) = بعدی_بلوک;
      free_بلوک = بعدی_بلوک;
      ++بلوک_شمار;
      بعدی_بلوک = اشاره_گر_جابه_جایی(بعدی_بلوک, بلوک_اندازه_2);
    }
    *((فراغ **)free_بلوک) = 0;
  } وإلا {
    *list = 0;
  }
  أرجع بلوک_شمار;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
ساكن فراغ *rpmalloc_بازه_initialize_new(انباشته_t *انباشته,
                                           انباشته_اندازه_class_t *انباشته_اندازه_class,
                                           بازه_t *بازه, uint32_t class_idx) {
  rpmalloc_assert(بازه->بازه_شمار == 1, "Internal failure");
  اندازه_class_t *اندازه_class = حافظه_اندازه_class + class_idx;
  بازه->اندازه_class = class_idx;
  بازه->انباشته = انباشته;
  بازه->flags &= ~بازه_FLAG_ALIGNED_بلوکها;
  بازه->بلوک_اندازه_2 = اندازه_class->بلوک_اندازه_2;
  بازه->بلوک_شمار = اندازه_class->بلوک_شمار;
  بازه->free_list = 0;
  بازه->list_اندازه = 0;
  atomic_store_ptr_release(&بازه->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  فراغ *بلوک;
  بازه->free_list_حد =
      free_list_partial_init(&انباشته_اندازه_class->free_list, &بلوک, بازه,
                             اشاره_گر_جابه_جایی(بازه, بازه_سرآیند_اندازه),
                             اندازه_class->بلوک_شمار, اندازه_class->بلوک_اندازه_2);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  إذا (بازه->free_list_حد < بازه->بلوک_شمار) {
    rpmalloc_بازه_double_link_list_جمع_کن(&انباشته_اندازه_class->partial_بازه, بازه);
    بازه->استفاده_شده_شمار = بازه->free_list_حد;
  } وإلا {
#إذا RPMALLOC_اولی_CLASS_HEAPS
    rpmalloc_بازه_double_link_list_جمع_کن(&انباشته->پر_بازه[class_idx], بازه);
#پایان_اگر
    ++انباشته->پر_بازه_شمار;
    بازه->استفاده_شده_شمار = بازه->بلوک_شمار;
  }
  أرجع بلوک;
}

ساكن فراغ rpmalloc_بازه_استخراج_free_list_deferred(بازه_t *بازه) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  افعل {
    بازه->free_list =
        atomic_exchange_ptr_acquire(&بازه->free_list_deferred, INVALID_اشاره_گر);
  } طالما (بازه->free_list == INVALID_اشاره_گر);
  بازه->استفاده_شده_شمار -= بازه->list_اندازه;
  بازه->list_اندازه = 0;
  atomic_store_ptr_release(&بازه->free_list_deferred, 0);
}

ساكن صحيح rpmalloc_بازه_is_fully_utilized(بازه_t *بازه) {
  rpmalloc_assert(بازه->free_list_حد <= بازه->بلوک_شمار,
                  "Span free list corrupted");
  أرجع !بازه->free_list && (بازه->free_list_حد >= بازه->بلوک_شمار);
}

ساكن صحيح rpmalloc_بازه_finalize(انباشته_t *انباشته, size_t iclass, بازه_t *بازه,
                                   بازه_t **list_سر) {
  فراغ *free_list = انباشته->اندازه_class[iclass].free_list;
  بازه_t *class_بازه = (بازه_t *)((uintptr_t)free_list & حافظه_بازه_ماسک);
  إذا (بازه == class_بازه) {
    // Adopt the heap class free list back into the span free list
    فراغ *بلوک = بازه->free_list;
    فراغ *آخرین_بلوک = 0;
    طالما (بلوک) {
      آخرین_بلوک = بلوک;
      بلوک = *((فراغ **)بلوک);
    }
    uint32_t free_شمار = 0;
    بلوک = free_list;
    طالما (بلوک) {
      ++free_شمار;
      بلوک = *((فراغ **)بلوک);
    }
    إذا (آخرین_بلوک) {
      *((فراغ **)آخرین_بلوک) = free_list;
    } وإلا {
      بازه->free_list = free_list;
    }
    انباشته->اندازه_class[iclass].free_list = 0;
    بازه->استفاده_شده_شمار -= free_شمار;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(بازه->list_اندازه == بازه->استفاده_شده_شمار, "Memory leak detected");
  إذا (بازه->list_اندازه == بازه->استفاده_شده_شمار) {
    _rpmalloc_stat_dec(&انباشته->بازه_use[0].current);
    _rpmalloc_stat_dec(&انباشته->اندازه_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    إذا (list_سر)
      rpmalloc_بازه_double_link_list_remove(list_سر, بازه);
    rpmalloc_بازه_unmap(بازه);
    أرجع 1;
  }
  أرجع 0;
}

////////////
///
/// Global cache
///
//////

#إذا ENABLE_GLOBAL_CACHE

//! Finalize a global cache
ساكن فراغ _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  طالما (!atomic_cas32_acquire(&cache->قفل_2, 1, 0))
    _rpmalloc_spin();

  لكل (size_t ispan = 0; ispan < cache->شمار; ++ispan)
    rpmalloc_بازه_unmap(cache->بازه[ispan]);
  cache->شمار = 0;

  طالما (cache->سرریز_2) {
    بازه_t *بازه = cache->سرریز_2;
    cache->سرریز_2 = بازه->بعدی;
    rpmalloc_بازه_unmap(بازه);
  }

  atomic_store32_release(&cache->قفل_2, 0);
}

ساكن فراغ rpmalloc_global_cache_درج_spans(بازه_t **بازه,
                                                size_t بازه_شمار,
                                                size_t شمار) {
  ثابت size_t cache_حد =
      (بازه_شمار == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_بازه_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_بازه_LARGE_CACHE - (بازه_شمار >> 1));

  global_cache_t *cache = &حافظه_بازه_cache[بازه_شمار - 1];

  size_t درج_شمار = شمار;
  طالما (!atomic_cas32_acquire(&cache->قفل_2, 1, 0))
    _rpmalloc_spin();

#إذا ENABLE_STATISTICS
  cache->درج_شمار += شمار;
#پایان_اگر
  إذا ((cache->شمار + درج_شمار) > cache_حد)
    درج_شمار = cache_حد - cache->شمار;

  کپی_حافظه(cache->بازه + cache->شمار, بازه, حجم(بازه_t *) * درج_شمار);
  cache->شمار += (uint32_t)درج_شمار;

#إذا ENABLE_UNLIMITED_CACHE
  طالما (درج_شمار < شمار) {
#وإلا
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  طالما ((حافظه_صفحه_اندازه > حافظه_بازه_اندازه) && (درج_شمار < شمار)) {
#پایان_اگر
    بازه_t *current_بازه = بازه[درج_شمار++];
    current_بازه->بعدی = cache->سرریز_2;
    cache->سرریز_2 = current_بازه;
  }
  atomic_store32_release(&cache->قفل_2, 0);

  بازه_t *keep = 0;
  لكل (size_t ispan = درج_شمار; ispan < شمار; ++ispan) {
    بازه_t *current_بازه = بازه[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    إذا ((current_بازه->flags & بازه_FLAG_MASTER) &&
        (atomic_load32(&current_بازه->مانده_spans) >
         (int32_t)current_بازه->بازه_شمار)) {
      current_بازه->بعدی = keep;
      keep = current_بازه;
    } وإلا {
      rpmalloc_بازه_unmap(current_بازه);
    }
  }

  إذا (keep) {
    طالما (!atomic_cas32_acquire(&cache->قفل_2, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    طالما (keep) {
      لكل (; islot < cache->شمار; ++islot) {
        بازه_t *current_بازه = cache->بازه[islot];
        إذا (!(current_بازه->flags & بازه_FLAG_MASTER) ||
            ((current_بازه->flags & بازه_FLAG_MASTER) &&
             (atomic_load32(&current_بازه->مانده_spans) <=
              (int32_t)current_بازه->بازه_شمار))) {
          rpmalloc_بازه_unmap(current_بازه);
          cache->بازه[islot] = keep;
          اكسر;
        }
      }
      إذا (islot == cache->شمار)
        اكسر;
      keep = keep->بعدی;
    }

    إذا (keep) {
      بازه_t *دم_2 = keep;
      طالما (دم_2->بعدی)
        دم_2 = دم_2->بعدی;
      دم_2->بعدی = cache->سرریز_2;
      cache->سرریز_2 = keep;
    }

    atomic_store32_release(&cache->قفل_2, 0);
  }
}

ساكن size_t rpmalloc_global_cache_استخراج_spans(بازه_t **بازه,
                                                   size_t بازه_شمار,
                                                   size_t شمار) {
  global_cache_t *cache = &حافظه_بازه_cache[بازه_شمار - 1];

  size_t استخراج_شمار = 0;
  طالما (!atomic_cas32_acquire(&cache->قفل_2, 1, 0))
    _rpmalloc_spin();

#إذا ENABLE_STATISTICS
  cache->استخراج_شمار += شمار;
#پایان_اگر
  size_t want = شمار - استخراج_شمار;
  إذا (want > cache->شمار)
    want = cache->شمار;

  کپی_حافظه(بازه + استخراج_شمار, cache->بازه + (cache->شمار - want),
         حجم(بازه_t *) * want);
  cache->شمار -= (uint32_t)want;
  استخراج_شمار += want;

  طالما ((استخراج_شمار < شمار) && cache->سرریز_2) {
    بازه_t *current_بازه = cache->سرریز_2;
    بازه[استخراج_شمار++] = current_بازه;
    cache->سرریز_2 = current_بازه->بعدی;
  }

#إذا ENABLE_ASSERTS
  لكل (size_t ispan = 0; ispan < استخراج_شمار; ++ispan) {
    rpmalloc_assert(بازه[ispan]->بازه_شمار == بازه_شمار,
                    "Global cache span count mismatch");
  }
#پایان_اگر

  atomic_store32_release(&cache->قفل_2, 0);

  أرجع استخراج_شمار;
}

#پایان_اگر

////////////
///
/// Heap control
///
//////

ساكن فراغ _rpmalloc_deallocate_huge(بازه_t *);

//! Store the given spans as reserve in the given heap
ساكن فراغ rpmalloc_انباشته_مجموعه_reserved_spans(انباشته_t *انباشته, بازه_t *master,
                                              بازه_t *reserve,
                                              size_t reserve_بازه_شمار) {
  انباشته->بازه_reserve_master = master;
  انباشته->بازه_reserve = reserve;
  انباشته->spans_reserved = (uint32_t)reserve_بازه_شمار;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
ساكن فراغ rpmalloc_انباشته_cache_adopt_deferred(انباشته_t *انباشته,
                                                بازه_t **single_بازه) {
  بازه_t *بازه = (بازه_t *)((فراغ *)atomic_exchange_ptr_acquire(
      &انباشته->بازه_free_deferred, 0));
  طالما (بازه) {
    بازه_t *بعدی_بازه = (بازه_t *)بازه->free_list;
    rpmalloc_assert(بازه->انباشته == انباشته, "Span heap pointer corrupted");
    إذا (EXPECTED(بازه->اندازه_class < اندازه_CLASS_شمار)) {
      rpmalloc_assert(انباشته->پر_بازه_شمار, "Heap span counter corrupted");
      --انباشته->پر_بازه_شمار;
      _rpmalloc_stat_dec(&انباشته->بازه_use[0].spans_deferred);
#إذا RPMALLOC_اولی_CLASS_HEAPS
      rpmalloc_بازه_double_link_list_remove(&انباشته->پر_بازه[بازه->اندازه_class],
                                             بازه);
#پایان_اگر
      _rpmalloc_stat_dec(&انباشته->بازه_use[0].current);
      _rpmalloc_stat_dec(&انباشته->اندازه_class_use[بازه->اندازه_class].spans_current);
      إذا (single_بازه && !*single_بازه)
        *single_بازه = بازه;
      وإلا
        rpmalloc_انباشته_cache_درج(انباشته, بازه);
    } وإلا {
      إذا (بازه->اندازه_class == اندازه_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(بازه);
      } وإلا {
        rpmalloc_assert(بازه->اندازه_class == اندازه_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(انباشته->پر_بازه_شمار, "Heap span counter corrupted");
        --انباشته->پر_بازه_شمار;
#إذا RPMALLOC_اولی_CLASS_HEAPS
        rpmalloc_بازه_double_link_list_remove(&انباشته->large_huge_بازه, بازه);
#پایان_اگر
        uint32_t idx = بازه->بازه_شمار - 1;
        _rpmalloc_stat_dec(&انباشته->بازه_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&انباشته->بازه_use[idx].current);
        إذا (!idx && single_بازه && !*single_بازه)
          *single_بازه = بازه;
        وإلا
          rpmalloc_انباشته_cache_درج(انباشته, بازه);
      }
    }
    بازه = بعدی_بازه;
  }
}

ساكن فراغ rpmalloc_انباشته_unmap(انباشته_t *انباشته) {
  إذا (!انباشته->master_انباشته) {
    إذا ((انباشته->finalize > 1) && !atomic_load32(&انباشته->فرزند_شمار)) {
      بازه_t *بازه = (بازه_t *)((uintptr_t)انباشته & حافظه_بازه_ماسک);
      rpmalloc_بازه_unmap(بازه);
    }
  } وإلا {
    إذا (atomic_decr32(&انباشته->master_انباشته->فرزند_شمار) == 0) {
      rpmalloc_انباشته_unmap(انباشته->master_انباشته);
    }
  }
}

ساكن فراغ rpmalloc_انباشته_global_finalize(انباشته_t *انباشته) {
  إذا (انباشته->finalize++ > 1) {
    --انباشته->finalize;
    أرجع;
  }

  rpmalloc_انباشته_finalize(انباشته);

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    بازه_cache_t *بازه_cache;
    إذا (!iclass)
      بازه_cache = &انباشته->بازه_cache;
    وإلا
      بازه_cache = (بازه_cache_t *)(انباشته->بازه_large_cache + (iclass - 1));
    لكل (size_t ispan = 0; ispan < بازه_cache->شمار; ++ispan)
      rpmalloc_بازه_unmap(بازه_cache->بازه[ispan]);
    بازه_cache->شمار = 0;
  }
#پایان_اگر

  إذا (انباشته->پر_بازه_شمار) {
    --انباشته->finalize;
    أرجع;
  }

  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمار; ++iclass) {
    إذا (انباشته->اندازه_class[iclass].free_list ||
        انباشته->اندازه_class[iclass].partial_بازه) {
      --انباشته->finalize;
      أرجع;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)انباشته->id % انباشته_ARRAY_اندازه;
  انباشته_t *list_انباشته = حافظه_heaps[list_idx];
  إذا (list_انباشته == انباشته) {
    حافظه_heaps[list_idx] = انباشته->بعدی_انباشته;
  } وإلا {
    طالما (list_انباشته->بعدی_انباشته != انباشته)
      list_انباشته = list_انباشته->بعدی_انباشته;
    list_انباشته->بعدی_انباشته = انباشته->بعدی_انباشته;
  }

  rpmalloc_انباشته_unmap(انباشته);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
ساكن فراغ rpmalloc_انباشته_cache_درج(انباشته_t *انباشته, بازه_t *بازه) {
  إذا (UNEXPECTED(انباشته->finalize != 0)) {
    rpmalloc_بازه_unmap(بازه);
    rpmalloc_انباشته_global_finalize(انباشته);
    أرجع;
  }
#إذا ENABLE_THREAD_CACHE
  size_t بازه_شمار = بازه->بازه_شمار;
  _rpmalloc_stat_inc(&انباشته->بازه_use[بازه_شمار - 1].spans_مقصد_cache);
  إذا (بازه_شمار == 1) {
    بازه_cache_t *بازه_cache = &انباشته->بازه_cache;
    بازه_cache->بازه[بازه_cache->شمار++] = بازه;
    إذا (بازه_cache->شمار == MAX_THREAD_بازه_CACHE) {
      ثابت size_t remain_شمار =
          MAX_THREAD_بازه_CACHE - THREAD_بازه_CACHE_TRANSFER;
#إذا ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&انباشته->thread_مقصد_global,
                           THREAD_بازه_CACHE_TRANSFER * حافظه_بازه_اندازه);
      rpmalloc_stat_جمع_کن(&انباشته->بازه_use[بازه_شمار - 1].spans_مقصد_global,
                         THREAD_بازه_CACHE_TRANSFER);
      rpmalloc_global_cache_درج_spans(بازه_cache->بازه + remain_شمار,
                                          بازه_شمار,
                                          THREAD_بازه_CACHE_TRANSFER);
#وإلا
      لكل (size_t ispan = 0; ispan < THREAD_بازه_CACHE_TRANSFER; ++ispan)
        rpmalloc_بازه_unmap(بازه_cache->بازه[remain_شمار + ispan]);
#پایان_اگر
      بازه_cache->شمار = remain_شمار;
    }
  } وإلا {
    size_t cache_idx = بازه_شمار - 2;
    بازه_large_cache_t *بازه_cache = انباشته->بازه_large_cache + cache_idx;
    بازه_cache->بازه[بازه_cache->شمار++] = بازه;
    ثابت size_t cache_حد =
        (MAX_THREAD_بازه_LARGE_CACHE - (بازه_شمار >> 1));
    إذا (بازه_cache->شمار == cache_حد) {
      ثابت size_t transfer_حد = 2 + (cache_حد >> 2);
      ثابت size_t transfer_شمار =
          (THREAD_بازه_LARGE_CACHE_TRANSFER <= transfer_حد
               ? THREAD_بازه_LARGE_CACHE_TRANSFER
               : transfer_حد);
      ثابت size_t remain_شمار = cache_حد - transfer_شمار;
#إذا ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&انباشته->thread_مقصد_global,
                           transfer_شمار * بازه_شمار * حافظه_بازه_اندازه);
      rpmalloc_stat_جمع_کن(&انباشته->بازه_use[بازه_شمار - 1].spans_مقصد_global,
                         transfer_شمار);
      rpmalloc_global_cache_درج_spans(بازه_cache->بازه + remain_شمار,
                                          بازه_شمار, transfer_شمار);
#وإلا
      لكل (size_t ispan = 0; ispan < transfer_شمار; ++ispan)
        rpmalloc_بازه_unmap(بازه_cache->بازه[remain_شمار + ispan]);
#پایان_اگر
      بازه_cache->شمار = remain_شمار;
    }
  }
#وإلا
  (فراغ)حجم(انباشته);
  rpmalloc_بازه_unmap(بازه);
#پایان_اگر
}

//! Extract the given number of spans from the different cache levels
ساكن بازه_t *rpmalloc_انباشته_thread_cache_استخراج(انباشته_t *انباشته,
                                                   size_t بازه_شمار) {
  بازه_t *بازه = 0;
#إذا ENABLE_THREAD_CACHE
  بازه_cache_t *بازه_cache;
  إذا (بازه_شمار == 1)
    بازه_cache = &انباشته->بازه_cache;
  وإلا
    بازه_cache = (بازه_cache_t *)(انباشته->بازه_large_cache + (بازه_شمار - 2));
  إذا (بازه_cache->شمار) {
    _rpmalloc_stat_inc(&انباشته->بازه_use[بازه_شمار - 1].spans_from_cache);
    أرجع بازه_cache->بازه[--بازه_cache->شمار];
  }
#پایان_اگر
  أرجع بازه;
}

ساكن بازه_t *rpmalloc_انباشته_thread_cache_deferred_استخراج(انباشته_t *انباشته,
                                                            size_t بازه_شمار) {
  بازه_t *بازه = 0;
  إذا (بازه_شمار == 1) {
    rpmalloc_انباشته_cache_adopt_deferred(انباشته, &بازه);
  } وإلا {
    rpmalloc_انباشته_cache_adopt_deferred(انباشته, 0);
    بازه = rpmalloc_انباشته_thread_cache_استخراج(انباشته, بازه_شمار);
  }
  أرجع بازه;
}

ساكن بازه_t *rpmalloc_انباشته_reserved_استخراج(انباشته_t *انباشته,
                                               size_t بازه_شمار) {
  إذا (انباشته->spans_reserved >= بازه_شمار)
    أرجع rpmalloc_بازه_map(انباشته, بازه_شمار);
  أرجع 0;
}

//! Extract a span from the global cache
ساكن بازه_t *rpmalloc_انباشته_global_cache_استخراج(انباشته_t *انباشته,
                                                   size_t بازه_شمار) {
#إذا ENABLE_GLOBAL_CACHE
#إذا ENABLE_THREAD_CACHE
  بازه_cache_t *بازه_cache;
  size_t wanted_شمار;
  إذا (بازه_شمار == 1) {
    بازه_cache = &انباشته->بازه_cache;
    wanted_شمار = THREAD_بازه_CACHE_TRANSFER;
  } وإلا {
    بازه_cache = (بازه_cache_t *)(انباشته->بازه_large_cache + (بازه_شمار - 2));
    wanted_شمار = THREAD_بازه_LARGE_CACHE_TRANSFER;
  }
  بازه_cache->شمار = rpmalloc_global_cache_استخراج_spans(
      بازه_cache->بازه, بازه_شمار, wanted_شمار);
  إذا (بازه_cache->شمار) {
    _rpmalloc_stat_add64(&انباشته->global_مقصد_thread,
                         بازه_شمار * بازه_cache->شمار * حافظه_بازه_اندازه);
    rpmalloc_stat_جمع_کن(&انباشته->بازه_use[بازه_شمار - 1].spans_from_global,
                       بازه_cache->شمار);
    أرجع بازه_cache->بازه[--بازه_cache->شمار];
  }
#وإلا
  بازه_t *بازه = 0;
  size_t شمار = rpmalloc_global_cache_استخراج_spans(&بازه, بازه_شمار, 1);
  إذا (شمار) {
    _rpmalloc_stat_add64(&انباشته->global_مقصد_thread,
                         بازه_شمار * شمار * حافظه_بازه_اندازه);
    rpmalloc_stat_جمع_کن(&انباشته->بازه_use[بازه_شمار - 1].spans_from_global,
                       شمار);
    أرجع بازه;
  }
#پایان_اگر
#پایان_اگر
  (فراغ)حجم(انباشته);
  (فراغ)حجم(بازه_شمار);
  أرجع 0;
}

ساكن فراغ rpmalloc_inc_بازه_statistics(انباشته_t *انباشته, size_t بازه_شمار,
                                          uint32_t class_idx) {
  (فراغ)حجم(انباشته);
  (فراغ)حجم(بازه_شمار);
  (فراغ)حجم(class_idx);
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)بازه_شمار - 1;
  uint32_t current_شمار =
      (uint32_t)atomic_incr32(&انباشته->بازه_use[idx].current);
  إذا (current_شمار > (uint32_t)atomic_load32(&انباشته->بازه_use[idx].بالا))
    atomic_store32(&انباشته->بازه_use[idx].بالا, (int32_t)current_شمار);
  rpmalloc_stat_جمع_کن_peak(&انباشته->اندازه_class_use[class_idx].spans_current, 1,
                          انباشته->اندازه_class_use[class_idx].spans_peak);
#پایان_اگر
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
ساكن بازه_t *
rpmalloc_انباشته_استخراج_new_بازه(انباشته_t *انباشته,
                                انباشته_اندازه_class_t *انباشته_اندازه_class,
                                size_t بازه_شمار, uint32_t class_idx) {
  بازه_t *بازه;
#إذا ENABLE_THREAD_CACHE
  إذا (انباشته_اندازه_class && انباشته_اندازه_class->cache) {
    بازه = انباشته_اندازه_class->cache;
    انباشته_اندازه_class->cache =
        (انباشته->بازه_cache.شمار
             ? انباشته->بازه_cache.بازه[--انباشته->بازه_cache.شمار]
             : 0);
    rpmalloc_inc_بازه_statistics(انباشته, بازه_شمار, class_idx);
    أرجع بازه;
  }
#پایان_اگر
  (فراغ)حجم(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t پایه_بازه_شمار = بازه_شمار;
  size_t حد_بازه_شمار =
      (بازه_شمار > 2) ? (بازه_شمار + (بازه_شمار >> 1)) : بازه_شمار;
  إذا (حد_بازه_شمار > LARGE_CLASS_شمار)
    حد_بازه_شمار = LARGE_CLASS_شمار;
  افعل {
    بازه = rpmalloc_انباشته_thread_cache_استخراج(انباشته, بازه_شمار);
    إذا (EXPECTED(بازه != 0)) {
      _rpmalloc_stat_inc(&انباشته->اندازه_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_بازه_statistics(انباشته, بازه_شمار, class_idx);
      أرجع بازه;
    }
    بازه = rpmalloc_انباشته_thread_cache_deferred_استخراج(انباشته, بازه_شمار);
    إذا (EXPECTED(بازه != 0)) {
      _rpmalloc_stat_inc(&انباشته->اندازه_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_بازه_statistics(انباشته, بازه_شمار, class_idx);
      أرجع بازه;
    }
    بازه = rpmalloc_انباشته_global_cache_استخراج(انباشته, بازه_شمار);
    إذا (EXPECTED(بازه != 0)) {
      _rpmalloc_stat_inc(&انباشته->اندازه_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_بازه_statistics(انباشته, بازه_شمار, class_idx);
      أرجع بازه;
    }
    بازه = rpmalloc_انباشته_reserved_استخراج(انباشته, بازه_شمار);
    إذا (EXPECTED(بازه != 0)) {
      _rpmalloc_stat_inc(&انباشته->اندازه_class_use[class_idx].spans_from_reserved);
      rpmalloc_inc_بازه_statistics(انباشته, بازه_شمار, class_idx);
      أرجع بازه;
    }
    ++بازه_شمار;
  } طالما (بازه_شمار <= حد_بازه_شمار);
  // Final fallback, map in more virtual memory
  بازه = rpmalloc_بازه_map(انباشته, پایه_بازه_شمار);
  rpmalloc_inc_بازه_statistics(انباشته, پایه_بازه_شمار, class_idx);
  _rpmalloc_stat_inc(&انباشته->اندازه_class_use[class_idx].spans_map_calls);
  أرجع بازه;
}

ساكن فراغ rpmalloc_انباشته_initialize(انباشته_t *انباشته) {
  _rpmalloc_memset_const(انباشته, 0, حجم(انباشته_t));
  // Get a new heap ID
  انباشته->id = 1 + atomic_incr32(&حافظه_انباشته_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)انباشته->id % انباشته_ARRAY_اندازه;
  انباشته->بعدی_انباشته = حافظه_heaps[list_idx];
  حافظه_heaps[list_idx] = انباشته;
}

ساكن فراغ rpmalloc_انباشته_orphan(انباشته_t *انباشته, صحيح اولی_class) {
  انباشته->owner_thread = (uintptr_t)-1;
#إذا RPMALLOC_اولی_CLASS_HEAPS
  انباشته_t **انباشته_list =
      (اولی_class ? &حافظه_اولی_class_orphan_heaps : &حافظه_orphan_heaps);
#وإلا
  (فراغ)حجم(اولی_class);
  انباشته_t **انباشته_list = &حافظه_orphan_heaps;
#پایان_اگر
  انباشته->بعدی_orphan = *انباشته_list;
  *انباشته_list = انباشته;
}

//! Allocate a new heap from newly mapped memory pages
ساكن انباشته_t *rpmalloc_انباشته_allocate_new(فراغ) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t انباشته_اندازه = حجم(انباشته_t);
  size_t aligned_انباشته_اندازه = 16 * ((انباشته_اندازه + 15) / 16);
  size_t request_انباشته_شمار = 16;
  size_t انباشته_بازه_شمار = ((aligned_انباشته_اندازه * request_انباشته_شمار) +
                            حجم(بازه_t) + حافظه_بازه_اندازه - 1) /
                           حافظه_بازه_اندازه;
  size_t بلوک_اندازه_2 = حافظه_بازه_اندازه * انباشته_بازه_شمار;
  size_t بازه_شمار = انباشته_بازه_شمار;
  بازه_t *بازه = 0;
  // If there are global reserved spans, use these first
  إذا (حافظه_global_reserve_شمار >= انباشته_بازه_شمار) {
    بازه = _rpmalloc_global_get_reserved_spans(انباشته_بازه_شمار);
  }
  إذا (!بازه) {
    إذا (حافظه_صفحه_اندازه > بلوک_اندازه_2) {
      بازه_شمار = حافظه_صفحه_اندازه / حافظه_بازه_اندازه;
      بلوک_اندازه_2 = حافظه_صفحه_اندازه;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_انباشته_شمار =
          (بلوک_اندازه_2 - حجم(بازه_t)) / aligned_انباشته_اندازه;
      إذا (possible_انباشته_شمار >= (request_انباشته_شمار * 16))
        request_انباشته_شمار *= 16;
      وإلا إذا (possible_انباشته_شمار < request_انباشته_شمار)
        request_انباشته_شمار = possible_انباشته_شمار;
      انباشته_بازه_شمار = ((aligned_انباشته_اندازه * request_انباشته_شمار) +
                         حجم(بازه_t) + حافظه_بازه_اندازه - 1) /
                        حافظه_بازه_اندازه;
    }

    size_t align_جابه_جایی = 0;
    بازه = (بازه_t *)_rpmalloc_mmap(بلوک_اندازه_2, &align_جابه_جایی);
    إذا (!بازه)
      أرجع 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    rpmalloc_بازه_initialize(بازه, بازه_شمار, انباشته_بازه_شمار, align_جابه_جایی);
  }

  size_t remain_اندازه = حافظه_بازه_اندازه - حجم(بازه_t);
  انباشته_t *انباشته = (انباشته_t *)اشاره_گر_جابه_جایی(بازه, حجم(بازه_t));
  rpmalloc_انباشته_initialize(انباشته);

  // Put extra heaps as orphans
  size_t رقم_heaps = remain_اندازه / aligned_انباشته_اندازه;
  إذا (رقم_heaps < request_انباشته_شمار)
    رقم_heaps = request_انباشته_شمار;
  atomic_store32(&انباشته->فرزند_شمار, (int32_t)رقم_heaps - 1);
  انباشته_t *extra_انباشته = (انباشته_t *)اشاره_گر_جابه_جایی(انباشته, aligned_انباشته_اندازه);
  طالما (رقم_heaps > 1) {
    rpmalloc_انباشته_initialize(extra_انباشته);
    extra_انباشته->master_انباشته = انباشته;
    rpmalloc_انباشته_orphan(extra_انباشته, 1);
    extra_انباشته = (انباشته_t *)اشاره_گر_جابه_جایی(extra_انباشته, aligned_انباشته_اندازه);
    --رقم_heaps;
  }

  إذا (بازه_شمار > انباشته_بازه_شمار) {
    // Cap reserved spans
    size_t remain_شمار = بازه_شمار - انباشته_بازه_شمار;
    size_t reserve_شمار =
        (remain_شمار > حافظه_انباشته_reserve_شمار ? حافظه_انباشته_reserve_شمار
                                                   : remain_شمار);
    بازه_t *remain_بازه =
        (بازه_t *)اشاره_گر_جابه_جایی(بازه, انباشته_بازه_شمار * حافظه_بازه_اندازه);
    rpmalloc_انباشته_مجموعه_reserved_spans(انباشته, بازه, remain_بازه, reserve_شمار);

    إذا (remain_شمار > reserve_شمار) {
      // Set to global reserved spans
      remain_بازه = (بازه_t *)اشاره_گر_جابه_جایی(remain_بازه,
                                             reserve_شمار * حافظه_بازه_اندازه);
      reserve_شمار = remain_شمار - reserve_شمار;
      rpmalloc_global_مجموعه_reserved_spans(بازه, remain_بازه, reserve_شمار);
    }
  }

  أرجع انباشته;
}

ساكن انباشته_t *rpmalloc_انباشته_استخراج_orphan(انباشته_t **انباشته_list) {
  انباشته_t *انباشته = *انباشته_list;
  *انباشته_list = (انباشته ? انباشته->بعدی_orphan : 0);
  أرجع انباشته;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
ساكن انباشته_t *rpmalloc_انباشته_allocate(صحيح اولی_class) {
  انباشته_t *انباشته = 0;
  طالما (!atomic_cas32_acquire(&حافظه_global_قفل, 1, 0))
    _rpmalloc_spin();
  إذا (اولی_class == 0)
    انباشته = rpmalloc_انباشته_استخراج_orphan(&حافظه_orphan_heaps);
#إذا RPMALLOC_اولی_CLASS_HEAPS
  إذا (!انباشته)
    انباشته = rpmalloc_انباشته_استخراج_orphan(&حافظه_اولی_class_orphan_heaps);
#پایان_اگر
  إذا (!انباشته)
    انباشته = rpmalloc_انباشته_allocate_new();
  atomic_store32_release(&حافظه_global_قفل, 0);
  إذا (انباشته)
    rpmalloc_انباشته_cache_adopt_deferred(انباشته, 0);
  أرجع انباشته;
}

ساكن فراغ rpmalloc_انباشته_release(فراغ *heapptr, صحيح اولی_class,
                                   صحيح release_cache) {
  انباشته_t *انباشته = (انباشته_t *)heapptr;
  إذا (!انباشته)
    أرجع;
  // Release thread cache spans back to global cache
  rpmalloc_انباشته_cache_adopt_deferred(انباشته, 0);
  إذا (release_cache || انباشته->finalize) {
#إذا ENABLE_THREAD_CACHE
    لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
      بازه_cache_t *بازه_cache;
      إذا (!iclass)
        بازه_cache = &انباشته->بازه_cache;
      وإلا
        بازه_cache = (بازه_cache_t *)(انباشته->بازه_large_cache + (iclass - 1));
      إذا (!بازه_cache->شمار)
        استمر;
#إذا ENABLE_GLOBAL_CACHE
      إذا (انباشته->finalize) {
        لكل (size_t ispan = 0; ispan < بازه_cache->شمار; ++ispan)
          rpmalloc_بازه_unmap(بازه_cache->بازه[ispan]);
      } وإلا {
        _rpmalloc_stat_add64(&انباشته->thread_مقصد_global, بازه_cache->شمار *
                                                          (iclass + 1) *
                                                          حافظه_بازه_اندازه);
        rpmalloc_stat_جمع_کن(&انباشته->بازه_use[iclass].spans_مقصد_global,
                           بازه_cache->شمار);
        rpmalloc_global_cache_درج_spans(بازه_cache->بازه, iclass + 1,
                                            بازه_cache->شمار);
      }
#وإلا
      لكل (size_t ispan = 0; ispan < بازه_cache->شمار; ++ispan)
        rpmalloc_بازه_unmap(بازه_cache->بازه[ispan]);
#پایان_اگر
      بازه_cache->شمار = 0;
    }
#پایان_اگر
  }

  إذا (get_thread_انباشته_raw() == انباشته)
    مجموعه_thread_انباشته(0);

#إذا ENABLE_STATISTICS
  atomic_decr32(&حافظه_فعال_heaps);
  rpmalloc_assert(atomic_load32(&حافظه_فعال_heaps) >= 0,
                  "Still active heaps during finalization");
#پایان_اگر

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  إذا (get_thread_id() != _rpmalloc_main_thread_id) {
    طالما (!atomic_cas32_acquire(&حافظه_global_قفل, 1, 0))
      _rpmalloc_spin();
  }
  rpmalloc_انباشته_orphan(انباشته, اولی_class);
  atomic_store32_release(&حافظه_global_قفل, 0);
}

ساكن فراغ rpmalloc_انباشته_release_raw(فراغ *heapptr, صحيح release_cache) {
  rpmalloc_انباشته_release(heapptr, 0, release_cache);
}

ساكن فراغ rpmalloc_انباشته_release_raw_fc(فراغ *heapptr) {
  rpmalloc_انباشته_release_raw(heapptr, 1);
}

ساكن فراغ rpmalloc_انباشته_finalize(انباشته_t *انباشته) {
  إذا (انباشته->spans_reserved) {
    بازه_t *بازه = rpmalloc_بازه_map(انباشته, انباشته->spans_reserved);
    rpmalloc_بازه_unmap(بازه);
    انباشته->spans_reserved = 0;
  }

  rpmalloc_انباشته_cache_adopt_deferred(انباشته, 0);

  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمار; ++iclass) {
    إذا (انباشته->اندازه_class[iclass].cache)
      rpmalloc_بازه_unmap(انباشته->اندازه_class[iclass].cache);
    انباشته->اندازه_class[iclass].cache = 0;
    بازه_t *بازه = انباشته->اندازه_class[iclass].partial_بازه;
    طالما (بازه) {
      بازه_t *بعدی = بازه->بعدی;
      rpmalloc_بازه_finalize(انباشته, iclass, بازه,
                              &انباشته->اندازه_class[iclass].partial_بازه);
      بازه = بعدی;
    }
    // If class still has a free list it must be a full span
    إذا (انباشته->اندازه_class[iclass].free_list) {
      بازه_t *class_بازه =
          (بازه_t *)((uintptr_t)انباشته->اندازه_class[iclass].free_list &
                     حافظه_بازه_ماسک);
      بازه_t **list = 0;
#إذا RPMALLOC_اولی_CLASS_HEAPS
      list = &انباشته->پر_بازه[iclass];
#پایان_اگر
      --انباشته->پر_بازه_شمار;
      إذا (!rpmalloc_بازه_finalize(انباشته, iclass, class_بازه, list)) {
        إذا (list)
          rpmalloc_بازه_double_link_list_remove(list, class_بازه);
        rpmalloc_بازه_double_link_list_جمع_کن(
            &انباشته->اندازه_class[iclass].partial_بازه, class_بازه);
      }
    }
  }

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    بازه_cache_t *بازه_cache;
    إذا (!iclass)
      بازه_cache = &انباشته->بازه_cache;
    وإلا
      بازه_cache = (بازه_cache_t *)(انباشته->بازه_large_cache + (iclass - 1));
    لكل (size_t ispan = 0; ispan < بازه_cache->شمار; ++ispan)
      rpmalloc_بازه_unmap(بازه_cache->بازه[ispan]);
    بازه_cache->شمار = 0;
  }
#پایان_اگر
  rpmalloc_assert(!atomic_بارگیری_ptr(&انباشته->بازه_free_deferred),
                  "Heaps still active during finalization");
}

////////////
///
/// Allocation entry points
///
//////

//! Pop first block from a free list
ساكن فراغ *free_list_برداشت(فراغ **list) {
  فراغ *بلوک = *list;
  *list = *((فراغ **)بلوک);
  أرجع بلوک;
}

//! Allocate a small/medium sized memory block from the given heap
ساكن فراغ *rpmalloc_allocate_from_انباشته_fallback(
    انباشته_t *انباشته, انباشته_اندازه_class_t *انباشته_اندازه_class, uint32_t class_idx) {
  بازه_t *بازه = انباشته_اندازه_class->partial_بازه;
  rpmalloc_assume(انباشته != 0);
  إذا (EXPECTED(بازه != 0)) {
    rpmalloc_assert(بازه->بلوک_شمار ==
                        حافظه_اندازه_class[بازه->اندازه_class].بلوک_شمار,
                    "Span block count corrupted");
    rpmalloc_assert(!rpmalloc_بازه_is_fully_utilized(بازه),
                    "Internal failure");
    فراغ *بلوک;
    إذا (بازه->free_list) {
      // Span local free list is not empty, swap to size class free list
      بلوک = free_list_برداشت(&بازه->free_list);
      انباشته_اندازه_class->free_list = بازه->free_list;
      بازه->free_list = 0;
    } وإلا {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      فراغ *بلوک_شروع = اشاره_گر_جابه_جایی(
          بازه, بازه_سرآیند_اندازه +
                    ((size_t)بازه->free_list_حد * بازه->بلوک_اندازه_2));
      بازه->free_list_حد += free_list_partial_init(
          &انباشته_اندازه_class->free_list, &بلوک,
          (فراغ *)((uintptr_t)بلوک_شروع & ~(حافظه_صفحه_اندازه - 1)),
          بلوک_شروع, بازه->بلوک_شمار - بازه->free_list_حد,
          بازه->بلوک_اندازه_2);
    }
    rpmalloc_assert(بازه->free_list_حد <= بازه->بلوک_شمار,
                    "Span block count corrupted");
    بازه->استفاده_شده_شمار = بازه->free_list_حد;

    // Swap in deferred free list if present
    إذا (atomic_بارگیری_ptr(&بازه->free_list_deferred))
      rpmalloc_بازه_استخراج_free_list_deferred(بازه);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    إذا (!rpmalloc_بازه_is_fully_utilized(بازه))
      أرجع بلوک;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_بازه_double_link_list_برداشت_سر(&انباشته_اندازه_class->partial_بازه,
                                             بازه);
#إذا RPMALLOC_اولی_CLASS_HEAPS
    rpmalloc_بازه_double_link_list_جمع_کن(&انباشته->پر_بازه[class_idx], بازه);
#پایان_اگر
    ++انباشته->پر_بازه_شمار;
    أرجع بلوک;
  }

  // Find a span in one of the cache levels
  بازه = rpmalloc_انباشته_استخراج_new_بازه(انباشته, انباشته_اندازه_class, 1, class_idx);
  إذا (EXPECTED(بازه != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    أرجع rpmalloc_بازه_initialize_new(انباشته, انباشته_اندازه_class, بازه,
                                         class_idx);
  }

  أرجع 0;
}

//! Allocate a small sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_small(انباشته_t *انباشته, size_t اندازه_2) {
  rpmalloc_assert(انباشته, "No thread heap");
  // Small sizes have unique size classes
  ثابت uint32_t class_idx =
      (uint32_t)((اندازه_2 + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_لغزش);
  انباشته_اندازه_class_t *انباشته_اندازه_class = انباشته->اندازه_class + class_idx;
  _rpmalloc_stat_inc_alloc(انباشته, class_idx);
  إذا (EXPECTED(انباشته_اندازه_class->free_list != 0))
    أرجع free_list_برداشت(&انباشته_اندازه_class->free_list);
  أرجع rpmalloc_allocate_from_انباشته_fallback(انباشته, انباشته_اندازه_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_medium(انباشته_t *انباشته, size_t اندازه_2) {
  rpmalloc_assert(انباشته, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ثابت uint32_t پایه_idx =
      (uint32_t)(SMALL_CLASS_شمار +
                 ((اندازه_2 - (SMALL_اندازه_حد + 1)) >> MEDIUM_GRANULARITY_لغزش));
  ثابت uint32_t class_idx = حافظه_اندازه_class[پایه_idx].class_idx;
  انباشته_اندازه_class_t *انباشته_اندازه_class = انباشته->اندازه_class + class_idx;
  _rpmalloc_stat_inc_alloc(انباشته, class_idx);
  إذا (EXPECTED(انباشته_اندازه_class->free_list != 0))
    أرجع free_list_برداشت(&انباشته_اندازه_class->free_list);
  أرجع rpmalloc_allocate_from_انباشته_fallback(انباشته, انباشته_اندازه_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_large(انباشته_t *انباشته, size_t اندازه_2) {
  rpmalloc_assert(انباشته, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  اندازه_2 += بازه_سرآیند_اندازه;
  size_t بازه_شمار = اندازه_2 >> حافظه_بازه_اندازه_لغزش;
  إذا (اندازه_2 & (حافظه_بازه_اندازه - 1))
    ++بازه_شمار;

  // Find a span in one of the cache levels
  بازه_t *بازه =
      rpmalloc_انباشته_استخراج_new_بازه(انباشته, 0, بازه_شمار, اندازه_CLASS_LARGE);
  إذا (!بازه)
    أرجع بازه;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(بازه->بازه_شمار >= بازه_شمار, "Internal failure");
  بازه->اندازه_class = اندازه_CLASS_LARGE;
  بازه->انباشته = انباشته;

#إذا RPMALLOC_اولی_CLASS_HEAPS
  rpmalloc_بازه_double_link_list_جمع_کن(&انباشته->large_huge_بازه, بازه);
#پایان_اگر
  ++انباشته->پر_بازه_شمار;

  أرجع اشاره_گر_جابه_جایی(بازه, بازه_سرآیند_اندازه);
}

//! Allocate a huge block by mapping memory pages directly
ساكن فراغ *_rpmalloc_allocate_huge(انباشته_t *انباشته, size_t اندازه_2) {
  rpmalloc_assert(انباشته, "No thread heap");
  rpmalloc_انباشته_cache_adopt_deferred(انباشته, 0);
  اندازه_2 += بازه_سرآیند_اندازه;
  size_t رقم_صفحات = اندازه_2 >> حافظه_صفحه_اندازه_لغزش;
  إذا (اندازه_2 & (حافظه_صفحه_اندازه - 1))
    ++رقم_صفحات;
  size_t align_جابه_جایی = 0;
  بازه_t *بازه =
      (بازه_t *)_rpmalloc_mmap(رقم_صفحات * حافظه_صفحه_اندازه, &align_جابه_جایی);
  إذا (!بازه)
    أرجع بازه;

  // Store page count in span_count
  بازه->اندازه_class = اندازه_CLASS_HUGE;
  بازه->بازه_شمار = (uint32_t)رقم_صفحات;
  بازه->align_جابه_جایی = (uint32_t)align_جابه_جایی;
  بازه->انباشته = انباشته;
  rpmalloc_stat_جمع_کن_peak(&huge_صفحات_current, رقم_صفحات, huge_صفحات_peak);

#إذا RPMALLOC_اولی_CLASS_HEAPS
  rpmalloc_بازه_double_link_list_جمع_کن(&انباشته->large_huge_بازه, بازه);
#پایان_اگر
  ++انباشته->پر_بازه_شمار;

  أرجع اشاره_گر_جابه_جایی(بازه, بازه_سرآیند_اندازه);
}

//! Allocate a block of the given size
ساكن فراغ *_rpmalloc_allocate(انباشته_t *انباشته, size_t اندازه_2) {
  _rpmalloc_stat_add64(&تخصیص_counter, 1);
  إذا (EXPECTED(اندازه_2 <= SMALL_اندازه_حد))
    أرجع _rpmalloc_allocate_small(انباشته, اندازه_2);
  وإلا إذا (اندازه_2 <= حافظه_medium_اندازه_حد)
    أرجع _rpmalloc_allocate_medium(انباشته, اندازه_2);
  وإلا إذا (اندازه_2 <= LARGE_اندازه_حد)
    أرجع _rpmalloc_allocate_large(انباشته, اندازه_2);
  أرجع _rpmalloc_allocate_huge(انباشته, اندازه_2);
}

ساكن فراغ *_rpmalloc_aligned_allocate(انباشته_t *انباشته, size_t alignment,
                                        size_t اندازه_2) {
  إذا (alignment <= SMALL_GRANULARITY)
    أرجع _rpmalloc_allocate(انباشته, اندازه_2);

#إذا ENABLE_VALIDATE_آرگومانها
  إذا ((اندازه_2 + alignment) < اندازه_2) {
    errno = EINVAL;
    أرجع 0;
  }
  إذا (alignment & (alignment - 1)) {
    errno = EINVAL;
    أرجع 0;
  }
#پایان_اگر

  إذا ((alignment <= بازه_سرآیند_اندازه) &&
      ((اندازه_2 + بازه_سرآیند_اندازه) < حافظه_medium_اندازه_حد)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t مضرب_اندازه = اندازه_2 ? (اندازه_2 + (بازه_سرآیند_اندازه - 1)) &
                                      ~(uintptr_t)(بازه_سرآیند_اندازه - 1)
                                : بازه_سرآیند_اندازه;
    rpmalloc_assert(!(مضرب_اندازه % بازه_سرآیند_اندازه),
                    "Failed alignment calculation");
    إذا (مضرب_اندازه <= (اندازه_2 + alignment))
      أرجع _rpmalloc_allocate(انباشته, مضرب_اندازه);
  }

  فراغ *ptr = 0;
  size_t align_ماسک = alignment - 1;
  إذا (alignment <= حافظه_صفحه_اندازه) {
    ptr = _rpmalloc_allocate(انباشته, اندازه_2 + alignment);
    إذا ((uintptr_t)ptr & align_ماسک) {
      ptr = (فراغ *)(((uintptr_t)ptr & ~(uintptr_t)align_ماسک) + alignment);
      // Mark as having aligned blocks
      بازه_t *بازه = (بازه_t *)((uintptr_t)ptr & حافظه_بازه_ماسک);
      بازه->flags |= بازه_FLAG_ALIGNED_بلوکها;
    }
    أرجع ptr;
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
  إذا (alignment & align_ماسک) {
    errno = EINVAL;
    أرجع 0;
  }
  إذا (alignment >= حافظه_بازه_اندازه) {
    errno = EINVAL;
    أرجع 0;
  }

  size_t extra_صفحات = alignment / حافظه_صفحه_اندازه;

  // Since each span has a header, we will at least need one extra memory page
  size_t رقم_صفحات = 1 + (اندازه_2 / حافظه_صفحه_اندازه);
  إذا (اندازه_2 & (حافظه_صفحه_اندازه - 1))
    ++رقم_صفحات;

  إذا (extra_صفحات > رقم_صفحات)
    رقم_صفحات = 1 + extra_صفحات;

  size_t original_صفحات = رقم_صفحات;
  size_t حد_صفحات = (حافظه_بازه_اندازه / حافظه_صفحه_اندازه) * 2;
  إذا (حد_صفحات < (original_صفحات * 2))
    حد_صفحات = original_صفحات * 2;

  size_t mapped_اندازه, align_جابه_جایی;
  بازه_t *بازه;

retry:
  align_جابه_جایی = 0;
  mapped_اندازه = رقم_صفحات * حافظه_صفحه_اندازه;

  بازه = (بازه_t *)_rpmalloc_mmap(mapped_اندازه, &align_جابه_جایی);
  إذا (!بازه) {
    errno = ENOMEM;
    أرجع 0;
  }
  ptr = اشاره_گر_جابه_جایی(بازه, بازه_سرآیند_اندازه);

  إذا ((uintptr_t)ptr & align_ماسک)
    ptr = (فراغ *)(((uintptr_t)ptr & ~(uintptr_t)align_ماسک) + alignment);

  إذا (((size_t)اشاره_گر_diff(ptr, بازه) >= حافظه_بازه_اندازه) ||
      (اشاره_گر_جابه_جایی(ptr, اندازه_2) > اشاره_گر_جابه_جایی(بازه, mapped_اندازه)) ||
      (((uintptr_t)ptr & حافظه_بازه_ماسک) != (uintptr_t)بازه)) {
    _rpmalloc_unmap(بازه, mapped_اندازه, align_جابه_جایی, mapped_اندازه);
    ++رقم_صفحات;
    إذا (رقم_صفحات > حد_صفحات) {
      errno = EINVAL;
      أرجع 0;
    }
    اذهب retry;
  }

  // Store page count in span_count
  بازه->اندازه_class = اندازه_CLASS_HUGE;
  بازه->بازه_شمار = (uint32_t)رقم_صفحات;
  بازه->align_جابه_جایی = (uint32_t)align_جابه_جایی;
  بازه->انباشته = انباشته;
  rpmalloc_stat_جمع_کن_peak(&huge_صفحات_current, رقم_صفحات, huge_صفحات_peak);

#إذا RPMALLOC_اولی_CLASS_HEAPS
  rpmalloc_بازه_double_link_list_جمع_کن(&انباشته->large_huge_بازه, بازه);
#پایان_اگر
  ++انباشته->پر_بازه_شمار;

  _rpmalloc_stat_add64(&تخصیص_counter, 1);

  أرجع ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
ساكن فراغ rpmalloc_deallocate_مستقیم_small_or_medium(بازه_t *بازه,
                                                        فراغ *بلوک) {
  انباشته_t *انباشته = بازه->انباشته;
  rpmalloc_assert(انباشته->owner_thread == get_thread_id() ||
                      !انباشته->owner_thread || انباشته->finalize,
                  "Internal failure");
  // Add block to free list
  إذا (UNEXPECTED(rpmalloc_بازه_is_fully_utilized(بازه))) {
    بازه->استفاده_شده_شمار = بازه->بلوک_شمار;
#إذا RPMALLOC_اولی_CLASS_HEAPS
    rpmalloc_بازه_double_link_list_remove(&انباشته->پر_بازه[بازه->اندازه_class],
                                           بازه);
#پایان_اگر
    rpmalloc_بازه_double_link_list_جمع_کن(
        &انباشته->اندازه_class[بازه->اندازه_class].partial_بازه, بازه);
    --انباشته->پر_بازه_شمار;
  }
  *((فراغ **)بلوک) = بازه->free_list;
  --بازه->استفاده_شده_شمار;
  بازه->free_list = بلوک;
  إذا (UNEXPECTED(بازه->استفاده_شده_شمار == بازه->list_اندازه)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    إذا (بازه->استفاده_شده_شمار) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      فراغ *free_list;
      افعل {
        free_list = atomic_exchange_ptr_acquire(&بازه->free_list_deferred,
                                                INVALID_اشاره_گر);
      } طالما (free_list == INVALID_اشاره_گر);
      atomic_store_ptr_release(&بازه->free_list_deferred, free_list);
    }
    rpmalloc_بازه_double_link_list_remove(
        &انباشته->اندازه_class[بازه->اندازه_class].partial_بازه, بازه);
    rpmalloc_بازه_release_مقصد_cache(انباشته, بازه);
  }
}

ساكن فراغ rpmalloc_deallocate_defer_free_بازه(انباشته_t *انباشته, بازه_t *بازه) {
  إذا (بازه->اندازه_class != اندازه_CLASS_HUGE)
    _rpmalloc_stat_inc(&انباشته->بازه_use[بازه->بازه_شمار - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  افعل {
    بازه->free_list = (فراغ *)atomic_بارگیری_ptr(&انباشته->بازه_free_deferred);
  } طالما (!atomic_cas_ptr(&انباشته->بازه_free_deferred, بازه, بازه->free_list));
}

//! Put the block in the deferred free list of the owning span
ساكن فراغ _rpmalloc_deallocate_defer_small_or_medium(بازه_t *بازه,
                                                       فراغ *بلوک) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  فراغ *free_list;
  افعل {
    free_list =
        atomic_exchange_ptr_acquire(&بازه->free_list_deferred, INVALID_اشاره_گر);
  } طالما (free_list == INVALID_اشاره_گر);
  *((فراغ **)بلوک) = free_list;
  uint32_t free_شمار = ++بازه->list_اندازه;
  صحيح all_deferred_free = (free_شمار == بازه->بلوک_شمار);
  atomic_store_ptr_release(&بازه->free_list_deferred, بلوک);
  إذا (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    rpmalloc_deallocate_defer_free_بازه(بازه->انباشته, بازه);
  }
}

ساكن فراغ _rpmalloc_deallocate_small_or_medium(بازه_t *بازه, فراغ *p) {
  _rpmalloc_stat_inc_free(بازه->انباشته, بازه->اندازه_class);
  إذا (بازه->flags & بازه_FLAG_ALIGNED_بلوکها) {
    // Realign pointer to block start
    فراغ *بلوکها_شروع = اشاره_گر_جابه_جایی(بازه, بازه_سرآیند_اندازه);
    uint32_t بلوک_جابه_جایی = (uint32_t)اشاره_گر_diff(p, بلوکها_شروع);
    p = اشاره_گر_جابه_جایی(p, -(int32_t)(بلوک_جابه_جایی % بازه->بلوک_اندازه_2));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#إذا RPMALLOC_اولی_CLASS_HEAPS
  صحيح defer =
      (بازه->انباشته->owner_thread &&
       (بازه->انباشته->owner_thread != get_thread_id()) && !بازه->انباشته->finalize);
#وإلا
  صحيح defer =
      ((بازه->انباشته->owner_thread != get_thread_id()) && !بازه->انباشته->finalize);
#پایان_اگر
  إذا (!defer)
    rpmalloc_deallocate_مستقیم_small_or_medium(بازه, p);
  وإلا
    _rpmalloc_deallocate_defer_small_or_medium(بازه, p);
}

//! Deallocate the given large memory block to the current heap
ساكن فراغ _rpmalloc_deallocate_large(بازه_t *بازه) {
  rpmalloc_assert(بازه->اندازه_class == اندازه_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(بازه->flags & بازه_FLAG_MASTER) ||
                      !(بازه->flags & بازه_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((بازه->flags & بازه_FLAG_MASTER) ||
                      (بازه->flags & بازه_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#إذا RPMALLOC_اولی_CLASS_HEAPS
  صحيح defer =
      (بازه->انباشته->owner_thread &&
       (بازه->انباشته->owner_thread != get_thread_id()) && !بازه->انباشته->finalize);
#وإلا
  صحيح defer =
      ((بازه->انباشته->owner_thread != get_thread_id()) && !بازه->انباشته->finalize);
#پایان_اگر
  إذا (defer) {
    rpmalloc_deallocate_defer_free_بازه(بازه->انباشته, بازه);
    أرجع;
  }
  rpmalloc_assert(بازه->انباشته->پر_بازه_شمار, "Heap span counter corrupted");
  --بازه->انباشته->پر_بازه_شمار;
#إذا RPMALLOC_اولی_CLASS_HEAPS
  rpmalloc_بازه_double_link_list_remove(&بازه->انباشته->large_huge_بازه, بازه);
#پایان_اگر
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = بازه->بازه_شمار - 1;
  atomic_decr32(&بازه->انباشته->بازه_use[idx].current);
#پایان_اگر
  انباشته_t *انباشته = بازه->انباشته;
  rpmalloc_assert(انباشته, "No thread heap");
#إذا ENABLE_THREAD_CACHE
  ثابت صحيح مجموعه_as_reserved =
      ((بازه->بازه_شمار > 1) && (انباشته->بازه_cache.شمار == 0) &&
       !انباشته->finalize && !انباشته->spans_reserved);
#وإلا
  ثابت صحيح مجموعه_as_reserved =
      ((بازه->بازه_شمار > 1) && !انباشته->finalize && !انباشته->spans_reserved);
#پایان_اگر
  إذا (مجموعه_as_reserved) {
    انباشته->بازه_reserve = بازه;
    انباشته->spans_reserved = بازه->بازه_شمار;
    إذا (بازه->flags & بازه_FLAG_MASTER) {
      انباشته->بازه_reserve_master = بازه;
    } وإلا { // SPAN_FLAG_SUBSPAN
      بازه_t *master = (بازه_t *)اشاره_گر_جابه_جایی(
          بازه,
          -(intptr_t)((size_t)بازه->جابه_جایی_from_master * حافظه_بازه_اندازه));
      انباشته->بازه_reserve_master = master;
      rpmalloc_assert(master->flags & بازه_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->مانده_spans) >=
                          (int32_t)بازه->بازه_شمار,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&انباشته->بازه_use[idx].spans_مقصد_reserved);
  } وإلا {
    // Insert into cache list
    rpmalloc_انباشته_cache_درج(انباشته, بازه);
  }
}

//! Deallocate the given huge span
ساكن فراغ _rpmalloc_deallocate_huge(بازه_t *بازه) {
  rpmalloc_assert(بازه->انباشته, "No span heap");
#إذا RPMALLOC_اولی_CLASS_HEAPS
  صحيح defer =
      (بازه->انباشته->owner_thread &&
       (بازه->انباشته->owner_thread != get_thread_id()) && !بازه->انباشته->finalize);
#وإلا
  صحيح defer =
      ((بازه->انباشته->owner_thread != get_thread_id()) && !بازه->انباشته->finalize);
#پایان_اگر
  إذا (defer) {
    rpmalloc_deallocate_defer_free_بازه(بازه->انباشته, بازه);
    أرجع;
  }
  rpmalloc_assert(بازه->انباشته->پر_بازه_شمار, "Heap span counter corrupted");
  --بازه->انباشته->پر_بازه_شمار;
#إذا RPMALLOC_اولی_CLASS_HEAPS
  rpmalloc_بازه_double_link_list_remove(&بازه->انباشته->large_huge_بازه, بازه);
#پایان_اگر

  // Oversized allocation, page count is stored in span_count
  size_t رقم_صفحات = بازه->بازه_شمار;
  _rpmalloc_unmap(بازه, رقم_صفحات * حافظه_صفحه_اندازه, بازه->align_جابه_جایی,
                  رقم_صفحات * حافظه_صفحه_اندازه);
  _rpmalloc_stat_sub(&huge_صفحات_current, رقم_صفحات);
}

//! Deallocate the given block
ساكن فراغ _rpmalloc_deallocate(فراغ *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  بازه_t *بازه = (بازه_t *)((uintptr_t)p & حافظه_بازه_ماسک);
  إذا (UNEXPECTED(!بازه))
    أرجع;
  إذا (EXPECTED(بازه->اندازه_class < اندازه_CLASS_شمار))
    _rpmalloc_deallocate_small_or_medium(بازه, p);
  وإلا إذا (بازه->اندازه_class == اندازه_CLASS_LARGE)
    _rpmalloc_deallocate_large(بازه);
  وإلا
    _rpmalloc_deallocate_huge(بازه);
}

////////////
///
/// Reallocation entry points
///
//////

ساكن size_t rpmalloc_usable_اندازه_2(فراغ *p);

//! Reallocate the given block to the given size
ساكن فراغ *_rpmalloc_reallocate(انباشته_t *انباشته, فراغ *p, size_t اندازه_2,
                                  size_t oldsize, بیعلامت صحيح flags) {
  إذا (p) {
    // Grab the span using guaranteed span alignment
    بازه_t *بازه = (بازه_t *)((uintptr_t)p & حافظه_بازه_ماسک);
    إذا (EXPECTED(بازه->اندازه_class < اندازه_CLASS_شمار)) {
      // Small/medium sized block
      rpmalloc_assert(بازه->بازه_شمار == 1, "Span counter corrupted");
      فراغ *بلوکها_شروع = اشاره_گر_جابه_جایی(بازه, بازه_سرآیند_اندازه);
      uint32_t بلوک_جابه_جایی = (uint32_t)اشاره_گر_diff(p, بلوکها_شروع);
      uint32_t بلوک_idx = بلوک_جابه_جایی / بازه->بلوک_اندازه_2;
      فراغ *بلوک =
          اشاره_گر_جابه_جایی(بلوکها_شروع, (size_t)بلوک_idx * بازه->بلوک_اندازه_2);
      إذا (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)بازه->بلوک_اندازه_2 - اشاره_گر_diff(p, بلوک));
      إذا ((size_t)بازه->بلوک_اندازه_2 >= اندازه_2) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != بلوک) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلوک, p, oldsize);
        أرجع بلوک;
      }
    } وإلا إذا (بازه->اندازه_class == اندازه_CLASS_LARGE) {
      // Large block
      size_t مجموع_اندازه_2 = اندازه_2 + بازه_سرآیند_اندازه;
      size_t رقم_spans = مجموع_اندازه_2 >> حافظه_بازه_اندازه_لغزش;
      إذا (مجموع_اندازه_2 & (حافظه_بازه_ماسک - 1))
        ++رقم_spans;
      size_t current_spans = بازه->بازه_شمار;
      فراغ *بلوک = اشاره_گر_جابه_جایی(بازه, بازه_سرآیند_اندازه);
      إذا (!oldsize)
        oldsize = (current_spans * حافظه_بازه_اندازه) -
                  (size_t)اشاره_گر_diff(p, بلوک) - بازه_سرآیند_اندازه;
      إذا ((current_spans >= رقم_spans) && (مجموع_اندازه_2 >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != بلوک) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلوک, p, oldsize);
        أرجع بلوک;
      }
    } وإلا {
      // Oversized block
      size_t مجموع_اندازه_2 = اندازه_2 + بازه_سرآیند_اندازه;
      size_t رقم_صفحات = مجموع_اندازه_2 >> حافظه_صفحه_اندازه_لغزش;
      إذا (مجموع_اندازه_2 & (حافظه_صفحه_اندازه - 1))
        ++رقم_صفحات;
      // Page count is stored in span_count
      size_t current_صفحات = بازه->بازه_شمار;
      فراغ *بلوک = اشاره_گر_جابه_جایی(بازه, بازه_سرآیند_اندازه);
      إذا (!oldsize)
        oldsize = (current_صفحات * حافظه_صفحه_اندازه) -
                  (size_t)اشاره_گر_diff(p, بلوک) - بازه_سرآیند_اندازه;
      إذا ((current_صفحات >= رقم_صفحات) && (رقم_صفحات >= (current_صفحات / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != بلوک) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلوک, p, oldsize);
        أرجع بلوک;
      }
    }
  } وإلا {
    oldsize = 0;
  }

  إذا (!!(flags & RPMALLOC_GROW_OR_FAIL))
    أرجع 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t پایینی_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_اندازه =
      (اندازه_2 > پایینی_bound) ? اندازه_2 : ((اندازه_2 > oldsize) ? پایینی_bound : اندازه_2);
  فراغ *بلوک = _rpmalloc_allocate(انباشته, new_اندازه);
  إذا (p && بلوک) {
    إذا (!(flags & RPMALLOC_NO_PRESERVE))
      کپی_حافظه(بلوک, p, oldsize < new_اندازه ? oldsize : new_اندازه);
    _rpmalloc_deallocate(p);
  }

  أرجع بلوک;
}

ساكن فراغ *_rpmalloc_aligned_reallocate(انباشته_t *انباشته, فراغ *ptr,
                                          size_t alignment, size_t اندازه_2,
                                          size_t oldsize, بیعلامت صحيح flags) {
  إذا (alignment <= SMALL_GRANULARITY)
    أرجع _rpmalloc_reallocate(انباشته, ptr, اندازه_2, oldsize, flags);

  صحيح no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? rpmalloc_usable_اندازه_2(ptr) : 0);
  إذا ((usablesize >= اندازه_2) && !((uintptr_t)ptr & (alignment - 1))) {
    إذا (no_alloc || (اندازه_2 >= (usablesize / 2)))
      أرجع ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  فراغ *بلوک =
      (!no_alloc ? _rpmalloc_aligned_allocate(انباشته, alignment, اندازه_2) : 0);
  إذا (EXPECTED(بلوک != 0)) {
    إذا (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      إذا (!oldsize)
        oldsize = usablesize;
      کپی_حافظه(بلوک, ptr, oldsize < اندازه_2 ? oldsize : اندازه_2);
    }
    _rpmalloc_deallocate(ptr);
  }
  أرجع بلوک;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
ساكن size_t rpmalloc_usable_اندازه_2(فراغ *p) {
  // Grab the span using guaranteed span alignment
  بازه_t *بازه = (بازه_t *)((uintptr_t)p & حافظه_بازه_ماسک);
  إذا (بازه->اندازه_class < اندازه_CLASS_شمار) {
    // Small/medium block
    فراغ *بلوکها_شروع = اشاره_گر_جابه_جایی(بازه, بازه_سرآیند_اندازه);
    أرجع بازه->بلوک_اندازه_2 -
           ((size_t)اشاره_گر_diff(p, بلوکها_شروع) % بازه->بلوک_اندازه_2);
  }
  إذا (بازه->اندازه_class == اندازه_CLASS_LARGE) {
    // Large block
    size_t current_spans = بازه->بازه_شمار;
    أرجع (current_spans * حافظه_بازه_اندازه) - (size_t)اشاره_گر_diff(p, بازه);
  }
  // Oversized block, page count is stored in span_count
  size_t current_صفحات = بازه->بازه_شمار;
  أرجع (current_صفحات * حافظه_صفحه_اندازه) - (size_t)اشاره_گر_diff(p, بازه);
}

//! Adjust and optimize the size class properties for the given class
ساكن فراغ rpmalloc_adjust_اندازه_class(size_t iclass) {
  size_t بلوک_اندازه_2 = حافظه_اندازه_class[iclass].بلوک_اندازه_2;
  size_t بلوک_شمار = (حافظه_بازه_اندازه - بازه_سرآیند_اندازه) / بلوک_اندازه_2;

  حافظه_اندازه_class[iclass].بلوک_شمار = (uint16_t)بلوک_شمار;
  حافظه_اندازه_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  إذا (iclass >= SMALL_CLASS_شمار) {
    size_t prevclass = iclass;
    طالما (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      إذا (حافظه_اندازه_class[prevclass].بلوک_شمار ==
          حافظه_اندازه_class[iclass].بلوک_شمار)
        _rpmalloc_memcpy_const(حافظه_اندازه_class + prevclass,
                               حافظه_اندازه_class + iclass,
                               حجم(حافظه_اندازه_class[iclass]));
      وإلا
        اكسر;
    }
  }
}

//! Initialize the allocator and setup global data
خارجي مضمن صحيح rpmalloc_initialize(فراغ) {
  إذا (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    أرجع 0;
  }
  أرجع rpmalloc_initialize_config(0);
}

صحيح rpmalloc_initialize_config(ثابت rpmalloc_config_t *config) {
  إذا (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    أرجع 0;
  }
  _rpmalloc_initialized = 1;

  إذا (config)
    کپی_حافظه(&حافظه_config, config, حجم(rpmalloc_config_t));
  وإلا
    _rpmalloc_memset_const(&حافظه_config, 0, حجم(rpmalloc_config_t));

  إذا (!حافظه_config.حافظه_map || !حافظه_config.حافظه_unmap) {
    حافظه_config.حافظه_map = _rpmalloc_mmap_os;
    حافظه_config.حافظه_unmap = _rpmalloc_unmap_os;
  }

#إذا PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, حجم(system_info));
  GetSystemInfo(&system_info);
  حافظه_map_granularity = system_info.dw_تخصیص_Granularity;
#وإلا
  حافظه_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#پایان_اگر

#إذا RPMALLOC_CONFIGURABLE
  حافظه_صفحه_اندازه = حافظه_config.صفحه_اندازه_2;
#وإلا
  حافظه_صفحه_اندازه = 0;
#پایان_اگر
  حافظه_huge_صفحات = 0;
  إذا (!حافظه_صفحه_اندازه) {
#إذا PLATFORM_WINDOWS
    حافظه_صفحه_اندازه = system_info.dw_صفحه_اندازه;
#وإلا
    حافظه_صفحه_اندازه = حافظه_map_granularity;
    إذا (حافظه_config.enable_huge_صفحات) {
#إذا defined(__linux__)
      size_t huge_صفحه_اندازه = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      إذا (meminfo) {
        حرف سطر_2[128];
        طالما (!huge_صفحه_اندازه && fgets(سطر_2, حجم(سطر_2) - 1, meminfo)) {
          سطر_2[حجم(سطر_2) - 1] = 0;
          إذا (جستجوی_رشته(سطر_2, "Hugepagesize:"))
            huge_صفحه_اندازه = (size_t)strtol(سطر_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      إذا (huge_صفحه_اندازه) {
        حافظه_huge_صفحات = 1;
        حافظه_صفحه_اندازه = huge_صفحه_اندازه;
        حافظه_map_granularity = huge_صفحه_اندازه;
      }
#وإلاإذا defined(__FreeBSD__)
      صحيح rc;
      size_t sz = حجم(rc);

      إذا (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, عدم, 0) == 0 &&
          rc == 1) {
        ساكن size_t defsize = 2 * 1024 * 1024;
        صحيح nsize = 0;
        size_t sizes[4] = {0};
        حافظه_huge_صفحات = 1;
        حافظه_صفحه_اندازه = defsize;
        إذا ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          لكل (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            إذا (defsize < csize) {
              حافظه_صفحه_اندازه = csize;
              اكسر;
            }
          }
        }
        حافظه_map_granularity = حافظه_صفحه_اندازه;
      }
#وإلاإذا defined(__APPLE__) || defined(__NetBSD__)
      حافظه_huge_صفحات = 1;
      حافظه_صفحه_اندازه = 2 * 1024 * 1024;
      حافظه_map_granularity = حافظه_صفحه_اندازه;
#پایان_اگر
    }
#پایان_اگر
  } وإلا {
    إذا (حافظه_config.enable_huge_صفحات)
      حافظه_huge_صفحات = 1;
  }

#إذا PLATFORM_WINDOWS
  إذا (حافظه_config.enable_huge_صفحات) {
    HANDLE token = 0;
    size_t large_صفحه_minimum = Get_Large_صفحه_Minimum();
    إذا (large_صفحه_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    إذا (token) {
      LUID luid;
      إذا (Lookup_Privilege_مقدار(0, SE_قفل_حافظه_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, حجم(token_privileges));
        token_privileges.Privilege_شمار = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].صفات = SE_PRIVILEGE_ENABLED;
        إذا (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          إذا (Get_آخرین_Error() == ERROR_SUCCESS)
            حافظه_huge_صفحات = 1;
        }
      }
      CloseHandle(token);
    }
    إذا (حافظه_huge_صفحات) {
      إذا (large_صفحه_minimum > حافظه_صفحه_اندازه)
        حافظه_صفحه_اندازه = large_صفحه_minimum;
      إذا (large_صفحه_minimum > حافظه_map_granularity)
        حافظه_map_granularity = large_صفحه_minimum;
    }
  }
#پایان_اگر

  size_t min_بازه_اندازه = 256;
  size_t max_صفحه_اندازه;
#إذا UINTPTR_MAX > 0xFFFFFFFF
  max_صفحه_اندازه = 4096ULL * 1024ULL * 1024ULL;
#وإلا
  max_صفحه_اندازه = 4 * 1024 * 1024;
#پایان_اگر
  إذا (حافظه_صفحه_اندازه < min_بازه_اندازه)
    حافظه_صفحه_اندازه = min_بازه_اندازه;
  إذا (حافظه_صفحه_اندازه > max_صفحه_اندازه)
    حافظه_صفحه_اندازه = max_صفحه_اندازه;
  حافظه_صفحه_اندازه_لغزش = 0;
  size_t صفحه_اندازه_بیت = حافظه_صفحه_اندازه;
  طالما (صفحه_اندازه_بیت != 1) {
    ++حافظه_صفحه_اندازه_لغزش;
    صفحه_اندازه_بیت >>= 1;
  }
  حافظه_صفحه_اندازه = ((size_t)1 << حافظه_صفحه_اندازه_لغزش);

#إذا RPMALLOC_CONFIGURABLE
  إذا (!حافظه_config.بازه_اندازه) {
    حافظه_بازه_اندازه = حافظه_default_بازه_اندازه;
    حافظه_بازه_اندازه_لغزش = حافظه_default_بازه_اندازه_لغزش;
    حافظه_بازه_ماسک = حافظه_default_بازه_ماسک;
  } وإلا {
    size_t بازه_اندازه = حافظه_config.بازه_اندازه;
    إذا (بازه_اندازه > (256 * 1024))
      بازه_اندازه = (256 * 1024);
    حافظه_بازه_اندازه = 4096;
    حافظه_بازه_اندازه_لغزش = 12;
    طالما (حافظه_بازه_اندازه < بازه_اندازه) {
      حافظه_بازه_اندازه <<= 1;
      ++حافظه_بازه_اندازه_لغزش;
    }
    حافظه_بازه_ماسک = ~(uintptr_t)(حافظه_بازه_اندازه - 1);
  }
#پایان_اگر

  حافظه_بازه_map_شمار =
      (حافظه_config.بازه_map_شمار ? حافظه_config.بازه_map_شمار
                                     : DEFAULT_بازه_MAP_شمار);
  إذا ((حافظه_بازه_اندازه * حافظه_بازه_map_شمار) < حافظه_صفحه_اندازه)
    حافظه_بازه_map_شمار = (حافظه_صفحه_اندازه / حافظه_بازه_اندازه);
  إذا ((حافظه_صفحه_اندازه >= حافظه_بازه_اندازه) &&
      ((حافظه_بازه_map_شمار * حافظه_بازه_اندازه) % حافظه_صفحه_اندازه))
    حافظه_بازه_map_شمار = (حافظه_صفحه_اندازه / حافظه_بازه_اندازه);
  حافظه_انباشته_reserve_شمار = (حافظه_بازه_map_شمار > DEFAULT_بازه_MAP_شمار)
                                   ? DEFAULT_بازه_MAP_شمار
                                   : حافظه_بازه_map_شمار;

  حافظه_config.صفحه_اندازه_2 = حافظه_صفحه_اندازه;
  حافظه_config.بازه_اندازه = حافظه_بازه_اندازه;
  حافظه_config.بازه_map_شمار = حافظه_بازه_map_شمار;
  حافظه_config.enable_huge_صفحات = حافظه_huge_صفحات;

#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  إذا (pthread_کلید_create(&حافظه_thread_انباشته, rpmalloc_انباشته_release_raw_fc))
    أرجع -1;
#پایان_اگر
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_کلید = FlsAlloc(&_rpmalloc_thread_destructor);
#پایان_اگر

  // Setup all small and medium size classes
  size_t iclass = 0;
  حافظه_اندازه_class[iclass].بلوک_اندازه_2 = SMALL_GRANULARITY;
  rpmalloc_adjust_اندازه_class(iclass);
  لكل (iclass = 1; iclass < SMALL_CLASS_شمار; ++iclass) {
    size_t اندازه_2 = iclass * SMALL_GRANULARITY;
    حافظه_اندازه_class[iclass].بلوک_اندازه_2 = (uint32_t)اندازه_2;
    rpmalloc_adjust_اندازه_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  حافظه_medium_اندازه_حد = (حافظه_بازه_اندازه - بازه_سرآیند_اندازه) >> 1;
  إذا (حافظه_medium_اندازه_حد > MEDIUM_اندازه_حد)
    حافظه_medium_اندازه_حد = MEDIUM_اندازه_حد;
  لكل (iclass = 0; iclass < MEDIUM_CLASS_شمار; ++iclass) {
    size_t اندازه_2 = SMALL_اندازه_حد + ((iclass + 1) * MEDIUM_GRANULARITY);
    إذا (اندازه_2 > حافظه_medium_اندازه_حد) {
      حافظه_medium_اندازه_حد =
          SMALL_اندازه_حد + (iclass * MEDIUM_GRANULARITY);
      اكسر;
    }
    حافظه_اندازه_class[SMALL_CLASS_شمار + iclass].بلوک_اندازه_2 = (uint32_t)اندازه_2;
    rpmalloc_adjust_اندازه_class(SMALL_CLASS_شمار + iclass);
  }

  حافظه_orphan_heaps = 0;
#إذا RPMALLOC_اولی_CLASS_HEAPS
  حافظه_اولی_class_orphan_heaps = 0;
#پایان_اگر
#إذا ENABLE_STATISTICS
  atomic_store32(&حافظه_فعال_heaps, 0);
  atomic_store32(&mapped_صفحات, 0);
  mapped_صفحات_peak = 0;
  atomic_store32(&_master_spans, 0);
  atomic_store32(&mapped_مجموع, 0);
  atomic_store32(&unmapped_مجموع, 0);
  atomic_store32(&mapped_صفحات_os, 0);
  atomic_store32(&huge_صفحات_current, 0);
  huge_صفحات_peak = 0;
#پایان_اگر
  memset(حافظه_heaps, 0, حجم(حافظه_heaps));
  atomic_store32_release(&حافظه_global_قفل, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  أرجع 0;
}

//! Finalize the allocator
فراغ rpmalloc_finalize(فراغ) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  إذا (حافظه_global_reserve) {
    atomic_add32(&حافظه_global_reserve_master->مانده_spans,
                 -(int32_t)حافظه_global_reserve_شمار);
    حافظه_global_reserve_master = 0;
    حافظه_global_reserve_شمار = 0;
    حافظه_global_reserve = 0;
  }
  atomic_store32_release(&حافظه_global_قفل, 0);

  // Free all thread caches and fully free spans
  لكل (size_t list_idx = 0; list_idx < انباشته_ARRAY_اندازه; ++list_idx) {
    انباشته_t *انباشته = حافظه_heaps[list_idx];
    طالما (انباشته) {
      انباشته_t *بعدی_انباشته = انباشته->بعدی_انباشته;
      انباشته->finalize = 1;
      rpmalloc_انباشته_global_finalize(انباشته);
      انباشته = بعدی_انباشته;
    }
  }

#إذا ENABLE_GLOBAL_CACHE
  // Free global caches
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass)
    _rpmalloc_global_cache_finalize(&حافظه_بازه_cache[iclass]);
#پایان_اگر

#إذا (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_کلید_delete(حافظه_thread_انباشته);
#پایان_اگر
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_کلید);
  fls_کلید = 0;
#پایان_اگر
#إذا ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&mapped_صفحات) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&mapped_صفحات_os) == 0,
                  "Memory leak detected");
#پایان_اگر

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
خارجي مضمن فراغ rpmalloc_thread_initialize(فراغ) {
  إذا (!get_thread_انباشته_raw()) {
    انباشته_t *انباشته = rpmalloc_انباشته_allocate(0);
    إذا (انباشته) {
      _rpmalloc_stat_inc(&حافظه_فعال_heaps);
      مجموعه_thread_انباشته(انباشته);
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_مجموعه_مقدار(fls_کلید, انباشته);
#پایان_اگر
    }
  }
}

//! Finalize thread, orphan heap
فراغ rpmalloc_thread_finalize(صحيح release_caches) {
  انباشته_t *انباشته = get_thread_انباشته_raw();
  إذا (انباشته)
    rpmalloc_انباشته_release_raw(انباشته, release_caches);
  مجموعه_thread_انباشته(0);
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_مجموعه_مقدار(fls_کلید, 0);
#پایان_اگر
}

صحيح rpmalloc_is_thread_initialized(فراغ) {
  أرجع (get_thread_انباشته_raw() != 0) ? 1 : 0;
}

ثابت rpmalloc_config_t *rpmalloc_config(فراغ) { أرجع &حافظه_config; }

// Extern interface

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpmalloc(size_t اندازه_2) {
#إذا ENABLE_VALIDATE_آرگومانها
  إذا (اندازه_2 >= MAX_ALLOC_اندازه) {
    errno = EINVAL;
    أرجع 0;
  }
#پایان_اگر
  انباشته_t *انباشته = get_thread_انباشته();
  أرجع _rpmalloc_allocate(انباشته, اندازه_2);
}

خارجي مضمن فراغ rpfree(فراغ *ptr) { _rpmalloc_deallocate(ptr); }

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpcalloc(size_t رقم_2, size_t اندازه_2) {
  size_t مجموع_2;
#إذا ENABLE_VALIDATE_آرگومانها
#إذا PLATFORM_WINDOWS
  صحيح err = اندازه_TMult(رقم_2, اندازه_2, &مجموع_2);
  إذا ((err != S_OK) || (مجموع_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(رقم_2, اندازه_2, &مجموع_2);
  إذا (err || (مجموع_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#پایان_اگر
#وإلا
  مجموع_2 = رقم_2 * اندازه_2;
#پایان_اگر
  انباشته_t *انباشته = get_thread_انباشته();
  فراغ *بلوک = _rpmalloc_allocate(انباشته, مجموع_2);
  إذا (بلوک)
    memset(بلوک, 0, مجموع_2);
  أرجع بلوک;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rprealloc(فراغ *ptr, size_t اندازه_2) {
#إذا ENABLE_VALIDATE_آرگومانها
  إذا (اندازه_2 >= MAX_ALLOC_اندازه) {
    errno = EINVAL;
    أرجع ptr;
  }
#پایان_اگر
  انباشته_t *انباشته = get_thread_انباشته();
  أرجع _rpmalloc_reallocate(انباشته, ptr, اندازه_2, 0, 0);
}

خارجي RPMALLOC_ALLOCATOR فراغ *rpaligned_realloc(فراغ *ptr, size_t alignment,
                                                  size_t اندازه_2, size_t oldsize,
                                                  بیعلامت صحيح flags) {
#إذا ENABLE_VALIDATE_آرگومانها
  إذا ((اندازه_2 + alignment < اندازه_2) || (alignment > حافظه_صفحه_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#پایان_اگر
  انباشته_t *انباشته = get_thread_انباشته();
  أرجع _rpmalloc_aligned_reallocate(انباشته, ptr, alignment, اندازه_2, oldsize,
                                      flags);
}

خارجي RPMALLOC_ALLOCATOR فراغ *rpaligned_alloc(size_t alignment, size_t اندازه_2) {
  انباشته_t *انباشته = get_thread_انباشته();
  أرجع _rpmalloc_aligned_allocate(انباشته, alignment, اندازه_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpaligned_calloc(size_t alignment, size_t رقم_2, size_t اندازه_2) {
  size_t مجموع_2;
#إذا ENABLE_VALIDATE_آرگومانها
#إذا PLATFORM_WINDOWS
  صحيح err = اندازه_TMult(رقم_2, اندازه_2, &مجموع_2);
  إذا ((err != S_OK) || (مجموع_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(رقم_2, اندازه_2, &مجموع_2);
  إذا (err || (مجموع_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#پایان_اگر
#وإلا
  مجموع_2 = رقم_2 * اندازه_2;
#پایان_اگر
  فراغ *بلوک = rpaligned_alloc(alignment, مجموع_2);
  إذا (بلوک)
    memset(بلوک, 0, مجموع_2);
  أرجع بلوک;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpmemalign(size_t alignment,
                                                  size_t اندازه_2) {
  أرجع rpaligned_alloc(alignment, اندازه_2);
}

خارجي مضمن صحيح rpposix_memalign(فراغ **memptr, size_t alignment,
                                   size_t اندازه_2) {
  إذا (memptr)
    *memptr = rpaligned_alloc(alignment, اندازه_2);
  وإلا
    أرجع EINVAL;
  أرجع *memptr ? 0 : ENOMEM;
}

خارجي مضمن size_t rpmalloc_usable_اندازه(فراغ *ptr) {
  أرجع (ptr ? rpmalloc_usable_اندازه_2(ptr) : 0);
}

خارجي مضمن فراغ rpmalloc_thread_collect(فراغ) {}

فراغ rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, حجم(rpmalloc_thread_statistics_t));
  انباشته_t *انباشته = get_thread_انباشته_raw();
  إذا (!انباشته)
    أرجع;

  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمار; ++iclass) {
    اندازه_class_t *اندازه_class = حافظه_اندازه_class + iclass;
    بازه_t *بازه = انباشته->اندازه_class[iclass].partial_بازه;
    طالما (بازه) {
      size_t free_شمار = بازه->list_اندازه;
      size_t بلوک_شمار = اندازه_class->بلوک_شمار;
      إذا (بازه->free_list_حد < بلوک_شمار)
        بلوک_شمار = بازه->free_list_حد;
      free_شمار += (بلوک_شمار - بازه->استفاده_شده_شمار);
      stats->sizecache += free_شمار * اندازه_class->بلوک_اندازه_2;
      بازه = بازه->بعدی;
    }
  }

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    بازه_cache_t *بازه_cache;
    إذا (!iclass)
      بازه_cache = &انباشته->بازه_cache;
    وإلا
      بازه_cache = (بازه_cache_t *)(انباشته->بازه_large_cache + (iclass - 1));
    stats->spancache += بازه_cache->شمار * (iclass + 1) * حافظه_بازه_اندازه;
  }
#پایان_اگر

  بازه_t *deferred = (بازه_t *)atomic_بارگیری_ptr(&انباشته->بازه_free_deferred);
  طالما (deferred) {
    إذا (deferred->اندازه_class != اندازه_CLASS_HUGE)
      stats->spancache += (size_t)deferred->بازه_شمار * حافظه_بازه_اندازه;
    deferred = (بازه_t *)deferred->free_list;
  }

#إذا ENABLE_STATISTICS
  stats->thread_مقصد_global = (size_t)atomic_load64(&انباشته->thread_مقصد_global);
  stats->global_مقصد_thread = (size_t)atomic_load64(&انباشته->global_مقصد_thread);

  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    stats->بازه_use[iclass].current =
        (size_t)atomic_load32(&انباشته->بازه_use[iclass].current);
    stats->بازه_use[iclass].peak =
        (size_t)atomic_load32(&انباشته->بازه_use[iclass].بالا);
    stats->بازه_use[iclass].مقصد_global =
        (size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_مقصد_global);
    stats->بازه_use[iclass].from_global =
        (size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_from_global);
    stats->بازه_use[iclass].مقصد_cache =
        (size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_مقصد_cache);
    stats->بازه_use[iclass].from_cache =
        (size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_from_cache);
    stats->بازه_use[iclass].مقصد_reserved =
        (size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_مقصد_reserved);
    stats->بازه_use[iclass].from_reserved =
        (size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_from_reserved);
    stats->بازه_use[iclass].map_calls =
        (size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_map_calls);
  }
  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمار; ++iclass) {
    stats->اندازه_use[iclass].alloc_current =
        (size_t)atomic_load32(&انباشته->اندازه_class_use[iclass].alloc_current);
    stats->اندازه_use[iclass].alloc_peak =
        (size_t)انباشته->اندازه_class_use[iclass].alloc_peak;
    stats->اندازه_use[iclass].alloc_مجموع =
        (size_t)atomic_load32(&انباشته->اندازه_class_use[iclass].alloc_مجموع);
    stats->اندازه_use[iclass].free_مجموع =
        (size_t)atomic_load32(&انباشته->اندازه_class_use[iclass].free_مجموع);
    stats->اندازه_use[iclass].spans_مقصد_cache =
        (size_t)atomic_load32(&انباشته->اندازه_class_use[iclass].spans_مقصد_cache);
    stats->اندازه_use[iclass].spans_from_cache =
        (size_t)atomic_load32(&انباشته->اندازه_class_use[iclass].spans_from_cache);
    stats->اندازه_use[iclass].spans_from_reserved = (size_t)atomic_load32(
        &انباشته->اندازه_class_use[iclass].spans_from_reserved);
    stats->اندازه_use[iclass].map_calls =
        (size_t)atomic_load32(&انباشته->اندازه_class_use[iclass].spans_map_calls);
  }
#پایان_اگر
}

فراغ rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, حجم(rpmalloc_global_statistics_t));
#إذا ENABLE_STATISTICS
  stats->mapped = (size_t)atomic_load32(&mapped_صفحات) * حافظه_صفحه_اندازه;
  stats->mapped_peak = (size_t)mapped_صفحات_peak * حافظه_صفحه_اندازه;
  stats->mapped_مجموع_2 =
      (size_t)atomic_load32(&mapped_مجموع) * حافظه_صفحه_اندازه;
  stats->unmapped_مجموع_2 =
      (size_t)atomic_load32(&unmapped_مجموع) * حافظه_صفحه_اندازه;
  stats->huge_alloc =
      (size_t)atomic_load32(&huge_صفحات_current) * حافظه_صفحه_اندازه;
  stats->huge_alloc_peak = (size_t)huge_صفحات_peak * حافظه_صفحه_اندازه;
#پایان_اگر
#إذا ENABLE_GLOBAL_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    global_cache_t *cache = &حافظه_بازه_cache[iclass];
    طالما (!atomic_cas32_acquire(&cache->قفل_2, 1, 0))
      _rpmalloc_spin();
    uint32_t شمار = cache->شمار;
#إذا ENABLE_UNLIMITED_CACHE
    بازه_t *current_بازه = cache->سرریز_2;
    طالما (current_بازه) {
      ++شمار;
      current_بازه = current_بازه->بعدی;
    }
#پایان_اگر
    atomic_store32_release(&cache->قفل_2, 0);
    stats->cached += شمار * (iclass + 1) * حافظه_بازه_اندازه;
  }
#پایان_اگر
}

#إذا ENABLE_STATISTICS

ساكن فراغ حافظه_انباشته_dump_statistics(انباشته_t *انباشته, فراغ *file) {
  fprintf(file, "Heap %d stats:\n", انباشته->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمار; ++iclass) {
    إذا (!atomic_load32(&انباشته->اندازه_class_use[iclass].alloc_مجموع))
      استمر;
    fprintf(
        file,
        "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu "
        "%9u\n",
        (uint32_t)iclass,
        atomic_load32(&انباشته->اندازه_class_use[iclass].alloc_current),
        انباشته->اندازه_class_use[iclass].alloc_peak,
        atomic_load32(&انباشته->اندازه_class_use[iclass].alloc_مجموع),
        atomic_load32(&انباشته->اندازه_class_use[iclass].free_مجموع),
        حافظه_اندازه_class[iclass].بلوک_اندازه_2,
        حافظه_اندازه_class[iclass].بلوک_شمار,
        atomic_load32(&انباشته->اندازه_class_use[iclass].spans_current),
        انباشته->اندازه_class_use[iclass].spans_peak,
        ((size_t)انباشته->اندازه_class_use[iclass].alloc_peak *
         (size_t)حافظه_اندازه_class[iclass].بلوک_اندازه_2) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&انباشته->اندازه_class_use[iclass].spans_مقصد_cache) *
         حافظه_بازه_اندازه) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&انباشته->اندازه_class_use[iclass].spans_from_cache) *
         حافظه_بازه_اندازه) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(
             &انباشته->اندازه_class_use[iclass].spans_from_reserved) *
         حافظه_بازه_اندازه) /
            (size_t)(1024 * 1024),
        atomic_load32(&انباشته->اندازه_class_use[iclass].spans_map_calls));
  }
  fprintf(file, "Spans  Current     Peak Deferred  PeakMiB  Cached  ToCacheMiB "
                "FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB "
                "FromGlobalMiB  MmapCalls\n");
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    إذا (!atomic_load32(&انباشته->بازه_use[iclass].بالا) &&
        !atomic_load32(&انباشته->بازه_use[iclass].spans_map_calls))
      استمر;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&انباشته->بازه_use[iclass].current),
        atomic_load32(&انباشته->بازه_use[iclass].بالا),
        atomic_load32(&انباشته->بازه_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&انباشته->بازه_use[iclass].بالا) *
         (size_t)حافظه_بازه_اندازه * (iclass + 1)) /
            (size_t)(1024 * 1024),
#إذا ENABLE_THREAD_CACHE
        (بیعلامت صحيح)(!iclass ? انباشته->بازه_cache.شمار
                               : انباشته->بازه_large_cache[iclass - 1].شمار),
        ((size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_مقصد_cache) *
         (iclass + 1) * حافظه_بازه_اندازه) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_from_cache) *
         (iclass + 1) * حافظه_بازه_اندازه) /
            (size_t)(1024 * 1024),
#وإلا
        0, (size_t)0, (size_t)0,
#پایان_اگر
        ((size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_مقصد_reserved) *
         (iclass + 1) * حافظه_بازه_اندازه) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_from_reserved) *
         (iclass + 1) * حافظه_بازه_اندازه) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_مقصد_global) *
         (size_t)حافظه_بازه_اندازه * (iclass + 1)) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&انباشته->بازه_use[iclass].spans_from_global) *
         (size_t)حافظه_بازه_اندازه * (iclass + 1)) /
            (size_t)(1024 * 1024),
        atomic_load32(&انباشته->بازه_use[iclass].spans_map_calls));
  }
  fprintf(file, "Full spans: %zu\n", انباشته->پر_بازه_شمار);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&انباشته->thread_مقصد_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&انباشته->global_مقصد_thread) / (size_t)(1024 * 1024));
}

#پایان_اگر

فراغ rpmalloc_dump_statistics(فراغ *file) {
#إذا ENABLE_STATISTICS
  لكل (size_t list_idx = 0; list_idx < انباشته_ARRAY_اندازه; ++list_idx) {
    انباشته_t *انباشته = حافظه_heaps[list_idx];
    طالما (انباشته) {
      صحيح need_dump = 0;
      لكل (size_t iclass = 0; !need_dump && (iclass < اندازه_CLASS_شمار);
           ++iclass) {
        إذا (!atomic_load32(&انباشته->اندازه_class_use[iclass].alloc_مجموع)) {
          rpmalloc_assert(
              !atomic_load32(&انباشته->اندازه_class_use[iclass].free_مجموع),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&انباشته->اندازه_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          استمر;
        }
        need_dump = 1;
      }
      لكل (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_شمار);
           ++iclass) {
        إذا (!atomic_load32(&انباشته->بازه_use[iclass].بالا) &&
            !atomic_load32(&انباشته->بازه_use[iclass].spans_map_calls))
          استمر;
        need_dump = 1;
      }
      إذا (need_dump)
        حافظه_انباشته_dump_statistics(انباشته, file);
      انباشته = انباشته->بعدی_انباشته;
    }
  }
  fprintf(file, "Global stats:\n");
  size_t huge_current =
      (size_t)atomic_load32(&huge_صفحات_current) * حافظه_صفحه_اندازه;
  size_t huge_peak = (size_t)huge_صفحات_peak * حافظه_صفحه_اندازه;
  fprintf(file, "HugeCurrentMiB HugePeakMiB\n");
  fprintf(file, "%14zu %11zu\n", huge_current / (size_t)(1024 * 1024),
          huge_peak / (size_t)(1024 * 1024));

#إذا ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    global_cache_t *cache = حافظه_بازه_cache + iclass;
    size_t global_cache = (size_t)cache->شمار * iclass * حافظه_بازه_اندازه;

    size_t global_سرریز_cache = 0;
    بازه_t *بازه = cache->سرریز_2;
    طالما (بازه) {
      global_سرریز_cache += iclass * حافظه_بازه_اندازه;
      بازه = بازه->بعدی;
    }
    إذا (global_cache || global_سرریز_cache || cache->درج_شمار ||
        cache->استخراج_شمار)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_سرریز_cache / (size_t)(1024 * 1024),
              cache->درج_شمار, cache->استخراج_شمار);
  }
#پایان_اگر

  size_t mapped = (size_t)atomic_load32(&mapped_صفحات) * حافظه_صفحه_اندازه;
  size_t mapped_os =
      (size_t)atomic_load32(&mapped_صفحات_os) * حافظه_صفحه_اندازه;
  size_t mapped_peak = (size_t)mapped_صفحات_peak * حافظه_صفحه_اندازه;
  size_t mapped_مجموع_2 =
      (size_t)atomic_load32(&mapped_مجموع) * حافظه_صفحه_اندازه;
  size_t unmapped_مجموع_2 =
      (size_t)atomic_load32(&unmapped_مجموع) * حافظه_صفحه_اندازه;
  fprintf(
      file,
      "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB\n");
  fprintf(file, "%9zu %11zu %13zu %14zu %16zu\n",
          mapped / (size_t)(1024 * 1024), mapped_os / (size_t)(1024 * 1024),
          mapped_peak / (size_t)(1024 * 1024),
          mapped_مجموع_2 / (size_t)(1024 * 1024),
          unmapped_مجموع_2 / (size_t)(1024 * 1024));

  fprintf(file, "\n");
#إذا 0
	int64_t allocated = atomic_load64(&تخصیص_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#پایان_اگر
#پایان_اگر
  (فراغ)حجم(file);
}

#إذا RPMALLOC_اولی_CLASS_HEAPS

خارجي مضمن rpmalloc_انباشته_t *rpmalloc_انباشته_acquire(فراغ) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  انباشته_t *انباشته = rpmalloc_انباشته_allocate(1);
  rpmalloc_assume(انباشته != عدم);
  انباشته->owner_thread = 0;
  _rpmalloc_stat_inc(&حافظه_فعال_heaps);
  أرجع انباشته;
}

خارجي مضمن فراغ rpmalloc_انباشته_release_2(rpmalloc_انباشته_t *انباشته) {
  إذا (انباشته)
    rpmalloc_انباشته_release(انباشته, 1, 1);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_انباشته_alloc(rpmalloc_انباشته_t *انباشته, size_t اندازه_2) {
#إذا ENABLE_VALIDATE_آرگومانها
  إذا (اندازه_2 >= MAX_ALLOC_اندازه) {
    errno = EINVAL;
    أرجع 0;
  }
#پایان_اگر
  أرجع _rpmalloc_allocate(انباشته, اندازه_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_انباشته_aligned_alloc(rpmalloc_انباشته_t *انباشته, size_t alignment,
                            size_t اندازه_2) {
#إذا ENABLE_VALIDATE_آرگومانها
  إذا (اندازه_2 >= MAX_ALLOC_اندازه) {
    errno = EINVAL;
    أرجع 0;
  }
#پایان_اگر
  أرجع _rpmalloc_aligned_allocate(انباشته, alignment, اندازه_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_انباشته_calloc(rpmalloc_انباشته_t *انباشته, size_t رقم_2, size_t اندازه_2) {
  أرجع rpmalloc_انباشته_aligned_calloc(انباشته, 0, رقم_2, اندازه_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_انباشته_aligned_calloc(rpmalloc_انباشته_t *انباشته, size_t alignment,
                             size_t رقم_2, size_t اندازه_2) {
  size_t مجموع_2;
#إذا ENABLE_VALIDATE_آرگومانها
#إذا PLATFORM_WINDOWS
  صحيح err = اندازه_TMult(رقم_2, اندازه_2, &مجموع_2);
  إذا ((err != S_OK) || (مجموع_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(رقم_2, اندازه_2, &مجموع_2);
  إذا (err || (مجموع_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#پایان_اگر
#وإلا
  مجموع_2 = رقم_2 * اندازه_2;
#پایان_اگر
  فراغ *بلوک = _rpmalloc_aligned_allocate(انباشته, alignment, مجموع_2);
  إذا (بلوک)
    memset(بلوک, 0, مجموع_2);
  أرجع بلوک;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_انباشته_realloc(rpmalloc_انباشته_t *انباشته, فراغ *ptr, size_t اندازه_2,
                      بیعلامت صحيح flags) {
#إذا ENABLE_VALIDATE_آرگومانها
  إذا (اندازه_2 >= MAX_ALLOC_اندازه) {
    errno = EINVAL;
    أرجع ptr;
  }
#پایان_اگر
  أرجع _rpmalloc_reallocate(انباشته, ptr, اندازه_2, 0, flags);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_انباشته_aligned_realloc(rpmalloc_انباشته_t *انباشته, فراغ *ptr,
                              size_t alignment, size_t اندازه_2,
                              بیعلامت صحيح flags) {
#إذا ENABLE_VALIDATE_آرگومانها
  إذا ((اندازه_2 + alignment < اندازه_2) || (alignment > حافظه_صفحه_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#پایان_اگر
  أرجع _rpmalloc_aligned_reallocate(انباشته, ptr, alignment, اندازه_2, 0, flags);
}

خارجي مضمن فراغ rpmalloc_انباشته_free(rpmalloc_انباشته_t *انباشته, فراغ *ptr) {
  (فراغ)حجم(انباشته);
  _rpmalloc_deallocate(ptr);
}

خارجي مضمن فراغ rpmalloc_انباشته_free_all(rpmalloc_انباشته_t *انباشته) {
  بازه_t *بازه;
  بازه_t *بعدی_بازه;

  rpmalloc_انباشته_cache_adopt_deferred(انباشته, 0);

  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمار; ++iclass) {
    بازه = انباشته->اندازه_class[iclass].partial_بازه;
    طالما (بازه) {
      بعدی_بازه = بازه->بعدی;
      rpmalloc_انباشته_cache_درج(انباشته, بازه);
      بازه = بعدی_بازه;
    }
    انباشته->اندازه_class[iclass].partial_بازه = 0;
    بازه = انباشته->پر_بازه[iclass];
    طالما (بازه) {
      بعدی_بازه = بازه->بعدی;
      rpmalloc_انباشته_cache_درج(انباشته, بازه);
      بازه = بعدی_بازه;
    }

    بازه = انباشته->اندازه_class[iclass].cache;
    إذا (بازه)
      rpmalloc_انباشته_cache_درج(انباشته, بازه);
    انباشته->اندازه_class[iclass].cache = 0;
  }
  memset(انباشته->اندازه_class, 0, حجم(انباشته->اندازه_class));
  memset(انباشته->پر_بازه, 0, حجم(انباشته->پر_بازه));

  بازه = انباشته->large_huge_بازه;
  طالما (بازه) {
    بعدی_بازه = بازه->بعدی;
    إذا (UNEXPECTED(بازه->اندازه_class == اندازه_CLASS_HUGE))
      _rpmalloc_deallocate_huge(بازه);
    وإلا
      rpmalloc_انباشته_cache_درج(انباشته, بازه);
    بازه = بعدی_بازه;
  }
  انباشته->large_huge_بازه = 0;
  انباشته->پر_بازه_شمار = 0;

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    بازه_cache_t *بازه_cache;
    إذا (!iclass)
      بازه_cache = &انباشته->بازه_cache;
    وإلا
      بازه_cache = (بازه_cache_t *)(انباشته->بازه_large_cache + (iclass - 1));
    إذا (!بازه_cache->شمار)
      استمر;
#إذا ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&انباشته->thread_مقصد_global,
                         بازه_cache->شمار * (iclass + 1) * حافظه_بازه_اندازه);
    rpmalloc_stat_جمع_کن(&انباشته->بازه_use[iclass].spans_مقصد_global,
                       بازه_cache->شمار);
    rpmalloc_global_cache_درج_spans(بازه_cache->بازه, iclass + 1,
                                        بازه_cache->شمار);
#وإلا
    لكل (size_t ispan = 0; ispan < بازه_cache->شمار; ++ispan)
      rpmalloc_بازه_unmap(بازه_cache->بازه[ispan]);
#پایان_اگر
    بازه_cache->شمار = 0;
  }
#پایان_اگر

#إذا ENABLE_STATISTICS
  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمار; ++iclass) {
    atomic_store32(&انباشته->اندازه_class_use[iclass].alloc_current, 0);
    atomic_store32(&انباشته->اندازه_class_use[iclass].spans_current, 0);
  }
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمار; ++iclass) {
    atomic_store32(&انباشته->بازه_use[iclass].current, 0);
  }
#پایان_اگر
}

خارجي مضمن فراغ rpmalloc_انباشته_thread_مجموعه_current(rpmalloc_انباشته_t *انباشته) {
  انباشته_t *prev_انباشته = get_thread_انباشته_raw();
  إذا (prev_انباشته != انباشته) {
    مجموعه_thread_انباشته(انباشته);
    إذا (prev_انباشته)
      rpmalloc_انباشته_release_2(prev_انباشته);
  }
}

خارجي مضمن rpmalloc_انباشته_t *rpmalloc_get_انباشته_for_ptr(فراغ *ptr) {
  // Grab the span, and then the heap from the span
  بازه_t *بازه = (بازه_t *)((uintptr_t)ptr & حافظه_بازه_ماسک);
  إذا (بازه) {
    أرجع بازه->انباشته;
  }
  أرجع 0;
}

#پایان_اگر

#إذا ENABLE_PRELOAD || ENABLE_OVERRIDE

#ضمن "malloc.c"

#پایان_اگر

فراغ rpmalloc_linker_reference(فراغ) { (فراغ)حجم(_rpmalloc_initialized); }
