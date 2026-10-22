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

#إذا defined(__TINYC__)
#ضمن <sys/types.h>
#پایان_اگر

#اگر_تعریف_نشده ARCH_64_بیت
#إذا defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#تعریف_کن ARCH_64_بیت 1
_Static_assert(حجم(size_t) == 8, "Data type size mismatch");
_Static_assert(حجم(فراغ *) == 8, "Data type size mismatch");
#وإلا
#تعریف_کن ARCH_64_بیت 0
_Static_assert(حجم(size_t) == 4, "Data type size mismatch");
_Static_assert(حجم(فراغ *) == 4, "Data type size mismatch");
#پایان_اگر
#پایان_اگر

#إذا (defined(__GNUC__) || defined(__clang__))
#رهنمود GCC visibility افزودن(افتراضي)
#پایان_اگر

#تعریف_کن USE_IMPLEMENT 1
#تعریف_کن USE_INTERPOSE 0
#تعریف_کن USE_ALIAS 0

#إذا defined(__APPLE__)
#لغو_تعریف USE_INTERPOSE
#تعریف_کن USE_INTERPOSE 1

تعريفنوع بنية interpose_t {
  فراغ *new_func;
  فراغ *orig_func;
} interpose_t;

#تعریف_کن MAC_INTERPOSE_PAIR(newf, oldf) {(فراغ *)newf, (فراغ *)oldf}
#تعریف_کن MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((استفاده_شده)) ساكن ثابت interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#پایان_اگر

#إذا !defined(_WIN32) && !defined(__APPLE__)
#لغو_تعریف USE_IMPLEMENT
#لغو_تعریف USE_ALIAS
#تعریف_کن USE_IMPLEMENT 0
#تعریف_کن USE_ALIAS 1
#پایان_اگر

#اگر_تعریف_شده _MSC_VER
#رهنمود warning(disable : 4100)
#لغو_تعریف حافظه_بده
#لغو_تعریف حرر
#لغو_تعریف حافظه_صفر_بده
#تعریف_کن RPMALLOC_RESTRICT __declspec(محدود)
#وإلا
#تعریف_کن RPMALLOC_RESTRICT
#پایان_اگر

#إذا ENABLE_OVERRIDE

تعريفنوع بنية rp_nothrow_t {
  صحيح __dummy;
} rp_nothrow_t;

#إذا USE_IMPLEMENT

خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL حافظه_بده(size_t اندازه_2) {
  أرجع rpmalloc(اندازه_2);
}
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL حافظه_صفر_بده(size_t شمار,
                                                            size_t اندازه_2) {
  أرجع rpcalloc(شمار, اندازه_2);
}
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL realloc(فراغ *ptr,
                                                             size_t اندازه_2) {
  أرجع rprealloc(ptr, اندازه_2);
}
خارجي مضمن فراغ *RPMALLOC_CDECL reallocf(فراغ *ptr, size_t اندازه_2) {
  أرجع rprealloc(ptr, اندازه_2);
}
خارجي مضمن فراغ *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t اندازه_2) {
  أرجع rpaligned_alloc(alignment, اندازه_2);
}
خارجي مضمن فراغ *RPMALLOC_CDECL memalign(size_t alignment, size_t اندازه_2) {
  أرجع rpmemalign(alignment, اندازه_2);
}
خارجي مضمن صحيح RPMALLOC_CDECL posix_memalign(فراغ **memptr, size_t alignment,
                                                size_t اندازه_2) {
  أرجع rpposix_memalign(memptr, alignment, اندازه_2);
}
خارجي مضمن فراغ RPMALLOC_CDECL حرر(فراغ *ptr) { rpfree(ptr); }
خارجي مضمن فراغ RPMALLOC_CDECL cfree(فراغ *ptr) { rpfree(ptr); }
خارجي مضمن size_t RPMALLOC_CDECL malloc_usable_اندازه(فراغ *ptr) {
  أرجع rpmalloc_usable_اندازه(ptr);
}
خارجي مضمن size_t RPMALLOC_CDECL malloc_اندازه(فراغ *ptr) {
  أرجع rpmalloc_usable_اندازه(ptr);
}

#اگر_تعریف_شده _WIN32
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL malloc_پایه(size_t اندازه_2) {
  أرجع rpmalloc(اندازه_2);
}
خارجي مضمن فراغ RPMALLOC_CDECL free_پایه(فراغ *ptr) { rpfree(ptr); }
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL calloc_پایه(size_t شمار,
                                                                  size_t اندازه_2) {
  أرجع rpcalloc(شمار, اندازه_2);
}
خارجي مضمن size_t RPMALLOC_CDECL _msize(فراغ *ptr) {
  أرجع rpmalloc_usable_اندازه(ptr);
}
خارجي مضمن size_t RPMALLOC_CDECL msize_پایه(فراغ *ptr) {
  أرجع rpmalloc_usable_اندازه(ptr);
}
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL
realloc_پایه(فراغ *ptr, size_t اندازه_2) {
  أرجع rprealloc(ptr, اندازه_2);
}
#پایان_اگر

#اگر_تعریف_شده _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#وإلا
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#تعریف_کن RPDEFVIS __attribute__((visibility("default")))
خارجي فراغ _ZdlPv(فراغ *p);
فراغ RPDEFVIS _ZdlPv(فراغ *p) { rpfree(p); }
خارجي فراغ _ZdaPv(فراغ *p);
فراغ RPDEFVIS _ZdaPv(فراغ *p) { rpfree(p); }
#إذا ARCH_64_بیت
// 64-bit operators new and new[], normal and aligned
خارجي فراغ *_Znwm(uint64_t اندازه_2);
فراغ *RPDEFVIS _Znwm(uint64_t اندازه_2) { أرجع rpmalloc(اندازه_2); }
خارجي فراغ *_Znam(uint64_t اندازه_2);
فراغ *RPDEFVIS _Znam(uint64_t اندازه_2) { أرجع rpmalloc(اندازه_2); }
خارجي فراغ *_Znwmm(uint64_t اندازه_2, uint64_t align);
فراغ *RPDEFVIS _Znwmm(uint64_t اندازه_2, uint64_t align) {
  أرجع rpaligned_alloc(align, اندازه_2);
}
خارجي فراغ *_Znamm(uint64_t اندازه_2, uint64_t align);
فراغ *RPDEFVIS _Znamm(uint64_t اندازه_2, uint64_t align) {
  أرجع rpaligned_alloc(align, اندازه_2);
}
خارجي فراغ *_ZnwmSt11align_val_t(uint64_t اندازه_2, uint64_t align);
فراغ *RPDEFVIS _ZnwmSt11align_val_t(uint64_t اندازه_2, uint64_t align) {
  أرجع rpaligned_alloc(align, اندازه_2);
}
خارجي فراغ *_ZnamSt11align_val_t(uint64_t اندازه_2, uint64_t align);
فراغ *RPDEFVIS _ZnamSt11align_val_t(uint64_t اندازه_2, uint64_t align) {
  أرجع rpaligned_alloc(align, اندازه_2);
}
خارجي فراغ *_ZnwmRKSt9nothrow_t(uint64_t اندازه_2, rp_nothrow_t t);
فراغ *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t اندازه_2, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(اندازه_2);
}
خارجي فراغ *_ZnamRKSt9nothrow_t(uint64_t اندازه_2, rp_nothrow_t t);
فراغ *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t اندازه_2, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(اندازه_2);
}
خارجي فراغ *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t اندازه_2, uint64_t align,
                                                rp_nothrow_t t);
فراغ *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t اندازه_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, اندازه_2);
}
خارجي فراغ *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t اندازه_2, uint64_t align,
                                                rp_nothrow_t t);
فراغ *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t اندازه_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, اندازه_2);
}
// 64-bit operators sized delete and delete[], normal and aligned
خارجي فراغ _ZdlPvm(فراغ *p, uint64_t اندازه_2);
فراغ RPDEFVIS _ZdlPvm(فراغ *p, uint64_t اندازه_2) {
  rpfree(p);
  (فراغ)حجم(اندازه_2);
}
خارجي فراغ _ZdaPvm(فراغ *p, uint64_t اندازه_2);
فراغ RPDEFVIS _ZdaPvm(فراغ *p, uint64_t اندازه_2) {
  rpfree(p);
  (فراغ)حجم(اندازه_2);
}
خارجي فراغ _ZdlPvSt11align_val_t(فراغ *p, uint64_t align);
فراغ RPDEFVIS _ZdlPvSt11align_val_t(فراغ *p, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(align);
}
خارجي فراغ _ZdaPvSt11align_val_t(فراغ *p, uint64_t align);
فراغ RPDEFVIS _ZdaPvSt11align_val_t(فراغ *p, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(align);
}
خارجي فراغ _ZdlPvmSt11align_val_t(فراغ *p, uint64_t اندازه_2, uint64_t align);
فراغ RPDEFVIS _ZdlPvmSt11align_val_t(فراغ *p, uint64_t اندازه_2, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(اندازه_2);
  (فراغ)حجم(align);
}
خارجي فراغ _ZdaPvmSt11align_val_t(فراغ *p, uint64_t اندازه_2, uint64_t align);
فراغ RPDEFVIS _ZdaPvmSt11align_val_t(فراغ *p, uint64_t اندازه_2, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(اندازه_2);
  (فراغ)حجم(align);
}
#وإلا
// 32-bit operators new and new[], normal and aligned
خارجي فراغ *_Znwj(uint32_t اندازه_2);
فراغ *RPDEFVIS _Znwj(uint32_t اندازه_2) { أرجع rpmalloc(اندازه_2); }
خارجي فراغ *_Znaj(uint32_t اندازه_2);
فراغ *RPDEFVIS _Znaj(uint32_t اندازه_2) { أرجع rpmalloc(اندازه_2); }
خارجي فراغ *_Znwjj(uint32_t اندازه_2, uint32_t align);
فراغ *RPDEFVIS _Znwjj(uint32_t اندازه_2, uint32_t align) {
  أرجع rpaligned_alloc(align, اندازه_2);
}
خارجي فراغ *_Znajj(uint32_t اندازه_2, uint32_t align);
فراغ *RPDEFVIS _Znajj(uint32_t اندازه_2, uint32_t align) {
  أرجع rpaligned_alloc(align, اندازه_2);
}
خارجي فراغ *_ZnwjSt11align_val_t(size_t اندازه_2, size_t align);
فراغ *RPDEFVIS _ZnwjSt11align_val_t(size_t اندازه_2, size_t align) {
  أرجع rpaligned_alloc(align, اندازه_2);
}
خارجي فراغ *_ZnajSt11align_val_t(size_t اندازه_2, size_t align);
فراغ *RPDEFVIS _ZnajSt11align_val_t(size_t اندازه_2, size_t align) {
  أرجع rpaligned_alloc(align, اندازه_2);
}
خارجي فراغ *_ZnwjRKSt9nothrow_t(size_t اندازه_2, rp_nothrow_t t);
فراغ *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t اندازه_2, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(اندازه_2);
}
خارجي فراغ *_ZnajRKSt9nothrow_t(size_t اندازه_2, rp_nothrow_t t);
فراغ *RPDEFVIS _ZnajRKSt9nothrow_t(size_t اندازه_2, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(اندازه_2);
}
خارجي فراغ *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t اندازه_2, size_t align,
                                                rp_nothrow_t t);
فراغ *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t اندازه_2, size_t align,
                                                  rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, اندازه_2);
}
خارجي فراغ *_ZnajSt11align_val_tRKSt9nothrow_t(size_t اندازه_2, size_t align,
                                                rp_nothrow_t t);
فراغ *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t اندازه_2, size_t align,
                                                  rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, اندازه_2);
}
// 32-bit operators sized delete and delete[], normal and aligned
خارجي فراغ _ZdlPvj(فراغ *p, uint64_t اندازه_2);
فراغ RPDEFVIS _ZdlPvj(فراغ *p, uint64_t اندازه_2) {
  rpfree(p);
  (فراغ)حجم(اندازه_2);
}
خارجي فراغ _ZdaPvj(فراغ *p, uint64_t اندازه_2);
فراغ RPDEFVIS _ZdaPvj(فراغ *p, uint64_t اندازه_2) {
  rpfree(p);
  (فراغ)حجم(اندازه_2);
}
خارجي فراغ _ZdlPvSt11align_val_t(فراغ *p, uint32_t align);
فراغ RPDEFVIS _ZdlPvSt11align_val_t(فراغ *p, uint64_t a) {
  rpfree(p);
  (فراغ)حجم(align);
}
خارجي فراغ _ZdaPvSt11align_val_t(فراغ *p, uint32_t align);
فراغ RPDEFVIS _ZdaPvSt11align_val_t(فراغ *p, uint64_t a) {
  rpfree(p);
  (فراغ)حجم(align);
}
خارجي فراغ _ZdlPvjSt11align_val_t(فراغ *p, uint32_t اندازه_2, uint32_t align);
فراغ RPDEFVIS _ZdlPvjSt11align_val_t(فراغ *p, uint64_t اندازه_2, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(اندازه_2);
  (فراغ)حجم(a);
}
خارجي فراغ _ZdaPvjSt11align_val_t(فراغ *p, uint32_t اندازه_2, uint32_t align);
فراغ RPDEFVIS _ZdaPvjSt11align_val_t(فراغ *p, uint64_t اندازه_2, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(اندازه_2);
  (فراغ)حجم(a);
}
#پایان_اگر
#پایان_اگر
#پایان_اگر

#إذا USE_INTERPOSE || USE_ALIAS

ساكن فراغ *rpmalloc_nothrow(size_t اندازه_2, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(اندازه_2);
}
ساكن فراغ *rpaligned_alloc_معکوس(size_t اندازه_2, size_t align) {
  أرجع rpaligned_alloc(align, اندازه_2);
}
ساكن فراغ *rpaligned_alloc_معکوس_nothrow(size_t اندازه_2, size_t align,
                                             rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, اندازه_2);
}
ساكن فراغ rpfree_اندازه(فراغ *p, size_t اندازه_2) {
  (فراغ)حجم(اندازه_2);
  rpfree(p);
}
ساكن فراغ rpfree_aligned(فراغ *p, size_t align) {
  (فراغ)حجم(align);
  rpfree(p);
}
ساكن فراغ rpfree_اندازه_aligned(فراغ *p, size_t اندازه_2, size_t align) {
  (فراغ)حجم(اندازه_2);
  (فراغ)حجم(align);
  rpfree(p);
}

#پایان_اگر

#إذا USE_INTERPOSE

__attribute__((استفاده_شده)) ساكن ثابت interpose_t macinterpose_malloc[]
    __attribute__((section("__DATA, __interpose"))) = {
        // new and new[]
        MAC_INTERPOSE_PAIR(rpmalloc, _Znwm),
        MAC_INTERPOSE_PAIR(rpmalloc, _Znam),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_معکوس, _Znwmm),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_معکوس, _Znamm),
        MAC_INTERPOSE_PAIR(rpmalloc_nothrow, _ZnwmRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpmalloc_nothrow, _ZnamRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_معکوس, _ZnwmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_معکوس, _ZnamSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_معکوس_nothrow,
                           _ZnwmSt11align_val_tRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_معکوس_nothrow,
                           _ZnamSt11align_val_tRKSt9nothrow_t),
        // delete and delete[]
        MAC_INTERPOSE_PAIR(rpfree, _ZdlPv), MAC_INTERPOSE_PAIR(rpfree, _ZdaPv),
        MAC_INTERPOSE_PAIR(rpfree_اندازه, _ZdlPvm),
        MAC_INTERPOSE_PAIR(rpfree_اندازه, _ZdaPvm),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdlPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdaPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_اندازه_aligned, _ZdlPvmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_اندازه_aligned, _ZdaPvmSt11align_val_t),
        // libc entry points
        MAC_INTERPOSE_PAIR(rpmalloc, حافظه_بده),
        MAC_INTERPOSE_PAIR(rpmalloc, حافظه_صفر_بده),
        MAC_INTERPOSE_PAIR(rprealloc, realloc),
        MAC_INTERPOSE_PAIR(rprealloc, reallocf),
#إذا defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#پایان_اگر
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, حرر), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_اندازه, malloc_usable_اندازه),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_اندازه, malloc_اندازه)};

#پایان_اگر

#إذا USE_ALIAS

#تعریف_کن RPALIAS(fn) __attribute__((alias(#fn), استفاده_شده, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
فراغ _ZdlPv(فراغ *p) RPALIAS(rpfree) فراغ _ZdaPv(فراغ *p) RPALIAS(rpfree)

#إذا ARCH_64_بیت
    // 64-bit operators new and new[], normal and aligned
    فراغ *_Znwm(uint64_t اندازه_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_اندازه(1)
        RPALIAS(rpmalloc) فراغ *_Znam(uint64_t اندازه_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_اندازه(1) RPALIAS(rpmalloc) فراغ *_Znwmm(uint64_t اندازه_2,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_معکوس) فراغ *_Znamm(uint64_t اندازه_2,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_معکوس) فراغ *_ZnwmSt11align_val_t(
                size_t اندازه_2, size_t align)
                RPALIAS(rpaligned_alloc_معکوس) فراغ *_ZnamSt11align_val_t(
                    size_t اندازه_2, size_t align)
                    RPALIAS(rpaligned_alloc_معکوس) فراغ *_ZnwmRKSt9nothrow_t(
                        size_t اندازه_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) فراغ *_ZnamRKSt9nothrow_t(
                            size_t اندازه_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) فراغ
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t اندازه_2,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_معکوس_nothrow) فراغ
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t اندازه_2, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_معکوس_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    فراغ _ZdlPvm(فراغ *p, size_t n) RPALIAS(rpfree_اندازه) فراغ _ZdaPvm(فراغ *p,
                                                                      size_t n)
        RPALIAS(rpfree_اندازه) فراغ _ZdlPvSt11align_val_t(فراغ *p, size_t a)
            RPALIAS(rpfree_aligned) فراغ _ZdaPvSt11align_val_t(فراغ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) فراغ _ZdlPvmSt11align_val_t(فراغ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_اندازه_aligned) فراغ _ZdaPvmSt11align_val_t(
                        فراغ *p, size_t n, size_t a)
                        RPALIAS(rpfree_اندازه_aligned)
#وإلا
    // 32-bit operators new and new[], normal and aligned
    فراغ *_Znwj(uint32_t اندازه_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_اندازه(1)
        RPALIAS(rpmalloc) فراغ *_Znaj(uint32_t اندازه_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_اندازه(1) RPALIAS(rpmalloc) فراغ *_Znwjj(uint32_t اندازه_2,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_معکوس) فراغ *_Znajj(uint32_t اندازه_2,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_معکوس) فراغ *_ZnwjSt11align_val_t(
                size_t اندازه_2, size_t align)
                RPALIAS(rpaligned_alloc_معکوس) فراغ *_ZnajSt11align_val_t(
                    size_t اندازه_2, size_t align)
                    RPALIAS(rpaligned_alloc_معکوس) فراغ *_ZnwjRKSt9nothrow_t(
                        size_t اندازه_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) فراغ *_ZnajRKSt9nothrow_t(
                            size_t اندازه_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) فراغ
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t اندازه_2,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_معکوس_nothrow) فراغ
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t اندازه_2, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_معکوس_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    فراغ _ZdlPvj(فراغ *p, size_t n) RPALIAS(rpfree_اندازه) فراغ _ZdaPvj(فراغ *p,
                                                                      size_t n)
        RPALIAS(rpfree_اندازه) فراغ _ZdlPvSt11align_val_t(فراغ *p, size_t a)
            RPALIAS(rpfree_aligned) فراغ _ZdaPvSt11align_val_t(فراغ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) فراغ _ZdlPvjSt11align_val_t(فراغ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_اندازه_aligned) فراغ _ZdaPvjSt11align_val_t(
                        فراغ *p, size_t n, size_t a)
                        RPALIAS(rpfree_اندازه_aligned)
#پایان_اگر

                            فراغ *حافظه_بده(size_t اندازه_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_اندازه(1)
        RPALIAS(rpmalloc) فراغ *حافظه_صفر_بده(size_t شمار, size_t اندازه_2)
            RPALIAS(rpcalloc) فراغ *realloc(فراغ *ptr, size_t اندازه_2)
                RPALIAS(rprealloc) فراغ *reallocf(فراغ *ptr, size_t اندازه_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_اندازه(2)
        RPALIAS(rprealloc) فراغ *aligned_alloc(size_t alignment, size_t اندازه_2)
            RPALIAS(rpaligned_alloc) فراغ *memalign(
                size_t alignment, size_t اندازه_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_اندازه(2)
        RPALIAS(rpmemalign) صحيح posix_memalign(فراغ **memptr, size_t alignment,
                                               size_t اندازه_2)
            RPALIAS(rpposix_memalign) فراغ حرر(فراغ *ptr)
                RPALIAS(rpfree) فراغ cfree(فراغ *ptr) RPALIAS(rpfree)
#إذا defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_اندازه(ثابت فراغ *ptr) RPALIAS(rpmalloc_usable_اندازه)
#وإلا
                    size_t
    malloc_usable_اندازه(فراغ *ptr) RPALIAS(rpmalloc_usable_اندازه)
#پایان_اگر
        size_t malloc_اندازه(فراغ *ptr) RPALIAS(rpmalloc_usable_اندازه)

#پایان_اگر

            ساكن مضمن size_t rpmalloc_صفحه_اندازه(فراغ) {
  أرجع حافظه_صفحه_اندازه;
}

خارجي فراغ *RPMALLOC_CDECL reallocarray(فراغ *ptr, size_t شمار, size_t اندازه_2);

خارجي فراغ *RPMALLOC_CDECL reallocarray(فراغ *ptr, size_t شمار, size_t اندازه_2) {
  size_t مجموع_2;
#إذا ENABLE_VALIDATE_آرگومانها
#اگر_تعریف_شده _MSC_VER
  صحيح err = اندازه_TMult(شمار, اندازه_2, &مجموع_2);
  إذا ((err != S_OK) || (مجموع_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(شمار, اندازه_2, &مجموع_2);
  إذا (err || (مجموع_2 >= MAX_ALLOC_اندازه)) {
    errno = EINVAL;
    أرجع 0;
  }
#پایان_اگر
#وإلا
  مجموع_2 = شمار * اندازه_2;
#پایان_اگر
  أرجع realloc(ptr, مجموع_2);
}

خارجي مضمن فراغ *RPMALLOC_CDECL valloc(size_t اندازه_2) {
  get_thread_انباشته();
  أرجع rpaligned_alloc(rpmalloc_صفحه_اندازه(), اندازه_2);
}

خارجي مضمن فراغ *RPMALLOC_CDECL pvalloc(size_t اندازه_2) {
  get_thread_انباشته();
  ثابت size_t صفحه_اندازه_2 = rpmalloc_صفحه_اندازه();
  ثابت size_t aligned_اندازه = ((اندازه_2 + صفحه_اندازه_2 - 1) / صفحه_اندازه_2) * صفحه_اندازه_2;
#إذا ENABLE_VALIDATE_آرگومانها
  إذا (aligned_اندازه < اندازه_2) {
    errno = EINVAL;
    أرجع 0;
  }
#پایان_اگر
  أرجع rpaligned_alloc(rpmalloc_صفحه_اندازه(), aligned_اندازه);
}

#پایان_اگر // ENABLE_OVERRIDE

#إذا ENABLE_PRELOAD

#اگر_تعریف_شده _WIN32

#إذا defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

خارجي __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

خارجي __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (فراغ)حجم(reserved);
  (فراغ)حجم(instance);
  إذا (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  وإلا إذا (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  وإلا إذا (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  وإلا إذا (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  أرجع TRUE;
}

// end BUILD_DYNAMIC_LINK
#وإلا

خارجي فراغ _global_rpmalloc_init(فراغ) {
  rpmalloc_مجموعه_main_thread();
  rpmalloc_initialize();
}

#إذا defined(__clang__) || defined(__GNUC__)

ساكن فراغ __attribute__((constructor)) initializer(فراغ) {
  _global_rpmalloc_init();
}

#وإلاإذا defined(_MSC_VER)

ساكن صحيح _global_rpmalloc_xib(فراغ) {
  _global_rpmalloc_init();
  أرجع 0;
}

#رهنمود section(".CRT$XIB", اقرأ)
__declspec(allocate(".CRT$XIB")) فراغ (*_rpmalloc_module_init)(فراغ) =
    _global_rpmalloc_xib;
#إذا defined(_M_IX86) || defined(__i386__)
#رهنمود comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#وإلا
#رهنمود comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#پایان_اگر

#پایان_اگر

// end !BUILD_DYNAMIC_LINK
#پایان_اگر

#وإلا

#ضمن <pthread.h>
#ضمن <stdint.h>
#ضمن <stdlib.h>
#ضمن <unistd.h>

خارجي فراغ rpmalloc_مجموعه_main_thread(فراغ);

ساكن pthread_key_t destructor_کلید;

ساكن فراغ thread_destructor(فراغ *);

ساكن فراغ __attribute__((constructor)) initializer(فراغ) {
  rpmalloc_مجموعه_main_thread();
  rpmalloc_initialize();
  pthread_کلید_create(&destructor_کلید, thread_destructor);
}

ساكن فراغ __attribute__((destructor)) finalizer(فراغ) { rpmalloc_finalize(); }

تعريفنوع بنية {
  فراغ *(*real_شروع)(فراغ *);
  فراغ *real_arg;
} thread_starter_arg;

ساكن فراغ *thread_starter(فراغ *argptr) {
  thread_starter_arg *arg = argptr;
  فراغ *(*real_شروع)(فراغ *) = arg->real_شروع;
  فراغ *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_کلید, (فراغ *)1);
  أرجع (*real_شروع)(real_arg);
}

ساكن فراغ thread_destructor(فراغ *مقدار_2) {
  (فراغ)حجم(مقدار_2);
  rpmalloc_thread_finalize(1);
}

#اگر_تعریف_شده __APPLE__

ساكن صحيح pthread_create_proxy(pthread_t *thread, ثابت pthread_attr_t *attr,
                                فراغ *(*شروع_routine)(فراغ *), فراغ *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(حجم(thread_starter_arg));
  starter_arg->real_شروع = شروع_routine;
  starter_arg->real_arg = arg;
  أرجع pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#وإلا

#ضمن <dlfcn.h>

صحيح pthread_create(pthread_t *thread, ثابت pthread_attr_t *attr,
                   فراغ *(*شروع_routine)(فراغ *), فراغ *arg) {
#إذا defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  حرف fname[] = "pthread_create";
#وإلا
  حرف fname[] = "_pthread_create";
#پایان_اگر
  فراغ *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(حجم(thread_starter_arg));
  starter_arg->real_شروع = شروع_routine;
  starter_arg->real_arg = arg;
  أرجع (*(صحيح (*)(pthread_t *, ثابت pthread_attr_t *, فراغ *(*)(فراغ *),
                    فراغ *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#پایان_اگر

#پایان_اگر

#پایان_اگر

#إذا ENABLE_OVERRIDE

#إذا defined(__GLIBC__) && defined(__linux__)

فراغ *__libc_malloc(size_t اندازه_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_اندازه(1)
        RPALIAS(rpmalloc) فراغ *__libc_calloc(size_t شمار, size_t اندازه_2)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) فراغ *__libc_realloc(فراغ *p, size_t اندازه_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_اندازه(2) RPALIAS(rprealloc) فراغ __libc_free(فراغ *p)
        RPALIAS(rpfree) فراغ __libc_cfree(فراغ *p)
            RPALIAS(rpfree) فراغ *__libc_memalign(size_t align, size_t اندازه_2)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_اندازه(2)
        RPALIAS(rpmemalign) صحيح __posix_memalign(فراغ **p, size_t align,
                                                 size_t اندازه_2)
            RPALIAS(rpposix_memalign)

                خارجي فراغ *__libc_valloc(size_t اندازه_2);
خارجي فراغ *__libc_pvalloc(size_t اندازه_2);

فراغ *__libc_valloc(size_t اندازه_2) { أرجع valloc(اندازه_2); }

فراغ *__libc_pvalloc(size_t اندازه_2) { أرجع pvalloc(اندازه_2); }

#پایان_اگر

#پایان_اگر

#إذا (defined(__GNUC__) || defined(__clang__))
#رهنمود GCC visibility برداشت
#پایان_اگر
