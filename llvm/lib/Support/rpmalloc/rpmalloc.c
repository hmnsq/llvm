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
#هدايت clang diagnostic ignored "-Wunused-macros"
#هدايت clang diagnostic ignored "-Wunused-function"
#إذا __has_warning("-Wreserved-identifier")
#هدايت clang diagnostic ignored "-Wreserved-identifier"
#جيڪڏهن_پورو
#إذا __has_warning("-Wstatic-in-inline")
#هدايت clang diagnostic ignored "-Wstatic-in-inline"
#جيڪڏهن_پورو
#وإلاإذا defined(__GNUC__)
#هدايت GCC diagnostic ignored "-Wunused-macros"
#هدايت GCC diagnostic ignored "-Wunused-function"
#جيڪڏهن_پورو

#إذا !defined(__has_builtin)
#وصف_ڪريو __has_builtin(b) 0
#جيڪڏهن_پورو

#إذا defined(__GNUC__) || defined(__clang__)

#إذا __has_builtin(__builtin_memcpy_inline)
#وصف_ڪريو _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#وإلا
#وصف_ڪريو _rpmalloc_memcpy_const(x, y, s)                                        \
  افعل {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    يادگيري_نقل(x, y, s);                                                           \
  } طالما (0)
#جيڪڏهن_پورو

#إذا __has_builtin(__builtin_memset_inline)
#وصف_ڪريو _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#وإلا
#وصف_ڪريو _rpmalloc_memset_const(x, y, s)                                        \
  افعل {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } طالما (0)
#جيڪڏهن_پورو
#وإلا
#وصف_ڪريو _rpmalloc_memcpy_const(x, y, s) يادگيري_نقل(x, y, s)
#وصف_ڪريو _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#جيڪڏهن_پورو

#إذا __has_builtin(__builtin_assume)
#وصف_ڪريو rpmalloc_assume(cond) __builtin_assume(cond)
#وإلاإذا defined(__GNUC__)
#وصف_ڪريو rpmalloc_assume(cond)                                                  \
  افعل {                                                                         \
    إذا (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } طالما (0)
#وإلاإذا defined(_MSC_VER)
#وصف_ڪريو rpmalloc_assume(cond) __assume(cond)
#وإلا
#وصف_ڪريو rpmalloc_assume(cond) 0
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل ڍير_ARRAY_سائيز
//! Size of heap hashmap
#وصف_ڪريو ڍير_ARRAY_سائيز 47
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ENABLE_THREAD_CACHE
//! Enable per-thread cache
#وصف_ڪريو ENABLE_THREAD_CACHE 1
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ENABLE_GLOBAL_CACHE
//! Enable global cache shared between all threads, requires thread cache
#وصف_ڪريو ENABLE_GLOBAL_CACHE 1
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ENABLE_VALIDATE_دليلون
//! Enable validation of args to public entry points
#وصف_ڪريو ENABLE_VALIDATE_دليلون 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ENABLE_STATISTICS
//! Enable statistics collection
#وصف_ڪريو ENABLE_STATISTICS 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ENABLE_ASSERTS
//! Enable asserts
#وصف_ڪريو ENABLE_ASSERTS 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ENABLE_OVERRIDE
//! Override standard library malloc/free and new/delete entry points
#وصف_ڪريو ENABLE_OVERRIDE 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ENABLE_PRELOAD
//! Support preloading
#وصف_ڪريو ENABLE_PRELOAD 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل DISABLE_UNMAP
//! Disable unmapping memory pages (also enables unlimited cache)
#وصف_ڪريو DISABLE_UNMAP 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ENABLE_UNLIMITED_CACHE
//! Enable unlimited global cache (no unmapping until finalization)
#وصف_ڪريو ENABLE_UNLIMITED_CACHE 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ENABLE_ADAPTIVE_THREAD_CACHE
//! Enable adaptive thread cache size based on use heuristics
#وصف_ڪريو ENABLE_ADAPTIVE_THREAD_CACHE 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل DEFAULT_وسعت_MAP_ڳڻپ
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#وصف_ڪريو DEFAULT_وسعت_MAP_ڳڻپ 64
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#وصف_ڪريو GLOBAL_CACHE_MULTIPLIER 8
#جيڪڏهن_پورو

#إذا DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#غلطي Must use global cache إذا unmap is disabled
#جيڪڏهن_پورو

#إذا DISABLE_UNMAP
#وصف_رد ENABLE_UNLIMITED_CACHE
#وصف_ڪريو ENABLE_UNLIMITED_CACHE 1
#جيڪڏهن_پورو

#إذا !ENABLE_GLOBAL_CACHE
#وصف_رد ENABLE_UNLIMITED_CACHE
#وصف_ڪريو ENABLE_UNLIMITED_CACHE 0
#جيڪڏهن_پورو

#إذا !ENABLE_THREAD_CACHE
#وصف_رد ENABLE_ADAPTIVE_THREAD_CACHE
#وصف_ڪريو ENABLE_ADAPTIVE_THREAD_CACHE 0
#جيڪڏهن_پورو

#إذا defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#وصف_ڪريو PLATFORM_WINDOWS 1
#وصف_ڪريو PLATFORM_POSIX 0
#وإلا
#وصف_ڪريو PLATFORM_WINDOWS 0
#وصف_ڪريو PLATFORM_POSIX 1
#جيڪڏهن_پورو

/// Platform and arch specifics
#إذا defined(_MSC_VER) && !defined(__clang__)
#هدايت warning(disable : 5105)
#جيڪڏهن_وصف_نه_ٿيل FORCEINLINE
#وصف_ڪريو FORCEINLINE مضمن __forceinline
#جيڪڏهن_پورو
#وصف_ڪريو _Static_assert static_assert
#وإلا
#جيڪڏهن_وصف_نه_ٿيل FORCEINLINE
#وصف_ڪريو FORCEINLINE مضمن __attribute__((__always_inline__))
#جيڪڏهن_پورو
#جيڪڏهن_پورو
#إذا PLATFORM_WINDOWS
#جيڪڏهن_وصف_نه_ٿيل WIN32_LEAN_AND_سراسري
#وصف_ڪريو WIN32_LEAN_AND_سراسري
#جيڪڏهن_پورو
#ضمن <windows.h>
#إذا ENABLE_VALIDATE_دليلون
#ضمن <intsafe.h>
#جيڪڏهن_پورو
#وإلا
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <time.h>
#ضمن <unistd.h>
#إذا defined(__linux__) || defined(__ANDROID__)
#ضمن <sys/prctl.h>
#إذا !defined(PR_سيٽ_VMA)
#وصف_ڪريو PR_سيٽ_VMA 0x53564d41
#وصف_ڪريو PR_سيٽ_VMA_ANON_NAME 0
#جيڪڏهن_پورو
#جيڪڏهن_پورو
#إذا defined(__APPLE__)
#ضمن <TargetConditionals.h>
#إذا !هدف_OS_IPHONE && !هدف_OS_SIMULATOR
#ضمن <mach/mach_vm.h>
#ضمن <mach/vm_statistics.h>
#جيڪڏهن_پورو
#ضمن <pthread.h>
#جيڪڏهن_پورو
#إذا defined(__HAIKU__) || defined(__TINYC__)
#ضمن <pthread.h>
#جيڪڏهن_پورو
#جيڪڏهن_پورو

#ضمن <errno.h>
#ضمن <stdint.h>
#ضمن <string.h>

#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#ضمن <fibersapi.h>
ساكن DWORD fls_ڪنجي;
#جيڪڏهن_پورو

#إذا PLATFORM_POSIX
#ضمن <sched.h>
#ضمن <sys/mman.h>
#جيڪڏهن_وصف_ٿيل __FreeBSD__
#ضمن <sys/sysctl.h>
#وصف_ڪريو MAP_HUGETLB MAP_ALIGNED_SUPER
#جيڪڏهن_وصف_نه_ٿيل PROT_MAX
#وصف_ڪريو PROT_MAX(f) 0
#جيڪڏهن_پورو
#وإلا
#وصف_ڪريو PROT_MAX(f) 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل __sun
خارجي صحيح madvise(caddr_t, size_t, صحيح);
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل MAP_UNINITIALIZED
#وصف_ڪريو MAP_UNINITIALIZED 0
#جيڪڏهن_پورو
#جيڪڏهن_پورو
#ضمن <errno.h>

#إذا ENABLE_ASSERTS
#وصف_رد NDEBUG
#إذا defined(_MSC_VER) && !defined(_DEBUG)
#وصف_ڪريو _DEBUG
#جيڪڏهن_پورو
#ضمن <assert.h>
#وصف_ڪريو RPMALLOC_TOSTRING_M(x) #x
#وصف_ڪريو RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#وصف_ڪريو rpmalloc_assert(truth, پيغام)                                        \
  افعل {                                                                         \
    إذا (!(truth)) {                                                            \
      إذا (يادگيري_config.error_callback) {                                     \
        يادگيري_config.error_callback(پيغام " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } وإلا {                                                                 \
        أكد((truth) && پيغام);                                            \
      }                                                                        \
    }                                                                          \
  } طالما (0)
#وإلا
#وصف_ڪريو rpmalloc_assert(truth, پيغام)                                        \
  افعل {                                                                         \
  } طالما (0)
#جيڪڏهن_پورو
#إذا ENABLE_STATISTICS
#ضمن <stdio.h>
#جيڪڏهن_پورو

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#إذا defined(_MSC_VER) && !defined(__clang__)

تعريفنوع متطاير ڊگهو atomic32_t;
تعريفنوع متطاير ڊگهو ڊگهو atomic64_t;
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
ساكن FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t جوڙيو) {
  أرجع (int32_t)Interlocked_Exchange_جوڙيو(val, جوڙيو) + جوڙيو;
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
ساكن FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t جوڙيو) {
  أرجع (int64_t)InterlockedExchangeAdd64(val, جوڙيو) + جوڙيو;
}
ساكن FORCEINLINE فراغ *atomic_ڀريو_ptr(atomicptr_t *src) {
  أرجع Interlocked_Compare_Exchange_اشارو(src, 0, 0);
}
ساكن FORCEINLINE فراغ atomic_store_ptr(atomicptr_t *dst, فراغ *val) {
  Interlocked_Exchange_اشارو(dst, val);
}
ساكن FORCEINLINE فراغ atomic_store_ptr_release(atomicptr_t *dst, فراغ *val) {
  Interlocked_Exchange_اشارو(dst, val);
}
ساكن FORCEINLINE فراغ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     فراغ *val) {
  أرجع (فراغ *)Interlocked_Exchange_اشارو((فراغ *متطاير *)dst, val);
}
ساكن FORCEINLINE صحيح atomic_cas_ptr(atomicptr_t *dst, فراغ *val, فراغ *ref) {
  أرجع (Interlocked_Compare_Exchange_اشارو((فراغ *متطاير *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#وصف_ڪريو EXPECTED(x) (x)
#وصف_ڪريو UNEXPECTED(x) (x)

#وإلا

#ضمن <stdatomic.h>

تعريفنوع متطاير _Atomic(int32_t) atomic32_t;
تعريفنوع متطاير _Atomic(int64_t) atomic64_t;
تعريفنوع متطاير _Atomic(فراغ *) atomicptr_t;

ساكن FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  أرجع atomic_ڀريو_explicit(src, memory_order_relaxed);
}
ساكن FORCEINLINE فراغ atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ساكن FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  أرجع atomic_fetch_جوڙيو_explicit(val, 1, memory_order_relaxed) + 1;
}
ساكن FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  أرجع atomic_fetch_جوڙيو_explicit(val, -1, memory_order_relaxed) - 1;
}
ساكن FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t جوڙيو) {
  أرجع atomic_fetch_جوڙيو_explicit(val, جوڙيو, memory_order_relaxed) + جوڙيو;
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
  أرجع atomic_ڀريو_explicit(val, memory_order_relaxed);
}
ساكن FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t جوڙيو) {
  أرجع atomic_fetch_جوڙيو_explicit(val, جوڙيو, memory_order_relaxed) + جوڙيو;
}
ساكن FORCEINLINE فراغ *atomic_ڀريو_ptr(atomicptr_t *src) {
  أرجع atomic_ڀريو_explicit(src, memory_order_relaxed);
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

#وصف_ڪريو EXPECTED(x) __builtin_expect((x), 1)
#وصف_ڪريو UNEXPECTED(x) __builtin_expect((x), 0)

#جيڪڏهن_پورو

////////////
///
/// Statistics related functions (evaluate to nothing when statistics not
/// enabled)
///
//////

#إذا ENABLE_STATISTICS
#وصف_ڪريو _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#وصف_ڪريو _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#وصف_ڪريو rpmalloc_stat_جوڙيو(counter, قدر_2)                                     \
  atomic_add32(counter, (int32_t)(قدر_2))
#وصف_ڪريو _rpmalloc_stat_add64(counter, قدر_2)                                   \
  atomic_add64(counter, (int64_t)(قدر_2))
#وصف_ڪريو rpmalloc_stat_جوڙيو_peak(counter, قدر_2, peak)                          \
  افعل {                                                                         \
    int32_t cur_ڳڻپ = atomic_add32(counter, (int32_t)(قدر_2));              \
    إذا (cur_ڳڻپ > (peak))                                                   \
      peak = cur_ڳڻپ;                                                       \
  } طالما (0)
#وصف_ڪريو _rpmalloc_stat_sub(counter, قدر_2)                                     \
  atomic_add32(counter, -(int32_t)(قدر_2))
#وصف_ڪريو _rpmalloc_stat_inc_alloc(ڍير_2, class_idx)                              \
  افعل {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&ڍير_2->سائيز_class_use[class_idx].alloc_current);         \
    إذا (alloc_current > ڍير_2->سائيز_class_use[class_idx].alloc_peak)            \
      ڍير_2->سائيز_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&ڍير_2->سائيز_class_use[class_idx].alloc_ڪل);               \
  } طالما (0)
#وصف_ڪريو _rpmalloc_stat_inc_free(ڍير_2, class_idx)                               \
  افعل {                                                                         \
    atomic_decr32(&ڍير_2->سائيز_class_use[class_idx].alloc_current);             \
    atomic_incr32(&ڍير_2->سائيز_class_use[class_idx].free_ڪل);                \
  } طالما (0)
#وإلا
#وصف_ڪريو _rpmalloc_stat_inc(counter)                                            \
  افعل {                                                                         \
  } طالما (0)
#وصف_ڪريو _rpmalloc_stat_dec(counter)                                            \
  افعل {                                                                         \
  } طالما (0)
#وصف_ڪريو rpmalloc_stat_جوڙيو(counter, قدر_2)                                     \
  افعل {                                                                         \
  } طالما (0)
#وصف_ڪريو _rpmalloc_stat_add64(counter, قدر_2)                                   \
  افعل {                                                                         \
  } طالما (0)
#وصف_ڪريو rpmalloc_stat_جوڙيو_peak(counter, قدر_2, peak)                          \
  افعل {                                                                         \
  } طالما (0)
#وصف_ڪريو _rpmalloc_stat_sub(counter, قدر_2)                                     \
  افعل {                                                                         \
  } طالما (0)
#وصف_ڪريو _rpmalloc_stat_inc_alloc(ڍير_2, class_idx)                              \
  افعل {                                                                         \
  } طالما (0)
#وصف_ڪريو _rpmalloc_stat_inc_free(ڍير_2, class_idx)                               \
  افعل {                                                                         \
  } طالما (0)
#جيڪڏهن_پورو

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#وصف_ڪريو SMALL_GRANULARITY 16
//! Small granularity shift count
#وصف_ڪريو SMALL_GRANULARITY_سرڪايو 4
//! Number of small block size classes
#وصف_ڪريو SMALL_CLASS_ڳڻپ 65
//! Maximum size of a small block
#وصف_ڪريو SMALL_سائيز_حد_بندي (SMALL_GRANULARITY * (SMALL_CLASS_ڳڻپ - 1))
//! Granularity of a medium allocation block
#وصف_ڪريو MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#وصف_ڪريو MEDIUM_GRANULARITY_سرڪايو 9
//! Number of medium block size classes
#وصف_ڪريو MEDIUM_CLASS_ڳڻپ 61
//! Total number of small + medium size classes
#وصف_ڪريو سائيز_CLASS_ڳڻپ (SMALL_CLASS_ڳڻپ + MEDIUM_CLASS_ڳڻپ)
//! Number of large block size classes
#وصف_ڪريو LARGE_CLASS_ڳڻپ 63
//! Maximum size of a medium block
#وصف_ڪريو MEDIUM_سائيز_حد_بندي                                                      \
  (SMALL_سائيز_حد_بندي + (MEDIUM_GRANULARITY * MEDIUM_CLASS_ڳڻپ))
//! Maximum size of a large block
#وصف_ڪريو LARGE_سائيز_حد_بندي                                                       \
  ((LARGE_CLASS_ڳڻپ * يادگيري_وسعت_سائيز) - وسعت_سرو_سائيز)
//! Size of a span header (must be a multiple of SMALL_GRANULARITY and a power
//! of two)
#وصف_ڪريو وسعت_سرو_سائيز 128
//! Number of spans in thread cache
#وصف_ڪريو MAX_THREAD_وسعت_CACHE 400
//! Number of spans to transfer between thread and global cache
#وصف_ڪريو THREAD_وسعت_CACHE_TRANSFER 64
//! Number of spans in thread cache for large spans (must be greater than
//! LARGE_CLASS_COUNT / 2)
#وصف_ڪريو MAX_THREAD_وسعت_LARGE_CACHE 100
//! Number of spans to transfer between thread and global cache for large spans
#وصف_ڪريو THREAD_وسعت_LARGE_CACHE_TRANSFER 6

_Static_assert((SMALL_GRANULARITY & (SMALL_GRANULARITY - 1)) == 0,
               "Small granularity must be power of two");
_Static_assert((وسعت_سرو_سائيز & (وسعت_سرو_سائيز - 1)) == 0,
               "Span header size must be power of two");

#إذا ENABLE_VALIDATE_دليلون
//! Maximum allocation size to avoid integer overflow
#وصف_رد MAX_ALLOC_سائيز
#وصف_ڪريو MAX_ALLOC_سائيز (((size_t) - 1) - يادگيري_وسعت_سائيز)
#جيڪڏهن_پورو

#وصف_ڪريو اشارو_وٿي(ptr, ofs) (فراغ *)((حرف *)(ptr) + (ptrdiff_t)(ofs))
#وصف_ڪريو اشارو_diff(پهريون_2, ٻيو_2)                                            \
  (ptrdiff_t)((ثابت حرف *)(پهريون_2) - (ثابت حرف *)(ٻيو_2))

#وصف_ڪريو INVALID_اشارو ((فراغ *)((uintptr_t) - 1))

#وصف_ڪريو سائيز_CLASS_LARGE سائيز_CLASS_ڳڻپ
#وصف_ڪريو سائيز_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
تعريفنوع بنية ڍير_t ڍير_t;
//! Span of memory pages
تعريفنوع بنية وسعت_t وسعت_t;
//! Span list
تعريفنوع بنية وسعت_list_t وسعت_list_t;
//! Span active data
تعريفنوع بنية وسعت_سرگرم_t وسعت_سرگرم_t;
//! Size class definition
تعريفنوع بنية سائيز_class_t سائيز_class_t;
//! Global cache
تعريفنوع بنية global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#وصف_ڪريو وسعت_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#وصف_ڪريو وسعت_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#وصف_ڪريو وسعت_FLAG_ALIGNED_بلاڪون 4U
//! Flag indicating an unmapped master span
#وصف_ڪريو وسعت_FLAG_UNMAPPED_MASTER 8U

#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
بنية وسعت_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t مٿاهون;
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
#جيڪڏهن_پورو
};
تعريفنوع بنية وسعت_use_t وسعت_use_t;
#جيڪڏهن_پورو

#إذا ENABLE_STATISTICS
بنية سائيز_class_use_t {
  //! Current number of allocations
  atomic32_t alloc_current;
  //! Peak number of allocations
  int32_t alloc_peak;
  //! Total number of allocations
  atomic32_t alloc_ڪل;
  //! Total number of frees
  atomic32_t free_ڪل;
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
  int32_t اڻ_استعمال_ٿيل_2;
};
تعريفنوع بنية سائيز_class_use_t سائيز_class_use_t;
#جيڪڏهن_پورو

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
  uint32_t بلاڪ_ڳڻپ;
  //! Size class
  uint32_t سائيز_class;
  //! Index of last block initialized in free list
  uint32_t free_list_حد_بندي;
  //! Number of used blocks remaining when in partial state
  uint32_t استعمال_ٿيل_ڳڻپ;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_سائيز;
  //! Size of a block
  uint32_t بلاڪ_سائيز_2;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t وسعت_ڳڻپ;
  //! Total span counter for master spans
  uint32_t ڪل_spans;
  //! Offset from master span for subspans
  uint32_t وٿي_from_master;
  //! Remaining span counter, for master spans
  atomic32_t بچيل_spans;
  //! Alignment offset
  uint32_t align_وٿي;
  //! Owning heap
  ڍير_t *ڍير_2;
  //! Next span
  وسعت_t *ايندڙ;
  //! Previous span
  وسعت_t *prev;
};
_Static_assert(حجم(وسعت_t) <= وسعت_سرو_سائيز, "span size mismatch");

بنية وسعت_cache_t {
  size_t ڳڻپ;
  وسعت_t *وسعت[MAX_THREAD_وسعت_CACHE];
};
تعريفنوع بنية وسعت_cache_t وسعت_cache_t;

بنية وسعت_large_cache_t {
  size_t ڳڻپ;
  وسعت_t *وسعت[MAX_THREAD_وسعت_LARGE_CACHE];
};
تعريفنوع بنية وسعت_large_cache_t وسعت_large_cache_t;

بنية ڍير_سائيز_class_t {
  //! Free list of active span
  فراغ *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  وسعت_t *partial_وسعت;
  //! Early level cache of fully free spans
  وسعت_t *cache;
};
تعريفنوع بنية ڍير_سائيز_class_t ڍير_سائيز_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
بنية ڍير_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  ڍير_سائيز_class_t سائيز_class[سائيز_CLASS_ڳڻپ];
#إذا ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  وسعت_cache_t وسعت_cache;
#جيڪڏهن_پورو
  //! List of deferred free spans (single linked list)
  atomicptr_t وسعت_free_deferred;
  //! Number of full spans
  size_t ڀريل_وسعت_ڳڻپ;
  //! Mapped but unused spans
  وسعت_t *وسعت_reserve;
  //! Master span for mapped but unused spans
  وسعت_t *وسعت_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t ٻار_ڳڻپ;
  //! Next heap in id list
  ڍير_t *ايندڙ_ڍير;
  //! Next heap in orphan list
  ڍير_t *ايندڙ_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  صحيح finalize;
  //! Master heap owning the memory pages
  ڍير_t *master_ڍير;
#إذا ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  وسعت_large_cache_t وسعت_large_cache[LARGE_CLASS_ڳڻپ - 1];
#جيڪڏهن_پورو
#إذا RPMALLOC_پهريون_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  وسعت_t *ڀريل_وسعت[سائيز_CLASS_ڳڻپ];
  //! Double linked list of large and huge spans allocated by this heap
  وسعت_t *large_huge_وسعت;
#جيڪڏهن_پورو
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  وسعت_use_t وسعت_use[LARGE_CLASS_ڳڻپ];
#جيڪڏهن_پورو
#إذا ENABLE_STATISTICS
  //! Allocation stats per size class
  سائيز_class_use_t سائيز_class_use[سائيز_CLASS_ڳڻپ + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_منزل_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_منزل_thread;
#جيڪڏهن_پورو
};

// Size class for defining a block size bucket
بنية سائيز_class_t {
  //! Size of blocks in this class
  uint32_t بلاڪ_سائيز_2;
  //! Number of blocks in each chunk
  uint16_t بلاڪ_ڳڻپ;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(حجم(سائيز_class_t) == 8, "Size class size mismatch");

بنية global_cache_t {
  //! Cache lock
  atomic32_t تالو;
  //! Cache count
  uint32_t ڳڻپ;
#إذا ENABLE_STATISTICS
  //! Insert count
  size_t وجهو_ڳڻپ;
  //! Extract count
  size_t ڪڍو_ڳڻپ_2;
#جيڪڏهن_پورو
  //! Cached spans
  وسعت_t *وسعت[GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_وسعت_CACHE];
  //! Unlimited cache overflow
  وسعت_t *حد_کان_مٿي_2;
};

////////////
///
/// Global data
///
//////

//! Default span size (64KiB)
#وصف_ڪريو يادگيري_default_وسعت_سائيز (64 * 1024)
#وصف_ڪريو يادگيري_default_وسعت_سائيز_سرڪايو 16
#وصف_ڪريو يادگيري_default_وسعت_نقاب (~((uintptr_t)(يادگيري_وسعت_سائيز - 1)))

//! Initialized flag
ساكن صحيح _rpmalloc_initialized;
//! Main thread ID
ساكن uintptr_t _rpmalloc_main_thread_id;
//! Configuration
ساكن rpmalloc_config_t يادگيري_config;
//! Memory page size
ساكن size_t يادگيري_صفحو_سائيز;
//! Shift to divide by page size
ساكن size_t يادگيري_صفحو_سائيز_سرڪايو;
//! Granularity at which memory pages are mapped by OS
ساكن size_t يادگيري_map_granularity;
#إذا RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
ساكن size_t يادگيري_وسعت_سائيز;
//! Shift to divide by span size
ساكن size_t يادگيري_وسعت_سائيز_سرڪايو;
//! Mask to get to start of a memory span
ساكن uintptr_t يادگيري_وسعت_نقاب;
#وإلا
//! Hardwired span size
#وصف_ڪريو يادگيري_وسعت_سائيز يادگيري_default_وسعت_سائيز
#وصف_ڪريو يادگيري_وسعت_سائيز_سرڪايو يادگيري_default_وسعت_سائيز_سرڪايو
#وصف_ڪريو يادگيري_وسعت_نقاب يادگيري_default_وسعت_نقاب
#جيڪڏهن_پورو
//! Number of spans to map in each map call
ساكن size_t يادگيري_وسعت_map_ڳڻپ;
//! Number of spans to keep reserved in each heap
ساكن size_t يادگيري_ڍير_reserve_ڳڻپ;
//! Global size classes
ساكن سائيز_class_t يادگيري_سائيز_class[سائيز_CLASS_ڳڻپ];
//! Run-time size limit of medium blocks
ساكن size_t يادگيري_medium_سائيز_حد_بندي;
//! Heap ID counter
ساكن atomic32_t يادگيري_ڍير_id;
//! Huge page support
ساكن صحيح يادگيري_huge_صفحا;
#إذا ENABLE_GLOBAL_CACHE
//! Global span cache
ساكن global_cache_t يادگيري_وسعت_cache[LARGE_CLASS_ڳڻپ];
#جيڪڏهن_پورو
//! Global reserved spans
ساكن وسعت_t *يادگيري_global_reserve;
//! Global reserved count
ساكن size_t يادگيري_global_reserve_ڳڻپ;
//! Global reserved master
ساكن وسعت_t *يادگيري_global_reserve_master;
//! All heaps
ساكن ڍير_t *يادگيري_heaps[ڍير_ARRAY_سائيز];
//! Used to restrict access to mapping memory for huge pages
ساكن atomic32_t يادگيري_global_تالو;
//! Orphaned heaps
ساكن ڍير_t *يادگيري_orphan_heaps;
#إذا RPMALLOC_پهريون_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
ساكن ڍير_t *يادگيري_پهريون_class_orphan_heaps;
#جيڪڏهن_پورو
#إذا ENABLE_STATISTICS
//! Allocations counter
ساكن atomic64_t ورهاست_counter;
//! Deallocations counter
ساكن atomic64_t _deallocation_counter;
//! Active heap count
ساكن atomic32_t يادگيري_سرگرم_heaps;
//! Number of currently mapped memory pages
ساكن atomic32_t mapped_صفحا;
//! Peak number of concurrently mapped memory pages
ساكن int32_t mapped_صفحا_peak;
//! Number of mapped master spans
ساكن atomic32_t _master_spans;
//! Number of unmapped dangling master spans
ساكن atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
ساكن atomic32_t mapped_ڪل;
//! Running counter of total number of unmapped memory pages since start
ساكن atomic32_t unmapped_ڪل;
//! Number of currently mapped memory pages in OS calls
ساكن atomic32_t mapped_صفحا_os;
//! Number of currently allocated pages in huge allocations
ساكن atomic32_t huge_صفحا_current;
//! Peak number of currently allocated pages in huge allocations
ساكن int32_t huge_صفحا_peak;
#جيڪڏهن_پورو

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
ساكن pthread_key_t يادگيري_thread_ڍير;
#وإلا
#جيڪڏهن_وصف_ٿيل _MSC_VER
#وصف_ڪريو _Thread_local __declspec(thread)
#وصف_ڪريو TLS_MODEL
#وإلا
#جيڪڏهن_وصف_نه_ٿيل __HAIKU__
#وصف_ڪريو TLS_MODEL __attribute__((tls_model("initial-exec")))
#وإلا
#وصف_ڪريو TLS_MODEL
#جيڪڏهن_پورو
#إذا !defined(__clang__) && defined(__GNUC__)
#وصف_ڪريو _Thread_local __thread
#جيڪڏهن_پورو
#جيڪڏهن_پورو
ساكن _Thread_local ڍير_t *يادگيري_thread_ڍير TLS_MODEL;
#جيڪڏهن_پورو

ساكن مضمن ڍير_t *get_thread_ڍير_raw(فراغ) {
#إذا (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  أرجع pthread_getspecific(يادگيري_thread_ڍير);
#وإلا
  أرجع يادگيري_thread_ڍير;
#جيڪڏهن_پورو
}

//! Get the current thread heap
ساكن مضمن ڍير_t *get_thread_ڍير(فراغ) {
  ڍير_t *ڍير_2 = get_thread_ڍير_raw();
#إذا ENABLE_PRELOAD
  إذا (EXPECTED(ڍير_2 != 0))
    أرجع ڍير_2;
  rpmalloc_initialize();
  أرجع get_thread_ڍير_raw();
#وإلا
  أرجع ڍير_2;
#جيڪڏهن_پورو
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
#جيڪڏهن_پورو
#وإلاإذا defined(__arm__)
  __asm__ متطاير("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#وإلاإذا defined(__aarch64__)
#إذا defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ متطاير("mrs %0, tpidrro_el0" : "=r"(tid));
#وإلا
  __asm__ متطاير("mrs %0, tpidr_el0" : "=r"(tid));
#جيڪڏهن_پورو
#وإلا
#غلطي This platform needs implementation جو_3 get_thread_id()
#جيڪڏهن_پورو
  أرجع tid;
#وإلا
#غلطي This platform needs implementation جو_3 get_thread_id()
#جيڪڏهن_پورو
}

//! Set the current thread heap
ساكن فراغ سيٽ_thread_ڍير(ڍير_t *ڍير_2) {
#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(يادگيري_thread_ڍير, ڍير_2);
#وإلا
  يادگيري_thread_ڍير = ڍير_2;
#جيڪڏهن_پورو
  إذا (ڍير_2)
    ڍير_2->owner_thread = get_thread_id();
}

//! Set main thread ID
خارجي فراغ rpmalloc_سيٽ_main_thread(فراغ);

فراغ rpmalloc_سيٽ_main_thread(فراغ) {
  _rpmalloc_main_thread_id = get_thread_id();
}

ساكن فراغ _rpmalloc_spin(فراغ) {
#إذا defined(_MSC_VER)
#إذا defined(_M_ARM64)
  __yield();
#وإلا
  _mm_pause();
#جيڪڏهن_پورو
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
#جيڪڏهن_پورو
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

#جيڪڏهن_وصف_ٿيل _WIN64
#هدايت comment(linker, "/INCLUDE:_tls_used")
#هدايت comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#هدايت const_seg(".CRT$XLY")

خارجي ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#هدايت const_seg()
#وإلا // _WIN64
#هدايت comment(linker, "/INCLUDE:__tls_used")
#هدايت comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#هدايت معلومات_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#هدايت معلومات_seg()
#جيڪڏهن_پورو // _WIN64

ساكن فراغ NTAPI _rpmalloc_thread_destructor(فراغ *قدر_2) {
#إذا ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  إذا (get_thread_id() == _rpmalloc_main_thread_id)
    أرجع;
#جيڪڏهن_پورو
  إذا (قدر_2)
    rpmalloc_thread_finalize(1);
}
#جيڪڏهن_پورو

////////////
///
/// Low level memory map/unmap
///
//////

ساكن فراغ rpmalloc_سيٽ_name(فراغ *پتو_2, size_t سائيز_2) {
#إذا defined(__linux__) || defined(__ANDROID__)
  ثابت حرف *name = يادگيري_huge_صفحا ? يادگيري_config.huge_صفحو_name
                                        : يادگيري_config.صفحو_name;
  إذا (پتو_2 == MAP_FAILED || !name)
    أرجع;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (فراغ)prctl(PR_سيٽ_VMA, PR_سيٽ_VMA_ANON_NAME, (uintptr_t)پتو_2, سائيز_2,
              (uintptr_t)name);
#وإلا
  (فراغ)حجم(سائيز_2);
  (فراغ)حجم(پتو_2);
#جيڪڏهن_پورو
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
ساكن فراغ *_rpmalloc_mmap(size_t سائيز_2, size_t *وٿي_2) {
  rpmalloc_assert(!(سائيز_2 % يادگيري_صفحو_سائيز), "Invalid mmap size");
  rpmalloc_assert(سائيز_2 >= يادگيري_صفحو_سائيز, "Invalid mmap size");
  فراغ *پتو_2 = يادگيري_config.يادگيري_map(سائيز_2, وٿي_2);
  إذا (EXPECTED(پتو_2 != 0)) {
    rpmalloc_stat_جوڙيو_peak(&mapped_صفحا, (سائيز_2 >> يادگيري_صفحو_سائيز_سرڪايو),
                            mapped_صفحا_peak);
    rpmalloc_stat_جوڙيو(&mapped_ڪل, (سائيز_2 >> يادگيري_صفحو_سائيز_سرڪايو));
  }
  أرجع پتو_2;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
ساكن فراغ _rpmalloc_unmap(فراغ *پتو_2, size_t سائيز_2, size_t وٿي_2,
                            size_t release) {
  rpmalloc_assert(!release || (release >= سائيز_2), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= يادگيري_صفحو_سائيز),
                  "Invalid unmap size");
  إذا (release) {
    rpmalloc_assert(!(release % يادگيري_صفحو_سائيز), "Invalid unmap size");
    _rpmalloc_stat_sub(&mapped_صفحا, (release >> يادگيري_صفحو_سائيز_سرڪايو));
    rpmalloc_stat_جوڙيو(&unmapped_ڪل, (release >> يادگيري_صفحو_سائيز_سرڪايو));
  }
  يادگيري_config.يادگيري_unmap(پتو_2, سائيز_2, وٿي_2, release);
}

//! Default implementation to map new pages to virtual memory
ساكن فراغ *_rpmalloc_mmap_os(size_t سائيز_2, size_t *وٿي_2) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((سائيز_2 >= يادگيري_وسعت_سائيز) &&
                    (يادگيري_وسعت_سائيز > يادگيري_map_granularity))
                       ? يادگيري_وسعت_سائيز
                       : 0;
  rpmalloc_assert(سائيز_2 >= يادگيري_صفحو_سائيز, "Invalid mmap size");
#إذا PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  فراغ *ptr = VirtualAlloc(0, سائيز_2 + padding,
                           (يادگيري_huge_صفحا ? MEM_LARGE_صفحا : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           صفحو_READWRITE);
  إذا (!ptr) {
    إذا (يادگيري_config.map_fail_callback) {
      إذا (يادگيري_config.map_fail_callback(سائيز_2 + padding))
        أرجع _rpmalloc_mmap_os(سائيز_2, وٿي_2);
    } وإلا {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    أرجع 0;
  }
#وإلا
  صحيح flags = MAP_ذاتي | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#إذا defined(__APPLE__) && !هدف_OS_IPHONE && !هدف_OS_SIMULATOR
  صحيح fd = (صحيح)VM_MAKE_ٽيگ(240U);
  إذا (يادگيري_huge_صفحا)
    fd |= VM_FLAGS_SUPERPAGE_سائيز_2_MB;
  فراغ *ptr = mmap(0, سائيز_2 + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#وإلاإذا defined(MAP_HUGETLB)
  فراغ *ptr = mmap(0, سائيز_2 + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (يادگيري_huge_صفحا ? MAP_HUGETLB : 0) | flags, -1, 0);
#إذا defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  إذا ((ptr == MAP_FAILED || !ptr) && يادگيري_huge_صفحا) {
    ptr = mmap(0, سائيز_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    إذا (ptr && ptr != MAP_FAILED) {
      صحيح prm = madvise(ptr, سائيز_2 + padding, MADV_HUGEPAGE);
      (فراغ)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#جيڪڏهن_پورو
  rpmalloc_سيٽ_name(ptr, سائيز_2 + padding);
#وإلاإذا defined(MAP_ALIGNED)
  ثابت size_t align =
      (حجم(size_t) * 8) - (size_t)(__builtin_clzl(سائيز_2 - 1));
  فراغ *ptr =
      mmap(0, سائيز_2 + padding, PROT_READ | PROT_WRITE,
           (يادگيري_huge_صفحا ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#وإلاإذا defined(MAP_ALIGN)
  caddr_t بنياد_2 = (يادگيري_huge_صفحا ? (caddr_t)(4 << 20) : 0);
  فراغ *ptr = mmap(بنياد_2, سائيز_2 + padding, PROT_READ | PROT_WRITE,
                   (يادگيري_huge_صفحا ? MAP_ALIGN : 0) | flags, -1, 0);
#وإلا
  فراغ *ptr = mmap(0, سائيز_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#جيڪڏهن_پورو
  إذا ((ptr == MAP_FAILED) || !ptr) {
    إذا (يادگيري_config.map_fail_callback) {
      إذا (يادگيري_config.map_fail_callback(سائيز_2 + padding))
        أرجع _rpmalloc_mmap_os(سائيز_2, وٿي_2);
    } وإلا إذا (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    أرجع 0;
  }
#جيڪڏهن_پورو
  rpmalloc_stat_جوڙيو(&mapped_صفحا_os,
                     (int32_t)((سائيز_2 + padding) >> يادگيري_صفحو_سائيز_سرڪايو));
  إذا (padding) {
    size_t final_padding = padding - ((uintptr_t)ptr & ~يادگيري_وسعت_نقاب);
    rpmalloc_assert(final_padding <= يادگيري_وسعت_سائيز,
                    "Internal failure in padding");
    rpmalloc_assert(final_padding <= padding, "Internal failure in padding");
    rpmalloc_assert(!(final_padding % 8), "Internal failure in padding");
    ptr = اشارو_وٿي(ptr, final_padding);
    *وٿي_2 = final_padding >> 3;
  }
  rpmalloc_assert((سائيز_2 < يادگيري_وسعت_سائيز) ||
                      !((uintptr_t)ptr & ~يادگيري_وسعت_نقاب),
                  "Internal failure in padding");
  أرجع ptr;
}

//! Default implementation to unmap pages from virtual memory
ساكن فراغ _rpmalloc_unmap_os(فراغ *پتو_2, size_t سائيز_2, size_t وٿي_2,
                               size_t release) {
  rpmalloc_assert(release || (وٿي_2 == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= يادگيري_صفحو_سائيز),
                  "Invalid unmap size");
  rpmalloc_assert(سائيز_2 >= يادگيري_صفحو_سائيز, "Invalid unmap size");
  إذا (release && وٿي_2) {
    وٿي_2 <<= 3;
    پتو_2 = اشارو_وٿي(پتو_2, -(int32_t)وٿي_2);
    إذا ((release >= يادگيري_وسعت_سائيز) &&
        (يادگيري_وسعت_سائيز > يادگيري_map_granularity)) {
      // Padding is always one span size
      release += يادگيري_وسعت_سائيز;
    }
  }
#إذا !DISABLE_UNMAP
#إذا PLATFORM_WINDOWS
  إذا (!VirtualFree(پتو_2, release ? 0 : سائيز_2,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#وإلا
  إذا (release) {
    إذا (munmap(پتو_2, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } وإلا {
#إذا defined(MADV_FREE_REUSABLE)
    صحيح ret;
    طالما ((ret = madvise(پتو_2, سائيز_2, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    إذا ((ret == -1) && (errno != 0)) {
#وإلاإذا defined(MADV_DONTNEED)
    إذا (madvise(پتو_2, سائيز_2, MADV_DONTNEED)) {
#وإلاإذا defined(MADV_PAGEOUT)
    إذا (madvise(پتو_2, سائيز_2, MADV_PAGEOUT)) {
#وإلاإذا defined(MADV_FREE)
    إذا (madvise(پتو_2, سائيز_2, MADV_FREE)) {
#وإلا
    إذا (posix_madvise(پتو_2, سائيز_2, POSIX_MADV_DONTNEED)) {
#جيڪڏهن_پورو
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#جيڪڏهن_پورو
#جيڪڏهن_پورو
  إذا (release)
    _rpmalloc_stat_sub(&mapped_صفحا_os, release >> يادگيري_صفحو_سائيز_سرڪايو);
}

ساكن فراغ rpmalloc_وسعت_mark_as_subspan_unless_master(وسعت_t *master,
                                                         وسعت_t *subspan,
                                                         size_t وسعت_ڳڻپ);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
ساكن وسعت_t *_rpmalloc_global_get_reserved_spans(size_t وسعت_ڳڻپ) {
  وسعت_t *وسعت = يادگيري_global_reserve;
  rpmalloc_وسعت_mark_as_subspan_unless_master(يادگيري_global_reserve_master,
                                               وسعت, وسعت_ڳڻپ);
  يادگيري_global_reserve_ڳڻپ -= وسعت_ڳڻپ;
  إذا (يادگيري_global_reserve_ڳڻپ)
    يادگيري_global_reserve =
        (وسعت_t *)اشارو_وٿي(وسعت, وسعت_ڳڻپ << يادگيري_وسعت_سائيز_سرڪايو);
  وإلا
    يادگيري_global_reserve = 0;
  أرجع وسعت;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
ساكن فراغ rpmalloc_global_سيٽ_reserved_spans(وسعت_t *master, وسعت_t *reserve,
                                                size_t reserve_وسعت_ڳڻپ) {
  يادگيري_global_reserve_master = master;
  يادگيري_global_reserve_ڳڻپ = reserve_وسعت_ڳڻپ;
  يادگيري_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
ساكن فراغ rpmalloc_وسعت_double_link_list_جوڙيو(وسعت_t **سر, وسعت_t *وسعت) {
  إذا (*سر)
    (*سر)->prev = وسعت;
  وسعت->ايندڙ = *سر;
  *سر = وسعت;
}

//! Pop head span from double linked list
ساكن فراغ rpmalloc_وسعت_double_link_list_ڪڍو_سر(وسعت_t **سر,
                                                     وسعت_t *وسعت) {
  rpmalloc_assert(*سر == وسعت, "Linked list corrupted");
  وسعت = *سر;
  *سر = وسعت->ايندڙ;
}

//! Remove a span from double linked list
ساكن فراغ rpmalloc_وسعت_double_link_list_remove(وسعت_t **سر,
                                                   وسعت_t *وسعت) {
  rpmalloc_assert(*سر, "Linked list corrupted");
  إذا (*سر == وسعت) {
    *سر = وسعت->ايندڙ;
  } وإلا {
    وسعت_t *ايندڙ_وسعت = وسعت->ايندڙ;
    وسعت_t *prev_وسعت = وسعت->prev;
    prev_وسعت->ايندڙ = ايندڙ_وسعت;
    إذا (EXPECTED(ايندڙ_وسعت != 0))
      ايندڙ_وسعت->prev = prev_وسعت;
  }
}

////////////
///
/// Span control
///
//////

ساكن فراغ rpmalloc_ڍير_cache_وجهو(ڍير_t *ڍير_2, وسعت_t *وسعت);

ساكن فراغ rpmalloc_ڍير_finalize(ڍير_t *ڍير_2);

ساكن فراغ rpmalloc_ڍير_سيٽ_reserved_spans(ڍير_t *ڍير_2, وسعت_t *master,
                                              وسعت_t *reserve,
                                              size_t reserve_وسعت_ڳڻپ);

//! Declare the span to be a subspan and store distance from master span and
//! span count
ساكن فراغ rpmalloc_وسعت_mark_as_subspan_unless_master(وسعت_t *master,
                                                         وسعت_t *subspan,
                                                         size_t وسعت_ڳڻپ) {
  rpmalloc_assert((subspan != master) || (subspan->flags & وسعت_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  إذا (subspan != master) {
    subspan->flags = وسعت_FLAG_SUBSPAN;
    subspan->وٿي_from_master =
        (uint32_t)((uintptr_t)اشارو_diff(subspan, master) >>
                   يادگيري_وسعت_سائيز_سرڪايو);
    subspan->align_وٿي = 0;
  }
  subspan->وسعت_ڳڻپ = (uint32_t)وسعت_ڳڻپ;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
ساكن وسعت_t *rpmalloc_وسعت_map_from_reserve(ڍير_t *ڍير_2,
                                               size_t وسعت_ڳڻپ) {
  // Update the heap span reserve
  وسعت_t *وسعت = ڍير_2->وسعت_reserve;
  ڍير_2->وسعت_reserve =
      (وسعت_t *)اشارو_وٿي(وسعت, وسعت_ڳڻپ * يادگيري_وسعت_سائيز);
  ڍير_2->spans_reserved -= (uint32_t)وسعت_ڳڻپ;

  rpmalloc_وسعت_mark_as_subspan_unless_master(ڍير_2->وسعت_reserve_master, وسعت,
                                               وسعت_ڳڻپ);
  إذا (وسعت_ڳڻپ <= LARGE_CLASS_ڳڻپ)
    _rpmalloc_stat_inc(&ڍير_2->وسعت_use[وسعت_ڳڻپ - 1].spans_from_reserved);

  أرجع وسعت;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
ساكن size_t rpmalloc_وسعت_align_ڳڻپ(size_t وسعت_ڳڻپ) {
  size_t request_ڳڻپ = (وسعت_ڳڻپ > يادگيري_وسعت_map_ڳڻپ)
                             ? وسعت_ڳڻپ
                             : يادگيري_وسعت_map_ڳڻپ;
  إذا ((يادگيري_صفحو_سائيز > يادگيري_وسعت_سائيز) &&
      ((request_ڳڻپ * يادگيري_وسعت_سائيز) % يادگيري_صفحو_سائيز))
    request_ڳڻپ +=
        يادگيري_وسعت_map_ڳڻپ - (request_ڳڻپ % يادگيري_وسعت_map_ڳڻپ);
  أرجع request_ڳڻپ;
}

//! Setup a newly mapped span
ساكن فراغ rpmalloc_وسعت_initialize(وسعت_t *وسعت, size_t ڪل_وسعت_ڳڻپ,
                                      size_t وسعت_ڳڻپ, size_t align_وٿي) {
  وسعت->ڪل_spans = (uint32_t)ڪل_وسعت_ڳڻپ;
  وسعت->وسعت_ڳڻپ = (uint32_t)وسعت_ڳڻپ;
  وسعت->align_وٿي = (uint32_t)align_وٿي;
  وسعت->flags = وسعت_FLAG_MASTER;
  atomic_store32(&وسعت->بچيل_spans, (int32_t)ڪل_وسعت_ڳڻپ);
}

ساكن فراغ rpmalloc_وسعت_unmap(وسعت_t *وسعت);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
ساكن وسعت_t *rpmalloc_وسعت_map_aligned_ڳڻپ(ڍير_t *ڍير_2,
                                                size_t وسعت_ڳڻپ) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_وسعت_ڳڻپ = rpmalloc_وسعت_align_ڳڻپ(وسعت_ڳڻپ);
  size_t align_وٿي = 0;
  وسعت_t *وسعت = (وسعت_t *)_rpmalloc_mmap(
      aligned_وسعت_ڳڻپ * يادگيري_وسعت_سائيز, &align_وٿي);
  إذا (!وسعت)
    أرجع 0;
  rpmalloc_وسعت_initialize(وسعت, aligned_وسعت_ڳڻپ, وسعت_ڳڻپ, align_وٿي);
  _rpmalloc_stat_inc(&_master_spans);
  إذا (وسعت_ڳڻپ <= LARGE_CLASS_ڳڻپ)
    _rpmalloc_stat_inc(&ڍير_2->وسعت_use[وسعت_ڳڻپ - 1].spans_map_calls);
  إذا (aligned_وسعت_ڳڻپ > وسعت_ڳڻپ) {
    وسعت_t *reserved_spans =
        (وسعت_t *)اشارو_وٿي(وسعت, وسعت_ڳڻپ * يادگيري_وسعت_سائيز);
    size_t reserved_ڳڻپ = aligned_وسعت_ڳڻپ - وسعت_ڳڻپ;
    إذا (ڍير_2->spans_reserved) {
      rpmalloc_وسعت_mark_as_subspan_unless_master(
          ڍير_2->وسعت_reserve_master, ڍير_2->وسعت_reserve, ڍير_2->spans_reserved);
      rpmalloc_ڍير_cache_وجهو(ڍير_2, ڍير_2->وسعت_reserve);
    }
    إذا (reserved_ڳڻپ > يادگيري_ڍير_reserve_ڳڻپ) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&يادگيري_global_تالو) == 1,
                      "Global spin lock not held as expected");
      size_t remain_ڳڻپ = reserved_ڳڻپ - يادگيري_ڍير_reserve_ڳڻپ;
      reserved_ڳڻپ = يادگيري_ڍير_reserve_ڳڻپ;
      وسعت_t *remain_وسعت = (وسعت_t *)اشارو_وٿي(
          reserved_spans, reserved_ڳڻپ * يادگيري_وسعت_سائيز);
      إذا (يادگيري_global_reserve) {
        rpmalloc_وسعت_mark_as_subspan_unless_master(
            يادگيري_global_reserve_master, يادگيري_global_reserve,
            يادگيري_global_reserve_ڳڻپ);
        rpmalloc_وسعت_unmap(يادگيري_global_reserve);
      }
      rpmalloc_global_سيٽ_reserved_spans(وسعت, remain_وسعت, remain_ڳڻپ);
    }
    rpmalloc_ڍير_سيٽ_reserved_spans(ڍير_2, وسعت, reserved_spans,
                                      reserved_ڳڻپ);
  }
  أرجع وسعت;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
ساكن وسعت_t *rpmalloc_وسعت_map(ڍير_t *ڍير_2, size_t وسعت_ڳڻپ) {
  إذا (وسعت_ڳڻپ <= ڍير_2->spans_reserved)
    أرجع rpmalloc_وسعت_map_from_reserve(ڍير_2, وسعت_ڳڻپ);
  وسعت_t *وسعت = 0;
  صحيح use_global_reserve =
      (يادگيري_صفحو_سائيز > يادگيري_وسعت_سائيز) ||
      (يادگيري_وسعت_map_ڳڻپ > يادگيري_ڍير_reserve_ڳڻپ);
  إذا (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    طالما (!atomic_cas32_acquire(&يادگيري_global_تالو, 1, 0))
      _rpmalloc_spin();
    إذا (يادگيري_global_reserve_ڳڻپ >= وسعت_ڳڻپ) {
      size_t reserve_ڳڻپ =
          (!ڍير_2->spans_reserved ? يادگيري_ڍير_reserve_ڳڻپ : وسعت_ڳڻپ);
      إذا (يادگيري_global_reserve_ڳڻپ < reserve_ڳڻپ)
        reserve_ڳڻپ = يادگيري_global_reserve_ڳڻپ;
      وسعت = _rpmalloc_global_get_reserved_spans(reserve_ڳڻپ);
      إذا (وسعت) {
        إذا (reserve_ڳڻپ > وسعت_ڳڻپ) {
          وسعت_t *reserved_وسعت = (وسعت_t *)اشارو_وٿي(
              وسعت, وسعت_ڳڻپ << يادگيري_وسعت_سائيز_سرڪايو);
          rpmalloc_ڍير_سيٽ_reserved_spans(ڍير_2, يادگيري_global_reserve_master,
                                            reserved_وسعت,
                                            reserve_ڳڻپ - وسعت_ڳڻپ);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        وسعت->وسعت_ڳڻپ = (uint32_t)وسعت_ڳڻپ;
      }
    }
  }
  إذا (!وسعت)
    وسعت = rpmalloc_وسعت_map_aligned_ڳڻپ(ڍير_2, وسعت_ڳڻپ);
  إذا (use_global_reserve)
    atomic_store32_release(&يادگيري_global_تالو, 0);
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
                : ((وسعت_t *)اشارو_وٿي(
                      وسعت, -(intptr_t)((uintptr_t)وسعت->وٿي_from_master *
                                        يادگيري_وسعت_سائيز)));
  rpmalloc_assert(is_master || (وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & وسعت_FLAG_MASTER, "Span flag corrupted");

  size_t وسعت_ڳڻپ = وسعت->وسعت_ڳڻپ;
  إذا (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(وسعت->align_وٿي == 0, "Span align offset corrupted");
    إذا (يادگيري_وسعت_سائيز >= يادگيري_صفحو_سائيز)
      _rpmalloc_unmap(وسعت, وسعت_ڳڻپ * يادگيري_وسعت_سائيز, 0, 0);
  } وإلا {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    وسعت->flags |=
        وسعت_FLAG_MASTER | وسعت_FLAG_SUBSPAN | وسعت_FLAG_UNMAPPED_MASTER;
    rpmalloc_stat_جوڙيو(&_unmapped_master_spans, 1);
  }

  إذا (atomic_add32(&master->بچيل_spans, -(int32_t)وسعت_ڳڻپ) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & وسعت_FLAG_MASTER) &&
                        !!(master->flags & وسعت_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_ڳڻپ = master->وسعت_ڳڻپ;
    إذا (يادگيري_وسعت_سائيز < يادگيري_صفحو_سائيز)
      unmap_ڳڻپ = master->ڪل_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_ڳڻپ * يادگيري_وسعت_سائيز,
                    master->align_وٿي,
                    (size_t)master->ڪل_spans * يادگيري_وسعت_سائيز);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
ساكن فراغ rpmalloc_وسعت_release_منزل_cache(ڍير_t *ڍير_2, وسعت_t *وسعت) {
  rpmalloc_assert(ڍير_2 == وسعت->ڍير_2, "Span heap pointer corrupted");
  rpmalloc_assert(وسعت->سائيز_class < سائيز_CLASS_ڳڻپ,
                  "Invalid span size class");
  rpmalloc_assert(وسعت->وسعت_ڳڻپ == 1, "Invalid span count");
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&ڍير_2->وسعت_use[0].current);
#جيڪڏهن_پورو
  _rpmalloc_stat_dec(&ڍير_2->سائيز_class_use[وسعت->سائيز_class].spans_current);
  إذا (!ڍير_2->finalize) {
    _rpmalloc_stat_inc(&ڍير_2->وسعت_use[0].spans_منزل_cache);
    _rpmalloc_stat_inc(&ڍير_2->سائيز_class_use[وسعت->سائيز_class].spans_منزل_cache);
    إذا (ڍير_2->سائيز_class[وسعت->سائيز_class].cache)
      rpmalloc_ڍير_cache_وجهو(ڍير_2,
                                  ڍير_2->سائيز_class[وسعت->سائيز_class].cache);
    ڍير_2->سائيز_class[وسعت->سائيز_class].cache = وسعت;
  } وإلا {
    rpmalloc_وسعت_unmap(وسعت);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
ساكن uint32_t free_list_partial_init(فراغ **list, فراغ **پهريون_بلاڪ,
                                       فراغ *صفحو_شروعات, فراغ *بلاڪ_شروعات,
                                       uint32_t بلاڪ_ڳڻپ,
                                       uint32_t بلاڪ_سائيز_2) {
  rpmalloc_assert(بلاڪ_ڳڻپ, "Internal failure");
  *پهريون_بلاڪ = بلاڪ_شروعات;
  إذا (بلاڪ_ڳڻپ > 1) {
    فراغ *free_بلاڪ = اشارو_وٿي(بلاڪ_شروعات, بلاڪ_سائيز_2);
    فراغ *بلاڪ_پڇاڙي =
        اشارو_وٿي(بلاڪ_شروعات, (size_t)بلاڪ_سائيز_2 * بلاڪ_ڳڻپ);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    إذا (بلاڪ_سائيز_2 < (يادگيري_صفحو_سائيز >> 1)) {
      فراغ *صفحو_پڇاڙي = اشارو_وٿي(صفحو_شروعات, يادگيري_صفحو_سائيز);
      إذا (صفحو_پڇاڙي < بلاڪ_پڇاڙي)
        بلاڪ_پڇاڙي = صفحو_پڇاڙي;
    }
    *list = free_بلاڪ;
    بلاڪ_ڳڻپ = 2;
    فراغ *ايندڙ_بلاڪ = اشارو_وٿي(free_بلاڪ, بلاڪ_سائيز_2);
    طالما (ايندڙ_بلاڪ < بلاڪ_پڇاڙي) {
      *((فراغ **)free_بلاڪ) = ايندڙ_بلاڪ;
      free_بلاڪ = ايندڙ_بلاڪ;
      ++بلاڪ_ڳڻپ;
      ايندڙ_بلاڪ = اشارو_وٿي(ايندڙ_بلاڪ, بلاڪ_سائيز_2);
    }
    *((فراغ **)free_بلاڪ) = 0;
  } وإلا {
    *list = 0;
  }
  أرجع بلاڪ_ڳڻپ;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
ساكن فراغ *rpmalloc_وسعت_initialize_new(ڍير_t *ڍير_2,
                                           ڍير_سائيز_class_t *ڍير_سائيز_class,
                                           وسعت_t *وسعت, uint32_t class_idx) {
  rpmalloc_assert(وسعت->وسعت_ڳڻپ == 1, "Internal failure");
  سائيز_class_t *سائيز_class = يادگيري_سائيز_class + class_idx;
  وسعت->سائيز_class = class_idx;
  وسعت->ڍير_2 = ڍير_2;
  وسعت->flags &= ~وسعت_FLAG_ALIGNED_بلاڪون;
  وسعت->بلاڪ_سائيز_2 = سائيز_class->بلاڪ_سائيز_2;
  وسعت->بلاڪ_ڳڻپ = سائيز_class->بلاڪ_ڳڻپ;
  وسعت->free_list = 0;
  وسعت->list_سائيز = 0;
  atomic_store_ptr_release(&وسعت->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  فراغ *بلاڪ;
  وسعت->free_list_حد_بندي =
      free_list_partial_init(&ڍير_سائيز_class->free_list, &بلاڪ, وسعت,
                             اشارو_وٿي(وسعت, وسعت_سرو_سائيز),
                             سائيز_class->بلاڪ_ڳڻپ, سائيز_class->بلاڪ_سائيز_2);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  إذا (وسعت->free_list_حد_بندي < وسعت->بلاڪ_ڳڻپ) {
    rpmalloc_وسعت_double_link_list_جوڙيو(&ڍير_سائيز_class->partial_وسعت, وسعت);
    وسعت->استعمال_ٿيل_ڳڻپ = وسعت->free_list_حد_بندي;
  } وإلا {
#إذا RPMALLOC_پهريون_CLASS_HEAPS
    rpmalloc_وسعت_double_link_list_جوڙيو(&ڍير_2->ڀريل_وسعت[class_idx], وسعت);
#جيڪڏهن_پورو
    ++ڍير_2->ڀريل_وسعت_ڳڻپ;
    وسعت->استعمال_ٿيل_ڳڻپ = وسعت->بلاڪ_ڳڻپ;
  }
  أرجع بلاڪ;
}

ساكن فراغ rpmalloc_وسعت_ڪڍو_free_list_deferred(وسعت_t *وسعت) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  افعل {
    وسعت->free_list =
        atomic_exchange_ptr_acquire(&وسعت->free_list_deferred, INVALID_اشارو);
  } طالما (وسعت->free_list == INVALID_اشارو);
  وسعت->استعمال_ٿيل_ڳڻپ -= وسعت->list_سائيز;
  وسعت->list_سائيز = 0;
  atomic_store_ptr_release(&وسعت->free_list_deferred, 0);
}

ساكن صحيح rpmalloc_وسعت_is_fully_utilized(وسعت_t *وسعت) {
  rpmalloc_assert(وسعت->free_list_حد_بندي <= وسعت->بلاڪ_ڳڻپ,
                  "Span free list corrupted");
  أرجع !وسعت->free_list && (وسعت->free_list_حد_بندي >= وسعت->بلاڪ_ڳڻپ);
}

ساكن صحيح rpmalloc_وسعت_finalize(ڍير_t *ڍير_2, size_t iclass, وسعت_t *وسعت,
                                   وسعت_t **list_سر) {
  فراغ *free_list = ڍير_2->سائيز_class[iclass].free_list;
  وسعت_t *class_وسعت = (وسعت_t *)((uintptr_t)free_list & يادگيري_وسعت_نقاب);
  إذا (وسعت == class_وسعت) {
    // Adopt the heap class free list back into the span free list
    فراغ *بلاڪ = وسعت->free_list;
    فراغ *آخري_بلاڪ = 0;
    طالما (بلاڪ) {
      آخري_بلاڪ = بلاڪ;
      بلاڪ = *((فراغ **)بلاڪ);
    }
    uint32_t free_ڳڻپ = 0;
    بلاڪ = free_list;
    طالما (بلاڪ) {
      ++free_ڳڻپ;
      بلاڪ = *((فراغ **)بلاڪ);
    }
    إذا (آخري_بلاڪ) {
      *((فراغ **)آخري_بلاڪ) = free_list;
    } وإلا {
      وسعت->free_list = free_list;
    }
    ڍير_2->سائيز_class[iclass].free_list = 0;
    وسعت->استعمال_ٿيل_ڳڻپ -= free_ڳڻپ;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(وسعت->list_سائيز == وسعت->استعمال_ٿيل_ڳڻپ, "Memory leak detected");
  إذا (وسعت->list_سائيز == وسعت->استعمال_ٿيل_ڳڻپ) {
    _rpmalloc_stat_dec(&ڍير_2->وسعت_use[0].current);
    _rpmalloc_stat_dec(&ڍير_2->سائيز_class_use[iclass].spans_current);
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
  طالما (!atomic_cas32_acquire(&cache->تالو, 1, 0))
    _rpmalloc_spin();

  لكل (size_t ispan = 0; ispan < cache->ڳڻپ; ++ispan)
    rpmalloc_وسعت_unmap(cache->وسعت[ispan]);
  cache->ڳڻپ = 0;

  طالما (cache->حد_کان_مٿي_2) {
    وسعت_t *وسعت = cache->حد_کان_مٿي_2;
    cache->حد_کان_مٿي_2 = وسعت->ايندڙ;
    rpmalloc_وسعت_unmap(وسعت);
  }

  atomic_store32_release(&cache->تالو, 0);
}

ساكن فراغ rpmalloc_global_cache_وجهو_spans(وسعت_t **وسعت,
                                                size_t وسعت_ڳڻپ,
                                                size_t ڳڻپ) {
  ثابت size_t cache_حد_بندي =
      (وسعت_ڳڻپ == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_وسعت_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_وسعت_LARGE_CACHE - (وسعت_ڳڻپ >> 1));

  global_cache_t *cache = &يادگيري_وسعت_cache[وسعت_ڳڻپ - 1];

  size_t وجهو_ڳڻپ = ڳڻپ;
  طالما (!atomic_cas32_acquire(&cache->تالو, 1, 0))
    _rpmalloc_spin();

#إذا ENABLE_STATISTICS
  cache->وجهو_ڳڻپ += ڳڻپ;
#جيڪڏهن_پورو
  إذا ((cache->ڳڻپ + وجهو_ڳڻپ) > cache_حد_بندي)
    وجهو_ڳڻپ = cache_حد_بندي - cache->ڳڻپ;

  يادگيري_نقل(cache->وسعت + cache->ڳڻپ, وسعت, حجم(وسعت_t *) * وجهو_ڳڻپ);
  cache->ڳڻپ += (uint32_t)وجهو_ڳڻپ;

#إذا ENABLE_UNLIMITED_CACHE
  طالما (وجهو_ڳڻپ < ڳڻپ) {
#وإلا
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  طالما ((يادگيري_صفحو_سائيز > يادگيري_وسعت_سائيز) && (وجهو_ڳڻپ < ڳڻپ)) {
#جيڪڏهن_پورو
    وسعت_t *current_وسعت = وسعت[وجهو_ڳڻپ++];
    current_وسعت->ايندڙ = cache->حد_کان_مٿي_2;
    cache->حد_کان_مٿي_2 = current_وسعت;
  }
  atomic_store32_release(&cache->تالو, 0);

  وسعت_t *keep = 0;
  لكل (size_t ispan = وجهو_ڳڻپ; ispan < ڳڻپ; ++ispan) {
    وسعت_t *current_وسعت = وسعت[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    إذا ((current_وسعت->flags & وسعت_FLAG_MASTER) &&
        (atomic_load32(&current_وسعت->بچيل_spans) >
         (int32_t)current_وسعت->وسعت_ڳڻپ)) {
      current_وسعت->ايندڙ = keep;
      keep = current_وسعت;
    } وإلا {
      rpmalloc_وسعت_unmap(current_وسعت);
    }
  }

  إذا (keep) {
    طالما (!atomic_cas32_acquire(&cache->تالو, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    طالما (keep) {
      لكل (; islot < cache->ڳڻپ; ++islot) {
        وسعت_t *current_وسعت = cache->وسعت[islot];
        إذا (!(current_وسعت->flags & وسعت_FLAG_MASTER) ||
            ((current_وسعت->flags & وسعت_FLAG_MASTER) &&
             (atomic_load32(&current_وسعت->بچيل_spans) <=
              (int32_t)current_وسعت->وسعت_ڳڻپ))) {
          rpmalloc_وسعت_unmap(current_وسعت);
          cache->وسعت[islot] = keep;
          اكسر;
        }
      }
      إذا (islot == cache->ڳڻپ)
        اكسر;
      keep = keep->ايندڙ;
    }

    إذا (keep) {
      وسعت_t *پڇ_2 = keep;
      طالما (پڇ_2->ايندڙ)
        پڇ_2 = پڇ_2->ايندڙ;
      پڇ_2->ايندڙ = cache->حد_کان_مٿي_2;
      cache->حد_کان_مٿي_2 = keep;
    }

    atomic_store32_release(&cache->تالو, 0);
  }
}

ساكن size_t rpmalloc_global_cache_ڪڍو_spans(وسعت_t **وسعت,
                                                   size_t وسعت_ڳڻپ,
                                                   size_t ڳڻپ) {
  global_cache_t *cache = &يادگيري_وسعت_cache[وسعت_ڳڻپ - 1];

  size_t ڪڍو_ڳڻپ_2 = 0;
  طالما (!atomic_cas32_acquire(&cache->تالو, 1, 0))
    _rpmalloc_spin();

#إذا ENABLE_STATISTICS
  cache->ڪڍو_ڳڻپ_2 += ڳڻپ;
#جيڪڏهن_پورو
  size_t want = ڳڻپ - ڪڍو_ڳڻپ_2;
  إذا (want > cache->ڳڻپ)
    want = cache->ڳڻپ;

  يادگيري_نقل(وسعت + ڪڍو_ڳڻپ_2, cache->وسعت + (cache->ڳڻپ - want),
         حجم(وسعت_t *) * want);
  cache->ڳڻپ -= (uint32_t)want;
  ڪڍو_ڳڻپ_2 += want;

  طالما ((ڪڍو_ڳڻپ_2 < ڳڻپ) && cache->حد_کان_مٿي_2) {
    وسعت_t *current_وسعت = cache->حد_کان_مٿي_2;
    وسعت[ڪڍو_ڳڻپ_2++] = current_وسعت;
    cache->حد_کان_مٿي_2 = current_وسعت->ايندڙ;
  }

#إذا ENABLE_ASSERTS
  لكل (size_t ispan = 0; ispan < ڪڍو_ڳڻپ_2; ++ispan) {
    rpmalloc_assert(وسعت[ispan]->وسعت_ڳڻپ == وسعت_ڳڻپ,
                    "Global cache span count mismatch");
  }
#جيڪڏهن_پورو

  atomic_store32_release(&cache->تالو, 0);

  أرجع ڪڍو_ڳڻپ_2;
}

#جيڪڏهن_پورو

////////////
///
/// Heap control
///
//////

ساكن فراغ _rpmalloc_deallocate_huge(وسعت_t *);

//! Store the given spans as reserve in the given heap
ساكن فراغ rpmalloc_ڍير_سيٽ_reserved_spans(ڍير_t *ڍير_2, وسعت_t *master,
                                              وسعت_t *reserve,
                                              size_t reserve_وسعت_ڳڻپ) {
  ڍير_2->وسعت_reserve_master = master;
  ڍير_2->وسعت_reserve = reserve;
  ڍير_2->spans_reserved = (uint32_t)reserve_وسعت_ڳڻپ;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
ساكن فراغ rpmalloc_ڍير_cache_adopt_deferred(ڍير_t *ڍير_2,
                                                وسعت_t **single_وسعت) {
  وسعت_t *وسعت = (وسعت_t *)((فراغ *)atomic_exchange_ptr_acquire(
      &ڍير_2->وسعت_free_deferred, 0));
  طالما (وسعت) {
    وسعت_t *ايندڙ_وسعت = (وسعت_t *)وسعت->free_list;
    rpmalloc_assert(وسعت->ڍير_2 == ڍير_2, "Span heap pointer corrupted");
    إذا (EXPECTED(وسعت->سائيز_class < سائيز_CLASS_ڳڻپ)) {
      rpmalloc_assert(ڍير_2->ڀريل_وسعت_ڳڻپ, "Heap span counter corrupted");
      --ڍير_2->ڀريل_وسعت_ڳڻپ;
      _rpmalloc_stat_dec(&ڍير_2->وسعت_use[0].spans_deferred);
#إذا RPMALLOC_پهريون_CLASS_HEAPS
      rpmalloc_وسعت_double_link_list_remove(&ڍير_2->ڀريل_وسعت[وسعت->سائيز_class],
                                             وسعت);
#جيڪڏهن_پورو
      _rpmalloc_stat_dec(&ڍير_2->وسعت_use[0].current);
      _rpmalloc_stat_dec(&ڍير_2->سائيز_class_use[وسعت->سائيز_class].spans_current);
      إذا (single_وسعت && !*single_وسعت)
        *single_وسعت = وسعت;
      وإلا
        rpmalloc_ڍير_cache_وجهو(ڍير_2, وسعت);
    } وإلا {
      إذا (وسعت->سائيز_class == سائيز_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(وسعت);
      } وإلا {
        rpmalloc_assert(وسعت->سائيز_class == سائيز_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(ڍير_2->ڀريل_وسعت_ڳڻپ, "Heap span counter corrupted");
        --ڍير_2->ڀريل_وسعت_ڳڻپ;
#إذا RPMALLOC_پهريون_CLASS_HEAPS
        rpmalloc_وسعت_double_link_list_remove(&ڍير_2->large_huge_وسعت, وسعت);
#جيڪڏهن_پورو
        uint32_t idx = وسعت->وسعت_ڳڻپ - 1;
        _rpmalloc_stat_dec(&ڍير_2->وسعت_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&ڍير_2->وسعت_use[idx].current);
        إذا (!idx && single_وسعت && !*single_وسعت)
          *single_وسعت = وسعت;
        وإلا
          rpmalloc_ڍير_cache_وجهو(ڍير_2, وسعت);
      }
    }
    وسعت = ايندڙ_وسعت;
  }
}

ساكن فراغ rpmalloc_ڍير_unmap(ڍير_t *ڍير_2) {
  إذا (!ڍير_2->master_ڍير) {
    إذا ((ڍير_2->finalize > 1) && !atomic_load32(&ڍير_2->ٻار_ڳڻپ)) {
      وسعت_t *وسعت = (وسعت_t *)((uintptr_t)ڍير_2 & يادگيري_وسعت_نقاب);
      rpmalloc_وسعت_unmap(وسعت);
    }
  } وإلا {
    إذا (atomic_decr32(&ڍير_2->master_ڍير->ٻار_ڳڻپ) == 0) {
      rpmalloc_ڍير_unmap(ڍير_2->master_ڍير);
    }
  }
}

ساكن فراغ rpmalloc_ڍير_global_finalize(ڍير_t *ڍير_2) {
  إذا (ڍير_2->finalize++ > 1) {
    --ڍير_2->finalize;
    أرجع;
  }

  rpmalloc_ڍير_finalize(ڍير_2);

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_ڳڻپ; ++iclass) {
    وسعت_cache_t *وسعت_cache;
    إذا (!iclass)
      وسعت_cache = &ڍير_2->وسعت_cache;
    وإلا
      وسعت_cache = (وسعت_cache_t *)(ڍير_2->وسعت_large_cache + (iclass - 1));
    لكل (size_t ispan = 0; ispan < وسعت_cache->ڳڻپ; ++ispan)
      rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
    وسعت_cache->ڳڻپ = 0;
  }
#جيڪڏهن_پورو

  إذا (ڍير_2->ڀريل_وسعت_ڳڻپ) {
    --ڍير_2->finalize;
    أرجع;
  }

  لكل (size_t iclass = 0; iclass < سائيز_CLASS_ڳڻپ; ++iclass) {
    إذا (ڍير_2->سائيز_class[iclass].free_list ||
        ڍير_2->سائيز_class[iclass].partial_وسعت) {
      --ڍير_2->finalize;
      أرجع;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)ڍير_2->id % ڍير_ARRAY_سائيز;
  ڍير_t *list_ڍير = يادگيري_heaps[list_idx];
  إذا (list_ڍير == ڍير_2) {
    يادگيري_heaps[list_idx] = ڍير_2->ايندڙ_ڍير;
  } وإلا {
    طالما (list_ڍير->ايندڙ_ڍير != ڍير_2)
      list_ڍير = list_ڍير->ايندڙ_ڍير;
    list_ڍير->ايندڙ_ڍير = ڍير_2->ايندڙ_ڍير;
  }

  rpmalloc_ڍير_unmap(ڍير_2);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
ساكن فراغ rpmalloc_ڍير_cache_وجهو(ڍير_t *ڍير_2, وسعت_t *وسعت) {
  إذا (UNEXPECTED(ڍير_2->finalize != 0)) {
    rpmalloc_وسعت_unmap(وسعت);
    rpmalloc_ڍير_global_finalize(ڍير_2);
    أرجع;
  }
#إذا ENABLE_THREAD_CACHE
  size_t وسعت_ڳڻپ = وسعت->وسعت_ڳڻپ;
  _rpmalloc_stat_inc(&ڍير_2->وسعت_use[وسعت_ڳڻپ - 1].spans_منزل_cache);
  إذا (وسعت_ڳڻپ == 1) {
    وسعت_cache_t *وسعت_cache = &ڍير_2->وسعت_cache;
    وسعت_cache->وسعت[وسعت_cache->ڳڻپ++] = وسعت;
    إذا (وسعت_cache->ڳڻپ == MAX_THREAD_وسعت_CACHE) {
      ثابت size_t remain_ڳڻپ =
          MAX_THREAD_وسعت_CACHE - THREAD_وسعت_CACHE_TRANSFER;
#إذا ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&ڍير_2->thread_منزل_global,
                           THREAD_وسعت_CACHE_TRANSFER * يادگيري_وسعت_سائيز);
      rpmalloc_stat_جوڙيو(&ڍير_2->وسعت_use[وسعت_ڳڻپ - 1].spans_منزل_global,
                         THREAD_وسعت_CACHE_TRANSFER);
      rpmalloc_global_cache_وجهو_spans(وسعت_cache->وسعت + remain_ڳڻپ,
                                          وسعت_ڳڻپ,
                                          THREAD_وسعت_CACHE_TRANSFER);
#وإلا
      لكل (size_t ispan = 0; ispan < THREAD_وسعت_CACHE_TRANSFER; ++ispan)
        rpmalloc_وسعت_unmap(وسعت_cache->وسعت[remain_ڳڻپ + ispan]);
#جيڪڏهن_پورو
      وسعت_cache->ڳڻپ = remain_ڳڻپ;
    }
  } وإلا {
    size_t cache_idx = وسعت_ڳڻپ - 2;
    وسعت_large_cache_t *وسعت_cache = ڍير_2->وسعت_large_cache + cache_idx;
    وسعت_cache->وسعت[وسعت_cache->ڳڻپ++] = وسعت;
    ثابت size_t cache_حد_بندي =
        (MAX_THREAD_وسعت_LARGE_CACHE - (وسعت_ڳڻپ >> 1));
    إذا (وسعت_cache->ڳڻپ == cache_حد_بندي) {
      ثابت size_t transfer_حد_بندي = 2 + (cache_حد_بندي >> 2);
      ثابت size_t transfer_ڳڻپ =
          (THREAD_وسعت_LARGE_CACHE_TRANSFER <= transfer_حد_بندي
               ? THREAD_وسعت_LARGE_CACHE_TRANSFER
               : transfer_حد_بندي);
      ثابت size_t remain_ڳڻپ = cache_حد_بندي - transfer_ڳڻپ;
#إذا ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&ڍير_2->thread_منزل_global,
                           transfer_ڳڻپ * وسعت_ڳڻپ * يادگيري_وسعت_سائيز);
      rpmalloc_stat_جوڙيو(&ڍير_2->وسعت_use[وسعت_ڳڻپ - 1].spans_منزل_global,
                         transfer_ڳڻپ);
      rpmalloc_global_cache_وجهو_spans(وسعت_cache->وسعت + remain_ڳڻپ,
                                          وسعت_ڳڻپ, transfer_ڳڻپ);
#وإلا
      لكل (size_t ispan = 0; ispan < transfer_ڳڻپ; ++ispan)
        rpmalloc_وسعت_unmap(وسعت_cache->وسعت[remain_ڳڻپ + ispan]);
#جيڪڏهن_پورو
      وسعت_cache->ڳڻپ = remain_ڳڻپ;
    }
  }
#وإلا
  (فراغ)حجم(ڍير_2);
  rpmalloc_وسعت_unmap(وسعت);
#جيڪڏهن_پورو
}

//! Extract the given number of spans from the different cache levels
ساكن وسعت_t *rpmalloc_ڍير_thread_cache_ڪڍو(ڍير_t *ڍير_2,
                                                   size_t وسعت_ڳڻپ) {
  وسعت_t *وسعت = 0;
#إذا ENABLE_THREAD_CACHE
  وسعت_cache_t *وسعت_cache;
  إذا (وسعت_ڳڻپ == 1)
    وسعت_cache = &ڍير_2->وسعت_cache;
  وإلا
    وسعت_cache = (وسعت_cache_t *)(ڍير_2->وسعت_large_cache + (وسعت_ڳڻپ - 2));
  إذا (وسعت_cache->ڳڻپ) {
    _rpmalloc_stat_inc(&ڍير_2->وسعت_use[وسعت_ڳڻپ - 1].spans_from_cache);
    أرجع وسعت_cache->وسعت[--وسعت_cache->ڳڻپ];
  }
#جيڪڏهن_پورو
  أرجع وسعت;
}

ساكن وسعت_t *rpmalloc_ڍير_thread_cache_deferred_ڪڍو(ڍير_t *ڍير_2,
                                                            size_t وسعت_ڳڻپ) {
  وسعت_t *وسعت = 0;
  إذا (وسعت_ڳڻپ == 1) {
    rpmalloc_ڍير_cache_adopt_deferred(ڍير_2, &وسعت);
  } وإلا {
    rpmalloc_ڍير_cache_adopt_deferred(ڍير_2, 0);
    وسعت = rpmalloc_ڍير_thread_cache_ڪڍو(ڍير_2, وسعت_ڳڻپ);
  }
  أرجع وسعت;
}

ساكن وسعت_t *rpmalloc_ڍير_reserved_ڪڍو(ڍير_t *ڍير_2,
                                               size_t وسعت_ڳڻپ) {
  إذا (ڍير_2->spans_reserved >= وسعت_ڳڻپ)
    أرجع rpmalloc_وسعت_map(ڍير_2, وسعت_ڳڻپ);
  أرجع 0;
}

//! Extract a span from the global cache
ساكن وسعت_t *rpmalloc_ڍير_global_cache_ڪڍو(ڍير_t *ڍير_2,
                                                   size_t وسعت_ڳڻپ) {
#إذا ENABLE_GLOBAL_CACHE
#إذا ENABLE_THREAD_CACHE
  وسعت_cache_t *وسعت_cache;
  size_t wanted_ڳڻپ;
  إذا (وسعت_ڳڻپ == 1) {
    وسعت_cache = &ڍير_2->وسعت_cache;
    wanted_ڳڻپ = THREAD_وسعت_CACHE_TRANSFER;
  } وإلا {
    وسعت_cache = (وسعت_cache_t *)(ڍير_2->وسعت_large_cache + (وسعت_ڳڻپ - 2));
    wanted_ڳڻپ = THREAD_وسعت_LARGE_CACHE_TRANSFER;
  }
  وسعت_cache->ڳڻپ = rpmalloc_global_cache_ڪڍو_spans(
      وسعت_cache->وسعت, وسعت_ڳڻپ, wanted_ڳڻپ);
  إذا (وسعت_cache->ڳڻپ) {
    _rpmalloc_stat_add64(&ڍير_2->global_منزل_thread,
                         وسعت_ڳڻپ * وسعت_cache->ڳڻپ * يادگيري_وسعت_سائيز);
    rpmalloc_stat_جوڙيو(&ڍير_2->وسعت_use[وسعت_ڳڻپ - 1].spans_from_global,
                       وسعت_cache->ڳڻپ);
    أرجع وسعت_cache->وسعت[--وسعت_cache->ڳڻپ];
  }
#وإلا
  وسعت_t *وسعت = 0;
  size_t ڳڻپ = rpmalloc_global_cache_ڪڍو_spans(&وسعت, وسعت_ڳڻپ, 1);
  إذا (ڳڻپ) {
    _rpmalloc_stat_add64(&ڍير_2->global_منزل_thread,
                         وسعت_ڳڻپ * ڳڻپ * يادگيري_وسعت_سائيز);
    rpmalloc_stat_جوڙيو(&ڍير_2->وسعت_use[وسعت_ڳڻپ - 1].spans_from_global,
                       ڳڻپ);
    أرجع وسعت;
  }
#جيڪڏهن_پورو
#جيڪڏهن_پورو
  (فراغ)حجم(ڍير_2);
  (فراغ)حجم(وسعت_ڳڻپ);
  أرجع 0;
}

ساكن فراغ rpmalloc_inc_وسعت_statistics(ڍير_t *ڍير_2, size_t وسعت_ڳڻپ,
                                          uint32_t class_idx) {
  (فراغ)حجم(ڍير_2);
  (فراغ)حجم(وسعت_ڳڻپ);
  (فراغ)حجم(class_idx);
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)وسعت_ڳڻپ - 1;
  uint32_t current_ڳڻپ =
      (uint32_t)atomic_incr32(&ڍير_2->وسعت_use[idx].current);
  إذا (current_ڳڻپ > (uint32_t)atomic_load32(&ڍير_2->وسعت_use[idx].مٿاهون))
    atomic_store32(&ڍير_2->وسعت_use[idx].مٿاهون, (int32_t)current_ڳڻپ);
  rpmalloc_stat_جوڙيو_peak(&ڍير_2->سائيز_class_use[class_idx].spans_current, 1,
                          ڍير_2->سائيز_class_use[class_idx].spans_peak);
#جيڪڏهن_پورو
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
ساكن وسعت_t *
rpmalloc_ڍير_ڪڍو_new_وسعت(ڍير_t *ڍير_2,
                                ڍير_سائيز_class_t *ڍير_سائيز_class,
                                size_t وسعت_ڳڻپ, uint32_t class_idx) {
  وسعت_t *وسعت;
#إذا ENABLE_THREAD_CACHE
  إذا (ڍير_سائيز_class && ڍير_سائيز_class->cache) {
    وسعت = ڍير_سائيز_class->cache;
    ڍير_سائيز_class->cache =
        (ڍير_2->وسعت_cache.ڳڻپ
             ? ڍير_2->وسعت_cache.وسعت[--ڍير_2->وسعت_cache.ڳڻپ]
             : 0);
    rpmalloc_inc_وسعت_statistics(ڍير_2, وسعت_ڳڻپ, class_idx);
    أرجع وسعت;
  }
#جيڪڏهن_پورو
  (فراغ)حجم(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t بنياد_وسعت_ڳڻپ = وسعت_ڳڻپ;
  size_t حد_بندي_وسعت_ڳڻپ =
      (وسعت_ڳڻپ > 2) ? (وسعت_ڳڻپ + (وسعت_ڳڻپ >> 1)) : وسعت_ڳڻپ;
  إذا (حد_بندي_وسعت_ڳڻپ > LARGE_CLASS_ڳڻپ)
    حد_بندي_وسعت_ڳڻپ = LARGE_CLASS_ڳڻپ;
  افعل {
    وسعت = rpmalloc_ڍير_thread_cache_ڪڍو(ڍير_2, وسعت_ڳڻپ);
    إذا (EXPECTED(وسعت != 0)) {
      _rpmalloc_stat_inc(&ڍير_2->سائيز_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_وسعت_statistics(ڍير_2, وسعت_ڳڻپ, class_idx);
      أرجع وسعت;
    }
    وسعت = rpmalloc_ڍير_thread_cache_deferred_ڪڍو(ڍير_2, وسعت_ڳڻپ);
    إذا (EXPECTED(وسعت != 0)) {
      _rpmalloc_stat_inc(&ڍير_2->سائيز_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_وسعت_statistics(ڍير_2, وسعت_ڳڻپ, class_idx);
      أرجع وسعت;
    }
    وسعت = rpmalloc_ڍير_global_cache_ڪڍو(ڍير_2, وسعت_ڳڻپ);
    إذا (EXPECTED(وسعت != 0)) {
      _rpmalloc_stat_inc(&ڍير_2->سائيز_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_وسعت_statistics(ڍير_2, وسعت_ڳڻپ, class_idx);
      أرجع وسعت;
    }
    وسعت = rpmalloc_ڍير_reserved_ڪڍو(ڍير_2, وسعت_ڳڻپ);
    إذا (EXPECTED(وسعت != 0)) {
      _rpmalloc_stat_inc(&ڍير_2->سائيز_class_use[class_idx].spans_from_reserved);
      rpmalloc_inc_وسعت_statistics(ڍير_2, وسعت_ڳڻپ, class_idx);
      أرجع وسعت;
    }
    ++وسعت_ڳڻپ;
  } طالما (وسعت_ڳڻپ <= حد_بندي_وسعت_ڳڻپ);
  // Final fallback, map in more virtual memory
  وسعت = rpmalloc_وسعت_map(ڍير_2, بنياد_وسعت_ڳڻپ);
  rpmalloc_inc_وسعت_statistics(ڍير_2, بنياد_وسعت_ڳڻپ, class_idx);
  _rpmalloc_stat_inc(&ڍير_2->سائيز_class_use[class_idx].spans_map_calls);
  أرجع وسعت;
}

ساكن فراغ rpmalloc_ڍير_initialize(ڍير_t *ڍير_2) {
  _rpmalloc_memset_const(ڍير_2, 0, حجم(ڍير_t));
  // Get a new heap ID
  ڍير_2->id = 1 + atomic_incr32(&يادگيري_ڍير_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)ڍير_2->id % ڍير_ARRAY_سائيز;
  ڍير_2->ايندڙ_ڍير = يادگيري_heaps[list_idx];
  يادگيري_heaps[list_idx] = ڍير_2;
}

ساكن فراغ rpmalloc_ڍير_orphan(ڍير_t *ڍير_2, صحيح پهريون_class) {
  ڍير_2->owner_thread = (uintptr_t)-1;
#إذا RPMALLOC_پهريون_CLASS_HEAPS
  ڍير_t **ڍير_list =
      (پهريون_class ? &يادگيري_پهريون_class_orphan_heaps : &يادگيري_orphan_heaps);
#وإلا
  (فراغ)حجم(پهريون_class);
  ڍير_t **ڍير_list = &يادگيري_orphan_heaps;
#جيڪڏهن_پورو
  ڍير_2->ايندڙ_orphan = *ڍير_list;
  *ڍير_list = ڍير_2;
}

//! Allocate a new heap from newly mapped memory pages
ساكن ڍير_t *rpmalloc_ڍير_allocate_new(فراغ) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t ڍير_سائيز_2 = حجم(ڍير_t);
  size_t aligned_ڍير_سائيز = 16 * ((ڍير_سائيز_2 + 15) / 16);
  size_t request_ڍير_ڳڻپ = 16;
  size_t ڍير_وسعت_ڳڻپ = ((aligned_ڍير_سائيز * request_ڍير_ڳڻپ) +
                            حجم(وسعت_t) + يادگيري_وسعت_سائيز - 1) /
                           يادگيري_وسعت_سائيز;
  size_t بلاڪ_سائيز_2 = يادگيري_وسعت_سائيز * ڍير_وسعت_ڳڻپ;
  size_t وسعت_ڳڻپ = ڍير_وسعت_ڳڻپ;
  وسعت_t *وسعت = 0;
  // If there are global reserved spans, use these first
  إذا (يادگيري_global_reserve_ڳڻپ >= ڍير_وسعت_ڳڻپ) {
    وسعت = _rpmalloc_global_get_reserved_spans(ڍير_وسعت_ڳڻپ);
  }
  إذا (!وسعت) {
    إذا (يادگيري_صفحو_سائيز > بلاڪ_سائيز_2) {
      وسعت_ڳڻپ = يادگيري_صفحو_سائيز / يادگيري_وسعت_سائيز;
      بلاڪ_سائيز_2 = يادگيري_صفحو_سائيز;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_ڍير_ڳڻپ =
          (بلاڪ_سائيز_2 - حجم(وسعت_t)) / aligned_ڍير_سائيز;
      إذا (possible_ڍير_ڳڻپ >= (request_ڍير_ڳڻپ * 16))
        request_ڍير_ڳڻپ *= 16;
      وإلا إذا (possible_ڍير_ڳڻپ < request_ڍير_ڳڻپ)
        request_ڍير_ڳڻپ = possible_ڍير_ڳڻپ;
      ڍير_وسعت_ڳڻپ = ((aligned_ڍير_سائيز * request_ڍير_ڳڻپ) +
                         حجم(وسعت_t) + يادگيري_وسعت_سائيز - 1) /
                        يادگيري_وسعت_سائيز;
    }

    size_t align_وٿي = 0;
    وسعت = (وسعت_t *)_rpmalloc_mmap(بلاڪ_سائيز_2, &align_وٿي);
    إذا (!وسعت)
      أرجع 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    rpmalloc_وسعت_initialize(وسعت, وسعت_ڳڻپ, ڍير_وسعت_ڳڻپ, align_وٿي);
  }

  size_t remain_سائيز = يادگيري_وسعت_سائيز - حجم(وسعت_t);
  ڍير_t *ڍير_2 = (ڍير_t *)اشارو_وٿي(وسعت, حجم(وسعت_t));
  rpmalloc_ڍير_initialize(ڍير_2);

  // Put extra heaps as orphans
  size_t انگ_heaps = remain_سائيز / aligned_ڍير_سائيز;
  إذا (انگ_heaps < request_ڍير_ڳڻپ)
    انگ_heaps = request_ڍير_ڳڻپ;
  atomic_store32(&ڍير_2->ٻار_ڳڻپ, (int32_t)انگ_heaps - 1);
  ڍير_t *extra_ڍير = (ڍير_t *)اشارو_وٿي(ڍير_2, aligned_ڍير_سائيز);
  طالما (انگ_heaps > 1) {
    rpmalloc_ڍير_initialize(extra_ڍير);
    extra_ڍير->master_ڍير = ڍير_2;
    rpmalloc_ڍير_orphan(extra_ڍير, 1);
    extra_ڍير = (ڍير_t *)اشارو_وٿي(extra_ڍير, aligned_ڍير_سائيز);
    --انگ_heaps;
  }

  إذا (وسعت_ڳڻپ > ڍير_وسعت_ڳڻپ) {
    // Cap reserved spans
    size_t remain_ڳڻپ = وسعت_ڳڻپ - ڍير_وسعت_ڳڻپ;
    size_t reserve_ڳڻپ =
        (remain_ڳڻپ > يادگيري_ڍير_reserve_ڳڻپ ? يادگيري_ڍير_reserve_ڳڻپ
                                                   : remain_ڳڻپ);
    وسعت_t *remain_وسعت =
        (وسعت_t *)اشارو_وٿي(وسعت, ڍير_وسعت_ڳڻپ * يادگيري_وسعت_سائيز);
    rpmalloc_ڍير_سيٽ_reserved_spans(ڍير_2, وسعت, remain_وسعت, reserve_ڳڻپ);

    إذا (remain_ڳڻپ > reserve_ڳڻپ) {
      // Set to global reserved spans
      remain_وسعت = (وسعت_t *)اشارو_وٿي(remain_وسعت,
                                             reserve_ڳڻپ * يادگيري_وسعت_سائيز);
      reserve_ڳڻپ = remain_ڳڻپ - reserve_ڳڻپ;
      rpmalloc_global_سيٽ_reserved_spans(وسعت, remain_وسعت, reserve_ڳڻپ);
    }
  }

  أرجع ڍير_2;
}

ساكن ڍير_t *rpmalloc_ڍير_ڪڍو_orphan(ڍير_t **ڍير_list) {
  ڍير_t *ڍير_2 = *ڍير_list;
  *ڍير_list = (ڍير_2 ? ڍير_2->ايندڙ_orphan : 0);
  أرجع ڍير_2;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
ساكن ڍير_t *rpmalloc_ڍير_allocate(صحيح پهريون_class) {
  ڍير_t *ڍير_2 = 0;
  طالما (!atomic_cas32_acquire(&يادگيري_global_تالو, 1, 0))
    _rpmalloc_spin();
  إذا (پهريون_class == 0)
    ڍير_2 = rpmalloc_ڍير_ڪڍو_orphan(&يادگيري_orphan_heaps);
#إذا RPMALLOC_پهريون_CLASS_HEAPS
  إذا (!ڍير_2)
    ڍير_2 = rpmalloc_ڍير_ڪڍو_orphan(&يادگيري_پهريون_class_orphan_heaps);
#جيڪڏهن_پورو
  إذا (!ڍير_2)
    ڍير_2 = rpmalloc_ڍير_allocate_new();
  atomic_store32_release(&يادگيري_global_تالو, 0);
  إذا (ڍير_2)
    rpmalloc_ڍير_cache_adopt_deferred(ڍير_2, 0);
  أرجع ڍير_2;
}

ساكن فراغ rpmalloc_ڍير_release(فراغ *heapptr, صحيح پهريون_class,
                                   صحيح release_cache) {
  ڍير_t *ڍير_2 = (ڍير_t *)heapptr;
  إذا (!ڍير_2)
    أرجع;
  // Release thread cache spans back to global cache
  rpmalloc_ڍير_cache_adopt_deferred(ڍير_2, 0);
  إذا (release_cache || ڍير_2->finalize) {
#إذا ENABLE_THREAD_CACHE
    لكل (size_t iclass = 0; iclass < LARGE_CLASS_ڳڻپ; ++iclass) {
      وسعت_cache_t *وسعت_cache;
      إذا (!iclass)
        وسعت_cache = &ڍير_2->وسعت_cache;
      وإلا
        وسعت_cache = (وسعت_cache_t *)(ڍير_2->وسعت_large_cache + (iclass - 1));
      إذا (!وسعت_cache->ڳڻپ)
        استمر;
#إذا ENABLE_GLOBAL_CACHE
      إذا (ڍير_2->finalize) {
        لكل (size_t ispan = 0; ispan < وسعت_cache->ڳڻپ; ++ispan)
          rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
      } وإلا {
        _rpmalloc_stat_add64(&ڍير_2->thread_منزل_global, وسعت_cache->ڳڻپ *
                                                          (iclass + 1) *
                                                          يادگيري_وسعت_سائيز);
        rpmalloc_stat_جوڙيو(&ڍير_2->وسعت_use[iclass].spans_منزل_global,
                           وسعت_cache->ڳڻپ);
        rpmalloc_global_cache_وجهو_spans(وسعت_cache->وسعت, iclass + 1,
                                            وسعت_cache->ڳڻپ);
      }
#وإلا
      لكل (size_t ispan = 0; ispan < وسعت_cache->ڳڻپ; ++ispan)
        rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
#جيڪڏهن_پورو
      وسعت_cache->ڳڻپ = 0;
    }
#جيڪڏهن_پورو
  }

  إذا (get_thread_ڍير_raw() == ڍير_2)
    سيٽ_thread_ڍير(0);

#إذا ENABLE_STATISTICS
  atomic_decr32(&يادگيري_سرگرم_heaps);
  rpmalloc_assert(atomic_load32(&يادگيري_سرگرم_heaps) >= 0,
                  "Still active heaps during finalization");
#جيڪڏهن_پورو

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  إذا (get_thread_id() != _rpmalloc_main_thread_id) {
    طالما (!atomic_cas32_acquire(&يادگيري_global_تالو, 1, 0))
      _rpmalloc_spin();
  }
  rpmalloc_ڍير_orphan(ڍير_2, پهريون_class);
  atomic_store32_release(&يادگيري_global_تالو, 0);
}

ساكن فراغ rpmalloc_ڍير_release_raw(فراغ *heapptr, صحيح release_cache) {
  rpmalloc_ڍير_release(heapptr, 0, release_cache);
}

ساكن فراغ rpmalloc_ڍير_release_raw_fc(فراغ *heapptr) {
  rpmalloc_ڍير_release_raw(heapptr, 1);
}

ساكن فراغ rpmalloc_ڍير_finalize(ڍير_t *ڍير_2) {
  إذا (ڍير_2->spans_reserved) {
    وسعت_t *وسعت = rpmalloc_وسعت_map(ڍير_2, ڍير_2->spans_reserved);
    rpmalloc_وسعت_unmap(وسعت);
    ڍير_2->spans_reserved = 0;
  }

  rpmalloc_ڍير_cache_adopt_deferred(ڍير_2, 0);

  لكل (size_t iclass = 0; iclass < سائيز_CLASS_ڳڻپ; ++iclass) {
    إذا (ڍير_2->سائيز_class[iclass].cache)
      rpmalloc_وسعت_unmap(ڍير_2->سائيز_class[iclass].cache);
    ڍير_2->سائيز_class[iclass].cache = 0;
    وسعت_t *وسعت = ڍير_2->سائيز_class[iclass].partial_وسعت;
    طالما (وسعت) {
      وسعت_t *ايندڙ = وسعت->ايندڙ;
      rpmalloc_وسعت_finalize(ڍير_2, iclass, وسعت,
                              &ڍير_2->سائيز_class[iclass].partial_وسعت);
      وسعت = ايندڙ;
    }
    // If class still has a free list it must be a full span
    إذا (ڍير_2->سائيز_class[iclass].free_list) {
      وسعت_t *class_وسعت =
          (وسعت_t *)((uintptr_t)ڍير_2->سائيز_class[iclass].free_list &
                     يادگيري_وسعت_نقاب);
      وسعت_t **list = 0;
#إذا RPMALLOC_پهريون_CLASS_HEAPS
      list = &ڍير_2->ڀريل_وسعت[iclass];
#جيڪڏهن_پورو
      --ڍير_2->ڀريل_وسعت_ڳڻپ;
      إذا (!rpmalloc_وسعت_finalize(ڍير_2, iclass, class_وسعت, list)) {
        إذا (list)
          rpmalloc_وسعت_double_link_list_remove(list, class_وسعت);
        rpmalloc_وسعت_double_link_list_جوڙيو(
            &ڍير_2->سائيز_class[iclass].partial_وسعت, class_وسعت);
      }
    }
  }

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_ڳڻپ; ++iclass) {
    وسعت_cache_t *وسعت_cache;
    إذا (!iclass)
      وسعت_cache = &ڍير_2->وسعت_cache;
    وإلا
      وسعت_cache = (وسعت_cache_t *)(ڍير_2->وسعت_large_cache + (iclass - 1));
    لكل (size_t ispan = 0; ispan < وسعت_cache->ڳڻپ; ++ispan)
      rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
    وسعت_cache->ڳڻپ = 0;
  }
#جيڪڏهن_پورو
  rpmalloc_assert(!atomic_ڀريو_ptr(&ڍير_2->وسعت_free_deferred),
                  "Heaps still active during finalization");
}

////////////
///
/// Allocation entry points
///
//////

//! Pop first block from a free list
ساكن فراغ *free_list_ڪڍو(فراغ **list) {
  فراغ *بلاڪ = *list;
  *list = *((فراغ **)بلاڪ);
  أرجع بلاڪ;
}

//! Allocate a small/medium sized memory block from the given heap
ساكن فراغ *rpmalloc_allocate_from_ڍير_fallback(
    ڍير_t *ڍير_2, ڍير_سائيز_class_t *ڍير_سائيز_class, uint32_t class_idx) {
  وسعت_t *وسعت = ڍير_سائيز_class->partial_وسعت;
  rpmalloc_assume(ڍير_2 != 0);
  إذا (EXPECTED(وسعت != 0)) {
    rpmalloc_assert(وسعت->بلاڪ_ڳڻپ ==
                        يادگيري_سائيز_class[وسعت->سائيز_class].بلاڪ_ڳڻپ,
                    "Span block count corrupted");
    rpmalloc_assert(!rpmalloc_وسعت_is_fully_utilized(وسعت),
                    "Internal failure");
    فراغ *بلاڪ;
    إذا (وسعت->free_list) {
      // Span local free list is not empty, swap to size class free list
      بلاڪ = free_list_ڪڍو(&وسعت->free_list);
      ڍير_سائيز_class->free_list = وسعت->free_list;
      وسعت->free_list = 0;
    } وإلا {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      فراغ *بلاڪ_شروعات = اشارو_وٿي(
          وسعت, وسعت_سرو_سائيز +
                    ((size_t)وسعت->free_list_حد_بندي * وسعت->بلاڪ_سائيز_2));
      وسعت->free_list_حد_بندي += free_list_partial_init(
          &ڍير_سائيز_class->free_list, &بلاڪ,
          (فراغ *)((uintptr_t)بلاڪ_شروعات & ~(يادگيري_صفحو_سائيز - 1)),
          بلاڪ_شروعات, وسعت->بلاڪ_ڳڻپ - وسعت->free_list_حد_بندي,
          وسعت->بلاڪ_سائيز_2);
    }
    rpmalloc_assert(وسعت->free_list_حد_بندي <= وسعت->بلاڪ_ڳڻپ,
                    "Span block count corrupted");
    وسعت->استعمال_ٿيل_ڳڻپ = وسعت->free_list_حد_بندي;

    // Swap in deferred free list if present
    إذا (atomic_ڀريو_ptr(&وسعت->free_list_deferred))
      rpmalloc_وسعت_ڪڍو_free_list_deferred(وسعت);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    إذا (!rpmalloc_وسعت_is_fully_utilized(وسعت))
      أرجع بلاڪ;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_وسعت_double_link_list_ڪڍو_سر(&ڍير_سائيز_class->partial_وسعت,
                                             وسعت);
#إذا RPMALLOC_پهريون_CLASS_HEAPS
    rpmalloc_وسعت_double_link_list_جوڙيو(&ڍير_2->ڀريل_وسعت[class_idx], وسعت);
#جيڪڏهن_پورو
    ++ڍير_2->ڀريل_وسعت_ڳڻپ;
    أرجع بلاڪ;
  }

  // Find a span in one of the cache levels
  وسعت = rpmalloc_ڍير_ڪڍو_new_وسعت(ڍير_2, ڍير_سائيز_class, 1, class_idx);
  إذا (EXPECTED(وسعت != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    أرجع rpmalloc_وسعت_initialize_new(ڍير_2, ڍير_سائيز_class, وسعت,
                                         class_idx);
  }

  أرجع 0;
}

//! Allocate a small sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_small(ڍير_t *ڍير_2, size_t سائيز_2) {
  rpmalloc_assert(ڍير_2, "No thread heap");
  // Small sizes have unique size classes
  ثابت uint32_t class_idx =
      (uint32_t)((سائيز_2 + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_سرڪايو);
  ڍير_سائيز_class_t *ڍير_سائيز_class = ڍير_2->سائيز_class + class_idx;
  _rpmalloc_stat_inc_alloc(ڍير_2, class_idx);
  إذا (EXPECTED(ڍير_سائيز_class->free_list != 0))
    أرجع free_list_ڪڍو(&ڍير_سائيز_class->free_list);
  أرجع rpmalloc_allocate_from_ڍير_fallback(ڍير_2, ڍير_سائيز_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_medium(ڍير_t *ڍير_2, size_t سائيز_2) {
  rpmalloc_assert(ڍير_2, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ثابت uint32_t بنياد_idx =
      (uint32_t)(SMALL_CLASS_ڳڻپ +
                 ((سائيز_2 - (SMALL_سائيز_حد_بندي + 1)) >> MEDIUM_GRANULARITY_سرڪايو));
  ثابت uint32_t class_idx = يادگيري_سائيز_class[بنياد_idx].class_idx;
  ڍير_سائيز_class_t *ڍير_سائيز_class = ڍير_2->سائيز_class + class_idx;
  _rpmalloc_stat_inc_alloc(ڍير_2, class_idx);
  إذا (EXPECTED(ڍير_سائيز_class->free_list != 0))
    أرجع free_list_ڪڍو(&ڍير_سائيز_class->free_list);
  أرجع rpmalloc_allocate_from_ڍير_fallback(ڍير_2, ڍير_سائيز_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_large(ڍير_t *ڍير_2, size_t سائيز_2) {
  rpmalloc_assert(ڍير_2, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  سائيز_2 += وسعت_سرو_سائيز;
  size_t وسعت_ڳڻپ = سائيز_2 >> يادگيري_وسعت_سائيز_سرڪايو;
  إذا (سائيز_2 & (يادگيري_وسعت_سائيز - 1))
    ++وسعت_ڳڻپ;

  // Find a span in one of the cache levels
  وسعت_t *وسعت =
      rpmalloc_ڍير_ڪڍو_new_وسعت(ڍير_2, 0, وسعت_ڳڻپ, سائيز_CLASS_LARGE);
  إذا (!وسعت)
    أرجع وسعت;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(وسعت->وسعت_ڳڻپ >= وسعت_ڳڻپ, "Internal failure");
  وسعت->سائيز_class = سائيز_CLASS_LARGE;
  وسعت->ڍير_2 = ڍير_2;

#إذا RPMALLOC_پهريون_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_جوڙيو(&ڍير_2->large_huge_وسعت, وسعت);
#جيڪڏهن_پورو
  ++ڍير_2->ڀريل_وسعت_ڳڻپ;

  أرجع اشارو_وٿي(وسعت, وسعت_سرو_سائيز);
}

//! Allocate a huge block by mapping memory pages directly
ساكن فراغ *_rpmalloc_allocate_huge(ڍير_t *ڍير_2, size_t سائيز_2) {
  rpmalloc_assert(ڍير_2, "No thread heap");
  rpmalloc_ڍير_cache_adopt_deferred(ڍير_2, 0);
  سائيز_2 += وسعت_سرو_سائيز;
  size_t انگ_صفحا = سائيز_2 >> يادگيري_صفحو_سائيز_سرڪايو;
  إذا (سائيز_2 & (يادگيري_صفحو_سائيز - 1))
    ++انگ_صفحا;
  size_t align_وٿي = 0;
  وسعت_t *وسعت =
      (وسعت_t *)_rpmalloc_mmap(انگ_صفحا * يادگيري_صفحو_سائيز, &align_وٿي);
  إذا (!وسعت)
    أرجع وسعت;

  // Store page count in span_count
  وسعت->سائيز_class = سائيز_CLASS_HUGE;
  وسعت->وسعت_ڳڻپ = (uint32_t)انگ_صفحا;
  وسعت->align_وٿي = (uint32_t)align_وٿي;
  وسعت->ڍير_2 = ڍير_2;
  rpmalloc_stat_جوڙيو_peak(&huge_صفحا_current, انگ_صفحا, huge_صفحا_peak);

#إذا RPMALLOC_پهريون_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_جوڙيو(&ڍير_2->large_huge_وسعت, وسعت);
#جيڪڏهن_پورو
  ++ڍير_2->ڀريل_وسعت_ڳڻپ;

  أرجع اشارو_وٿي(وسعت, وسعت_سرو_سائيز);
}

//! Allocate a block of the given size
ساكن فراغ *_rpmalloc_allocate(ڍير_t *ڍير_2, size_t سائيز_2) {
  _rpmalloc_stat_add64(&ورهاست_counter, 1);
  إذا (EXPECTED(سائيز_2 <= SMALL_سائيز_حد_بندي))
    أرجع _rpmalloc_allocate_small(ڍير_2, سائيز_2);
  وإلا إذا (سائيز_2 <= يادگيري_medium_سائيز_حد_بندي)
    أرجع _rpmalloc_allocate_medium(ڍير_2, سائيز_2);
  وإلا إذا (سائيز_2 <= LARGE_سائيز_حد_بندي)
    أرجع _rpmalloc_allocate_large(ڍير_2, سائيز_2);
  أرجع _rpmalloc_allocate_huge(ڍير_2, سائيز_2);
}

ساكن فراغ *_rpmalloc_aligned_allocate(ڍير_t *ڍير_2, size_t alignment,
                                        size_t سائيز_2) {
  إذا (alignment <= SMALL_GRANULARITY)
    أرجع _rpmalloc_allocate(ڍير_2, سائيز_2);

#إذا ENABLE_VALIDATE_دليلون
  إذا ((سائيز_2 + alignment) < سائيز_2) {
    errno = EINVAL;
    أرجع 0;
  }
  إذا (alignment & (alignment - 1)) {
    errno = EINVAL;
    أرجع 0;
  }
#جيڪڏهن_پورو

  إذا ((alignment <= وسعت_سرو_سائيز) &&
      ((سائيز_2 + وسعت_سرو_سائيز) < يادگيري_medium_سائيز_حد_بندي)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t گڻو_سائيز = سائيز_2 ? (سائيز_2 + (وسعت_سرو_سائيز - 1)) &
                                      ~(uintptr_t)(وسعت_سرو_سائيز - 1)
                                : وسعت_سرو_سائيز;
    rpmalloc_assert(!(گڻو_سائيز % وسعت_سرو_سائيز),
                    "Failed alignment calculation");
    إذا (گڻو_سائيز <= (سائيز_2 + alignment))
      أرجع _rpmalloc_allocate(ڍير_2, گڻو_سائيز);
  }

  فراغ *ptr = 0;
  size_t align_نقاب = alignment - 1;
  إذا (alignment <= يادگيري_صفحو_سائيز) {
    ptr = _rpmalloc_allocate(ڍير_2, سائيز_2 + alignment);
    إذا ((uintptr_t)ptr & align_نقاب) {
      ptr = (فراغ *)(((uintptr_t)ptr & ~(uintptr_t)align_نقاب) + alignment);
      // Mark as having aligned blocks
      وسعت_t *وسعت = (وسعت_t *)((uintptr_t)ptr & يادگيري_وسعت_نقاب);
      وسعت->flags |= وسعت_FLAG_ALIGNED_بلاڪون;
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
  إذا (alignment >= يادگيري_وسعت_سائيز) {
    errno = EINVAL;
    أرجع 0;
  }

  size_t extra_صفحا = alignment / يادگيري_صفحو_سائيز;

  // Since each span has a header, we will at least need one extra memory page
  size_t انگ_صفحا = 1 + (سائيز_2 / يادگيري_صفحو_سائيز);
  إذا (سائيز_2 & (يادگيري_صفحو_سائيز - 1))
    ++انگ_صفحا;

  إذا (extra_صفحا > انگ_صفحا)
    انگ_صفحا = 1 + extra_صفحا;

  size_t original_صفحا = انگ_صفحا;
  size_t حد_بندي_صفحا = (يادگيري_وسعت_سائيز / يادگيري_صفحو_سائيز) * 2;
  إذا (حد_بندي_صفحا < (original_صفحا * 2))
    حد_بندي_صفحا = original_صفحا * 2;

  size_t mapped_سائيز, align_وٿي;
  وسعت_t *وسعت;

retry:
  align_وٿي = 0;
  mapped_سائيز = انگ_صفحا * يادگيري_صفحو_سائيز;

  وسعت = (وسعت_t *)_rpmalloc_mmap(mapped_سائيز, &align_وٿي);
  إذا (!وسعت) {
    errno = ENOMEM;
    أرجع 0;
  }
  ptr = اشارو_وٿي(وسعت, وسعت_سرو_سائيز);

  إذا ((uintptr_t)ptr & align_نقاب)
    ptr = (فراغ *)(((uintptr_t)ptr & ~(uintptr_t)align_نقاب) + alignment);

  إذا (((size_t)اشارو_diff(ptr, وسعت) >= يادگيري_وسعت_سائيز) ||
      (اشارو_وٿي(ptr, سائيز_2) > اشارو_وٿي(وسعت, mapped_سائيز)) ||
      (((uintptr_t)ptr & يادگيري_وسعت_نقاب) != (uintptr_t)وسعت)) {
    _rpmalloc_unmap(وسعت, mapped_سائيز, align_وٿي, mapped_سائيز);
    ++انگ_صفحا;
    إذا (انگ_صفحا > حد_بندي_صفحا) {
      errno = EINVAL;
      أرجع 0;
    }
    اذهب retry;
  }

  // Store page count in span_count
  وسعت->سائيز_class = سائيز_CLASS_HUGE;
  وسعت->وسعت_ڳڻپ = (uint32_t)انگ_صفحا;
  وسعت->align_وٿي = (uint32_t)align_وٿي;
  وسعت->ڍير_2 = ڍير_2;
  rpmalloc_stat_جوڙيو_peak(&huge_صفحا_current, انگ_صفحا, huge_صفحا_peak);

#إذا RPMALLOC_پهريون_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_جوڙيو(&ڍير_2->large_huge_وسعت, وسعت);
#جيڪڏهن_پورو
  ++ڍير_2->ڀريل_وسعت_ڳڻپ;

  _rpmalloc_stat_add64(&ورهاست_counter, 1);

  أرجع ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
ساكن فراغ rpmalloc_deallocate_سڌو_small_or_medium(وسعت_t *وسعت,
                                                        فراغ *بلاڪ) {
  ڍير_t *ڍير_2 = وسعت->ڍير_2;
  rpmalloc_assert(ڍير_2->owner_thread == get_thread_id() ||
                      !ڍير_2->owner_thread || ڍير_2->finalize,
                  "Internal failure");
  // Add block to free list
  إذا (UNEXPECTED(rpmalloc_وسعت_is_fully_utilized(وسعت))) {
    وسعت->استعمال_ٿيل_ڳڻپ = وسعت->بلاڪ_ڳڻپ;
#إذا RPMALLOC_پهريون_CLASS_HEAPS
    rpmalloc_وسعت_double_link_list_remove(&ڍير_2->ڀريل_وسعت[وسعت->سائيز_class],
                                           وسعت);
#جيڪڏهن_پورو
    rpmalloc_وسعت_double_link_list_جوڙيو(
        &ڍير_2->سائيز_class[وسعت->سائيز_class].partial_وسعت, وسعت);
    --ڍير_2->ڀريل_وسعت_ڳڻپ;
  }
  *((فراغ **)بلاڪ) = وسعت->free_list;
  --وسعت->استعمال_ٿيل_ڳڻپ;
  وسعت->free_list = بلاڪ;
  إذا (UNEXPECTED(وسعت->استعمال_ٿيل_ڳڻپ == وسعت->list_سائيز)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    إذا (وسعت->استعمال_ٿيل_ڳڻپ) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      فراغ *free_list;
      افعل {
        free_list = atomic_exchange_ptr_acquire(&وسعت->free_list_deferred,
                                                INVALID_اشارو);
      } طالما (free_list == INVALID_اشارو);
      atomic_store_ptr_release(&وسعت->free_list_deferred, free_list);
    }
    rpmalloc_وسعت_double_link_list_remove(
        &ڍير_2->سائيز_class[وسعت->سائيز_class].partial_وسعت, وسعت);
    rpmalloc_وسعت_release_منزل_cache(ڍير_2, وسعت);
  }
}

ساكن فراغ rpmalloc_deallocate_defer_free_وسعت(ڍير_t *ڍير_2, وسعت_t *وسعت) {
  إذا (وسعت->سائيز_class != سائيز_CLASS_HUGE)
    _rpmalloc_stat_inc(&ڍير_2->وسعت_use[وسعت->وسعت_ڳڻپ - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  افعل {
    وسعت->free_list = (فراغ *)atomic_ڀريو_ptr(&ڍير_2->وسعت_free_deferred);
  } طالما (!atomic_cas_ptr(&ڍير_2->وسعت_free_deferred, وسعت, وسعت->free_list));
}

//! Put the block in the deferred free list of the owning span
ساكن فراغ _rpmalloc_deallocate_defer_small_or_medium(وسعت_t *وسعت,
                                                       فراغ *بلاڪ) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  فراغ *free_list;
  افعل {
    free_list =
        atomic_exchange_ptr_acquire(&وسعت->free_list_deferred, INVALID_اشارو);
  } طالما (free_list == INVALID_اشارو);
  *((فراغ **)بلاڪ) = free_list;
  uint32_t free_ڳڻپ = ++وسعت->list_سائيز;
  صحيح all_deferred_free = (free_ڳڻپ == وسعت->بلاڪ_ڳڻپ);
  atomic_store_ptr_release(&وسعت->free_list_deferred, بلاڪ);
  إذا (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    rpmalloc_deallocate_defer_free_وسعت(وسعت->ڍير_2, وسعت);
  }
}

ساكن فراغ _rpmalloc_deallocate_small_or_medium(وسعت_t *وسعت, فراغ *p) {
  _rpmalloc_stat_inc_free(وسعت->ڍير_2, وسعت->سائيز_class);
  إذا (وسعت->flags & وسعت_FLAG_ALIGNED_بلاڪون) {
    // Realign pointer to block start
    فراغ *بلاڪون_شروعات = اشارو_وٿي(وسعت, وسعت_سرو_سائيز);
    uint32_t بلاڪ_وٿي = (uint32_t)اشارو_diff(p, بلاڪون_شروعات);
    p = اشارو_وٿي(p, -(int32_t)(بلاڪ_وٿي % وسعت->بلاڪ_سائيز_2));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#إذا RPMALLOC_پهريون_CLASS_HEAPS
  صحيح defer =
      (وسعت->ڍير_2->owner_thread &&
       (وسعت->ڍير_2->owner_thread != get_thread_id()) && !وسعت->ڍير_2->finalize);
#وإلا
  صحيح defer =
      ((وسعت->ڍير_2->owner_thread != get_thread_id()) && !وسعت->ڍير_2->finalize);
#جيڪڏهن_پورو
  إذا (!defer)
    rpmalloc_deallocate_سڌو_small_or_medium(وسعت, p);
  وإلا
    _rpmalloc_deallocate_defer_small_or_medium(وسعت, p);
}

//! Deallocate the given large memory block to the current heap
ساكن فراغ _rpmalloc_deallocate_large(وسعت_t *وسعت) {
  rpmalloc_assert(وسعت->سائيز_class == سائيز_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(وسعت->flags & وسعت_FLAG_MASTER) ||
                      !(وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((وسعت->flags & وسعت_FLAG_MASTER) ||
                      (وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#إذا RPMALLOC_پهريون_CLASS_HEAPS
  صحيح defer =
      (وسعت->ڍير_2->owner_thread &&
       (وسعت->ڍير_2->owner_thread != get_thread_id()) && !وسعت->ڍير_2->finalize);
#وإلا
  صحيح defer =
      ((وسعت->ڍير_2->owner_thread != get_thread_id()) && !وسعت->ڍير_2->finalize);
#جيڪڏهن_پورو
  إذا (defer) {
    rpmalloc_deallocate_defer_free_وسعت(وسعت->ڍير_2, وسعت);
    أرجع;
  }
  rpmalloc_assert(وسعت->ڍير_2->ڀريل_وسعت_ڳڻپ, "Heap span counter corrupted");
  --وسعت->ڍير_2->ڀريل_وسعت_ڳڻپ;
#إذا RPMALLOC_پهريون_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_remove(&وسعت->ڍير_2->large_huge_وسعت, وسعت);
#جيڪڏهن_پورو
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = وسعت->وسعت_ڳڻپ - 1;
  atomic_decr32(&وسعت->ڍير_2->وسعت_use[idx].current);
#جيڪڏهن_پورو
  ڍير_t *ڍير_2 = وسعت->ڍير_2;
  rpmalloc_assert(ڍير_2, "No thread heap");
#إذا ENABLE_THREAD_CACHE
  ثابت صحيح سيٽ_as_reserved =
      ((وسعت->وسعت_ڳڻپ > 1) && (ڍير_2->وسعت_cache.ڳڻپ == 0) &&
       !ڍير_2->finalize && !ڍير_2->spans_reserved);
#وإلا
  ثابت صحيح سيٽ_as_reserved =
      ((وسعت->وسعت_ڳڻپ > 1) && !ڍير_2->finalize && !ڍير_2->spans_reserved);
#جيڪڏهن_پورو
  إذا (سيٽ_as_reserved) {
    ڍير_2->وسعت_reserve = وسعت;
    ڍير_2->spans_reserved = وسعت->وسعت_ڳڻپ;
    إذا (وسعت->flags & وسعت_FLAG_MASTER) {
      ڍير_2->وسعت_reserve_master = وسعت;
    } وإلا { // SPAN_FLAG_SUBSPAN
      وسعت_t *master = (وسعت_t *)اشارو_وٿي(
          وسعت,
          -(intptr_t)((size_t)وسعت->وٿي_from_master * يادگيري_وسعت_سائيز));
      ڍير_2->وسعت_reserve_master = master;
      rpmalloc_assert(master->flags & وسعت_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->بچيل_spans) >=
                          (int32_t)وسعت->وسعت_ڳڻپ,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&ڍير_2->وسعت_use[idx].spans_منزل_reserved);
  } وإلا {
    // Insert into cache list
    rpmalloc_ڍير_cache_وجهو(ڍير_2, وسعت);
  }
}

//! Deallocate the given huge span
ساكن فراغ _rpmalloc_deallocate_huge(وسعت_t *وسعت) {
  rpmalloc_assert(وسعت->ڍير_2, "No span heap");
#إذا RPMALLOC_پهريون_CLASS_HEAPS
  صحيح defer =
      (وسعت->ڍير_2->owner_thread &&
       (وسعت->ڍير_2->owner_thread != get_thread_id()) && !وسعت->ڍير_2->finalize);
#وإلا
  صحيح defer =
      ((وسعت->ڍير_2->owner_thread != get_thread_id()) && !وسعت->ڍير_2->finalize);
#جيڪڏهن_پورو
  إذا (defer) {
    rpmalloc_deallocate_defer_free_وسعت(وسعت->ڍير_2, وسعت);
    أرجع;
  }
  rpmalloc_assert(وسعت->ڍير_2->ڀريل_وسعت_ڳڻپ, "Heap span counter corrupted");
  --وسعت->ڍير_2->ڀريل_وسعت_ڳڻپ;
#إذا RPMALLOC_پهريون_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_remove(&وسعت->ڍير_2->large_huge_وسعت, وسعت);
#جيڪڏهن_پورو

  // Oversized allocation, page count is stored in span_count
  size_t انگ_صفحا = وسعت->وسعت_ڳڻپ;
  _rpmalloc_unmap(وسعت, انگ_صفحا * يادگيري_صفحو_سائيز, وسعت->align_وٿي,
                  انگ_صفحا * يادگيري_صفحو_سائيز);
  _rpmalloc_stat_sub(&huge_صفحا_current, انگ_صفحا);
}

//! Deallocate the given block
ساكن فراغ _rpmalloc_deallocate(فراغ *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  وسعت_t *وسعت = (وسعت_t *)((uintptr_t)p & يادگيري_وسعت_نقاب);
  إذا (UNEXPECTED(!وسعت))
    أرجع;
  إذا (EXPECTED(وسعت->سائيز_class < سائيز_CLASS_ڳڻپ))
    _rpmalloc_deallocate_small_or_medium(وسعت, p);
  وإلا إذا (وسعت->سائيز_class == سائيز_CLASS_LARGE)
    _rpmalloc_deallocate_large(وسعت);
  وإلا
    _rpmalloc_deallocate_huge(وسعت);
}

////////////
///
/// Reallocation entry points
///
//////

ساكن size_t rpmalloc_usable_سائيز_2(فراغ *p);

//! Reallocate the given block to the given size
ساكن فراغ *_rpmalloc_reallocate(ڍير_t *ڍير_2, فراغ *p, size_t سائيز_2,
                                  size_t oldsize, بنانشان صحيح flags) {
  إذا (p) {
    // Grab the span using guaranteed span alignment
    وسعت_t *وسعت = (وسعت_t *)((uintptr_t)p & يادگيري_وسعت_نقاب);
    إذا (EXPECTED(وسعت->سائيز_class < سائيز_CLASS_ڳڻپ)) {
      // Small/medium sized block
      rpmalloc_assert(وسعت->وسعت_ڳڻپ == 1, "Span counter corrupted");
      فراغ *بلاڪون_شروعات = اشارو_وٿي(وسعت, وسعت_سرو_سائيز);
      uint32_t بلاڪ_وٿي = (uint32_t)اشارو_diff(p, بلاڪون_شروعات);
      uint32_t بلاڪ_idx = بلاڪ_وٿي / وسعت->بلاڪ_سائيز_2;
      فراغ *بلاڪ =
          اشارو_وٿي(بلاڪون_شروعات, (size_t)بلاڪ_idx * وسعت->بلاڪ_سائيز_2);
      إذا (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)وسعت->بلاڪ_سائيز_2 - اشارو_diff(p, بلاڪ));
      إذا ((size_t)وسعت->بلاڪ_سائيز_2 >= سائيز_2) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != بلاڪ) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلاڪ, p, oldsize);
        أرجع بلاڪ;
      }
    } وإلا إذا (وسعت->سائيز_class == سائيز_CLASS_LARGE) {
      // Large block
      size_t ڪل_سائيز_2 = سائيز_2 + وسعت_سرو_سائيز;
      size_t انگ_spans = ڪل_سائيز_2 >> يادگيري_وسعت_سائيز_سرڪايو;
      إذا (ڪل_سائيز_2 & (يادگيري_وسعت_نقاب - 1))
        ++انگ_spans;
      size_t current_spans = وسعت->وسعت_ڳڻپ;
      فراغ *بلاڪ = اشارو_وٿي(وسعت, وسعت_سرو_سائيز);
      إذا (!oldsize)
        oldsize = (current_spans * يادگيري_وسعت_سائيز) -
                  (size_t)اشارو_diff(p, بلاڪ) - وسعت_سرو_سائيز;
      إذا ((current_spans >= انگ_spans) && (ڪل_سائيز_2 >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != بلاڪ) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلاڪ, p, oldsize);
        أرجع بلاڪ;
      }
    } وإلا {
      // Oversized block
      size_t ڪل_سائيز_2 = سائيز_2 + وسعت_سرو_سائيز;
      size_t انگ_صفحا = ڪل_سائيز_2 >> يادگيري_صفحو_سائيز_سرڪايو;
      إذا (ڪل_سائيز_2 & (يادگيري_صفحو_سائيز - 1))
        ++انگ_صفحا;
      // Page count is stored in span_count
      size_t current_صفحا = وسعت->وسعت_ڳڻپ;
      فراغ *بلاڪ = اشارو_وٿي(وسعت, وسعت_سرو_سائيز);
      إذا (!oldsize)
        oldsize = (current_صفحا * يادگيري_صفحو_سائيز) -
                  (size_t)اشارو_diff(p, بلاڪ) - وسعت_سرو_سائيز;
      إذا ((current_صفحا >= انگ_صفحا) && (انگ_صفحا >= (current_صفحا / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != بلاڪ) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلاڪ, p, oldsize);
        أرجع بلاڪ;
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
  size_t هيٺاهون_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_سائيز =
      (سائيز_2 > هيٺاهون_bound) ? سائيز_2 : ((سائيز_2 > oldsize) ? هيٺاهون_bound : سائيز_2);
  فراغ *بلاڪ = _rpmalloc_allocate(ڍير_2, new_سائيز);
  إذا (p && بلاڪ) {
    إذا (!(flags & RPMALLOC_NO_PRESERVE))
      يادگيري_نقل(بلاڪ, p, oldsize < new_سائيز ? oldsize : new_سائيز);
    _rpmalloc_deallocate(p);
  }

  أرجع بلاڪ;
}

ساكن فراغ *_rpmalloc_aligned_reallocate(ڍير_t *ڍير_2, فراغ *ptr,
                                          size_t alignment, size_t سائيز_2,
                                          size_t oldsize, بنانشان صحيح flags) {
  إذا (alignment <= SMALL_GRANULARITY)
    أرجع _rpmalloc_reallocate(ڍير_2, ptr, سائيز_2, oldsize, flags);

  صحيح no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? rpmalloc_usable_سائيز_2(ptr) : 0);
  إذا ((usablesize >= سائيز_2) && !((uintptr_t)ptr & (alignment - 1))) {
    إذا (no_alloc || (سائيز_2 >= (usablesize / 2)))
      أرجع ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  فراغ *بلاڪ =
      (!no_alloc ? _rpmalloc_aligned_allocate(ڍير_2, alignment, سائيز_2) : 0);
  إذا (EXPECTED(بلاڪ != 0)) {
    إذا (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      إذا (!oldsize)
        oldsize = usablesize;
      يادگيري_نقل(بلاڪ, ptr, oldsize < سائيز_2 ? oldsize : سائيز_2);
    }
    _rpmalloc_deallocate(ptr);
  }
  أرجع بلاڪ;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
ساكن size_t rpmalloc_usable_سائيز_2(فراغ *p) {
  // Grab the span using guaranteed span alignment
  وسعت_t *وسعت = (وسعت_t *)((uintptr_t)p & يادگيري_وسعت_نقاب);
  إذا (وسعت->سائيز_class < سائيز_CLASS_ڳڻپ) {
    // Small/medium block
    فراغ *بلاڪون_شروعات = اشارو_وٿي(وسعت, وسعت_سرو_سائيز);
    أرجع وسعت->بلاڪ_سائيز_2 -
           ((size_t)اشارو_diff(p, بلاڪون_شروعات) % وسعت->بلاڪ_سائيز_2);
  }
  إذا (وسعت->سائيز_class == سائيز_CLASS_LARGE) {
    // Large block
    size_t current_spans = وسعت->وسعت_ڳڻپ;
    أرجع (current_spans * يادگيري_وسعت_سائيز) - (size_t)اشارو_diff(p, وسعت);
  }
  // Oversized block, page count is stored in span_count
  size_t current_صفحا = وسعت->وسعت_ڳڻپ;
  أرجع (current_صفحا * يادگيري_صفحو_سائيز) - (size_t)اشارو_diff(p, وسعت);
}

//! Adjust and optimize the size class properties for the given class
ساكن فراغ rpmalloc_adjust_سائيز_class(size_t iclass) {
  size_t بلاڪ_سائيز_2 = يادگيري_سائيز_class[iclass].بلاڪ_سائيز_2;
  size_t بلاڪ_ڳڻپ = (يادگيري_وسعت_سائيز - وسعت_سرو_سائيز) / بلاڪ_سائيز_2;

  يادگيري_سائيز_class[iclass].بلاڪ_ڳڻپ = (uint16_t)بلاڪ_ڳڻپ;
  يادگيري_سائيز_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  إذا (iclass >= SMALL_CLASS_ڳڻپ) {
    size_t prevclass = iclass;
    طالما (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      إذا (يادگيري_سائيز_class[prevclass].بلاڪ_ڳڻپ ==
          يادگيري_سائيز_class[iclass].بلاڪ_ڳڻپ)
        _rpmalloc_memcpy_const(يادگيري_سائيز_class + prevclass,
                               يادگيري_سائيز_class + iclass,
                               حجم(يادگيري_سائيز_class[iclass]));
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
    يادگيري_نقل(&يادگيري_config, config, حجم(rpmalloc_config_t));
  وإلا
    _rpmalloc_memset_const(&يادگيري_config, 0, حجم(rpmalloc_config_t));

  إذا (!يادگيري_config.يادگيري_map || !يادگيري_config.يادگيري_unmap) {
    يادگيري_config.يادگيري_map = _rpmalloc_mmap_os;
    يادگيري_config.يادگيري_unmap = _rpmalloc_unmap_os;
  }

#إذا PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, حجم(system_info));
  GetSystemInfo(&system_info);
  يادگيري_map_granularity = system_info.dw_ورهاست_Granularity;
#وإلا
  يادگيري_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#جيڪڏهن_پورو

#إذا RPMALLOC_CONFIGURABLE
  يادگيري_صفحو_سائيز = يادگيري_config.صفحو_سائيز_2;
#وإلا
  يادگيري_صفحو_سائيز = 0;
#جيڪڏهن_پورو
  يادگيري_huge_صفحا = 0;
  إذا (!يادگيري_صفحو_سائيز) {
#إذا PLATFORM_WINDOWS
    يادگيري_صفحو_سائيز = system_info.dw_صفحو_سائيز;
#وإلا
    يادگيري_صفحو_سائيز = يادگيري_map_granularity;
    إذا (يادگيري_config.enable_huge_صفحا) {
#إذا defined(__linux__)
      size_t huge_صفحو_سائيز = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      إذا (meminfo) {
        حرف سٽ_2[128];
        طالما (!huge_صفحو_سائيز && fgets(سٽ_2, حجم(سٽ_2) - 1, meminfo)) {
          سٽ_2[حجم(سٽ_2) - 1] = 0;
          إذا (لڙي_ڳولا(سٽ_2, "Hugepagesize:"))
            huge_صفحو_سائيز = (size_t)strtol(سٽ_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      إذا (huge_صفحو_سائيز) {
        يادگيري_huge_صفحا = 1;
        يادگيري_صفحو_سائيز = huge_صفحو_سائيز;
        يادگيري_map_granularity = huge_صفحو_سائيز;
      }
#وإلاإذا defined(__FreeBSD__)
      صحيح rc;
      size_t sz = حجم(rc);

      إذا (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, عدم, 0) == 0 &&
          rc == 1) {
        ساكن size_t defsize = 2 * 1024 * 1024;
        صحيح nsize = 0;
        size_t sizes[4] = {0};
        يادگيري_huge_صفحا = 1;
        يادگيري_صفحو_سائيز = defsize;
        إذا ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          لكل (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            إذا (defsize < csize) {
              يادگيري_صفحو_سائيز = csize;
              اكسر;
            }
          }
        }
        يادگيري_map_granularity = يادگيري_صفحو_سائيز;
      }
#وإلاإذا defined(__APPLE__) || defined(__NetBSD__)
      يادگيري_huge_صفحا = 1;
      يادگيري_صفحو_سائيز = 2 * 1024 * 1024;
      يادگيري_map_granularity = يادگيري_صفحو_سائيز;
#جيڪڏهن_پورو
    }
#جيڪڏهن_پورو
  } وإلا {
    إذا (يادگيري_config.enable_huge_صفحا)
      يادگيري_huge_صفحا = 1;
  }

#إذا PLATFORM_WINDOWS
  إذا (يادگيري_config.enable_huge_صفحا) {
    HANDLE token = 0;
    size_t large_صفحو_minimum = Get_Large_صفحو_Minimum();
    إذا (large_صفحو_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    إذا (token) {
      LUID luid;
      إذا (Lookup_Privilege_قدر(0, SE_تالو_يادگيري_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, حجم(token_privileges));
        token_privileges.Privilege_ڳڻپ = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].خاصيتون = SE_PRIVILEGE_ENABLED;
        إذا (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          إذا (Get_آخري_Error() == ERROR_SUCCESS)
            يادگيري_huge_صفحا = 1;
        }
      }
      CloseHandle(token);
    }
    إذا (يادگيري_huge_صفحا) {
      إذا (large_صفحو_minimum > يادگيري_صفحو_سائيز)
        يادگيري_صفحو_سائيز = large_صفحو_minimum;
      إذا (large_صفحو_minimum > يادگيري_map_granularity)
        يادگيري_map_granularity = large_صفحو_minimum;
    }
  }
#جيڪڏهن_پورو

  size_t min_وسعت_سائيز = 256;
  size_t max_صفحو_سائيز;
#إذا UINTPTR_MAX > 0xFFFFFFFF
  max_صفحو_سائيز = 4096ULL * 1024ULL * 1024ULL;
#وإلا
  max_صفحو_سائيز = 4 * 1024 * 1024;
#جيڪڏهن_پورو
  إذا (يادگيري_صفحو_سائيز < min_وسعت_سائيز)
    يادگيري_صفحو_سائيز = min_وسعت_سائيز;
  إذا (يادگيري_صفحو_سائيز > max_صفحو_سائيز)
    يادگيري_صفحو_سائيز = max_صفحو_سائيز;
  يادگيري_صفحو_سائيز_سرڪايو = 0;
  size_t صفحو_سائيز_ٻٽو_انگ = يادگيري_صفحو_سائيز;
  طالما (صفحو_سائيز_ٻٽو_انگ != 1) {
    ++يادگيري_صفحو_سائيز_سرڪايو;
    صفحو_سائيز_ٻٽو_انگ >>= 1;
  }
  يادگيري_صفحو_سائيز = ((size_t)1 << يادگيري_صفحو_سائيز_سرڪايو);

#إذا RPMALLOC_CONFIGURABLE
  إذا (!يادگيري_config.وسعت_سائيز) {
    يادگيري_وسعت_سائيز = يادگيري_default_وسعت_سائيز;
    يادگيري_وسعت_سائيز_سرڪايو = يادگيري_default_وسعت_سائيز_سرڪايو;
    يادگيري_وسعت_نقاب = يادگيري_default_وسعت_نقاب;
  } وإلا {
    size_t وسعت_سائيز = يادگيري_config.وسعت_سائيز;
    إذا (وسعت_سائيز > (256 * 1024))
      وسعت_سائيز = (256 * 1024);
    يادگيري_وسعت_سائيز = 4096;
    يادگيري_وسعت_سائيز_سرڪايو = 12;
    طالما (يادگيري_وسعت_سائيز < وسعت_سائيز) {
      يادگيري_وسعت_سائيز <<= 1;
      ++يادگيري_وسعت_سائيز_سرڪايو;
    }
    يادگيري_وسعت_نقاب = ~(uintptr_t)(يادگيري_وسعت_سائيز - 1);
  }
#جيڪڏهن_پورو

  يادگيري_وسعت_map_ڳڻپ =
      (يادگيري_config.وسعت_map_ڳڻپ ? يادگيري_config.وسعت_map_ڳڻپ
                                     : DEFAULT_وسعت_MAP_ڳڻپ);
  إذا ((يادگيري_وسعت_سائيز * يادگيري_وسعت_map_ڳڻپ) < يادگيري_صفحو_سائيز)
    يادگيري_وسعت_map_ڳڻپ = (يادگيري_صفحو_سائيز / يادگيري_وسعت_سائيز);
  إذا ((يادگيري_صفحو_سائيز >= يادگيري_وسعت_سائيز) &&
      ((يادگيري_وسعت_map_ڳڻپ * يادگيري_وسعت_سائيز) % يادگيري_صفحو_سائيز))
    يادگيري_وسعت_map_ڳڻپ = (يادگيري_صفحو_سائيز / يادگيري_وسعت_سائيز);
  يادگيري_ڍير_reserve_ڳڻپ = (يادگيري_وسعت_map_ڳڻپ > DEFAULT_وسعت_MAP_ڳڻپ)
                                   ? DEFAULT_وسعت_MAP_ڳڻپ
                                   : يادگيري_وسعت_map_ڳڻپ;

  يادگيري_config.صفحو_سائيز_2 = يادگيري_صفحو_سائيز;
  يادگيري_config.وسعت_سائيز = يادگيري_وسعت_سائيز;
  يادگيري_config.وسعت_map_ڳڻپ = يادگيري_وسعت_map_ڳڻپ;
  يادگيري_config.enable_huge_صفحا = يادگيري_huge_صفحا;

#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  إذا (pthread_ڪنجي_create(&يادگيري_thread_ڍير, rpmalloc_ڍير_release_raw_fc))
    أرجع -1;
#جيڪڏهن_پورو
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_ڪنجي = FlsAlloc(&_rpmalloc_thread_destructor);
#جيڪڏهن_پورو

  // Setup all small and medium size classes
  size_t iclass = 0;
  يادگيري_سائيز_class[iclass].بلاڪ_سائيز_2 = SMALL_GRANULARITY;
  rpmalloc_adjust_سائيز_class(iclass);
  لكل (iclass = 1; iclass < SMALL_CLASS_ڳڻپ; ++iclass) {
    size_t سائيز_2 = iclass * SMALL_GRANULARITY;
    يادگيري_سائيز_class[iclass].بلاڪ_سائيز_2 = (uint32_t)سائيز_2;
    rpmalloc_adjust_سائيز_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  يادگيري_medium_سائيز_حد_بندي = (يادگيري_وسعت_سائيز - وسعت_سرو_سائيز) >> 1;
  إذا (يادگيري_medium_سائيز_حد_بندي > MEDIUM_سائيز_حد_بندي)
    يادگيري_medium_سائيز_حد_بندي = MEDIUM_سائيز_حد_بندي;
  لكل (iclass = 0; iclass < MEDIUM_CLASS_ڳڻپ; ++iclass) {
    size_t سائيز_2 = SMALL_سائيز_حد_بندي + ((iclass + 1) * MEDIUM_GRANULARITY);
    إذا (سائيز_2 > يادگيري_medium_سائيز_حد_بندي) {
      يادگيري_medium_سائيز_حد_بندي =
          SMALL_سائيز_حد_بندي + (iclass * MEDIUM_GRANULARITY);
      اكسر;
    }
    يادگيري_سائيز_class[SMALL_CLASS_ڳڻپ + iclass].بلاڪ_سائيز_2 = (uint32_t)سائيز_2;
    rpmalloc_adjust_سائيز_class(SMALL_CLASS_ڳڻپ + iclass);
  }

  يادگيري_orphan_heaps = 0;
#إذا RPMALLOC_پهريون_CLASS_HEAPS
  يادگيري_پهريون_class_orphan_heaps = 0;
#جيڪڏهن_پورو
#إذا ENABLE_STATISTICS
  atomic_store32(&يادگيري_سرگرم_heaps, 0);
  atomic_store32(&mapped_صفحا, 0);
  mapped_صفحا_peak = 0;
  atomic_store32(&_master_spans, 0);
  atomic_store32(&mapped_ڪل, 0);
  atomic_store32(&unmapped_ڪل, 0);
  atomic_store32(&mapped_صفحا_os, 0);
  atomic_store32(&huge_صفحا_current, 0);
  huge_صفحا_peak = 0;
#جيڪڏهن_پورو
  memset(يادگيري_heaps, 0, حجم(يادگيري_heaps));
  atomic_store32_release(&يادگيري_global_تالو, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  أرجع 0;
}

//! Finalize the allocator
فراغ rpmalloc_finalize(فراغ) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  إذا (يادگيري_global_reserve) {
    atomic_add32(&يادگيري_global_reserve_master->بچيل_spans,
                 -(int32_t)يادگيري_global_reserve_ڳڻپ);
    يادگيري_global_reserve_master = 0;
    يادگيري_global_reserve_ڳڻپ = 0;
    يادگيري_global_reserve = 0;
  }
  atomic_store32_release(&يادگيري_global_تالو, 0);

  // Free all thread caches and fully free spans
  لكل (size_t list_idx = 0; list_idx < ڍير_ARRAY_سائيز; ++list_idx) {
    ڍير_t *ڍير_2 = يادگيري_heaps[list_idx];
    طالما (ڍير_2) {
      ڍير_t *ايندڙ_ڍير = ڍير_2->ايندڙ_ڍير;
      ڍير_2->finalize = 1;
      rpmalloc_ڍير_global_finalize(ڍير_2);
      ڍير_2 = ايندڙ_ڍير;
    }
  }

#إذا ENABLE_GLOBAL_CACHE
  // Free global caches
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_ڳڻپ; ++iclass)
    _rpmalloc_global_cache_finalize(&يادگيري_وسعت_cache[iclass]);
#جيڪڏهن_پورو

#إذا (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_ڪنجي_delete(يادگيري_thread_ڍير);
#جيڪڏهن_پورو
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_ڪنجي);
  fls_ڪنجي = 0;
#جيڪڏهن_پورو
#إذا ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&mapped_صفحا) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&mapped_صفحا_os) == 0,
                  "Memory leak detected");
#جيڪڏهن_پورو

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
خارجي مضمن فراغ rpmalloc_thread_initialize(فراغ) {
  إذا (!get_thread_ڍير_raw()) {
    ڍير_t *ڍير_2 = rpmalloc_ڍير_allocate(0);
    إذا (ڍير_2) {
      _rpmalloc_stat_inc(&يادگيري_سرگرم_heaps);
      سيٽ_thread_ڍير(ڍير_2);
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_سيٽ_قدر(fls_ڪنجي, ڍير_2);
#جيڪڏهن_پورو
    }
  }
}

//! Finalize thread, orphan heap
فراغ rpmalloc_thread_finalize(صحيح release_caches) {
  ڍير_t *ڍير_2 = get_thread_ڍير_raw();
  إذا (ڍير_2)
    rpmalloc_ڍير_release_raw(ڍير_2, release_caches);
  سيٽ_thread_ڍير(0);
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_سيٽ_قدر(fls_ڪنجي, 0);
#جيڪڏهن_پورو
}

صحيح rpmalloc_is_thread_initialized(فراغ) {
  أرجع (get_thread_ڍير_raw() != 0) ? 1 : 0;
}

ثابت rpmalloc_config_t *rpmalloc_config(فراغ) { أرجع &يادگيري_config; }

// Extern interface

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpmalloc(size_t سائيز_2) {
#إذا ENABLE_VALIDATE_دليلون
  إذا (سائيز_2 >= MAX_ALLOC_سائيز) {
    errno = EINVAL;
    أرجع 0;
  }
#جيڪڏهن_پورو
  ڍير_t *ڍير_2 = get_thread_ڍير();
  أرجع _rpmalloc_allocate(ڍير_2, سائيز_2);
}

خارجي مضمن فراغ rpfree(فراغ *ptr) { _rpmalloc_deallocate(ptr); }

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpcalloc(size_t انگ_2, size_t سائيز_2) {
  size_t ڪل_2;
#إذا ENABLE_VALIDATE_دليلون
#إذا PLATFORM_WINDOWS
  صحيح err = سائيز_TMult(انگ_2, سائيز_2, &ڪل_2);
  إذا ((err != S_OK) || (ڪل_2 >= MAX_ALLOC_سائيز)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(انگ_2, سائيز_2, &ڪل_2);
  إذا (err || (ڪل_2 >= MAX_ALLOC_سائيز)) {
    errno = EINVAL;
    أرجع 0;
  }
#جيڪڏهن_پورو
#وإلا
  ڪل_2 = انگ_2 * سائيز_2;
#جيڪڏهن_پورو
  ڍير_t *ڍير_2 = get_thread_ڍير();
  فراغ *بلاڪ = _rpmalloc_allocate(ڍير_2, ڪل_2);
  إذا (بلاڪ)
    memset(بلاڪ, 0, ڪل_2);
  أرجع بلاڪ;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rprealloc(فراغ *ptr, size_t سائيز_2) {
#إذا ENABLE_VALIDATE_دليلون
  إذا (سائيز_2 >= MAX_ALLOC_سائيز) {
    errno = EINVAL;
    أرجع ptr;
  }
#جيڪڏهن_پورو
  ڍير_t *ڍير_2 = get_thread_ڍير();
  أرجع _rpmalloc_reallocate(ڍير_2, ptr, سائيز_2, 0, 0);
}

خارجي RPMALLOC_ALLOCATOR فراغ *rpaligned_realloc(فراغ *ptr, size_t alignment,
                                                  size_t سائيز_2, size_t oldsize,
                                                  بنانشان صحيح flags) {
#إذا ENABLE_VALIDATE_دليلون
  إذا ((سائيز_2 + alignment < سائيز_2) || (alignment > يادگيري_صفحو_سائيز)) {
    errno = EINVAL;
    أرجع 0;
  }
#جيڪڏهن_پورو
  ڍير_t *ڍير_2 = get_thread_ڍير();
  أرجع _rpmalloc_aligned_reallocate(ڍير_2, ptr, alignment, سائيز_2, oldsize,
                                      flags);
}

خارجي RPMALLOC_ALLOCATOR فراغ *rpaligned_alloc(size_t alignment, size_t سائيز_2) {
  ڍير_t *ڍير_2 = get_thread_ڍير();
  أرجع _rpmalloc_aligned_allocate(ڍير_2, alignment, سائيز_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpaligned_calloc(size_t alignment, size_t انگ_2, size_t سائيز_2) {
  size_t ڪل_2;
#إذا ENABLE_VALIDATE_دليلون
#إذا PLATFORM_WINDOWS
  صحيح err = سائيز_TMult(انگ_2, سائيز_2, &ڪل_2);
  إذا ((err != S_OK) || (ڪل_2 >= MAX_ALLOC_سائيز)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(انگ_2, سائيز_2, &ڪل_2);
  إذا (err || (ڪل_2 >= MAX_ALLOC_سائيز)) {
    errno = EINVAL;
    أرجع 0;
  }
#جيڪڏهن_پورو
#وإلا
  ڪل_2 = انگ_2 * سائيز_2;
#جيڪڏهن_پورو
  فراغ *بلاڪ = rpaligned_alloc(alignment, ڪل_2);
  إذا (بلاڪ)
    memset(بلاڪ, 0, ڪل_2);
  أرجع بلاڪ;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpmemalign(size_t alignment,
                                                  size_t سائيز_2) {
  أرجع rpaligned_alloc(alignment, سائيز_2);
}

خارجي مضمن صحيح rpposix_memalign(فراغ **memptr, size_t alignment,
                                   size_t سائيز_2) {
  إذا (memptr)
    *memptr = rpaligned_alloc(alignment, سائيز_2);
  وإلا
    أرجع EINVAL;
  أرجع *memptr ? 0 : ENOMEM;
}

خارجي مضمن size_t rpmalloc_usable_سائيز(فراغ *ptr) {
  أرجع (ptr ? rpmalloc_usable_سائيز_2(ptr) : 0);
}

خارجي مضمن فراغ rpmalloc_thread_collect(فراغ) {}

فراغ rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, حجم(rpmalloc_thread_statistics_t));
  ڍير_t *ڍير_2 = get_thread_ڍير_raw();
  إذا (!ڍير_2)
    أرجع;

  لكل (size_t iclass = 0; iclass < سائيز_CLASS_ڳڻپ; ++iclass) {
    سائيز_class_t *سائيز_class = يادگيري_سائيز_class + iclass;
    وسعت_t *وسعت = ڍير_2->سائيز_class[iclass].partial_وسعت;
    طالما (وسعت) {
      size_t free_ڳڻپ = وسعت->list_سائيز;
      size_t بلاڪ_ڳڻپ = سائيز_class->بلاڪ_ڳڻپ;
      إذا (وسعت->free_list_حد_بندي < بلاڪ_ڳڻپ)
        بلاڪ_ڳڻپ = وسعت->free_list_حد_بندي;
      free_ڳڻپ += (بلاڪ_ڳڻپ - وسعت->استعمال_ٿيل_ڳڻپ);
      stats->sizecache += free_ڳڻپ * سائيز_class->بلاڪ_سائيز_2;
      وسعت = وسعت->ايندڙ;
    }
  }

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_ڳڻپ; ++iclass) {
    وسعت_cache_t *وسعت_cache;
    إذا (!iclass)
      وسعت_cache = &ڍير_2->وسعت_cache;
    وإلا
      وسعت_cache = (وسعت_cache_t *)(ڍير_2->وسعت_large_cache + (iclass - 1));
    stats->spancache += وسعت_cache->ڳڻپ * (iclass + 1) * يادگيري_وسعت_سائيز;
  }
#جيڪڏهن_پورو

  وسعت_t *deferred = (وسعت_t *)atomic_ڀريو_ptr(&ڍير_2->وسعت_free_deferred);
  طالما (deferred) {
    إذا (deferred->سائيز_class != سائيز_CLASS_HUGE)
      stats->spancache += (size_t)deferred->وسعت_ڳڻپ * يادگيري_وسعت_سائيز;
    deferred = (وسعت_t *)deferred->free_list;
  }

#إذا ENABLE_STATISTICS
  stats->thread_منزل_global = (size_t)atomic_load64(&ڍير_2->thread_منزل_global);
  stats->global_منزل_thread = (size_t)atomic_load64(&ڍير_2->global_منزل_thread);

  لكل (size_t iclass = 0; iclass < LARGE_CLASS_ڳڻپ; ++iclass) {
    stats->وسعت_use[iclass].current =
        (size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].current);
    stats->وسعت_use[iclass].peak =
        (size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].مٿاهون);
    stats->وسعت_use[iclass].منزل_global =
        (size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_منزل_global);
    stats->وسعت_use[iclass].from_global =
        (size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_from_global);
    stats->وسعت_use[iclass].منزل_cache =
        (size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_منزل_cache);
    stats->وسعت_use[iclass].from_cache =
        (size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_from_cache);
    stats->وسعت_use[iclass].منزل_reserved =
        (size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_منزل_reserved);
    stats->وسعت_use[iclass].from_reserved =
        (size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_from_reserved);
    stats->وسعت_use[iclass].map_calls =
        (size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_map_calls);
  }
  لكل (size_t iclass = 0; iclass < سائيز_CLASS_ڳڻپ; ++iclass) {
    stats->سائيز_use[iclass].alloc_current =
        (size_t)atomic_load32(&ڍير_2->سائيز_class_use[iclass].alloc_current);
    stats->سائيز_use[iclass].alloc_peak =
        (size_t)ڍير_2->سائيز_class_use[iclass].alloc_peak;
    stats->سائيز_use[iclass].alloc_ڪل =
        (size_t)atomic_load32(&ڍير_2->سائيز_class_use[iclass].alloc_ڪل);
    stats->سائيز_use[iclass].free_ڪل =
        (size_t)atomic_load32(&ڍير_2->سائيز_class_use[iclass].free_ڪل);
    stats->سائيز_use[iclass].spans_منزل_cache =
        (size_t)atomic_load32(&ڍير_2->سائيز_class_use[iclass].spans_منزل_cache);
    stats->سائيز_use[iclass].spans_from_cache =
        (size_t)atomic_load32(&ڍير_2->سائيز_class_use[iclass].spans_from_cache);
    stats->سائيز_use[iclass].spans_from_reserved = (size_t)atomic_load32(
        &ڍير_2->سائيز_class_use[iclass].spans_from_reserved);
    stats->سائيز_use[iclass].map_calls =
        (size_t)atomic_load32(&ڍير_2->سائيز_class_use[iclass].spans_map_calls);
  }
#جيڪڏهن_پورو
}

فراغ rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, حجم(rpmalloc_global_statistics_t));
#إذا ENABLE_STATISTICS
  stats->mapped = (size_t)atomic_load32(&mapped_صفحا) * يادگيري_صفحو_سائيز;
  stats->mapped_peak = (size_t)mapped_صفحا_peak * يادگيري_صفحو_سائيز;
  stats->mapped_ڪل_2 =
      (size_t)atomic_load32(&mapped_ڪل) * يادگيري_صفحو_سائيز;
  stats->unmapped_ڪل_2 =
      (size_t)atomic_load32(&unmapped_ڪل) * يادگيري_صفحو_سائيز;
  stats->huge_alloc =
      (size_t)atomic_load32(&huge_صفحا_current) * يادگيري_صفحو_سائيز;
  stats->huge_alloc_peak = (size_t)huge_صفحا_peak * يادگيري_صفحو_سائيز;
#جيڪڏهن_پورو
#إذا ENABLE_GLOBAL_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_ڳڻپ; ++iclass) {
    global_cache_t *cache = &يادگيري_وسعت_cache[iclass];
    طالما (!atomic_cas32_acquire(&cache->تالو, 1, 0))
      _rpmalloc_spin();
    uint32_t ڳڻپ = cache->ڳڻپ;
#إذا ENABLE_UNLIMITED_CACHE
    وسعت_t *current_وسعت = cache->حد_کان_مٿي_2;
    طالما (current_وسعت) {
      ++ڳڻپ;
      current_وسعت = current_وسعت->ايندڙ;
    }
#جيڪڏهن_پورو
    atomic_store32_release(&cache->تالو, 0);
    stats->cached += ڳڻپ * (iclass + 1) * يادگيري_وسعت_سائيز;
  }
#جيڪڏهن_پورو
}

#إذا ENABLE_STATISTICS

ساكن فراغ يادگيري_ڍير_dump_statistics(ڍير_t *ڍير_2, فراغ *file) {
  fprintf(file, "Heap %d stats:\n", ڍير_2->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  لكل (size_t iclass = 0; iclass < سائيز_CLASS_ڳڻپ; ++iclass) {
    إذا (!atomic_load32(&ڍير_2->سائيز_class_use[iclass].alloc_ڪل))
      استمر;
    fprintf(
        file,
        "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu "
        "%9u\n",
        (uint32_t)iclass,
        atomic_load32(&ڍير_2->سائيز_class_use[iclass].alloc_current),
        ڍير_2->سائيز_class_use[iclass].alloc_peak,
        atomic_load32(&ڍير_2->سائيز_class_use[iclass].alloc_ڪل),
        atomic_load32(&ڍير_2->سائيز_class_use[iclass].free_ڪل),
        يادگيري_سائيز_class[iclass].بلاڪ_سائيز_2,
        يادگيري_سائيز_class[iclass].بلاڪ_ڳڻپ,
        atomic_load32(&ڍير_2->سائيز_class_use[iclass].spans_current),
        ڍير_2->سائيز_class_use[iclass].spans_peak,
        ((size_t)ڍير_2->سائيز_class_use[iclass].alloc_peak *
         (size_t)يادگيري_سائيز_class[iclass].بلاڪ_سائيز_2) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڍير_2->سائيز_class_use[iclass].spans_منزل_cache) *
         يادگيري_وسعت_سائيز) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڍير_2->سائيز_class_use[iclass].spans_from_cache) *
         يادگيري_وسعت_سائيز) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(
             &ڍير_2->سائيز_class_use[iclass].spans_from_reserved) *
         يادگيري_وسعت_سائيز) /
            (size_t)(1024 * 1024),
        atomic_load32(&ڍير_2->سائيز_class_use[iclass].spans_map_calls));
  }
  fprintf(file, "Spans  Current     Peak Deferred  PeakMiB  Cached  ToCacheMiB "
                "FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB "
                "FromGlobalMiB  MmapCalls\n");
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_ڳڻپ; ++iclass) {
    إذا (!atomic_load32(&ڍير_2->وسعت_use[iclass].مٿاهون) &&
        !atomic_load32(&ڍير_2->وسعت_use[iclass].spans_map_calls))
      استمر;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&ڍير_2->وسعت_use[iclass].current),
        atomic_load32(&ڍير_2->وسعت_use[iclass].مٿاهون),
        atomic_load32(&ڍير_2->وسعت_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].مٿاهون) *
         (size_t)يادگيري_وسعت_سائيز * (iclass + 1)) /
            (size_t)(1024 * 1024),
#إذا ENABLE_THREAD_CACHE
        (بنانشان صحيح)(!iclass ? ڍير_2->وسعت_cache.ڳڻپ
                               : ڍير_2->وسعت_large_cache[iclass - 1].ڳڻپ),
        ((size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_منزل_cache) *
         (iclass + 1) * يادگيري_وسعت_سائيز) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_from_cache) *
         (iclass + 1) * يادگيري_وسعت_سائيز) /
            (size_t)(1024 * 1024),
#وإلا
        0, (size_t)0, (size_t)0,
#جيڪڏهن_پورو
        ((size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_منزل_reserved) *
         (iclass + 1) * يادگيري_وسعت_سائيز) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_from_reserved) *
         (iclass + 1) * يادگيري_وسعت_سائيز) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_منزل_global) *
         (size_t)يادگيري_وسعت_سائيز * (iclass + 1)) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڍير_2->وسعت_use[iclass].spans_from_global) *
         (size_t)يادگيري_وسعت_سائيز * (iclass + 1)) /
            (size_t)(1024 * 1024),
        atomic_load32(&ڍير_2->وسعت_use[iclass].spans_map_calls));
  }
  fprintf(file, "Full spans: %zu\n", ڍير_2->ڀريل_وسعت_ڳڻپ);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&ڍير_2->thread_منزل_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&ڍير_2->global_منزل_thread) / (size_t)(1024 * 1024));
}

#جيڪڏهن_پورو

فراغ rpmalloc_dump_statistics(فراغ *file) {
#إذا ENABLE_STATISTICS
  لكل (size_t list_idx = 0; list_idx < ڍير_ARRAY_سائيز; ++list_idx) {
    ڍير_t *ڍير_2 = يادگيري_heaps[list_idx];
    طالما (ڍير_2) {
      صحيح need_dump = 0;
      لكل (size_t iclass = 0; !need_dump && (iclass < سائيز_CLASS_ڳڻپ);
           ++iclass) {
        إذا (!atomic_load32(&ڍير_2->سائيز_class_use[iclass].alloc_ڪل)) {
          rpmalloc_assert(
              !atomic_load32(&ڍير_2->سائيز_class_use[iclass].free_ڪل),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&ڍير_2->سائيز_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          استمر;
        }
        need_dump = 1;
      }
      لكل (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_ڳڻپ);
           ++iclass) {
        إذا (!atomic_load32(&ڍير_2->وسعت_use[iclass].مٿاهون) &&
            !atomic_load32(&ڍير_2->وسعت_use[iclass].spans_map_calls))
          استمر;
        need_dump = 1;
      }
      إذا (need_dump)
        يادگيري_ڍير_dump_statistics(ڍير_2, file);
      ڍير_2 = ڍير_2->ايندڙ_ڍير;
    }
  }
  fprintf(file, "Global stats:\n");
  size_t huge_current =
      (size_t)atomic_load32(&huge_صفحا_current) * يادگيري_صفحو_سائيز;
  size_t huge_peak = (size_t)huge_صفحا_peak * يادگيري_صفحو_سائيز;
  fprintf(file, "HugeCurrentMiB HugePeakMiB\n");
  fprintf(file, "%14zu %11zu\n", huge_current / (size_t)(1024 * 1024),
          huge_peak / (size_t)(1024 * 1024));

#إذا ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_ڳڻپ; ++iclass) {
    global_cache_t *cache = يادگيري_وسعت_cache + iclass;
    size_t global_cache = (size_t)cache->ڳڻپ * iclass * يادگيري_وسعت_سائيز;

    size_t global_حد_کان_مٿي_cache = 0;
    وسعت_t *وسعت = cache->حد_کان_مٿي_2;
    طالما (وسعت) {
      global_حد_کان_مٿي_cache += iclass * يادگيري_وسعت_سائيز;
      وسعت = وسعت->ايندڙ;
    }
    إذا (global_cache || global_حد_کان_مٿي_cache || cache->وجهو_ڳڻپ ||
        cache->ڪڍو_ڳڻپ_2)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_حد_کان_مٿي_cache / (size_t)(1024 * 1024),
              cache->وجهو_ڳڻپ, cache->ڪڍو_ڳڻپ_2);
  }
#جيڪڏهن_پورو

  size_t mapped = (size_t)atomic_load32(&mapped_صفحا) * يادگيري_صفحو_سائيز;
  size_t mapped_os =
      (size_t)atomic_load32(&mapped_صفحا_os) * يادگيري_صفحو_سائيز;
  size_t mapped_peak = (size_t)mapped_صفحا_peak * يادگيري_صفحو_سائيز;
  size_t mapped_ڪل_2 =
      (size_t)atomic_load32(&mapped_ڪل) * يادگيري_صفحو_سائيز;
  size_t unmapped_ڪل_2 =
      (size_t)atomic_load32(&unmapped_ڪل) * يادگيري_صفحو_سائيز;
  fprintf(
      file,
      "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB\n");
  fprintf(file, "%9zu %11zu %13zu %14zu %16zu\n",
          mapped / (size_t)(1024 * 1024), mapped_os / (size_t)(1024 * 1024),
          mapped_peak / (size_t)(1024 * 1024),
          mapped_ڪل_2 / (size_t)(1024 * 1024),
          unmapped_ڪل_2 / (size_t)(1024 * 1024));

  fprintf(file, "\n");
#إذا 0
	int64_t allocated = atomic_load64(&ورهاست_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#جيڪڏهن_پورو
#جيڪڏهن_پورو
  (فراغ)حجم(file);
}

#إذا RPMALLOC_پهريون_CLASS_HEAPS

خارجي مضمن rpmalloc_ڍير_t *rpmalloc_ڍير_acquire(فراغ) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  ڍير_t *ڍير_2 = rpmalloc_ڍير_allocate(1);
  rpmalloc_assume(ڍير_2 != عدم);
  ڍير_2->owner_thread = 0;
  _rpmalloc_stat_inc(&يادگيري_سرگرم_heaps);
  أرجع ڍير_2;
}

خارجي مضمن فراغ rpmalloc_ڍير_release_2(rpmalloc_ڍير_t *ڍير_2) {
  إذا (ڍير_2)
    rpmalloc_ڍير_release(ڍير_2, 1, 1);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ڍير_alloc(rpmalloc_ڍير_t *ڍير_2, size_t سائيز_2) {
#إذا ENABLE_VALIDATE_دليلون
  إذا (سائيز_2 >= MAX_ALLOC_سائيز) {
    errno = EINVAL;
    أرجع 0;
  }
#جيڪڏهن_پورو
  أرجع _rpmalloc_allocate(ڍير_2, سائيز_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ڍير_aligned_alloc(rpmalloc_ڍير_t *ڍير_2, size_t alignment,
                            size_t سائيز_2) {
#إذا ENABLE_VALIDATE_دليلون
  إذا (سائيز_2 >= MAX_ALLOC_سائيز) {
    errno = EINVAL;
    أرجع 0;
  }
#جيڪڏهن_پورو
  أرجع _rpmalloc_aligned_allocate(ڍير_2, alignment, سائيز_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ڍير_calloc(rpmalloc_ڍير_t *ڍير_2, size_t انگ_2, size_t سائيز_2) {
  أرجع rpmalloc_ڍير_aligned_calloc(ڍير_2, 0, انگ_2, سائيز_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ڍير_aligned_calloc(rpmalloc_ڍير_t *ڍير_2, size_t alignment,
                             size_t انگ_2, size_t سائيز_2) {
  size_t ڪل_2;
#إذا ENABLE_VALIDATE_دليلون
#إذا PLATFORM_WINDOWS
  صحيح err = سائيز_TMult(انگ_2, سائيز_2, &ڪل_2);
  إذا ((err != S_OK) || (ڪل_2 >= MAX_ALLOC_سائيز)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(انگ_2, سائيز_2, &ڪل_2);
  إذا (err || (ڪل_2 >= MAX_ALLOC_سائيز)) {
    errno = EINVAL;
    أرجع 0;
  }
#جيڪڏهن_پورو
#وإلا
  ڪل_2 = انگ_2 * سائيز_2;
#جيڪڏهن_پورو
  فراغ *بلاڪ = _rpmalloc_aligned_allocate(ڍير_2, alignment, ڪل_2);
  إذا (بلاڪ)
    memset(بلاڪ, 0, ڪل_2);
  أرجع بلاڪ;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ڍير_realloc(rpmalloc_ڍير_t *ڍير_2, فراغ *ptr, size_t سائيز_2,
                      بنانشان صحيح flags) {
#إذا ENABLE_VALIDATE_دليلون
  إذا (سائيز_2 >= MAX_ALLOC_سائيز) {
    errno = EINVAL;
    أرجع ptr;
  }
#جيڪڏهن_پورو
  أرجع _rpmalloc_reallocate(ڍير_2, ptr, سائيز_2, 0, flags);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ڍير_aligned_realloc(rpmalloc_ڍير_t *ڍير_2, فراغ *ptr,
                              size_t alignment, size_t سائيز_2,
                              بنانشان صحيح flags) {
#إذا ENABLE_VALIDATE_دليلون
  إذا ((سائيز_2 + alignment < سائيز_2) || (alignment > يادگيري_صفحو_سائيز)) {
    errno = EINVAL;
    أرجع 0;
  }
#جيڪڏهن_پورو
  أرجع _rpmalloc_aligned_reallocate(ڍير_2, ptr, alignment, سائيز_2, 0, flags);
}

خارجي مضمن فراغ rpmalloc_ڍير_free(rpmalloc_ڍير_t *ڍير_2, فراغ *ptr) {
  (فراغ)حجم(ڍير_2);
  _rpmalloc_deallocate(ptr);
}

خارجي مضمن فراغ rpmalloc_ڍير_free_all(rpmalloc_ڍير_t *ڍير_2) {
  وسعت_t *وسعت;
  وسعت_t *ايندڙ_وسعت;

  rpmalloc_ڍير_cache_adopt_deferred(ڍير_2, 0);

  لكل (size_t iclass = 0; iclass < سائيز_CLASS_ڳڻپ; ++iclass) {
    وسعت = ڍير_2->سائيز_class[iclass].partial_وسعت;
    طالما (وسعت) {
      ايندڙ_وسعت = وسعت->ايندڙ;
      rpmalloc_ڍير_cache_وجهو(ڍير_2, وسعت);
      وسعت = ايندڙ_وسعت;
    }
    ڍير_2->سائيز_class[iclass].partial_وسعت = 0;
    وسعت = ڍير_2->ڀريل_وسعت[iclass];
    طالما (وسعت) {
      ايندڙ_وسعت = وسعت->ايندڙ;
      rpmalloc_ڍير_cache_وجهو(ڍير_2, وسعت);
      وسعت = ايندڙ_وسعت;
    }

    وسعت = ڍير_2->سائيز_class[iclass].cache;
    إذا (وسعت)
      rpmalloc_ڍير_cache_وجهو(ڍير_2, وسعت);
    ڍير_2->سائيز_class[iclass].cache = 0;
  }
  memset(ڍير_2->سائيز_class, 0, حجم(ڍير_2->سائيز_class));
  memset(ڍير_2->ڀريل_وسعت, 0, حجم(ڍير_2->ڀريل_وسعت));

  وسعت = ڍير_2->large_huge_وسعت;
  طالما (وسعت) {
    ايندڙ_وسعت = وسعت->ايندڙ;
    إذا (UNEXPECTED(وسعت->سائيز_class == سائيز_CLASS_HUGE))
      _rpmalloc_deallocate_huge(وسعت);
    وإلا
      rpmalloc_ڍير_cache_وجهو(ڍير_2, وسعت);
    وسعت = ايندڙ_وسعت;
  }
  ڍير_2->large_huge_وسعت = 0;
  ڍير_2->ڀريل_وسعت_ڳڻپ = 0;

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_ڳڻپ; ++iclass) {
    وسعت_cache_t *وسعت_cache;
    إذا (!iclass)
      وسعت_cache = &ڍير_2->وسعت_cache;
    وإلا
      وسعت_cache = (وسعت_cache_t *)(ڍير_2->وسعت_large_cache + (iclass - 1));
    إذا (!وسعت_cache->ڳڻپ)
      استمر;
#إذا ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&ڍير_2->thread_منزل_global,
                         وسعت_cache->ڳڻپ * (iclass + 1) * يادگيري_وسعت_سائيز);
    rpmalloc_stat_جوڙيو(&ڍير_2->وسعت_use[iclass].spans_منزل_global,
                       وسعت_cache->ڳڻپ);
    rpmalloc_global_cache_وجهو_spans(وسعت_cache->وسعت, iclass + 1,
                                        وسعت_cache->ڳڻپ);
#وإلا
    لكل (size_t ispan = 0; ispan < وسعت_cache->ڳڻپ; ++ispan)
      rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
#جيڪڏهن_پورو
    وسعت_cache->ڳڻپ = 0;
  }
#جيڪڏهن_پورو

#إذا ENABLE_STATISTICS
  لكل (size_t iclass = 0; iclass < سائيز_CLASS_ڳڻپ; ++iclass) {
    atomic_store32(&ڍير_2->سائيز_class_use[iclass].alloc_current, 0);
    atomic_store32(&ڍير_2->سائيز_class_use[iclass].spans_current, 0);
  }
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_ڳڻپ; ++iclass) {
    atomic_store32(&ڍير_2->وسعت_use[iclass].current, 0);
  }
#جيڪڏهن_پورو
}

خارجي مضمن فراغ rpmalloc_ڍير_thread_سيٽ_current(rpmalloc_ڍير_t *ڍير_2) {
  ڍير_t *prev_ڍير = get_thread_ڍير_raw();
  إذا (prev_ڍير != ڍير_2) {
    سيٽ_thread_ڍير(ڍير_2);
    إذا (prev_ڍير)
      rpmalloc_ڍير_release_2(prev_ڍير);
  }
}

خارجي مضمن rpmalloc_ڍير_t *rpmalloc_get_ڍير_for_ptr(فراغ *ptr) {
  // Grab the span, and then the heap from the span
  وسعت_t *وسعت = (وسعت_t *)((uintptr_t)ptr & يادگيري_وسعت_نقاب);
  إذا (وسعت) {
    أرجع وسعت->ڍير_2;
  }
  أرجع 0;
}

#جيڪڏهن_پورو

#إذا ENABLE_PRELOAD || ENABLE_OVERRIDE

#ضمن "malloc.c"

#جيڪڏهن_پورو

فراغ rpmalloc_linker_reference(فراغ) { (فراغ)حجم(_rpmalloc_initialized); }
