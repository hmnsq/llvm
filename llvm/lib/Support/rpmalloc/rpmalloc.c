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
#لارښوونه clang diagnostic ignored "-Wunused-macros"
#لارښوونه clang diagnostic ignored "-Wunused-function"
#إذا __has_warning("-Wreserved-identifier")
#لارښوونه clang diagnostic ignored "-Wreserved-identifier"
#د_که_پای
#إذا __has_warning("-Wstatic-in-inline")
#لارښوونه clang diagnostic ignored "-Wstatic-in-inline"
#د_که_پای
#وإلاإذا defined(__GNUC__)
#لارښوونه GCC diagnostic ignored "-Wunused-macros"
#لارښوونه GCC diagnostic ignored "-Wunused-function"
#د_که_پای

#إذا !defined(__has_builtin)
#تعریف_کړه __has_builtin(b) 0
#د_که_پای

#إذا defined(__GNUC__) || defined(__clang__)

#إذا __has_builtin(__builtin_memcpy_inline)
#تعریف_کړه _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#وإلا
#تعریف_کړه _rpmalloc_memcpy_const(x, y, s)                                        \
  افعل {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    حافظه_نقل(x, y, s);                                                           \
  } طالما (0)
#د_که_پای

#إذا __has_builtin(__builtin_memset_inline)
#تعریف_کړه _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#وإلا
#تعریف_کړه _rpmalloc_memset_const(x, y, s)                                        \
  افعل {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } طالما (0)
#د_که_پای
#وإلا
#تعریف_کړه _rpmalloc_memcpy_const(x, y, s) حافظه_نقل(x, y, s)
#تعریف_کړه _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#د_که_پای

#إذا __has_builtin(__builtin_assume)
#تعریف_کړه rpmalloc_assume(cond) __builtin_assume(cond)
#وإلاإذا defined(__GNUC__)
#تعریف_کړه rpmalloc_assume(cond)                                                  \
  افعل {                                                                         \
    إذا (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } طالما (0)
#وإلاإذا defined(_MSC_VER)
#تعریف_کړه rpmalloc_assume(cond) __assume(cond)
#وإلا
#تعریف_کړه rpmalloc_assume(cond) 0
#د_که_پای

#که_نه_تعریف_شوی ډېرۍ_ARRAY_اندازه
//! Size of heap hashmap
#تعریف_کړه ډېرۍ_ARRAY_اندازه 47
#د_که_پای
#که_نه_تعریف_شوی ENABLE_THREAD_CACHE
//! Enable per-thread cache
#تعریف_کړه ENABLE_THREAD_CACHE 1
#د_که_پای
#که_نه_تعریف_شوی ENABLE_GLOBAL_CACHE
//! Enable global cache shared between all threads, requires thread cache
#تعریف_کړه ENABLE_GLOBAL_CACHE 1
#د_که_پای
#که_نه_تعریف_شوی ENABLE_VALIDATE_دلیلونه
//! Enable validation of args to public entry points
#تعریف_کړه ENABLE_VALIDATE_دلیلونه 0
#د_که_پای
#که_نه_تعریف_شوی ENABLE_STATISTICS
//! Enable statistics collection
#تعریف_کړه ENABLE_STATISTICS 0
#د_که_پای
#که_نه_تعریف_شوی ENABLE_ASSERTS
//! Enable asserts
#تعریف_کړه ENABLE_ASSERTS 0
#د_که_پای
#که_نه_تعریف_شوی ENABLE_OVERRIDE
//! Override standard library malloc/free and new/delete entry points
#تعریف_کړه ENABLE_OVERRIDE 0
#د_که_پای
#که_نه_تعریف_شوی ENABLE_PRELOAD
//! Support preloading
#تعریف_کړه ENABLE_PRELOAD 0
#د_که_پای
#که_نه_تعریف_شوی DISABLE_UNMAP
//! Disable unmapping memory pages (also enables unlimited cache)
#تعریف_کړه DISABLE_UNMAP 0
#د_که_پای
#که_نه_تعریف_شوی ENABLE_UNLIMITED_CACHE
//! Enable unlimited global cache (no unmapping until finalization)
#تعریف_کړه ENABLE_UNLIMITED_CACHE 0
#د_که_پای
#که_نه_تعریف_شوی ENABLE_ADAPTIVE_THREAD_CACHE
//! Enable adaptive thread cache size based on use heuristics
#تعریف_کړه ENABLE_ADAPTIVE_THREAD_CACHE 0
#د_که_پای
#که_نه_تعریف_شوی DEFAULT_پراختیا_MAP_شمېر
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#تعریف_کړه DEFAULT_پراختیا_MAP_شمېر 64
#د_که_پای
#که_نه_تعریف_شوی GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#تعریف_کړه GLOBAL_CACHE_MULTIPLIER 8
#د_که_پای

#إذا DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#تېروتنه Must use global cache إذا unmap is disabled
#د_که_پای

#إذا DISABLE_UNMAP
#تعریف_لغوه ENABLE_UNLIMITED_CACHE
#تعریف_کړه ENABLE_UNLIMITED_CACHE 1
#د_که_پای

#إذا !ENABLE_GLOBAL_CACHE
#تعریف_لغوه ENABLE_UNLIMITED_CACHE
#تعریف_کړه ENABLE_UNLIMITED_CACHE 0
#د_که_پای

#إذا !ENABLE_THREAD_CACHE
#تعریف_لغوه ENABLE_ADAPTIVE_THREAD_CACHE
#تعریف_کړه ENABLE_ADAPTIVE_THREAD_CACHE 0
#د_که_پای

#إذا defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#تعریف_کړه PLATFORM_WINDOWS 1
#تعریف_کړه PLATFORM_POSIX 0
#وإلا
#تعریف_کړه PLATFORM_WINDOWS 0
#تعریف_کړه PLATFORM_POSIX 1
#د_که_پای

/// Platform and arch specifics
#إذا defined(_MSC_VER) && !defined(__clang__)
#لارښوونه warning(disable : 5105)
#که_نه_تعریف_شوی FORCEINLINE
#تعریف_کړه FORCEINLINE مضمن __forceinline
#د_که_پای
#تعریف_کړه _Static_assert static_assert
#وإلا
#که_نه_تعریف_شوی FORCEINLINE
#تعریف_کړه FORCEINLINE مضمن __attribute__((__always_inline__))
#د_که_پای
#د_که_پای
#إذا PLATFORM_WINDOWS
#که_نه_تعریف_شوی WIN32_LEAN_AND_منځنۍ
#تعریف_کړه WIN32_LEAN_AND_منځنۍ
#د_که_پای
#ضمن <windows.h>
#إذا ENABLE_VALIDATE_دلیلونه
#ضمن <intsafe.h>
#د_که_پای
#وإلا
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <time.h>
#ضمن <unistd.h>
#إذا defined(__linux__) || defined(__ANDROID__)
#ضمن <sys/prctl.h>
#إذا !defined(PR_سېټ_VMA)
#تعریف_کړه PR_سېټ_VMA 0x53564d41
#تعریف_کړه PR_سېټ_VMA_ANON_NAME 0
#د_که_پای
#د_که_پای
#إذا defined(__APPLE__)
#ضمن <TargetConditionals.h>
#إذا !موخه_OS_IPHONE && !موخه_OS_SIMULATOR
#ضمن <mach/mach_vm.h>
#ضمن <mach/vm_statistics.h>
#د_که_پای
#ضمن <pthread.h>
#د_که_پای
#إذا defined(__HAIKU__) || defined(__TINYC__)
#ضمن <pthread.h>
#د_که_پای
#د_که_پای

#ضمن <errno.h>
#ضمن <stdint.h>
#ضمن <string.h>

#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#ضمن <fibersapi.h>
ساكن DWORD fls_کلی;
#د_که_پای

#إذا PLATFORM_POSIX
#ضمن <sched.h>
#ضمن <sys/mman.h>
#که_تعریف_شوی __FreeBSD__
#ضمن <sys/sysctl.h>
#تعریف_کړه MAP_HUGETLB MAP_ALIGNED_SUPER
#که_نه_تعریف_شوی PROT_MAX
#تعریف_کړه PROT_MAX(f) 0
#د_که_پای
#وإلا
#تعریف_کړه PROT_MAX(f) 0
#د_که_پای
#که_تعریف_شوی __sun
خارجي صحيح madvise(caddr_t, size_t, صحيح);
#د_که_پای
#که_نه_تعریف_شوی MAP_UNINITIALIZED
#تعریف_کړه MAP_UNINITIALIZED 0
#د_که_پای
#د_که_پای
#ضمن <errno.h>

#إذا ENABLE_ASSERTS
#تعریف_لغوه NDEBUG
#إذا defined(_MSC_VER) && !defined(_DEBUG)
#تعریف_کړه _DEBUG
#د_که_پای
#ضمن <assert.h>
#تعریف_کړه RPMALLOC_TOSTRING_M(x) #x
#تعریف_کړه RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#تعریف_کړه rpmalloc_assert(truth, پیغام_2)                                        \
  افعل {                                                                         \
    إذا (!(truth)) {                                                            \
      إذا (حافظه_config.error_callback) {                                     \
        حافظه_config.error_callback(پیغام_2 " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } وإلا {                                                                 \
        أكد((truth) && پیغام_2);                                            \
      }                                                                        \
    }                                                                          \
  } طالما (0)
#وإلا
#تعریف_کړه rpmalloc_assert(truth, پیغام_2)                                        \
  افعل {                                                                         \
  } طالما (0)
#د_که_پای
#إذا ENABLE_STATISTICS
#ضمن <stdio.h>
#د_که_پای

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#إذا defined(_MSC_VER) && !defined(__clang__)

تعريفنوع متطاير اوږد atomic32_t;
تعريفنوع متطاير اوږد اوږد atomic64_t;
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
ساكن FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t جمع_کړه) {
  أرجع (int32_t)Interlocked_Exchange_جمع_کړه(val, جمع_کړه) + جمع_کړه;
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
ساكن FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t جمع_کړه) {
  أرجع (int64_t)InterlockedExchangeAdd64(val, جمع_کړه) + جمع_کړه;
}
ساكن FORCEINLINE فراغ *atomic_بارول_ptr(atomicptr_t *src) {
  أرجع Interlocked_Compare_Exchange_اشاره_ګر(src, 0, 0);
}
ساكن FORCEINLINE فراغ atomic_store_ptr(atomicptr_t *dst, فراغ *val) {
  Interlocked_Exchange_اشاره_ګر(dst, val);
}
ساكن FORCEINLINE فراغ atomic_store_ptr_release(atomicptr_t *dst, فراغ *val) {
  Interlocked_Exchange_اشاره_ګر(dst, val);
}
ساكن FORCEINLINE فراغ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     فراغ *val) {
  أرجع (فراغ *)Interlocked_Exchange_اشاره_ګر((فراغ *متطاير *)dst, val);
}
ساكن FORCEINLINE صحيح atomic_cas_ptr(atomicptr_t *dst, فراغ *val, فراغ *ref) {
  أرجع (Interlocked_Compare_Exchange_اشاره_ګر((فراغ *متطاير *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#تعریف_کړه EXPECTED(x) (x)
#تعریف_کړه UNEXPECTED(x) (x)

#وإلا

#ضمن <stdatomic.h>

تعريفنوع متطاير _Atomic(int32_t) atomic32_t;
تعريفنوع متطاير _Atomic(int64_t) atomic64_t;
تعريفنوع متطاير _Atomic(فراغ *) atomicptr_t;

ساكن FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  أرجع atomic_بارول_explicit(src, memory_order_relaxed);
}
ساكن FORCEINLINE فراغ atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ساكن FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  أرجع atomic_fetch_جمع_کړه_explicit(val, 1, memory_order_relaxed) + 1;
}
ساكن FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  أرجع atomic_fetch_جمع_کړه_explicit(val, -1, memory_order_relaxed) - 1;
}
ساكن FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t جمع_کړه) {
  أرجع atomic_fetch_جمع_کړه_explicit(val, جمع_کړه, memory_order_relaxed) + جمع_کړه;
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
  أرجع atomic_بارول_explicit(val, memory_order_relaxed);
}
ساكن FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t جمع_کړه) {
  أرجع atomic_fetch_جمع_کړه_explicit(val, جمع_کړه, memory_order_relaxed) + جمع_کړه;
}
ساكن FORCEINLINE فراغ *atomic_بارول_ptr(atomicptr_t *src) {
  أرجع atomic_بارول_explicit(src, memory_order_relaxed);
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

#تعریف_کړه EXPECTED(x) __builtin_expect((x), 1)
#تعریف_کړه UNEXPECTED(x) __builtin_expect((x), 0)

#د_که_پای

////////////
///
/// Statistics related functions (evaluate to nothing when statistics not
/// enabled)
///
//////

#إذا ENABLE_STATISTICS
#تعریف_کړه _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#تعریف_کړه _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#تعریف_کړه rpmalloc_stat_جمع_کړه(counter, ارزښت_2)                                     \
  atomic_add32(counter, (int32_t)(ارزښت_2))
#تعریف_کړه _rpmalloc_stat_add64(counter, ارزښت_2)                                   \
  atomic_add64(counter, (int64_t)(ارزښت_2))
#تعریف_کړه rpmalloc_stat_جمع_کړه_peak(counter, ارزښت_2, peak)                          \
  افعل {                                                                         \
    int32_t cur_شمېر = atomic_add32(counter, (int32_t)(ارزښت_2));              \
    إذا (cur_شمېر > (peak))                                                   \
      peak = cur_شمېر;                                                       \
  } طالما (0)
#تعریف_کړه _rpmalloc_stat_sub(counter, ارزښت_2)                                     \
  atomic_add32(counter, -(int32_t)(ارزښت_2))
#تعریف_کړه _rpmalloc_stat_inc_alloc(ډېرۍ, class_idx)                              \
  افعل {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&ډېرۍ->اندازه_class_use[class_idx].alloc_current);         \
    إذا (alloc_current > ډېرۍ->اندازه_class_use[class_idx].alloc_peak)            \
      ډېرۍ->اندازه_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&ډېرۍ->اندازه_class_use[class_idx].alloc_ټول);               \
  } طالما (0)
#تعریف_کړه _rpmalloc_stat_inc_free(ډېرۍ, class_idx)                               \
  افعل {                                                                         \
    atomic_decr32(&ډېرۍ->اندازه_class_use[class_idx].alloc_current);             \
    atomic_incr32(&ډېرۍ->اندازه_class_use[class_idx].free_ټول);                \
  } طالما (0)
#وإلا
#تعریف_کړه _rpmalloc_stat_inc(counter)                                            \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کړه _rpmalloc_stat_dec(counter)                                            \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کړه rpmalloc_stat_جمع_کړه(counter, ارزښت_2)                                     \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کړه _rpmalloc_stat_add64(counter, ارزښت_2)                                   \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کړه rpmalloc_stat_جمع_کړه_peak(counter, ارزښت_2, peak)                          \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کړه _rpmalloc_stat_sub(counter, ارزښت_2)                                     \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کړه _rpmalloc_stat_inc_alloc(ډېرۍ, class_idx)                              \
  افعل {                                                                         \
  } طالما (0)
#تعریف_کړه _rpmalloc_stat_inc_free(ډېرۍ, class_idx)                               \
  افعل {                                                                         \
  } طالما (0)
#د_که_پای

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#تعریف_کړه SMALL_GRANULARITY 16
//! Small granularity shift count
#تعریف_کړه SMALL_GRANULARITY_ښويدنه 4
//! Number of small block size classes
#تعریف_کړه SMALL_CLASS_شمېر 65
//! Maximum size of a small block
#تعریف_کړه SMALL_اندازه_حد (SMALL_GRANULARITY * (SMALL_CLASS_شمېر - 1))
//! Granularity of a medium allocation block
#تعریف_کړه MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#تعریف_کړه MEDIUM_GRANULARITY_ښويدنه 9
//! Number of medium block size classes
#تعریف_کړه MEDIUM_CLASS_شمېر 61
//! Total number of small + medium size classes
#تعریف_کړه اندازه_CLASS_شمېر (SMALL_CLASS_شمېر + MEDIUM_CLASS_شمېر)
//! Number of large block size classes
#تعریف_کړه LARGE_CLASS_شمېر 63
//! Maximum size of a medium block
#تعریف_کړه MEDIUM_اندازه_حد                                                      \
  (SMALL_اندازه_حد + (MEDIUM_GRANULARITY * MEDIUM_CLASS_شمېر))
//! Maximum size of a large block
#تعریف_کړه LARGE_اندازه_حد                                                       \
  ((LARGE_CLASS_شمېر * حافظه_پراختیا_اندازه) - پراختیا_سرلیک_اندازه)
//! Size of a span header (must be a multiple of SMALL_GRANULARITY and a power
//! of two)
#تعریف_کړه پراختیا_سرلیک_اندازه 128
//! Number of spans in thread cache
#تعریف_کړه MAX_THREAD_پراختیا_CACHE 400
//! Number of spans to transfer between thread and global cache
#تعریف_کړه THREAD_پراختیا_CACHE_TRANSFER 64
//! Number of spans in thread cache for large spans (must be greater than
//! LARGE_CLASS_COUNT / 2)
#تعریف_کړه MAX_THREAD_پراختیا_LARGE_CACHE 100
//! Number of spans to transfer between thread and global cache for large spans
#تعریف_کړه THREAD_پراختیا_LARGE_CACHE_TRANSFER 6

_Static_assert((SMALL_GRANULARITY & (SMALL_GRANULARITY - 1)) == 0,
               "Small granularity must be power of two");
_Static_assert((پراختیا_سرلیک_اندازه & (پراختیا_سرلیک_اندازه - 1)) == 0,
               "Span header size must be power of two");

#إذا ENABLE_VALIDATE_دلیلونه
//! Maximum allocation size to avoid integer overflow
#تعریف_لغوه MAX_ALLOC_اندازه
#تعریف_کړه MAX_ALLOC_اندازه (((size_t) - 1) - حافظه_پراختیا_اندازه)
#د_که_پای

#تعریف_کړه اشاره_ګر_دننه_واټن(ptr, ofs) (فراغ *)((حرف *)(ptr) + (ptrdiff_t)(ofs))
#تعریف_کړه اشاره_ګر_diff(لومړی_2, دوهم_2)                                            \
  (ptrdiff_t)((ثابت حرف *)(لومړی_2) - (ثابت حرف *)(دوهم_2))

#تعریف_کړه INVALID_اشاره_ګر ((فراغ *)((uintptr_t) - 1))

#تعریف_کړه اندازه_CLASS_LARGE اندازه_CLASS_شمېر
#تعریف_کړه اندازه_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
تعريفنوع بنية ډېرۍ_t ډېرۍ_t;
//! Span of memory pages
تعريفنوع بنية پراختیا_t پراختیا_t;
//! Span list
تعريفنوع بنية پراختیا_list_t پراختیا_list_t;
//! Span active data
تعريفنوع بنية پراختیا_فعال_t پراختیا_فعال_t;
//! Size class definition
تعريفنوع بنية اندازه_class_t اندازه_class_t;
//! Global cache
تعريفنوع بنية global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#تعریف_کړه پراختیا_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#تعریف_کړه پراختیا_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#تعریف_کړه پراختیا_FLAG_ALIGNED_بلاکونه 4U
//! Flag indicating an unmapped master span
#تعریف_کړه پراختیا_FLAG_UNMAPPED_MASTER 8U

#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
بنية پراختیا_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t لوړ;
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
#د_که_پای
};
تعريفنوع بنية پراختیا_use_t پراختیا_use_t;
#د_که_پای

#إذا ENABLE_STATISTICS
بنية اندازه_class_use_t {
  //! Current number of allocations
  atomic32_t alloc_current;
  //! Peak number of allocations
  int32_t alloc_peak;
  //! Total number of allocations
  atomic32_t alloc_ټول;
  //! Total number of frees
  atomic32_t free_ټول;
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
  int32_t ناکارول_شوی_2;
};
تعريفنوع بنية اندازه_class_use_t اندازه_class_use_t;
#د_که_پای

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
بنية پراختیا_t {
  //! Free list
  فراغ *free_list;
  //! Total block count of size class
  uint32_t بلاک_شمېر;
  //! Size class
  uint32_t اندازه_class;
  //! Index of last block initialized in free list
  uint32_t free_list_حد;
  //! Number of used blocks remaining when in partial state
  uint32_t کارول_شوی_شمېر;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_اندازه;
  //! Size of a block
  uint32_t بلاک_اندازه_2;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t پراختیا_شمېر;
  //! Total span counter for master spans
  uint32_t ټول_spans;
  //! Offset from master span for subspans
  uint32_t دننه_واټن_from_master;
  //! Remaining span counter, for master spans
  atomic32_t پاتې_شوی_spans;
  //! Alignment offset
  uint32_t align_دننه_واټن;
  //! Owning heap
  ډېرۍ_t *ډېرۍ;
  //! Next span
  پراختیا_t *بل;
  //! Previous span
  پراختیا_t *prev;
};
_Static_assert(حجم(پراختیا_t) <= پراختیا_سرلیک_اندازه, "span size mismatch");

بنية پراختیا_cache_t {
  size_t شمېر;
  پراختیا_t *پراختیا[MAX_THREAD_پراختیا_CACHE];
};
تعريفنوع بنية پراختیا_cache_t پراختیا_cache_t;

بنية پراختیا_large_cache_t {
  size_t شمېر;
  پراختیا_t *پراختیا[MAX_THREAD_پراختیا_LARGE_CACHE];
};
تعريفنوع بنية پراختیا_large_cache_t پراختیا_large_cache_t;

بنية ډېرۍ_اندازه_class_t {
  //! Free list of active span
  فراغ *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  پراختیا_t *partial_پراختیا;
  //! Early level cache of fully free spans
  پراختیا_t *cache;
};
تعريفنوع بنية ډېرۍ_اندازه_class_t ډېرۍ_اندازه_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
بنية ډېرۍ_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  ډېرۍ_اندازه_class_t اندازه_class[اندازه_CLASS_شمېر];
#إذا ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  پراختیا_cache_t پراختیا_cache;
#د_که_پای
  //! List of deferred free spans (single linked list)
  atomicptr_t پراختیا_free_deferred;
  //! Number of full spans
  size_t ډک_پراختیا_شمېر;
  //! Mapped but unused spans
  پراختیا_t *پراختیا_reserve;
  //! Master span for mapped but unused spans
  پراختیا_t *پراختیا_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t ماشوم_شمېر;
  //! Next heap in id list
  ډېرۍ_t *بل_ډېرۍ;
  //! Next heap in orphan list
  ډېرۍ_t *بل_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  صحيح finalize;
  //! Master heap owning the memory pages
  ډېرۍ_t *master_ډېرۍ;
#إذا ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  پراختیا_large_cache_t پراختیا_large_cache[LARGE_CLASS_شمېر - 1];
#د_که_پای
#إذا RPMALLOC_لومړی_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  پراختیا_t *ډک_پراختیا[اندازه_CLASS_شمېر];
  //! Double linked list of large and huge spans allocated by this heap
  پراختیا_t *large_huge_پراختیا;
#د_که_پای
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  پراختیا_use_t پراختیا_use[LARGE_CLASS_شمېر];
#د_که_پای
#إذا ENABLE_STATISTICS
  //! Allocation stats per size class
  اندازه_class_use_t اندازه_class_use[اندازه_CLASS_شمېر + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_منزل_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_منزل_thread;
#د_که_پای
};

// Size class for defining a block size bucket
بنية اندازه_class_t {
  //! Size of blocks in this class
  uint32_t بلاک_اندازه_2;
  //! Number of blocks in each chunk
  uint16_t بلاک_شمېر;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(حجم(اندازه_class_t) == 8, "Size class size mismatch");

بنية global_cache_t {
  //! Cache lock
  atomic32_t قلف;
  //! Cache count
  uint32_t شمېر;
#إذا ENABLE_STATISTICS
  //! Insert count
  size_t ورننباسه_شمېر;
  //! Extract count
  size_t ایستل_شمېر;
#د_که_پای
  //! Cached spans
  پراختیا_t *پراختیا[GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_پراختیا_CACHE];
  //! Unlimited cache overflow
  پراختیا_t *ډکېدنه_2;
};

////////////
///
/// Global data
///
//////

//! Default span size (64KiB)
#تعریف_کړه حافظه_default_پراختیا_اندازه (64 * 1024)
#تعریف_کړه حافظه_default_پراختیا_اندازه_ښويدنه 16
#تعریف_کړه حافظه_default_پراختیا_ماسک (~((uintptr_t)(حافظه_پراختیا_اندازه - 1)))

//! Initialized flag
ساكن صحيح _rpmalloc_initialized;
//! Main thread ID
ساكن uintptr_t _rpmalloc_main_thread_id;
//! Configuration
ساكن rpmalloc_config_t حافظه_config;
//! Memory page size
ساكن size_t حافظه_پاڼه_اندازه;
//! Shift to divide by page size
ساكن size_t حافظه_پاڼه_اندازه_ښويدنه;
//! Granularity at which memory pages are mapped by OS
ساكن size_t حافظه_map_granularity;
#إذا RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
ساكن size_t حافظه_پراختیا_اندازه;
//! Shift to divide by span size
ساكن size_t حافظه_پراختیا_اندازه_ښويدنه;
//! Mask to get to start of a memory span
ساكن uintptr_t حافظه_پراختیا_ماسک;
#وإلا
//! Hardwired span size
#تعریف_کړه حافظه_پراختیا_اندازه حافظه_default_پراختیا_اندازه
#تعریف_کړه حافظه_پراختیا_اندازه_ښويدنه حافظه_default_پراختیا_اندازه_ښويدنه
#تعریف_کړه حافظه_پراختیا_ماسک حافظه_default_پراختیا_ماسک
#د_که_پای
//! Number of spans to map in each map call
ساكن size_t حافظه_پراختیا_map_شمېر;
//! Number of spans to keep reserved in each heap
ساكن size_t حافظه_ډېرۍ_reserve_شمېر;
//! Global size classes
ساكن اندازه_class_t حافظه_اندازه_class[اندازه_CLASS_شمېر];
//! Run-time size limit of medium blocks
ساكن size_t حافظه_medium_اندازه_حد;
//! Heap ID counter
ساكن atomic32_t حافظه_ډېرۍ_id;
//! Huge page support
ساكن صحيح حافظه_huge_پاڼې;
#إذا ENABLE_GLOBAL_CACHE
//! Global span cache
ساكن global_cache_t حافظه_پراختیا_cache[LARGE_CLASS_شمېر];
#د_که_پای
//! Global reserved spans
ساكن پراختیا_t *حافظه_global_reserve;
//! Global reserved count
ساكن size_t حافظه_global_reserve_شمېر;
//! Global reserved master
ساكن پراختیا_t *حافظه_global_reserve_master;
//! All heaps
ساكن ډېرۍ_t *حافظه_heaps[ډېرۍ_ARRAY_اندازه];
//! Used to restrict access to mapping memory for huge pages
ساكن atomic32_t حافظه_global_قلف;
//! Orphaned heaps
ساكن ډېرۍ_t *حافظه_orphan_heaps;
#إذا RPMALLOC_لومړی_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
ساكن ډېرۍ_t *حافظه_لومړی_class_orphan_heaps;
#د_که_پای
#إذا ENABLE_STATISTICS
//! Allocations counter
ساكن atomic64_t ویش_counter;
//! Deallocations counter
ساكن atomic64_t _deallocation_counter;
//! Active heap count
ساكن atomic32_t حافظه_فعال_heaps;
//! Number of currently mapped memory pages
ساكن atomic32_t mapped_پاڼې;
//! Peak number of concurrently mapped memory pages
ساكن int32_t mapped_پاڼې_peak;
//! Number of mapped master spans
ساكن atomic32_t _master_spans;
//! Number of unmapped dangling master spans
ساكن atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
ساكن atomic32_t mapped_ټول;
//! Running counter of total number of unmapped memory pages since start
ساكن atomic32_t unmapped_ټول;
//! Number of currently mapped memory pages in OS calls
ساكن atomic32_t mapped_پاڼې_os;
//! Number of currently allocated pages in huge allocations
ساكن atomic32_t huge_پاڼې_current;
//! Peak number of currently allocated pages in huge allocations
ساكن int32_t huge_پاڼې_peak;
#د_که_پای

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
ساكن pthread_key_t حافظه_thread_ډېرۍ;
#وإلا
#که_تعریف_شوی _MSC_VER
#تعریف_کړه _Thread_local __declspec(thread)
#تعریف_کړه TLS_MODEL
#وإلا
#که_نه_تعریف_شوی __HAIKU__
#تعریف_کړه TLS_MODEL __attribute__((tls_model("initial-exec")))
#وإلا
#تعریف_کړه TLS_MODEL
#د_که_پای
#إذا !defined(__clang__) && defined(__GNUC__)
#تعریف_کړه _Thread_local __thread
#د_که_پای
#د_که_پای
ساكن _Thread_local ډېرۍ_t *حافظه_thread_ډېرۍ TLS_MODEL;
#د_که_پای

ساكن مضمن ډېرۍ_t *get_thread_ډېرۍ_raw(فراغ) {
#إذا (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  أرجع pthread_getspecific(حافظه_thread_ډېرۍ);
#وإلا
  أرجع حافظه_thread_ډېرۍ;
#د_که_پای
}

//! Get the current thread heap
ساكن مضمن ډېرۍ_t *get_thread_ډېرۍ(فراغ) {
  ډېرۍ_t *ډېرۍ = get_thread_ډېرۍ_raw();
#إذا ENABLE_PRELOAD
  إذا (EXPECTED(ډېرۍ != 0))
    أرجع ډېرۍ;
  rpmalloc_initialize();
  أرجع get_thread_ډېرۍ_raw();
#وإلا
  أرجع ډېرۍ;
#د_که_پای
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
#د_که_پای
#وإلاإذا defined(__arm__)
  __asm__ متطاير("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#وإلاإذا defined(__aarch64__)
#إذا defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ متطاير("mrs %0, tpidrro_el0" : "=r"(tid));
#وإلا
  __asm__ متطاير("mrs %0, tpidr_el0" : "=r"(tid));
#د_که_پای
#وإلا
#تېروتنه This platform needs implementation د_3 get_thread_id()
#د_که_پای
  أرجع tid;
#وإلا
#تېروتنه This platform needs implementation د_3 get_thread_id()
#د_که_پای
}

//! Set the current thread heap
ساكن فراغ سېټ_thread_ډېرۍ(ډېرۍ_t *ډېرۍ) {
#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(حافظه_thread_ډېرۍ, ډېرۍ);
#وإلا
  حافظه_thread_ډېرۍ = ډېرۍ;
#د_که_پای
  إذا (ډېرۍ)
    ډېرۍ->owner_thread = get_thread_id();
}

//! Set main thread ID
خارجي فراغ rpmalloc_سېټ_main_thread(فراغ);

فراغ rpmalloc_سېټ_main_thread(فراغ) {
  _rpmalloc_main_thread_id = get_thread_id();
}

ساكن فراغ _rpmalloc_spin(فراغ) {
#إذا defined(_MSC_VER)
#إذا defined(_M_ARM64)
  __yield();
#وإلا
  _mm_pause();
#د_که_پای
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
#د_که_پای
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

#که_تعریف_شوی _WIN64
#لارښوونه comment(linker, "/INCLUDE:_tls_used")
#لارښوونه comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#لارښوونه const_seg(".CRT$XLY")

خارجي ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ثابت PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#لارښوونه const_seg()
#وإلا // _WIN64
#لارښوونه comment(linker, "/INCLUDE:__tls_used")
#لارښوونه comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#لارښوونه معلومات_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#لارښوونه معلومات_seg()
#د_که_پای // _WIN64

ساكن فراغ NTAPI _rpmalloc_thread_destructor(فراغ *ارزښت_2) {
#إذا ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  إذا (get_thread_id() == _rpmalloc_main_thread_id)
    أرجع;
#د_که_پای
  إذا (ارزښت_2)
    rpmalloc_thread_finalize(1);
}
#د_که_پای

////////////
///
/// Low level memory map/unmap
///
//////

ساكن فراغ rpmalloc_سېټ_name(فراغ *پته_2, size_t اندازه_2) {
#إذا defined(__linux__) || defined(__ANDROID__)
  ثابت حرف *name = حافظه_huge_پاڼې ? حافظه_config.huge_پاڼه_name
                                        : حافظه_config.پاڼه_name;
  إذا (پته_2 == MAP_FAILED || !name)
    أرجع;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (فراغ)prctl(PR_سېټ_VMA, PR_سېټ_VMA_ANON_NAME, (uintptr_t)پته_2, اندازه_2,
              (uintptr_t)name);
#وإلا
  (فراغ)حجم(اندازه_2);
  (فراغ)حجم(پته_2);
#د_که_پای
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
ساكن فراغ *_rpmalloc_mmap(size_t اندازه_2, size_t *دننه_واټن_2) {
  rpmalloc_assert(!(اندازه_2 % حافظه_پاڼه_اندازه), "Invalid mmap size");
  rpmalloc_assert(اندازه_2 >= حافظه_پاڼه_اندازه, "Invalid mmap size");
  فراغ *پته_2 = حافظه_config.حافظه_map(اندازه_2, دننه_واټن_2);
  إذا (EXPECTED(پته_2 != 0)) {
    rpmalloc_stat_جمع_کړه_peak(&mapped_پاڼې, (اندازه_2 >> حافظه_پاڼه_اندازه_ښويدنه),
                            mapped_پاڼې_peak);
    rpmalloc_stat_جمع_کړه(&mapped_ټول, (اندازه_2 >> حافظه_پاڼه_اندازه_ښويدنه));
  }
  أرجع پته_2;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
ساكن فراغ _rpmalloc_unmap(فراغ *پته_2, size_t اندازه_2, size_t دننه_واټن_2,
                            size_t release) {
  rpmalloc_assert(!release || (release >= اندازه_2), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= حافظه_پاڼه_اندازه),
                  "Invalid unmap size");
  إذا (release) {
    rpmalloc_assert(!(release % حافظه_پاڼه_اندازه), "Invalid unmap size");
    _rpmalloc_stat_sub(&mapped_پاڼې, (release >> حافظه_پاڼه_اندازه_ښويدنه));
    rpmalloc_stat_جمع_کړه(&unmapped_ټول, (release >> حافظه_پاڼه_اندازه_ښويدنه));
  }
  حافظه_config.حافظه_unmap(پته_2, اندازه_2, دننه_واټن_2, release);
}

//! Default implementation to map new pages to virtual memory
ساكن فراغ *_rpmalloc_mmap_os(size_t اندازه_2, size_t *دننه_واټن_2) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((اندازه_2 >= حافظه_پراختیا_اندازه) &&
                    (حافظه_پراختیا_اندازه > حافظه_map_granularity))
                       ? حافظه_پراختیا_اندازه
                       : 0;
  rpmalloc_assert(اندازه_2 >= حافظه_پاڼه_اندازه, "Invalid mmap size");
#إذا PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  فراغ *ptr = VirtualAlloc(0, اندازه_2 + padding,
                           (حافظه_huge_پاڼې ? MEM_LARGE_پاڼې : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           پاڼه_READWRITE);
  إذا (!ptr) {
    إذا (حافظه_config.map_fail_callback) {
      إذا (حافظه_config.map_fail_callback(اندازه_2 + padding))
        أرجع _rpmalloc_mmap_os(اندازه_2, دننه_واټن_2);
    } وإلا {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    أرجع 0;
  }
#وإلا
  صحيح flags = MAP_شخصي | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#إذا defined(__APPLE__) && !موخه_OS_IPHONE && !موخه_OS_SIMULATOR
  صحيح fd = (صحيح)VM_MAKE_نښان(240U);
  إذا (حافظه_huge_پاڼې)
    fd |= VM_FLAGS_SUPERPAGE_اندازه_2_MB;
  فراغ *ptr = mmap(0, اندازه_2 + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#وإلاإذا defined(MAP_HUGETLB)
  فراغ *ptr = mmap(0, اندازه_2 + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (حافظه_huge_پاڼې ? MAP_HUGETLB : 0) | flags, -1, 0);
#إذا defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  إذا ((ptr == MAP_FAILED || !ptr) && حافظه_huge_پاڼې) {
    ptr = mmap(0, اندازه_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    إذا (ptr && ptr != MAP_FAILED) {
      صحيح prm = madvise(ptr, اندازه_2 + padding, MADV_HUGEPAGE);
      (فراغ)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#د_که_پای
  rpmalloc_سېټ_name(ptr, اندازه_2 + padding);
#وإلاإذا defined(MAP_ALIGNED)
  ثابت size_t align =
      (حجم(size_t) * 8) - (size_t)(__builtin_clzl(اندازه_2 - 1));
  فراغ *ptr =
      mmap(0, اندازه_2 + padding, PROT_READ | PROT_WRITE,
           (حافظه_huge_پاڼې ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#وإلاإذا defined(MAP_ALIGN)
  caddr_t بنسټ_2 = (حافظه_huge_پاڼې ? (caddr_t)(4 << 20) : 0);
  فراغ *ptr = mmap(بنسټ_2, اندازه_2 + padding, PROT_READ | PROT_WRITE,
                   (حافظه_huge_پاڼې ? MAP_ALIGN : 0) | flags, -1, 0);
#وإلا
  فراغ *ptr = mmap(0, اندازه_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#د_که_پای
  إذا ((ptr == MAP_FAILED) || !ptr) {
    إذا (حافظه_config.map_fail_callback) {
      إذا (حافظه_config.map_fail_callback(اندازه_2 + padding))
        أرجع _rpmalloc_mmap_os(اندازه_2, دننه_واټن_2);
    } وإلا إذا (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    أرجع 0;
  }
#د_که_پای
  rpmalloc_stat_جمع_کړه(&mapped_پاڼې_os,
                     (int32_t)((اندازه_2 + padding) >> حافظه_پاڼه_اندازه_ښويدنه));
  إذا (padding) {
    size_t final_padding = padding - ((uintptr_t)ptr & ~حافظه_پراختیا_ماسک);
    rpmalloc_assert(final_padding <= حافظه_پراختیا_اندازه,
                    "Internal failure in padding");
    rpmalloc_assert(final_padding <= padding, "Internal failure in padding");
    rpmalloc_assert(!(final_padding % 8), "Internal failure in padding");
    ptr = اشاره_ګر_دننه_واټن(ptr, final_padding);
    *دننه_واټن_2 = final_padding >> 3;
  }
  rpmalloc_assert((اندازه_2 < حافظه_پراختیا_اندازه) ||
                      !((uintptr_t)ptr & ~حافظه_پراختیا_ماسک),
                  "Internal failure in padding");
  أرجع ptr;
}

//! Default implementation to unmap pages from virtual memory
ساكن فراغ _rpmalloc_unmap_os(فراغ *پته_2, size_t اندازه_2, size_t دننه_واټن_2,
                               size_t release) {
  rpmalloc_assert(release || (دننه_واټن_2 == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= حافظه_پاڼه_اندازه),
                  "Invalid unmap size");
  rpmalloc_assert(اندازه_2 >= حافظه_پاڼه_اندازه, "Invalid unmap size");
  إذا (release && دننه_واټن_2) {
    دننه_واټن_2 <<= 3;
    پته_2 = اشاره_ګر_دننه_واټن(پته_2, -(int32_t)دننه_واټن_2);
    إذا ((release >= حافظه_پراختیا_اندازه) &&
        (حافظه_پراختیا_اندازه > حافظه_map_granularity)) {
      // Padding is always one span size
      release += حافظه_پراختیا_اندازه;
    }
  }
#إذا !DISABLE_UNMAP
#إذا PLATFORM_WINDOWS
  إذا (!VirtualFree(پته_2, release ? 0 : اندازه_2,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#وإلا
  إذا (release) {
    إذا (munmap(پته_2, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } وإلا {
#إذا defined(MADV_FREE_REUSABLE)
    صحيح ret;
    طالما ((ret = madvise(پته_2, اندازه_2, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    إذا ((ret == -1) && (errno != 0)) {
#وإلاإذا defined(MADV_DONTNEED)
    إذا (madvise(پته_2, اندازه_2, MADV_DONTNEED)) {
#وإلاإذا defined(MADV_PAGEOUT)
    إذا (madvise(پته_2, اندازه_2, MADV_PAGEOUT)) {
#وإلاإذا defined(MADV_FREE)
    إذا (madvise(پته_2, اندازه_2, MADV_FREE)) {
#وإلا
    إذا (posix_madvise(پته_2, اندازه_2, POSIX_MADV_DONTNEED)) {
#د_که_پای
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#د_که_پای
#د_که_پای
  إذا (release)
    _rpmalloc_stat_sub(&mapped_پاڼې_os, release >> حافظه_پاڼه_اندازه_ښويدنه);
}

ساكن فراغ rpmalloc_پراختیا_mark_as_subspan_unless_master(پراختیا_t *master,
                                                         پراختیا_t *subspan,
                                                         size_t پراختیا_شمېر);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
ساكن پراختیا_t *_rpmalloc_global_get_reserved_spans(size_t پراختیا_شمېر) {
  پراختیا_t *پراختیا = حافظه_global_reserve;
  rpmalloc_پراختیا_mark_as_subspan_unless_master(حافظه_global_reserve_master,
                                               پراختیا, پراختیا_شمېر);
  حافظه_global_reserve_شمېر -= پراختیا_شمېر;
  إذا (حافظه_global_reserve_شمېر)
    حافظه_global_reserve =
        (پراختیا_t *)اشاره_ګر_دننه_واټن(پراختیا, پراختیا_شمېر << حافظه_پراختیا_اندازه_ښويدنه);
  وإلا
    حافظه_global_reserve = 0;
  أرجع پراختیا;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
ساكن فراغ rpmalloc_global_سېټ_reserved_spans(پراختیا_t *master, پراختیا_t *reserve,
                                                size_t reserve_پراختیا_شمېر) {
  حافظه_global_reserve_master = master;
  حافظه_global_reserve_شمېر = reserve_پراختیا_شمېر;
  حافظه_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
ساكن فراغ rpmalloc_پراختیا_double_link_list_جمع_کړه(پراختیا_t **سر, پراختیا_t *پراختیا) {
  إذا (*سر)
    (*سر)->prev = پراختیا;
  پراختیا->بل = *سر;
  *سر = پراختیا;
}

//! Pop head span from double linked list
ساكن فراغ rpmalloc_پراختیا_double_link_list_راوباسه_سر(پراختیا_t **سر,
                                                     پراختیا_t *پراختیا) {
  rpmalloc_assert(*سر == پراختیا, "Linked list corrupted");
  پراختیا = *سر;
  *سر = پراختیا->بل;
}

//! Remove a span from double linked list
ساكن فراغ rpmalloc_پراختیا_double_link_list_remove(پراختیا_t **سر,
                                                   پراختیا_t *پراختیا) {
  rpmalloc_assert(*سر, "Linked list corrupted");
  إذا (*سر == پراختیا) {
    *سر = پراختیا->بل;
  } وإلا {
    پراختیا_t *بل_پراختیا = پراختیا->بل;
    پراختیا_t *prev_پراختیا = پراختیا->prev;
    prev_پراختیا->بل = بل_پراختیا;
    إذا (EXPECTED(بل_پراختیا != 0))
      بل_پراختیا->prev = prev_پراختیا;
  }
}

////////////
///
/// Span control
///
//////

ساكن فراغ rpmalloc_ډېرۍ_cache_ورننباسه(ډېرۍ_t *ډېرۍ, پراختیا_t *پراختیا);

ساكن فراغ rpmalloc_ډېرۍ_finalize(ډېرۍ_t *ډېرۍ);

ساكن فراغ rpmalloc_ډېرۍ_سېټ_reserved_spans(ډېرۍ_t *ډېرۍ, پراختیا_t *master,
                                              پراختیا_t *reserve,
                                              size_t reserve_پراختیا_شمېر);

//! Declare the span to be a subspan and store distance from master span and
//! span count
ساكن فراغ rpmalloc_پراختیا_mark_as_subspan_unless_master(پراختیا_t *master,
                                                         پراختیا_t *subspan,
                                                         size_t پراختیا_شمېر) {
  rpmalloc_assert((subspan != master) || (subspan->flags & پراختیا_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  إذا (subspan != master) {
    subspan->flags = پراختیا_FLAG_SUBSPAN;
    subspan->دننه_واټن_from_master =
        (uint32_t)((uintptr_t)اشاره_ګر_diff(subspan, master) >>
                   حافظه_پراختیا_اندازه_ښويدنه);
    subspan->align_دننه_واټن = 0;
  }
  subspan->پراختیا_شمېر = (uint32_t)پراختیا_شمېر;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
ساكن پراختیا_t *rpmalloc_پراختیا_map_from_reserve(ډېرۍ_t *ډېرۍ,
                                               size_t پراختیا_شمېر) {
  // Update the heap span reserve
  پراختیا_t *پراختیا = ډېرۍ->پراختیا_reserve;
  ډېرۍ->پراختیا_reserve =
      (پراختیا_t *)اشاره_ګر_دننه_واټن(پراختیا, پراختیا_شمېر * حافظه_پراختیا_اندازه);
  ډېرۍ->spans_reserved -= (uint32_t)پراختیا_شمېر;

  rpmalloc_پراختیا_mark_as_subspan_unless_master(ډېرۍ->پراختیا_reserve_master, پراختیا,
                                               پراختیا_شمېر);
  إذا (پراختیا_شمېر <= LARGE_CLASS_شمېر)
    _rpmalloc_stat_inc(&ډېرۍ->پراختیا_use[پراختیا_شمېر - 1].spans_from_reserved);

  أرجع پراختیا;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
ساكن size_t rpmalloc_پراختیا_align_شمېر(size_t پراختیا_شمېر) {
  size_t request_شمېر = (پراختیا_شمېر > حافظه_پراختیا_map_شمېر)
                             ? پراختیا_شمېر
                             : حافظه_پراختیا_map_شمېر;
  إذا ((حافظه_پاڼه_اندازه > حافظه_پراختیا_اندازه) &&
      ((request_شمېر * حافظه_پراختیا_اندازه) % حافظه_پاڼه_اندازه))
    request_شمېر +=
        حافظه_پراختیا_map_شمېر - (request_شمېر % حافظه_پراختیا_map_شمېر);
  أرجع request_شمېر;
}

//! Setup a newly mapped span
ساكن فراغ rpmalloc_پراختیا_initialize(پراختیا_t *پراختیا, size_t ټول_پراختیا_شمېر,
                                      size_t پراختیا_شمېر, size_t align_دننه_واټن) {
  پراختیا->ټول_spans = (uint32_t)ټول_پراختیا_شمېر;
  پراختیا->پراختیا_شمېر = (uint32_t)پراختیا_شمېر;
  پراختیا->align_دننه_واټن = (uint32_t)align_دننه_واټن;
  پراختیا->flags = پراختیا_FLAG_MASTER;
  atomic_store32(&پراختیا->پاتې_شوی_spans, (int32_t)ټول_پراختیا_شمېر);
}

ساكن فراغ rpmalloc_پراختیا_unmap(پراختیا_t *پراختیا);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
ساكن پراختیا_t *rpmalloc_پراختیا_map_aligned_شمېر(ډېرۍ_t *ډېرۍ,
                                                size_t پراختیا_شمېر) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_پراختیا_شمېر = rpmalloc_پراختیا_align_شمېر(پراختیا_شمېر);
  size_t align_دننه_واټن = 0;
  پراختیا_t *پراختیا = (پراختیا_t *)_rpmalloc_mmap(
      aligned_پراختیا_شمېر * حافظه_پراختیا_اندازه, &align_دننه_واټن);
  إذا (!پراختیا)
    أرجع 0;
  rpmalloc_پراختیا_initialize(پراختیا, aligned_پراختیا_شمېر, پراختیا_شمېر, align_دننه_واټن);
  _rpmalloc_stat_inc(&_master_spans);
  إذا (پراختیا_شمېر <= LARGE_CLASS_شمېر)
    _rpmalloc_stat_inc(&ډېرۍ->پراختیا_use[پراختیا_شمېر - 1].spans_map_calls);
  إذا (aligned_پراختیا_شمېر > پراختیا_شمېر) {
    پراختیا_t *reserved_spans =
        (پراختیا_t *)اشاره_ګر_دننه_واټن(پراختیا, پراختیا_شمېر * حافظه_پراختیا_اندازه);
    size_t reserved_شمېر = aligned_پراختیا_شمېر - پراختیا_شمېر;
    إذا (ډېرۍ->spans_reserved) {
      rpmalloc_پراختیا_mark_as_subspan_unless_master(
          ډېرۍ->پراختیا_reserve_master, ډېرۍ->پراختیا_reserve, ډېرۍ->spans_reserved);
      rpmalloc_ډېرۍ_cache_ورننباسه(ډېرۍ, ډېرۍ->پراختیا_reserve);
    }
    إذا (reserved_شمېر > حافظه_ډېرۍ_reserve_شمېر) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&حافظه_global_قلف) == 1,
                      "Global spin lock not held as expected");
      size_t remain_شمېر = reserved_شمېر - حافظه_ډېرۍ_reserve_شمېر;
      reserved_شمېر = حافظه_ډېرۍ_reserve_شمېر;
      پراختیا_t *remain_پراختیا = (پراختیا_t *)اشاره_ګر_دننه_واټن(
          reserved_spans, reserved_شمېر * حافظه_پراختیا_اندازه);
      إذا (حافظه_global_reserve) {
        rpmalloc_پراختیا_mark_as_subspan_unless_master(
            حافظه_global_reserve_master, حافظه_global_reserve,
            حافظه_global_reserve_شمېر);
        rpmalloc_پراختیا_unmap(حافظه_global_reserve);
      }
      rpmalloc_global_سېټ_reserved_spans(پراختیا, remain_پراختیا, remain_شمېر);
    }
    rpmalloc_ډېرۍ_سېټ_reserved_spans(ډېرۍ, پراختیا, reserved_spans,
                                      reserved_شمېر);
  }
  أرجع پراختیا;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
ساكن پراختیا_t *rpmalloc_پراختیا_map(ډېرۍ_t *ډېرۍ, size_t پراختیا_شمېر) {
  إذا (پراختیا_شمېر <= ډېرۍ->spans_reserved)
    أرجع rpmalloc_پراختیا_map_from_reserve(ډېرۍ, پراختیا_شمېر);
  پراختیا_t *پراختیا = 0;
  صحيح use_global_reserve =
      (حافظه_پاڼه_اندازه > حافظه_پراختیا_اندازه) ||
      (حافظه_پراختیا_map_شمېر > حافظه_ډېرۍ_reserve_شمېر);
  إذا (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    طالما (!atomic_cas32_acquire(&حافظه_global_قلف, 1, 0))
      _rpmalloc_spin();
    إذا (حافظه_global_reserve_شمېر >= پراختیا_شمېر) {
      size_t reserve_شمېر =
          (!ډېرۍ->spans_reserved ? حافظه_ډېرۍ_reserve_شمېر : پراختیا_شمېر);
      إذا (حافظه_global_reserve_شمېر < reserve_شمېر)
        reserve_شمېر = حافظه_global_reserve_شمېر;
      پراختیا = _rpmalloc_global_get_reserved_spans(reserve_شمېر);
      إذا (پراختیا) {
        إذا (reserve_شمېر > پراختیا_شمېر) {
          پراختیا_t *reserved_پراختیا = (پراختیا_t *)اشاره_ګر_دننه_واټن(
              پراختیا, پراختیا_شمېر << حافظه_پراختیا_اندازه_ښويدنه);
          rpmalloc_ډېرۍ_سېټ_reserved_spans(ډېرۍ, حافظه_global_reserve_master,
                                            reserved_پراختیا,
                                            reserve_شمېر - پراختیا_شمېر);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        پراختیا->پراختیا_شمېر = (uint32_t)پراختیا_شمېر;
      }
    }
  }
  إذا (!پراختیا)
    پراختیا = rpmalloc_پراختیا_map_aligned_شمېر(ډېرۍ, پراختیا_شمېر);
  إذا (use_global_reserve)
    atomic_store32_release(&حافظه_global_قلف, 0);
  أرجع پراختیا;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
ساكن فراغ rpmalloc_پراختیا_unmap(پراختیا_t *پراختیا) {
  rpmalloc_assert((پراختیا->flags & پراختیا_FLAG_MASTER) ||
                      (پراختیا->flags & پراختیا_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(پراختیا->flags & پراختیا_FLAG_MASTER) ||
                      !(پراختیا->flags & پراختیا_FLAG_SUBSPAN),
                  "Span flag corrupted");

  صحيح is_master = !!(پراختیا->flags & پراختیا_FLAG_MASTER);
  پراختیا_t *master =
      is_master ? پراختیا
                : ((پراختیا_t *)اشاره_ګر_دننه_واټن(
                      پراختیا, -(intptr_t)((uintptr_t)پراختیا->دننه_واټن_from_master *
                                        حافظه_پراختیا_اندازه)));
  rpmalloc_assert(is_master || (پراختیا->flags & پراختیا_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & پراختیا_FLAG_MASTER, "Span flag corrupted");

  size_t پراختیا_شمېر = پراختیا->پراختیا_شمېر;
  إذا (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(پراختیا->align_دننه_واټن == 0, "Span align offset corrupted");
    إذا (حافظه_پراختیا_اندازه >= حافظه_پاڼه_اندازه)
      _rpmalloc_unmap(پراختیا, پراختیا_شمېر * حافظه_پراختیا_اندازه, 0, 0);
  } وإلا {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    پراختیا->flags |=
        پراختیا_FLAG_MASTER | پراختیا_FLAG_SUBSPAN | پراختیا_FLAG_UNMAPPED_MASTER;
    rpmalloc_stat_جمع_کړه(&_unmapped_master_spans, 1);
  }

  إذا (atomic_add32(&master->پاتې_شوی_spans, -(int32_t)پراختیا_شمېر) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & پراختیا_FLAG_MASTER) &&
                        !!(master->flags & پراختیا_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_شمېر = master->پراختیا_شمېر;
    إذا (حافظه_پراختیا_اندازه < حافظه_پاڼه_اندازه)
      unmap_شمېر = master->ټول_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_شمېر * حافظه_پراختیا_اندازه,
                    master->align_دننه_واټن,
                    (size_t)master->ټول_spans * حافظه_پراختیا_اندازه);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
ساكن فراغ rpmalloc_پراختیا_release_منزل_cache(ډېرۍ_t *ډېرۍ, پراختیا_t *پراختیا) {
  rpmalloc_assert(ډېرۍ == پراختیا->ډېرۍ, "Span heap pointer corrupted");
  rpmalloc_assert(پراختیا->اندازه_class < اندازه_CLASS_شمېر,
                  "Invalid span size class");
  rpmalloc_assert(پراختیا->پراختیا_شمېر == 1, "Invalid span count");
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&ډېرۍ->پراختیا_use[0].current);
#د_که_پای
  _rpmalloc_stat_dec(&ډېرۍ->اندازه_class_use[پراختیا->اندازه_class].spans_current);
  إذا (!ډېرۍ->finalize) {
    _rpmalloc_stat_inc(&ډېرۍ->پراختیا_use[0].spans_منزل_cache);
    _rpmalloc_stat_inc(&ډېرۍ->اندازه_class_use[پراختیا->اندازه_class].spans_منزل_cache);
    إذا (ډېرۍ->اندازه_class[پراختیا->اندازه_class].cache)
      rpmalloc_ډېرۍ_cache_ورننباسه(ډېرۍ,
                                  ډېرۍ->اندازه_class[پراختیا->اندازه_class].cache);
    ډېرۍ->اندازه_class[پراختیا->اندازه_class].cache = پراختیا;
  } وإلا {
    rpmalloc_پراختیا_unmap(پراختیا);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
ساكن uint32_t free_list_partial_init(فراغ **list, فراغ **لومړی_بلاک,
                                       فراغ *پاڼه_پیل, فراغ *بلاک_پیل,
                                       uint32_t بلاک_شمېر,
                                       uint32_t بلاک_اندازه_2) {
  rpmalloc_assert(بلاک_شمېر, "Internal failure");
  *لومړی_بلاک = بلاک_پیل;
  إذا (بلاک_شمېر > 1) {
    فراغ *free_بلاک = اشاره_ګر_دننه_واټن(بلاک_پیل, بلاک_اندازه_2);
    فراغ *بلاک_پای_ټکی =
        اشاره_ګر_دننه_واټن(بلاک_پیل, (size_t)بلاک_اندازه_2 * بلاک_شمېر);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    إذا (بلاک_اندازه_2 < (حافظه_پاڼه_اندازه >> 1)) {
      فراغ *پاڼه_پای_ټکی = اشاره_ګر_دننه_واټن(پاڼه_پیل, حافظه_پاڼه_اندازه);
      إذا (پاڼه_پای_ټکی < بلاک_پای_ټکی)
        بلاک_پای_ټکی = پاڼه_پای_ټکی;
    }
    *list = free_بلاک;
    بلاک_شمېر = 2;
    فراغ *بل_بلاک = اشاره_ګر_دننه_واټن(free_بلاک, بلاک_اندازه_2);
    طالما (بل_بلاک < بلاک_پای_ټکی) {
      *((فراغ **)free_بلاک) = بل_بلاک;
      free_بلاک = بل_بلاک;
      ++بلاک_شمېر;
      بل_بلاک = اشاره_ګر_دننه_واټن(بل_بلاک, بلاک_اندازه_2);
    }
    *((فراغ **)free_بلاک) = 0;
  } وإلا {
    *list = 0;
  }
  أرجع بلاک_شمېر;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
ساكن فراغ *rpmalloc_پراختیا_initialize_new(ډېرۍ_t *ډېرۍ,
                                           ډېرۍ_اندازه_class_t *ډېرۍ_اندازه_class,
                                           پراختیا_t *پراختیا, uint32_t class_idx) {
  rpmalloc_assert(پراختیا->پراختیا_شمېر == 1, "Internal failure");
  اندازه_class_t *اندازه_class = حافظه_اندازه_class + class_idx;
  پراختیا->اندازه_class = class_idx;
  پراختیا->ډېرۍ = ډېرۍ;
  پراختیا->flags &= ~پراختیا_FLAG_ALIGNED_بلاکونه;
  پراختیا->بلاک_اندازه_2 = اندازه_class->بلاک_اندازه_2;
  پراختیا->بلاک_شمېر = اندازه_class->بلاک_شمېر;
  پراختیا->free_list = 0;
  پراختیا->list_اندازه = 0;
  atomic_store_ptr_release(&پراختیا->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  فراغ *بلاک;
  پراختیا->free_list_حد =
      free_list_partial_init(&ډېرۍ_اندازه_class->free_list, &بلاک, پراختیا,
                             اشاره_ګر_دننه_واټن(پراختیا, پراختیا_سرلیک_اندازه),
                             اندازه_class->بلاک_شمېر, اندازه_class->بلاک_اندازه_2);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  إذا (پراختیا->free_list_حد < پراختیا->بلاک_شمېر) {
    rpmalloc_پراختیا_double_link_list_جمع_کړه(&ډېرۍ_اندازه_class->partial_پراختیا, پراختیا);
    پراختیا->کارول_شوی_شمېر = پراختیا->free_list_حد;
  } وإلا {
#إذا RPMALLOC_لومړی_CLASS_HEAPS
    rpmalloc_پراختیا_double_link_list_جمع_کړه(&ډېرۍ->ډک_پراختیا[class_idx], پراختیا);
#د_که_پای
    ++ډېرۍ->ډک_پراختیا_شمېر;
    پراختیا->کارول_شوی_شمېر = پراختیا->بلاک_شمېر;
  }
  أرجع بلاک;
}

ساكن فراغ rpmalloc_پراختیا_ایستل_free_list_deferred(پراختیا_t *پراختیا) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  افعل {
    پراختیا->free_list =
        atomic_exchange_ptr_acquire(&پراختیا->free_list_deferred, INVALID_اشاره_ګر);
  } طالما (پراختیا->free_list == INVALID_اشاره_ګر);
  پراختیا->کارول_شوی_شمېر -= پراختیا->list_اندازه;
  پراختیا->list_اندازه = 0;
  atomic_store_ptr_release(&پراختیا->free_list_deferred, 0);
}

ساكن صحيح rpmalloc_پراختیا_is_fully_utilized(پراختیا_t *پراختیا) {
  rpmalloc_assert(پراختیا->free_list_حد <= پراختیا->بلاک_شمېر,
                  "Span free list corrupted");
  أرجع !پراختیا->free_list && (پراختیا->free_list_حد >= پراختیا->بلاک_شمېر);
}

ساكن صحيح rpmalloc_پراختیا_finalize(ډېرۍ_t *ډېرۍ, size_t iclass, پراختیا_t *پراختیا,
                                   پراختیا_t **list_سر) {
  فراغ *free_list = ډېرۍ->اندازه_class[iclass].free_list;
  پراختیا_t *class_پراختیا = (پراختیا_t *)((uintptr_t)free_list & حافظه_پراختیا_ماسک);
  إذا (پراختیا == class_پراختیا) {
    // Adopt the heap class free list back into the span free list
    فراغ *بلاک = پراختیا->free_list;
    فراغ *وروستی_بلاک = 0;
    طالما (بلاک) {
      وروستی_بلاک = بلاک;
      بلاک = *((فراغ **)بلاک);
    }
    uint32_t free_شمېر = 0;
    بلاک = free_list;
    طالما (بلاک) {
      ++free_شمېر;
      بلاک = *((فراغ **)بلاک);
    }
    إذا (وروستی_بلاک) {
      *((فراغ **)وروستی_بلاک) = free_list;
    } وإلا {
      پراختیا->free_list = free_list;
    }
    ډېرۍ->اندازه_class[iclass].free_list = 0;
    پراختیا->کارول_شوی_شمېر -= free_شمېر;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(پراختیا->list_اندازه == پراختیا->کارول_شوی_شمېر, "Memory leak detected");
  إذا (پراختیا->list_اندازه == پراختیا->کارول_شوی_شمېر) {
    _rpmalloc_stat_dec(&ډېرۍ->پراختیا_use[0].current);
    _rpmalloc_stat_dec(&ډېرۍ->اندازه_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    إذا (list_سر)
      rpmalloc_پراختیا_double_link_list_remove(list_سر, پراختیا);
    rpmalloc_پراختیا_unmap(پراختیا);
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
  طالما (!atomic_cas32_acquire(&cache->قلف, 1, 0))
    _rpmalloc_spin();

  لكل (size_t ispan = 0; ispan < cache->شمېر; ++ispan)
    rpmalloc_پراختیا_unmap(cache->پراختیا[ispan]);
  cache->شمېر = 0;

  طالما (cache->ډکېدنه_2) {
    پراختیا_t *پراختیا = cache->ډکېدنه_2;
    cache->ډکېدنه_2 = پراختیا->بل;
    rpmalloc_پراختیا_unmap(پراختیا);
  }

  atomic_store32_release(&cache->قلف, 0);
}

ساكن فراغ rpmalloc_global_cache_ورننباسه_spans(پراختیا_t **پراختیا,
                                                size_t پراختیا_شمېر,
                                                size_t شمېر) {
  ثابت size_t cache_حد =
      (پراختیا_شمېر == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_پراختیا_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_پراختیا_LARGE_CACHE - (پراختیا_شمېر >> 1));

  global_cache_t *cache = &حافظه_پراختیا_cache[پراختیا_شمېر - 1];

  size_t ورننباسه_شمېر = شمېر;
  طالما (!atomic_cas32_acquire(&cache->قلف, 1, 0))
    _rpmalloc_spin();

#إذا ENABLE_STATISTICS
  cache->ورننباسه_شمېر += شمېر;
#د_که_پای
  إذا ((cache->شمېر + ورننباسه_شمېر) > cache_حد)
    ورننباسه_شمېر = cache_حد - cache->شمېر;

  حافظه_نقل(cache->پراختیا + cache->شمېر, پراختیا, حجم(پراختیا_t *) * ورننباسه_شمېر);
  cache->شمېر += (uint32_t)ورننباسه_شمېر;

#إذا ENABLE_UNLIMITED_CACHE
  طالما (ورننباسه_شمېر < شمېر) {
#وإلا
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  طالما ((حافظه_پاڼه_اندازه > حافظه_پراختیا_اندازه) && (ورننباسه_شمېر < شمېر)) {
#د_که_پای
    پراختیا_t *current_پراختیا = پراختیا[ورننباسه_شمېر++];
    current_پراختیا->بل = cache->ډکېدنه_2;
    cache->ډکېدنه_2 = current_پراختیا;
  }
  atomic_store32_release(&cache->قلف, 0);

  پراختیا_t *keep = 0;
  لكل (size_t ispan = ورننباسه_شمېر; ispan < شمېر; ++ispan) {
    پراختیا_t *current_پراختیا = پراختیا[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    إذا ((current_پراختیا->flags & پراختیا_FLAG_MASTER) &&
        (atomic_load32(&current_پراختیا->پاتې_شوی_spans) >
         (int32_t)current_پراختیا->پراختیا_شمېر)) {
      current_پراختیا->بل = keep;
      keep = current_پراختیا;
    } وإلا {
      rpmalloc_پراختیا_unmap(current_پراختیا);
    }
  }

  إذا (keep) {
    طالما (!atomic_cas32_acquire(&cache->قلف, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    طالما (keep) {
      لكل (; islot < cache->شمېر; ++islot) {
        پراختیا_t *current_پراختیا = cache->پراختیا[islot];
        إذا (!(current_پراختیا->flags & پراختیا_FLAG_MASTER) ||
            ((current_پراختیا->flags & پراختیا_FLAG_MASTER) &&
             (atomic_load32(&current_پراختیا->پاتې_شوی_spans) <=
              (int32_t)current_پراختیا->پراختیا_شمېر))) {
          rpmalloc_پراختیا_unmap(current_پراختیا);
          cache->پراختیا[islot] = keep;
          اكسر;
        }
      }
      إذا (islot == cache->شمېر)
        اكسر;
      keep = keep->بل;
    }

    إذا (keep) {
      پراختیا_t *لکۍ_2 = keep;
      طالما (لکۍ_2->بل)
        لکۍ_2 = لکۍ_2->بل;
      لکۍ_2->بل = cache->ډکېدنه_2;
      cache->ډکېدنه_2 = keep;
    }

    atomic_store32_release(&cache->قلف, 0);
  }
}

ساكن size_t rpmalloc_global_cache_ایستل_spans(پراختیا_t **پراختیا,
                                                   size_t پراختیا_شمېر,
                                                   size_t شمېر) {
  global_cache_t *cache = &حافظه_پراختیا_cache[پراختیا_شمېر - 1];

  size_t ایستل_شمېر = 0;
  طالما (!atomic_cas32_acquire(&cache->قلف, 1, 0))
    _rpmalloc_spin();

#إذا ENABLE_STATISTICS
  cache->ایستل_شمېر += شمېر;
#د_که_پای
  size_t want = شمېر - ایستل_شمېر;
  إذا (want > cache->شمېر)
    want = cache->شمېر;

  حافظه_نقل(پراختیا + ایستل_شمېر, cache->پراختیا + (cache->شمېر - want),
         حجم(پراختیا_t *) * want);
  cache->شمېر -= (uint32_t)want;
  ایستل_شمېر += want;

  طالما ((ایستل_شمېر < شمېر) && cache->ډکېدنه_2) {
    پراختیا_t *current_پراختیا = cache->ډکېدنه_2;
    پراختیا[ایستل_شمېر++] = current_پراختیا;
    cache->ډکېدنه_2 = current_پراختیا->بل;
  }

#إذا ENABLE_ASSERTS
  لكل (size_t ispan = 0; ispan < ایستل_شمېر; ++ispan) {
    rpmalloc_assert(پراختیا[ispan]->پراختیا_شمېر == پراختیا_شمېر,
                    "Global cache span count mismatch");
  }
#د_که_پای

  atomic_store32_release(&cache->قلف, 0);

  أرجع ایستل_شمېر;
}

#د_که_پای

////////////
///
/// Heap control
///
//////

ساكن فراغ _rpmalloc_deallocate_huge(پراختیا_t *);

//! Store the given spans as reserve in the given heap
ساكن فراغ rpmalloc_ډېرۍ_سېټ_reserved_spans(ډېرۍ_t *ډېرۍ, پراختیا_t *master,
                                              پراختیا_t *reserve,
                                              size_t reserve_پراختیا_شمېر) {
  ډېرۍ->پراختیا_reserve_master = master;
  ډېرۍ->پراختیا_reserve = reserve;
  ډېرۍ->spans_reserved = (uint32_t)reserve_پراختیا_شمېر;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
ساكن فراغ rpmalloc_ډېرۍ_cache_adopt_deferred(ډېرۍ_t *ډېرۍ,
                                                پراختیا_t **single_پراختیا) {
  پراختیا_t *پراختیا = (پراختیا_t *)((فراغ *)atomic_exchange_ptr_acquire(
      &ډېرۍ->پراختیا_free_deferred, 0));
  طالما (پراختیا) {
    پراختیا_t *بل_پراختیا = (پراختیا_t *)پراختیا->free_list;
    rpmalloc_assert(پراختیا->ډېرۍ == ډېرۍ, "Span heap pointer corrupted");
    إذا (EXPECTED(پراختیا->اندازه_class < اندازه_CLASS_شمېر)) {
      rpmalloc_assert(ډېرۍ->ډک_پراختیا_شمېر, "Heap span counter corrupted");
      --ډېرۍ->ډک_پراختیا_شمېر;
      _rpmalloc_stat_dec(&ډېرۍ->پراختیا_use[0].spans_deferred);
#إذا RPMALLOC_لومړی_CLASS_HEAPS
      rpmalloc_پراختیا_double_link_list_remove(&ډېرۍ->ډک_پراختیا[پراختیا->اندازه_class],
                                             پراختیا);
#د_که_پای
      _rpmalloc_stat_dec(&ډېرۍ->پراختیا_use[0].current);
      _rpmalloc_stat_dec(&ډېرۍ->اندازه_class_use[پراختیا->اندازه_class].spans_current);
      إذا (single_پراختیا && !*single_پراختیا)
        *single_پراختیا = پراختیا;
      وإلا
        rpmalloc_ډېرۍ_cache_ورننباسه(ډېرۍ, پراختیا);
    } وإلا {
      إذا (پراختیا->اندازه_class == اندازه_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(پراختیا);
      } وإلا {
        rpmalloc_assert(پراختیا->اندازه_class == اندازه_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(ډېرۍ->ډک_پراختیا_شمېر, "Heap span counter corrupted");
        --ډېرۍ->ډک_پراختیا_شمېر;
#إذا RPMALLOC_لومړی_CLASS_HEAPS
        rpmalloc_پراختیا_double_link_list_remove(&ډېرۍ->large_huge_پراختیا, پراختیا);
#د_که_پای
        uint32_t idx = پراختیا->پراختیا_شمېر - 1;
        _rpmalloc_stat_dec(&ډېرۍ->پراختیا_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&ډېرۍ->پراختیا_use[idx].current);
        إذا (!idx && single_پراختیا && !*single_پراختیا)
          *single_پراختیا = پراختیا;
        وإلا
          rpmalloc_ډېرۍ_cache_ورننباسه(ډېرۍ, پراختیا);
      }
    }
    پراختیا = بل_پراختیا;
  }
}

ساكن فراغ rpmalloc_ډېرۍ_unmap(ډېرۍ_t *ډېرۍ) {
  إذا (!ډېرۍ->master_ډېرۍ) {
    إذا ((ډېرۍ->finalize > 1) && !atomic_load32(&ډېرۍ->ماشوم_شمېر)) {
      پراختیا_t *پراختیا = (پراختیا_t *)((uintptr_t)ډېرۍ & حافظه_پراختیا_ماسک);
      rpmalloc_پراختیا_unmap(پراختیا);
    }
  } وإلا {
    إذا (atomic_decr32(&ډېرۍ->master_ډېرۍ->ماشوم_شمېر) == 0) {
      rpmalloc_ډېرۍ_unmap(ډېرۍ->master_ډېرۍ);
    }
  }
}

ساكن فراغ rpmalloc_ډېرۍ_global_finalize(ډېرۍ_t *ډېرۍ) {
  إذا (ډېرۍ->finalize++ > 1) {
    --ډېرۍ->finalize;
    أرجع;
  }

  rpmalloc_ډېرۍ_finalize(ډېرۍ);

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمېر; ++iclass) {
    پراختیا_cache_t *پراختیا_cache;
    إذا (!iclass)
      پراختیا_cache = &ډېرۍ->پراختیا_cache;
    وإلا
      پراختیا_cache = (پراختیا_cache_t *)(ډېرۍ->پراختیا_large_cache + (iclass - 1));
    لكل (size_t ispan = 0; ispan < پراختیا_cache->شمېر; ++ispan)
      rpmalloc_پراختیا_unmap(پراختیا_cache->پراختیا[ispan]);
    پراختیا_cache->شمېر = 0;
  }
#د_که_پای

  إذا (ډېرۍ->ډک_پراختیا_شمېر) {
    --ډېرۍ->finalize;
    أرجع;
  }

  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمېر; ++iclass) {
    إذا (ډېرۍ->اندازه_class[iclass].free_list ||
        ډېرۍ->اندازه_class[iclass].partial_پراختیا) {
      --ډېرۍ->finalize;
      أرجع;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)ډېرۍ->id % ډېرۍ_ARRAY_اندازه;
  ډېرۍ_t *list_ډېرۍ = حافظه_heaps[list_idx];
  إذا (list_ډېرۍ == ډېرۍ) {
    حافظه_heaps[list_idx] = ډېرۍ->بل_ډېرۍ;
  } وإلا {
    طالما (list_ډېرۍ->بل_ډېرۍ != ډېرۍ)
      list_ډېرۍ = list_ډېرۍ->بل_ډېرۍ;
    list_ډېرۍ->بل_ډېرۍ = ډېرۍ->بل_ډېرۍ;
  }

  rpmalloc_ډېرۍ_unmap(ډېرۍ);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
ساكن فراغ rpmalloc_ډېرۍ_cache_ورننباسه(ډېرۍ_t *ډېرۍ, پراختیا_t *پراختیا) {
  إذا (UNEXPECTED(ډېرۍ->finalize != 0)) {
    rpmalloc_پراختیا_unmap(پراختیا);
    rpmalloc_ډېرۍ_global_finalize(ډېرۍ);
    أرجع;
  }
#إذا ENABLE_THREAD_CACHE
  size_t پراختیا_شمېر = پراختیا->پراختیا_شمېر;
  _rpmalloc_stat_inc(&ډېرۍ->پراختیا_use[پراختیا_شمېر - 1].spans_منزل_cache);
  إذا (پراختیا_شمېر == 1) {
    پراختیا_cache_t *پراختیا_cache = &ډېرۍ->پراختیا_cache;
    پراختیا_cache->پراختیا[پراختیا_cache->شمېر++] = پراختیا;
    إذا (پراختیا_cache->شمېر == MAX_THREAD_پراختیا_CACHE) {
      ثابت size_t remain_شمېر =
          MAX_THREAD_پراختیا_CACHE - THREAD_پراختیا_CACHE_TRANSFER;
#إذا ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&ډېرۍ->thread_منزل_global,
                           THREAD_پراختیا_CACHE_TRANSFER * حافظه_پراختیا_اندازه);
      rpmalloc_stat_جمع_کړه(&ډېرۍ->پراختیا_use[پراختیا_شمېر - 1].spans_منزل_global,
                         THREAD_پراختیا_CACHE_TRANSFER);
      rpmalloc_global_cache_ورننباسه_spans(پراختیا_cache->پراختیا + remain_شمېر,
                                          پراختیا_شمېر,
                                          THREAD_پراختیا_CACHE_TRANSFER);
#وإلا
      لكل (size_t ispan = 0; ispan < THREAD_پراختیا_CACHE_TRANSFER; ++ispan)
        rpmalloc_پراختیا_unmap(پراختیا_cache->پراختیا[remain_شمېر + ispan]);
#د_که_پای
      پراختیا_cache->شمېر = remain_شمېر;
    }
  } وإلا {
    size_t cache_idx = پراختیا_شمېر - 2;
    پراختیا_large_cache_t *پراختیا_cache = ډېرۍ->پراختیا_large_cache + cache_idx;
    پراختیا_cache->پراختیا[پراختیا_cache->شمېر++] = پراختیا;
    ثابت size_t cache_حد =
        (MAX_THREAD_پراختیا_LARGE_CACHE - (پراختیا_شمېر >> 1));
    إذا (پراختیا_cache->شمېر == cache_حد) {
      ثابت size_t transfer_حد = 2 + (cache_حد >> 2);
      ثابت size_t transfer_شمېر =
          (THREAD_پراختیا_LARGE_CACHE_TRANSFER <= transfer_حد
               ? THREAD_پراختیا_LARGE_CACHE_TRANSFER
               : transfer_حد);
      ثابت size_t remain_شمېر = cache_حد - transfer_شمېر;
#إذا ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&ډېرۍ->thread_منزل_global,
                           transfer_شمېر * پراختیا_شمېر * حافظه_پراختیا_اندازه);
      rpmalloc_stat_جمع_کړه(&ډېرۍ->پراختیا_use[پراختیا_شمېر - 1].spans_منزل_global,
                         transfer_شمېر);
      rpmalloc_global_cache_ورننباسه_spans(پراختیا_cache->پراختیا + remain_شمېر,
                                          پراختیا_شمېر, transfer_شمېر);
#وإلا
      لكل (size_t ispan = 0; ispan < transfer_شمېر; ++ispan)
        rpmalloc_پراختیا_unmap(پراختیا_cache->پراختیا[remain_شمېر + ispan]);
#د_که_پای
      پراختیا_cache->شمېر = remain_شمېر;
    }
  }
#وإلا
  (فراغ)حجم(ډېرۍ);
  rpmalloc_پراختیا_unmap(پراختیا);
#د_که_پای
}

//! Extract the given number of spans from the different cache levels
ساكن پراختیا_t *rpmalloc_ډېرۍ_thread_cache_ایستل(ډېرۍ_t *ډېرۍ,
                                                   size_t پراختیا_شمېر) {
  پراختیا_t *پراختیا = 0;
#إذا ENABLE_THREAD_CACHE
  پراختیا_cache_t *پراختیا_cache;
  إذا (پراختیا_شمېر == 1)
    پراختیا_cache = &ډېرۍ->پراختیا_cache;
  وإلا
    پراختیا_cache = (پراختیا_cache_t *)(ډېرۍ->پراختیا_large_cache + (پراختیا_شمېر - 2));
  إذا (پراختیا_cache->شمېر) {
    _rpmalloc_stat_inc(&ډېرۍ->پراختیا_use[پراختیا_شمېر - 1].spans_from_cache);
    أرجع پراختیا_cache->پراختیا[--پراختیا_cache->شمېر];
  }
#د_که_پای
  أرجع پراختیا;
}

ساكن پراختیا_t *rpmalloc_ډېرۍ_thread_cache_deferred_ایستل(ډېرۍ_t *ډېرۍ,
                                                            size_t پراختیا_شمېر) {
  پراختیا_t *پراختیا = 0;
  إذا (پراختیا_شمېر == 1) {
    rpmalloc_ډېرۍ_cache_adopt_deferred(ډېرۍ, &پراختیا);
  } وإلا {
    rpmalloc_ډېرۍ_cache_adopt_deferred(ډېرۍ, 0);
    پراختیا = rpmalloc_ډېرۍ_thread_cache_ایستل(ډېرۍ, پراختیا_شمېر);
  }
  أرجع پراختیا;
}

ساكن پراختیا_t *rpmalloc_ډېرۍ_reserved_ایستل(ډېرۍ_t *ډېرۍ,
                                               size_t پراختیا_شمېر) {
  إذا (ډېرۍ->spans_reserved >= پراختیا_شمېر)
    أرجع rpmalloc_پراختیا_map(ډېرۍ, پراختیا_شمېر);
  أرجع 0;
}

//! Extract a span from the global cache
ساكن پراختیا_t *rpmalloc_ډېرۍ_global_cache_ایستل(ډېرۍ_t *ډېرۍ,
                                                   size_t پراختیا_شمېر) {
#إذا ENABLE_GLOBAL_CACHE
#إذا ENABLE_THREAD_CACHE
  پراختیا_cache_t *پراختیا_cache;
  size_t wanted_شمېر;
  إذا (پراختیا_شمېر == 1) {
    پراختیا_cache = &ډېرۍ->پراختیا_cache;
    wanted_شمېر = THREAD_پراختیا_CACHE_TRANSFER;
  } وإلا {
    پراختیا_cache = (پراختیا_cache_t *)(ډېرۍ->پراختیا_large_cache + (پراختیا_شمېر - 2));
    wanted_شمېر = THREAD_پراختیا_LARGE_CACHE_TRANSFER;
  }
  پراختیا_cache->شمېر = rpmalloc_global_cache_ایستل_spans(
      پراختیا_cache->پراختیا, پراختیا_شمېر, wanted_شمېر);
  إذا (پراختیا_cache->شمېر) {
    _rpmalloc_stat_add64(&ډېرۍ->global_منزل_thread,
                         پراختیا_شمېر * پراختیا_cache->شمېر * حافظه_پراختیا_اندازه);
    rpmalloc_stat_جمع_کړه(&ډېرۍ->پراختیا_use[پراختیا_شمېر - 1].spans_from_global,
                       پراختیا_cache->شمېر);
    أرجع پراختیا_cache->پراختیا[--پراختیا_cache->شمېر];
  }
#وإلا
  پراختیا_t *پراختیا = 0;
  size_t شمېر = rpmalloc_global_cache_ایستل_spans(&پراختیا, پراختیا_شمېر, 1);
  إذا (شمېر) {
    _rpmalloc_stat_add64(&ډېرۍ->global_منزل_thread,
                         پراختیا_شمېر * شمېر * حافظه_پراختیا_اندازه);
    rpmalloc_stat_جمع_کړه(&ډېرۍ->پراختیا_use[پراختیا_شمېر - 1].spans_from_global,
                       شمېر);
    أرجع پراختیا;
  }
#د_که_پای
#د_که_پای
  (فراغ)حجم(ډېرۍ);
  (فراغ)حجم(پراختیا_شمېر);
  أرجع 0;
}

ساكن فراغ rpmalloc_inc_پراختیا_statistics(ډېرۍ_t *ډېرۍ, size_t پراختیا_شمېر,
                                          uint32_t class_idx) {
  (فراغ)حجم(ډېرۍ);
  (فراغ)حجم(پراختیا_شمېر);
  (فراغ)حجم(class_idx);
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)پراختیا_شمېر - 1;
  uint32_t current_شمېر =
      (uint32_t)atomic_incr32(&ډېرۍ->پراختیا_use[idx].current);
  إذا (current_شمېر > (uint32_t)atomic_load32(&ډېرۍ->پراختیا_use[idx].لوړ))
    atomic_store32(&ډېرۍ->پراختیا_use[idx].لوړ, (int32_t)current_شمېر);
  rpmalloc_stat_جمع_کړه_peak(&ډېرۍ->اندازه_class_use[class_idx].spans_current, 1,
                          ډېرۍ->اندازه_class_use[class_idx].spans_peak);
#د_که_پای
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
ساكن پراختیا_t *
rpmalloc_ډېرۍ_ایستل_new_پراختیا(ډېرۍ_t *ډېرۍ,
                                ډېرۍ_اندازه_class_t *ډېرۍ_اندازه_class,
                                size_t پراختیا_شمېر, uint32_t class_idx) {
  پراختیا_t *پراختیا;
#إذا ENABLE_THREAD_CACHE
  إذا (ډېرۍ_اندازه_class && ډېرۍ_اندازه_class->cache) {
    پراختیا = ډېرۍ_اندازه_class->cache;
    ډېرۍ_اندازه_class->cache =
        (ډېرۍ->پراختیا_cache.شمېر
             ? ډېرۍ->پراختیا_cache.پراختیا[--ډېرۍ->پراختیا_cache.شمېر]
             : 0);
    rpmalloc_inc_پراختیا_statistics(ډېرۍ, پراختیا_شمېر, class_idx);
    أرجع پراختیا;
  }
#د_که_پای
  (فراغ)حجم(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t بنسټ_پراختیا_شمېر = پراختیا_شمېر;
  size_t حد_پراختیا_شمېر =
      (پراختیا_شمېر > 2) ? (پراختیا_شمېر + (پراختیا_شمېر >> 1)) : پراختیا_شمېر;
  إذا (حد_پراختیا_شمېر > LARGE_CLASS_شمېر)
    حد_پراختیا_شمېر = LARGE_CLASS_شمېر;
  افعل {
    پراختیا = rpmalloc_ډېرۍ_thread_cache_ایستل(ډېرۍ, پراختیا_شمېر);
    إذا (EXPECTED(پراختیا != 0)) {
      _rpmalloc_stat_inc(&ډېرۍ->اندازه_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_پراختیا_statistics(ډېرۍ, پراختیا_شمېر, class_idx);
      أرجع پراختیا;
    }
    پراختیا = rpmalloc_ډېرۍ_thread_cache_deferred_ایستل(ډېرۍ, پراختیا_شمېر);
    إذا (EXPECTED(پراختیا != 0)) {
      _rpmalloc_stat_inc(&ډېرۍ->اندازه_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_پراختیا_statistics(ډېرۍ, پراختیا_شمېر, class_idx);
      أرجع پراختیا;
    }
    پراختیا = rpmalloc_ډېرۍ_global_cache_ایستل(ډېرۍ, پراختیا_شمېر);
    إذا (EXPECTED(پراختیا != 0)) {
      _rpmalloc_stat_inc(&ډېرۍ->اندازه_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_پراختیا_statistics(ډېرۍ, پراختیا_شمېر, class_idx);
      أرجع پراختیا;
    }
    پراختیا = rpmalloc_ډېرۍ_reserved_ایستل(ډېرۍ, پراختیا_شمېر);
    إذا (EXPECTED(پراختیا != 0)) {
      _rpmalloc_stat_inc(&ډېرۍ->اندازه_class_use[class_idx].spans_from_reserved);
      rpmalloc_inc_پراختیا_statistics(ډېرۍ, پراختیا_شمېر, class_idx);
      أرجع پراختیا;
    }
    ++پراختیا_شمېر;
  } طالما (پراختیا_شمېر <= حد_پراختیا_شمېر);
  // Final fallback, map in more virtual memory
  پراختیا = rpmalloc_پراختیا_map(ډېرۍ, بنسټ_پراختیا_شمېر);
  rpmalloc_inc_پراختیا_statistics(ډېرۍ, بنسټ_پراختیا_شمېر, class_idx);
  _rpmalloc_stat_inc(&ډېرۍ->اندازه_class_use[class_idx].spans_map_calls);
  أرجع پراختیا;
}

ساكن فراغ rpmalloc_ډېرۍ_initialize(ډېرۍ_t *ډېرۍ) {
  _rpmalloc_memset_const(ډېرۍ, 0, حجم(ډېرۍ_t));
  // Get a new heap ID
  ډېرۍ->id = 1 + atomic_incr32(&حافظه_ډېرۍ_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)ډېرۍ->id % ډېرۍ_ARRAY_اندازه;
  ډېرۍ->بل_ډېرۍ = حافظه_heaps[list_idx];
  حافظه_heaps[list_idx] = ډېرۍ;
}

ساكن فراغ rpmalloc_ډېرۍ_orphan(ډېرۍ_t *ډېرۍ, صحيح لومړی_class) {
  ډېرۍ->owner_thread = (uintptr_t)-1;
#إذا RPMALLOC_لومړی_CLASS_HEAPS
  ډېرۍ_t **ډېرۍ_list =
      (لومړی_class ? &حافظه_لومړی_class_orphan_heaps : &حافظه_orphan_heaps);
#وإلا
  (فراغ)حجم(لومړی_class);
  ډېرۍ_t **ډېرۍ_list = &حافظه_orphan_heaps;
#د_که_پای
  ډېرۍ->بل_orphan = *ډېرۍ_list;
  *ډېرۍ_list = ډېرۍ;
}

//! Allocate a new heap from newly mapped memory pages
ساكن ډېرۍ_t *rpmalloc_ډېرۍ_allocate_new(فراغ) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t ډېرۍ_اندازه = حجم(ډېرۍ_t);
  size_t aligned_ډېرۍ_اندازه = 16 * ((ډېرۍ_اندازه + 15) / 16);
  size_t request_ډېرۍ_شمېر = 16;
  size_t ډېرۍ_پراختیا_شمېر = ((aligned_ډېرۍ_اندازه * request_ډېرۍ_شمېر) +
                            حجم(پراختیا_t) + حافظه_پراختیا_اندازه - 1) /
                           حافظه_پراختیا_اندازه;
  size_t بلاک_اندازه_2 = حافظه_پراختیا_اندازه * ډېرۍ_پراختیا_شمېر;
  size_t پراختیا_شمېر = ډېرۍ_پراختیا_شمېر;
  پراختیا_t *پراختیا = 0;
  // If there are global reserved spans, use these first
  إذا (حافظه_global_reserve_شمېر >= ډېرۍ_پراختیا_شمېر) {
    پراختیا = _rpmalloc_global_get_reserved_spans(ډېرۍ_پراختیا_شمېر);
  }
  إذا (!پراختیا) {
    إذا (حافظه_پاڼه_اندازه > بلاک_اندازه_2) {
      پراختیا_شمېر = حافظه_پاڼه_اندازه / حافظه_پراختیا_اندازه;
      بلاک_اندازه_2 = حافظه_پاڼه_اندازه;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_ډېرۍ_شمېر =
          (بلاک_اندازه_2 - حجم(پراختیا_t)) / aligned_ډېرۍ_اندازه;
      إذا (possible_ډېرۍ_شمېر >= (request_ډېرۍ_شمېر * 16))
        request_ډېرۍ_شمېر *= 16;
      وإلا إذا (possible_ډېرۍ_شمېر < request_ډېرۍ_شمېر)
        request_ډېرۍ_شمېر = possible_ډېرۍ_شمېر;
      ډېرۍ_پراختیا_شمېر = ((aligned_ډېرۍ_اندازه * request_ډېرۍ_شمېر) +
                         حجم(پراختیا_t) + حافظه_پراختیا_اندازه - 1) /
                        حافظه_پراختیا_اندازه;
    }

    size_t align_دننه_واټن = 0;
    پراختیا = (پراختیا_t *)_rpmalloc_mmap(بلاک_اندازه_2, &align_دننه_واټن);
    إذا (!پراختیا)
      أرجع 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    rpmalloc_پراختیا_initialize(پراختیا, پراختیا_شمېر, ډېرۍ_پراختیا_شمېر, align_دننه_واټن);
  }

  size_t remain_اندازه = حافظه_پراختیا_اندازه - حجم(پراختیا_t);
  ډېرۍ_t *ډېرۍ = (ډېرۍ_t *)اشاره_ګر_دننه_واټن(پراختیا, حجم(پراختیا_t));
  rpmalloc_ډېرۍ_initialize(ډېرۍ);

  // Put extra heaps as orphans
  size_t عدد_heaps = remain_اندازه / aligned_ډېرۍ_اندازه;
  إذا (عدد_heaps < request_ډېرۍ_شمېر)
    عدد_heaps = request_ډېرۍ_شمېر;
  atomic_store32(&ډېرۍ->ماشوم_شمېر, (int32_t)عدد_heaps - 1);
  ډېرۍ_t *extra_ډېرۍ = (ډېرۍ_t *)اشاره_ګر_دننه_واټن(ډېرۍ, aligned_ډېرۍ_اندازه);
  طالما (عدد_heaps > 1) {
    rpmalloc_ډېرۍ_initialize(extra_ډېرۍ);
    extra_ډېرۍ->master_ډېرۍ = ډېرۍ;
    rpmalloc_ډېرۍ_orphan(extra_ډېرۍ, 1);
    extra_ډېرۍ = (ډېرۍ_t *)اشاره_ګر_دننه_واټن(extra_ډېرۍ, aligned_ډېرۍ_اندازه);
    --عدد_heaps;
  }

  إذا (پراختیا_شمېر > ډېرۍ_پراختیا_شمېر) {
    // Cap reserved spans
    size_t remain_شمېر = پراختیا_شمېر - ډېرۍ_پراختیا_شمېر;
    size_t reserve_شمېر =
        (remain_شمېر > حافظه_ډېرۍ_reserve_شمېر ? حافظه_ډېرۍ_reserve_شمېر
                                                   : remain_شمېر);
    پراختیا_t *remain_پراختیا =
        (پراختیا_t *)اشاره_ګر_دننه_واټن(پراختیا, ډېرۍ_پراختیا_شمېر * حافظه_پراختیا_اندازه);
    rpmalloc_ډېرۍ_سېټ_reserved_spans(ډېرۍ, پراختیا, remain_پراختیا, reserve_شمېر);

    إذا (remain_شمېر > reserve_شمېر) {
      // Set to global reserved spans
      remain_پراختیا = (پراختیا_t *)اشاره_ګر_دننه_واټن(remain_پراختیا,
                                             reserve_شمېر * حافظه_پراختیا_اندازه);
      reserve_شمېر = remain_شمېر - reserve_شمېر;
      rpmalloc_global_سېټ_reserved_spans(پراختیا, remain_پراختیا, reserve_شمېر);
    }
  }

  أرجع ډېرۍ;
}

ساكن ډېرۍ_t *rpmalloc_ډېرۍ_ایستل_orphan(ډېرۍ_t **ډېرۍ_list) {
  ډېرۍ_t *ډېرۍ = *ډېرۍ_list;
  *ډېرۍ_list = (ډېرۍ ? ډېرۍ->بل_orphan : 0);
  أرجع ډېرۍ;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
ساكن ډېرۍ_t *rpmalloc_ډېرۍ_allocate(صحيح لومړی_class) {
  ډېرۍ_t *ډېرۍ = 0;
  طالما (!atomic_cas32_acquire(&حافظه_global_قلف, 1, 0))
    _rpmalloc_spin();
  إذا (لومړی_class == 0)
    ډېرۍ = rpmalloc_ډېرۍ_ایستل_orphan(&حافظه_orphan_heaps);
#إذا RPMALLOC_لومړی_CLASS_HEAPS
  إذا (!ډېرۍ)
    ډېرۍ = rpmalloc_ډېرۍ_ایستل_orphan(&حافظه_لومړی_class_orphan_heaps);
#د_که_پای
  إذا (!ډېرۍ)
    ډېرۍ = rpmalloc_ډېرۍ_allocate_new();
  atomic_store32_release(&حافظه_global_قلف, 0);
  إذا (ډېرۍ)
    rpmalloc_ډېرۍ_cache_adopt_deferred(ډېرۍ, 0);
  أرجع ډېرۍ;
}

ساكن فراغ rpmalloc_ډېرۍ_release(فراغ *heapptr, صحيح لومړی_class,
                                   صحيح release_cache) {
  ډېرۍ_t *ډېرۍ = (ډېرۍ_t *)heapptr;
  إذا (!ډېرۍ)
    أرجع;
  // Release thread cache spans back to global cache
  rpmalloc_ډېرۍ_cache_adopt_deferred(ډېرۍ, 0);
  إذا (release_cache || ډېرۍ->finalize) {
#إذا ENABLE_THREAD_CACHE
    لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمېر; ++iclass) {
      پراختیا_cache_t *پراختیا_cache;
      إذا (!iclass)
        پراختیا_cache = &ډېرۍ->پراختیا_cache;
      وإلا
        پراختیا_cache = (پراختیا_cache_t *)(ډېرۍ->پراختیا_large_cache + (iclass - 1));
      إذا (!پراختیا_cache->شمېر)
        استمر;
#إذا ENABLE_GLOBAL_CACHE
      إذا (ډېرۍ->finalize) {
        لكل (size_t ispan = 0; ispan < پراختیا_cache->شمېر; ++ispan)
          rpmalloc_پراختیا_unmap(پراختیا_cache->پراختیا[ispan]);
      } وإلا {
        _rpmalloc_stat_add64(&ډېرۍ->thread_منزل_global, پراختیا_cache->شمېر *
                                                          (iclass + 1) *
                                                          حافظه_پراختیا_اندازه);
        rpmalloc_stat_جمع_کړه(&ډېرۍ->پراختیا_use[iclass].spans_منزل_global,
                           پراختیا_cache->شمېر);
        rpmalloc_global_cache_ورننباسه_spans(پراختیا_cache->پراختیا, iclass + 1,
                                            پراختیا_cache->شمېر);
      }
#وإلا
      لكل (size_t ispan = 0; ispan < پراختیا_cache->شمېر; ++ispan)
        rpmalloc_پراختیا_unmap(پراختیا_cache->پراختیا[ispan]);
#د_که_پای
      پراختیا_cache->شمېر = 0;
    }
#د_که_پای
  }

  إذا (get_thread_ډېرۍ_raw() == ډېرۍ)
    سېټ_thread_ډېرۍ(0);

#إذا ENABLE_STATISTICS
  atomic_decr32(&حافظه_فعال_heaps);
  rpmalloc_assert(atomic_load32(&حافظه_فعال_heaps) >= 0,
                  "Still active heaps during finalization");
#د_که_پای

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  إذا (get_thread_id() != _rpmalloc_main_thread_id) {
    طالما (!atomic_cas32_acquire(&حافظه_global_قلف, 1, 0))
      _rpmalloc_spin();
  }
  rpmalloc_ډېرۍ_orphan(ډېرۍ, لومړی_class);
  atomic_store32_release(&حافظه_global_قلف, 0);
}

ساكن فراغ rpmalloc_ډېرۍ_release_raw(فراغ *heapptr, صحيح release_cache) {
  rpmalloc_ډېرۍ_release(heapptr, 0, release_cache);
}

ساكن فراغ rpmalloc_ډېرۍ_release_raw_fc(فراغ *heapptr) {
  rpmalloc_ډېرۍ_release_raw(heapptr, 1);
}

ساكن فراغ rpmalloc_ډېرۍ_finalize(ډېرۍ_t *ډېرۍ) {
  إذا (ډېرۍ->spans_reserved) {
    پراختیا_t *پراختیا = rpmalloc_پراختیا_map(ډېرۍ, ډېرۍ->spans_reserved);
    rpmalloc_پراختیا_unmap(پراختیا);
    ډېرۍ->spans_reserved = 0;
  }

  rpmalloc_ډېرۍ_cache_adopt_deferred(ډېرۍ, 0);

  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمېر; ++iclass) {
    إذا (ډېرۍ->اندازه_class[iclass].cache)
      rpmalloc_پراختیا_unmap(ډېرۍ->اندازه_class[iclass].cache);
    ډېرۍ->اندازه_class[iclass].cache = 0;
    پراختیا_t *پراختیا = ډېرۍ->اندازه_class[iclass].partial_پراختیا;
    طالما (پراختیا) {
      پراختیا_t *بل = پراختیا->بل;
      rpmalloc_پراختیا_finalize(ډېرۍ, iclass, پراختیا,
                              &ډېرۍ->اندازه_class[iclass].partial_پراختیا);
      پراختیا = بل;
    }
    // If class still has a free list it must be a full span
    إذا (ډېرۍ->اندازه_class[iclass].free_list) {
      پراختیا_t *class_پراختیا =
          (پراختیا_t *)((uintptr_t)ډېرۍ->اندازه_class[iclass].free_list &
                     حافظه_پراختیا_ماسک);
      پراختیا_t **list = 0;
#إذا RPMALLOC_لومړی_CLASS_HEAPS
      list = &ډېرۍ->ډک_پراختیا[iclass];
#د_که_پای
      --ډېرۍ->ډک_پراختیا_شمېر;
      إذا (!rpmalloc_پراختیا_finalize(ډېرۍ, iclass, class_پراختیا, list)) {
        إذا (list)
          rpmalloc_پراختیا_double_link_list_remove(list, class_پراختیا);
        rpmalloc_پراختیا_double_link_list_جمع_کړه(
            &ډېرۍ->اندازه_class[iclass].partial_پراختیا, class_پراختیا);
      }
    }
  }

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمېر; ++iclass) {
    پراختیا_cache_t *پراختیا_cache;
    إذا (!iclass)
      پراختیا_cache = &ډېرۍ->پراختیا_cache;
    وإلا
      پراختیا_cache = (پراختیا_cache_t *)(ډېرۍ->پراختیا_large_cache + (iclass - 1));
    لكل (size_t ispan = 0; ispan < پراختیا_cache->شمېر; ++ispan)
      rpmalloc_پراختیا_unmap(پراختیا_cache->پراختیا[ispan]);
    پراختیا_cache->شمېر = 0;
  }
#د_که_پای
  rpmalloc_assert(!atomic_بارول_ptr(&ډېرۍ->پراختیا_free_deferred),
                  "Heaps still active during finalization");
}

////////////
///
/// Allocation entry points
///
//////

//! Pop first block from a free list
ساكن فراغ *free_list_راوباسه(فراغ **list) {
  فراغ *بلاک = *list;
  *list = *((فراغ **)بلاک);
  أرجع بلاک;
}

//! Allocate a small/medium sized memory block from the given heap
ساكن فراغ *rpmalloc_allocate_from_ډېرۍ_fallback(
    ډېرۍ_t *ډېرۍ, ډېرۍ_اندازه_class_t *ډېرۍ_اندازه_class, uint32_t class_idx) {
  پراختیا_t *پراختیا = ډېرۍ_اندازه_class->partial_پراختیا;
  rpmalloc_assume(ډېرۍ != 0);
  إذا (EXPECTED(پراختیا != 0)) {
    rpmalloc_assert(پراختیا->بلاک_شمېر ==
                        حافظه_اندازه_class[پراختیا->اندازه_class].بلاک_شمېر,
                    "Span block count corrupted");
    rpmalloc_assert(!rpmalloc_پراختیا_is_fully_utilized(پراختیا),
                    "Internal failure");
    فراغ *بلاک;
    إذا (پراختیا->free_list) {
      // Span local free list is not empty, swap to size class free list
      بلاک = free_list_راوباسه(&پراختیا->free_list);
      ډېرۍ_اندازه_class->free_list = پراختیا->free_list;
      پراختیا->free_list = 0;
    } وإلا {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      فراغ *بلاک_پیل = اشاره_ګر_دننه_واټن(
          پراختیا, پراختیا_سرلیک_اندازه +
                    ((size_t)پراختیا->free_list_حد * پراختیا->بلاک_اندازه_2));
      پراختیا->free_list_حد += free_list_partial_init(
          &ډېرۍ_اندازه_class->free_list, &بلاک,
          (فراغ *)((uintptr_t)بلاک_پیل & ~(حافظه_پاڼه_اندازه - 1)),
          بلاک_پیل, پراختیا->بلاک_شمېر - پراختیا->free_list_حد,
          پراختیا->بلاک_اندازه_2);
    }
    rpmalloc_assert(پراختیا->free_list_حد <= پراختیا->بلاک_شمېر,
                    "Span block count corrupted");
    پراختیا->کارول_شوی_شمېر = پراختیا->free_list_حد;

    // Swap in deferred free list if present
    إذا (atomic_بارول_ptr(&پراختیا->free_list_deferred))
      rpmalloc_پراختیا_ایستل_free_list_deferred(پراختیا);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    إذا (!rpmalloc_پراختیا_is_fully_utilized(پراختیا))
      أرجع بلاک;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_پراختیا_double_link_list_راوباسه_سر(&ډېرۍ_اندازه_class->partial_پراختیا,
                                             پراختیا);
#إذا RPMALLOC_لومړی_CLASS_HEAPS
    rpmalloc_پراختیا_double_link_list_جمع_کړه(&ډېرۍ->ډک_پراختیا[class_idx], پراختیا);
#د_که_پای
    ++ډېرۍ->ډک_پراختیا_شمېر;
    أرجع بلاک;
  }

  // Find a span in one of the cache levels
  پراختیا = rpmalloc_ډېرۍ_ایستل_new_پراختیا(ډېرۍ, ډېرۍ_اندازه_class, 1, class_idx);
  إذا (EXPECTED(پراختیا != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    أرجع rpmalloc_پراختیا_initialize_new(ډېرۍ, ډېرۍ_اندازه_class, پراختیا,
                                         class_idx);
  }

  أرجع 0;
}

//! Allocate a small sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_small(ډېرۍ_t *ډېرۍ, size_t اندازه_2) {
  rpmalloc_assert(ډېرۍ, "No thread heap");
  // Small sizes have unique size classes
  ثابت uint32_t class_idx =
      (uint32_t)((اندازه_2 + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_ښويدنه);
  ډېرۍ_اندازه_class_t *ډېرۍ_اندازه_class = ډېرۍ->اندازه_class + class_idx;
  _rpmalloc_stat_inc_alloc(ډېرۍ, class_idx);
  إذا (EXPECTED(ډېرۍ_اندازه_class->free_list != 0))
    أرجع free_list_راوباسه(&ډېرۍ_اندازه_class->free_list);
  أرجع rpmalloc_allocate_from_ډېرۍ_fallback(ډېرۍ, ډېرۍ_اندازه_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_medium(ډېرۍ_t *ډېرۍ, size_t اندازه_2) {
  rpmalloc_assert(ډېرۍ, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ثابت uint32_t بنسټ_idx =
      (uint32_t)(SMALL_CLASS_شمېر +
                 ((اندازه_2 - (SMALL_اندازه_حد + 1)) >> MEDIUM_GRANULARITY_ښويدنه));
  ثابت uint32_t class_idx = حافظه_اندازه_class[بنسټ_idx].class_idx;
  ډېرۍ_اندازه_class_t *ډېرۍ_اندازه_class = ډېرۍ->اندازه_class + class_idx;
  _rpmalloc_stat_inc_alloc(ډېرۍ, class_idx);
  إذا (EXPECTED(ډېرۍ_اندازه_class->free_list != 0))
    أرجع free_list_راوباسه(&ډېرۍ_اندازه_class->free_list);
  أرجع rpmalloc_allocate_from_ډېرۍ_fallback(ډېرۍ, ډېرۍ_اندازه_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
ساكن فراغ *_rpmalloc_allocate_large(ډېرۍ_t *ډېرۍ, size_t اندازه_2) {
  rpmalloc_assert(ډېرۍ, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  اندازه_2 += پراختیا_سرلیک_اندازه;
  size_t پراختیا_شمېر = اندازه_2 >> حافظه_پراختیا_اندازه_ښويدنه;
  إذا (اندازه_2 & (حافظه_پراختیا_اندازه - 1))
    ++پراختیا_شمېر;

  // Find a span in one of the cache levels
  پراختیا_t *پراختیا =
      rpmalloc_ډېرۍ_ایستل_new_پراختیا(ډېرۍ, 0, پراختیا_شمېر, اندازه_CLASS_LARGE);
  إذا (!پراختیا)
    أرجع پراختیا;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(پراختیا->پراختیا_شمېر >= پراختیا_شمېر, "Internal failure");
  پراختیا->اندازه_class = اندازه_CLASS_LARGE;
  پراختیا->ډېرۍ = ډېرۍ;

#إذا RPMALLOC_لومړی_CLASS_HEAPS
  rpmalloc_پراختیا_double_link_list_جمع_کړه(&ډېرۍ->large_huge_پراختیا, پراختیا);
#د_که_پای
  ++ډېرۍ->ډک_پراختیا_شمېر;

  أرجع اشاره_ګر_دننه_واټن(پراختیا, پراختیا_سرلیک_اندازه);
}

//! Allocate a huge block by mapping memory pages directly
ساكن فراغ *_rpmalloc_allocate_huge(ډېرۍ_t *ډېرۍ, size_t اندازه_2) {
  rpmalloc_assert(ډېرۍ, "No thread heap");
  rpmalloc_ډېرۍ_cache_adopt_deferred(ډېرۍ, 0);
  اندازه_2 += پراختیا_سرلیک_اندازه;
  size_t عدد_پاڼې = اندازه_2 >> حافظه_پاڼه_اندازه_ښويدنه;
  إذا (اندازه_2 & (حافظه_پاڼه_اندازه - 1))
    ++عدد_پاڼې;
  size_t align_دننه_واټن = 0;
  پراختیا_t *پراختیا =
      (پراختیا_t *)_rpmalloc_mmap(عدد_پاڼې * حافظه_پاڼه_اندازه, &align_دننه_واټن);
  إذا (!پراختیا)
    أرجع پراختیا;

  // Store page count in span_count
  پراختیا->اندازه_class = اندازه_CLASS_HUGE;
  پراختیا->پراختیا_شمېر = (uint32_t)عدد_پاڼې;
  پراختیا->align_دننه_واټن = (uint32_t)align_دننه_واټن;
  پراختیا->ډېرۍ = ډېرۍ;
  rpmalloc_stat_جمع_کړه_peak(&huge_پاڼې_current, عدد_پاڼې, huge_پاڼې_peak);

#إذا RPMALLOC_لومړی_CLASS_HEAPS
  rpmalloc_پراختیا_double_link_list_جمع_کړه(&ډېرۍ->large_huge_پراختیا, پراختیا);
#د_که_پای
  ++ډېرۍ->ډک_پراختیا_شمېر;

  أرجع اشاره_ګر_دننه_واټن(پراختیا, پراختیا_سرلیک_اندازه);
}

//! Allocate a block of the given size
ساكن فراغ *_rpmalloc_allocate(ډېرۍ_t *ډېرۍ, size_t اندازه_2) {
  _rpmalloc_stat_add64(&ویش_counter, 1);
  إذا (EXPECTED(اندازه_2 <= SMALL_اندازه_حد))
    أرجع _rpmalloc_allocate_small(ډېرۍ, اندازه_2);
  وإلا إذا (اندازه_2 <= حافظه_medium_اندازه_حد)
    أرجع _rpmalloc_allocate_medium(ډېرۍ, اندازه_2);
  وإلا إذا (اندازه_2 <= LARGE_اندازه_حد)
    أرجع _rpmalloc_allocate_large(ډېرۍ, اندازه_2);
  أرجع _rpmalloc_allocate_huge(ډېرۍ, اندازه_2);
}

ساكن فراغ *_rpmalloc_aligned_allocate(ډېرۍ_t *ډېرۍ, size_t alignment,
                                        size_t اندازه_2) {
  إذا (alignment <= SMALL_GRANULARITY)
    أرجع _rpmalloc_allocate(ډېرۍ, اندازه_2);

#إذا ENABLE_VALIDATE_دلیلونه
  إذا ((اندازه_2 + alignment) < اندازه_2) {
    errno = EINVAL;
    أرجع 0;
  }
  إذا (alignment & (alignment - 1)) {
    errno = EINVAL;
    أرجع 0;
  }
#د_که_پای

  إذا ((alignment <= پراختیا_سرلیک_اندازه) &&
      ((اندازه_2 + پراختیا_سرلیک_اندازه) < حافظه_medium_اندازه_حد)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t مضرب_اندازه = اندازه_2 ? (اندازه_2 + (پراختیا_سرلیک_اندازه - 1)) &
                                      ~(uintptr_t)(پراختیا_سرلیک_اندازه - 1)
                                : پراختیا_سرلیک_اندازه;
    rpmalloc_assert(!(مضرب_اندازه % پراختیا_سرلیک_اندازه),
                    "Failed alignment calculation");
    إذا (مضرب_اندازه <= (اندازه_2 + alignment))
      أرجع _rpmalloc_allocate(ډېرۍ, مضرب_اندازه);
  }

  فراغ *ptr = 0;
  size_t align_ماسک = alignment - 1;
  إذا (alignment <= حافظه_پاڼه_اندازه) {
    ptr = _rpmalloc_allocate(ډېرۍ, اندازه_2 + alignment);
    إذا ((uintptr_t)ptr & align_ماسک) {
      ptr = (فراغ *)(((uintptr_t)ptr & ~(uintptr_t)align_ماسک) + alignment);
      // Mark as having aligned blocks
      پراختیا_t *پراختیا = (پراختیا_t *)((uintptr_t)ptr & حافظه_پراختیا_ماسک);
      پراختیا->flags |= پراختیا_FLAG_ALIGNED_بلاکونه;
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
  إذا (alignment >= حافظه_پراختیا_اندازه) {
    errno = EINVAL;
    أرجع 0;
  }

  size_t extra_پاڼې = alignment / حافظه_پاڼه_اندازه;

  // Since each span has a header, we will at least need one extra memory page
  size_t عدد_پاڼې = 1 + (اندازه_2 / حافظه_پاڼه_اندازه);
  إذا (اندازه_2 & (حافظه_پاڼه_اندازه - 1))
    ++عدد_پاڼې;

  إذا (extra_پاڼې > عدد_پاڼې)
    عدد_پاڼې = 1 + extra_پاڼې;

  size_t original_پاڼې = عدد_پاڼې;
  size_t حد_پاڼې = (حافظه_پراختیا_اندازه / حافظه_پاڼه_اندازه) * 2;
  إذا (حد_پاڼې < (original_پاڼې * 2))
    حد_پاڼې = original_پاڼې * 2;

  size_t mapped_اندازه, align_دننه_واټن;
  پراختیا_t *پراختیا;

retry:
  align_دننه_واټن = 0;
  mapped_اندازه = عدد_پاڼې * حافظه_پاڼه_اندازه;

  پراختیا = (پراختیا_t *)_rpmalloc_mmap(mapped_اندازه, &align_دننه_واټن);
  إذا (!پراختیا) {
    errno = ENOMEM;
    أرجع 0;
  }
  ptr = اشاره_ګر_دننه_واټن(پراختیا, پراختیا_سرلیک_اندازه);

  إذا ((uintptr_t)ptr & align_ماسک)
    ptr = (فراغ *)(((uintptr_t)ptr & ~(uintptr_t)align_ماسک) + alignment);

  إذا (((size_t)اشاره_ګر_diff(ptr, پراختیا) >= حافظه_پراختیا_اندازه) ||
      (اشاره_ګر_دننه_واټن(ptr, اندازه_2) > اشاره_ګر_دننه_واټن(پراختیا, mapped_اندازه)) ||
      (((uintptr_t)ptr & حافظه_پراختیا_ماسک) != (uintptr_t)پراختیا)) {
    _rpmalloc_unmap(پراختیا, mapped_اندازه, align_دننه_واټن, mapped_اندازه);
    ++عدد_پاڼې;
    إذا (عدد_پاڼې > حد_پاڼې) {
      errno = EINVAL;
      أرجع 0;
    }
    اذهب retry;
  }

  // Store page count in span_count
  پراختیا->اندازه_class = اندازه_CLASS_HUGE;
  پراختیا->پراختیا_شمېر = (uint32_t)عدد_پاڼې;
  پراختیا->align_دننه_واټن = (uint32_t)align_دننه_واټن;
  پراختیا->ډېرۍ = ډېرۍ;
  rpmalloc_stat_جمع_کړه_peak(&huge_پاڼې_current, عدد_پاڼې, huge_پاڼې_peak);

#إذا RPMALLOC_لومړی_CLASS_HEAPS
  rpmalloc_پراختیا_double_link_list_جمع_کړه(&ډېرۍ->large_huge_پراختیا, پراختیا);
#د_که_پای
  ++ډېرۍ->ډک_پراختیا_شمېر;

  _rpmalloc_stat_add64(&ویش_counter, 1);

  أرجع ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
ساكن فراغ rpmalloc_deallocate_مستقیم_small_or_medium(پراختیا_t *پراختیا,
                                                        فراغ *بلاک) {
  ډېرۍ_t *ډېرۍ = پراختیا->ډېرۍ;
  rpmalloc_assert(ډېرۍ->owner_thread == get_thread_id() ||
                      !ډېرۍ->owner_thread || ډېرۍ->finalize,
                  "Internal failure");
  // Add block to free list
  إذا (UNEXPECTED(rpmalloc_پراختیا_is_fully_utilized(پراختیا))) {
    پراختیا->کارول_شوی_شمېر = پراختیا->بلاک_شمېر;
#إذا RPMALLOC_لومړی_CLASS_HEAPS
    rpmalloc_پراختیا_double_link_list_remove(&ډېرۍ->ډک_پراختیا[پراختیا->اندازه_class],
                                           پراختیا);
#د_که_پای
    rpmalloc_پراختیا_double_link_list_جمع_کړه(
        &ډېرۍ->اندازه_class[پراختیا->اندازه_class].partial_پراختیا, پراختیا);
    --ډېرۍ->ډک_پراختیا_شمېر;
  }
  *((فراغ **)بلاک) = پراختیا->free_list;
  --پراختیا->کارول_شوی_شمېر;
  پراختیا->free_list = بلاک;
  إذا (UNEXPECTED(پراختیا->کارول_شوی_شمېر == پراختیا->list_اندازه)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    إذا (پراختیا->کارول_شوی_شمېر) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      فراغ *free_list;
      افعل {
        free_list = atomic_exchange_ptr_acquire(&پراختیا->free_list_deferred,
                                                INVALID_اشاره_ګر);
      } طالما (free_list == INVALID_اشاره_ګر);
      atomic_store_ptr_release(&پراختیا->free_list_deferred, free_list);
    }
    rpmalloc_پراختیا_double_link_list_remove(
        &ډېرۍ->اندازه_class[پراختیا->اندازه_class].partial_پراختیا, پراختیا);
    rpmalloc_پراختیا_release_منزل_cache(ډېرۍ, پراختیا);
  }
}

ساكن فراغ rpmalloc_deallocate_defer_free_پراختیا(ډېرۍ_t *ډېرۍ, پراختیا_t *پراختیا) {
  إذا (پراختیا->اندازه_class != اندازه_CLASS_HUGE)
    _rpmalloc_stat_inc(&ډېرۍ->پراختیا_use[پراختیا->پراختیا_شمېر - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  افعل {
    پراختیا->free_list = (فراغ *)atomic_بارول_ptr(&ډېرۍ->پراختیا_free_deferred);
  } طالما (!atomic_cas_ptr(&ډېرۍ->پراختیا_free_deferred, پراختیا, پراختیا->free_list));
}

//! Put the block in the deferred free list of the owning span
ساكن فراغ _rpmalloc_deallocate_defer_small_or_medium(پراختیا_t *پراختیا,
                                                       فراغ *بلاک) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  فراغ *free_list;
  افعل {
    free_list =
        atomic_exchange_ptr_acquire(&پراختیا->free_list_deferred, INVALID_اشاره_ګر);
  } طالما (free_list == INVALID_اشاره_ګر);
  *((فراغ **)بلاک) = free_list;
  uint32_t free_شمېر = ++پراختیا->list_اندازه;
  صحيح all_deferred_free = (free_شمېر == پراختیا->بلاک_شمېر);
  atomic_store_ptr_release(&پراختیا->free_list_deferred, بلاک);
  إذا (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    rpmalloc_deallocate_defer_free_پراختیا(پراختیا->ډېرۍ, پراختیا);
  }
}

ساكن فراغ _rpmalloc_deallocate_small_or_medium(پراختیا_t *پراختیا, فراغ *p) {
  _rpmalloc_stat_inc_free(پراختیا->ډېرۍ, پراختیا->اندازه_class);
  إذا (پراختیا->flags & پراختیا_FLAG_ALIGNED_بلاکونه) {
    // Realign pointer to block start
    فراغ *بلاکونه_پیل = اشاره_ګر_دننه_واټن(پراختیا, پراختیا_سرلیک_اندازه);
    uint32_t بلاک_دننه_واټن = (uint32_t)اشاره_ګر_diff(p, بلاکونه_پیل);
    p = اشاره_ګر_دننه_واټن(p, -(int32_t)(بلاک_دننه_واټن % پراختیا->بلاک_اندازه_2));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#إذا RPMALLOC_لومړی_CLASS_HEAPS
  صحيح defer =
      (پراختیا->ډېرۍ->owner_thread &&
       (پراختیا->ډېرۍ->owner_thread != get_thread_id()) && !پراختیا->ډېرۍ->finalize);
#وإلا
  صحيح defer =
      ((پراختیا->ډېرۍ->owner_thread != get_thread_id()) && !پراختیا->ډېرۍ->finalize);
#د_که_پای
  إذا (!defer)
    rpmalloc_deallocate_مستقیم_small_or_medium(پراختیا, p);
  وإلا
    _rpmalloc_deallocate_defer_small_or_medium(پراختیا, p);
}

//! Deallocate the given large memory block to the current heap
ساكن فراغ _rpmalloc_deallocate_large(پراختیا_t *پراختیا) {
  rpmalloc_assert(پراختیا->اندازه_class == اندازه_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(پراختیا->flags & پراختیا_FLAG_MASTER) ||
                      !(پراختیا->flags & پراختیا_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((پراختیا->flags & پراختیا_FLAG_MASTER) ||
                      (پراختیا->flags & پراختیا_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#إذا RPMALLOC_لومړی_CLASS_HEAPS
  صحيح defer =
      (پراختیا->ډېرۍ->owner_thread &&
       (پراختیا->ډېرۍ->owner_thread != get_thread_id()) && !پراختیا->ډېرۍ->finalize);
#وإلا
  صحيح defer =
      ((پراختیا->ډېرۍ->owner_thread != get_thread_id()) && !پراختیا->ډېرۍ->finalize);
#د_که_پای
  إذا (defer) {
    rpmalloc_deallocate_defer_free_پراختیا(پراختیا->ډېرۍ, پراختیا);
    أرجع;
  }
  rpmalloc_assert(پراختیا->ډېرۍ->ډک_پراختیا_شمېر, "Heap span counter corrupted");
  --پراختیا->ډېرۍ->ډک_پراختیا_شمېر;
#إذا RPMALLOC_لومړی_CLASS_HEAPS
  rpmalloc_پراختیا_double_link_list_remove(&پراختیا->ډېرۍ->large_huge_پراختیا, پراختیا);
#د_که_پای
#إذا ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = پراختیا->پراختیا_شمېر - 1;
  atomic_decr32(&پراختیا->ډېرۍ->پراختیا_use[idx].current);
#د_که_پای
  ډېرۍ_t *ډېرۍ = پراختیا->ډېرۍ;
  rpmalloc_assert(ډېرۍ, "No thread heap");
#إذا ENABLE_THREAD_CACHE
  ثابت صحيح سېټ_as_reserved =
      ((پراختیا->پراختیا_شمېر > 1) && (ډېرۍ->پراختیا_cache.شمېر == 0) &&
       !ډېرۍ->finalize && !ډېرۍ->spans_reserved);
#وإلا
  ثابت صحيح سېټ_as_reserved =
      ((پراختیا->پراختیا_شمېر > 1) && !ډېرۍ->finalize && !ډېرۍ->spans_reserved);
#د_که_پای
  إذا (سېټ_as_reserved) {
    ډېرۍ->پراختیا_reserve = پراختیا;
    ډېرۍ->spans_reserved = پراختیا->پراختیا_شمېر;
    إذا (پراختیا->flags & پراختیا_FLAG_MASTER) {
      ډېرۍ->پراختیا_reserve_master = پراختیا;
    } وإلا { // SPAN_FLAG_SUBSPAN
      پراختیا_t *master = (پراختیا_t *)اشاره_ګر_دننه_واټن(
          پراختیا,
          -(intptr_t)((size_t)پراختیا->دننه_واټن_from_master * حافظه_پراختیا_اندازه));
      ډېرۍ->پراختیا_reserve_master = master;
      rpmalloc_assert(master->flags & پراختیا_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->پاتې_شوی_spans) >=
                          (int32_t)پراختیا->پراختیا_شمېر,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&ډېرۍ->پراختیا_use[idx].spans_منزل_reserved);
  } وإلا {
    // Insert into cache list
    rpmalloc_ډېرۍ_cache_ورننباسه(ډېرۍ, پراختیا);
  }
}

//! Deallocate the given huge span
ساكن فراغ _rpmalloc_deallocate_huge(پراختیا_t *پراختیا) {
  rpmalloc_assert(پراختیا->ډېرۍ, "No span heap");
#إذا RPMALLOC_لومړی_CLASS_HEAPS
  صحيح defer =
      (پراختیا->ډېرۍ->owner_thread &&
       (پراختیا->ډېرۍ->owner_thread != get_thread_id()) && !پراختیا->ډېرۍ->finalize);
#وإلا
  صحيح defer =
      ((پراختیا->ډېرۍ->owner_thread != get_thread_id()) && !پراختیا->ډېرۍ->finalize);
#د_که_پای
  إذا (defer) {
    rpmalloc_deallocate_defer_free_پراختیا(پراختیا->ډېرۍ, پراختیا);
    أرجع;
  }
  rpmalloc_assert(پراختیا->ډېرۍ->ډک_پراختیا_شمېر, "Heap span counter corrupted");
  --پراختیا->ډېرۍ->ډک_پراختیا_شمېر;
#إذا RPMALLOC_لومړی_CLASS_HEAPS
  rpmalloc_پراختیا_double_link_list_remove(&پراختیا->ډېرۍ->large_huge_پراختیا, پراختیا);
#د_که_پای

  // Oversized allocation, page count is stored in span_count
  size_t عدد_پاڼې = پراختیا->پراختیا_شمېر;
  _rpmalloc_unmap(پراختیا, عدد_پاڼې * حافظه_پاڼه_اندازه, پراختیا->align_دننه_واټن,
                  عدد_پاڼې * حافظه_پاڼه_اندازه);
  _rpmalloc_stat_sub(&huge_پاڼې_current, عدد_پاڼې);
}

//! Deallocate the given block
ساكن فراغ _rpmalloc_deallocate(فراغ *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  پراختیا_t *پراختیا = (پراختیا_t *)((uintptr_t)p & حافظه_پراختیا_ماسک);
  إذا (UNEXPECTED(!پراختیا))
    أرجع;
  إذا (EXPECTED(پراختیا->اندازه_class < اندازه_CLASS_شمېر))
    _rpmalloc_deallocate_small_or_medium(پراختیا, p);
  وإلا إذا (پراختیا->اندازه_class == اندازه_CLASS_LARGE)
    _rpmalloc_deallocate_large(پراختیا);
  وإلا
    _rpmalloc_deallocate_huge(پراختیا);
}

////////////
///
/// Reallocation entry points
///
//////

ساكن size_t rpmalloc_usable_اندازه_2(فراغ *p);

//! Reallocate the given block to the given size
ساكن فراغ *_rpmalloc_reallocate(ډېرۍ_t *ډېرۍ, فراغ *p, size_t اندازه_2,
                                  size_t oldsize, بینښه صحيح flags) {
  إذا (p) {
    // Grab the span using guaranteed span alignment
    پراختیا_t *پراختیا = (پراختیا_t *)((uintptr_t)p & حافظه_پراختیا_ماسک);
    إذا (EXPECTED(پراختیا->اندازه_class < اندازه_CLASS_شمېر)) {
      // Small/medium sized block
      rpmalloc_assert(پراختیا->پراختیا_شمېر == 1, "Span counter corrupted");
      فراغ *بلاکونه_پیل = اشاره_ګر_دننه_واټن(پراختیا, پراختیا_سرلیک_اندازه);
      uint32_t بلاک_دننه_واټن = (uint32_t)اشاره_ګر_diff(p, بلاکونه_پیل);
      uint32_t بلاک_idx = بلاک_دننه_واټن / پراختیا->بلاک_اندازه_2;
      فراغ *بلاک =
          اشاره_ګر_دننه_واټن(بلاکونه_پیل, (size_t)بلاک_idx * پراختیا->بلاک_اندازه_2);
      إذا (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)پراختیا->بلاک_اندازه_2 - اشاره_ګر_diff(p, بلاک));
      إذا ((size_t)پراختیا->بلاک_اندازه_2 >= اندازه_2) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != بلاک) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلاک, p, oldsize);
        أرجع بلاک;
      }
    } وإلا إذا (پراختیا->اندازه_class == اندازه_CLASS_LARGE) {
      // Large block
      size_t ټول_اندازه_2 = اندازه_2 + پراختیا_سرلیک_اندازه;
      size_t عدد_spans = ټول_اندازه_2 >> حافظه_پراختیا_اندازه_ښويدنه;
      إذا (ټول_اندازه_2 & (حافظه_پراختیا_ماسک - 1))
        ++عدد_spans;
      size_t current_spans = پراختیا->پراختیا_شمېر;
      فراغ *بلاک = اشاره_ګر_دننه_واټن(پراختیا, پراختیا_سرلیک_اندازه);
      إذا (!oldsize)
        oldsize = (current_spans * حافظه_پراختیا_اندازه) -
                  (size_t)اشاره_ګر_diff(p, بلاک) - پراختیا_سرلیک_اندازه;
      إذا ((current_spans >= عدد_spans) && (ټول_اندازه_2 >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        إذا ((p != بلاک) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(بلاک, p, oldsize);
        أرجع بلاک;
      }
    } وإلا {
      // Oversized block
      size_t ټول_اندازه_2 = اندازه_2 + پراختیا_سرلیک_اندازه;
      size_t عدد_پاڼې = ټول_اندازه_2 >> حافظه_پاڼه_اندازه_ښويدنه;
      إذا (ټول_اندازه_2 & (حافظه_پاڼه_اندازه - 1))
        ++عدد_پاڼې;
      // Page count is stored in span_count
      size_t current_پاڼې = پراختیا->پراختیا_شمېر;
      فراغ *بلاک = اشاره_ګر_دننه_واټن(پراختیا, پراختیا_سرلیک_اندازه);
      إذا (!oldsize)
        oldsize = (current_پاڼې * حافظه_پاڼه_اندازه) -
                  (size_t)اشاره_ګر_diff(p, بلاک) - پراختیا_سرلیک_اندازه;
      إذا ((current_پاڼې >= عدد_پاڼې) && (عدد_پاڼې >= (current_پاڼې / 2))) {
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
  size_t ښکتنی_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_اندازه =
      (اندازه_2 > ښکتنی_bound) ? اندازه_2 : ((اندازه_2 > oldsize) ? ښکتنی_bound : اندازه_2);
  فراغ *بلاک = _rpmalloc_allocate(ډېرۍ, new_اندازه);
  إذا (p && بلاک) {
    إذا (!(flags & RPMALLOC_NO_PRESERVE))
      حافظه_نقل(بلاک, p, oldsize < new_اندازه ? oldsize : new_اندازه);
    _rpmalloc_deallocate(p);
  }

  أرجع بلاک;
}

ساكن فراغ *_rpmalloc_aligned_reallocate(ډېرۍ_t *ډېرۍ, فراغ *ptr,
                                          size_t alignment, size_t اندازه_2,
                                          size_t oldsize, بینښه صحيح flags) {
  إذا (alignment <= SMALL_GRANULARITY)
    أرجع _rpmalloc_reallocate(ډېرۍ, ptr, اندازه_2, oldsize, flags);

  صحيح no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? rpmalloc_usable_اندازه_2(ptr) : 0);
  إذا ((usablesize >= اندازه_2) && !((uintptr_t)ptr & (alignment - 1))) {
    إذا (no_alloc || (اندازه_2 >= (usablesize / 2)))
      أرجع ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  فراغ *بلاک =
      (!no_alloc ? _rpmalloc_aligned_allocate(ډېرۍ, alignment, اندازه_2) : 0);
  إذا (EXPECTED(بلاک != 0)) {
    إذا (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      إذا (!oldsize)
        oldsize = usablesize;
      حافظه_نقل(بلاک, ptr, oldsize < اندازه_2 ? oldsize : اندازه_2);
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
ساكن size_t rpmalloc_usable_اندازه_2(فراغ *p) {
  // Grab the span using guaranteed span alignment
  پراختیا_t *پراختیا = (پراختیا_t *)((uintptr_t)p & حافظه_پراختیا_ماسک);
  إذا (پراختیا->اندازه_class < اندازه_CLASS_شمېر) {
    // Small/medium block
    فراغ *بلاکونه_پیل = اشاره_ګر_دننه_واټن(پراختیا, پراختیا_سرلیک_اندازه);
    أرجع پراختیا->بلاک_اندازه_2 -
           ((size_t)اشاره_ګر_diff(p, بلاکونه_پیل) % پراختیا->بلاک_اندازه_2);
  }
  إذا (پراختیا->اندازه_class == اندازه_CLASS_LARGE) {
    // Large block
    size_t current_spans = پراختیا->پراختیا_شمېر;
    أرجع (current_spans * حافظه_پراختیا_اندازه) - (size_t)اشاره_ګر_diff(p, پراختیا);
  }
  // Oversized block, page count is stored in span_count
  size_t current_پاڼې = پراختیا->پراختیا_شمېر;
  أرجع (current_پاڼې * حافظه_پاڼه_اندازه) - (size_t)اشاره_ګر_diff(p, پراختیا);
}

//! Adjust and optimize the size class properties for the given class
ساكن فراغ rpmalloc_adjust_اندازه_class(size_t iclass) {
  size_t بلاک_اندازه_2 = حافظه_اندازه_class[iclass].بلاک_اندازه_2;
  size_t بلاک_شمېر = (حافظه_پراختیا_اندازه - پراختیا_سرلیک_اندازه) / بلاک_اندازه_2;

  حافظه_اندازه_class[iclass].بلاک_شمېر = (uint16_t)بلاک_شمېر;
  حافظه_اندازه_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  إذا (iclass >= SMALL_CLASS_شمېر) {
    size_t prevclass = iclass;
    طالما (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      إذا (حافظه_اندازه_class[prevclass].بلاک_شمېر ==
          حافظه_اندازه_class[iclass].بلاک_شمېر)
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
    حافظه_نقل(&حافظه_config, config, حجم(rpmalloc_config_t));
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
  حافظه_map_granularity = system_info.dw_ویش_Granularity;
#وإلا
  حافظه_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#د_که_پای

#إذا RPMALLOC_CONFIGURABLE
  حافظه_پاڼه_اندازه = حافظه_config.پاڼه_اندازه_2;
#وإلا
  حافظه_پاڼه_اندازه = 0;
#د_که_پای
  حافظه_huge_پاڼې = 0;
  إذا (!حافظه_پاڼه_اندازه) {
#إذا PLATFORM_WINDOWS
    حافظه_پاڼه_اندازه = system_info.dw_پاڼه_اندازه;
#وإلا
    حافظه_پاڼه_اندازه = حافظه_map_granularity;
    إذا (حافظه_config.enable_huge_پاڼې) {
#إذا defined(__linux__)
      size_t huge_پاڼه_اندازه = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      إذا (meminfo) {
        حرف کرښه_2[128];
        طالما (!huge_پاڼه_اندازه && fgets(کرښه_2, حجم(کرښه_2) - 1, meminfo)) {
          کرښه_2[حجم(کرښه_2) - 1] = 0;
          إذا (مزي_پلټنه(کرښه_2, "Hugepagesize:"))
            huge_پاڼه_اندازه = (size_t)strtol(کرښه_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      إذا (huge_پاڼه_اندازه) {
        حافظه_huge_پاڼې = 1;
        حافظه_پاڼه_اندازه = huge_پاڼه_اندازه;
        حافظه_map_granularity = huge_پاڼه_اندازه;
      }
#وإلاإذا defined(__FreeBSD__)
      صحيح rc;
      size_t sz = حجم(rc);

      إذا (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, عدم, 0) == 0 &&
          rc == 1) {
        ساكن size_t defsize = 2 * 1024 * 1024;
        صحيح nsize = 0;
        size_t sizes[4] = {0};
        حافظه_huge_پاڼې = 1;
        حافظه_پاڼه_اندازه = defsize;
        إذا ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          لكل (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            إذا (defsize < csize) {
              حافظه_پاڼه_اندازه = csize;
              اكسر;
            }
          }
        }
        حافظه_map_granularity = حافظه_پاڼه_اندازه;
      }
#وإلاإذا defined(__APPLE__) || defined(__NetBSD__)
      حافظه_huge_پاڼې = 1;
      حافظه_پاڼه_اندازه = 2 * 1024 * 1024;
      حافظه_map_granularity = حافظه_پاڼه_اندازه;
#د_که_پای
    }
#د_که_پای
  } وإلا {
    إذا (حافظه_config.enable_huge_پاڼې)
      حافظه_huge_پاڼې = 1;
  }

#إذا PLATFORM_WINDOWS
  إذا (حافظه_config.enable_huge_پاڼې) {
    HANDLE token = 0;
    size_t large_پاڼه_minimum = Get_Large_پاڼه_Minimum();
    إذا (large_پاڼه_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    إذا (token) {
      LUID luid;
      إذا (Lookup_Privilege_ارزښت(0, SE_قلف_حافظه_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, حجم(token_privileges));
        token_privileges.Privilege_شمېر = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].ځانګړتیاوې = SE_PRIVILEGE_ENABLED;
        إذا (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          إذا (Get_وروستی_Error() == ERROR_SUCCESS)
            حافظه_huge_پاڼې = 1;
        }
      }
      CloseHandle(token);
    }
    إذا (حافظه_huge_پاڼې) {
      إذا (large_پاڼه_minimum > حافظه_پاڼه_اندازه)
        حافظه_پاڼه_اندازه = large_پاڼه_minimum;
      إذا (large_پاڼه_minimum > حافظه_map_granularity)
        حافظه_map_granularity = large_پاڼه_minimum;
    }
  }
#د_که_پای

  size_t min_پراختیا_اندازه = 256;
  size_t max_پاڼه_اندازه;
#إذا UINTPTR_MAX > 0xFFFFFFFF
  max_پاڼه_اندازه = 4096ULL * 1024ULL * 1024ULL;
#وإلا
  max_پاڼه_اندازه = 4 * 1024 * 1024;
#د_که_پای
  إذا (حافظه_پاڼه_اندازه < min_پراختیا_اندازه)
    حافظه_پاڼه_اندازه = min_پراختیا_اندازه;
  إذا (حافظه_پاڼه_اندازه > max_پاڼه_اندازه)
    حافظه_پاڼه_اندازه = max_پاڼه_اندازه;
  حافظه_پاڼه_اندازه_ښويدنه = 0;
  size_t پاڼه_اندازه_بټ = حافظه_پاڼه_اندازه;
  طالما (پاڼه_اندازه_بټ != 1) {
    ++حافظه_پاڼه_اندازه_ښويدنه;
    پاڼه_اندازه_بټ >>= 1;
  }
  حافظه_پاڼه_اندازه = ((size_t)1 << حافظه_پاڼه_اندازه_ښويدنه);

#إذا RPMALLOC_CONFIGURABLE
  إذا (!حافظه_config.پراختیا_اندازه) {
    حافظه_پراختیا_اندازه = حافظه_default_پراختیا_اندازه;
    حافظه_پراختیا_اندازه_ښويدنه = حافظه_default_پراختیا_اندازه_ښويدنه;
    حافظه_پراختیا_ماسک = حافظه_default_پراختیا_ماسک;
  } وإلا {
    size_t پراختیا_اندازه = حافظه_config.پراختیا_اندازه;
    إذا (پراختیا_اندازه > (256 * 1024))
      پراختیا_اندازه = (256 * 1024);
    حافظه_پراختیا_اندازه = 4096;
    حافظه_پراختیا_اندازه_ښويدنه = 12;
    طالما (حافظه_پراختیا_اندازه < پراختیا_اندازه) {
      حافظه_پراختیا_اندازه <<= 1;
      ++حافظه_پراختیا_اندازه_ښويدنه;
    }
    حافظه_پراختیا_ماسک = ~(uintptr_t)(حافظه_پراختیا_اندازه - 1);
  }
#د_که_پای

  حافظه_پراختیا_map_شمېر =
      (حافظه_config.پراختیا_map_شمېر ? حافظه_config.پراختیا_map_شمېر
                                     : DEFAULT_پراختیا_MAP_شمېر);
  إذا ((حافظه_پراختیا_اندازه * حافظه_پراختیا_map_شمېر) < حافظه_پاڼه_اندازه)
    حافظه_پراختیا_map_شمېر = (حافظه_پاڼه_اندازه / حافظه_پراختیا_اندازه);
  إذا ((حافظه_پاڼه_اندازه >= حافظه_پراختیا_اندازه) &&
      ((حافظه_پراختیا_map_شمېر * حافظه_پراختیا_اندازه) % حافظه_پاڼه_اندازه))
    حافظه_پراختیا_map_شمېر = (حافظه_پاڼه_اندازه / حافظه_پراختیا_اندازه);
  حافظه_ډېرۍ_reserve_شمېر = (حافظه_پراختیا_map_شمېر > DEFAULT_پراختیا_MAP_شمېر)
                                   ? DEFAULT_پراختیا_MAP_شمېر
                                   : حافظه_پراختیا_map_شمېر;

  حافظه_config.پاڼه_اندازه_2 = حافظه_پاڼه_اندازه;
  حافظه_config.پراختیا_اندازه = حافظه_پراختیا_اندازه;
  حافظه_config.پراختیا_map_شمېر = حافظه_پراختیا_map_شمېر;
  حافظه_config.enable_huge_پاڼې = حافظه_huge_پاڼې;

#إذا ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  إذا (pthread_کلی_create(&حافظه_thread_ډېرۍ, rpmalloc_ډېرۍ_release_raw_fc))
    أرجع -1;
#د_که_پای
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_کلی = FlsAlloc(&_rpmalloc_thread_destructor);
#د_که_پای

  // Setup all small and medium size classes
  size_t iclass = 0;
  حافظه_اندازه_class[iclass].بلاک_اندازه_2 = SMALL_GRANULARITY;
  rpmalloc_adjust_اندازه_class(iclass);
  لكل (iclass = 1; iclass < SMALL_CLASS_شمېر; ++iclass) {
    size_t اندازه_2 = iclass * SMALL_GRANULARITY;
    حافظه_اندازه_class[iclass].بلاک_اندازه_2 = (uint32_t)اندازه_2;
    rpmalloc_adjust_اندازه_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  حافظه_medium_اندازه_حد = (حافظه_پراختیا_اندازه - پراختیا_سرلیک_اندازه) >> 1;
  إذا (حافظه_medium_اندازه_حد > MEDIUM_اندازه_حد)
    حافظه_medium_اندازه_حد = MEDIUM_اندازه_حد;
  لكل (iclass = 0; iclass < MEDIUM_CLASS_شمېر; ++iclass) {
    size_t اندازه_2 = SMALL_اندازه_حد + ((iclass + 1) * MEDIUM_GRANULARITY);
    إذا (اندازه_2 > حافظه_medium_اندازه_حد) {
      حافظه_medium_اندازه_حد =
          SMALL_اندازه_حد + (iclass * MEDIUM_GRANULARITY);
      اكسر;
    }
    حافظه_اندازه_class[SMALL_CLASS_شمېر + iclass].بلاک_اندازه_2 = (uint32_t)اندازه_2;
    rpmalloc_adjust_اندازه_class(SMALL_CLASS_شمېر + iclass);
  }

  حافظه_orphan_heaps = 0;
#إذا RPMALLOC_لومړی_CLASS_HEAPS
  حافظه_لومړی_class_orphan_heaps = 0;
#د_که_پای
#إذا ENABLE_STATISTICS
  atomic_store32(&حافظه_فعال_heaps, 0);
  atomic_store32(&mapped_پاڼې, 0);
  mapped_پاڼې_peak = 0;
  atomic_store32(&_master_spans, 0);
  atomic_store32(&mapped_ټول, 0);
  atomic_store32(&unmapped_ټول, 0);
  atomic_store32(&mapped_پاڼې_os, 0);
  atomic_store32(&huge_پاڼې_current, 0);
  huge_پاڼې_peak = 0;
#د_که_پای
  memset(حافظه_heaps, 0, حجم(حافظه_heaps));
  atomic_store32_release(&حافظه_global_قلف, 0);

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
    atomic_add32(&حافظه_global_reserve_master->پاتې_شوی_spans,
                 -(int32_t)حافظه_global_reserve_شمېر);
    حافظه_global_reserve_master = 0;
    حافظه_global_reserve_شمېر = 0;
    حافظه_global_reserve = 0;
  }
  atomic_store32_release(&حافظه_global_قلف, 0);

  // Free all thread caches and fully free spans
  لكل (size_t list_idx = 0; list_idx < ډېرۍ_ARRAY_اندازه; ++list_idx) {
    ډېرۍ_t *ډېرۍ = حافظه_heaps[list_idx];
    طالما (ډېرۍ) {
      ډېرۍ_t *بل_ډېرۍ = ډېرۍ->بل_ډېرۍ;
      ډېرۍ->finalize = 1;
      rpmalloc_ډېرۍ_global_finalize(ډېرۍ);
      ډېرۍ = بل_ډېرۍ;
    }
  }

#إذا ENABLE_GLOBAL_CACHE
  // Free global caches
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمېر; ++iclass)
    _rpmalloc_global_cache_finalize(&حافظه_پراختیا_cache[iclass]);
#د_که_پای

#إذا (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_کلی_delete(حافظه_thread_ډېرۍ);
#د_که_پای
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_کلی);
  fls_کلی = 0;
#د_که_پای
#إذا ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&mapped_پاڼې) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&mapped_پاڼې_os) == 0,
                  "Memory leak detected");
#د_که_پای

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
خارجي مضمن فراغ rpmalloc_thread_initialize(فراغ) {
  إذا (!get_thread_ډېرۍ_raw()) {
    ډېرۍ_t *ډېرۍ = rpmalloc_ډېرۍ_allocate(0);
    إذا (ډېرۍ) {
      _rpmalloc_stat_inc(&حافظه_فعال_heaps);
      سېټ_thread_ډېرۍ(ډېرۍ);
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_سېټ_ارزښت(fls_کلی, ډېرۍ);
#د_که_پای
    }
  }
}

//! Finalize thread, orphan heap
فراغ rpmalloc_thread_finalize(صحيح release_caches) {
  ډېرۍ_t *ډېرۍ = get_thread_ډېرۍ_raw();
  إذا (ډېرۍ)
    rpmalloc_ډېرۍ_release_raw(ډېرۍ, release_caches);
  سېټ_thread_ډېرۍ(0);
#إذا defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_سېټ_ارزښت(fls_کلی, 0);
#د_که_پای
}

صحيح rpmalloc_is_thread_initialized(فراغ) {
  أرجع (get_thread_ډېرۍ_raw() != 0) ? 1 : 0;
}

ثابت rpmalloc_config_t *rpmalloc_config(فراغ) { أرجع &حافظه_config; }

// Extern interface

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpmalloc(size_t اندازه_2) {
#إذا ENABLE_VALIDATE_دلیلونه
  إذا (اندازه_2 >= MAX_ALLOC_اندازه) {
    errno = EINVAL;
    أرجع 0;
  }
#د_که_پای
  ډېرۍ_t *ډېرۍ = get_thread_ډېرۍ();
  أرجع _rpmalloc_allocate(ډېرۍ, اندازه_2);
}

خارجي مضمن فراغ rpfree(فراغ *ptr) { _rpmalloc_deallocate(ptr); }

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rpcalloc(size_t عدد_2, size_t اندازه_2) {
  size_t ټول_2;
#إذا ENABLE_VALIDATE_دلیلونه
#إذا PLATFORM_WINDOWS
  صحيح err = اندازه_TMult(عدد_2, اندازه_2, &ټول_2);
  إذا ((err != S_OK) || (ټول_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(عدد_2, اندازه_2, &ټول_2);
  إذا (err || (ټول_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#د_که_پای
#وإلا
  ټول_2 = عدد_2 * اندازه_2;
#د_که_پای
  ډېرۍ_t *ډېرۍ = get_thread_ډېرۍ();
  فراغ *بلاک = _rpmalloc_allocate(ډېرۍ, ټول_2);
  إذا (بلاک)
    memset(بلاک, 0, ټول_2);
  أرجع بلاک;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *rprealloc(فراغ *ptr, size_t اندازه_2) {
#إذا ENABLE_VALIDATE_دلیلونه
  إذا (اندازه_2 >= MAX_ALLOC_اندازه) {
    errno = EINVAL;
    أرجع ptr;
  }
#د_که_پای
  ډېرۍ_t *ډېرۍ = get_thread_ډېرۍ();
  أرجع _rpmalloc_reallocate(ډېرۍ, ptr, اندازه_2, 0, 0);
}

خارجي RPMALLOC_ALLOCATOR فراغ *rpaligned_realloc(فراغ *ptr, size_t alignment,
                                                  size_t اندازه_2, size_t oldsize,
                                                  بینښه صحيح flags) {
#إذا ENABLE_VALIDATE_دلیلونه
  إذا ((اندازه_2 + alignment < اندازه_2) || (alignment > حافظه_پاڼه_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#د_که_پای
  ډېرۍ_t *ډېرۍ = get_thread_ډېرۍ();
  أرجع _rpmalloc_aligned_reallocate(ډېرۍ, ptr, alignment, اندازه_2, oldsize,
                                      flags);
}

خارجي RPMALLOC_ALLOCATOR فراغ *rpaligned_alloc(size_t alignment, size_t اندازه_2) {
  ډېرۍ_t *ډېرۍ = get_thread_ډېرۍ();
  أرجع _rpmalloc_aligned_allocate(ډېرۍ, alignment, اندازه_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpaligned_calloc(size_t alignment, size_t عدد_2, size_t اندازه_2) {
  size_t ټول_2;
#إذا ENABLE_VALIDATE_دلیلونه
#إذا PLATFORM_WINDOWS
  صحيح err = اندازه_TMult(عدد_2, اندازه_2, &ټول_2);
  إذا ((err != S_OK) || (ټول_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(عدد_2, اندازه_2, &ټول_2);
  إذا (err || (ټول_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#د_که_پای
#وإلا
  ټول_2 = عدد_2 * اندازه_2;
#د_که_پای
  فراغ *بلاک = rpaligned_alloc(alignment, ټول_2);
  إذا (بلاک)
    memset(بلاک, 0, ټول_2);
  أرجع بلاک;
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
  ډېرۍ_t *ډېرۍ = get_thread_ډېرۍ_raw();
  إذا (!ډېرۍ)
    أرجع;

  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمېر; ++iclass) {
    اندازه_class_t *اندازه_class = حافظه_اندازه_class + iclass;
    پراختیا_t *پراختیا = ډېرۍ->اندازه_class[iclass].partial_پراختیا;
    طالما (پراختیا) {
      size_t free_شمېر = پراختیا->list_اندازه;
      size_t بلاک_شمېر = اندازه_class->بلاک_شمېر;
      إذا (پراختیا->free_list_حد < بلاک_شمېر)
        بلاک_شمېر = پراختیا->free_list_حد;
      free_شمېر += (بلاک_شمېر - پراختیا->کارول_شوی_شمېر);
      stats->sizecache += free_شمېر * اندازه_class->بلاک_اندازه_2;
      پراختیا = پراختیا->بل;
    }
  }

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمېر; ++iclass) {
    پراختیا_cache_t *پراختیا_cache;
    إذا (!iclass)
      پراختیا_cache = &ډېرۍ->پراختیا_cache;
    وإلا
      پراختیا_cache = (پراختیا_cache_t *)(ډېرۍ->پراختیا_large_cache + (iclass - 1));
    stats->spancache += پراختیا_cache->شمېر * (iclass + 1) * حافظه_پراختیا_اندازه;
  }
#د_که_پای

  پراختیا_t *deferred = (پراختیا_t *)atomic_بارول_ptr(&ډېرۍ->پراختیا_free_deferred);
  طالما (deferred) {
    إذا (deferred->اندازه_class != اندازه_CLASS_HUGE)
      stats->spancache += (size_t)deferred->پراختیا_شمېر * حافظه_پراختیا_اندازه;
    deferred = (پراختیا_t *)deferred->free_list;
  }

#إذا ENABLE_STATISTICS
  stats->thread_منزل_global = (size_t)atomic_load64(&ډېرۍ->thread_منزل_global);
  stats->global_منزل_thread = (size_t)atomic_load64(&ډېرۍ->global_منزل_thread);

  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمېر; ++iclass) {
    stats->پراختیا_use[iclass].current =
        (size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].current);
    stats->پراختیا_use[iclass].peak =
        (size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].لوړ);
    stats->پراختیا_use[iclass].منزل_global =
        (size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_منزل_global);
    stats->پراختیا_use[iclass].from_global =
        (size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_from_global);
    stats->پراختیا_use[iclass].منزل_cache =
        (size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_منزل_cache);
    stats->پراختیا_use[iclass].from_cache =
        (size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_from_cache);
    stats->پراختیا_use[iclass].منزل_reserved =
        (size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_منزل_reserved);
    stats->پراختیا_use[iclass].from_reserved =
        (size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_from_reserved);
    stats->پراختیا_use[iclass].map_calls =
        (size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_map_calls);
  }
  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمېر; ++iclass) {
    stats->اندازه_use[iclass].alloc_current =
        (size_t)atomic_load32(&ډېرۍ->اندازه_class_use[iclass].alloc_current);
    stats->اندازه_use[iclass].alloc_peak =
        (size_t)ډېرۍ->اندازه_class_use[iclass].alloc_peak;
    stats->اندازه_use[iclass].alloc_ټول =
        (size_t)atomic_load32(&ډېرۍ->اندازه_class_use[iclass].alloc_ټول);
    stats->اندازه_use[iclass].free_ټول =
        (size_t)atomic_load32(&ډېرۍ->اندازه_class_use[iclass].free_ټول);
    stats->اندازه_use[iclass].spans_منزل_cache =
        (size_t)atomic_load32(&ډېرۍ->اندازه_class_use[iclass].spans_منزل_cache);
    stats->اندازه_use[iclass].spans_from_cache =
        (size_t)atomic_load32(&ډېرۍ->اندازه_class_use[iclass].spans_from_cache);
    stats->اندازه_use[iclass].spans_from_reserved = (size_t)atomic_load32(
        &ډېرۍ->اندازه_class_use[iclass].spans_from_reserved);
    stats->اندازه_use[iclass].map_calls =
        (size_t)atomic_load32(&ډېرۍ->اندازه_class_use[iclass].spans_map_calls);
  }
#د_که_پای
}

فراغ rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, حجم(rpmalloc_global_statistics_t));
#إذا ENABLE_STATISTICS
  stats->mapped = (size_t)atomic_load32(&mapped_پاڼې) * حافظه_پاڼه_اندازه;
  stats->mapped_peak = (size_t)mapped_پاڼې_peak * حافظه_پاڼه_اندازه;
  stats->mapped_ټول_2 =
      (size_t)atomic_load32(&mapped_ټول) * حافظه_پاڼه_اندازه;
  stats->unmapped_ټول_2 =
      (size_t)atomic_load32(&unmapped_ټول) * حافظه_پاڼه_اندازه;
  stats->huge_alloc =
      (size_t)atomic_load32(&huge_پاڼې_current) * حافظه_پاڼه_اندازه;
  stats->huge_alloc_peak = (size_t)huge_پاڼې_peak * حافظه_پاڼه_اندازه;
#د_که_پای
#إذا ENABLE_GLOBAL_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمېر; ++iclass) {
    global_cache_t *cache = &حافظه_پراختیا_cache[iclass];
    طالما (!atomic_cas32_acquire(&cache->قلف, 1, 0))
      _rpmalloc_spin();
    uint32_t شمېر = cache->شمېر;
#إذا ENABLE_UNLIMITED_CACHE
    پراختیا_t *current_پراختیا = cache->ډکېدنه_2;
    طالما (current_پراختیا) {
      ++شمېر;
      current_پراختیا = current_پراختیا->بل;
    }
#د_که_پای
    atomic_store32_release(&cache->قلف, 0);
    stats->cached += شمېر * (iclass + 1) * حافظه_پراختیا_اندازه;
  }
#د_که_پای
}

#إذا ENABLE_STATISTICS

ساكن فراغ حافظه_ډېرۍ_dump_statistics(ډېرۍ_t *ډېرۍ, فراغ *file) {
  fprintf(file, "Heap %d stats:\n", ډېرۍ->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمېر; ++iclass) {
    إذا (!atomic_load32(&ډېرۍ->اندازه_class_use[iclass].alloc_ټول))
      استمر;
    fprintf(
        file,
        "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu "
        "%9u\n",
        (uint32_t)iclass,
        atomic_load32(&ډېرۍ->اندازه_class_use[iclass].alloc_current),
        ډېرۍ->اندازه_class_use[iclass].alloc_peak,
        atomic_load32(&ډېرۍ->اندازه_class_use[iclass].alloc_ټول),
        atomic_load32(&ډېرۍ->اندازه_class_use[iclass].free_ټول),
        حافظه_اندازه_class[iclass].بلاک_اندازه_2,
        حافظه_اندازه_class[iclass].بلاک_شمېر,
        atomic_load32(&ډېرۍ->اندازه_class_use[iclass].spans_current),
        ډېرۍ->اندازه_class_use[iclass].spans_peak,
        ((size_t)ډېرۍ->اندازه_class_use[iclass].alloc_peak *
         (size_t)حافظه_اندازه_class[iclass].بلاک_اندازه_2) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ډېرۍ->اندازه_class_use[iclass].spans_منزل_cache) *
         حافظه_پراختیا_اندازه) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ډېرۍ->اندازه_class_use[iclass].spans_from_cache) *
         حافظه_پراختیا_اندازه) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(
             &ډېرۍ->اندازه_class_use[iclass].spans_from_reserved) *
         حافظه_پراختیا_اندازه) /
            (size_t)(1024 * 1024),
        atomic_load32(&ډېرۍ->اندازه_class_use[iclass].spans_map_calls));
  }
  fprintf(file, "Spans  Current     Peak Deferred  PeakMiB  Cached  ToCacheMiB "
                "FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB "
                "FromGlobalMiB  MmapCalls\n");
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمېر; ++iclass) {
    إذا (!atomic_load32(&ډېرۍ->پراختیا_use[iclass].لوړ) &&
        !atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_map_calls))
      استمر;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&ډېرۍ->پراختیا_use[iclass].current),
        atomic_load32(&ډېرۍ->پراختیا_use[iclass].لوړ),
        atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].لوړ) *
         (size_t)حافظه_پراختیا_اندازه * (iclass + 1)) /
            (size_t)(1024 * 1024),
#إذا ENABLE_THREAD_CACHE
        (بینښه صحيح)(!iclass ? ډېرۍ->پراختیا_cache.شمېر
                               : ډېرۍ->پراختیا_large_cache[iclass - 1].شمېر),
        ((size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_منزل_cache) *
         (iclass + 1) * حافظه_پراختیا_اندازه) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_from_cache) *
         (iclass + 1) * حافظه_پراختیا_اندازه) /
            (size_t)(1024 * 1024),
#وإلا
        0, (size_t)0, (size_t)0,
#د_که_پای
        ((size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_منزل_reserved) *
         (iclass + 1) * حافظه_پراختیا_اندازه) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_from_reserved) *
         (iclass + 1) * حافظه_پراختیا_اندازه) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_منزل_global) *
         (size_t)حافظه_پراختیا_اندازه * (iclass + 1)) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_from_global) *
         (size_t)حافظه_پراختیا_اندازه * (iclass + 1)) /
            (size_t)(1024 * 1024),
        atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_map_calls));
  }
  fprintf(file, "Full spans: %zu\n", ډېرۍ->ډک_پراختیا_شمېر);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&ډېرۍ->thread_منزل_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&ډېرۍ->global_منزل_thread) / (size_t)(1024 * 1024));
}

#د_که_پای

فراغ rpmalloc_dump_statistics(فراغ *file) {
#إذا ENABLE_STATISTICS
  لكل (size_t list_idx = 0; list_idx < ډېرۍ_ARRAY_اندازه; ++list_idx) {
    ډېرۍ_t *ډېرۍ = حافظه_heaps[list_idx];
    طالما (ډېرۍ) {
      صحيح need_dump = 0;
      لكل (size_t iclass = 0; !need_dump && (iclass < اندازه_CLASS_شمېر);
           ++iclass) {
        إذا (!atomic_load32(&ډېرۍ->اندازه_class_use[iclass].alloc_ټول)) {
          rpmalloc_assert(
              !atomic_load32(&ډېرۍ->اندازه_class_use[iclass].free_ټول),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&ډېرۍ->اندازه_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          استمر;
        }
        need_dump = 1;
      }
      لكل (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_شمېر);
           ++iclass) {
        إذا (!atomic_load32(&ډېرۍ->پراختیا_use[iclass].لوړ) &&
            !atomic_load32(&ډېرۍ->پراختیا_use[iclass].spans_map_calls))
          استمر;
        need_dump = 1;
      }
      إذا (need_dump)
        حافظه_ډېرۍ_dump_statistics(ډېرۍ, file);
      ډېرۍ = ډېرۍ->بل_ډېرۍ;
    }
  }
  fprintf(file, "Global stats:\n");
  size_t huge_current =
      (size_t)atomic_load32(&huge_پاڼې_current) * حافظه_پاڼه_اندازه;
  size_t huge_peak = (size_t)huge_پاڼې_peak * حافظه_پاڼه_اندازه;
  fprintf(file, "HugeCurrentMiB HugePeakMiB\n");
  fprintf(file, "%14zu %11zu\n", huge_current / (size_t)(1024 * 1024),
          huge_peak / (size_t)(1024 * 1024));

#إذا ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمېر; ++iclass) {
    global_cache_t *cache = حافظه_پراختیا_cache + iclass;
    size_t global_cache = (size_t)cache->شمېر * iclass * حافظه_پراختیا_اندازه;

    size_t global_ډکېدنه_cache = 0;
    پراختیا_t *پراختیا = cache->ډکېدنه_2;
    طالما (پراختیا) {
      global_ډکېدنه_cache += iclass * حافظه_پراختیا_اندازه;
      پراختیا = پراختیا->بل;
    }
    إذا (global_cache || global_ډکېدنه_cache || cache->ورننباسه_شمېر ||
        cache->ایستل_شمېر)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_ډکېدنه_cache / (size_t)(1024 * 1024),
              cache->ورننباسه_شمېر, cache->ایستل_شمېر);
  }
#د_که_پای

  size_t mapped = (size_t)atomic_load32(&mapped_پاڼې) * حافظه_پاڼه_اندازه;
  size_t mapped_os =
      (size_t)atomic_load32(&mapped_پاڼې_os) * حافظه_پاڼه_اندازه;
  size_t mapped_peak = (size_t)mapped_پاڼې_peak * حافظه_پاڼه_اندازه;
  size_t mapped_ټول_2 =
      (size_t)atomic_load32(&mapped_ټول) * حافظه_پاڼه_اندازه;
  size_t unmapped_ټول_2 =
      (size_t)atomic_load32(&unmapped_ټول) * حافظه_پاڼه_اندازه;
  fprintf(
      file,
      "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB\n");
  fprintf(file, "%9zu %11zu %13zu %14zu %16zu\n",
          mapped / (size_t)(1024 * 1024), mapped_os / (size_t)(1024 * 1024),
          mapped_peak / (size_t)(1024 * 1024),
          mapped_ټول_2 / (size_t)(1024 * 1024),
          unmapped_ټول_2 / (size_t)(1024 * 1024));

  fprintf(file, "\n");
#إذا 0
	int64_t allocated = atomic_load64(&ویش_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#د_که_پای
#د_که_پای
  (فراغ)حجم(file);
}

#إذا RPMALLOC_لومړی_CLASS_HEAPS

خارجي مضمن rpmalloc_ډېرۍ_t *rpmalloc_ډېرۍ_acquire(فراغ) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  ډېرۍ_t *ډېرۍ = rpmalloc_ډېرۍ_allocate(1);
  rpmalloc_assume(ډېرۍ != عدم);
  ډېرۍ->owner_thread = 0;
  _rpmalloc_stat_inc(&حافظه_فعال_heaps);
  أرجع ډېرۍ;
}

خارجي مضمن فراغ rpmalloc_ډېرۍ_release_2(rpmalloc_ډېرۍ_t *ډېرۍ) {
  إذا (ډېرۍ)
    rpmalloc_ډېرۍ_release(ډېرۍ, 1, 1);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ډېرۍ_alloc(rpmalloc_ډېرۍ_t *ډېرۍ, size_t اندازه_2) {
#إذا ENABLE_VALIDATE_دلیلونه
  إذا (اندازه_2 >= MAX_ALLOC_اندازه) {
    errno = EINVAL;
    أرجع 0;
  }
#د_که_پای
  أرجع _rpmalloc_allocate(ډېرۍ, اندازه_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ډېرۍ_aligned_alloc(rpmalloc_ډېرۍ_t *ډېرۍ, size_t alignment,
                            size_t اندازه_2) {
#إذا ENABLE_VALIDATE_دلیلونه
  إذا (اندازه_2 >= MAX_ALLOC_اندازه) {
    errno = EINVAL;
    أرجع 0;
  }
#د_که_پای
  أرجع _rpmalloc_aligned_allocate(ډېرۍ, alignment, اندازه_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ډېرۍ_calloc(rpmalloc_ډېرۍ_t *ډېرۍ, size_t عدد_2, size_t اندازه_2) {
  أرجع rpmalloc_ډېرۍ_aligned_calloc(ډېرۍ, 0, عدد_2, اندازه_2);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ډېرۍ_aligned_calloc(rpmalloc_ډېرۍ_t *ډېرۍ, size_t alignment,
                             size_t عدد_2, size_t اندازه_2) {
  size_t ټول_2;
#إذا ENABLE_VALIDATE_دلیلونه
#إذا PLATFORM_WINDOWS
  صحيح err = اندازه_TMult(عدد_2, اندازه_2, &ټول_2);
  إذا ((err != S_OK) || (ټول_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(عدد_2, اندازه_2, &ټول_2);
  إذا (err || (ټول_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#د_که_پای
#وإلا
  ټول_2 = عدد_2 * اندازه_2;
#د_که_پای
  فراغ *بلاک = _rpmalloc_aligned_allocate(ډېرۍ, alignment, ټول_2);
  إذا (بلاک)
    memset(بلاک, 0, ټول_2);
  أرجع بلاک;
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ډېرۍ_realloc(rpmalloc_ډېرۍ_t *ډېرۍ, فراغ *ptr, size_t اندازه_2,
                      بینښه صحيح flags) {
#إذا ENABLE_VALIDATE_دلیلونه
  إذا (اندازه_2 >= MAX_ALLOC_اندازه) {
    errno = EINVAL;
    أرجع ptr;
  }
#د_که_پای
  أرجع _rpmalloc_reallocate(ډېرۍ, ptr, اندازه_2, 0, flags);
}

خارجي مضمن RPMALLOC_ALLOCATOR فراغ *
rpmalloc_ډېرۍ_aligned_realloc(rpmalloc_ډېرۍ_t *ډېرۍ, فراغ *ptr,
                              size_t alignment, size_t اندازه_2,
                              بینښه صحيح flags) {
#إذا ENABLE_VALIDATE_دلیلونه
  إذا ((اندازه_2 + alignment < اندازه_2) || (alignment > حافظه_پاڼه_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#د_که_پای
  أرجع _rpmalloc_aligned_reallocate(ډېرۍ, ptr, alignment, اندازه_2, 0, flags);
}

خارجي مضمن فراغ rpmalloc_ډېرۍ_free(rpmalloc_ډېرۍ_t *ډېرۍ, فراغ *ptr) {
  (فراغ)حجم(ډېرۍ);
  _rpmalloc_deallocate(ptr);
}

خارجي مضمن فراغ rpmalloc_ډېرۍ_free_all(rpmalloc_ډېرۍ_t *ډېرۍ) {
  پراختیا_t *پراختیا;
  پراختیا_t *بل_پراختیا;

  rpmalloc_ډېرۍ_cache_adopt_deferred(ډېرۍ, 0);

  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمېر; ++iclass) {
    پراختیا = ډېرۍ->اندازه_class[iclass].partial_پراختیا;
    طالما (پراختیا) {
      بل_پراختیا = پراختیا->بل;
      rpmalloc_ډېرۍ_cache_ورننباسه(ډېرۍ, پراختیا);
      پراختیا = بل_پراختیا;
    }
    ډېرۍ->اندازه_class[iclass].partial_پراختیا = 0;
    پراختیا = ډېرۍ->ډک_پراختیا[iclass];
    طالما (پراختیا) {
      بل_پراختیا = پراختیا->بل;
      rpmalloc_ډېرۍ_cache_ورننباسه(ډېرۍ, پراختیا);
      پراختیا = بل_پراختیا;
    }

    پراختیا = ډېرۍ->اندازه_class[iclass].cache;
    إذا (پراختیا)
      rpmalloc_ډېرۍ_cache_ورننباسه(ډېرۍ, پراختیا);
    ډېرۍ->اندازه_class[iclass].cache = 0;
  }
  memset(ډېرۍ->اندازه_class, 0, حجم(ډېرۍ->اندازه_class));
  memset(ډېرۍ->ډک_پراختیا, 0, حجم(ډېرۍ->ډک_پراختیا));

  پراختیا = ډېرۍ->large_huge_پراختیا;
  طالما (پراختیا) {
    بل_پراختیا = پراختیا->بل;
    إذا (UNEXPECTED(پراختیا->اندازه_class == اندازه_CLASS_HUGE))
      _rpmalloc_deallocate_huge(پراختیا);
    وإلا
      rpmalloc_ډېرۍ_cache_ورننباسه(ډېرۍ, پراختیا);
    پراختیا = بل_پراختیا;
  }
  ډېرۍ->large_huge_پراختیا = 0;
  ډېرۍ->ډک_پراختیا_شمېر = 0;

#إذا ENABLE_THREAD_CACHE
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمېر; ++iclass) {
    پراختیا_cache_t *پراختیا_cache;
    إذا (!iclass)
      پراختیا_cache = &ډېرۍ->پراختیا_cache;
    وإلا
      پراختیا_cache = (پراختیا_cache_t *)(ډېرۍ->پراختیا_large_cache + (iclass - 1));
    إذا (!پراختیا_cache->شمېر)
      استمر;
#إذا ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&ډېرۍ->thread_منزل_global,
                         پراختیا_cache->شمېر * (iclass + 1) * حافظه_پراختیا_اندازه);
    rpmalloc_stat_جمع_کړه(&ډېرۍ->پراختیا_use[iclass].spans_منزل_global,
                       پراختیا_cache->شمېر);
    rpmalloc_global_cache_ورننباسه_spans(پراختیا_cache->پراختیا, iclass + 1,
                                        پراختیا_cache->شمېر);
#وإلا
    لكل (size_t ispan = 0; ispan < پراختیا_cache->شمېر; ++ispan)
      rpmalloc_پراختیا_unmap(پراختیا_cache->پراختیا[ispan]);
#د_که_پای
    پراختیا_cache->شمېر = 0;
  }
#د_که_پای

#إذا ENABLE_STATISTICS
  لكل (size_t iclass = 0; iclass < اندازه_CLASS_شمېر; ++iclass) {
    atomic_store32(&ډېرۍ->اندازه_class_use[iclass].alloc_current, 0);
    atomic_store32(&ډېرۍ->اندازه_class_use[iclass].spans_current, 0);
  }
  لكل (size_t iclass = 0; iclass < LARGE_CLASS_شمېر; ++iclass) {
    atomic_store32(&ډېرۍ->پراختیا_use[iclass].current, 0);
  }
#د_که_پای
}

خارجي مضمن فراغ rpmalloc_ډېرۍ_thread_سېټ_current(rpmalloc_ډېرۍ_t *ډېرۍ) {
  ډېرۍ_t *prev_ډېرۍ = get_thread_ډېرۍ_raw();
  إذا (prev_ډېرۍ != ډېرۍ) {
    سېټ_thread_ډېرۍ(ډېرۍ);
    إذا (prev_ډېرۍ)
      rpmalloc_ډېرۍ_release_2(prev_ډېرۍ);
  }
}

خارجي مضمن rpmalloc_ډېرۍ_t *rpmalloc_get_ډېرۍ_for_ptr(فراغ *ptr) {
  // Grab the span, and then the heap from the span
  پراختیا_t *پراختیا = (پراختیا_t *)((uintptr_t)ptr & حافظه_پراختیا_ماسک);
  إذا (پراختیا) {
    أرجع پراختیا->ډېرۍ;
  }
  أرجع 0;
}

#د_که_پای

#إذا ENABLE_PRELOAD || ENABLE_OVERRIDE

#ضمن "malloc.c"

#د_که_پای

فراغ rpmalloc_linker_reference(فراغ) { (فراغ)حجم(_rpmalloc_initialized); }
