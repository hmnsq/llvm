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

#అన్తర్భావ "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#ఒకవేళ defined(__clang__)
#సూచన clang diagnostic ignored "-Wunused-macros"
#సూచన clang diagnostic ignored "-Wunused-function"
#ఒకవేళ __has_warning("-Wreserved-identifier")
#సూచన clang diagnostic ignored "-Wreserved-identifier"
#అయితే_ముగింపు
#ఒకవేళ __has_warning("-Wstatic-in-inline")
#సూచన clang diagnostic ignored "-Wstatic-in-inline"
#అయితే_ముగింపు
#లేదావేళ defined(__GNUC__)
#సూచన GCC diagnostic ignored "-Wunused-macros"
#సూచన GCC diagnostic ignored "-Wunused-function"
#అయితే_ముగింపు

#ఒకవేళ !defined(__has_builtin)
#నిర్వచించు __has_builtin(b) 0
#అయితే_ముగింపు

#ఒకవేళ defined(__GNUC__) || defined(__clang__)

#ఒకవేళ __has_builtin(__builtin_memcpy_inline)
#నిర్వచించు _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#లేకపోతే
#నిర్వచించు _rpmalloc_memcpy_const(x, y, s)                                        \
  చేయి {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    జ్ఞాపకం_నకలు(x, y, s);                                                           \
  } అయితే (0)
#అయితే_ముగింపు

#ఒకవేళ __has_builtin(__builtin_memset_inline)
#నిర్వచించు _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#లేకపోతే
#నిర్వచించు _rpmalloc_memset_const(x, y, s)                                        \
  చేయి {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } అయితే (0)
#అయితే_ముగింపు
#లేకపోతే
#నిర్వచించు _rpmalloc_memcpy_const(x, y, s) జ్ఞాపకం_నకలు(x, y, s)
#నిర్వచించు _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#అయితే_ముగింపు

#ఒకవేళ __has_builtin(__builtin_assume)
#నిర్వచించు rpmalloc_assume(cond) __builtin_assume(cond)
#లేదావేళ defined(__GNUC__)
#నిర్వచించు rpmalloc_assume(cond)                                                  \
  చేయి {                                                                         \
    ఒకవేళ (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } అయితే (0)
#లేదావేళ defined(_MSC_VER)
#నిర్వచించు rpmalloc_assume(cond) __assume(cond)
#లేకపోతే
#నిర్వచించు rpmalloc_assume(cond) 0
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే కుప్ప_ARRAY_కొలత
//! Size of heap hashmap
#నిర్వచించు కుప్ప_ARRAY_కొలత 47
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ENABLE_THREAD_CACHE
//! Enable per-thread cache
#నిర్వచించు ENABLE_THREAD_CACHE 1
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ENABLE_GLOBAL_CACHE
//! Enable global cache shared between all threads, requires thread cache
#నిర్వచించు ENABLE_GLOBAL_CACHE 1
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ENABLE_VALIDATE_వాదనలు
//! Enable validation of args to public entry points
#నిర్వచించు ENABLE_VALIDATE_వాదనలు 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ENABLE_STATISTICS
//! Enable statistics collection
#నిర్వచించు ENABLE_STATISTICS 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ENABLE_ASSERTS
//! Enable asserts
#నిర్వచించు ENABLE_ASSERTS 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ENABLE_OVERRIDE
//! Override standard library malloc/free and new/delete entry points
#నిర్వచించు ENABLE_OVERRIDE 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ENABLE_PRELOAD
//! Support preloading
#నిర్వచించు ENABLE_PRELOAD 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే DISABLE_UNMAP
//! Disable unmapping memory pages (also enables unlimited cache)
#నిర్వచించు DISABLE_UNMAP 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ENABLE_UNLIMITED_CACHE
//! Enable unlimited global cache (no unmapping until finalization)
#నిర్వచించు ENABLE_UNLIMITED_CACHE 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ENABLE_ADAPTIVE_THREAD_CACHE
//! Enable adaptive thread cache size based on use heuristics
#నిర్వచించు ENABLE_ADAPTIVE_THREAD_CACHE 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే DEFAULT_పరిధి_MAP_లెక్క
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#నిర్వచించు DEFAULT_పరిధి_MAP_లెక్క 64
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#నిర్వచించు GLOBAL_CACHE_MULTIPLIER 8
#అయితే_ముగింపు

#ఒకవేళ DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#దోషం Must use global cache ఒకవేళ unmap is disabled
#అయితే_ముగింపు

#ఒకవేళ DISABLE_UNMAP
#నిర్వచనం_రద్దు ENABLE_UNLIMITED_CACHE
#నిర్వచించు ENABLE_UNLIMITED_CACHE 1
#అయితే_ముగింపు

#ఒకవేళ !ENABLE_GLOBAL_CACHE
#నిర్వచనం_రద్దు ENABLE_UNLIMITED_CACHE
#నిర్వచించు ENABLE_UNLIMITED_CACHE 0
#అయితే_ముగింపు

#ఒకవేళ !ENABLE_THREAD_CACHE
#నిర్వచనం_రద్దు ENABLE_ADAPTIVE_THREAD_CACHE
#నిర్వచించు ENABLE_ADAPTIVE_THREAD_CACHE 0
#అయితే_ముగింపు

#ఒకవేళ defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#నిర్వచించు PLATFORM_WINDOWS 1
#నిర్వచించు PLATFORM_POSIX 0
#లేకపోతే
#నిర్వచించు PLATFORM_WINDOWS 0
#నిర్వచించు PLATFORM_POSIX 1
#అయితే_ముగింపు

/// Platform and arch specifics
#ఒకవేళ defined(_MSC_VER) && !defined(__clang__)
#సూచన warning(disable : 5105)
#నిర్వచితం_కాకపోతే FORCEINLINE
#నిర్వచించు FORCEINLINE అంతఃవరుస __forceinline
#అయితే_ముగింపు
#నిర్వచించు _Static_assert static_assert
#లేకపోతే
#నిర్వచితం_కాకపోతే FORCEINLINE
#నిర్వచించు FORCEINLINE అంతఃవరుస __attribute__((__always_inline__))
#అయితే_ముగింపు
#అయితే_ముగింపు
#ఒకవేళ PLATFORM_WINDOWS
#నిర్వచితం_కాకపోతే WIN32_LEAN_AND_సగటు
#నిర్వచించు WIN32_LEAN_AND_సగటు
#అయితే_ముగింపు
#అన్తర్భావ <windows.h>
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
#అన్తర్భావ <intsafe.h>
#అయితే_ముగింపు
#లేకపోతే
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <unistd.h>
#ఒకవేళ defined(__linux__) || defined(__ANDROID__)
#అన్తర్భావ <sys/prctl.h>
#ఒకవేళ !defined(PR_సమితి_VMA)
#నిర్వచించు PR_సమితి_VMA 0x53564d41
#నిర్వచించు PR_సమితి_VMA_ANON_NAME 0
#అయితే_ముగింపు
#అయితే_ముగింపు
#ఒకవేళ defined(__APPLE__)
#అన్తర్భావ <TargetConditionals.h>
#ఒకవేళ !లక్ష్యం_OS_IPHONE && !లక్ష్యం_OS_SIMULATOR
#అన్తర్భావ <mach/mach_vm.h>
#అన్తర్భావ <mach/vm_statistics.h>
#అయితే_ముగింపు
#అన్తర్భావ <pthread.h>
#అయితే_ముగింపు
#ఒకవేళ defined(__HAIKU__) || defined(__TINYC__)
#అన్తర్భావ <pthread.h>
#అయితే_ముగింపు
#అయితే_ముగింపు

#అన్తర్భావ <errno.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <string.h>

#ఒకవేళ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#అన్తర్భావ <fibersapi.h>
స్థిర DWORD fls_కీలకం;
#అయితే_ముగింపు

#ఒకవేళ PLATFORM_POSIX
#అన్తర్భావ <sched.h>
#అన్తర్భావ <sys/mman.h>
#నిర్వచితమైతే __FreeBSD__
#అన్తర్భావ <sys/sysctl.h>
#నిర్వచించు MAP_HUGETLB MAP_ALIGNED_SUPER
#నిర్వచితం_కాకపోతే PROT_MAX
#నిర్వచించు PROT_MAX(f) 0
#అయితే_ముగింపు
#లేకపోతే
#నిర్వచించు PROT_MAX(f) 0
#అయితే_ముగింపు
#నిర్వచితమైతే __sun
బాహ్య పూర్ణాంకం madvise(caddr_t, size_t, పూర్ణాంకం);
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే MAP_UNINITIALIZED
#నిర్వచించు MAP_UNINITIALIZED 0
#అయితే_ముగింపు
#అయితే_ముగింపు
#అన్తర్భావ <errno.h>

#ఒకవేళ ENABLE_ASSERTS
#నిర్వచనం_రద్దు NDEBUG
#ఒకవేళ defined(_MSC_VER) && !defined(_DEBUG)
#నిర్వచించు _DEBUG
#అయితే_ముగింపు
#అన్తర్భావ <assert.h>
#నిర్వచించు RPMALLOC_TOSTRING_M(x) #x
#నిర్వచించు RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#నిర్వచించు rpmalloc_assert(truth, సందేశం_2)                                        \
  చేయి {                                                                         \
    ఒకవేళ (!(truth)) {                                                            \
      ఒకవేళ (జ్ఞాపకం_config.error_callback) {                                     \
        జ్ఞాపకం_config.error_callback(సందేశం_2 " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } లేకపోతే {                                                                 \
        ప్రతిజ్ఞా((truth) && సందేశం_2);                                            \
      }                                                                        \
    }                                                                          \
  } అయితే (0)
#లేకపోతే
#నిర్వచించు rpmalloc_assert(truth, సందేశం_2)                                        \
  చేయి {                                                                         \
  } అయితే (0)
#అయితే_ముగింపు
#ఒకవేళ ENABLE_STATISTICS
#అన్తర్భావ <stdio.h>
#అయితే_ముగింపు

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#ఒకవేళ defined(_MSC_VER) && !defined(__clang__)

రకనిర్వచనం అస్థిర దీర్ఘ atomic32_t;
రకనిర్వచనం అస్థిర దీర్ఘ దీర్ఘ atomic64_t;
రకనిర్వచనం అస్థిర శూన్య *atomicptr_t;

స్థిర FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  తిరిగి (int32_t)InterlockedOr(src, 0);
}
స్థిర FORCEINLINE శూన్య atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
స్థిర FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  తిరిగి (int32_t)InterlockedIncrement(val);
}
స్థిర FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  తిరిగి (int32_t)InterlockedDecrement(val);
}
స్థిర FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t కూడు) {
  తిరిగి (int32_t)Interlocked_Exchange_కూడు(val, కూడు) + కూడు;
}
స్థిర FORCEINLINE పూర్ణాంకం atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  తిరిగి (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
స్థిర FORCEINLINE శూన్య atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
స్థిర FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  తిరిగి (int64_t)InterlockedOr64(src, 0);
}
స్థిర FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t కూడు) {
  తిరిగి (int64_t)InterlockedExchangeAdd64(val, కూడు) + కూడు;
}
స్థిర FORCEINLINE శూన్య *atomic_ఎక్కించు_ptr(atomicptr_t *src) {
  తిరిగి Interlocked_Compare_Exchange_సూచిక(src, 0, 0);
}
స్థిర FORCEINLINE శూన్య atomic_store_ptr(atomicptr_t *dst, శూన్య *val) {
  Interlocked_Exchange_సూచిక(dst, val);
}
స్థిర FORCEINLINE శూన్య atomic_store_ptr_release(atomicptr_t *dst, శూన్య *val) {
  Interlocked_Exchange_సూచిక(dst, val);
}
స్థిర FORCEINLINE శూన్య *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     శూన్య *val) {
  తిరిగి (శూన్య *)Interlocked_Exchange_సూచిక((శూన్య *అస్థిర *)dst, val);
}
స్థిర FORCEINLINE పూర్ణాంకం atomic_cas_ptr(atomicptr_t *dst, శూన్య *val, శూన్య *ref) {
  తిరిగి (Interlocked_Compare_Exchange_సూచిక((శూన్య *అస్థిర *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#నిర్వచించు EXPECTED(x) (x)
#నిర్వచించు UNEXPECTED(x) (x)

#లేకపోతే

#అన్తర్భావ <stdatomic.h>

రకనిర్వచనం అస్థిర _Atomic(int32_t) atomic32_t;
రకనిర్వచనం అస్థిర _Atomic(int64_t) atomic64_t;
రకనిర్వచనం అస్థిర _Atomic(శూన్య *) atomicptr_t;

స్థిర FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  తిరిగి atomic_ఎక్కించు_explicit(src, memory_order_relaxed);
}
స్థిర FORCEINLINE శూన్య atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
స్థిర FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  తిరిగి atomic_fetch_కూడు_explicit(val, 1, memory_order_relaxed) + 1;
}
స్థిర FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  తిరిగి atomic_fetch_కూడు_explicit(val, -1, memory_order_relaxed) - 1;
}
స్థిర FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t కూడు) {
  తిరిగి atomic_fetch_కూడు_explicit(val, కూడు, memory_order_relaxed) + కూడు;
}
స్థిర FORCEINLINE పూర్ణాంకం atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  తిరిగి atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
స్థిర FORCEINLINE శూన్య atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
స్థిర FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  తిరిగి atomic_ఎక్కించు_explicit(val, memory_order_relaxed);
}
స్థిర FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t కూడు) {
  తిరిగి atomic_fetch_కూడు_explicit(val, కూడు, memory_order_relaxed) + కూడు;
}
స్థిర FORCEINLINE శూన్య *atomic_ఎక్కించు_ptr(atomicptr_t *src) {
  తిరిగి atomic_ఎక్కించు_explicit(src, memory_order_relaxed);
}
స్థిర FORCEINLINE శూన్య atomic_store_ptr(atomicptr_t *dst, శూన్య *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
స్థిర FORCEINLINE శూన్య atomic_store_ptr_release(atomicptr_t *dst, శూన్య *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
స్థిర FORCEINLINE శూన్య *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     శూన్య *val) {
  తిరిగి atomic_exchange_explicit(dst, val, memory_order_acquire);
}
స్థిర FORCEINLINE పూర్ణాంకం atomic_cas_ptr(atomicptr_t *dst, శూన్య *val, శూన్య *ref) {
  తిరిగి atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_relaxed, memory_order_relaxed);
}

#నిర్వచించు EXPECTED(x) __builtin_expect((x), 1)
#నిర్వచించు UNEXPECTED(x) __builtin_expect((x), 0)

#అయితే_ముగింపు

////////////
///
/// Statistics related functions (evaluate to nothing when statistics not
/// enabled)
///
//////

#ఒకవేళ ENABLE_STATISTICS
#నిర్వచించు _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#నిర్వచించు _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#నిర్వచించు rpmalloc_stat_కూడు(counter, విలువ_2)                                     \
  atomic_add32(counter, (int32_t)(విలువ_2))
#నిర్వచించు _rpmalloc_stat_add64(counter, విలువ_2)                                   \
  atomic_add64(counter, (int64_t)(విలువ_2))
#నిర్వచించు rpmalloc_stat_కూడు_peak(counter, విలువ_2, peak)                          \
  చేయి {                                                                         \
    int32_t cur_లెక్క = atomic_add32(counter, (int32_t)(విలువ_2));              \
    ఒకవేళ (cur_లెక్క > (peak))                                                   \
      peak = cur_లెక్క;                                                       \
  } అయితే (0)
#నిర్వచించు _rpmalloc_stat_sub(counter, విలువ_2)                                     \
  atomic_add32(counter, -(int32_t)(విలువ_2))
#నిర్వచించు _rpmalloc_stat_inc_alloc(కుప్ప, class_idx)                              \
  చేయి {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&కుప్ప->కొలత_class_use[class_idx].alloc_current);         \
    ఒకవేళ (alloc_current > కుప్ప->కొలత_class_use[class_idx].alloc_peak)            \
      కుప్ప->కొలత_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&కుప్ప->కొలత_class_use[class_idx].alloc_మొత్తం);               \
  } అయితే (0)
#నిర్వచించు _rpmalloc_stat_inc_free(కుప్ప, class_idx)                               \
  చేయి {                                                                         \
    atomic_decr32(&కుప్ప->కొలత_class_use[class_idx].alloc_current);             \
    atomic_incr32(&కుప్ప->కొలత_class_use[class_idx].free_మొత్తం);                \
  } అయితే (0)
#లేకపోతే
#నిర్వచించు _rpmalloc_stat_inc(counter)                                            \
  చేయి {                                                                         \
  } అయితే (0)
#నిర్వచించు _rpmalloc_stat_dec(counter)                                            \
  చేయి {                                                                         \
  } అయితే (0)
#నిర్వచించు rpmalloc_stat_కూడు(counter, విలువ_2)                                     \
  చేయి {                                                                         \
  } అయితే (0)
#నిర్వచించు _rpmalloc_stat_add64(counter, విలువ_2)                                   \
  చేయి {                                                                         \
  } అయితే (0)
#నిర్వచించు rpmalloc_stat_కూడు_peak(counter, విలువ_2, peak)                          \
  చేయి {                                                                         \
  } అయితే (0)
#నిర్వచించు _rpmalloc_stat_sub(counter, విలువ_2)                                     \
  చేయి {                                                                         \
  } అయితే (0)
#నిర్వచించు _rpmalloc_stat_inc_alloc(కుప్ప, class_idx)                              \
  చేయి {                                                                         \
  } అయితే (0)
#నిర్వచించు _rpmalloc_stat_inc_free(కుప్ప, class_idx)                               \
  చేయి {                                                                         \
  } అయితే (0)
#అయితే_ముగింపు

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#నిర్వచించు SMALL_GRANULARITY 16
//! Small granularity shift count
#నిర్వచించు SMALL_GRANULARITY_జరుపు 4
//! Number of small block size classes
#నిర్వచించు SMALL_CLASS_లెక్క 65
//! Maximum size of a small block
#నిర్వచించు SMALL_కొలత_హద్దు (SMALL_GRANULARITY * (SMALL_CLASS_లెక్క - 1))
//! Granularity of a medium allocation block
#నిర్వచించు MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#నిర్వచించు MEDIUM_GRANULARITY_జరుపు 9
//! Number of medium block size classes
#నిర్వచించు MEDIUM_CLASS_లెక్క 61
//! Total number of small + medium size classes
#నిర్వచించు కొలత_CLASS_లెక్క (SMALL_CLASS_లెక్క + MEDIUM_CLASS_లెక్క)
//! Number of large block size classes
#నిర్వచించు LARGE_CLASS_లెక్క 63
//! Maximum size of a medium block
#నిర్వచించు MEDIUM_కొలత_హద్దు                                                      \
  (SMALL_కొలత_హద్దు + (MEDIUM_GRANULARITY * MEDIUM_CLASS_లెక్క))
//! Maximum size of a large block
#నిర్వచించు LARGE_కొలత_హద్దు                                                       \
  ((LARGE_CLASS_లెక్క * జ్ఞాపకం_పరిధి_కొలత) - పరిధి_శీర్షిక_కొలత)
//! Size of a span header (must be a multiple of SMALL_GRANULARITY and a power
//! of two)
#నిర్వచించు పరిధి_శీర్షిక_కొలత 128
//! Number of spans in thread cache
#నిర్వచించు MAX_THREAD_పరిధి_CACHE 400
//! Number of spans to transfer between thread and global cache
#నిర్వచించు THREAD_పరిధి_CACHE_TRANSFER 64
//! Number of spans in thread cache for large spans (must be greater than
//! LARGE_CLASS_COUNT / 2)
#నిర్వచించు MAX_THREAD_పరిధి_LARGE_CACHE 100
//! Number of spans to transfer between thread and global cache for large spans
#నిర్వచించు THREAD_పరిధి_LARGE_CACHE_TRANSFER 6

_Static_assert((SMALL_GRANULARITY & (SMALL_GRANULARITY - 1)) == 0,
               "Small granularity must be power of two");
_Static_assert((పరిధి_శీర్షిక_కొలత & (పరిధి_శీర్షిక_కొలత - 1)) == 0,
               "Span header size must be power of two");

#ఒకవేళ ENABLE_VALIDATE_వాదనలు
//! Maximum allocation size to avoid integer overflow
#నిర్వచనం_రద్దు MAX_ALLOC_కొలత
#నిర్వచించు MAX_ALLOC_కొలత (((size_t) - 1) - జ్ఞాపకం_పరిధి_కొలత)
#అయితే_ముగింపు

#నిర్వచించు సూచిక_స్థానభ్రంశం(ptr, ofs) (శూన్య *)((అక్షరం *)(ptr) + (ptrdiff_t)(ofs))
#నిర్వచించు సూచిక_diff(మొదటి_2, రెండవ_2)                                            \
  (ptrdiff_t)((స్థిరరాశి అక్షరం *)(మొదటి_2) - (స్థిరరాశి అక్షరం *)(రెండవ_2))

#నిర్వచించు INVALID_సూచిక ((శూన్య *)((uintptr_t) - 1))

#నిర్వచించు కొలత_CLASS_LARGE కొలత_CLASS_లెక్క
#నిర్వచించు కొలత_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
రకనిర్వచనం నిర్మాణం కుప్ప_t కుప్ప_t;
//! Span of memory pages
రకనిర్వచనం నిర్మాణం పరిధి_t పరిధి_t;
//! Span list
రకనిర్వచనం నిర్మాణం పరిధి_list_t పరిధి_list_t;
//! Span active data
రకనిర్వచనం నిర్మాణం పరిధి_చురుకైన_t పరిధి_చురుకైన_t;
//! Size class definition
రకనిర్వచనం నిర్మాణం కొలత_class_t కొలత_class_t;
//! Global cache
రకనిర్వచనం నిర్మాణం global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#నిర్వచించు పరిధి_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#నిర్వచించు పరిధి_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#నిర్వచించు పరిధి_FLAG_ALIGNED_ఖండాలు 4U
//! Flag indicating an unmapped master span
#నిర్వచించు పరిధి_FLAG_UNMAPPED_MASTER 8U

#ఒకవేళ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
నిర్మాణం పరిధి_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t ఎగువ;
#ఒకవేళ ENABLE_STATISTICS
  //! Number of spans in deferred list
  atomic32_t spans_deferred;
  //! Number of spans transitioned to global cache
  atomic32_t spans_గమ్యం_global;
  //! Number of spans transitioned from global cache
  atomic32_t spans_from_global;
  //! Number of spans transitioned to thread cache
  atomic32_t spans_గమ్యం_cache;
  //! Number of spans transitioned from thread cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned to reserved state
  atomic32_t spans_గమ్యం_reserved;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of raw memory map calls
  atomic32_t spans_map_calls;
#అయితే_ముగింపు
};
రకనిర్వచనం నిర్మాణం పరిధి_use_t పరిధి_use_t;
#అయితే_ముగింపు

#ఒకవేళ ENABLE_STATISTICS
నిర్మాణం కొలత_class_use_t {
  //! Current number of allocations
  atomic32_t alloc_current;
  //! Peak number of allocations
  int32_t alloc_peak;
  //! Total number of allocations
  atomic32_t alloc_మొత్తం;
  //! Total number of frees
  atomic32_t free_మొత్తం;
  //! Number of spans in use
  atomic32_t spans_current;
  //! Number of spans transitioned to cache
  int32_t spans_peak;
  //! Number of spans transitioned to cache
  atomic32_t spans_గమ్యం_cache;
  //! Number of spans transitioned from cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of spans mapped
  atomic32_t spans_map_calls;
  int32_t వాడని_2;
};
రకనిర్వచనం నిర్మాణం కొలత_class_use_t కొలత_class_use_t;
#అయితే_ముగింపు

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
నిర్మాణం పరిధి_t {
  //! Free list
  శూన్య *free_list;
  //! Total block count of size class
  uint32_t ఖండం_లెక్క;
  //! Size class
  uint32_t కొలత_class;
  //! Index of last block initialized in free list
  uint32_t free_list_హద్దు;
  //! Number of used blocks remaining when in partial state
  uint32_t వాడిన_లెక్క;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_కొలత;
  //! Size of a block
  uint32_t ఖండం_కొలత_2;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t పరిధి_లెక్క;
  //! Total span counter for master spans
  uint32_t మొత్తం_spans;
  //! Offset from master span for subspans
  uint32_t స్థానభ్రంశం_from_master;
  //! Remaining span counter, for master spans
  atomic32_t మిగిలిన_spans;
  //! Alignment offset
  uint32_t align_స్థానభ్రంశం;
  //! Owning heap
  కుప్ప_t *కుప్ప;
  //! Next span
  పరిధి_t *తదుపరి;
  //! Previous span
  పరిధి_t *prev;
};
_Static_assert(పరిమాణం(పరిధి_t) <= పరిధి_శీర్షిక_కొలత, "span size mismatch");

నిర్మాణం పరిధి_cache_t {
  size_t లెక్క;
  పరిధి_t *పరిధి[MAX_THREAD_పరిధి_CACHE];
};
రకనిర్వచనం నిర్మాణం పరిధి_cache_t పరిధి_cache_t;

నిర్మాణం పరిధి_large_cache_t {
  size_t లెక్క;
  పరిధి_t *పరిధి[MAX_THREAD_పరిధి_LARGE_CACHE];
};
రకనిర్వచనం నిర్మాణం పరిధి_large_cache_t పరిధి_large_cache_t;

నిర్మాణం కుప్ప_కొలత_class_t {
  //! Free list of active span
  శూన్య *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  పరిధి_t *partial_పరిధి;
  //! Early level cache of fully free spans
  పరిధి_t *cache;
};
రకనిర్వచనం నిర్మాణం కుప్ప_కొలత_class_t కుప్ప_కొలత_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
నిర్మాణం కుప్ప_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  కుప్ప_కొలత_class_t కొలత_class[కొలత_CLASS_లెక్క];
#ఒకవేళ ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  పరిధి_cache_t పరిధి_cache;
#అయితే_ముగింపు
  //! List of deferred free spans (single linked list)
  atomicptr_t పరిధి_free_deferred;
  //! Number of full spans
  size_t నిండిన_పరిధి_లెక్క;
  //! Mapped but unused spans
  పరిధి_t *పరిధి_reserve;
  //! Master span for mapped but unused spans
  పరిధి_t *పరిధి_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t బిడ్డ_లెక్క;
  //! Next heap in id list
  కుప్ప_t *తదుపరి_కుప్ప;
  //! Next heap in orphan list
  కుప్ప_t *తదుపరి_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  పూర్ణాంకం finalize;
  //! Master heap owning the memory pages
  కుప్ప_t *master_కుప్ప;
#ఒకవేళ ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  పరిధి_large_cache_t పరిధి_large_cache[LARGE_CLASS_లెక్క - 1];
#అయితే_ముగింపు
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  పరిధి_t *నిండిన_పరిధి[కొలత_CLASS_లెక్క];
  //! Double linked list of large and huge spans allocated by this heap
  పరిధి_t *large_huge_పరిధి;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  పరిధి_use_t పరిధి_use[LARGE_CLASS_లెక్క];
#అయితే_ముగింపు
#ఒకవేళ ENABLE_STATISTICS
  //! Allocation stats per size class
  కొలత_class_use_t కొలత_class_use[కొలత_CLASS_లెక్క + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_గమ్యం_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_గమ్యం_thread;
#అయితే_ముగింపు
};

// Size class for defining a block size bucket
నిర్మాణం కొలత_class_t {
  //! Size of blocks in this class
  uint32_t ఖండం_కొలత_2;
  //! Number of blocks in each chunk
  uint16_t ఖండం_లెక్క;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(పరిమాణం(కొలత_class_t) == 8, "Size class size mismatch");

నిర్మాణం global_cache_t {
  //! Cache lock
  atomic32_t తాళం_2;
  //! Cache count
  uint32_t లెక్క;
#ఒకవేళ ENABLE_STATISTICS
  //! Insert count
  size_t చొప్పించు_లెక్క;
  //! Extract count
  size_t వెలికితీయి_లెక్క;
#అయితే_ముగింపు
  //! Cached spans
  పరిధి_t *పరిధి[GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_పరిధి_CACHE];
  //! Unlimited cache overflow
  పరిధి_t *అధికప్రవాహం_2;
};

////////////
///
/// Global data
///
//////

//! Default span size (64KiB)
#నిర్వచించు జ్ఞాపకం_default_పరిధి_కొలత (64 * 1024)
#నిర్వచించు జ్ఞాపకం_default_పరిధి_కొలత_జరుపు 16
#నిర్వచించు జ్ఞాపకం_default_పరిధి_ముసుగు (~((uintptr_t)(జ్ఞాపకం_పరిధి_కొలత - 1)))

//! Initialized flag
స్థిర పూర్ణాంకం _rpmalloc_initialized;
//! Main thread ID
స్థిర uintptr_t _rpmalloc_main_thread_id;
//! Configuration
స్థిర rpmalloc_config_t జ్ఞాపకం_config;
//! Memory page size
స్థిర size_t జ్ఞాపకం_పుట_కొలత;
//! Shift to divide by page size
స్థిర size_t జ్ఞాపకం_పుట_కొలత_జరుపు;
//! Granularity at which memory pages are mapped by OS
స్థిర size_t జ్ఞాపకం_map_granularity;
#ఒకవేళ RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
స్థిర size_t జ్ఞాపకం_పరిధి_కొలత;
//! Shift to divide by span size
స్థిర size_t జ్ఞాపకం_పరిధి_కొలత_జరుపు;
//! Mask to get to start of a memory span
స్థిర uintptr_t జ్ఞాపకం_పరిధి_ముసుగు;
#లేకపోతే
//! Hardwired span size
#నిర్వచించు జ్ఞాపకం_పరిధి_కొలత జ్ఞాపకం_default_పరిధి_కొలత
#నిర్వచించు జ్ఞాపకం_పరిధి_కొలత_జరుపు జ్ఞాపకం_default_పరిధి_కొలత_జరుపు
#నిర్వచించు జ్ఞాపకం_పరిధి_ముసుగు జ్ఞాపకం_default_పరిధి_ముసుగు
#అయితే_ముగింపు
//! Number of spans to map in each map call
స్థిర size_t జ్ఞాపకం_పరిధి_map_లెక్క;
//! Number of spans to keep reserved in each heap
స్థిర size_t జ్ఞాపకం_కుప్ప_reserve_లెక్క;
//! Global size classes
స్థిర కొలత_class_t జ్ఞాపకం_కొలత_class[కొలత_CLASS_లెక్క];
//! Run-time size limit of medium blocks
స్థిర size_t జ్ఞాపకం_medium_కొలత_హద్దు;
//! Heap ID counter
స్థిర atomic32_t జ్ఞాపకం_కుప్ప_id;
//! Huge page support
స్థిర పూర్ణాంకం జ్ఞాపకం_huge_పుటలు;
#ఒకవేళ ENABLE_GLOBAL_CACHE
//! Global span cache
స్థిర global_cache_t జ్ఞాపకం_పరిధి_cache[LARGE_CLASS_లెక్క];
#అయితే_ముగింపు
//! Global reserved spans
స్థిర పరిధి_t *జ్ఞాపకం_global_reserve;
//! Global reserved count
స్థిర size_t జ్ఞాపకం_global_reserve_లెక్క;
//! Global reserved master
స్థిర పరిధి_t *జ్ఞాపకం_global_reserve_master;
//! All heaps
స్థిర కుప్ప_t *జ్ఞాపకం_heaps[కుప్ప_ARRAY_కొలత];
//! Used to restrict access to mapping memory for huge pages
స్థిర atomic32_t జ్ఞాపకం_global_తాళం;
//! Orphaned heaps
స్థిర కుప్ప_t *జ్ఞాపకం_orphan_heaps;
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
స్థిర కుప్ప_t *జ్ఞాపకం_మొదటి_class_orphan_heaps;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_STATISTICS
//! Allocations counter
స్థిర atomic64_t కేటాయింపు_counter;
//! Deallocations counter
స్థిర atomic64_t _deallocation_counter;
//! Active heap count
స్థిర atomic32_t జ్ఞాపకం_చురుకైన_heaps;
//! Number of currently mapped memory pages
స్థిర atomic32_t mapped_పుటలు;
//! Peak number of concurrently mapped memory pages
స్థిర int32_t mapped_పుటలు_peak;
//! Number of mapped master spans
స్థిర atomic32_t _master_spans;
//! Number of unmapped dangling master spans
స్థిర atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
స్థిర atomic32_t mapped_మొత్తం;
//! Running counter of total number of unmapped memory pages since start
స్థిర atomic32_t unmapped_మొత్తం;
//! Number of currently mapped memory pages in OS calls
స్థిర atomic32_t mapped_పుటలు_os;
//! Number of currently allocated pages in huge allocations
స్థిర atomic32_t huge_పుటలు_current;
//! Peak number of currently allocated pages in huge allocations
స్థిర int32_t huge_పుటలు_peak;
#అయితే_ముగింపు

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#ఒకవేళ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
స్థిర pthread_key_t జ్ఞాపకం_thread_కుప్ప;
#లేకపోతే
#నిర్వచితమైతే _MSC_VER
#నిర్వచించు _Thread_local __declspec(thread)
#నిర్వచించు TLS_MODEL
#లేకపోతే
#నిర్వచితం_కాకపోతే __HAIKU__
#నిర్వచించు TLS_MODEL __attribute__((tls_model("initial-exec")))
#లేకపోతే
#నిర్వచించు TLS_MODEL
#అయితే_ముగింపు
#ఒకవేళ !defined(__clang__) && defined(__GNUC__)
#నిర్వచించు _Thread_local __thread
#అయితే_ముగింపు
#అయితే_ముగింపు
స్థిర _Thread_local కుప్ప_t *జ్ఞాపకం_thread_కుప్ప TLS_MODEL;
#అయితే_ముగింపు

స్థిర అంతఃవరుస కుప్ప_t *get_thread_కుప్ప_raw(శూన్య) {
#ఒకవేళ (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  తిరిగి pthread_getspecific(జ్ఞాపకం_thread_కుప్ప);
#లేకపోతే
  తిరిగి జ్ఞాపకం_thread_కుప్ప;
#అయితే_ముగింపు
}

//! Get the current thread heap
స్థిర అంతఃవరుస కుప్ప_t *get_thread_కుప్ప(శూన్య) {
  కుప్ప_t *కుప్ప = get_thread_కుప్ప_raw();
#ఒకవేళ ENABLE_PRELOAD
  ఒకవేళ (EXPECTED(కుప్ప != 0))
    తిరిగి కుప్ప;
  rpmalloc_initialize();
  తిరిగి get_thread_కుప్ప_raw();
#లేకపోతే
  తిరిగి కుప్ప;
#అయితే_ముగింపు
}

//! Fast thread ID
స్థిర అంతఃవరుస uintptr_t get_thread_id(శూన్య) {
#ఒకవేళ defined(_WIN32)
  తిరిగి (uintptr_t)((శూన్య *)NtCurrentTeb());
#లేదావేళ (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#ఒకవేళ defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#లేదావేళ defined(__x86_64__)
#ఒకవేళ defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#లేకపోతే
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#అయితే_ముగింపు
#లేదావేళ defined(__arm__)
  __asm__ అస్థిర("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#లేదావేళ defined(__aarch64__)
#ఒకవేళ defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ అస్థిర("mrs %0, tpidrro_el0" : "=r"(tid));
#లేకపోతే
  __asm__ అస్థిర("mrs %0, tpidr_el0" : "=r"(tid));
#అయితే_ముగింపు
#లేకపోతే
#దోషం This platform needs implementation యొక్క_3 get_thread_id()
#అయితే_ముగింపు
  తిరిగి tid;
#లేకపోతే
#దోషం This platform needs implementation యొక్క_3 get_thread_id()
#అయితే_ముగింపు
}

//! Set the current thread heap
స్థిర శూన్య సమితి_thread_కుప్ప(కుప్ప_t *కుప్ప) {
#ఒకవేళ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(జ్ఞాపకం_thread_కుప్ప, కుప్ప);
#లేకపోతే
  జ్ఞాపకం_thread_కుప్ప = కుప్ప;
#అయితే_ముగింపు
  ఒకవేళ (కుప్ప)
    కుప్ప->owner_thread = get_thread_id();
}

//! Set main thread ID
బాహ్య శూన్య rpmalloc_సమితి_main_thread(శూన్య);

శూన్య rpmalloc_సమితి_main_thread(శూన్య) {
  _rpmalloc_main_thread_id = get_thread_id();
}

స్థిర శూన్య _rpmalloc_spin(శూన్య) {
#ఒకవేళ defined(_MSC_VER)
#ఒకవేళ defined(_M_ARM64)
  __yield();
#లేకపోతే
  _mm_pause();
#అయితే_ముగింపు
#లేదావేళ defined(__x86_64__) || defined(__i386__)
  __asm__ అస్థిర("pause" ::: "memory");
#లేదావేళ defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ అస్థిర("yield" ::: "memory");
#లేదావేళ defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ అస్థిర("or 27,27,27");
#లేదావేళ defined(__sparc__)
  __asm__ అస్థిర("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#లేకపోతే
  నిర్మాణం timespec ts = {0};
  nanosleep(&ts, 0);
#అయితే_ముగింపు
}

#ఒకవేళ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

స్థిర శూన్య NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  ఎంపిక (reason) {
  స్థితి DLL_PROCESS_ATTACH:
    ఆపు;
  స్థితి DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    ఆపు;
  స్థితి DLL_THREAD_ATTACH:
    ఆపు;
  స్థితి DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    ఆపు;
  }
}

#నిర్వచితమైతే _WIN64
#సూచన comment(linker, "/INCLUDE:_tls_used")
#సూచన comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#సూచన const_seg(".CRT$XLY")

బాహ్య స్థిరరాశి PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
స్థిరరాశి PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#సూచన const_seg()
#లేకపోతే // _WIN64
#సూచన comment(linker, "/INCLUDE:__tls_used")
#సూచన comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#సూచన దత్తాంశం_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#సూచన దత్తాంశం_seg()
#అయితే_ముగింపు // _WIN64

స్థిర శూన్య NTAPI _rpmalloc_thread_destructor(శూన్య *విలువ_2) {
#ఒకవేళ ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  ఒకవేళ (get_thread_id() == _rpmalloc_main_thread_id)
    తిరిగి;
#అయితే_ముగింపు
  ఒకవేళ (విలువ_2)
    rpmalloc_thread_finalize(1);
}
#అయితే_ముగింపు

////////////
///
/// Low level memory map/unmap
///
//////

స్థిర శూన్య rpmalloc_సమితి_name(శూన్య *చిరునామా_2, size_t కొలత_2) {
#ఒకవేళ defined(__linux__) || defined(__ANDROID__)
  స్థిరరాశి అక్షరం *name = జ్ఞాపకం_huge_పుటలు ? జ్ఞాపకం_config.huge_పుట_name
                                        : జ్ఞాపకం_config.పుట_name;
  ఒకవేళ (చిరునామా_2 == MAP_FAILED || !name)
    తిరిగి;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (శూన్య)prctl(PR_సమితి_VMA, PR_సమితి_VMA_ANON_NAME, (uintptr_t)చిరునామా_2, కొలత_2,
              (uintptr_t)name);
#లేకపోతే
  (శూన్య)పరిమాణం(కొలత_2);
  (శూన్య)పరిమాణం(చిరునామా_2);
#అయితే_ముగింపు
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
స్థిర శూన్య *_rpmalloc_mmap(size_t కొలత_2, size_t *స్థానభ్రంశం_2) {
  rpmalloc_assert(!(కొలత_2 % జ్ఞాపకం_పుట_కొలత), "Invalid mmap size");
  rpmalloc_assert(కొలత_2 >= జ్ఞాపకం_పుట_కొలత, "Invalid mmap size");
  శూన్య *చిరునామా_2 = జ్ఞాపకం_config.జ్ఞాపకం_map(కొలత_2, స్థానభ్రంశం_2);
  ఒకవేళ (EXPECTED(చిరునామా_2 != 0)) {
    rpmalloc_stat_కూడు_peak(&mapped_పుటలు, (కొలత_2 >> జ్ఞాపకం_పుట_కొలత_జరుపు),
                            mapped_పుటలు_peak);
    rpmalloc_stat_కూడు(&mapped_మొత్తం, (కొలత_2 >> జ్ఞాపకం_పుట_కొలత_జరుపు));
  }
  తిరిగి చిరునామా_2;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
స్థిర శూన్య _rpmalloc_unmap(శూన్య *చిరునామా_2, size_t కొలత_2, size_t స్థానభ్రంశం_2,
                            size_t release) {
  rpmalloc_assert(!release || (release >= కొలత_2), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= జ్ఞాపకం_పుట_కొలత),
                  "Invalid unmap size");
  ఒకవేళ (release) {
    rpmalloc_assert(!(release % జ్ఞాపకం_పుట_కొలత), "Invalid unmap size");
    _rpmalloc_stat_sub(&mapped_పుటలు, (release >> జ్ఞాపకం_పుట_కొలత_జరుపు));
    rpmalloc_stat_కూడు(&unmapped_మొత్తం, (release >> జ్ఞాపకం_పుట_కొలత_జరుపు));
  }
  జ్ఞాపకం_config.జ్ఞాపకం_unmap(చిరునామా_2, కొలత_2, స్థానభ్రంశం_2, release);
}

//! Default implementation to map new pages to virtual memory
స్థిర శూన్య *_rpmalloc_mmap_os(size_t కొలత_2, size_t *స్థానభ్రంశం_2) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((కొలత_2 >= జ్ఞాపకం_పరిధి_కొలత) &&
                    (జ్ఞాపకం_పరిధి_కొలత > జ్ఞాపకం_map_granularity))
                       ? జ్ఞాపకం_పరిధి_కొలత
                       : 0;
  rpmalloc_assert(కొలత_2 >= జ్ఞాపకం_పుట_కొలత, "Invalid mmap size");
#ఒకవేళ PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  శూన్య *ptr = VirtualAlloc(0, కొలత_2 + padding,
                           (జ్ఞాపకం_huge_పుటలు ? MEM_LARGE_పుటలు : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           పుట_READWRITE);
  ఒకవేళ (!ptr) {
    ఒకవేళ (జ్ఞాపకం_config.map_fail_callback) {
      ఒకవేళ (జ్ఞాపకం_config.map_fail_callback(కొలత_2 + padding))
        తిరిగి _rpmalloc_mmap_os(కొలత_2, స్థానభ్రంశం_2);
    } లేకపోతే {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    తిరిగి 0;
  }
#లేకపోతే
  పూర్ణాంకం flags = MAP_వ్యక్తిగత | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#ఒకవేళ defined(__APPLE__) && !లక్ష్యం_OS_IPHONE && !లక్ష్యం_OS_SIMULATOR
  పూర్ణాంకం fd = (పూర్ణాంకం)VM_MAKE_ట్యాగ్(240U);
  ఒకవేళ (జ్ఞాపకం_huge_పుటలు)
    fd |= VM_FLAGS_SUPERPAGE_కొలత_2_MB;
  శూన్య *ptr = mmap(0, కొలత_2 + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#లేదావేళ defined(MAP_HUGETLB)
  శూన్య *ptr = mmap(0, కొలత_2 + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (జ్ఞాపకం_huge_పుటలు ? MAP_HUGETLB : 0) | flags, -1, 0);
#ఒకవేళ defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  ఒకవేళ ((ptr == MAP_FAILED || !ptr) && జ్ఞాపకం_huge_పుటలు) {
    ptr = mmap(0, కొలత_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    ఒకవేళ (ptr && ptr != MAP_FAILED) {
      పూర్ణాంకం prm = madvise(ptr, కొలత_2 + padding, MADV_HUGEPAGE);
      (శూన్య)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#అయితే_ముగింపు
  rpmalloc_సమితి_name(ptr, కొలత_2 + padding);
#లేదావేళ defined(MAP_ALIGNED)
  స్థిరరాశి size_t align =
      (పరిమాణం(size_t) * 8) - (size_t)(__builtin_clzl(కొలత_2 - 1));
  శూన్య *ptr =
      mmap(0, కొలత_2 + padding, PROT_READ | PROT_WRITE,
           (జ్ఞాపకం_huge_పుటలు ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#లేదావేళ defined(MAP_ALIGN)
  caddr_t పునాది_2 = (జ్ఞాపకం_huge_పుటలు ? (caddr_t)(4 << 20) : 0);
  శూన్య *ptr = mmap(పునాది_2, కొలత_2 + padding, PROT_READ | PROT_WRITE,
                   (జ్ఞాపకం_huge_పుటలు ? MAP_ALIGN : 0) | flags, -1, 0);
#లేకపోతే
  శూన్య *ptr = mmap(0, కొలత_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#అయితే_ముగింపు
  ఒకవేళ ((ptr == MAP_FAILED) || !ptr) {
    ఒకవేళ (జ్ఞాపకం_config.map_fail_callback) {
      ఒకవేళ (జ్ఞాపకం_config.map_fail_callback(కొలత_2 + padding))
        తిరిగి _rpmalloc_mmap_os(కొలత_2, స్థానభ్రంశం_2);
    } లేకపోతే ఒకవేళ (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    తిరిగి 0;
  }
#అయితే_ముగింపు
  rpmalloc_stat_కూడు(&mapped_పుటలు_os,
                     (int32_t)((కొలత_2 + padding) >> జ్ఞాపకం_పుట_కొలత_జరుపు));
  ఒకవేళ (padding) {
    size_t final_padding = padding - ((uintptr_t)ptr & ~జ్ఞాపకం_పరిధి_ముసుగు);
    rpmalloc_assert(final_padding <= జ్ఞాపకం_పరిధి_కొలత,
                    "Internal failure in padding");
    rpmalloc_assert(final_padding <= padding, "Internal failure in padding");
    rpmalloc_assert(!(final_padding % 8), "Internal failure in padding");
    ptr = సూచిక_స్థానభ్రంశం(ptr, final_padding);
    *స్థానభ్రంశం_2 = final_padding >> 3;
  }
  rpmalloc_assert((కొలత_2 < జ్ఞాపకం_పరిధి_కొలత) ||
                      !((uintptr_t)ptr & ~జ్ఞాపకం_పరిధి_ముసుగు),
                  "Internal failure in padding");
  తిరిగి ptr;
}

//! Default implementation to unmap pages from virtual memory
స్థిర శూన్య _rpmalloc_unmap_os(శూన్య *చిరునామా_2, size_t కొలత_2, size_t స్థానభ్రంశం_2,
                               size_t release) {
  rpmalloc_assert(release || (స్థానభ్రంశం_2 == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= జ్ఞాపకం_పుట_కొలత),
                  "Invalid unmap size");
  rpmalloc_assert(కొలత_2 >= జ్ఞాపకం_పుట_కొలత, "Invalid unmap size");
  ఒకవేళ (release && స్థానభ్రంశం_2) {
    స్థానభ్రంశం_2 <<= 3;
    చిరునామా_2 = సూచిక_స్థానభ్రంశం(చిరునామా_2, -(int32_t)స్థానభ్రంశం_2);
    ఒకవేళ ((release >= జ్ఞాపకం_పరిధి_కొలత) &&
        (జ్ఞాపకం_పరిధి_కొలత > జ్ఞాపకం_map_granularity)) {
      // Padding is always one span size
      release += జ్ఞాపకం_పరిధి_కొలత;
    }
  }
#ఒకవేళ !DISABLE_UNMAP
#ఒకవేళ PLATFORM_WINDOWS
  ఒకవేళ (!VirtualFree(చిరునామా_2, release ? 0 : కొలత_2,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#లేకపోతే
  ఒకవేళ (release) {
    ఒకవేళ (munmap(చిరునామా_2, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } లేకపోతే {
#ఒకవేళ defined(MADV_FREE_REUSABLE)
    పూర్ణాంకం ret;
    అయితే ((ret = madvise(చిరునామా_2, కొలత_2, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    ఒకవేళ ((ret == -1) && (errno != 0)) {
#లేదావేళ defined(MADV_DONTNEED)
    ఒకవేళ (madvise(చిరునామా_2, కొలత_2, MADV_DONTNEED)) {
#లేదావేళ defined(MADV_PAGEOUT)
    ఒకవేళ (madvise(చిరునామా_2, కొలత_2, MADV_PAGEOUT)) {
#లేదావేళ defined(MADV_FREE)
    ఒకవేళ (madvise(చిరునామా_2, కొలత_2, MADV_FREE)) {
#లేకపోతే
    ఒకవేళ (posix_madvise(చిరునామా_2, కొలత_2, POSIX_MADV_DONTNEED)) {
#అయితే_ముగింపు
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#అయితే_ముగింపు
#అయితే_ముగింపు
  ఒకవేళ (release)
    _rpmalloc_stat_sub(&mapped_పుటలు_os, release >> జ్ఞాపకం_పుట_కొలత_జరుపు);
}

స్థిర శూన్య rpmalloc_పరిధి_mark_as_subspan_unless_master(పరిధి_t *master,
                                                         పరిధి_t *subspan,
                                                         size_t పరిధి_లెక్క);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
స్థిర పరిధి_t *_rpmalloc_global_get_reserved_spans(size_t పరిధి_లెక్క) {
  పరిధి_t *పరిధి = జ్ఞాపకం_global_reserve;
  rpmalloc_పరిధి_mark_as_subspan_unless_master(జ్ఞాపకం_global_reserve_master,
                                               పరిధి, పరిధి_లెక్క);
  జ్ఞాపకం_global_reserve_లెక్క -= పరిధి_లెక్క;
  ఒకవేళ (జ్ఞాపకం_global_reserve_లెక్క)
    జ్ఞాపకం_global_reserve =
        (పరిధి_t *)సూచిక_స్థానభ్రంశం(పరిధి, పరిధి_లెక్క << జ్ఞాపకం_పరిధి_కొలత_జరుపు);
  లేకపోతే
    జ్ఞాపకం_global_reserve = 0;
  తిరిగి పరిధి;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
స్థిర శూన్య rpmalloc_global_సమితి_reserved_spans(పరిధి_t *master, పరిధి_t *reserve,
                                                size_t reserve_పరిధి_లెక్క) {
  జ్ఞాపకం_global_reserve_master = master;
  జ్ఞాపకం_global_reserve_లెక్క = reserve_పరిధి_లెక్క;
  జ్ఞాపకం_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
స్థిర శూన్య rpmalloc_పరిధి_double_link_list_కూడు(పరిధి_t **తల, పరిధి_t *పరిధి) {
  ఒకవేళ (*తల)
    (*తల)->prev = పరిధి;
  పరిధి->తదుపరి = *తల;
  *తల = పరిధి;
}

//! Pop head span from double linked list
స్థిర శూన్య rpmalloc_పరిధి_double_link_list_తీయి_తల(పరిధి_t **తల,
                                                     పరిధి_t *పరిధి) {
  rpmalloc_assert(*తల == పరిధి, "Linked list corrupted");
  పరిధి = *తల;
  *తల = పరిధి->తదుపరి;
}

//! Remove a span from double linked list
స్థిర శూన్య rpmalloc_పరిధి_double_link_list_remove(పరిధి_t **తల,
                                                   పరిధి_t *పరిధి) {
  rpmalloc_assert(*తల, "Linked list corrupted");
  ఒకవేళ (*తల == పరిధి) {
    *తల = పరిధి->తదుపరి;
  } లేకపోతే {
    పరిధి_t *తదుపరి_పరిధి = పరిధి->తదుపరి;
    పరిధి_t *prev_పరిధి = పరిధి->prev;
    prev_పరిధి->తదుపరి = తదుపరి_పరిధి;
    ఒకవేళ (EXPECTED(తదుపరి_పరిధి != 0))
      తదుపరి_పరిధి->prev = prev_పరిధి;
  }
}

////////////
///
/// Span control
///
//////

స్థిర శూన్య rpmalloc_కుప్ప_cache_చొప్పించు(కుప్ప_t *కుప్ప, పరిధి_t *పరిధి);

స్థిర శూన్య rpmalloc_కుప్ప_finalize(కుప్ప_t *కుప్ప);

స్థిర శూన్య rpmalloc_కుప్ప_సమితి_reserved_spans(కుప్ప_t *కుప్ప, పరిధి_t *master,
                                              పరిధి_t *reserve,
                                              size_t reserve_పరిధి_లెక్క);

//! Declare the span to be a subspan and store distance from master span and
//! span count
స్థిర శూన్య rpmalloc_పరిధి_mark_as_subspan_unless_master(పరిధి_t *master,
                                                         పరిధి_t *subspan,
                                                         size_t పరిధి_లెక్క) {
  rpmalloc_assert((subspan != master) || (subspan->flags & పరిధి_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  ఒకవేళ (subspan != master) {
    subspan->flags = పరిధి_FLAG_SUBSPAN;
    subspan->స్థానభ్రంశం_from_master =
        (uint32_t)((uintptr_t)సూచిక_diff(subspan, master) >>
                   జ్ఞాపకం_పరిధి_కొలత_జరుపు);
    subspan->align_స్థానభ్రంశం = 0;
  }
  subspan->పరిధి_లెక్క = (uint32_t)పరిధి_లెక్క;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
స్థిర పరిధి_t *rpmalloc_పరిధి_map_from_reserve(కుప్ప_t *కుప్ప,
                                               size_t పరిధి_లెక్క) {
  // Update the heap span reserve
  పరిధి_t *పరిధి = కుప్ప->పరిధి_reserve;
  కుప్ప->పరిధి_reserve =
      (పరిధి_t *)సూచిక_స్థానభ్రంశం(పరిధి, పరిధి_లెక్క * జ్ఞాపకం_పరిధి_కొలత);
  కుప్ప->spans_reserved -= (uint32_t)పరిధి_లెక్క;

  rpmalloc_పరిధి_mark_as_subspan_unless_master(కుప్ప->పరిధి_reserve_master, పరిధి,
                                               పరిధి_లెక్క);
  ఒకవేళ (పరిధి_లెక్క <= LARGE_CLASS_లెక్క)
    _rpmalloc_stat_inc(&కుప్ప->పరిధి_use[పరిధి_లెక్క - 1].spans_from_reserved);

  తిరిగి పరిధి;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
స్థిర size_t rpmalloc_పరిధి_align_లెక్క(size_t పరిధి_లెక్క) {
  size_t request_లెక్క = (పరిధి_లెక్క > జ్ఞాపకం_పరిధి_map_లెక్క)
                             ? పరిధి_లెక్క
                             : జ్ఞాపకం_పరిధి_map_లెక్క;
  ఒకవేళ ((జ్ఞాపకం_పుట_కొలత > జ్ఞాపకం_పరిధి_కొలత) &&
      ((request_లెక్క * జ్ఞాపకం_పరిధి_కొలత) % జ్ఞాపకం_పుట_కొలత))
    request_లెక్క +=
        జ్ఞాపకం_పరిధి_map_లెక్క - (request_లెక్క % జ్ఞాపకం_పరిధి_map_లెక్క);
  తిరిగి request_లెక్క;
}

//! Setup a newly mapped span
స్థిర శూన్య rpmalloc_పరిధి_initialize(పరిధి_t *పరిధి, size_t మొత్తం_పరిధి_లెక్క,
                                      size_t పరిధి_లెక్క, size_t align_స్థానభ్రంశం) {
  పరిధి->మొత్తం_spans = (uint32_t)మొత్తం_పరిధి_లెక్క;
  పరిధి->పరిధి_లెక్క = (uint32_t)పరిధి_లెక్క;
  పరిధి->align_స్థానభ్రంశం = (uint32_t)align_స్థానభ్రంశం;
  పరిధి->flags = పరిధి_FLAG_MASTER;
  atomic_store32(&పరిధి->మిగిలిన_spans, (int32_t)మొత్తం_పరిధి_లెక్క);
}

స్థిర శూన్య rpmalloc_పరిధి_unmap(పరిధి_t *పరిధి);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
స్థిర పరిధి_t *rpmalloc_పరిధి_map_aligned_లెక్క(కుప్ప_t *కుప్ప,
                                                size_t పరిధి_లెక్క) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_పరిధి_లెక్క = rpmalloc_పరిధి_align_లెక్క(పరిధి_లెక్క);
  size_t align_స్థానభ్రంశం = 0;
  పరిధి_t *పరిధి = (పరిధి_t *)_rpmalloc_mmap(
      aligned_పరిధి_లెక్క * జ్ఞాపకం_పరిధి_కొలత, &align_స్థానభ్రంశం);
  ఒకవేళ (!పరిధి)
    తిరిగి 0;
  rpmalloc_పరిధి_initialize(పరిధి, aligned_పరిధి_లెక్క, పరిధి_లెక్క, align_స్థానభ్రంశం);
  _rpmalloc_stat_inc(&_master_spans);
  ఒకవేళ (పరిధి_లెక్క <= LARGE_CLASS_లెక్క)
    _rpmalloc_stat_inc(&కుప్ప->పరిధి_use[పరిధి_లెక్క - 1].spans_map_calls);
  ఒకవేళ (aligned_పరిధి_లెక్క > పరిధి_లెక్క) {
    పరిధి_t *reserved_spans =
        (పరిధి_t *)సూచిక_స్థానభ్రంశం(పరిధి, పరిధి_లెక్క * జ్ఞాపకం_పరిధి_కొలత);
    size_t reserved_లెక్క = aligned_పరిధి_లెక్క - పరిధి_లెక్క;
    ఒకవేళ (కుప్ప->spans_reserved) {
      rpmalloc_పరిధి_mark_as_subspan_unless_master(
          కుప్ప->పరిధి_reserve_master, కుప్ప->పరిధి_reserve, కుప్ప->spans_reserved);
      rpmalloc_కుప్ప_cache_చొప్పించు(కుప్ప, కుప్ప->పరిధి_reserve);
    }
    ఒకవేళ (reserved_లెక్క > జ్ఞాపకం_కుప్ప_reserve_లెక్క) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&జ్ఞాపకం_global_తాళం) == 1,
                      "Global spin lock not held as expected");
      size_t remain_లెక్క = reserved_లెక్క - జ్ఞాపకం_కుప్ప_reserve_లెక్క;
      reserved_లెక్క = జ్ఞాపకం_కుప్ప_reserve_లెక్క;
      పరిధి_t *remain_పరిధి = (పరిధి_t *)సూచిక_స్థానభ్రంశం(
          reserved_spans, reserved_లెక్క * జ్ఞాపకం_పరిధి_కొలత);
      ఒకవేళ (జ్ఞాపకం_global_reserve) {
        rpmalloc_పరిధి_mark_as_subspan_unless_master(
            జ్ఞాపకం_global_reserve_master, జ్ఞాపకం_global_reserve,
            జ్ఞాపకం_global_reserve_లెక్క);
        rpmalloc_పరిధి_unmap(జ్ఞాపకం_global_reserve);
      }
      rpmalloc_global_సమితి_reserved_spans(పరిధి, remain_పరిధి, remain_లెక్క);
    }
    rpmalloc_కుప్ప_సమితి_reserved_spans(కుప్ప, పరిధి, reserved_spans,
                                      reserved_లెక్క);
  }
  తిరిగి పరిధి;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
స్థిర పరిధి_t *rpmalloc_పరిధి_map(కుప్ప_t *కుప్ప, size_t పరిధి_లెక్క) {
  ఒకవేళ (పరిధి_లెక్క <= కుప్ప->spans_reserved)
    తిరిగి rpmalloc_పరిధి_map_from_reserve(కుప్ప, పరిధి_లెక్క);
  పరిధి_t *పరిధి = 0;
  పూర్ణాంకం use_global_reserve =
      (జ్ఞాపకం_పుట_కొలత > జ్ఞాపకం_పరిధి_కొలత) ||
      (జ్ఞాపకం_పరిధి_map_లెక్క > జ్ఞాపకం_కుప్ప_reserve_లెక్క);
  ఒకవేళ (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    అయితే (!atomic_cas32_acquire(&జ్ఞాపకం_global_తాళం, 1, 0))
      _rpmalloc_spin();
    ఒకవేళ (జ్ఞాపకం_global_reserve_లెక్క >= పరిధి_లెక్క) {
      size_t reserve_లెక్క =
          (!కుప్ప->spans_reserved ? జ్ఞాపకం_కుప్ప_reserve_లెక్క : పరిధి_లెక్క);
      ఒకవేళ (జ్ఞాపకం_global_reserve_లెక్క < reserve_లెక్క)
        reserve_లెక్క = జ్ఞాపకం_global_reserve_లెక్క;
      పరిధి = _rpmalloc_global_get_reserved_spans(reserve_లెక్క);
      ఒకవేళ (పరిధి) {
        ఒకవేళ (reserve_లెక్క > పరిధి_లెక్క) {
          పరిధి_t *reserved_పరిధి = (పరిధి_t *)సూచిక_స్థానభ్రంశం(
              పరిధి, పరిధి_లెక్క << జ్ఞాపకం_పరిధి_కొలత_జరుపు);
          rpmalloc_కుప్ప_సమితి_reserved_spans(కుప్ప, జ్ఞాపకం_global_reserve_master,
                                            reserved_పరిధి,
                                            reserve_లెక్క - పరిధి_లెక్క);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        పరిధి->పరిధి_లెక్క = (uint32_t)పరిధి_లెక్క;
      }
    }
  }
  ఒకవేళ (!పరిధి)
    పరిధి = rpmalloc_పరిధి_map_aligned_లెక్క(కుప్ప, పరిధి_లెక్క);
  ఒకవేళ (use_global_reserve)
    atomic_store32_release(&జ్ఞాపకం_global_తాళం, 0);
  తిరిగి పరిధి;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
స్థిర శూన్య rpmalloc_పరిధి_unmap(పరిధి_t *పరిధి) {
  rpmalloc_assert((పరిధి->flags & పరిధి_FLAG_MASTER) ||
                      (పరిధి->flags & పరిధి_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(పరిధి->flags & పరిధి_FLAG_MASTER) ||
                      !(పరిధి->flags & పరిధి_FLAG_SUBSPAN),
                  "Span flag corrupted");

  పూర్ణాంకం is_master = !!(పరిధి->flags & పరిధి_FLAG_MASTER);
  పరిధి_t *master =
      is_master ? పరిధి
                : ((పరిధి_t *)సూచిక_స్థానభ్రంశం(
                      పరిధి, -(intptr_t)((uintptr_t)పరిధి->స్థానభ్రంశం_from_master *
                                        జ్ఞాపకం_పరిధి_కొలత)));
  rpmalloc_assert(is_master || (పరిధి->flags & పరిధి_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & పరిధి_FLAG_MASTER, "Span flag corrupted");

  size_t పరిధి_లెక్క = పరిధి->పరిధి_లెక్క;
  ఒకవేళ (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(పరిధి->align_స్థానభ్రంశం == 0, "Span align offset corrupted");
    ఒకవేళ (జ్ఞాపకం_పరిధి_కొలత >= జ్ఞాపకం_పుట_కొలత)
      _rpmalloc_unmap(పరిధి, పరిధి_లెక్క * జ్ఞాపకం_పరిధి_కొలత, 0, 0);
  } లేకపోతే {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    పరిధి->flags |=
        పరిధి_FLAG_MASTER | పరిధి_FLAG_SUBSPAN | పరిధి_FLAG_UNMAPPED_MASTER;
    rpmalloc_stat_కూడు(&_unmapped_master_spans, 1);
  }

  ఒకవేళ (atomic_add32(&master->మిగిలిన_spans, -(int32_t)పరిధి_లెక్క) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & పరిధి_FLAG_MASTER) &&
                        !!(master->flags & పరిధి_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_లెక్క = master->పరిధి_లెక్క;
    ఒకవేళ (జ్ఞాపకం_పరిధి_కొలత < జ్ఞాపకం_పుట_కొలత)
      unmap_లెక్క = master->మొత్తం_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_లెక్క * జ్ఞాపకం_పరిధి_కొలత,
                    master->align_స్థానభ్రంశం,
                    (size_t)master->మొత్తం_spans * జ్ఞాపకం_పరిధి_కొలత);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
స్థిర శూన్య rpmalloc_పరిధి_release_గమ్యం_cache(కుప్ప_t *కుప్ప, పరిధి_t *పరిధి) {
  rpmalloc_assert(కుప్ప == పరిధి->కుప్ప, "Span heap pointer corrupted");
  rpmalloc_assert(పరిధి->కొలత_class < కొలత_CLASS_లెక్క,
                  "Invalid span size class");
  rpmalloc_assert(పరిధి->పరిధి_లెక్క == 1, "Invalid span count");
#ఒకవేళ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&కుప్ప->పరిధి_use[0].current);
#అయితే_ముగింపు
  _rpmalloc_stat_dec(&కుప్ప->కొలత_class_use[పరిధి->కొలత_class].spans_current);
  ఒకవేళ (!కుప్ప->finalize) {
    _rpmalloc_stat_inc(&కుప్ప->పరిధి_use[0].spans_గమ్యం_cache);
    _rpmalloc_stat_inc(&కుప్ప->కొలత_class_use[పరిధి->కొలత_class].spans_గమ్యం_cache);
    ఒకవేళ (కుప్ప->కొలత_class[పరిధి->కొలత_class].cache)
      rpmalloc_కుప్ప_cache_చొప్పించు(కుప్ప,
                                  కుప్ప->కొలత_class[పరిధి->కొలత_class].cache);
    కుప్ప->కొలత_class[పరిధి->కొలత_class].cache = పరిధి;
  } లేకపోతే {
    rpmalloc_పరిధి_unmap(పరిధి);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
స్థిర uint32_t free_list_partial_init(శూన్య **list, శూన్య **మొదటి_ఖండం,
                                       శూన్య *పుట_ప్రారంభం, శూన్య *ఖండం_ప్రారంభం,
                                       uint32_t ఖండం_లెక్క,
                                       uint32_t ఖండం_కొలత_2) {
  rpmalloc_assert(ఖండం_లెక్క, "Internal failure");
  *మొదటి_ఖండం = ఖండం_ప్రారంభం;
  ఒకవేళ (ఖండం_లెక్క > 1) {
    శూన్య *free_ఖండం = సూచిక_స్థానభ్రంశం(ఖండం_ప్రారంభం, ఖండం_కొలత_2);
    శూన్య *ఖండం_చివర =
        సూచిక_స్థానభ్రంశం(ఖండం_ప్రారంభం, (size_t)ఖండం_కొలత_2 * ఖండం_లెక్క);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    ఒకవేళ (ఖండం_కొలత_2 < (జ్ఞాపకం_పుట_కొలత >> 1)) {
      శూన్య *పుట_చివర = సూచిక_స్థానభ్రంశం(పుట_ప్రారంభం, జ్ఞాపకం_పుట_కొలత);
      ఒకవేళ (పుట_చివర < ఖండం_చివర)
        ఖండం_చివర = పుట_చివర;
    }
    *list = free_ఖండం;
    ఖండం_లెక్క = 2;
    శూన్య *తదుపరి_ఖండం = సూచిక_స్థానభ్రంశం(free_ఖండం, ఖండం_కొలత_2);
    అయితే (తదుపరి_ఖండం < ఖండం_చివర) {
      *((శూన్య **)free_ఖండం) = తదుపరి_ఖండం;
      free_ఖండం = తదుపరి_ఖండం;
      ++ఖండం_లెక్క;
      తదుపరి_ఖండం = సూచిక_స్థానభ్రంశం(తదుపరి_ఖండం, ఖండం_కొలత_2);
    }
    *((శూన్య **)free_ఖండం) = 0;
  } లేకపోతే {
    *list = 0;
  }
  తిరిగి ఖండం_లెక్క;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
స్థిర శూన్య *rpmalloc_పరిధి_initialize_new(కుప్ప_t *కుప్ప,
                                           కుప్ప_కొలత_class_t *కుప్ప_కొలత_class,
                                           పరిధి_t *పరిధి, uint32_t class_idx) {
  rpmalloc_assert(పరిధి->పరిధి_లెక్క == 1, "Internal failure");
  కొలత_class_t *కొలత_class = జ్ఞాపకం_కొలత_class + class_idx;
  పరిధి->కొలత_class = class_idx;
  పరిధి->కుప్ప = కుప్ప;
  పరిధి->flags &= ~పరిధి_FLAG_ALIGNED_ఖండాలు;
  పరిధి->ఖండం_కొలత_2 = కొలత_class->ఖండం_కొలత_2;
  పరిధి->ఖండం_లెక్క = కొలత_class->ఖండం_లెక్క;
  పరిధి->free_list = 0;
  పరిధి->list_కొలత = 0;
  atomic_store_ptr_release(&పరిధి->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  శూన్య *ఖండం;
  పరిధి->free_list_హద్దు =
      free_list_partial_init(&కుప్ప_కొలత_class->free_list, &ఖండం, పరిధి,
                             సూచిక_స్థానభ్రంశం(పరిధి, పరిధి_శీర్షిక_కొలత),
                             కొలత_class->ఖండం_లెక్క, కొలత_class->ఖండం_కొలత_2);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  ఒకవేళ (పరిధి->free_list_హద్దు < పరిధి->ఖండం_లెక్క) {
    rpmalloc_పరిధి_double_link_list_కూడు(&కుప్ప_కొలత_class->partial_పరిధి, పరిధి);
    పరిధి->వాడిన_లెక్క = పరిధి->free_list_హద్దు;
  } లేకపోతే {
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
    rpmalloc_పరిధి_double_link_list_కూడు(&కుప్ప->నిండిన_పరిధి[class_idx], పరిధి);
#అయితే_ముగింపు
    ++కుప్ప->నిండిన_పరిధి_లెక్క;
    పరిధి->వాడిన_లెక్క = పరిధి->ఖండం_లెక్క;
  }
  తిరిగి ఖండం;
}

స్థిర శూన్య rpmalloc_పరిధి_వెలికితీయి_free_list_deferred(పరిధి_t *పరిధి) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  చేయి {
    పరిధి->free_list =
        atomic_exchange_ptr_acquire(&పరిధి->free_list_deferred, INVALID_సూచిక);
  } అయితే (పరిధి->free_list == INVALID_సూచిక);
  పరిధి->వాడిన_లెక్క -= పరిధి->list_కొలత;
  పరిధి->list_కొలత = 0;
  atomic_store_ptr_release(&పరిధి->free_list_deferred, 0);
}

స్థిర పూర్ణాంకం rpmalloc_పరిధి_is_fully_utilized(పరిధి_t *పరిధి) {
  rpmalloc_assert(పరిధి->free_list_హద్దు <= పరిధి->ఖండం_లెక్క,
                  "Span free list corrupted");
  తిరిగి !పరిధి->free_list && (పరిధి->free_list_హద్దు >= పరిధి->ఖండం_లెక్క);
}

స్థిర పూర్ణాంకం rpmalloc_పరిధి_finalize(కుప్ప_t *కుప్ప, size_t iclass, పరిధి_t *పరిధి,
                                   పరిధి_t **list_తల) {
  శూన్య *free_list = కుప్ప->కొలత_class[iclass].free_list;
  పరిధి_t *class_పరిధి = (పరిధి_t *)((uintptr_t)free_list & జ్ఞాపకం_పరిధి_ముసుగు);
  ఒకవేళ (పరిధి == class_పరిధి) {
    // Adopt the heap class free list back into the span free list
    శూన్య *ఖండం = పరిధి->free_list;
    శూన్య *చివరి_ఖండం = 0;
    అయితే (ఖండం) {
      చివరి_ఖండం = ఖండం;
      ఖండం = *((శూన్య **)ఖండం);
    }
    uint32_t free_లెక్క = 0;
    ఖండం = free_list;
    అయితే (ఖండం) {
      ++free_లెక్క;
      ఖండం = *((శూన్య **)ఖండం);
    }
    ఒకవేళ (చివరి_ఖండం) {
      *((శూన్య **)చివరి_ఖండం) = free_list;
    } లేకపోతే {
      పరిధి->free_list = free_list;
    }
    కుప్ప->కొలత_class[iclass].free_list = 0;
    పరిధి->వాడిన_లెక్క -= free_లెక్క;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(పరిధి->list_కొలత == పరిధి->వాడిన_లెక్క, "Memory leak detected");
  ఒకవేళ (పరిధి->list_కొలత == పరిధి->వాడిన_లెక్క) {
    _rpmalloc_stat_dec(&కుప్ప->పరిధి_use[0].current);
    _rpmalloc_stat_dec(&కుప్ప->కొలత_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    ఒకవేళ (list_తల)
      rpmalloc_పరిధి_double_link_list_remove(list_తల, పరిధి);
    rpmalloc_పరిధి_unmap(పరిధి);
    తిరిగి 1;
  }
  తిరిగి 0;
}

////////////
///
/// Global cache
///
//////

#ఒకవేళ ENABLE_GLOBAL_CACHE

//! Finalize a global cache
స్థిర శూన్య _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  అయితే (!atomic_cas32_acquire(&cache->తాళం_2, 1, 0))
    _rpmalloc_spin();

  కొరకు (size_t ispan = 0; ispan < cache->లెక్క; ++ispan)
    rpmalloc_పరిధి_unmap(cache->పరిధి[ispan]);
  cache->లెక్క = 0;

  అయితే (cache->అధికప్రవాహం_2) {
    పరిధి_t *పరిధి = cache->అధికప్రవాహం_2;
    cache->అధికప్రవాహం_2 = పరిధి->తదుపరి;
    rpmalloc_పరిధి_unmap(పరిధి);
  }

  atomic_store32_release(&cache->తాళం_2, 0);
}

స్థిర శూన్య rpmalloc_global_cache_చొప్పించు_spans(పరిధి_t **పరిధి,
                                                size_t పరిధి_లెక్క,
                                                size_t లెక్క) {
  స్థిరరాశి size_t cache_హద్దు =
      (పరిధి_లెక్క == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_పరిధి_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_పరిధి_LARGE_CACHE - (పరిధి_లెక్క >> 1));

  global_cache_t *cache = &జ్ఞాపకం_పరిధి_cache[పరిధి_లెక్క - 1];

  size_t చొప్పించు_లెక్క = లెక్క;
  అయితే (!atomic_cas32_acquire(&cache->తాళం_2, 1, 0))
    _rpmalloc_spin();

#ఒకవేళ ENABLE_STATISTICS
  cache->చొప్పించు_లెక్క += లెక్క;
#అయితే_ముగింపు
  ఒకవేళ ((cache->లెక్క + చొప్పించు_లెక్క) > cache_హద్దు)
    చొప్పించు_లెక్క = cache_హద్దు - cache->లెక్క;

  జ్ఞాపకం_నకలు(cache->పరిధి + cache->లెక్క, పరిధి, పరిమాణం(పరిధి_t *) * చొప్పించు_లెక్క);
  cache->లెక్క += (uint32_t)చొప్పించు_లెక్క;

#ఒకవేళ ENABLE_UNLIMITED_CACHE
  అయితే (చొప్పించు_లెక్క < లెక్క) {
#లేకపోతే
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  అయితే ((జ్ఞాపకం_పుట_కొలత > జ్ఞాపకం_పరిధి_కొలత) && (చొప్పించు_లెక్క < లెక్క)) {
#అయితే_ముగింపు
    పరిధి_t *current_పరిధి = పరిధి[చొప్పించు_లెక్క++];
    current_పరిధి->తదుపరి = cache->అధికప్రవాహం_2;
    cache->అధికప్రవాహం_2 = current_పరిధి;
  }
  atomic_store32_release(&cache->తాళం_2, 0);

  పరిధి_t *keep = 0;
  కొరకు (size_t ispan = చొప్పించు_లెక్క; ispan < లెక్క; ++ispan) {
    పరిధి_t *current_పరిధి = పరిధి[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    ఒకవేళ ((current_పరిధి->flags & పరిధి_FLAG_MASTER) &&
        (atomic_load32(&current_పరిధి->మిగిలిన_spans) >
         (int32_t)current_పరిధి->పరిధి_లెక్క)) {
      current_పరిధి->తదుపరి = keep;
      keep = current_పరిధి;
    } లేకపోతే {
      rpmalloc_పరిధి_unmap(current_పరిధి);
    }
  }

  ఒకవేళ (keep) {
    అయితే (!atomic_cas32_acquire(&cache->తాళం_2, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    అయితే (keep) {
      కొరకు (; islot < cache->లెక్క; ++islot) {
        పరిధి_t *current_పరిధి = cache->పరిధి[islot];
        ఒకవేళ (!(current_పరిధి->flags & పరిధి_FLAG_MASTER) ||
            ((current_పరిధి->flags & పరిధి_FLAG_MASTER) &&
             (atomic_load32(&current_పరిధి->మిగిలిన_spans) <=
              (int32_t)current_పరిధి->పరిధి_లెక్క))) {
          rpmalloc_పరిధి_unmap(current_పరిధి);
          cache->పరిధి[islot] = keep;
          ఆపు;
        }
      }
      ఒకవేళ (islot == cache->లెక్క)
        ఆపు;
      keep = keep->తదుపరి;
    }

    ఒకవేళ (keep) {
      పరిధి_t *తోక_2 = keep;
      అయితే (తోక_2->తదుపరి)
        తోక_2 = తోక_2->తదుపరి;
      తోక_2->తదుపరి = cache->అధికప్రవాహం_2;
      cache->అధికప్రవాహం_2 = keep;
    }

    atomic_store32_release(&cache->తాళం_2, 0);
  }
}

స్థిర size_t rpmalloc_global_cache_వెలికితీయి_spans(పరిధి_t **పరిధి,
                                                   size_t పరిధి_లెక్క,
                                                   size_t లెక్క) {
  global_cache_t *cache = &జ్ఞాపకం_పరిధి_cache[పరిధి_లెక్క - 1];

  size_t వెలికితీయి_లెక్క = 0;
  అయితే (!atomic_cas32_acquire(&cache->తాళం_2, 1, 0))
    _rpmalloc_spin();

#ఒకవేళ ENABLE_STATISTICS
  cache->వెలికితీయి_లెక్క += లెక్క;
#అయితే_ముగింపు
  size_t want = లెక్క - వెలికితీయి_లెక్క;
  ఒకవేళ (want > cache->లెక్క)
    want = cache->లెక్క;

  జ్ఞాపకం_నకలు(పరిధి + వెలికితీయి_లెక్క, cache->పరిధి + (cache->లెక్క - want),
         పరిమాణం(పరిధి_t *) * want);
  cache->లెక్క -= (uint32_t)want;
  వెలికితీయి_లెక్క += want;

  అయితే ((వెలికితీయి_లెక్క < లెక్క) && cache->అధికప్రవాహం_2) {
    పరిధి_t *current_పరిధి = cache->అధికప్రవాహం_2;
    పరిధి[వెలికితీయి_లెక్క++] = current_పరిధి;
    cache->అధికప్రవాహం_2 = current_పరిధి->తదుపరి;
  }

#ఒకవేళ ENABLE_ASSERTS
  కొరకు (size_t ispan = 0; ispan < వెలికితీయి_లెక్క; ++ispan) {
    rpmalloc_assert(పరిధి[ispan]->పరిధి_లెక్క == పరిధి_లెక్క,
                    "Global cache span count mismatch");
  }
#అయితే_ముగింపు

  atomic_store32_release(&cache->తాళం_2, 0);

  తిరిగి వెలికితీయి_లెక్క;
}

#అయితే_ముగింపు

////////////
///
/// Heap control
///
//////

స్థిర శూన్య _rpmalloc_deallocate_huge(పరిధి_t *);

//! Store the given spans as reserve in the given heap
స్థిర శూన్య rpmalloc_కుప్ప_సమితి_reserved_spans(కుప్ప_t *కుప్ప, పరిధి_t *master,
                                              పరిధి_t *reserve,
                                              size_t reserve_పరిధి_లెక్క) {
  కుప్ప->పరిధి_reserve_master = master;
  కుప్ప->పరిధి_reserve = reserve;
  కుప్ప->spans_reserved = (uint32_t)reserve_పరిధి_లెక్క;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
స్థిర శూన్య rpmalloc_కుప్ప_cache_adopt_deferred(కుప్ప_t *కుప్ప,
                                                పరిధి_t **single_పరిధి) {
  పరిధి_t *పరిధి = (పరిధి_t *)((శూన్య *)atomic_exchange_ptr_acquire(
      &కుప్ప->పరిధి_free_deferred, 0));
  అయితే (పరిధి) {
    పరిధి_t *తదుపరి_పరిధి = (పరిధి_t *)పరిధి->free_list;
    rpmalloc_assert(పరిధి->కుప్ప == కుప్ప, "Span heap pointer corrupted");
    ఒకవేళ (EXPECTED(పరిధి->కొలత_class < కొలత_CLASS_లెక్క)) {
      rpmalloc_assert(కుప్ప->నిండిన_పరిధి_లెక్క, "Heap span counter corrupted");
      --కుప్ప->నిండిన_పరిధి_లెక్క;
      _rpmalloc_stat_dec(&కుప్ప->పరిధి_use[0].spans_deferred);
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
      rpmalloc_పరిధి_double_link_list_remove(&కుప్ప->నిండిన_పరిధి[పరిధి->కొలత_class],
                                             పరిధి);
#అయితే_ముగింపు
      _rpmalloc_stat_dec(&కుప్ప->పరిధి_use[0].current);
      _rpmalloc_stat_dec(&కుప్ప->కొలత_class_use[పరిధి->కొలత_class].spans_current);
      ఒకవేళ (single_పరిధి && !*single_పరిధి)
        *single_పరిధి = పరిధి;
      లేకపోతే
        rpmalloc_కుప్ప_cache_చొప్పించు(కుప్ప, పరిధి);
    } లేకపోతే {
      ఒకవేళ (పరిధి->కొలత_class == కొలత_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(పరిధి);
      } లేకపోతే {
        rpmalloc_assert(పరిధి->కొలత_class == కొలత_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(కుప్ప->నిండిన_పరిధి_లెక్క, "Heap span counter corrupted");
        --కుప్ప->నిండిన_పరిధి_లెక్క;
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
        rpmalloc_పరిధి_double_link_list_remove(&కుప్ప->large_huge_పరిధి, పరిధి);
#అయితే_ముగింపు
        uint32_t idx = పరిధి->పరిధి_లెక్క - 1;
        _rpmalloc_stat_dec(&కుప్ప->పరిధి_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&కుప్ప->పరిధి_use[idx].current);
        ఒకవేళ (!idx && single_పరిధి && !*single_పరిధి)
          *single_పరిధి = పరిధి;
        లేకపోతే
          rpmalloc_కుప్ప_cache_చొప్పించు(కుప్ప, పరిధి);
      }
    }
    పరిధి = తదుపరి_పరిధి;
  }
}

స్థిర శూన్య rpmalloc_కుప్ప_unmap(కుప్ప_t *కుప్ప) {
  ఒకవేళ (!కుప్ప->master_కుప్ప) {
    ఒకవేళ ((కుప్ప->finalize > 1) && !atomic_load32(&కుప్ప->బిడ్డ_లెక్క)) {
      పరిధి_t *పరిధి = (పరిధి_t *)((uintptr_t)కుప్ప & జ్ఞాపకం_పరిధి_ముసుగు);
      rpmalloc_పరిధి_unmap(పరిధి);
    }
  } లేకపోతే {
    ఒకవేళ (atomic_decr32(&కుప్ప->master_కుప్ప->బిడ్డ_లెక్క) == 0) {
      rpmalloc_కుప్ప_unmap(కుప్ప->master_కుప్ప);
    }
  }
}

స్థిర శూన్య rpmalloc_కుప్ప_global_finalize(కుప్ప_t *కుప్ప) {
  ఒకవేళ (కుప్ప->finalize++ > 1) {
    --కుప్ప->finalize;
    తిరిగి;
  }

  rpmalloc_కుప్ప_finalize(కుప్ప);

#ఒకవేళ ENABLE_THREAD_CACHE
  కొరకు (size_t iclass = 0; iclass < LARGE_CLASS_లెక్క; ++iclass) {
    పరిధి_cache_t *పరిధి_cache;
    ఒకవేళ (!iclass)
      పరిధి_cache = &కుప్ప->పరిధి_cache;
    లేకపోతే
      పరిధి_cache = (పరిధి_cache_t *)(కుప్ప->పరిధి_large_cache + (iclass - 1));
    కొరకు (size_t ispan = 0; ispan < పరిధి_cache->లెక్క; ++ispan)
      rpmalloc_పరిధి_unmap(పరిధి_cache->పరిధి[ispan]);
    పరిధి_cache->లెక్క = 0;
  }
#అయితే_ముగింపు

  ఒకవేళ (కుప్ప->నిండిన_పరిధి_లెక్క) {
    --కుప్ప->finalize;
    తిరిగి;
  }

  కొరకు (size_t iclass = 0; iclass < కొలత_CLASS_లెక్క; ++iclass) {
    ఒకవేళ (కుప్ప->కొలత_class[iclass].free_list ||
        కుప్ప->కొలత_class[iclass].partial_పరిధి) {
      --కుప్ప->finalize;
      తిరిగి;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)కుప్ప->id % కుప్ప_ARRAY_కొలత;
  కుప్ప_t *list_కుప్ప = జ్ఞాపకం_heaps[list_idx];
  ఒకవేళ (list_కుప్ప == కుప్ప) {
    జ్ఞాపకం_heaps[list_idx] = కుప్ప->తదుపరి_కుప్ప;
  } లేకపోతే {
    అయితే (list_కుప్ప->తదుపరి_కుప్ప != కుప్ప)
      list_కుప్ప = list_కుప్ప->తదుపరి_కుప్ప;
    list_కుప్ప->తదుపరి_కుప్ప = కుప్ప->తదుపరి_కుప్ప;
  }

  rpmalloc_కుప్ప_unmap(కుప్ప);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
స్థిర శూన్య rpmalloc_కుప్ప_cache_చొప్పించు(కుప్ప_t *కుప్ప, పరిధి_t *పరిధి) {
  ఒకవేళ (UNEXPECTED(కుప్ప->finalize != 0)) {
    rpmalloc_పరిధి_unmap(పరిధి);
    rpmalloc_కుప్ప_global_finalize(కుప్ప);
    తిరిగి;
  }
#ఒకవేళ ENABLE_THREAD_CACHE
  size_t పరిధి_లెక్క = పరిధి->పరిధి_లెక్క;
  _rpmalloc_stat_inc(&కుప్ప->పరిధి_use[పరిధి_లెక్క - 1].spans_గమ్యం_cache);
  ఒకవేళ (పరిధి_లెక్క == 1) {
    పరిధి_cache_t *పరిధి_cache = &కుప్ప->పరిధి_cache;
    పరిధి_cache->పరిధి[పరిధి_cache->లెక్క++] = పరిధి;
    ఒకవేళ (పరిధి_cache->లెక్క == MAX_THREAD_పరిధి_CACHE) {
      స్థిరరాశి size_t remain_లెక్క =
          MAX_THREAD_పరిధి_CACHE - THREAD_పరిధి_CACHE_TRANSFER;
#ఒకవేళ ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&కుప్ప->thread_గమ్యం_global,
                           THREAD_పరిధి_CACHE_TRANSFER * జ్ఞాపకం_పరిధి_కొలత);
      rpmalloc_stat_కూడు(&కుప్ప->పరిధి_use[పరిధి_లెక్క - 1].spans_గమ్యం_global,
                         THREAD_పరిధి_CACHE_TRANSFER);
      rpmalloc_global_cache_చొప్పించు_spans(పరిధి_cache->పరిధి + remain_లెక్క,
                                          పరిధి_లెక్క,
                                          THREAD_పరిధి_CACHE_TRANSFER);
#లేకపోతే
      కొరకు (size_t ispan = 0; ispan < THREAD_పరిధి_CACHE_TRANSFER; ++ispan)
        rpmalloc_పరిధి_unmap(పరిధి_cache->పరిధి[remain_లెక్క + ispan]);
#అయితే_ముగింపు
      పరిధి_cache->లెక్క = remain_లెక్క;
    }
  } లేకపోతే {
    size_t cache_idx = పరిధి_లెక్క - 2;
    పరిధి_large_cache_t *పరిధి_cache = కుప్ప->పరిధి_large_cache + cache_idx;
    పరిధి_cache->పరిధి[పరిధి_cache->లెక్క++] = పరిధి;
    స్థిరరాశి size_t cache_హద్దు =
        (MAX_THREAD_పరిధి_LARGE_CACHE - (పరిధి_లెక్క >> 1));
    ఒకవేళ (పరిధి_cache->లెక్క == cache_హద్దు) {
      స్థిరరాశి size_t transfer_హద్దు = 2 + (cache_హద్దు >> 2);
      స్థిరరాశి size_t transfer_లెక్క =
          (THREAD_పరిధి_LARGE_CACHE_TRANSFER <= transfer_హద్దు
               ? THREAD_పరిధి_LARGE_CACHE_TRANSFER
               : transfer_హద్దు);
      స్థిరరాశి size_t remain_లెక్క = cache_హద్దు - transfer_లెక్క;
#ఒకవేళ ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&కుప్ప->thread_గమ్యం_global,
                           transfer_లెక్క * పరిధి_లెక్క * జ్ఞాపకం_పరిధి_కొలత);
      rpmalloc_stat_కూడు(&కుప్ప->పరిధి_use[పరిధి_లెక్క - 1].spans_గమ్యం_global,
                         transfer_లెక్క);
      rpmalloc_global_cache_చొప్పించు_spans(పరిధి_cache->పరిధి + remain_లెక్క,
                                          పరిధి_లెక్క, transfer_లెక్క);
#లేకపోతే
      కొరకు (size_t ispan = 0; ispan < transfer_లెక్క; ++ispan)
        rpmalloc_పరిధి_unmap(పరిధి_cache->పరిధి[remain_లెక్క + ispan]);
#అయితే_ముగింపు
      పరిధి_cache->లెక్క = remain_లెక్క;
    }
  }
#లేకపోతే
  (శూన్య)పరిమాణం(కుప్ప);
  rpmalloc_పరిధి_unmap(పరిధి);
#అయితే_ముగింపు
}

//! Extract the given number of spans from the different cache levels
స్థిర పరిధి_t *rpmalloc_కుప్ప_thread_cache_వెలికితీయి(కుప్ప_t *కుప్ప,
                                                   size_t పరిధి_లెక్క) {
  పరిధి_t *పరిధి = 0;
#ఒకవేళ ENABLE_THREAD_CACHE
  పరిధి_cache_t *పరిధి_cache;
  ఒకవేళ (పరిధి_లెక్క == 1)
    పరిధి_cache = &కుప్ప->పరిధి_cache;
  లేకపోతే
    పరిధి_cache = (పరిధి_cache_t *)(కుప్ప->పరిధి_large_cache + (పరిధి_లెక్క - 2));
  ఒకవేళ (పరిధి_cache->లెక్క) {
    _rpmalloc_stat_inc(&కుప్ప->పరిధి_use[పరిధి_లెక్క - 1].spans_from_cache);
    తిరిగి పరిధి_cache->పరిధి[--పరిధి_cache->లెక్క];
  }
#అయితే_ముగింపు
  తిరిగి పరిధి;
}

స్థిర పరిధి_t *rpmalloc_కుప్ప_thread_cache_deferred_వెలికితీయి(కుప్ప_t *కుప్ప,
                                                            size_t పరిధి_లెక్క) {
  పరిధి_t *పరిధి = 0;
  ఒకవేళ (పరిధి_లెక్క == 1) {
    rpmalloc_కుప్ప_cache_adopt_deferred(కుప్ప, &పరిధి);
  } లేకపోతే {
    rpmalloc_కుప్ప_cache_adopt_deferred(కుప్ప, 0);
    పరిధి = rpmalloc_కుప్ప_thread_cache_వెలికితీయి(కుప్ప, పరిధి_లెక్క);
  }
  తిరిగి పరిధి;
}

స్థిర పరిధి_t *rpmalloc_కుప్ప_reserved_వెలికితీయి(కుప్ప_t *కుప్ప,
                                               size_t పరిధి_లెక్క) {
  ఒకవేళ (కుప్ప->spans_reserved >= పరిధి_లెక్క)
    తిరిగి rpmalloc_పరిధి_map(కుప్ప, పరిధి_లెక్క);
  తిరిగి 0;
}

//! Extract a span from the global cache
స్థిర పరిధి_t *rpmalloc_కుప్ప_global_cache_వెలికితీయి(కుప్ప_t *కుప్ప,
                                                   size_t పరిధి_లెక్క) {
#ఒకవేళ ENABLE_GLOBAL_CACHE
#ఒకవేళ ENABLE_THREAD_CACHE
  పరిధి_cache_t *పరిధి_cache;
  size_t wanted_లెక్క;
  ఒకవేళ (పరిధి_లెక్క == 1) {
    పరిధి_cache = &కుప్ప->పరిధి_cache;
    wanted_లెక్క = THREAD_పరిధి_CACHE_TRANSFER;
  } లేకపోతే {
    పరిధి_cache = (పరిధి_cache_t *)(కుప్ప->పరిధి_large_cache + (పరిధి_లెక్క - 2));
    wanted_లెక్క = THREAD_పరిధి_LARGE_CACHE_TRANSFER;
  }
  పరిధి_cache->లెక్క = rpmalloc_global_cache_వెలికితీయి_spans(
      పరిధి_cache->పరిధి, పరిధి_లెక్క, wanted_లెక్క);
  ఒకవేళ (పరిధి_cache->లెక్క) {
    _rpmalloc_stat_add64(&కుప్ప->global_గమ్యం_thread,
                         పరిధి_లెక్క * పరిధి_cache->లెక్క * జ్ఞాపకం_పరిధి_కొలత);
    rpmalloc_stat_కూడు(&కుప్ప->పరిధి_use[పరిధి_లెక్క - 1].spans_from_global,
                       పరిధి_cache->లెక్క);
    తిరిగి పరిధి_cache->పరిధి[--పరిధి_cache->లెక్క];
  }
#లేకపోతే
  పరిధి_t *పరిధి = 0;
  size_t లెక్క = rpmalloc_global_cache_వెలికితీయి_spans(&పరిధి, పరిధి_లెక్క, 1);
  ఒకవేళ (లెక్క) {
    _rpmalloc_stat_add64(&కుప్ప->global_గమ్యం_thread,
                         పరిధి_లెక్క * లెక్క * జ్ఞాపకం_పరిధి_కొలత);
    rpmalloc_stat_కూడు(&కుప్ప->పరిధి_use[పరిధి_లెక్క - 1].spans_from_global,
                       లెక్క);
    తిరిగి పరిధి;
  }
#అయితే_ముగింపు
#అయితే_ముగింపు
  (శూన్య)పరిమాణం(కుప్ప);
  (శూన్య)పరిమాణం(పరిధి_లెక్క);
  తిరిగి 0;
}

స్థిర శూన్య rpmalloc_inc_పరిధి_statistics(కుప్ప_t *కుప్ప, size_t పరిధి_లెక్క,
                                          uint32_t class_idx) {
  (శూన్య)పరిమాణం(కుప్ప);
  (శూన్య)పరిమాణం(పరిధి_లెక్క);
  (శూన్య)పరిమాణం(class_idx);
#ఒకవేళ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)పరిధి_లెక్క - 1;
  uint32_t current_లెక్క =
      (uint32_t)atomic_incr32(&కుప్ప->పరిధి_use[idx].current);
  ఒకవేళ (current_లెక్క > (uint32_t)atomic_load32(&కుప్ప->పరిధి_use[idx].ఎగువ))
    atomic_store32(&కుప్ప->పరిధి_use[idx].ఎగువ, (int32_t)current_లెక్క);
  rpmalloc_stat_కూడు_peak(&కుప్ప->కొలత_class_use[class_idx].spans_current, 1,
                          కుప్ప->కొలత_class_use[class_idx].spans_peak);
#అయితే_ముగింపు
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
స్థిర పరిధి_t *
rpmalloc_కుప్ప_వెలికితీయి_new_పరిధి(కుప్ప_t *కుప్ప,
                                కుప్ప_కొలత_class_t *కుప్ప_కొలత_class,
                                size_t పరిధి_లెక్క, uint32_t class_idx) {
  పరిధి_t *పరిధి;
#ఒకవేళ ENABLE_THREAD_CACHE
  ఒకవేళ (కుప్ప_కొలత_class && కుప్ప_కొలత_class->cache) {
    పరిధి = కుప్ప_కొలత_class->cache;
    కుప్ప_కొలత_class->cache =
        (కుప్ప->పరిధి_cache.లెక్క
             ? కుప్ప->పరిధి_cache.పరిధి[--కుప్ప->పరిధి_cache.లెక్క]
             : 0);
    rpmalloc_inc_పరిధి_statistics(కుప్ప, పరిధి_లెక్క, class_idx);
    తిరిగి పరిధి;
  }
#అయితే_ముగింపు
  (శూన్య)పరిమాణం(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t పునాది_పరిధి_లెక్క = పరిధి_లెక్క;
  size_t హద్దు_పరిధి_లెక్క =
      (పరిధి_లెక్క > 2) ? (పరిధి_లెక్క + (పరిధి_లెక్క >> 1)) : పరిధి_లెక్క;
  ఒకవేళ (హద్దు_పరిధి_లెక్క > LARGE_CLASS_లెక్క)
    హద్దు_పరిధి_లెక్క = LARGE_CLASS_లెక్క;
  చేయి {
    పరిధి = rpmalloc_కుప్ప_thread_cache_వెలికితీయి(కుప్ప, పరిధి_లెక్క);
    ఒకవేళ (EXPECTED(పరిధి != 0)) {
      _rpmalloc_stat_inc(&కుప్ప->కొలత_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_పరిధి_statistics(కుప్ప, పరిధి_లెక్క, class_idx);
      తిరిగి పరిధి;
    }
    పరిధి = rpmalloc_కుప్ప_thread_cache_deferred_వెలికితీయి(కుప్ప, పరిధి_లెక్క);
    ఒకవేళ (EXPECTED(పరిధి != 0)) {
      _rpmalloc_stat_inc(&కుప్ప->కొలత_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_పరిధి_statistics(కుప్ప, పరిధి_లెక్క, class_idx);
      తిరిగి పరిధి;
    }
    పరిధి = rpmalloc_కుప్ప_global_cache_వెలికితీయి(కుప్ప, పరిధి_లెక్క);
    ఒకవేళ (EXPECTED(పరిధి != 0)) {
      _rpmalloc_stat_inc(&కుప్ప->కొలత_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_పరిధి_statistics(కుప్ప, పరిధి_లెక్క, class_idx);
      తిరిగి పరిధి;
    }
    పరిధి = rpmalloc_కుప్ప_reserved_వెలికితీయి(కుప్ప, పరిధి_లెక్క);
    ఒకవేళ (EXPECTED(పరిధి != 0)) {
      _rpmalloc_stat_inc(&కుప్ప->కొలత_class_use[class_idx].spans_from_reserved);
      rpmalloc_inc_పరిధి_statistics(కుప్ప, పరిధి_లెక్క, class_idx);
      తిరిగి పరిధి;
    }
    ++పరిధి_లెక్క;
  } అయితే (పరిధి_లెక్క <= హద్దు_పరిధి_లెక్క);
  // Final fallback, map in more virtual memory
  పరిధి = rpmalloc_పరిధి_map(కుప్ప, పునాది_పరిధి_లెక్క);
  rpmalloc_inc_పరిధి_statistics(కుప్ప, పునాది_పరిధి_లెక్క, class_idx);
  _rpmalloc_stat_inc(&కుప్ప->కొలత_class_use[class_idx].spans_map_calls);
  తిరిగి పరిధి;
}

స్థిర శూన్య rpmalloc_కుప్ప_initialize(కుప్ప_t *కుప్ప) {
  _rpmalloc_memset_const(కుప్ప, 0, పరిమాణం(కుప్ప_t));
  // Get a new heap ID
  కుప్ప->id = 1 + atomic_incr32(&జ్ఞాపకం_కుప్ప_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)కుప్ప->id % కుప్ప_ARRAY_కొలత;
  కుప్ప->తదుపరి_కుప్ప = జ్ఞాపకం_heaps[list_idx];
  జ్ఞాపకం_heaps[list_idx] = కుప్ప;
}

స్థిర శూన్య rpmalloc_కుప్ప_orphan(కుప్ప_t *కుప్ప, పూర్ణాంకం మొదటి_class) {
  కుప్ప->owner_thread = (uintptr_t)-1;
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
  కుప్ప_t **కుప్ప_list =
      (మొదటి_class ? &జ్ఞాపకం_మొదటి_class_orphan_heaps : &జ్ఞాపకం_orphan_heaps);
#లేకపోతే
  (శూన్య)పరిమాణం(మొదటి_class);
  కుప్ప_t **కుప్ప_list = &జ్ఞాపకం_orphan_heaps;
#అయితే_ముగింపు
  కుప్ప->తదుపరి_orphan = *కుప్ప_list;
  *కుప్ప_list = కుప్ప;
}

//! Allocate a new heap from newly mapped memory pages
స్థిర కుప్ప_t *rpmalloc_కుప్ప_allocate_new(శూన్య) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t కుప్ప_కొలత = పరిమాణం(కుప్ప_t);
  size_t aligned_కుప్ప_కొలత = 16 * ((కుప్ప_కొలత + 15) / 16);
  size_t request_కుప్ప_లెక్క = 16;
  size_t కుప్ప_పరిధి_లెక్క = ((aligned_కుప్ప_కొలత * request_కుప్ప_లెక్క) +
                            పరిమాణం(పరిధి_t) + జ్ఞాపకం_పరిధి_కొలత - 1) /
                           జ్ఞాపకం_పరిధి_కొలత;
  size_t ఖండం_కొలత_2 = జ్ఞాపకం_పరిధి_కొలత * కుప్ప_పరిధి_లెక్క;
  size_t పరిధి_లెక్క = కుప్ప_పరిధి_లెక్క;
  పరిధి_t *పరిధి = 0;
  // If there are global reserved spans, use these first
  ఒకవేళ (జ్ఞాపకం_global_reserve_లెక్క >= కుప్ప_పరిధి_లెక్క) {
    పరిధి = _rpmalloc_global_get_reserved_spans(కుప్ప_పరిధి_లెక్క);
  }
  ఒకవేళ (!పరిధి) {
    ఒకవేళ (జ్ఞాపకం_పుట_కొలత > ఖండం_కొలత_2) {
      పరిధి_లెక్క = జ్ఞాపకం_పుట_కొలత / జ్ఞాపకం_పరిధి_కొలత;
      ఖండం_కొలత_2 = జ్ఞాపకం_పుట_కొలత;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_కుప్ప_లెక్క =
          (ఖండం_కొలత_2 - పరిమాణం(పరిధి_t)) / aligned_కుప్ప_కొలత;
      ఒకవేళ (possible_కుప్ప_లెక్క >= (request_కుప్ప_లెక్క * 16))
        request_కుప్ప_లెక్క *= 16;
      లేకపోతే ఒకవేళ (possible_కుప్ప_లెక్క < request_కుప్ప_లెక్క)
        request_కుప్ప_లెక్క = possible_కుప్ప_లెక్క;
      కుప్ప_పరిధి_లెక్క = ((aligned_కుప్ప_కొలత * request_కుప్ప_లెక్క) +
                         పరిమాణం(పరిధి_t) + జ్ఞాపకం_పరిధి_కొలత - 1) /
                        జ్ఞాపకం_పరిధి_కొలత;
    }

    size_t align_స్థానభ్రంశం = 0;
    పరిధి = (పరిధి_t *)_rpmalloc_mmap(ఖండం_కొలత_2, &align_స్థానభ్రంశం);
    ఒకవేళ (!పరిధి)
      తిరిగి 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    rpmalloc_పరిధి_initialize(పరిధి, పరిధి_లెక్క, కుప్ప_పరిధి_లెక్క, align_స్థానభ్రంశం);
  }

  size_t remain_కొలత = జ్ఞాపకం_పరిధి_కొలత - పరిమాణం(పరిధి_t);
  కుప్ప_t *కుప్ప = (కుప్ప_t *)సూచిక_స్థానభ్రంశం(పరిధి, పరిమాణం(పరిధి_t));
  rpmalloc_కుప్ప_initialize(కుప్ప);

  // Put extra heaps as orphans
  size_t అంకె_heaps = remain_కొలత / aligned_కుప్ప_కొలత;
  ఒకవేళ (అంకె_heaps < request_కుప్ప_లెక్క)
    అంకె_heaps = request_కుప్ప_లెక్క;
  atomic_store32(&కుప్ప->బిడ్డ_లెక్క, (int32_t)అంకె_heaps - 1);
  కుప్ప_t *extra_కుప్ప = (కుప్ప_t *)సూచిక_స్థానభ్రంశం(కుప్ప, aligned_కుప్ప_కొలత);
  అయితే (అంకె_heaps > 1) {
    rpmalloc_కుప్ప_initialize(extra_కుప్ప);
    extra_కుప్ప->master_కుప్ప = కుప్ప;
    rpmalloc_కుప్ప_orphan(extra_కుప్ప, 1);
    extra_కుప్ప = (కుప్ప_t *)సూచిక_స్థానభ్రంశం(extra_కుప్ప, aligned_కుప్ప_కొలత);
    --అంకె_heaps;
  }

  ఒకవేళ (పరిధి_లెక్క > కుప్ప_పరిధి_లెక్క) {
    // Cap reserved spans
    size_t remain_లెక్క = పరిధి_లెక్క - కుప్ప_పరిధి_లెక్క;
    size_t reserve_లెక్క =
        (remain_లెక్క > జ్ఞాపకం_కుప్ప_reserve_లెక్క ? జ్ఞాపకం_కుప్ప_reserve_లెక్క
                                                   : remain_లెక్క);
    పరిధి_t *remain_పరిధి =
        (పరిధి_t *)సూచిక_స్థానభ్రంశం(పరిధి, కుప్ప_పరిధి_లెక్క * జ్ఞాపకం_పరిధి_కొలత);
    rpmalloc_కుప్ప_సమితి_reserved_spans(కుప్ప, పరిధి, remain_పరిధి, reserve_లెక్క);

    ఒకవేళ (remain_లెక్క > reserve_లెక్క) {
      // Set to global reserved spans
      remain_పరిధి = (పరిధి_t *)సూచిక_స్థానభ్రంశం(remain_పరిధి,
                                             reserve_లెక్క * జ్ఞాపకం_పరిధి_కొలత);
      reserve_లెక్క = remain_లెక్క - reserve_లెక్క;
      rpmalloc_global_సమితి_reserved_spans(పరిధి, remain_పరిధి, reserve_లెక్క);
    }
  }

  తిరిగి కుప్ప;
}

స్థిర కుప్ప_t *rpmalloc_కుప్ప_వెలికితీయి_orphan(కుప్ప_t **కుప్ప_list) {
  కుప్ప_t *కుప్ప = *కుప్ప_list;
  *కుప్ప_list = (కుప్ప ? కుప్ప->తదుపరి_orphan : 0);
  తిరిగి కుప్ప;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
స్థిర కుప్ప_t *rpmalloc_కుప్ప_allocate(పూర్ణాంకం మొదటి_class) {
  కుప్ప_t *కుప్ప = 0;
  అయితే (!atomic_cas32_acquire(&జ్ఞాపకం_global_తాళం, 1, 0))
    _rpmalloc_spin();
  ఒకవేళ (మొదటి_class == 0)
    కుప్ప = rpmalloc_కుప్ప_వెలికితీయి_orphan(&జ్ఞాపకం_orphan_heaps);
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
  ఒకవేళ (!కుప్ప)
    కుప్ప = rpmalloc_కుప్ప_వెలికితీయి_orphan(&జ్ఞాపకం_మొదటి_class_orphan_heaps);
#అయితే_ముగింపు
  ఒకవేళ (!కుప్ప)
    కుప్ప = rpmalloc_కుప్ప_allocate_new();
  atomic_store32_release(&జ్ఞాపకం_global_తాళం, 0);
  ఒకవేళ (కుప్ప)
    rpmalloc_కుప్ప_cache_adopt_deferred(కుప్ప, 0);
  తిరిగి కుప్ప;
}

స్థిర శూన్య rpmalloc_కుప్ప_release(శూన్య *heapptr, పూర్ణాంకం మొదటి_class,
                                   పూర్ణాంకం release_cache) {
  కుప్ప_t *కుప్ప = (కుప్ప_t *)heapptr;
  ఒకవేళ (!కుప్ప)
    తిరిగి;
  // Release thread cache spans back to global cache
  rpmalloc_కుప్ప_cache_adopt_deferred(కుప్ప, 0);
  ఒకవేళ (release_cache || కుప్ప->finalize) {
#ఒకవేళ ENABLE_THREAD_CACHE
    కొరకు (size_t iclass = 0; iclass < LARGE_CLASS_లెక్క; ++iclass) {
      పరిధి_cache_t *పరిధి_cache;
      ఒకవేళ (!iclass)
        పరిధి_cache = &కుప్ప->పరిధి_cache;
      లేకపోతే
        పరిధి_cache = (పరిధి_cache_t *)(కుప్ప->పరిధి_large_cache + (iclass - 1));
      ఒకవేళ (!పరిధి_cache->లెక్క)
        కొనసాగు;
#ఒకవేళ ENABLE_GLOBAL_CACHE
      ఒకవేళ (కుప్ప->finalize) {
        కొరకు (size_t ispan = 0; ispan < పరిధి_cache->లెక్క; ++ispan)
          rpmalloc_పరిధి_unmap(పరిధి_cache->పరిధి[ispan]);
      } లేకపోతే {
        _rpmalloc_stat_add64(&కుప్ప->thread_గమ్యం_global, పరిధి_cache->లెక్క *
                                                          (iclass + 1) *
                                                          జ్ఞాపకం_పరిధి_కొలత);
        rpmalloc_stat_కూడు(&కుప్ప->పరిధి_use[iclass].spans_గమ్యం_global,
                           పరిధి_cache->లెక్క);
        rpmalloc_global_cache_చొప్పించు_spans(పరిధి_cache->పరిధి, iclass + 1,
                                            పరిధి_cache->లెక్క);
      }
#లేకపోతే
      కొరకు (size_t ispan = 0; ispan < పరిధి_cache->లెక్క; ++ispan)
        rpmalloc_పరిధి_unmap(పరిధి_cache->పరిధి[ispan]);
#అయితే_ముగింపు
      పరిధి_cache->లెక్క = 0;
    }
#అయితే_ముగింపు
  }

  ఒకవేళ (get_thread_కుప్ప_raw() == కుప్ప)
    సమితి_thread_కుప్ప(0);

#ఒకవేళ ENABLE_STATISTICS
  atomic_decr32(&జ్ఞాపకం_చురుకైన_heaps);
  rpmalloc_assert(atomic_load32(&జ్ఞాపకం_చురుకైన_heaps) >= 0,
                  "Still active heaps during finalization");
#అయితే_ముగింపు

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  ఒకవేళ (get_thread_id() != _rpmalloc_main_thread_id) {
    అయితే (!atomic_cas32_acquire(&జ్ఞాపకం_global_తాళం, 1, 0))
      _rpmalloc_spin();
  }
  rpmalloc_కుప్ప_orphan(కుప్ప, మొదటి_class);
  atomic_store32_release(&జ్ఞాపకం_global_తాళం, 0);
}

స్థిర శూన్య rpmalloc_కుప్ప_release_raw(శూన్య *heapptr, పూర్ణాంకం release_cache) {
  rpmalloc_కుప్ప_release(heapptr, 0, release_cache);
}

స్థిర శూన్య rpmalloc_కుప్ప_release_raw_fc(శూన్య *heapptr) {
  rpmalloc_కుప్ప_release_raw(heapptr, 1);
}

స్థిర శూన్య rpmalloc_కుప్ప_finalize(కుప్ప_t *కుప్ప) {
  ఒకవేళ (కుప్ప->spans_reserved) {
    పరిధి_t *పరిధి = rpmalloc_పరిధి_map(కుప్ప, కుప్ప->spans_reserved);
    rpmalloc_పరిధి_unmap(పరిధి);
    కుప్ప->spans_reserved = 0;
  }

  rpmalloc_కుప్ప_cache_adopt_deferred(కుప్ప, 0);

  కొరకు (size_t iclass = 0; iclass < కొలత_CLASS_లెక్క; ++iclass) {
    ఒకవేళ (కుప్ప->కొలత_class[iclass].cache)
      rpmalloc_పరిధి_unmap(కుప్ప->కొలత_class[iclass].cache);
    కుప్ప->కొలత_class[iclass].cache = 0;
    పరిధి_t *పరిధి = కుప్ప->కొలత_class[iclass].partial_పరిధి;
    అయితే (పరిధి) {
      పరిధి_t *తదుపరి = పరిధి->తదుపరి;
      rpmalloc_పరిధి_finalize(కుప్ప, iclass, పరిధి,
                              &కుప్ప->కొలత_class[iclass].partial_పరిధి);
      పరిధి = తదుపరి;
    }
    // If class still has a free list it must be a full span
    ఒకవేళ (కుప్ప->కొలత_class[iclass].free_list) {
      పరిధి_t *class_పరిధి =
          (పరిధి_t *)((uintptr_t)కుప్ప->కొలత_class[iclass].free_list &
                     జ్ఞాపకం_పరిధి_ముసుగు);
      పరిధి_t **list = 0;
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
      list = &కుప్ప->నిండిన_పరిధి[iclass];
#అయితే_ముగింపు
      --కుప్ప->నిండిన_పరిధి_లెక్క;
      ఒకవేళ (!rpmalloc_పరిధి_finalize(కుప్ప, iclass, class_పరిధి, list)) {
        ఒకవేళ (list)
          rpmalloc_పరిధి_double_link_list_remove(list, class_పరిధి);
        rpmalloc_పరిధి_double_link_list_కూడు(
            &కుప్ప->కొలత_class[iclass].partial_పరిధి, class_పరిధి);
      }
    }
  }

#ఒకవేళ ENABLE_THREAD_CACHE
  కొరకు (size_t iclass = 0; iclass < LARGE_CLASS_లెక్క; ++iclass) {
    పరిధి_cache_t *పరిధి_cache;
    ఒకవేళ (!iclass)
      పరిధి_cache = &కుప్ప->పరిధి_cache;
    లేకపోతే
      పరిధి_cache = (పరిధి_cache_t *)(కుప్ప->పరిధి_large_cache + (iclass - 1));
    కొరకు (size_t ispan = 0; ispan < పరిధి_cache->లెక్క; ++ispan)
      rpmalloc_పరిధి_unmap(పరిధి_cache->పరిధి[ispan]);
    పరిధి_cache->లెక్క = 0;
  }
#అయితే_ముగింపు
  rpmalloc_assert(!atomic_ఎక్కించు_ptr(&కుప్ప->పరిధి_free_deferred),
                  "Heaps still active during finalization");
}

////////////
///
/// Allocation entry points
///
//////

//! Pop first block from a free list
స్థిర శూన్య *free_list_తీయి(శూన్య **list) {
  శూన్య *ఖండం = *list;
  *list = *((శూన్య **)ఖండం);
  తిరిగి ఖండం;
}

//! Allocate a small/medium sized memory block from the given heap
స్థిర శూన్య *rpmalloc_allocate_from_కుప్ప_fallback(
    కుప్ప_t *కుప్ప, కుప్ప_కొలత_class_t *కుప్ప_కొలత_class, uint32_t class_idx) {
  పరిధి_t *పరిధి = కుప్ప_కొలత_class->partial_పరిధి;
  rpmalloc_assume(కుప్ప != 0);
  ఒకవేళ (EXPECTED(పరిధి != 0)) {
    rpmalloc_assert(పరిధి->ఖండం_లెక్క ==
                        జ్ఞాపకం_కొలత_class[పరిధి->కొలత_class].ఖండం_లెక్క,
                    "Span block count corrupted");
    rpmalloc_assert(!rpmalloc_పరిధి_is_fully_utilized(పరిధి),
                    "Internal failure");
    శూన్య *ఖండం;
    ఒకవేళ (పరిధి->free_list) {
      // Span local free list is not empty, swap to size class free list
      ఖండం = free_list_తీయి(&పరిధి->free_list);
      కుప్ప_కొలత_class->free_list = పరిధి->free_list;
      పరిధి->free_list = 0;
    } లేకపోతే {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      శూన్య *ఖండం_ప్రారంభం = సూచిక_స్థానభ్రంశం(
          పరిధి, పరిధి_శీర్షిక_కొలత +
                    ((size_t)పరిధి->free_list_హద్దు * పరిధి->ఖండం_కొలత_2));
      పరిధి->free_list_హద్దు += free_list_partial_init(
          &కుప్ప_కొలత_class->free_list, &ఖండం,
          (శూన్య *)((uintptr_t)ఖండం_ప్రారంభం & ~(జ్ఞాపకం_పుట_కొలత - 1)),
          ఖండం_ప్రారంభం, పరిధి->ఖండం_లెక్క - పరిధి->free_list_హద్దు,
          పరిధి->ఖండం_కొలత_2);
    }
    rpmalloc_assert(పరిధి->free_list_హద్దు <= పరిధి->ఖండం_లెక్క,
                    "Span block count corrupted");
    పరిధి->వాడిన_లెక్క = పరిధి->free_list_హద్దు;

    // Swap in deferred free list if present
    ఒకవేళ (atomic_ఎక్కించు_ptr(&పరిధి->free_list_deferred))
      rpmalloc_పరిధి_వెలికితీయి_free_list_deferred(పరిధి);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    ఒకవేళ (!rpmalloc_పరిధి_is_fully_utilized(పరిధి))
      తిరిగి ఖండం;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_పరిధి_double_link_list_తీయి_తల(&కుప్ప_కొలత_class->partial_పరిధి,
                                             పరిధి);
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
    rpmalloc_పరిధి_double_link_list_కూడు(&కుప్ప->నిండిన_పరిధి[class_idx], పరిధి);
#అయితే_ముగింపు
    ++కుప్ప->నిండిన_పరిధి_లెక్క;
    తిరిగి ఖండం;
  }

  // Find a span in one of the cache levels
  పరిధి = rpmalloc_కుప్ప_వెలికితీయి_new_పరిధి(కుప్ప, కుప్ప_కొలత_class, 1, class_idx);
  ఒకవేళ (EXPECTED(పరిధి != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    తిరిగి rpmalloc_పరిధి_initialize_new(కుప్ప, కుప్ప_కొలత_class, పరిధి,
                                         class_idx);
  }

  తిరిగి 0;
}

//! Allocate a small sized memory block from the given heap
స్థిర శూన్య *_rpmalloc_allocate_small(కుప్ప_t *కుప్ప, size_t కొలత_2) {
  rpmalloc_assert(కుప్ప, "No thread heap");
  // Small sizes have unique size classes
  స్థిరరాశి uint32_t class_idx =
      (uint32_t)((కొలత_2 + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_జరుపు);
  కుప్ప_కొలత_class_t *కుప్ప_కొలత_class = కుప్ప->కొలత_class + class_idx;
  _rpmalloc_stat_inc_alloc(కుప్ప, class_idx);
  ఒకవేళ (EXPECTED(కుప్ప_కొలత_class->free_list != 0))
    తిరిగి free_list_తీయి(&కుప్ప_కొలత_class->free_list);
  తిరిగి rpmalloc_allocate_from_కుప్ప_fallback(కుప్ప, కుప్ప_కొలత_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
స్థిర శూన్య *_rpmalloc_allocate_medium(కుప్ప_t *కుప్ప, size_t కొలత_2) {
  rpmalloc_assert(కుప్ప, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  స్థిరరాశి uint32_t పునాది_idx =
      (uint32_t)(SMALL_CLASS_లెక్క +
                 ((కొలత_2 - (SMALL_కొలత_హద్దు + 1)) >> MEDIUM_GRANULARITY_జరుపు));
  స్థిరరాశి uint32_t class_idx = జ్ఞాపకం_కొలత_class[పునాది_idx].class_idx;
  కుప్ప_కొలత_class_t *కుప్ప_కొలత_class = కుప్ప->కొలత_class + class_idx;
  _rpmalloc_stat_inc_alloc(కుప్ప, class_idx);
  ఒకవేళ (EXPECTED(కుప్ప_కొలత_class->free_list != 0))
    తిరిగి free_list_తీయి(&కుప్ప_కొలత_class->free_list);
  తిరిగి rpmalloc_allocate_from_కుప్ప_fallback(కుప్ప, కుప్ప_కొలత_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
స్థిర శూన్య *_rpmalloc_allocate_large(కుప్ప_t *కుప్ప, size_t కొలత_2) {
  rpmalloc_assert(కుప్ప, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  కొలత_2 += పరిధి_శీర్షిక_కొలత;
  size_t పరిధి_లెక్క = కొలత_2 >> జ్ఞాపకం_పరిధి_కొలత_జరుపు;
  ఒకవేళ (కొలత_2 & (జ్ఞాపకం_పరిధి_కొలత - 1))
    ++పరిధి_లెక్క;

  // Find a span in one of the cache levels
  పరిధి_t *పరిధి =
      rpmalloc_కుప్ప_వెలికితీయి_new_పరిధి(కుప్ప, 0, పరిధి_లెక్క, కొలత_CLASS_LARGE);
  ఒకవేళ (!పరిధి)
    తిరిగి పరిధి;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(పరిధి->పరిధి_లెక్క >= పరిధి_లెక్క, "Internal failure");
  పరిధి->కొలత_class = కొలత_CLASS_LARGE;
  పరిధి->కుప్ప = కుప్ప;

#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
  rpmalloc_పరిధి_double_link_list_కూడు(&కుప్ప->large_huge_పరిధి, పరిధి);
#అయితే_ముగింపు
  ++కుప్ప->నిండిన_పరిధి_లెక్క;

  తిరిగి సూచిక_స్థానభ్రంశం(పరిధి, పరిధి_శీర్షిక_కొలత);
}

//! Allocate a huge block by mapping memory pages directly
స్థిర శూన్య *_rpmalloc_allocate_huge(కుప్ప_t *కుప్ప, size_t కొలత_2) {
  rpmalloc_assert(కుప్ప, "No thread heap");
  rpmalloc_కుప్ప_cache_adopt_deferred(కుప్ప, 0);
  కొలత_2 += పరిధి_శీర్షిక_కొలత;
  size_t అంకె_పుటలు = కొలత_2 >> జ్ఞాపకం_పుట_కొలత_జరుపు;
  ఒకవేళ (కొలత_2 & (జ్ఞాపకం_పుట_కొలత - 1))
    ++అంకె_పుటలు;
  size_t align_స్థానభ్రంశం = 0;
  పరిధి_t *పరిధి =
      (పరిధి_t *)_rpmalloc_mmap(అంకె_పుటలు * జ్ఞాపకం_పుట_కొలత, &align_స్థానభ్రంశం);
  ఒకవేళ (!పరిధి)
    తిరిగి పరిధి;

  // Store page count in span_count
  పరిధి->కొలత_class = కొలత_CLASS_HUGE;
  పరిధి->పరిధి_లెక్క = (uint32_t)అంకె_పుటలు;
  పరిధి->align_స్థానభ్రంశం = (uint32_t)align_స్థానభ్రంశం;
  పరిధి->కుప్ప = కుప్ప;
  rpmalloc_stat_కూడు_peak(&huge_పుటలు_current, అంకె_పుటలు, huge_పుటలు_peak);

#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
  rpmalloc_పరిధి_double_link_list_కూడు(&కుప్ప->large_huge_పరిధి, పరిధి);
#అయితే_ముగింపు
  ++కుప్ప->నిండిన_పరిధి_లెక్క;

  తిరిగి సూచిక_స్థానభ్రంశం(పరిధి, పరిధి_శీర్షిక_కొలత);
}

//! Allocate a block of the given size
స్థిర శూన్య *_rpmalloc_allocate(కుప్ప_t *కుప్ప, size_t కొలత_2) {
  _rpmalloc_stat_add64(&కేటాయింపు_counter, 1);
  ఒకవేళ (EXPECTED(కొలత_2 <= SMALL_కొలత_హద్దు))
    తిరిగి _rpmalloc_allocate_small(కుప్ప, కొలత_2);
  లేకపోతే ఒకవేళ (కొలత_2 <= జ్ఞాపకం_medium_కొలత_హద్దు)
    తిరిగి _rpmalloc_allocate_medium(కుప్ప, కొలత_2);
  లేకపోతే ఒకవేళ (కొలత_2 <= LARGE_కొలత_హద్దు)
    తిరిగి _rpmalloc_allocate_large(కుప్ప, కొలత_2);
  తిరిగి _rpmalloc_allocate_huge(కుప్ప, కొలత_2);
}

స్థిర శూన్య *_rpmalloc_aligned_allocate(కుప్ప_t *కుప్ప, size_t alignment,
                                        size_t కొలత_2) {
  ఒకవేళ (alignment <= SMALL_GRANULARITY)
    తిరిగి _rpmalloc_allocate(కుప్ప, కొలత_2);

#ఒకవేళ ENABLE_VALIDATE_వాదనలు
  ఒకవేళ ((కొలత_2 + alignment) < కొలత_2) {
    errno = EINVAL;
    తిరిగి 0;
  }
  ఒకవేళ (alignment & (alignment - 1)) {
    errno = EINVAL;
    తిరిగి 0;
  }
#అయితే_ముగింపు

  ఒకవేళ ((alignment <= పరిధి_శీర్షిక_కొలత) &&
      ((కొలత_2 + పరిధి_శీర్షిక_కొలత) < జ్ఞాపకం_medium_కొలత_హద్దు)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t గుణిజం_కొలత = కొలత_2 ? (కొలత_2 + (పరిధి_శీర్షిక_కొలత - 1)) &
                                      ~(uintptr_t)(పరిధి_శీర్షిక_కొలత - 1)
                                : పరిధి_శీర్షిక_కొలత;
    rpmalloc_assert(!(గుణిజం_కొలత % పరిధి_శీర్షిక_కొలత),
                    "Failed alignment calculation");
    ఒకవేళ (గుణిజం_కొలత <= (కొలత_2 + alignment))
      తిరిగి _rpmalloc_allocate(కుప్ప, గుణిజం_కొలత);
  }

  శూన్య *ptr = 0;
  size_t align_ముసుగు = alignment - 1;
  ఒకవేళ (alignment <= జ్ఞాపకం_పుట_కొలత) {
    ptr = _rpmalloc_allocate(కుప్ప, కొలత_2 + alignment);
    ఒకవేళ ((uintptr_t)ptr & align_ముసుగు) {
      ptr = (శూన్య *)(((uintptr_t)ptr & ~(uintptr_t)align_ముసుగు) + alignment);
      // Mark as having aligned blocks
      పరిధి_t *పరిధి = (పరిధి_t *)((uintptr_t)ptr & జ్ఞాపకం_పరిధి_ముసుగు);
      పరిధి->flags |= పరిధి_FLAG_ALIGNED_ఖండాలు;
    }
    తిరిగి ptr;
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
  ఒకవేళ (alignment & align_ముసుగు) {
    errno = EINVAL;
    తిరిగి 0;
  }
  ఒకవేళ (alignment >= జ్ఞాపకం_పరిధి_కొలత) {
    errno = EINVAL;
    తిరిగి 0;
  }

  size_t extra_పుటలు = alignment / జ్ఞాపకం_పుట_కొలత;

  // Since each span has a header, we will at least need one extra memory page
  size_t అంకె_పుటలు = 1 + (కొలత_2 / జ్ఞాపకం_పుట_కొలత);
  ఒకవేళ (కొలత_2 & (జ్ఞాపకం_పుట_కొలత - 1))
    ++అంకె_పుటలు;

  ఒకవేళ (extra_పుటలు > అంకె_పుటలు)
    అంకె_పుటలు = 1 + extra_పుటలు;

  size_t original_పుటలు = అంకె_పుటలు;
  size_t హద్దు_పుటలు = (జ్ఞాపకం_పరిధి_కొలత / జ్ఞాపకం_పుట_కొలత) * 2;
  ఒకవేళ (హద్దు_పుటలు < (original_పుటలు * 2))
    హద్దు_పుటలు = original_పుటలు * 2;

  size_t mapped_కొలత, align_స్థానభ్రంశం;
  పరిధి_t *పరిధి;

retry:
  align_స్థానభ్రంశం = 0;
  mapped_కొలత = అంకె_పుటలు * జ్ఞాపకం_పుట_కొలత;

  పరిధి = (పరిధి_t *)_rpmalloc_mmap(mapped_కొలత, &align_స్థానభ్రంశం);
  ఒకవేళ (!పరిధి) {
    errno = ENOMEM;
    తిరిగి 0;
  }
  ptr = సూచిక_స్థానభ్రంశం(పరిధి, పరిధి_శీర్షిక_కొలత);

  ఒకవేళ ((uintptr_t)ptr & align_ముసుగు)
    ptr = (శూన్య *)(((uintptr_t)ptr & ~(uintptr_t)align_ముసుగు) + alignment);

  ఒకవేళ (((size_t)సూచిక_diff(ptr, పరిధి) >= జ్ఞాపకం_పరిధి_కొలత) ||
      (సూచిక_స్థానభ్రంశం(ptr, కొలత_2) > సూచిక_స్థానభ్రంశం(పరిధి, mapped_కొలత)) ||
      (((uintptr_t)ptr & జ్ఞాపకం_పరిధి_ముసుగు) != (uintptr_t)పరిధి)) {
    _rpmalloc_unmap(పరిధి, mapped_కొలత, align_స్థానభ్రంశం, mapped_కొలత);
    ++అంకె_పుటలు;
    ఒకవేళ (అంకె_పుటలు > హద్దు_పుటలు) {
      errno = EINVAL;
      తిరిగి 0;
    }
    వెళ్ళు retry;
  }

  // Store page count in span_count
  పరిధి->కొలత_class = కొలత_CLASS_HUGE;
  పరిధి->పరిధి_లెక్క = (uint32_t)అంకె_పుటలు;
  పరిధి->align_స్థానభ్రంశం = (uint32_t)align_స్థానభ్రంశం;
  పరిధి->కుప్ప = కుప్ప;
  rpmalloc_stat_కూడు_peak(&huge_పుటలు_current, అంకె_పుటలు, huge_పుటలు_peak);

#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
  rpmalloc_పరిధి_double_link_list_కూడు(&కుప్ప->large_huge_పరిధి, పరిధి);
#అయితే_ముగింపు
  ++కుప్ప->నిండిన_పరిధి_లెక్క;

  _rpmalloc_stat_add64(&కేటాయింపు_counter, 1);

  తిరిగి ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
స్థిర శూన్య rpmalloc_deallocate_ప్రత్యక్ష_small_or_medium(పరిధి_t *పరిధి,
                                                        శూన్య *ఖండం) {
  కుప్ప_t *కుప్ప = పరిధి->కుప్ప;
  rpmalloc_assert(కుప్ప->owner_thread == get_thread_id() ||
                      !కుప్ప->owner_thread || కుప్ప->finalize,
                  "Internal failure");
  // Add block to free list
  ఒకవేళ (UNEXPECTED(rpmalloc_పరిధి_is_fully_utilized(పరిధి))) {
    పరిధి->వాడిన_లెక్క = పరిధి->ఖండం_లెక్క;
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
    rpmalloc_పరిధి_double_link_list_remove(&కుప్ప->నిండిన_పరిధి[పరిధి->కొలత_class],
                                           పరిధి);
#అయితే_ముగింపు
    rpmalloc_పరిధి_double_link_list_కూడు(
        &కుప్ప->కొలత_class[పరిధి->కొలత_class].partial_పరిధి, పరిధి);
    --కుప్ప->నిండిన_పరిధి_లెక్క;
  }
  *((శూన్య **)ఖండం) = పరిధి->free_list;
  --పరిధి->వాడిన_లెక్క;
  పరిధి->free_list = ఖండం;
  ఒకవేళ (UNEXPECTED(పరిధి->వాడిన_లెక్క == పరిధి->list_కొలత)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    ఒకవేళ (పరిధి->వాడిన_లెక్క) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      శూన్య *free_list;
      చేయి {
        free_list = atomic_exchange_ptr_acquire(&పరిధి->free_list_deferred,
                                                INVALID_సూచిక);
      } అయితే (free_list == INVALID_సూచిక);
      atomic_store_ptr_release(&పరిధి->free_list_deferred, free_list);
    }
    rpmalloc_పరిధి_double_link_list_remove(
        &కుప్ప->కొలత_class[పరిధి->కొలత_class].partial_పరిధి, పరిధి);
    rpmalloc_పరిధి_release_గమ్యం_cache(కుప్ప, పరిధి);
  }
}

స్థిర శూన్య rpmalloc_deallocate_defer_free_పరిధి(కుప్ప_t *కుప్ప, పరిధి_t *పరిధి) {
  ఒకవేళ (పరిధి->కొలత_class != కొలత_CLASS_HUGE)
    _rpmalloc_stat_inc(&కుప్ప->పరిధి_use[పరిధి->పరిధి_లెక్క - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  చేయి {
    పరిధి->free_list = (శూన్య *)atomic_ఎక్కించు_ptr(&కుప్ప->పరిధి_free_deferred);
  } అయితే (!atomic_cas_ptr(&కుప్ప->పరిధి_free_deferred, పరిధి, పరిధి->free_list));
}

//! Put the block in the deferred free list of the owning span
స్థిర శూన్య _rpmalloc_deallocate_defer_small_or_medium(పరిధి_t *పరిధి,
                                                       శూన్య *ఖండం) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  శూన్య *free_list;
  చేయి {
    free_list =
        atomic_exchange_ptr_acquire(&పరిధి->free_list_deferred, INVALID_సూచిక);
  } అయితే (free_list == INVALID_సూచిక);
  *((శూన్య **)ఖండం) = free_list;
  uint32_t free_లెక్క = ++పరిధి->list_కొలత;
  పూర్ణాంకం all_deferred_free = (free_లెక్క == పరిధి->ఖండం_లెక్క);
  atomic_store_ptr_release(&పరిధి->free_list_deferred, ఖండం);
  ఒకవేళ (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    rpmalloc_deallocate_defer_free_పరిధి(పరిధి->కుప్ప, పరిధి);
  }
}

స్థిర శూన్య _rpmalloc_deallocate_small_or_medium(పరిధి_t *పరిధి, శూన్య *p) {
  _rpmalloc_stat_inc_free(పరిధి->కుప్ప, పరిధి->కొలత_class);
  ఒకవేళ (పరిధి->flags & పరిధి_FLAG_ALIGNED_ఖండాలు) {
    // Realign pointer to block start
    శూన్య *ఖండాలు_ప్రారంభం = సూచిక_స్థానభ్రంశం(పరిధి, పరిధి_శీర్షిక_కొలత);
    uint32_t ఖండం_స్థానభ్రంశం = (uint32_t)సూచిక_diff(p, ఖండాలు_ప్రారంభం);
    p = సూచిక_స్థానభ్రంశం(p, -(int32_t)(ఖండం_స్థానభ్రంశం % పరిధి->ఖండం_కొలత_2));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
  పూర్ణాంకం defer =
      (పరిధి->కుప్ప->owner_thread &&
       (పరిధి->కుప్ప->owner_thread != get_thread_id()) && !పరిధి->కుప్ప->finalize);
#లేకపోతే
  పూర్ణాంకం defer =
      ((పరిధి->కుప్ప->owner_thread != get_thread_id()) && !పరిధి->కుప్ప->finalize);
#అయితే_ముగింపు
  ఒకవేళ (!defer)
    rpmalloc_deallocate_ప్రత్యక్ష_small_or_medium(పరిధి, p);
  లేకపోతే
    _rpmalloc_deallocate_defer_small_or_medium(పరిధి, p);
}

//! Deallocate the given large memory block to the current heap
స్థిర శూన్య _rpmalloc_deallocate_large(పరిధి_t *పరిధి) {
  rpmalloc_assert(పరిధి->కొలత_class == కొలత_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(పరిధి->flags & పరిధి_FLAG_MASTER) ||
                      !(పరిధి->flags & పరిధి_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((పరిధి->flags & పరిధి_FLAG_MASTER) ||
                      (పరిధి->flags & పరిధి_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
  పూర్ణాంకం defer =
      (పరిధి->కుప్ప->owner_thread &&
       (పరిధి->కుప్ప->owner_thread != get_thread_id()) && !పరిధి->కుప్ప->finalize);
#లేకపోతే
  పూర్ణాంకం defer =
      ((పరిధి->కుప్ప->owner_thread != get_thread_id()) && !పరిధి->కుప్ప->finalize);
#అయితే_ముగింపు
  ఒకవేళ (defer) {
    rpmalloc_deallocate_defer_free_పరిధి(పరిధి->కుప్ప, పరిధి);
    తిరిగి;
  }
  rpmalloc_assert(పరిధి->కుప్ప->నిండిన_పరిధి_లెక్క, "Heap span counter corrupted");
  --పరిధి->కుప్ప->నిండిన_పరిధి_లెక్క;
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
  rpmalloc_పరిధి_double_link_list_remove(&పరిధి->కుప్ప->large_huge_పరిధి, పరిధి);
#అయితే_ముగింపు
#ఒకవేళ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = పరిధి->పరిధి_లెక్క - 1;
  atomic_decr32(&పరిధి->కుప్ప->పరిధి_use[idx].current);
#అయితే_ముగింపు
  కుప్ప_t *కుప్ప = పరిధి->కుప్ప;
  rpmalloc_assert(కుప్ప, "No thread heap");
#ఒకవేళ ENABLE_THREAD_CACHE
  స్థిరరాశి పూర్ణాంకం సమితి_as_reserved =
      ((పరిధి->పరిధి_లెక్క > 1) && (కుప్ప->పరిధి_cache.లెక్క == 0) &&
       !కుప్ప->finalize && !కుప్ప->spans_reserved);
#లేకపోతే
  స్థిరరాశి పూర్ణాంకం సమితి_as_reserved =
      ((పరిధి->పరిధి_లెక్క > 1) && !కుప్ప->finalize && !కుప్ప->spans_reserved);
#అయితే_ముగింపు
  ఒకవేళ (సమితి_as_reserved) {
    కుప్ప->పరిధి_reserve = పరిధి;
    కుప్ప->spans_reserved = పరిధి->పరిధి_లెక్క;
    ఒకవేళ (పరిధి->flags & పరిధి_FLAG_MASTER) {
      కుప్ప->పరిధి_reserve_master = పరిధి;
    } లేకపోతే { // SPAN_FLAG_SUBSPAN
      పరిధి_t *master = (పరిధి_t *)సూచిక_స్థానభ్రంశం(
          పరిధి,
          -(intptr_t)((size_t)పరిధి->స్థానభ్రంశం_from_master * జ్ఞాపకం_పరిధి_కొలత));
      కుప్ప->పరిధి_reserve_master = master;
      rpmalloc_assert(master->flags & పరిధి_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->మిగిలిన_spans) >=
                          (int32_t)పరిధి->పరిధి_లెక్క,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&కుప్ప->పరిధి_use[idx].spans_గమ్యం_reserved);
  } లేకపోతే {
    // Insert into cache list
    rpmalloc_కుప్ప_cache_చొప్పించు(కుప్ప, పరిధి);
  }
}

//! Deallocate the given huge span
స్థిర శూన్య _rpmalloc_deallocate_huge(పరిధి_t *పరిధి) {
  rpmalloc_assert(పరిధి->కుప్ప, "No span heap");
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
  పూర్ణాంకం defer =
      (పరిధి->కుప్ప->owner_thread &&
       (పరిధి->కుప్ప->owner_thread != get_thread_id()) && !పరిధి->కుప్ప->finalize);
#లేకపోతే
  పూర్ణాంకం defer =
      ((పరిధి->కుప్ప->owner_thread != get_thread_id()) && !పరిధి->కుప్ప->finalize);
#అయితే_ముగింపు
  ఒకవేళ (defer) {
    rpmalloc_deallocate_defer_free_పరిధి(పరిధి->కుప్ప, పరిధి);
    తిరిగి;
  }
  rpmalloc_assert(పరిధి->కుప్ప->నిండిన_పరిధి_లెక్క, "Heap span counter corrupted");
  --పరిధి->కుప్ప->నిండిన_పరిధి_లెక్క;
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
  rpmalloc_పరిధి_double_link_list_remove(&పరిధి->కుప్ప->large_huge_పరిధి, పరిధి);
#అయితే_ముగింపు

  // Oversized allocation, page count is stored in span_count
  size_t అంకె_పుటలు = పరిధి->పరిధి_లెక్క;
  _rpmalloc_unmap(పరిధి, అంకె_పుటలు * జ్ఞాపకం_పుట_కొలత, పరిధి->align_స్థానభ్రంశం,
                  అంకె_పుటలు * జ్ఞాపకం_పుట_కొలత);
  _rpmalloc_stat_sub(&huge_పుటలు_current, అంకె_పుటలు);
}

//! Deallocate the given block
స్థిర శూన్య _rpmalloc_deallocate(శూన్య *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  పరిధి_t *పరిధి = (పరిధి_t *)((uintptr_t)p & జ్ఞాపకం_పరిధి_ముసుగు);
  ఒకవేళ (UNEXPECTED(!పరిధి))
    తిరిగి;
  ఒకవేళ (EXPECTED(పరిధి->కొలత_class < కొలత_CLASS_లెక్క))
    _rpmalloc_deallocate_small_or_medium(పరిధి, p);
  లేకపోతే ఒకవేళ (పరిధి->కొలత_class == కొలత_CLASS_LARGE)
    _rpmalloc_deallocate_large(పరిధి);
  లేకపోతే
    _rpmalloc_deallocate_huge(పరిధి);
}

////////////
///
/// Reallocation entry points
///
//////

స్థిర size_t rpmalloc_usable_కొలత_2(శూన్య *p);

//! Reallocate the given block to the given size
స్థిర శూన్య *_rpmalloc_reallocate(కుప్ప_t *కుప్ప, శూన్య *p, size_t కొలత_2,
                                  size_t oldsize, గుర్తులేని పూర్ణాంకం flags) {
  ఒకవేళ (p) {
    // Grab the span using guaranteed span alignment
    పరిధి_t *పరిధి = (పరిధి_t *)((uintptr_t)p & జ్ఞాపకం_పరిధి_ముసుగు);
    ఒకవేళ (EXPECTED(పరిధి->కొలత_class < కొలత_CLASS_లెక్క)) {
      // Small/medium sized block
      rpmalloc_assert(పరిధి->పరిధి_లెక్క == 1, "Span counter corrupted");
      శూన్య *ఖండాలు_ప్రారంభం = సూచిక_స్థానభ్రంశం(పరిధి, పరిధి_శీర్షిక_కొలత);
      uint32_t ఖండం_స్థానభ్రంశం = (uint32_t)సూచిక_diff(p, ఖండాలు_ప్రారంభం);
      uint32_t ఖండం_idx = ఖండం_స్థానభ్రంశం / పరిధి->ఖండం_కొలత_2;
      శూన్య *ఖండం =
          సూచిక_స్థానభ్రంశం(ఖండాలు_ప్రారంభం, (size_t)ఖండం_idx * పరిధి->ఖండం_కొలత_2);
      ఒకవేళ (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)పరిధి->ఖండం_కొలత_2 - సూచిక_diff(p, ఖండం));
      ఒకవేళ ((size_t)పరిధి->ఖండం_కొలత_2 >= కొలత_2) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ఒకవేళ ((p != ఖండం) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(ఖండం, p, oldsize);
        తిరిగి ఖండం;
      }
    } లేకపోతే ఒకవేళ (పరిధి->కొలత_class == కొలత_CLASS_LARGE) {
      // Large block
      size_t మొత్తం_కొలత_2 = కొలత_2 + పరిధి_శీర్షిక_కొలత;
      size_t అంకె_spans = మొత్తం_కొలత_2 >> జ్ఞాపకం_పరిధి_కొలత_జరుపు;
      ఒకవేళ (మొత్తం_కొలత_2 & (జ్ఞాపకం_పరిధి_ముసుగు - 1))
        ++అంకె_spans;
      size_t current_spans = పరిధి->పరిధి_లెక్క;
      శూన్య *ఖండం = సూచిక_స్థానభ్రంశం(పరిధి, పరిధి_శీర్షిక_కొలత);
      ఒకవేళ (!oldsize)
        oldsize = (current_spans * జ్ఞాపకం_పరిధి_కొలత) -
                  (size_t)సూచిక_diff(p, ఖండం) - పరిధి_శీర్షిక_కొలత;
      ఒకవేళ ((current_spans >= అంకె_spans) && (మొత్తం_కొలత_2 >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ఒకవేళ ((p != ఖండం) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(ఖండం, p, oldsize);
        తిరిగి ఖండం;
      }
    } లేకపోతే {
      // Oversized block
      size_t మొత్తం_కొలత_2 = కొలత_2 + పరిధి_శీర్షిక_కొలత;
      size_t అంకె_పుటలు = మొత్తం_కొలత_2 >> జ్ఞాపకం_పుట_కొలత_జరుపు;
      ఒకవేళ (మొత్తం_కొలత_2 & (జ్ఞాపకం_పుట_కొలత - 1))
        ++అంకె_పుటలు;
      // Page count is stored in span_count
      size_t current_పుటలు = పరిధి->పరిధి_లెక్క;
      శూన్య *ఖండం = సూచిక_స్థానభ్రంశం(పరిధి, పరిధి_శీర్షిక_కొలత);
      ఒకవేళ (!oldsize)
        oldsize = (current_పుటలు * జ్ఞాపకం_పుట_కొలత) -
                  (size_t)సూచిక_diff(p, ఖండం) - పరిధి_శీర్షిక_కొలత;
      ఒకవేళ ((current_పుటలు >= అంకె_పుటలు) && (అంకె_పుటలు >= (current_పుటలు / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ఒకవేళ ((p != ఖండం) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(ఖండం, p, oldsize);
        తిరిగి ఖండం;
      }
    }
  } లేకపోతే {
    oldsize = 0;
  }

  ఒకవేళ (!!(flags & RPMALLOC_GROW_OR_FAIL))
    తిరిగి 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t క్రింది_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_కొలత =
      (కొలత_2 > క్రింది_bound) ? కొలత_2 : ((కొలత_2 > oldsize) ? క్రింది_bound : కొలత_2);
  శూన్య *ఖండం = _rpmalloc_allocate(కుప్ప, new_కొలత);
  ఒకవేళ (p && ఖండం) {
    ఒకవేళ (!(flags & RPMALLOC_NO_PRESERVE))
      జ్ఞాపకం_నకలు(ఖండం, p, oldsize < new_కొలత ? oldsize : new_కొలత);
    _rpmalloc_deallocate(p);
  }

  తిరిగి ఖండం;
}

స్థిర శూన్య *_rpmalloc_aligned_reallocate(కుప్ప_t *కుప్ప, శూన్య *ptr,
                                          size_t alignment, size_t కొలత_2,
                                          size_t oldsize, గుర్తులేని పూర్ణాంకం flags) {
  ఒకవేళ (alignment <= SMALL_GRANULARITY)
    తిరిగి _rpmalloc_reallocate(కుప్ప, ptr, కొలత_2, oldsize, flags);

  పూర్ణాంకం no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? rpmalloc_usable_కొలత_2(ptr) : 0);
  ఒకవేళ ((usablesize >= కొలత_2) && !((uintptr_t)ptr & (alignment - 1))) {
    ఒకవేళ (no_alloc || (కొలత_2 >= (usablesize / 2)))
      తిరిగి ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  శూన్య *ఖండం =
      (!no_alloc ? _rpmalloc_aligned_allocate(కుప్ప, alignment, కొలత_2) : 0);
  ఒకవేళ (EXPECTED(ఖండం != 0)) {
    ఒకవేళ (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      ఒకవేళ (!oldsize)
        oldsize = usablesize;
      జ్ఞాపకం_నకలు(ఖండం, ptr, oldsize < కొలత_2 ? oldsize : కొలత_2);
    }
    _rpmalloc_deallocate(ptr);
  }
  తిరిగి ఖండం;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
స్థిర size_t rpmalloc_usable_కొలత_2(శూన్య *p) {
  // Grab the span using guaranteed span alignment
  పరిధి_t *పరిధి = (పరిధి_t *)((uintptr_t)p & జ్ఞాపకం_పరిధి_ముసుగు);
  ఒకవేళ (పరిధి->కొలత_class < కొలత_CLASS_లెక్క) {
    // Small/medium block
    శూన్య *ఖండాలు_ప్రారంభం = సూచిక_స్థానభ్రంశం(పరిధి, పరిధి_శీర్షిక_కొలత);
    తిరిగి పరిధి->ఖండం_కొలత_2 -
           ((size_t)సూచిక_diff(p, ఖండాలు_ప్రారంభం) % పరిధి->ఖండం_కొలత_2);
  }
  ఒకవేళ (పరిధి->కొలత_class == కొలత_CLASS_LARGE) {
    // Large block
    size_t current_spans = పరిధి->పరిధి_లెక్క;
    తిరిగి (current_spans * జ్ఞాపకం_పరిధి_కొలత) - (size_t)సూచిక_diff(p, పరిధి);
  }
  // Oversized block, page count is stored in span_count
  size_t current_పుటలు = పరిధి->పరిధి_లెక్క;
  తిరిగి (current_పుటలు * జ్ఞాపకం_పుట_కొలత) - (size_t)సూచిక_diff(p, పరిధి);
}

//! Adjust and optimize the size class properties for the given class
స్థిర శూన్య rpmalloc_adjust_కొలత_class(size_t iclass) {
  size_t ఖండం_కొలత_2 = జ్ఞాపకం_కొలత_class[iclass].ఖండం_కొలత_2;
  size_t ఖండం_లెక్క = (జ్ఞాపకం_పరిధి_కొలత - పరిధి_శీర్షిక_కొలత) / ఖండం_కొలత_2;

  జ్ఞాపకం_కొలత_class[iclass].ఖండం_లెక్క = (uint16_t)ఖండం_లెక్క;
  జ్ఞాపకం_కొలత_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  ఒకవేళ (iclass >= SMALL_CLASS_లెక్క) {
    size_t prevclass = iclass;
    అయితే (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      ఒకవేళ (జ్ఞాపకం_కొలత_class[prevclass].ఖండం_లెక్క ==
          జ్ఞాపకం_కొలత_class[iclass].ఖండం_లెక్క)
        _rpmalloc_memcpy_const(జ్ఞాపకం_కొలత_class + prevclass,
                               జ్ఞాపకం_కొలత_class + iclass,
                               పరిమాణం(జ్ఞాపకం_కొలత_class[iclass]));
      లేకపోతే
        ఆపు;
    }
  }
}

//! Initialize the allocator and setup global data
బాహ్య అంతఃవరుస పూర్ణాంకం rpmalloc_initialize(శూన్య) {
  ఒకవేళ (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    తిరిగి 0;
  }
  తిరిగి rpmalloc_initialize_config(0);
}

పూర్ణాంకం rpmalloc_initialize_config(స్థిరరాశి rpmalloc_config_t *config) {
  ఒకవేళ (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    తిరిగి 0;
  }
  _rpmalloc_initialized = 1;

  ఒకవేళ (config)
    జ్ఞాపకం_నకలు(&జ్ఞాపకం_config, config, పరిమాణం(rpmalloc_config_t));
  లేకపోతే
    _rpmalloc_memset_const(&జ్ఞాపకం_config, 0, పరిమాణం(rpmalloc_config_t));

  ఒకవేళ (!జ్ఞాపకం_config.జ్ఞాపకం_map || !జ్ఞాపకం_config.జ్ఞాపకం_unmap) {
    జ్ఞాపకం_config.జ్ఞాపకం_map = _rpmalloc_mmap_os;
    జ్ఞాపకం_config.జ్ఞాపకం_unmap = _rpmalloc_unmap_os;
  }

#ఒకవేళ PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, పరిమాణం(system_info));
  GetSystemInfo(&system_info);
  జ్ఞాపకం_map_granularity = system_info.dw_కేటాయింపు_Granularity;
#లేకపోతే
  జ్ఞాపకం_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#అయితే_ముగింపు

#ఒకవేళ RPMALLOC_CONFIGURABLE
  జ్ఞాపకం_పుట_కొలత = జ్ఞాపకం_config.పుట_కొలత_2;
#లేకపోతే
  జ్ఞాపకం_పుట_కొలత = 0;
#అయితే_ముగింపు
  జ్ఞాపకం_huge_పుటలు = 0;
  ఒకవేళ (!జ్ఞాపకం_పుట_కొలత) {
#ఒకవేళ PLATFORM_WINDOWS
    జ్ఞాపకం_పుట_కొలత = system_info.dw_పుట_కొలత;
#లేకపోతే
    జ్ఞాపకం_పుట_కొలత = జ్ఞాపకం_map_granularity;
    ఒకవేళ (జ్ఞాపకం_config.enable_huge_పుటలు) {
#ఒకవేళ defined(__linux__)
      size_t huge_పుట_కొలత = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      ఒకవేళ (meminfo) {
        అక్షరం పంక్తి_2[128];
        అయితే (!huge_పుట_కొలత && fgets(పంక్తి_2, పరిమాణం(పంక్తి_2) - 1, meminfo)) {
          పంక్తి_2[పరిమాణం(పంక్తి_2) - 1] = 0;
          ఒకవేళ (అక్షరశ్రేణి_శోధన(పంక్తి_2, "Hugepagesize:"))
            huge_పుట_కొలత = (size_t)strtol(పంక్తి_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      ఒకవేళ (huge_పుట_కొలత) {
        జ్ఞాపకం_huge_పుటలు = 1;
        జ్ఞాపకం_పుట_కొలత = huge_పుట_కొలత;
        జ్ఞాపకం_map_granularity = huge_పుట_కొలత;
      }
#లేదావేళ defined(__FreeBSD__)
      పూర్ణాంకం rc;
      size_t sz = పరిమాణం(rc);

      ఒకవేళ (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, ఖాళీ, 0) == 0 &&
          rc == 1) {
        స్థిర size_t defsize = 2 * 1024 * 1024;
        పూర్ణాంకం nsize = 0;
        size_t sizes[4] = {0};
        జ్ఞాపకం_huge_పుటలు = 1;
        జ్ఞాపకం_పుట_కొలత = defsize;
        ఒకవేళ ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          కొరకు (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            ఒకవేళ (defsize < csize) {
              జ్ఞాపకం_పుట_కొలత = csize;
              ఆపు;
            }
          }
        }
        జ్ఞాపకం_map_granularity = జ్ఞాపకం_పుట_కొలత;
      }
#లేదావేళ defined(__APPLE__) || defined(__NetBSD__)
      జ్ఞాపకం_huge_పుటలు = 1;
      జ్ఞాపకం_పుట_కొలత = 2 * 1024 * 1024;
      జ్ఞాపకం_map_granularity = జ్ఞాపకం_పుట_కొలత;
#అయితే_ముగింపు
    }
#అయితే_ముగింపు
  } లేకపోతే {
    ఒకవేళ (జ్ఞాపకం_config.enable_huge_పుటలు)
      జ్ఞాపకం_huge_పుటలు = 1;
  }

#ఒకవేళ PLATFORM_WINDOWS
  ఒకవేళ (జ్ఞాపకం_config.enable_huge_పుటలు) {
    HANDLE token = 0;
    size_t large_పుట_minimum = Get_Large_పుట_Minimum();
    ఒకవేళ (large_పుట_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    ఒకవేళ (token) {
      LUID luid;
      ఒకవేళ (Lookup_Privilege_విలువ(0, SE_తాళం_జ్ఞాపకం_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, పరిమాణం(token_privileges));
        token_privileges.Privilege_లెక్క = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].లక్షణాలు = SE_PRIVILEGE_ENABLED;
        ఒకవేళ (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          ఒకవేళ (Get_చివరి_Error() == ERROR_SUCCESS)
            జ్ఞాపకం_huge_పుటలు = 1;
        }
      }
      CloseHandle(token);
    }
    ఒకవేళ (జ్ఞాపకం_huge_పుటలు) {
      ఒకవేళ (large_పుట_minimum > జ్ఞాపకం_పుట_కొలత)
        జ్ఞాపకం_పుట_కొలత = large_పుట_minimum;
      ఒకవేళ (large_పుట_minimum > జ్ఞాపకం_map_granularity)
        జ్ఞాపకం_map_granularity = large_పుట_minimum;
    }
  }
#అయితే_ముగింపు

  size_t min_పరిధి_కొలత = 256;
  size_t max_పుట_కొలత;
#ఒకవేళ UINTPTR_MAX > 0xFFFFFFFF
  max_పుట_కొలత = 4096ULL * 1024ULL * 1024ULL;
#లేకపోతే
  max_పుట_కొలత = 4 * 1024 * 1024;
#అయితే_ముగింపు
  ఒకవేళ (జ్ఞాపకం_పుట_కొలత < min_పరిధి_కొలత)
    జ్ఞాపకం_పుట_కొలత = min_పరిధి_కొలత;
  ఒకవేళ (జ్ఞాపకం_పుట_కొలత > max_పుట_కొలత)
    జ్ఞాపకం_పుట_కొలత = max_పుట_కొలత;
  జ్ఞాపకం_పుట_కొలత_జరుపు = 0;
  size_t పుట_కొలత_బిట్టు = జ్ఞాపకం_పుట_కొలత;
  అయితే (పుట_కొలత_బిట్టు != 1) {
    ++జ్ఞాపకం_పుట_కొలత_జరుపు;
    పుట_కొలత_బిట్టు >>= 1;
  }
  జ్ఞాపకం_పుట_కొలత = ((size_t)1 << జ్ఞాపకం_పుట_కొలత_జరుపు);

#ఒకవేళ RPMALLOC_CONFIGURABLE
  ఒకవేళ (!జ్ఞాపకం_config.పరిధి_కొలత) {
    జ్ఞాపకం_పరిధి_కొలత = జ్ఞాపకం_default_పరిధి_కొలత;
    జ్ఞాపకం_పరిధి_కొలత_జరుపు = జ్ఞాపకం_default_పరిధి_కొలత_జరుపు;
    జ్ఞాపకం_పరిధి_ముసుగు = జ్ఞాపకం_default_పరిధి_ముసుగు;
  } లేకపోతే {
    size_t పరిధి_కొలత = జ్ఞాపకం_config.పరిధి_కొలత;
    ఒకవేళ (పరిధి_కొలత > (256 * 1024))
      పరిధి_కొలత = (256 * 1024);
    జ్ఞాపకం_పరిధి_కొలత = 4096;
    జ్ఞాపకం_పరిధి_కొలత_జరుపు = 12;
    అయితే (జ్ఞాపకం_పరిధి_కొలత < పరిధి_కొలత) {
      జ్ఞాపకం_పరిధి_కొలత <<= 1;
      ++జ్ఞాపకం_పరిధి_కొలత_జరుపు;
    }
    జ్ఞాపకం_పరిధి_ముసుగు = ~(uintptr_t)(జ్ఞాపకం_పరిధి_కొలత - 1);
  }
#అయితే_ముగింపు

  జ్ఞాపకం_పరిధి_map_లెక్క =
      (జ్ఞాపకం_config.పరిధి_map_లెక్క ? జ్ఞాపకం_config.పరిధి_map_లెక్క
                                     : DEFAULT_పరిధి_MAP_లెక్క);
  ఒకవేళ ((జ్ఞాపకం_పరిధి_కొలత * జ్ఞాపకం_పరిధి_map_లెక్క) < జ్ఞాపకం_పుట_కొలత)
    జ్ఞాపకం_పరిధి_map_లెక్క = (జ్ఞాపకం_పుట_కొలత / జ్ఞాపకం_పరిధి_కొలత);
  ఒకవేళ ((జ్ఞాపకం_పుట_కొలత >= జ్ఞాపకం_పరిధి_కొలత) &&
      ((జ్ఞాపకం_పరిధి_map_లెక్క * జ్ఞాపకం_పరిధి_కొలత) % జ్ఞాపకం_పుట_కొలత))
    జ్ఞాపకం_పరిధి_map_లెక్క = (జ్ఞాపకం_పుట_కొలత / జ్ఞాపకం_పరిధి_కొలత);
  జ్ఞాపకం_కుప్ప_reserve_లెక్క = (జ్ఞాపకం_పరిధి_map_లెక్క > DEFAULT_పరిధి_MAP_లెక్క)
                                   ? DEFAULT_పరిధి_MAP_లెక్క
                                   : జ్ఞాపకం_పరిధి_map_లెక్క;

  జ్ఞాపకం_config.పుట_కొలత_2 = జ్ఞాపకం_పుట_కొలత;
  జ్ఞాపకం_config.పరిధి_కొలత = జ్ఞాపకం_పరిధి_కొలత;
  జ్ఞాపకం_config.పరిధి_map_లెక్క = జ్ఞాపకం_పరిధి_map_లెక్క;
  జ్ఞాపకం_config.enable_huge_పుటలు = జ్ఞాపకం_huge_పుటలు;

#ఒకవేళ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  ఒకవేళ (pthread_కీలకం_create(&జ్ఞాపకం_thread_కుప్ప, rpmalloc_కుప్ప_release_raw_fc))
    తిరిగి -1;
#అయితే_ముగింపు
#ఒకవేళ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_కీలకం = FlsAlloc(&_rpmalloc_thread_destructor);
#అయితే_ముగింపు

  // Setup all small and medium size classes
  size_t iclass = 0;
  జ్ఞాపకం_కొలత_class[iclass].ఖండం_కొలత_2 = SMALL_GRANULARITY;
  rpmalloc_adjust_కొలత_class(iclass);
  కొరకు (iclass = 1; iclass < SMALL_CLASS_లెక్క; ++iclass) {
    size_t కొలత_2 = iclass * SMALL_GRANULARITY;
    జ్ఞాపకం_కొలత_class[iclass].ఖండం_కొలత_2 = (uint32_t)కొలత_2;
    rpmalloc_adjust_కొలత_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  జ్ఞాపకం_medium_కొలత_హద్దు = (జ్ఞాపకం_పరిధి_కొలత - పరిధి_శీర్షిక_కొలత) >> 1;
  ఒకవేళ (జ్ఞాపకం_medium_కొలత_హద్దు > MEDIUM_కొలత_హద్దు)
    జ్ఞాపకం_medium_కొలత_హద్దు = MEDIUM_కొలత_హద్దు;
  కొరకు (iclass = 0; iclass < MEDIUM_CLASS_లెక్క; ++iclass) {
    size_t కొలత_2 = SMALL_కొలత_హద్దు + ((iclass + 1) * MEDIUM_GRANULARITY);
    ఒకవేళ (కొలత_2 > జ్ఞాపకం_medium_కొలత_హద్దు) {
      జ్ఞాపకం_medium_కొలత_హద్దు =
          SMALL_కొలత_హద్దు + (iclass * MEDIUM_GRANULARITY);
      ఆపు;
    }
    జ్ఞాపకం_కొలత_class[SMALL_CLASS_లెక్క + iclass].ఖండం_కొలత_2 = (uint32_t)కొలత_2;
    rpmalloc_adjust_కొలత_class(SMALL_CLASS_లెక్క + iclass);
  }

  జ్ఞాపకం_orphan_heaps = 0;
#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS
  జ్ఞాపకం_మొదటి_class_orphan_heaps = 0;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_STATISTICS
  atomic_store32(&జ్ఞాపకం_చురుకైన_heaps, 0);
  atomic_store32(&mapped_పుటలు, 0);
  mapped_పుటలు_peak = 0;
  atomic_store32(&_master_spans, 0);
  atomic_store32(&mapped_మొత్తం, 0);
  atomic_store32(&unmapped_మొత్తం, 0);
  atomic_store32(&mapped_పుటలు_os, 0);
  atomic_store32(&huge_పుటలు_current, 0);
  huge_పుటలు_peak = 0;
#అయితే_ముగింపు
  memset(జ్ఞాపకం_heaps, 0, పరిమాణం(జ్ఞాపకం_heaps));
  atomic_store32_release(&జ్ఞాపకం_global_తాళం, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  తిరిగి 0;
}

//! Finalize the allocator
శూన్య rpmalloc_finalize(శూన్య) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  ఒకవేళ (జ్ఞాపకం_global_reserve) {
    atomic_add32(&జ్ఞాపకం_global_reserve_master->మిగిలిన_spans,
                 -(int32_t)జ్ఞాపకం_global_reserve_లెక్క);
    జ్ఞాపకం_global_reserve_master = 0;
    జ్ఞాపకం_global_reserve_లెక్క = 0;
    జ్ఞాపకం_global_reserve = 0;
  }
  atomic_store32_release(&జ్ఞాపకం_global_తాళం, 0);

  // Free all thread caches and fully free spans
  కొరకు (size_t list_idx = 0; list_idx < కుప్ప_ARRAY_కొలత; ++list_idx) {
    కుప్ప_t *కుప్ప = జ్ఞాపకం_heaps[list_idx];
    అయితే (కుప్ప) {
      కుప్ప_t *తదుపరి_కుప్ప = కుప్ప->తదుపరి_కుప్ప;
      కుప్ప->finalize = 1;
      rpmalloc_కుప్ప_global_finalize(కుప్ప);
      కుప్ప = తదుపరి_కుప్ప;
    }
  }

#ఒకవేళ ENABLE_GLOBAL_CACHE
  // Free global caches
  కొరకు (size_t iclass = 0; iclass < LARGE_CLASS_లెక్క; ++iclass)
    _rpmalloc_global_cache_finalize(&జ్ఞాపకం_పరిధి_cache[iclass]);
#అయితే_ముగింపు

#ఒకవేళ (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_కీలకం_delete(జ్ఞాపకం_thread_కుప్ప);
#అయితే_ముగింపు
#ఒకవేళ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_కీలకం);
  fls_కీలకం = 0;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&mapped_పుటలు) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&mapped_పుటలు_os) == 0,
                  "Memory leak detected");
#అయితే_ముగింపు

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
బాహ్య అంతఃవరుస శూన్య rpmalloc_thread_initialize(శూన్య) {
  ఒకవేళ (!get_thread_కుప్ప_raw()) {
    కుప్ప_t *కుప్ప = rpmalloc_కుప్ప_allocate(0);
    ఒకవేళ (కుప్ప) {
      _rpmalloc_stat_inc(&జ్ఞాపకం_చురుకైన_heaps);
      సమితి_thread_కుప్ప(కుప్ప);
#ఒకవేళ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_సమితి_విలువ(fls_కీలకం, కుప్ప);
#అయితే_ముగింపు
    }
  }
}

//! Finalize thread, orphan heap
శూన్య rpmalloc_thread_finalize(పూర్ణాంకం release_caches) {
  కుప్ప_t *కుప్ప = get_thread_కుప్ప_raw();
  ఒకవేళ (కుప్ప)
    rpmalloc_కుప్ప_release_raw(కుప్ప, release_caches);
  సమితి_thread_కుప్ప(0);
#ఒకవేళ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_సమితి_విలువ(fls_కీలకం, 0);
#అయితే_ముగింపు
}

పూర్ణాంకం rpmalloc_is_thread_initialized(శూన్య) {
  తిరిగి (get_thread_కుప్ప_raw() != 0) ? 1 : 0;
}

స్థిరరాశి rpmalloc_config_t *rpmalloc_config(శూన్య) { తిరిగి &జ్ఞాపకం_config; }

// Extern interface

బాహ్య అంతఃవరుస RPMALLOC_ALLOCATOR శూన్య *rpmalloc(size_t కొలత_2) {
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
  ఒకవేళ (కొలత_2 >= MAX_ALLOC_కొలత) {
    errno = EINVAL;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  కుప్ప_t *కుప్ప = get_thread_కుప్ప();
  తిరిగి _rpmalloc_allocate(కుప్ప, కొలత_2);
}

బాహ్య అంతఃవరుస శూన్య rpfree(శూన్య *ptr) { _rpmalloc_deallocate(ptr); }

బాహ్య అంతఃవరుస RPMALLOC_ALLOCATOR శూన్య *rpcalloc(size_t అంకె_2, size_t కొలత_2) {
  size_t మొత్తం_2;
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
#ఒకవేళ PLATFORM_WINDOWS
  పూర్ణాంకం err = కొలత_TMult(అంకె_2, కొలత_2, &మొత్తం_2);
  ఒకవేళ ((err != S_OK) || (మొత్తం_2 >= MAX_ALLOC_కొలత)) {
    errno = EINVAL;
    తిరిగి 0;
  }
#లేకపోతే
  పూర్ణాంకం err = __builtin_umull_overflow(అంకె_2, కొలత_2, &మొత్తం_2);
  ఒకవేళ (err || (మొత్తం_2 >= MAX_ALLOC_కొలత)) {
    errno = EINVAL;
    తిరిగి 0;
  }
#అయితే_ముగింపు
#లేకపోతే
  మొత్తం_2 = అంకె_2 * కొలత_2;
#అయితే_ముగింపు
  కుప్ప_t *కుప్ప = get_thread_కుప్ప();
  శూన్య *ఖండం = _rpmalloc_allocate(కుప్ప, మొత్తం_2);
  ఒకవేళ (ఖండం)
    memset(ఖండం, 0, మొత్తం_2);
  తిరిగి ఖండం;
}

బాహ్య అంతఃవరుస RPMALLOC_ALLOCATOR శూన్య *rprealloc(శూన్య *ptr, size_t కొలత_2) {
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
  ఒకవేళ (కొలత_2 >= MAX_ALLOC_కొలత) {
    errno = EINVAL;
    తిరిగి ptr;
  }
#అయితే_ముగింపు
  కుప్ప_t *కుప్ప = get_thread_కుప్ప();
  తిరిగి _rpmalloc_reallocate(కుప్ప, ptr, కొలత_2, 0, 0);
}

బాహ్య RPMALLOC_ALLOCATOR శూన్య *rpaligned_realloc(శూన్య *ptr, size_t alignment,
                                                  size_t కొలత_2, size_t oldsize,
                                                  గుర్తులేని పూర్ణాంకం flags) {
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
  ఒకవేళ ((కొలత_2 + alignment < కొలత_2) || (alignment > జ్ఞాపకం_పుట_కొలత)) {
    errno = EINVAL;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  కుప్ప_t *కుప్ప = get_thread_కుప్ప();
  తిరిగి _rpmalloc_aligned_reallocate(కుప్ప, ptr, alignment, కొలత_2, oldsize,
                                      flags);
}

బాహ్య RPMALLOC_ALLOCATOR శూన్య *rpaligned_alloc(size_t alignment, size_t కొలత_2) {
  కుప్ప_t *కుప్ప = get_thread_కుప్ప();
  తిరిగి _rpmalloc_aligned_allocate(కుప్ప, alignment, కొలత_2);
}

బాహ్య అంతఃవరుస RPMALLOC_ALLOCATOR శూన్య *
rpaligned_calloc(size_t alignment, size_t అంకె_2, size_t కొలత_2) {
  size_t మొత్తం_2;
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
#ఒకవేళ PLATFORM_WINDOWS
  పూర్ణాంకం err = కొలత_TMult(అంకె_2, కొలత_2, &మొత్తం_2);
  ఒకవేళ ((err != S_OK) || (మొత్తం_2 >= MAX_ALLOC_కొలత)) {
    errno = EINVAL;
    తిరిగి 0;
  }
#లేకపోతే
  పూర్ణాంకం err = __builtin_umull_overflow(అంకె_2, కొలత_2, &మొత్తం_2);
  ఒకవేళ (err || (మొత్తం_2 >= MAX_ALLOC_కొలత)) {
    errno = EINVAL;
    తిరిగి 0;
  }
#అయితే_ముగింపు
#లేకపోతే
  మొత్తం_2 = అంకె_2 * కొలత_2;
#అయితే_ముగింపు
  శూన్య *ఖండం = rpaligned_alloc(alignment, మొత్తం_2);
  ఒకవేళ (ఖండం)
    memset(ఖండం, 0, మొత్తం_2);
  తిరిగి ఖండం;
}

బాహ్య అంతఃవరుస RPMALLOC_ALLOCATOR శూన్య *rpmemalign(size_t alignment,
                                                  size_t కొలత_2) {
  తిరిగి rpaligned_alloc(alignment, కొలత_2);
}

బాహ్య అంతఃవరుస పూర్ణాంకం rpposix_memalign(శూన్య **memptr, size_t alignment,
                                   size_t కొలత_2) {
  ఒకవేళ (memptr)
    *memptr = rpaligned_alloc(alignment, కొలత_2);
  లేకపోతే
    తిరిగి EINVAL;
  తిరిగి *memptr ? 0 : ENOMEM;
}

బాహ్య అంతఃవరుస size_t rpmalloc_usable_కొలత(శూన్య *ptr) {
  తిరిగి (ptr ? rpmalloc_usable_కొలత_2(ptr) : 0);
}

బాహ్య అంతఃవరుస శూన్య rpmalloc_thread_collect(శూన్య) {}

శూన్య rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, పరిమాణం(rpmalloc_thread_statistics_t));
  కుప్ప_t *కుప్ప = get_thread_కుప్ప_raw();
  ఒకవేళ (!కుప్ప)
    తిరిగి;

  కొరకు (size_t iclass = 0; iclass < కొలత_CLASS_లెక్క; ++iclass) {
    కొలత_class_t *కొలత_class = జ్ఞాపకం_కొలత_class + iclass;
    పరిధి_t *పరిధి = కుప్ప->కొలత_class[iclass].partial_పరిధి;
    అయితే (పరిధి) {
      size_t free_లెక్క = పరిధి->list_కొలత;
      size_t ఖండం_లెక్క = కొలత_class->ఖండం_లెక్క;
      ఒకవేళ (పరిధి->free_list_హద్దు < ఖండం_లెక్క)
        ఖండం_లెక్క = పరిధి->free_list_హద్దు;
      free_లెక్క += (ఖండం_లెక్క - పరిధి->వాడిన_లెక్క);
      stats->sizecache += free_లెక్క * కొలత_class->ఖండం_కొలత_2;
      పరిధి = పరిధి->తదుపరి;
    }
  }

#ఒకవేళ ENABLE_THREAD_CACHE
  కొరకు (size_t iclass = 0; iclass < LARGE_CLASS_లెక్క; ++iclass) {
    పరిధి_cache_t *పరిధి_cache;
    ఒకవేళ (!iclass)
      పరిధి_cache = &కుప్ప->పరిధి_cache;
    లేకపోతే
      పరిధి_cache = (పరిధి_cache_t *)(కుప్ప->పరిధి_large_cache + (iclass - 1));
    stats->spancache += పరిధి_cache->లెక్క * (iclass + 1) * జ్ఞాపకం_పరిధి_కొలత;
  }
#అయితే_ముగింపు

  పరిధి_t *deferred = (పరిధి_t *)atomic_ఎక్కించు_ptr(&కుప్ప->పరిధి_free_deferred);
  అయితే (deferred) {
    ఒకవేళ (deferred->కొలత_class != కొలత_CLASS_HUGE)
      stats->spancache += (size_t)deferred->పరిధి_లెక్క * జ్ఞాపకం_పరిధి_కొలత;
    deferred = (పరిధి_t *)deferred->free_list;
  }

#ఒకవేళ ENABLE_STATISTICS
  stats->thread_గమ్యం_global = (size_t)atomic_load64(&కుప్ప->thread_గమ్యం_global);
  stats->global_గమ్యం_thread = (size_t)atomic_load64(&కుప్ప->global_గమ్యం_thread);

  కొరకు (size_t iclass = 0; iclass < LARGE_CLASS_లెక్క; ++iclass) {
    stats->పరిధి_use[iclass].current =
        (size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].current);
    stats->పరిధి_use[iclass].peak =
        (size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].ఎగువ);
    stats->పరిధి_use[iclass].గమ్యం_global =
        (size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_గమ్యం_global);
    stats->పరిధి_use[iclass].from_global =
        (size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_from_global);
    stats->పరిధి_use[iclass].గమ్యం_cache =
        (size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_గమ్యం_cache);
    stats->పరిధి_use[iclass].from_cache =
        (size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_from_cache);
    stats->పరిధి_use[iclass].గమ్యం_reserved =
        (size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_గమ్యం_reserved);
    stats->పరిధి_use[iclass].from_reserved =
        (size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_from_reserved);
    stats->పరిధి_use[iclass].map_calls =
        (size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_map_calls);
  }
  కొరకు (size_t iclass = 0; iclass < కొలత_CLASS_లెక్క; ++iclass) {
    stats->కొలత_use[iclass].alloc_current =
        (size_t)atomic_load32(&కుప్ప->కొలత_class_use[iclass].alloc_current);
    stats->కొలత_use[iclass].alloc_peak =
        (size_t)కుప్ప->కొలత_class_use[iclass].alloc_peak;
    stats->కొలత_use[iclass].alloc_మొత్తం =
        (size_t)atomic_load32(&కుప్ప->కొలత_class_use[iclass].alloc_మొత్తం);
    stats->కొలత_use[iclass].free_మొత్తం =
        (size_t)atomic_load32(&కుప్ప->కొలత_class_use[iclass].free_మొత్తం);
    stats->కొలత_use[iclass].spans_గమ్యం_cache =
        (size_t)atomic_load32(&కుప్ప->కొలత_class_use[iclass].spans_గమ్యం_cache);
    stats->కొలత_use[iclass].spans_from_cache =
        (size_t)atomic_load32(&కుప్ప->కొలత_class_use[iclass].spans_from_cache);
    stats->కొలత_use[iclass].spans_from_reserved = (size_t)atomic_load32(
        &కుప్ప->కొలత_class_use[iclass].spans_from_reserved);
    stats->కొలత_use[iclass].map_calls =
        (size_t)atomic_load32(&కుప్ప->కొలత_class_use[iclass].spans_map_calls);
  }
#అయితే_ముగింపు
}

శూన్య rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, పరిమాణం(rpmalloc_global_statistics_t));
#ఒకవేళ ENABLE_STATISTICS
  stats->mapped = (size_t)atomic_load32(&mapped_పుటలు) * జ్ఞాపకం_పుట_కొలత;
  stats->mapped_peak = (size_t)mapped_పుటలు_peak * జ్ఞాపకం_పుట_కొలత;
  stats->mapped_మొత్తం_2 =
      (size_t)atomic_load32(&mapped_మొత్తం) * జ్ఞాపకం_పుట_కొలత;
  stats->unmapped_మొత్తం_2 =
      (size_t)atomic_load32(&unmapped_మొత్తం) * జ్ఞాపకం_పుట_కొలత;
  stats->huge_alloc =
      (size_t)atomic_load32(&huge_పుటలు_current) * జ్ఞాపకం_పుట_కొలత;
  stats->huge_alloc_peak = (size_t)huge_పుటలు_peak * జ్ఞాపకం_పుట_కొలత;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_GLOBAL_CACHE
  కొరకు (size_t iclass = 0; iclass < LARGE_CLASS_లెక్క; ++iclass) {
    global_cache_t *cache = &జ్ఞాపకం_పరిధి_cache[iclass];
    అయితే (!atomic_cas32_acquire(&cache->తాళం_2, 1, 0))
      _rpmalloc_spin();
    uint32_t లెక్క = cache->లెక్క;
#ఒకవేళ ENABLE_UNLIMITED_CACHE
    పరిధి_t *current_పరిధి = cache->అధికప్రవాహం_2;
    అయితే (current_పరిధి) {
      ++లెక్క;
      current_పరిధి = current_పరిధి->తదుపరి;
    }
#అయితే_ముగింపు
    atomic_store32_release(&cache->తాళం_2, 0);
    stats->cached += లెక్క * (iclass + 1) * జ్ఞాపకం_పరిధి_కొలత;
  }
#అయితే_ముగింపు
}

#ఒకవేళ ENABLE_STATISTICS

స్థిర శూన్య జ్ఞాపకం_కుప్ప_dump_statistics(కుప్ప_t *కుప్ప, శూన్య *file) {
  fprintf(file, "Heap %d stats:\n", కుప్ప->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  కొరకు (size_t iclass = 0; iclass < కొలత_CLASS_లెక్క; ++iclass) {
    ఒకవేళ (!atomic_load32(&కుప్ప->కొలత_class_use[iclass].alloc_మొత్తం))
      కొనసాగు;
    fprintf(
        file,
        "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu "
        "%9u\n",
        (uint32_t)iclass,
        atomic_load32(&కుప్ప->కొలత_class_use[iclass].alloc_current),
        కుప్ప->కొలత_class_use[iclass].alloc_peak,
        atomic_load32(&కుప్ప->కొలత_class_use[iclass].alloc_మొత్తం),
        atomic_load32(&కుప్ప->కొలత_class_use[iclass].free_మొత్తం),
        జ్ఞాపకం_కొలత_class[iclass].ఖండం_కొలత_2,
        జ్ఞాపకం_కొలత_class[iclass].ఖండం_లెక్క,
        atomic_load32(&కుప్ప->కొలత_class_use[iclass].spans_current),
        కుప్ప->కొలత_class_use[iclass].spans_peak,
        ((size_t)కుప్ప->కొలత_class_use[iclass].alloc_peak *
         (size_t)జ్ఞాపకం_కొలత_class[iclass].ఖండం_కొలత_2) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&కుప్ప->కొలత_class_use[iclass].spans_గమ్యం_cache) *
         జ్ఞాపకం_పరిధి_కొలత) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&కుప్ప->కొలత_class_use[iclass].spans_from_cache) *
         జ్ఞాపకం_పరిధి_కొలత) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(
             &కుప్ప->కొలత_class_use[iclass].spans_from_reserved) *
         జ్ఞాపకం_పరిధి_కొలత) /
            (size_t)(1024 * 1024),
        atomic_load32(&కుప్ప->కొలత_class_use[iclass].spans_map_calls));
  }
  fprintf(file, "Spans  Current     Peak Deferred  PeakMiB  Cached  ToCacheMiB "
                "FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB "
                "FromGlobalMiB  MmapCalls\n");
  కొరకు (size_t iclass = 0; iclass < LARGE_CLASS_లెక్క; ++iclass) {
    ఒకవేళ (!atomic_load32(&కుప్ప->పరిధి_use[iclass].ఎగువ) &&
        !atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_map_calls))
      కొనసాగు;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&కుప్ప->పరిధి_use[iclass].current),
        atomic_load32(&కుప్ప->పరిధి_use[iclass].ఎగువ),
        atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].ఎగువ) *
         (size_t)జ్ఞాపకం_పరిధి_కొలత * (iclass + 1)) /
            (size_t)(1024 * 1024),
#ఒకవేళ ENABLE_THREAD_CACHE
        (గుర్తులేని పూర్ణాంకం)(!iclass ? కుప్ప->పరిధి_cache.లెక్క
                               : కుప్ప->పరిధి_large_cache[iclass - 1].లెక్క),
        ((size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_గమ్యం_cache) *
         (iclass + 1) * జ్ఞాపకం_పరిధి_కొలత) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_from_cache) *
         (iclass + 1) * జ్ఞాపకం_పరిధి_కొలత) /
            (size_t)(1024 * 1024),
#లేకపోతే
        0, (size_t)0, (size_t)0,
#అయితే_ముగింపు
        ((size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_గమ్యం_reserved) *
         (iclass + 1) * జ్ఞాపకం_పరిధి_కొలత) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_from_reserved) *
         (iclass + 1) * జ్ఞాపకం_పరిధి_కొలత) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_గమ్యం_global) *
         (size_t)జ్ఞాపకం_పరిధి_కొలత * (iclass + 1)) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_from_global) *
         (size_t)జ్ఞాపకం_పరిధి_కొలత * (iclass + 1)) /
            (size_t)(1024 * 1024),
        atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_map_calls));
  }
  fprintf(file, "Full spans: %zu\n", కుప్ప->నిండిన_పరిధి_లెక్క);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&కుప్ప->thread_గమ్యం_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&కుప్ప->global_గమ్యం_thread) / (size_t)(1024 * 1024));
}

#అయితే_ముగింపు

శూన్య rpmalloc_dump_statistics(శూన్య *file) {
#ఒకవేళ ENABLE_STATISTICS
  కొరకు (size_t list_idx = 0; list_idx < కుప్ప_ARRAY_కొలత; ++list_idx) {
    కుప్ప_t *కుప్ప = జ్ఞాపకం_heaps[list_idx];
    అయితే (కుప్ప) {
      పూర్ణాంకం need_dump = 0;
      కొరకు (size_t iclass = 0; !need_dump && (iclass < కొలత_CLASS_లెక్క);
           ++iclass) {
        ఒకవేళ (!atomic_load32(&కుప్ప->కొలత_class_use[iclass].alloc_మొత్తం)) {
          rpmalloc_assert(
              !atomic_load32(&కుప్ప->కొలత_class_use[iclass].free_మొత్తం),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&కుప్ప->కొలత_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          కొనసాగు;
        }
        need_dump = 1;
      }
      కొరకు (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_లెక్క);
           ++iclass) {
        ఒకవేళ (!atomic_load32(&కుప్ప->పరిధి_use[iclass].ఎగువ) &&
            !atomic_load32(&కుప్ప->పరిధి_use[iclass].spans_map_calls))
          కొనసాగు;
        need_dump = 1;
      }
      ఒకవేళ (need_dump)
        జ్ఞాపకం_కుప్ప_dump_statistics(కుప్ప, file);
      కుప్ప = కుప్ప->తదుపరి_కుప్ప;
    }
  }
  fprintf(file, "Global stats:\n");
  size_t huge_current =
      (size_t)atomic_load32(&huge_పుటలు_current) * జ్ఞాపకం_పుట_కొలత;
  size_t huge_peak = (size_t)huge_పుటలు_peak * జ్ఞాపకం_పుట_కొలత;
  fprintf(file, "HugeCurrentMiB HugePeakMiB\n");
  fprintf(file, "%14zu %11zu\n", huge_current / (size_t)(1024 * 1024),
          huge_peak / (size_t)(1024 * 1024));

#ఒకవేళ ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  కొరకు (size_t iclass = 0; iclass < LARGE_CLASS_లెక్క; ++iclass) {
    global_cache_t *cache = జ్ఞాపకం_పరిధి_cache + iclass;
    size_t global_cache = (size_t)cache->లెక్క * iclass * జ్ఞాపకం_పరిధి_కొలత;

    size_t global_అధికప్రవాహం_cache = 0;
    పరిధి_t *పరిధి = cache->అధికప్రవాహం_2;
    అయితే (పరిధి) {
      global_అధికప్రవాహం_cache += iclass * జ్ఞాపకం_పరిధి_కొలత;
      పరిధి = పరిధి->తదుపరి;
    }
    ఒకవేళ (global_cache || global_అధికప్రవాహం_cache || cache->చొప్పించు_లెక్క ||
        cache->వెలికితీయి_లెక్క)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_అధికప్రవాహం_cache / (size_t)(1024 * 1024),
              cache->చొప్పించు_లెక్క, cache->వెలికితీయి_లెక్క);
  }
#అయితే_ముగింపు

  size_t mapped = (size_t)atomic_load32(&mapped_పుటలు) * జ్ఞాపకం_పుట_కొలత;
  size_t mapped_os =
      (size_t)atomic_load32(&mapped_పుటలు_os) * జ్ఞాపకం_పుట_కొలత;
  size_t mapped_peak = (size_t)mapped_పుటలు_peak * జ్ఞాపకం_పుట_కొలత;
  size_t mapped_మొత్తం_2 =
      (size_t)atomic_load32(&mapped_మొత్తం) * జ్ఞాపకం_పుట_కొలత;
  size_t unmapped_మొత్తం_2 =
      (size_t)atomic_load32(&unmapped_మొత్తం) * జ్ఞాపకం_పుట_కొలత;
  fprintf(
      file,
      "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB\n");
  fprintf(file, "%9zu %11zu %13zu %14zu %16zu\n",
          mapped / (size_t)(1024 * 1024), mapped_os / (size_t)(1024 * 1024),
          mapped_peak / (size_t)(1024 * 1024),
          mapped_మొత్తం_2 / (size_t)(1024 * 1024),
          unmapped_మొత్తం_2 / (size_t)(1024 * 1024));

  fprintf(file, "\n");
#ఒకవేళ 0
	int64_t allocated = atomic_load64(&కేటాయింపు_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#అయితే_ముగింపు
#అయితే_ముగింపు
  (శూన్య)పరిమాణం(file);
}

#ఒకవేళ RPMALLOC_మొదటి_CLASS_HEAPS

బాహ్య అంతఃవరుస rpmalloc_కుప్ప_t *rpmalloc_కుప్ప_acquire(శూన్య) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  కుప్ప_t *కుప్ప = rpmalloc_కుప్ప_allocate(1);
  rpmalloc_assume(కుప్ప != ఖాళీ);
  కుప్ప->owner_thread = 0;
  _rpmalloc_stat_inc(&జ్ఞాపకం_చురుకైన_heaps);
  తిరిగి కుప్ప;
}

బాహ్య అంతఃవరుస శూన్య rpmalloc_కుప్ప_release_2(rpmalloc_కుప్ప_t *కుప్ప) {
  ఒకవేళ (కుప్ప)
    rpmalloc_కుప్ప_release(కుప్ప, 1, 1);
}

బాహ్య అంతఃవరుస RPMALLOC_ALLOCATOR శూన్య *
rpmalloc_కుప్ప_alloc(rpmalloc_కుప్ప_t *కుప్ప, size_t కొలత_2) {
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
  ఒకవేళ (కొలత_2 >= MAX_ALLOC_కొలత) {
    errno = EINVAL;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  తిరిగి _rpmalloc_allocate(కుప్ప, కొలత_2);
}

బాహ్య అంతఃవరుస RPMALLOC_ALLOCATOR శూన్య *
rpmalloc_కుప్ప_aligned_alloc(rpmalloc_కుప్ప_t *కుప్ప, size_t alignment,
                            size_t కొలత_2) {
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
  ఒకవేళ (కొలత_2 >= MAX_ALLOC_కొలత) {
    errno = EINVAL;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  తిరిగి _rpmalloc_aligned_allocate(కుప్ప, alignment, కొలత_2);
}

బాహ్య అంతఃవరుస RPMALLOC_ALLOCATOR శూన్య *
rpmalloc_కుప్ప_calloc(rpmalloc_కుప్ప_t *కుప్ప, size_t అంకె_2, size_t కొలత_2) {
  తిరిగి rpmalloc_కుప్ప_aligned_calloc(కుప్ప, 0, అంకె_2, కొలత_2);
}

బాహ్య అంతఃవరుస RPMALLOC_ALLOCATOR శూన్య *
rpmalloc_కుప్ప_aligned_calloc(rpmalloc_కుప్ప_t *కుప్ప, size_t alignment,
                             size_t అంకె_2, size_t కొలత_2) {
  size_t మొత్తం_2;
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
#ఒకవేళ PLATFORM_WINDOWS
  పూర్ణాంకం err = కొలత_TMult(అంకె_2, కొలత_2, &మొత్తం_2);
  ఒకవేళ ((err != S_OK) || (మొత్తం_2 >= MAX_ALLOC_కొలత)) {
    errno = EINVAL;
    తిరిగి 0;
  }
#లేకపోతే
  పూర్ణాంకం err = __builtin_umull_overflow(అంకె_2, కొలత_2, &మొత్తం_2);
  ఒకవేళ (err || (మొత్తం_2 >= MAX_ALLOC_కొలత)) {
    errno = EINVAL;
    తిరిగి 0;
  }
#అయితే_ముగింపు
#లేకపోతే
  మొత్తం_2 = అంకె_2 * కొలత_2;
#అయితే_ముగింపు
  శూన్య *ఖండం = _rpmalloc_aligned_allocate(కుప్ప, alignment, మొత్తం_2);
  ఒకవేళ (ఖండం)
    memset(ఖండం, 0, మొత్తం_2);
  తిరిగి ఖండం;
}

బాహ్య అంతఃవరుస RPMALLOC_ALLOCATOR శూన్య *
rpmalloc_కుప్ప_realloc(rpmalloc_కుప్ప_t *కుప్ప, శూన్య *ptr, size_t కొలత_2,
                      గుర్తులేని పూర్ణాంకం flags) {
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
  ఒకవేళ (కొలత_2 >= MAX_ALLOC_కొలత) {
    errno = EINVAL;
    తిరిగి ptr;
  }
#అయితే_ముగింపు
  తిరిగి _rpmalloc_reallocate(కుప్ప, ptr, కొలత_2, 0, flags);
}

బాహ్య అంతఃవరుస RPMALLOC_ALLOCATOR శూన్య *
rpmalloc_కుప్ప_aligned_realloc(rpmalloc_కుప్ప_t *కుప్ప, శూన్య *ptr,
                              size_t alignment, size_t కొలత_2,
                              గుర్తులేని పూర్ణాంకం flags) {
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
  ఒకవేళ ((కొలత_2 + alignment < కొలత_2) || (alignment > జ్ఞాపకం_పుట_కొలత)) {
    errno = EINVAL;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  తిరిగి _rpmalloc_aligned_reallocate(కుప్ప, ptr, alignment, కొలత_2, 0, flags);
}

బాహ్య అంతఃవరుస శూన్య rpmalloc_కుప్ప_free(rpmalloc_కుప్ప_t *కుప్ప, శూన్య *ptr) {
  (శూన్య)పరిమాణం(కుప్ప);
  _rpmalloc_deallocate(ptr);
}

బాహ్య అంతఃవరుస శూన్య rpmalloc_కుప్ప_free_all(rpmalloc_కుప్ప_t *కుప్ప) {
  పరిధి_t *పరిధి;
  పరిధి_t *తదుపరి_పరిధి;

  rpmalloc_కుప్ప_cache_adopt_deferred(కుప్ప, 0);

  కొరకు (size_t iclass = 0; iclass < కొలత_CLASS_లెక్క; ++iclass) {
    పరిధి = కుప్ప->కొలత_class[iclass].partial_పరిధి;
    అయితే (పరిధి) {
      తదుపరి_పరిధి = పరిధి->తదుపరి;
      rpmalloc_కుప్ప_cache_చొప్పించు(కుప్ప, పరిధి);
      పరిధి = తదుపరి_పరిధి;
    }
    కుప్ప->కొలత_class[iclass].partial_పరిధి = 0;
    పరిధి = కుప్ప->నిండిన_పరిధి[iclass];
    అయితే (పరిధి) {
      తదుపరి_పరిధి = పరిధి->తదుపరి;
      rpmalloc_కుప్ప_cache_చొప్పించు(కుప్ప, పరిధి);
      పరిధి = తదుపరి_పరిధి;
    }

    పరిధి = కుప్ప->కొలత_class[iclass].cache;
    ఒకవేళ (పరిధి)
      rpmalloc_కుప్ప_cache_చొప్పించు(కుప్ప, పరిధి);
    కుప్ప->కొలత_class[iclass].cache = 0;
  }
  memset(కుప్ప->కొలత_class, 0, పరిమాణం(కుప్ప->కొలత_class));
  memset(కుప్ప->నిండిన_పరిధి, 0, పరిమాణం(కుప్ప->నిండిన_పరిధి));

  పరిధి = కుప్ప->large_huge_పరిధి;
  అయితే (పరిధి) {
    తదుపరి_పరిధి = పరిధి->తదుపరి;
    ఒకవేళ (UNEXPECTED(పరిధి->కొలత_class == కొలత_CLASS_HUGE))
      _rpmalloc_deallocate_huge(పరిధి);
    లేకపోతే
      rpmalloc_కుప్ప_cache_చొప్పించు(కుప్ప, పరిధి);
    పరిధి = తదుపరి_పరిధి;
  }
  కుప్ప->large_huge_పరిధి = 0;
  కుప్ప->నిండిన_పరిధి_లెక్క = 0;

#ఒకవేళ ENABLE_THREAD_CACHE
  కొరకు (size_t iclass = 0; iclass < LARGE_CLASS_లెక్క; ++iclass) {
    పరిధి_cache_t *పరిధి_cache;
    ఒకవేళ (!iclass)
      పరిధి_cache = &కుప్ప->పరిధి_cache;
    లేకపోతే
      పరిధి_cache = (పరిధి_cache_t *)(కుప్ప->పరిధి_large_cache + (iclass - 1));
    ఒకవేళ (!పరిధి_cache->లెక్క)
      కొనసాగు;
#ఒకవేళ ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&కుప్ప->thread_గమ్యం_global,
                         పరిధి_cache->లెక్క * (iclass + 1) * జ్ఞాపకం_పరిధి_కొలత);
    rpmalloc_stat_కూడు(&కుప్ప->పరిధి_use[iclass].spans_గమ్యం_global,
                       పరిధి_cache->లెక్క);
    rpmalloc_global_cache_చొప్పించు_spans(పరిధి_cache->పరిధి, iclass + 1,
                                        పరిధి_cache->లెక్క);
#లేకపోతే
    కొరకు (size_t ispan = 0; ispan < పరిధి_cache->లెక్క; ++ispan)
      rpmalloc_పరిధి_unmap(పరిధి_cache->పరిధి[ispan]);
#అయితే_ముగింపు
    పరిధి_cache->లెక్క = 0;
  }
#అయితే_ముగింపు

#ఒకవేళ ENABLE_STATISTICS
  కొరకు (size_t iclass = 0; iclass < కొలత_CLASS_లెక్క; ++iclass) {
    atomic_store32(&కుప్ప->కొలత_class_use[iclass].alloc_current, 0);
    atomic_store32(&కుప్ప->కొలత_class_use[iclass].spans_current, 0);
  }
  కొరకు (size_t iclass = 0; iclass < LARGE_CLASS_లెక్క; ++iclass) {
    atomic_store32(&కుప్ప->పరిధి_use[iclass].current, 0);
  }
#అయితే_ముగింపు
}

బాహ్య అంతఃవరుస శూన్య rpmalloc_కుప్ప_thread_సమితి_current(rpmalloc_కుప్ప_t *కుప్ప) {
  కుప్ప_t *prev_కుప్ప = get_thread_కుప్ప_raw();
  ఒకవేళ (prev_కుప్ప != కుప్ప) {
    సమితి_thread_కుప్ప(కుప్ప);
    ఒకవేళ (prev_కుప్ప)
      rpmalloc_కుప్ప_release_2(prev_కుప్ప);
  }
}

బాహ్య అంతఃవరుస rpmalloc_కుప్ప_t *rpmalloc_get_కుప్ప_for_ptr(శూన్య *ptr) {
  // Grab the span, and then the heap from the span
  పరిధి_t *పరిధి = (పరిధి_t *)((uintptr_t)ptr & జ్ఞాపకం_పరిధి_ముసుగు);
  ఒకవేళ (పరిధి) {
    తిరిగి పరిధి->కుప్ప;
  }
  తిరిగి 0;
}

#అయితే_ముగింపు

#ఒకవేళ ENABLE_PRELOAD || ENABLE_OVERRIDE

#అన్తర్భావ "malloc.c"

#అయితే_ముగింపు

శూన్య rpmalloc_linker_reference(శూన్య) { (శూన్య)పరిమాణం(_rpmalloc_initialized); }
