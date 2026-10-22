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
#جے_انت

#جے_بنا_تعریف ARCH_64_ثنائی_ہندسہ
#إذا defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#تعریف_کرو ARCH_64_ثنائی_ہندسہ 1
_Static_assert(حجم(size_t) == 8, "Data type size mismatch");
_Static_assert(حجم(فراغ *) == 8, "Data type size mismatch");
#وإلا
#تعریف_کرو ARCH_64_ثنائی_ہندسہ 0
_Static_assert(حجم(size_t) == 4, "Data type size mismatch");
_Static_assert(حجم(فراغ *) == 4, "Data type size mismatch");
#جے_انت
#جے_انت

#إذا (defined(__GNUC__) || defined(__clang__))
#ہدایت GCC visibility ڈالو(افتراضي)
#جے_انت

#تعریف_کرو USE_IMPLEMENT 1
#تعریف_کرو USE_INTERPOSE 0
#تعریف_کرو USE_ALIAS 0

#إذا defined(__APPLE__)
#تعریف_رد USE_INTERPOSE
#تعریف_کرو USE_INTERPOSE 1

تعريفنوع بنية interpose_t {
  فراغ *new_func;
  فراغ *orig_func;
} interpose_t;

#تعریف_کرو MAC_INTERPOSE_PAIR(newf, oldf) {(فراغ *)newf, (فراغ *)oldf}
#تعریف_کرو MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((ورتیا)) ساكن ثابت interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#جے_انت

#إذا !defined(_WIN32) && !defined(__APPLE__)
#تعریف_رد USE_IMPLEMENT
#تعریف_رد USE_ALIAS
#تعریف_کرو USE_IMPLEMENT 0
#تعریف_کرو USE_ALIAS 1
#جے_انت

#جے_تعریف_شدہ _MSC_VER
#ہدایت warning(disable : 4100)
#تعریف_رد حافظہ_دیو
#تعریف_رد حرر
#تعریف_رد صفر_حافظہ_دیو
#تعریف_کرو RPMALLOC_RESTRICT __declspec(محدود)
#وإلا
#تعریف_کرو RPMALLOC_RESTRICT
#جے_انت

#إذا ENABLE_OVERRIDE

تعريفنوع بنية rp_nothrow_t {
  صحيح __dummy;
} rp_nothrow_t;

#إذا USE_IMPLEMENT

خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL حافظہ_دیو(size_t ناپ_2) {
  أرجع rpmalloc(ناپ_2);
}
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL صفر_حافظہ_دیو(size_t گنتی,
                                                            size_t ناپ_2) {
  أرجع rpcalloc(گنتی, ناپ_2);
}
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL realloc(فراغ *ptr,
                                                             size_t ناپ_2) {
  أرجع rprealloc(ptr, ناپ_2);
}
خارجي مضمن فراغ *RPMALLOC_CDECL reallocf(فراغ *ptr, size_t ناپ_2) {
  أرجع rprealloc(ptr, ناپ_2);
}
خارجي مضمن فراغ *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t ناپ_2) {
  أرجع rpaligned_alloc(alignment, ناپ_2);
}
خارجي مضمن فراغ *RPMALLOC_CDECL memalign(size_t alignment, size_t ناپ_2) {
  أرجع rpmemalign(alignment, ناپ_2);
}
خارجي مضمن صحيح RPMALLOC_CDECL posix_memalign(فراغ **memptr, size_t alignment,
                                                size_t ناپ_2) {
  أرجع rpposix_memalign(memptr, alignment, ناپ_2);
}
خارجي مضمن فراغ RPMALLOC_CDECL حرر(فراغ *ptr) { rpfree(ptr); }
خارجي مضمن فراغ RPMALLOC_CDECL cfree(فراغ *ptr) { rpfree(ptr); }
خارجي مضمن size_t RPMALLOC_CDECL malloc_usable_ناپ(فراغ *ptr) {
  أرجع rpmalloc_usable_ناپ(ptr);
}
خارجي مضمن size_t RPMALLOC_CDECL malloc_ناپ(فراغ *ptr) {
  أرجع rpmalloc_usable_ناپ(ptr);
}

#جے_تعریف_شدہ _WIN32
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL malloc_ادھار(size_t ناپ_2) {
  أرجع rpmalloc(ناپ_2);
}
خارجي مضمن فراغ RPMALLOC_CDECL free_ادھار(فراغ *ptr) { rpfree(ptr); }
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL calloc_ادھار(size_t گنتی,
                                                                  size_t ناپ_2) {
  أرجع rpcalloc(گنتی, ناپ_2);
}
خارجي مضمن size_t RPMALLOC_CDECL _msize(فراغ *ptr) {
  أرجع rpmalloc_usable_ناپ(ptr);
}
خارجي مضمن size_t RPMALLOC_CDECL msize_ادھار(فراغ *ptr) {
  أرجع rpmalloc_usable_ناپ(ptr);
}
خارجي مضمن RPMALLOC_RESTRICT فراغ *RPMALLOC_CDECL
realloc_ادھار(فراغ *ptr, size_t ناپ_2) {
  أرجع rprealloc(ptr, ناپ_2);
}
#جے_انت

#جے_تعریف_شدہ _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#وإلا
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#تعریف_کرو RPDEFVIS __attribute__((visibility("default")))
خارجي فراغ _ZdlPv(فراغ *p);
فراغ RPDEFVIS _ZdlPv(فراغ *p) { rpfree(p); }
خارجي فراغ _ZdaPv(فراغ *p);
فراغ RPDEFVIS _ZdaPv(فراغ *p) { rpfree(p); }
#إذا ARCH_64_ثنائی_ہندسہ
// 64-bit operators new and new[], normal and aligned
خارجي فراغ *_Znwm(uint64_t ناپ_2);
فراغ *RPDEFVIS _Znwm(uint64_t ناپ_2) { أرجع rpmalloc(ناپ_2); }
خارجي فراغ *_Znam(uint64_t ناپ_2);
فراغ *RPDEFVIS _Znam(uint64_t ناپ_2) { أرجع rpmalloc(ناپ_2); }
خارجي فراغ *_Znwmm(uint64_t ناپ_2, uint64_t align);
فراغ *RPDEFVIS _Znwmm(uint64_t ناپ_2, uint64_t align) {
  أرجع rpaligned_alloc(align, ناپ_2);
}
خارجي فراغ *_Znamm(uint64_t ناپ_2, uint64_t align);
فراغ *RPDEFVIS _Znamm(uint64_t ناپ_2, uint64_t align) {
  أرجع rpaligned_alloc(align, ناپ_2);
}
خارجي فراغ *_ZnwmSt11align_val_t(uint64_t ناپ_2, uint64_t align);
فراغ *RPDEFVIS _ZnwmSt11align_val_t(uint64_t ناپ_2, uint64_t align) {
  أرجع rpaligned_alloc(align, ناپ_2);
}
خارجي فراغ *_ZnamSt11align_val_t(uint64_t ناپ_2, uint64_t align);
فراغ *RPDEFVIS _ZnamSt11align_val_t(uint64_t ناپ_2, uint64_t align) {
  أرجع rpaligned_alloc(align, ناپ_2);
}
خارجي فراغ *_ZnwmRKSt9nothrow_t(uint64_t ناپ_2, rp_nothrow_t t);
فراغ *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t ناپ_2, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(ناپ_2);
}
خارجي فراغ *_ZnamRKSt9nothrow_t(uint64_t ناپ_2, rp_nothrow_t t);
فراغ *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t ناپ_2, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(ناپ_2);
}
خارجي فراغ *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t ناپ_2, uint64_t align,
                                                rp_nothrow_t t);
فراغ *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t ناپ_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, ناپ_2);
}
خارجي فراغ *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t ناپ_2, uint64_t align,
                                                rp_nothrow_t t);
فراغ *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t ناپ_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, ناپ_2);
}
// 64-bit operators sized delete and delete[], normal and aligned
خارجي فراغ _ZdlPvm(فراغ *p, uint64_t ناپ_2);
فراغ RPDEFVIS _ZdlPvm(فراغ *p, uint64_t ناپ_2) {
  rpfree(p);
  (فراغ)حجم(ناپ_2);
}
خارجي فراغ _ZdaPvm(فراغ *p, uint64_t ناپ_2);
فراغ RPDEFVIS _ZdaPvm(فراغ *p, uint64_t ناپ_2) {
  rpfree(p);
  (فراغ)حجم(ناپ_2);
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
خارجي فراغ _ZdlPvmSt11align_val_t(فراغ *p, uint64_t ناپ_2, uint64_t align);
فراغ RPDEFVIS _ZdlPvmSt11align_val_t(فراغ *p, uint64_t ناپ_2, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(ناپ_2);
  (فراغ)حجم(align);
}
خارجي فراغ _ZdaPvmSt11align_val_t(فراغ *p, uint64_t ناپ_2, uint64_t align);
فراغ RPDEFVIS _ZdaPvmSt11align_val_t(فراغ *p, uint64_t ناپ_2, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(ناپ_2);
  (فراغ)حجم(align);
}
#وإلا
// 32-bit operators new and new[], normal and aligned
خارجي فراغ *_Znwj(uint32_t ناپ_2);
فراغ *RPDEFVIS _Znwj(uint32_t ناپ_2) { أرجع rpmalloc(ناپ_2); }
خارجي فراغ *_Znaj(uint32_t ناپ_2);
فراغ *RPDEFVIS _Znaj(uint32_t ناپ_2) { أرجع rpmalloc(ناپ_2); }
خارجي فراغ *_Znwjj(uint32_t ناپ_2, uint32_t align);
فراغ *RPDEFVIS _Znwjj(uint32_t ناپ_2, uint32_t align) {
  أرجع rpaligned_alloc(align, ناپ_2);
}
خارجي فراغ *_Znajj(uint32_t ناپ_2, uint32_t align);
فراغ *RPDEFVIS _Znajj(uint32_t ناپ_2, uint32_t align) {
  أرجع rpaligned_alloc(align, ناپ_2);
}
خارجي فراغ *_ZnwjSt11align_val_t(size_t ناپ_2, size_t align);
فراغ *RPDEFVIS _ZnwjSt11align_val_t(size_t ناپ_2, size_t align) {
  أرجع rpaligned_alloc(align, ناپ_2);
}
خارجي فراغ *_ZnajSt11align_val_t(size_t ناپ_2, size_t align);
فراغ *RPDEFVIS _ZnajSt11align_val_t(size_t ناپ_2, size_t align) {
  أرجع rpaligned_alloc(align, ناپ_2);
}
خارجي فراغ *_ZnwjRKSt9nothrow_t(size_t ناپ_2, rp_nothrow_t t);
فراغ *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t ناپ_2, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(ناپ_2);
}
خارجي فراغ *_ZnajRKSt9nothrow_t(size_t ناپ_2, rp_nothrow_t t);
فراغ *RPDEFVIS _ZnajRKSt9nothrow_t(size_t ناپ_2, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(ناپ_2);
}
خارجي فراغ *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t ناپ_2, size_t align,
                                                rp_nothrow_t t);
فراغ *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t ناپ_2, size_t align,
                                                  rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, ناپ_2);
}
خارجي فراغ *_ZnajSt11align_val_tRKSt9nothrow_t(size_t ناپ_2, size_t align,
                                                rp_nothrow_t t);
فراغ *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t ناپ_2, size_t align,
                                                  rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, ناپ_2);
}
// 32-bit operators sized delete and delete[], normal and aligned
خارجي فراغ _ZdlPvj(فراغ *p, uint64_t ناپ_2);
فراغ RPDEFVIS _ZdlPvj(فراغ *p, uint64_t ناپ_2) {
  rpfree(p);
  (فراغ)حجم(ناپ_2);
}
خارجي فراغ _ZdaPvj(فراغ *p, uint64_t ناپ_2);
فراغ RPDEFVIS _ZdaPvj(فراغ *p, uint64_t ناپ_2) {
  rpfree(p);
  (فراغ)حجم(ناپ_2);
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
خارجي فراغ _ZdlPvjSt11align_val_t(فراغ *p, uint32_t ناپ_2, uint32_t align);
فراغ RPDEFVIS _ZdlPvjSt11align_val_t(فراغ *p, uint64_t ناپ_2, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(ناپ_2);
  (فراغ)حجم(a);
}
خارجي فراغ _ZdaPvjSt11align_val_t(فراغ *p, uint32_t ناپ_2, uint32_t align);
فراغ RPDEFVIS _ZdaPvjSt11align_val_t(فراغ *p, uint64_t ناپ_2, uint64_t align) {
  rpfree(p);
  (فراغ)حجم(ناپ_2);
  (فراغ)حجم(a);
}
#جے_انت
#جے_انت
#جے_انت

#إذا USE_INTERPOSE || USE_ALIAS

ساكن فراغ *rpmalloc_nothrow(size_t ناپ_2, rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpmalloc(ناپ_2);
}
ساكن فراغ *rpaligned_alloc_الٹ(size_t ناپ_2, size_t align) {
  أرجع rpaligned_alloc(align, ناپ_2);
}
ساكن فراغ *rpaligned_alloc_الٹ_nothrow(size_t ناپ_2, size_t align,
                                             rp_nothrow_t t) {
  (فراغ)حجم(t);
  أرجع rpaligned_alloc(align, ناپ_2);
}
ساكن فراغ rpfree_ناپ(فراغ *p, size_t ناپ_2) {
  (فراغ)حجم(ناپ_2);
  rpfree(p);
}
ساكن فراغ rpfree_aligned(فراغ *p, size_t align) {
  (فراغ)حجم(align);
  rpfree(p);
}
ساكن فراغ rpfree_ناپ_aligned(فراغ *p, size_t ناپ_2, size_t align) {
  (فراغ)حجم(ناپ_2);
  (فراغ)حجم(align);
  rpfree(p);
}

#جے_انت

#إذا USE_INTERPOSE

__attribute__((ورتیا)) ساكن ثابت interpose_t macinterpose_malloc[]
    __attribute__((section("__DATA, __interpose"))) = {
        // new and new[]
        MAC_INTERPOSE_PAIR(rpmalloc, _Znwm),
        MAC_INTERPOSE_PAIR(rpmalloc, _Znam),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_الٹ, _Znwmm),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_الٹ, _Znamm),
        MAC_INTERPOSE_PAIR(rpmalloc_nothrow, _ZnwmRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpmalloc_nothrow, _ZnamRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_الٹ, _ZnwmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_الٹ, _ZnamSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_الٹ_nothrow,
                           _ZnwmSt11align_val_tRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_الٹ_nothrow,
                           _ZnamSt11align_val_tRKSt9nothrow_t),
        // delete and delete[]
        MAC_INTERPOSE_PAIR(rpfree, _ZdlPv), MAC_INTERPOSE_PAIR(rpfree, _ZdaPv),
        MAC_INTERPOSE_PAIR(rpfree_ناپ, _ZdlPvm),
        MAC_INTERPOSE_PAIR(rpfree_ناپ, _ZdaPvm),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdlPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdaPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_ناپ_aligned, _ZdlPvmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_ناپ_aligned, _ZdaPvmSt11align_val_t),
        // libc entry points
        MAC_INTERPOSE_PAIR(rpmalloc, حافظہ_دیو),
        MAC_INTERPOSE_PAIR(rpmalloc, صفر_حافظہ_دیو),
        MAC_INTERPOSE_PAIR(rprealloc, realloc),
        MAC_INTERPOSE_PAIR(rprealloc, reallocf),
#إذا defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#جے_انت
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, حرر), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_ناپ, malloc_usable_ناپ),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_ناپ, malloc_ناپ)};

#جے_انت

#إذا USE_ALIAS

#تعریف_کرو RPALIAS(fn) __attribute__((alias(#fn), ورتیا, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
فراغ _ZdlPv(فراغ *p) RPALIAS(rpfree) فراغ _ZdaPv(فراغ *p) RPALIAS(rpfree)

#إذا ARCH_64_ثنائی_ہندسہ
    // 64-bit operators new and new[], normal and aligned
    فراغ *_Znwm(uint64_t ناپ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ناپ(1)
        RPALIAS(rpmalloc) فراغ *_Znam(uint64_t ناپ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ناپ(1) RPALIAS(rpmalloc) فراغ *_Znwmm(uint64_t ناپ_2,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_الٹ) فراغ *_Znamm(uint64_t ناپ_2,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_الٹ) فراغ *_ZnwmSt11align_val_t(
                size_t ناپ_2, size_t align)
                RPALIAS(rpaligned_alloc_الٹ) فراغ *_ZnamSt11align_val_t(
                    size_t ناپ_2, size_t align)
                    RPALIAS(rpaligned_alloc_الٹ) فراغ *_ZnwmRKSt9nothrow_t(
                        size_t ناپ_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) فراغ *_ZnamRKSt9nothrow_t(
                            size_t ناپ_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) فراغ
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t ناپ_2,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_الٹ_nothrow) فراغ
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t ناپ_2, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_الٹ_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    فراغ _ZdlPvm(فراغ *p, size_t n) RPALIAS(rpfree_ناپ) فراغ _ZdaPvm(فراغ *p,
                                                                      size_t n)
        RPALIAS(rpfree_ناپ) فراغ _ZdlPvSt11align_val_t(فراغ *p, size_t a)
            RPALIAS(rpfree_aligned) فراغ _ZdaPvSt11align_val_t(فراغ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) فراغ _ZdlPvmSt11align_val_t(فراغ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_ناپ_aligned) فراغ _ZdaPvmSt11align_val_t(
                        فراغ *p, size_t n, size_t a)
                        RPALIAS(rpfree_ناپ_aligned)
#وإلا
    // 32-bit operators new and new[], normal and aligned
    فراغ *_Znwj(uint32_t ناپ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ناپ(1)
        RPALIAS(rpmalloc) فراغ *_Znaj(uint32_t ناپ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ناپ(1) RPALIAS(rpmalloc) فراغ *_Znwjj(uint32_t ناپ_2,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_الٹ) فراغ *_Znajj(uint32_t ناپ_2,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_الٹ) فراغ *_ZnwjSt11align_val_t(
                size_t ناپ_2, size_t align)
                RPALIAS(rpaligned_alloc_الٹ) فراغ *_ZnajSt11align_val_t(
                    size_t ناپ_2, size_t align)
                    RPALIAS(rpaligned_alloc_الٹ) فراغ *_ZnwjRKSt9nothrow_t(
                        size_t ناپ_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) فراغ *_ZnajRKSt9nothrow_t(
                            size_t ناپ_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) فراغ
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t ناپ_2,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_الٹ_nothrow) فراغ
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t ناپ_2, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_الٹ_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    فراغ _ZdlPvj(فراغ *p, size_t n) RPALIAS(rpfree_ناپ) فراغ _ZdaPvj(فراغ *p,
                                                                      size_t n)
        RPALIAS(rpfree_ناپ) فراغ _ZdlPvSt11align_val_t(فراغ *p, size_t a)
            RPALIAS(rpfree_aligned) فراغ _ZdaPvSt11align_val_t(فراغ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) فراغ _ZdlPvjSt11align_val_t(فراغ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_ناپ_aligned) فراغ _ZdaPvjSt11align_val_t(
                        فراغ *p, size_t n, size_t a)
                        RPALIAS(rpfree_ناپ_aligned)
#جے_انت

                            فراغ *حافظہ_دیو(size_t ناپ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ناپ(1)
        RPALIAS(rpmalloc) فراغ *صفر_حافظہ_دیو(size_t گنتی, size_t ناپ_2)
            RPALIAS(rpcalloc) فراغ *realloc(فراغ *ptr, size_t ناپ_2)
                RPALIAS(rprealloc) فراغ *reallocf(فراغ *ptr, size_t ناپ_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ناپ(2)
        RPALIAS(rprealloc) فراغ *aligned_alloc(size_t alignment, size_t ناپ_2)
            RPALIAS(rpaligned_alloc) فراغ *memalign(
                size_t alignment, size_t ناپ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ناپ(2)
        RPALIAS(rpmemalign) صحيح posix_memalign(فراغ **memptr, size_t alignment,
                                               size_t ناپ_2)
            RPALIAS(rpposix_memalign) فراغ حرر(فراغ *ptr)
                RPALIAS(rpfree) فراغ cfree(فراغ *ptr) RPALIAS(rpfree)
#إذا defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_ناپ(ثابت فراغ *ptr) RPALIAS(rpmalloc_usable_ناپ)
#وإلا
                    size_t
    malloc_usable_ناپ(فراغ *ptr) RPALIAS(rpmalloc_usable_ناپ)
#جے_انت
        size_t malloc_ناپ(فراغ *ptr) RPALIAS(rpmalloc_usable_ناپ)

#جے_انت

            ساكن مضمن size_t rpmalloc_ورقہ_ناپ(فراغ) {
  أرجع حافظہ_ورقہ_ناپ;
}

خارجي فراغ *RPMALLOC_CDECL reallocarray(فراغ *ptr, size_t گنتی, size_t ناپ_2);

خارجي فراغ *RPMALLOC_CDECL reallocarray(فراغ *ptr, size_t گنتی, size_t ناپ_2) {
  size_t کل_2;
#إذا ENABLE_VALIDATE_دلیلاں
#جے_تعریف_شدہ _MSC_VER
  صحيح err = ناپ_TMult(گنتی, ناپ_2, &کل_2);
  إذا ((err != S_OK) || (کل_2 >= MAX_ALLOC_ناپ)) {
    errno = EINVAL;
    أرجع 0;
  }
#وإلا
  صحيح err = __builtin_umull_overflow(گنتی, ناپ_2, &کل_2);
  إذا (err || (کل_2 >= MAX_ALLOC_ناپ)) {
    errno = EINVAL;
    أرجع 0;
  }
#جے_انت
#وإلا
  کل_2 = گنتی * ناپ_2;
#جے_انت
  أرجع realloc(ptr, کل_2);
}

خارجي مضمن فراغ *RPMALLOC_CDECL valloc(size_t ناپ_2) {
  get_thread_ڈھیر();
  أرجع rpaligned_alloc(rpmalloc_ورقہ_ناپ(), ناپ_2);
}

خارجي مضمن فراغ *RPMALLOC_CDECL pvalloc(size_t ناپ_2) {
  get_thread_ڈھیر();
  ثابت size_t ورقہ_ناپ_2 = rpmalloc_ورقہ_ناپ();
  ثابت size_t aligned_ناپ = ((ناپ_2 + ورقہ_ناپ_2 - 1) / ورقہ_ناپ_2) * ورقہ_ناپ_2;
#إذا ENABLE_VALIDATE_دلیلاں
  إذا (aligned_ناپ < ناپ_2) {
    errno = EINVAL;
    أرجع 0;
  }
#جے_انت
  أرجع rpaligned_alloc(rpmalloc_ورقہ_ناپ(), aligned_ناپ);
}

#جے_انت // ENABLE_OVERRIDE

#إذا ENABLE_PRELOAD

#جے_تعریف_شدہ _WIN32

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
  rpmalloc_سیٹ_main_thread();
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

#ہدایت section(".CRT$XIB", اقرأ)
__declspec(allocate(".CRT$XIB")) فراغ (*_rpmalloc_module_init)(فراغ) =
    _global_rpmalloc_xib;
#إذا defined(_M_IX86) || defined(__i386__)
#ہدایت comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#وإلا
#ہدایت comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#جے_انت

#جے_انت

// end !BUILD_DYNAMIC_LINK
#جے_انت

#وإلا

#ضمن <pthread.h>
#ضمن <stdint.h>
#ضمن <stdlib.h>
#ضمن <unistd.h>

خارجي فراغ rpmalloc_سیٹ_main_thread(فراغ);

ساكن pthread_key_t destructor_کلید;

ساكن فراغ thread_destructor(فراغ *);

ساكن فراغ __attribute__((constructor)) initializer(فراغ) {
  rpmalloc_سیٹ_main_thread();
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

ساكن فراغ thread_destructor(فراغ *قدر_2) {
  (فراغ)حجم(قدر_2);
  rpmalloc_thread_finalize(1);
}

#جے_تعریف_شدہ __APPLE__

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
#جے_انت
  فراغ *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(حجم(thread_starter_arg));
  starter_arg->real_شروع = شروع_routine;
  starter_arg->real_arg = arg;
  أرجع (*(صحيح (*)(pthread_t *, ثابت pthread_attr_t *, فراغ *(*)(فراغ *),
                    فراغ *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#جے_انت

#جے_انت

#جے_انت

#إذا ENABLE_OVERRIDE

#إذا defined(__GLIBC__) && defined(__linux__)

فراغ *__libc_malloc(size_t ناپ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ناپ(1)
        RPALIAS(rpmalloc) فراغ *__libc_calloc(size_t گنتی, size_t ناپ_2)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) فراغ *__libc_realloc(فراغ *p, size_t ناپ_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ناپ(2) RPALIAS(rprealloc) فراغ __libc_free(فراغ *p)
        RPALIAS(rpfree) فراغ __libc_cfree(فراغ *p)
            RPALIAS(rpfree) فراغ *__libc_memalign(size_t align, size_t ناپ_2)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ناپ(2)
        RPALIAS(rpmemalign) صحيح __posix_memalign(فراغ **p, size_t align,
                                                 size_t ناپ_2)
            RPALIAS(rpposix_memalign)

                خارجي فراغ *__libc_valloc(size_t ناپ_2);
خارجي فراغ *__libc_pvalloc(size_t ناپ_2);

فراغ *__libc_valloc(size_t ناپ_2) { أرجع valloc(ناپ_2); }

فراغ *__libc_pvalloc(size_t ناپ_2) { أرجع pvalloc(ناپ_2); }

#جے_انت

#جے_انت

#إذا (defined(__GNUC__) || defined(__clang__))
#ہدایت GCC visibility نکالو
#جے_انت
