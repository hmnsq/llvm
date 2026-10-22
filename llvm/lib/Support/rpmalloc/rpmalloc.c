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

#شامل "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#اگر defined(__clang__)
#ہدایت clang diagnostic ignored "-Wunused-macros"
#ہدایت clang diagnostic ignored "-Wunused-function"
#اگر __has_warning("-Wreserved-identifier")
#ہدایت clang diagnostic ignored "-Wreserved-identifier"
#اگر_ختم
#اگر __has_warning("-Wstatic-in-inline")
#ہدایت clang diagnostic ignored "-Wstatic-in-inline"
#اگر_ختم
#وگرنہ_اگر defined(__GNUC__)
#ہدایت GCC diagnostic ignored "-Wunused-macros"
#ہدایت GCC diagnostic ignored "-Wunused-function"
#اگر_ختم

#اگر !defined(__has_builtin)
#تعریف_کرو __has_builtin(b) 0
#اگر_ختم

#اگر defined(__GNUC__) || defined(__clang__)

#اگر __has_builtin(__builtin_memcpy_inline)
#تعریف_کرو _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#ورنہ
#تعریف_کرو _rpmalloc_memcpy_const(x, y, s)                                        \
  کرو {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    حافظہ_نقل(x, y, s);                                                           \
  } جب (0)
#اگر_ختم

#اگر __has_builtin(__builtin_memset_inline)
#تعریف_کرو _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#ورنہ
#تعریف_کرو _rpmalloc_memset_const(x, y, s)                                        \
  کرو {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } جب (0)
#اگر_ختم
#ورنہ
#تعریف_کرو _rpmalloc_memcpy_const(x, y, s) حافظہ_نقل(x, y, s)
#تعریف_کرو _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#اگر_ختم

#اگر __has_builtin(__builtin_assume)
#تعریف_کرو rpmalloc_assume(cond) __builtin_assume(cond)
#وگرنہ_اگر defined(__GNUC__)
#تعریف_کرو rpmalloc_assume(cond)                                                  \
  کرو {                                                                         \
    اگر (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } جب (0)
#وگرنہ_اگر defined(_MSC_VER)
#تعریف_کرو rpmalloc_assume(cond) __assume(cond)
#ورنہ
#تعریف_کرو rpmalloc_assume(cond) 0
#اگر_ختم

#اگر_غیر_تعریف_شدہ ڈھیر_ARRAY_سائز
//! Size of heap hashmap
#تعریف_کرو ڈھیر_ARRAY_سائز 47
#اگر_ختم
#اگر_غیر_تعریف_شدہ ENABLE_THREAD_CACHE
//! Enable per-thread cache
#تعریف_کرو ENABLE_THREAD_CACHE 1
#اگر_ختم
#اگر_غیر_تعریف_شدہ ENABLE_GLOBAL_CACHE
//! Enable global cache shared between all threads, requires thread cache
#تعریف_کرو ENABLE_GLOBAL_CACHE 1
#اگر_ختم
#اگر_غیر_تعریف_شدہ ENABLE_VALIDATE_دلائل
//! Enable validation of args to public entry points
#تعریف_کرو ENABLE_VALIDATE_دلائل 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ ENABLE_STATISTICS
//! Enable statistics collection
#تعریف_کرو ENABLE_STATISTICS 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ ENABLE_ASSERTS
//! Enable asserts
#تعریف_کرو ENABLE_ASSERTS 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ ENABLE_OVERRIDE
//! Override standard library malloc/free and new/delete entry points
#تعریف_کرو ENABLE_OVERRIDE 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ ENABLE_PRELOAD
//! Support preloading
#تعریف_کرو ENABLE_PRELOAD 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ DISABLE_UNMAP
//! Disable unmapping memory pages (also enables unlimited cache)
#تعریف_کرو DISABLE_UNMAP 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ ENABLE_UNLIMITED_CACHE
//! Enable unlimited global cache (no unmapping until finalization)
#تعریف_کرو ENABLE_UNLIMITED_CACHE 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ ENABLE_ADAPTIVE_THREAD_CACHE
//! Enable adaptive thread cache size based on use heuristics
#تعریف_کرو ENABLE_ADAPTIVE_THREAD_CACHE 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ DEFAULT_وسعت_MAP_گنتی
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#تعریف_کرو DEFAULT_وسعت_MAP_گنتی 64
#اگر_ختم
#اگر_غیر_تعریف_شدہ GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#تعریف_کرو GLOBAL_CACHE_MULTIPLIER 8
#اگر_ختم

#اگر DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#غلطی Must use global cache اگر unmap is disabled
#اگر_ختم

#اگر DISABLE_UNMAP
#تعریف_منسوخ ENABLE_UNLIMITED_CACHE
#تعریف_کرو ENABLE_UNLIMITED_CACHE 1
#اگر_ختم

#اگر !ENABLE_GLOBAL_CACHE
#تعریف_منسوخ ENABLE_UNLIMITED_CACHE
#تعریف_کرو ENABLE_UNLIMITED_CACHE 0
#اگر_ختم

#اگر !ENABLE_THREAD_CACHE
#تعریف_منسوخ ENABLE_ADAPTIVE_THREAD_CACHE
#تعریف_کرو ENABLE_ADAPTIVE_THREAD_CACHE 0
#اگر_ختم

#اگر defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#تعریف_کرو PLATFORM_WINDOWS 1
#تعریف_کرو PLATFORM_POSIX 0
#ورنہ
#تعریف_کرو PLATFORM_WINDOWS 0
#تعریف_کرو PLATFORM_POSIX 1
#اگر_ختم

/// Platform and arch specifics
#اگر defined(_MSC_VER) && !defined(__clang__)
#ہدایت warning(disable : 5105)
#اگر_غیر_تعریف_شدہ FORCEINLINE
#تعریف_کرو FORCEINLINE اندرونی __forceinline
#اگر_ختم
#تعریف_کرو _Static_assert static_assert
#ورنہ
#اگر_غیر_تعریف_شدہ FORCEINLINE
#تعریف_کرو FORCEINLINE اندرونی __attribute__((__always_inline__))
#اگر_ختم
#اگر_ختم
#اگر PLATFORM_WINDOWS
#اگر_غیر_تعریف_شدہ WIN32_LEAN_AND_اوسط
#تعریف_کرو WIN32_LEAN_AND_اوسط
#اگر_ختم
#شامل <windows.h>
#اگر ENABLE_VALIDATE_دلائل
#شامل <intsafe.h>
#اگر_ختم
#ورنہ
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <time.h>
#شامل <unistd.h>
#اگر defined(__linux__) || defined(__ANDROID__)
#شامل <sys/prctl.h>
#اگر !defined(PR_سیٹ_VMA)
#تعریف_کرو PR_سیٹ_VMA 0x53564d41
#تعریف_کرو PR_سیٹ_VMA_ANON_NAME 0
#اگر_ختم
#اگر_ختم
#اگر defined(__APPLE__)
#شامل <TargetConditionals.h>
#اگر !ہدف_OS_IPHONE && !ہدف_OS_SIMULATOR
#شامل <mach/mach_vm.h>
#شامل <mach/vm_statistics.h>
#اگر_ختم
#شامل <pthread.h>
#اگر_ختم
#اگر defined(__HAIKU__) || defined(__TINYC__)
#شامل <pthread.h>
#اگر_ختم
#اگر_ختم

#شامل <errno.h>
#شامل <stdint.h>
#شامل <string.h>

#اگر defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#شامل <fibersapi.h>
ساکن DWORD fls_کلید;
#اگر_ختم

#اگر PLATFORM_POSIX
#شامل <sched.h>
#شامل <sys/mman.h>
#اگر_تعریف_شدہ __FreeBSD__
#شامل <sys/sysctl.h>
#تعریف_کرو MAP_HUGETLB MAP_ALIGNED_SUPER
#اگر_غیر_تعریف_شدہ PROT_MAX
#تعریف_کرو PROT_MAX(f) 0
#اگر_ختم
#ورنہ
#تعریف_کرو PROT_MAX(f) 0
#اگر_ختم
#اگر_تعریف_شدہ __sun
خارجی صحیح madvise(caddr_t, size_t, صحیح);
#اگر_ختم
#اگر_غیر_تعریف_شدہ MAP_UNINITIALIZED
#تعریف_کرو MAP_UNINITIALIZED 0
#اگر_ختم
#اگر_ختم
#شامل <errno.h>

#اگر ENABLE_ASSERTS
#تعریف_منسوخ NDEBUG
#اگر defined(_MSC_VER) && !defined(_DEBUG)
#تعریف_کرو _DEBUG
#اگر_ختم
#شامل <assert.h>
#تعریف_کرو RPMALLOC_TOSTRING_M(x) #x
#تعریف_کرو RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#تعریف_کرو rpmalloc_assert(truth, پیغام_2)                                        \
  کرو {                                                                         \
    اگر (!(truth)) {                                                            \
      اگر (حافظہ_config.error_callback) {                                     \
        حافظہ_config.error_callback(پیغام_2 " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } ورنہ {                                                                 \
        أكد((truth) && پیغام_2);                                            \
      }                                                                        \
    }                                                                          \
  } جب (0)
#ورنہ
#تعریف_کرو rpmalloc_assert(truth, پیغام_2)                                        \
  کرو {                                                                         \
  } جب (0)
#اگر_ختم
#اگر ENABLE_STATISTICS
#شامل <stdio.h>
#اگر_ختم

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#اگر defined(_MSC_VER) && !defined(__clang__)

قسمتعریف متلون طویل atomic32_t;
قسمتعریف متلون طویل طویل atomic64_t;
قسمتعریف متلون خالی *atomicptr_t;

ساکن FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  واپس (int32_t)InterlockedOr(src, 0);
}
ساکن FORCEINLINE خالی atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ساکن FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  واپس (int32_t)InterlockedIncrement(val);
}
ساکن FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  واپس (int32_t)InterlockedDecrement(val);
}
ساکن FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t جوڑو) {
  واپس (int32_t)Interlocked_Exchange_جوڑو(val, جوڑو) + جوڑو;
}
ساکن FORCEINLINE صحیح atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  واپس (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
ساکن FORCEINLINE خالی atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ساکن FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  واپس (int64_t)InterlockedOr64(src, 0);
}
ساکن FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t جوڑو) {
  واپس (int64_t)InterlockedExchangeAdd64(val, جوڑو) + جوڑو;
}
ساکن FORCEINLINE خالی *atomic_بھرو_ptr(atomicptr_t *src) {
  واپس Interlocked_Compare_Exchange_اشارہ_گر(src, 0, 0);
}
ساکن FORCEINLINE خالی atomic_store_ptr(atomicptr_t *dst, خالی *val) {
  Interlocked_Exchange_اشارہ_گر(dst, val);
}
ساکن FORCEINLINE خالی atomic_store_ptr_release(atomicptr_t *dst, خالی *val) {
  Interlocked_Exchange_اشارہ_گر(dst, val);
}
ساکن FORCEINLINE خالی *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     خالی *val) {
  واپس (خالی *)Interlocked_Exchange_اشارہ_گر((خالی *متلون *)dst, val);
}
ساکن FORCEINLINE صحیح atomic_cas_ptr(atomicptr_t *dst, خالی *val, خالی *ref) {
  واپس (Interlocked_Compare_Exchange_اشارہ_گر((خالی *متلون *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#تعریف_کرو EXPECTED(x) (x)
#تعریف_کرو UNEXPECTED(x) (x)

#ورنہ

#شامل <stdatomic.h>

قسمتعریف متلون _Atomic(int32_t) atomic32_t;
قسمتعریف متلون _Atomic(int64_t) atomic64_t;
قسمتعریف متلون _Atomic(خالی *) atomicptr_t;

ساکن FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  واپس atomic_بھرو_explicit(src, memory_order_relaxed);
}
ساکن FORCEINLINE خالی atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ساکن FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  واپس atomic_fetch_جوڑو_explicit(val, 1, memory_order_relaxed) + 1;
}
ساکن FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  واپس atomic_fetch_جوڑو_explicit(val, -1, memory_order_relaxed) - 1;
}
ساکن FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t جوڑو) {
  واپس atomic_fetch_جوڑو_explicit(val, جوڑو, memory_order_relaxed) + جوڑو;
}
ساکن FORCEINLINE صحیح atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  واپس atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
ساکن FORCEINLINE خالی atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ساکن FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  واپس atomic_بھرو_explicit(val, memory_order_relaxed);
}
ساکن FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t جوڑو) {
  واپس atomic_fetch_جوڑو_explicit(val, جوڑو, memory_order_relaxed) + جوڑو;
}
ساکن FORCEINLINE خالی *atomic_بھرو_ptr(atomicptr_t *src) {
  واپس atomic_بھرو_explicit(src, memory_order_relaxed);
}
ساکن FORCEINLINE خالی atomic_store_ptr(atomicptr_t *dst, خالی *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ساکن FORCEINLINE خالی atomic_store_ptr_release(atomicptr_t *dst, خالی *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ساکن FORCEINLINE خالی *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     خالی *val) {
  واپس atomic_exchange_explicit(dst, val, memory_order_acquire);
}
ساکن FORCEINLINE صحیح atomic_cas_ptr(atomicptr_t *dst, خالی *val, خالی *ref) {
  واپس atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_relaxed, memory_order_relaxed);
}

#تعریف_کرو EXPECTED(x) __builtin_expect((x), 1)
#تعریف_کرو UNEXPECTED(x) __builtin_expect((x), 0)

#اگر_ختم

////////////
///
/// Statistics related functions (evaluate to nothing when statistics not
/// enabled)
///
//////

#اگر ENABLE_STATISTICS
#تعریف_کرو _rpmalloc_stat_inc(شمار_3) atomic_incr32(شمار_3)
#تعریف_کرو _rpmalloc_stat_dec(شمار_3) atomic_decr32(شمار_3)
#تعریف_کرو rpmalloc_stat_جوڑو(شمار_3, قدر_2)                                     \
  atomic_add32(شمار_3, (int32_t)(قدر_2))
#تعریف_کرو _rpmalloc_stat_add64(شمار_3, قدر_2)                                   \
  atomic_add64(شمار_3, (int64_t)(قدر_2))
#تعریف_کرو rpmalloc_stat_جوڑو_peak(شمار_3, قدر_2, peak)                          \
  کرو {                                                                         \
    int32_t cur_گنتی = atomic_add32(شمار_3, (int32_t)(قدر_2));              \
    اگر (cur_گنتی > (peak))                                                   \
      peak = cur_گنتی;                                                       \
  } جب (0)
#تعریف_کرو _rpmalloc_stat_sub(شمار_3, قدر_2)                                     \
  atomic_add32(شمار_3, -(int32_t)(قدر_2))
#تعریف_کرو _rpmalloc_stat_inc_alloc(ڈھیر_2, class_idx)                              \
  کرو {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&ڈھیر_2->سائز_class_use[class_idx].alloc_current);         \
    اگر (alloc_current > ڈھیر_2->سائز_class_use[class_idx].alloc_peak)            \
      ڈھیر_2->سائز_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&ڈھیر_2->سائز_class_use[class_idx].alloc_کل);               \
  } جب (0)
#تعریف_کرو _rpmalloc_stat_inc_free(ڈھیر_2, class_idx)                               \
  کرو {                                                                         \
    atomic_decr32(&ڈھیر_2->سائز_class_use[class_idx].alloc_current);             \
    atomic_incr32(&ڈھیر_2->سائز_class_use[class_idx].free_کل);                \
  } جب (0)
#ورنہ
#تعریف_کرو _rpmalloc_stat_inc(شمار_3)                                            \
  کرو {                                                                         \
  } جب (0)
#تعریف_کرو _rpmalloc_stat_dec(شمار_3)                                            \
  کرو {                                                                         \
  } جب (0)
#تعریف_کرو rpmalloc_stat_جوڑو(شمار_3, قدر_2)                                     \
  کرو {                                                                         \
  } جب (0)
#تعریف_کرو _rpmalloc_stat_add64(شمار_3, قدر_2)                                   \
  کرو {                                                                         \
  } جب (0)
#تعریف_کرو rpmalloc_stat_جوڑو_peak(شمار_3, قدر_2, peak)                          \
  کرو {                                                                         \
  } جب (0)
#تعریف_کرو _rpmalloc_stat_sub(شمار_3, قدر_2)                                     \
  کرو {                                                                         \
  } جب (0)
#تعریف_کرو _rpmalloc_stat_inc_alloc(ڈھیر_2, class_idx)                              \
  کرو {                                                                         \
  } جب (0)
#تعریف_کرو _rpmalloc_stat_inc_free(ڈھیر_2, class_idx)                               \
  کرو {                                                                         \
  } جب (0)
#اگر_ختم

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#تعریف_کرو SMALL_GRANULARITY 16
//! Small granularity shift count
#تعریف_کرو SMALL_GRANULARITY_سرکاؤ 4
//! Number of small block size classes
#تعریف_کرو SMALL_CLASS_گنتی 65
//! Maximum size of a small block
#تعریف_کرو SMALL_سائز_حد_بندی (SMALL_GRANULARITY * (SMALL_CLASS_گنتی - 1))
//! Granularity of a medium allocation block
#تعریف_کرو MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#تعریف_کرو MEDIUM_GRANULARITY_سرکاؤ 9
//! Number of medium block size classes
#تعریف_کرو MEDIUM_CLASS_گنتی 61
//! Total number of small + medium size classes
#تعریف_کرو سائز_CLASS_گنتی (SMALL_CLASS_گنتی + MEDIUM_CLASS_گنتی)
//! Number of large block size classes
#تعریف_کرو LARGE_CLASS_گنتی 63
//! Maximum size of a medium block
#تعریف_کرو MEDIUM_سائز_حد_بندی                                                      \
  (SMALL_سائز_حد_بندی + (MEDIUM_GRANULARITY * MEDIUM_CLASS_گنتی))
//! Maximum size of a large block
#تعریف_کرو LARGE_سائز_حد_بندی                                                       \
  ((LARGE_CLASS_گنتی * حافظہ_وسعت_سائز) - وسعت_سرخی_سائز)
//! Size of a span header (must be a multiple of SMALL_GRANULARITY and a power
//! of two)
#تعریف_کرو وسعت_سرخی_سائز 128
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
_Static_assert((وسعت_سرخی_سائز & (وسعت_سرخی_سائز - 1)) == 0,
               "Span header size must be power of two");

#اگر ENABLE_VALIDATE_دلائل
//! Maximum allocation size to avoid integer overflow
#تعریف_منسوخ MAX_ALLOC_سائز
#تعریف_کرو MAX_ALLOC_سائز (((size_t) - 1) - حافظہ_وسعت_سائز)
#اگر_ختم

#تعریف_کرو اشارہ_گر_انحراف(ptr, ofs) (خالی *)((حرف *)(ptr) + (ptrdiff_t)(ofs))
#تعریف_کرو اشارہ_گر_diff(پہلا_2, دوسرا_2)                                            \
  (ptrdiff_t)((ثابت حرف *)(پہلا_2) - (ثابت حرف *)(دوسرا_2))

#تعریف_کرو INVALID_اشارہ_گر ((خالی *)((uintptr_t) - 1))

#تعریف_کرو سائز_CLASS_LARGE سائز_CLASS_گنتی
#تعریف_کرو سائز_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
قسمتعریف ساخت ڈھیر_t ڈھیر_t;
//! Span of memory pages
قسمتعریف ساخت وسعت_t وسعت_t;
//! Span list
قسمتعریف ساخت وسعت_list_t وسعت_list_t;
//! Span active data
قسمتعریف ساخت وسعت_فعال_t وسعت_فعال_t;
//! Size class definition
قسمتعریف ساخت سائز_class_t سائز_class_t;
//! Global cache
قسمتعریف ساخت global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#تعریف_کرو وسعت_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#تعریف_کرو وسعت_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#تعریف_کرو وسعت_FLAG_ALIGNED_بلاکیں 4U
//! Flag indicating an unmapped master span
#تعریف_کرو وسعت_FLAG_UNMAPPED_MASTER 8U

#اگر ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
ساخت وسعت_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t بلند;
#اگر ENABLE_STATISTICS
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
#اگر_ختم
};
قسمتعریف ساخت وسعت_use_t وسعت_use_t;
#اگر_ختم

#اگر ENABLE_STATISTICS
ساخت سائز_class_use_t {
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
  atomic32_t spans_مقصد_cache;
  //! Number of spans transitioned from cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of spans mapped
  atomic32_t spans_map_calls;
  int32_t غیر_مستعمل_2;
};
قسمتعریف ساخت سائز_class_use_t سائز_class_use_t;
#اگر_ختم

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
ساخت وسعت_t {
  //! Free list
  خالی *free_list;
  //! Total block count of size class
  uint32_t بلاک_گنتی;
  //! Size class
  uint32_t سائز_class;
  //! Index of last block initialized in free list
  uint32_t free_list_حد_بندی;
  //! Number of used blocks remaining when in partial state
  uint32_t مستعمل_گنتی;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_سائز;
  //! Size of a block
  uint32_t بلاک_سائز_2;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t وسعت_گنتی;
  //! Total span counter for master spans
  uint32_t کل_spans;
  //! Offset from master span for subspans
  uint32_t انحراف_from_master;
  //! Remaining span counter, for master spans
  atomic32_t بقایا_spans;
  //! Alignment offset
  uint32_t align_انحراف;
  //! Owning heap
  ڈھیر_t *ڈھیر_2;
  //! Next span
  وسعت_t *اگلا;
  //! Previous span
  وسعت_t *پچھلا;
};
_Static_assert(حجم(وسعت_t) <= وسعت_سرخی_سائز, "span size mismatch");

ساخت وسعت_cache_t {
  size_t گنتی;
  وسعت_t *وسعت[MAX_THREAD_وسعت_CACHE];
};
قسمتعریف ساخت وسعت_cache_t وسعت_cache_t;

ساخت وسعت_large_cache_t {
  size_t گنتی;
  وسعت_t *وسعت[MAX_THREAD_وسعت_LARGE_CACHE];
};
قسمتعریف ساخت وسعت_large_cache_t وسعت_large_cache_t;

ساخت ڈھیر_سائز_class_t {
  //! Free list of active span
  خالی *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  وسعت_t *partial_وسعت;
  //! Early level cache of fully free spans
  وسعت_t *cache;
};
قسمتعریف ساخت ڈھیر_سائز_class_t ڈھیر_سائز_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
ساخت ڈھیر_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  ڈھیر_سائز_class_t سائز_class[سائز_CLASS_گنتی];
#اگر ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  وسعت_cache_t وسعت_cache;
#اگر_ختم
  //! List of deferred free spans (single linked list)
  atomicptr_t وسعت_free_deferred;
  //! Number of full spans
  size_t بھرا_وسعت_گنتی;
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
  صحیح finalize;
  //! Master heap owning the memory pages
  ڈھیر_t *master_ڈھیر;
#اگر ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  وسعت_large_cache_t وسعت_large_cache[LARGE_CLASS_گنتی - 1];
#اگر_ختم
#اگر RPMALLOC_پہلا_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  وسعت_t *بھرا_وسعت[سائز_CLASS_گنتی];
  //! Double linked list of large and huge spans allocated by this heap
  وسعت_t *large_huge_وسعت;
#اگر_ختم
#اگر ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  وسعت_use_t وسعت_use[LARGE_CLASS_گنتی];
#اگر_ختم
#اگر ENABLE_STATISTICS
  //! Allocation stats per size class
  سائز_class_use_t سائز_class_use[سائز_CLASS_گنتی + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_مقصد_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_مقصد_thread;
#اگر_ختم
};

// Size class for defining a block size bucket
ساخت سائز_class_t {
  //! Size of blocks in this class
  uint32_t بلاک_سائز_2;
  //! Number of blocks in each chunk
  uint16_t بلاک_گنتی;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(حجم(سائز_class_t) == 8, "Size class size mismatch");

ساخت global_cache_t {
  //! Cache lock
  atomic32_t تالا_2;
  //! Cache count
  uint32_t گنتی;
#اگر ENABLE_STATISTICS
  //! Insert count
  size_t داخل_کرو_گنتی;
  //! Extract count
  size_t نکالو_گنتی_2;
#اگر_ختم
  //! Cached spans
  وسعت_t *وسعت[GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_وسعت_CACHE];
  //! Unlimited cache overflow
  وسعت_t *حد_سے_تجاوز_2;
};

////////////
///
/// Global data
///
//////

//! Default span size (64KiB)
#تعریف_کرو حافظہ_default_وسعت_سائز (64 * 1024)
#تعریف_کرو حافظہ_default_وسعت_سائز_سرکاؤ 16
#تعریف_کرو حافظہ_default_وسعت_نقاب (~((uintptr_t)(حافظہ_وسعت_سائز - 1)))

//! Initialized flag
ساکن صحیح _rpmalloc_initialized;
//! Main thread ID
ساکن uintptr_t _rpmalloc_main_thread_id;
//! Configuration
ساکن rpmalloc_config_t حافظہ_config;
//! Memory page size
ساکن size_t حافظہ_صفحہ_سائز;
//! Shift to divide by page size
ساکن size_t حافظہ_صفحہ_سائز_سرکاؤ;
//! Granularity at which memory pages are mapped by OS
ساکن size_t حافظہ_map_granularity;
#اگر RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
ساکن size_t حافظہ_وسعت_سائز;
//! Shift to divide by span size
ساکن size_t حافظہ_وسعت_سائز_سرکاؤ;
//! Mask to get to start of a memory span
ساکن uintptr_t حافظہ_وسعت_نقاب;
#ورنہ
//! Hardwired span size
#تعریف_کرو حافظہ_وسعت_سائز حافظہ_default_وسعت_سائز
#تعریف_کرو حافظہ_وسعت_سائز_سرکاؤ حافظہ_default_وسعت_سائز_سرکاؤ
#تعریف_کرو حافظہ_وسعت_نقاب حافظہ_default_وسعت_نقاب
#اگر_ختم
//! Number of spans to map in each map call
ساکن size_t حافظہ_وسعت_map_گنتی;
//! Number of spans to keep reserved in each heap
ساکن size_t حافظہ_ڈھیر_reserve_گنتی;
//! Global size classes
ساکن سائز_class_t حافظہ_سائز_class[سائز_CLASS_گنتی];
//! Run-time size limit of medium blocks
ساکن size_t حافظہ_medium_سائز_حد_بندی;
//! Heap ID counter
ساکن atomic32_t حافظہ_ڈھیر_id;
//! Huge page support
ساکن صحیح حافظہ_huge_صفحات;
#اگر ENABLE_GLOBAL_CACHE
//! Global span cache
ساکن global_cache_t حافظہ_وسعت_cache[LARGE_CLASS_گنتی];
#اگر_ختم
//! Global reserved spans
ساکن وسعت_t *حافظہ_global_reserve;
//! Global reserved count
ساکن size_t حافظہ_global_reserve_گنتی;
//! Global reserved master
ساکن وسعت_t *حافظہ_global_reserve_master;
//! All heaps
ساکن ڈھیر_t *حافظہ_heaps[ڈھیر_ARRAY_سائز];
//! Used to restrict access to mapping memory for huge pages
ساکن atomic32_t حافظہ_global_تالا;
//! Orphaned heaps
ساکن ڈھیر_t *حافظہ_orphan_heaps;
#اگر RPMALLOC_پہلا_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
ساکن ڈھیر_t *حافظہ_پہلا_class_orphan_heaps;
#اگر_ختم
#اگر ENABLE_STATISTICS
//! Allocations counter
ساکن atomic64_t تخصیص_شمار;
//! Deallocations counter
ساکن atomic64_t deallocation_شمار;
//! Active heap count
ساکن atomic32_t حافظہ_فعال_heaps;
//! Number of currently mapped memory pages
ساکن atomic32_t mapped_صفحات;
//! Peak number of concurrently mapped memory pages
ساکن int32_t mapped_صفحات_peak;
//! Number of mapped master spans
ساکن atomic32_t _master_spans;
//! Number of unmapped dangling master spans
ساکن atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
ساکن atomic32_t mapped_کل;
//! Running counter of total number of unmapped memory pages since start
ساکن atomic32_t unmapped_کل;
//! Number of currently mapped memory pages in OS calls
ساکن atomic32_t mapped_صفحات_os;
//! Number of currently allocated pages in huge allocations
ساکن atomic32_t huge_صفحات_current;
//! Peak number of currently allocated pages in huge allocations
ساکن int32_t huge_صفحات_peak;
#اگر_ختم

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#اگر ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
ساکن pthread_key_t حافظہ_thread_ڈھیر;
#ورنہ
#اگر_تعریف_شدہ _MSC_VER
#تعریف_کرو _Thread_local __declspec(thread)
#تعریف_کرو TLS_MODEL
#ورنہ
#اگر_غیر_تعریف_شدہ __HAIKU__
#تعریف_کرو TLS_MODEL __attribute__((tls_model("initial-exec")))
#ورنہ
#تعریف_کرو TLS_MODEL
#اگر_ختم
#اگر !defined(__clang__) && defined(__GNUC__)
#تعریف_کرو _Thread_local __thread
#اگر_ختم
#اگر_ختم
ساکن _Thread_local ڈھیر_t *حافظہ_thread_ڈھیر TLS_MODEL;
#اگر_ختم

ساکن اندرونی ڈھیر_t *get_thread_ڈھیر_raw(خالی) {
#اگر (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  واپس pthread_getspecific(حافظہ_thread_ڈھیر);
#ورنہ
  واپس حافظہ_thread_ڈھیر;
#اگر_ختم
}

//! Get the current thread heap
ساکن اندرونی ڈھیر_t *get_thread_ڈھیر(خالی) {
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر_raw();
#اگر ENABLE_PRELOAD
  اگر (EXPECTED(ڈھیر_2 != 0))
    واپس ڈھیر_2;
  rpmalloc_initialize();
  واپس get_thread_ڈھیر_raw();
#ورنہ
  واپس ڈھیر_2;
#اگر_ختم
}

//! Fast thread ID
ساکن اندرونی uintptr_t get_thread_id(خالی) {
#اگر defined(_WIN32)
  واپس (uintptr_t)((خالی *)NtCurrentTeb());
#وگرنہ_اگر (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#اگر defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#وگرنہ_اگر defined(__x86_64__)
#اگر defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#ورنہ
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#اگر_ختم
#وگرنہ_اگر defined(__arm__)
  __asm__ متلون("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#وگرنہ_اگر defined(__aarch64__)
#اگر defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ متلون("mrs %0, tpidrro_el0" : "=r"(tid));
#ورنہ
  __asm__ متلون("mrs %0, tpidr_el0" : "=r"(tid));
#اگر_ختم
#ورنہ
#غلطی This platform needs implementation کا_3 get_thread_id()
#اگر_ختم
  واپس tid;
#ورنہ
#غلطی This platform needs implementation کا_3 get_thread_id()
#اگر_ختم
}

//! Set the current thread heap
ساکن خالی سیٹ_thread_ڈھیر(ڈھیر_t *ڈھیر_2) {
#اگر ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(حافظہ_thread_ڈھیر, ڈھیر_2);
#ورنہ
  حافظہ_thread_ڈھیر = ڈھیر_2;
#اگر_ختم
  اگر (ڈھیر_2)
    ڈھیر_2->owner_thread = get_thread_id();
}

//! Set main thread ID
خارجی خالی rpmalloc_سیٹ_main_thread(خالی);

خالی rpmalloc_سیٹ_main_thread(خالی) {
  _rpmalloc_main_thread_id = get_thread_id();
}

ساکن خالی _rpmalloc_spin(خالی) {
#اگر defined(_MSC_VER)
#اگر defined(_M_ARM64)
  __yield();
#ورنہ
  _mm_pause();
#اگر_ختم
#وگرنہ_اگر defined(__x86_64__) || defined(__i386__)
  __asm__ متلون("pause" ::: "memory");
#وگرنہ_اگر defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ متلون("yield" ::: "memory");
#وگرنہ_اگر defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ متلون("or 27,27,27");
#وگرنہ_اگر defined(__sparc__)
  __asm__ متلون("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#ورنہ
  ساخت timespec ts = {0};
  nanosleep(&ts, 0);
#اگر_ختم
}

#اگر defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

ساکن خالی NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  انتخاب (reason) {
  حالت DLL_PROCESS_ATTACH:
    توڑو;
  حالت DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    توڑو;
  حالت DLL_THREAD_ATTACH:
    توڑو;
  حالت DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    توڑو;
  }
}

#اگر_تعریف_شدہ _WIN64
#ہدایت comment(linker, "/INCLUDE:_tls_used")
#ہدایت comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#ہدایت const_seg(".CRT$XLY")

خارجی ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#ہدایت const_seg()
#ورنہ // _WIN64
#ہدایت comment(linker, "/INCLUDE:__tls_used")
#ہدایت comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#ہدایت ڈیٹا_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#ہدایت ڈیٹا_seg()
#اگر_ختم // _WIN64

ساکن خالی NTAPI _rpmalloc_thread_destructor(خالی *قدر_2) {
#اگر ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  اگر (get_thread_id() == _rpmalloc_main_thread_id)
    واپس;
#اگر_ختم
  اگر (قدر_2)
    rpmalloc_thread_finalize(1);
}
#اگر_ختم

////////////
///
/// Low level memory map/unmap
///
//////

ساکن خالی rpmalloc_سیٹ_name(خالی *پتہ_2, size_t سائز_2) {
#اگر defined(__linux__) || defined(__ANDROID__)
  ثابت حرف *name = حافظہ_huge_صفحات ? حافظہ_config.huge_صفحہ_name
                                        : حافظہ_config.صفحہ_name;
  اگر (پتہ_2 == MAP_FAILED || !name)
    واپس;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (خالی)prctl(PR_سیٹ_VMA, PR_سیٹ_VMA_ANON_NAME, (uintptr_t)پتہ_2, سائز_2,
              (uintptr_t)name);
#ورنہ
  (خالی)حجم(سائز_2);
  (خالی)حجم(پتہ_2);
#اگر_ختم
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
ساکن خالی *_rpmalloc_mmap(size_t سائز_2, size_t *انحراف_2) {
  rpmalloc_assert(!(سائز_2 % حافظہ_صفحہ_سائز), "Invalid mmap size");
  rpmalloc_assert(سائز_2 >= حافظہ_صفحہ_سائز, "Invalid mmap size");
  خالی *پتہ_2 = حافظہ_config.حافظہ_map(سائز_2, انحراف_2);
  اگر (EXPECTED(پتہ_2 != 0)) {
    rpmalloc_stat_جوڑو_peak(&mapped_صفحات, (سائز_2 >> حافظہ_صفحہ_سائز_سرکاؤ),
                            mapped_صفحات_peak);
    rpmalloc_stat_جوڑو(&mapped_کل, (سائز_2 >> حافظہ_صفحہ_سائز_سرکاؤ));
  }
  واپس پتہ_2;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
ساکن خالی _rpmalloc_unmap(خالی *پتہ_2, size_t سائز_2, size_t انحراف_2,
                            size_t release) {
  rpmalloc_assert(!release || (release >= سائز_2), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= حافظہ_صفحہ_سائز),
                  "Invalid unmap size");
  اگر (release) {
    rpmalloc_assert(!(release % حافظہ_صفحہ_سائز), "Invalid unmap size");
    _rpmalloc_stat_sub(&mapped_صفحات, (release >> حافظہ_صفحہ_سائز_سرکاؤ));
    rpmalloc_stat_جوڑو(&unmapped_کل, (release >> حافظہ_صفحہ_سائز_سرکاؤ));
  }
  حافظہ_config.حافظہ_unmap(پتہ_2, سائز_2, انحراف_2, release);
}

//! Default implementation to map new pages to virtual memory
ساکن خالی *_rpmalloc_mmap_os(size_t سائز_2, size_t *انحراف_2) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((سائز_2 >= حافظہ_وسعت_سائز) &&
                    (حافظہ_وسعت_سائز > حافظہ_map_granularity))
                       ? حافظہ_وسعت_سائز
                       : 0;
  rpmalloc_assert(سائز_2 >= حافظہ_صفحہ_سائز, "Invalid mmap size");
#اگر PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  خالی *ptr = VirtualAlloc(0, سائز_2 + padding,
                           (حافظہ_huge_صفحات ? MEM_LARGE_صفحات : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           صفحہ_READWRITE);
  اگر (!ptr) {
    اگر (حافظہ_config.map_fail_callback) {
      اگر (حافظہ_config.map_fail_callback(سائز_2 + padding))
        واپس _rpmalloc_mmap_os(سائز_2, انحراف_2);
    } ورنہ {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    واپس 0;
  }
#ورنہ
  صحیح flags = MAP_نجی | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#اگر defined(__APPLE__) && !ہدف_OS_IPHONE && !ہدف_OS_SIMULATOR
  صحیح fd = (صحیح)VM_MAKE_لیبل(240U);
  اگر (حافظہ_huge_صفحات)
    fd |= VM_FLAGS_SUPERPAGE_سائز_2_MB;
  خالی *ptr = mmap(0, سائز_2 + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#وگرنہ_اگر defined(MAP_HUGETLB)
  خالی *ptr = mmap(0, سائز_2 + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (حافظہ_huge_صفحات ? MAP_HUGETLB : 0) | flags, -1, 0);
#اگر defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  اگر ((ptr == MAP_FAILED || !ptr) && حافظہ_huge_صفحات) {
    ptr = mmap(0, سائز_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    اگر (ptr && ptr != MAP_FAILED) {
      صحیح prm = madvise(ptr, سائز_2 + padding, MADV_HUGEPAGE);
      (خالی)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#اگر_ختم
  rpmalloc_سیٹ_name(ptr, سائز_2 + padding);
#وگرنہ_اگر defined(MAP_ALIGNED)
  ثابت size_t align =
      (حجم(size_t) * 8) - (size_t)(__builtin_clzl(سائز_2 - 1));
  خالی *ptr =
      mmap(0, سائز_2 + padding, PROT_READ | PROT_WRITE,
           (حافظہ_huge_صفحات ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#وگرنہ_اگر defined(MAP_ALIGN)
  caddr_t بنیاد_2 = (حافظہ_huge_صفحات ? (caddr_t)(4 << 20) : 0);
  خالی *ptr = mmap(بنیاد_2, سائز_2 + padding, PROT_READ | PROT_WRITE,
                   (حافظہ_huge_صفحات ? MAP_ALIGN : 0) | flags, -1, 0);
#ورنہ
  خالی *ptr = mmap(0, سائز_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#اگر_ختم
  اگر ((ptr == MAP_FAILED) || !ptr) {
    اگر (حافظہ_config.map_fail_callback) {
      اگر (حافظہ_config.map_fail_callback(سائز_2 + padding))
        واپس _rpmalloc_mmap_os(سائز_2, انحراف_2);
    } ورنہ اگر (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    واپس 0;
  }
#اگر_ختم
  rpmalloc_stat_جوڑو(&mapped_صفحات_os,
                     (int32_t)((سائز_2 + padding) >> حافظہ_صفحہ_سائز_سرکاؤ));
  اگر (padding) {
    size_t final_padding = padding - ((uintptr_t)ptr & ~حافظہ_وسعت_نقاب);
    rpmalloc_assert(final_padding <= حافظہ_وسعت_سائز,
                    "Internal failure in padding");
    rpmalloc_assert(final_padding <= padding, "Internal failure in padding");
    rpmalloc_assert(!(final_padding % 8), "Internal failure in padding");
    ptr = اشارہ_گر_انحراف(ptr, final_padding);
    *انحراف_2 = final_padding >> 3;
  }
  rpmalloc_assert((سائز_2 < حافظہ_وسعت_سائز) ||
                      !((uintptr_t)ptr & ~حافظہ_وسعت_نقاب),
                  "Internal failure in padding");
  واپس ptr;
}

//! Default implementation to unmap pages from virtual memory
ساکن خالی _rpmalloc_unmap_os(خالی *پتہ_2, size_t سائز_2, size_t انحراف_2,
                               size_t release) {
  rpmalloc_assert(release || (انحراف_2 == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= حافظہ_صفحہ_سائز),
                  "Invalid unmap size");
  rpmalloc_assert(سائز_2 >= حافظہ_صفحہ_سائز, "Invalid unmap size");
  اگر (release && انحراف_2) {
    انحراف_2 <<= 3;
    پتہ_2 = اشارہ_گر_انحراف(پتہ_2, -(int32_t)انحراف_2);
    اگر ((release >= حافظہ_وسعت_سائز) &&
        (حافظہ_وسعت_سائز > حافظہ_map_granularity)) {
      // Padding is always one span size
      release += حافظہ_وسعت_سائز;
    }
  }
#اگر !DISABLE_UNMAP
#اگر PLATFORM_WINDOWS
  اگر (!VirtualFree(پتہ_2, release ? 0 : سائز_2,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#ورنہ
  اگر (release) {
    اگر (munmap(پتہ_2, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } ورنہ {
#اگر defined(MADV_FREE_REUSABLE)
    صحیح ret;
    جب ((ret = madvise(پتہ_2, سائز_2, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    اگر ((ret == -1) && (errno != 0)) {
#وگرنہ_اگر defined(MADV_DONTNEED)
    اگر (madvise(پتہ_2, سائز_2, MADV_DONTNEED)) {
#وگرنہ_اگر defined(MADV_PAGEOUT)
    اگر (madvise(پتہ_2, سائز_2, MADV_PAGEOUT)) {
#وگرنہ_اگر defined(MADV_FREE)
    اگر (madvise(پتہ_2, سائز_2, MADV_FREE)) {
#ورنہ
    اگر (posix_madvise(پتہ_2, سائز_2, POSIX_MADV_DONTNEED)) {
#اگر_ختم
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#اگر_ختم
#اگر_ختم
  اگر (release)
    _rpmalloc_stat_sub(&mapped_صفحات_os, release >> حافظہ_صفحہ_سائز_سرکاؤ);
}

ساکن خالی rpmalloc_وسعت_mark_as_subspan_unless_master(وسعت_t *master,
                                                         وسعت_t *subspan,
                                                         size_t وسعت_گنتی);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
ساکن وسعت_t *_rpmalloc_global_get_reserved_spans(size_t وسعت_گنتی) {
  وسعت_t *وسعت = حافظہ_global_reserve;
  rpmalloc_وسعت_mark_as_subspan_unless_master(حافظہ_global_reserve_master,
                                               وسعت, وسعت_گنتی);
  حافظہ_global_reserve_گنتی -= وسعت_گنتی;
  اگر (حافظہ_global_reserve_گنتی)
    حافظہ_global_reserve =
        (وسعت_t *)اشارہ_گر_انحراف(وسعت, وسعت_گنتی << حافظہ_وسعت_سائز_سرکاؤ);
  ورنہ
    حافظہ_global_reserve = 0;
  واپس وسعت;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
ساکن خالی rpmalloc_global_سیٹ_reserved_spans(وسعت_t *master, وسعت_t *reserve,
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
ساکن خالی rpmalloc_وسعت_double_link_list_جوڑو(وسعت_t **سر, وسعت_t *وسعت) {
  اگر (*سر)
    (*سر)->پچھلا = وسعت;
  وسعت->اگلا = *سر;
  *سر = وسعت;
}

//! Pop head span from double linked list
ساکن خالی rpmalloc_وسعت_double_link_list_نکالو_سر(وسعت_t **سر,
                                                     وسعت_t *وسعت) {
  rpmalloc_assert(*سر == وسعت, "Linked list corrupted");
  وسعت = *سر;
  *سر = وسعت->اگلا;
}

//! Remove a span from double linked list
ساکن خالی rpmalloc_وسعت_double_link_list_remove(وسعت_t **سر,
                                                   وسعت_t *وسعت) {
  rpmalloc_assert(*سر, "Linked list corrupted");
  اگر (*سر == وسعت) {
    *سر = وسعت->اگلا;
  } ورنہ {
    وسعت_t *اگلا_وسعت = وسعت->اگلا;
    وسعت_t *پچھلا_وسعت = وسعت->پچھلا;
    پچھلا_وسعت->اگلا = اگلا_وسعت;
    اگر (EXPECTED(اگلا_وسعت != 0))
      اگلا_وسعت->پچھلا = پچھلا_وسعت;
  }
}

////////////
///
/// Span control
///
//////

ساکن خالی rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_t *ڈھیر_2, وسعت_t *وسعت);

ساکن خالی rpmalloc_ڈھیر_finalize(ڈھیر_t *ڈھیر_2);

ساکن خالی rpmalloc_ڈھیر_سیٹ_reserved_spans(ڈھیر_t *ڈھیر_2, وسعت_t *master,
                                              وسعت_t *reserve,
                                              size_t reserve_وسعت_گنتی);

//! Declare the span to be a subspan and store distance from master span and
//! span count
ساکن خالی rpmalloc_وسعت_mark_as_subspan_unless_master(وسعت_t *master,
                                                         وسعت_t *subspan,
                                                         size_t وسعت_گنتی) {
  rpmalloc_assert((subspan != master) || (subspan->flags & وسعت_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  اگر (subspan != master) {
    subspan->flags = وسعت_FLAG_SUBSPAN;
    subspan->انحراف_from_master =
        (uint32_t)((uintptr_t)اشارہ_گر_diff(subspan, master) >>
                   حافظہ_وسعت_سائز_سرکاؤ);
    subspan->align_انحراف = 0;
  }
  subspan->وسعت_گنتی = (uint32_t)وسعت_گنتی;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
ساکن وسعت_t *rpmalloc_وسعت_map_from_reserve(ڈھیر_t *ڈھیر_2,
                                               size_t وسعت_گنتی) {
  // Update the heap span reserve
  وسعت_t *وسعت = ڈھیر_2->وسعت_reserve;
  ڈھیر_2->وسعت_reserve =
      (وسعت_t *)اشارہ_گر_انحراف(وسعت, وسعت_گنتی * حافظہ_وسعت_سائز);
  ڈھیر_2->spans_reserved -= (uint32_t)وسعت_گنتی;

  rpmalloc_وسعت_mark_as_subspan_unless_master(ڈھیر_2->وسعت_reserve_master, وسعت,
                                               وسعت_گنتی);
  اگر (وسعت_گنتی <= LARGE_CLASS_گنتی)
    _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_from_reserved);

  واپس وسعت;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
ساکن size_t rpmalloc_وسعت_align_گنتی(size_t وسعت_گنتی) {
  size_t request_گنتی = (وسعت_گنتی > حافظہ_وسعت_map_گنتی)
                             ? وسعت_گنتی
                             : حافظہ_وسعت_map_گنتی;
  اگر ((حافظہ_صفحہ_سائز > حافظہ_وسعت_سائز) &&
      ((request_گنتی * حافظہ_وسعت_سائز) % حافظہ_صفحہ_سائز))
    request_گنتی +=
        حافظہ_وسعت_map_گنتی - (request_گنتی % حافظہ_وسعت_map_گنتی);
  واپس request_گنتی;
}

//! Setup a newly mapped span
ساکن خالی rpmalloc_وسعت_initialize(وسعت_t *وسعت, size_t کل_وسعت_گنتی,
                                      size_t وسعت_گنتی, size_t align_انحراف) {
  وسعت->کل_spans = (uint32_t)کل_وسعت_گنتی;
  وسعت->وسعت_گنتی = (uint32_t)وسعت_گنتی;
  وسعت->align_انحراف = (uint32_t)align_انحراف;
  وسعت->flags = وسعت_FLAG_MASTER;
  atomic_store32(&وسعت->بقایا_spans, (int32_t)کل_وسعت_گنتی);
}

ساکن خالی rpmalloc_وسعت_unmap(وسعت_t *وسعت);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
ساکن وسعت_t *rpmalloc_وسعت_map_aligned_گنتی(ڈھیر_t *ڈھیر_2,
                                                size_t وسعت_گنتی) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_وسعت_گنتی = rpmalloc_وسعت_align_گنتی(وسعت_گنتی);
  size_t align_انحراف = 0;
  وسعت_t *وسعت = (وسعت_t *)_rpmalloc_mmap(
      aligned_وسعت_گنتی * حافظہ_وسعت_سائز, &align_انحراف);
  اگر (!وسعت)
    واپس 0;
  rpmalloc_وسعت_initialize(وسعت, aligned_وسعت_گنتی, وسعت_گنتی, align_انحراف);
  _rpmalloc_stat_inc(&_master_spans);
  اگر (وسعت_گنتی <= LARGE_CLASS_گنتی)
    _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_map_calls);
  اگر (aligned_وسعت_گنتی > وسعت_گنتی) {
    وسعت_t *reserved_spans =
        (وسعت_t *)اشارہ_گر_انحراف(وسعت, وسعت_گنتی * حافظہ_وسعت_سائز);
    size_t reserved_گنتی = aligned_وسعت_گنتی - وسعت_گنتی;
    اگر (ڈھیر_2->spans_reserved) {
      rpmalloc_وسعت_mark_as_subspan_unless_master(
          ڈھیر_2->وسعت_reserve_master, ڈھیر_2->وسعت_reserve, ڈھیر_2->spans_reserved);
      rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, ڈھیر_2->وسعت_reserve);
    }
    اگر (reserved_گنتی > حافظہ_ڈھیر_reserve_گنتی) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&حافظہ_global_تالا) == 1,
                      "Global spin lock not held as expected");
      size_t remain_گنتی = reserved_گنتی - حافظہ_ڈھیر_reserve_گنتی;
      reserved_گنتی = حافظہ_ڈھیر_reserve_گنتی;
      وسعت_t *remain_وسعت = (وسعت_t *)اشارہ_گر_انحراف(
          reserved_spans, reserved_گنتی * حافظہ_وسعت_سائز);
      اگر (حافظہ_global_reserve) {
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
  واپس وسعت;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
ساکن وسعت_t *rpmalloc_وسعت_map(ڈھیر_t *ڈھیر_2, size_t وسعت_گنتی) {
  اگر (وسعت_گنتی <= ڈھیر_2->spans_reserved)
    واپس rpmalloc_وسعت_map_from_reserve(ڈھیر_2, وسعت_گنتی);
  وسعت_t *وسعت = 0;
  صحیح use_global_reserve =
      (حافظہ_صفحہ_سائز > حافظہ_وسعت_سائز) ||
      (حافظہ_وسعت_map_گنتی > حافظہ_ڈھیر_reserve_گنتی);
  اگر (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    جب (!atomic_cas32_acquire(&حافظہ_global_تالا, 1, 0))
      _rpmalloc_spin();
    اگر (حافظہ_global_reserve_گنتی >= وسعت_گنتی) {
      size_t reserve_گنتی =
          (!ڈھیر_2->spans_reserved ? حافظہ_ڈھیر_reserve_گنتی : وسعت_گنتی);
      اگر (حافظہ_global_reserve_گنتی < reserve_گنتی)
        reserve_گنتی = حافظہ_global_reserve_گنتی;
      وسعت = _rpmalloc_global_get_reserved_spans(reserve_گنتی);
      اگر (وسعت) {
        اگر (reserve_گنتی > وسعت_گنتی) {
          وسعت_t *reserved_وسعت = (وسعت_t *)اشارہ_گر_انحراف(
              وسعت, وسعت_گنتی << حافظہ_وسعت_سائز_سرکاؤ);
          rpmalloc_ڈھیر_سیٹ_reserved_spans(ڈھیر_2, حافظہ_global_reserve_master,
                                            reserved_وسعت,
                                            reserve_گنتی - وسعت_گنتی);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        وسعت->وسعت_گنتی = (uint32_t)وسعت_گنتی;
      }
    }
  }
  اگر (!وسعت)
    وسعت = rpmalloc_وسعت_map_aligned_گنتی(ڈھیر_2, وسعت_گنتی);
  اگر (use_global_reserve)
    atomic_store32_release(&حافظہ_global_تالا, 0);
  واپس وسعت;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
ساکن خالی rpmalloc_وسعت_unmap(وسعت_t *وسعت) {
  rpmalloc_assert((وسعت->flags & وسعت_FLAG_MASTER) ||
                      (وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(وسعت->flags & وسعت_FLAG_MASTER) ||
                      !(وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");

  صحیح is_master = !!(وسعت->flags & وسعت_FLAG_MASTER);
  وسعت_t *master =
      is_master ? وسعت
                : ((وسعت_t *)اشارہ_گر_انحراف(
                      وسعت, -(intptr_t)((uintptr_t)وسعت->انحراف_from_master *
                                        حافظہ_وسعت_سائز)));
  rpmalloc_assert(is_master || (وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & وسعت_FLAG_MASTER, "Span flag corrupted");

  size_t وسعت_گنتی = وسعت->وسعت_گنتی;
  اگر (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(وسعت->align_انحراف == 0, "Span align offset corrupted");
    اگر (حافظہ_وسعت_سائز >= حافظہ_صفحہ_سائز)
      _rpmalloc_unmap(وسعت, وسعت_گنتی * حافظہ_وسعت_سائز, 0, 0);
  } ورنہ {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    وسعت->flags |=
        وسعت_FLAG_MASTER | وسعت_FLAG_SUBSPAN | وسعت_FLAG_UNMAPPED_MASTER;
    rpmalloc_stat_جوڑو(&_unmapped_master_spans, 1);
  }

  اگر (atomic_add32(&master->بقایا_spans, -(int32_t)وسعت_گنتی) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & وسعت_FLAG_MASTER) &&
                        !!(master->flags & وسعت_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_گنتی = master->وسعت_گنتی;
    اگر (حافظہ_وسعت_سائز < حافظہ_صفحہ_سائز)
      unmap_گنتی = master->کل_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_گنتی * حافظہ_وسعت_سائز,
                    master->align_انحراف,
                    (size_t)master->کل_spans * حافظہ_وسعت_سائز);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
ساکن خالی rpmalloc_وسعت_release_مقصد_cache(ڈھیر_t *ڈھیر_2, وسعت_t *وسعت) {
  rpmalloc_assert(ڈھیر_2 == وسعت->ڈھیر_2, "Span heap pointer corrupted");
  rpmalloc_assert(وسعت->سائز_class < سائز_CLASS_گنتی,
                  "Invalid span size class");
  rpmalloc_assert(وسعت->وسعت_گنتی == 1, "Invalid span count");
#اگر ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&ڈھیر_2->وسعت_use[0].current);
#اگر_ختم
  _rpmalloc_stat_dec(&ڈھیر_2->سائز_class_use[وسعت->سائز_class].spans_current);
  اگر (!ڈھیر_2->finalize) {
    _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[0].spans_مقصد_cache);
    _rpmalloc_stat_inc(&ڈھیر_2->سائز_class_use[وسعت->سائز_class].spans_مقصد_cache);
    اگر (ڈھیر_2->سائز_class[وسعت->سائز_class].cache)
      rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2,
                                  ڈھیر_2->سائز_class[وسعت->سائز_class].cache);
    ڈھیر_2->سائز_class[وسعت->سائز_class].cache = وسعت;
  } ورنہ {
    rpmalloc_وسعت_unmap(وسعت);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
ساکن uint32_t free_list_partial_init(خالی **list, خالی **پہلا_بلاک,
                                       خالی *صفحہ_آغاز, خالی *بلاک_آغاز,
                                       uint32_t بلاک_گنتی,
                                       uint32_t بلاک_سائز_2) {
  rpmalloc_assert(بلاک_گنتی, "Internal failure");
  *پہلا_بلاک = بلاک_آغاز;
  اگر (بلاک_گنتی > 1) {
    خالی *free_بلاک = اشارہ_گر_انحراف(بلاک_آغاز, بلاک_سائز_2);
    خالی *بلاک_انتہا =
        اشارہ_گر_انحراف(بلاک_آغاز, (size_t)بلاک_سائز_2 * بلاک_گنتی);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    اگر (بلاک_سائز_2 < (حافظہ_صفحہ_سائز >> 1)) {
      خالی *صفحہ_انتہا = اشارہ_گر_انحراف(صفحہ_آغاز, حافظہ_صفحہ_سائز);
      اگر (صفحہ_انتہا < بلاک_انتہا)
        بلاک_انتہا = صفحہ_انتہا;
    }
    *list = free_بلاک;
    بلاک_گنتی = 2;
    خالی *اگلا_بلاک = اشارہ_گر_انحراف(free_بلاک, بلاک_سائز_2);
    جب (اگلا_بلاک < بلاک_انتہا) {
      *((خالی **)free_بلاک) = اگلا_بلاک;
      free_بلاک = اگلا_بلاک;
      ++بلاک_گنتی;
      اگلا_بلاک = اشارہ_گر_انحراف(اگلا_بلاک, بلاک_سائز_2);
    }
    *((خالی **)free_بلاک) = 0;
  } ورنہ {
    *list = 0;
  }
  واپس بلاک_گنتی;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
ساکن خالی *rpmalloc_وسعت_initialize_new(ڈھیر_t *ڈھیر_2,
                                           ڈھیر_سائز_class_t *ڈھیر_سائز_class,
                                           وسعت_t *وسعت, uint32_t class_idx) {
  rpmalloc_assert(وسعت->وسعت_گنتی == 1, "Internal failure");
  سائز_class_t *سائز_class = حافظہ_سائز_class + class_idx;
  وسعت->سائز_class = class_idx;
  وسعت->ڈھیر_2 = ڈھیر_2;
  وسعت->flags &= ~وسعت_FLAG_ALIGNED_بلاکیں;
  وسعت->بلاک_سائز_2 = سائز_class->بلاک_سائز_2;
  وسعت->بلاک_گنتی = سائز_class->بلاک_گنتی;
  وسعت->free_list = 0;
  وسعت->list_سائز = 0;
  atomic_store_ptr_release(&وسعت->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  خالی *بلاک;
  وسعت->free_list_حد_بندی =
      free_list_partial_init(&ڈھیر_سائز_class->free_list, &بلاک, وسعت,
                             اشارہ_گر_انحراف(وسعت, وسعت_سرخی_سائز),
                             سائز_class->بلاک_گنتی, سائز_class->بلاک_سائز_2);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  اگر (وسعت->free_list_حد_بندی < وسعت->بلاک_گنتی) {
    rpmalloc_وسعت_double_link_list_جوڑو(&ڈھیر_سائز_class->partial_وسعت, وسعت);
    وسعت->مستعمل_گنتی = وسعت->free_list_حد_بندی;
  } ورنہ {
#اگر RPMALLOC_پہلا_CLASS_HEAPS
    rpmalloc_وسعت_double_link_list_جوڑو(&ڈھیر_2->بھرا_وسعت[class_idx], وسعت);
#اگر_ختم
    ++ڈھیر_2->بھرا_وسعت_گنتی;
    وسعت->مستعمل_گنتی = وسعت->بلاک_گنتی;
  }
  واپس بلاک;
}

ساکن خالی rpmalloc_وسعت_نکالو_free_list_deferred(وسعت_t *وسعت) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  کرو {
    وسعت->free_list =
        atomic_exchange_ptr_acquire(&وسعت->free_list_deferred, INVALID_اشارہ_گر);
  } جب (وسعت->free_list == INVALID_اشارہ_گر);
  وسعت->مستعمل_گنتی -= وسعت->list_سائز;
  وسعت->list_سائز = 0;
  atomic_store_ptr_release(&وسعت->free_list_deferred, 0);
}

ساکن صحیح rpmalloc_وسعت_is_fully_utilized(وسعت_t *وسعت) {
  rpmalloc_assert(وسعت->free_list_حد_بندی <= وسعت->بلاک_گنتی,
                  "Span free list corrupted");
  واپس !وسعت->free_list && (وسعت->free_list_حد_بندی >= وسعت->بلاک_گنتی);
}

ساکن صحیح rpmalloc_وسعت_finalize(ڈھیر_t *ڈھیر_2, size_t iclass, وسعت_t *وسعت,
                                   وسعت_t **list_سر) {
  خالی *free_list = ڈھیر_2->سائز_class[iclass].free_list;
  وسعت_t *class_وسعت = (وسعت_t *)((uintptr_t)free_list & حافظہ_وسعت_نقاب);
  اگر (وسعت == class_وسعت) {
    // Adopt the heap class free list back into the span free list
    خالی *بلاک = وسعت->free_list;
    خالی *آخری_بلاک = 0;
    جب (بلاک) {
      آخری_بلاک = بلاک;
      بلاک = *((خالی **)بلاک);
    }
    uint32_t free_گنتی = 0;
    بلاک = free_list;
    جب (بلاک) {
      ++free_گنتی;
      بلاک = *((خالی **)بلاک);
    }
    اگر (آخری_بلاک) {
      *((خالی **)آخری_بلاک) = free_list;
    } ورنہ {
      وسعت->free_list = free_list;
    }
    ڈھیر_2->سائز_class[iclass].free_list = 0;
    وسعت->مستعمل_گنتی -= free_گنتی;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(وسعت->list_سائز == وسعت->مستعمل_گنتی, "Memory leak detected");
  اگر (وسعت->list_سائز == وسعت->مستعمل_گنتی) {
    _rpmalloc_stat_dec(&ڈھیر_2->وسعت_use[0].current);
    _rpmalloc_stat_dec(&ڈھیر_2->سائز_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    اگر (list_سر)
      rpmalloc_وسعت_double_link_list_remove(list_سر, وسعت);
    rpmalloc_وسعت_unmap(وسعت);
    واپس 1;
  }
  واپس 0;
}

////////////
///
/// Global cache
///
//////

#اگر ENABLE_GLOBAL_CACHE

//! Finalize a global cache
ساکن خالی _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  جب (!atomic_cas32_acquire(&cache->تالا_2, 1, 0))
    _rpmalloc_spin();

  برائے (size_t ispan = 0; ispan < cache->گنتی; ++ispan)
    rpmalloc_وسعت_unmap(cache->وسعت[ispan]);
  cache->گنتی = 0;

  جب (cache->حد_سے_تجاوز_2) {
    وسعت_t *وسعت = cache->حد_سے_تجاوز_2;
    cache->حد_سے_تجاوز_2 = وسعت->اگلا;
    rpmalloc_وسعت_unmap(وسعت);
  }

  atomic_store32_release(&cache->تالا_2, 0);
}

ساکن خالی rpmalloc_global_cache_داخل_کرو_spans(وسعت_t **وسعت,
                                                size_t وسعت_گنتی,
                                                size_t گنتی) {
  ثابت size_t cache_حد_بندی =
      (وسعت_گنتی == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_وسعت_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_وسعت_LARGE_CACHE - (وسعت_گنتی >> 1));

  global_cache_t *cache = &حافظہ_وسعت_cache[وسعت_گنتی - 1];

  size_t داخل_کرو_گنتی = گنتی;
  جب (!atomic_cas32_acquire(&cache->تالا_2, 1, 0))
    _rpmalloc_spin();

#اگر ENABLE_STATISTICS
  cache->داخل_کرو_گنتی += گنتی;
#اگر_ختم
  اگر ((cache->گنتی + داخل_کرو_گنتی) > cache_حد_بندی)
    داخل_کرو_گنتی = cache_حد_بندی - cache->گنتی;

  حافظہ_نقل(cache->وسعت + cache->گنتی, وسعت, حجم(وسعت_t *) * داخل_کرو_گنتی);
  cache->گنتی += (uint32_t)داخل_کرو_گنتی;

#اگر ENABLE_UNLIMITED_CACHE
  جب (داخل_کرو_گنتی < گنتی) {
#ورنہ
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  جب ((حافظہ_صفحہ_سائز > حافظہ_وسعت_سائز) && (داخل_کرو_گنتی < گنتی)) {
#اگر_ختم
    وسعت_t *current_وسعت = وسعت[داخل_کرو_گنتی++];
    current_وسعت->اگلا = cache->حد_سے_تجاوز_2;
    cache->حد_سے_تجاوز_2 = current_وسعت;
  }
  atomic_store32_release(&cache->تالا_2, 0);

  وسعت_t *keep = 0;
  برائے (size_t ispan = داخل_کرو_گنتی; ispan < گنتی; ++ispan) {
    وسعت_t *current_وسعت = وسعت[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    اگر ((current_وسعت->flags & وسعت_FLAG_MASTER) &&
        (atomic_load32(&current_وسعت->بقایا_spans) >
         (int32_t)current_وسعت->وسعت_گنتی)) {
      current_وسعت->اگلا = keep;
      keep = current_وسعت;
    } ورنہ {
      rpmalloc_وسعت_unmap(current_وسعت);
    }
  }

  اگر (keep) {
    جب (!atomic_cas32_acquire(&cache->تالا_2, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    جب (keep) {
      برائے (; islot < cache->گنتی; ++islot) {
        وسعت_t *current_وسعت = cache->وسعت[islot];
        اگر (!(current_وسعت->flags & وسعت_FLAG_MASTER) ||
            ((current_وسعت->flags & وسعت_FLAG_MASTER) &&
             (atomic_load32(&current_وسعت->بقایا_spans) <=
              (int32_t)current_وسعت->وسعت_گنتی))) {
          rpmalloc_وسعت_unmap(current_وسعت);
          cache->وسعت[islot] = keep;
          توڑو;
        }
      }
      اگر (islot == cache->گنتی)
        توڑو;
      keep = keep->اگلا;
    }

    اگر (keep) {
      وسعت_t *دم_2 = keep;
      جب (دم_2->اگلا)
        دم_2 = دم_2->اگلا;
      دم_2->اگلا = cache->حد_سے_تجاوز_2;
      cache->حد_سے_تجاوز_2 = keep;
    }

    atomic_store32_release(&cache->تالا_2, 0);
  }
}

ساکن size_t rpmalloc_global_cache_نکالو_spans(وسعت_t **وسعت,
                                                   size_t وسعت_گنتی,
                                                   size_t گنتی) {
  global_cache_t *cache = &حافظہ_وسعت_cache[وسعت_گنتی - 1];

  size_t نکالو_گنتی_2 = 0;
  جب (!atomic_cas32_acquire(&cache->تالا_2, 1, 0))
    _rpmalloc_spin();

#اگر ENABLE_STATISTICS
  cache->نکالو_گنتی_2 += گنتی;
#اگر_ختم
  size_t want = گنتی - نکالو_گنتی_2;
  اگر (want > cache->گنتی)
    want = cache->گنتی;

  حافظہ_نقل(وسعت + نکالو_گنتی_2, cache->وسعت + (cache->گنتی - want),
         حجم(وسعت_t *) * want);
  cache->گنتی -= (uint32_t)want;
  نکالو_گنتی_2 += want;

  جب ((نکالو_گنتی_2 < گنتی) && cache->حد_سے_تجاوز_2) {
    وسعت_t *current_وسعت = cache->حد_سے_تجاوز_2;
    وسعت[نکالو_گنتی_2++] = current_وسعت;
    cache->حد_سے_تجاوز_2 = current_وسعت->اگلا;
  }

#اگر ENABLE_ASSERTS
  برائے (size_t ispan = 0; ispan < نکالو_گنتی_2; ++ispan) {
    rpmalloc_assert(وسعت[ispan]->وسعت_گنتی == وسعت_گنتی,
                    "Global cache span count mismatch");
  }
#اگر_ختم

  atomic_store32_release(&cache->تالا_2, 0);

  واپس نکالو_گنتی_2;
}

#اگر_ختم

////////////
///
/// Heap control
///
//////

ساکن خالی _rpmalloc_deallocate_huge(وسعت_t *);

//! Store the given spans as reserve in the given heap
ساکن خالی rpmalloc_ڈھیر_سیٹ_reserved_spans(ڈھیر_t *ڈھیر_2, وسعت_t *master,
                                              وسعت_t *reserve,
                                              size_t reserve_وسعت_گنتی) {
  ڈھیر_2->وسعت_reserve_master = master;
  ڈھیر_2->وسعت_reserve = reserve;
  ڈھیر_2->spans_reserved = (uint32_t)reserve_وسعت_گنتی;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
ساکن خالی rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_t *ڈھیر_2,
                                                وسعت_t **single_وسعت) {
  وسعت_t *وسعت = (وسعت_t *)((خالی *)atomic_exchange_ptr_acquire(
      &ڈھیر_2->وسعت_free_deferred, 0));
  جب (وسعت) {
    وسعت_t *اگلا_وسعت = (وسعت_t *)وسعت->free_list;
    rpmalloc_assert(وسعت->ڈھیر_2 == ڈھیر_2, "Span heap pointer corrupted");
    اگر (EXPECTED(وسعت->سائز_class < سائز_CLASS_گنتی)) {
      rpmalloc_assert(ڈھیر_2->بھرا_وسعت_گنتی, "Heap span counter corrupted");
      --ڈھیر_2->بھرا_وسعت_گنتی;
      _rpmalloc_stat_dec(&ڈھیر_2->وسعت_use[0].spans_deferred);
#اگر RPMALLOC_پہلا_CLASS_HEAPS
      rpmalloc_وسعت_double_link_list_remove(&ڈھیر_2->بھرا_وسعت[وسعت->سائز_class],
                                             وسعت);
#اگر_ختم
      _rpmalloc_stat_dec(&ڈھیر_2->وسعت_use[0].current);
      _rpmalloc_stat_dec(&ڈھیر_2->سائز_class_use[وسعت->سائز_class].spans_current);
      اگر (single_وسعت && !*single_وسعت)
        *single_وسعت = وسعت;
      ورنہ
        rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
    } ورنہ {
      اگر (وسعت->سائز_class == سائز_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(وسعت);
      } ورنہ {
        rpmalloc_assert(وسعت->سائز_class == سائز_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(ڈھیر_2->بھرا_وسعت_گنتی, "Heap span counter corrupted");
        --ڈھیر_2->بھرا_وسعت_گنتی;
#اگر RPMALLOC_پہلا_CLASS_HEAPS
        rpmalloc_وسعت_double_link_list_remove(&ڈھیر_2->large_huge_وسعت, وسعت);
#اگر_ختم
        uint32_t idx = وسعت->وسعت_گنتی - 1;
        _rpmalloc_stat_dec(&ڈھیر_2->وسعت_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&ڈھیر_2->وسعت_use[idx].current);
        اگر (!idx && single_وسعت && !*single_وسعت)
          *single_وسعت = وسعت;
        ورنہ
          rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
      }
    }
    وسعت = اگلا_وسعت;
  }
}

ساکن خالی rpmalloc_ڈھیر_unmap(ڈھیر_t *ڈھیر_2) {
  اگر (!ڈھیر_2->master_ڈھیر) {
    اگر ((ڈھیر_2->finalize > 1) && !atomic_load32(&ڈھیر_2->بچہ_گنتی)) {
      وسعت_t *وسعت = (وسعت_t *)((uintptr_t)ڈھیر_2 & حافظہ_وسعت_نقاب);
      rpmalloc_وسعت_unmap(وسعت);
    }
  } ورنہ {
    اگر (atomic_decr32(&ڈھیر_2->master_ڈھیر->بچہ_گنتی) == 0) {
      rpmalloc_ڈھیر_unmap(ڈھیر_2->master_ڈھیر);
    }
  }
}

ساکن خالی rpmalloc_ڈھیر_global_finalize(ڈھیر_t *ڈھیر_2) {
  اگر (ڈھیر_2->finalize++ > 1) {
    --ڈھیر_2->finalize;
    واپس;
  }

  rpmalloc_ڈھیر_finalize(ڈھیر_2);

#اگر ENABLE_THREAD_CACHE
  برائے (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    وسعت_cache_t *وسعت_cache;
    اگر (!iclass)
      وسعت_cache = &ڈھیر_2->وسعت_cache;
    ورنہ
      وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (iclass - 1));
    برائے (size_t ispan = 0; ispan < وسعت_cache->گنتی; ++ispan)
      rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
    وسعت_cache->گنتی = 0;
  }
#اگر_ختم

  اگر (ڈھیر_2->بھرا_وسعت_گنتی) {
    --ڈھیر_2->finalize;
    واپس;
  }

  برائے (size_t iclass = 0; iclass < سائز_CLASS_گنتی; ++iclass) {
    اگر (ڈھیر_2->سائز_class[iclass].free_list ||
        ڈھیر_2->سائز_class[iclass].partial_وسعت) {
      --ڈھیر_2->finalize;
      واپس;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)ڈھیر_2->id % ڈھیر_ARRAY_سائز;
  ڈھیر_t *list_ڈھیر = حافظہ_heaps[list_idx];
  اگر (list_ڈھیر == ڈھیر_2) {
    حافظہ_heaps[list_idx] = ڈھیر_2->اگلا_ڈھیر;
  } ورنہ {
    جب (list_ڈھیر->اگلا_ڈھیر != ڈھیر_2)
      list_ڈھیر = list_ڈھیر->اگلا_ڈھیر;
    list_ڈھیر->اگلا_ڈھیر = ڈھیر_2->اگلا_ڈھیر;
  }

  rpmalloc_ڈھیر_unmap(ڈھیر_2);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
ساکن خالی rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_t *ڈھیر_2, وسعت_t *وسعت) {
  اگر (UNEXPECTED(ڈھیر_2->finalize != 0)) {
    rpmalloc_وسعت_unmap(وسعت);
    rpmalloc_ڈھیر_global_finalize(ڈھیر_2);
    واپس;
  }
#اگر ENABLE_THREAD_CACHE
  size_t وسعت_گنتی = وسعت->وسعت_گنتی;
  _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_مقصد_cache);
  اگر (وسعت_گنتی == 1) {
    وسعت_cache_t *وسعت_cache = &ڈھیر_2->وسعت_cache;
    وسعت_cache->وسعت[وسعت_cache->گنتی++] = وسعت;
    اگر (وسعت_cache->گنتی == MAX_THREAD_وسعت_CACHE) {
      ثابت size_t remain_گنتی =
          MAX_THREAD_وسعت_CACHE - THREAD_وسعت_CACHE_TRANSFER;
#اگر ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&ڈھیر_2->thread_مقصد_global,
                           THREAD_وسعت_CACHE_TRANSFER * حافظہ_وسعت_سائز);
      rpmalloc_stat_جوڑو(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_مقصد_global,
                         THREAD_وسعت_CACHE_TRANSFER);
      rpmalloc_global_cache_داخل_کرو_spans(وسعت_cache->وسعت + remain_گنتی,
                                          وسعت_گنتی,
                                          THREAD_وسعت_CACHE_TRANSFER);
#ورنہ
      برائے (size_t ispan = 0; ispan < THREAD_وسعت_CACHE_TRANSFER; ++ispan)
        rpmalloc_وسعت_unmap(وسعت_cache->وسعت[remain_گنتی + ispan]);
#اگر_ختم
      وسعت_cache->گنتی = remain_گنتی;
    }
  } ورنہ {
    size_t cache_idx = وسعت_گنتی - 2;
    وسعت_large_cache_t *وسعت_cache = ڈھیر_2->وسعت_large_cache + cache_idx;
    وسعت_cache->وسعت[وسعت_cache->گنتی++] = وسعت;
    ثابت size_t cache_حد_بندی =
        (MAX_THREAD_وسعت_LARGE_CACHE - (وسعت_گنتی >> 1));
    اگر (وسعت_cache->گنتی == cache_حد_بندی) {
      ثابت size_t transfer_حد_بندی = 2 + (cache_حد_بندی >> 2);
      ثابت size_t transfer_گنتی =
          (THREAD_وسعت_LARGE_CACHE_TRANSFER <= transfer_حد_بندی
               ? THREAD_وسعت_LARGE_CACHE_TRANSFER
               : transfer_حد_بندی);
      ثابت size_t remain_گنتی = cache_حد_بندی - transfer_گنتی;
#اگر ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&ڈھیر_2->thread_مقصد_global,
                           transfer_گنتی * وسعت_گنتی * حافظہ_وسعت_سائز);
      rpmalloc_stat_جوڑو(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_مقصد_global,
                         transfer_گنتی);
      rpmalloc_global_cache_داخل_کرو_spans(وسعت_cache->وسعت + remain_گنتی,
                                          وسعت_گنتی, transfer_گنتی);
#ورنہ
      برائے (size_t ispan = 0; ispan < transfer_گنتی; ++ispan)
        rpmalloc_وسعت_unmap(وسعت_cache->وسعت[remain_گنتی + ispan]);
#اگر_ختم
      وسعت_cache->گنتی = remain_گنتی;
    }
  }
#ورنہ
  (خالی)حجم(ڈھیر_2);
  rpmalloc_وسعت_unmap(وسعت);
#اگر_ختم
}

//! Extract the given number of spans from the different cache levels
ساکن وسعت_t *rpmalloc_ڈھیر_thread_cache_نکالو(ڈھیر_t *ڈھیر_2,
                                                   size_t وسعت_گنتی) {
  وسعت_t *وسعت = 0;
#اگر ENABLE_THREAD_CACHE
  وسعت_cache_t *وسعت_cache;
  اگر (وسعت_گنتی == 1)
    وسعت_cache = &ڈھیر_2->وسعت_cache;
  ورنہ
    وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (وسعت_گنتی - 2));
  اگر (وسعت_cache->گنتی) {
    _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_from_cache);
    واپس وسعت_cache->وسعت[--وسعت_cache->گنتی];
  }
#اگر_ختم
  واپس وسعت;
}

ساکن وسعت_t *rpmalloc_ڈھیر_thread_cache_deferred_نکالو(ڈھیر_t *ڈھیر_2,
                                                            size_t وسعت_گنتی) {
  وسعت_t *وسعت = 0;
  اگر (وسعت_گنتی == 1) {
    rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, &وسعت);
  } ورنہ {
    rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, 0);
    وسعت = rpmalloc_ڈھیر_thread_cache_نکالو(ڈھیر_2, وسعت_گنتی);
  }
  واپس وسعت;
}

ساکن وسعت_t *rpmalloc_ڈھیر_reserved_نکالو(ڈھیر_t *ڈھیر_2,
                                               size_t وسعت_گنتی) {
  اگر (ڈھیر_2->spans_reserved >= وسعت_گنتی)
    واپس rpmalloc_وسعت_map(ڈھیر_2, وسعت_گنتی);
  واپس 0;
}

//! Extract a span from the global cache
ساکن وسعت_t *rpmalloc_ڈھیر_global_cache_نکالو(ڈھیر_t *ڈھیر_2,
                                                   size_t وسعت_گنتی) {
#اگر ENABLE_GLOBAL_CACHE
#اگر ENABLE_THREAD_CACHE
  وسعت_cache_t *وسعت_cache;
  size_t wanted_گنتی;
  اگر (وسعت_گنتی == 1) {
    وسعت_cache = &ڈھیر_2->وسعت_cache;
    wanted_گنتی = THREAD_وسعت_CACHE_TRANSFER;
  } ورنہ {
    وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (وسعت_گنتی - 2));
    wanted_گنتی = THREAD_وسعت_LARGE_CACHE_TRANSFER;
  }
  وسعت_cache->گنتی = rpmalloc_global_cache_نکالو_spans(
      وسعت_cache->وسعت, وسعت_گنتی, wanted_گنتی);
  اگر (وسعت_cache->گنتی) {
    _rpmalloc_stat_add64(&ڈھیر_2->global_مقصد_thread,
                         وسعت_گنتی * وسعت_cache->گنتی * حافظہ_وسعت_سائز);
    rpmalloc_stat_جوڑو(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_from_global,
                       وسعت_cache->گنتی);
    واپس وسعت_cache->وسعت[--وسعت_cache->گنتی];
  }
#ورنہ
  وسعت_t *وسعت = 0;
  size_t گنتی = rpmalloc_global_cache_نکالو_spans(&وسعت, وسعت_گنتی, 1);
  اگر (گنتی) {
    _rpmalloc_stat_add64(&ڈھیر_2->global_مقصد_thread,
                         وسعت_گنتی * گنتی * حافظہ_وسعت_سائز);
    rpmalloc_stat_جوڑو(&ڈھیر_2->وسعت_use[وسعت_گنتی - 1].spans_from_global,
                       گنتی);
    واپس وسعت;
  }
#اگر_ختم
#اگر_ختم
  (خالی)حجم(ڈھیر_2);
  (خالی)حجم(وسعت_گنتی);
  واپس 0;
}

ساکن خالی rpmalloc_inc_وسعت_statistics(ڈھیر_t *ڈھیر_2, size_t وسعت_گنتی,
                                          uint32_t class_idx) {
  (خالی)حجم(ڈھیر_2);
  (خالی)حجم(وسعت_گنتی);
  (خالی)حجم(class_idx);
#اگر ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)وسعت_گنتی - 1;
  uint32_t current_گنتی =
      (uint32_t)atomic_incr32(&ڈھیر_2->وسعت_use[idx].current);
  اگر (current_گنتی > (uint32_t)atomic_load32(&ڈھیر_2->وسعت_use[idx].بلند))
    atomic_store32(&ڈھیر_2->وسعت_use[idx].بلند, (int32_t)current_گنتی);
  rpmalloc_stat_جوڑو_peak(&ڈھیر_2->سائز_class_use[class_idx].spans_current, 1,
                          ڈھیر_2->سائز_class_use[class_idx].spans_peak);
#اگر_ختم
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
ساکن وسعت_t *
rpmalloc_ڈھیر_نکالو_new_وسعت(ڈھیر_t *ڈھیر_2,
                                ڈھیر_سائز_class_t *ڈھیر_سائز_class,
                                size_t وسعت_گنتی, uint32_t class_idx) {
  وسعت_t *وسعت;
#اگر ENABLE_THREAD_CACHE
  اگر (ڈھیر_سائز_class && ڈھیر_سائز_class->cache) {
    وسعت = ڈھیر_سائز_class->cache;
    ڈھیر_سائز_class->cache =
        (ڈھیر_2->وسعت_cache.گنتی
             ? ڈھیر_2->وسعت_cache.وسعت[--ڈھیر_2->وسعت_cache.گنتی]
             : 0);
    rpmalloc_inc_وسعت_statistics(ڈھیر_2, وسعت_گنتی, class_idx);
    واپس وسعت;
  }
#اگر_ختم
  (خالی)حجم(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t بنیاد_وسعت_گنتی = وسعت_گنتی;
  size_t حد_بندی_وسعت_گنتی =
      (وسعت_گنتی > 2) ? (وسعت_گنتی + (وسعت_گنتی >> 1)) : وسعت_گنتی;
  اگر (حد_بندی_وسعت_گنتی > LARGE_CLASS_گنتی)
    حد_بندی_وسعت_گنتی = LARGE_CLASS_گنتی;
  کرو {
    وسعت = rpmalloc_ڈھیر_thread_cache_نکالو(ڈھیر_2, وسعت_گنتی);
    اگر (EXPECTED(وسعت != 0)) {
      _rpmalloc_stat_inc(&ڈھیر_2->سائز_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_وسعت_statistics(ڈھیر_2, وسعت_گنتی, class_idx);
      واپس وسعت;
    }
    وسعت = rpmalloc_ڈھیر_thread_cache_deferred_نکالو(ڈھیر_2, وسعت_گنتی);
    اگر (EXPECTED(وسعت != 0)) {
      _rpmalloc_stat_inc(&ڈھیر_2->سائز_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_وسعت_statistics(ڈھیر_2, وسعت_گنتی, class_idx);
      واپس وسعت;
    }
    وسعت = rpmalloc_ڈھیر_global_cache_نکالو(ڈھیر_2, وسعت_گنتی);
    اگر (EXPECTED(وسعت != 0)) {
      _rpmalloc_stat_inc(&ڈھیر_2->سائز_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_وسعت_statistics(ڈھیر_2, وسعت_گنتی, class_idx);
      واپس وسعت;
    }
    وسعت = rpmalloc_ڈھیر_reserved_نکالو(ڈھیر_2, وسعت_گنتی);
    اگر (EXPECTED(وسعت != 0)) {
      _rpmalloc_stat_inc(&ڈھیر_2->سائز_class_use[class_idx].spans_from_reserved);
      rpmalloc_inc_وسعت_statistics(ڈھیر_2, وسعت_گنتی, class_idx);
      واپس وسعت;
    }
    ++وسعت_گنتی;
  } جب (وسعت_گنتی <= حد_بندی_وسعت_گنتی);
  // Final fallback, map in more virtual memory
  وسعت = rpmalloc_وسعت_map(ڈھیر_2, بنیاد_وسعت_گنتی);
  rpmalloc_inc_وسعت_statistics(ڈھیر_2, بنیاد_وسعت_گنتی, class_idx);
  _rpmalloc_stat_inc(&ڈھیر_2->سائز_class_use[class_idx].spans_map_calls);
  واپس وسعت;
}

ساکن خالی rpmalloc_ڈھیر_initialize(ڈھیر_t *ڈھیر_2) {
  _rpmalloc_memset_const(ڈھیر_2, 0, حجم(ڈھیر_t));
  // Get a new heap ID
  ڈھیر_2->id = 1 + atomic_incr32(&حافظہ_ڈھیر_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)ڈھیر_2->id % ڈھیر_ARRAY_سائز;
  ڈھیر_2->اگلا_ڈھیر = حافظہ_heaps[list_idx];
  حافظہ_heaps[list_idx] = ڈھیر_2;
}

ساکن خالی rpmalloc_ڈھیر_orphan(ڈھیر_t *ڈھیر_2, صحیح پہلا_class) {
  ڈھیر_2->owner_thread = (uintptr_t)-1;
#اگر RPMALLOC_پہلا_CLASS_HEAPS
  ڈھیر_t **ڈھیر_list =
      (پہلا_class ? &حافظہ_پہلا_class_orphan_heaps : &حافظہ_orphan_heaps);
#ورنہ
  (خالی)حجم(پہلا_class);
  ڈھیر_t **ڈھیر_list = &حافظہ_orphan_heaps;
#اگر_ختم
  ڈھیر_2->اگلا_orphan = *ڈھیر_list;
  *ڈھیر_list = ڈھیر_2;
}

//! Allocate a new heap from newly mapped memory pages
ساکن ڈھیر_t *rpmalloc_ڈھیر_allocate_new(خالی) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t ڈھیر_سائز_2 = حجم(ڈھیر_t);
  size_t aligned_ڈھیر_سائز = 16 * ((ڈھیر_سائز_2 + 15) / 16);
  size_t request_ڈھیر_گنتی = 16;
  size_t ڈھیر_وسعت_گنتی = ((aligned_ڈھیر_سائز * request_ڈھیر_گنتی) +
                            حجم(وسعت_t) + حافظہ_وسعت_سائز - 1) /
                           حافظہ_وسعت_سائز;
  size_t بلاک_سائز_2 = حافظہ_وسعت_سائز * ڈھیر_وسعت_گنتی;
  size_t وسعت_گنتی = ڈھیر_وسعت_گنتی;
  وسعت_t *وسعت = 0;
  // If there are global reserved spans, use these first
  اگر (حافظہ_global_reserve_گنتی >= ڈھیر_وسعت_گنتی) {
    وسعت = _rpmalloc_global_get_reserved_spans(ڈھیر_وسعت_گنتی);
  }
  اگر (!وسعت) {
    اگر (حافظہ_صفحہ_سائز > بلاک_سائز_2) {
      وسعت_گنتی = حافظہ_صفحہ_سائز / حافظہ_وسعت_سائز;
      بلاک_سائز_2 = حافظہ_صفحہ_سائز;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_ڈھیر_گنتی =
          (بلاک_سائز_2 - حجم(وسعت_t)) / aligned_ڈھیر_سائز;
      اگر (possible_ڈھیر_گنتی >= (request_ڈھیر_گنتی * 16))
        request_ڈھیر_گنتی *= 16;
      ورنہ اگر (possible_ڈھیر_گنتی < request_ڈھیر_گنتی)
        request_ڈھیر_گنتی = possible_ڈھیر_گنتی;
      ڈھیر_وسعت_گنتی = ((aligned_ڈھیر_سائز * request_ڈھیر_گنتی) +
                         حجم(وسعت_t) + حافظہ_وسعت_سائز - 1) /
                        حافظہ_وسعت_سائز;
    }

    size_t align_انحراف = 0;
    وسعت = (وسعت_t *)_rpmalloc_mmap(بلاک_سائز_2, &align_انحراف);
    اگر (!وسعت)
      واپس 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    rpmalloc_وسعت_initialize(وسعت, وسعت_گنتی, ڈھیر_وسعت_گنتی, align_انحراف);
  }

  size_t remain_سائز = حافظہ_وسعت_سائز - حجم(وسعت_t);
  ڈھیر_t *ڈھیر_2 = (ڈھیر_t *)اشارہ_گر_انحراف(وسعت, حجم(وسعت_t));
  rpmalloc_ڈھیر_initialize(ڈھیر_2);

  // Put extra heaps as orphans
  size_t عدد_heaps = remain_سائز / aligned_ڈھیر_سائز;
  اگر (عدد_heaps < request_ڈھیر_گنتی)
    عدد_heaps = request_ڈھیر_گنتی;
  atomic_store32(&ڈھیر_2->بچہ_گنتی, (int32_t)عدد_heaps - 1);
  ڈھیر_t *extra_ڈھیر = (ڈھیر_t *)اشارہ_گر_انحراف(ڈھیر_2, aligned_ڈھیر_سائز);
  جب (عدد_heaps > 1) {
    rpmalloc_ڈھیر_initialize(extra_ڈھیر);
    extra_ڈھیر->master_ڈھیر = ڈھیر_2;
    rpmalloc_ڈھیر_orphan(extra_ڈھیر, 1);
    extra_ڈھیر = (ڈھیر_t *)اشارہ_گر_انحراف(extra_ڈھیر, aligned_ڈھیر_سائز);
    --عدد_heaps;
  }

  اگر (وسعت_گنتی > ڈھیر_وسعت_گنتی) {
    // Cap reserved spans
    size_t remain_گنتی = وسعت_گنتی - ڈھیر_وسعت_گنتی;
    size_t reserve_گنتی =
        (remain_گنتی > حافظہ_ڈھیر_reserve_گنتی ? حافظہ_ڈھیر_reserve_گنتی
                                                   : remain_گنتی);
    وسعت_t *remain_وسعت =
        (وسعت_t *)اشارہ_گر_انحراف(وسعت, ڈھیر_وسعت_گنتی * حافظہ_وسعت_سائز);
    rpmalloc_ڈھیر_سیٹ_reserved_spans(ڈھیر_2, وسعت, remain_وسعت, reserve_گنتی);

    اگر (remain_گنتی > reserve_گنتی) {
      // Set to global reserved spans
      remain_وسعت = (وسعت_t *)اشارہ_گر_انحراف(remain_وسعت,
                                             reserve_گنتی * حافظہ_وسعت_سائز);
      reserve_گنتی = remain_گنتی - reserve_گنتی;
      rpmalloc_global_سیٹ_reserved_spans(وسعت, remain_وسعت, reserve_گنتی);
    }
  }

  واپس ڈھیر_2;
}

ساکن ڈھیر_t *rpmalloc_ڈھیر_نکالو_orphan(ڈھیر_t **ڈھیر_list) {
  ڈھیر_t *ڈھیر_2 = *ڈھیر_list;
  *ڈھیر_list = (ڈھیر_2 ? ڈھیر_2->اگلا_orphan : 0);
  واپس ڈھیر_2;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
ساکن ڈھیر_t *rpmalloc_ڈھیر_allocate(صحیح پہلا_class) {
  ڈھیر_t *ڈھیر_2 = 0;
  جب (!atomic_cas32_acquire(&حافظہ_global_تالا, 1, 0))
    _rpmalloc_spin();
  اگر (پہلا_class == 0)
    ڈھیر_2 = rpmalloc_ڈھیر_نکالو_orphan(&حافظہ_orphan_heaps);
#اگر RPMALLOC_پہلا_CLASS_HEAPS
  اگر (!ڈھیر_2)
    ڈھیر_2 = rpmalloc_ڈھیر_نکالو_orphan(&حافظہ_پہلا_class_orphan_heaps);
#اگر_ختم
  اگر (!ڈھیر_2)
    ڈھیر_2 = rpmalloc_ڈھیر_allocate_new();
  atomic_store32_release(&حافظہ_global_تالا, 0);
  اگر (ڈھیر_2)
    rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, 0);
  واپس ڈھیر_2;
}

ساکن خالی rpmalloc_ڈھیر_release(خالی *heapptr, صحیح پہلا_class,
                                   صحیح release_cache) {
  ڈھیر_t *ڈھیر_2 = (ڈھیر_t *)heapptr;
  اگر (!ڈھیر_2)
    واپس;
  // Release thread cache spans back to global cache
  rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, 0);
  اگر (release_cache || ڈھیر_2->finalize) {
#اگر ENABLE_THREAD_CACHE
    برائے (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
      وسعت_cache_t *وسعت_cache;
      اگر (!iclass)
        وسعت_cache = &ڈھیر_2->وسعت_cache;
      ورنہ
        وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (iclass - 1));
      اگر (!وسعت_cache->گنتی)
        جاری;
#اگر ENABLE_GLOBAL_CACHE
      اگر (ڈھیر_2->finalize) {
        برائے (size_t ispan = 0; ispan < وسعت_cache->گنتی; ++ispan)
          rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
      } ورنہ {
        _rpmalloc_stat_add64(&ڈھیر_2->thread_مقصد_global, وسعت_cache->گنتی *
                                                          (iclass + 1) *
                                                          حافظہ_وسعت_سائز);
        rpmalloc_stat_جوڑو(&ڈھیر_2->وسعت_use[iclass].spans_مقصد_global,
                           وسعت_cache->گنتی);
        rpmalloc_global_cache_داخل_کرو_spans(وسعت_cache->وسعت, iclass + 1,
                                            وسعت_cache->گنتی);
      }
#ورنہ
      برائے (size_t ispan = 0; ispan < وسعت_cache->گنتی; ++ispan)
        rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
#اگر_ختم
      وسعت_cache->گنتی = 0;
    }
#اگر_ختم
  }

  اگر (get_thread_ڈھیر_raw() == ڈھیر_2)
    سیٹ_thread_ڈھیر(0);

#اگر ENABLE_STATISTICS
  atomic_decr32(&حافظہ_فعال_heaps);
  rpmalloc_assert(atomic_load32(&حافظہ_فعال_heaps) >= 0,
                  "Still active heaps during finalization");
#اگر_ختم

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  اگر (get_thread_id() != _rpmalloc_main_thread_id) {
    جب (!atomic_cas32_acquire(&حافظہ_global_تالا, 1, 0))
      _rpmalloc_spin();
  }
  rpmalloc_ڈھیر_orphan(ڈھیر_2, پہلا_class);
  atomic_store32_release(&حافظہ_global_تالا, 0);
}

ساکن خالی rpmalloc_ڈھیر_release_raw(خالی *heapptr, صحیح release_cache) {
  rpmalloc_ڈھیر_release(heapptr, 0, release_cache);
}

ساکن خالی rpmalloc_ڈھیر_release_raw_fc(خالی *heapptr) {
  rpmalloc_ڈھیر_release_raw(heapptr, 1);
}

ساکن خالی rpmalloc_ڈھیر_finalize(ڈھیر_t *ڈھیر_2) {
  اگر (ڈھیر_2->spans_reserved) {
    وسعت_t *وسعت = rpmalloc_وسعت_map(ڈھیر_2, ڈھیر_2->spans_reserved);
    rpmalloc_وسعت_unmap(وسعت);
    ڈھیر_2->spans_reserved = 0;
  }

  rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, 0);

  برائے (size_t iclass = 0; iclass < سائز_CLASS_گنتی; ++iclass) {
    اگر (ڈھیر_2->سائز_class[iclass].cache)
      rpmalloc_وسعت_unmap(ڈھیر_2->سائز_class[iclass].cache);
    ڈھیر_2->سائز_class[iclass].cache = 0;
    وسعت_t *وسعت = ڈھیر_2->سائز_class[iclass].partial_وسعت;
    جب (وسعت) {
      وسعت_t *اگلا = وسعت->اگلا;
      rpmalloc_وسعت_finalize(ڈھیر_2, iclass, وسعت,
                              &ڈھیر_2->سائز_class[iclass].partial_وسعت);
      وسعت = اگلا;
    }
    // If class still has a free list it must be a full span
    اگر (ڈھیر_2->سائز_class[iclass].free_list) {
      وسعت_t *class_وسعت =
          (وسعت_t *)((uintptr_t)ڈھیر_2->سائز_class[iclass].free_list &
                     حافظہ_وسعت_نقاب);
      وسعت_t **list = 0;
#اگر RPMALLOC_پہلا_CLASS_HEAPS
      list = &ڈھیر_2->بھرا_وسعت[iclass];
#اگر_ختم
      --ڈھیر_2->بھرا_وسعت_گنتی;
      اگر (!rpmalloc_وسعت_finalize(ڈھیر_2, iclass, class_وسعت, list)) {
        اگر (list)
          rpmalloc_وسعت_double_link_list_remove(list, class_وسعت);
        rpmalloc_وسعت_double_link_list_جوڑو(
            &ڈھیر_2->سائز_class[iclass].partial_وسعت, class_وسعت);
      }
    }
  }

#اگر ENABLE_THREAD_CACHE
  برائے (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    وسعت_cache_t *وسعت_cache;
    اگر (!iclass)
      وسعت_cache = &ڈھیر_2->وسعت_cache;
    ورنہ
      وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (iclass - 1));
    برائے (size_t ispan = 0; ispan < وسعت_cache->گنتی; ++ispan)
      rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
    وسعت_cache->گنتی = 0;
  }
#اگر_ختم
  rpmalloc_assert(!atomic_بھرو_ptr(&ڈھیر_2->وسعت_free_deferred),
                  "Heaps still active during finalization");
}

////////////
///
/// Allocation entry points
///
//////

//! Pop first block from a free list
ساکن خالی *free_list_نکالو(خالی **list) {
  خالی *بلاک = *list;
  *list = *((خالی **)بلاک);
  واپس بلاک;
}

//! Allocate a small/medium sized memory block from the given heap
ساکن خالی *rpmalloc_allocate_from_ڈھیر_fallback(
    ڈھیر_t *ڈھیر_2, ڈھیر_سائز_class_t *ڈھیر_سائز_class, uint32_t class_idx) {
  وسعت_t *وسعت = ڈھیر_سائز_class->partial_وسعت;
  rpmalloc_assume(ڈھیر_2 != 0);
  اگر (EXPECTED(وسعت != 0)) {
    rpmalloc_assert(وسعت->بلاک_گنتی ==
                        حافظہ_سائز_class[وسعت->سائز_class].بلاک_گنتی,
                    "Span block count corrupted");
    rpmalloc_assert(!rpmalloc_وسعت_is_fully_utilized(وسعت),
                    "Internal failure");
    خالی *بلاک;
    اگر (وسعت->free_list) {
      // Span local free list is not empty, swap to size class free list
      بلاک = free_list_نکالو(&وسعت->free_list);
      ڈھیر_سائز_class->free_list = وسعت->free_list;
      وسعت->free_list = 0;
    } ورنہ {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      خالی *بلاک_آغاز = اشارہ_گر_انحراف(
          وسعت, وسعت_سرخی_سائز +
                    ((size_t)وسعت->free_list_حد_بندی * وسعت->بلاک_سائز_2));
      وسعت->free_list_حد_بندی += free_list_partial_init(
          &ڈھیر_سائز_class->free_list, &بلاک,
          (خالی *)((uintptr_t)بلاک_آغاز & ~(حافظہ_صفحہ_سائز - 1)),
          بلاک_آغاز, وسعت->بلاک_گنتی - وسعت->free_list_حد_بندی,
          وسعت->بلاک_سائز_2);
    }
    rpmalloc_assert(وسعت->free_list_حد_بندی <= وسعت->بلاک_گنتی,
                    "Span block count corrupted");
    وسعت->مستعمل_گنتی = وسعت->free_list_حد_بندی;

    // Swap in deferred free list if present
    اگر (atomic_بھرو_ptr(&وسعت->free_list_deferred))
      rpmalloc_وسعت_نکالو_free_list_deferred(وسعت);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    اگر (!rpmalloc_وسعت_is_fully_utilized(وسعت))
      واپس بلاک;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_وسعت_double_link_list_نکالو_سر(&ڈھیر_سائز_class->partial_وسعت,
                                             وسعت);
#اگر RPMALLOC_پہلا_CLASS_HEAPS
    rpmalloc_وسعت_double_link_list_جوڑو(&ڈھیر_2->بھرا_وسعت[class_idx], وسعت);
#اگر_ختم
    ++ڈھیر_2->بھرا_وسعت_گنتی;
    واپس بلاک;
  }

  // Find a span in one of the cache levels
  وسعت = rpmalloc_ڈھیر_نکالو_new_وسعت(ڈھیر_2, ڈھیر_سائز_class, 1, class_idx);
  اگر (EXPECTED(وسعت != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    واپس rpmalloc_وسعت_initialize_new(ڈھیر_2, ڈھیر_سائز_class, وسعت,
                                         class_idx);
  }

  واپس 0;
}

//! Allocate a small sized memory block from the given heap
ساکن خالی *_rpmalloc_allocate_small(ڈھیر_t *ڈھیر_2, size_t سائز_2) {
  rpmalloc_assert(ڈھیر_2, "No thread heap");
  // Small sizes have unique size classes
  ثابت uint32_t class_idx =
      (uint32_t)((سائز_2 + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_سرکاؤ);
  ڈھیر_سائز_class_t *ڈھیر_سائز_class = ڈھیر_2->سائز_class + class_idx;
  _rpmalloc_stat_inc_alloc(ڈھیر_2, class_idx);
  اگر (EXPECTED(ڈھیر_سائز_class->free_list != 0))
    واپس free_list_نکالو(&ڈھیر_سائز_class->free_list);
  واپس rpmalloc_allocate_from_ڈھیر_fallback(ڈھیر_2, ڈھیر_سائز_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
ساکن خالی *_rpmalloc_allocate_medium(ڈھیر_t *ڈھیر_2, size_t سائز_2) {
  rpmalloc_assert(ڈھیر_2, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ثابت uint32_t بنیاد_idx =
      (uint32_t)(SMALL_CLASS_گنتی +
                 ((سائز_2 - (SMALL_سائز_حد_بندی + 1)) >> MEDIUM_GRANULARITY_سرکاؤ));
  ثابت uint32_t class_idx = حافظہ_سائز_class[بنیاد_idx].class_idx;
  ڈھیر_سائز_class_t *ڈھیر_سائز_class = ڈھیر_2->سائز_class + class_idx;
  _rpmalloc_stat_inc_alloc(ڈھیر_2, class_idx);
  اگر (EXPECTED(ڈھیر_سائز_class->free_list != 0))
    واپس free_list_نکالو(&ڈھیر_سائز_class->free_list);
  واپس rpmalloc_allocate_from_ڈھیر_fallback(ڈھیر_2, ڈھیر_سائز_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
ساکن خالی *_rpmalloc_allocate_large(ڈھیر_t *ڈھیر_2, size_t سائز_2) {
  rpmalloc_assert(ڈھیر_2, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  سائز_2 += وسعت_سرخی_سائز;
  size_t وسعت_گنتی = سائز_2 >> حافظہ_وسعت_سائز_سرکاؤ;
  اگر (سائز_2 & (حافظہ_وسعت_سائز - 1))
    ++وسعت_گنتی;

  // Find a span in one of the cache levels
  وسعت_t *وسعت =
      rpmalloc_ڈھیر_نکالو_new_وسعت(ڈھیر_2, 0, وسعت_گنتی, سائز_CLASS_LARGE);
  اگر (!وسعت)
    واپس وسعت;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(وسعت->وسعت_گنتی >= وسعت_گنتی, "Internal failure");
  وسعت->سائز_class = سائز_CLASS_LARGE;
  وسعت->ڈھیر_2 = ڈھیر_2;

#اگر RPMALLOC_پہلا_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_جوڑو(&ڈھیر_2->large_huge_وسعت, وسعت);
#اگر_ختم
  ++ڈھیر_2->بھرا_وسعت_گنتی;

  واپس اشارہ_گر_انحراف(وسعت, وسعت_سرخی_سائز);
}

//! Allocate a huge block by mapping memory pages directly
ساکن خالی *_rpmalloc_allocate_huge(ڈھیر_t *ڈھیر_2, size_t سائز_2) {
  rpmalloc_assert(ڈھیر_2, "No thread heap");
  rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, 0);
  سائز_2 += وسعت_سرخی_سائز;
  size_t عدد_صفحات = سائز_2 >> حافظہ_صفحہ_سائز_سرکاؤ;
  اگر (سائز_2 & (حافظہ_صفحہ_سائز - 1))
    ++عدد_صفحات;
  size_t align_انحراف = 0;
  وسعت_t *وسعت =
      (وسعت_t *)_rpmalloc_mmap(عدد_صفحات * حافظہ_صفحہ_سائز, &align_انحراف);
  اگر (!وسعت)
    واپس وسعت;

  // Store page count in span_count
  وسعت->سائز_class = سائز_CLASS_HUGE;
  وسعت->وسعت_گنتی = (uint32_t)عدد_صفحات;
  وسعت->align_انحراف = (uint32_t)align_انحراف;
  وسعت->ڈھیر_2 = ڈھیر_2;
  rpmalloc_stat_جوڑو_peak(&huge_صفحات_current, عدد_صفحات, huge_صفحات_peak);

#اگر RPMALLOC_پہلا_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_جوڑو(&ڈھیر_2->large_huge_وسعت, وسعت);
#اگر_ختم
  ++ڈھیر_2->بھرا_وسعت_گنتی;

  واپس اشارہ_گر_انحراف(وسعت, وسعت_سرخی_سائز);
}

//! Allocate a block of the given size
ساکن خالی *_rpmalloc_allocate(ڈھیر_t *ڈھیر_2, size_t سائز_2) {
  _rpmalloc_stat_add64(&تخصیص_شمار, 1);
  اگر (EXPECTED(سائز_2 <= SMALL_سائز_حد_بندی))
    واپس _rpmalloc_allocate_small(ڈھیر_2, سائز_2);
  ورنہ اگر (سائز_2 <= حافظہ_medium_سائز_حد_بندی)
    واپس _rpmalloc_allocate_medium(ڈھیر_2, سائز_2);
  ورنہ اگر (سائز_2 <= LARGE_سائز_حد_بندی)
    واپس _rpmalloc_allocate_large(ڈھیر_2, سائز_2);
  واپس _rpmalloc_allocate_huge(ڈھیر_2, سائز_2);
}

ساکن خالی *_rpmalloc_aligned_allocate(ڈھیر_t *ڈھیر_2, size_t alignment,
                                        size_t سائز_2) {
  اگر (alignment <= SMALL_GRANULARITY)
    واپس _rpmalloc_allocate(ڈھیر_2, سائز_2);

#اگر ENABLE_VALIDATE_دلائل
  اگر ((سائز_2 + alignment) < سائز_2) {
    errno = EINVAL;
    واپس 0;
  }
  اگر (alignment & (alignment - 1)) {
    errno = EINVAL;
    واپس 0;
  }
#اگر_ختم

  اگر ((alignment <= وسعت_سرخی_سائز) &&
      ((سائز_2 + وسعت_سرخی_سائز) < حافظہ_medium_سائز_حد_بندی)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t مضرب_سائز = سائز_2 ? (سائز_2 + (وسعت_سرخی_سائز - 1)) &
                                      ~(uintptr_t)(وسعت_سرخی_سائز - 1)
                                : وسعت_سرخی_سائز;
    rpmalloc_assert(!(مضرب_سائز % وسعت_سرخی_سائز),
                    "Failed alignment calculation");
    اگر (مضرب_سائز <= (سائز_2 + alignment))
      واپس _rpmalloc_allocate(ڈھیر_2, مضرب_سائز);
  }

  خالی *ptr = 0;
  size_t align_نقاب = alignment - 1;
  اگر (alignment <= حافظہ_صفحہ_سائز) {
    ptr = _rpmalloc_allocate(ڈھیر_2, سائز_2 + alignment);
    اگر ((uintptr_t)ptr & align_نقاب) {
      ptr = (خالی *)(((uintptr_t)ptr & ~(uintptr_t)align_نقاب) + alignment);
      // Mark as having aligned blocks
      وسعت_t *وسعت = (وسعت_t *)((uintptr_t)ptr & حافظہ_وسعت_نقاب);
      وسعت->flags |= وسعت_FLAG_ALIGNED_بلاکیں;
    }
    واپس ptr;
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
  اگر (alignment & align_نقاب) {
    errno = EINVAL;
    واپس 0;
  }
  اگر (alignment >= حافظہ_وسعت_سائز) {
    errno = EINVAL;
    واپس 0;
  }

  size_t extra_صفحات = alignment / حافظہ_صفحہ_سائز;

  // Since each span has a header, we will at least need one extra memory page
  size_t عدد_صفحات = 1 + (سائز_2 / حافظہ_صفحہ_سائز);
  اگر (سائز_2 & (حافظہ_صفحہ_سائز - 1))
    ++عدد_صفحات;

  اگر (extra_صفحات > عدد_صفحات)
    عدد_صفحات = 1 + extra_صفحات;

  size_t original_صفحات = عدد_صفحات;
  size_t حد_بندی_صفحات = (حافظہ_وسعت_سائز / حافظہ_صفحہ_سائز) * 2;
  اگر (حد_بندی_صفحات < (original_صفحات * 2))
    حد_بندی_صفحات = original_صفحات * 2;

  size_t mapped_سائز, align_انحراف;
  وسعت_t *وسعت;

retry:
  align_انحراف = 0;
  mapped_سائز = عدد_صفحات * حافظہ_صفحہ_سائز;

  وسعت = (وسعت_t *)_rpmalloc_mmap(mapped_سائز, &align_انحراف);
  اگر (!وسعت) {
    errno = ENOMEM;
    واپس 0;
  }
  ptr = اشارہ_گر_انحراف(وسعت, وسعت_سرخی_سائز);

  اگر ((uintptr_t)ptr & align_نقاب)
    ptr = (خالی *)(((uintptr_t)ptr & ~(uintptr_t)align_نقاب) + alignment);

  اگر (((size_t)اشارہ_گر_diff(ptr, وسعت) >= حافظہ_وسعت_سائز) ||
      (اشارہ_گر_انحراف(ptr, سائز_2) > اشارہ_گر_انحراف(وسعت, mapped_سائز)) ||
      (((uintptr_t)ptr & حافظہ_وسعت_نقاب) != (uintptr_t)وسعت)) {
    _rpmalloc_unmap(وسعت, mapped_سائز, align_انحراف, mapped_سائز);
    ++عدد_صفحات;
    اگر (عدد_صفحات > حد_بندی_صفحات) {
      errno = EINVAL;
      واپس 0;
    }
    جاؤ retry;
  }

  // Store page count in span_count
  وسعت->سائز_class = سائز_CLASS_HUGE;
  وسعت->وسعت_گنتی = (uint32_t)عدد_صفحات;
  وسعت->align_انحراف = (uint32_t)align_انحراف;
  وسعت->ڈھیر_2 = ڈھیر_2;
  rpmalloc_stat_جوڑو_peak(&huge_صفحات_current, عدد_صفحات, huge_صفحات_peak);

#اگر RPMALLOC_پہلا_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_جوڑو(&ڈھیر_2->large_huge_وسعت, وسعت);
#اگر_ختم
  ++ڈھیر_2->بھرا_وسعت_گنتی;

  _rpmalloc_stat_add64(&تخصیص_شمار, 1);

  واپس ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
ساکن خالی rpmalloc_deallocate_براہ_راست_small_or_medium(وسعت_t *وسعت,
                                                        خالی *بلاک) {
  ڈھیر_t *ڈھیر_2 = وسعت->ڈھیر_2;
  rpmalloc_assert(ڈھیر_2->owner_thread == get_thread_id() ||
                      !ڈھیر_2->owner_thread || ڈھیر_2->finalize,
                  "Internal failure");
  // Add block to free list
  اگر (UNEXPECTED(rpmalloc_وسعت_is_fully_utilized(وسعت))) {
    وسعت->مستعمل_گنتی = وسعت->بلاک_گنتی;
#اگر RPMALLOC_پہلا_CLASS_HEAPS
    rpmalloc_وسعت_double_link_list_remove(&ڈھیر_2->بھرا_وسعت[وسعت->سائز_class],
                                           وسعت);
#اگر_ختم
    rpmalloc_وسعت_double_link_list_جوڑو(
        &ڈھیر_2->سائز_class[وسعت->سائز_class].partial_وسعت, وسعت);
    --ڈھیر_2->بھرا_وسعت_گنتی;
  }
  *((خالی **)بلاک) = وسعت->free_list;
  --وسعت->مستعمل_گنتی;
  وسعت->free_list = بلاک;
  اگر (UNEXPECTED(وسعت->مستعمل_گنتی == وسعت->list_سائز)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    اگر (وسعت->مستعمل_گنتی) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      خالی *free_list;
      کرو {
        free_list = atomic_exchange_ptr_acquire(&وسعت->free_list_deferred,
                                                INVALID_اشارہ_گر);
      } جب (free_list == INVALID_اشارہ_گر);
      atomic_store_ptr_release(&وسعت->free_list_deferred, free_list);
    }
    rpmalloc_وسعت_double_link_list_remove(
        &ڈھیر_2->سائز_class[وسعت->سائز_class].partial_وسعت, وسعت);
    rpmalloc_وسعت_release_مقصد_cache(ڈھیر_2, وسعت);
  }
}

ساکن خالی rpmalloc_deallocate_defer_free_وسعت(ڈھیر_t *ڈھیر_2, وسعت_t *وسعت) {
  اگر (وسعت->سائز_class != سائز_CLASS_HUGE)
    _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[وسعت->وسعت_گنتی - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  کرو {
    وسعت->free_list = (خالی *)atomic_بھرو_ptr(&ڈھیر_2->وسعت_free_deferred);
  } جب (!atomic_cas_ptr(&ڈھیر_2->وسعت_free_deferred, وسعت, وسعت->free_list));
}

//! Put the block in the deferred free list of the owning span
ساکن خالی _rpmalloc_deallocate_defer_small_or_medium(وسعت_t *وسعت,
                                                       خالی *بلاک) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  خالی *free_list;
  کرو {
    free_list =
        atomic_exchange_ptr_acquire(&وسعت->free_list_deferred, INVALID_اشارہ_گر);
  } جب (free_list == INVALID_اشارہ_گر);
  *((خالی **)بلاک) = free_list;
  uint32_t free_گنتی = ++وسعت->list_سائز;
  صحیح all_deferred_free = (free_گنتی == وسعت->بلاک_گنتی);
  atomic_store_ptr_release(&وسعت->free_list_deferred, بلاک);
  اگر (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    rpmalloc_deallocate_defer_free_وسعت(وسعت->ڈھیر_2, وسعت);
  }
}

ساکن خالی _rpmalloc_deallocate_small_or_medium(وسعت_t *وسعت, خالی *p) {
  _rpmalloc_stat_inc_free(وسعت->ڈھیر_2, وسعت->سائز_class);
  اگر (وسعت->flags & وسعت_FLAG_ALIGNED_بلاکیں) {
    // Realign pointer to block start
    خالی *بلاکیں_آغاز = اشارہ_گر_انحراف(وسعت, وسعت_سرخی_سائز);
    uint32_t بلاک_انحراف = (uint32_t)اشارہ_گر_diff(p, بلاکیں_آغاز);
    p = اشارہ_گر_انحراف(p, -(int32_t)(بلاک_انحراف % وسعت->بلاک_سائز_2));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#اگر RPMALLOC_پہلا_CLASS_HEAPS
  صحیح defer =
      (وسعت->ڈھیر_2->owner_thread &&
       (وسعت->ڈھیر_2->owner_thread != get_thread_id()) && !وسعت->ڈھیر_2->finalize);
#ورنہ
  صحیح defer =
      ((وسعت->ڈھیر_2->owner_thread != get_thread_id()) && !وسعت->ڈھیر_2->finalize);
#اگر_ختم
  اگر (!defer)
    rpmalloc_deallocate_براہ_راست_small_or_medium(وسعت, p);
  ورنہ
    _rpmalloc_deallocate_defer_small_or_medium(وسعت, p);
}

//! Deallocate the given large memory block to the current heap
ساکن خالی _rpmalloc_deallocate_large(وسعت_t *وسعت) {
  rpmalloc_assert(وسعت->سائز_class == سائز_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(وسعت->flags & وسعت_FLAG_MASTER) ||
                      !(وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((وسعت->flags & وسعت_FLAG_MASTER) ||
                      (وسعت->flags & وسعت_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#اگر RPMALLOC_پہلا_CLASS_HEAPS
  صحیح defer =
      (وسعت->ڈھیر_2->owner_thread &&
       (وسعت->ڈھیر_2->owner_thread != get_thread_id()) && !وسعت->ڈھیر_2->finalize);
#ورنہ
  صحیح defer =
      ((وسعت->ڈھیر_2->owner_thread != get_thread_id()) && !وسعت->ڈھیر_2->finalize);
#اگر_ختم
  اگر (defer) {
    rpmalloc_deallocate_defer_free_وسعت(وسعت->ڈھیر_2, وسعت);
    واپس;
  }
  rpmalloc_assert(وسعت->ڈھیر_2->بھرا_وسعت_گنتی, "Heap span counter corrupted");
  --وسعت->ڈھیر_2->بھرا_وسعت_گنتی;
#اگر RPMALLOC_پہلا_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_remove(&وسعت->ڈھیر_2->large_huge_وسعت, وسعت);
#اگر_ختم
#اگر ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = وسعت->وسعت_گنتی - 1;
  atomic_decr32(&وسعت->ڈھیر_2->وسعت_use[idx].current);
#اگر_ختم
  ڈھیر_t *ڈھیر_2 = وسعت->ڈھیر_2;
  rpmalloc_assert(ڈھیر_2, "No thread heap");
#اگر ENABLE_THREAD_CACHE
  ثابت صحیح سیٹ_as_reserved =
      ((وسعت->وسعت_گنتی > 1) && (ڈھیر_2->وسعت_cache.گنتی == 0) &&
       !ڈھیر_2->finalize && !ڈھیر_2->spans_reserved);
#ورنہ
  ثابت صحیح سیٹ_as_reserved =
      ((وسعت->وسعت_گنتی > 1) && !ڈھیر_2->finalize && !ڈھیر_2->spans_reserved);
#اگر_ختم
  اگر (سیٹ_as_reserved) {
    ڈھیر_2->وسعت_reserve = وسعت;
    ڈھیر_2->spans_reserved = وسعت->وسعت_گنتی;
    اگر (وسعت->flags & وسعت_FLAG_MASTER) {
      ڈھیر_2->وسعت_reserve_master = وسعت;
    } ورنہ { // SPAN_FLAG_SUBSPAN
      وسعت_t *master = (وسعت_t *)اشارہ_گر_انحراف(
          وسعت,
          -(intptr_t)((size_t)وسعت->انحراف_from_master * حافظہ_وسعت_سائز));
      ڈھیر_2->وسعت_reserve_master = master;
      rpmalloc_assert(master->flags & وسعت_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->بقایا_spans) >=
                          (int32_t)وسعت->وسعت_گنتی,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&ڈھیر_2->وسعت_use[idx].spans_مقصد_reserved);
  } ورنہ {
    // Insert into cache list
    rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
  }
}

//! Deallocate the given huge span
ساکن خالی _rpmalloc_deallocate_huge(وسعت_t *وسعت) {
  rpmalloc_assert(وسعت->ڈھیر_2, "No span heap");
#اگر RPMALLOC_پہلا_CLASS_HEAPS
  صحیح defer =
      (وسعت->ڈھیر_2->owner_thread &&
       (وسعت->ڈھیر_2->owner_thread != get_thread_id()) && !وسعت->ڈھیر_2->finalize);
#ورنہ
  صحیح defer =
      ((وسعت->ڈھیر_2->owner_thread != get_thread_id()) && !وسعت->ڈھیر_2->finalize);
#اگر_ختم
  اگر (defer) {
    rpmalloc_deallocate_defer_free_وسعت(وسعت->ڈھیر_2, وسعت);
    واپس;
  }
  rpmalloc_assert(وسعت->ڈھیر_2->بھرا_وسعت_گنتی, "Heap span counter corrupted");
  --وسعت->ڈھیر_2->بھرا_وسعت_گنتی;
#اگر RPMALLOC_پہلا_CLASS_HEAPS
  rpmalloc_وسعت_double_link_list_remove(&وسعت->ڈھیر_2->large_huge_وسعت, وسعت);
#اگر_ختم

  // Oversized allocation, page count is stored in span_count
  size_t عدد_صفحات = وسعت->وسعت_گنتی;
  _rpmalloc_unmap(وسعت, عدد_صفحات * حافظہ_صفحہ_سائز, وسعت->align_انحراف,
                  عدد_صفحات * حافظہ_صفحہ_سائز);
  _rpmalloc_stat_sub(&huge_صفحات_current, عدد_صفحات);
}

//! Deallocate the given block
ساکن خالی _rpmalloc_deallocate(خالی *p) {
  _rpmalloc_stat_add64(&deallocation_شمار, 1);
  // Grab the span (always at start of span, using span alignment)
  وسعت_t *وسعت = (وسعت_t *)((uintptr_t)p & حافظہ_وسعت_نقاب);
  اگر (UNEXPECTED(!وسعت))
    واپس;
  اگر (EXPECTED(وسعت->سائز_class < سائز_CLASS_گنتی))
    _rpmalloc_deallocate_small_or_medium(وسعت, p);
  ورنہ اگر (وسعت->سائز_class == سائز_CLASS_LARGE)
    _rpmalloc_deallocate_large(وسعت);
  ورنہ
    _rpmalloc_deallocate_huge(وسعت);
}

////////////
///
/// Reallocation entry points
///
//////

ساکن size_t rpmalloc_usable_سائز_2(خالی *p);

//! Reallocate the given block to the given size
ساکن خالی *_rpmalloc_reallocate(ڈھیر_t *ڈھیر_2, خالی *p, size_t سائز_2,
                                  size_t oldsize, غیرعلامتی صحیح flags) {
  اگر (p) {
    // Grab the span using guaranteed span alignment
    وسعت_t *وسعت = (وسعت_t *)((uintptr_t)p & حافظہ_وسعت_نقاب);
    اگر (EXPECTED(وسعت->سائز_class < سائز_CLASS_گنتی)) {
      // Small/medium sized block
      rpmalloc_assert(وسعت->وسعت_گنتی == 1, "Span counter corrupted");
      خالی *بلاکیں_آغاز = اشارہ_گر_انحراف(وسعت, وسعت_سرخی_سائز);
      uint32_t بلاک_انحراف = (uint32_t)اشارہ_گر_diff(p, بلاکیں_آغاز);
      uint32_t بلاک_idx = بلاک_انحراف / وسعت->بلاک_سائز_2;
      خالی *بلاک =
          اشارہ_گر_انحراف(بلاکیں_آغاز, (size_t)بلاک_idx * وسعت->بلاک_سائز_2);
      اگر (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)وسعت->بلاک_سائز_2 - اشارہ_گر_diff(p, بلاک));
      اگر ((size_t)وسعت->بلاک_سائز_2 >= سائز_2) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        اگر ((p != بلاک) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلاک, p, oldsize);
        واپس بلاک;
      }
    } ورنہ اگر (وسعت->سائز_class == سائز_CLASS_LARGE) {
      // Large block
      size_t کل_سائز_2 = سائز_2 + وسعت_سرخی_سائز;
      size_t عدد_spans = کل_سائز_2 >> حافظہ_وسعت_سائز_سرکاؤ;
      اگر (کل_سائز_2 & (حافظہ_وسعت_نقاب - 1))
        ++عدد_spans;
      size_t current_spans = وسعت->وسعت_گنتی;
      خالی *بلاک = اشارہ_گر_انحراف(وسعت, وسعت_سرخی_سائز);
      اگر (!oldsize)
        oldsize = (current_spans * حافظہ_وسعت_سائز) -
                  (size_t)اشارہ_گر_diff(p, بلاک) - وسعت_سرخی_سائز;
      اگر ((current_spans >= عدد_spans) && (کل_سائز_2 >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        اگر ((p != بلاک) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلاک, p, oldsize);
        واپس بلاک;
      }
    } ورنہ {
      // Oversized block
      size_t کل_سائز_2 = سائز_2 + وسعت_سرخی_سائز;
      size_t عدد_صفحات = کل_سائز_2 >> حافظہ_صفحہ_سائز_سرکاؤ;
      اگر (کل_سائز_2 & (حافظہ_صفحہ_سائز - 1))
        ++عدد_صفحات;
      // Page count is stored in span_count
      size_t current_صفحات = وسعت->وسعت_گنتی;
      خالی *بلاک = اشارہ_گر_انحراف(وسعت, وسعت_سرخی_سائز);
      اگر (!oldsize)
        oldsize = (current_صفحات * حافظہ_صفحہ_سائز) -
                  (size_t)اشارہ_گر_diff(p, بلاک) - وسعت_سرخی_سائز;
      اگر ((current_صفحات >= عدد_صفحات) && (عدد_صفحات >= (current_صفحات / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        اگر ((p != بلاک) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلاک, p, oldsize);
        واپس بلاک;
      }
    }
  } ورنہ {
    oldsize = 0;
  }

  اگر (!!(flags & RPMALLOC_GROW_OR_FAIL))
    واپس 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t زیریں_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_سائز =
      (سائز_2 > زیریں_bound) ? سائز_2 : ((سائز_2 > oldsize) ? زیریں_bound : سائز_2);
  خالی *بلاک = _rpmalloc_allocate(ڈھیر_2, new_سائز);
  اگر (p && بلاک) {
    اگر (!(flags & RPMALLOC_NO_PRESERVE))
      حافظہ_نقل(بلاک, p, oldsize < new_سائز ? oldsize : new_سائز);
    _rpmalloc_deallocate(p);
  }

  واپس بلاک;
}

ساکن خالی *_rpmalloc_aligned_reallocate(ڈھیر_t *ڈھیر_2, خالی *ptr,
                                          size_t alignment, size_t سائز_2,
                                          size_t oldsize, غیرعلامتی صحیح flags) {
  اگر (alignment <= SMALL_GRANULARITY)
    واپس _rpmalloc_reallocate(ڈھیر_2, ptr, سائز_2, oldsize, flags);

  صحیح no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? rpmalloc_usable_سائز_2(ptr) : 0);
  اگر ((usablesize >= سائز_2) && !((uintptr_t)ptr & (alignment - 1))) {
    اگر (no_alloc || (سائز_2 >= (usablesize / 2)))
      واپس ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  خالی *بلاک =
      (!no_alloc ? _rpmalloc_aligned_allocate(ڈھیر_2, alignment, سائز_2) : 0);
  اگر (EXPECTED(بلاک != 0)) {
    اگر (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      اگر (!oldsize)
        oldsize = usablesize;
      حافظہ_نقل(بلاک, ptr, oldsize < سائز_2 ? oldsize : سائز_2);
    }
    _rpmalloc_deallocate(ptr);
  }
  واپس بلاک;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
ساکن size_t rpmalloc_usable_سائز_2(خالی *p) {
  // Grab the span using guaranteed span alignment
  وسعت_t *وسعت = (وسعت_t *)((uintptr_t)p & حافظہ_وسعت_نقاب);
  اگر (وسعت->سائز_class < سائز_CLASS_گنتی) {
    // Small/medium block
    خالی *بلاکیں_آغاز = اشارہ_گر_انحراف(وسعت, وسعت_سرخی_سائز);
    واپس وسعت->بلاک_سائز_2 -
           ((size_t)اشارہ_گر_diff(p, بلاکیں_آغاز) % وسعت->بلاک_سائز_2);
  }
  اگر (وسعت->سائز_class == سائز_CLASS_LARGE) {
    // Large block
    size_t current_spans = وسعت->وسعت_گنتی;
    واپس (current_spans * حافظہ_وسعت_سائز) - (size_t)اشارہ_گر_diff(p, وسعت);
  }
  // Oversized block, page count is stored in span_count
  size_t current_صفحات = وسعت->وسعت_گنتی;
  واپس (current_صفحات * حافظہ_صفحہ_سائز) - (size_t)اشارہ_گر_diff(p, وسعت);
}

//! Adjust and optimize the size class properties for the given class
ساکن خالی rpmalloc_adjust_سائز_class(size_t iclass) {
  size_t بلاک_سائز_2 = حافظہ_سائز_class[iclass].بلاک_سائز_2;
  size_t بلاک_گنتی = (حافظہ_وسعت_سائز - وسعت_سرخی_سائز) / بلاک_سائز_2;

  حافظہ_سائز_class[iclass].بلاک_گنتی = (uint16_t)بلاک_گنتی;
  حافظہ_سائز_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  اگر (iclass >= SMALL_CLASS_گنتی) {
    size_t prevclass = iclass;
    جب (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      اگر (حافظہ_سائز_class[prevclass].بلاک_گنتی ==
          حافظہ_سائز_class[iclass].بلاک_گنتی)
        _rpmalloc_memcpy_const(حافظہ_سائز_class + prevclass,
                               حافظہ_سائز_class + iclass,
                               حجم(حافظہ_سائز_class[iclass]));
      ورنہ
        توڑو;
    }
  }
}

//! Initialize the allocator and setup global data
خارجی اندرونی صحیح rpmalloc_initialize(خالی) {
  اگر (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    واپس 0;
  }
  واپس rpmalloc_initialize_config(0);
}

صحیح rpmalloc_initialize_config(ثابت rpmalloc_config_t *config) {
  اگر (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    واپس 0;
  }
  _rpmalloc_initialized = 1;

  اگر (config)
    حافظہ_نقل(&حافظہ_config, config, حجم(rpmalloc_config_t));
  ورنہ
    _rpmalloc_memset_const(&حافظہ_config, 0, حجم(rpmalloc_config_t));

  اگر (!حافظہ_config.حافظہ_map || !حافظہ_config.حافظہ_unmap) {
    حافظہ_config.حافظہ_map = _rpmalloc_mmap_os;
    حافظہ_config.حافظہ_unmap = _rpmalloc_unmap_os;
  }

#اگر PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, حجم(system_info));
  GetSystemInfo(&system_info);
  حافظہ_map_granularity = system_info.dw_تخصیص_Granularity;
#ورنہ
  حافظہ_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#اگر_ختم

#اگر RPMALLOC_CONFIGURABLE
  حافظہ_صفحہ_سائز = حافظہ_config.صفحہ_سائز_2;
#ورنہ
  حافظہ_صفحہ_سائز = 0;
#اگر_ختم
  حافظہ_huge_صفحات = 0;
  اگر (!حافظہ_صفحہ_سائز) {
#اگر PLATFORM_WINDOWS
    حافظہ_صفحہ_سائز = system_info.dw_صفحہ_سائز;
#ورنہ
    حافظہ_صفحہ_سائز = حافظہ_map_granularity;
    اگر (حافظہ_config.enable_huge_صفحات) {
#اگر defined(__linux__)
      size_t huge_صفحہ_سائز = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      اگر (meminfo) {
        حرف سطر_2[128];
        جب (!huge_صفحہ_سائز && fgets(سطر_2, حجم(سطر_2) - 1, meminfo)) {
          سطر_2[حجم(سطر_2) - 1] = 0;
          اگر (لڑی_تلاش(سطر_2, "Hugepagesize:"))
            huge_صفحہ_سائز = (size_t)strtol(سطر_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      اگر (huge_صفحہ_سائز) {
        حافظہ_huge_صفحات = 1;
        حافظہ_صفحہ_سائز = huge_صفحہ_سائز;
        حافظہ_map_granularity = huge_صفحہ_سائز;
      }
#وگرنہ_اگر defined(__FreeBSD__)
      صحیح rc;
      size_t sz = حجم(rc);

      اگر (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, تہی, 0) == 0 &&
          rc == 1) {
        ساکن size_t defsize = 2 * 1024 * 1024;
        صحیح nsize = 0;
        size_t sizes[4] = {0};
        حافظہ_huge_صفحات = 1;
        حافظہ_صفحہ_سائز = defsize;
        اگر ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          برائے (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            اگر (defsize < csize) {
              حافظہ_صفحہ_سائز = csize;
              توڑو;
            }
          }
        }
        حافظہ_map_granularity = حافظہ_صفحہ_سائز;
      }
#وگرنہ_اگر defined(__APPLE__) || defined(__NetBSD__)
      حافظہ_huge_صفحات = 1;
      حافظہ_صفحہ_سائز = 2 * 1024 * 1024;
      حافظہ_map_granularity = حافظہ_صفحہ_سائز;
#اگر_ختم
    }
#اگر_ختم
  } ورنہ {
    اگر (حافظہ_config.enable_huge_صفحات)
      حافظہ_huge_صفحات = 1;
  }

#اگر PLATFORM_WINDOWS
  اگر (حافظہ_config.enable_huge_صفحات) {
    HANDLE token = 0;
    size_t large_صفحہ_minimum = Get_Large_صفحہ_Minimum();
    اگر (large_صفحہ_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    اگر (token) {
      LUID luid;
      اگر (Lookup_Privilege_قدر(0, SE_تالا_حافظہ_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, حجم(token_privileges));
        token_privileges.Privilege_گنتی = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].اوصاف = SE_PRIVILEGE_ENABLED;
        اگر (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          اگر (Get_آخری_Error() == ERROR_SUCCESS)
            حافظہ_huge_صفحات = 1;
        }
      }
      CloseHandle(token);
    }
    اگر (حافظہ_huge_صفحات) {
      اگر (large_صفحہ_minimum > حافظہ_صفحہ_سائز)
        حافظہ_صفحہ_سائز = large_صفحہ_minimum;
      اگر (large_صفحہ_minimum > حافظہ_map_granularity)
        حافظہ_map_granularity = large_صفحہ_minimum;
    }
  }
#اگر_ختم

  size_t min_وسعت_سائز = 256;
  size_t max_صفحہ_سائز;
#اگر UINTPTR_MAX > 0xFFFFFFFF
  max_صفحہ_سائز = 4096ULL * 1024ULL * 1024ULL;
#ورنہ
  max_صفحہ_سائز = 4 * 1024 * 1024;
#اگر_ختم
  اگر (حافظہ_صفحہ_سائز < min_وسعت_سائز)
    حافظہ_صفحہ_سائز = min_وسعت_سائز;
  اگر (حافظہ_صفحہ_سائز > max_صفحہ_سائز)
    حافظہ_صفحہ_سائز = max_صفحہ_سائز;
  حافظہ_صفحہ_سائز_سرکاؤ = 0;
  size_t صفحہ_سائز_ثنائی_ہندسہ = حافظہ_صفحہ_سائز;
  جب (صفحہ_سائز_ثنائی_ہندسہ != 1) {
    ++حافظہ_صفحہ_سائز_سرکاؤ;
    صفحہ_سائز_ثنائی_ہندسہ >>= 1;
  }
  حافظہ_صفحہ_سائز = ((size_t)1 << حافظہ_صفحہ_سائز_سرکاؤ);

#اگر RPMALLOC_CONFIGURABLE
  اگر (!حافظہ_config.وسعت_سائز) {
    حافظہ_وسعت_سائز = حافظہ_default_وسعت_سائز;
    حافظہ_وسعت_سائز_سرکاؤ = حافظہ_default_وسعت_سائز_سرکاؤ;
    حافظہ_وسعت_نقاب = حافظہ_default_وسعت_نقاب;
  } ورنہ {
    size_t وسعت_سائز = حافظہ_config.وسعت_سائز;
    اگر (وسعت_سائز > (256 * 1024))
      وسعت_سائز = (256 * 1024);
    حافظہ_وسعت_سائز = 4096;
    حافظہ_وسعت_سائز_سرکاؤ = 12;
    جب (حافظہ_وسعت_سائز < وسعت_سائز) {
      حافظہ_وسعت_سائز <<= 1;
      ++حافظہ_وسعت_سائز_سرکاؤ;
    }
    حافظہ_وسعت_نقاب = ~(uintptr_t)(حافظہ_وسعت_سائز - 1);
  }
#اگر_ختم

  حافظہ_وسعت_map_گنتی =
      (حافظہ_config.وسعت_map_گنتی ? حافظہ_config.وسعت_map_گنتی
                                     : DEFAULT_وسعت_MAP_گنتی);
  اگر ((حافظہ_وسعت_سائز * حافظہ_وسعت_map_گنتی) < حافظہ_صفحہ_سائز)
    حافظہ_وسعت_map_گنتی = (حافظہ_صفحہ_سائز / حافظہ_وسعت_سائز);
  اگر ((حافظہ_صفحہ_سائز >= حافظہ_وسعت_سائز) &&
      ((حافظہ_وسعت_map_گنتی * حافظہ_وسعت_سائز) % حافظہ_صفحہ_سائز))
    حافظہ_وسعت_map_گنتی = (حافظہ_صفحہ_سائز / حافظہ_وسعت_سائز);
  حافظہ_ڈھیر_reserve_گنتی = (حافظہ_وسعت_map_گنتی > DEFAULT_وسعت_MAP_گنتی)
                                   ? DEFAULT_وسعت_MAP_گنتی
                                   : حافظہ_وسعت_map_گنتی;

  حافظہ_config.صفحہ_سائز_2 = حافظہ_صفحہ_سائز;
  حافظہ_config.وسعت_سائز = حافظہ_وسعت_سائز;
  حافظہ_config.وسعت_map_گنتی = حافظہ_وسعت_map_گنتی;
  حافظہ_config.enable_huge_صفحات = حافظہ_huge_صفحات;

#اگر ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  اگر (pthread_کلید_create(&حافظہ_thread_ڈھیر, rpmalloc_ڈھیر_release_raw_fc))
    واپس -1;
#اگر_ختم
#اگر defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_کلید = FlsAlloc(&_rpmalloc_thread_destructor);
#اگر_ختم

  // Setup all small and medium size classes
  size_t iclass = 0;
  حافظہ_سائز_class[iclass].بلاک_سائز_2 = SMALL_GRANULARITY;
  rpmalloc_adjust_سائز_class(iclass);
  برائے (iclass = 1; iclass < SMALL_CLASS_گنتی; ++iclass) {
    size_t سائز_2 = iclass * SMALL_GRANULARITY;
    حافظہ_سائز_class[iclass].بلاک_سائز_2 = (uint32_t)سائز_2;
    rpmalloc_adjust_سائز_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  حافظہ_medium_سائز_حد_بندی = (حافظہ_وسعت_سائز - وسعت_سرخی_سائز) >> 1;
  اگر (حافظہ_medium_سائز_حد_بندی > MEDIUM_سائز_حد_بندی)
    حافظہ_medium_سائز_حد_بندی = MEDIUM_سائز_حد_بندی;
  برائے (iclass = 0; iclass < MEDIUM_CLASS_گنتی; ++iclass) {
    size_t سائز_2 = SMALL_سائز_حد_بندی + ((iclass + 1) * MEDIUM_GRANULARITY);
    اگر (سائز_2 > حافظہ_medium_سائز_حد_بندی) {
      حافظہ_medium_سائز_حد_بندی =
          SMALL_سائز_حد_بندی + (iclass * MEDIUM_GRANULARITY);
      توڑو;
    }
    حافظہ_سائز_class[SMALL_CLASS_گنتی + iclass].بلاک_سائز_2 = (uint32_t)سائز_2;
    rpmalloc_adjust_سائز_class(SMALL_CLASS_گنتی + iclass);
  }

  حافظہ_orphan_heaps = 0;
#اگر RPMALLOC_پہلا_CLASS_HEAPS
  حافظہ_پہلا_class_orphan_heaps = 0;
#اگر_ختم
#اگر ENABLE_STATISTICS
  atomic_store32(&حافظہ_فعال_heaps, 0);
  atomic_store32(&mapped_صفحات, 0);
  mapped_صفحات_peak = 0;
  atomic_store32(&_master_spans, 0);
  atomic_store32(&mapped_کل, 0);
  atomic_store32(&unmapped_کل, 0);
  atomic_store32(&mapped_صفحات_os, 0);
  atomic_store32(&huge_صفحات_current, 0);
  huge_صفحات_peak = 0;
#اگر_ختم
  memset(حافظہ_heaps, 0, حجم(حافظہ_heaps));
  atomic_store32_release(&حافظہ_global_تالا, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  واپس 0;
}

//! Finalize the allocator
خالی rpmalloc_finalize(خالی) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  اگر (حافظہ_global_reserve) {
    atomic_add32(&حافظہ_global_reserve_master->بقایا_spans,
                 -(int32_t)حافظہ_global_reserve_گنتی);
    حافظہ_global_reserve_master = 0;
    حافظہ_global_reserve_گنتی = 0;
    حافظہ_global_reserve = 0;
  }
  atomic_store32_release(&حافظہ_global_تالا, 0);

  // Free all thread caches and fully free spans
  برائے (size_t list_idx = 0; list_idx < ڈھیر_ARRAY_سائز; ++list_idx) {
    ڈھیر_t *ڈھیر_2 = حافظہ_heaps[list_idx];
    جب (ڈھیر_2) {
      ڈھیر_t *اگلا_ڈھیر = ڈھیر_2->اگلا_ڈھیر;
      ڈھیر_2->finalize = 1;
      rpmalloc_ڈھیر_global_finalize(ڈھیر_2);
      ڈھیر_2 = اگلا_ڈھیر;
    }
  }

#اگر ENABLE_GLOBAL_CACHE
  // Free global caches
  برائے (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass)
    _rpmalloc_global_cache_finalize(&حافظہ_وسعت_cache[iclass]);
#اگر_ختم

#اگر (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_کلید_delete(حافظہ_thread_ڈھیر);
#اگر_ختم
#اگر defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_کلید);
  fls_کلید = 0;
#اگر_ختم
#اگر ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&mapped_صفحات) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&mapped_صفحات_os) == 0,
                  "Memory leak detected");
#اگر_ختم

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
خارجی اندرونی خالی rpmalloc_thread_initialize(خالی) {
  اگر (!get_thread_ڈھیر_raw()) {
    ڈھیر_t *ڈھیر_2 = rpmalloc_ڈھیر_allocate(0);
    اگر (ڈھیر_2) {
      _rpmalloc_stat_inc(&حافظہ_فعال_heaps);
      سیٹ_thread_ڈھیر(ڈھیر_2);
#اگر defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_سیٹ_قدر(fls_کلید, ڈھیر_2);
#اگر_ختم
    }
  }
}

//! Finalize thread, orphan heap
خالی rpmalloc_thread_finalize(صحیح release_caches) {
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر_raw();
  اگر (ڈھیر_2)
    rpmalloc_ڈھیر_release_raw(ڈھیر_2, release_caches);
  سیٹ_thread_ڈھیر(0);
#اگر defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_سیٹ_قدر(fls_کلید, 0);
#اگر_ختم
}

صحیح rpmalloc_is_thread_initialized(خالی) {
  واپس (get_thread_ڈھیر_raw() != 0) ? 1 : 0;
}

ثابت rpmalloc_config_t *rpmalloc_config(خالی) { واپس &حافظہ_config; }

// Extern interface

خارجی اندرونی RPMALLOC_ALLOCATOR خالی *rpmalloc(size_t سائز_2) {
#اگر ENABLE_VALIDATE_دلائل
  اگر (سائز_2 >= MAX_ALLOC_سائز) {
    errno = EINVAL;
    واپس 0;
  }
#اگر_ختم
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر();
  واپس _rpmalloc_allocate(ڈھیر_2, سائز_2);
}

خارجی اندرونی خالی rpfree(خالی *ptr) { _rpmalloc_deallocate(ptr); }

خارجی اندرونی RPMALLOC_ALLOCATOR خالی *rpcalloc(size_t عدد_2, size_t سائز_2) {
  size_t کل_2;
#اگر ENABLE_VALIDATE_دلائل
#اگر PLATFORM_WINDOWS
  صحیح err = سائز_TMult(عدد_2, سائز_2, &کل_2);
  اگر ((err != S_OK) || (کل_2 >= MAX_ALLOC_سائز)) {
    errno = EINVAL;
    واپس 0;
  }
#ورنہ
  صحیح err = __builtin_umull_overflow(عدد_2, سائز_2, &کل_2);
  اگر (err || (کل_2 >= MAX_ALLOC_سائز)) {
    errno = EINVAL;
    واپس 0;
  }
#اگر_ختم
#ورنہ
  کل_2 = عدد_2 * سائز_2;
#اگر_ختم
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر();
  خالی *بلاک = _rpmalloc_allocate(ڈھیر_2, کل_2);
  اگر (بلاک)
    memset(بلاک, 0, کل_2);
  واپس بلاک;
}

خارجی اندرونی RPMALLOC_ALLOCATOR خالی *rprealloc(خالی *ptr, size_t سائز_2) {
#اگر ENABLE_VALIDATE_دلائل
  اگر (سائز_2 >= MAX_ALLOC_سائز) {
    errno = EINVAL;
    واپس ptr;
  }
#اگر_ختم
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر();
  واپس _rpmalloc_reallocate(ڈھیر_2, ptr, سائز_2, 0, 0);
}

خارجی RPMALLOC_ALLOCATOR خالی *rpaligned_realloc(خالی *ptr, size_t alignment,
                                                  size_t سائز_2, size_t oldsize,
                                                  غیرعلامتی صحیح flags) {
#اگر ENABLE_VALIDATE_دلائل
  اگر ((سائز_2 + alignment < سائز_2) || (alignment > حافظہ_صفحہ_سائز)) {
    errno = EINVAL;
    واپس 0;
  }
#اگر_ختم
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر();
  واپس _rpmalloc_aligned_reallocate(ڈھیر_2, ptr, alignment, سائز_2, oldsize,
                                      flags);
}

خارجی RPMALLOC_ALLOCATOR خالی *rpaligned_alloc(size_t alignment, size_t سائز_2) {
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر();
  واپس _rpmalloc_aligned_allocate(ڈھیر_2, alignment, سائز_2);
}

خارجی اندرونی RPMALLOC_ALLOCATOR خالی *
rpaligned_calloc(size_t alignment, size_t عدد_2, size_t سائز_2) {
  size_t کل_2;
#اگر ENABLE_VALIDATE_دلائل
#اگر PLATFORM_WINDOWS
  صحیح err = سائز_TMult(عدد_2, سائز_2, &کل_2);
  اگر ((err != S_OK) || (کل_2 >= MAX_ALLOC_سائز)) {
    errno = EINVAL;
    واپس 0;
  }
#ورنہ
  صحیح err = __builtin_umull_overflow(عدد_2, سائز_2, &کل_2);
  اگر (err || (کل_2 >= MAX_ALLOC_سائز)) {
    errno = EINVAL;
    واپس 0;
  }
#اگر_ختم
#ورنہ
  کل_2 = عدد_2 * سائز_2;
#اگر_ختم
  خالی *بلاک = rpaligned_alloc(alignment, کل_2);
  اگر (بلاک)
    memset(بلاک, 0, کل_2);
  واپس بلاک;
}

خارجی اندرونی RPMALLOC_ALLOCATOR خالی *rpmemalign(size_t alignment,
                                                  size_t سائز_2) {
  واپس rpaligned_alloc(alignment, سائز_2);
}

خارجی اندرونی صحیح rpposix_memalign(خالی **memptr, size_t alignment,
                                   size_t سائز_2) {
  اگر (memptr)
    *memptr = rpaligned_alloc(alignment, سائز_2);
  ورنہ
    واپس EINVAL;
  واپس *memptr ? 0 : ENOMEM;
}

خارجی اندرونی size_t rpmalloc_usable_سائز(خالی *ptr) {
  واپس (ptr ? rpmalloc_usable_سائز_2(ptr) : 0);
}

خارجی اندرونی خالی rpmalloc_thread_collect(خالی) {}

خالی rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, حجم(rpmalloc_thread_statistics_t));
  ڈھیر_t *ڈھیر_2 = get_thread_ڈھیر_raw();
  اگر (!ڈھیر_2)
    واپس;

  برائے (size_t iclass = 0; iclass < سائز_CLASS_گنتی; ++iclass) {
    سائز_class_t *سائز_class = حافظہ_سائز_class + iclass;
    وسعت_t *وسعت = ڈھیر_2->سائز_class[iclass].partial_وسعت;
    جب (وسعت) {
      size_t free_گنتی = وسعت->list_سائز;
      size_t بلاک_گنتی = سائز_class->بلاک_گنتی;
      اگر (وسعت->free_list_حد_بندی < بلاک_گنتی)
        بلاک_گنتی = وسعت->free_list_حد_بندی;
      free_گنتی += (بلاک_گنتی - وسعت->مستعمل_گنتی);
      stats->sizecache += free_گنتی * سائز_class->بلاک_سائز_2;
      وسعت = وسعت->اگلا;
    }
  }

#اگر ENABLE_THREAD_CACHE
  برائے (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    وسعت_cache_t *وسعت_cache;
    اگر (!iclass)
      وسعت_cache = &ڈھیر_2->وسعت_cache;
    ورنہ
      وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (iclass - 1));
    stats->spancache += وسعت_cache->گنتی * (iclass + 1) * حافظہ_وسعت_سائز;
  }
#اگر_ختم

  وسعت_t *deferred = (وسعت_t *)atomic_بھرو_ptr(&ڈھیر_2->وسعت_free_deferred);
  جب (deferred) {
    اگر (deferred->سائز_class != سائز_CLASS_HUGE)
      stats->spancache += (size_t)deferred->وسعت_گنتی * حافظہ_وسعت_سائز;
    deferred = (وسعت_t *)deferred->free_list;
  }

#اگر ENABLE_STATISTICS
  stats->thread_مقصد_global = (size_t)atomic_load64(&ڈھیر_2->thread_مقصد_global);
  stats->global_مقصد_thread = (size_t)atomic_load64(&ڈھیر_2->global_مقصد_thread);

  برائے (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    stats->وسعت_use[iclass].current =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].current);
    stats->وسعت_use[iclass].peak =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].بلند);
    stats->وسعت_use[iclass].مقصد_global =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_مقصد_global);
    stats->وسعت_use[iclass].from_global =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_from_global);
    stats->وسعت_use[iclass].مقصد_cache =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_مقصد_cache);
    stats->وسعت_use[iclass].from_cache =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_from_cache);
    stats->وسعت_use[iclass].مقصد_reserved =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_مقصد_reserved);
    stats->وسعت_use[iclass].from_reserved =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_from_reserved);
    stats->وسعت_use[iclass].map_calls =
        (size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_map_calls);
  }
  برائے (size_t iclass = 0; iclass < سائز_CLASS_گنتی; ++iclass) {
    stats->سائز_use[iclass].alloc_current =
        (size_t)atomic_load32(&ڈھیر_2->سائز_class_use[iclass].alloc_current);
    stats->سائز_use[iclass].alloc_peak =
        (size_t)ڈھیر_2->سائز_class_use[iclass].alloc_peak;
    stats->سائز_use[iclass].alloc_کل =
        (size_t)atomic_load32(&ڈھیر_2->سائز_class_use[iclass].alloc_کل);
    stats->سائز_use[iclass].free_کل =
        (size_t)atomic_load32(&ڈھیر_2->سائز_class_use[iclass].free_کل);
    stats->سائز_use[iclass].spans_مقصد_cache =
        (size_t)atomic_load32(&ڈھیر_2->سائز_class_use[iclass].spans_مقصد_cache);
    stats->سائز_use[iclass].spans_from_cache =
        (size_t)atomic_load32(&ڈھیر_2->سائز_class_use[iclass].spans_from_cache);
    stats->سائز_use[iclass].spans_from_reserved = (size_t)atomic_load32(
        &ڈھیر_2->سائز_class_use[iclass].spans_from_reserved);
    stats->سائز_use[iclass].map_calls =
        (size_t)atomic_load32(&ڈھیر_2->سائز_class_use[iclass].spans_map_calls);
  }
#اگر_ختم
}

خالی rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, حجم(rpmalloc_global_statistics_t));
#اگر ENABLE_STATISTICS
  stats->mapped = (size_t)atomic_load32(&mapped_صفحات) * حافظہ_صفحہ_سائز;
  stats->mapped_peak = (size_t)mapped_صفحات_peak * حافظہ_صفحہ_سائز;
  stats->mapped_کل_2 =
      (size_t)atomic_load32(&mapped_کل) * حافظہ_صفحہ_سائز;
  stats->unmapped_کل_2 =
      (size_t)atomic_load32(&unmapped_کل) * حافظہ_صفحہ_سائز;
  stats->huge_alloc =
      (size_t)atomic_load32(&huge_صفحات_current) * حافظہ_صفحہ_سائز;
  stats->huge_alloc_peak = (size_t)huge_صفحات_peak * حافظہ_صفحہ_سائز;
#اگر_ختم
#اگر ENABLE_GLOBAL_CACHE
  برائے (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    global_cache_t *cache = &حافظہ_وسعت_cache[iclass];
    جب (!atomic_cas32_acquire(&cache->تالا_2, 1, 0))
      _rpmalloc_spin();
    uint32_t گنتی = cache->گنتی;
#اگر ENABLE_UNLIMITED_CACHE
    وسعت_t *current_وسعت = cache->حد_سے_تجاوز_2;
    جب (current_وسعت) {
      ++گنتی;
      current_وسعت = current_وسعت->اگلا;
    }
#اگر_ختم
    atomic_store32_release(&cache->تالا_2, 0);
    stats->cached += گنتی * (iclass + 1) * حافظہ_وسعت_سائز;
  }
#اگر_ختم
}

#اگر ENABLE_STATISTICS

ساکن خالی حافظہ_ڈھیر_dump_statistics(ڈھیر_t *ڈھیر_2, خالی *file) {
  fprintf(file, "Heap %d stats:\n", ڈھیر_2->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  برائے (size_t iclass = 0; iclass < سائز_CLASS_گنتی; ++iclass) {
    اگر (!atomic_load32(&ڈھیر_2->سائز_class_use[iclass].alloc_کل))
      جاری;
    fprintf(
        file,
        "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu "
        "%9u\n",
        (uint32_t)iclass,
        atomic_load32(&ڈھیر_2->سائز_class_use[iclass].alloc_current),
        ڈھیر_2->سائز_class_use[iclass].alloc_peak,
        atomic_load32(&ڈھیر_2->سائز_class_use[iclass].alloc_کل),
        atomic_load32(&ڈھیر_2->سائز_class_use[iclass].free_کل),
        حافظہ_سائز_class[iclass].بلاک_سائز_2,
        حافظہ_سائز_class[iclass].بلاک_گنتی,
        atomic_load32(&ڈھیر_2->سائز_class_use[iclass].spans_current),
        ڈھیر_2->سائز_class_use[iclass].spans_peak,
        ((size_t)ڈھیر_2->سائز_class_use[iclass].alloc_peak *
         (size_t)حافظہ_سائز_class[iclass].بلاک_سائز_2) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڈھیر_2->سائز_class_use[iclass].spans_مقصد_cache) *
         حافظہ_وسعت_سائز) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڈھیر_2->سائز_class_use[iclass].spans_from_cache) *
         حافظہ_وسعت_سائز) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(
             &ڈھیر_2->سائز_class_use[iclass].spans_from_reserved) *
         حافظہ_وسعت_سائز) /
            (size_t)(1024 * 1024),
        atomic_load32(&ڈھیر_2->سائز_class_use[iclass].spans_map_calls));
  }
  fprintf(file, "Spans  Current     Peak Deferred  PeakMiB  Cached  ToCacheMiB "
                "FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB "
                "FromGlobalMiB  MmapCalls\n");
  برائے (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    اگر (!atomic_load32(&ڈھیر_2->وسعت_use[iclass].بلند) &&
        !atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_map_calls))
      جاری;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&ڈھیر_2->وسعت_use[iclass].current),
        atomic_load32(&ڈھیر_2->وسعت_use[iclass].بلند),
        atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].بلند) *
         (size_t)حافظہ_وسعت_سائز * (iclass + 1)) /
            (size_t)(1024 * 1024),
#اگر ENABLE_THREAD_CACHE
        (غیرعلامتی صحیح)(!iclass ? ڈھیر_2->وسعت_cache.گنتی
                               : ڈھیر_2->وسعت_large_cache[iclass - 1].گنتی),
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_مقصد_cache) *
         (iclass + 1) * حافظہ_وسعت_سائز) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_from_cache) *
         (iclass + 1) * حافظہ_وسعت_سائز) /
            (size_t)(1024 * 1024),
#ورنہ
        0, (size_t)0, (size_t)0,
#اگر_ختم
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_مقصد_reserved) *
         (iclass + 1) * حافظہ_وسعت_سائز) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_from_reserved) *
         (iclass + 1) * حافظہ_وسعت_سائز) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_مقصد_global) *
         (size_t)حافظہ_وسعت_سائز * (iclass + 1)) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_from_global) *
         (size_t)حافظہ_وسعت_سائز * (iclass + 1)) /
            (size_t)(1024 * 1024),
        atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_map_calls));
  }
  fprintf(file, "Full spans: %zu\n", ڈھیر_2->بھرا_وسعت_گنتی);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&ڈھیر_2->thread_مقصد_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&ڈھیر_2->global_مقصد_thread) / (size_t)(1024 * 1024));
}

#اگر_ختم

خالی rpmalloc_dump_statistics(خالی *file) {
#اگر ENABLE_STATISTICS
  برائے (size_t list_idx = 0; list_idx < ڈھیر_ARRAY_سائز; ++list_idx) {
    ڈھیر_t *ڈھیر_2 = حافظہ_heaps[list_idx];
    جب (ڈھیر_2) {
      صحیح need_dump = 0;
      برائے (size_t iclass = 0; !need_dump && (iclass < سائز_CLASS_گنتی);
           ++iclass) {
        اگر (!atomic_load32(&ڈھیر_2->سائز_class_use[iclass].alloc_کل)) {
          rpmalloc_assert(
              !atomic_load32(&ڈھیر_2->سائز_class_use[iclass].free_کل),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&ڈھیر_2->سائز_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          جاری;
        }
        need_dump = 1;
      }
      برائے (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_گنتی);
           ++iclass) {
        اگر (!atomic_load32(&ڈھیر_2->وسعت_use[iclass].بلند) &&
            !atomic_load32(&ڈھیر_2->وسعت_use[iclass].spans_map_calls))
          جاری;
        need_dump = 1;
      }
      اگر (need_dump)
        حافظہ_ڈھیر_dump_statistics(ڈھیر_2, file);
      ڈھیر_2 = ڈھیر_2->اگلا_ڈھیر;
    }
  }
  fprintf(file, "Global stats:\n");
  size_t huge_current =
      (size_t)atomic_load32(&huge_صفحات_current) * حافظہ_صفحہ_سائز;
  size_t huge_peak = (size_t)huge_صفحات_peak * حافظہ_صفحہ_سائز;
  fprintf(file, "HugeCurrentMiB HugePeakMiB\n");
  fprintf(file, "%14zu %11zu\n", huge_current / (size_t)(1024 * 1024),
          huge_peak / (size_t)(1024 * 1024));

#اگر ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  برائے (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    global_cache_t *cache = حافظہ_وسعت_cache + iclass;
    size_t global_cache = (size_t)cache->گنتی * iclass * حافظہ_وسعت_سائز;

    size_t global_حد_سے_تجاوز_cache = 0;
    وسعت_t *وسعت = cache->حد_سے_تجاوز_2;
    جب (وسعت) {
      global_حد_سے_تجاوز_cache += iclass * حافظہ_وسعت_سائز;
      وسعت = وسعت->اگلا;
    }
    اگر (global_cache || global_حد_سے_تجاوز_cache || cache->داخل_کرو_گنتی ||
        cache->نکالو_گنتی_2)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_حد_سے_تجاوز_cache / (size_t)(1024 * 1024),
              cache->داخل_کرو_گنتی, cache->نکالو_گنتی_2);
  }
#اگر_ختم

  size_t mapped = (size_t)atomic_load32(&mapped_صفحات) * حافظہ_صفحہ_سائز;
  size_t mapped_os =
      (size_t)atomic_load32(&mapped_صفحات_os) * حافظہ_صفحہ_سائز;
  size_t mapped_peak = (size_t)mapped_صفحات_peak * حافظہ_صفحہ_سائز;
  size_t mapped_کل_2 =
      (size_t)atomic_load32(&mapped_کل) * حافظہ_صفحہ_سائز;
  size_t unmapped_کل_2 =
      (size_t)atomic_load32(&unmapped_کل) * حافظہ_صفحہ_سائز;
  fprintf(
      file,
      "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB\n");
  fprintf(file, "%9zu %11zu %13zu %14zu %16zu\n",
          mapped / (size_t)(1024 * 1024), mapped_os / (size_t)(1024 * 1024),
          mapped_peak / (size_t)(1024 * 1024),
          mapped_کل_2 / (size_t)(1024 * 1024),
          unmapped_کل_2 / (size_t)(1024 * 1024));

  fprintf(file, "\n");
#اگر 0
	int64_t allocated = atomic_load64(&تخصیص_شمار);
	int64_t deallocated = atomic_load64(&deallocation_شمار);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#اگر_ختم
#اگر_ختم
  (خالی)حجم(file);
}

#اگر RPMALLOC_پہلا_CLASS_HEAPS

خارجی اندرونی rpmalloc_ڈھیر_t *rpmalloc_ڈھیر_acquire(خالی) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  ڈھیر_t *ڈھیر_2 = rpmalloc_ڈھیر_allocate(1);
  rpmalloc_assume(ڈھیر_2 != تہی);
  ڈھیر_2->owner_thread = 0;
  _rpmalloc_stat_inc(&حافظہ_فعال_heaps);
  واپس ڈھیر_2;
}

خارجی اندرونی خالی rpmalloc_ڈھیر_release_2(rpmalloc_ڈھیر_t *ڈھیر_2) {
  اگر (ڈھیر_2)
    rpmalloc_ڈھیر_release(ڈھیر_2, 1, 1);
}

خارجی اندرونی RPMALLOC_ALLOCATOR خالی *
rpmalloc_ڈھیر_alloc(rpmalloc_ڈھیر_t *ڈھیر_2, size_t سائز_2) {
#اگر ENABLE_VALIDATE_دلائل
  اگر (سائز_2 >= MAX_ALLOC_سائز) {
    errno = EINVAL;
    واپس 0;
  }
#اگر_ختم
  واپس _rpmalloc_allocate(ڈھیر_2, سائز_2);
}

خارجی اندرونی RPMALLOC_ALLOCATOR خالی *
rpmalloc_ڈھیر_aligned_alloc(rpmalloc_ڈھیر_t *ڈھیر_2, size_t alignment,
                            size_t سائز_2) {
#اگر ENABLE_VALIDATE_دلائل
  اگر (سائز_2 >= MAX_ALLOC_سائز) {
    errno = EINVAL;
    واپس 0;
  }
#اگر_ختم
  واپس _rpmalloc_aligned_allocate(ڈھیر_2, alignment, سائز_2);
}

خارجی اندرونی RPMALLOC_ALLOCATOR خالی *
rpmalloc_ڈھیر_calloc(rpmalloc_ڈھیر_t *ڈھیر_2, size_t عدد_2, size_t سائز_2) {
  واپس rpmalloc_ڈھیر_aligned_calloc(ڈھیر_2, 0, عدد_2, سائز_2);
}

خارجی اندرونی RPMALLOC_ALLOCATOR خالی *
rpmalloc_ڈھیر_aligned_calloc(rpmalloc_ڈھیر_t *ڈھیر_2, size_t alignment,
                             size_t عدد_2, size_t سائز_2) {
  size_t کل_2;
#اگر ENABLE_VALIDATE_دلائل
#اگر PLATFORM_WINDOWS
  صحیح err = سائز_TMult(عدد_2, سائز_2, &کل_2);
  اگر ((err != S_OK) || (کل_2 >= MAX_ALLOC_سائز)) {
    errno = EINVAL;
    واپس 0;
  }
#ورنہ
  صحیح err = __builtin_umull_overflow(عدد_2, سائز_2, &کل_2);
  اگر (err || (کل_2 >= MAX_ALLOC_سائز)) {
    errno = EINVAL;
    واپس 0;
  }
#اگر_ختم
#ورنہ
  کل_2 = عدد_2 * سائز_2;
#اگر_ختم
  خالی *بلاک = _rpmalloc_aligned_allocate(ڈھیر_2, alignment, کل_2);
  اگر (بلاک)
    memset(بلاک, 0, کل_2);
  واپس بلاک;
}

خارجی اندرونی RPMALLOC_ALLOCATOR خالی *
rpmalloc_ڈھیر_realloc(rpmalloc_ڈھیر_t *ڈھیر_2, خالی *ptr, size_t سائز_2,
                      غیرعلامتی صحیح flags) {
#اگر ENABLE_VALIDATE_دلائل
  اگر (سائز_2 >= MAX_ALLOC_سائز) {
    errno = EINVAL;
    واپس ptr;
  }
#اگر_ختم
  واپس _rpmalloc_reallocate(ڈھیر_2, ptr, سائز_2, 0, flags);
}

خارجی اندرونی RPMALLOC_ALLOCATOR خالی *
rpmalloc_ڈھیر_aligned_realloc(rpmalloc_ڈھیر_t *ڈھیر_2, خالی *ptr,
                              size_t alignment, size_t سائز_2,
                              غیرعلامتی صحیح flags) {
#اگر ENABLE_VALIDATE_دلائل
  اگر ((سائز_2 + alignment < سائز_2) || (alignment > حافظہ_صفحہ_سائز)) {
    errno = EINVAL;
    واپس 0;
  }
#اگر_ختم
  واپس _rpmalloc_aligned_reallocate(ڈھیر_2, ptr, alignment, سائز_2, 0, flags);
}

خارجی اندرونی خالی rpmalloc_ڈھیر_free(rpmalloc_ڈھیر_t *ڈھیر_2, خالی *ptr) {
  (خالی)حجم(ڈھیر_2);
  _rpmalloc_deallocate(ptr);
}

خارجی اندرونی خالی rpmalloc_ڈھیر_free_all(rpmalloc_ڈھیر_t *ڈھیر_2) {
  وسعت_t *وسعت;
  وسعت_t *اگلا_وسعت;

  rpmalloc_ڈھیر_cache_adopt_deferred(ڈھیر_2, 0);

  برائے (size_t iclass = 0; iclass < سائز_CLASS_گنتی; ++iclass) {
    وسعت = ڈھیر_2->سائز_class[iclass].partial_وسعت;
    جب (وسعت) {
      اگلا_وسعت = وسعت->اگلا;
      rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
      وسعت = اگلا_وسعت;
    }
    ڈھیر_2->سائز_class[iclass].partial_وسعت = 0;
    وسعت = ڈھیر_2->بھرا_وسعت[iclass];
    جب (وسعت) {
      اگلا_وسعت = وسعت->اگلا;
      rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
      وسعت = اگلا_وسعت;
    }

    وسعت = ڈھیر_2->سائز_class[iclass].cache;
    اگر (وسعت)
      rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
    ڈھیر_2->سائز_class[iclass].cache = 0;
  }
  memset(ڈھیر_2->سائز_class, 0, حجم(ڈھیر_2->سائز_class));
  memset(ڈھیر_2->بھرا_وسعت, 0, حجم(ڈھیر_2->بھرا_وسعت));

  وسعت = ڈھیر_2->large_huge_وسعت;
  جب (وسعت) {
    اگلا_وسعت = وسعت->اگلا;
    اگر (UNEXPECTED(وسعت->سائز_class == سائز_CLASS_HUGE))
      _rpmalloc_deallocate_huge(وسعت);
    ورنہ
      rpmalloc_ڈھیر_cache_داخل_کرو(ڈھیر_2, وسعت);
    وسعت = اگلا_وسعت;
  }
  ڈھیر_2->large_huge_وسعت = 0;
  ڈھیر_2->بھرا_وسعت_گنتی = 0;

#اگر ENABLE_THREAD_CACHE
  برائے (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    وسعت_cache_t *وسعت_cache;
    اگر (!iclass)
      وسعت_cache = &ڈھیر_2->وسعت_cache;
    ورنہ
      وسعت_cache = (وسعت_cache_t *)(ڈھیر_2->وسعت_large_cache + (iclass - 1));
    اگر (!وسعت_cache->گنتی)
      جاری;
#اگر ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&ڈھیر_2->thread_مقصد_global,
                         وسعت_cache->گنتی * (iclass + 1) * حافظہ_وسعت_سائز);
    rpmalloc_stat_جوڑو(&ڈھیر_2->وسعت_use[iclass].spans_مقصد_global,
                       وسعت_cache->گنتی);
    rpmalloc_global_cache_داخل_کرو_spans(وسعت_cache->وسعت, iclass + 1,
                                        وسعت_cache->گنتی);
#ورنہ
    برائے (size_t ispan = 0; ispan < وسعت_cache->گنتی; ++ispan)
      rpmalloc_وسعت_unmap(وسعت_cache->وسعت[ispan]);
#اگر_ختم
    وسعت_cache->گنتی = 0;
  }
#اگر_ختم

#اگر ENABLE_STATISTICS
  برائے (size_t iclass = 0; iclass < سائز_CLASS_گنتی; ++iclass) {
    atomic_store32(&ڈھیر_2->سائز_class_use[iclass].alloc_current, 0);
    atomic_store32(&ڈھیر_2->سائز_class_use[iclass].spans_current, 0);
  }
  برائے (size_t iclass = 0; iclass < LARGE_CLASS_گنتی; ++iclass) {
    atomic_store32(&ڈھیر_2->وسعت_use[iclass].current, 0);
  }
#اگر_ختم
}

خارجی اندرونی خالی rpmalloc_ڈھیر_thread_سیٹ_current(rpmalloc_ڈھیر_t *ڈھیر_2) {
  ڈھیر_t *پچھلا_ڈھیر = get_thread_ڈھیر_raw();
  اگر (پچھلا_ڈھیر != ڈھیر_2) {
    سیٹ_thread_ڈھیر(ڈھیر_2);
    اگر (پچھلا_ڈھیر)
      rpmalloc_ڈھیر_release_2(پچھلا_ڈھیر);
  }
}

خارجی اندرونی rpmalloc_ڈھیر_t *rpmalloc_get_ڈھیر_for_ptr(خالی *ptr) {
  // Grab the span, and then the heap from the span
  وسعت_t *وسعت = (وسعت_t *)((uintptr_t)ptr & حافظہ_وسعت_نقاب);
  اگر (وسعت) {
    واپس وسعت->ڈھیر_2;
  }
  واپس 0;
}

#اگر_ختم

#اگر ENABLE_PRELOAD || ENABLE_OVERRIDE

#شامل "malloc.c"

#اگر_ختم

خالی rpmalloc_linker_reference(خالی) { (خالی)حجم(_rpmalloc_initialized); }
