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

#যদি defined(__TINYC__)
#অন্তর্ভাব <sys/types.h>
#endif

#ifndef ARCH_64BIT
#যদি defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#define ARCH_64BIT 1
_Static_assert(পরিমাণ(size_t) == 8, "Data type size mismatch");
_Static_assert(পরিমাণ(শূন্য *) == 8, "Data type size mismatch");
#অন্যথা
#define ARCH_64BIT 0
_Static_assert(পরিমাণ(size_t) == 4, "Data type size mismatch");
_Static_assert(পরিমাণ(শূন্য *) == 4, "Data type size mismatch");
#endif
#endif

#যদি (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility ঠেলো(স্বভাব)
#endif

#define USE_IMPLEMENT 1
#define USE_INTERPOSE 0
#define USE_ALIAS 0

#যদি defined(__APPLE__)
#undef USE_INTERPOSE
#define USE_INTERPOSE 1

প্রকারনির্দেশ সংরচনা interpose_t {
  শূন্য *new_func;
  শূন্য *orig_func;
} interpose_t;

#define MAC_INTERPOSE_PAIR(newf, oldf) {(শূন্য *)newf, (শূন্য *)oldf}
#define MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((used)) স্থির ধ্রুব interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#endif

#যদি !defined(_WIN32) && !defined(__APPLE__)
#undef USE_IMPLEMENT
#undef USE_ALIAS
#define USE_IMPLEMENT 0
#define USE_ALIAS 1
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#undef malloc
#undef মুক্ত
#undef calloc
#define RPMALLOC_RESTRICT __declspec(restrict)
#অন্যথা
#define RPMALLOC_RESTRICT
#endif

#যদি ENABLE_OVERRIDE

প্রকারনির্দেশ সংরচনা rp_nothrow_t {
  পূর্ণাঙ্ক __dummy;
} rp_nothrow_t;

#যদি USE_IMPLEMENT

বাহ্য অন্তঃস্থ RPMALLOC_RESTRICT শূন্য *RPMALLOC_CDECL malloc(size_t size) {
  প্রত্যাগম rpmalloc(size);
}
বাহ্য অন্তঃস্থ RPMALLOC_RESTRICT শূন্য *RPMALLOC_CDECL calloc(size_t গুনতি,
                                                            size_t size) {
  প্রত্যাগম rpcalloc(গুনতি, size);
}
বাহ্য অন্তঃস্থ RPMALLOC_RESTRICT শূন্য *RPMALLOC_CDECL realloc(শূন্য *ptr,
                                                             size_t size) {
  প্রত্যাগম rprealloc(ptr, size);
}
বাহ্য অন্তঃস্থ শূন্য *RPMALLOC_CDECL reallocf(শূন্য *ptr, size_t size) {
  প্রত্যাগম rprealloc(ptr, size);
}
বাহ্য অন্তঃস্থ শূন্য *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t size) {
  প্রত্যাগম rpaligned_alloc(alignment, size);
}
বাহ্য অন্তঃস্থ শূন্য *RPMALLOC_CDECL memalign(size_t alignment, size_t size) {
  প্রত্যাগম rpmemalign(alignment, size);
}
বাহ্য অন্তঃস্থ পূর্ণাঙ্ক RPMALLOC_CDECL posix_memalign(শূন্য **memptr, size_t alignment,
                                                size_t size) {
  প্রত্যাগম rpposix_memalign(memptr, alignment, size);
}
বাহ্য অন্তঃস্থ শূন্য RPMALLOC_CDECL মুক্ত(শূন্য *ptr) { rpfree(ptr); }
বাহ্য অন্তঃস্থ শূন্য RPMALLOC_CDECL cfree(শূন্য *ptr) { rpfree(ptr); }
বাহ্য অন্তঃস্থ size_t RPMALLOC_CDECL malloc_usable_size(শূন্য *ptr) {
  প্রত্যাগম rpmalloc_usable_size(ptr);
}
বাহ্য অন্তঃস্থ size_t RPMALLOC_CDECL malloc_size(শূন্য *ptr) {
  প্রত্যাগম rpmalloc_usable_size(ptr);
}

#ifdef _WIN32
বাহ্য অন্তঃস্থ RPMALLOC_RESTRICT শূন্য *RPMALLOC_CDECL _malloc_base(size_t size) {
  প্রত্যাগম rpmalloc(size);
}
বাহ্য অন্তঃস্থ শূন্য RPMALLOC_CDECL _free_base(শূন্য *ptr) { rpfree(ptr); }
বাহ্য অন্তঃস্থ RPMALLOC_RESTRICT শূন্য *RPMALLOC_CDECL _calloc_base(size_t গুনতি,
                                                                  size_t size) {
  প্রত্যাগম rpcalloc(গুনতি, size);
}
বাহ্য অন্তঃস্থ size_t RPMALLOC_CDECL _msize(শূন্য *ptr) {
  প্রত্যাগম rpmalloc_usable_size(ptr);
}
বাহ্য অন্তঃস্থ size_t RPMALLOC_CDECL _msize_base(শূন্য *ptr) {
  প্রত্যাগম rpmalloc_usable_size(ptr);
}
বাহ্য অন্তঃস্থ RPMALLOC_RESTRICT শূন্য *RPMALLOC_CDECL
_realloc_base(শূন্য *ptr, size_t size) {
  প্রত্যাগম rprealloc(ptr, size);
}
#endif

#ifdef _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#অন্যথা
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#define RPDEFVIS __attribute__((visibility("default")))
বাহ্য শূন্য _ZdlPv(শূন্য *p);
শূন্য RPDEFVIS _ZdlPv(শূন্য *p) { rpfree(p); }
বাহ্য শূন্য _ZdaPv(শূন্য *p);
শূন্য RPDEFVIS _ZdaPv(শূন্য *p) { rpfree(p); }
#যদি ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
বাহ্য শূন্য *_Znwm(uint64_t size);
শূন্য *RPDEFVIS _Znwm(uint64_t size) { প্রত্যাগম rpmalloc(size); }
বাহ্য শূন্য *_Znam(uint64_t size);
শূন্য *RPDEFVIS _Znam(uint64_t size) { প্রত্যাগম rpmalloc(size); }
বাহ্য শূন্য *_Znwmm(uint64_t size, uint64_t align);
শূন্য *RPDEFVIS _Znwmm(uint64_t size, uint64_t align) {
  প্রত্যাগম rpaligned_alloc(align, size);
}
বাহ্য শূন্য *_Znamm(uint64_t size, uint64_t align);
শূন্য *RPDEFVIS _Znamm(uint64_t size, uint64_t align) {
  প্রত্যাগম rpaligned_alloc(align, size);
}
বাহ্য শূন্য *_ZnwmSt11align_val_t(uint64_t size, uint64_t align);
শূন্য *RPDEFVIS _ZnwmSt11align_val_t(uint64_t size, uint64_t align) {
  প্রত্যাগম rpaligned_alloc(align, size);
}
বাহ্য শূন্য *_ZnamSt11align_val_t(uint64_t size, uint64_t align);
শূন্য *RPDEFVIS _ZnamSt11align_val_t(uint64_t size, uint64_t align) {
  প্রত্যাগম rpaligned_alloc(align, size);
}
বাহ্য শূন্য *_ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (শূন্য)পরিমাণ(t);
  প্রত্যাগম rpmalloc(size);
}
বাহ্য শূন্য *_ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (শূন্য)পরিমাণ(t);
  প্রত্যাগম rpmalloc(size);
}
বাহ্য শূন্য *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (শূন্য)পরিমাণ(t);
  প্রত্যাগম rpaligned_alloc(align, size);
}
বাহ্য শূন্য *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (শূন্য)পরিমাণ(t);
  প্রত্যাগম rpaligned_alloc(align, size);
}
// 64-bit operators sized delete and delete[], normal and aligned
বাহ্য শূন্য _ZdlPvm(শূন্য *p, uint64_t size);
শূন্য RPDEFVIS _ZdlPvm(শূন্য *p, uint64_t size) {
  rpfree(p);
  (শূন্য)পরিমাণ(size);
}
বাহ্য শূন্য _ZdaPvm(শূন্য *p, uint64_t size);
শূন্য RPDEFVIS _ZdaPvm(শূন্য *p, uint64_t size) {
  rpfree(p);
  (শূন্য)পরিমাণ(size);
}
বাহ্য শূন্য _ZdlPvSt11align_val_t(শূন্য *p, uint64_t align);
শূন্য RPDEFVIS _ZdlPvSt11align_val_t(শূন্য *p, uint64_t align) {
  rpfree(p);
  (শূন্য)পরিমাণ(align);
}
বাহ্য শূন্য _ZdaPvSt11align_val_t(শূন্য *p, uint64_t align);
শূন্য RPDEFVIS _ZdaPvSt11align_val_t(শূন্য *p, uint64_t align) {
  rpfree(p);
  (শূন্য)পরিমাণ(align);
}
বাহ্য শূন্য _ZdlPvmSt11align_val_t(শূন্য *p, uint64_t size, uint64_t align);
শূন্য RPDEFVIS _ZdlPvmSt11align_val_t(শূন্য *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (শূন্য)পরিমাণ(size);
  (শূন্য)পরিমাণ(align);
}
বাহ্য শূন্য _ZdaPvmSt11align_val_t(শূন্য *p, uint64_t size, uint64_t align);
শূন্য RPDEFVIS _ZdaPvmSt11align_val_t(শূন্য *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (শূন্য)পরিমাণ(size);
  (শূন্য)পরিমাণ(align);
}
#অন্যথা
// 32-bit operators new and new[], normal and aligned
বাহ্য শূন্য *_Znwj(uint32_t size);
শূন্য *RPDEFVIS _Znwj(uint32_t size) { প্রত্যাগম rpmalloc(size); }
বাহ্য শূন্য *_Znaj(uint32_t size);
শূন্য *RPDEFVIS _Znaj(uint32_t size) { প্রত্যাগম rpmalloc(size); }
বাহ্য শূন্য *_Znwjj(uint32_t size, uint32_t align);
শূন্য *RPDEFVIS _Znwjj(uint32_t size, uint32_t align) {
  প্রত্যাগম rpaligned_alloc(align, size);
}
বাহ্য শূন্য *_Znajj(uint32_t size, uint32_t align);
শূন্য *RPDEFVIS _Znajj(uint32_t size, uint32_t align) {
  প্রত্যাগম rpaligned_alloc(align, size);
}
বাহ্য শূন্য *_ZnwjSt11align_val_t(size_t size, size_t align);
শূন্য *RPDEFVIS _ZnwjSt11align_val_t(size_t size, size_t align) {
  প্রত্যাগম rpaligned_alloc(align, size);
}
বাহ্য শূন্য *_ZnajSt11align_val_t(size_t size, size_t align);
শূন্য *RPDEFVIS _ZnajSt11align_val_t(size_t size, size_t align) {
  প্রত্যাগম rpaligned_alloc(align, size);
}
বাহ্য শূন্য *_ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (শূন্য)পরিমাণ(t);
  প্রত্যাগম rpmalloc(size);
}
বাহ্য শূন্য *_ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (শূন্য)পরিমাণ(t);
  প্রত্যাগম rpmalloc(size);
}
বাহ্য শূন্য *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (শূন্য)পরিমাণ(t);
  প্রত্যাগম rpaligned_alloc(align, size);
}
বাহ্য শূন্য *_ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (শূন্য)পরিমাণ(t);
  প্রত্যাগম rpaligned_alloc(align, size);
}
// 32-bit operators sized delete and delete[], normal and aligned
বাহ্য শূন্য _ZdlPvj(শূন্য *p, uint64_t size);
শূন্য RPDEFVIS _ZdlPvj(শূন্য *p, uint64_t size) {
  rpfree(p);
  (শূন্য)পরিমাণ(size);
}
বাহ্য শূন্য _ZdaPvj(শূন্য *p, uint64_t size);
শূন্য RPDEFVIS _ZdaPvj(শূন্য *p, uint64_t size) {
  rpfree(p);
  (শূন্য)পরিমাণ(size);
}
বাহ্য শূন্য _ZdlPvSt11align_val_t(শূন্য *p, uint32_t align);
শূন্য RPDEFVIS _ZdlPvSt11align_val_t(শূন্য *p, uint64_t a) {
  rpfree(p);
  (শূন্য)পরিমাণ(align);
}
বাহ্য শূন্য _ZdaPvSt11align_val_t(শূন্য *p, uint32_t align);
শূন্য RPDEFVIS _ZdaPvSt11align_val_t(শূন্য *p, uint64_t a) {
  rpfree(p);
  (শূন্য)পরিমাণ(align);
}
বাহ্য শূন্য _ZdlPvjSt11align_val_t(শূন্য *p, uint32_t size, uint32_t align);
শূন্য RPDEFVIS _ZdlPvjSt11align_val_t(শূন্য *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (শূন্য)পরিমাণ(size);
  (শূন্য)পরিমাণ(a);
}
বাহ্য শূন্য _ZdaPvjSt11align_val_t(শূন্য *p, uint32_t size, uint32_t align);
শূন্য RPDEFVIS _ZdaPvjSt11align_val_t(শূন্য *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (শূন্য)পরিমাণ(size);
  (শূন্য)পরিমাণ(a);
}
#endif
#endif
#endif

#যদি USE_INTERPOSE || USE_ALIAS

স্থির শূন্য *rpmalloc_nothrow(size_t size, rp_nothrow_t t) {
  (শূন্য)পরিমাণ(t);
  প্রত্যাগম rpmalloc(size);
}
স্থির শূন্য *rpaligned_alloc_reverse(size_t size, size_t align) {
  প্রত্যাগম rpaligned_alloc(align, size);
}
স্থির শূন্য *rpaligned_alloc_reverse_nothrow(size_t size, size_t align,
                                             rp_nothrow_t t) {
  (শূন্য)পরিমাণ(t);
  প্রত্যাগম rpaligned_alloc(align, size);
}
স্থির শূন্য rpfree_size(শূন্য *p, size_t size) {
  (শূন্য)পরিমাণ(size);
  rpfree(p);
}
স্থির শূন্য rpfree_aligned(শূন্য *p, size_t align) {
  (শূন্য)পরিমাণ(align);
  rpfree(p);
}
স্থির শূন্য rpfree_size_aligned(শূন্য *p, size_t size, size_t align) {
  (শূন্য)পরিমাণ(size);
  (শূন্য)পরিমাণ(align);
  rpfree(p);
}

#endif

#যদি USE_INTERPOSE

__attribute__((used)) স্থির ধ্রুব interpose_t macinterpose_malloc[]
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
#যদি defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#endif
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, মুক্ত), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_usable_size),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_size)};

#endif

#যদি USE_ALIAS

#define RPALIAS(fn) __attribute__((alias(#fn), used, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
শূন্য _ZdlPv(শূন্য *p) RPALIAS(rpfree) শূন্য _ZdaPv(শূন্য *p) RPALIAS(rpfree)

#যদি ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    শূন্য *_Znwm(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) শূন্য *_Znam(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) শূন্য *_Znwmm(uint64_t size,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) শূন্য *_Znamm(uint64_t size,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) শূন্য *_ZnwmSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) শূন্য *_ZnamSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) শূন্য *_ZnwmRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) শূন্য *_ZnamRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) শূন্য
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) শূন্য
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    শূন্য _ZdlPvm(শূন্য *p, size_t n) RPALIAS(rpfree_size) শূন্য _ZdaPvm(শূন্য *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) শূন্য _ZdlPvSt11align_val_t(শূন্য *p, size_t a)
            RPALIAS(rpfree_aligned) শূন্য _ZdaPvSt11align_val_t(শূন্য *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) শূন্য _ZdlPvmSt11align_val_t(শূন্য *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) শূন্য _ZdaPvmSt11align_val_t(
                        শূন্য *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#অন্যথা
    // 32-bit operators new and new[], normal and aligned
    শূন্য *_Znwj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) শূন্য *_Znaj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) শূন্য *_Znwjj(uint32_t size,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) শূন্য *_Znajj(uint32_t size,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) শূন্য *_ZnwjSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) শূন্য *_ZnajSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) শূন্য *_ZnwjRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) শূন্য *_ZnajRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) শূন্য
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) শূন্য
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    শূন্য _ZdlPvj(শূন্য *p, size_t n) RPALIAS(rpfree_size) শূন্য _ZdaPvj(শূন্য *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) শূন্য _ZdlPvSt11align_val_t(শূন্য *p, size_t a)
            RPALIAS(rpfree_aligned) শূন্য _ZdaPvSt11align_val_t(শূন্য *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) শূন্য _ZdlPvjSt11align_val_t(শূন্য *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) শূন্য _ZdaPvjSt11align_val_t(
                        শূন্য *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#endif

                            শূন্য *malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) শূন্য *calloc(size_t গুনতি, size_t size)
            RPALIAS(rpcalloc) শূন্য *realloc(শূন্য *ptr, size_t size)
                RPALIAS(rprealloc) শূন্য *reallocf(শূন্য *ptr, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rprealloc) শূন্য *aligned_alloc(size_t alignment, size_t size)
            RPALIAS(rpaligned_alloc) শূন্য *memalign(
                size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) পূর্ণাঙ্ক posix_memalign(শূন্য **memptr, size_t alignment,
                                               size_t size)
            RPALIAS(rpposix_memalign) শূন্য মুক্ত(শূন্য *ptr)
                RPALIAS(rpfree) শূন্য cfree(শূন্য *ptr) RPALIAS(rpfree)
#যদি defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_size(ধ্রুব শূন্য *ptr) RPALIAS(rpmalloc_usable_size)
#অন্যথা
                    size_t
    malloc_usable_size(শূন্য *ptr) RPALIAS(rpmalloc_usable_size)
#endif
        size_t malloc_size(শূন্য *ptr) RPALIAS(rpmalloc_usable_size)

#endif

            স্থির অন্তঃস্থ size_t _rpmalloc_page_size(শূন্য) {
  প্রত্যাগম _memory_page_size;
}

বাহ্য শূন্য *RPMALLOC_CDECL reallocarray(শূন্য *ptr, size_t গুনতি, size_t size);

বাহ্য শূন্য *RPMALLOC_CDECL reallocarray(শূন্য *ptr, size_t গুনতি, size_t size) {
  size_t total;
#যদি ENABLE_VALIDATE_ARGS
#ifdef _MSC_VER
  পূর্ণাঙ্ক err = SizeTMult(গুনতি, size, &total);
  যদি ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#অন্যথা
  পূর্ণাঙ্ক err = __builtin_umull_overflow(গুনতি, size, &total);
  যদি (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#endif
#অন্যথা
  total = গুনতি * size;
#endif
  প্রত্যাগম realloc(ptr, total);
}

বাহ্য অন্তঃস্থ শূন্য *RPMALLOC_CDECL valloc(size_t size) {
  get_thread_heap();
  প্রত্যাগম rpaligned_alloc(_rpmalloc_page_size(), size);
}

বাহ্য অন্তঃস্থ শূন্য *RPMALLOC_CDECL pvalloc(size_t size) {
  get_thread_heap();
  ধ্রুব size_t page_size = _rpmalloc_page_size();
  ধ্রুব size_t aligned_size = ((size + page_size - 1) / page_size) * page_size;
#যদি ENABLE_VALIDATE_ARGS
  যদি (aligned_size < size) {
    errno = EINVAL;
    প্রত্যাগম 0;
  }
#endif
  প্রত্যাগম rpaligned_alloc(_rpmalloc_page_size(), aligned_size);
}

#endif // ENABLE_OVERRIDE

#যদি ENABLE_PRELOAD

#ifdef _WIN32

#যদি defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

বাহ্য __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

বাহ্য __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (শূন্য)পরিমাণ(reserved);
  (শূন্য)পরিমাণ(instance);
  যদি (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  অন্যথা যদি (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  অন্যথা যদি (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  অন্যথা যদি (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  প্রত্যাগম TRUE;
}

// end BUILD_DYNAMIC_LINK
#অন্যথা

বাহ্য শূন্য _global_rpmalloc_init(শূন্য) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
}

#যদি defined(__clang__) || defined(__GNUC__)

স্থির শূন্য __attribute__((constructor)) initializer(শূন্য) {
  _global_rpmalloc_init();
}

#নোচেদ্যদি defined(_MSC_VER)

স্থির পূর্ণাঙ্ক _global_rpmalloc_xib(শূন্য) {
  _global_rpmalloc_init();
  প্রত্যাগম 0;
}

#pragma section(".CRT$XIB", পঠ)
__declspec(allocate(".CRT$XIB")) শূন্য (*_rpmalloc_module_init)(শূন্য) =
    _global_rpmalloc_xib;
#যদি defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#অন্যথা
#pragma comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#endif

#endif

// end !BUILD_DYNAMIC_LINK
#endif

#অন্যথা

#অন্তর্ভাব <pthread.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <unistd.h>

বাহ্য শূন্য rpmalloc_set_main_thread(শূন্য);

স্থির pthread_key_t destructor_চাবি;

স্থির শূন্য thread_destructor(শূন্য *);

স্থির শূন্য __attribute__((constructor)) initializer(শূন্য) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
  pthread_চাবি_create(&destructor_চাবি, thread_destructor);
}

স্থির শূন্য __attribute__((destructor)) finalizer(শূন্য) { rpmalloc_finalize(); }

প্রকারনির্দেশ সংরচনা {
  শূন্য *(*real_start)(শূন্য *);
  শূন্য *real_arg;
} thread_starter_arg;

স্থির শূন্য *thread_starter(শূন্য *argptr) {
  thread_starter_arg *arg = argptr;
  শূন্য *(*real_start)(শূন্য *) = arg->real_start;
  শূন্য *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_চাবি, (শূন্য *)1);
  প্রত্যাগম (*real_start)(real_arg);
}

স্থির শূন্য thread_destructor(শূন্য *মান_2) {
  (শূন্য)পরিমাণ(মান_2);
  rpmalloc_thread_finalize(1);
}

#ifdef __APPLE__

স্থির পূর্ণাঙ্ক pthread_create_proxy(pthread_t *thread, ধ্রুব pthread_attr_t *attr,
                                শূন্য *(*start_routine)(শূন্য *), শূন্য *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(পরিমাণ(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  প্রত্যাগম pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#অন্যথা

#অন্তর্ভাব <dlfcn.h>

পূর্ণাঙ্ক pthread_create(pthread_t *thread, ধ্রুব pthread_attr_t *attr,
                   শূন্য *(*start_routine)(শূন্য *), শূন্য *arg) {
#যদি defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  বর্ণ fname[] = "pthread_create";
#অন্যথা
  বর্ণ fname[] = "_pthread_create";
#endif
  শূন্য *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(পরিমাণ(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  প্রত্যাগম (*(পূর্ণাঙ্ক (*)(pthread_t *, ধ্রুব pthread_attr_t *, শূন্য *(*)(শূন্য *),
                    শূন্য *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#endif

#endif

#endif

#যদি ENABLE_OVERRIDE

#যদি defined(__GLIBC__) && defined(__linux__)

শূন্য *__libc_malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) শূন্য *__libc_calloc(size_t গুনতি, size_t size)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) শূন্য *__libc_realloc(শূন্য *p, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2) RPALIAS(rprealloc) শূন্য __libc_free(শূন্য *p)
        RPALIAS(rpfree) শূন্য __libc_cfree(শূন্য *p)
            RPALIAS(rpfree) শূন্য *__libc_memalign(size_t align, size_t size)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) পূর্ণাঙ্ক __posix_memalign(শূন্য **p, size_t align,
                                                 size_t size)
            RPALIAS(rpposix_memalign)

                বাহ্য শূন্য *__libc_valloc(size_t size);
বাহ্য শূন্য *__libc_pvalloc(size_t size);

শূন্য *__libc_valloc(size_t size) { প্রত্যাগম valloc(size); }

শূন্য *__libc_pvalloc(size_t size) { প্রত্যাগম pvalloc(size); }

#endif

#endif

#যদি (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility বের_করো
#endif
