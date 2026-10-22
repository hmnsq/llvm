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

#ਅਨ੍ਤਰ੍ਭਾਵ "rpmalloc.h"

////////////
///
/// Build time configurable limits
///
//////

#ਜੇ defined(__clang__)
#ਹਦਾਇਤ clang diagnostic ignored "-Wunused-macros"
#ਹਦਾਇਤ clang diagnostic ignored "-Wunused-function"
#ਜੇ __has_warning("-Wreserved-identifier")
#ਹਦਾਇਤ clang diagnostic ignored "-Wreserved-identifier"
#ਜੇ_ਅੰਤ
#ਜੇ __has_warning("-Wstatic-in-inline")
#ਹਦਾਇਤ clang diagnostic ignored "-Wstatic-in-inline"
#ਜੇ_ਅੰਤ
#ਜਾਂਜੇ defined(__GNUC__)
#ਹਦਾਇਤ GCC diagnostic ignored "-Wunused-macros"
#ਹਦਾਇਤ GCC diagnostic ignored "-Wunused-function"
#ਜੇ_ਅੰਤ

#ਜੇ !defined(__has_builtin)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __has_builtin(b) 0
#ਜੇ_ਅੰਤ

#ਜੇ defined(__GNUC__) || defined(__clang__)

#ਜੇ __has_builtin(__builtin_memcpy_inline)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_memcpy_const(x, y, s) __builtin_memcpy_inline(x, y, s)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_memcpy_const(x, y, s)                                        \
  ਕਰੋ {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(x, y, s);                                                           \
  } ਜਦੋਂ (0)
#ਜੇ_ਅੰਤ

#ਜੇ __has_builtin(__builtin_memset_inline)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_memset_const(x, y, s) __builtin_memset_inline(x, y, s)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_memset_const(x, y, s)                                        \
  ਕਰੋ {                                                                         \
    _Static_assert(__builtin_choose_expr(__builtin_constant_p(s), 1, 0),       \
                   "len must be a constant integer");                          \
    memset(x, y, s);                                                           \
  } ਜਦੋਂ (0)
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_memcpy_const(x, y, s) ਸਮ੍ਰਿਤੀ_ਨਕਲ(x, y, s)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_memset_const(x, y, s) memset(x, y, s)
#ਜੇ_ਅੰਤ

#ਜੇ __has_builtin(__builtin_assume)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rpmalloc_assume(cond) __builtin_assume(cond)
#ਜਾਂਜੇ defined(__GNUC__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rpmalloc_assume(cond)                                                  \
  ਕਰੋ {                                                                         \
    ਜੇ (!__builtin_expect(cond, 0))                                            \
      __builtin_unreachable();                                                 \
  } ਜਦੋਂ (0)
#ਜਾਂਜੇ defined(_MSC_VER)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rpmalloc_assume(cond) __assume(cond)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rpmalloc_assume(cond) 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ਢੇਰ_ARRAY_ਮਾਪ
//! Size of heap hashmap
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਢੇਰ_ARRAY_ਮਾਪ 47
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ENABLE_THREAD_CACHE
//! Enable per-thread cache
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_THREAD_CACHE 1
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ENABLE_GLOBAL_CACHE
//! Enable global cache shared between all threads, requires thread cache
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_GLOBAL_CACHE 1
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ENABLE_VALIDATE_ਦਲੀਲਾਂ
//! Enable validation of args to public entry points
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_VALIDATE_ਦਲੀਲਾਂ 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ENABLE_STATISTICS
//! Enable statistics collection
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_STATISTICS 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ENABLE_ASSERTS
//! Enable asserts
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_ASSERTS 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ENABLE_OVERRIDE
//! Override standard library malloc/free and new/delete entry points
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_OVERRIDE 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ENABLE_PRELOAD
//! Support preloading
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_PRELOAD 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DISABLE_UNMAP
//! Disable unmapping memory pages (also enables unlimited cache)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISABLE_UNMAP 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ENABLE_UNLIMITED_CACHE
//! Enable unlimited global cache (no unmapping until finalization)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_UNLIMITED_CACHE 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ENABLE_ADAPTIVE_THREAD_CACHE
//! Enable adaptive thread cache size based on use heuristics
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_ADAPTIVE_THREAD_CACHE 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DEFAULT_ਵਿਸਤਾਰ_MAP_ਗਿਣਤੀ
//! Default number of spans to map in call to map more virtual memory (default
//! values yield 4MiB here)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_ਵਿਸਤਾਰ_MAP_ਗਿਣਤੀ 64
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ GLOBAL_CACHE_MULTIPLIER
//! Multiplier for global cache
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GLOBAL_CACHE_MULTIPLIER 8
#ਜੇ_ਅੰਤ

#ਜੇ DISABLE_UNMAP && !ENABLE_GLOBAL_CACHE
#ਗਲਤੀ Must use global cache ਜੇ unmap is disabled
#ਜੇ_ਅੰਤ

#ਜੇ DISABLE_UNMAP
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ENABLE_UNLIMITED_CACHE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_UNLIMITED_CACHE 1
#ਜੇ_ਅੰਤ

#ਜੇ !ENABLE_GLOBAL_CACHE
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ENABLE_UNLIMITED_CACHE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_UNLIMITED_CACHE 0
#ਜੇ_ਅੰਤ

#ਜੇ !ENABLE_THREAD_CACHE
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ENABLE_ADAPTIVE_THREAD_CACHE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_ADAPTIVE_THREAD_CACHE 0
#ਜੇ_ਅੰਤ

#ਜੇ defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PLATFORM_WINDOWS 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PLATFORM_POSIX 0
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PLATFORM_WINDOWS 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PLATFORM_POSIX 1
#ਜੇ_ਅੰਤ

/// Platform and arch specifics
#ਜੇ defined(_MSC_VER) && !defined(__clang__)
#ਹਦਾਇਤ warning(disable : 5105)
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FORCEINLINE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FORCEINLINE ਸਤਰਅੰਦਰ __forceinline
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _Static_assert static_assert
#ਨਹੀਂਤਾਂ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FORCEINLINE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FORCEINLINE ਸਤਰਅੰਦਰ __attribute__((__always_inline__))
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ PLATFORM_WINDOWS
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ WIN32_LEAN_AND_ਔਸਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WIN32_LEAN_AND_ਔਸਤ
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <windows.h>
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <intsafe.h>
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਜੇ defined(__linux__) || defined(__ANDROID__)
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/prctl.h>
#ਜੇ !defined(PR_ਸੈੱਟ_VMA)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PR_ਸੈੱਟ_VMA 0x53564d41
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PR_ਸੈੱਟ_VMA_ANON_NAME 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ defined(__APPLE__)
#ਅਨ੍ਤਰ੍ਭਾਵ <TargetConditionals.h>
#ਜੇ !ਨਿਸ਼ਾਨਾ_OS_IPHONE && !ਨਿਸ਼ਾਨਾ_OS_SIMULATOR
#ਅਨ੍ਤਰ੍ਭਾਵ <mach/mach_vm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mach/vm_statistics.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>
#ਜੇ_ਅੰਤ
#ਜੇ defined(__HAIKU__) || defined(__TINYC__)
#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਜੇ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
#ਅਨ੍ਤਰ੍ਭਾਵ <fibersapi.h>
ਸਥਿਰ DWORD fls_ਕੁੰਜੀ;
#ਜੇ_ਅੰਤ

#ਜੇ PLATFORM_POSIX
#ਅਨ੍ਤਰ੍ਭਾਵ <sched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/mman.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __FreeBSD__
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/sysctl.h>
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAP_HUGETLB MAP_ALIGNED_SUPER
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ PROT_MAX
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PROT_MAX(f) 0
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PROT_MAX(f) 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __sun
ਬਾਹਰੀ ਪੂਰਨ madvise(caddr_t, size_t, ਪੂਰਨ);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MAP_UNINITIALIZED
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAP_UNINITIALIZED 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਜੇ ENABLE_ASSERTS
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ NDEBUG
#ਜੇ defined(_MSC_VER) && !defined(_DEBUG)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _DEBUG
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_TOSTRING_M(x) #x
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_TOSTRING(x) RPMALLOC_TOSTRING_M(x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rpmalloc_assert(truth, ਸੁਨੇਹਾ_2)                                        \
  ਕਰੋ {                                                                         \
    ਜੇ (!(truth)) {                                                            \
      ਜੇ (ਸਮ੍ਰਿਤੀ_config.error_callback) {                                     \
        ਸਮ੍ਰਿਤੀ_config.error_callback(ਸੁਨੇਹਾ_2 " (" RPMALLOC_TOSTRING(          \
            truth) ") at " __FILE__ ":" RPMALLOC_TOSTRING(__LINE__));          \
      } ਨਹੀਂਤਾਂ {                                                                 \
        ਪ੍ਰਤਿਜ੍ਞਾ((truth) && ਸੁਨੇਹਾ_2);                                            \
      }                                                                        \
    }                                                                          \
  } ਜਦੋਂ (0)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rpmalloc_assert(truth, ਸੁਨੇਹਾ_2)                                        \
  ਕਰੋ {                                                                         \
  } ਜਦੋਂ (0)
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_STATISTICS
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਜੇ_ਅੰਤ

//////
///
/// Atomic access abstraction (since MSVC does not do C11 yet)
///
//////

#ਜੇ defined(_MSC_VER) && !defined(__clang__)

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅਸਥਿਰ ਲੰਮਾ atomic32_t;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅਸਥਿਰ ਲੰਮਾ ਲੰਮਾ atomic64_t;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅਸਥਿਰ ਖਾਲੀ *atomicptr_t;

ਸਥਿਰ FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  ਵਾਪਸ (int32_t)InterlockedOr(src, 0);
}
ਸਥਿਰ FORCEINLINE ਖਾਲੀ atomic_store32(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ਸਥਿਰ FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  ਵਾਪਸ (int32_t)InterlockedIncrement(val);
}
ਸਥਿਰ FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  ਵਾਪਸ (int32_t)InterlockedDecrement(val);
}
ਸਥਿਰ FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t ਜੋੜੋ) {
  ਵਾਪਸ (int32_t)Interlocked_Exchange_ਜੋੜੋ(val, ਜੋੜੋ) + ਜੋੜੋ;
}
ਸਥਿਰ FORCEINLINE ਪੂਰਨ atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  ਵਾਪਸ (InterlockedCompareExchange(dst, val, ref) == ref) ? 1 : 0;
}
ਸਥਿਰ FORCEINLINE ਖਾਲੀ atomic_store32_release(atomic32_t *dst, int32_t val) {
  InterlockedExchange(dst, val);
}
ਸਥਿਰ FORCEINLINE int64_t atomic_load64(atomic64_t *src) {
  ਵਾਪਸ (int64_t)InterlockedOr64(src, 0);
}
ਸਥਿਰ FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t ਜੋੜੋ) {
  ਵਾਪਸ (int64_t)InterlockedExchangeAdd64(val, ਜੋੜੋ) + ਜੋੜੋ;
}
ਸਥਿਰ FORCEINLINE ਖਾਲੀ *atomic_ਲੱਦੋ_ptr(atomicptr_t *src) {
  ਵਾਪਸ Interlocked_Compare_Exchange_ਸੂਚਕ(src, 0, 0);
}
ਸਥਿਰ FORCEINLINE ਖਾਲੀ atomic_store_ptr(atomicptr_t *dst, ਖਾਲੀ *val) {
  Interlocked_Exchange_ਸੂਚਕ(dst, val);
}
ਸਥਿਰ FORCEINLINE ਖਾਲੀ atomic_store_ptr_release(atomicptr_t *dst, ਖਾਲੀ *val) {
  Interlocked_Exchange_ਸੂਚਕ(dst, val);
}
ਸਥਿਰ FORCEINLINE ਖਾਲੀ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     ਖਾਲੀ *val) {
  ਵਾਪਸ (ਖਾਲੀ *)Interlocked_Exchange_ਸੂਚਕ((ਖਾਲੀ *ਅਸਥਿਰ *)dst, val);
}
ਸਥਿਰ FORCEINLINE ਪੂਰਨ atomic_cas_ptr(atomicptr_t *dst, ਖਾਲੀ *val, ਖਾਲੀ *ref) {
  ਵਾਪਸ (Interlocked_Compare_Exchange_ਸੂਚਕ((ਖਾਲੀ *ਅਸਥਿਰ *)dst, val, ref) ==
          ref)
             ? 1
             : 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXPECTED(x) (x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNEXPECTED(x) (x)

#ਨਹੀਂਤਾਂ

#ਅਨ੍ਤਰ੍ਭਾਵ <stdatomic.h>

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅਸਥਿਰ _Atomic(int32_t) atomic32_t;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅਸਥਿਰ _Atomic(int64_t) atomic64_t;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅਸਥਿਰ _Atomic(ਖਾਲੀ *) atomicptr_t;

ਸਥਿਰ FORCEINLINE int32_t atomic_load32(atomic32_t *src) {
  ਵਾਪਸ atomic_ਲੱਦੋ_explicit(src, memory_order_relaxed);
}
ਸਥਿਰ FORCEINLINE ਖਾਲੀ atomic_store32(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ਸਥਿਰ FORCEINLINE int32_t atomic_incr32(atomic32_t *val) {
  ਵਾਪਸ atomic_fetch_ਜੋੜੋ_explicit(val, 1, memory_order_relaxed) + 1;
}
ਸਥਿਰ FORCEINLINE int32_t atomic_decr32(atomic32_t *val) {
  ਵਾਪਸ atomic_fetch_ਜੋੜੋ_explicit(val, -1, memory_order_relaxed) - 1;
}
ਸਥਿਰ FORCEINLINE int32_t atomic_add32(atomic32_t *val, int32_t ਜੋੜੋ) {
  ਵਾਪਸ atomic_fetch_ਜੋੜੋ_explicit(val, ਜੋੜੋ, memory_order_relaxed) + ਜੋੜੋ;
}
ਸਥਿਰ FORCEINLINE ਪੂਰਨ atomic_cas32_acquire(atomic32_t *dst, int32_t val,
                                            int32_t ref) {
  ਵਾਪਸ atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_acquire, memory_order_relaxed);
}
ਸਥਿਰ FORCEINLINE ਖਾਲੀ atomic_store32_release(atomic32_t *dst, int32_t val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ਸਥਿਰ FORCEINLINE int64_t atomic_load64(atomic64_t *val) {
  ਵਾਪਸ atomic_ਲੱਦੋ_explicit(val, memory_order_relaxed);
}
ਸਥਿਰ FORCEINLINE int64_t atomic_add64(atomic64_t *val, int64_t ਜੋੜੋ) {
  ਵਾਪਸ atomic_fetch_ਜੋੜੋ_explicit(val, ਜੋੜੋ, memory_order_relaxed) + ਜੋੜੋ;
}
ਸਥਿਰ FORCEINLINE ਖਾਲੀ *atomic_ਲੱਦੋ_ptr(atomicptr_t *src) {
  ਵਾਪਸ atomic_ਲੱਦੋ_explicit(src, memory_order_relaxed);
}
ਸਥਿਰ FORCEINLINE ਖਾਲੀ atomic_store_ptr(atomicptr_t *dst, ਖਾਲੀ *val) {
  atomic_store_explicit(dst, val, memory_order_relaxed);
}
ਸਥਿਰ FORCEINLINE ਖਾਲੀ atomic_store_ptr_release(atomicptr_t *dst, ਖਾਲੀ *val) {
  atomic_store_explicit(dst, val, memory_order_release);
}
ਸਥਿਰ FORCEINLINE ਖਾਲੀ *atomic_exchange_ptr_acquire(atomicptr_t *dst,
                                                     ਖਾਲੀ *val) {
  ਵਾਪਸ atomic_exchange_explicit(dst, val, memory_order_acquire);
}
ਸਥਿਰ FORCEINLINE ਪੂਰਨ atomic_cas_ptr(atomicptr_t *dst, ਖਾਲੀ *val, ਖਾਲੀ *ref) {
  ਵਾਪਸ atomic_compare_exchange_weak_explicit(
      dst, &ref, val, memory_order_relaxed, memory_order_relaxed);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXPECTED(x) __builtin_expect((x), 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNEXPECTED(x) __builtin_expect((x), 0)

#ਜੇ_ਅੰਤ

////////////
///
/// Statistics related functions (evaluate to nothing when statistics not
/// enabled)
///
//////

#ਜੇ ENABLE_STATISTICS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_stat_inc(counter) atomic_incr32(counter)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_stat_dec(counter) atomic_decr32(counter)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rpmalloc_stat_ਜੋੜੋ(counter, ਮੁੱਲ_2)                                     \
  atomic_add32(counter, (int32_t)(ਮੁੱਲ_2))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_stat_add64(counter, ਮੁੱਲ_2)                                   \
  atomic_add64(counter, (int64_t)(ਮੁੱਲ_2))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rpmalloc_stat_ਜੋੜੋ_peak(counter, ਮੁੱਲ_2, peak)                          \
  ਕਰੋ {                                                                         \
    int32_t cur_ਗਿਣਤੀ = atomic_add32(counter, (int32_t)(ਮੁੱਲ_2));              \
    ਜੇ (cur_ਗਿਣਤੀ > (peak))                                                   \
      peak = cur_ਗਿਣਤੀ;                                                       \
  } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_stat_sub(counter, ਮੁੱਲ_2)                                     \
  atomic_add32(counter, -(int32_t)(ਮੁੱਲ_2))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_stat_inc_alloc(ਢੇਰ_2, class_idx)                              \
  ਕਰੋ {                                                                         \
    int32_t alloc_current =                                                    \
        atomic_incr32(&ਢੇਰ_2->ਮਾਪ_class_use[class_idx].alloc_current);         \
    ਜੇ (alloc_current > ਢੇਰ_2->ਮਾਪ_class_use[class_idx].alloc_peak)            \
      ਢੇਰ_2->ਮਾਪ_class_use[class_idx].alloc_peak = alloc_current;              \
    atomic_incr32(&ਢੇਰ_2->ਮਾਪ_class_use[class_idx].alloc_ਕੁੱਲ);               \
  } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_stat_inc_free(ਢੇਰ_2, class_idx)                               \
  ਕਰੋ {                                                                         \
    atomic_decr32(&ਢੇਰ_2->ਮਾਪ_class_use[class_idx].alloc_current);             \
    atomic_incr32(&ਢੇਰ_2->ਮਾਪ_class_use[class_idx].free_ਕੁੱਲ);                \
  } ਜਦੋਂ (0)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_stat_inc(counter)                                            \
  ਕਰੋ {                                                                         \
  } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_stat_dec(counter)                                            \
  ਕਰੋ {                                                                         \
  } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rpmalloc_stat_ਜੋੜੋ(counter, ਮੁੱਲ_2)                                     \
  ਕਰੋ {                                                                         \
  } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_stat_add64(counter, ਮੁੱਲ_2)                                   \
  ਕਰੋ {                                                                         \
  } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rpmalloc_stat_ਜੋੜੋ_peak(counter, ਮੁੱਲ_2, peak)                          \
  ਕਰੋ {                                                                         \
  } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_stat_sub(counter, ਮੁੱਲ_2)                                     \
  ਕਰੋ {                                                                         \
  } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_stat_inc_alloc(ਢੇਰ_2, class_idx)                              \
  ਕਰੋ {                                                                         \
  } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _rpmalloc_stat_inc_free(ਢੇਰ_2, class_idx)                               \
  ਕਰੋ {                                                                         \
  } ਜਦੋਂ (0)
#ਜੇ_ਅੰਤ

///
/// Preconfigured limits and sizes
///

//! Granularity of a small allocation block (must be power of two)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMALL_GRANULARITY 16
//! Small granularity shift count
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMALL_GRANULARITY_ਖਿਸਕਾਓ 4
//! Number of small block size classes
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMALL_CLASS_ਗਿਣਤੀ 65
//! Maximum size of a small block
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMALL_ਮਾਪ_ਹੱਦਬੰਦੀ (SMALL_GRANULARITY * (SMALL_CLASS_ਗਿਣਤੀ - 1))
//! Granularity of a medium allocation block
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEDIUM_GRANULARITY 512
//! Medium granularity shift count
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEDIUM_GRANULARITY_ਖਿਸਕਾਓ 9
//! Number of medium block size classes
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEDIUM_CLASS_ਗਿਣਤੀ 61
//! Total number of small + medium size classes
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_CLASS_ਗਿਣਤੀ (SMALL_CLASS_ਗਿਣਤੀ + MEDIUM_CLASS_ਗਿਣਤੀ)
//! Number of large block size classes
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LARGE_CLASS_ਗਿਣਤੀ 63
//! Maximum size of a medium block
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEDIUM_ਮਾਪ_ਹੱਦਬੰਦੀ                                                      \
  (SMALL_ਮਾਪ_ਹੱਦਬੰਦੀ + (MEDIUM_GRANULARITY * MEDIUM_CLASS_ਗਿਣਤੀ))
//! Maximum size of a large block
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LARGE_ਮਾਪ_ਹੱਦਬੰਦੀ                                                       \
  ((LARGE_CLASS_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) - ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ)
//! Size of a span header (must be a multiple of SMALL_GRANULARITY and a power
//! of two)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ 128
//! Number of spans in thread cache
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_THREAD_ਵਿਸਤਾਰ_CACHE 400
//! Number of spans to transfer between thread and global cache
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ THREAD_ਵਿਸਤਾਰ_CACHE_TRANSFER 64
//! Number of spans in thread cache for large spans (must be greater than
//! LARGE_CLASS_COUNT / 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_THREAD_ਵਿਸਤਾਰ_LARGE_CACHE 100
//! Number of spans to transfer between thread and global cache for large spans
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ THREAD_ਵਿਸਤਾਰ_LARGE_CACHE_TRANSFER 6

_Static_assert((SMALL_GRANULARITY & (SMALL_GRANULARITY - 1)) == 0,
               "Small granularity must be power of two");
_Static_assert((ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ & (ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ - 1)) == 0,
               "Span header size must be power of two");

#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
//! Maximum allocation size to avoid integer overflow
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ MAX_ALLOC_ਮਾਪ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_ALLOC_ਮਾਪ (((size_t) - 1) - ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੂਚਕ_ਵਿੱਥ(ptr, ofs) (ਖਾਲੀ *)((ਅੱਖਰ *)(ptr) + (ptrdiff_t)(ofs))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੂਚਕ_diff(ਪਹਿਲਾ_2, ਦੂਜਾ_2)                                            \
  (ptrdiff_t)((ਅਚਲ ਅੱਖਰ *)(ਪਹਿਲਾ_2) - (ਅਚਲ ਅੱਖਰ *)(ਦੂਜਾ_2))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INVALID_ਸੂਚਕ ((ਖਾਲੀ *)((uintptr_t) - 1))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_CLASS_LARGE ਮਾਪ_CLASS_ਗਿਣਤੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_CLASS_HUGE ((uint32_t) - 1)

////////////
///
/// Data types
///
//////

//! A memory heap, per thread
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਢੇਰ_t ਢੇਰ_t;
//! Span of memory pages
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਵਿਸਤਾਰ_t ਵਿਸਤਾਰ_t;
//! Span list
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਵਿਸਤਾਰ_list_t ਵਿਸਤਾਰ_list_t;
//! Span active data
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਵਿਸਤਾਰ_ਸਰਗਰਮ_t ਵਿਸਤਾਰ_ਸਰਗਰਮ_t;
//! Size class definition
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਮਾਪ_class_t ਮਾਪ_class_t;
//! Global cache
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ global_cache_t global_cache_t;

//! Flag indicating span is the first (master) span of a split superspan
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਸਤਾਰ_FLAG_MASTER 1U
//! Flag indicating span is a secondary (sub) span of a split superspan
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਸਤਾਰ_FLAG_SUBSPAN 2U
//! Flag indicating span has blocks with increased alignment
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਸਤਾਰ_FLAG_ALIGNED_ਖੰਡਾਂ 4U
//! Flag indicating an unmapped master span
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਸਤਾਰ_FLAG_UNMAPPED_MASTER 8U

#ਜੇ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
ਬਣਤਰ ਵਿਸਤਾਰ_use_t {
  //! Current number of spans used (actually used, not in cache)
  atomic32_t current;
  //! High water mark of spans used
  atomic32_t ਉੱਚਾ;
#ਜੇ ENABLE_STATISTICS
  //! Number of spans in deferred list
  atomic32_t spans_deferred;
  //! Number of spans transitioned to global cache
  atomic32_t spans_ਮੰਜ਼ਿਲ_global;
  //! Number of spans transitioned from global cache
  atomic32_t spans_from_global;
  //! Number of spans transitioned to thread cache
  atomic32_t spans_ਮੰਜ਼ਿਲ_cache;
  //! Number of spans transitioned from thread cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned to reserved state
  atomic32_t spans_ਮੰਜ਼ਿਲ_reserved;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of raw memory map calls
  atomic32_t spans_map_calls;
#ਜੇ_ਅੰਤ
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਵਿਸਤਾਰ_use_t ਵਿਸਤਾਰ_use_t;
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_STATISTICS
ਬਣਤਰ ਮਾਪ_class_use_t {
  //! Current number of allocations
  atomic32_t alloc_current;
  //! Peak number of allocations
  int32_t alloc_peak;
  //! Total number of allocations
  atomic32_t alloc_ਕੁੱਲ;
  //! Total number of frees
  atomic32_t free_ਕੁੱਲ;
  //! Number of spans in use
  atomic32_t spans_current;
  //! Number of spans transitioned to cache
  int32_t spans_peak;
  //! Number of spans transitioned to cache
  atomic32_t spans_ਮੰਜ਼ਿਲ_cache;
  //! Number of spans transitioned from cache
  atomic32_t spans_from_cache;
  //! Number of spans transitioned from reserved state
  atomic32_t spans_from_reserved;
  //! Number of spans mapped
  atomic32_t spans_map_calls;
  int32_t ਅਣਵਰਤਿਆ_2;
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਮਾਪ_class_use_t ਮਾਪ_class_use_t;
#ਜੇ_ਅੰਤ

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
ਬਣਤਰ ਵਿਸਤਾਰ_t {
  //! Free list
  ਖਾਲੀ *free_list;
  //! Total block count of size class
  uint32_t ਖੰਡ_ਗਿਣਤੀ;
  //! Size class
  uint32_t ਮਾਪ_class;
  //! Index of last block initialized in free list
  uint32_t free_list_ਹੱਦਬੰਦੀ;
  //! Number of used blocks remaining when in partial state
  uint32_t ਵਰਤਿਆ_ਗਿਣਤੀ;
  //! Deferred free list
  atomicptr_t free_list_deferred;
  //! Size of deferred free list, or list of spans when part of a cache list
  uint32_t list_ਮਾਪ;
  //! Size of a block
  uint32_t ਖੰਡ_ਮਾਪ_2;
  //! Flags and counters
  uint32_t flags;
  //! Number of spans
  uint32_t ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  //! Total span counter for master spans
  uint32_t ਕੁੱਲ_spans;
  //! Offset from master span for subspans
  uint32_t ਵਿੱਥ_from_master;
  //! Remaining span counter, for master spans
  atomic32_t ਬਚਿਆ_spans;
  //! Alignment offset
  uint32_t align_ਵਿੱਥ;
  //! Owning heap
  ਢੇਰ_t *ਢੇਰ_2;
  //! Next span
  ਵਿਸਤਾਰ_t *ਅਗਲਾ;
  //! Previous span
  ਵਿਸਤਾਰ_t *prev;
};
_Static_assert(ਆਕਾਰ(ਵਿਸਤਾਰ_t) <= ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ, "span size mismatch");

ਬਣਤਰ ਵਿਸਤਾਰ_cache_t {
  size_t ਗਿਣਤੀ_2;
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ[MAX_THREAD_ਵਿਸਤਾਰ_CACHE];
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਵਿਸਤਾਰ_cache_t ਵਿਸਤਾਰ_cache_t;

ਬਣਤਰ ਵਿਸਤਾਰ_large_cache_t {
  size_t ਗਿਣਤੀ_2;
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ[MAX_THREAD_ਵਿਸਤਾਰ_LARGE_CACHE];
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਵਿਸਤਾਰ_large_cache_t ਵਿਸਤਾਰ_large_cache_t;

ਬਣਤਰ ਢੇਰ_ਮਾਪ_class_t {
  //! Free list of active span
  ਖਾਲੀ *free_list;
  //! Double linked list of partially used spans with free blocks.
  //  Previous span pointer in head points to tail span of list.
  ਵਿਸਤਾਰ_t *partial_ਵਿਸਤਾਰ;
  //! Early level cache of fully free spans
  ਵਿਸਤਾਰ_t *cache;
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਢੇਰ_ਮਾਪ_class_t ਢੇਰ_ਮਾਪ_class_t;

// Control structure for a heap, either a thread heap or a first class heap if
// enabled
ਬਣਤਰ ਢੇਰ_t {
  //! Owning thread ID
  uintptr_t owner_thread;
  //! Free lists for each size class
  ਢੇਰ_ਮਾਪ_class_t ਮਾਪ_class[ਮਾਪ_CLASS_ਗਿਣਤੀ];
#ਜੇ ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, single span
  ਵਿਸਤਾਰ_cache_t ਵਿਸਤਾਰ_cache;
#ਜੇ_ਅੰਤ
  //! List of deferred free spans (single linked list)
  atomicptr_t ਵਿਸਤਾਰ_free_deferred;
  //! Number of full spans
  size_t ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  //! Mapped but unused spans
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ_reserve;
  //! Master span for mapped but unused spans
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ_reserve_master;
  //! Number of mapped but unused spans
  uint32_t spans_reserved;
  //! Child count
  atomic32_t ਬੱਚਾ_ਗਿਣਤੀ;
  //! Next heap in id list
  ਢੇਰ_t *ਅਗਲਾ_ਢੇਰ;
  //! Next heap in orphan list
  ਢੇਰ_t *ਅਗਲਾ_orphan;
  //! Heap ID
  int32_t id;
  //! Finalization state flag
  ਪੂਰਨ finalize;
  //! Master heap owning the memory pages
  ਢੇਰ_t *master_ਢੇਰ;
#ਜੇ ENABLE_THREAD_CACHE
  //! Arrays of fully freed spans, large spans with > 1 span count
  ਵਿਸਤਾਰ_large_cache_t ਵਿਸਤਾਰ_large_cache[LARGE_CLASS_ਗਿਣਤੀ - 1];
#ਜੇ_ਅੰਤ
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
  //! Double linked list of fully utilized spans with free blocks for each size
  //! class.
  //  Previous span pointer in head points to tail span of list.
  ਵਿਸਤਾਰ_t *ਭਰਿਆ_ਵਿਸਤਾਰ[ਮਾਪ_CLASS_ਗਿਣਤੀ];
  //! Double linked list of large and huge spans allocated by this heap
  ਵਿਸਤਾਰ_t *large_huge_ਵਿਸਤਾਰ;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  //! Current and high water mark of spans used per span count
  ਵਿਸਤਾਰ_use_t ਵਿਸਤਾਰ_use[LARGE_CLASS_ਗਿਣਤੀ];
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_STATISTICS
  //! Allocation stats per size class
  ਮਾਪ_class_use_t ਮਾਪ_class_use[ਮਾਪ_CLASS_ਗਿਣਤੀ + 1];
  //! Number of bytes transitioned thread -> global
  atomic64_t thread_ਮੰਜ਼ਿਲ_global;
  //! Number of bytes transitioned global -> thread
  atomic64_t global_ਮੰਜ਼ਿਲ_thread;
#ਜੇ_ਅੰਤ
};

// Size class for defining a block size bucket
ਬਣਤਰ ਮਾਪ_class_t {
  //! Size of blocks in this class
  uint32_t ਖੰਡ_ਮਾਪ_2;
  //! Number of blocks in each chunk
  uint16_t ਖੰਡ_ਗਿਣਤੀ;
  //! Class index this class is merged with
  uint16_t class_idx;
};
_Static_assert(ਆਕਾਰ(ਮਾਪ_class_t) == 8, "Size class size mismatch");

ਬਣਤਰ global_cache_t {
  //! Cache lock
  atomic32_t ਤਾਲਾ;
  //! Cache count
  uint32_t ਗਿਣਤੀ_2;
#ਜੇ ENABLE_STATISTICS
  //! Insert count
  size_t ਪਾਓ_ਗਿਣਤੀ;
  //! Extract count
  size_t ਕੱਢੋ_ਗਿਣਤੀ_2;
#ਜੇ_ਅੰਤ
  //! Cached spans
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ[GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_ਵਿਸਤਾਰ_CACHE];
  //! Unlimited cache overflow
  ਵਿਸਤਾਰ_t *ਉਛਾਲ_2;
};

////////////
///
/// Global data
///
//////

//! Default span size (64KiB)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਮ੍ਰਿਤੀ_default_ਵਿਸਤਾਰ_ਮਾਪ (64 * 1024)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਮ੍ਰਿਤੀ_default_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ 16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਮ੍ਰਿਤੀ_default_ਵਿਸਤਾਰ_ਨਕਾਬ (~((uintptr_t)(ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ - 1)))

//! Initialized flag
ਸਥਿਰ ਪੂਰਨ _rpmalloc_initialized;
//! Main thread ID
ਸਥਿਰ uintptr_t _rpmalloc_main_thread_id;
//! Configuration
ਸਥਿਰ rpmalloc_config_t ਸਮ੍ਰਿਤੀ_config;
//! Memory page size
ਸਥਿਰ size_t ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
//! Shift to divide by page size
ਸਥਿਰ size_t ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ_ਖਿਸਕਾਓ;
//! Granularity at which memory pages are mapped by OS
ਸਥਿਰ size_t ਸਮ੍ਰਿਤੀ_map_granularity;
#ਜੇ RPMALLOC_CONFIGURABLE
//! Size of a span of memory pages
ਸਥਿਰ size_t ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ;
//! Shift to divide by span size
ਸਥਿਰ size_t ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ;
//! Mask to get to start of a memory span
ਸਥਿਰ uintptr_t ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ;
#ਨਹੀਂਤਾਂ
//! Hardwired span size
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ ਸਮ੍ਰਿਤੀ_default_ਵਿਸਤਾਰ_ਮਾਪ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ ਸਮ੍ਰਿਤੀ_default_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ ਸਮ੍ਰਿਤੀ_default_ਵਿਸਤਾਰ_ਨਕਾਬ
#ਜੇ_ਅੰਤ
//! Number of spans to map in each map call
ਸਥਿਰ size_t ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ;
//! Number of spans to keep reserved in each heap
ਸਥਿਰ size_t ਸਮ੍ਰਿਤੀ_ਢੇਰ_reserve_ਗਿਣਤੀ;
//! Global size classes
ਸਥਿਰ ਮਾਪ_class_t ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[ਮਾਪ_CLASS_ਗਿਣਤੀ];
//! Run-time size limit of medium blocks
ਸਥਿਰ size_t ਸਮ੍ਰਿਤੀ_medium_ਮਾਪ_ਹੱਦਬੰਦੀ;
//! Heap ID counter
ਸਥਿਰ atomic32_t ਸਮ੍ਰਿਤੀ_ਢੇਰ_id;
//! Huge page support
ਸਥਿਰ ਪੂਰਨ ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ;
#ਜੇ ENABLE_GLOBAL_CACHE
//! Global span cache
ਸਥਿਰ global_cache_t ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_cache[LARGE_CLASS_ਗਿਣਤੀ];
#ਜੇ_ਅੰਤ
//! Global reserved spans
ਸਥਿਰ ਵਿਸਤਾਰ_t *ਸਮ੍ਰਿਤੀ_global_reserve;
//! Global reserved count
ਸਥਿਰ size_t ਸਮ੍ਰਿਤੀ_global_reserve_ਗਿਣਤੀ;
//! Global reserved master
ਸਥਿਰ ਵਿਸਤਾਰ_t *ਸਮ੍ਰਿਤੀ_global_reserve_master;
//! All heaps
ਸਥਿਰ ਢੇਰ_t *ਸਮ੍ਰਿਤੀ_heaps[ਢੇਰ_ARRAY_ਮਾਪ];
//! Used to restrict access to mapping memory for huge pages
ਸਥਿਰ atomic32_t ਸਮ੍ਰਿਤੀ_global_ਤਾਲਾ;
//! Orphaned heaps
ਸਥਿਰ ਢੇਰ_t *ਸਮ੍ਰਿਤੀ_orphan_heaps;
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
//! Orphaned heaps (first class heaps)
ਸਥਿਰ ਢੇਰ_t *ਸਮ੍ਰਿਤੀ_ਪਹਿਲਾ_class_orphan_heaps;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_STATISTICS
//! Allocations counter
ਸਥਿਰ atomic64_t ਵੰਡ_counter;
//! Deallocations counter
ਸਥਿਰ atomic64_t _deallocation_counter;
//! Active heap count
ਸਥਿਰ atomic32_t ਸਮ੍ਰਿਤੀ_ਸਰਗਰਮ_heaps;
//! Number of currently mapped memory pages
ਸਥਿਰ atomic32_t mapped_ਪੰਨੇ;
//! Peak number of concurrently mapped memory pages
ਸਥਿਰ int32_t mapped_ਪੰਨੇ_peak;
//! Number of mapped master spans
ਸਥਿਰ atomic32_t _master_spans;
//! Number of unmapped dangling master spans
ਸਥਿਰ atomic32_t _unmapped_master_spans;
//! Running counter of total number of mapped memory pages since start
ਸਥਿਰ atomic32_t mapped_ਕੁੱਲ;
//! Running counter of total number of unmapped memory pages since start
ਸਥਿਰ atomic32_t unmapped_ਕੁੱਲ;
//! Number of currently mapped memory pages in OS calls
ਸਥਿਰ atomic32_t mapped_ਪੰਨੇ_os;
//! Number of currently allocated pages in huge allocations
ਸਥਿਰ atomic32_t huge_ਪੰਨੇ_current;
//! Peak number of currently allocated pages in huge allocations
ਸਥਿਰ int32_t huge_ਪੰਨੇ_peak;
#ਜੇ_ਅੰਤ

////////////
///
/// Thread local heap and ID
///
//////

//! Current thread heap
#ਜੇ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
ਸਥਿਰ pthread_key_t ਸਮ੍ਰਿਤੀ_thread_ਢੇਰ;
#ਨਹੀਂਤਾਂ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _Thread_local __declspec(thread)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TLS_MODEL
#ਨਹੀਂਤਾਂ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __HAIKU__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TLS_MODEL __attribute__((tls_model("initial-exec")))
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TLS_MODEL
#ਜੇ_ਅੰਤ
#ਜੇ !defined(__clang__) && defined(__GNUC__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _Thread_local __thread
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
ਸਥਿਰ _Thread_local ਢੇਰ_t *ਸਮ੍ਰਿਤੀ_thread_ਢੇਰ TLS_MODEL;
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਸਤਰਅੰਦਰ ਢੇਰ_t *get_thread_ਢੇਰ_raw(ਖਾਲੀ) {
#ਜੇ (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  ਵਾਪਸ pthread_getspecific(ਸਮ੍ਰਿਤੀ_thread_ਢੇਰ);
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ ਸਮ੍ਰਿਤੀ_thread_ਢੇਰ;
#ਜੇ_ਅੰਤ
}

//! Get the current thread heap
ਸਥਿਰ ਸਤਰਅੰਦਰ ਢੇਰ_t *get_thread_ਢੇਰ(ਖਾਲੀ) {
  ਢੇਰ_t *ਢੇਰ_2 = get_thread_ਢੇਰ_raw();
#ਜੇ ENABLE_PRELOAD
  ਜੇ (EXPECTED(ਢੇਰ_2 != 0))
    ਵਾਪਸ ਢੇਰ_2;
  rpmalloc_initialize();
  ਵਾਪਸ get_thread_ਢੇਰ_raw();
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ ਢੇਰ_2;
#ਜੇ_ਅੰਤ
}

//! Fast thread ID
ਸਥਿਰ ਸਤਰਅੰਦਰ uintptr_t get_thread_id(ਖਾਲੀ) {
#ਜੇ defined(_WIN32)
  ਵਾਪਸ (uintptr_t)((ਖਾਲੀ *)NtCurrentTeb());
#ਜਾਂਜੇ (defined(__GNUC__) || defined(__clang__)) && !defined(__CYGWIN__)
  uintptr_t tid;
#ਜੇ defined(__i386__)
  __asm__("movl %%gs:0, %0" : "=r"(tid) : :);
#ਜਾਂਜੇ defined(__x86_64__)
#ਜੇ defined(__MACH__)
  __asm__("movq %%gs:0, %0" : "=r"(tid) : :);
#ਨਹੀਂਤਾਂ
  __asm__("movq %%fs:0, %0" : "=r"(tid) : :);
#ਜੇ_ਅੰਤ
#ਜਾਂਜੇ defined(__arm__)
  __asm__ ਅਸਥਿਰ("mrc p15, 0, %0, c13, c0, 3" : "=r"(tid));
#ਜਾਂਜੇ defined(__aarch64__)
#ਜੇ defined(__MACH__)
  // tpidr_el0 likely unused, always return 0 on iOS
  __asm__ ਅਸਥਿਰ("mrs %0, tpidrro_el0" : "=r"(tid));
#ਨਹੀਂਤਾਂ
  __asm__ ਅਸਥਿਰ("mrs %0, tpidr_el0" : "=r"(tid));
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
#ਗਲਤੀ This platform needs implementation ਦਾ_3 get_thread_id()
#ਜੇ_ਅੰਤ
  ਵਾਪਸ tid;
#ਨਹੀਂਤਾਂ
#ਗਲਤੀ This platform needs implementation ਦਾ_3 get_thread_id()
#ਜੇ_ਅੰਤ
}

//! Set the current thread heap
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_thread_ਢੇਰ(ਢੇਰ_t *ਢੇਰ_2) {
#ਜੇ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  pthread_setspecific(ਸਮ੍ਰਿਤੀ_thread_ਢੇਰ, ਢੇਰ_2);
#ਨਹੀਂਤਾਂ
  ਸਮ੍ਰਿਤੀ_thread_ਢੇਰ = ਢੇਰ_2;
#ਜੇ_ਅੰਤ
  ਜੇ (ਢੇਰ_2)
    ਢੇਰ_2->owner_thread = get_thread_id();
}

//! Set main thread ID
ਬਾਹਰੀ ਖਾਲੀ rpmalloc_ਸੈੱਟ_main_thread(ਖਾਲੀ);

ਖਾਲੀ rpmalloc_ਸੈੱਟ_main_thread(ਖਾਲੀ) {
  _rpmalloc_main_thread_id = get_thread_id();
}

ਸਥਿਰ ਖਾਲੀ _rpmalloc_spin(ਖਾਲੀ) {
#ਜੇ defined(_MSC_VER)
#ਜੇ defined(_M_ARM64)
  __yield();
#ਨਹੀਂਤਾਂ
  _mm_pause();
#ਜੇ_ਅੰਤ
#ਜਾਂਜੇ defined(__x86_64__) || defined(__i386__)
  __asm__ ਅਸਥਿਰ("pause" ::: "memory");
#ਜਾਂਜੇ defined(__aarch64__) || (defined(__arm__) && __ARM_ARCH >= 7)
  __asm__ ਅਸਥਿਰ("yield" ::: "memory");
#ਜਾਂਜੇ defined(__powerpc__) || defined(__powerpc64__)
  // No idea if ever been compiled in such archs but ... as precaution
  __asm__ ਅਸਥਿਰ("or 27,27,27");
#ਜਾਂਜੇ defined(__sparc__)
  __asm__ ਅਸਥਿਰ("rd %ccr, %g0 \n\trd %ccr, %g0 \n\trd %ccr, %g0");
#ਨਹੀਂਤਾਂ
  ਬਣਤਰ timespec ts = {0};
  nanosleep(&ts, 0);
#ਜੇ_ਅੰਤ
}

#ਜੇ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)

ਸਥਿਰ ਖਾਲੀ NTAPI RPMallocTlsOnThreadExit(PVOID module, DWORD reason,
                                          PVOID reserved) {
  ਚੋਣ (reason) {
  ਹਾਲਤ DLL_PROCESS_ATTACH:
    ਰੋਕੋ;
  ਹਾਲਤ DLL_PROCESS_DETACH:
    rpmalloc_finalize();
    ਰੋਕੋ;
  ਹਾਲਤ DLL_THREAD_ATTACH:
    ਰੋਕੋ;
  ਹਾਲਤ DLL_THREAD_DETACH:
    rpmalloc_thread_finalize(1);
    ਰੋਕੋ;
  }
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN64
#ਹਦਾਇਤ comment(linker, "/INCLUDE:_tls_used")
#ਹਦਾਇਤ comment(linker, "/INCLUDE:rpmalloc_tls_thread_exit_callback")

// .CRT$XL* is an array of PIMAGE_TLS_CALLBACK on Windows. It is executed
// alphabetically (A-Z) during CRT calls. As an allocator, the allocator must be
// deinitialized last. Therefore, Y(.CRT$XLY) is used here.
#ਹਦਾਇਤ const_seg(".CRT$XLY")

ਬਾਹਰੀ ਅਚਲ PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback;
ਅਚਲ PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback =
    RPMallocTlsOnThreadExit;

// Reset const section
#ਹਦਾਇਤ const_seg()
#ਨਹੀਂਤਾਂ // _WIN64
#ਹਦਾਇਤ comment(linker, "/INCLUDE:__tls_used")
#ਹਦਾਇਤ comment(linker, "/INCLUDE:_rpmalloc_tls_thread_exit_callback")

#ਹਦਾਇਤ ਅੰਕੜੇ_seg(".CRT$XLY")

PIMAGE_TLS_CALLBACK rpmalloc_tls_thread_exit_callback = RPMallocTlsOnThreadExit;

// Reset data section
#ਹਦਾਇਤ ਅੰਕੜੇ_seg()
#ਜੇ_ਅੰਤ // _WIN64

ਸਥਿਰ ਖਾਲੀ NTAPI _rpmalloc_thread_destructor(ਖਾਲੀ *ਮੁੱਲ_2) {
#ਜੇ ENABLE_OVERRIDE
  // If this is called on main thread it means rpmalloc_finalize
  // has not been called and shutdown is forced (through _exit) or unclean
  ਜੇ (get_thread_id() == _rpmalloc_main_thread_id)
    ਵਾਪਸ;
#ਜੇ_ਅੰਤ
  ਜੇ (ਮੁੱਲ_2)
    rpmalloc_thread_finalize(1);
}
#ਜੇ_ਅੰਤ

////////////
///
/// Low level memory map/unmap
///
//////

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਸੈੱਟ_name(ਖਾਲੀ *ਪਤਾ_2, size_t ਮਾਪ_2) {
#ਜੇ defined(__linux__) || defined(__ANDROID__)
  ਅਚਲ ਅੱਖਰ *name = ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ ? ਸਮ੍ਰਿਤੀ_config.huge_ਪੰਨਾ_name
                                        : ਸਮ੍ਰਿਤੀ_config.ਪੰਨਾ_name;
  ਜੇ (ਪਤਾ_2 == MAP_FAILED || !name)
    ਵਾਪਸ;
  // If the kernel does not support CONFIG_ANON_VMA_NAME or if the call fails
  // (e.g. invalid name) it is a no-op basically.
  (ਖਾਲੀ)prctl(PR_ਸੈੱਟ_VMA, PR_ਸੈੱਟ_VMA_ANON_NAME, (uintptr_t)ਪਤਾ_2, ਮਾਪ_2,
              (uintptr_t)name);
#ਨਹੀਂਤਾਂ
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
  (ਖਾਲੀ)ਆਕਾਰ(ਪਤਾ_2);
#ਜੇ_ਅੰਤ
}

//! Map more virtual memory
//  size is number of bytes to map
//  offset receives the offset in bytes from start of mapped region
//  returns address to start of mapped region to use
ਸਥਿਰ ਖਾਲੀ *_rpmalloc_mmap(size_t ਮਾਪ_2, size_t *ਵਿੱਥ_2) {
  rpmalloc_assert(!(ਮਾਪ_2 % ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ), "Invalid mmap size");
  rpmalloc_assert(ਮਾਪ_2 >= ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ, "Invalid mmap size");
  ਖਾਲੀ *ਪਤਾ_2 = ਸਮ੍ਰਿਤੀ_config.ਸਮ੍ਰਿਤੀ_map(ਮਾਪ_2, ਵਿੱਥ_2);
  ਜੇ (EXPECTED(ਪਤਾ_2 != 0)) {
    rpmalloc_stat_ਜੋੜੋ_peak(&mapped_ਪੰਨੇ, (ਮਾਪ_2 >> ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ_ਖਿਸਕਾਓ),
                            mapped_ਪੰਨੇ_peak);
    rpmalloc_stat_ਜੋੜੋ(&mapped_ਕੁੱਲ, (ਮਾਪ_2 >> ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ_ਖਿਸਕਾਓ));
  }
  ਵਾਪਸ ਪਤਾ_2;
}

//! Unmap virtual memory
//  address is the memory address to unmap, as returned from _memory_map
//  size is the number of bytes to unmap, which might be less than full region
//  for a partial unmap offset is the offset in bytes to the actual mapped
//  region, as set by _memory_map release is set to 0 for partial unmap, or size
//  of entire range for a full unmap
ਸਥਿਰ ਖਾਲੀ _rpmalloc_unmap(ਖਾਲੀ *ਪਤਾ_2, size_t ਮਾਪ_2, size_t ਵਿੱਥ_2,
                            size_t release) {
  rpmalloc_assert(!release || (release >= ਮਾਪ_2), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ),
                  "Invalid unmap size");
  ਜੇ (release) {
    rpmalloc_assert(!(release % ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ), "Invalid unmap size");
    _rpmalloc_stat_sub(&mapped_ਪੰਨੇ, (release >> ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ_ਖਿਸਕਾਓ));
    rpmalloc_stat_ਜੋੜੋ(&unmapped_ਕੁੱਲ, (release >> ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ_ਖਿਸਕਾਓ));
  }
  ਸਮ੍ਰਿਤੀ_config.ਸਮ੍ਰਿਤੀ_unmap(ਪਤਾ_2, ਮਾਪ_2, ਵਿੱਥ_2, release);
}

//! Default implementation to map new pages to virtual memory
ਸਥਿਰ ਖਾਲੀ *_rpmalloc_mmap_os(size_t ਮਾਪ_2, size_t *ਵਿੱਥ_2) {
  // Either size is a heap (a single page) or a (multiple) span - we only need
  // to align spans, and only if larger than map granularity
  size_t padding = ((ਮਾਪ_2 >= ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) &&
                    (ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ > ਸਮ੍ਰਿਤੀ_map_granularity))
                       ? ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ
                       : 0;
  rpmalloc_assert(ਮਾਪ_2 >= ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ, "Invalid mmap size");
#ਜੇ PLATFORM_WINDOWS
  // Ok to MEM_COMMIT - according to MSDN, "actual physical pages are not
  // allocated unless/until the virtual addresses are actually accessed"
  ਖਾਲੀ *ptr = VirtualAlloc(0, ਮਾਪ_2 + padding,
                           (ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ ? MEM_LARGE_ਪੰਨੇ : 0) |
                               MEM_RESERVE | MEM_COMMIT,
                           ਪੰਨਾ_READWRITE);
  ਜੇ (!ptr) {
    ਜੇ (ਸਮ੍ਰਿਤੀ_config.map_fail_callback) {
      ਜੇ (ਸਮ੍ਰਿਤੀ_config.map_fail_callback(ਮਾਪ_2 + padding))
        ਵਾਪਸ _rpmalloc_mmap_os(ਮਾਪ_2, ਵਿੱਥ_2);
    } ਨਹੀਂਤਾਂ {
      rpmalloc_assert(ptr, "Failed to map virtual memory block");
    }
    ਵਾਪਸ 0;
  }
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ flags = MAP_ਨਿੱਜੀ | MAP_ANONYMOUS | MAP_UNINITIALIZED;
#ਜੇ defined(__APPLE__) && !ਨਿਸ਼ਾਨਾ_OS_IPHONE && !ਨਿਸ਼ਾਨਾ_OS_SIMULATOR
  ਪੂਰਨ fd = (ਪੂਰਨ)VM_MAKE_ਟੈਗ(240U);
  ਜੇ (ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ)
    fd |= VM_FLAGS_SUPERPAGE_ਮਾਪ_2_MB;
  ਖਾਲੀ *ptr = mmap(0, ਮਾਪ_2 + padding, PROT_READ | PROT_WRITE, flags, fd, 0);
#ਜਾਂਜੇ defined(MAP_HUGETLB)
  ਖਾਲੀ *ptr = mmap(0, ਮਾਪ_2 + padding,
                   PROT_READ | PROT_WRITE | PROT_MAX(PROT_READ | PROT_WRITE),
                   (ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ ? MAP_HUGETLB : 0) | flags, -1, 0);
#ਜੇ defined(MADV_HUGEPAGE)
  // In some configurations, huge pages allocations might fail thus
  // we fallback to normal allocations and promote the region as transparent
  // huge page
  ਜੇ ((ptr == MAP_FAILED || !ptr) && ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ) {
    ptr = mmap(0, ਮਾਪ_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
    ਜੇ (ptr && ptr != MAP_FAILED) {
      ਪੂਰਨ prm = madvise(ptr, ਮਾਪ_2 + padding, MADV_HUGEPAGE);
      (ਖਾਲੀ)prm;
      rpmalloc_assert((prm == 0), "Failed to promote the page to THP");
    }
  }
#ਜੇ_ਅੰਤ
  rpmalloc_ਸੈੱਟ_name(ptr, ਮਾਪ_2 + padding);
#ਜਾਂਜੇ defined(MAP_ALIGNED)
  ਅਚਲ size_t align =
      (ਆਕਾਰ(size_t) * 8) - (size_t)(__builtin_clzl(ਮਾਪ_2 - 1));
  ਖਾਲੀ *ptr =
      mmap(0, ਮਾਪ_2 + padding, PROT_READ | PROT_WRITE,
           (ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ ? MAP_ALIGNED(align) : 0) | flags, -1, 0);
#ਜਾਂਜੇ defined(MAP_ALIGN)
  caddr_t ਅਧਾਰ_2 = (ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ ? (caddr_t)(4 << 20) : 0);
  ਖਾਲੀ *ptr = mmap(ਅਧਾਰ_2, ਮਾਪ_2 + padding, PROT_READ | PROT_WRITE,
                   (ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ ? MAP_ALIGN : 0) | flags, -1, 0);
#ਨਹੀਂਤਾਂ
  ਖਾਲੀ *ptr = mmap(0, ਮਾਪ_2 + padding, PROT_READ | PROT_WRITE, flags, -1, 0);
#ਜੇ_ਅੰਤ
  ਜੇ ((ptr == MAP_FAILED) || !ptr) {
    ਜੇ (ਸਮ੍ਰਿਤੀ_config.map_fail_callback) {
      ਜੇ (ਸਮ੍ਰਿਤੀ_config.map_fail_callback(ਮਾਪ_2 + padding))
        ਵਾਪਸ _rpmalloc_mmap_os(ਮਾਪ_2, ਵਿੱਥ_2);
    } ਨਹੀਂਤਾਂ ਜੇ (errno != ENOMEM) {
      rpmalloc_assert((ptr != MAP_FAILED) && ptr,
                      "Failed to map virtual memory block");
    }
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  rpmalloc_stat_ਜੋੜੋ(&mapped_ਪੰਨੇ_os,
                     (int32_t)((ਮਾਪ_2 + padding) >> ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ_ਖਿਸਕਾਓ));
  ਜੇ (padding) {
    size_t final_padding = padding - ((uintptr_t)ptr & ~ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ);
    rpmalloc_assert(final_padding <= ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ,
                    "Internal failure in padding");
    rpmalloc_assert(final_padding <= padding, "Internal failure in padding");
    rpmalloc_assert(!(final_padding % 8), "Internal failure in padding");
    ptr = ਸੂਚਕ_ਵਿੱਥ(ptr, final_padding);
    *ਵਿੱਥ_2 = final_padding >> 3;
  }
  rpmalloc_assert((ਮਾਪ_2 < ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) ||
                      !((uintptr_t)ptr & ~ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ),
                  "Internal failure in padding");
  ਵਾਪਸ ptr;
}

//! Default implementation to unmap pages from virtual memory
ਸਥਿਰ ਖਾਲੀ _rpmalloc_unmap_os(ਖਾਲੀ *ਪਤਾ_2, size_t ਮਾਪ_2, size_t ਵਿੱਥ_2,
                               size_t release) {
  rpmalloc_assert(release || (ਵਿੱਥ_2 == 0), "Invalid unmap size");
  rpmalloc_assert(!release || (release >= ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ),
                  "Invalid unmap size");
  rpmalloc_assert(ਮਾਪ_2 >= ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ, "Invalid unmap size");
  ਜੇ (release && ਵਿੱਥ_2) {
    ਵਿੱਥ_2 <<= 3;
    ਪਤਾ_2 = ਸੂਚਕ_ਵਿੱਥ(ਪਤਾ_2, -(int32_t)ਵਿੱਥ_2);
    ਜੇ ((release >= ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) &&
        (ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ > ਸਮ੍ਰਿਤੀ_map_granularity)) {
      // Padding is always one span size
      release += ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ;
    }
  }
#ਜੇ !DISABLE_UNMAP
#ਜੇ PLATFORM_WINDOWS
  ਜੇ (!VirtualFree(ਪਤਾ_2, release ? 0 : ਮਾਪ_2,
                   release ? MEM_RELEASE : MEM_DECOMMIT)) {
    rpmalloc_assert(0, "Failed to unmap virtual memory block");
  }
#ਨਹੀਂਤਾਂ
  ਜੇ (release) {
    ਜੇ (munmap(ਪਤਾ_2, release)) {
      rpmalloc_assert(0, "Failed to unmap virtual memory block");
    }
  } ਨਹੀਂਤਾਂ {
#ਜੇ defined(MADV_FREE_REUSABLE)
    ਪੂਰਨ ret;
    ਜਦੋਂ ((ret = madvise(ਪਤਾ_2, ਮਾਪ_2, MADV_FREE_REUSABLE)) == -1 &&
           (errno == EAGAIN))
      errno = 0;
    ਜੇ ((ret == -1) && (errno != 0)) {
#ਜਾਂਜੇ defined(MADV_DONTNEED)
    ਜੇ (madvise(ਪਤਾ_2, ਮਾਪ_2, MADV_DONTNEED)) {
#ਜਾਂਜੇ defined(MADV_PAGEOUT)
    ਜੇ (madvise(ਪਤਾ_2, ਮਾਪ_2, MADV_PAGEOUT)) {
#ਜਾਂਜੇ defined(MADV_FREE)
    ਜੇ (madvise(ਪਤਾ_2, ਮਾਪ_2, MADV_FREE)) {
#ਨਹੀਂਤਾਂ
    ਜੇ (posix_madvise(ਪਤਾ_2, ਮਾਪ_2, POSIX_MADV_DONTNEED)) {
#ਜੇ_ਅੰਤ
      rpmalloc_assert(0, "Failed to madvise virtual memory block as free");
    }
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
  ਜੇ (release)
    _rpmalloc_stat_sub(&mapped_ਪੰਨੇ_os, release >> ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ_ਖਿਸਕਾਓ);
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਵਿਸਤਾਰ_mark_as_subspan_unless_master(ਵਿਸਤਾਰ_t *master,
                                                         ਵਿਸਤਾਰ_t *subspan,
                                                         size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ);

//! Use global reserved spans to fulfill a memory map request (reserve size must
//! be checked by caller)
ਸਥਿਰ ਵਿਸਤਾਰ_t *_rpmalloc_global_get_reserved_spans(size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = ਸਮ੍ਰਿਤੀ_global_reserve;
  rpmalloc_ਵਿਸਤਾਰ_mark_as_subspan_unless_master(ਸਮ੍ਰਿਤੀ_global_reserve_master,
                                               ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  ਸਮ੍ਰਿਤੀ_global_reserve_ਗਿਣਤੀ -= ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  ਜੇ (ਸਮ੍ਰਿਤੀ_global_reserve_ਗਿਣਤੀ)
    ਸਮ੍ਰਿਤੀ_global_reserve =
        (ਵਿਸਤਾਰ_t *)ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਗਿਣਤੀ << ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ);
  ਨਹੀਂਤਾਂ
    ਸਮ੍ਰਿਤੀ_global_reserve = 0;
  ਵਾਪਸ ਵਿਸਤਾਰ;
}

//! Store the given spans as global reserve (must only be called from within new
//! heap allocation, not thread safe)
ਸਥਿਰ ਖਾਲੀ rpmalloc_global_ਸੈੱਟ_reserved_spans(ਵਿਸਤਾਰ_t *master, ਵਿਸਤਾਰ_t *reserve,
                                                size_t reserve_ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
  ਸਮ੍ਰਿਤੀ_global_reserve_master = master;
  ਸਮ੍ਰਿਤੀ_global_reserve_ਗਿਣਤੀ = reserve_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  ਸਮ੍ਰਿਤੀ_global_reserve = reserve;
}

////////////
///
/// Span linked list management
///
//////

//! Add a span to double linked list at the head
ਸਥਿਰ ਖਾਲੀ rpmalloc_ਵਿਸਤਾਰ_double_link_list_ਜੋੜੋ(ਵਿਸਤਾਰ_t **ਸਿਰ, ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ) {
  ਜੇ (*ਸਿਰ)
    (*ਸਿਰ)->prev = ਵਿਸਤਾਰ;
  ਵਿਸਤਾਰ->ਅਗਲਾ = *ਸਿਰ;
  *ਸਿਰ = ਵਿਸਤਾਰ;
}

//! Pop head span from double linked list
ਸਥਿਰ ਖਾਲੀ rpmalloc_ਵਿਸਤਾਰ_double_link_list_ਕੱਢੋ_ਸਿਰ(ਵਿਸਤਾਰ_t **ਸਿਰ,
                                                     ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ) {
  rpmalloc_assert(*ਸਿਰ == ਵਿਸਤਾਰ, "Linked list corrupted");
  ਵਿਸਤਾਰ = *ਸਿਰ;
  *ਸਿਰ = ਵਿਸਤਾਰ->ਅਗਲਾ;
}

//! Remove a span from double linked list
ਸਥਿਰ ਖਾਲੀ rpmalloc_ਵਿਸਤਾਰ_double_link_list_remove(ਵਿਸਤਾਰ_t **ਸਿਰ,
                                                   ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ) {
  rpmalloc_assert(*ਸਿਰ, "Linked list corrupted");
  ਜੇ (*ਸਿਰ == ਵਿਸਤਾਰ) {
    *ਸਿਰ = ਵਿਸਤਾਰ->ਅਗਲਾ;
  } ਨਹੀਂਤਾਂ {
    ਵਿਸਤਾਰ_t *ਅਗਲਾ_ਵਿਸਤਾਰ = ਵਿਸਤਾਰ->ਅਗਲਾ;
    ਵਿਸਤਾਰ_t *prev_ਵਿਸਤਾਰ = ਵਿਸਤਾਰ->prev;
    prev_ਵਿਸਤਾਰ->ਅਗਲਾ = ਅਗਲਾ_ਵਿਸਤਾਰ;
    ਜੇ (EXPECTED(ਅਗਲਾ_ਵਿਸਤਾਰ != 0))
      ਅਗਲਾ_ਵਿਸਤਾਰ->prev = prev_ਵਿਸਤਾਰ;
  }
}

////////////
///
/// Span control
///
//////

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_cache_ਪਾਓ(ਢੇਰ_t *ਢੇਰ_2, ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ);

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_finalize(ਢੇਰ_t *ਢੇਰ_2);

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_ਸੈੱਟ_reserved_spans(ਢੇਰ_t *ਢੇਰ_2, ਵਿਸਤਾਰ_t *master,
                                              ਵਿਸਤਾਰ_t *reserve,
                                              size_t reserve_ਵਿਸਤਾਰ_ਗਿਣਤੀ);

//! Declare the span to be a subspan and store distance from master span and
//! span count
ਸਥਿਰ ਖਾਲੀ rpmalloc_ਵਿਸਤਾਰ_mark_as_subspan_unless_master(ਵਿਸਤਾਰ_t *master,
                                                         ਵਿਸਤਾਰ_t *subspan,
                                                         size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
  rpmalloc_assert((subspan != master) || (subspan->flags & ਵਿਸਤਾਰ_FLAG_MASTER),
                  "Span master pointer and/or flag mismatch");
  ਜੇ (subspan != master) {
    subspan->flags = ਵਿਸਤਾਰ_FLAG_SUBSPAN;
    subspan->ਵਿੱਥ_from_master =
        (uint32_t)((uintptr_t)ਸੂਚਕ_diff(subspan, master) >>
                   ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ);
    subspan->align_ਵਿੱਥ = 0;
  }
  subspan->ਵਿਸਤਾਰ_ਗਿਣਤੀ = (uint32_t)ਵਿਸਤਾਰ_ਗਿਣਤੀ;
}

//! Use reserved spans to fulfill a memory map request (reserve size must be
//! checked by caller)
ਸਥਿਰ ਵਿਸਤਾਰ_t *rpmalloc_ਵਿਸਤਾਰ_map_from_reserve(ਢੇਰ_t *ਢੇਰ_2,
                                               size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
  // Update the heap span reserve
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = ਢੇਰ_2->ਵਿਸਤਾਰ_reserve;
  ਢੇਰ_2->ਵਿਸਤਾਰ_reserve =
      (ਵਿਸਤਾਰ_t *)ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
  ਢੇਰ_2->spans_reserved -= (uint32_t)ਵਿਸਤਾਰ_ਗਿਣਤੀ;

  rpmalloc_ਵਿਸਤਾਰ_mark_as_subspan_unless_master(ਢੇਰ_2->ਵਿਸਤਾਰ_reserve_master, ਵਿਸਤਾਰ,
                                               ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  ਜੇ (ਵਿਸਤਾਰ_ਗਿਣਤੀ <= LARGE_CLASS_ਗਿਣਤੀ)
    _rpmalloc_stat_inc(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1].spans_from_reserved);

  ਵਾਪਸ ਵਿਸਤਾਰ;
}

//! Get the aligned number of spans to map in based on wanted count, configured
//! mapping granularity and the page size
ਸਥਿਰ size_t rpmalloc_ਵਿਸਤਾਰ_align_ਗਿਣਤੀ(size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
  size_t request_ਗਿਣਤੀ = (ਵਿਸਤਾਰ_ਗਿਣਤੀ > ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ)
                             ? ਵਿਸਤਾਰ_ਗਿਣਤੀ
                             : ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ;
  ਜੇ ((ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ > ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) &&
      ((request_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) % ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ))
    request_ਗਿਣਤੀ +=
        ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ - (request_ਗਿਣਤੀ % ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ);
  ਵਾਪਸ request_ਗਿਣਤੀ;
}

//! Setup a newly mapped span
ਸਥਿਰ ਖਾਲੀ rpmalloc_ਵਿਸਤਾਰ_initialize(ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ, size_t ਕੁੱਲ_ਵਿਸਤਾਰ_ਗਿਣਤੀ,
                                      size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ, size_t align_ਵਿੱਥ) {
  ਵਿਸਤਾਰ->ਕੁੱਲ_spans = (uint32_t)ਕੁੱਲ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ = (uint32_t)ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  ਵਿਸਤਾਰ->align_ਵਿੱਥ = (uint32_t)align_ਵਿੱਥ;
  ਵਿਸਤਾਰ->flags = ਵਿਸਤਾਰ_FLAG_MASTER;
  atomic_store32(&ਵਿਸਤਾਰ->ਬਚਿਆ_spans, (int32_t)ਕੁੱਲ_ਵਿਸਤਾਰ_ਗਿਣਤੀ);
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ);

//! Map an aligned set of spans, taking configured mapping granularity and the
//! page size into account
ਸਥਿਰ ਵਿਸਤਾਰ_t *rpmalloc_ਵਿਸਤਾਰ_map_aligned_ਗਿਣਤੀ(ਢੇਰ_t *ਢੇਰ_2,
                                                size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
  // If we already have some, but not enough, reserved spans, release those to
  // heap cache and map a new full set of spans. Otherwise we would waste memory
  // if page size > span size (huge pages)
  size_t aligned_ਵਿਸਤਾਰ_ਗਿਣਤੀ = rpmalloc_ਵਿਸਤਾਰ_align_ਗਿਣਤੀ(ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  size_t align_ਵਿੱਥ = 0;
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)_rpmalloc_mmap(
      aligned_ਵਿਸਤਾਰ_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ, &align_ਵਿੱਥ);
  ਜੇ (!ਵਿਸਤਾਰ)
    ਵਾਪਸ 0;
  rpmalloc_ਵਿਸਤਾਰ_initialize(ਵਿਸਤਾਰ, aligned_ਵਿਸਤਾਰ_ਗਿਣਤੀ, ਵਿਸਤਾਰ_ਗਿਣਤੀ, align_ਵਿੱਥ);
  _rpmalloc_stat_inc(&_master_spans);
  ਜੇ (ਵਿਸਤਾਰ_ਗਿਣਤੀ <= LARGE_CLASS_ਗਿਣਤੀ)
    _rpmalloc_stat_inc(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1].spans_map_calls);
  ਜੇ (aligned_ਵਿਸਤਾਰ_ਗਿਣਤੀ > ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
    ਵਿਸਤਾਰ_t *reserved_spans =
        (ਵਿਸਤਾਰ_t *)ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
    size_t reserved_ਗਿਣਤੀ = aligned_ਵਿਸਤਾਰ_ਗਿਣਤੀ - ਵਿਸਤਾਰ_ਗਿਣਤੀ;
    ਜੇ (ਢੇਰ_2->spans_reserved) {
      rpmalloc_ਵਿਸਤਾਰ_mark_as_subspan_unless_master(
          ਢੇਰ_2->ਵਿਸਤਾਰ_reserve_master, ਢੇਰ_2->ਵਿਸਤਾਰ_reserve, ਢੇਰ_2->spans_reserved);
      rpmalloc_ਢੇਰ_cache_ਪਾਓ(ਢੇਰ_2, ਢੇਰ_2->ਵਿਸਤਾਰ_reserve);
    }
    ਜੇ (reserved_ਗਿਣਤੀ > ਸਮ੍ਰਿਤੀ_ਢੇਰ_reserve_ਗਿਣਤੀ) {
      // If huge pages or eager spam map count, the global reserve spin lock is
      // held by caller, _rpmalloc_span_map
      rpmalloc_assert(atomic_load32(&ਸਮ੍ਰਿਤੀ_global_ਤਾਲਾ) == 1,
                      "Global spin lock not held as expected");
      size_t remain_ਗਿਣਤੀ = reserved_ਗਿਣਤੀ - ਸਮ੍ਰਿਤੀ_ਢੇਰ_reserve_ਗਿਣਤੀ;
      reserved_ਗਿਣਤੀ = ਸਮ੍ਰਿਤੀ_ਢੇਰ_reserve_ਗਿਣਤੀ;
      ਵਿਸਤਾਰ_t *remain_ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)ਸੂਚਕ_ਵਿੱਥ(
          reserved_spans, reserved_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
      ਜੇ (ਸਮ੍ਰਿਤੀ_global_reserve) {
        rpmalloc_ਵਿਸਤਾਰ_mark_as_subspan_unless_master(
            ਸਮ੍ਰਿਤੀ_global_reserve_master, ਸਮ੍ਰਿਤੀ_global_reserve,
            ਸਮ੍ਰਿਤੀ_global_reserve_ਗਿਣਤੀ);
        rpmalloc_ਵਿਸਤਾਰ_unmap(ਸਮ੍ਰਿਤੀ_global_reserve);
      }
      rpmalloc_global_ਸੈੱਟ_reserved_spans(ਵਿਸਤਾਰ, remain_ਵਿਸਤਾਰ, remain_ਗਿਣਤੀ);
    }
    rpmalloc_ਢੇਰ_ਸੈੱਟ_reserved_spans(ਢੇਰ_2, ਵਿਸਤਾਰ, reserved_spans,
                                      reserved_ਗਿਣਤੀ);
  }
  ਵਾਪਸ ਵਿਸਤਾਰ;
}

//! Map in memory pages for the given number of spans (or use previously
//! reserved pages)
ਸਥਿਰ ਵਿਸਤਾਰ_t *rpmalloc_ਵਿਸਤਾਰ_map(ਢੇਰ_t *ਢੇਰ_2, size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
  ਜੇ (ਵਿਸਤਾਰ_ਗਿਣਤੀ <= ਢੇਰ_2->spans_reserved)
    ਵਾਪਸ rpmalloc_ਵਿਸਤਾਰ_map_from_reserve(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = 0;
  ਪੂਰਨ use_global_reserve =
      (ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ > ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) ||
      (ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ > ਸਮ੍ਰਿਤੀ_ਢੇਰ_reserve_ਗਿਣਤੀ);
  ਜੇ (use_global_reserve) {
    // If huge pages, make sure only one thread maps more memory to avoid bloat
    ਜਦੋਂ (!atomic_cas32_acquire(&ਸਮ੍ਰਿਤੀ_global_ਤਾਲਾ, 1, 0))
      _rpmalloc_spin();
    ਜੇ (ਸਮ੍ਰਿਤੀ_global_reserve_ਗਿਣਤੀ >= ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
      size_t reserve_ਗਿਣਤੀ =
          (!ਢੇਰ_2->spans_reserved ? ਸਮ੍ਰਿਤੀ_ਢੇਰ_reserve_ਗਿਣਤੀ : ਵਿਸਤਾਰ_ਗਿਣਤੀ);
      ਜੇ (ਸਮ੍ਰਿਤੀ_global_reserve_ਗਿਣਤੀ < reserve_ਗਿਣਤੀ)
        reserve_ਗਿਣਤੀ = ਸਮ੍ਰਿਤੀ_global_reserve_ਗਿਣਤੀ;
      ਵਿਸਤਾਰ = _rpmalloc_global_get_reserved_spans(reserve_ਗਿਣਤੀ);
      ਜੇ (ਵਿਸਤਾਰ) {
        ਜੇ (reserve_ਗਿਣਤੀ > ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
          ਵਿਸਤਾਰ_t *reserved_ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)ਸੂਚਕ_ਵਿੱਥ(
              ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਗਿਣਤੀ << ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ);
          rpmalloc_ਢੇਰ_ਸੈੱਟ_reserved_spans(ਢੇਰ_2, ਸਮ੍ਰਿਤੀ_global_reserve_master,
                                            reserved_ਵਿਸਤਾਰ,
                                            reserve_ਗਿਣਤੀ - ਵਿਸਤਾਰ_ਗਿਣਤੀ);
        }
        // Already marked as subspan in _rpmalloc_global_get_reserved_spans
        ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ = (uint32_t)ਵਿਸਤਾਰ_ਗਿਣਤੀ;
      }
    }
  }
  ਜੇ (!ਵਿਸਤਾਰ)
    ਵਿਸਤਾਰ = rpmalloc_ਵਿਸਤਾਰ_map_aligned_ਗਿਣਤੀ(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  ਜੇ (use_global_reserve)
    atomic_store32_release(&ਸਮ੍ਰਿਤੀ_global_ਤਾਲਾ, 0);
  ਵਾਪਸ ਵਿਸਤਾਰ;
}

//! Unmap memory pages for the given number of spans (or mark as unused if no
//! partial unmappings)
ਸਥਿਰ ਖਾਲੀ rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ) {
  rpmalloc_assert((ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_MASTER) ||
                      (ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(!(ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_MASTER) ||
                      !(ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_SUBSPAN),
                  "Span flag corrupted");

  ਪੂਰਨ is_master = !!(ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_MASTER);
  ਵਿਸਤਾਰ_t *master =
      is_master ? ਵਿਸਤਾਰ
                : ((ਵਿਸਤਾਰ_t *)ਸੂਚਕ_ਵਿੱਥ(
                      ਵਿਸਤਾਰ, -(intptr_t)((uintptr_t)ਵਿਸਤਾਰ->ਵਿੱਥ_from_master *
                                        ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ)));
  rpmalloc_assert(is_master || (ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert(master->flags & ਵਿਸਤਾਰ_FLAG_MASTER, "Span flag corrupted");

  size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ = ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  ਜੇ (!is_master) {
    // Directly unmap subspans (unless huge pages, in which case we defer and
    // unmap entire page range with master)
    rpmalloc_assert(ਵਿਸਤਾਰ->align_ਵਿੱਥ == 0, "Span align offset corrupted");
    ਜੇ (ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ >= ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ)
      _rpmalloc_unmap(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ, 0, 0);
  } ਨਹੀਂਤਾਂ {
    // Special double flag to denote an unmapped master
    // It must be kept in memory since span header must be used
    ਵਿਸਤਾਰ->flags |=
        ਵਿਸਤਾਰ_FLAG_MASTER | ਵਿਸਤਾਰ_FLAG_SUBSPAN | ਵਿਸਤਾਰ_FLAG_UNMAPPED_MASTER;
    rpmalloc_stat_ਜੋੜੋ(&_unmapped_master_spans, 1);
  }

  ਜੇ (atomic_add32(&master->ਬਚਿਆ_spans, -(int32_t)ਵਿਸਤਾਰ_ਗਿਣਤੀ) <= 0) {
    // Everything unmapped, unmap the master span with release flag to unmap the
    // entire range of the super span
    rpmalloc_assert(!!(master->flags & ਵਿਸਤਾਰ_FLAG_MASTER) &&
                        !!(master->flags & ਵਿਸਤਾਰ_FLAG_SUBSPAN),
                    "Span flag corrupted");
    size_t unmap_ਗਿਣਤੀ = master->ਵਿਸਤਾਰ_ਗਿਣਤੀ;
    ਜੇ (ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ < ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ)
      unmap_ਗਿਣਤੀ = master->ਕੁੱਲ_spans;
    _rpmalloc_stat_sub(&_master_spans, 1);
    _rpmalloc_stat_sub(&_unmapped_master_spans, 1);
    _rpmalloc_unmap(master, unmap_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ,
                    master->align_ਵਿੱਥ,
                    (size_t)master->ਕੁੱਲ_spans * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
  }
}

//! Move the span (used for small or medium allocations) to the heap thread
//! cache
ਸਥਿਰ ਖਾਲੀ rpmalloc_ਵਿਸਤਾਰ_release_ਮੰਜ਼ਿਲ_cache(ਢੇਰ_t *ਢੇਰ_2, ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ) {
  rpmalloc_assert(ਢੇਰ_2 == ਵਿਸਤਾਰ->ਢੇਰ_2, "Span heap pointer corrupted");
  rpmalloc_assert(ਵਿਸਤਾਰ->ਮਾਪ_class < ਮਾਪ_CLASS_ਗਿਣਤੀ,
                  "Invalid span size class");
  rpmalloc_assert(ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ == 1, "Invalid span count");
#ਜੇ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  atomic_decr32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[0].current);
#ਜੇ_ਅੰਤ
  _rpmalloc_stat_dec(&ਢੇਰ_2->ਮਾਪ_class_use[ਵਿਸਤਾਰ->ਮਾਪ_class].spans_current);
  ਜੇ (!ਢੇਰ_2->finalize) {
    _rpmalloc_stat_inc(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[0].spans_ਮੰਜ਼ਿਲ_cache);
    _rpmalloc_stat_inc(&ਢੇਰ_2->ਮਾਪ_class_use[ਵਿਸਤਾਰ->ਮਾਪ_class].spans_ਮੰਜ਼ਿਲ_cache);
    ਜੇ (ਢੇਰ_2->ਮਾਪ_class[ਵਿਸਤਾਰ->ਮਾਪ_class].cache)
      rpmalloc_ਢੇਰ_cache_ਪਾਓ(ਢੇਰ_2,
                                  ਢੇਰ_2->ਮਾਪ_class[ਵਿਸਤਾਰ->ਮਾਪ_class].cache);
    ਢੇਰ_2->ਮਾਪ_class[ਵਿਸਤਾਰ->ਮਾਪ_class].cache = ਵਿਸਤਾਰ;
  } ਨਹੀਂਤਾਂ {
    rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ);
  }
}

//! Initialize a (partial) free list up to next system memory page, while
//! reserving the first block as allocated, returning number of blocks in list
ਸਥਿਰ uint32_t free_list_partial_init(ਖਾਲੀ **list, ਖਾਲੀ **ਪਹਿਲਾ_ਖੰਡ,
                                       ਖਾਲੀ *ਪੰਨਾ_ਸ਼ੁਰੂ, ਖਾਲੀ *ਖੰਡ_ਸ਼ੁਰੂ,
                                       uint32_t ਖੰਡ_ਗਿਣਤੀ,
                                       uint32_t ਖੰਡ_ਮਾਪ_2) {
  rpmalloc_assert(ਖੰਡ_ਗਿਣਤੀ, "Internal failure");
  *ਪਹਿਲਾ_ਖੰਡ = ਖੰਡ_ਸ਼ੁਰੂ;
  ਜੇ (ਖੰਡ_ਗਿਣਤੀ > 1) {
    ਖਾਲੀ *free_ਖੰਡ = ਸੂਚਕ_ਵਿੱਥ(ਖੰਡ_ਸ਼ੁਰੂ, ਖੰਡ_ਮਾਪ_2);
    ਖਾਲੀ *ਖੰਡ_ਸਿਰਾ =
        ਸੂਚਕ_ਵਿੱਥ(ਖੰਡ_ਸ਼ੁਰੂ, (size_t)ਖੰਡ_ਮਾਪ_2 * ਖੰਡ_ਗਿਣਤੀ);
    // If block size is less than half a memory page, bound init to next memory
    // page boundary
    ਜੇ (ਖੰਡ_ਮਾਪ_2 < (ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ >> 1)) {
      ਖਾਲੀ *ਪੰਨਾ_ਸਿਰਾ = ਸੂਚਕ_ਵਿੱਥ(ਪੰਨਾ_ਸ਼ੁਰੂ, ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ);
      ਜੇ (ਪੰਨਾ_ਸਿਰਾ < ਖੰਡ_ਸਿਰਾ)
        ਖੰਡ_ਸਿਰਾ = ਪੰਨਾ_ਸਿਰਾ;
    }
    *list = free_ਖੰਡ;
    ਖੰਡ_ਗਿਣਤੀ = 2;
    ਖਾਲੀ *ਅਗਲਾ_ਖੰਡ = ਸੂਚਕ_ਵਿੱਥ(free_ਖੰਡ, ਖੰਡ_ਮਾਪ_2);
    ਜਦੋਂ (ਅਗਲਾ_ਖੰਡ < ਖੰਡ_ਸਿਰਾ) {
      *((ਖਾਲੀ **)free_ਖੰਡ) = ਅਗਲਾ_ਖੰਡ;
      free_ਖੰਡ = ਅਗਲਾ_ਖੰਡ;
      ++ਖੰਡ_ਗਿਣਤੀ;
      ਅਗਲਾ_ਖੰਡ = ਸੂਚਕ_ਵਿੱਥ(ਅਗਲਾ_ਖੰਡ, ਖੰਡ_ਮਾਪ_2);
    }
    *((ਖਾਲੀ **)free_ਖੰਡ) = 0;
  } ਨਹੀਂਤਾਂ {
    *list = 0;
  }
  ਵਾਪਸ ਖੰਡ_ਗਿਣਤੀ;
}

//! Initialize an unused span (from cache or mapped) to be new active span,
//! putting the initial free list in heap class free list
ਸਥਿਰ ਖਾਲੀ *rpmalloc_ਵਿਸਤਾਰ_initialize_new(ਢੇਰ_t *ਢੇਰ_2,
                                           ਢੇਰ_ਮਾਪ_class_t *ਢੇਰ_ਮਾਪ_class,
                                           ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ, uint32_t class_idx) {
  rpmalloc_assert(ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ == 1, "Internal failure");
  ਮਾਪ_class_t *ਮਾਪ_class = ਸਮ੍ਰਿਤੀ_ਮਾਪ_class + class_idx;
  ਵਿਸਤਾਰ->ਮਾਪ_class = class_idx;
  ਵਿਸਤਾਰ->ਢੇਰ_2 = ਢੇਰ_2;
  ਵਿਸਤਾਰ->flags &= ~ਵਿਸਤਾਰ_FLAG_ALIGNED_ਖੰਡਾਂ;
  ਵਿਸਤਾਰ->ਖੰਡ_ਮਾਪ_2 = ਮਾਪ_class->ਖੰਡ_ਮਾਪ_2;
  ਵਿਸਤਾਰ->ਖੰਡ_ਗਿਣਤੀ = ਮਾਪ_class->ਖੰਡ_ਗਿਣਤੀ;
  ਵਿਸਤਾਰ->free_list = 0;
  ਵਿਸਤਾਰ->list_ਮਾਪ = 0;
  atomic_store_ptr_release(&ਵਿਸਤਾਰ->free_list_deferred, 0);

  // Setup free list. Only initialize one system page worth of free blocks in
  // list
  ਖਾਲੀ *ਖੰਡ;
  ਵਿਸਤਾਰ->free_list_ਹੱਦਬੰਦੀ =
      free_list_partial_init(&ਢੇਰ_ਮਾਪ_class->free_list, &ਖੰਡ, ਵਿਸਤਾਰ,
                             ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ),
                             ਮਾਪ_class->ਖੰਡ_ਗਿਣਤੀ, ਮਾਪ_class->ਖੰਡ_ਮਾਪ_2);
  // Link span as partial if there remains blocks to be initialized as free
  // list, or full if fully initialized
  ਜੇ (ਵਿਸਤਾਰ->free_list_ਹੱਦਬੰਦੀ < ਵਿਸਤਾਰ->ਖੰਡ_ਗਿਣਤੀ) {
    rpmalloc_ਵਿਸਤਾਰ_double_link_list_ਜੋੜੋ(&ਢੇਰ_ਮਾਪ_class->partial_ਵਿਸਤਾਰ, ਵਿਸਤਾਰ);
    ਵਿਸਤਾਰ->ਵਰਤਿਆ_ਗਿਣਤੀ = ਵਿਸਤਾਰ->free_list_ਹੱਦਬੰਦੀ;
  } ਨਹੀਂਤਾਂ {
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
    rpmalloc_ਵਿਸਤਾਰ_double_link_list_ਜੋੜੋ(&ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ[class_idx], ਵਿਸਤਾਰ);
#ਜੇ_ਅੰਤ
    ++ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
    ਵਿਸਤਾਰ->ਵਰਤਿਆ_ਗਿਣਤੀ = ਵਿਸਤਾਰ->ਖੰਡ_ਗਿਣਤੀ;
  }
  ਵਾਪਸ ਖੰਡ;
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਵਿਸਤਾਰ_ਕੱਢੋ_free_list_deferred(ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ) {
  // We need acquire semantics on the CAS operation since we are interested in
  // the list size Refer to _rpmalloc_deallocate_defer_small_or_medium for
  // further comments on this dependency
  ਕਰੋ {
    ਵਿਸਤਾਰ->free_list =
        atomic_exchange_ptr_acquire(&ਵਿਸਤਾਰ->free_list_deferred, INVALID_ਸੂਚਕ);
  } ਜਦੋਂ (ਵਿਸਤਾਰ->free_list == INVALID_ਸੂਚਕ);
  ਵਿਸਤਾਰ->ਵਰਤਿਆ_ਗਿਣਤੀ -= ਵਿਸਤਾਰ->list_ਮਾਪ;
  ਵਿਸਤਾਰ->list_ਮਾਪ = 0;
  atomic_store_ptr_release(&ਵਿਸਤਾਰ->free_list_deferred, 0);
}

ਸਥਿਰ ਪੂਰਨ rpmalloc_ਵਿਸਤਾਰ_is_fully_utilized(ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ) {
  rpmalloc_assert(ਵਿਸਤਾਰ->free_list_ਹੱਦਬੰਦੀ <= ਵਿਸਤਾਰ->ਖੰਡ_ਗਿਣਤੀ,
                  "Span free list corrupted");
  ਵਾਪਸ !ਵਿਸਤਾਰ->free_list && (ਵਿਸਤਾਰ->free_list_ਹੱਦਬੰਦੀ >= ਵਿਸਤਾਰ->ਖੰਡ_ਗਿਣਤੀ);
}

ਸਥਿਰ ਪੂਰਨ rpmalloc_ਵਿਸਤਾਰ_finalize(ਢੇਰ_t *ਢੇਰ_2, size_t iclass, ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ,
                                   ਵਿਸਤਾਰ_t **list_ਸਿਰ) {
  ਖਾਲੀ *free_list = ਢੇਰ_2->ਮਾਪ_class[iclass].free_list;
  ਵਿਸਤਾਰ_t *class_ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)((uintptr_t)free_list & ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ);
  ਜੇ (ਵਿਸਤਾਰ == class_ਵਿਸਤਾਰ) {
    // Adopt the heap class free list back into the span free list
    ਖਾਲੀ *ਖੰਡ = ਵਿਸਤਾਰ->free_list;
    ਖਾਲੀ *ਆਖਰੀ_ਖੰਡ = 0;
    ਜਦੋਂ (ਖੰਡ) {
      ਆਖਰੀ_ਖੰਡ = ਖੰਡ;
      ਖੰਡ = *((ਖਾਲੀ **)ਖੰਡ);
    }
    uint32_t free_ਗਿਣਤੀ = 0;
    ਖੰਡ = free_list;
    ਜਦੋਂ (ਖੰਡ) {
      ++free_ਗਿਣਤੀ;
      ਖੰਡ = *((ਖਾਲੀ **)ਖੰਡ);
    }
    ਜੇ (ਆਖਰੀ_ਖੰਡ) {
      *((ਖਾਲੀ **)ਆਖਰੀ_ਖੰਡ) = free_list;
    } ਨਹੀਂਤਾਂ {
      ਵਿਸਤਾਰ->free_list = free_list;
    }
    ਢੇਰ_2->ਮਾਪ_class[iclass].free_list = 0;
    ਵਿਸਤਾਰ->ਵਰਤਿਆ_ਗਿਣਤੀ -= free_ਗਿਣਤੀ;
  }
  // If this assert triggers you have memory leaks
  rpmalloc_assert(ਵਿਸਤਾਰ->list_ਮਾਪ == ਵਿਸਤਾਰ->ਵਰਤਿਆ_ਗਿਣਤੀ, "Memory leak detected");
  ਜੇ (ਵਿਸਤਾਰ->list_ਮਾਪ == ਵਿਸਤਾਰ->ਵਰਤਿਆ_ਗਿਣਤੀ) {
    _rpmalloc_stat_dec(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[0].current);
    _rpmalloc_stat_dec(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_current);
    // This function only used for spans in double linked lists
    ਜੇ (list_ਸਿਰ)
      rpmalloc_ਵਿਸਤਾਰ_double_link_list_remove(list_ਸਿਰ, ਵਿਸਤਾਰ);
    rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ);
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

////////////
///
/// Global cache
///
//////

#ਜੇ ENABLE_GLOBAL_CACHE

//! Finalize a global cache
ਸਥਿਰ ਖਾਲੀ _rpmalloc_global_cache_finalize(global_cache_t *cache) {
  ਜਦੋਂ (!atomic_cas32_acquire(&cache->ਤਾਲਾ, 1, 0))
    _rpmalloc_spin();

  ਲਈ (size_t ispan = 0; ispan < cache->ਗਿਣਤੀ_2; ++ispan)
    rpmalloc_ਵਿਸਤਾਰ_unmap(cache->ਵਿਸਤਾਰ[ispan]);
  cache->ਗਿਣਤੀ_2 = 0;

  ਜਦੋਂ (cache->ਉਛਾਲ_2) {
    ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = cache->ਉਛਾਲ_2;
    cache->ਉਛਾਲ_2 = ਵਿਸਤਾਰ->ਅਗਲਾ;
    rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ);
  }

  atomic_store32_release(&cache->ਤਾਲਾ, 0);
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_global_cache_ਪਾਓ_spans(ਵਿਸਤਾਰ_t **ਵਿਸਤਾਰ,
                                                size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ,
                                                size_t ਗਿਣਤੀ_2) {
  ਅਚਲ size_t cache_ਹੱਦਬੰਦੀ =
      (ਵਿਸਤਾਰ_ਗਿਣਤੀ == 1) ? GLOBAL_CACHE_MULTIPLIER * MAX_THREAD_ਵਿਸਤਾਰ_CACHE
                        : GLOBAL_CACHE_MULTIPLIER *
                              (MAX_THREAD_ਵਿਸਤਾਰ_LARGE_CACHE - (ਵਿਸਤਾਰ_ਗਿਣਤੀ >> 1));

  global_cache_t *cache = &ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_cache[ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1];

  size_t ਪਾਓ_ਗਿਣਤੀ = ਗਿਣਤੀ_2;
  ਜਦੋਂ (!atomic_cas32_acquire(&cache->ਤਾਲਾ, 1, 0))
    _rpmalloc_spin();

#ਜੇ ENABLE_STATISTICS
  cache->ਪਾਓ_ਗਿਣਤੀ += ਗਿਣਤੀ_2;
#ਜੇ_ਅੰਤ
  ਜੇ ((cache->ਗਿਣਤੀ_2 + ਪਾਓ_ਗਿਣਤੀ) > cache_ਹੱਦਬੰਦੀ)
    ਪਾਓ_ਗਿਣਤੀ = cache_ਹੱਦਬੰਦੀ - cache->ਗਿਣਤੀ_2;

  ਸਮ੍ਰਿਤੀ_ਨਕਲ(cache->ਵਿਸਤਾਰ + cache->ਗਿਣਤੀ_2, ਵਿਸਤਾਰ, ਆਕਾਰ(ਵਿਸਤਾਰ_t *) * ਪਾਓ_ਗਿਣਤੀ);
  cache->ਗਿਣਤੀ_2 += (uint32_t)ਪਾਓ_ਗਿਣਤੀ;

#ਜੇ ENABLE_UNLIMITED_CACHE
  ਜਦੋਂ (ਪਾਓ_ਗਿਣਤੀ < ਗਿਣਤੀ_2) {
#ਨਹੀਂਤਾਂ
  // Enable unlimited cache if huge pages, or we will leak since it is unlikely
  // that an entire huge page will be unmapped, and we're unable to partially
  // decommit a huge page
  ਜਦੋਂ ((ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ > ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) && (ਪਾਓ_ਗਿਣਤੀ < ਗਿਣਤੀ_2)) {
#ਜੇ_ਅੰਤ
    ਵਿਸਤਾਰ_t *current_ਵਿਸਤਾਰ = ਵਿਸਤਾਰ[ਪਾਓ_ਗਿਣਤੀ++];
    current_ਵਿਸਤਾਰ->ਅਗਲਾ = cache->ਉਛਾਲ_2;
    cache->ਉਛਾਲ_2 = current_ਵਿਸਤਾਰ;
  }
  atomic_store32_release(&cache->ਤਾਲਾ, 0);

  ਵਿਸਤਾਰ_t *keep = 0;
  ਲਈ (size_t ispan = ਪਾਓ_ਗਿਣਤੀ; ispan < ਗਿਣਤੀ_2; ++ispan) {
    ਵਿਸਤਾਰ_t *current_ਵਿਸਤਾਰ = ਵਿਸਤਾਰ[ispan];
    // Keep master spans that has remaining subspans to avoid dangling them
    ਜੇ ((current_ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_MASTER) &&
        (atomic_load32(&current_ਵਿਸਤਾਰ->ਬਚਿਆ_spans) >
         (int32_t)current_ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ)) {
      current_ਵਿਸਤਾਰ->ਅਗਲਾ = keep;
      keep = current_ਵਿਸਤਾਰ;
    } ਨਹੀਂਤਾਂ {
      rpmalloc_ਵਿਸਤਾਰ_unmap(current_ਵਿਸਤਾਰ);
    }
  }

  ਜੇ (keep) {
    ਜਦੋਂ (!atomic_cas32_acquire(&cache->ਤਾਲਾ, 1, 0))
      _rpmalloc_spin();

    size_t islot = 0;
    ਜਦੋਂ (keep) {
      ਲਈ (; islot < cache->ਗਿਣਤੀ_2; ++islot) {
        ਵਿਸਤਾਰ_t *current_ਵਿਸਤਾਰ = cache->ਵਿਸਤਾਰ[islot];
        ਜੇ (!(current_ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_MASTER) ||
            ((current_ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_MASTER) &&
             (atomic_load32(&current_ਵਿਸਤਾਰ->ਬਚਿਆ_spans) <=
              (int32_t)current_ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ))) {
          rpmalloc_ਵਿਸਤਾਰ_unmap(current_ਵਿਸਤਾਰ);
          cache->ਵਿਸਤਾਰ[islot] = keep;
          ਰੋਕੋ;
        }
      }
      ਜੇ (islot == cache->ਗਿਣਤੀ_2)
        ਰੋਕੋ;
      keep = keep->ਅਗਲਾ;
    }

    ਜੇ (keep) {
      ਵਿਸਤਾਰ_t *ਪੂਛ_2 = keep;
      ਜਦੋਂ (ਪੂਛ_2->ਅਗਲਾ)
        ਪੂਛ_2 = ਪੂਛ_2->ਅਗਲਾ;
      ਪੂਛ_2->ਅਗਲਾ = cache->ਉਛਾਲ_2;
      cache->ਉਛਾਲ_2 = keep;
    }

    atomic_store32_release(&cache->ਤਾਲਾ, 0);
  }
}

ਸਥਿਰ size_t rpmalloc_global_cache_ਕੱਢੋ_spans(ਵਿਸਤਾਰ_t **ਵਿਸਤਾਰ,
                                                   size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ,
                                                   size_t ਗਿਣਤੀ_2) {
  global_cache_t *cache = &ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_cache[ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1];

  size_t ਕੱਢੋ_ਗਿਣਤੀ_2 = 0;
  ਜਦੋਂ (!atomic_cas32_acquire(&cache->ਤਾਲਾ, 1, 0))
    _rpmalloc_spin();

#ਜੇ ENABLE_STATISTICS
  cache->ਕੱਢੋ_ਗਿਣਤੀ_2 += ਗਿਣਤੀ_2;
#ਜੇ_ਅੰਤ
  size_t want = ਗਿਣਤੀ_2 - ਕੱਢੋ_ਗਿਣਤੀ_2;
  ਜੇ (want > cache->ਗਿਣਤੀ_2)
    want = cache->ਗਿਣਤੀ_2;

  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਵਿਸਤਾਰ + ਕੱਢੋ_ਗਿਣਤੀ_2, cache->ਵਿਸਤਾਰ + (cache->ਗਿਣਤੀ_2 - want),
         ਆਕਾਰ(ਵਿਸਤਾਰ_t *) * want);
  cache->ਗਿਣਤੀ_2 -= (uint32_t)want;
  ਕੱਢੋ_ਗਿਣਤੀ_2 += want;

  ਜਦੋਂ ((ਕੱਢੋ_ਗਿਣਤੀ_2 < ਗਿਣਤੀ_2) && cache->ਉਛਾਲ_2) {
    ਵਿਸਤਾਰ_t *current_ਵਿਸਤਾਰ = cache->ਉਛਾਲ_2;
    ਵਿਸਤਾਰ[ਕੱਢੋ_ਗਿਣਤੀ_2++] = current_ਵਿਸਤਾਰ;
    cache->ਉਛਾਲ_2 = current_ਵਿਸਤਾਰ->ਅਗਲਾ;
  }

#ਜੇ ENABLE_ASSERTS
  ਲਈ (size_t ispan = 0; ispan < ਕੱਢੋ_ਗਿਣਤੀ_2; ++ispan) {
    rpmalloc_assert(ਵਿਸਤਾਰ[ispan]->ਵਿਸਤਾਰ_ਗਿਣਤੀ == ਵਿਸਤਾਰ_ਗਿਣਤੀ,
                    "Global cache span count mismatch");
  }
#ਜੇ_ਅੰਤ

  atomic_store32_release(&cache->ਤਾਲਾ, 0);

  ਵਾਪਸ ਕੱਢੋ_ਗਿਣਤੀ_2;
}

#ਜੇ_ਅੰਤ

////////////
///
/// Heap control
///
//////

ਸਥਿਰ ਖਾਲੀ _rpmalloc_deallocate_huge(ਵਿਸਤਾਰ_t *);

//! Store the given spans as reserve in the given heap
ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_ਸੈੱਟ_reserved_spans(ਢੇਰ_t *ਢੇਰ_2, ਵਿਸਤਾਰ_t *master,
                                              ਵਿਸਤਾਰ_t *reserve,
                                              size_t reserve_ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
  ਢੇਰ_2->ਵਿਸਤਾਰ_reserve_master = master;
  ਢੇਰ_2->ਵਿਸਤਾਰ_reserve = reserve;
  ਢੇਰ_2->spans_reserved = (uint32_t)reserve_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
}

//! Adopt the deferred span cache list, optionally extracting the first single
//! span for immediate re-use
ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_cache_adopt_deferred(ਢੇਰ_t *ਢੇਰ_2,
                                                ਵਿਸਤਾਰ_t **single_ਵਿਸਤਾਰ) {
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)((ਖਾਲੀ *)atomic_exchange_ptr_acquire(
      &ਢੇਰ_2->ਵਿਸਤਾਰ_free_deferred, 0));
  ਜਦੋਂ (ਵਿਸਤਾਰ) {
    ਵਿਸਤਾਰ_t *ਅਗਲਾ_ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)ਵਿਸਤਾਰ->free_list;
    rpmalloc_assert(ਵਿਸਤਾਰ->ਢੇਰ_2 == ਢੇਰ_2, "Span heap pointer corrupted");
    ਜੇ (EXPECTED(ਵਿਸਤਾਰ->ਮਾਪ_class < ਮਾਪ_CLASS_ਗਿਣਤੀ)) {
      rpmalloc_assert(ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ, "Heap span counter corrupted");
      --ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
      _rpmalloc_stat_dec(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[0].spans_deferred);
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
      rpmalloc_ਵਿਸਤਾਰ_double_link_list_remove(&ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ[ਵਿਸਤਾਰ->ਮਾਪ_class],
                                             ਵਿਸਤਾਰ);
#ਜੇ_ਅੰਤ
      _rpmalloc_stat_dec(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[0].current);
      _rpmalloc_stat_dec(&ਢੇਰ_2->ਮਾਪ_class_use[ਵਿਸਤਾਰ->ਮਾਪ_class].spans_current);
      ਜੇ (single_ਵਿਸਤਾਰ && !*single_ਵਿਸਤਾਰ)
        *single_ਵਿਸਤਾਰ = ਵਿਸਤਾਰ;
      ਨਹੀਂਤਾਂ
        rpmalloc_ਢੇਰ_cache_ਪਾਓ(ਢੇਰ_2, ਵਿਸਤਾਰ);
    } ਨਹੀਂਤਾਂ {
      ਜੇ (ਵਿਸਤਾਰ->ਮਾਪ_class == ਮਾਪ_CLASS_HUGE) {
        _rpmalloc_deallocate_huge(ਵਿਸਤਾਰ);
      } ਨਹੀਂਤਾਂ {
        rpmalloc_assert(ਵਿਸਤਾਰ->ਮਾਪ_class == ਮਾਪ_CLASS_LARGE,
                        "Span size class invalid");
        rpmalloc_assert(ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ, "Heap span counter corrupted");
        --ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
        rpmalloc_ਵਿਸਤਾਰ_double_link_list_remove(&ਢੇਰ_2->large_huge_ਵਿਸਤਾਰ, ਵਿਸਤਾਰ);
#ਜੇ_ਅੰਤ
        uint32_t idx = ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1;
        _rpmalloc_stat_dec(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[idx].spans_deferred);
        _rpmalloc_stat_dec(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[idx].current);
        ਜੇ (!idx && single_ਵਿਸਤਾਰ && !*single_ਵਿਸਤਾਰ)
          *single_ਵਿਸਤਾਰ = ਵਿਸਤਾਰ;
        ਨਹੀਂਤਾਂ
          rpmalloc_ਢੇਰ_cache_ਪਾਓ(ਢੇਰ_2, ਵਿਸਤਾਰ);
      }
    }
    ਵਿਸਤਾਰ = ਅਗਲਾ_ਵਿਸਤਾਰ;
  }
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_unmap(ਢੇਰ_t *ਢੇਰ_2) {
  ਜੇ (!ਢੇਰ_2->master_ਢੇਰ) {
    ਜੇ ((ਢੇਰ_2->finalize > 1) && !atomic_load32(&ਢੇਰ_2->ਬੱਚਾ_ਗਿਣਤੀ)) {
      ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)((uintptr_t)ਢੇਰ_2 & ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ);
      rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ);
    }
  } ਨਹੀਂਤਾਂ {
    ਜੇ (atomic_decr32(&ਢੇਰ_2->master_ਢੇਰ->ਬੱਚਾ_ਗਿਣਤੀ) == 0) {
      rpmalloc_ਢੇਰ_unmap(ਢੇਰ_2->master_ਢੇਰ);
    }
  }
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_global_finalize(ਢੇਰ_t *ਢੇਰ_2) {
  ਜੇ (ਢੇਰ_2->finalize++ > 1) {
    --ਢੇਰ_2->finalize;
    ਵਾਪਸ;
  }

  rpmalloc_ਢੇਰ_finalize(ਢੇਰ_2);

#ਜੇ ENABLE_THREAD_CACHE
  ਲਈ (size_t iclass = 0; iclass < LARGE_CLASS_ਗਿਣਤੀ; ++iclass) {
    ਵਿਸਤਾਰ_cache_t *ਵਿਸਤਾਰ_cache;
    ਜੇ (!iclass)
      ਵਿਸਤਾਰ_cache = &ਢੇਰ_2->ਵਿਸਤਾਰ_cache;
    ਨਹੀਂਤਾਂ
      ਵਿਸਤਾਰ_cache = (ਵਿਸਤਾਰ_cache_t *)(ਢੇਰ_2->ਵਿਸਤਾਰ_large_cache + (iclass - 1));
    ਲਈ (size_t ispan = 0; ispan < ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2; ++ispan)
      rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ[ispan]);
    ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 = 0;
  }
#ਜੇ_ਅੰਤ

  ਜੇ (ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
    --ਢੇਰ_2->finalize;
    ਵਾਪਸ;
  }

  ਲਈ (size_t iclass = 0; iclass < ਮਾਪ_CLASS_ਗਿਣਤੀ; ++iclass) {
    ਜੇ (ਢੇਰ_2->ਮਾਪ_class[iclass].free_list ||
        ਢੇਰ_2->ਮਾਪ_class[iclass].partial_ਵਿਸਤਾਰ) {
      --ਢੇਰ_2->finalize;
      ਵਾਪਸ;
    }
  }
  // Heap is now completely free, unmap and remove from heap list
  size_t list_idx = (size_t)ਢੇਰ_2->id % ਢੇਰ_ARRAY_ਮਾਪ;
  ਢੇਰ_t *list_ਢੇਰ = ਸਮ੍ਰਿਤੀ_heaps[list_idx];
  ਜੇ (list_ਢੇਰ == ਢੇਰ_2) {
    ਸਮ੍ਰਿਤੀ_heaps[list_idx] = ਢੇਰ_2->ਅਗਲਾ_ਢੇਰ;
  } ਨਹੀਂਤਾਂ {
    ਜਦੋਂ (list_ਢੇਰ->ਅਗਲਾ_ਢੇਰ != ਢੇਰ_2)
      list_ਢੇਰ = list_ਢੇਰ->ਅਗਲਾ_ਢੇਰ;
    list_ਢੇਰ->ਅਗਲਾ_ਢੇਰ = ਢੇਰ_2->ਅਗਲਾ_ਢੇਰ;
  }

  rpmalloc_ਢੇਰ_unmap(ਢੇਰ_2);
}

//! Insert a single span into thread heap cache, releasing to global cache if
//! overflow
ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_cache_ਪਾਓ(ਢੇਰ_t *ਢੇਰ_2, ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ) {
  ਜੇ (UNEXPECTED(ਢੇਰ_2->finalize != 0)) {
    rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ);
    rpmalloc_ਢੇਰ_global_finalize(ਢੇਰ_2);
    ਵਾਪਸ;
  }
#ਜੇ ENABLE_THREAD_CACHE
  size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ = ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  _rpmalloc_stat_inc(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1].spans_ਮੰਜ਼ਿਲ_cache);
  ਜੇ (ਵਿਸਤਾਰ_ਗਿਣਤੀ == 1) {
    ਵਿਸਤਾਰ_cache_t *ਵਿਸਤਾਰ_cache = &ਢੇਰ_2->ਵਿਸਤਾਰ_cache;
    ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ[ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2++] = ਵਿਸਤਾਰ;
    ਜੇ (ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 == MAX_THREAD_ਵਿਸਤਾਰ_CACHE) {
      ਅਚਲ size_t remain_ਗਿਣਤੀ =
          MAX_THREAD_ਵਿਸਤਾਰ_CACHE - THREAD_ਵਿਸਤਾਰ_CACHE_TRANSFER;
#ਜੇ ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&ਢੇਰ_2->thread_ਮੰਜ਼ਿਲ_global,
                           THREAD_ਵਿਸਤਾਰ_CACHE_TRANSFER * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
      rpmalloc_stat_ਜੋੜੋ(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1].spans_ਮੰਜ਼ਿਲ_global,
                         THREAD_ਵਿਸਤਾਰ_CACHE_TRANSFER);
      rpmalloc_global_cache_ਪਾਓ_spans(ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ + remain_ਗਿਣਤੀ,
                                          ਵਿਸਤਾਰ_ਗਿਣਤੀ,
                                          THREAD_ਵਿਸਤਾਰ_CACHE_TRANSFER);
#ਨਹੀਂਤਾਂ
      ਲਈ (size_t ispan = 0; ispan < THREAD_ਵਿਸਤਾਰ_CACHE_TRANSFER; ++ispan)
        rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ[remain_ਗਿਣਤੀ + ispan]);
#ਜੇ_ਅੰਤ
      ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 = remain_ਗਿਣਤੀ;
    }
  } ਨਹੀਂਤਾਂ {
    size_t cache_idx = ਵਿਸਤਾਰ_ਗਿਣਤੀ - 2;
    ਵਿਸਤਾਰ_large_cache_t *ਵਿਸਤਾਰ_cache = ਢੇਰ_2->ਵਿਸਤਾਰ_large_cache + cache_idx;
    ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ[ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2++] = ਵਿਸਤਾਰ;
    ਅਚਲ size_t cache_ਹੱਦਬੰਦੀ =
        (MAX_THREAD_ਵਿਸਤਾਰ_LARGE_CACHE - (ਵਿਸਤਾਰ_ਗਿਣਤੀ >> 1));
    ਜੇ (ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 == cache_ਹੱਦਬੰਦੀ) {
      ਅਚਲ size_t transfer_ਹੱਦਬੰਦੀ = 2 + (cache_ਹੱਦਬੰਦੀ >> 2);
      ਅਚਲ size_t transfer_ਗਿਣਤੀ =
          (THREAD_ਵਿਸਤਾਰ_LARGE_CACHE_TRANSFER <= transfer_ਹੱਦਬੰਦੀ
               ? THREAD_ਵਿਸਤਾਰ_LARGE_CACHE_TRANSFER
               : transfer_ਹੱਦਬੰਦੀ);
      ਅਚਲ size_t remain_ਗਿਣਤੀ = cache_ਹੱਦਬੰਦੀ - transfer_ਗਿਣਤੀ;
#ਜੇ ENABLE_GLOBAL_CACHE
      _rpmalloc_stat_add64(&ਢੇਰ_2->thread_ਮੰਜ਼ਿਲ_global,
                           transfer_ਗਿਣਤੀ * ਵਿਸਤਾਰ_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
      rpmalloc_stat_ਜੋੜੋ(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1].spans_ਮੰਜ਼ਿਲ_global,
                         transfer_ਗਿਣਤੀ);
      rpmalloc_global_cache_ਪਾਓ_spans(ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ + remain_ਗਿਣਤੀ,
                                          ਵਿਸਤਾਰ_ਗਿਣਤੀ, transfer_ਗਿਣਤੀ);
#ਨਹੀਂਤਾਂ
      ਲਈ (size_t ispan = 0; ispan < transfer_ਗਿਣਤੀ; ++ispan)
        rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ[remain_ਗਿਣਤੀ + ispan]);
#ਜੇ_ਅੰਤ
      ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 = remain_ਗਿਣਤੀ;
    }
  }
#ਨਹੀਂਤਾਂ
  (ਖਾਲੀ)ਆਕਾਰ(ਢੇਰ_2);
  rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ);
#ਜੇ_ਅੰਤ
}

//! Extract the given number of spans from the different cache levels
ਸਥਿਰ ਵਿਸਤਾਰ_t *rpmalloc_ਢੇਰ_thread_cache_ਕੱਢੋ(ਢੇਰ_t *ਢੇਰ_2,
                                                   size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = 0;
#ਜੇ ENABLE_THREAD_CACHE
  ਵਿਸਤਾਰ_cache_t *ਵਿਸਤਾਰ_cache;
  ਜੇ (ਵਿਸਤਾਰ_ਗਿਣਤੀ == 1)
    ਵਿਸਤਾਰ_cache = &ਢੇਰ_2->ਵਿਸਤਾਰ_cache;
  ਨਹੀਂਤਾਂ
    ਵਿਸਤਾਰ_cache = (ਵਿਸਤਾਰ_cache_t *)(ਢੇਰ_2->ਵਿਸਤਾਰ_large_cache + (ਵਿਸਤਾਰ_ਗਿਣਤੀ - 2));
  ਜੇ (ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2) {
    _rpmalloc_stat_inc(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1].spans_from_cache);
    ਵਾਪਸ ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ[--ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2];
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ ਵਿਸਤਾਰ;
}

ਸਥਿਰ ਵਿਸਤਾਰ_t *rpmalloc_ਢੇਰ_thread_cache_deferred_ਕੱਢੋ(ਢੇਰ_t *ਢੇਰ_2,
                                                            size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = 0;
  ਜੇ (ਵਿਸਤਾਰ_ਗਿਣਤੀ == 1) {
    rpmalloc_ਢੇਰ_cache_adopt_deferred(ਢੇਰ_2, &ਵਿਸਤਾਰ);
  } ਨਹੀਂਤਾਂ {
    rpmalloc_ਢੇਰ_cache_adopt_deferred(ਢੇਰ_2, 0);
    ਵਿਸਤਾਰ = rpmalloc_ਢੇਰ_thread_cache_ਕੱਢੋ(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  }
  ਵਾਪਸ ਵਿਸਤਾਰ;
}

ਸਥਿਰ ਵਿਸਤਾਰ_t *rpmalloc_ਢੇਰ_reserved_ਕੱਢੋ(ਢੇਰ_t *ਢੇਰ_2,
                                               size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
  ਜੇ (ਢੇਰ_2->spans_reserved >= ਵਿਸਤਾਰ_ਗਿਣਤੀ)
    ਵਾਪਸ rpmalloc_ਵਿਸਤਾਰ_map(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  ਵਾਪਸ 0;
}

//! Extract a span from the global cache
ਸਥਿਰ ਵਿਸਤਾਰ_t *rpmalloc_ਢੇਰ_global_cache_ਕੱਢੋ(ਢੇਰ_t *ਢੇਰ_2,
                                                   size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
#ਜੇ ENABLE_GLOBAL_CACHE
#ਜੇ ENABLE_THREAD_CACHE
  ਵਿਸਤਾਰ_cache_t *ਵਿਸਤਾਰ_cache;
  size_t wanted_ਗਿਣਤੀ;
  ਜੇ (ਵਿਸਤਾਰ_ਗਿਣਤੀ == 1) {
    ਵਿਸਤਾਰ_cache = &ਢੇਰ_2->ਵਿਸਤਾਰ_cache;
    wanted_ਗਿਣਤੀ = THREAD_ਵਿਸਤਾਰ_CACHE_TRANSFER;
  } ਨਹੀਂਤਾਂ {
    ਵਿਸਤਾਰ_cache = (ਵਿਸਤਾਰ_cache_t *)(ਢੇਰ_2->ਵਿਸਤਾਰ_large_cache + (ਵਿਸਤਾਰ_ਗਿਣਤੀ - 2));
    wanted_ਗਿਣਤੀ = THREAD_ਵਿਸਤਾਰ_LARGE_CACHE_TRANSFER;
  }
  ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 = rpmalloc_global_cache_ਕੱਢੋ_spans(
      ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਗਿਣਤੀ, wanted_ਗਿਣਤੀ);
  ਜੇ (ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2) {
    _rpmalloc_stat_add64(&ਢੇਰ_2->global_ਮੰਜ਼ਿਲ_thread,
                         ਵਿਸਤਾਰ_ਗਿਣਤੀ * ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
    rpmalloc_stat_ਜੋੜੋ(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1].spans_from_global,
                       ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2);
    ਵਾਪਸ ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ[--ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2];
  }
#ਨਹੀਂਤਾਂ
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = 0;
  size_t ਗਿਣਤੀ_2 = rpmalloc_global_cache_ਕੱਢੋ_spans(&ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਗਿਣਤੀ, 1);
  ਜੇ (ਗਿਣਤੀ_2) {
    _rpmalloc_stat_add64(&ਢੇਰ_2->global_ਮੰਜ਼ਿਲ_thread,
                         ਵਿਸਤਾਰ_ਗਿਣਤੀ * ਗਿਣਤੀ_2 * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
    rpmalloc_stat_ਜੋੜੋ(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1].spans_from_global,
                       ਗਿਣਤੀ_2);
    ਵਾਪਸ ਵਿਸਤਾਰ;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
  (ਖਾਲੀ)ਆਕਾਰ(ਢੇਰ_2);
  (ਖਾਲੀ)ਆਕਾਰ(ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_inc_ਵਿਸਤਾਰ_statistics(ਢੇਰ_t *ਢੇਰ_2, size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ,
                                          uint32_t class_idx) {
  (ਖਾਲੀ)ਆਕਾਰ(ਢੇਰ_2);
  (ਖਾਲੀ)ਆਕਾਰ(ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  (ਖਾਲੀ)ਆਕਾਰ(class_idx);
#ਜੇ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  uint32_t idx = (uint32_t)ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1;
  uint32_t current_ਗਿਣਤੀ =
      (uint32_t)atomic_incr32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[idx].current);
  ਜੇ (current_ਗਿਣਤੀ > (uint32_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[idx].ਉੱਚਾ))
    atomic_store32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[idx].ਉੱਚਾ, (int32_t)current_ਗਿਣਤੀ);
  rpmalloc_stat_ਜੋੜੋ_peak(&ਢੇਰ_2->ਮਾਪ_class_use[class_idx].spans_current, 1,
                          ਢੇਰ_2->ਮਾਪ_class_use[class_idx].spans_peak);
#ਜੇ_ਅੰਤ
}

//! Get a span from one of the cache levels (thread cache, reserved, global
//! cache) or fallback to mapping more memory
ਸਥਿਰ ਵਿਸਤਾਰ_t *
rpmalloc_ਢੇਰ_ਕੱਢੋ_new_ਵਿਸਤਾਰ(ਢੇਰ_t *ਢੇਰ_2,
                                ਢੇਰ_ਮਾਪ_class_t *ਢੇਰ_ਮਾਪ_class,
                                size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ, uint32_t class_idx) {
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ;
#ਜੇ ENABLE_THREAD_CACHE
  ਜੇ (ਢੇਰ_ਮਾਪ_class && ਢੇਰ_ਮਾਪ_class->cache) {
    ਵਿਸਤਾਰ = ਢੇਰ_ਮਾਪ_class->cache;
    ਢੇਰ_ਮਾਪ_class->cache =
        (ਢੇਰ_2->ਵਿਸਤਾਰ_cache.ਗਿਣਤੀ_2
             ? ਢੇਰ_2->ਵਿਸਤਾਰ_cache.ਵਿਸਤਾਰ[--ਢੇਰ_2->ਵਿਸਤਾਰ_cache.ਗਿਣਤੀ_2]
             : 0);
    rpmalloc_inc_ਵਿਸਤਾਰ_statistics(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ, class_idx);
    ਵਾਪਸ ਵਿਸਤਾਰ;
  }
#ਜੇ_ਅੰਤ
  (ਖਾਲੀ)ਆਕਾਰ(class_idx);
  // Allow 50% overhead to increase cache hits
  size_t ਅਧਾਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ = ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  size_t ਹੱਦਬੰਦੀ_ਵਿਸਤਾਰ_ਗਿਣਤੀ =
      (ਵਿਸਤਾਰ_ਗਿਣਤੀ > 2) ? (ਵਿਸਤਾਰ_ਗਿਣਤੀ + (ਵਿਸਤਾਰ_ਗਿਣਤੀ >> 1)) : ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  ਜੇ (ਹੱਦਬੰਦੀ_ਵਿਸਤਾਰ_ਗਿਣਤੀ > LARGE_CLASS_ਗਿਣਤੀ)
    ਹੱਦਬੰਦੀ_ਵਿਸਤਾਰ_ਗਿਣਤੀ = LARGE_CLASS_ਗਿਣਤੀ;
  ਕਰੋ {
    ਵਿਸਤਾਰ = rpmalloc_ਢੇਰ_thread_cache_ਕੱਢੋ(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ);
    ਜੇ (EXPECTED(ਵਿਸਤਾਰ != 0)) {
      _rpmalloc_stat_inc(&ਢੇਰ_2->ਮਾਪ_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_ਵਿਸਤਾਰ_statistics(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ, class_idx);
      ਵਾਪਸ ਵਿਸਤਾਰ;
    }
    ਵਿਸਤਾਰ = rpmalloc_ਢੇਰ_thread_cache_deferred_ਕੱਢੋ(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ);
    ਜੇ (EXPECTED(ਵਿਸਤਾਰ != 0)) {
      _rpmalloc_stat_inc(&ਢੇਰ_2->ਮਾਪ_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_ਵਿਸਤਾਰ_statistics(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ, class_idx);
      ਵਾਪਸ ਵਿਸਤਾਰ;
    }
    ਵਿਸਤਾਰ = rpmalloc_ਢੇਰ_global_cache_ਕੱਢੋ(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ);
    ਜੇ (EXPECTED(ਵਿਸਤਾਰ != 0)) {
      _rpmalloc_stat_inc(&ਢੇਰ_2->ਮਾਪ_class_use[class_idx].spans_from_cache);
      rpmalloc_inc_ਵਿਸਤਾਰ_statistics(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ, class_idx);
      ਵਾਪਸ ਵਿਸਤਾਰ;
    }
    ਵਿਸਤਾਰ = rpmalloc_ਢੇਰ_reserved_ਕੱਢੋ(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ);
    ਜੇ (EXPECTED(ਵਿਸਤਾਰ != 0)) {
      _rpmalloc_stat_inc(&ਢੇਰ_2->ਮਾਪ_class_use[class_idx].spans_from_reserved);
      rpmalloc_inc_ਵਿਸਤਾਰ_statistics(ਢੇਰ_2, ਵਿਸਤਾਰ_ਗਿਣਤੀ, class_idx);
      ਵਾਪਸ ਵਿਸਤਾਰ;
    }
    ++ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  } ਜਦੋਂ (ਵਿਸਤਾਰ_ਗਿਣਤੀ <= ਹੱਦਬੰਦੀ_ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  // Final fallback, map in more virtual memory
  ਵਿਸਤਾਰ = rpmalloc_ਵਿਸਤਾਰ_map(ਢੇਰ_2, ਅਧਾਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  rpmalloc_inc_ਵਿਸਤਾਰ_statistics(ਢੇਰ_2, ਅਧਾਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ, class_idx);
  _rpmalloc_stat_inc(&ਢੇਰ_2->ਮਾਪ_class_use[class_idx].spans_map_calls);
  ਵਾਪਸ ਵਿਸਤਾਰ;
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_initialize(ਢੇਰ_t *ਢੇਰ_2) {
  _rpmalloc_memset_const(ਢੇਰ_2, 0, ਆਕਾਰ(ਢੇਰ_t));
  // Get a new heap ID
  ਢੇਰ_2->id = 1 + atomic_incr32(&ਸਮ੍ਰਿਤੀ_ਢੇਰ_id);

  // Link in heap in heap ID map
  size_t list_idx = (size_t)ਢੇਰ_2->id % ਢੇਰ_ARRAY_ਮਾਪ;
  ਢੇਰ_2->ਅਗਲਾ_ਢੇਰ = ਸਮ੍ਰਿਤੀ_heaps[list_idx];
  ਸਮ੍ਰਿਤੀ_heaps[list_idx] = ਢੇਰ_2;
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_orphan(ਢੇਰ_t *ਢੇਰ_2, ਪੂਰਨ ਪਹਿਲਾ_class) {
  ਢੇਰ_2->owner_thread = (uintptr_t)-1;
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
  ਢੇਰ_t **ਢੇਰ_list =
      (ਪਹਿਲਾ_class ? &ਸਮ੍ਰਿਤੀ_ਪਹਿਲਾ_class_orphan_heaps : &ਸਮ੍ਰਿਤੀ_orphan_heaps);
#ਨਹੀਂਤਾਂ
  (ਖਾਲੀ)ਆਕਾਰ(ਪਹਿਲਾ_class);
  ਢੇਰ_t **ਢੇਰ_list = &ਸਮ੍ਰਿਤੀ_orphan_heaps;
#ਜੇ_ਅੰਤ
  ਢੇਰ_2->ਅਗਲਾ_orphan = *ਢੇਰ_list;
  *ਢੇਰ_list = ਢੇਰ_2;
}

//! Allocate a new heap from newly mapped memory pages
ਸਥਿਰ ਢੇਰ_t *rpmalloc_ਢੇਰ_allocate_new(ਖਾਲੀ) {
  // Map in pages for a 16 heaps. If page size is greater than required size for
  // this, map a page and use first part for heaps and remaining part for spans
  // for allocations. Adds a lot of complexity, but saves a lot of memory on
  // systems where page size > 64 spans (4MiB)
  size_t ਢੇਰ_ਮਾਪ_2 = ਆਕਾਰ(ਢੇਰ_t);
  size_t aligned_ਢੇਰ_ਮਾਪ = 16 * ((ਢੇਰ_ਮਾਪ_2 + 15) / 16);
  size_t request_ਢੇਰ_ਗਿਣਤੀ = 16;
  size_t ਢੇਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ = ((aligned_ਢੇਰ_ਮਾਪ * request_ਢੇਰ_ਗਿਣਤੀ) +
                            ਆਕਾਰ(ਵਿਸਤਾਰ_t) + ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ - 1) /
                           ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ;
  size_t ਖੰਡ_ਮਾਪ_2 = ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ * ਢੇਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ = ਢੇਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = 0;
  // If there are global reserved spans, use these first
  ਜੇ (ਸਮ੍ਰਿਤੀ_global_reserve_ਗਿਣਤੀ >= ਢੇਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
    ਵਿਸਤਾਰ = _rpmalloc_global_get_reserved_spans(ਢੇਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  }
  ਜੇ (!ਵਿਸਤਾਰ) {
    ਜੇ (ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ > ਖੰਡ_ਮਾਪ_2) {
      ਵਿਸਤਾਰ_ਗਿਣਤੀ = ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ / ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ;
      ਖੰਡ_ਮਾਪ_2 = ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
      // If using huge pages, make sure to grab enough heaps to avoid
      // reallocating a huge page just to serve new heaps
      size_t possible_ਢੇਰ_ਗਿਣਤੀ =
          (ਖੰਡ_ਮਾਪ_2 - ਆਕਾਰ(ਵਿਸਤਾਰ_t)) / aligned_ਢੇਰ_ਮਾਪ;
      ਜੇ (possible_ਢੇਰ_ਗਿਣਤੀ >= (request_ਢੇਰ_ਗਿਣਤੀ * 16))
        request_ਢੇਰ_ਗਿਣਤੀ *= 16;
      ਨਹੀਂਤਾਂ ਜੇ (possible_ਢੇਰ_ਗਿਣਤੀ < request_ਢੇਰ_ਗਿਣਤੀ)
        request_ਢੇਰ_ਗਿਣਤੀ = possible_ਢੇਰ_ਗਿਣਤੀ;
      ਢੇਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ = ((aligned_ਢੇਰ_ਮਾਪ * request_ਢੇਰ_ਗਿਣਤੀ) +
                         ਆਕਾਰ(ਵਿਸਤਾਰ_t) + ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ - 1) /
                        ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ;
    }

    size_t align_ਵਿੱਥ = 0;
    ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)_rpmalloc_mmap(ਖੰਡ_ਮਾਪ_2, &align_ਵਿੱਥ);
    ਜੇ (!ਵਿਸਤਾਰ)
      ਵਾਪਸ 0;

    // Master span will contain the heaps
    _rpmalloc_stat_inc(&_master_spans);
    rpmalloc_ਵਿਸਤਾਰ_initialize(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਗਿਣਤੀ, ਢੇਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ, align_ਵਿੱਥ);
  }

  size_t remain_ਮਾਪ = ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ - ਆਕਾਰ(ਵਿਸਤਾਰ_t);
  ਢੇਰ_t *ਢੇਰ_2 = (ਢੇਰ_t *)ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਆਕਾਰ(ਵਿਸਤਾਰ_t));
  rpmalloc_ਢੇਰ_initialize(ਢੇਰ_2);

  // Put extra heaps as orphans
  size_t ਅੰਕ_heaps = remain_ਮਾਪ / aligned_ਢੇਰ_ਮਾਪ;
  ਜੇ (ਅੰਕ_heaps < request_ਢੇਰ_ਗਿਣਤੀ)
    ਅੰਕ_heaps = request_ਢੇਰ_ਗਿਣਤੀ;
  atomic_store32(&ਢੇਰ_2->ਬੱਚਾ_ਗਿਣਤੀ, (int32_t)ਅੰਕ_heaps - 1);
  ਢੇਰ_t *extra_ਢੇਰ = (ਢੇਰ_t *)ਸੂਚਕ_ਵਿੱਥ(ਢੇਰ_2, aligned_ਢੇਰ_ਮਾਪ);
  ਜਦੋਂ (ਅੰਕ_heaps > 1) {
    rpmalloc_ਢੇਰ_initialize(extra_ਢੇਰ);
    extra_ਢੇਰ->master_ਢੇਰ = ਢੇਰ_2;
    rpmalloc_ਢੇਰ_orphan(extra_ਢੇਰ, 1);
    extra_ਢੇਰ = (ਢੇਰ_t *)ਸੂਚਕ_ਵਿੱਥ(extra_ਢੇਰ, aligned_ਢੇਰ_ਮਾਪ);
    --ਅੰਕ_heaps;
  }

  ਜੇ (ਵਿਸਤਾਰ_ਗਿਣਤੀ > ਢੇਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ) {
    // Cap reserved spans
    size_t remain_ਗਿਣਤੀ = ਵਿਸਤਾਰ_ਗਿਣਤੀ - ਢੇਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
    size_t reserve_ਗਿਣਤੀ =
        (remain_ਗਿਣਤੀ > ਸਮ੍ਰਿਤੀ_ਢੇਰ_reserve_ਗਿਣਤੀ ? ਸਮ੍ਰਿਤੀ_ਢੇਰ_reserve_ਗਿਣਤੀ
                                                   : remain_ਗਿਣਤੀ);
    ਵਿਸਤਾਰ_t *remain_ਵਿਸਤਾਰ =
        (ਵਿਸਤਾਰ_t *)ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਢੇਰ_ਵਿਸਤਾਰ_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
    rpmalloc_ਢੇਰ_ਸੈੱਟ_reserved_spans(ਢੇਰ_2, ਵਿਸਤਾਰ, remain_ਵਿਸਤਾਰ, reserve_ਗਿਣਤੀ);

    ਜੇ (remain_ਗਿਣਤੀ > reserve_ਗਿਣਤੀ) {
      // Set to global reserved spans
      remain_ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)ਸੂਚਕ_ਵਿੱਥ(remain_ਵਿਸਤਾਰ,
                                             reserve_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
      reserve_ਗਿਣਤੀ = remain_ਗਿਣਤੀ - reserve_ਗਿਣਤੀ;
      rpmalloc_global_ਸੈੱਟ_reserved_spans(ਵਿਸਤਾਰ, remain_ਵਿਸਤਾਰ, reserve_ਗਿਣਤੀ);
    }
  }

  ਵਾਪਸ ਢੇਰ_2;
}

ਸਥਿਰ ਢੇਰ_t *rpmalloc_ਢੇਰ_ਕੱਢੋ_orphan(ਢੇਰ_t **ਢੇਰ_list) {
  ਢੇਰ_t *ਢੇਰ_2 = *ਢੇਰ_list;
  *ਢੇਰ_list = (ਢੇਰ_2 ? ਢੇਰ_2->ਅਗਲਾ_orphan : 0);
  ਵਾਪਸ ਢੇਰ_2;
}

//! Allocate a new heap, potentially reusing a previously orphaned heap
ਸਥਿਰ ਢੇਰ_t *rpmalloc_ਢੇਰ_allocate(ਪੂਰਨ ਪਹਿਲਾ_class) {
  ਢੇਰ_t *ਢੇਰ_2 = 0;
  ਜਦੋਂ (!atomic_cas32_acquire(&ਸਮ੍ਰਿਤੀ_global_ਤਾਲਾ, 1, 0))
    _rpmalloc_spin();
  ਜੇ (ਪਹਿਲਾ_class == 0)
    ਢੇਰ_2 = rpmalloc_ਢੇਰ_ਕੱਢੋ_orphan(&ਸਮ੍ਰਿਤੀ_orphan_heaps);
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
  ਜੇ (!ਢੇਰ_2)
    ਢੇਰ_2 = rpmalloc_ਢੇਰ_ਕੱਢੋ_orphan(&ਸਮ੍ਰਿਤੀ_ਪਹਿਲਾ_class_orphan_heaps);
#ਜੇ_ਅੰਤ
  ਜੇ (!ਢੇਰ_2)
    ਢੇਰ_2 = rpmalloc_ਢੇਰ_allocate_new();
  atomic_store32_release(&ਸਮ੍ਰਿਤੀ_global_ਤਾਲਾ, 0);
  ਜੇ (ਢੇਰ_2)
    rpmalloc_ਢੇਰ_cache_adopt_deferred(ਢੇਰ_2, 0);
  ਵਾਪਸ ਢੇਰ_2;
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_release(ਖਾਲੀ *heapptr, ਪੂਰਨ ਪਹਿਲਾ_class,
                                   ਪੂਰਨ release_cache) {
  ਢੇਰ_t *ਢੇਰ_2 = (ਢੇਰ_t *)heapptr;
  ਜੇ (!ਢੇਰ_2)
    ਵਾਪਸ;
  // Release thread cache spans back to global cache
  rpmalloc_ਢੇਰ_cache_adopt_deferred(ਢੇਰ_2, 0);
  ਜੇ (release_cache || ਢੇਰ_2->finalize) {
#ਜੇ ENABLE_THREAD_CACHE
    ਲਈ (size_t iclass = 0; iclass < LARGE_CLASS_ਗਿਣਤੀ; ++iclass) {
      ਵਿਸਤਾਰ_cache_t *ਵਿਸਤਾਰ_cache;
      ਜੇ (!iclass)
        ਵਿਸਤਾਰ_cache = &ਢੇਰ_2->ਵਿਸਤਾਰ_cache;
      ਨਹੀਂਤਾਂ
        ਵਿਸਤਾਰ_cache = (ਵਿਸਤਾਰ_cache_t *)(ਢੇਰ_2->ਵਿਸਤਾਰ_large_cache + (iclass - 1));
      ਜੇ (!ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2)
        ਜਾਰੀ;
#ਜੇ ENABLE_GLOBAL_CACHE
      ਜੇ (ਢੇਰ_2->finalize) {
        ਲਈ (size_t ispan = 0; ispan < ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2; ++ispan)
          rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ[ispan]);
      } ਨਹੀਂਤਾਂ {
        _rpmalloc_stat_add64(&ਢੇਰ_2->thread_ਮੰਜ਼ਿਲ_global, ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 *
                                                          (iclass + 1) *
                                                          ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
        rpmalloc_stat_ਜੋੜੋ(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_ਮੰਜ਼ਿਲ_global,
                           ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2);
        rpmalloc_global_cache_ਪਾਓ_spans(ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ, iclass + 1,
                                            ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2);
      }
#ਨਹੀਂਤਾਂ
      ਲਈ (size_t ispan = 0; ispan < ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2; ++ispan)
        rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ[ispan]);
#ਜੇ_ਅੰਤ
      ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 = 0;
    }
#ਜੇ_ਅੰਤ
  }

  ਜੇ (get_thread_ਢੇਰ_raw() == ਢੇਰ_2)
    ਸੈੱਟ_thread_ਢੇਰ(0);

#ਜੇ ENABLE_STATISTICS
  atomic_decr32(&ਸਮ੍ਰਿਤੀ_ਸਰਗਰਮ_heaps);
  rpmalloc_assert(atomic_load32(&ਸਮ੍ਰਿਤੀ_ਸਰਗਰਮ_heaps) >= 0,
                  "Still active heaps during finalization");
#ਜੇ_ਅੰਤ

  // If we are forcibly terminating with _exit the state of the
  // lock atomic is unknown and it's best to just go ahead and exit
  ਜੇ (get_thread_id() != _rpmalloc_main_thread_id) {
    ਜਦੋਂ (!atomic_cas32_acquire(&ਸਮ੍ਰਿਤੀ_global_ਤਾਲਾ, 1, 0))
      _rpmalloc_spin();
  }
  rpmalloc_ਢੇਰ_orphan(ਢੇਰ_2, ਪਹਿਲਾ_class);
  atomic_store32_release(&ਸਮ੍ਰਿਤੀ_global_ਤਾਲਾ, 0);
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_release_raw(ਖਾਲੀ *heapptr, ਪੂਰਨ release_cache) {
  rpmalloc_ਢੇਰ_release(heapptr, 0, release_cache);
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_release_raw_fc(ਖਾਲੀ *heapptr) {
  rpmalloc_ਢੇਰ_release_raw(heapptr, 1);
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_ਢੇਰ_finalize(ਢੇਰ_t *ਢੇਰ_2) {
  ਜੇ (ਢੇਰ_2->spans_reserved) {
    ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = rpmalloc_ਵਿਸਤਾਰ_map(ਢੇਰ_2, ਢੇਰ_2->spans_reserved);
    rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ);
    ਢੇਰ_2->spans_reserved = 0;
  }

  rpmalloc_ਢੇਰ_cache_adopt_deferred(ਢੇਰ_2, 0);

  ਲਈ (size_t iclass = 0; iclass < ਮਾਪ_CLASS_ਗਿਣਤੀ; ++iclass) {
    ਜੇ (ਢੇਰ_2->ਮਾਪ_class[iclass].cache)
      rpmalloc_ਵਿਸਤਾਰ_unmap(ਢੇਰ_2->ਮਾਪ_class[iclass].cache);
    ਢੇਰ_2->ਮਾਪ_class[iclass].cache = 0;
    ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = ਢੇਰ_2->ਮਾਪ_class[iclass].partial_ਵਿਸਤਾਰ;
    ਜਦੋਂ (ਵਿਸਤਾਰ) {
      ਵਿਸਤਾਰ_t *ਅਗਲਾ = ਵਿਸਤਾਰ->ਅਗਲਾ;
      rpmalloc_ਵਿਸਤਾਰ_finalize(ਢੇਰ_2, iclass, ਵਿਸਤਾਰ,
                              &ਢੇਰ_2->ਮਾਪ_class[iclass].partial_ਵਿਸਤਾਰ);
      ਵਿਸਤਾਰ = ਅਗਲਾ;
    }
    // If class still has a free list it must be a full span
    ਜੇ (ਢੇਰ_2->ਮਾਪ_class[iclass].free_list) {
      ਵਿਸਤਾਰ_t *class_ਵਿਸਤਾਰ =
          (ਵਿਸਤਾਰ_t *)((uintptr_t)ਢੇਰ_2->ਮਾਪ_class[iclass].free_list &
                     ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ);
      ਵਿਸਤਾਰ_t **list = 0;
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
      list = &ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ[iclass];
#ਜੇ_ਅੰਤ
      --ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
      ਜੇ (!rpmalloc_ਵਿਸਤਾਰ_finalize(ਢੇਰ_2, iclass, class_ਵਿਸਤਾਰ, list)) {
        ਜੇ (list)
          rpmalloc_ਵਿਸਤਾਰ_double_link_list_remove(list, class_ਵਿਸਤਾਰ);
        rpmalloc_ਵਿਸਤਾਰ_double_link_list_ਜੋੜੋ(
            &ਢੇਰ_2->ਮਾਪ_class[iclass].partial_ਵਿਸਤਾਰ, class_ਵਿਸਤਾਰ);
      }
    }
  }

#ਜੇ ENABLE_THREAD_CACHE
  ਲਈ (size_t iclass = 0; iclass < LARGE_CLASS_ਗਿਣਤੀ; ++iclass) {
    ਵਿਸਤਾਰ_cache_t *ਵਿਸਤਾਰ_cache;
    ਜੇ (!iclass)
      ਵਿਸਤਾਰ_cache = &ਢੇਰ_2->ਵਿਸਤਾਰ_cache;
    ਨਹੀਂਤਾਂ
      ਵਿਸਤਾਰ_cache = (ਵਿਸਤਾਰ_cache_t *)(ਢੇਰ_2->ਵਿਸਤਾਰ_large_cache + (iclass - 1));
    ਲਈ (size_t ispan = 0; ispan < ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2; ++ispan)
      rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ[ispan]);
    ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 = 0;
  }
#ਜੇ_ਅੰਤ
  rpmalloc_assert(!atomic_ਲੱਦੋ_ptr(&ਢੇਰ_2->ਵਿਸਤਾਰ_free_deferred),
                  "Heaps still active during finalization");
}

////////////
///
/// Allocation entry points
///
//////

//! Pop first block from a free list
ਸਥਿਰ ਖਾਲੀ *free_list_ਕੱਢੋ(ਖਾਲੀ **list) {
  ਖਾਲੀ *ਖੰਡ = *list;
  *list = *((ਖਾਲੀ **)ਖੰਡ);
  ਵਾਪਸ ਖੰਡ;
}

//! Allocate a small/medium sized memory block from the given heap
ਸਥਿਰ ਖਾਲੀ *rpmalloc_allocate_from_ਢੇਰ_fallback(
    ਢੇਰ_t *ਢੇਰ_2, ਢੇਰ_ਮਾਪ_class_t *ਢੇਰ_ਮਾਪ_class, uint32_t class_idx) {
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = ਢੇਰ_ਮਾਪ_class->partial_ਵਿਸਤਾਰ;
  rpmalloc_assume(ਢੇਰ_2 != 0);
  ਜੇ (EXPECTED(ਵਿਸਤਾਰ != 0)) {
    rpmalloc_assert(ਵਿਸਤਾਰ->ਖੰਡ_ਗਿਣਤੀ ==
                        ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[ਵਿਸਤਾਰ->ਮਾਪ_class].ਖੰਡ_ਗਿਣਤੀ,
                    "Span block count corrupted");
    rpmalloc_assert(!rpmalloc_ਵਿਸਤਾਰ_is_fully_utilized(ਵਿਸਤਾਰ),
                    "Internal failure");
    ਖਾਲੀ *ਖੰਡ;
    ਜੇ (ਵਿਸਤਾਰ->free_list) {
      // Span local free list is not empty, swap to size class free list
      ਖੰਡ = free_list_ਕੱਢੋ(&ਵਿਸਤਾਰ->free_list);
      ਢੇਰ_ਮਾਪ_class->free_list = ਵਿਸਤਾਰ->free_list;
      ਵਿਸਤਾਰ->free_list = 0;
    } ਨਹੀਂਤਾਂ {
      // If the span did not fully initialize free list, link up another page
      // worth of blocks
      ਖਾਲੀ *ਖੰਡ_ਸ਼ੁਰੂ = ਸੂਚਕ_ਵਿੱਥ(
          ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ +
                    ((size_t)ਵਿਸਤਾਰ->free_list_ਹੱਦਬੰਦੀ * ਵਿਸਤਾਰ->ਖੰਡ_ਮਾਪ_2));
      ਵਿਸਤਾਰ->free_list_ਹੱਦਬੰਦੀ += free_list_partial_init(
          &ਢੇਰ_ਮਾਪ_class->free_list, &ਖੰਡ,
          (ਖਾਲੀ *)((uintptr_t)ਖੰਡ_ਸ਼ੁਰੂ & ~(ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ - 1)),
          ਖੰਡ_ਸ਼ੁਰੂ, ਵਿਸਤਾਰ->ਖੰਡ_ਗਿਣਤੀ - ਵਿਸਤਾਰ->free_list_ਹੱਦਬੰਦੀ,
          ਵਿਸਤਾਰ->ਖੰਡ_ਮਾਪ_2);
    }
    rpmalloc_assert(ਵਿਸਤਾਰ->free_list_ਹੱਦਬੰਦੀ <= ਵਿਸਤਾਰ->ਖੰਡ_ਗਿਣਤੀ,
                    "Span block count corrupted");
    ਵਿਸਤਾਰ->ਵਰਤਿਆ_ਗਿਣਤੀ = ਵਿਸਤਾਰ->free_list_ਹੱਦਬੰਦੀ;

    // Swap in deferred free list if present
    ਜੇ (atomic_ਲੱਦੋ_ptr(&ਵਿਸਤਾਰ->free_list_deferred))
      rpmalloc_ਵਿਸਤਾਰ_ਕੱਢੋ_free_list_deferred(ਵਿਸਤਾਰ);

    // If span is still not fully utilized keep it in partial list and early
    // return block
    ਜੇ (!rpmalloc_ਵਿਸਤਾਰ_is_fully_utilized(ਵਿਸਤਾਰ))
      ਵਾਪਸ ਖੰਡ;

    // The span is fully utilized, unlink from partial list and add to fully
    // utilized list
    rpmalloc_ਵਿਸਤਾਰ_double_link_list_ਕੱਢੋ_ਸਿਰ(&ਢੇਰ_ਮਾਪ_class->partial_ਵਿਸਤਾਰ,
                                             ਵਿਸਤਾਰ);
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
    rpmalloc_ਵਿਸਤਾਰ_double_link_list_ਜੋੜੋ(&ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ[class_idx], ਵਿਸਤਾਰ);
#ਜੇ_ਅੰਤ
    ++ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
    ਵਾਪਸ ਖੰਡ;
  }

  // Find a span in one of the cache levels
  ਵਿਸਤਾਰ = rpmalloc_ਢੇਰ_ਕੱਢੋ_new_ਵਿਸਤਾਰ(ਢੇਰ_2, ਢੇਰ_ਮਾਪ_class, 1, class_idx);
  ਜੇ (EXPECTED(ਵਿਸਤਾਰ != 0)) {
    // Mark span as owned by this heap and set base data, return first block
    ਵਾਪਸ rpmalloc_ਵਿਸਤਾਰ_initialize_new(ਢੇਰ_2, ਢੇਰ_ਮਾਪ_class, ਵਿਸਤਾਰ,
                                         class_idx);
  }

  ਵਾਪਸ 0;
}

//! Allocate a small sized memory block from the given heap
ਸਥਿਰ ਖਾਲੀ *_rpmalloc_allocate_small(ਢੇਰ_t *ਢੇਰ_2, size_t ਮਾਪ_2) {
  rpmalloc_assert(ਢੇਰ_2, "No thread heap");
  // Small sizes have unique size classes
  ਅਚਲ uint32_t class_idx =
      (uint32_t)((ਮਾਪ_2 + (SMALL_GRANULARITY - 1)) >> SMALL_GRANULARITY_ਖਿਸਕਾਓ);
  ਢੇਰ_ਮਾਪ_class_t *ਢੇਰ_ਮਾਪ_class = ਢੇਰ_2->ਮਾਪ_class + class_idx;
  _rpmalloc_stat_inc_alloc(ਢੇਰ_2, class_idx);
  ਜੇ (EXPECTED(ਢੇਰ_ਮਾਪ_class->free_list != 0))
    ਵਾਪਸ free_list_ਕੱਢੋ(&ਢੇਰ_ਮਾਪ_class->free_list);
  ਵਾਪਸ rpmalloc_allocate_from_ਢੇਰ_fallback(ਢੇਰ_2, ਢੇਰ_ਮਾਪ_class,
                                               class_idx);
}

//! Allocate a medium sized memory block from the given heap
ਸਥਿਰ ਖਾਲੀ *_rpmalloc_allocate_medium(ਢੇਰ_t *ਢੇਰ_2, size_t ਮਾਪ_2) {
  rpmalloc_assert(ਢੇਰ_2, "No thread heap");
  // Calculate the size class index and do a dependent lookup of the final class
  // index (in case of merged classes)
  ਅਚਲ uint32_t ਅਧਾਰ_idx =
      (uint32_t)(SMALL_CLASS_ਗਿਣਤੀ +
                 ((ਮਾਪ_2 - (SMALL_ਮਾਪ_ਹੱਦਬੰਦੀ + 1)) >> MEDIUM_GRANULARITY_ਖਿਸਕਾਓ));
  ਅਚਲ uint32_t class_idx = ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[ਅਧਾਰ_idx].class_idx;
  ਢੇਰ_ਮਾਪ_class_t *ਢੇਰ_ਮਾਪ_class = ਢੇਰ_2->ਮਾਪ_class + class_idx;
  _rpmalloc_stat_inc_alloc(ਢੇਰ_2, class_idx);
  ਜੇ (EXPECTED(ਢੇਰ_ਮਾਪ_class->free_list != 0))
    ਵਾਪਸ free_list_ਕੱਢੋ(&ਢੇਰ_ਮਾਪ_class->free_list);
  ਵਾਪਸ rpmalloc_allocate_from_ਢੇਰ_fallback(ਢੇਰ_2, ਢੇਰ_ਮਾਪ_class,
                                               class_idx);
}

//! Allocate a large sized memory block from the given heap
ਸਥਿਰ ਖਾਲੀ *_rpmalloc_allocate_large(ਢੇਰ_t *ਢੇਰ_2, size_t ਮਾਪ_2) {
  rpmalloc_assert(ਢੇਰ_2, "No thread heap");
  // Calculate number of needed max sized spans (including header)
  // Since this function is never called if size > LARGE_SIZE_LIMIT
  // the span_count is guaranteed to be <= LARGE_CLASS_COUNT
  ਮਾਪ_2 += ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ;
  size_t ਵਿਸਤਾਰ_ਗਿਣਤੀ = ਮਾਪ_2 >> ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ;
  ਜੇ (ਮਾਪ_2 & (ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ - 1))
    ++ਵਿਸਤਾਰ_ਗਿਣਤੀ;

  // Find a span in one of the cache levels
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ =
      rpmalloc_ਢੇਰ_ਕੱਢੋ_new_ਵਿਸਤਾਰ(ਢੇਰ_2, 0, ਵਿਸਤਾਰ_ਗਿਣਤੀ, ਮਾਪ_CLASS_LARGE);
  ਜੇ (!ਵਿਸਤਾਰ)
    ਵਾਪਸ ਵਿਸਤਾਰ;

  // Mark span as owned by this heap and set base data
  rpmalloc_assert(ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ >= ਵਿਸਤਾਰ_ਗਿਣਤੀ, "Internal failure");
  ਵਿਸਤਾਰ->ਮਾਪ_class = ਮਾਪ_CLASS_LARGE;
  ਵਿਸਤਾਰ->ਢੇਰ_2 = ਢੇਰ_2;

#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
  rpmalloc_ਵਿਸਤਾਰ_double_link_list_ਜੋੜੋ(&ਢੇਰ_2->large_huge_ਵਿਸਤਾਰ, ਵਿਸਤਾਰ);
#ਜੇ_ਅੰਤ
  ++ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;

  ਵਾਪਸ ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ);
}

//! Allocate a huge block by mapping memory pages directly
ਸਥਿਰ ਖਾਲੀ *_rpmalloc_allocate_huge(ਢੇਰ_t *ਢੇਰ_2, size_t ਮਾਪ_2) {
  rpmalloc_assert(ਢੇਰ_2, "No thread heap");
  rpmalloc_ਢੇਰ_cache_adopt_deferred(ਢੇਰ_2, 0);
  ਮਾਪ_2 += ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ;
  size_t ਅੰਕ_ਪੰਨੇ = ਮਾਪ_2 >> ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ_ਖਿਸਕਾਓ;
  ਜੇ (ਮਾਪ_2 & (ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ - 1))
    ++ਅੰਕ_ਪੰਨੇ;
  size_t align_ਵਿੱਥ = 0;
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ =
      (ਵਿਸਤਾਰ_t *)_rpmalloc_mmap(ਅੰਕ_ਪੰਨੇ * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ, &align_ਵਿੱਥ);
  ਜੇ (!ਵਿਸਤਾਰ)
    ਵਾਪਸ ਵਿਸਤਾਰ;

  // Store page count in span_count
  ਵਿਸਤਾਰ->ਮਾਪ_class = ਮਾਪ_CLASS_HUGE;
  ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ = (uint32_t)ਅੰਕ_ਪੰਨੇ;
  ਵਿਸਤਾਰ->align_ਵਿੱਥ = (uint32_t)align_ਵਿੱਥ;
  ਵਿਸਤਾਰ->ਢੇਰ_2 = ਢੇਰ_2;
  rpmalloc_stat_ਜੋੜੋ_peak(&huge_ਪੰਨੇ_current, ਅੰਕ_ਪੰਨੇ, huge_ਪੰਨੇ_peak);

#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
  rpmalloc_ਵਿਸਤਾਰ_double_link_list_ਜੋੜੋ(&ਢੇਰ_2->large_huge_ਵਿਸਤਾਰ, ਵਿਸਤਾਰ);
#ਜੇ_ਅੰਤ
  ++ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;

  ਵਾਪਸ ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ);
}

//! Allocate a block of the given size
ਸਥਿਰ ਖਾਲੀ *_rpmalloc_allocate(ਢੇਰ_t *ਢੇਰ_2, size_t ਮਾਪ_2) {
  _rpmalloc_stat_add64(&ਵੰਡ_counter, 1);
  ਜੇ (EXPECTED(ਮਾਪ_2 <= SMALL_ਮਾਪ_ਹੱਦਬੰਦੀ))
    ਵਾਪਸ _rpmalloc_allocate_small(ਢੇਰ_2, ਮਾਪ_2);
  ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ_2 <= ਸਮ੍ਰਿਤੀ_medium_ਮਾਪ_ਹੱਦਬੰਦੀ)
    ਵਾਪਸ _rpmalloc_allocate_medium(ਢੇਰ_2, ਮਾਪ_2);
  ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ_2 <= LARGE_ਮਾਪ_ਹੱਦਬੰਦੀ)
    ਵਾਪਸ _rpmalloc_allocate_large(ਢੇਰ_2, ਮਾਪ_2);
  ਵਾਪਸ _rpmalloc_allocate_huge(ਢੇਰ_2, ਮਾਪ_2);
}

ਸਥਿਰ ਖਾਲੀ *_rpmalloc_aligned_allocate(ਢੇਰ_t *ਢੇਰ_2, size_t alignment,
                                        size_t ਮਾਪ_2) {
  ਜੇ (alignment <= SMALL_GRANULARITY)
    ਵਾਪਸ _rpmalloc_allocate(ਢੇਰ_2, ਮਾਪ_2);

#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
  ਜੇ ((ਮਾਪ_2 + alignment) < ਮਾਪ_2) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
  ਜੇ (alignment & (alignment - 1)) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ

  ਜੇ ((alignment <= ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ) &&
      ((ਮਾਪ_2 + ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ) < ਸਮ੍ਰਿਤੀ_medium_ਮਾਪ_ਹੱਦਬੰਦੀ)) {
    // If alignment is less or equal to span header size (which is power of
    // two), and size aligned to span header size multiples is less than size +
    // alignment, then use natural alignment of blocks to provide alignment
    size_t ਗੁਣਜ_ਮਾਪ = ਮਾਪ_2 ? (ਮਾਪ_2 + (ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ - 1)) &
                                      ~(uintptr_t)(ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ - 1)
                                : ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ;
    rpmalloc_assert(!(ਗੁਣਜ_ਮਾਪ % ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ),
                    "Failed alignment calculation");
    ਜੇ (ਗੁਣਜ_ਮਾਪ <= (ਮਾਪ_2 + alignment))
      ਵਾਪਸ _rpmalloc_allocate(ਢੇਰ_2, ਗੁਣਜ_ਮਾਪ);
  }

  ਖਾਲੀ *ptr = 0;
  size_t align_ਨਕਾਬ = alignment - 1;
  ਜੇ (alignment <= ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ) {
    ptr = _rpmalloc_allocate(ਢੇਰ_2, ਮਾਪ_2 + alignment);
    ਜੇ ((uintptr_t)ptr & align_ਨਕਾਬ) {
      ptr = (ਖਾਲੀ *)(((uintptr_t)ptr & ~(uintptr_t)align_ਨਕਾਬ) + alignment);
      // Mark as having aligned blocks
      ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)((uintptr_t)ptr & ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ);
      ਵਿਸਤਾਰ->flags |= ਵਿਸਤਾਰ_FLAG_ALIGNED_ਖੰਡਾਂ;
    }
    ਵਾਪਸ ptr;
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
  ਜੇ (alignment & align_ਨਕਾਬ) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
  ਜੇ (alignment >= ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }

  size_t extra_ਪੰਨੇ = alignment / ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;

  // Since each span has a header, we will at least need one extra memory page
  size_t ਅੰਕ_ਪੰਨੇ = 1 + (ਮਾਪ_2 / ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ);
  ਜੇ (ਮਾਪ_2 & (ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ - 1))
    ++ਅੰਕ_ਪੰਨੇ;

  ਜੇ (extra_ਪੰਨੇ > ਅੰਕ_ਪੰਨੇ)
    ਅੰਕ_ਪੰਨੇ = 1 + extra_ਪੰਨੇ;

  size_t original_ਪੰਨੇ = ਅੰਕ_ਪੰਨੇ;
  size_t ਹੱਦਬੰਦੀ_ਪੰਨੇ = (ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ / ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ) * 2;
  ਜੇ (ਹੱਦਬੰਦੀ_ਪੰਨੇ < (original_ਪੰਨੇ * 2))
    ਹੱਦਬੰਦੀ_ਪੰਨੇ = original_ਪੰਨੇ * 2;

  size_t mapped_ਮਾਪ, align_ਵਿੱਥ;
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ;

retry:
  align_ਵਿੱਥ = 0;
  mapped_ਮਾਪ = ਅੰਕ_ਪੰਨੇ * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;

  ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)_rpmalloc_mmap(mapped_ਮਾਪ, &align_ਵਿੱਥ);
  ਜੇ (!ਵਿਸਤਾਰ) {
    errno = ENOMEM;
    ਵਾਪਸ 0;
  }
  ptr = ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ);

  ਜੇ ((uintptr_t)ptr & align_ਨਕਾਬ)
    ptr = (ਖਾਲੀ *)(((uintptr_t)ptr & ~(uintptr_t)align_ਨਕਾਬ) + alignment);

  ਜੇ (((size_t)ਸੂਚਕ_diff(ptr, ਵਿਸਤਾਰ) >= ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) ||
      (ਸੂਚਕ_ਵਿੱਥ(ptr, ਮਾਪ_2) > ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, mapped_ਮਾਪ)) ||
      (((uintptr_t)ptr & ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ) != (uintptr_t)ਵਿਸਤਾਰ)) {
    _rpmalloc_unmap(ਵਿਸਤਾਰ, mapped_ਮਾਪ, align_ਵਿੱਥ, mapped_ਮਾਪ);
    ++ਅੰਕ_ਪੰਨੇ;
    ਜੇ (ਅੰਕ_ਪੰਨੇ > ਹੱਦਬੰਦੀ_ਪੰਨੇ) {
      errno = EINVAL;
      ਵਾਪਸ 0;
    }
    ਜਾਓ retry;
  }

  // Store page count in span_count
  ਵਿਸਤਾਰ->ਮਾਪ_class = ਮਾਪ_CLASS_HUGE;
  ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ = (uint32_t)ਅੰਕ_ਪੰਨੇ;
  ਵਿਸਤਾਰ->align_ਵਿੱਥ = (uint32_t)align_ਵਿੱਥ;
  ਵਿਸਤਾਰ->ਢੇਰ_2 = ਢੇਰ_2;
  rpmalloc_stat_ਜੋੜੋ_peak(&huge_ਪੰਨੇ_current, ਅੰਕ_ਪੰਨੇ, huge_ਪੰਨੇ_peak);

#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
  rpmalloc_ਵਿਸਤਾਰ_double_link_list_ਜੋੜੋ(&ਢੇਰ_2->large_huge_ਵਿਸਤਾਰ, ਵਿਸਤਾਰ);
#ਜੇ_ਅੰਤ
  ++ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;

  _rpmalloc_stat_add64(&ਵੰਡ_counter, 1);

  ਵਾਪਸ ptr;
}

////////////
///
/// Deallocation entry points
///
//////

//! Deallocate the given small/medium memory block in the current thread local
//! heap
ਸਥਿਰ ਖਾਲੀ rpmalloc_deallocate_ਸਿੱਧਾ_small_or_medium(ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ,
                                                        ਖਾਲੀ *ਖੰਡ) {
  ਢੇਰ_t *ਢੇਰ_2 = ਵਿਸਤਾਰ->ਢੇਰ_2;
  rpmalloc_assert(ਢੇਰ_2->owner_thread == get_thread_id() ||
                      !ਢੇਰ_2->owner_thread || ਢੇਰ_2->finalize,
                  "Internal failure");
  // Add block to free list
  ਜੇ (UNEXPECTED(rpmalloc_ਵਿਸਤਾਰ_is_fully_utilized(ਵਿਸਤਾਰ))) {
    ਵਿਸਤਾਰ->ਵਰਤਿਆ_ਗਿਣਤੀ = ਵਿਸਤਾਰ->ਖੰਡ_ਗਿਣਤੀ;
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
    rpmalloc_ਵਿਸਤਾਰ_double_link_list_remove(&ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ[ਵਿਸਤਾਰ->ਮਾਪ_class],
                                           ਵਿਸਤਾਰ);
#ਜੇ_ਅੰਤ
    rpmalloc_ਵਿਸਤਾਰ_double_link_list_ਜੋੜੋ(
        &ਢੇਰ_2->ਮਾਪ_class[ਵਿਸਤਾਰ->ਮਾਪ_class].partial_ਵਿਸਤਾਰ, ਵਿਸਤਾਰ);
    --ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  }
  *((ਖਾਲੀ **)ਖੰਡ) = ਵਿਸਤਾਰ->free_list;
  --ਵਿਸਤਾਰ->ਵਰਤਿਆ_ਗਿਣਤੀ;
  ਵਿਸਤਾਰ->free_list = ਖੰਡ;
  ਜੇ (UNEXPECTED(ਵਿਸਤਾਰ->ਵਰਤਿਆ_ਗਿਣਤੀ == ਵਿਸਤਾਰ->list_ਮਾਪ)) {
    // If there are no used blocks it is guaranteed that no other external
    // thread is accessing the span
    ਜੇ (ਵਿਸਤਾਰ->ਵਰਤਿਆ_ਗਿਣਤੀ) {
      // Make sure we have synchronized the deferred list and list size by using
      // acquire semantics and guarantee that no external thread is accessing
      // span concurrently
      ਖਾਲੀ *free_list;
      ਕਰੋ {
        free_list = atomic_exchange_ptr_acquire(&ਵਿਸਤਾਰ->free_list_deferred,
                                                INVALID_ਸੂਚਕ);
      } ਜਦੋਂ (free_list == INVALID_ਸੂਚਕ);
      atomic_store_ptr_release(&ਵਿਸਤਾਰ->free_list_deferred, free_list);
    }
    rpmalloc_ਵਿਸਤਾਰ_double_link_list_remove(
        &ਢੇਰ_2->ਮਾਪ_class[ਵਿਸਤਾਰ->ਮਾਪ_class].partial_ਵਿਸਤਾਰ, ਵਿਸਤਾਰ);
    rpmalloc_ਵਿਸਤਾਰ_release_ਮੰਜ਼ਿਲ_cache(ਢੇਰ_2, ਵਿਸਤਾਰ);
  }
}

ਸਥਿਰ ਖਾਲੀ rpmalloc_deallocate_defer_free_ਵਿਸਤਾਰ(ਢੇਰ_t *ਢੇਰ_2, ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ) {
  ਜੇ (ਵਿਸਤਾਰ->ਮਾਪ_class != ਮਾਪ_CLASS_HUGE)
    _rpmalloc_stat_inc(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1].spans_deferred);
  // This list does not need ABA protection, no mutable side state
  ਕਰੋ {
    ਵਿਸਤਾਰ->free_list = (ਖਾਲੀ *)atomic_ਲੱਦੋ_ptr(&ਢੇਰ_2->ਵਿਸਤਾਰ_free_deferred);
  } ਜਦੋਂ (!atomic_cas_ptr(&ਢੇਰ_2->ਵਿਸਤਾਰ_free_deferred, ਵਿਸਤਾਰ, ਵਿਸਤਾਰ->free_list));
}

//! Put the block in the deferred free list of the owning span
ਸਥਿਰ ਖਾਲੀ _rpmalloc_deallocate_defer_small_or_medium(ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ,
                                                       ਖਾਲੀ *ਖੰਡ) {
  // The memory ordering here is a bit tricky, to avoid having to ABA protect
  // the deferred free list to avoid desynchronization of list and list size
  // we need to have acquire semantics on successful CAS of the pointer to
  // guarantee the list_size variable validity + release semantics on pointer
  // store
  ਖਾਲੀ *free_list;
  ਕਰੋ {
    free_list =
        atomic_exchange_ptr_acquire(&ਵਿਸਤਾਰ->free_list_deferred, INVALID_ਸੂਚਕ);
  } ਜਦੋਂ (free_list == INVALID_ਸੂਚਕ);
  *((ਖਾਲੀ **)ਖੰਡ) = free_list;
  uint32_t free_ਗਿਣਤੀ = ++ਵਿਸਤਾਰ->list_ਮਾਪ;
  ਪੂਰਨ all_deferred_free = (free_ਗਿਣਤੀ == ਵਿਸਤਾਰ->ਖੰਡ_ਗਿਣਤੀ);
  atomic_store_ptr_release(&ਵਿਸਤਾਰ->free_list_deferred, ਖੰਡ);
  ਜੇ (all_deferred_free) {
    // Span was completely freed by this block. Due to the INVALID_POINTER spin
    // lock no other thread can reach this state simultaneously on this span.
    // Safe to move to owner heap deferred cache
    rpmalloc_deallocate_defer_free_ਵਿਸਤਾਰ(ਵਿਸਤਾਰ->ਢੇਰ_2, ਵਿਸਤਾਰ);
  }
}

ਸਥਿਰ ਖਾਲੀ _rpmalloc_deallocate_small_or_medium(ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ, ਖਾਲੀ *p) {
  _rpmalloc_stat_inc_free(ਵਿਸਤਾਰ->ਢੇਰ_2, ਵਿਸਤਾਰ->ਮਾਪ_class);
  ਜੇ (ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_ALIGNED_ਖੰਡਾਂ) {
    // Realign pointer to block start
    ਖਾਲੀ *ਖੰਡਾਂ_ਸ਼ੁਰੂ = ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ);
    uint32_t ਖੰਡ_ਵਿੱਥ = (uint32_t)ਸੂਚਕ_diff(p, ਖੰਡਾਂ_ਸ਼ੁਰੂ);
    p = ਸੂਚਕ_ਵਿੱਥ(p, -(int32_t)(ਖੰਡ_ਵਿੱਥ % ਵਿਸਤਾਰ->ਖੰਡ_ਮਾਪ_2));
  }
  // Check if block belongs to this heap or if deallocation should be deferred
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
  ਪੂਰਨ defer =
      (ਵਿਸਤਾਰ->ਢੇਰ_2->owner_thread &&
       (ਵਿਸਤਾਰ->ਢੇਰ_2->owner_thread != get_thread_id()) && !ਵਿਸਤਾਰ->ਢੇਰ_2->finalize);
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ defer =
      ((ਵਿਸਤਾਰ->ਢੇਰ_2->owner_thread != get_thread_id()) && !ਵਿਸਤਾਰ->ਢੇਰ_2->finalize);
#ਜੇ_ਅੰਤ
  ਜੇ (!defer)
    rpmalloc_deallocate_ਸਿੱਧਾ_small_or_medium(ਵਿਸਤਾਰ, p);
  ਨਹੀਂਤਾਂ
    _rpmalloc_deallocate_defer_small_or_medium(ਵਿਸਤਾਰ, p);
}

//! Deallocate the given large memory block to the current heap
ਸਥਿਰ ਖਾਲੀ _rpmalloc_deallocate_large(ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ) {
  rpmalloc_assert(ਵਿਸਤਾਰ->ਮਾਪ_class == ਮਾਪ_CLASS_LARGE, "Bad span size class");
  rpmalloc_assert(!(ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_MASTER) ||
                      !(ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_SUBSPAN),
                  "Span flag corrupted");
  rpmalloc_assert((ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_MASTER) ||
                      (ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_SUBSPAN),
                  "Span flag corrupted");
  // We must always defer (unless finalizing) if from another heap since we
  // cannot touch the list or counters of another heap
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
  ਪੂਰਨ defer =
      (ਵਿਸਤਾਰ->ਢੇਰ_2->owner_thread &&
       (ਵਿਸਤਾਰ->ਢੇਰ_2->owner_thread != get_thread_id()) && !ਵਿਸਤਾਰ->ਢੇਰ_2->finalize);
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ defer =
      ((ਵਿਸਤਾਰ->ਢੇਰ_2->owner_thread != get_thread_id()) && !ਵਿਸਤਾਰ->ਢੇਰ_2->finalize);
#ਜੇ_ਅੰਤ
  ਜੇ (defer) {
    rpmalloc_deallocate_defer_free_ਵਿਸਤਾਰ(ਵਿਸਤਾਰ->ਢੇਰ_2, ਵਿਸਤਾਰ);
    ਵਾਪਸ;
  }
  rpmalloc_assert(ਵਿਸਤਾਰ->ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ, "Heap span counter corrupted");
  --ਵਿਸਤਾਰ->ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
  rpmalloc_ਵਿਸਤਾਰ_double_link_list_remove(&ਵਿਸਤਾਰ->ਢੇਰ_2->large_huge_ਵਿਸਤਾਰ, ਵਿਸਤਾਰ);
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_ADAPTIVE_THREAD_CACHE || ENABLE_STATISTICS
  // Decrease counter
  size_t idx = ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ - 1;
  atomic_decr32(&ਵਿਸਤਾਰ->ਢੇਰ_2->ਵਿਸਤਾਰ_use[idx].current);
#ਜੇ_ਅੰਤ
  ਢੇਰ_t *ਢੇਰ_2 = ਵਿਸਤਾਰ->ਢੇਰ_2;
  rpmalloc_assert(ਢੇਰ_2, "No thread heap");
#ਜੇ ENABLE_THREAD_CACHE
  ਅਚਲ ਪੂਰਨ ਸੈੱਟ_as_reserved =
      ((ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ > 1) && (ਢੇਰ_2->ਵਿਸਤਾਰ_cache.ਗਿਣਤੀ_2 == 0) &&
       !ਢੇਰ_2->finalize && !ਢੇਰ_2->spans_reserved);
#ਨਹੀਂਤਾਂ
  ਅਚਲ ਪੂਰਨ ਸੈੱਟ_as_reserved =
      ((ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ > 1) && !ਢੇਰ_2->finalize && !ਢੇਰ_2->spans_reserved);
#ਜੇ_ਅੰਤ
  ਜੇ (ਸੈੱਟ_as_reserved) {
    ਢੇਰ_2->ਵਿਸਤਾਰ_reserve = ਵਿਸਤਾਰ;
    ਢੇਰ_2->spans_reserved = ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ;
    ਜੇ (ਵਿਸਤਾਰ->flags & ਵਿਸਤਾਰ_FLAG_MASTER) {
      ਢੇਰ_2->ਵਿਸਤਾਰ_reserve_master = ਵਿਸਤਾਰ;
    } ਨਹੀਂਤਾਂ { // SPAN_FLAG_SUBSPAN
      ਵਿਸਤਾਰ_t *master = (ਵਿਸਤਾਰ_t *)ਸੂਚਕ_ਵਿੱਥ(
          ਵਿਸਤਾਰ,
          -(intptr_t)((size_t)ਵਿਸਤਾਰ->ਵਿੱਥ_from_master * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ));
      ਢੇਰ_2->ਵਿਸਤਾਰ_reserve_master = master;
      rpmalloc_assert(master->flags & ਵਿਸਤਾਰ_FLAG_MASTER, "Span flag corrupted");
      rpmalloc_assert(atomic_load32(&master->ਬਚਿਆ_spans) >=
                          (int32_t)ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ,
                      "Master span count corrupted");
    }
    _rpmalloc_stat_inc(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[idx].spans_ਮੰਜ਼ਿਲ_reserved);
  } ਨਹੀਂਤਾਂ {
    // Insert into cache list
    rpmalloc_ਢੇਰ_cache_ਪਾਓ(ਢੇਰ_2, ਵਿਸਤਾਰ);
  }
}

//! Deallocate the given huge span
ਸਥਿਰ ਖਾਲੀ _rpmalloc_deallocate_huge(ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ) {
  rpmalloc_assert(ਵਿਸਤਾਰ->ਢੇਰ_2, "No span heap");
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
  ਪੂਰਨ defer =
      (ਵਿਸਤਾਰ->ਢੇਰ_2->owner_thread &&
       (ਵਿਸਤਾਰ->ਢੇਰ_2->owner_thread != get_thread_id()) && !ਵਿਸਤਾਰ->ਢੇਰ_2->finalize);
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ defer =
      ((ਵਿਸਤਾਰ->ਢੇਰ_2->owner_thread != get_thread_id()) && !ਵਿਸਤਾਰ->ਢੇਰ_2->finalize);
#ਜੇ_ਅੰਤ
  ਜੇ (defer) {
    rpmalloc_deallocate_defer_free_ਵਿਸਤਾਰ(ਵਿਸਤਾਰ->ਢੇਰ_2, ਵਿਸਤਾਰ);
    ਵਾਪਸ;
  }
  rpmalloc_assert(ਵਿਸਤਾਰ->ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ, "Heap span counter corrupted");
  --ਵਿਸਤਾਰ->ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ;
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
  rpmalloc_ਵਿਸਤਾਰ_double_link_list_remove(&ਵਿਸਤਾਰ->ਢੇਰ_2->large_huge_ਵਿਸਤਾਰ, ਵਿਸਤਾਰ);
#ਜੇ_ਅੰਤ

  // Oversized allocation, page count is stored in span_count
  size_t ਅੰਕ_ਪੰਨੇ = ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  _rpmalloc_unmap(ਵਿਸਤਾਰ, ਅੰਕ_ਪੰਨੇ * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ, ਵਿਸਤਾਰ->align_ਵਿੱਥ,
                  ਅੰਕ_ਪੰਨੇ * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ);
  _rpmalloc_stat_sub(&huge_ਪੰਨੇ_current, ਅੰਕ_ਪੰਨੇ);
}

//! Deallocate the given block
ਸਥਿਰ ਖਾਲੀ _rpmalloc_deallocate(ਖਾਲੀ *p) {
  _rpmalloc_stat_add64(&_deallocation_counter, 1);
  // Grab the span (always at start of span, using span alignment)
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)((uintptr_t)p & ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ);
  ਜੇ (UNEXPECTED(!ਵਿਸਤਾਰ))
    ਵਾਪਸ;
  ਜੇ (EXPECTED(ਵਿਸਤਾਰ->ਮਾਪ_class < ਮਾਪ_CLASS_ਗਿਣਤੀ))
    _rpmalloc_deallocate_small_or_medium(ਵਿਸਤਾਰ, p);
  ਨਹੀਂਤਾਂ ਜੇ (ਵਿਸਤਾਰ->ਮਾਪ_class == ਮਾਪ_CLASS_LARGE)
    _rpmalloc_deallocate_large(ਵਿਸਤਾਰ);
  ਨਹੀਂਤਾਂ
    _rpmalloc_deallocate_huge(ਵਿਸਤਾਰ);
}

////////////
///
/// Reallocation entry points
///
//////

ਸਥਿਰ size_t rpmalloc_usable_ਮਾਪ_2(ਖਾਲੀ *p);

//! Reallocate the given block to the given size
ਸਥਿਰ ਖਾਲੀ *_rpmalloc_reallocate(ਢੇਰ_t *ਢੇਰ_2, ਖਾਲੀ *p, size_t ਮਾਪ_2,
                                  size_t oldsize, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags) {
  ਜੇ (p) {
    // Grab the span using guaranteed span alignment
    ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)((uintptr_t)p & ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ);
    ਜੇ (EXPECTED(ਵਿਸਤਾਰ->ਮਾਪ_class < ਮਾਪ_CLASS_ਗਿਣਤੀ)) {
      // Small/medium sized block
      rpmalloc_assert(ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ == 1, "Span counter corrupted");
      ਖਾਲੀ *ਖੰਡਾਂ_ਸ਼ੁਰੂ = ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ);
      uint32_t ਖੰਡ_ਵਿੱਥ = (uint32_t)ਸੂਚਕ_diff(p, ਖੰਡਾਂ_ਸ਼ੁਰੂ);
      uint32_t ਖੰਡ_idx = ਖੰਡ_ਵਿੱਥ / ਵਿਸਤਾਰ->ਖੰਡ_ਮਾਪ_2;
      ਖਾਲੀ *ਖੰਡ =
          ਸੂਚਕ_ਵਿੱਥ(ਖੰਡਾਂ_ਸ਼ੁਰੂ, (size_t)ਖੰਡ_idx * ਵਿਸਤਾਰ->ਖੰਡ_ਮਾਪ_2);
      ਜੇ (!oldsize)
        oldsize =
            (size_t)((ptrdiff_t)ਵਿਸਤਾਰ->ਖੰਡ_ਮਾਪ_2 - ਸੂਚਕ_diff(p, ਖੰਡ));
      ਜੇ ((size_t)ਵਿਸਤਾਰ->ਖੰਡ_ਮਾਪ_2 >= ਮਾਪ_2) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ਜੇ ((p != ਖੰਡ) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(ਖੰਡ, p, oldsize);
        ਵਾਪਸ ਖੰਡ;
      }
    } ਨਹੀਂਤਾਂ ਜੇ (ਵਿਸਤਾਰ->ਮਾਪ_class == ਮਾਪ_CLASS_LARGE) {
      // Large block
      size_t ਕੁੱਲ_ਮਾਪ_2 = ਮਾਪ_2 + ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ;
      size_t ਅੰਕ_spans = ਕੁੱਲ_ਮਾਪ_2 >> ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ;
      ਜੇ (ਕੁੱਲ_ਮਾਪ_2 & (ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ - 1))
        ++ਅੰਕ_spans;
      size_t current_spans = ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ;
      ਖਾਲੀ *ਖੰਡ = ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ);
      ਜੇ (!oldsize)
        oldsize = (current_spans * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) -
                  (size_t)ਸੂਚਕ_diff(p, ਖੰਡ) - ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ;
      ਜੇ ((current_spans >= ਅੰਕ_spans) && (ਕੁੱਲ_ਮਾਪ_2 >= (oldsize / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ਜੇ ((p != ਖੰਡ) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(ਖੰਡ, p, oldsize);
        ਵਾਪਸ ਖੰਡ;
      }
    } ਨਹੀਂਤਾਂ {
      // Oversized block
      size_t ਕੁੱਲ_ਮਾਪ_2 = ਮਾਪ_2 + ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ;
      size_t ਅੰਕ_ਪੰਨੇ = ਕੁੱਲ_ਮਾਪ_2 >> ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ_ਖਿਸਕਾਓ;
      ਜੇ (ਕੁੱਲ_ਮਾਪ_2 & (ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ - 1))
        ++ਅੰਕ_ਪੰਨੇ;
      // Page count is stored in span_count
      size_t current_ਪੰਨੇ = ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ;
      ਖਾਲੀ *ਖੰਡ = ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ);
      ਜੇ (!oldsize)
        oldsize = (current_ਪੰਨੇ * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ) -
                  (size_t)ਸੂਚਕ_diff(p, ਖੰਡ) - ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ;
      ਜੇ ((current_ਪੰਨੇ >= ਅੰਕ_ਪੰਨੇ) && (ਅੰਕ_ਪੰਨੇ >= (current_ਪੰਨੇ / 2))) {
        // Still fits in block, never mind trying to save memory, but preserve
        // data if alignment changed
        ਜੇ ((p != ਖੰਡ) && !(flags & RPMALLOC_NO_PRESERVE))
          memmove(ਖੰਡ, p, oldsize);
        ਵਾਪਸ ਖੰਡ;
      }
    }
  } ਨਹੀਂਤਾਂ {
    oldsize = 0;
  }

  ਜੇ (!!(flags & RPMALLOC_GROW_OR_FAIL))
    ਵਾਪਸ 0;

  // Size is greater than block size, need to allocate a new block and
  // deallocate the old Avoid hysteresis by overallocating if increase is small
  // (below 37%)
  size_t ਹੇਠਲਾ_bound = oldsize + (oldsize >> 2) + (oldsize >> 3);
  size_t new_ਮਾਪ =
      (ਮਾਪ_2 > ਹੇਠਲਾ_bound) ? ਮਾਪ_2 : ((ਮਾਪ_2 > oldsize) ? ਹੇਠਲਾ_bound : ਮਾਪ_2);
  ਖਾਲੀ *ਖੰਡ = _rpmalloc_allocate(ਢੇਰ_2, new_ਮਾਪ);
  ਜੇ (p && ਖੰਡ) {
    ਜੇ (!(flags & RPMALLOC_NO_PRESERVE))
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਖੰਡ, p, oldsize < new_ਮਾਪ ? oldsize : new_ਮਾਪ);
    _rpmalloc_deallocate(p);
  }

  ਵਾਪਸ ਖੰਡ;
}

ਸਥਿਰ ਖਾਲੀ *_rpmalloc_aligned_reallocate(ਢੇਰ_t *ਢੇਰ_2, ਖਾਲੀ *ptr,
                                          size_t alignment, size_t ਮਾਪ_2,
                                          size_t oldsize, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags) {
  ਜੇ (alignment <= SMALL_GRANULARITY)
    ਵਾਪਸ _rpmalloc_reallocate(ਢੇਰ_2, ptr, ਮਾਪ_2, oldsize, flags);

  ਪੂਰਨ no_alloc = !!(flags & RPMALLOC_GROW_OR_FAIL);
  size_t usablesize = (ptr ? rpmalloc_usable_ਮਾਪ_2(ptr) : 0);
  ਜੇ ((usablesize >= ਮਾਪ_2) && !((uintptr_t)ptr & (alignment - 1))) {
    ਜੇ (no_alloc || (ਮਾਪ_2 >= (usablesize / 2)))
      ਵਾਪਸ ptr;
  }
  // Aligned alloc marks span as having aligned blocks
  ਖਾਲੀ *ਖੰਡ =
      (!no_alloc ? _rpmalloc_aligned_allocate(ਢੇਰ_2, alignment, ਮਾਪ_2) : 0);
  ਜੇ (EXPECTED(ਖੰਡ != 0)) {
    ਜੇ (!(flags & RPMALLOC_NO_PRESERVE) && ptr) {
      ਜੇ (!oldsize)
        oldsize = usablesize;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਖੰਡ, ptr, oldsize < ਮਾਪ_2 ? oldsize : ਮਾਪ_2);
    }
    _rpmalloc_deallocate(ptr);
  }
  ਵਾਪਸ ਖੰਡ;
}

////////////
///
/// Initialization, finalization and utility
///
//////

//! Get the usable size of the given block
ਸਥਿਰ size_t rpmalloc_usable_ਮਾਪ_2(ਖਾਲੀ *p) {
  // Grab the span using guaranteed span alignment
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)((uintptr_t)p & ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ);
  ਜੇ (ਵਿਸਤਾਰ->ਮਾਪ_class < ਮਾਪ_CLASS_ਗਿਣਤੀ) {
    // Small/medium block
    ਖਾਲੀ *ਖੰਡਾਂ_ਸ਼ੁਰੂ = ਸੂਚਕ_ਵਿੱਥ(ਵਿਸਤਾਰ, ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ);
    ਵਾਪਸ ਵਿਸਤਾਰ->ਖੰਡ_ਮਾਪ_2 -
           ((size_t)ਸੂਚਕ_diff(p, ਖੰਡਾਂ_ਸ਼ੁਰੂ) % ਵਿਸਤਾਰ->ਖੰਡ_ਮਾਪ_2);
  }
  ਜੇ (ਵਿਸਤਾਰ->ਮਾਪ_class == ਮਾਪ_CLASS_LARGE) {
    // Large block
    size_t current_spans = ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ;
    ਵਾਪਸ (current_spans * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) - (size_t)ਸੂਚਕ_diff(p, ਵਿਸਤਾਰ);
  }
  // Oversized block, page count is stored in span_count
  size_t current_ਪੰਨੇ = ਵਿਸਤਾਰ->ਵਿਸਤਾਰ_ਗਿਣਤੀ;
  ਵਾਪਸ (current_ਪੰਨੇ * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ) - (size_t)ਸੂਚਕ_diff(p, ਵਿਸਤਾਰ);
}

//! Adjust and optimize the size class properties for the given class
ਸਥਿਰ ਖਾਲੀ rpmalloc_adjust_ਮਾਪ_class(size_t iclass) {
  size_t ਖੰਡ_ਮਾਪ_2 = ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[iclass].ਖੰਡ_ਮਾਪ_2;
  size_t ਖੰਡ_ਗਿਣਤੀ = (ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ - ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ) / ਖੰਡ_ਮਾਪ_2;

  ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[iclass].ਖੰਡ_ਗਿਣਤੀ = (uint16_t)ਖੰਡ_ਗਿਣਤੀ;
  ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[iclass].class_idx = (uint16_t)iclass;

  // Check if previous size classes can be merged
  ਜੇ (iclass >= SMALL_CLASS_ਗਿਣਤੀ) {
    size_t prevclass = iclass;
    ਜਦੋਂ (prevclass > 0) {
      --prevclass;
      // A class can be merged if number of pages and number of blocks are equal
      ਜੇ (ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[prevclass].ਖੰਡ_ਗਿਣਤੀ ==
          ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[iclass].ਖੰਡ_ਗਿਣਤੀ)
        _rpmalloc_memcpy_const(ਸਮ੍ਰਿਤੀ_ਮਾਪ_class + prevclass,
                               ਸਮ੍ਰਿਤੀ_ਮਾਪ_class + iclass,
                               ਆਕਾਰ(ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[iclass]));
      ਨਹੀਂਤਾਂ
        ਰੋਕੋ;
    }
  }
}

//! Initialize the allocator and setup global data
ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਪੂਰਨ rpmalloc_initialize(ਖਾਲੀ) {
  ਜੇ (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    ਵਾਪਸ 0;
  }
  ਵਾਪਸ rpmalloc_initialize_config(0);
}

ਪੂਰਨ rpmalloc_initialize_config(ਅਚਲ rpmalloc_config_t *config) {
  ਜੇ (_rpmalloc_initialized) {
    rpmalloc_thread_initialize();
    ਵਾਪਸ 0;
  }
  _rpmalloc_initialized = 1;

  ਜੇ (config)
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&ਸਮ੍ਰਿਤੀ_config, config, ਆਕਾਰ(rpmalloc_config_t));
  ਨਹੀਂਤਾਂ
    _rpmalloc_memset_const(&ਸਮ੍ਰਿਤੀ_config, 0, ਆਕਾਰ(rpmalloc_config_t));

  ਜੇ (!ਸਮ੍ਰਿਤੀ_config.ਸਮ੍ਰਿਤੀ_map || !ਸਮ੍ਰਿਤੀ_config.ਸਮ੍ਰਿਤੀ_unmap) {
    ਸਮ੍ਰਿਤੀ_config.ਸਮ੍ਰਿਤੀ_map = _rpmalloc_mmap_os;
    ਸਮ੍ਰਿਤੀ_config.ਸਮ੍ਰਿਤੀ_unmap = _rpmalloc_unmap_os;
  }

#ਜੇ PLATFORM_WINDOWS
  SYSTEM_INFO system_info;
  memset(&system_info, 0, ਆਕਾਰ(system_info));
  GetSystemInfo(&system_info);
  ਸਮ੍ਰਿਤੀ_map_granularity = system_info.dw_ਵੰਡ_Granularity;
#ਨਹੀਂਤਾਂ
  ਸਮ੍ਰਿਤੀ_map_granularity = (size_t)sysconf(_SC_PAGESIZE);
#ਜੇ_ਅੰਤ

#ਜੇ RPMALLOC_CONFIGURABLE
  ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ = ਸਮ੍ਰਿਤੀ_config.ਪੰਨਾ_ਮਾਪ_2;
#ਨਹੀਂਤਾਂ
  ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ = 0;
#ਜੇ_ਅੰਤ
  ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ = 0;
  ਜੇ (!ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ) {
#ਜੇ PLATFORM_WINDOWS
    ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ = system_info.dw_ਪੰਨਾ_ਮਾਪ;
#ਨਹੀਂਤਾਂ
    ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ = ਸਮ੍ਰਿਤੀ_map_granularity;
    ਜੇ (ਸਮ੍ਰਿਤੀ_config.enable_huge_ਪੰਨੇ) {
#ਜੇ defined(__linux__)
      size_t huge_ਪੰਨਾ_ਮਾਪ = 0;
      FILE *meminfo = fopen("/proc/meminfo", "r");
      ਜੇ (meminfo) {
        ਅੱਖਰ ਸਤਰ_2[128];
        ਜਦੋਂ (!huge_ਪੰਨਾ_ਮਾਪ && fgets(ਸਤਰ_2, ਆਕਾਰ(ਸਤਰ_2) - 1, meminfo)) {
          ਸਤਰ_2[ਆਕਾਰ(ਸਤਰ_2) - 1] = 0;
          ਜੇ (ਲੜੀ_ਖੋਜ(ਸਤਰ_2, "Hugepagesize:"))
            huge_ਪੰਨਾ_ਮਾਪ = (size_t)strtol(ਸਤਰ_2 + 13, 0, 10) * 1024;
        }
        fclose(meminfo);
      }
      ਜੇ (huge_ਪੰਨਾ_ਮਾਪ) {
        ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ = 1;
        ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ = huge_ਪੰਨਾ_ਮਾਪ;
        ਸਮ੍ਰਿਤੀ_map_granularity = huge_ਪੰਨਾ_ਮਾਪ;
      }
#ਜਾਂਜੇ defined(__FreeBSD__)
      ਪੂਰਨ rc;
      size_t sz = ਆਕਾਰ(rc);

      ਜੇ (sysctlbyname("vm.pmap.pg_ps_enabled", &rc, &sz, ਨਿਰਾ, 0) == 0 &&
          rc == 1) {
        ਸਥਿਰ size_t defsize = 2 * 1024 * 1024;
        ਪੂਰਨ nsize = 0;
        size_t sizes[4] = {0};
        ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ = 1;
        ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ = defsize;
        ਜੇ ((nsize = getpagesizes(sizes, 4)) >= 2) {
          nsize--;
          ਲਈ (size_t csize = sizes[nsize]; nsize >= 0 && csize;
               --nsize, csize = sizes[nsize]) {
            //! Unlikely, but as a precaution..
            rpmalloc_assert(!(csize & (csize - 1)) && !(csize % 1024),
                            "Invalid page size");
            ਜੇ (defsize < csize) {
              ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ = csize;
              ਰੋਕੋ;
            }
          }
        }
        ਸਮ੍ਰਿਤੀ_map_granularity = ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
      }
#ਜਾਂਜੇ defined(__APPLE__) || defined(__NetBSD__)
      ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ = 1;
      ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ = 2 * 1024 * 1024;
      ਸਮ੍ਰਿਤੀ_map_granularity = ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
#ਜੇ_ਅੰਤ
    }
#ਜੇ_ਅੰਤ
  } ਨਹੀਂਤਾਂ {
    ਜੇ (ਸਮ੍ਰਿਤੀ_config.enable_huge_ਪੰਨੇ)
      ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ = 1;
  }

#ਜੇ PLATFORM_WINDOWS
  ਜੇ (ਸਮ੍ਰਿਤੀ_config.enable_huge_ਪੰਨੇ) {
    HANDLE token = 0;
    size_t large_ਪੰਨਾ_minimum = Get_Large_ਪੰਨਾ_Minimum();
    ਜੇ (large_ਪੰਨਾ_minimum)
      OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
    ਜੇ (token) {
      LUID luid;
      ਜੇ (Lookup_Privilege_ਮੁੱਲ(0, SE_ਤਾਲਾ_ਸਮ੍ਰਿਤੀ_NAME, &luid)) {
        TOKEN_PRIVILEGES token_privileges;
        memset(&token_privileges, 0, ਆਕਾਰ(token_privileges));
        token_privileges.Privilege_ਗਿਣਤੀ = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].ਗੁਣ = SE_PRIVILEGE_ENABLED;
        ਜੇ (AdjustTokenPrivileges(token, FALSE, &token_privileges, 0, 0, 0)) {
          ਜੇ (Get_ਆਖਰੀ_Error() == ERROR_SUCCESS)
            ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ = 1;
        }
      }
      CloseHandle(token);
    }
    ਜੇ (ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ) {
      ਜੇ (large_ਪੰਨਾ_minimum > ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ)
        ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ = large_ਪੰਨਾ_minimum;
      ਜੇ (large_ਪੰਨਾ_minimum > ਸਮ੍ਰਿਤੀ_map_granularity)
        ਸਮ੍ਰਿਤੀ_map_granularity = large_ਪੰਨਾ_minimum;
    }
  }
#ਜੇ_ਅੰਤ

  size_t min_ਵਿਸਤਾਰ_ਮਾਪ = 256;
  size_t max_ਪੰਨਾ_ਮਾਪ;
#ਜੇ UINTPTR_MAX > 0xFFFFFFFF
  max_ਪੰਨਾ_ਮਾਪ = 4096ULL * 1024ULL * 1024ULL;
#ਨਹੀਂਤਾਂ
  max_ਪੰਨਾ_ਮਾਪ = 4 * 1024 * 1024;
#ਜੇ_ਅੰਤ
  ਜੇ (ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ < min_ਵਿਸਤਾਰ_ਮਾਪ)
    ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ = min_ਵਿਸਤਾਰ_ਮਾਪ;
  ਜੇ (ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ > max_ਪੰਨਾ_ਮਾਪ)
    ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ = max_ਪੰਨਾ_ਮਾਪ;
  ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ_ਖਿਸਕਾਓ = 0;
  size_t ਪੰਨਾ_ਮਾਪ_ਦੋਆਧਾਰੀ_ਅੰਕ = ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  ਜਦੋਂ (ਪੰਨਾ_ਮਾਪ_ਦੋਆਧਾਰੀ_ਅੰਕ != 1) {
    ++ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ_ਖਿਸਕਾਓ;
    ਪੰਨਾ_ਮਾਪ_ਦੋਆਧਾਰੀ_ਅੰਕ >>= 1;
  }
  ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ = ((size_t)1 << ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ_ਖਿਸਕਾਓ);

#ਜੇ RPMALLOC_CONFIGURABLE
  ਜੇ (!ਸਮ੍ਰਿਤੀ_config.ਵਿਸਤਾਰ_ਮਾਪ) {
    ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ = ਸਮ੍ਰਿਤੀ_default_ਵਿਸਤਾਰ_ਮਾਪ;
    ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ = ਸਮ੍ਰਿਤੀ_default_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ;
    ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ = ਸਮ੍ਰਿਤੀ_default_ਵਿਸਤਾਰ_ਨਕਾਬ;
  } ਨਹੀਂਤਾਂ {
    size_t ਵਿਸਤਾਰ_ਮਾਪ = ਸਮ੍ਰਿਤੀ_config.ਵਿਸਤਾਰ_ਮਾਪ;
    ਜੇ (ਵਿਸਤਾਰ_ਮਾਪ > (256 * 1024))
      ਵਿਸਤਾਰ_ਮਾਪ = (256 * 1024);
    ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ = 4096;
    ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ = 12;
    ਜਦੋਂ (ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ < ਵਿਸਤਾਰ_ਮਾਪ) {
      ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ <<= 1;
      ++ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ_ਖਿਸਕਾਓ;
    }
    ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ = ~(uintptr_t)(ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ - 1);
  }
#ਜੇ_ਅੰਤ

  ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ =
      (ਸਮ੍ਰਿਤੀ_config.ਵਿਸਤਾਰ_map_ਗਿਣਤੀ ? ਸਮ੍ਰਿਤੀ_config.ਵਿਸਤਾਰ_map_ਗਿਣਤੀ
                                     : DEFAULT_ਵਿਸਤਾਰ_MAP_ਗਿਣਤੀ);
  ਜੇ ((ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ) < ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ)
    ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ = (ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ / ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
  ਜੇ ((ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ >= ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) &&
      ((ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) % ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ))
    ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ = (ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ / ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
  ਸਮ੍ਰਿਤੀ_ਢੇਰ_reserve_ਗਿਣਤੀ = (ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ > DEFAULT_ਵਿਸਤਾਰ_MAP_ਗਿਣਤੀ)
                                   ? DEFAULT_ਵਿਸਤਾਰ_MAP_ਗਿਣਤੀ
                                   : ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ;

  ਸਮ੍ਰਿਤੀ_config.ਪੰਨਾ_ਮਾਪ_2 = ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  ਸਮ੍ਰਿਤੀ_config.ਵਿਸਤਾਰ_ਮਾਪ = ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ;
  ਸਮ੍ਰਿਤੀ_config.ਵਿਸਤਾਰ_map_ਗਿਣਤੀ = ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_map_ਗਿਣਤੀ;
  ਸਮ੍ਰਿਤੀ_config.enable_huge_ਪੰਨੇ = ਸਮ੍ਰਿਤੀ_huge_ਪੰਨੇ;

#ਜੇ ((defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD) ||          \
    defined(__TINYC__)
  ਜੇ (pthread_ਕੁੰਜੀ_create(&ਸਮ੍ਰਿਤੀ_thread_ਢੇਰ, rpmalloc_ਢੇਰ_release_raw_fc))
    ਵਾਪਸ -1;
#ਜੇ_ਅੰਤ
#ਜੇ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  fls_ਕੁੰਜੀ = FlsAlloc(&_rpmalloc_thread_destructor);
#ਜੇ_ਅੰਤ

  // Setup all small and medium size classes
  size_t iclass = 0;
  ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[iclass].ਖੰਡ_ਮਾਪ_2 = SMALL_GRANULARITY;
  rpmalloc_adjust_ਮਾਪ_class(iclass);
  ਲਈ (iclass = 1; iclass < SMALL_CLASS_ਗਿਣਤੀ; ++iclass) {
    size_t ਮਾਪ_2 = iclass * SMALL_GRANULARITY;
    ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[iclass].ਖੰਡ_ਮਾਪ_2 = (uint32_t)ਮਾਪ_2;
    rpmalloc_adjust_ਮਾਪ_class(iclass);
  }
  // At least two blocks per span, then fall back to large allocations
  ਸਮ੍ਰਿਤੀ_medium_ਮਾਪ_ਹੱਦਬੰਦੀ = (ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ - ਵਿਸਤਾਰ_ਸਿਰਲੇਖ_ਮਾਪ) >> 1;
  ਜੇ (ਸਮ੍ਰਿਤੀ_medium_ਮਾਪ_ਹੱਦਬੰਦੀ > MEDIUM_ਮਾਪ_ਹੱਦਬੰਦੀ)
    ਸਮ੍ਰਿਤੀ_medium_ਮਾਪ_ਹੱਦਬੰਦੀ = MEDIUM_ਮਾਪ_ਹੱਦਬੰਦੀ;
  ਲਈ (iclass = 0; iclass < MEDIUM_CLASS_ਗਿਣਤੀ; ++iclass) {
    size_t ਮਾਪ_2 = SMALL_ਮਾਪ_ਹੱਦਬੰਦੀ + ((iclass + 1) * MEDIUM_GRANULARITY);
    ਜੇ (ਮਾਪ_2 > ਸਮ੍ਰਿਤੀ_medium_ਮਾਪ_ਹੱਦਬੰਦੀ) {
      ਸਮ੍ਰਿਤੀ_medium_ਮਾਪ_ਹੱਦਬੰਦੀ =
          SMALL_ਮਾਪ_ਹੱਦਬੰਦੀ + (iclass * MEDIUM_GRANULARITY);
      ਰੋਕੋ;
    }
    ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[SMALL_CLASS_ਗਿਣਤੀ + iclass].ਖੰਡ_ਮਾਪ_2 = (uint32_t)ਮਾਪ_2;
    rpmalloc_adjust_ਮਾਪ_class(SMALL_CLASS_ਗਿਣਤੀ + iclass);
  }

  ਸਮ੍ਰਿਤੀ_orphan_heaps = 0;
#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
  ਸਮ੍ਰਿਤੀ_ਪਹਿਲਾ_class_orphan_heaps = 0;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_STATISTICS
  atomic_store32(&ਸਮ੍ਰਿਤੀ_ਸਰਗਰਮ_heaps, 0);
  atomic_store32(&mapped_ਪੰਨੇ, 0);
  mapped_ਪੰਨੇ_peak = 0;
  atomic_store32(&_master_spans, 0);
  atomic_store32(&mapped_ਕੁੱਲ, 0);
  atomic_store32(&unmapped_ਕੁੱਲ, 0);
  atomic_store32(&mapped_ਪੰਨੇ_os, 0);
  atomic_store32(&huge_ਪੰਨੇ_current, 0);
  huge_ਪੰਨੇ_peak = 0;
#ਜੇ_ਅੰਤ
  memset(ਸਮ੍ਰਿਤੀ_heaps, 0, ਆਕਾਰ(ਸਮ੍ਰਿਤੀ_heaps));
  atomic_store32_release(&ਸਮ੍ਰਿਤੀ_global_ਤਾਲਾ, 0);

  rpmalloc_linker_reference();

  // Initialize this thread
  rpmalloc_thread_initialize();
  ਵਾਪਸ 0;
}

//! Finalize the allocator
ਖਾਲੀ rpmalloc_finalize(ਖਾਲੀ) {
  rpmalloc_thread_finalize(1);
  // rpmalloc_dump_statistics(stdout);

  ਜੇ (ਸਮ੍ਰਿਤੀ_global_reserve) {
    atomic_add32(&ਸਮ੍ਰਿਤੀ_global_reserve_master->ਬਚਿਆ_spans,
                 -(int32_t)ਸਮ੍ਰਿਤੀ_global_reserve_ਗਿਣਤੀ);
    ਸਮ੍ਰਿਤੀ_global_reserve_master = 0;
    ਸਮ੍ਰਿਤੀ_global_reserve_ਗਿਣਤੀ = 0;
    ਸਮ੍ਰਿਤੀ_global_reserve = 0;
  }
  atomic_store32_release(&ਸਮ੍ਰਿਤੀ_global_ਤਾਲਾ, 0);

  // Free all thread caches and fully free spans
  ਲਈ (size_t list_idx = 0; list_idx < ਢੇਰ_ARRAY_ਮਾਪ; ++list_idx) {
    ਢੇਰ_t *ਢੇਰ_2 = ਸਮ੍ਰਿਤੀ_heaps[list_idx];
    ਜਦੋਂ (ਢੇਰ_2) {
      ਢੇਰ_t *ਅਗਲਾ_ਢੇਰ = ਢੇਰ_2->ਅਗਲਾ_ਢੇਰ;
      ਢੇਰ_2->finalize = 1;
      rpmalloc_ਢੇਰ_global_finalize(ਢੇਰ_2);
      ਢੇਰ_2 = ਅਗਲਾ_ਢੇਰ;
    }
  }

#ਜੇ ENABLE_GLOBAL_CACHE
  // Free global caches
  ਲਈ (size_t iclass = 0; iclass < LARGE_CLASS_ਗਿਣਤੀ; ++iclass)
    _rpmalloc_global_cache_finalize(&ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_cache[iclass]);
#ਜੇ_ਅੰਤ

#ਜੇ (defined(__APPLE__) || defined(__HAIKU__)) && ENABLE_PRELOAD
  pthread_ਕੁੰਜੀ_delete(ਸਮ੍ਰਿਤੀ_thread_ਢੇਰ);
#ਜੇ_ਅੰਤ
#ਜੇ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  FlsFree(fls_ਕੁੰਜੀ);
  fls_ਕੁੰਜੀ = 0;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_STATISTICS
  // If you hit these asserts you probably have memory leaks (perhaps global
  // scope data doing dynamic allocations) or double frees in your code
  rpmalloc_assert(atomic_load32(&mapped_ਪੰਨੇ) == 0, "Memory leak detected");
  rpmalloc_assert(atomic_load32(&mapped_ਪੰਨੇ_os) == 0,
                  "Memory leak detected");
#ਜੇ_ਅੰਤ

  _rpmalloc_initialized = 0;
}

//! Initialize thread, assign heap
ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ rpmalloc_thread_initialize(ਖਾਲੀ) {
  ਜੇ (!get_thread_ਢੇਰ_raw()) {
    ਢੇਰ_t *ਢੇਰ_2 = rpmalloc_ਢੇਰ_allocate(0);
    ਜੇ (ਢੇਰ_2) {
      _rpmalloc_stat_inc(&ਸਮ੍ਰਿਤੀ_ਸਰਗਰਮ_heaps);
      ਸੈੱਟ_thread_ਢੇਰ(ਢੇਰ_2);
#ਜੇ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
      Fls_ਸੈੱਟ_ਮੁੱਲ(fls_ਕੁੰਜੀ, ਢੇਰ_2);
#ਜੇ_ਅੰਤ
    }
  }
}

//! Finalize thread, orphan heap
ਖਾਲੀ rpmalloc_thread_finalize(ਪੂਰਨ release_caches) {
  ਢੇਰ_t *ਢੇਰ_2 = get_thread_ਢੇਰ_raw();
  ਜੇ (ਢੇਰ_2)
    rpmalloc_ਢੇਰ_release_raw(ਢੇਰ_2, release_caches);
  ਸੈੱਟ_thread_ਢੇਰ(0);
#ਜੇ defined(_WIN32) && (!defined(BUILD_DYNAMIC_LINK) || !BUILD_DYNAMIC_LINK)
  Fls_ਸੈੱਟ_ਮੁੱਲ(fls_ਕੁੰਜੀ, 0);
#ਜੇ_ਅੰਤ
}

ਪੂਰਨ rpmalloc_is_thread_initialized(ਖਾਲੀ) {
  ਵਾਪਸ (get_thread_ਢੇਰ_raw() != 0) ? 1 : 0;
}

ਅਚਲ rpmalloc_config_t *rpmalloc_config(ਖਾਲੀ) { ਵਾਪਸ &ਸਮ੍ਰਿਤੀ_config; }

// Extern interface

ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_ALLOCATOR ਖਾਲੀ *rpmalloc(size_t ਮਾਪ_2) {
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
  ਜੇ (ਮਾਪ_2 >= MAX_ALLOC_ਮਾਪ) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਢੇਰ_t *ਢੇਰ_2 = get_thread_ਢੇਰ();
  ਵਾਪਸ _rpmalloc_allocate(ਢੇਰ_2, ਮਾਪ_2);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ rpfree(ਖਾਲੀ *ptr) { _rpmalloc_deallocate(ptr); }

ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_ALLOCATOR ਖਾਲੀ *rpcalloc(size_t ਅੰਕ_2, size_t ਮਾਪ_2) {
  size_t ਕੁੱਲ_2;
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
#ਜੇ PLATFORM_WINDOWS
  ਪੂਰਨ err = ਮਾਪ_TMult(ਅੰਕ_2, ਮਾਪ_2, &ਕੁੱਲ_2);
  ਜੇ ((err != S_OK) || (ਕੁੱਲ_2 >= MAX_ALLOC_ਮਾਪ)) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ err = __builtin_umull_overflow(ਅੰਕ_2, ਮਾਪ_2, &ਕੁੱਲ_2);
  ਜੇ (err || (ਕੁੱਲ_2 >= MAX_ALLOC_ਮਾਪ)) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
  ਕੁੱਲ_2 = ਅੰਕ_2 * ਮਾਪ_2;
#ਜੇ_ਅੰਤ
  ਢੇਰ_t *ਢੇਰ_2 = get_thread_ਢੇਰ();
  ਖਾਲੀ *ਖੰਡ = _rpmalloc_allocate(ਢੇਰ_2, ਕੁੱਲ_2);
  ਜੇ (ਖੰਡ)
    memset(ਖੰਡ, 0, ਕੁੱਲ_2);
  ਵਾਪਸ ਖੰਡ;
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_ALLOCATOR ਖਾਲੀ *rprealloc(ਖਾਲੀ *ptr, size_t ਮਾਪ_2) {
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
  ਜੇ (ਮਾਪ_2 >= MAX_ALLOC_ਮਾਪ) {
    errno = EINVAL;
    ਵਾਪਸ ptr;
  }
#ਜੇ_ਅੰਤ
  ਢੇਰ_t *ਢੇਰ_2 = get_thread_ਢੇਰ();
  ਵਾਪਸ _rpmalloc_reallocate(ਢੇਰ_2, ptr, ਮਾਪ_2, 0, 0);
}

ਬਾਹਰੀ RPMALLOC_ALLOCATOR ਖਾਲੀ *rpaligned_realloc(ਖਾਲੀ *ptr, size_t alignment,
                                                  size_t ਮਾਪ_2, size_t oldsize,
                                                  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags) {
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
  ਜੇ ((ਮਾਪ_2 + alignment < ਮਾਪ_2) || (alignment > ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ)) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਢੇਰ_t *ਢੇਰ_2 = get_thread_ਢੇਰ();
  ਵਾਪਸ _rpmalloc_aligned_reallocate(ਢੇਰ_2, ptr, alignment, ਮਾਪ_2, oldsize,
                                      flags);
}

ਬਾਹਰੀ RPMALLOC_ALLOCATOR ਖਾਲੀ *rpaligned_alloc(size_t alignment, size_t ਮਾਪ_2) {
  ਢੇਰ_t *ਢੇਰ_2 = get_thread_ਢੇਰ();
  ਵਾਪਸ _rpmalloc_aligned_allocate(ਢੇਰ_2, alignment, ਮਾਪ_2);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpaligned_calloc(size_t alignment, size_t ਅੰਕ_2, size_t ਮਾਪ_2) {
  size_t ਕੁੱਲ_2;
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
#ਜੇ PLATFORM_WINDOWS
  ਪੂਰਨ err = ਮਾਪ_TMult(ਅੰਕ_2, ਮਾਪ_2, &ਕੁੱਲ_2);
  ਜੇ ((err != S_OK) || (ਕੁੱਲ_2 >= MAX_ALLOC_ਮਾਪ)) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ err = __builtin_umull_overflow(ਅੰਕ_2, ਮਾਪ_2, &ਕੁੱਲ_2);
  ਜੇ (err || (ਕੁੱਲ_2 >= MAX_ALLOC_ਮਾਪ)) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
  ਕੁੱਲ_2 = ਅੰਕ_2 * ਮਾਪ_2;
#ਜੇ_ਅੰਤ
  ਖਾਲੀ *ਖੰਡ = rpaligned_alloc(alignment, ਕੁੱਲ_2);
  ਜੇ (ਖੰਡ)
    memset(ਖੰਡ, 0, ਕੁੱਲ_2);
  ਵਾਪਸ ਖੰਡ;
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_ALLOCATOR ਖਾਲੀ *rpmemalign(size_t alignment,
                                                  size_t ਮਾਪ_2) {
  ਵਾਪਸ rpaligned_alloc(alignment, ਮਾਪ_2);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਪੂਰਨ rpposix_memalign(ਖਾਲੀ **memptr, size_t alignment,
                                   size_t ਮਾਪ_2) {
  ਜੇ (memptr)
    *memptr = rpaligned_alloc(alignment, ਮਾਪ_2);
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ EINVAL;
  ਵਾਪਸ *memptr ? 0 : ENOMEM;
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ size_t rpmalloc_usable_ਮਾਪ(ਖਾਲੀ *ptr) {
  ਵਾਪਸ (ptr ? rpmalloc_usable_ਮਾਪ_2(ptr) : 0);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ rpmalloc_thread_collect(ਖਾਲੀ) {}

ਖਾਲੀ rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats) {
  memset(stats, 0, ਆਕਾਰ(rpmalloc_thread_statistics_t));
  ਢੇਰ_t *ਢੇਰ_2 = get_thread_ਢੇਰ_raw();
  ਜੇ (!ਢੇਰ_2)
    ਵਾਪਸ;

  ਲਈ (size_t iclass = 0; iclass < ਮਾਪ_CLASS_ਗਿਣਤੀ; ++iclass) {
    ਮਾਪ_class_t *ਮਾਪ_class = ਸਮ੍ਰਿਤੀ_ਮਾਪ_class + iclass;
    ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = ਢੇਰ_2->ਮਾਪ_class[iclass].partial_ਵਿਸਤਾਰ;
    ਜਦੋਂ (ਵਿਸਤਾਰ) {
      size_t free_ਗਿਣਤੀ = ਵਿਸਤਾਰ->list_ਮਾਪ;
      size_t ਖੰਡ_ਗਿਣਤੀ = ਮਾਪ_class->ਖੰਡ_ਗਿਣਤੀ;
      ਜੇ (ਵਿਸਤਾਰ->free_list_ਹੱਦਬੰਦੀ < ਖੰਡ_ਗਿਣਤੀ)
        ਖੰਡ_ਗਿਣਤੀ = ਵਿਸਤਾਰ->free_list_ਹੱਦਬੰਦੀ;
      free_ਗਿਣਤੀ += (ਖੰਡ_ਗਿਣਤੀ - ਵਿਸਤਾਰ->ਵਰਤਿਆ_ਗਿਣਤੀ);
      stats->sizecache += free_ਗਿਣਤੀ * ਮਾਪ_class->ਖੰਡ_ਮਾਪ_2;
      ਵਿਸਤਾਰ = ਵਿਸਤਾਰ->ਅਗਲਾ;
    }
  }

#ਜੇ ENABLE_THREAD_CACHE
  ਲਈ (size_t iclass = 0; iclass < LARGE_CLASS_ਗਿਣਤੀ; ++iclass) {
    ਵਿਸਤਾਰ_cache_t *ਵਿਸਤਾਰ_cache;
    ਜੇ (!iclass)
      ਵਿਸਤਾਰ_cache = &ਢੇਰ_2->ਵਿਸਤਾਰ_cache;
    ਨਹੀਂਤਾਂ
      ਵਿਸਤਾਰ_cache = (ਵਿਸਤਾਰ_cache_t *)(ਢੇਰ_2->ਵਿਸਤਾਰ_large_cache + (iclass - 1));
    stats->spancache += ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 * (iclass + 1) * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ;
  }
#ਜੇ_ਅੰਤ

  ਵਿਸਤਾਰ_t *deferred = (ਵਿਸਤਾਰ_t *)atomic_ਲੱਦੋ_ptr(&ਢੇਰ_2->ਵਿਸਤਾਰ_free_deferred);
  ਜਦੋਂ (deferred) {
    ਜੇ (deferred->ਮਾਪ_class != ਮਾਪ_CLASS_HUGE)
      stats->spancache += (size_t)deferred->ਵਿਸਤਾਰ_ਗਿਣਤੀ * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ;
    deferred = (ਵਿਸਤਾਰ_t *)deferred->free_list;
  }

#ਜੇ ENABLE_STATISTICS
  stats->thread_ਮੰਜ਼ਿਲ_global = (size_t)atomic_load64(&ਢੇਰ_2->thread_ਮੰਜ਼ਿਲ_global);
  stats->global_ਮੰਜ਼ਿਲ_thread = (size_t)atomic_load64(&ਢੇਰ_2->global_ਮੰਜ਼ਿਲ_thread);

  ਲਈ (size_t iclass = 0; iclass < LARGE_CLASS_ਗਿਣਤੀ; ++iclass) {
    stats->ਵਿਸਤਾਰ_use[iclass].current =
        (size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].current);
    stats->ਵਿਸਤਾਰ_use[iclass].peak =
        (size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].ਉੱਚਾ);
    stats->ਵਿਸਤਾਰ_use[iclass].ਮੰਜ਼ਿਲ_global =
        (size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_ਮੰਜ਼ਿਲ_global);
    stats->ਵਿਸਤਾਰ_use[iclass].from_global =
        (size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_from_global);
    stats->ਵਿਸਤਾਰ_use[iclass].ਮੰਜ਼ਿਲ_cache =
        (size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_ਮੰਜ਼ਿਲ_cache);
    stats->ਵਿਸਤਾਰ_use[iclass].from_cache =
        (size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_from_cache);
    stats->ਵਿਸਤਾਰ_use[iclass].ਮੰਜ਼ਿਲ_reserved =
        (size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_ਮੰਜ਼ਿਲ_reserved);
    stats->ਵਿਸਤਾਰ_use[iclass].from_reserved =
        (size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_from_reserved);
    stats->ਵਿਸਤਾਰ_use[iclass].map_calls =
        (size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_map_calls);
  }
  ਲਈ (size_t iclass = 0; iclass < ਮਾਪ_CLASS_ਗਿਣਤੀ; ++iclass) {
    stats->ਮਾਪ_use[iclass].alloc_current =
        (size_t)atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].alloc_current);
    stats->ਮਾਪ_use[iclass].alloc_peak =
        (size_t)ਢੇਰ_2->ਮਾਪ_class_use[iclass].alloc_peak;
    stats->ਮਾਪ_use[iclass].alloc_ਕੁੱਲ =
        (size_t)atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].alloc_ਕੁੱਲ);
    stats->ਮਾਪ_use[iclass].free_ਕੁੱਲ =
        (size_t)atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].free_ਕੁੱਲ);
    stats->ਮਾਪ_use[iclass].spans_ਮੰਜ਼ਿਲ_cache =
        (size_t)atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_ਮੰਜ਼ਿਲ_cache);
    stats->ਮਾਪ_use[iclass].spans_from_cache =
        (size_t)atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_from_cache);
    stats->ਮਾਪ_use[iclass].spans_from_reserved = (size_t)atomic_load32(
        &ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_from_reserved);
    stats->ਮਾਪ_use[iclass].map_calls =
        (size_t)atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_map_calls);
  }
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats) {
  memset(stats, 0, ਆਕਾਰ(rpmalloc_global_statistics_t));
#ਜੇ ENABLE_STATISTICS
  stats->mapped = (size_t)atomic_load32(&mapped_ਪੰਨੇ) * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  stats->mapped_peak = (size_t)mapped_ਪੰਨੇ_peak * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  stats->mapped_ਕੁੱਲ_2 =
      (size_t)atomic_load32(&mapped_ਕੁੱਲ) * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  stats->unmapped_ਕੁੱਲ_2 =
      (size_t)atomic_load32(&unmapped_ਕੁੱਲ) * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  stats->huge_alloc =
      (size_t)atomic_load32(&huge_ਪੰਨੇ_current) * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  stats->huge_alloc_peak = (size_t)huge_ਪੰਨੇ_peak * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_GLOBAL_CACHE
  ਲਈ (size_t iclass = 0; iclass < LARGE_CLASS_ਗਿਣਤੀ; ++iclass) {
    global_cache_t *cache = &ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_cache[iclass];
    ਜਦੋਂ (!atomic_cas32_acquire(&cache->ਤਾਲਾ, 1, 0))
      _rpmalloc_spin();
    uint32_t ਗਿਣਤੀ_2 = cache->ਗਿਣਤੀ_2;
#ਜੇ ENABLE_UNLIMITED_CACHE
    ਵਿਸਤਾਰ_t *current_ਵਿਸਤਾਰ = cache->ਉਛਾਲ_2;
    ਜਦੋਂ (current_ਵਿਸਤਾਰ) {
      ++ਗਿਣਤੀ_2;
      current_ਵਿਸਤਾਰ = current_ਵਿਸਤਾਰ->ਅਗਲਾ;
    }
#ਜੇ_ਅੰਤ
    atomic_store32_release(&cache->ਤਾਲਾ, 0);
    stats->cached += ਗਿਣਤੀ_2 * (iclass + 1) * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ;
  }
#ਜੇ_ਅੰਤ
}

#ਜੇ ENABLE_STATISTICS

ਸਥਿਰ ਖਾਲੀ ਸਮ੍ਰਿਤੀ_ਢੇਰ_dump_statistics(ਢੇਰ_t *ਢੇਰ_2, ਖਾਲੀ *file) {
  fprintf(file, "Heap %d stats:\n", ਢੇਰ_2->id);
  fprintf(file, "Class   CurAlloc  PeakAlloc   TotAlloc    TotFree  BlkSize "
                "BlkCount SpansCur SpansPeak  PeakAllocMiB  ToCacheMiB "
                "FromCacheMiB FromReserveMiB MmapCalls\n");
  ਲਈ (size_t iclass = 0; iclass < ਮਾਪ_CLASS_ਗਿਣਤੀ; ++iclass) {
    ਜੇ (!atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].alloc_ਕੁੱਲ))
      ਜਾਰੀ;
    fprintf(
        file,
        "%3u:  %10u %10u %10u %10u %8u %8u %8d %9d %13zu %11zu %12zu %14zu "
        "%9u\n",
        (uint32_t)iclass,
        atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].alloc_current),
        ਢੇਰ_2->ਮਾਪ_class_use[iclass].alloc_peak,
        atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].alloc_ਕੁੱਲ),
        atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].free_ਕੁੱਲ),
        ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[iclass].ਖੰਡ_ਮਾਪ_2,
        ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[iclass].ਖੰਡ_ਗਿਣਤੀ,
        atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_current),
        ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_peak,
        ((size_t)ਢੇਰ_2->ਮਾਪ_class_use[iclass].alloc_peak *
         (size_t)ਸਮ੍ਰਿਤੀ_ਮਾਪ_class[iclass].ਖੰਡ_ਮਾਪ_2) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_ਮੰਜ਼ਿਲ_cache) *
         ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_from_cache) *
         ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(
             &ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_from_reserved) *
         ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) /
            (size_t)(1024 * 1024),
        atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_map_calls));
  }
  fprintf(file, "Spans  Current     Peak Deferred  PeakMiB  Cached  ToCacheMiB "
                "FromCacheMiB ToReserveMiB FromReserveMiB ToGlobalMiB "
                "FromGlobalMiB  MmapCalls\n");
  ਲਈ (size_t iclass = 0; iclass < LARGE_CLASS_ਗਿਣਤੀ; ++iclass) {
    ਜੇ (!atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].ਉੱਚਾ) &&
        !atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_map_calls))
      ਜਾਰੀ;
    fprintf(
        file,
        "%4u: %8d %8u %8u %8zu %7u %11zu %12zu %12zu %14zu %11zu %13zu %10u\n",
        (uint32_t)(iclass + 1), atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].current),
        atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].ਉੱਚਾ),
        atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_deferred),
        ((size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].ਉੱਚਾ) *
         (size_t)ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ * (iclass + 1)) /
            (size_t)(1024 * 1024),
#ਜੇ ENABLE_THREAD_CACHE
        (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(!iclass ? ਢੇਰ_2->ਵਿਸਤਾਰ_cache.ਗਿਣਤੀ_2
                               : ਢੇਰ_2->ਵਿਸਤਾਰ_large_cache[iclass - 1].ਗਿਣਤੀ_2),
        ((size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_ਮੰਜ਼ਿਲ_cache) *
         (iclass + 1) * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_from_cache) *
         (iclass + 1) * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) /
            (size_t)(1024 * 1024),
#ਨਹੀਂਤਾਂ
        0, (size_t)0, (size_t)0,
#ਜੇ_ਅੰਤ
        ((size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_ਮੰਜ਼ਿਲ_reserved) *
         (iclass + 1) * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_from_reserved) *
         (iclass + 1) * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_ਮੰਜ਼ਿਲ_global) *
         (size_t)ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ * (iclass + 1)) /
            (size_t)(1024 * 1024),
        ((size_t)atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_from_global) *
         (size_t)ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ * (iclass + 1)) /
            (size_t)(1024 * 1024),
        atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_map_calls));
  }
  fprintf(file, "Full spans: %zu\n", ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ);
  fprintf(file, "ThreadToGlobalMiB GlobalToThreadMiB\n");
  fprintf(
      file, "%17zu %17zu\n",
      (size_t)atomic_load64(&ਢੇਰ_2->thread_ਮੰਜ਼ਿਲ_global) / (size_t)(1024 * 1024),
      (size_t)atomic_load64(&ਢੇਰ_2->global_ਮੰਜ਼ਿਲ_thread) / (size_t)(1024 * 1024));
}

#ਜੇ_ਅੰਤ

ਖਾਲੀ rpmalloc_dump_statistics(ਖਾਲੀ *file) {
#ਜੇ ENABLE_STATISTICS
  ਲਈ (size_t list_idx = 0; list_idx < ਢੇਰ_ARRAY_ਮਾਪ; ++list_idx) {
    ਢੇਰ_t *ਢੇਰ_2 = ਸਮ੍ਰਿਤੀ_heaps[list_idx];
    ਜਦੋਂ (ਢੇਰ_2) {
      ਪੂਰਨ need_dump = 0;
      ਲਈ (size_t iclass = 0; !need_dump && (iclass < ਮਾਪ_CLASS_ਗਿਣਤੀ);
           ++iclass) {
        ਜੇ (!atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].alloc_ਕੁੱਲ)) {
          rpmalloc_assert(
              !atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].free_ਕੁੱਲ),
              "Heap statistics counter mismatch");
          rpmalloc_assert(
              !atomic_load32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_map_calls),
              "Heap statistics counter mismatch");
          ਜਾਰੀ;
        }
        need_dump = 1;
      }
      ਲਈ (size_t iclass = 0; !need_dump && (iclass < LARGE_CLASS_ਗਿਣਤੀ);
           ++iclass) {
        ਜੇ (!atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].ਉੱਚਾ) &&
            !atomic_load32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_map_calls))
          ਜਾਰੀ;
        need_dump = 1;
      }
      ਜੇ (need_dump)
        ਸਮ੍ਰਿਤੀ_ਢੇਰ_dump_statistics(ਢੇਰ_2, file);
      ਢੇਰ_2 = ਢੇਰ_2->ਅਗਲਾ_ਢੇਰ;
    }
  }
  fprintf(file, "Global stats:\n");
  size_t huge_current =
      (size_t)atomic_load32(&huge_ਪੰਨੇ_current) * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  size_t huge_peak = (size_t)huge_ਪੰਨੇ_peak * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  fprintf(file, "HugeCurrentMiB HugePeakMiB\n");
  fprintf(file, "%14zu %11zu\n", huge_current / (size_t)(1024 * 1024),
          huge_peak / (size_t)(1024 * 1024));

#ਜੇ ENABLE_GLOBAL_CACHE
  fprintf(file, "GlobalCacheMiB\n");
  ਲਈ (size_t iclass = 0; iclass < LARGE_CLASS_ਗਿਣਤੀ; ++iclass) {
    global_cache_t *cache = ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_cache + iclass;
    size_t global_cache = (size_t)cache->ਗਿਣਤੀ_2 * iclass * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ;

    size_t global_ਉਛਾਲ_cache = 0;
    ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = cache->ਉਛਾਲ_2;
    ਜਦੋਂ (ਵਿਸਤਾਰ) {
      global_ਉਛਾਲ_cache += iclass * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ;
      ਵਿਸਤਾਰ = ਵਿਸਤਾਰ->ਅਗਲਾ;
    }
    ਜੇ (global_cache || global_ਉਛਾਲ_cache || cache->ਪਾਓ_ਗਿਣਤੀ ||
        cache->ਕੱਢੋ_ਗਿਣਤੀ_2)
      fprintf(file,
              "%4zu: %8zuMiB (%8zuMiB overflow) %14zu insert %14zu extract\n",
              iclass + 1, global_cache / (size_t)(1024 * 1024),
              global_ਉਛਾਲ_cache / (size_t)(1024 * 1024),
              cache->ਪਾਓ_ਗਿਣਤੀ, cache->ਕੱਢੋ_ਗਿਣਤੀ_2);
  }
#ਜੇ_ਅੰਤ

  size_t mapped = (size_t)atomic_load32(&mapped_ਪੰਨੇ) * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  size_t mapped_os =
      (size_t)atomic_load32(&mapped_ਪੰਨੇ_os) * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  size_t mapped_peak = (size_t)mapped_ਪੰਨੇ_peak * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  size_t mapped_ਕੁੱਲ_2 =
      (size_t)atomic_load32(&mapped_ਕੁੱਲ) * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  size_t unmapped_ਕੁੱਲ_2 =
      (size_t)atomic_load32(&unmapped_ਕੁੱਲ) * ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
  fprintf(
      file,
      "MappedMiB MappedOSMiB MappedPeakMiB MappedTotalMiB UnmappedTotalMiB\n");
  fprintf(file, "%9zu %11zu %13zu %14zu %16zu\n",
          mapped / (size_t)(1024 * 1024), mapped_os / (size_t)(1024 * 1024),
          mapped_peak / (size_t)(1024 * 1024),
          mapped_ਕੁੱਲ_2 / (size_t)(1024 * 1024),
          unmapped_ਕੁੱਲ_2 / (size_t)(1024 * 1024));

  fprintf(file, "\n");
#ਜੇ 0
	int64_t allocated = atomic_load64(&ਵੰਡ_counter);
	int64_t deallocated = atomic_load64(&_deallocation_counter);
	fprintf(file, "Allocation count: %lli\n", allocated);
	fprintf(file, "Deallocation count: %lli\n", deallocated);
	fprintf(file, "Current allocations: %lli\n", (allocated - deallocated));
	fprintf(file, "Master spans: %d\n", atomic_load32(&_master_spans));
	fprintf(file, "Dangling master spans: %d\n", atomic_load32(&_unmapped_master_spans));
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
  (ਖਾਲੀ)ਆਕਾਰ(file);
}

#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS

ਬਾਹਰੀ ਸਤਰਅੰਦਰ rpmalloc_ਢੇਰ_t *rpmalloc_ਢੇਰ_acquire(ਖਾਲੀ) {
  // Must be a pristine heap from newly mapped memory pages, or else memory
  // blocks could already be allocated from the heap which would (wrongly) be
  // released when heap is cleared with rpmalloc_heap_free_all(). Also heaps
  // guaranteed to be pristine from the dedicated orphan list can be used.
  ਢੇਰ_t *ਢੇਰ_2 = rpmalloc_ਢੇਰ_allocate(1);
  rpmalloc_assume(ਢੇਰ_2 != ਨਿਰਾ);
  ਢੇਰ_2->owner_thread = 0;
  _rpmalloc_stat_inc(&ਸਮ੍ਰਿਤੀ_ਸਰਗਰਮ_heaps);
  ਵਾਪਸ ਢੇਰ_2;
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ rpmalloc_ਢੇਰ_release_2(rpmalloc_ਢੇਰ_t *ਢੇਰ_2) {
  ਜੇ (ਢੇਰ_2)
    rpmalloc_ਢੇਰ_release(ਢੇਰ_2, 1, 1);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmalloc_ਢੇਰ_alloc(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, size_t ਮਾਪ_2) {
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
  ਜੇ (ਮਾਪ_2 >= MAX_ALLOC_ਮਾਪ) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ _rpmalloc_allocate(ਢੇਰ_2, ਮਾਪ_2);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmalloc_ਢੇਰ_aligned_alloc(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, size_t alignment,
                            size_t ਮਾਪ_2) {
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
  ਜੇ (ਮਾਪ_2 >= MAX_ALLOC_ਮਾਪ) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ _rpmalloc_aligned_allocate(ਢੇਰ_2, alignment, ਮਾਪ_2);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmalloc_ਢੇਰ_calloc(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, size_t ਅੰਕ_2, size_t ਮਾਪ_2) {
  ਵਾਪਸ rpmalloc_ਢੇਰ_aligned_calloc(ਢੇਰ_2, 0, ਅੰਕ_2, ਮਾਪ_2);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmalloc_ਢੇਰ_aligned_calloc(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, size_t alignment,
                             size_t ਅੰਕ_2, size_t ਮਾਪ_2) {
  size_t ਕੁੱਲ_2;
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
#ਜੇ PLATFORM_WINDOWS
  ਪੂਰਨ err = ਮਾਪ_TMult(ਅੰਕ_2, ਮਾਪ_2, &ਕੁੱਲ_2);
  ਜੇ ((err != S_OK) || (ਕੁੱਲ_2 >= MAX_ALLOC_ਮਾਪ)) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ err = __builtin_umull_overflow(ਅੰਕ_2, ਮਾਪ_2, &ਕੁੱਲ_2);
  ਜੇ (err || (ਕੁੱਲ_2 >= MAX_ALLOC_ਮਾਪ)) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
  ਕੁੱਲ_2 = ਅੰਕ_2 * ਮਾਪ_2;
#ਜੇ_ਅੰਤ
  ਖਾਲੀ *ਖੰਡ = _rpmalloc_aligned_allocate(ਢੇਰ_2, alignment, ਕੁੱਲ_2);
  ਜੇ (ਖੰਡ)
    memset(ਖੰਡ, 0, ਕੁੱਲ_2);
  ਵਾਪਸ ਖੰਡ;
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmalloc_ਢੇਰ_realloc(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, ਖਾਲੀ *ptr, size_t ਮਾਪ_2,
                      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags) {
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
  ਜੇ (ਮਾਪ_2 >= MAX_ALLOC_ਮਾਪ) {
    errno = EINVAL;
    ਵਾਪਸ ptr;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ _rpmalloc_reallocate(ਢੇਰ_2, ptr, ਮਾਪ_2, 0, flags);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmalloc_ਢੇਰ_aligned_realloc(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, ਖਾਲੀ *ptr,
                              size_t alignment, size_t ਮਾਪ_2,
                              ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags) {
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
  ਜੇ ((ਮਾਪ_2 + alignment < ਮਾਪ_2) || (alignment > ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ)) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ _rpmalloc_aligned_reallocate(ਢੇਰ_2, ptr, alignment, ਮਾਪ_2, 0, flags);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ rpmalloc_ਢੇਰ_free(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, ਖਾਲੀ *ptr) {
  (ਖਾਲੀ)ਆਕਾਰ(ਢੇਰ_2);
  _rpmalloc_deallocate(ptr);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ rpmalloc_ਢੇਰ_free_all(rpmalloc_ਢੇਰ_t *ਢੇਰ_2) {
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ;
  ਵਿਸਤਾਰ_t *ਅਗਲਾ_ਵਿਸਤਾਰ;

  rpmalloc_ਢੇਰ_cache_adopt_deferred(ਢੇਰ_2, 0);

  ਲਈ (size_t iclass = 0; iclass < ਮਾਪ_CLASS_ਗਿਣਤੀ; ++iclass) {
    ਵਿਸਤਾਰ = ਢੇਰ_2->ਮਾਪ_class[iclass].partial_ਵਿਸਤਾਰ;
    ਜਦੋਂ (ਵਿਸਤਾਰ) {
      ਅਗਲਾ_ਵਿਸਤਾਰ = ਵਿਸਤਾਰ->ਅਗਲਾ;
      rpmalloc_ਢੇਰ_cache_ਪਾਓ(ਢੇਰ_2, ਵਿਸਤਾਰ);
      ਵਿਸਤਾਰ = ਅਗਲਾ_ਵਿਸਤਾਰ;
    }
    ਢੇਰ_2->ਮਾਪ_class[iclass].partial_ਵਿਸਤਾਰ = 0;
    ਵਿਸਤਾਰ = ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ[iclass];
    ਜਦੋਂ (ਵਿਸਤਾਰ) {
      ਅਗਲਾ_ਵਿਸਤਾਰ = ਵਿਸਤਾਰ->ਅਗਲਾ;
      rpmalloc_ਢੇਰ_cache_ਪਾਓ(ਢੇਰ_2, ਵਿਸਤਾਰ);
      ਵਿਸਤਾਰ = ਅਗਲਾ_ਵਿਸਤਾਰ;
    }

    ਵਿਸਤਾਰ = ਢੇਰ_2->ਮਾਪ_class[iclass].cache;
    ਜੇ (ਵਿਸਤਾਰ)
      rpmalloc_ਢੇਰ_cache_ਪਾਓ(ਢੇਰ_2, ਵਿਸਤਾਰ);
    ਢੇਰ_2->ਮਾਪ_class[iclass].cache = 0;
  }
  memset(ਢੇਰ_2->ਮਾਪ_class, 0, ਆਕਾਰ(ਢੇਰ_2->ਮਾਪ_class));
  memset(ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ, 0, ਆਕਾਰ(ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ));

  ਵਿਸਤਾਰ = ਢੇਰ_2->large_huge_ਵਿਸਤਾਰ;
  ਜਦੋਂ (ਵਿਸਤਾਰ) {
    ਅਗਲਾ_ਵਿਸਤਾਰ = ਵਿਸਤਾਰ->ਅਗਲਾ;
    ਜੇ (UNEXPECTED(ਵਿਸਤਾਰ->ਮਾਪ_class == ਮਾਪ_CLASS_HUGE))
      _rpmalloc_deallocate_huge(ਵਿਸਤਾਰ);
    ਨਹੀਂਤਾਂ
      rpmalloc_ਢੇਰ_cache_ਪਾਓ(ਢੇਰ_2, ਵਿਸਤਾਰ);
    ਵਿਸਤਾਰ = ਅਗਲਾ_ਵਿਸਤਾਰ;
  }
  ਢੇਰ_2->large_huge_ਵਿਸਤਾਰ = 0;
  ਢੇਰ_2->ਭਰਿਆ_ਵਿਸਤਾਰ_ਗਿਣਤੀ = 0;

#ਜੇ ENABLE_THREAD_CACHE
  ਲਈ (size_t iclass = 0; iclass < LARGE_CLASS_ਗਿਣਤੀ; ++iclass) {
    ਵਿਸਤਾਰ_cache_t *ਵਿਸਤਾਰ_cache;
    ਜੇ (!iclass)
      ਵਿਸਤਾਰ_cache = &ਢੇਰ_2->ਵਿਸਤਾਰ_cache;
    ਨਹੀਂਤਾਂ
      ਵਿਸਤਾਰ_cache = (ਵਿਸਤਾਰ_cache_t *)(ਢੇਰ_2->ਵਿਸਤਾਰ_large_cache + (iclass - 1));
    ਜੇ (!ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2)
      ਜਾਰੀ;
#ਜੇ ENABLE_GLOBAL_CACHE
    _rpmalloc_stat_add64(&ਢੇਰ_2->thread_ਮੰਜ਼ਿਲ_global,
                         ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 * (iclass + 1) * ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਮਾਪ);
    rpmalloc_stat_ਜੋੜੋ(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].spans_ਮੰਜ਼ਿਲ_global,
                       ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2);
    rpmalloc_global_cache_ਪਾਓ_spans(ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ, iclass + 1,
                                        ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2);
#ਨਹੀਂਤਾਂ
    ਲਈ (size_t ispan = 0; ispan < ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2; ++ispan)
      rpmalloc_ਵਿਸਤਾਰ_unmap(ਵਿਸਤਾਰ_cache->ਵਿਸਤਾਰ[ispan]);
#ਜੇ_ਅੰਤ
    ਵਿਸਤਾਰ_cache->ਗਿਣਤੀ_2 = 0;
  }
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_STATISTICS
  ਲਈ (size_t iclass = 0; iclass < ਮਾਪ_CLASS_ਗਿਣਤੀ; ++iclass) {
    atomic_store32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].alloc_current, 0);
    atomic_store32(&ਢੇਰ_2->ਮਾਪ_class_use[iclass].spans_current, 0);
  }
  ਲਈ (size_t iclass = 0; iclass < LARGE_CLASS_ਗਿਣਤੀ; ++iclass) {
    atomic_store32(&ਢੇਰ_2->ਵਿਸਤਾਰ_use[iclass].current, 0);
  }
#ਜੇ_ਅੰਤ
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ rpmalloc_ਢੇਰ_thread_ਸੈੱਟ_current(rpmalloc_ਢੇਰ_t *ਢੇਰ_2) {
  ਢੇਰ_t *prev_ਢੇਰ = get_thread_ਢੇਰ_raw();
  ਜੇ (prev_ਢੇਰ != ਢੇਰ_2) {
    ਸੈੱਟ_thread_ਢੇਰ(ਢੇਰ_2);
    ਜੇ (prev_ਢੇਰ)
      rpmalloc_ਢੇਰ_release_2(prev_ਢੇਰ);
  }
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ rpmalloc_ਢੇਰ_t *rpmalloc_get_ਢੇਰ_for_ptr(ਖਾਲੀ *ptr) {
  // Grab the span, and then the heap from the span
  ਵਿਸਤਾਰ_t *ਵਿਸਤਾਰ = (ਵਿਸਤਾਰ_t *)((uintptr_t)ptr & ਸਮ੍ਰਿਤੀ_ਵਿਸਤਾਰ_ਨਕਾਬ);
  ਜੇ (ਵਿਸਤਾਰ) {
    ਵਾਪਸ ਵਿਸਤਾਰ->ਢੇਰ_2;
  }
  ਵਾਪਸ 0;
}

#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_PRELOAD || ENABLE_OVERRIDE

#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.c"

#ਜੇ_ਅੰਤ

ਖਾਲੀ rpmalloc_linker_reference(ਖਾਲੀ) { (ਖਾਲੀ)ਆਕਾਰ(_rpmalloc_initialized); }
