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
#ہدایت clang diagnostic ignored "-Wunused-macros"
#ہدایت clang diagnostic ignored "-Wunused-function"
#إذا __has_warning("-Wreserved-identifier")
#ہدایت clang diagnostic ignored "-Wreserved-identifier"
#جے_انت
#إذا __has_warning("-Wstatic-in-inline")
#ہدایت clang diagnostic ignored "-Wstatic-in-inline"
#جے_انت
#وإلاإذا defined(__GNUC__)
#ہدایت GCC diagnostic ignored "-Wunused-macros"
#ہدایت GCC diagnostic ignored "-Wunused-function"
#جے_انت

#إذا !defined(__has_builtin)
#تعریف_کرو __has_builtin(b) 0
#جے_انت

#إذا defined(__GNUC__) || defined(__clang__)

#إذا __has_builtin(__builtin_memcpy_inline)
#تعریف_کرو _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#وإلا
#تعریف_کرو _rpmalloc_memcpy_const(x, y, s)                                        \
  افعل {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    حافظہ_نقل(x, y, s);                                                           \
  } طالما (0)
#جے_انت

#إذا __has_builtin(__builtin_memset_inline)
#تعریف_کرو _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#وإلا
#تعریف_کرو _rpmalloc_memset_const(x, y, s)                                        \
  افعل {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } طالما (0)
#جے_انت
#وإلا
#تعریف_کرو _rpmalloc_memcpy_const(x, y, s) حافظہ_نقل(x, y, s)
#تعریف_کرو _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#جے_انت

#إذا __has_builtin(__builtin_assume)
#تعریف_کرو rpmalloc_assume(cond) __builtin_assume(cond)
#وإلاإذا defined(__GNUC__)
#تعریف_کرو rpmalloc_assume(cond)                                                  \
  افعل {                                                                         \
    إذا (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } طالما (0)
#وإلاإذا defined(_MSC_VER)
#تعریف_کرو rpmalloc_assume(cond) __assume(cond)
#وإلا
#تعریف_کرو rpmalloc_assume(cond) 0
#جے_انت

#جے_بنا_تعریف ڈھیر_ARRAY_ناپ
//! Size of heap hashmap
#تعریف_کرو ڈھیر_ARRAY_ناپ 47
#جے_انت
#جے_بنا_تعریف ENABLE_THREAD_CACHE
//! Enable per-thread cache
#تعریف_کرو ENABLE_THREAD_CACHE 1
#جے_انت
#جے_بنا_تعریف ENABLE_GLOBAL_CACHE
//! Enable global cache shared between all threads, requires thread cache
#تعریف_کرو ENABLE_GLOBAL_CACHE 1
#جے_انت
#جے_بنا_تعریف ENABLE_VALIDATE_دلیلاں
//! Enable validation of args to public entry points
#تعریف_کرو ENABLE_VALIDATE_دلیلاں 0
#جے_انت
#جے_بنا_تعریف ENABLE_STATISTICS
//! Enable statistics collection
#تعریف_کرو ENABLE_STATISTICS 0
#جے_انت
#جے_بنا_تعریف ENABLE_ASSERTS
//! Enable asserts
#تعریف_کرو ENABLE_ASSERTS 0
#جے_انت
#جے_بنا_تعریف ENABLE_OVERRIDE
//! Override standard library malloc/free and new/delete entry points
#تعریف_کرو ENABLE_OVERRIDE 0
#جے_انت
#جے_بنا_تعریف ENABLE_PRELOAD
//! Support preloading
#تعریف_کرو ENABLE_PRELOAD 0
#جے_انت
#جے_بنا_تعریف DISABLE_UNMAP
//! Disable unmapping memory pages (also enables unlimited cache)
#تعریف_کرو DISABLE_UNMAP 0
#جے_انت
#جے_بنا_تعریف ENABLE_UNLIMITED_CACHE
//! Enable unlimited global cache (no unmapping until finalization)
#تعریف_کرو ENABLE_UNLIMITED_CACHE 0
#جے_انت
#جے_بنا_تعریف ENABLE_ADAPTIVE_THREAD_CACHE
//! Enable adaptive thread cache size based on use heuristics
#تعریف_کرو ENABLE_ADAPTIVE_THREAD_CACHE 0
#جے_انت
#جے_بنا_تعریف DEFAULT_وسعت_MAP_گنتی
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#تعریف_کرو DEFAULT_وسعت_MAP_گنتی 64
#جے_انت
#جے_بنا_تعریف GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#تعریف_کرو GLOBAL_CACHE_MULTIPLIER 8
#جے_انت

#إذا DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#غلطی Must use global cache إذا unmap is disabled
#جے_انت

#إذا DISABLE_UNMAP
#تعریف_رد ENABLE_UNLIMITED_CACHE
#تعریف_کرو ENABLE_UNLIMITED_CACHE 1
#جے_انت

#إذا !ENABLE_GLOBAL_CACHE
#تعریف_رد ENABLE_UNLIMITED_CACHE
#تعریف_کرو ENABLE_UNLIMITED_CACHE 0
#جے_انت

#إذا !ENABLE_THREAD_CACHE
#تعریف_رد ENABLE_ADAPTIVE_THREAD_CACHE
#تعریف_کرو ENABLE_ADAPTIVE_THREAD_CACHE 0
#جے_انت

#إذا defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#تعریف_کرو PLATFORM_WINDOWS 1
#تعریف_کرو PLATFORM_POSIX 0
#وإلا
#تعریف_کرو PLATFORM_WINDOWS 0
#تعریف_کرو PLATFORM_POSIX 1
#جے_انت

/// Platform and arch specifics
#إذا defined(_MSC_VER) && !defined(__clang__)
#ہدایت warning(disable : 5105)
#جے_بنا_تعریف FORCEINLINE
#تعریف_کرو FORCEINLINE مضمن __forceinline
#جے_انت
#تعریف_کرو _Static_assert static_assert
#وإلا
#جے_بنا_تعریف FORCEINLINE
#تعریف_کرو FORCEINLINE مضمن __attribute__((__always_inline__))
#جے_انت
#جے_انت
#إذا PLATFORM_WINDOWS
#جے_بنا_تعریف WIN32_LEAN_AND_اوسط
#تعریف_کرو WIN32_LEAN_AND_اوسط
#جے_انت
#ضمن <windows.h>
#إذا ENABLE_VALIDATE_دلیلاں
#ضمن <intsafe.h>
#جے_انت
#وإلا
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <time.h>
#ضمن <unistd.h>
#إذا defined(__linux__) || defined(__ANDROID__)
#ضمن <sys/prctl.h>
#إذا !defined(PR_سیٹ_VMA)
#تعریف_کرو PR_سیٹ_VMA 0x53564d41
#تعریف_کرو PR_سیٹ_VMA_ANON_NAME 0
#جے_انت
#جے_انت
#إذا defined(__APPLE__)
#ضمن <TargetConditionals.h>
#إذا !نشانہ_OS_IPHONE && !نشانہ_OS_SIMULATOR
#ضمن <mach/mach_vm.h>
#ضمن <mach/vm_statistics.h>
#جے_انت
#ضمن <pthread.h>
#جے_انت
#إذا defined(__HAIKU__) || defined(__TINYC__)
#ضمن <pthread.h>
#جے_انت
#جے_انت

#ضمن <errno.h>
#ضمن <stdint.h>
#ضمن <string.h>

#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#ضمن <fibersapi.h>
ساكن DWORD fls_کلید;
#جے_انت

#إذا PLATFORM_POSIX
#ضمن <sched.h>
#ضمن <sys/mman.h>
#جے_تعریف_شدہ __FreeBSD__
#ضمن <sys/sysctl.h>
#تعریف_کرو MAP_HUGETLB MAP_ALIGNED_SUPER
#جے_بنا_تعریف PROT_MAX
#تعریف_کرو PROT_MAX(f) 0
#جے_انت
#وإلا
#تعریف_کرو PROT_MAX(f) 0
#جے_انت
#جے_تعریف_شدہ __sun
خارجي صحيح madvise(caddr_t, size_t, صحيح);
#جے_انت
#جے_بنا_تعریف MAP_UNINITIALIZED
#تعریف_کرو MAP_UNINITIALIZED 0
#جے_انت
#جے_انت
#ضمن <errno.h>

#إذا ENABLE_ASSERTS
#تعریف_رد NDEBUG
#إذا defined(_MSC_VER) && !defined(_DEBUG)
#تعریف_کرو _DEBUG
#جے_انت
#ضمن <assert.h>
#تعریف_کرو RPMALLOC_TOSTRING_M(x) #x
#تعریف_کرو RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#تعریف_کرو rpmalloc_assert(truth, سنیہا)                                        \
  افعل {                                                                         \
    إذا (!(truth)) {                                                            \
      إذا (حافظہ_config.error_callback) {                                     \
        حافظہ_config.error_callback(سنیہا " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } وإلا {                                                                 \
        أكد((truth) && سنیہا);                                            \
      }                                                                        \
    }                                                                          \
  } طالما (0)
#وإلا
#تعریف_کرو rpmalloc_assert(truth, سنیہا)                                        \
  افعل {                                                                         \
  } طالما (0)
#جے_انت
#إذا ENABLE_STATISTICS
#ضمن <stdio.h>
#جے_انت

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#إذا defined(_MSC_VER) && !defined(__clang__)

تعريفنوع متطاير لما atomic32_t;
تعريفنوع متطاير لما لما atomic64_t;
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
ساكن FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t جوڑو) {
  أرجع (int32_t)Interlocked_Exchange_جوڑو(val, جوڑو) + جوڑو;
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
ساكن FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t جوڑو) {
  أرجع (int64_t)InterlockedExchangeAdd64(val, جوڑو) + جوڑو;
}
ساكن FORCEINLINE فراغ *atomic_لدو_ptr(atomicptr_t *src) {
  أرجع Interlocked_Compare_Exchange_اشارہ_گر(src, 0, 0);
}
ساكن FORCEINLINE فراغ atomic_store_ptr(atomicptr_t *dst, فراغ *val) {
  Interlocked_Exchange_اشارہ_گر(dst, val);
}
ساكن FORCEINLINE فراغ atomic_store_ptr_release(atomicptr_t *dst, فراغ *val) {
  Interlocked_Exchange_اشارہ_گر(dst, val);
}
ساكن FORCEINLINE فراغ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     فراغ *val) {
  أرجع (فراغ *)Interlocked_Exchange_اشارہ_گر((فراغ *متطاير *)dst, val);
}
ساكن FORCEINLINE صحيح atomic_cas_ptr(atomicptr_t *dst, فراغ *val, فراغ *ref) {
  أرجع (Interlocked_Compare_Exchange_اشارہ_گر((فراغ *متطاير *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#تعریف_کرو EXPECTED(x) (x)
#تعریف_کرو UNEXPECTED(x) (x)

#وإلا

#ضمن <stdatomic.h>

تعريفنوع متطاير _Atomic(int32_t) atomic32_t;
تعريفنوع متطاير _Atomic(int64_t) atomic64_t;
تعريفنوع متطاير _Atomic(فراغ *) atomicptr_t;

ساكن FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  أرجع atomic_لدو_explicit(src, memory_order_relaxed);
}
ساكن FORCEINLINE فراغ atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ساكن FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  أرجع atomic_fetch_جوڑو_explicit(val, 1, memory_order_relaxed) + 1;
}
ساكن FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  أرجع atomic_fetch_جوڑو_explicit(val, -1, memory_order_relaxed) - 1;
}
ساكن FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t جوڑو) {
  أرجع atomic_fetch_جوڑو_explicit(val, جوڑو, memory_order_relaxed) + جوڑو;
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
  أرجع atomic_لدو_explicit(val, memory_order_relaxed);
}
ساكن FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t جوڑو) {
  أرجع atomic_fetch_جوڑو_explicit(val, جوڑو, memory_order_relaxed) + جوڑو;
}
ساكن FORCEINLINE فراغ *atomic_لدو_ptr(atomicptr_t *src) {
  أرجع atomic_لدو_explicit(src, memory_order_relaxed);
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

#تعریف_کرو EXPECTED(x) __builtin_expect((x), 1)
#تعریف_کرو UNEXPECTED(x) __builtin_expect((x), 0)

#جے_انت

////////////
///
/// Statistics related functions (evaluate to nothing when statistics not
/// enabled)
///
//////

#إذا ENABLE_STATISTICS
#تعریف_کرو _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#تعریف_کرو _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#تعریف_کرو rpmalloc_stat_جوڑو(counter, قدر_2)                                     \
  atomic_add32(counter, (int32_t)(قدر_2))
#تعریف_کرو _rpmalloc_stat_add64(counter, قدر_2)                                   \
  atomic_add64(counter, (int64_t)(قدر_2))
#تعریف_کرو rpmalloc_stat_جوڑو_peak(counter, قدر_2, peak)                          \
  افعل {                                                                         \
    int32_t cur_گنتی = atomic_add32(counter, (int32_t)(قدر_2));              \
    إذا (cur_گنتی > (peak))                                                   \
      peak = cur_گنتی;                                                       \
  } طالما (0)
#تعریف_کرو _rpmalloc_stat_sub(counter, قدر_2)                                     \
  atomic_add32(counter, -(int32_t)(قدر_2))
#تعریف_کرو _rpmalloc_stat_inc_alloc(ڈھیر_2, class_idx)                              \
  افعل {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&ڈھیر_2->ناپ_class_use[class_idx].alloc_current);         \
    إذا (alloc_current > ڈھیر_2->ناپ_class_use[class_idx].alloc_peak)            \
      ڈھیر_2->ناپ_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&ڈھیر_2->ناپ_class_use[class_idx].alloc_کل);               \
  } طالما (0)
#تعریف_کرو _rpmalloc_stat_inc_free(ڈھیر_2, class_idx)                               \
  افعل {                                                                         \
    atomic_decr32(&ڈھیر_2->ناپ_class_use[class_idx].alloc_current);             \
    atomic_incr32(&ڈھیر_2->ناپ_class_use[class_idx].free_کل);                \
  } طالما (0)
#وإلا
#تعریف_کرو _rpmalloc_stat_inc(counter)                                            \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کرو _rpmalloc_stat_dec(counter)                                            \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کرو rpmalloc_stat_جوڑو(counter, قدر_2)                                     \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کرو _rpmalloc_stat_add64(counter, قدر_2)                                   \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کرو rpmalloc_stat_جوڑو_peak(counter, قدر_2, peak)                          \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کرو _rpmalloc_stat_sub(counter, قدر_2)                                     \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کرو _rpmalloc_stat_inc_alloc(ڈھیر_2, class_idx)                              \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کرو _rpmalloc_stat_inc_free(ڈھیر_2, class_idx)                               \
  افعل {                                                                         \
  } طالما (0)
#جے_انت

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#تعریف_کرو SMALL_GRANULARITY 16
//! Small granularity shift count
#تعریف_کرو SMALL_GRANULARITY_کھسکاؤ 4
//! Number of small block size classes
#تعریف_کرو SMALL_CLASS_گنتی 65
//! Maximum size of a small block
#تعریف_کرو SMALL_ناپ_حد_بندی (SMALL_GRANULARITY * (SMALL_CLASS_گنتی - 1))
//! Granularity of a medium allocation block
#تعریف_کرو MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#تعریف_کرو MEDIUM_GRANULARITY_کھسکاؤ 9
//! Number of medium block size classes
#تعریف_کرو MEDIUM_CLASS_گنتی 61
//! Total number of small + medium size classes
#تعریف_کرو ناپ_CLASS_گنتی (SMALL_CLASS_گنتی + MEDIUM_CLASS_گنتی)
//! Number of large block size classes
#تعریف_کرو LARGE_CLASS_گنتی 63
//! Maximum size of a medium block
#تعریف_کرو MEDIUM_ناپ_حد_بندی                                                      \
  (SMALL_ناپ_حد_بندی + (MEDIUM_GRANULARITY * MEDIUM_CLASS_گنتی))
//! Maximum size of a large block
#تعریف_کرو LARGE_ناپ_حد_بندی                                                       \
  ((LARGE_CLASS_گنتی * حافظہ_وسعت_ناپ) - وسعت_سرخی_ناپ)
//! Size of a span header (must be a multiple of SMALL_GRANULARITY and a power
//! of two)
#تعریف_کرو وسعت_سرخی_ناپ 128
//! Number of spans in thread cache
#تعریف_کرو MAX_THREAD_وسعت_CACHE 400
//! Number of spans to transfer between thread and global cache
#تعریف_کرو THREAD_وسعت_CACHE_TRANSFER 64
//! Number of spans in thread cache for large spans (must be greater than
//! LARGE_CLASS_COUNT / 2)
#تعریف_کرو MAX_THREAD_وسعت_LARGE_CACHE 100
//! Number of spans to transfer between thread and global cache for large spans
#تعریف_کرو THREAD_وسعت_LARGE_CACHE_TRANSFER 6

_Static_assert((SMALL_GRANULARITY & (SMALL_GRANULARITY - 1)) == 0,
               "Small granularity must be power of two");
_Static_assert((وسعت_سرخی_ناپ & (وسعت_سرخی_ناپ - 1)) == 0,
               "Span header size must be power of two");

#إذا ENABLE_VALIDATE_دلیلاں
//! Maximum allocation size to avoid integer overflow
#تعریف_رد MAX_ALLOC_ناپ
#تعریف_کرو MAX_ALLOC_ناپ (((size_t) - 1) - حافظہ_وسعت_ناپ)
#جے_انت

#تعریف_کرو اشارہ_گر_وتھ(ptr, ofs) (فراغ *)((حرف *)(ptr) + (ptrdiff_t)(ofs))
#تعریف_کرو اشارہ_گر_diff(پہلا_2, دوجا_2)                                            \
  (ptrdiff_t)((ثابت حرف *)(پہلا_2) - (ثابت حرف *)(دوجا_2))

#تعریف_کرو INVALID_اشارہ_گر ((فراغ *)((uintptr_t) - 1))

#تعریف_کرو ناپ_CLASS_LARGE ناپ_CLASS_گنتی
#تعریف_کرو ناپ_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
تعريفنوع بنية ڈھیر_t ڈھیر_t;
//! Span of memory pages
تعريفنوع بنية وسعت_t وسعت_t;
//! Span list
تعريفنوع بنية وسعت_list_t وسعت_list_t;
//! Span active data
تعريفنوع بنية وسعت_سرگرم_t وسعت_سرگرم_t;
//! Size class definition
تعريفنوع بنية ناپ_class_t ناپ_class_t;
//! Global cache
تعريفنوع بنية global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#تعریف_کرو وسعت_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#تعریف_کرو وسعت_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#تعریف_کرو وسعت_FLAG_ALIGNED_بلاکاں 4U
//! Flag indicating an unmapped master span
#تعریف_کرو وسعت_FLAG_UNMAPPED_MASTER 8U

#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
بنية وسعت_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t بلند;
#إذا ENABLE_STATISTICS
  //! Number of spans in deferred list
  atomic32_t spans_deferred;
  //! Number of spans transitioned to global cache
  atomic32_t spans_منزل_global;
  //! Number of spans transitioned from global cache
  atomic32_t spans_from_global;
  //! Number of spans transitioned to thread cache
  atomic32_t spans_منزل_cache;
  //! Number of spans transitioned from thread cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned to reserved state
  atomic32_t spans_منزل_reserved;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of raw memory map calls
  atomic32_t spans_map_calls;
#جے_انت
};
تعريفنوع بنية وسعت_use_t وسعت_use_t;
#جے_انت

#إذا ENABLE_STATISTICS
بنية ناپ_class_use_t {
  //! Current number of allocations
  atomic32_t alloc_current;
  //! Peak number of allocations
  int32_t alloc_peak;
  //! Total number of allocations
  atomic32_t alloc_کل;
  //! Total number of frees
  atomic32_t free_کل;
  //! Number of spans in use
  atomic32_t spans_current;
  //! Number of spans transitioned to cache
  int32_t spans_peak;
  //! Number of spans transitioned to cache
  atomic32_t spans_منزل_cache;
  //! Number of spans transitioned from cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of spans mapped
  atomic32_t spans_map_calls;
  int32_t بن_ورتیا_2;
};
تعريفنوع بنية ناپ_class_use_t ناپ_class_use_t;
#جے_انت

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
بنية وسعت_t {
  //! Free list
  فراغ *free_list;
  //! Total block count of size class
  uint32_t بلاک_گنتی;
  //! Size class
  uint32_t ناپ_class;
  //! Index of last block initialized in free list
  uint32_t free_list_حد_بندی;
  //! Number of used blocks remaining when in partial state
  uint32_t ورتیا_گنتی;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_ناپ;
  //! Size of a block
  uint32_t بلاک_ناپ_2;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t وسعت_گنتی;
  //! Total span counter for master spans
  uint32_t کل_spans;
  //! Offset from master span for subspans
  uint32_t وتھ_from_master;
  //! Remaining span counter, for master spans
  atomic32_t بچیا_spans;
  //! Alignment offset
  uint32_t align_وتھ;
  //! Owning heap
  ڈھیر_t *ڈھیر_2;
  //! Next span
  وسعت_t *اگلا;
  //! Previous span
  وسعت_t *prev;
};
_Static_assert(حجم(وسعت_t) <= وسعت_سرخی_ناپ, "span size mismatch");

بنية وسعت_cache_t {
  size_t گنتی;
  وسعت_t *وسعت[MAX_THREAD_وسعت_CACHE];
};
تعريفنوع بنية وسعت_cache_t وسعت_cache_t;

بنية وسعت_large_cache_t {
  size_t گنتی;
  وسعت_t *وسعت[MAX_THREAD_وسعت_LARGE_CACHE];
};
تعريفنوع بنية وسعت_large_cache_t وسعت_large_cache_t;

بنية ڈھیر_ناپ_class_t {
  //! Free list of active span
  فراغ *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  وسعت_t *partial_وسعت;
  //! Early level cache of fully free spans
  وسعت_t *cache;
};
تعريفنوع بنية ڈھیر_ناپ_class_t ڈھیر_ناپ_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
بنية ڈھیر_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  ڈھیر_ناپ_class_t ناپ_class[ناپ_CLASS_گنتی];
#إذا ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  وسعت_cache_t وسعت_cache;
#جے_انت
  //! List of deferred free spans (single linked list)
  atomicptr_t وسعت_free_deferred;
  //! Number of full spans
  size_t بھریا_وسعت_گنتی;
  //! Mapped but unused spans
  وسعت_t *وسعت_reserve;
  //! Master span for mapped but unused spans
  وسعت_t *وسعت_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t بچہ_گنتی;
  //! Next heap in id list
  ڈھیر_t *اگلا_ڈھیر;
  //! Next heap in orphan list
  ڈھیر_t *اگلا_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  صحيح finalize;
  //! Master heap owning the memory pages
  ڈھیر_t *master_ڈھیر;
#إذا ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  وسعت_large_cache_t وسعت_large_cache[LARGE_CLASS_گنتی - 1];
#جے_انت
#إذا RPMALLOC_پہلا_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  وسعت_t *بھریا_وسعت[ناپ_CLASS_گنتی];
  //! Double linked list of large and huge spans allocated by this heap
  وسعت_t *large_huge_وسعت;
#جے_انت
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  وسعت_use_t وسعت_use[LARGE_CLASS_گنتی];
#جے_انت
#إذا ENABLE_STATISTICS
  //! Allocation stats per size class
  ناپ_class_use_t ناپ_class_use[ناپ_CLASS_گنتی + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_منزل_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_منزل_thread;
#جے_انت
};

// Size class for defining a block size bucket
بنية ناپ_class_t {
  //! Size of blocks in this class
  uint32_t بلاک_ناپ_2;
  //! Number of blocks in each chunk
  uint16_t بلاک_گنتی;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(حجم(ناپ_class_t) == 8, "Size class size mismatch");

بنية global_cache_t {
  //! Cache lock
  atomic32_t جندرا;
  //! Cache count
  uint32_t گنتی;
#إذا ENABLE_STATISTICS
  //! Insert count
  size_t داخل_کرو_گنتی;
  //! Extract count
  size_t کڈھو_گنتی;
#جے_انت
  //! Cached spans
  وسعت_t *وسعت[GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_وسعت_CACHE];
  //! Unlimited cache overflow
  وسعت_t *حد_توں_ودھ_2;
};

////////////
///
/// Global data
///
//////

//! Default span size (64KiB)
#تعریف_کرو حافظہ_default_وسعت_ناپ (64 * 1024)
#تعریف_کرو حافظہ_default_وسعت_ناپ_کھسکاؤ 16
#تعریف_کرو حافظہ_default_وسعت_نقاب (~((uintptr_t)(حافظہ_وسعت_ناپ - 1)))

//! Initialized flag
ساكن صحيح _rpmalloc_initialized;
//! Main thread ID
ساكن uintptr_t _rpmalloc_main_thread_id;
//! Configuration
ساكن rpmalloc_config_t حافظہ_config;
//! Memory page size
ساكن size_t حافظہ_ورقہ_ناپ;
//! Shift to divide by page size
ساكن size_t حافظہ_ورقہ_ناپ_کھسکاؤ;
//! Granularity at which memory pages are mapped by OS
ساكن size_t حافظہ_map_granularity;
#إذا RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
ساكن size_t حافظہ_وسعت_ناپ;
//! Shift to divide by span size
ساكن size_t حافظہ_وسعت_ناپ_کھسکاؤ;
//! Mask to get to start of a memory span
ساكن uintptr_t حافظہ_وسعت_نقاب;
#وإلا
//! Hardwired span size
#تعریف_کرو حافظہ_وسعت_ناپ حافظہ_default_وسعت_ناپ
#تعریف_کرو حافظہ_وسعت_ناپ_کھسکاؤ حافظہ_default_وسعت_ناپ_کھسکاؤ
#تعریف_کرو حافظہ_وسعت_نقاب حافظہ_default_وسعت_نقاب
#جے_انت
//! Number of spans to map in each map call
ساكن size_t حافظہ_وسعت_map_گنتی;
//! Number of spans to keep reserved in each heap
ساكن size_t حافظہ_ڈھیر_reserve_گنتی;
//! Global size classes
ساكن ناپ_class_t حافظہ_ناپ_class[ناپ_CLASS_گنتی];
//! Run-time size limit of medium blocks
ساكن size_t حافظہ_medium_ناپ_حد_بندی;
//! Heap ID counter
ساكن atomic32_t حافظہ_ڈھیر_id;
//! Huge page support
ساكن صحيح حافظہ_huge_ورقے;
#إذا ENABLE_GLOBAL_CACHE
//! Global span cache
ساكن global_cache_t حافظہ_وسعت_cache[LARGE_CLASS_گنتی];
#جے_انت
//! Global reserved spans
ساكن وسعت_t *حافظہ_global_reserve;
//! Global reserved count
ساكن size_t حافظہ_global_reserve_گنتی;
//! Global reserved master
ساكن وسعت_t *حافظہ_global_reserve_master;
//! All heaps
ساكن ڈھیر_t *حافظہ_heaps[ڈھیر_ARRAY_ناپ];
//! Used to restrict access to mapping memory for huge pages
ساكن atomic32_t حافظہ_global_جندرا;
//! Orphaned heaps
ساكن ڈھیر_t *حافظہ_orphan_heaps;
#إذا RPMALLOC_پہلا_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
ساكن ڈھیر_t *حافظہ_پہلا_class_orphan_heaps;
#جے_انت
#إذا ENABLE_STATISTICS
//! Allocations counter
ساكن atomic64_t ونڈ_counter;
//! Deallocations counter
ساكن atomic64_t _deallocation_counter;
//! Active heap count
ساكن atomic32_t حافظہ_سرگرم_heaps;
//! Number of currently mapped memory pages
ساكن atomic32_t mapped_ورقے;
//! Peak number of concurrently mapped memory pages
ساكن int32_t mapped_ورقے_peak;
//! Number of mapped master spans
ساكن atomic32_t _master_spans;
//! Number of unmapped dangling master spans
ساكن atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
ساكن atomic32_t mapped_کل;
//! Running counter of total number of unmapped memory pages since start
ساكن atomic32_t unmapped_کل;
//! Number of currently mapped memory pages in OS calls
ساكن atomic32_t mapped_ورقے_os;
//! Number of currently allocated pages in huge allocations
ساكن atomic32_t huge_ورقے_current;
//! Peak number of currently allocated pages in huge allocations
ساكن int32_t huge_ورقے_peak;
#جے_انت

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
ساكن pthread_key_t حافظہ_thread_ڈھیر;
#وإلا
#جے_تعریف_شدہ _MSC_VER
#تعریف_کرو _Thread_local __declspec(thread)
#تعریف_کرو TLS_MODEL
#وإلا
#جے_بنا_تعریف __HAIKU__
#تعریف_کرو TLS_MODEL __attribute__((tls_model("initial-exec")))
#وإلا
#تعریف_کرو TLS_MODEL
#جے_انت
#إذا !defined(__clang__) && defined(__GNUC__)
#تعریف_کرو _Thread_local __thread
#جے_انت
#جے_انت
ساكن _Thread_local ڈھیر_t *حافظہ_thread_ڈھیر TLS_MODEL;
#جے_انت

ساكن مضمن ڈھیر_t *get_thread_ڈھیر_raw(فراغ) {
#إذا (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  أرجع pthread_getspecific(حافظہ_thread_ڈھیر);
#وإلا
  أرجع حافظہ_thread_ڈھیر;
#جے_انت
}

//! Get the current thread heap
ساكن مضمن ڈھیر_t *get_thread_ڈھیر(فراغ) {
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر_raw();
#إذا ENABLE_PRELOAD
  إذا (EXPECTED(ڈھیر_2 != 0))
    أرجع ڈھیر_2;
  rpmalloc_initialize();
  أرجع get_thread_ڈھیر_raw();
#وإلا
  أرجع ڈھیر_2;
#جے_انت
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
#جے_انت
#وإلاإذا defined(__arm__)
  __asm__ متطاير("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#وإلاإذا defined(__aarch64__)
#إذا defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ متطاير("mrs %0, tpidrro_el0" : "=r"(tid));
#وإلا
  __asm__ متطاير("mrs %0, tpidr_el0" : "=r"(tid));
#جے_انت
#وإلا
#غلطی This platform needs implementation دا_3 get_thread_id()
#جے_انت
  أرجع tid;
#وإلا
#غلطی This platform needs implementation دا_3 get_thread_id()
#جے_انت
}

//! Set the current thread heap
ساكن فراغ سیٹ_thread_ڈھیر(ڈھیر_t *ڈھیر_2) {
#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(حافظہ_thread_ڈھیر, ڈھیر_2);
#وإلا
  حافظہ_thread_ڈھیر = ڈھیر_2;
#جے_انت
  إذا (ڈھیر_2)
    ڈھیر_2->owner_thread = get_thread_id();
}

//! Set main thread ID
خارجي فراغ rpmalloc_سیٹ_main_thread(فراغ);

فراغ rpmalloc_سیٹ_main_thread(فراغ) {
  _rpmalloc_main_thread_id = get_thread_id();
}

ساكن فراغ _rpmalloc_spin(فراغ) {
#إذا defined(_MSC_VER)
#إذا defined(_M_ARM64)
  __yield();
#وإلا
  _mm_pause();
#جے_انت
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
#جے_انت
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

#جے_تعریف_شدہ _WIN64
#ہدایت comment(linker, "/INCLUDE:_tls_used")
#ہدایت comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#ہدایت const_seg(".CRT$XLY")

خارجي ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#ہدایت const_seg()
#وإلا // _WIN64
#ہدایت comment(linker, "/INCLUDE:__tls_used")
#ہدایت comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#ہدایت انکڑے_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#ہدایت انکڑے_seg()
#جے_انت // _WIN64

ساكن فراغ NTAPI _rpmalloc_thread_destructor(فراغ *قدر_2) {
#إذا ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  إذا (get_thread_id() == _rpmalloc_main_thread_id)
    أرجع;
#جے_انت
  إذا (قدر_2)
    rpmalloc_thread_finalize(1);
}
#جے_انت

////////////
///
/// Low level memory map/unmap
///
//////

ساكن فراغ rpmalloc_سیٹ_name(فراغ *پتہ_2, size_t ناپ_2) {
#إذا defined(__linux__) || defined(__ANDROID__)
  ثابت حرف *name = حافظہ_huge_ورقے ? حافظہ_config.huge_ورقہ_name
                                        : حافظہ_config.ورقہ_name;
  إذا (پتہ_2 == MAP_FAILED || !name)
    أرجع;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (فراغ)prctl(PR_سیٹ_VMA, PR_سیٹ_VMA_ANON_NAME, (uintptr_t)پتہ_2, ناپ_2,
              (uintptr_t)name);
#وإلا
  (فراغ)حجم(ناپ_2);
  (فراغ)حجم(پتہ_2);
#جے_انت
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
ساكن فراغ *_rpmalloc_mmap(size_t ناپ_2, size_t *وتھ_2) {
  rpmalloc_assert(!(ناپ_2 % حافظہ_ورقہ_ناپ), "Invalid mmap size");
  rpmalloc_assert(ناپ_2 >= حافظہ_ورقہ_ناپ, "Invalid mmap size");
  فراغ *پتہ_2 = حافظہ_config.حافظہ_map(ناپ_2, وتھ_2);
  إذا (EXPECTED(پتہ_2 != 0)) {
    rpmalloc_stat_جوڑو_peak(&mapped_ورقے, (ناپ_2 >> حافظہ_ورقہ_ناپ_کھسکاؤ),
                            mapped_ورقے_peak);
    rpmalloc_stat_جوڑو(&mapped_کل, (ناپ_2 >> حافظہ_ورقہ_ناپ_کھسکاؤ));
  }
  أرجع پتہ_2;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
ساكن فراغ _rpmalloc_unmap(فراغ *پتہ_2, size_t ناپ_2, size_t وتھ_2,
                            size_t release) {
  rpmalloc_assert(!release || (release >= ناپ_2), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= حافظہ_ورقہ_ناپ),
                  "Invalid unmap size");
  إذا (release) {
    rpmalloc_assert(!(release % حافظہ_ورقہ_ناپ), "Invalid unmap size");
    _rpmalloc_stat_sub(&mapped_ورقے, (release >> حافظہ_ورقہ_ناپ_کھسکاؤ));
    rpmalloc_stat_جوڑو(&unmapped_کل, (release >> حافظہ_ورقہ_ناپ_کھسکاؤ));
  }
  حافظہ_config.حافظہ_unmap(پتہ_2, ناپ_2, وتھ_2, release);
}

//! Default implementation to map new pages to virtual memory
ساكن فراغ *_rpmalloc_mmap_os(size_t ناپ_2, size_t *وتھ_2) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((ناپ_2 >= حافظہ_وسعت_ناپ) &&
                    (حافظہ_وسعت_ناپ > حافظہ_map_granularity))
                       ? حافظہ_وسعت_ناپ
                       : 0;
  rpmalloc_assert(ناپ_2 >= حافظہ_ورقہ_ناپ, "Invalid mmap size");
#إذا PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  فراغ *ptr = VirtualAlloc(0, ناپ_2 + padding,
                           (حافظہ_huge_ورقے ? MEM_LARGE_ورقے : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           ورقہ_READWRITE);
  إذا (!ptr) {
    إذا (حافظہ_config.map_fail_callback) {
      إذا (حافظہ_config.map_fail_callback(ناپ_2 + padding))
        أرجع _rpmalloc_mmap_os(ناپ_2, وتھ_2);
    } وإلا {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    أرجع 0;
  }
#وإلا
  صحيح flags = MAP_نجی | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#إذا defined(__APPLE__) && !نشانہ_OS_IPHONE && !نشانہ_OS_SIMULATOR
  صحيح fd = (صحيح)VM_MAKE_ٹیگ(240U);
  إذا (حافظہ_huge_ورقے)
    fd |= VM_FLAGS_SUPERPAGE_ناپ_2_MB;
  فراغ *ptr = mmap(0, ناپ_2 + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#وإلاإذا defined(MAP_HUGETLB)
  فراغ *ptr = mmap(0, ناپ_2 + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (حافظہ_huge_ورقے ? MAP_HUGETLB : 0) | flags, -1, 0);
#إذا defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  إذا ((ptr == MAP_FAILED || !ptr) && حافظہ_huge_ورقے) {
    ptr = mmap(0, ناپ_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    إذا (ptr && ptr != MAP_FAILED) {
      صحيح prm = madvise(ptr, ناپ_2 + padding, MADV_HUGEPAGE);
      (فراغ)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#جے_انت
  rpmalloc_سیٹ_name(ptr, ناپ_2 + padding);
#وإلاإذا defined(MAP_ALIGNED)
  ثابت size_t align =
      (حجم(size_t) * 8) - (size_t)(__builtin_clzl(ناپ_2 - 1));
  فراغ *ptr =
      mmap(0, ناپ_2 + padding, PROT_READ | PROT_WRITE,
           (حافظہ_huge_ورقے ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#وإلاإذا defined(MAP_ALIGN)
  caddr_t ادھار_2 = (حافظہ_huge_ورقے ? (caddr_t)(4 << 20) : 0);
  فراغ *ptr = mmap(ادھار_2, ناپ_2 + padding, PROT_READ | PROT_WRITE,
                   (حافظہ_huge_ورقے ? MAP_ALIGN : 0) | flags, -1, 0);
#وإلا
  فراغ *ptr = mmap(0, ناپ_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#جے_انت
  إذا ((ptr == MAP_FAILED) || !ptr) {
    إذا (حافظہ_config.map_fail_callback) {
      إذا (حافظہ_config.map_fail_callback(ناپ_2 + padding))
        أرجع _rpmalloc_mmap_os(ناپ_2, وتھ_2);
    } وإلا إذا (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    أرجع 0;
  }
#جے_انت
  rpmalloc_stat_جوڑو(&mapped_ورقے_os,
                     (int32_t)((ناپ_2 + padding) >> حافظہ_ورقہ_ناپ_کھسکاؤ));
  إذا (padding) {
    size_t final_padding = padding - ((uintptr_t)ptr & ~حافظہ_وسعت_نقاب);
    rpmalloc_assert(final_padding <= حافظہ_وسعت_ناپ,
                    "Internal failure in padding");
    rpmalloc_assert(final_padding <= padding, "Internal failure in padding");
    rpmalloc_assert(!(final_padding % 8), "Internal failure in padding");
    ptr = اشارہ_گر_وتھ(ptr, final_padding);
    *وتھ_2 = final_padding >> 3;
  }
  rpmalloc_assert((ناپ_2 < حافظہ_وسعت_ناپ) ||
                      !((uintptr_t)ptr & ~حافظہ_وسعت_نقاب),
                  "Internal failure in padding");
  أرجع ptr;
}

//! Default implementation to unmap pages from virtual memory
ساكن فراغ _rpmalloc_unmap_os(فراغ *پتہ_2, size_t ناپ_2, size_t وتھ_2,
                               size_t release) {
  rpmalloc_assert(release || (وتھ_2 == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= حافظہ_ورقہ_ناپ),
                  "Invalid unmap size");
  rpmalloc_assert(ناپ_2 >= حافظہ_ورقہ_ناپ, "Invalid unmap size");
  إذا (release && وتھ_2) {
    وتھ_2 <<= 3;
    پتہ_2 = اشارہ_گر_وتھ(پتہ_2, -(int32_t)وتھ_2);
    إذا ((release >= حافظہ_وسعت_ناپ) &&
        (حافظہ_وسعت_ناپ > حافظہ_map_granularity)) {
      // Padding is always one span size
      release += حافظہ_وسعت_ناپ;
    }
  }
#إذا !DISABLE_UNMAP
#إذا PLATFORM_WINDOWS
  إذا (!VirtualFree(پتہ_2, release ? 0 : ناپ_2,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#وإلا
  إذا (release) {
    إذا (munmap(پتہ_2, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } وإلا {
#إذا defined(MADV_FREE_REUSABLE)
    صحيح ret;
    طالما ((ret = madvise(پتہ_2, ناپ_2, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    إذا ((ret == -1) && (errno != 0)) {
#وإلاإذا defined(MADV_DONTNEED)
    إذا (madvise(پتہ_2, ناپ_2, MADV_DONTNEED)) {
#وإلاإذا defined(MADV_PAGEOUT)
    إذا (madvise(پتہ_2, ناپ_2, MADV_PAGEOUT)) {
#وإلاإذا defined(MADV_FREE)
    إذا (madvise(پتہ_2, ناپ_2, MADV_FREE)) {
#وإلا
    إذا (posix_madvise(پتہ_2, ناپ_2, POSIX_MADV_DONTNEED)) {
#جے_انت
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#جے_انت
#جے_انت
  إذا (release)
    _rpmalloc_stat_sub(&mapped_ورقے_os, release >> حافظہ_ورقہ_ناپ_کھسکاؤ);
}

ساكن فراغ rpmalloc_وسعت_mark_as_subspan_unless_master(وسعت_t *master,
                                                         وسعت_t *subspan,
                                                         size_t وسعت_گنتی);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
ساكن وسعت_t *_rpmalloc_global_get_reserved_spans(size_t وسعت_گنتی) {
  وسعت_t *وسعت = حافظہ_global_reserve;
  rpmalloc_وسعت_mark_as_subspan_unless_master(حافظہ_global_reserve_master,
                                               وسعت, وسعت_گنتی);
  حافظہ_global_reserve_گنتی -= وسعت_گنتی;
  إذا (حافظہ_global_reserve_گنتی)
    حافظہ_global_reserve =
        (وسعت_t *)اشارہ_گر_وتھ(وسعت, وسعت_گنتی << حافظہ_وسعت_ناپ_کھسکاؤ);
  وإلا
    حافظہ_global_reserve = 0;
  أرجع وسعت;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
ساكن فراغ rpmalloc_global_سیٹ_reserved_spans(وسعت_t *master, وسعت_t *reserve,
                                                size_t reserve_وسعت_گنتی) {
  حافظہ_global_reserve_master = master;
  حافظہ_global_reserve_گنتی = reserve_وسعت_گنتی;
  حافظہ_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
ساكن فراغ rpmalloc_وسعت_double_link_list_جوڑو(وسعت_t **سر, وسعت_t *وسعت) {
  إذا (*سر)
    (*سر)->prev = وسعت;
  وسعت->اگلا = *سر;
  *سر = وسعت;
}

//! Pop head span from double linked list
ساكن فراغ rpmalloc_وسعت_double_link_list_نکالو_سر(وسعت_t **سر,
                                                     وسعت_t *وسعت) {
  rpmalloc_assert(*سر == وسعت, "Linked list corrupted");
  وسعت = *سر;
  *سر = وسعت->اگلا;
}

//! Remove a span from double linked list
ساكن فراغ rpmalloc_وسعت_double_link_list_remove(وسعت_t **سر,
                                                   وسعت_t *وسعت) {
  rpmalloc_assert(*سر, "Linked list corrupted");
  إذا (*سر == وسعت) {
    *سر = وسعت->اگلا;
  } وإلا {
    وسعت_t *اگلا_وسعت = وسعت->اگلا;
    وسعت_t *prev_وسعت = وسعت->prev;
    prev_وسعت->اگلا = اگلا_وسعت;
    إذا (EXPECTED(اگلا_وسعت != 0))
      اگلا_وسعت->prev = prev_وسعت;
  }
}

////////////
///
/// Span control
///
//////

ساكن فراغ rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_t *ڈھیر_2, وسعت_t *وسعت);

ساكن فراغ rpmalloc_ڈھیر_finalize(ڈھیر_t *ڈھیر_2);

ساكن فراغ rpmalloc_ڈھیر_سیٹ_reserved_spans(ڈھیر_t *ڈھیر_2, وسعت_t *master,
                                              وسعت_t *reserve,
                                              size_t reserve_وسعت_گنتی);

//! Declare the span to be a subspan and store distance from master span and
//! span count
ساكن فراغ rpmalloc_وسعت_mark_as_subspan_unless_master(وسعت_t *master,
                                                         وسعت_t *subspan,
                                                         size_t وسعت_گنتی) {
  rpmalloc_assert((subspan != master) || (subspan->flags & وسعت_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  إذا (subspan != master) {
    subspan->flags = وسعت_FLAG_SUBSPAN;
    subspan->وتھ_from_master =
        (uint32_t)((uintptr_t)اشارہ_گر_diff(subspan, master) >>
                   حافظہ_وسعت_ناپ_کھسکاؤ);
    subspan->align_وتھ = 0;
  }
  subspan->وسعت_گنتی = (uint32_t)وسعت_گنتی;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
ساكن وسعت_t *rpmalloc_وسعت_map_from_reserve(ڈھیر_t *ڈھیر_2,
                                               size_t وسعت_گنتی) {
  // Update the heap span reserve
  وسعت_t *وسعت = ڈھیر_2->وسعت_reserve;
  ڈھیر_2->وسعت_reserve =
      (وسعت_t *)اشارہ_گر_وتھ(وسعت, وسعت_گنتی * حافظہ_وسعت_ناپ);
  ڈھیر_2->spans_reserved -= (uint32_t)وسعت_گنتی;

  rpmalloc_وسعت_mark_as_subspan_unless_master(ڈھیر_2->وسعت_reserve_master, وسعت,
                                               وسعت_گنتی);
  إذا (وسعت_گنتی <= LARGE_CLASS_گنتی)
    _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_from_reserved);

  أرجع وسعت;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
ساكن size_t rpmalloc_وسعت_align_گنتی(size_t وسعت_گنتی) {
  size_t request_گنتی = (وسعت_گنتی > حافظہ_وسعت_map_گنتی)
                             ? وسعت_گنتی
                             : حافظہ_وسعت_map_گنتی;
  إذا ((حافظہ_ورقہ_ناپ > حافظہ_وسعت_ناپ) &&
      ((request_گنتی * حافظہ_وسعت_ناپ) % حافظہ_ورقہ_ناپ))
    request_گنتی +=
        حافظہ_وسعت_map_گنتی - (request_گنتی % حافظہ_وسعت_map_گنتی);
  أرجع request_گنتی;
}

//! Setup a newly mapped span
ساكن فراغ rpmalloc_وسعت_initialize(وسعت_t *وسعت, size_t کل_وسعت_گنتی,
                                      size_t وسعت_گنتی, size_t align_وتھ) {
  وسعت->کل_spans = (uint32_t)کل_وسعت_گنتی;
  وسعت->وسعت_گنتی = (uint32_t)وسعت_گنتی;
  وسعت->align_وتھ = (uint32_t)align_وتھ;
  وسعت->flags = وسعت_FLAG_MASTER;
  atomic_store32(&وسعت->بچیا_spans, (int32_t)کل_وسعت_گنتی);
}

ساكن فراغ rpmalloc_وسعت_unmap(وسعت_t *وسعت);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
ساكن وسعت_t *rpmalloc_وسعت_map_aligned_گنتی(ڈھیر_t *ڈھیر_2,
                                                size_t وسعت_گنتی) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_وسعت_گنتی = rpmalloc_وسعت_align_گنتی(وسعت_گنتی);
  size_t align_وتھ = 0;
  وسعت_t *وسعت = (وسعت_t *)_rpmalloc_mmap(
      aligned_وسعت_گنتی * حافظہ_وسعت_ناپ, &align_وتھ);
  إذا (!وسعت)
    أرجع 0;
  rpmalloc_وسعت_initialize(وسعت, aligned_وسعت_گنتی, وسعت_گنتی, align_وتھ);
  _rpmalloc_stat_inc(&_master_spans);
  إذا (وسعت_گنتی <= LARGE_CLASS_گنتی)
    _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_map_calls);
  إذا (aligned_وسعت_گنتی > وسعت_گنتی) {
    وسعت_t *reserved_spans =
        (وسعت_t *)اشارہ_گر_وتھ(وسعت, وسعت_گنتی * حافظہ_وسعت_ناپ);
    size_t reserved_گنتی = aligned_وسعت_گنتی - وسعت_گنتی;
    إذا (ڈھیر_2->spans_reserved) {
      rpmalloc_وسعت_mark_as_subspan_unless_master(
          ڈھیر_2->وسعت_reserve_master, ڈھیر_2->وسعت_reserve, ڈھیر_2->spans_reserved);
      rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, ڈھیر_2->وسعت_reserve);
    }
    إذا (reserved_گنتی > حافظہ_ڈھیر_reserve_گنتی) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&حافظہ_global_جندرا) == 1,
                      "Global spin lock not held as expected");
      size_t remain_گنتی = reserved_گنتی - حافظہ_ڈھیر_reserve_گنتی;
      reserved_گنتی = حافظہ_ڈھیر_reserve_گنتی;
      وسعت_t *remain_وسعت = (وسعت_t *)اشارہ_گر_وتھ(
          reserved_spans, reserved_گنتی * حافظہ_وسعت_ناپ);
      إذا (حافظہ_global_reserve) {
        rpmalloc_وسعت_mark_as_subspan_unless_master(
            حافظہ_global_reserve_master, حافظہ_global_reserve,
            حافظہ_global_reserve_گنتی);
        rpmalloc_وسعت_unmap(حافظہ_global_reserve);
      }
      rpmalloc_global_سیٹ_reserved_spans(وسعت, remain_وسعت, remain_گنتی);
    }
    rpmalloc_ڈھیر_سیٹ_reserved_spans(ڈھیر_2, وسعت, reserved_spans,
                                      reserved_گنتی);
  }
  أرجع وسعت;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
ساكن وسعت_t *rpmalloc_وسعت_map(ڈھیر_t *ڈھیر_2, size_t وسعت_گنتی) {
  إذا (وسعت_گنتی <= ڈھیر_2->spans_reserved)
    أرجع rpmalloc_وسعت_map_from_reserve(ڈھیر_2, وسعت_گنتی);
  وسعت_t *وسعت = 0;
  صحيح use_global_reserve =
      (حافظہ_ورقہ_ناپ > حافظہ_وسعت_ناپ) ||
      (حافظہ_وسعت_map_گنتی > حافظہ_ڈھیر_reserve_گنتی);
  إذا (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    طالما (!atomic_cas32_acquire(&حافظہ_global_جندرا, 1, 0))
      _rpmalloc_spin();
    إذا (حافظہ_global_reserve_گنتی >= وسعت_گنتی) {
      size_t reserve_گنتی =
          (!ڈھیر_2->spans_reserved ? حافظہ_ڈھیر_reserve_گنتی : وسعت_گنتی);
      إذا (حافظہ_global_reserve_گنتی < reserve_گنتی)
        reserve_گنتی = حافظہ_global_reserve_گنتی;
      وسعت = _rpmalloc_global_get_reserved_spans(reserve_گنتی);
      إذا (وسعت) {
        إذا (reserve_گنتی > وسعت_گنتی) {
          وسعت_t *reserved_وسعت = (وسعت_t *)اشارہ_گر_وتھ(
              وسعت, وسعت_گنتی << حافظہ_وسعت_ناپ_کھسکاؤ);
          rpmalloc_ڈھیر_سیٹ_reserved_spans(ڈھیر_2, حافظہ_global_reserve_master,
                                            reserved_وسعت,
                                            reserve_گنتی - وسعت_گنتی);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        وسعت->وسعت_گنتی = (uint32_t)وسعت_گنتی;
      }
    }
  }
  إذا (!وسعت)
    وسعت = rpmalloc_وسعت_map_aligned_گنتی(ڈھیر_2, وسعت_گنتی);
  إذا (use_global_reserve)
    atomic_store32_release(&حافظہ_global_جندرا, 0);
  أرجع وسعت;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
ساكن فراغ rpmalloc_وسعت_unmap(وسعت_t *وسعت) {
  rpmalloc_assert((وسعت->flags & وسعت_FLAG_MASTER) ||
                      (وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(وسعت->flags & وسعت_FLAG_MASTER) ||
                      !(وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");

  صحيح is_master = !!(وسعت->flags & وسعت_FLAG_MASTER);
  وسعت_t *master =
      is_master ? وسعت
                : ((وسعت_t *)اشارہ_گر_وتھ(
                      وسعت, -(intptr_t)((uintptr_t)وسعت->وتھ_from_master *
                                        حافظہ_وسعت_ناپ)));
  rpmalloc_assert(is_master || (وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & وسعت_FLAG_MASTER, "Span flag corrupted");

  size_t وسعت_گنتی = وسعت->وسعت_گنتی;
  إذا (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(وسعت->align_وتھ == 0, "Span align offset corrupted");
    إذا (حافظہ_وسعت_ناپ >= حافظہ_ورقہ_ناپ)
      _rpmalloc_unmap(وسعت, وسعت_گنتی * حافظہ_وسعت_ناپ, 0, 0);
  } وإلا {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    وسعت->flags |=
        وسعت_FLAG_MASTER | وسعت_FLAG_SUBSPAN | وسعت_FLAG_UNMAPPED_MASTER;
    rpmalloc_stat_جوڑو(&_unmapped_master_spans, 1);
  }

  إذا (atomic_add32(&master->بچیا_spans, -(int32_t)وسعت_گنتی) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & وسعت_FLAG_MASTER) &&
                        !!(master->flags & وسعت_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_گنتی = master->وسعت_گنتی;
    إذا (حافظہ_وسعت_ناپ < حافظہ_ورقہ_ناپ)
      unmap_گنتی = master->کل_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_گنتی * حافظہ_وسعت_ناپ,
                    master->align_وتھ,
                    (size_t)master->کل_spans * حافظہ_وسعت_ناپ);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
ساكن فراغ rpmalloc_وسعت_release_منزل_cache(ڈھیر_t *ڈھیر_2, وسعت_t *وسعت) {
  rpmalloc_assert(ڈھیر_2 == وسعت->ڈھیر_2, "Span heap pointer corrupted");
  rpmalloc_assert(وسعت->ناپ_class < ناپ_CLASS_گنتی,
                  "Invalid span size class");
  rpmalloc_assert(وسعت->وسعت_گنتی == 1, "Invalid span count");
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&ڈھیر_2->وسعت_use[0].current);
#جے_انت
  _rpmalloc_stat_dec(&ڈھیر_2->ناپ_class_use[وسعت->ناپ_class].spans_current);
  إذا (!ڈھیر_2->finalize) {
    _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[0].spans_منزل_cache);
    _rpmalloc_stat_inc(&ڈھیر_2->ناپ_class_use[وسعت->ناپ_class].spans_منزل_cache);
    إذا (ڈھیر_2->ناپ_class[وسعت->ناپ_class].cache)
      rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2,
                                  ڈھیر_2->ناپ_class[وسعت->ناپ_class].cache);
    ڈھیر_2->ناپ_class[وسعت->ناپ_class].cache = وسعت;
  } وإلا {
    rpmalloc_وسعت_unmap(وسعت);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
ساكن uint32_t free_list_partial_init(فراغ **list, فراغ **پہلا_بلاک,
                                       فراغ *ورقہ_شروع, فراغ *بلاک_شروع,
                                       uint32_t بلاک_گنتی,
                                       uint32_t بلاک_ناپ_2) {
  rpmalloc_assert(بلاک_گنتی, "Internal failure");
  *پہلا_بلاک = بلاک_شروع;
  إذا (بلاک_گنتی > 1) {
    فراغ *free_بلاک = اشارہ_گر_وتھ(بلاک_شروع, بلاک_ناپ_2);
    فراغ *بلاک_سرا =
        اشارہ_گر_وتھ(بلاک_شروع, (size_t)بلاک_ناپ_2 * بلاک_گنتی);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    إذا (بلاک_ناپ_2 < (حافظہ_ورقہ_ناپ >> 1)) {
      فراغ *ورقہ_سرا = اشارہ_گر_وتھ(ورقہ_شروع, حافظہ_ورقہ_ناپ);
      إذا (ورقہ_سرا < بلاک_سرا)
        بلاک_سرا = ورقہ_سرا;
    }
    *list = free_بلاک;
    بلاک_گنتی = 2;
    فراغ *اگلا_بلاک = اشارہ_گر_وتھ(free_بلاک, بلاک_ناپ_2);
    طالما (اگلا_بلاک < بلاک_سرا) {
      *((فراغ **)free_بلاک) = اگلا_بلاک;
      free_بلاک = اگلا_بلاک;
      ++بلاک_گنتی;
      اگلا_بلاک = اشارہ_گر_وتھ(اگلا_بلاک, بلاک_ناپ_2);
    }
    *((فراغ **)free_بلاک) = 0;
  } وإلا {
    *list = 0;
  }
  أرجع بلاک_گنتی;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
ساكن فراغ *rpmalloc_وسعت_initialize_new(ڈھیر_t *ڈھیر_2,
                                           ڈھیر_ناپ_class_t *ڈھیر_ناپ_class,
                                           وسعت_t *وسعت, uint32_t class_idx) {
  rpmalloc_assert(وسعت->وسعت_گنتی == 1, "Internal failure");
  ناپ_class_t *ناپ_class = حافظہ_ناپ_class + class_idx;
  وسعت->ناپ_class = class_idx;
  وسعت->ڈھیر_2 = ڈھیر_2;
  وسعت->flags &= ~وسعت_FLAG_ALIGNED_بلاکاں;
  وسعت->بلاک_ناپ_2 = ناپ_class->بلاک_ناپ_2;
  وسعت->بلاک_گنتی = ناپ_class->بلاک_گنتی;
  وسعت->free_list = 0;
  وسعت->list_ناپ = 0;
  atomic_store_ptr_release(&وسعت->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  فراغ *بلاک;
  وسعت->free_list_حد_بندی =
      free_list_partial_init(&ڈھیر_ناپ_class->free_list, &بلاک, وسعت,
                             اشارہ_گر_وتھ(وسعت, وسعت_سرخی_ناپ),
                             ناپ_class->بلاک_گنتی, ناپ_class->بلاک_ناپ_2);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  إذا (وسعت->free_list_حد_بندی < وسعت->بلاک_گنتی) {
    rpmalloc_وسعت_double_link_list_جوڑو(&ڈھیر_ناپ_class->partial_وسعت, وسعت);
    وسعت->ورتیا_گنتی = وسعت->free_list_حد_بندی;
  } وإلا {
#إذا RPMALLOC_پہلا_CLASS_HEAPS
    rpmalloc_وسعت_double_link_list_جوڑو(&ڈھیر_2->بھریا_وسعت[class_idx], وسعت);
#جے_انت
    ++ڈھیر_2->بھریا_وسعت_گنتی;
    وسعت->ورتیا_گنتی = وسعت->بلاک_گنتی;
  }
  أرجع بلاک;
}

ساكن فراغ rpmalloc_وسعت_کڈھو_free_list_deferred(وسعت_t *وسعت) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  افعل {
    وسعت->free_list =
        atomic_exchange_ptr_acquire(&وسعت->free_list_deferred, INVALID_اشارہ_گر);
  } طالما (وسعت->free_list == INVALID_اشارہ_گر);
  وسعت->ورتیا_گنتی -= وسعت->list_ناپ;
  وسعت->list_ناپ = 0;
  atomic_store_ptr_release(&وسعت->free_list_deferred, 0);
}

ساكن صحيح rpmalloc_وسعت_is_fully_utilized(وسعت_t *وسعت) {
  rpmalloc_assert(وسعت->free_list_حد_بندی <= وسعت->بلاک_گنتی,
                  "Span free list corrupted");
  أرجع !وسعت->free_list && (وسعت->free_list_حد_بندی >= وسعت->بلاک_گنتی);
}

ساكن صحيح rpmalloc_وسعت_finalize(ڈھیر_t *ڈھیر_2, size_t iclass, وسعت_t *وسعت,
                                   وسعت_t **list_سر) {
  فراغ *free_list = ڈھیر_2->ناپ_class[iclass].free_list;
  وسعت_t *class_وسعت = (وسعت_t *)((uintptr_t)free_list & حافظہ_وسعت_نقاب);
  إذا (وسعت == class_وسعت) {
    // Adopt the heap class free list back into the span free list
    فراغ *بلاک = وسعت->free_list;
    فراغ *آخری_بلاک = 0;
    طالما (بلاک) {
      آخری_بلاک = بلاک;
      بلاک = *((فراغ **)بلاک);
    }
    uint32_t free_گنتی = 0;
    بلاک = free_list;
    طالما (بلاک) {
      ++free_گنتی;
      بلاک = *((فراغ **)بلاک);
    }
    إذا (آخری_بلاک) {
      *((فراغ **)آخری_بلاک) = free_list;
    } وإلا {
      وسعت->free_list = free_list;
    }
    ڈھیر_2->ناپ_class[iclass].free_list = 0;
    وسعت->ورتیا_گنتی -= free_گنتی;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(وسعت->list_ناپ == وسعت->ورتیا_گنتی, "Memory leak detected");
  إذا (وسعت->list_ناپ == وسعت->ورتیا_گنتی) {
    _rpmalloc_stat_dec(&ڈھیر_2->وسعت_use[0].current);
    _rpmalloc_stat_dec(&ڈھیر_2->ناپ_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    إذا (list_سر)
      rpmalloc_وسعت_double_link_list_remove(list_سر, وسعت);
    rpmalloc_وسعت_unmap(وسعت);
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
  طالما (!atomic_cas32_acquire(&cache->جندرا, 1, 0))
    _rpmalloc_spin();

  لكل (size_t ispan = 0; ispan < cache->گنتی; ++ispan)
    rpmalloc_وسعت_unmap(cache->وسعت[ispan]);
  cache->گنتی = 0;

  طالما (cache->حد_توں_ودھ_2) {
    وسعت_t *وسعت = cache->حد_توں_ودھ_2;
    cache->حد_توں_ودھ_2 = وسعت->اگلا;
    rpmalloc_وسعت_unmap(وسعت);
  }

  atomic_store32_release(&cache->جندرا, 0);
}

ساكن فراغ rpmalloc_global_cache_داخل_کرو_spans(وسعت_t **وسعت,
                                                size_t وسعت_گنتی,
                                                size_t گنتی) {
  ثابت size_t cache_حد_بندی =
      (وسعت_گنتی == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_وسعت_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_وسعت_LARGE_CACHE - (وسعت_گنتی >> 1));

  global_cache_t *cache = &حافظہ_وسعت_cache[وسعت_گنتی - 1];

  size_t داخل_کرو_گنتی = گنتی;
  طالما (!atomic_cas32_acquire(&cache->جندرا, 1, 0))
    _rpmalloc_spin();

#إذا ENABLE_STATISTICS
  cache->داخل_کرو_گنتی += گنتی;
#جے_انت
  إذا ((cache->گنتی + داخل_کرو_گنتی) > cache_حد_بندی)
    داخل_کرو_گنتی = cache_حد_بندی - cache->گنتی;

  حافظہ_نقل(cache->وسعت + cache->گنتی, وسعت, حجم(وسعت_t *) * داخل_کرو_گنتی);
  cache->گنتی += (uint32_t)داخل_کرو_گنتی;

#إذا ENABLE_UNLIMITED_CACHE
  طالما (داخل_کرو_گنتی < گنتی) {
#وإلا
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  طالما ((حافظہ_ورقہ_ناپ > حافظہ_وسعت_ناپ) && (داخل_کرو_گنتی < گنتی)) {
#جے_انت
    وسعت_t *current_وسعت = وسعت[داخل_کرو_گنتی++];
    current_وسعت->اگلا = cache->حد_توں_ودھ_2;
    cache->حد_توں_ودھ_2 = current_وسعت;
  }
  atomic_store32_release(&cache->جندرا, 0);

  وسعت_t *keep = 0;
  لكل (size_t ispan = داخل_کرو_گنتی; ispan < گنتی; ++ispan) {
    وسعت_t *current_وسعت = وسعت[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    إذا ((current_وسعت->flags & وسعت_FLAG_MASTER) &&
        (atomic_load32(&current_وسعت->بچیا_spans) >
         (int32_t)current_وسعت->وسعت_گنتی)) {
      current_وسعت->اگلا = keep;
      keep = current_وسعت;
    } وإلا {
      rpmalloc_وسعت_unmap(current_وسعت);
    }
  }

  إذا (keep) {
    طالما (!atomic_cas32_acquire(&cache->جندرا, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    طالما (keep) {
      لكل (; islot < cache->گنتی; ++islot) {
        وسعت_t *current_وسعت = cache->وسعت[islot];
        إذا (!(current_وسعت->flags & وسعت_FLAG_MASTER) ||
            ((current_وسعت->flags & وسعت_FLAG_MASTER) &&
             (atomic_load32(&current_وسعت->بچیا_spans) <=
              (int32_t)current_وسعت->وسعت_گنتی))) {
          rpmalloc_وسعت_unmap(current_وسعت);
          cache->وسعت[islot] = keep;
          اكسر;
        }
      }
      إذا (islot == cache->گنتی)
        اكسر;
      keep = keep->اگلا;
    }

    إذا (keep) {
      وسعت_t *پوچھ_2 = keep;
      طالما (پوچھ_2->اگلا)
        پوچھ_2 = پوچھ_2->اگلا;
      پوچھ_2->اگلا = cache->حد_توں_ودھ_2;
      cache->حد_توں_ودھ_2 = keep;
    }

    atomic_store32_release(&cache->جندرا, 0);
  }
}

ساكن size_t rpmalloc_global_cache_کڈھو_spans(وسعت_t **وسعت,
                                                   size_t وسعت_گنتی,
                                                   size_t گنتی) {
  global_cache_t *cache = &حافظہ_وسعت_cache[وسعت_گنتی - 1];

  size_t کڈھو_گنتی = 0;
  طالما (!atomic_cas32_acquire(&cache->جندرا, 1, 0))
    _rpmalloc_spin();

#إذا ENABLE_STATISTICS
  cache->کڈھو_گنتی += گنتی;
#جے_انت
  size_t want = گنتی - کڈھو_گنتی;
  إذا (want > cache->گنتی)
    want = cache->گنتی;

  حافظہ_نقل(وسعت + کڈھو_گنتی, cache->وسعت + (cache->گنتی - want),
         حجم(وسعت_t *) * want);
  cache->گنتی -= (uint32_t)want;
  کڈھو_گنتی += want;

  طالما ((کڈھو_گنتی < گنتی) && cache->حد_توں_ودھ_2) {
    وسعت_t *current_وسعت = cache->حد_توں_ودھ_2;
    وسعت[کڈھو_گنتی++] = current_وسعت;
    cache->حد_توں_ودھ_2 = current_وسعت->اگلا;
  }

#إذا ENABLE_ASSERTS
  لكل (size_t ispan = 0; ispan < کڈھو_گنتی; ++ispan) {
    rpmalloc_assert(وسعت[ispan]->وسعت_گنتی == وسعت_گنتی,
                    "Global cache span count mismatch");
  }
#جے_انت

  atomic_store32_release(&cache->جندرا, 0);

  أرجع کڈھو_گنتی;
}

#جے_انت

////////////
///
/// Heap control
///
//////

ساكن فراغ _rpmalloc_deallocate_huge(وسعت_t *);

//! Store the given spans as reserve in the given heap
ساكن فراغ rpmalloc_ڈھیر_سیٹ_reserved_spans(ڈھیر_t *ڈھیر_2, وسعت_t *master,
                                              وسعت_t *reserve,
                                              size_t reserve_وسعت_گنتی) {
  ڈھیر_2->وسعت_reserve_master = master;
  ڈھیر_2->وسعت_reserve = reserve;
  ڈھیر_2->spans_reserved = (uint32_t)reserve_وسعت_گنتی;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
ساكن فراغ rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_t *ڈھیر_2,
                                                وسعت_t **single_وسعت) {
  وسعت_t *وسعت = (وسعت_t *)((فراغ *)atomic_exchange_ptr_acquire(
      &ڈھیر_2->وسعت_free_deferred, 0));
  طالما (وسعت) {
    وسعت_t *اگلا_وسعت = (وسعت_t *)وسعت->free_list;
    rpmalloc_assert(وسعت->ڈھیر_2 == ڈھیر_2, "Span heap pointer corrupted");
    إذا (EXPECTED(وسعت->ناپ_class < ناپ_CLASS_گنتی)) {
      rpmalloc_assert(ڈھیر_2->بھریا_وسعت_گنتی, "Heap span counter corrupted");
      --ڈھیر_2->بھریا_وسعت_گنتی;
      _rpmalloc_stat_dec(&ڈھیر_2->وسعت_use[0].spans_deferred);
#إذا RPMALLOC_پہلا_CLASS_HEAPS
      rpmalloc_وسعت_double_link_list_remove(&ڈھیر_2->بھریا_وسعت[وسعت->ناپ_class],
                                             وسعت);
#جے_انت
      _rpmalloc_stat_dec(&ڈھیر_2->وسعت_use[0].current);
      _rpmalloc_stat_dec(&ڈھیر_2->ناپ_class_use[وسعت->ناپ_class].spans_current);
      إذا (single_وسعت && !*single_وسعت)
        *single_وسعت = وسعت;
      وإلا
        rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
    } وإلا {
      إذا (وسعت->ناپ_class == ناپ_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(وسعت);
      } وإلا {
        rpmalloc_assert(وسعت->ناپ_class == ناپ_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(ڈھیر_2->بھریا_وسعت_گنتی, "Heap span counter corrupted");
        --ڈھیر_2->بھریا_وسعت_گنتی;
#إذا RPMALLOC_پہلا_CLASS_HEAPS
        rpmalloc_وسعت_double_link_list_remove(&ڈھیر_2->large_huge_وسعت, وسعت);
#جے_انت
        uint32_t idx = وسعت->وسعت_گنتی - 1;
        _rpmalloc_stat_dec(&ڈھیر_2->وسعت_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&ڈھیر_2->وسعت_use[idx].current);
        إذا (!idx && single_وسعت && !*single_وسعت)
          *single_وسعت = وسعت;
        وإلا
          rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
      }
    }
    وسعت = اگلا_وسعت;
  }
}

ساكن فراغ rpmalloc_ڈھیر_unmap(ڈھیر_t *ڈھیر_2) {
  إذا (!ڈھیر_2->master_ڈھیر) {
    إذا ((ڈھیر_2->finalize > 1) && !atomic_load32(&ڈھیر_2->بچہ_گنتی)) {
      وسعت_t *وسعت = (وسعت_t *)((uintptr_t)ڈھیر_2 & حافظہ_وسعت_نقاب);
      rpmalloc_وسعت_unmap(وسعت);
    }
  } وإلا {
    إذا (atomic_decr32(&ڈھیر_2->master_ڈھیر->بچہ_گنتی) == 0) {
      rpmalloc_ڈھیر_unmap(ڈھیر_2->master_ڈھیر);
    }
  }
}

ساكن فراغ rpmalloc_ڈھیر_global_finalize(ڈھیر_t *ڈھیر_2) {
  إذا (ڈھیر_2->finalize++ > 1) {
    --ڈھیر_2->finalize;
    أرجع;
  }

  rpmalloc_ڈھیر_finalize(ڈھیر_2);

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    وسعت_cache_t *وسعت_cache;
    إذا (!iclass)
      وسعت_cache = &ڈھیر_2->وسعت_cache;
    وإلا
      وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (iclass - 1));
    لكل (size_t ispan = 0; ispan < وسعت_cache->گنتی; ++ispan)
      rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
    وسعت_cache->گنتی = 0;
  }
#جے_انت

  إذا (ڈھیر_2->بھریا_وسعت_گنتی) {
    --ڈھیر_2->finalize;
    أرجع;
  }

  لكل (size_t iclass = 0; iclass < ناپ_CLASS_گنتی; ++iclass) {
    إذا (ڈھیر_2->ناپ_class[iclass].free_list ||
        ڈھیر_2->ناپ_class[iclass].partial_وسعت) {
      --ڈھیر_2->finalize;
      أرجع;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)ڈھیر_2->id % ڈھیر_ARRAY_ناپ;
  ڈھیر_t *list_ڈھیر = حافظہ_heaps[list_idx];
  إذا (list_ڈھیر == ڈھیر_2) {
    حافظہ_heaps[list_idx] = ڈھیر_2->اگلا_ڈھیر;
  } وإلا {
    طالما (list_ڈھیر->اگلا_ڈھیر != ڈھیر_2)
      list_ڈھیر = list_ڈھیر->اگلا_ڈھیر;
    list_ڈھیر->اگلا_ڈھیر = ڈھیر_2->اگلا_ڈھیر;
  }

  rpmalloc_ڈھیر_unmap(ڈھیر_2);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
ساكن فراغ rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_t *ڈھیر_2, وسعت_t *وسعت) {
  إذا (UNEXPECTED(ڈھیر_2->finalize != 0)) {
    rpmalloc_وسعت_unmap(وسعت);
    rpmalloc_ڈھیر_global_finalize(ڈھیر_2);
    أرجع;
  }
#إذا ENABLE_THREAD_CACHE
  size_t وسعت_گنتی = وسعت->وسعت_گنتی;
  _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_منزل_cache);
  إذا (وسعت_گنتی == 1) {
    وسعت_cache_t *وسعت_cache = &ڈھیر_2->وسعت_cache;
    وسعت_cache->وسعت[وسعت_cache->گنتی++] = وسعت;
    إذا (وسعت_cache->گنتی == MAX_THREAD_وسعت_CACHE) {
      ثابت size_t remain_گنتی =
          MAX_THREAD_وسعت_CACHE - THREAD_وسعت_CACHE_TRANSFER;
#إذا ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&ڈھیر_2->thread_منزل_global,
                           THREAD_وسعت_CACHE_TRANSFER * حافظہ_وسعت_ناپ);
      rpmalloc_stat_جوڑو(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_منزل_global,
                         THREAD_وسعت_CACHE_TRANSFER);
      rpmalloc_global_cache_داخل_کرو_spans(وسعت_cache->وسعت + remain_گنتی,
                                          وسعت_گنتی,
                                          THREAD_وسعت_CACHE_TRANSFER);
#وإلا
      لكل (size_t ispan = 0; ispan < THREAD_وسعت_CACHE_TRANSFER; ++ispan)
        rpmalloc_وسعت_unmap(وسعت_cache->وسعت[remain_گنتی + ispan]);
#جے_انت
      وسعت_cache->گنتی = remain_گنتی;
    }
  } وإلا {
    size_t cache_idx = وسعت_گنتی - 2;
    وسعت_large_cache_t *وسعت_cache = ڈھیر_2->وسعت_large_cache + cache_idx;
    وسعت_cache->وسعت[وسعت_cache->گنتی++] = وسعت;
    ثابت size_t cache_حد_بندی =
        (MAX_THREAD_وسعت_LARGE_CACHE - (وسعت_گنتی >> 1));
    إذا (وسعت_cache->گنتی == cache_حد_بندی) {
      ثابت size_t transfer_حد_بندی = 2 + (cache_حد_بندی >> 2);
      ثابت size_t transfer_گنتی =
          (THREAD_وسعت_LARGE_CACHE_TRANSFER <= transfer_حد_بندی
               ? THREAD_وسعت_LARGE_CACHE_TRANSFER
               : transfer_حد_بندی);
      ثابت size_t remain_گنتی = cache_حد_بندی - transfer_گنتی;
#إذا ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&ڈھیر_2->thread_منزل_global,
                           transfer_گنتی * وسعت_گنتی * حافظہ_وسعت_ناپ);
      rpmalloc_stat_جوڑو(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_منزل_global,
                         transfer_گنتی);
      rpmalloc_global_cache_داخل_کرو_spans(وسعت_cache->وسعت + remain_گنتی,
                                          وسعت_گنتی, transfer_گنتی);
#وإلا
      لكل (size_t ispan = 0; ispan < transfer_گنتی; ++ispan)
        rpmalloc_وسعت_unmap(وسعت_cache->وسعت[remain_گنتی + ispan]);
#جے_انت
      وسعت_cache->گنتی = remain_گنتی;
    }
  }
#وإلا
  (فراغ)حجم(ڈھیر_2);
  rpmalloc_وسعت_unmap(وسعت);
#جے_انت
}

//! Extract the given number of spans from the different cache levels
ساكن وسعت_t *rpmalloc_ڈھیر_thread_cache_کڈھو(ڈھیر_t *ڈھیر_2,
                                                   size_t وسعت_گنتی) {
  وسعت_t *وسعت = 0;
#إذا ENABLE_THREAD_CACHE
  وسعت_cache_t *وسعت_cache;
  إذا (وسعت_گنتی == 1)
    وسعت_cache = &ڈھیر_2->وسعت_cache;
  وإلا
    وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (وسعت_گنتی - 2));
  إذا (وسعت_cache->گنتی) {
    _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_from_cache);
    أرجع وسعت_cache->وسعت[--وسعت_cache->گنتی];
  }
#جے_انت
  أرجع وسعت;
}

ساكن وسعت_t *rpmalloc_ڈھیر_thread_cache_deferred_کڈھو(ڈھیر_t *ڈھیر_2,
                                                            size_t وسعت_گنتی) {
  وسعت_t *وسعت = 0;
  إذا (وسعت_گنتی == 1) {
    rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, &وسعت);
  } وإلا {
    rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, 0);
    وسعت = rpmalloc_ڈھیر_thread_cache_کڈھو(ڈھیر_2, وسعت_گنتی);
  }
  أرجع وسعت;
}

ساكن وسعت_t *rpmalloc_ڈھیر_reserved_کڈھو(ڈھیر_t *ڈھیر_2,
                                               size_t وسعت_گنتی) {
  إذا (ڈھیر_2->spans_reserved >= وسعت_گنتی)
    أرجع rpmalloc_وسعت_map(ڈھیر_2, وسعت_گنتی);
  أرجع 0;
}

//! Extract a span from the global cache
ساكن وسعت_t *rpmalloc_ڈھیر_global_cache_کڈھو(ڈھیر_t *ڈھیر_2,
                                                   size_t وسعت_گنتی) {
#إذا ENABLE_GLOBAL_CACHE
#إذا ENABLE_THREAD_CACHE
  وسعت_cache_t *وسعت_cache;
  size_t wanted_گنتی;
  إذا (وسعت_گنتی == 1) {
    وسعت_cache = &ڈھیر_2->وسعت_cache;
    wanted_گنتی = THREAD_وسعت_CACHE_TRANSFER;
  } وإلا {
    وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (وسعت_گنتی - 2));
    wanted_گنتی = THREAD_وسعت_LARGE_CACHE_TRANSFER;
  }
  وسعت_cache->گنتی = rpmalloc_global_cache_کڈھو_spans(
      وسعت_cache->وسعت, وسعت_گنتی, wanted_گنتی);
  إذا (وسعت_cache->گنتی) {
    _rpmalloc_stat_add64(&ڈھیر_2->global_منزل_thread,
                         وسعت_گنتی * وسعت_cache->گنتی * حافظہ_وسعت_ناپ);
    rpmalloc_stat_جوڑو(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_from_global,
                       وسعت_cache->گنتی);
    أرجع وسعت_cache->وسعت[--وسعت_cache->گنتی];
  }
#وإلا
  وسعت_t *وسعت = 0;
  size_t گنتی = rpmalloc_global_cache_کڈھو_spans(&وسعت, وسعت_گنتی, 1);
  إذا (گنتی) {
    _rpmalloc_stat_add64(&ڈھیر_2->global_منزل_thread,
                         وسعت_گنتی * گنتی * حافظہ_وسعت_ناپ);
    rpmalloc_stat_جوڑو(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_from_global,
                       گنتی);
    أرجع وسعت;
  }
#جے_انت
#جے_انت
  (فراغ)حجم(ڈھیر_2);
  (فراغ)حجم(وسعت_گنتی);
  أرجع 0;
}

ساكن فراغ rpmalloc_inc_وسعت_statistics(ڈھیر_t *ڈھیر_2, size_t وسعت_گنتی,
                                          uint32_t class_idx) {
  (فراغ)حجم(ڈھیر_2);
  (فراغ)حجم(وسعت_گنتی);
  (فراغ)حجم(class_idx);
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)وسعت_گنتی - 1;
  uint32_t current_گنتی =
      (uint32_t)atomic_incr32(&ڈھیر_2->وسعت_use[idx].current);
  إذا (current_گنتی > (uint32_t)atomic_load32(&ڈھیر_2->وسعت_use[idx].بلند))
    atomic_store32(&ڈھیر_2->وسعت_use[idx].بلند, (int32_t)current_گنتی);
  rpmalloc_stat_جوڑو_peak(&ڈھیر_2->ناپ_class_use[class_idx].spans_current, 1,
                          ڈھیر_2->ناپ_class_use[class_idx].spans_peak);
#جے_انت
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
ساكن وسعت_t *
rpmalloc_ڈھیر_کڈھو_new_وسعت(ڈھیر_t *ڈھیر_2,
                                ڈھیر_ناپ_class_t *ڈھیر_ناپ_class,
                                size_t وسعت_گنتی, uint32_t class_idx) {
  وسعت_t *وسعت;
#إذا ENABLE_THREAD_CACHE
  إذا (ڈھیر_ناپ_class && ڈھیر_ناپ_class->cache) {
    وسعت = ڈھیر_ناپ_class->cache;
    ڈھیر_ناپ_class->cache =
        (ڈھیر_2->وسعت_cache.گنتی
             ? ڈھیر_2->وسعت_cache.وسعت[--ڈھیر_2->وسعت_cache.گنتی]
             : 0);
    rpmalloc_inc_وسعت_statistics(ڈھیر_2, وسعت_گنتی, class_idx);
    أرجع وسعت;
  }
#جے_انت
  (فراغ)حجم(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t ادھار_وسعت_گنتی = وسعت_گنتی;
  size_t حد_بندی_وسعت_گنتی =
      (وسعت_گنتی > 2) ? (وسعت_گنتی + (وسعت_گنتی >> 1)) : وسعت_گنتی;
  إذا (حد_بندی_وسعت_گنتی > LARGE_CLASS_گنتی)
    حد_بندی_وسعت_گنتی = LARGE_CLASS_گنتی;
  افعل {
    وسعت = rpmalloc_ڈھیر_thread_cache_کڈھو(ڈھیر_2, وسعت_گنتی);
    إذا (EXPECTED(وسعت != 0)) {
      _rpmalloc_stat_inc(&ڈھیر_2->ناپ_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_وسعت_statistics(ڈھیر_2, وسعت_گنتی, class_idx);
      أرجع وسعت;
    }
    وسعت = rpmalloc_ڈھیر_thread_cache_deferred_کڈھو(ڈھیر_2, وسعت_گنتی);
    إذا (EXPECTED(وسعت != 0)) {
      _rpmalloc_stat_inc(&ڈھیر_2->ناپ_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_وسعت_statistics(ڈھیر_2, وسعت_گنتی, class_idx);
      أرجع وسعت;
    }
    وسعت = rpmalloc_ڈھیر_global_cache_کڈھو(ڈھیر_2, وسعت_گنتی);
    إذا (EXPECTED(وسعت != 0)) {
      _rpmalloc_stat_inc(&ڈھیر_2->ناپ_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_وسعت_statistics(ڈھیر_2, وسعت_گنتی, class_idx);
      أرجع وسعت;
    }
    وسعت = rpmalloc_ڈھیر_reserved_کڈھو(ڈھیر_2, وسعت_گنتی);
    إذا (EXPECTED(وسعت != 0)) {
      _rpmalloc_stat_inc(&ڈھیر_2->ناپ_class_use[class_idx].spans_from_reserved);
      rpmalloc_inc_وسعت_statistics(ڈھیر_2, وسعت_گنتی, class_idx);
      أرجع وسعت;
    }
    ++وسعت_گنتی;
  } طالما (وسعت_گنتی <= حد_بندی_وسعت_گنتی);
  // Final fallback, map in more virtual memory
  وسعت = rpmalloc_وسعت_map(ڈھیر_2, ادھار_وسعت_گنتی);
  rpmalloc_inc_وسعت_statistics(ڈھیر_2, ادھار_وسعت_گنتی, class_idx);
  _rpmalloc_stat_inc(&ڈھیر_2->ناپ_class_use[class_idx].spans_map_calls);
  أرجع وسعت;
}

ساكن فراغ rpmalloc_ڈھیر_initialize(ڈھیر_t *ڈھیر_2) {
  _rpmalloc_memset_const(ڈھیر_2, 0, حجم(ڈھیر_t));
  // Get a new heap ID
  ڈھیر_2->id = 1 + atomic_incr32(&حافظہ_ڈھیر_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)ڈھیر_2->id % ڈھیر_ARRAY_ناپ;
  ڈھیر_2->اگلا_ڈھیر = حافظہ_heaps[list_idx];
  حافظہ_heaps[list_idx] = ڈھیر_2;
}

ساكن فراغ rpmalloc_ڈھیر_orphan(ڈھیر_t *ڈھیر_2, صحيح پہلا_class) {
  ڈھیر_2->owner_thread = (uintptr_t)-1;
#إذا RPMALLOC_پہلا_CLASS_HEAPS
  ڈھیر_t **ڈھیر_list =
      (پہلا_class ? &حافظہ_پہلا_class_orphan_heaps : &حافظہ_orphan_heaps);
#وإلا
  (فراغ)حجم(پہلا_class);
  ڈھیر_t **ڈھیر_list = &حافظہ_orphan_heaps;
#جے_انت
  ڈھیر_2->اگلا_orphan = *ڈھیر_list;
  *ڈھیر_list = ڈھیر_2;
}

//! Allocate a new heap from newly mapped memory pages
ساكن ڈھیر_t *rpmalloc_ڈھیر_allocate_new(فراغ) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t ڈھیر_ناپ_2 = حجم(ڈھیر_t);
  size_t aligned_ڈھیر_ناپ = 16 * ((ڈھیر_ناپ_2 + 15) / 16);
  size_t request_ڈھیر_گنتی = 16;
  size_t ڈھیر_وسعت_گنتی = ((aligned_ڈھیر_ناپ * request_ڈھیر_گنتی) +
                            حجم(وسعت_t) + حافظہ_وسعت_ناپ - 1) /
                           حافظہ_وسعت_ناپ;
  size_t بلاک_ناپ_2 = حافظہ_وسعت_ناپ * ڈھیر_وسعت_گنتی;
  size_t وسعت_گنتی = ڈھیر_وسعت_گنتی;
  وسعت_t *وسعت = 0;
  // If there are global reserved spans, use these first
  إذا (حافظہ_global_reserve_گنتی >= ڈھیر_وسعت_گنتی) {
    وسعت = _rpmalloc_global_get_reserved_spans(ڈھیر_وسعت_گنتی);
  }
  إذا (!وسعت) {
    إذا (حافظہ_ورقہ_ناپ > بلاک_ناپ_2) {
      وسعت_گنتی = حافظہ_ورقہ_ناپ / حافظہ_وسعت_ناپ;
      بلاک_ناپ_2 = حافظہ_ورقہ_ناپ;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_ڈھیر_گنتی =
          (بلاک_ناپ_2 - حجم(وسعت_t)) / aligned_ڈھیر_ناپ;
      إذا (possible_ڈھیر_گنتی >= (request_ڈھیر_گنتی * 16))
        request_ڈھیر_گنتی *= 16;
      وإلا إذا (possible_ڈھیر_گنتی < request_ڈھیر_گنتی)
        request_ڈھیر_گنتی = possible_ڈھیر_گنتی;
      ڈھیر_وسعت_گنتی = ((aligned_ڈھیر_ناپ * request_ڈھیر_گنتی) +
                         حجم(وسعت_t) + حافظہ_وسعت_ناپ - 1) /
                        حافظہ_وسعت_ناپ;
    }

    size_t align_وتھ = 0;
    وسعت = (وسعت_t *)_rpmalloc_mmap(بلاک_ناپ_2, &align_وتھ);
    إذا (!وسعت)
      أرجع 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    rpmalloc_وسعت_initialize(وسعت, وسعت_گنتی, ڈھیر_وسعت_گنتی, align_وتھ);
  }

  size_t remain_ناپ = حافظہ_وسعت_ناپ - حجم(وسعت_t);
  ڈھیر_t *ڈھیر_2 = (ڈھیر_t *)اشارہ_گر_وتھ(وسعت, حجم(وسعت_t));
  rpmalloc_ڈھیر_initialize(ڈھیر_2);

  // Put extra heaps as orphans
  size_t عدد_heaps = remain_ناپ / aligned_ڈھیر_ناپ;
  إذا (عدد_heaps < request_ڈھیر_گنتی)
    عدد_heaps = request_ڈھیر_گنتی;
  atomic_store32(&ڈھیر_2->بچہ_گنتی, (int32_t)عدد_heaps - 1);
  ڈھیر_t *extra_ڈھیر = (ڈھیر_t *)اشارہ_گر_وتھ(ڈھیر_2, aligned_ڈھیر_ناپ);
  طالما (عدد_heaps > 1) {
    rpmalloc_ڈھیر_initialize(extra_ڈھیر);
    extra_ڈھیر->master_ڈھیر = ڈھیر_2;
    rpmalloc_ڈھیر_orphan(extra_ڈھیر, 1);
    extra_ڈھیر = (ڈھیر_t *)اشارہ_گر_وتھ(extra_ڈھیر, aligned_ڈھیر_ناپ);
    --عدد_heaps;
  }

  إذا (وسعت_گنتی > ڈھیر_وسعت_گنتی) {
    // Cap reserved spans
    size_t remain_گنتی = وسعت_گنتی - ڈھیر_وسعت_گنتی;
    size_t reserve_گنتی =
        (remain_گنتی > حافظہ_ڈھیر_reserve_گنتی ? حافظہ_ڈھیر_reserve_گنتی
                                                   : remain_گنتی);
    وسعت_t *remain_وسعت =
        (وسعت_t *)اشارہ_گر_وتھ(وسعت, ڈھیر_وسعت_گنتی * حافظہ_وسعت_ناپ);
    rpmalloc_ڈھیر_سیٹ_reserved_spans(ڈھیر_2, وسعت, remain_وسعت, reserve_گنتی);

    إذا (remain_گنتی > reserve_گنتی) {
      // Set to global reserved spans
      remain_وسعت = (وسعت_t *)اشارہ_گر_وتھ(remain_وسعت,
                                             reserve_گنتی * حافظہ_وسعت_ناپ);
      reserve_گنتی = remain_گنتی - reserve_گنتی;
      rpmalloc_global_سیٹ_reserved_spans(وسعت, remain_وسعت, reserve_گنتی);
    }
  }

  أرجع ڈھیر_2;
}

ساكن ڈھیر_t *rpmalloc_ڈھیر_کڈھو_orphan(ڈھیر_t **ڈھیر_list) {
  ڈھیر_t *ڈھیر_2 = *ڈھیر_list;
  *ڈھیر_list = (ڈھیر_2 ? ڈھیر_2->اگلا_orphan : 0);
  أرجع ڈھیر_2;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
ساكن ڈھیر_t *rpmalloc_ڈھیر_allocate(صحيح پہلا_class) {
  ڈھیر_t *ڈھیر_2 = 0;
  طالما (!atomic_cas32_acquire(&حافظہ_global_جندرا, 1, 0))
    _rpmalloc_spin();
  إذا (پہلا_class == 0)
    ڈھیر_2 = rpmalloc_ڈھیر_کڈھو_orphan(&حافظہ_orphan_heaps);
#إذا RPMALLOC_پہلا_CLASS_HEAPS
  إذا (!ڈھیر_2)
    ڈھیر_2 = rpmalloc_ڈھیر_کڈھو_orphan(&حافظہ_پہلا_class_orphan_heaps);
#جے_انت
  إذا (!ڈھیر_2)
    ڈھیر_2 = rpmalloc_ڈھیر_allocate_new();
  atomic_store32_release(&حافظہ_global_جندرا, 0);
  إذا (ڈھیر_2)
    rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, 0);
  أرجع ڈھیر_2;
}

ساكن فراغ rpmalloc_ڈھیر_release(فراغ *heapptr, صحيح پہلا_class,
                                   صحيح release_cache) {
  ڈھیر_t *ڈھیر_2 = (ڈھیر_t *)heapptr;
  إذا (!ڈھیر_2)
    أرجع;
  // Release thread cache spans back to global cache
  rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, 0);
  إذا (release_cache || ڈھیر_2->finalize) {
#إذا ENABLE_THREAD_CACHE
    لكل (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
      وسعت_cache_t *وسعت_cache;
      إذا (!iclass)
        وسعت_cache = &ڈھیر_2->وسعت_cache;
      وإلا
        وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (iclass - 1));
      إذا (!وسعت_cache->گنتی)
        استمر;
#إذا ENABLE_GLOBAL_CACHE
      إذا (ڈھیر_2->finalize) {
        لكل (size_t ispan = 0; ispan < وسعت_cache->گنتی; ++ispan)
          rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
      } وإلا {
        _rpmalloc_stat_add64(&ڈھیر_2->thread_منزل_global, وسعت_cache->گنتی *
                                                          (iclass + 1) *
                                                          حافظہ_وسعت_ناپ);
        rpmalloc_stat_جوڑو(&ڈھیر_2->وسعت_use[iclass].spans_منزل_global,
                           وسعت_cache->گنتی);
        rpmalloc_global_cache_داخل_کرو_spans(وسعت_cache->وسعت, iclass + 1,
                                            وسعت_cache->گنتی);
      }
#وإلا
      لكل (size_t ispan = 0; ispan < وسعت_cache->گنتی; ++ispan)
        rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
#جے_انت
      وسعت_cache->گنتی = 0;
    }
#جے_انت
  }

  إذا (get_thread_ڈھیر_raw() == ڈھیر_2)
    سیٹ_thread_ڈھیر(0);

#إذا ENABLE_STATISTICS
  atomic_decr32(&حافظہ_سرگرم_heaps);
  rpmalloc_assert(atomic_load32(&حافظہ_سرگرم_heaps) >= 0,
                  "Still active heaps during finalization");
#جے_انت

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  إذا (get_thread_id() != _rpmalloc_main_thread_id) {
    طالما (!atomic_cas32_acquire(&حافظہ_global_جندرا, 1, 0))
      _rpmalloc_spin();
  }
  rpmalloc_ڈھیر_orphan(ڈھیر_2, پہلا_class);
  atomic_store32_release(&حافظہ_global_جندرا, 0);
}

ساكن فراغ rpmalloc_ڈھیر_release_raw(فراغ *heapptr, صحيح release_cache) {
  rpmalloc_ڈھیر_release(heapptr, 0, release_cache);
}

ساكن فراغ rpmalloc_ڈھیر_release_raw_fc(فراغ *heapptr) {
  rpmalloc_ڈھیر_release_raw(heapptr, 1);
}

ساكن فراغ rpmalloc_ڈھیر_finalize(ڈھیر_t *ڈھیر_2) {
  إذا (ڈھیر_2->spans_reserved) {
    وسعت_t *وسعت = rpmalloc_وسعت_map(ڈھیر_2, ڈھیر_2->spans_reserved);
    rpmalloc_وسعت_unmap(وسعت);
    ڈھیر_2->spans_reserved = 0;
  }

  rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, 0);

  لكل (size_t iclass = 0; iclass < ناپ_CLASS_گنتی; ++iclass) {
    إذا (ڈھیر_2->ناپ_class[iclass].cache)
      rpmalloc_وسعت_unmap(ڈھیر_2->ناپ_class[iclass].cache);
    ڈھیر_2->ناپ_class[iclass].cache = 0;
    وسعت_t *وسعت = ڈھیر_2->ناپ_class[iclass].partial_وسعت;
    طالما (وسعت) {
      وسعت_t *اگلا = وسعت->اگلا;
      rpmalloc_وسعت_finalize(ڈھیر_2, iclass, وسعت,
                              &ڈھیر_2->ناپ_class[iclass].partial_وسعت);
      وسعت = اگلا;
    }
    // If class still has a free list it must be a full span
    إذا (ڈھیر_2->ناپ_class[iclass].free_list) {
      وسعت_t *class_وسعت =
          (وسعت_t *)((uintptr_t)ڈھیر_2->ناپ_class[iclass].free_list &
                     حافظہ_وسعت_نقاب);
      وسعت_t **list = 0;
#إذا RPMALLOC_پہلا_CLASS_HEAPS
      list = &ڈھیر_2->بھریا_وسعت[iclass];
#جے_انت
      --ڈھیر_2->بھریا_وسعت_گنتی;
      إذا (!rpmalloc_وسعت_finalize(ڈھیر_2, iclass, class_وسعت, list)) {
        إذا (list)
          rpmalloc_وسعت_double_link_list_remove(list, class_وسعت);
        rpmalloc_وسعت_double_link_list_جوڑو(
            &ڈھیر_2->ناپ_class[iclass].partial_وسعت, class_وسعت);
      }
    }
  }

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    وسعت_cache_t *وسعت_cache;
    إذا (!iclass)
      وسعت_cache = &ڈھیر_2->وسعت_cache;
    وإلا
      وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (iclass - 1));
    لكل (size_t ispan = 0; ispan < وسعت_cache->گنتی; ++ispan)
      rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
    وسعت_cache->گنتی = 0;
  }
#جے_انت
  rpmalloc_assert(!atomic_لدو_ptr(&ڈھیر_2->وسعت_free_deferred),
                  "Heaps still active during finalization");
}

////////////
///
/// Allocation entry points
///
//////

//! Pop first block from a free list
ساكن فراغ *free_list_نکالو(فراغ **list) {
  فراغ *بلاک = *list;
  *list = *((فراغ **)بلاک);
  أرجع بلاک;
}

//! Allocate a small/medium sized memory block from the given heap
ساكن فراغ *rpmalloc_allocate_from_ڈھیر_fallback(
    ڈھیر_t *ڈھیر_2, ڈھیر_ناپ_class_t *ڈھیر_ناپ_class, uint32_t class_idx) {
  وسعت_t *وسعت = ڈھیر_ناپ_class->partial_وسعت;
  rpmalloc_assume(ڈھیر_2 != 0);
  إذا (EXPECTED(وسعت != 0)) {
    rpmalloc_assert(وسعت->بلاک_گنتی ==
                        حافظہ_ناپ_class[وسعت->ناپ_class].بلاک_گنتی,
                    "Span block count corrupted");
    rpmalloc_assert(!rpmalloc_وسعت_is_fully_utilized(وسعت),
                    "Internal failure");
    فراغ *بلاک;
    إذا (وسعت->free_list) {
      // Span local free list is not empty, swap to size class free list
      بلاک = free_list_نکالو(&وسعت->free_list);
      ڈھیر_ناپ_class->free_list = وسعت->free_list;
      وسعت->free_list = 0;
    } وإلا {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      فراغ *بلاک_شروع = اشارہ_گر_وتھ(
          وسعت, وسعت_سرخی_ناپ +
                    ((size_t)وسعت->free_list_حد_بندی * وسعت->بلاک_ناپ_2));
      وسعت->free_list_حد_بندی += free_list_partial_init(
          &ڈھیر_ناپ_class->free_list, &بلاک,
          (فراغ *)((uintptr_t)بلاک_شروع & ~(حافظہ_ورقہ_ناپ - 1)),
          بلاک_شروع, وسعت->بلاک_گنتی - وسعت->free_list_حد_بندی,
          وسعت->بلاک_ناپ_2);
    }
    rpmalloc_assert(وسعت->free_list_حد_بندی <= وسعت->بلاک_گنتی,
                    "Span block count corrupted");
    وسعت->ورتیا_گنتی = وسعت->free_list_حد_بندی;

    // Swap in deferred free list if present
    إذا (atomic_لدو_ptr(&وسعت->free_list_deferred))
      rpmalloc_وسعت_کڈھو_free_list_deferred(وسعت);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    إذا (!rpmalloc_وسعت_is_fully_utilized(وسعت))
      أرجع بلاک;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_وسعت_double_link_list_نکالو_سر(&ڈھیر_ناپ_class->partial_وسعت,
                                             وسعت);
#إذا RPMALLOC_پہلا_CLASS_HEAPS
    rpmalloc_وسعت_double_link_list_جوڑو(&ڈھیر_2->بھریا_وسعت[class_idx], وسعت);
#جے_انت
    ++ڈھیر_2->بھریا_وسعت_گنتی;
    أرجع بلاک;
  }

  // Find a span in one of the cache levels
  وسعت = rpmalloc_ڈھیر_کڈھو_new_وسعت(ڈھیر_2, ڈھیر_ناپ_class, 1, class_idx);
  إذا (EXPECTED(وسعت != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    أرجع rpmalloc_وسعت_initialize_new(ڈھیر_2, ڈھیر_ناپ_class, وسعت,
                                         class_idx);
  }

  أرجع 0;
}

//! Allocate a small sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_small(ڈھیر_t *ڈھیر_2, size_t ناپ_2) {
  rpmalloc_assert(ڈھیر_2, "No thread heap");
  // Small sizes have unique size classes
  ثابت uint32_t class_idx =
      (uint32_t)((ناپ_2 + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_کھسکاؤ);
  ڈھیر_ناپ_class_t *ڈھیر_ناپ_class = ڈھیر_2->ناپ_class + class_idx;
  _rpmalloc_stat_inc_alloc(ڈھیر_2, class_idx);
  إذا (EXPECTED(ڈھیر_ناپ_class->free_list != 0))
    أرجع free_list_نکالو(&ڈھیر_ناپ_class->free_list);
  أرجع rpmalloc_allocate_from_ڈھیر_fallback(ڈھیر_2, ڈھیر_ناپ_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_medium(ڈھیر_t *ڈھیر_2, size_t ناپ_2) {
  rpmalloc_assert(ڈھیر_2, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ثابت uint32_t ادھار_idx =
      (uint32_t)(SMALL_CLASS_گنتی +
                 ((ناپ_2 - (SMALL_ناپ_حد_بندی + 1)) >> MEDIUM_GRANULARITY_کھسکاؤ));
  ثابت uint32_t class_idx = حافظہ_ناپ_class[ادھار_idx].class_idx;
  ڈھیر_ناپ_class_t *ڈھیر_ناپ_class = ڈھیر_2->ناپ_class + class_idx;
  _rpmalloc_stat_inc_alloc(ڈھیر_2, class_idx);
  إذا (EXPECTED(ڈھیر_ناپ_class->free_list != 0))
    أرجع free_list_نکالو(&ڈھیر_ناپ_class->free_list);
  أرجع rpmalloc_allocate_from_ڈھیر_fallback(ڈھیر_2, ڈھیر_ناپ_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_large(ڈھیر_t *ڈھیر_2, size_t ناپ_2) {
  rpmalloc_assert(ڈھیر_2, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  ناپ_2 += وسعت_سرخی_ناپ;
  size_t وسعت_گنتی = ناپ_2 >> حافظہ_وسعت_ناپ_کھسکاؤ;
  إذا (ناپ_2 & (حافظہ_وسعت_ناپ - 1))
    ++وسعت_گنتی;

  // Find a span in one of the cache levels
  وسعت_t *وسعت =
      rpmalloc_ڈھیر_کڈھو_new_وسعت(ڈھیر_2, 0, وسعت_گنتی, ناپ_CLASS_LARGE);
  إذا (!وسعت)
    أرجع وسعت;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(وسعت->وسعت_گنتی >= وسعت_گنتی, "Internal failure");
  وسعت->ناپ_class = ناپ_CLASS_LARGE;
  وسعت->ڈھیر_2 = ڈھیر_2;

#إذا RPMALLOC_پہلا_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_جوڑو(&ڈھیر_2->large_huge_وسعت, وسعت);
#جے_انت
  ++ڈھیر_2->بھریا_وسعت_گنتی;

  أرجع اشارہ_گر_وتھ(وسعت, وسعت_سرخی_ناپ);
}

//! Allocate a huge block by mapping memory pages directly
ساكن فراغ *_rpmalloc_allocate_huge(ڈھیر_t *ڈھیر_2, size_t ناپ_2) {
  rpmalloc_assert(ڈھیر_2, "No thread heap");
  rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, 0);
  ناپ_2 += وسعت_سرخی_ناپ;
  size_t عدد_ورقے = ناپ_2 >> حافظہ_ورقہ_ناپ_کھسکاؤ;
  إذا (ناپ_2 & (حافظہ_ورقہ_ناپ - 1))
    ++عدد_ورقے;
  size_t align_وتھ = 0;
  وسعت_t *وسعت =
      (وسعت_t *)_rpmalloc_mmap(عدد_ورقے * حافظہ_ورقہ_ناپ, &align_وتھ);
  إذا (!وسعت)
    أرجع وسعت;

  // Store page count in span_count
  وسعت->ناپ_class = ناپ_CLASS_HUGE;
  وسعت->وسعت_گنتی = (uint32_t)عدد_ورقے;
  وسعت->align_وتھ = (uint32_t)align_وتھ;
  وسعت->ڈھیر_2 = ڈھیر_2;
  rpmalloc_stat_جوڑو_peak(&huge_ورقے_current, عدد_ورقے, huge_ورقے_peak);

#إذا RPMALLOC_پہلا_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_جوڑو(&ڈھیر_2->large_huge_وسعت, وسعت);
#جے_انت
  ++ڈھیر_2->بھریا_وسعت_گنتی;

  أرجع اشارہ_گر_وتھ(وسعت, وسعت_سرخی_ناپ);
}

//! Allocate a block of the given size
ساكن فراغ *_rpmalloc_allocate(ڈھیر_t *ڈھیر_2, size_t ناپ_2) {
  _rpmalloc_stat_add64(&ونڈ_counter, 1);
  إذا (EXPECTED(ناپ_2 <= SMALL_ناپ_حد_بندی))
    أرجع _rpmalloc_allocate_small(ڈھیر_2, ناپ_2);
  وإلا إذا (ناپ_2 <= حافظہ_medium_ناپ_حد_بندی)
    أرجع _rpmalloc_allocate_medium(ڈھیر_2, ناپ_2);
  وإلا إذا (ناپ_2 <= LARGE_ناپ_حد_بندی)
    أرجع _rpmalloc_allocate_large(ڈھیر_2, ناپ_2);
  أرجع _rpmalloc_allocate_huge(ڈھیر_2, ناپ_2);
}

ساكن فراغ *_rpmalloc_aligned_allocate(ڈھیر_t *ڈھیر_2, size_t alignment,
                                        size_t ناپ_2) {
  إذا (alignment <= SMALL_GRANULARITY)
    أرجع _rpmalloc_allocate(ڈھیر_2, ناپ_2);

#إذا ENABLE_VALIDATE_دلیلاں
  إذا ((ناپ_2 + alignment) < ناپ_2) {
    errno = EINVAL;
    أرجع 0;
  }
  إذا (alignment & (alignment - 1)) {
    errno = EINVAL;
    أرجع 0;
  }
#جے_انت

  إذا ((alignment <= وسعت_سرخی_ناپ) &&
      ((ناپ_2 + وسعت_سرخی_ناپ) < حافظہ_medium_ناپ_حد_بندی)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t مضرب_ناپ = ناپ_2 ? (ناپ_2 + (وسعت_سرخی_ناپ - 1)) &
                                      ~(uintptr_t)(وسعت_سرخی_ناپ - 1)
                                : وسعت_سرخی_ناپ;
    rpmalloc_assert(!(مضرب_ناپ % وسعت_سرخی_ناپ),
                    "Failed alignment calculation");
    إذا (مضرب_ناپ <= (ناپ_2 + alignment))
      أرجع _rpmalloc_allocate(ڈھیر_2, مضرب_ناپ);
  }

  فراغ *ptr = 0;
  size_t align_نقاب = alignment - 1;
  إذا (alignment <= حافظہ_ورقہ_ناپ) {
    ptr = _rpmalloc_allocate(ڈھیر_2, ناپ_2 + alignment);
    إذا ((uintptr_t)ptr & align_نقاب) {
      ptr = (فراغ *)(((uintptr_t)ptr & ~(uintptr_t)align_نقاب) + alignment);
      // Mark as having aligned blocks
      وسعت_t *وسعت = (وسعت_t *)((uintptr_t)ptr & حافظہ_وسعت_نقاب);
      وسعت->flags |= وسعت_FLAG_ALIGNED_بلاکاں;
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
  إذا (alignment & align_نقاب) {
    errno = EINVAL;
    أرجع 0;
  }
  إذا (alignment >= حافظہ_وسعت_ناپ) {
    errno = EINVAL;
    أرجع 0;
  }

  size_t extra_ورقے = alignment / حافظہ_ورقہ_ناپ;

  // Since each span has a header, we will at least need one extra memory page
  size_t عدد_ورقے = 1 + (ناپ_2 / حافظہ_ورقہ_ناپ);
  إذا (ناپ_2 & (حافظہ_ورقہ_ناپ - 1))
    ++عدد_ورقے;

  إذا (extra_ورقے > عدد_ورقے)
    عدد_ورقے = 1 + extra_ورقے;

  size_t original_ورقے = عدد_ورقے;
  size_t حد_بندی_ورقے = (حافظہ_وسعت_ناپ / حافظہ_ورقہ_ناپ) * 2;
  إذا (حد_بندی_ورقے < (original_ورقے * 2))
    حد_بندی_ورقے = original_ورقے * 2;

  size_t mapped_ناپ, align_وتھ;
  وسعت_t *وسعت;

retry:
  align_وتھ = 0;
  mapped_ناپ = عدد_ورقے * حافظہ_ورقہ_ناپ;

  وسعت = (وسعت_t *)_rpmalloc_mmap(mapped_ناپ, &align_وتھ);
  إذا (!وسعت) {
    errno = ENOMEM;
    أرجع 0;
  }
  ptr = اشارہ_گر_وتھ(وسعت, وسعت_سرخی_ناپ);

  إذا ((uintptr_t)ptr & align_نقاب)
    ptr = (فراغ *)(((uintptr_t)ptr & ~(uintptr_t)align_نقاب) + alignment);

  إذا (((size_t)اشارہ_گر_diff(ptr, وسعت) >= حافظہ_وسعت_ناپ) ||
      (اشارہ_گر_وتھ(ptr, ناپ_2) > اشارہ_گر_وتھ(وسعت, mapped_ناپ)) ||
      (((uintptr_t)ptr & حافظہ_وسعت_نقاب) != (uintptr_t)وسعت)) {
    _rpmalloc_unmap(وسعت, mapped_ناپ, align_وتھ, mapped_ناپ);
    ++عدد_ورقے;
    إذا (عدد_ورقے > حد_بندی_ورقے) {
      errno = EINVAL;
      أرجع 0;
    }
    اذهب retry;
  }

  // Store page count in span_count
  وسعت->ناپ_class = ناپ_CLASS_HUGE;
  وسعت->وسعت_گنتی = (uint32_t)عدد_ورقے;
  وسعت->align_وتھ = (uint32_t)align_وتھ;
  وسعت->ڈھیر_2 = ڈھیر_2;
  rpmalloc_stat_جوڑو_peak(&huge_ورقے_current, عدد_ورقے, huge_ورقے_peak);

#إذا RPMALLOC_پہلا_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_جوڑو(&ڈھیر_2->large_huge_وسعت, وسعت);
#جے_انت
  ++ڈھیر_2->بھریا_وسعت_گنتی;

  _rpmalloc_stat_add64(&ونڈ_counter, 1);

  أرجع ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
ساكن فراغ rpmalloc_deallocate_سدھا_small_or_medium(وسعت_t *وسعت,
                                                        فراغ *بلاک) {
  ڈھیر_t *ڈھیر_2 = وسعت->ڈھیر_2;
  rpmalloc_assert(ڈھیر_2->owner_thread == get_thread_id() ||
                      !ڈھیر_2->owner_thread || ڈھیر_2->finalize,
                  "Internal failure");
  // Add block to free list
  إذا (UNEXPECTED(rpmalloc_وسعت_is_fully_utilized(وسعت))) {
    وسعت->ورتیا_گنتی = وسعت->بلاک_گنتی;
#إذا RPMALLOC_پہلا_CLASS_HEAPS
    rpmalloc_وسعت_double_link_list_remove(&ڈھیر_2->بھریا_وسعت[وسعت->ناپ_class],
                                           وسعت);
#جے_انت
    rpmalloc_وسعت_double_link_list_جوڑو(
        &ڈھیر_2->ناپ_class[وسعت->ناپ_class].partial_وسعت, وسعت);
    --ڈھیر_2->بھریا_وسعت_گنتی;
  }
  *((فراغ **)بلاک) = وسعت->free_list;
  --وسعت->ورتیا_گنتی;
  وسعت->free_list = بلاک;
  إذا (UNEXPECTED(وسعت->ورتیا_گنتی == وسعت->list_ناپ)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    إذا (وسعت->ورتیا_گنتی) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      فراغ *free_list;
      افعل {
        free_list = atomic_exchange_ptr_acquire(&وسعت->free_list_deferred,
                                                INVALID_اشارہ_گر);
      } طالما (free_list == INVALID_اشارہ_گر);
      atomic_store_ptr_release(&وسعت->free_list_deferred, free_list);
    }
    rpmalloc_وسعت_double_link_list_remove(
        &ڈھیر_2->ناپ_class[وسعت->ناپ_class].partial_وسعت, وسعت);
    rpmalloc_وسعت_release_منزل_cache(ڈھیر_2, وسعت);
  }
}

ساكن فراغ rpmalloc_deallocate_defer_free_وسعت(ڈھیر_t *ڈھیر_2, وسعت_t *وسعت) {
  إذا (وسعت->ناپ_class != ناپ_CLASS_HUGE)
    _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[وسعت->وسعت_گنتی - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  افعل {
    وسعت->free_list = (فراغ *)atomic_لدو_ptr(&ڈھیر_2->وسعت_free_deferred);
  } طالما (!atomic_cas_ptr(&ڈھیر_2->وسعت_free_deferred, وسعت, وسعت->free_list));
}

//! Put the block in the deferred free list of the owning span
ساكن فراغ _rpmalloc_deallocate_defer_small_or_medium(وسعت_t *وسعت,
                                                       فراغ *بلاک) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  فراغ *free_list;
  افعل {
    free_list =
        atomic_exchange_ptr_acquire(&وسعت->free_list_deferred, INVALID_اشارہ_گر);
  } طالما (free_list == INVALID_اشارہ_گر);
  *((فراغ **)بلاک) = free_list;
  uint32_t free_گنتی = ++وسعت->list_ناپ;
  صحيح all_deferred_free = (free_گنتی == وسعت->بلاک_گنتی);
  atomic_store_ptr_release(&وسعت->free_list_deferred, بلاک);
  إذا (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    rpmalloc_deallocate_defer_free_وسعت(وسعت->ڈھیر_2, وسعت);
  }
}

ساكن فراغ _rpmalloc_deallocate_small_or_medium(وسعت_t *وسعت, فراغ *p) {
  _rpmalloc_stat_inc_free(وسعت->ڈھیر_2, وسعت->ناپ_class);
  إذا (وسعت->flags & وسعت_FLAG_ALIGNED_بلاکاں) {
    // Realign pointer to block start
    فراغ *بلاکاں_شروع = اشارہ_گر_وتھ(وسعت, وسعت_سرخی_ناپ);
    uint32_t بلاک_وتھ = (uint32_t)اشارہ_گر_diff(p, بلاکاں_شروع);
    p = اشارہ_گر_وتھ(p, -(int32_t)(بلاک_وتھ % وسعت->بلاک_ناپ_2));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#إذا RPMALLOC_پہلا_CLASS_HEAPS
  صحيح defer =
      (وسعت->ڈھیر_2->owner_thread &&
       (وسعت->ڈھیر_2->owner_thread != get_thread_id()) && !وسعت->ڈھیر_2->finalize);
#وإلا
  صحيح defer =
      ((وسعت->ڈھیر_2->owner_thread != get_thread_id()) && !وسعت->ڈھیر_2->finalize);
#جے_انت
  إذا (!defer)
    rpmalloc_deallocate_سدھا_small_or_medium(وسعت, p);
  وإلا
    _rpmalloc_deallocate_defer_small_or_medium(وسعت, p);
}

//! Deallocate the given large memory block to the current heap
ساكن فراغ _rpmalloc_deallocate_large(وسعت_t *وسعت) {
  rpmalloc_assert(وسعت->ناپ_class == ناپ_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(وسعت->flags & وسعت_FLAG_MASTER) ||
                      !(وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((وسعت->flags & وسعت_FLAG_MASTER) ||
                      (وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#إذا RPMALLOC_پہلا_CLASS_HEAPS
  صحيح defer =
      (وسعت->ڈھیر_2->owner_thread &&
       (وسعت->ڈھیر_2->owner_thread != get_thread_id()) && !وسعت->ڈھیر_2->finalize);
#وإلا
  صحيح defer =
      ((وسعت->ڈھیر_2->owner_thread != get_thread_id()) && !وسعت->ڈھیر_2->finalize);
#جے_انت
  إذا (defer) {
    rpmalloc_deallocate_defer_free_وسعت(وسعت->ڈھیر_2, وسعت);
    أرجع;
  }
  rpmalloc_assert(وسعت->ڈھیر_2->بھریا_وسعت_گنتی, "Heap span counter corrupted");
  --وسعت->ڈھیر_2->بھریا_وسعت_گنتی;
#إذا RPMALLOC_پہلا_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_remove(&وسعت->ڈھیر_2->large_huge_وسعت, وسعت);
#جے_انت
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = وسعت->وسعت_گنتی - 1;
  atomic_decr32(&وسعت->ڈھیر_2->وسعت_use[idx].current);
#جے_انت
  ڈھیر_t *ڈھیر_2 = وسعت->ڈھیر_2;
  rpmalloc_assert(ڈھیر_2, "No thread heap");
#إذا ENABLE_THREAD_CACHE
  ثابت صحيح سیٹ_as_reserved =
      ((وسعت->وسعت_گنتی > 1) && (ڈھیر_2->وسعت_cache.گنتی == 0) &&
       !ڈھیر_2->finalize && !ڈھیر_2->spans_reserved);
#وإلا
  ثابت صحيح سیٹ_as_reserved =
      ((وسعت->وسعت_گنتی > 1) && !ڈھیر_2->finalize && !ڈھیر_2->spans_reserved);
#جے_انت
  إذا (سیٹ_as_reserved) {
    ڈھیر_2->وسعت_reserve = وسعت;
    ڈھیر_2->spans_reserved = وسعت->وسعت_گنتی;
    إذا (وسعت->flags & وسعت_FLAG_MASTER) {
      ڈھیر_2->وسعت_reserve_master = وسعت;
    } وإلا { // SPAN_FLAG_SUBSPAN
      وسعت_t *master = (وسعت_t *)اشارہ_گر_وتھ(
          وسعت,
          -(intptr_t)((size_t)وسعت->وتھ_from_master * حافظہ_وسعت_ناپ));
      ڈھیر_2->وسعت_reserve_master = master;
      rpmalloc_assert(master->flags & وسعت_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->بچیا_spans) >=
                          (int32_t)وسعت->وسعت_گنتی,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[idx].spans_منزل_reserved);
  } وإلا {
    // Insert into cache list
    rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
  }
}

//! Deallocate the given huge span
ساكن فراغ _rpmalloc_deallocate_huge(وسعت_t *وسعت) {
  rpmalloc_assert(وسعت->ڈھیر_2, "No span heap");
#إذا RPMALLOC_پہلا_CLASS_HEAPS
  صحيح defer =
      (وسعت->ڈھیر_2->owner_thread &&
       (وسعت->ڈھیر_2->owner_thread != get_thread_id()) && !وسعت->ڈھیر_2->finalize);
#وإلا
  صحيح defer =
      ((وسعت->ڈھیر_2->owner_thread != get_thread_id()) && !وسعت->ڈھیر_2->finalize);
#جے_انت
  إذا (defer) {
    rpmalloc_deallocate_defer_free_وسعت(وسعت->ڈھیر_2, وسعت);
    أرجع;
  }
  rpmalloc_assert(وسعت->ڈھیر_2->بھریا_وسعت_گنتی, "Heap span counter corrupted");
  --وسعت->ڈھیر_2->بھریا_وسعت_گنتی;
#إذا RPMALLOC_پہلا_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_remove(&وسعت->ڈھیر_2->large_huge_وسعت, وسعت);
#جے_انت

  // Oversized allocation, page count is stored in span_count
  size_t عدد_ورقے = وسعت->وسعت_گنتی;
  _rpmalloc_unmap(وسعت, عدد_ورقے * حافظہ_ورقہ_ناپ, وسعت->align_وتھ,
                  عدد_ورقے * حافظہ_ورقہ_ناپ);
  _rpmalloc_stat_sub(&huge_ورقے_current, عدد_ورقے);
}

//! Deallocate the given block
ساكن فراغ _rpmalloc_deallocate(فراغ *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  وسعت_t *وسعت = (وسعت_t *)((uintptr_t)p & حافظہ_وسعت_نقاب);
  إذا (UNEXPECTED(!وسعت))
    أرجع;
  إذا (EXPECTED(وسعت->ناپ_class < ناپ_CLASS_گنتی))
    _rpmalloc_deallocate_small_or_medium(وسعت, p);
  وإلا إذا (وسعت->ناپ_class == ناپ_CLASS_LARGE)
    _rpmalloc_deallocate_large(وسعت);
  وإلا
    _rpmalloc_deallocate_huge(وسعت);
}

////////////
///
/// Reallocation entry points
///
//////

ساكن size_t rpmalloc_usable_ناپ_2(فراغ *p);

//! Reallocate the given block to the given size
ساكن فراغ *_rpmalloc_reallocate(ڈھیر_t *ڈھیر_2, فراغ *p, size_t ناپ_2,
                                  size_t oldsize, بنانشانی صحيح flags) {
  إذا (p) {
    // Grab the span using guaranteed span alignment
    وسعت_t *وسعت = (وسعت_t *)((uintptr_t)p & حافظہ_وسعت_نقاب);
    إذا (EXPECTED(وسعت->ناپ_class < ناپ_CLASS_گنتی)) {
      // Small/medium sized block
      rpmalloc_assert(وسعت->وسعت_گنتی == 1, "Span counter corrupted");
      فراغ *بلاکاں_شروع = اشارہ_گر_وتھ(وسعت, وسعت_سرخی_ناپ);
      uint32_t بلاک_وتھ = (uint32_t)اشارہ_گر_diff(p, بلاکاں_شروع);
      uint32_t بلاک_idx = بلاک_وتھ / وسعت->بلاک_ناپ_2;
      فراغ *بلاک =
          اشارہ_گر_وتھ(بلاکاں_شروع, (size_t)بلاک_idx * وسعت->بلاک_ناپ_2);
      إذا (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)وسعت->بلاک_ناپ_2 - اشارہ_گر_diff(p, بلاک));
      إذا ((size_t)وسعت->بلاک_ناپ_2 >= ناپ_2) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != بلاک) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلاک, p, oldsize);
        أرجع بلاک;
      }
    } وإلا إذا (وسعت->ناپ_class == ناپ_CLASS_LARGE) {
      // Large block
      size_t کل_ناپ_2 = ناپ_2 + وسعت_سرخی_ناپ;
      size_t عدد_spans = کل_ناپ_2 >> حافظہ_وسعت_ناپ_کھسکاؤ;
      إذا (کل_ناپ_2 & (حافظہ_وسعت_نقاب - 1))
        ++عدد_spans;
      size_t current_spans = وسعت->وسعت_گنتی;
      فراغ *بلاک = اشارہ_گر_وتھ(وسعت, وسعت_سرخی_ناپ);
      إذا (!oldsize)
        oldsize = (current_spans * حافظہ_وسعت_ناپ) -
                  (size_t)اشارہ_گر_diff(p, بلاک) - وسعت_سرخی_ناپ;
      إذا ((current_spans >= عدد_spans) && (کل_ناپ_2 >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != بلاک) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلاک, p, oldsize);
        أرجع بلاک;
      }
    } وإلا {
      // Oversized block
      size_t کل_ناپ_2 = ناپ_2 + وسعت_سرخی_ناپ;
      size_t عدد_ورقے = کل_ناپ_2 >> حافظہ_ورقہ_ناپ_کھسکاؤ;
      إذا (کل_ناپ_2 & (حافظہ_ورقہ_ناپ - 1))
        ++عدد_ورقے;
      // Page count is stored in span_count
      size_t current_ورقے = وسعت->وسعت_گنتی;
      فراغ *بلاک = اشارہ_گر_وتھ(وسعت, وسعت_سرخی_ناپ);
      إذا (!oldsize)
        oldsize = (current_ورقے * حافظہ_ورقہ_ناپ) -
                  (size_t)اشارہ_گر_diff(p, بلاک) - وسعت_سرخی_ناپ;
      إذا ((current_ورقے >= عدد_ورقے) && (عدد_ورقے >= (current_ورقے / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != بلاک) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلاک, p, oldsize);
        أرجع بلاک;
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
  size_t ہیٹھلا_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_ناپ =
      (ناپ_2 > ہیٹھلا_bound) ? ناپ_2 : ((ناپ_2 > oldsize) ? ہیٹھلا_bound : ناپ_2);
  فراغ *بلاک = _rpmalloc_allocate(ڈھیر_2, new_ناپ);
  إذا (p && بلاک) {
    إذا (!(flags & RPMALLOC_NO_PRESERVE))
      حافظہ_نقل(بلاک, p, oldsize < new_ناپ ? oldsize : new_ناپ);
    _rpmalloc_deallocate(p);
  }

  أرجع بلاک;
}

ساكن فراغ *_rpmalloc_aligned_reallocate(ڈھیر_t *ڈھیر_2, فراغ *ptr,
                                          size_t alignment, size_t ناپ_2,
                                          size_t oldsize, بنانشانی صحيح flags) {
  إذا (alignment <= SMALL_GRANULARITY)
    أرجع _rpmalloc_reallocate(ڈھیر_2, ptr, ناپ_2, oldsize, flags);

  صحيح no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? rpmalloc_usable_ناپ_2(ptr) : 0);
  إذا ((usablesize >= ناپ_2) && !((uintptr_t)ptr & (alignment - 1))) {
    إذا (no_alloc || (ناپ_2 >= (usablesize / 2)))
      أرجع ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  فراغ *بلاک =
      (!no_alloc ? _rpmalloc_aligned_allocate(ڈھیر_2, alignment, ناپ_2) : 0);
  إذا (EXPECTED(بلاک != 0)) {
    إذا (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      إذا (!oldsize)
        oldsize = usablesize;
      حافظہ_نقل(بلاک, ptr, oldsize < ناپ_2 ? oldsize : ناپ_2);
    }
    _rpmalloc_deallocate(ptr);
  }
  أرجع بلاک;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
ساكن size_t rpmalloc_usable_ناپ_2(فراغ *p) {
  // Grab the span using guaranteed span alignment
  وسعت_t *وسعت = (وسعت_t *)((uintptr_t)p & حافظہ_وسعت_نقاب);
  إذا (وسعت->ناپ_class < ناپ_CLASS_گنتی) {
    // Small/medium block
    فراغ *بلاکاں_شروع = اشارہ_گر_وتھ(وسعت, وسعت_سرخی_ناپ);
    أرجع وسعت->بلاک_ناپ_2 -
           ((size_t)اشارہ_گر_diff(p, بلاکاں_شروع) % وسعت->بلاک_ناپ_2);
  }
  إذا (وسعت->ناپ_class == ناپ_CLASS_LARGE) {
    // Large block
    size_t current_spans = وسعت->وسعت_گنتی;
    أرجع (current_spans * حافظہ_وسعت_ناپ) - (size_t)اشارہ_گر_diff(p, وسعت);
  }
  // Oversized block, page count is stored in span_count
  size_t current_ورقے = وسعت->وسعت_گنتی;
  أرجع (current_ورقے * حافظہ_ورقہ_ناپ) - (size_t)اشارہ_گر_diff(p, وسعت);
}

//! Adjust and optimize the size class properties for the given class
ساكن فراغ rpmalloc_adjust_ناپ_class(size_t iclass) {
  size_t بلاک_ناپ_2 = حافظہ_ناپ_class[iclass].بلاک_ناپ_2;
  size_t بلاک_گنتی = (حافظہ_وسعت_ناپ - وسعت_سرخی_ناپ) / بلاک_ناپ_2;

  حافظہ_ناپ_class[iclass].بلاک_گنتی = (uint16_t)بلاک_گنتی;
  حافظہ_ناپ_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  إذا (iclass >= SMALL_CLASS_گنتی) {
    size_t prevclass = iclass;
    طالما (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      إذا (حافظہ_ناپ_class[prevclass].بلاک_گنتی ==
          حافظہ_ناپ_class[iclass].بلاک_گنتی)
        _rpmalloc_memcpy_const(حافظہ_ناپ_class + prevclass,
                               حافظہ_ناپ_class + iclass,
                               حجم(حافظہ_ناپ_class[iclass]));
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
    حافظہ_نقل(&حافظہ_config, config, حجم(rpmalloc_config_t));
  وإلا
    _rpmalloc_memset_const(&حافظہ_config, 0, حجم(rpmalloc_config_t));

  إذا (!حافظہ_config.حافظہ_map || !حافظہ_config.حافظہ_unmap) {
    حافظہ_config.حافظہ_map = _rpmalloc_mmap_os;
    حافظہ_config.حافظہ_unmap = _rpmalloc_unmap_os;
  }

#إذا PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, حجم(system_info));
  GetSystemInfo(&system_info);
  حافظہ_map_granularity = system_info.dw_ونڈ_Granularity;
#وإلا
  حافظہ_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#جے_انت

#إذا RPMALLOC_CONFIGURABLE
  حافظہ_ورقہ_ناپ = حافظہ_config.ورقہ_ناپ_2;
#وإلا
  حافظہ_ورقہ_ناپ = 0;
#جے_انت
  حافظہ_huge_ورقے = 0;
  إذا (!حافظہ_ورقہ_ناپ) {
#إذا PLATFORM_WINDOWS
    حافظہ_ورقہ_ناپ = system_info.dw_ورقہ_ناپ;
#وإلا
    حافظہ_ورقہ_ناپ = حافظہ_map_granularity;
    إذا (حافظہ_config.enable_huge_ورقے) {
#إذا defined(__linux__)
      size_t huge_ورقہ_ناپ = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      إذا (meminfo) {
        حرف سطر_2[128];
        طالما (!huge_ورقہ_ناپ && fgets(سطر_2, حجم(سطر_2) - 1, meminfo)) {
          سطر_2[حجم(سطر_2) - 1] = 0;
          إذا (لڑی_کھوج(سطر_2, "Hugepagesize:"))
            huge_ورقہ_ناپ = (size_t)strtol(سطر_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      إذا (huge_ورقہ_ناپ) {
        حافظہ_huge_ورقے = 1;
        حافظہ_ورقہ_ناپ = huge_ورقہ_ناپ;
        حافظہ_map_granularity = huge_ورقہ_ناپ;
      }
#وإلاإذا defined(__FreeBSD__)
      صحيح rc;
      size_t sz = حجم(rc);

      إذا (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, عدم, 0) == 0 &&
          rc == 1) {
        ساكن size_t defsize = 2 * 1024 * 1024;
        صحيح nsize = 0;
        size_t sizes[4] = {0};
        حافظہ_huge_ورقے = 1;
        حافظہ_ورقہ_ناپ = defsize;
        إذا ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          لكل (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            إذا (defsize < csize) {
              حافظہ_ورقہ_ناپ = csize;
              اكسر;
            }
          }
        }
        حافظہ_map_granularity = حافظہ_ورقہ_ناپ;
      }
#وإلاإذا defined(__APPLE__) || defined(__NetBSD__)
      حافظہ_huge_ورقے = 1;
      حافظہ_ورقہ_ناپ = 2 * 1024 * 1024;
      حافظہ_map_granularity = حافظہ_ورقہ_ناپ;
#جے_انت
    }
#جے_انت
  } وإلا {
    إذا (حافظہ_config.enable_huge_ورقے)
      حافظہ_huge_ورقے = 1;
  }

#إذا PLATFORM_WINDOWS
  إذا (حافظہ_config.enable_huge_ورقے) {
    HANDLE token = 0;
    size_t large_ورقہ_minimum = Get_Large_ورقہ_Minimum();
    إذا (large_ورقہ_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    إذا (token) {
      LUID luid;
      إذا (Lookup_Privilege_قدر(0, SE_جندرا_حافظہ_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, حجم(token_privileges));
        token_privileges.Privilege_گنتی = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].گن = SE_PRIVILEGE_ENABLED;
        إذا (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          إذا (Get_آخری_Error() == ERROR_SUCCESS)
            حافظہ_huge_ورقے = 1;
        }
      }
      CloseHandle(token);
    }
    إذا (حافظہ_huge_ورقے) {
      إذا (large_ورقہ_minimum > حافظہ_ورقہ_ناپ)
        حافظہ_ورقہ_ناپ = large_ورقہ_minimum;
      إذا (large_ورقہ_minimum > حافظہ_map_granularity)
        حافظہ_map_granularity = large_ورقہ_minimum;
    }
  }
#جے_انت

  size_t min_وسعت_ناپ = 256;
  size_t max_ورقہ_ناپ;
#إذا UINTPTR_MAX > 0xFFFFFFFF
  max_ورقہ_ناپ = 4096ULL * 1024ULL * 1024ULL;
#وإلا
  max_ورقہ_ناپ = 4 * 1024 * 1024;
#جے_انت
  إذا (حافظہ_ورقہ_ناپ < min_وسعت_ناپ)
    حافظہ_ورقہ_ناپ = min_وسعت_ناپ;
  إذا (حافظہ_ورقہ_ناپ > max_ورقہ_ناپ)
    حافظہ_ورقہ_ناپ = max_ورقہ_ناپ;
  حافظہ_ورقہ_ناپ_کھسکاؤ = 0;
  size_t ورقہ_ناپ_ثنائی_ہندسہ = حافظہ_ورقہ_ناپ;
  طالما (ورقہ_ناپ_ثنائی_ہندسہ != 1) {
    ++حافظہ_ورقہ_ناپ_کھسکاؤ;
    ورقہ_ناپ_ثنائی_ہندسہ >>= 1;
  }
  حافظہ_ورقہ_ناپ = ((size_t)1 << حافظہ_ورقہ_ناپ_کھسکاؤ);

#إذا RPMALLOC_CONFIGURABLE
  إذا (!حافظہ_config.وسعت_ناپ) {
    حافظہ_وسعت_ناپ = حافظہ_default_وسعت_ناپ;
    حافظہ_وسعت_ناپ_کھسکاؤ = حافظہ_default_وسعت_ناپ_کھسکاؤ;
    حافظہ_وسعت_نقاب = حافظہ_default_وسعت_نقاب;
  } وإلا {
    size_t وسعت_ناپ = حافظہ_config.وسعت_ناپ;
    إذا (وسعت_ناپ > (256 * 1024))
      وسعت_ناپ = (256 * 1024);
    حافظہ_وسعت_ناپ = 4096;
    حافظہ_وسعت_ناپ_کھسکاؤ = 12;
    طالما (حافظہ_وسعت_ناپ < وسعت_ناپ) {
      حافظہ_وسعت_ناپ <<= 1;
      ++حافظہ_وسعت_ناپ_کھسکاؤ;
    }
    حافظہ_وسعت_نقاب = ~(uintptr_t)(حافظہ_وسعت_ناپ - 1);
  }
#جے_انت

  حافظہ_وسعت_map_گنتی =
      (حافظہ_config.وسعت_map_گنتی ? حافظہ_config.وسعت_map_گنتی
                                     : DEFAULT_وسعت_MAP_گنتی);
  إذا ((حافظہ_وسعت_ناپ * حافظہ_وسعت_map_گنتی) < حافظہ_ورقہ_ناپ)
    حافظہ_وسعت_map_گنتی = (حافظہ_ورقہ_ناپ / حافظہ_وسعت_ناپ);
  إذا ((حافظہ_ورقہ_ناپ >= حافظہ_وسعت_ناپ) &&
      ((حافظہ_وسعت_map_گنتی * حافظہ_وسعت_ناپ) % حافظہ_ورقہ_ناپ))
    حافظہ_وسعت_map_گنتی = (حافظہ_ورقہ_ناپ / حافظہ_وسعت_ناپ);
  حافظہ_ڈھیر_reserve_گنتی = (حافظہ_وسعت_map_گنتی > DEFAULT_وسعت_MAP_گنتی)
                                   ? DEFAULT_وسعت_MAP_گنتی
                                   : حافظہ_وسعت_map_گنتی;

  حافظہ_config.ورقہ_ناپ_2 = حافظہ_ورقہ_ناپ;
  حافظہ_config.وسعت_ناپ = حافظہ_وسعت_ناپ;
  حافظہ_config.وسعت_map_گنتی = حافظہ_وسعت_map_گنتی;
  حافظہ_config.enable_huge_ورقے = حافظہ_huge_ورقے;

#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  إذا (pthread_کلید_create(&حافظہ_thread_ڈھیر, rpmalloc_ڈھیر_release_raw_fc))
    أرجع -1;
#جے_انت
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_کلید = FlsAlloc(&_rpmalloc_thread_destructor);
#جے_انت

  // Setup all small and medium size classes
  size_t iclass = 0;
  حافظہ_ناپ_class[iclass].بلاک_ناپ_2 = SMALL_GRANULARITY;
  rpmalloc_adjust_ناپ_class(iclass);
  لكل (iclass = 1; iclass < SMALL_CLASS_گنتی; ++iclass) {
    size_t ناپ_2 = iclass * SMALL_GRANULARITY;
    حافظہ_ناپ_class[iclass].بلاک_ناپ_2 = (uint32_t)ناپ_2;
    rpmalloc_adjust_ناپ_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  حافظہ_medium_ناپ_حد_بندی = (حافظہ_وسعت_ناپ - وسعت_سرخی_ناپ) >> 1;
  إذا (حافظہ_medium_ناپ_حد_بندی > MEDIUM_ناپ_حد_بندی)
    حافظہ_medium_ناپ_حد_بندی = MEDIUM_ناپ_حد_بندی;
  لكل (iclass = 0; iclass < MEDIUM_CLASS_گنتی; ++iclass) {
    size_t ناپ_2 = SMALL_ناپ_حد_بندی + ((iclass + 1) * MEDIUM_GRANULARITY);
    إذا (ناپ_2 > حافظہ_medium_ناپ_حد_بندی) {
      حافظہ_medium_ناپ_حد_بندی =
          SMALL_ناپ_حد_بندی + (iclass * MEDIUM_GRANULARITY);
      اكسر;
    }
    حافظہ_ناپ_class[SMALL_CLASS_گنتی + iclass].بلاک_ناپ_2 = (uint32_t)ناپ_2;
    rpmalloc_adjust_ناپ_class(SMALL_CLASS_گنتی + iclass);
  }

  حافظہ_orphan_heaps = 0;
#إذا RPMALLOC_پہلا_CLASS_HEAPS
  حافظہ_پہلا_class_orphan_heaps = 0;
#جے_انت
#إذا ENABLE_STATISTICS
  atomic_store32(&حافظہ_سرگرم_heaps, 0);
  atomic_store32(&mapped_ورقے, 0);
  mapped_ورقے_peak = 0;
  atomic_store32(&_master_spans, 0);
  atomic_store32(&mapped_کل, 0);
  atomic_store32(&unmapped_کل, 0);
  atomic_store32(&mapped_ورقے_os, 0);
  atomic_store32(&huge_ورقے_current, 0);
  huge_ورقے_peak = 0;
#جے_انت
  memset(حافظہ_heaps, 0, حجم(حافظہ_heaps));
  atomic_store32_release(&حافظہ_global_جندرا, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  أرجع 0;
}

//! Finalize the allocator
فراغ rpmalloc_finalize(فراغ) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  إذا (حافظہ_global_reserve) {
    atomic_add32(&حافظہ_global_reserve_master->بچیا_spans,
                 -(int32_t)حافظہ_global_reserve_گنتی);
    حافظہ_global_reserve_master = 0;
    حافظہ_global_reserve_گنتی = 0;
    حافظہ_global_reserve = 0;
  }
  atomic_store32_release(&حافظہ_global_جندرا, 0);

  // Free all thread caches and fully free spans
  لكل (size_t list_idx = 0; list_idx < ڈھیر_ARRAY_ناپ; ++list_idx) {
    ڈھیر_t *ڈھیر_2 = حافظہ_heaps[list_idx];
    طالما (ڈھیر_2) {
      ڈھیر_t *اگلا_ڈھیر = ڈھیر_2->اگلا_ڈھیر;
      ڈھیر_2->finalize = 1;
      rpmalloc_ڈھیر_global_finalize(ڈھیر_2);
      ڈھیر_2 = اگلا_ڈھیر;
    }
  }

#إذا ENABLE_GLOBAL_CACHE
  // Free global caches
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass)
    _rpmalloc_global_cache_finalize(&حافظہ_وسعت_cache[iclass]);
#جے_انت

#إذا (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_کلید_delete(حافظہ_thread_ڈھیر);
#جے_انت
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_کلید);
  fls_کلید = 0;
#جے_انت
#إذا ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&mapped_ورقے) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&mapped_ورقے_os) == 0,
                  "Memory leak detected");
#جے_انت

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
خارجي مضمن فراغ rpmalloc_thread_initialize(فراغ) {
  إذا (!get_thread_ڈھیر_raw()) {
    ڈھیر_t *ڈھیر_2 = rpmalloc_ڈھیر_allocate(0);
    إذا (ڈھیر_2) {
      _rpmalloc_stat_inc(&حافظہ_سرگرم_heaps);
      سیٹ_thread_ڈھیر(ڈھیر_2);
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_سیٹ_قدر(fls_کلید, ڈھیر_2);
#جے_انت
    }
  }
}

//! Finalize thread, orphan heap
فراغ rpmalloc_thread_finalize(صحيح release_caches) {
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر_raw();
  إذا (ڈھیر_2)
    rpmalloc_ڈھیر_release_raw(ڈھیر_2, release_caches);
  سیٹ_thread_ڈھیر(0);
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_سیٹ_قدر(fls_کلید, 0);
#جے_انت
}

صحيح rpmalloc_is_thread_initialized(فراغ) {
  أرجع (get_thread_ڈھیر_raw() != 0) ? 1 : 0;
}

ثابت rpmalloc_config_t *rpmalloc_config(فراغ) { أرجع &حافظہ_config; }

// Extern interface

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpmalloc(size_t ناپ_2) {
#إذا ENABLE_VALIDATE_دلیلاں
  إذا (ناپ_2 >= MAX_ALLOC_ناپ) {
    errno = EINVAL;
    أرجع 0;
  }
#جے_انت
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر();
  أرجع _rpmalloc_allocate(ڈھیر_2, ناپ_2);
}

خارجي مضمن فراغ rpfree(فراغ *ptr) { _rpmalloc_deallocate(ptr); }

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpcalloc(size_t عدد_2, size_t ناپ_2) {
  size_t کل_2;
#إذا ENABLE_VALIDATE_دلیلاں
#إذا PLATFORM_WINDOWS
  صحيح err = ناپ_TMult(عدد_2, ناپ_2, &کل_2);
  إذا ((err != S_OK) || (کل_2 >= MAX_ALLOC_ناپ)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(عدد_2, ناپ_2, &کل_2);
  إذا (err || (کل_2 >= MAX_ALLOC_ناپ)) {
    errno = EINVAL;
    أرجع 0;
  }
#جے_انت
#وإلا
  کل_2 = عدد_2 * ناپ_2;
#جے_انت
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر();
  فراغ *بلاک = _rpmalloc_allocate(ڈھیر_2, کل_2);
  إذا (بلاک)
    memset(بلاک, 0, کل_2);
  أرجع بلاک;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rprealloc(فراغ *ptr, size_t ناپ_2) {
#إذا ENABLE_VALIDATE_دلیلاں
  إذا (ناپ_2 >= MAX_ALLOC_ناپ) {
    errno = EINVAL;
    أرجع ptr;
  }
#جے_انت
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر();
  أرجع _rpmalloc_reallocate(ڈھیر_2, ptr, ناپ_2, 0, 0);
}

خارجي RPMALLOC_ALLOCATOR فراغ *rpaligned_realloc(فراغ *ptr, size_t alignment,
                                                  size_t ناپ_2, size_t oldsize,
                                                  بنانشانی صحيح flags) {
#إذا ENABLE_VALIDATE_دلیلاں
  إذا ((ناپ_2 + alignment < ناپ_2) || (alignment > حافظہ_ورقہ_ناپ)) {
    errno = EINVAL;
    أرجع 0;
  }
#جے_انت
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر();
  أرجع _rpmalloc_aligned_reallocate(ڈھیر_2, ptr, alignment, ناپ_2, oldsize,
                                      flags);
}

خارجي RPMALLOC_ALLOCATOR فراغ *rpaligned_alloc(size_t alignment, size_t ناپ_2) {
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر();
  أرجع _rpmalloc_aligned_allocate(ڈھیر_2, alignment, ناپ_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpaligned_calloc(size_t alignment, size_t عدد_2, size_t ناپ_2) {
  size_t کل_2;
#إذا ENABLE_VALIDATE_دلیلاں
#إذا PLATFORM_WINDOWS
  صحيح err = ناپ_TMult(عدد_2, ناپ_2, &کل_2);
  إذا ((err != S_OK) || (کل_2 >= MAX_ALLOC_ناپ)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(عدد_2, ناپ_2, &کل_2);
  إذا (err || (کل_2 >= MAX_ALLOC_ناپ)) {
    errno = EINVAL;
    أرجع 0;
  }
#جے_انت
#وإلا
  کل_2 = عدد_2 * ناپ_2;
#جے_انت
  فراغ *بلاک = rpaligned_alloc(alignment, کل_2);
  إذا (بلاک)
    memset(بلاک, 0, کل_2);
  أرجع بلاک;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpmemalign(size_t alignment,
                                                  size_t ناپ_2) {
  أرجع rpaligned_alloc(alignment, ناپ_2);
}

خارجي مضمن صحيح rpposix_memalign(فراغ **memptr, size_t alignment,
                                   size_t ناپ_2) {
  إذا (memptr)
    *memptr = rpaligned_alloc(alignment, ناپ_2);
  وإلا
    أرجع EINVAL;
  أرجع *memptr ? 0 : ENOMEM;
}

خارجي مضمن size_t rpmalloc_usable_ناپ(فراغ *ptr) {
  أرجع (ptr ? rpmalloc_usable_ناپ_2(ptr) : 0);
}

خارجي مضمن فراغ rpmalloc_thread_collect(فراغ) {}

فراغ rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, حجم(rpmalloc_thread_statistics_t));
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر_raw();
  إذا (!ڈھیر_2)
    أرجع;

  لكل (size_t iclass = 0; iclass < ناپ_CLASS_گنتی; ++iclass) {
    ناپ_class_t *ناپ_class = حافظہ_ناپ_class + iclass;
    وسعت_t *وسعت = ڈھیر_2->ناپ_class[iclass].partial_وسعت;
    طالما (وسعت) {
      size_t free_گنتی = وسعت->list_ناپ;
      size_t بلاک_گنتی = ناپ_class->بلاک_گنتی;
      إذا (وسعت->free_list_حد_بندی < بلاک_گنتی)
        بلاک_گنتی = وسعت->free_list_حد_بندی;
      free_گنتی += (بلاک_گنتی - وسعت->ورتیا_گنتی);
      stats->sizecache += free_گنتی * ناپ_class->بلاک_ناپ_2;
      وسعت = وسعت->اگلا;
    }
  }

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    وسعت_cache_t *وسعت_cache;
    إذا (!iclass)
      وسعت_cache = &ڈھیر_2->وسعت_cache;
    وإلا
      وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (iclass - 1));
    stats->spancache += وسعت_cache->گنتی * (iclass + 1) * حافظہ_وسعت_ناپ;
  }
#جے_انت

  وسعت_t *deferred = (وسعت_t *)atomic_لدو_ptr(&ڈھیر_2->وسعت_free_deferred);
  طالما (deferred) {
    إذا (deferred->ناپ_class != ناپ_CLASS_HUGE)
      stats->spancache += (size_t)deferred->وسعت_گنتی * حافظہ_وسعت_ناپ;
    deferred = (وسعت_t *)deferred->free_list;
  }

#إذا ENABLE_STATISTICS
  stats->thread_منزل_global = (size_t)atomic_load64(&ڈھیر_2->thread_منزل_global);
  stats->global_منزل_thread = (size_t)atomic_load64(&ڈھیر_2->global_منزل_thread);

  لكل (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    stats->وسعت_use[iclass].current =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].current);
    stats->وسعت_use[iclass].peak =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].بلند);
    stats->وسعت_use[iclass].منزل_global =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_منزل_global);
    stats->وسعت_use[iclass].from_global =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_from_global);
    stats->وسعت_use[iclass].منزل_cache =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_منزل_cache);
    stats->وسعت_use[iclass].from_cache =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_from_cache);
    stats->وسعت_use[iclass].منزل_reserved =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_منزل_reserved);
    stats->وسعت_use[iclass].from_reserved =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_from_reserved);
    stats->وسعت_use[iclass].map_calls =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_map_calls);
  }
  لكل (size_t iclass = 0; iclass < ناپ_CLASS_گنتی; ++iclass) {
    stats->ناپ_use[iclass].alloc_current =
        (size_t)atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].alloc_current);
    stats->ناپ_use[iclass].alloc_peak =
        (size_t)ڈھیر_2->ناپ_class_use[iclass].alloc_peak;
    stats->ناپ_use[iclass].alloc_کل =
        (size_t)atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].alloc_کل);
    stats->ناپ_use[iclass].free_کل =
        (size_t)atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].free_کل);
    stats->ناپ_use[iclass].spans_منزل_cache =
        (size_t)atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].spans_منزل_cache);
    stats->ناپ_use[iclass].spans_from_cache =
        (size_t)atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].spans_from_cache);
    stats->ناپ_use[iclass].spans_from_reserved = (size_t)atomic_load32(
        &ڈھیر_2->ناپ_class_use[iclass].spans_from_reserved);
    stats->ناپ_use[iclass].map_calls =
        (size_t)atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].spans_map_calls);
  }
#جے_انت
}

فراغ rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, حجم(rpmalloc_global_statistics_t));
#إذا ENABLE_STATISTICS
  stats->mapped = (size_t)atomic_load32(&mapped_ورقے) * حافظہ_ورقہ_ناپ;
  stats->mapped_peak = (size_t)mapped_ورقے_peak * حافظہ_ورقہ_ناپ;
  stats->mapped_کل_2 =
      (size_t)atomic_load32(&mapped_کل) * حافظہ_ورقہ_ناپ;
  stats->unmapped_کل_2 =
      (size_t)atomic_load32(&unmapped_کل) * حافظہ_ورقہ_ناپ;
  stats->huge_alloc =
      (size_t)atomic_load32(&huge_ورقے_current) * حافظہ_ورقہ_ناپ;
  stats->huge_alloc_peak = (size_t)huge_ورقے_peak * حافظہ_ورقہ_ناپ;
#جے_انت
#إذا ENABLE_GLOBAL_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    global_cache_t *cache = &حافظہ_وسعت_cache[iclass];
    طالما (!atomic_cas32_acquire(&cache->جندرا, 1, 0))
      _rpmalloc_spin();
    uint32_t گنتی = cache->گنتی;
#إذا ENABLE_UNLIMITED_CACHE
    وسعت_t *current_وسعت = cache->حد_توں_ودھ_2;
    طالما (current_وسعت) {
      ++گنتی;
      current_وسعت = current_وسعت->اگلا;
    }
#جے_انت
    atomic_store32_release(&cache->جندرا, 0);
    stats->cached += گنتی * (iclass + 1) * حافظہ_وسعت_ناپ;
  }
#جے_انت
}

#إذا ENABLE_STATISTICS

ساكن فراغ حافظہ_ڈھیر_dump_statistics(ڈھیر_t *ڈھیر_2, فراغ *file) {
  fprintf(file, "Heap %d stats:\n", ڈھیر_2->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  لكل (size_t iclass = 0; iclass < ناپ_CLASS_گنتی; ++iclass) {
    إذا (!atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].alloc_کل))
      استمر;
    fprintf(
        file,
        "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu "
        "%9u\n",
        (uint32_t)iclass,
        atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].alloc_current),
        ڈھیر_2->ناپ_class_use[iclass].alloc_peak,
        atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].alloc_کل),
        atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].free_کل),
        حافظہ_ناپ_class[iclass].بلاک_ناپ_2,
        حافظہ_ناپ_class[iclass].بلاک_گنتی,
        atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].spans_current),
        ڈھیر_2->ناپ_class_use[iclass].spans_peak,
        ((size_t)ڈھیر_2->ناپ_class_use[iclass].alloc_peak *
         (size_t)حافظہ_ناپ_class[iclass].بلاک_ناپ_2) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].spans_منزل_cache) *
         حافظہ_وسعت_ناپ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].spans_from_cache) *
         حافظہ_وسعت_ناپ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(
             &ڈھیر_2->ناپ_class_use[iclass].spans_from_reserved) *
         حافظہ_وسعت_ناپ) /
            (size_t)(1024 * 1024),
        atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].spans_map_calls));
  }
  fprintf(file, "Spans  Current     Peak Deferred  PeakMiB  Cached  ToCacheMiB "
                "FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB "
                "FromGlobalMiB  MmapCalls\n");
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    إذا (!atomic_load32(&ڈھیر_2->وسعت_use[iclass].بلند) &&
        !atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_map_calls))
      استمر;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&ڈھیر_2->وسعت_use[iclass].current),
        atomic_load32(&ڈھیر_2->وسعت_use[iclass].بلند),
        atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].بلند) *
         (size_t)حافظہ_وسعت_ناپ * (iclass + 1)) /
            (size_t)(1024 * 1024),
#إذا ENABLE_THREAD_CACHE
        (بنانشانی صحيح)(!iclass ? ڈھیر_2->وسعت_cache.گنتی
                               : ڈھیر_2->وسعت_large_cache[iclass - 1].گنتی),
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_منزل_cache) *
         (iclass + 1) * حافظہ_وسعت_ناپ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_from_cache) *
         (iclass + 1) * حافظہ_وسعت_ناپ) /
            (size_t)(1024 * 1024),
#وإلا
        0, (size_t)0, (size_t)0,
#جے_انت
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_منزل_reserved) *
         (iclass + 1) * حافظہ_وسعت_ناپ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_from_reserved) *
         (iclass + 1) * حافظہ_وسعت_ناپ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_منزل_global) *
         (size_t)حافظہ_وسعت_ناپ * (iclass + 1)) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_from_global) *
         (size_t)حافظہ_وسعت_ناپ * (iclass + 1)) /
            (size_t)(1024 * 1024),
        atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_map_calls));
  }
  fprintf(file, "Full spans: %zu\n", ڈھیر_2->بھریا_وسعت_گنتی);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&ڈھیر_2->thread_منزل_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&ڈھیر_2->global_منزل_thread) / (size_t)(1024 * 1024));
}

#جے_انت

فراغ rpmalloc_dump_statistics(فراغ *file) {
#إذا ENABLE_STATISTICS
  لكل (size_t list_idx = 0; list_idx < ڈھیر_ARRAY_ناپ; ++list_idx) {
    ڈھیر_t *ڈھیر_2 = حافظہ_heaps[list_idx];
    طالما (ڈھیر_2) {
      صحيح need_dump = 0;
      لكل (size_t iclass = 0; !need_dump && (iclass < ناپ_CLASS_گنتی);
           ++iclass) {
        إذا (!atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].alloc_کل)) {
          rpmalloc_assert(
              !atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].free_کل),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&ڈھیر_2->ناپ_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          استمر;
        }
        need_dump = 1;
      }
      لكل (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_گنتی);
           ++iclass) {
        إذا (!atomic_load32(&ڈھیر_2->وسعت_use[iclass].بلند) &&
            !atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_map_calls))
          استمر;
        need_dump = 1;
      }
      إذا (need_dump)
        حافظہ_ڈھیر_dump_statistics(ڈھیر_2, file);
      ڈھیر_2 = ڈھیر_2->اگلا_ڈھیر;
    }
  }
  fprintf(file, "Global stats:\n");
  size_t huge_current =
      (size_t)atomic_load32(&huge_ورقے_current) * حافظہ_ورقہ_ناپ;
  size_t huge_peak = (size_t)huge_ورقے_peak * حافظہ_ورقہ_ناپ;
  fprintf(file, "HugeCurrentMiB HugePeakMiB\n");
  fprintf(file, "%14zu %11zu\n", huge_current / (size_t)(1024 * 1024),
          huge_peak / (size_t)(1024 * 1024));

#إذا ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    global_cache_t *cache = حافظہ_وسعت_cache + iclass;
    size_t global_cache = (size_t)cache->گنتی * iclass * حافظہ_وسعت_ناپ;

    size_t global_حد_توں_ودھ_cache = 0;
    وسعت_t *وسعت = cache->حد_توں_ودھ_2;
    طالما (وسعت) {
      global_حد_توں_ودھ_cache += iclass * حافظہ_وسعت_ناپ;
      وسعت = وسعت->اگلا;
    }
    إذا (global_cache || global_حد_توں_ودھ_cache || cache->داخل_کرو_گنتی ||
        cache->کڈھو_گنتی)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_حد_توں_ودھ_cache / (size_t)(1024 * 1024),
              cache->داخل_کرو_گنتی, cache->کڈھو_گنتی);
  }
#جے_انت

  size_t mapped = (size_t)atomic_load32(&mapped_ورقے) * حافظہ_ورقہ_ناپ;
  size_t mapped_os =
      (size_t)atomic_load32(&mapped_ورقے_os) * حافظہ_ورقہ_ناپ;
  size_t mapped_peak = (size_t)mapped_ورقے_peak * حافظہ_ورقہ_ناپ;
  size_t mapped_کل_2 =
      (size_t)atomic_load32(&mapped_کل) * حافظہ_ورقہ_ناپ;
  size_t unmapped_کل_2 =
      (size_t)atomic_load32(&unmapped_کل) * حافظہ_ورقہ_ناپ;
  fprintf(
      file,
      "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB\n");
  fprintf(file, "%9zu %11zu %13zu %14zu %16zu\n",
          mapped / (size_t)(1024 * 1024), mapped_os / (size_t)(1024 * 1024),
          mapped_peak / (size_t)(1024 * 1024),
          mapped_کل_2 / (size_t)(1024 * 1024),
          unmapped_کل_2 / (size_t)(1024 * 1024));

  fprintf(file, "\n");
#إذا 0
	int64_t allocated = atomic_load64(&ونڈ_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#جے_انت
#جے_انت
  (فراغ)حجم(file);
}

#إذا RPMALLOC_پہلا_CLASS_HEAPS

خارجي مضمن rpmalloc_ڈھیر_t *rpmalloc_ڈھیر_acquire(فراغ) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  ڈھیر_t *ڈھیر_2 = rpmalloc_ڈھیر_allocate(1);
  rpmalloc_assume(ڈھیر_2 != عدم);
  ڈھیر_2->owner_thread = 0;
  _rpmalloc_stat_inc(&حافظہ_سرگرم_heaps);
  أرجع ڈھیر_2;
}

خارجي مضمن فراغ rpmalloc_ڈھیر_release_2(rpmalloc_ڈھیر_t *ڈھیر_2) {
  إذا (ڈھیر_2)
    rpmalloc_ڈھیر_release(ڈھیر_2, 1, 1);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ڈھیر_alloc(rpmalloc_ڈھیر_t *ڈھیر_2, size_t ناپ_2) {
#إذا ENABLE_VALIDATE_دلیلاں
  إذا (ناپ_2 >= MAX_ALLOC_ناپ) {
    errno = EINVAL;
    أرجع 0;
  }
#جے_انت
  أرجع _rpmalloc_allocate(ڈھیر_2, ناپ_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ڈھیر_aligned_alloc(rpmalloc_ڈھیر_t *ڈھیر_2, size_t alignment,
                            size_t ناپ_2) {
#إذا ENABLE_VALIDATE_دلیلاں
  إذا (ناپ_2 >= MAX_ALLOC_ناپ) {
    errno = EINVAL;
    أرجع 0;
  }
#جے_انت
  أرجع _rpmalloc_aligned_allocate(ڈھیر_2, alignment, ناپ_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ڈھیر_calloc(rpmalloc_ڈھیر_t *ڈھیر_2, size_t عدد_2, size_t ناپ_2) {
  أرجع rpmalloc_ڈھیر_aligned_calloc(ڈھیر_2, 0, عدد_2, ناپ_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ڈھیر_aligned_calloc(rpmalloc_ڈھیر_t *ڈھیر_2, size_t alignment,
                             size_t عدد_2, size_t ناپ_2) {
  size_t کل_2;
#إذا ENABLE_VALIDATE_دلیلاں
#إذا PLATFORM_WINDOWS
  صحيح err = ناپ_TMult(عدد_2, ناپ_2, &کل_2);
  إذا ((err != S_OK) || (کل_2 >= MAX_ALLOC_ناپ)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(عدد_2, ناپ_2, &کل_2);
  إذا (err || (کل_2 >= MAX_ALLOC_ناپ)) {
    errno = EINVAL;
    أرجع 0;
  }
#جے_انت
#وإلا
  کل_2 = عدد_2 * ناپ_2;
#جے_انت
  فراغ *بلاک = _rpmalloc_aligned_allocate(ڈھیر_2, alignment, کل_2);
  إذا (بلاک)
    memset(بلاک, 0, کل_2);
  أرجع بلاک;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ڈھیر_realloc(rpmalloc_ڈھیر_t *ڈھیر_2, فراغ *ptr, size_t ناپ_2,
                      بنانشانی صحيح flags) {
#إذا ENABLE_VALIDATE_دلیلاں
  إذا (ناپ_2 >= MAX_ALLOC_ناپ) {
    errno = EINVAL;
    أرجع ptr;
  }
#جے_انت
  أرجع _rpmalloc_reallocate(ڈھیر_2, ptr, ناپ_2, 0, flags);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ڈھیر_aligned_realloc(rpmalloc_ڈھیر_t *ڈھیر_2, فراغ *ptr,
                              size_t alignment, size_t ناپ_2,
                              بنانشانی صحيح flags) {
#إذا ENABLE_VALIDATE_دلیلاں
  إذا ((ناپ_2 + alignment < ناپ_2) || (alignment > حافظہ_ورقہ_ناپ)) {
    errno = EINVAL;
    أرجع 0;
  }
#جے_انت
  أرجع _rpmalloc_aligned_reallocate(ڈھیر_2, ptr, alignment, ناپ_2, 0, flags);
}

خارجي مضمن فراغ rpmalloc_ڈھیر_free(rpmalloc_ڈھیر_t *ڈھیر_2, فراغ *ptr) {
  (فراغ)حجم(ڈھیر_2);
  _rpmalloc_deallocate(ptr);
}

خارجي مضمن فراغ rpmalloc_ڈھیر_free_all(rpmalloc_ڈھیر_t *ڈھیر_2) {
  وسعت_t *وسعت;
  وسعت_t *اگلا_وسعت;

  rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, 0);

  لكل (size_t iclass = 0; iclass < ناپ_CLASS_گنتی; ++iclass) {
    وسعت = ڈھیر_2->ناپ_class[iclass].partial_وسعت;
    طالما (وسعت) {
      اگلا_وسعت = وسعت->اگلا;
      rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
      وسعت = اگلا_وسعت;
    }
    ڈھیر_2->ناپ_class[iclass].partial_وسعت = 0;
    وسعت = ڈھیر_2->بھریا_وسعت[iclass];
    طالما (وسعت) {
      اگلا_وسعت = وسعت->اگلا;
      rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
      وسعت = اگلا_وسعت;
    }

    وسعت = ڈھیر_2->ناپ_class[iclass].cache;
    إذا (وسعت)
      rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
    ڈھیر_2->ناپ_class[iclass].cache = 0;
  }
  memset(ڈھیر_2->ناپ_class, 0, حجم(ڈھیر_2->ناپ_class));
  memset(ڈھیر_2->بھریا_وسعت, 0, حجم(ڈھیر_2->بھریا_وسعت));

  وسعت = ڈھیر_2->large_huge_وسعت;
  طالما (وسعت) {
    اگلا_وسعت = وسعت->اگلا;
    إذا (UNEXPECTED(وسعت->ناپ_class == ناپ_CLASS_HUGE))
      _rpmalloc_deallocate_huge(وسعت);
    وإلا
      rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
    وسعت = اگلا_وسعت;
  }
  ڈھیر_2->large_huge_وسعت = 0;
  ڈھیر_2->بھریا_وسعت_گنتی = 0;

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    وسعت_cache_t *وسعت_cache;
    إذا (!iclass)
      وسعت_cache = &ڈھیر_2->وسعت_cache;
    وإلا
      وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (iclass - 1));
    إذا (!وسعت_cache->گنتی)
      استمر;
#إذا ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&ڈھیر_2->thread_منزل_global,
                         وسعت_cache->گنتی * (iclass + 1) * حافظہ_وسعت_ناپ);
    rpmalloc_stat_جوڑو(&ڈھیر_2->وسعت_use[iclass].spans_منزل_global,
                       وسعت_cache->گنتی);
    rpmalloc_global_cache_داخل_کرو_spans(وسعت_cache->وسعت, iclass + 1,
                                        وسعت_cache->گنتی);
#وإلا
    لكل (size_t ispan = 0; ispan < وسعت_cache->گنتی; ++ispan)
      rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
#جے_انت
    وسعت_cache->گنتی = 0;
  }
#جے_انت

#إذا ENABLE_STATISTICS
  لكل (size_t iclass = 0; iclass < ناپ_CLASS_گنتی; ++iclass) {
    atomic_store32(&ڈھیر_2->ناپ_class_use[iclass].alloc_current, 0);
    atomic_store32(&ڈھیر_2->ناپ_class_use[iclass].spans_current, 0);
  }
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    atomic_store32(&ڈھیر_2->وسعت_use[iclass].current, 0);
  }
#جے_انت
}

خارجي مضمن فراغ rpmalloc_ڈھیر_thread_سیٹ_current(rpmalloc_ڈھیر_t *ڈھیر_2) {
  ڈھیر_t *prev_ڈھیر = get_thread_ڈھیر_raw();
  إذا (prev_ڈھیر != ڈھیر_2) {
    سیٹ_thread_ڈھیر(ڈھیر_2);
    إذا (prev_ڈھیر)
      rpmalloc_ڈھیر_release_2(prev_ڈھیر);
  }
}

خارجي مضمن rpmalloc_ڈھیر_t *rpmalloc_get_ڈھیر_for_ptr(فراغ *ptr) {
  // Grab the span, and then the heap from the span
  وسعت_t *وسعت = (وسعت_t *)((uintptr_t)ptr & حافظہ_وسعت_نقاب);
  إذا (وسعت) {
    أرجع وسعت->ڈھیر_2;
  }
  أرجع 0;
}

#جے_انت

#إذا ENABLE_PRELOAD || ENABLE_OVERRIDE

#ضمن "malloc.c"

#جے_انت

فراغ rpmalloc_linker_reference(فراغ) { (فراغ)حجم(_rpmalloc_initialized); }
