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

#𐡄𐡍 defined(__TINYC__)
#ضمن <sys/types.h>
#endif

#ifndef ARCH_64BIT
#𐡄𐡍 defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#define ARCH_64BIT 1
_Static_assert(حجم(size_t) == 8, "Data type size mismatch");
_Static_assert(حجم(فراغ *) == 8, "Data type size mismatch");
#وإلا
#define ARCH_64BIT 0
_Static_assert(حجم(size_t) == 4, "Data type size mismatch");
_Static_assert(حجم(فراغ *) == 4, "Data type size mismatch");
#endif
#endif

#𐡄𐡍 (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility push(افتراضي)
#endif

#define USE_IMPLEMENT 1
#define USE_INTERPOSE 0
#define USE_ALIAS 0

#𐡄𐡍 defined(__APPLE__)
#undef USE_INTERPOSE
#define USE_INTERPOSE 1

تعريفنوع بنية interpose_t {
  فراغ *new_func;
  فراغ *orig_func;
} interpose_t;

#define MAC_INTERPOSE_PAIR(newf, oldf) {(فراغ *)newf, (فراغ *)oldf}
#define MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((used)) ساكن ثابت interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#endif

#𐡄𐡍 !defined(_WIN32) && !defined(__APPLE__)
#undef USE_IMPLEMENT
#undef USE_ALIAS
#define USE_IMPLEMENT 0
#define USE_ALIAS 1
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#undef malloc
#undef حرر
#undef calloc
#define RPMALLOC_RESTRICT __declspec(restrict)
#وإلا
#define RPMALLOC_RESTRICT
#endif

#𐡄𐡍 ENABLE_OVERRIDE

تعريفنوع بنية rp_nothrow_t {
  صحيح __dummy;
} rp_nothrow_t;

#𐡄𐡍 USE_IMPLEMENT

خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL malloc(size_t size) {
  أرجع rpmalloc(size);
}
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL calloc(size_t count,
                                                            size_t size) {
  أرجع rpcalloc(count, size);
}
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL realloc(فراغ *ptr,
                                                             size_t size) {
  أرجع rprealloc(ptr, size);
}
خارجي مضمن فراغ *RPMALLOC_CDECL reallocf(فراغ *ptr, size_t size) {
  أرجع rprealloc(ptr, size);
}
خارجي مضمن فراغ *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t size) {
  أرجع rpaligned_alloc(alignment, size);
}
خارجي مضمن فراغ *RPMALLOC_CDECL memalign(size_t alignment, size_t size) {
  أرجع rpmemalign(alignment, size);
}
خارجي مضمن صحيح RPMALLOC_CDECL posix_memalign(فراغ **memptr, size_t alignment,
                                                size_t size) {
  أرجع rpposix_memalign(memptr, alignment, size);
}
خارجي مضمن فراغ RPMALLOC_CDECL حرر(فراغ *ptr) { rpfree(ptr); }
خارجي مضمن فراغ RPMALLOC_CDECL cfree(فراغ *ptr) { rpfree(ptr); }
خارجي مضمن size_t RPMALLOC_CDECL malloc_usable_size(فراغ *ptr) {
  أرجع rpmalloc_usable_size(ptr);
}
خارجي مضمن size_t RPMALLOC_CDECL malloc_size(فراغ *ptr) {
  أرجع rpmalloc_usable_size(ptr);
}

#ifdef _WIN32
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL _malloc_base(size_t size) {
  أرجع rpmalloc(size);
}
خارجي مضمن فراغ RPMALLOC_CDECL _free_base(فراغ *ptr) { rpfree(ptr); }
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL _calloc_base(size_t count,
                                                                  size_t size) {
  أرجع rpcalloc(count, size);
}
خارجي مضمن size_t RPMALLOC_CDECL _msize(فراغ *ptr) {
  أرجع rpmalloc_usable_size(ptr);
}
خارجي مضمن size_t RPMALLOC_CDECL _msize_base(فراغ *ptr) {
  أرجع rpmalloc_usable_size(ptr);
}
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL
_realloc_base(فراغ *ptr, size_t size) {
  أرجع rprealloc(ptr, size);
}
#endif

#ifdef _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#وإلا
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#define RPDEFVIS __attribute__((visibility("default")))
خارجي فراغ _ZdlPv(فراغ *p);
فراغ RPDEFVIS _ZdlPv(فراغ *p) { rpfree(p); }
خارجي فراغ _ZdaPv(فراغ *p);
فراغ RPDEFVIS _ZdaPv(فراغ *p) { rpfree(p); }
#𐡄𐡍 ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
خارجي فراغ *_Znwm(uint64_t size);
فراغ *RPDEFVIS _Znwm(uint64_t size) { أرجع rpmalloc(size); }
خارجي فراغ *_Znam(uint64_t size);
فراغ *RPDEFVIS _Znam(uint64_t size) { أرجع rpmalloc(size); }
خارجي فراغ *_Znwmm(uint64_t size, uint64_t align);
فراغ *RPDEFVIS _Znwmm(uint64_t size, uint64_t align) {
  أرجع rpaligned_alloc(align, size);
}
خارجي فراغ *_Znamm(uint64_t size, uint64_t align);
فراغ *RPDEFVIS _Znamm(uint64_t size, uint64_t align) {
  أرجع rpaligned_alloc(align, size);
}
خارجي فراغ *_ZnwmSt11align_val_t(uint64_t size, uint64_t align);
فراغ *RPDEFVIS _ZnwmSt11align_val_t(uint64_t size, uint64_t align) {
  أرجع rpaligned_alloc(align, size);
}
خارجي فراغ *_ZnamSt11align_val_t(uint64_t size, uint64_t align);
فراغ *RPDEFVIS _ZnamSt11align_val_t(uint64_t size, uint64_t align) {
  أرجع rpaligned_alloc(align, size);
}
خارجي فراغ *_ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
فراغ *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(size);
}
خارجي فراغ *_ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
فراغ *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(size);
}
خارجي فراغ *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
فراغ *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, size);
}
خارجي فراغ *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
فراغ *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, size);
}
// 64-bit operators sized delete and delete[], normal and aligned
خارجي فراغ _ZdlPvm(فراغ *p, uint64_t size);
فراغ RPDEFVIS _ZdlPvm(فراغ *p, uint64_t size) {
  rpfree(p);
  (فراغ)حجم(size);
}
خارجي فراغ _ZdaPvm(فراغ *p, uint64_t size);
فراغ RPDEFVIS _ZdaPvm(فراغ *p, uint64_t size) {
  rpfree(p);
  (فراغ)حجم(size);
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
خارجي فراغ _ZdlPvmSt11align_val_t(فراغ *p, uint64_t size, uint64_t align);
فراغ RPDEFVIS _ZdlPvmSt11align_val_t(فراغ *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(size);
  (فراغ)حجم(align);
}
خارجي فراغ _ZdaPvmSt11align_val_t(فراغ *p, uint64_t size, uint64_t align);
فراغ RPDEFVIS _ZdaPvmSt11align_val_t(فراغ *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(size);
  (فراغ)حجم(align);
}
#وإلا
// 32-bit operators new and new[], normal and aligned
خارجي فراغ *_Znwj(uint32_t size);
فراغ *RPDEFVIS _Znwj(uint32_t size) { أرجع rpmalloc(size); }
خارجي فراغ *_Znaj(uint32_t size);
فراغ *RPDEFVIS _Znaj(uint32_t size) { أرجع rpmalloc(size); }
خارجي فراغ *_Znwjj(uint32_t size, uint32_t align);
فراغ *RPDEFVIS _Znwjj(uint32_t size, uint32_t align) {
  أرجع rpaligned_alloc(align, size);
}
خارجي فراغ *_Znajj(uint32_t size, uint32_t align);
فراغ *RPDEFVIS _Znajj(uint32_t size, uint32_t align) {
  أرجع rpaligned_alloc(align, size);
}
خارجي فراغ *_ZnwjSt11align_val_t(size_t size, size_t align);
فراغ *RPDEFVIS _ZnwjSt11align_val_t(size_t size, size_t align) {
  أرجع rpaligned_alloc(align, size);
}
خارجي فراغ *_ZnajSt11align_val_t(size_t size, size_t align);
فراغ *RPDEFVIS _ZnajSt11align_val_t(size_t size, size_t align) {
  أرجع rpaligned_alloc(align, size);
}
خارجي فراغ *_ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t);
فراغ *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(size);
}
خارجي فراغ *_ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t);
فراغ *RPDEFVIS _ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(size);
}
خارجي فراغ *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
فراغ *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, size);
}
خارجي فراغ *_ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
فراغ *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, size);
}
// 32-bit operators sized delete and delete[], normal and aligned
خارجي فراغ _ZdlPvj(فراغ *p, uint64_t size);
فراغ RPDEFVIS _ZdlPvj(فراغ *p, uint64_t size) {
  rpfree(p);
  (فراغ)حجم(size);
}
خارجي فراغ _ZdaPvj(فراغ *p, uint64_t size);
فراغ RPDEFVIS _ZdaPvj(فراغ *p, uint64_t size) {
  rpfree(p);
  (فراغ)حجم(size);
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
خارجي فراغ _ZdlPvjSt11align_val_t(فراغ *p, uint32_t size, uint32_t align);
فراغ RPDEFVIS _ZdlPvjSt11align_val_t(فراغ *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(size);
  (فراغ)حجم(a);
}
خارجي فراغ _ZdaPvjSt11align_val_t(فراغ *p, uint32_t size, uint32_t align);
فراغ RPDEFVIS _ZdaPvjSt11align_val_t(فراغ *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(size);
  (فراغ)حجم(a);
}
#endif
#endif
#endif

#𐡄𐡍 USE_INTERPOSE || USE_ALIAS

ساكن فراغ *rpmalloc_nothrow(size_t size, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(size);
}
ساكن فراغ *rpaligned_alloc_reverse(size_t size, size_t align) {
  أرجع rpaligned_alloc(align, size);
}
ساكن فراغ *rpaligned_alloc_reverse_nothrow(size_t size, size_t align,
                                             rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, size);
}
ساكن فراغ rpfree_size(فراغ *p, size_t size) {
  (فراغ)حجم(size);
  rpfree(p);
}
ساكن فراغ rpfree_aligned(فراغ *p, size_t align) {
  (فراغ)حجم(align);
  rpfree(p);
}
ساكن فراغ rpfree_size_aligned(فراغ *p, size_t size, size_t align) {
  (فراغ)حجم(size);
  (فراغ)حجم(align);
  rpfree(p);
}

#endif

#𐡄𐡍 USE_INTERPOSE

__attribute__((used)) ساكن ثابت interpose_t macinterpose_malloc[]
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
#𐡄𐡍 defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#endif
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, حرر), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_usable_size),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_size)};

#endif

#𐡄𐡍 USE_ALIAS

#define RPALIAS(fn) __attribute__((alias(#fn), used, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
فراغ _ZdlPv(فراغ *p) RPALIAS(rpfree) فراغ _ZdaPv(فراغ *p) RPALIAS(rpfree)

#𐡄𐡍 ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    فراغ *_Znwm(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) فراغ *_Znam(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) فراغ *_Znwmm(uint64_t size,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) فراغ *_Znamm(uint64_t size,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) فراغ *_ZnwmSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) فراغ *_ZnamSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) فراغ *_ZnwmRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) فراغ *_ZnamRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) فراغ
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) فراغ
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    فراغ _ZdlPvm(فراغ *p, size_t n) RPALIAS(rpfree_size) فراغ _ZdaPvm(فراغ *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) فراغ _ZdlPvSt11align_val_t(فراغ *p, size_t a)
            RPALIAS(rpfree_aligned) فراغ _ZdaPvSt11align_val_t(فراغ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) فراغ _ZdlPvmSt11align_val_t(فراغ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) فراغ _ZdaPvmSt11align_val_t(
                        فراغ *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#وإلا
    // 32-bit operators new and new[], normal and aligned
    فراغ *_Znwj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) فراغ *_Znaj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) فراغ *_Znwjj(uint32_t size,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) فراغ *_Znajj(uint32_t size,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) فراغ *_ZnwjSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) فراغ *_ZnajSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) فراغ *_ZnwjRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) فراغ *_ZnajRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) فراغ
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) فراغ
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    فراغ _ZdlPvj(فراغ *p, size_t n) RPALIAS(rpfree_size) فراغ _ZdaPvj(فراغ *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) فراغ _ZdlPvSt11align_val_t(فراغ *p, size_t a)
            RPALIAS(rpfree_aligned) فراغ _ZdaPvSt11align_val_t(فراغ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) فراغ _ZdlPvjSt11align_val_t(فراغ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) فراغ _ZdaPvjSt11align_val_t(
                        فراغ *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#endif

                            فراغ *malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) فراغ *calloc(size_t count, size_t size)
            RPALIAS(rpcalloc) فراغ *realloc(فراغ *ptr, size_t size)
                RPALIAS(rprealloc) فراغ *reallocf(فراغ *ptr, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rprealloc) فراغ *aligned_alloc(size_t alignment, size_t size)
            RPALIAS(rpaligned_alloc) فراغ *memalign(
                size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) صحيح posix_memalign(فراغ **memptr, size_t alignment,
                                               size_t size)
            RPALIAS(rpposix_memalign) فراغ حرر(فراغ *ptr)
                RPALIAS(rpfree) فراغ cfree(فراغ *ptr) RPALIAS(rpfree)
#𐡄𐡍 defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_size(ثابت فراغ *ptr) RPALIAS(rpmalloc_usable_size)
#وإلا
                    size_t
    malloc_usable_size(فراغ *ptr) RPALIAS(rpmalloc_usable_size)
#endif
        size_t malloc_size(فراغ *ptr) RPALIAS(rpmalloc_usable_size)

#endif

            ساكن مضمن size_t _rpmalloc_page_size(فراغ) {
  أرجع _memory_page_size;
}

خارجي فراغ *RPMALLOC_CDECL reallocarray(فراغ *ptr, size_t count, size_t size);

خارجي فراغ *RPMALLOC_CDECL reallocarray(فراغ *ptr, size_t count, size_t size) {
  size_t total;
#𐡄𐡍 ENABLE_VALIDATE_ARGS
#ifdef _MSC_VER
  صحيح err = SizeTMult(count, size, &total);
  𐡄𐡍 ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(count, size, &total);
  𐡄𐡍 (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    أرجع 0;
  }
#endif
#وإلا
  total = count * size;
#endif
  أرجع realloc(ptr, total);
}

خارجي مضمن فراغ *RPMALLOC_CDECL valloc(size_t size) {
  get_thread_heap();
  أرجع rpaligned_alloc(_rpmalloc_page_size(), size);
}

خارجي مضمن فراغ *RPMALLOC_CDECL pvalloc(size_t size) {
  get_thread_heap();
  ثابت size_t page_size = _rpmalloc_page_size();
  ثابت size_t aligned_size = ((size + page_size - 1) / page_size) * page_size;
#𐡄𐡍 ENABLE_VALIDATE_ARGS
  𐡄𐡍 (aligned_size < size) {
    errno = EINVAL;
    أرجع 0;
  }
#endif
  أرجع rpaligned_alloc(_rpmalloc_page_size(), aligned_size);
}

#endif // ENABLE_OVERRIDE

#𐡄𐡍 ENABLE_PRELOAD

#ifdef _WIN32

#𐡄𐡍 defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

خارجي __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

خارجي __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (فراغ)حجم(reserved);
  (فراغ)حجم(instance);
  𐡄𐡍 (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  وإلا 𐡄𐡍 (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  وإلا 𐡄𐡍 (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  وإلا 𐡄𐡍 (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  أرجع TRUE;
}

// end BUILD_DYNAMIC_LINK
#وإلا

خارجي فراغ _global_rpmalloc_init(فراغ) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
}

#𐡄𐡍 defined(__clang__) || defined(__GNUC__)

ساكن فراغ __attribute__((constructor)) initializer(فراغ) {
  _global_rpmalloc_init();
}

#وإلاإذا defined(_MSC_VER)

ساكن صحيح _global_rpmalloc_xib(فراغ) {
  _global_rpmalloc_init();
  أرجع 0;
}

#pragma section(".CRT$XIB", اقرأ)
__declspec(allocate(".CRT$XIB")) فراغ (*_rpmalloc_module_init)(فراغ) =
    _global_rpmalloc_xib;
#𐡄𐡍 defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#وإلا
#pragma comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#endif

#endif

// end !BUILD_DYNAMIC_LINK
#endif

#وإلا

#ضمن <pthread.h>
#ضمن <stdint.h>
#ضمن <stdlib.h>
#ضمن <unistd.h>

خارجي فراغ rpmalloc_set_main_thread(فراغ);

ساكن pthread_key_t destructor_key;

ساكن فراغ thread_destructor(فراغ *);

ساكن فراغ __attribute__((constructor)) initializer(فراغ) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
  pthread_key_create(&destructor_key, thread_destructor);
}

ساكن فراغ __attribute__((destructor)) finalizer(فراغ) { rpmalloc_finalize(); }

تعريفنوع بنية {
  فراغ *(*real_start)(فراغ *);
  فراغ *real_arg;
} thread_starter_arg;

ساكن فراغ *thread_starter(فراغ *argptr) {
  thread_starter_arg *arg = argptr;
  فراغ *(*real_start)(فراغ *) = arg->real_start;
  فراغ *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_key, (فراغ *)1);
  أرجع (*real_start)(real_arg);
}

ساكن فراغ thread_destructor(فراغ *value) {
  (فراغ)حجم(value);
  rpmalloc_thread_finalize(1);
}

#ifdef __APPLE__

ساكن صحيح pthread_create_proxy(pthread_t *thread, ثابت pthread_attr_t *attr,
                                فراغ *(*start_routine)(فراغ *), فراغ *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(حجم(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  أرجع pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#وإلا

#ضمن <dlfcn.h>

صحيح pthread_create(pthread_t *thread, ثابت pthread_attr_t *attr,
                   فراغ *(*start_routine)(فراغ *), فراغ *arg) {
#𐡄𐡍 defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  حرف fname[] = "pthread_create";
#وإلا
  حرف fname[] = "_pthread_create";
#endif
  فراغ *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(حجم(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  أرجع (*(صحيح (*)(pthread_t *, ثابت pthread_attr_t *, فراغ *(*)(فراغ *),
                    فراغ *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#endif

#endif

#endif

#𐡄𐡍 ENABLE_OVERRIDE

#𐡄𐡍 defined(__GLIBC__) && defined(__linux__)

فراغ *__libc_malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) فراغ *__libc_calloc(size_t count, size_t size)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) فراغ *__libc_realloc(فراغ *p, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2) RPALIAS(rprealloc) فراغ __libc_free(فراغ *p)
        RPALIAS(rpfree) فراغ __libc_cfree(فراغ *p)
            RPALIAS(rpfree) فراغ *__libc_memalign(size_t align, size_t size)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) صحيح __posix_memalign(فراغ **p, size_t align,
                                                 size_t size)
            RPALIAS(rpposix_memalign)

                خارجي فراغ *__libc_valloc(size_t size);
خارجي فراغ *__libc_pvalloc(size_t size);

فراغ *__libc_valloc(size_t size) { أرجع valloc(size); }

فراغ *__libc_pvalloc(size_t size) { أرجع pvalloc(size); }

#endif

#endif

#𐡄𐡍 (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility pop
#endif
