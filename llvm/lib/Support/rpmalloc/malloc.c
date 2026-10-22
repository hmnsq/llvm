//===------------------------ malloc.c ------------------*- C -*-=============//
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
//
// This file provides overrides for the standard library malloc entry points for
// C and new/delete operators for C++ It also provides automatic
// initialization/finalization of process and threads
//
//===----------------------------------------------------------------------===//

#اگر defined(__TINYC__)
#شامل <sys/types.h>
#endif

#ifndef ARCH_64BIT
#اگر defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#define ARCH_64BIT 1
_Static_assert(اندازه(size_t) == 8, "Data type size mismatch");
_Static_assert(اندازه(تهی *) == 8, "Data type size mismatch");
#وگرنه
#define ARCH_64BIT 0
_Static_assert(اندازه(size_t) == 4, "Data type size mismatch");
_Static_assert(اندازه(تهی *) == 4, "Data type size mismatch");
#endif
#endif

#اگر (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility افزودن(پیشفرض)
#endif

#define USE_IMPLEMENT 1
#define USE_INTERPOSE 0
#define USE_ALIAS 0

#اگر defined(__APPLE__)
#undef USE_INTERPOSE
#define USE_INTERPOSE 1

تعریفنوع ساختار interpose_t {
  تهی *new_func;
  تهی *orig_func;
} interpose_t;

#define MAC_INTERPOSE_PAIR(newf, oldf) {(تهی *)newf, (تهی *)oldf}
#define MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((used)) ایستا ثابت interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#endif

#اگر !defined(_WIN32) && !defined(__APPLE__)
#undef USE_IMPLEMENT
#undef USE_ALIAS
#define USE_IMPLEMENT 0
#define USE_ALIAS 1
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#undef malloc
#undef آزاد
#undef calloc
#define RPMALLOC_RESTRICT __declspec(restrict)
#وگرنه
#define RPMALLOC_RESTRICT
#endif

#اگر ENABLE_OVERRIDE

تعریفنوع ساختار rp_nothrow_t {
  صحیح __dummy;
} rp_nothrow_t;

#اگر USE_IMPLEMENT

بیرونی درونخطی RPMALLOC_RESTRICT تهی *RPMALLOC_CDECL malloc(size_t size) {
  بازگشت rpmalloc(size);
}
بیرونی درونخطی RPMALLOC_RESTRICT تهی *RPMALLOC_CDECL calloc(size_t شمار,
                                                            size_t size) {
  بازگشت rpcalloc(شمار, size);
}
بیرونی درونخطی RPMALLOC_RESTRICT تهی *RPMALLOC_CDECL realloc(تهی *ptr,
                                                             size_t size) {
  بازگشت rprealloc(ptr, size);
}
بیرونی درونخطی تهی *RPMALLOC_CDECL reallocf(تهی *ptr, size_t size) {
  بازگشت rprealloc(ptr, size);
}
بیرونی درونخطی تهی *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t size) {
  بازگشت rpaligned_alloc(alignment, size);
}
بیرونی درونخطی تهی *RPMALLOC_CDECL memalign(size_t alignment, size_t size) {
  بازگشت rpmemalign(alignment, size);
}
بیرونی درونخطی صحیح RPMALLOC_CDECL posix_memalign(تهی **memptr, size_t alignment,
                                                size_t size) {
  بازگشت rpposix_memalign(memptr, alignment, size);
}
بیرونی درونخطی تهی RPMALLOC_CDECL آزاد(تهی *ptr) { rpfree(ptr); }
بیرونی درونخطی تهی RPMALLOC_CDECL cfree(تهی *ptr) { rpfree(ptr); }
بیرونی درونخطی size_t RPMALLOC_CDECL malloc_usable_size(تهی *ptr) {
  بازگشت rpmalloc_usable_size(ptr);
}
بیرونی درونخطی size_t RPMALLOC_CDECL malloc_size(تهی *ptr) {
  بازگشت rpmalloc_usable_size(ptr);
}

#ifdef _WIN32
بیرونی درونخطی RPMALLOC_RESTRICT تهی *RPMALLOC_CDECL _malloc_base(size_t size) {
  بازگشت rpmalloc(size);
}
بیرونی درونخطی تهی RPMALLOC_CDECL _free_base(تهی *ptr) { rpfree(ptr); }
بیرونی درونخطی RPMALLOC_RESTRICT تهی *RPMALLOC_CDECL _calloc_base(size_t شمار,
                                                                  size_t size) {
  بازگشت rpcalloc(شمار, size);
}
بیرونی درونخطی size_t RPMALLOC_CDECL _msize(تهی *ptr) {
  بازگشت rpmalloc_usable_size(ptr);
}
بیرونی درونخطی size_t RPMALLOC_CDECL _msize_base(تهی *ptr) {
  بازگشت rpmalloc_usable_size(ptr);
}
بیرونی درونخطی RPMALLOC_RESTRICT تهی *RPMALLOC_CDECL
_realloc_base(تهی *ptr, size_t size) {
  بازگشت rprealloc(ptr, size);
}
#endif

#ifdef _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#وگرنه
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#define RPDEFVIS __attribute__((visibility("default")))
بیرونی تهی _ZdlPv(تهی *p);
تهی RPDEFVIS _ZdlPv(تهی *p) { rpfree(p); }
بیرونی تهی _ZdaPv(تهی *p);
تهی RPDEFVIS _ZdaPv(تهی *p) { rpfree(p); }
#اگر ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
بیرونی تهی *_Znwm(uint64_t size);
تهی *RPDEFVIS _Znwm(uint64_t size) { بازگشت rpmalloc(size); }
بیرونی تهی *_Znam(uint64_t size);
تهی *RPDEFVIS _Znam(uint64_t size) { بازگشت rpmalloc(size); }
بیرونی تهی *_Znwmm(uint64_t size, uint64_t align);
تهی *RPDEFVIS _Znwmm(uint64_t size, uint64_t align) {
  بازگشت rpaligned_alloc(align, size);
}
بیرونی تهی *_Znamm(uint64_t size, uint64_t align);
تهی *RPDEFVIS _Znamm(uint64_t size, uint64_t align) {
  بازگشت rpaligned_alloc(align, size);
}
بیرونی تهی *_ZnwmSt11align_val_t(uint64_t size, uint64_t align);
تهی *RPDEFVIS _ZnwmSt11align_val_t(uint64_t size, uint64_t align) {
  بازگشت rpaligned_alloc(align, size);
}
بیرونی تهی *_ZnamSt11align_val_t(uint64_t size, uint64_t align);
تهی *RPDEFVIS _ZnamSt11align_val_t(uint64_t size, uint64_t align) {
  بازگشت rpaligned_alloc(align, size);
}
بیرونی تهی *_ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
تهی *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (تهی)اندازه(t);
  بازگشت rpmalloc(size);
}
بیرونی تهی *_ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
تهی *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (تهی)اندازه(t);
  بازگشت rpmalloc(size);
}
بیرونی تهی *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
تهی *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (تهی)اندازه(t);
  بازگشت rpaligned_alloc(align, size);
}
بیرونی تهی *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
تهی *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (تهی)اندازه(t);
  بازگشت rpaligned_alloc(align, size);
}
// 64-bit operators sized delete and delete[], normal and aligned
بیرونی تهی _ZdlPvm(تهی *p, uint64_t size);
تهی RPDEFVIS _ZdlPvm(تهی *p, uint64_t size) {
  rpfree(p);
  (تهی)اندازه(size);
}
بیرونی تهی _ZdaPvm(تهی *p, uint64_t size);
تهی RPDEFVIS _ZdaPvm(تهی *p, uint64_t size) {
  rpfree(p);
  (تهی)اندازه(size);
}
بیرونی تهی _ZdlPvSt11align_val_t(تهی *p, uint64_t align);
تهی RPDEFVIS _ZdlPvSt11align_val_t(تهی *p, uint64_t align) {
  rpfree(p);
  (تهی)اندازه(align);
}
بیرونی تهی _ZdaPvSt11align_val_t(تهی *p, uint64_t align);
تهی RPDEFVIS _ZdaPvSt11align_val_t(تهی *p, uint64_t align) {
  rpfree(p);
  (تهی)اندازه(align);
}
بیرونی تهی _ZdlPvmSt11align_val_t(تهی *p, uint64_t size, uint64_t align);
تهی RPDEFVIS _ZdlPvmSt11align_val_t(تهی *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (تهی)اندازه(size);
  (تهی)اندازه(align);
}
بیرونی تهی _ZdaPvmSt11align_val_t(تهی *p, uint64_t size, uint64_t align);
تهی RPDEFVIS _ZdaPvmSt11align_val_t(تهی *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (تهی)اندازه(size);
  (تهی)اندازه(align);
}
#وگرنه
// 32-bit operators new and new[], normal and aligned
بیرونی تهی *_Znwj(uint32_t size);
تهی *RPDEFVIS _Znwj(uint32_t size) { بازگشت rpmalloc(size); }
بیرونی تهی *_Znaj(uint32_t size);
تهی *RPDEFVIS _Znaj(uint32_t size) { بازگشت rpmalloc(size); }
بیرونی تهی *_Znwjj(uint32_t size, uint32_t align);
تهی *RPDEFVIS _Znwjj(uint32_t size, uint32_t align) {
  بازگشت rpaligned_alloc(align, size);
}
بیرونی تهی *_Znajj(uint32_t size, uint32_t align);
تهی *RPDEFVIS _Znajj(uint32_t size, uint32_t align) {
  بازگشت rpaligned_alloc(align, size);
}
بیرونی تهی *_ZnwjSt11align_val_t(size_t size, size_t align);
تهی *RPDEFVIS _ZnwjSt11align_val_t(size_t size, size_t align) {
  بازگشت rpaligned_alloc(align, size);
}
بیرونی تهی *_ZnajSt11align_val_t(size_t size, size_t align);
تهی *RPDEFVIS _ZnajSt11align_val_t(size_t size, size_t align) {
  بازگشت rpaligned_alloc(align, size);
}
بیرونی تهی *_ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t);
تهی *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (تهی)اندازه(t);
  بازگشت rpmalloc(size);
}
بیرونی تهی *_ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t);
تهی *RPDEFVIS _ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (تهی)اندازه(t);
  بازگشت rpmalloc(size);
}
بیرونی تهی *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
تهی *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (تهی)اندازه(t);
  بازگشت rpaligned_alloc(align, size);
}
بیرونی تهی *_ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
تهی *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (تهی)اندازه(t);
  بازگشت rpaligned_alloc(align, size);
}
// 32-bit operators sized delete and delete[], normal and aligned
بیرونی تهی _ZdlPvj(تهی *p, uint64_t size);
تهی RPDEFVIS _ZdlPvj(تهی *p, uint64_t size) {
  rpfree(p);
  (تهی)اندازه(size);
}
بیرونی تهی _ZdaPvj(تهی *p, uint64_t size);
تهی RPDEFVIS _ZdaPvj(تهی *p, uint64_t size) {
  rpfree(p);
  (تهی)اندازه(size);
}
بیرونی تهی _ZdlPvSt11align_val_t(تهی *p, uint32_t align);
تهی RPDEFVIS _ZdlPvSt11align_val_t(تهی *p, uint64_t a) {
  rpfree(p);
  (تهی)اندازه(align);
}
بیرونی تهی _ZdaPvSt11align_val_t(تهی *p, uint32_t align);
تهی RPDEFVIS _ZdaPvSt11align_val_t(تهی *p, uint64_t a) {
  rpfree(p);
  (تهی)اندازه(align);
}
بیرونی تهی _ZdlPvjSt11align_val_t(تهی *p, uint32_t size, uint32_t align);
تهی RPDEFVIS _ZdlPvjSt11align_val_t(تهی *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (تهی)اندازه(size);
  (تهی)اندازه(a);
}
بیرونی تهی _ZdaPvjSt11align_val_t(تهی *p, uint32_t size, uint32_t align);
تهی RPDEFVIS _ZdaPvjSt11align_val_t(تهی *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (تهی)اندازه(size);
  (تهی)اندازه(a);
}
#endif
#endif
#endif

#اگر USE_INTERPOSE || USE_ALIAS

ایستا تهی *rpmalloc_nothrow(size_t size, rp_nothrow_t t) {
  (تهی)اندازه(t);
  بازگشت rpmalloc(size);
}
ایستا تهی *rpaligned_alloc_reverse(size_t size, size_t align) {
  بازگشت rpaligned_alloc(align, size);
}
ایستا تهی *rpaligned_alloc_reverse_nothrow(size_t size, size_t align,
                                             rp_nothrow_t t) {
  (تهی)اندازه(t);
  بازگشت rpaligned_alloc(align, size);
}
ایستا تهی rpfree_size(تهی *p, size_t size) {
  (تهی)اندازه(size);
  rpfree(p);
}
ایستا تهی rpfree_aligned(تهی *p, size_t align) {
  (تهی)اندازه(align);
  rpfree(p);
}
ایستا تهی rpfree_size_aligned(تهی *p, size_t size, size_t align) {
  (تهی)اندازه(size);
  (تهی)اندازه(align);
  rpfree(p);
}

#endif

#اگر USE_INTERPOSE

__attribute__((used)) ایستا ثابت interpose_t macinterpose_malloc[]
    __attribute__((section("__DATA, __interpose"))) = {
        // new and new[]
        MAC_INTERPOSE_PAIR(rpmalloc, _Znwm),
        MAC_INTERPOSE_PAIR(rpmalloc, _Znam),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_reverse, _Znwmm),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_reverse, _Znamm),
        MAC_INTERPOSE_PAIR(rpmalloc_nothrow, _ZnwmRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpmalloc_nothrow, _ZnamRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_reverse, _ZnwmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_reverse, _ZnamSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_reverse_nothrow,
                           _ZnwmSt11align_val_tRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_reverse_nothrow,
                           _ZnamSt11align_val_tRKSt9nothrow_t),
        // delete and delete[]
        MAC_INTERPOSE_PAIR(rpfree, _ZdlPv), MAC_INTERPOSE_PAIR(rpfree, _ZdaPv),
        MAC_INTERPOSE_PAIR(rpfree_size, _ZdlPvm),
        MAC_INTERPOSE_PAIR(rpfree_size, _ZdaPvm),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdlPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdaPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_size_aligned, _ZdlPvmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_size_aligned, _ZdaPvmSt11align_val_t),
        // libc entry points
        MAC_INTERPOSE_PAIR(rpmalloc, malloc),
        MAC_INTERPOSE_PAIR(rpmalloc, calloc),
        MAC_INTERPOSE_PAIR(rprealloc, realloc),
        MAC_INTERPOSE_PAIR(rprealloc, reallocf),
#اگر defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#endif
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, آزاد), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_usable_size),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_size)};

#endif

#اگر USE_ALIAS

#define RPALIAS(fn) __attribute__((alias(#fn), used, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
تهی _ZdlPv(تهی *p) RPALIAS(rpfree) تهی _ZdaPv(تهی *p) RPALIAS(rpfree)

#اگر ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    تهی *_Znwm(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) تهی *_Znam(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) تهی *_Znwmm(uint64_t size,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) تهی *_Znamm(uint64_t size,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) تهی *_ZnwmSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) تهی *_ZnamSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) تهی *_ZnwmRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) تهی *_ZnamRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) تهی
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) تهی
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    تهی _ZdlPvm(تهی *p, size_t n) RPALIAS(rpfree_size) تهی _ZdaPvm(تهی *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) تهی _ZdlPvSt11align_val_t(تهی *p, size_t a)
            RPALIAS(rpfree_aligned) تهی _ZdaPvSt11align_val_t(تهی *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) تهی _ZdlPvmSt11align_val_t(تهی *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) تهی _ZdaPvmSt11align_val_t(
                        تهی *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#وگرنه
    // 32-bit operators new and new[], normal and aligned
    تهی *_Znwj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) تهی *_Znaj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) تهی *_Znwjj(uint32_t size,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) تهی *_Znajj(uint32_t size,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) تهی *_ZnwjSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) تهی *_ZnajSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) تهی *_ZnwjRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) تهی *_ZnajRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) تهی
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) تهی
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    تهی _ZdlPvj(تهی *p, size_t n) RPALIAS(rpfree_size) تهی _ZdaPvj(تهی *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) تهی _ZdlPvSt11align_val_t(تهی *p, size_t a)
            RPALIAS(rpfree_aligned) تهی _ZdaPvSt11align_val_t(تهی *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) تهی _ZdlPvjSt11align_val_t(تهی *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) تهی _ZdaPvjSt11align_val_t(
                        تهی *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#endif

                            تهی *malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) تهی *calloc(size_t شمار, size_t size)
            RPALIAS(rpcalloc) تهی *realloc(تهی *ptr, size_t size)
                RPALIAS(rprealloc) تهی *reallocf(تهی *ptr, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rprealloc) تهی *aligned_alloc(size_t alignment, size_t size)
            RPALIAS(rpaligned_alloc) تهی *memalign(
                size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) صحیح posix_memalign(تهی **memptr, size_t alignment,
                                               size_t size)
            RPALIAS(rpposix_memalign) تهی آزاد(تهی *ptr)
                RPALIAS(rpfree) تهی cfree(تهی *ptr) RPALIAS(rpfree)
#اگر defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_size(ثابت تهی *ptr) RPALIAS(rpmalloc_usable_size)
#وگرنه
                    size_t
    malloc_usable_size(تهی *ptr) RPALIAS(rpmalloc_usable_size)
#endif
        size_t malloc_size(تهی *ptr) RPALIAS(rpmalloc_usable_size)

#endif

            ایستا درونخطی size_t _rpmalloc_page_size(تهی) {
  بازگشت _memory_page_size;
}

بیرونی تهی *RPMALLOC_CDECL reallocarray(تهی *ptr, size_t شمار, size_t size);

بیرونی تهی *RPMALLOC_CDECL reallocarray(تهی *ptr, size_t شمار, size_t size) {
  size_t total;
#اگر ENABLE_VALIDATE_ARGS
#ifdef _MSC_VER
  صحیح err = SizeTMult(شمار, size, &total);
  اگر ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    بازگشت 0;
  }
#وگرنه
  صحیح err = __builtin_umull_overflow(شمار, size, &total);
  اگر (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    بازگشت 0;
  }
#endif
#وگرنه
  total = شمار * size;
#endif
  بازگشت realloc(ptr, total);
}

بیرونی درونخطی تهی *RPMALLOC_CDECL valloc(size_t size) {
  get_thread_heap();
  بازگشت rpaligned_alloc(_rpmalloc_page_size(), size);
}

بیرونی درونخطی تهی *RPMALLOC_CDECL pvalloc(size_t size) {
  get_thread_heap();
  ثابت size_t page_size = _rpmalloc_page_size();
  ثابت size_t aligned_size = ((size + page_size - 1) / page_size) * page_size;
#اگر ENABLE_VALIDATE_ARGS
  اگر (aligned_size < size) {
    errno = EINVAL;
    بازگشت 0;
  }
#endif
  بازگشت rpaligned_alloc(_rpmalloc_page_size(), aligned_size);
}

#endif // ENABLE_OVERRIDE

#اگر ENABLE_PRELOAD

#ifdef _WIN32

#اگر defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

بیرونی __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

بیرونی __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (تهی)اندازه(reserved);
  (تهی)اندازه(instance);
  اگر (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  وگرنه اگر (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  وگرنه اگر (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  وگرنه اگر (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  بازگشت TRUE;
}

// end BUILD_DYNAMIC_LINK
#وگرنه

بیرونی تهی _global_rpmalloc_init(تهی) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
}

#اگر defined(__clang__) || defined(__GNUC__)

ایستا تهی __attribute__((constructor)) initializer(تهی) {
  _global_rpmalloc_init();
}

#وگرنهاگر defined(_MSC_VER)

ایستا صحیح _global_rpmalloc_xib(تهی) {
  _global_rpmalloc_init();
  بازگشت 0;
}

#pragma section(".CRT$XIB", اقرأ)
__declspec(allocate(".CRT$XIB")) تهی (*_rpmalloc_module_init)(تهی) =
    _global_rpmalloc_xib;
#اگر defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#وگرنه
#pragma comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#endif

#endif

// end !BUILD_DYNAMIC_LINK
#endif

#وگرنه

#شامل <pthread.h>
#شامل <stdint.h>
#شامل <stdlib.h>
#شامل <unistd.h>

بیرونی تهی rpmalloc_set_main_thread(تهی);

ایستا pthread_key_t destructor_کلید;

ایستا تهی thread_destructor(تهی *);

ایستا تهی __attribute__((constructor)) initializer(تهی) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
  pthread_کلید_create(&destructor_کلید, thread_destructor);
}

ایستا تهی __attribute__((destructor)) finalizer(تهی) { rpmalloc_finalize(); }

تعریفنوع ساختار {
  تهی *(*real_start)(تهی *);
  تهی *real_arg;
} thread_starter_arg;

ایستا تهی *thread_starter(تهی *argptr) {
  thread_starter_arg *arg = argptr;
  تهی *(*real_start)(تهی *) = arg->real_start;
  تهی *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_کلید, (تهی *)1);
  بازگشت (*real_start)(real_arg);
}

ایستا تهی thread_destructor(تهی *مقدار_2) {
  (تهی)اندازه(مقدار_2);
  rpmalloc_thread_finalize(1);
}

#ifdef __APPLE__

ایستا صحیح pthread_create_proxy(pthread_t *thread, ثابت pthread_attr_t *attr,
                                تهی *(*start_routine)(تهی *), تهی *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(اندازه(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  بازگشت pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#وگرنه

#شامل <dlfcn.h>

صحیح pthread_create(pthread_t *thread, ثابت pthread_attr_t *attr,
                   تهی *(*start_routine)(تهی *), تهی *arg) {
#اگر defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  نویسه fname[] = "pthread_create";
#وگرنه
  نویسه fname[] = "_pthread_create";
#endif
  تهی *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(اندازه(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  بازگشت (*(صحیح (*)(pthread_t *, ثابت pthread_attr_t *, تهی *(*)(تهی *),
                    تهی *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#endif

#endif

#endif

#اگر ENABLE_OVERRIDE

#اگر defined(__GLIBC__) && defined(__linux__)

تهی *__libc_malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) تهی *__libc_calloc(size_t شمار, size_t size)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) تهی *__libc_realloc(تهی *p, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2) RPALIAS(rprealloc) تهی __libc_free(تهی *p)
        RPALIAS(rpfree) تهی __libc_cfree(تهی *p)
            RPALIAS(rpfree) تهی *__libc_memalign(size_t align, size_t size)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) صحیح __posix_memalign(تهی **p, size_t align,
                                                 size_t size)
            RPALIAS(rpposix_memalign)

                بیرونی تهی *__libc_valloc(size_t size);
بیرونی تهی *__libc_pvalloc(size_t size);

تهی *__libc_valloc(size_t size) { بازگشت valloc(size); }

تهی *__libc_pvalloc(size_t size) { بازگشت pvalloc(size); }

#endif

#endif

#اگر (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility برداشت
#endif
