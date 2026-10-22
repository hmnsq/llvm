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
_Static_assert(আকার(size_t) == 8, "Data type size mismatch");
_Static_assert(আকার(শূন্য *) == 8, "Data type size mismatch");
#অন্যথা
#define ARCH_64BIT 0
_Static_assert(আকার(size_t) == 4, "Data type size mismatch");
_Static_assert(আকার(শূন্য *) == 4, "Data type size mismatch");
#endif
#endif

#যদি (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility ঠেলো(পূর্বনির্ধারিত)
#endif

#define USE_IMPLEMENT 1
#define USE_INTERPOSE 0
#define USE_ALIAS 0

#যদি defined(__APPLE__)
#undef USE_INTERPOSE
#define USE_INTERPOSE 1

প্রকারলেখ কাঠামো interpose_t {
  শূন্য *new_func;
  শূন্য *orig_func;
} interpose_t;

#define MAC_INTERPOSE_PAIR(newf, oldf) {(শূন্য *)newf, (শূন্য *)oldf}
#define MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((used)) স্থির ধ্রুবক interpose_t macinterpose##newf##oldf      \
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

প্রকারলেখ কাঠামো rp_nothrow_t {
  পূর্ণসংখ্যা __dummy;
} rp_nothrow_t;

#যদি USE_IMPLEMENT

বাহ্যিক অন্তঃরেখ RPMALLOC_RESTRICT শূন্য *RPMALLOC_CDECL malloc(size_t আকার_3) {
  ফেরত rpmalloc(আকার_3);
}
বাহ্যিক অন্তঃরেখ RPMALLOC_RESTRICT শূন্য *RPMALLOC_CDECL calloc(size_t গুনতি,
                                                            size_t আকার_3) {
  ফেরত rpcalloc(গুনতি, আকার_3);
}
বাহ্যিক অন্তঃরেখ RPMALLOC_RESTRICT শূন্য *RPMALLOC_CDECL realloc(শূন্য *ptr,
                                                             size_t আকার_3) {
  ফেরত rprealloc(ptr, আকার_3);
}
বাহ্যিক অন্তঃরেখ শূন্য *RPMALLOC_CDECL reallocf(শূন্য *ptr, size_t আকার_3) {
  ফেরত rprealloc(ptr, আকার_3);
}
বাহ্যিক অন্তঃরেখ শূন্য *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t আকার_3) {
  ফেরত rpaligned_alloc(alignment, আকার_3);
}
বাহ্যিক অন্তঃরেখ শূন্য *RPMALLOC_CDECL memalign(size_t alignment, size_t আকার_3) {
  ফেরত rpmemalign(alignment, আকার_3);
}
বাহ্যিক অন্তঃরেখ পূর্ণসংখ্যা RPMALLOC_CDECL posix_memalign(শূন্য **memptr, size_t alignment,
                                                size_t আকার_3) {
  ফেরত rpposix_memalign(memptr, alignment, আকার_3);
}
বাহ্যিক অন্তঃরেখ শূন্য RPMALLOC_CDECL মুক্ত(শূন্য *ptr) { rpfree(ptr); }
বাহ্যিক অন্তঃরেখ শূন্য RPMALLOC_CDECL cfree(শূন্য *ptr) { rpfree(ptr); }
বাহ্যিক অন্তঃরেখ size_t RPMALLOC_CDECL malloc_usable_আকার(শূন্য *ptr) {
  ফেরত rpmalloc_usable_আকার(ptr);
}
বাহ্যিক অন্তঃরেখ size_t RPMALLOC_CDECL malloc_আকার(শূন্য *ptr) {
  ফেরত rpmalloc_usable_আকার(ptr);
}

#ifdef _WIN32
বাহ্যিক অন্তঃরেখ RPMALLOC_RESTRICT শূন্য *RPMALLOC_CDECL _malloc_base(size_t আকার_3) {
  ফেরত rpmalloc(আকার_3);
}
বাহ্যিক অন্তঃরেখ শূন্য RPMALLOC_CDECL _free_base(শূন্য *ptr) { rpfree(ptr); }
বাহ্যিক অন্তঃরেখ RPMALLOC_RESTRICT শূন্য *RPMALLOC_CDECL _calloc_base(size_t গুনতি,
                                                                  size_t আকার_3) {
  ফেরত rpcalloc(গুনতি, আকার_3);
}
বাহ্যিক অন্তঃরেখ size_t RPMALLOC_CDECL _msize(শূন্য *ptr) {
  ফেরত rpmalloc_usable_আকার(ptr);
}
বাহ্যিক অন্তঃরেখ size_t RPMALLOC_CDECL _msize_base(শূন্য *ptr) {
  ফেরত rpmalloc_usable_আকার(ptr);
}
বাহ্যিক অন্তঃরেখ RPMALLOC_RESTRICT শূন্য *RPMALLOC_CDECL
_realloc_base(শূন্য *ptr, size_t আকার_3) {
  ফেরত rprealloc(ptr, আকার_3);
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
বাহ্যিক শূন্য _ZdlPv(শূন্য *p);
শূন্য RPDEFVIS _ZdlPv(শূন্য *p) { rpfree(p); }
বাহ্যিক শূন্য _ZdaPv(শূন্য *p);
শূন্য RPDEFVIS _ZdaPv(শূন্য *p) { rpfree(p); }
#যদি ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
বাহ্যিক শূন্য *_Znwm(uint64_t আকার_3);
শূন্য *RPDEFVIS _Znwm(uint64_t আকার_3) { ফেরত rpmalloc(আকার_3); }
বাহ্যিক শূন্য *_Znam(uint64_t আকার_3);
শূন্য *RPDEFVIS _Znam(uint64_t আকার_3) { ফেরত rpmalloc(আকার_3); }
বাহ্যিক শূন্য *_Znwmm(uint64_t আকার_3, uint64_t align);
শূন্য *RPDEFVIS _Znwmm(uint64_t আকার_3, uint64_t align) {
  ফেরত rpaligned_alloc(align, আকার_3);
}
বাহ্যিক শূন্য *_Znamm(uint64_t আকার_3, uint64_t align);
শূন্য *RPDEFVIS _Znamm(uint64_t আকার_3, uint64_t align) {
  ফেরত rpaligned_alloc(align, আকার_3);
}
বাহ্যিক শূন্য *_ZnwmSt11align_val_t(uint64_t আকার_3, uint64_t align);
শূন্য *RPDEFVIS _ZnwmSt11align_val_t(uint64_t আকার_3, uint64_t align) {
  ফেরত rpaligned_alloc(align, আকার_3);
}
বাহ্যিক শূন্য *_ZnamSt11align_val_t(uint64_t আকার_3, uint64_t align);
শূন্য *RPDEFVIS _ZnamSt11align_val_t(uint64_t আকার_3, uint64_t align) {
  ফেরত rpaligned_alloc(align, আকার_3);
}
বাহ্যিক শূন্য *_ZnwmRKSt9nothrow_t(uint64_t আকার_3, rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t আকার_3, rp_nothrow_t t) {
  (শূন্য)আকার(t);
  ফেরত rpmalloc(আকার_3);
}
বাহ্যিক শূন্য *_ZnamRKSt9nothrow_t(uint64_t আকার_3, rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t আকার_3, rp_nothrow_t t) {
  (শূন্য)আকার(t);
  ফেরত rpmalloc(আকার_3);
}
বাহ্যিক শূন্য *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t আকার_3, uint64_t align,
                                                rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t আকার_3, uint64_t align,
                                                  rp_nothrow_t t) {
  (শূন্য)আকার(t);
  ফেরত rpaligned_alloc(align, আকার_3);
}
বাহ্যিক শূন্য *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t আকার_3, uint64_t align,
                                                rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t আকার_3, uint64_t align,
                                                  rp_nothrow_t t) {
  (শূন্য)আকার(t);
  ফেরত rpaligned_alloc(align, আকার_3);
}
// 64-bit operators sized delete and delete[], normal and aligned
বাহ্যিক শূন্য _ZdlPvm(শূন্য *p, uint64_t আকার_3);
শূন্য RPDEFVIS _ZdlPvm(শূন্য *p, uint64_t আকার_3) {
  rpfree(p);
  (শূন্য)আকার(আকার_3);
}
বাহ্যিক শূন্য _ZdaPvm(শূন্য *p, uint64_t আকার_3);
শূন্য RPDEFVIS _ZdaPvm(শূন্য *p, uint64_t আকার_3) {
  rpfree(p);
  (শূন্য)আকার(আকার_3);
}
বাহ্যিক শূন্য _ZdlPvSt11align_val_t(শূন্য *p, uint64_t align);
শূন্য RPDEFVIS _ZdlPvSt11align_val_t(শূন্য *p, uint64_t align) {
  rpfree(p);
  (শূন্য)আকার(align);
}
বাহ্যিক শূন্য _ZdaPvSt11align_val_t(শূন্য *p, uint64_t align);
শূন্য RPDEFVIS _ZdaPvSt11align_val_t(শূন্য *p, uint64_t align) {
  rpfree(p);
  (শূন্য)আকার(align);
}
বাহ্যিক শূন্য _ZdlPvmSt11align_val_t(শূন্য *p, uint64_t আকার_3, uint64_t align);
শূন্য RPDEFVIS _ZdlPvmSt11align_val_t(শূন্য *p, uint64_t আকার_3, uint64_t align) {
  rpfree(p);
  (শূন্য)আকার(আকার_3);
  (শূন্য)আকার(align);
}
বাহ্যিক শূন্য _ZdaPvmSt11align_val_t(শূন্য *p, uint64_t আকার_3, uint64_t align);
শূন্য RPDEFVIS _ZdaPvmSt11align_val_t(শূন্য *p, uint64_t আকার_3, uint64_t align) {
  rpfree(p);
  (শূন্য)আকার(আকার_3);
  (শূন্য)আকার(align);
}
#অন্যথা
// 32-bit operators new and new[], normal and aligned
বাহ্যিক শূন্য *_Znwj(uint32_t আকার_3);
শূন্য *RPDEFVIS _Znwj(uint32_t আকার_3) { ফেরত rpmalloc(আকার_3); }
বাহ্যিক শূন্য *_Znaj(uint32_t আকার_3);
শূন্য *RPDEFVIS _Znaj(uint32_t আকার_3) { ফেরত rpmalloc(আকার_3); }
বাহ্যিক শূন্য *_Znwjj(uint32_t আকার_3, uint32_t align);
শূন্য *RPDEFVIS _Znwjj(uint32_t আকার_3, uint32_t align) {
  ফেরত rpaligned_alloc(align, আকার_3);
}
বাহ্যিক শূন্য *_Znajj(uint32_t আকার_3, uint32_t align);
শূন্য *RPDEFVIS _Znajj(uint32_t আকার_3, uint32_t align) {
  ফেরত rpaligned_alloc(align, আকার_3);
}
বাহ্যিক শূন্য *_ZnwjSt11align_val_t(size_t আকার_3, size_t align);
শূন্য *RPDEFVIS _ZnwjSt11align_val_t(size_t আকার_3, size_t align) {
  ফেরত rpaligned_alloc(align, আকার_3);
}
বাহ্যিক শূন্য *_ZnajSt11align_val_t(size_t আকার_3, size_t align);
শূন্য *RPDEFVIS _ZnajSt11align_val_t(size_t আকার_3, size_t align) {
  ফেরত rpaligned_alloc(align, আকার_3);
}
বাহ্যিক শূন্য *_ZnwjRKSt9nothrow_t(size_t আকার_3, rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t আকার_3, rp_nothrow_t t) {
  (শূন্য)আকার(t);
  ফেরত rpmalloc(আকার_3);
}
বাহ্যিক শূন্য *_ZnajRKSt9nothrow_t(size_t আকার_3, rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnajRKSt9nothrow_t(size_t আকার_3, rp_nothrow_t t) {
  (শূন্য)আকার(t);
  ফেরত rpmalloc(আকার_3);
}
বাহ্যিক শূন্য *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t আকার_3, size_t align,
                                                rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t আকার_3, size_t align,
                                                  rp_nothrow_t t) {
  (শূন্য)আকার(t);
  ফেরত rpaligned_alloc(align, আকার_3);
}
বাহ্যিক শূন্য *_ZnajSt11align_val_tRKSt9nothrow_t(size_t আকার_3, size_t align,
                                                rp_nothrow_t t);
শূন্য *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t আকার_3, size_t align,
                                                  rp_nothrow_t t) {
  (শূন্য)আকার(t);
  ফেরত rpaligned_alloc(align, আকার_3);
}
// 32-bit operators sized delete and delete[], normal and aligned
বাহ্যিক শূন্য _ZdlPvj(শূন্য *p, uint64_t আকার_3);
শূন্য RPDEFVIS _ZdlPvj(শূন্য *p, uint64_t আকার_3) {
  rpfree(p);
  (শূন্য)আকার(আকার_3);
}
বাহ্যিক শূন্য _ZdaPvj(শূন্য *p, uint64_t আকার_3);
শূন্য RPDEFVIS _ZdaPvj(শূন্য *p, uint64_t আকার_3) {
  rpfree(p);
  (শূন্য)আকার(আকার_3);
}
বাহ্যিক শূন্য _ZdlPvSt11align_val_t(শূন্য *p, uint32_t align);
শূন্য RPDEFVIS _ZdlPvSt11align_val_t(শূন্য *p, uint64_t a) {
  rpfree(p);
  (শূন্য)আকার(align);
}
বাহ্যিক শূন্য _ZdaPvSt11align_val_t(শূন্য *p, uint32_t align);
শূন্য RPDEFVIS _ZdaPvSt11align_val_t(শূন্য *p, uint64_t a) {
  rpfree(p);
  (শূন্য)আকার(align);
}
বাহ্যিক শূন্য _ZdlPvjSt11align_val_t(শূন্য *p, uint32_t আকার_3, uint32_t align);
শূন্য RPDEFVIS _ZdlPvjSt11align_val_t(শূন্য *p, uint64_t আকার_3, uint64_t align) {
  rpfree(p);
  (শূন্য)আকার(আকার_3);
  (শূন্য)আকার(a);
}
বাহ্যিক শূন্য _ZdaPvjSt11align_val_t(শূন্য *p, uint32_t আকার_3, uint32_t align);
শূন্য RPDEFVIS _ZdaPvjSt11align_val_t(শূন্য *p, uint64_t আকার_3, uint64_t align) {
  rpfree(p);
  (শূন্য)আকার(আকার_3);
  (শূন্য)আকার(a);
}
#endif
#endif
#endif

#যদি USE_INTERPOSE || USE_ALIAS

স্থির শূন্য *rpmalloc_nothrow(size_t আকার_3, rp_nothrow_t t) {
  (শূন্য)আকার(t);
  ফেরত rpmalloc(আকার_3);
}
স্থির শূন্য *rpaligned_alloc_reverse(size_t আকার_3, size_t align) {
  ফেরত rpaligned_alloc(align, আকার_3);
}
স্থির শূন্য *rpaligned_alloc_reverse_nothrow(size_t আকার_3, size_t align,
                                             rp_nothrow_t t) {
  (শূন্য)আকার(t);
  ফেরত rpaligned_alloc(align, আকার_3);
}
স্থির শূন্য rpfree_আকার(শূন্য *p, size_t আকার_3) {
  (শূন্য)আকার(আকার_3);
  rpfree(p);
}
স্থির শূন্য rpfree_aligned(শূন্য *p, size_t align) {
  (শূন্য)আকার(align);
  rpfree(p);
}
স্থির শূন্য rpfree_আকার_aligned(শূন্য *p, size_t আকার_3, size_t align) {
  (শূন্য)আকার(আকার_3);
  (শূন্য)আকার(align);
  rpfree(p);
}

#endif

#যদি USE_INTERPOSE

__attribute__((used)) স্থির ধ্রুবক interpose_t macinterpose_malloc[]
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
        MAC_INTERPOSE_PAIR(rpfree_আকার, _ZdlPvm),
        MAC_INTERPOSE_PAIR(rpfree_আকার, _ZdaPvm),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdlPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdaPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_আকার_aligned, _ZdlPvmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_আকার_aligned, _ZdaPvmSt11align_val_t),
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
        MAC_INTERPOSE_PAIR(rpmalloc_usable_আকার, malloc_usable_আকার),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_আকার, malloc_আকার)};

#endif

#যদি USE_ALIAS

#define RPALIAS(fn) __attribute__((alias(#fn), used, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
শূন্য _ZdlPv(শূন্য *p) RPALIAS(rpfree) শূন্য _ZdaPv(শূন্য *p) RPALIAS(rpfree)

#যদি ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    শূন্য *_Znwm(uint64_t আকার_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_আকার(1)
        RPALIAS(rpmalloc) শূন্য *_Znam(uint64_t আকার_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_আকার(1) RPALIAS(rpmalloc) শূন্য *_Znwmm(uint64_t আকার_3,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) শূন্য *_Znamm(uint64_t আকার_3,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) শূন্য *_ZnwmSt11align_val_t(
                size_t আকার_3, size_t align)
                RPALIAS(rpaligned_alloc_reverse) শূন্য *_ZnamSt11align_val_t(
                    size_t আকার_3, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) শূন্য *_ZnwmRKSt9nothrow_t(
                        size_t আকার_3, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) শূন্য *_ZnamRKSt9nothrow_t(
                            size_t আকার_3,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) শূন্য
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t আকার_3,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) শূন্য
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t আকার_3, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    শূন্য _ZdlPvm(শূন্য *p, size_t n) RPALIAS(rpfree_আকার) শূন্য _ZdaPvm(শূন্য *p,
                                                                      size_t n)
        RPALIAS(rpfree_আকার) শূন্য _ZdlPvSt11align_val_t(শূন্য *p, size_t a)
            RPALIAS(rpfree_aligned) শূন্য _ZdaPvSt11align_val_t(শূন্য *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) শূন্য _ZdlPvmSt11align_val_t(শূন্য *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_আকার_aligned) শূন্য _ZdaPvmSt11align_val_t(
                        শূন্য *p, size_t n, size_t a)
                        RPALIAS(rpfree_আকার_aligned)
#অন্যথা
    // 32-bit operators new and new[], normal and aligned
    শূন্য *_Znwj(uint32_t আকার_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_আকার(1)
        RPALIAS(rpmalloc) শূন্য *_Znaj(uint32_t আকার_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_আকার(1) RPALIAS(rpmalloc) শূন্য *_Znwjj(uint32_t আকার_3,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) শূন্য *_Znajj(uint32_t আকার_3,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) শূন্য *_ZnwjSt11align_val_t(
                size_t আকার_3, size_t align)
                RPALIAS(rpaligned_alloc_reverse) শূন্য *_ZnajSt11align_val_t(
                    size_t আকার_3, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) শূন্য *_ZnwjRKSt9nothrow_t(
                        size_t আকার_3, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) শূন্য *_ZnajRKSt9nothrow_t(
                            size_t আকার_3,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) শূন্য
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t আকার_3,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) শূন্য
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t আকার_3, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    শূন্য _ZdlPvj(শূন্য *p, size_t n) RPALIAS(rpfree_আকার) শূন্য _ZdaPvj(শূন্য *p,
                                                                      size_t n)
        RPALIAS(rpfree_আকার) শূন্য _ZdlPvSt11align_val_t(শূন্য *p, size_t a)
            RPALIAS(rpfree_aligned) শূন্য _ZdaPvSt11align_val_t(শূন্য *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) শূন্য _ZdlPvjSt11align_val_t(শূন্য *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_আকার_aligned) শূন্য _ZdaPvjSt11align_val_t(
                        শূন্য *p, size_t n, size_t a)
                        RPALIAS(rpfree_আকার_aligned)
#endif

                            শূন্য *malloc(size_t আকার_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_আকার(1)
        RPALIAS(rpmalloc) শূন্য *calloc(size_t গুনতি, size_t আকার_3)
            RPALIAS(rpcalloc) শূন্য *realloc(শূন্য *ptr, size_t আকার_3)
                RPALIAS(rprealloc) শূন্য *reallocf(শূন্য *ptr, size_t আকার_3)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_আকার(2)
        RPALIAS(rprealloc) শূন্য *aligned_alloc(size_t alignment, size_t আকার_3)
            RPALIAS(rpaligned_alloc) শূন্য *memalign(
                size_t alignment, size_t আকার_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_আকার(2)
        RPALIAS(rpmemalign) পূর্ণসংখ্যা posix_memalign(শূন্য **memptr, size_t alignment,
                                               size_t আকার_3)
            RPALIAS(rpposix_memalign) শূন্য মুক্ত(শূন্য *ptr)
                RPALIAS(rpfree) শূন্য cfree(শূন্য *ptr) RPALIAS(rpfree)
#যদি defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_আকার(ধ্রুবক শূন্য *ptr) RPALIAS(rpmalloc_usable_আকার)
#অন্যথা
                    size_t
    malloc_usable_আকার(শূন্য *ptr) RPALIAS(rpmalloc_usable_আকার)
#endif
        size_t malloc_আকার(শূন্য *ptr) RPALIAS(rpmalloc_usable_আকার)

#endif

            স্থির অন্তঃরেখ size_t rpmalloc_page_আকার(শূন্য) {
  ফেরত memory_page_আকার;
}

বাহ্যিক শূন্য *RPMALLOC_CDECL reallocarray(শূন্য *ptr, size_t গুনতি, size_t আকার_3);

বাহ্যিক শূন্য *RPMALLOC_CDECL reallocarray(শূন্য *ptr, size_t গুনতি, size_t আকার_3) {
  size_t যোগ_3;
#যদি ENABLE_VALIDATE_ARGS
#ifdef _MSC_VER
  পূর্ণসংখ্যা err = আকার_TMult(গুনতি, আকার_3, &যোগ_3);
  যদি ((err != S_OK) || (যোগ_3 >= MAX_ALLOC_আকার)) {
    errno = EINVAL;
    ফেরত 0;
  }
#অন্যথা
  পূর্ণসংখ্যা err = __builtin_umull_overflow(গুনতি, আকার_3, &যোগ_3);
  যদি (err || (যোগ_3 >= MAX_ALLOC_আকার)) {
    errno = EINVAL;
    ফেরত 0;
  }
#endif
#অন্যথা
  যোগ_3 = গুনতি * আকার_3;
#endif
  ফেরত realloc(ptr, যোগ_3);
}

বাহ্যিক অন্তঃরেখ শূন্য *RPMALLOC_CDECL valloc(size_t আকার_3) {
  get_thread_heap();
  ফেরত rpaligned_alloc(rpmalloc_page_আকার(), আকার_3);
}

বাহ্যিক অন্তঃরেখ শূন্য *RPMALLOC_CDECL pvalloc(size_t আকার_3) {
  get_thread_heap();
  ধ্রুবক size_t page_আকার = rpmalloc_page_আকার();
  ধ্রুবক size_t aligned_আকার = ((আকার_3 + page_আকার - 1) / page_আকার) * page_আকার;
#যদি ENABLE_VALIDATE_ARGS
  যদি (aligned_আকার < আকার_3) {
    errno = EINVAL;
    ফেরত 0;
  }
#endif
  ফেরত rpaligned_alloc(rpmalloc_page_আকার(), aligned_আকার);
}

#endif // ENABLE_OVERRIDE

#যদি ENABLE_PRELOAD

#ifdef _WIN32

#যদি defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

বাহ্যিক __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

বাহ্যিক __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (শূন্য)আকার(reserved);
  (শূন্য)আকার(instance);
  যদি (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  অন্যথা যদি (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  অন্যথা যদি (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  অন্যথা যদি (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  ফেরত TRUE;
}

// end BUILD_DYNAMIC_LINK
#অন্যথা

বাহ্যিক শূন্য _global_rpmalloc_init(শূন্য) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
}

#যদি defined(__clang__) || defined(__GNUC__)

স্থির শূন্য __attribute__((constructor)) initializer(শূন্য) {
  _global_rpmalloc_init();
}

#নতুবা_যদি defined(_MSC_VER)

স্থির পূর্ণসংখ্যা _global_rpmalloc_xib(শূন্য) {
  _global_rpmalloc_init();
  ফেরত 0;
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

বাহ্যিক শূন্য rpmalloc_set_main_thread(শূন্য);

স্থির pthread_key_t destructor_চাবি;

স্থির শূন্য thread_destructor(শূন্য *);

স্থির শূন্য __attribute__((constructor)) initializer(শূন্য) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
  pthread_চাবি_create(&destructor_চাবি, thread_destructor);
}

স্থির শূন্য __attribute__((destructor)) finalizer(শূন্য) { rpmalloc_finalize(); }

প্রকারলেখ কাঠামো {
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
  ফেরত (*real_start)(real_arg);
}

স্থির শূন্য thread_destructor(শূন্য *মান_2) {
  (শূন্য)আকার(মান_2);
  rpmalloc_thread_finalize(1);
}

#ifdef __APPLE__

স্থির পূর্ণসংখ্যা pthread_create_proxy(pthread_t *thread, ধ্রুবক pthread_attr_t *attr,
                                শূন্য *(*start_routine)(শূন্য *), শূন্য *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(আকার(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  ফেরত pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#অন্যথা

#অন্তর্ভাব <dlfcn.h>

পূর্ণসংখ্যা pthread_create(pthread_t *thread, ধ্রুবক pthread_attr_t *attr,
                   শূন্য *(*start_routine)(শূন্য *), শূন্য *arg) {
#যদি defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  অক্ষর fname[] = "pthread_create";
#অন্যথা
  অক্ষর fname[] = "_pthread_create";
#endif
  শূন্য *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(আকার(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  ফেরত (*(পূর্ণসংখ্যা (*)(pthread_t *, ধ্রুবক pthread_attr_t *, শূন্য *(*)(শূন্য *),
                    শূন্য *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#endif

#endif

#endif

#যদি ENABLE_OVERRIDE

#যদি defined(__GLIBC__) && defined(__linux__)

শূন্য *__libc_malloc(size_t আকার_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_আকার(1)
        RPALIAS(rpmalloc) শূন্য *__libc_calloc(size_t গুনতি, size_t আকার_3)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) শূন্য *__libc_realloc(শূন্য *p, size_t আকার_3)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_আকার(2) RPALIAS(rprealloc) শূন্য __libc_free(শূন্য *p)
        RPALIAS(rpfree) শূন্য __libc_cfree(শূন্য *p)
            RPALIAS(rpfree) শূন্য *__libc_memalign(size_t align, size_t আকার_3)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_আকার(2)
        RPALIAS(rpmemalign) পূর্ণসংখ্যা __posix_memalign(শূন্য **p, size_t align,
                                                 size_t আকার_3)
            RPALIAS(rpposix_memalign)

                বাহ্যিক শূন্য *__libc_valloc(size_t আকার_3);
বাহ্যিক শূন্য *__libc_pvalloc(size_t আকার_3);

শূন্য *__libc_valloc(size_t আকার_3) { ফেরত valloc(আকার_3); }

শূন্য *__libc_pvalloc(size_t আকার_3) { ফেরত pvalloc(আকার_3); }

#endif

#endif

#যদি (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility বের_করো
#endif
