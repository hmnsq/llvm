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
#اگر_ختم

#اگر_غیر_تعریف_شدہ ARCH_64_ثنائی_ہندسہ
#اگر defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#تعریف_کرو ARCH_64_ثنائی_ہندسہ 1
_Static_assert(حجم(size_t) == 8, "Data type size mismatch");
_Static_assert(حجم(خالی *) == 8, "Data type size mismatch");
#ورنہ
#تعریف_کرو ARCH_64_ثنائی_ہندسہ 0
_Static_assert(حجم(size_t) == 4, "Data type size mismatch");
_Static_assert(حجم(خالی *) == 4, "Data type size mismatch");
#اگر_ختم
#اگر_ختم

#اگر (defined(__GNUC__) || defined(__clang__))
#ہدایت GCC visibility ڈالو(طےشدہ)
#اگر_ختم

#تعریف_کرو USE_IMPLEMENT 1
#تعریف_کرو USE_INTERPOSE 0
#تعریف_کرو USE_ALIAS 0

#اگر defined(__APPLE__)
#تعریف_منسوخ USE_INTERPOSE
#تعریف_کرو USE_INTERPOSE 1

قسمتعریف ساخت interpose_t {
  خالی *new_func;
  خالی *orig_func;
} interpose_t;

#تعریف_کرو MAC_INTERPOSE_PAIR(newf, oldf) {(خالی *)newf, (خالی *)oldf}
#تعریف_کرو MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((مستعمل)) ساکن ثابت interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#اگر_ختم

#اگر !defined(_WIN32) && !defined(__APPLE__)
#تعریف_منسوخ USE_IMPLEMENT
#تعریف_منسوخ USE_ALIAS
#تعریف_کرو USE_IMPLEMENT 0
#تعریف_کرو USE_ALIAS 1
#اگر_ختم

#اگر_تعریف_شدہ _MSC_VER
#ہدایت warning(disable : 4100)
#تعریف_منسوخ حافظہ_دو
#تعریف_منسوخ آزاد
#تعریف_منسوخ صفر_حافظہ_دو
#تعریف_کرو RPMALLOC_RESTRICT __declspec(محدود)
#ورنہ
#تعریف_کرو RPMALLOC_RESTRICT
#اگر_ختم

#اگر ENABLE_OVERRIDE

قسمتعریف ساخت rp_nothrow_t {
  صحیح __dummy;
} rp_nothrow_t;

#اگر USE_IMPLEMENT

خارجی اندرونی RPMALLOC_RESTRICT خالی *RPMALLOC_CDECL حافظہ_دو(size_t سائز_2) {
  واپس rpmalloc(سائز_2);
}
خارجی اندرونی RPMALLOC_RESTRICT خالی *RPMALLOC_CDECL صفر_حافظہ_دو(size_t گنتی,
                                                            size_t سائز_2) {
  واپس rpcalloc(گنتی, سائز_2);
}
خارجی اندرونی RPMALLOC_RESTRICT خالی *RPMALLOC_CDECL realloc(خالی *ptr,
                                                             size_t سائز_2) {
  واپس rprealloc(ptr, سائز_2);
}
خارجی اندرونی خالی *RPMALLOC_CDECL reallocf(خالی *ptr, size_t سائز_2) {
  واپس rprealloc(ptr, سائز_2);
}
خارجی اندرونی خالی *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t سائز_2) {
  واپس rpaligned_alloc(alignment, سائز_2);
}
خارجی اندرونی خالی *RPMALLOC_CDECL memalign(size_t alignment, size_t سائز_2) {
  واپس rpmemalign(alignment, سائز_2);
}
خارجی اندرونی صحیح RPMALLOC_CDECL posix_memalign(خالی **memptr, size_t alignment,
                                                size_t سائز_2) {
  واپس rpposix_memalign(memptr, alignment, سائز_2);
}
خارجی اندرونی خالی RPMALLOC_CDECL آزاد(خالی *ptr) { rpfree(ptr); }
خارجی اندرونی خالی RPMALLOC_CDECL cfree(خالی *ptr) { rpfree(ptr); }
خارجی اندرونی size_t RPMALLOC_CDECL malloc_usable_سائز(خالی *ptr) {
  واپس rpmalloc_usable_سائز(ptr);
}
خارجی اندرونی size_t RPMALLOC_CDECL malloc_سائز(خالی *ptr) {
  واپس rpmalloc_usable_سائز(ptr);
}

#اگر_تعریف_شدہ _WIN32
خارجی اندرونی RPMALLOC_RESTRICT خالی *RPMALLOC_CDECL malloc_بنیاد(size_t سائز_2) {
  واپس rpmalloc(سائز_2);
}
خارجی اندرونی خالی RPMALLOC_CDECL free_بنیاد(خالی *ptr) { rpfree(ptr); }
خارجی اندرونی RPMALLOC_RESTRICT خالی *RPMALLOC_CDECL calloc_بنیاد(size_t گنتی,
                                                                  size_t سائز_2) {
  واپس rpcalloc(گنتی, سائز_2);
}
خارجی اندرونی size_t RPMALLOC_CDECL _msize(خالی *ptr) {
  واپس rpmalloc_usable_سائز(ptr);
}
خارجی اندرونی size_t RPMALLOC_CDECL msize_بنیاد(خالی *ptr) {
  واپس rpmalloc_usable_سائز(ptr);
}
خارجی اندرونی RPMALLOC_RESTRICT خالی *RPMALLOC_CDECL
realloc_بنیاد(خالی *ptr, size_t سائز_2) {
  واپس rprealloc(ptr, سائز_2);
}
#اگر_ختم

#اگر_تعریف_شدہ _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#ورنہ
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#تعریف_کرو RPDEFVIS __attribute__((visibility("default")))
خارجی خالی _ZdlPv(خالی *p);
خالی RPDEFVIS _ZdlPv(خالی *p) { rpfree(p); }
خارجی خالی _ZdaPv(خالی *p);
خالی RPDEFVIS _ZdaPv(خالی *p) { rpfree(p); }
#اگر ARCH_64_ثنائی_ہندسہ
// 64-bit operators new and new[], normal and aligned
خارجی خالی *_Znwm(uint64_t سائز_2);
خالی *RPDEFVIS _Znwm(uint64_t سائز_2) { واپس rpmalloc(سائز_2); }
خارجی خالی *_Znam(uint64_t سائز_2);
خالی *RPDEFVIS _Znam(uint64_t سائز_2) { واپس rpmalloc(سائز_2); }
خارجی خالی *_Znwmm(uint64_t سائز_2, uint64_t align);
خالی *RPDEFVIS _Znwmm(uint64_t سائز_2, uint64_t align) {
  واپس rpaligned_alloc(align, سائز_2);
}
خارجی خالی *_Znamm(uint64_t سائز_2, uint64_t align);
خالی *RPDEFVIS _Znamm(uint64_t سائز_2, uint64_t align) {
  واپس rpaligned_alloc(align, سائز_2);
}
خارجی خالی *_ZnwmSt11align_val_t(uint64_t سائز_2, uint64_t align);
خالی *RPDEFVIS _ZnwmSt11align_val_t(uint64_t سائز_2, uint64_t align) {
  واپس rpaligned_alloc(align, سائز_2);
}
خارجی خالی *_ZnamSt11align_val_t(uint64_t سائز_2, uint64_t align);
خالی *RPDEFVIS _ZnamSt11align_val_t(uint64_t سائز_2, uint64_t align) {
  واپس rpaligned_alloc(align, سائز_2);
}
خارجی خالی *_ZnwmRKSt9nothrow_t(uint64_t سائز_2, rp_nothrow_t t);
خالی *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t سائز_2, rp_nothrow_t t) {
  (خالی)حجم(t);
  واپس rpmalloc(سائز_2);
}
خارجی خالی *_ZnamRKSt9nothrow_t(uint64_t سائز_2, rp_nothrow_t t);
خالی *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t سائز_2, rp_nothrow_t t) {
  (خالی)حجم(t);
  واپس rpmalloc(سائز_2);
}
خارجی خالی *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t سائز_2, uint64_t align,
                                                rp_nothrow_t t);
خالی *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t سائز_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (خالی)حجم(t);
  واپس rpaligned_alloc(align, سائز_2);
}
خارجی خالی *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t سائز_2, uint64_t align,
                                                rp_nothrow_t t);
خالی *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t سائز_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (خالی)حجم(t);
  واپس rpaligned_alloc(align, سائز_2);
}
// 64-bit operators sized delete and delete[], normal and aligned
خارجی خالی _ZdlPvm(خالی *p, uint64_t سائز_2);
خالی RPDEFVIS _ZdlPvm(خالی *p, uint64_t سائز_2) {
  rpfree(p);
  (خالی)حجم(سائز_2);
}
خارجی خالی _ZdaPvm(خالی *p, uint64_t سائز_2);
خالی RPDEFVIS _ZdaPvm(خالی *p, uint64_t سائز_2) {
  rpfree(p);
  (خالی)حجم(سائز_2);
}
خارجی خالی _ZdlPvSt11align_val_t(خالی *p, uint64_t align);
خالی RPDEFVIS _ZdlPvSt11align_val_t(خالی *p, uint64_t align) {
  rpfree(p);
  (خالی)حجم(align);
}
خارجی خالی _ZdaPvSt11align_val_t(خالی *p, uint64_t align);
خالی RPDEFVIS _ZdaPvSt11align_val_t(خالی *p, uint64_t align) {
  rpfree(p);
  (خالی)حجم(align);
}
خارجی خالی _ZdlPvmSt11align_val_t(خالی *p, uint64_t سائز_2, uint64_t align);
خالی RPDEFVIS _ZdlPvmSt11align_val_t(خالی *p, uint64_t سائز_2, uint64_t align) {
  rpfree(p);
  (خالی)حجم(سائز_2);
  (خالی)حجم(align);
}
خارجی خالی _ZdaPvmSt11align_val_t(خالی *p, uint64_t سائز_2, uint64_t align);
خالی RPDEFVIS _ZdaPvmSt11align_val_t(خالی *p, uint64_t سائز_2, uint64_t align) {
  rpfree(p);
  (خالی)حجم(سائز_2);
  (خالی)حجم(align);
}
#ورنہ
// 32-bit operators new and new[], normal and aligned
خارجی خالی *_Znwj(uint32_t سائز_2);
خالی *RPDEFVIS _Znwj(uint32_t سائز_2) { واپس rpmalloc(سائز_2); }
خارجی خالی *_Znaj(uint32_t سائز_2);
خالی *RPDEFVIS _Znaj(uint32_t سائز_2) { واپس rpmalloc(سائز_2); }
خارجی خالی *_Znwjj(uint32_t سائز_2, uint32_t align);
خالی *RPDEFVIS _Znwjj(uint32_t سائز_2, uint32_t align) {
  واپس rpaligned_alloc(align, سائز_2);
}
خارجی خالی *_Znajj(uint32_t سائز_2, uint32_t align);
خالی *RPDEFVIS _Znajj(uint32_t سائز_2, uint32_t align) {
  واپس rpaligned_alloc(align, سائز_2);
}
خارجی خالی *_ZnwjSt11align_val_t(size_t سائز_2, size_t align);
خالی *RPDEFVIS _ZnwjSt11align_val_t(size_t سائز_2, size_t align) {
  واپس rpaligned_alloc(align, سائز_2);
}
خارجی خالی *_ZnajSt11align_val_t(size_t سائز_2, size_t align);
خالی *RPDEFVIS _ZnajSt11align_val_t(size_t سائز_2, size_t align) {
  واپس rpaligned_alloc(align, سائز_2);
}
خارجی خالی *_ZnwjRKSt9nothrow_t(size_t سائز_2, rp_nothrow_t t);
خالی *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t سائز_2, rp_nothrow_t t) {
  (خالی)حجم(t);
  واپس rpmalloc(سائز_2);
}
خارجی خالی *_ZnajRKSt9nothrow_t(size_t سائز_2, rp_nothrow_t t);
خالی *RPDEFVIS _ZnajRKSt9nothrow_t(size_t سائز_2, rp_nothrow_t t) {
  (خالی)حجم(t);
  واپس rpmalloc(سائز_2);
}
خارجی خالی *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t سائز_2, size_t align,
                                                rp_nothrow_t t);
خالی *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t سائز_2, size_t align,
                                                  rp_nothrow_t t) {
  (خالی)حجم(t);
  واپس rpaligned_alloc(align, سائز_2);
}
خارجی خالی *_ZnajSt11align_val_tRKSt9nothrow_t(size_t سائز_2, size_t align,
                                                rp_nothrow_t t);
خالی *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t سائز_2, size_t align,
                                                  rp_nothrow_t t) {
  (خالی)حجم(t);
  واپس rpaligned_alloc(align, سائز_2);
}
// 32-bit operators sized delete and delete[], normal and aligned
خارجی خالی _ZdlPvj(خالی *p, uint64_t سائز_2);
خالی RPDEFVIS _ZdlPvj(خالی *p, uint64_t سائز_2) {
  rpfree(p);
  (خالی)حجم(سائز_2);
}
خارجی خالی _ZdaPvj(خالی *p, uint64_t سائز_2);
خالی RPDEFVIS _ZdaPvj(خالی *p, uint64_t سائز_2) {
  rpfree(p);
  (خالی)حجم(سائز_2);
}
خارجی خالی _ZdlPvSt11align_val_t(خالی *p, uint32_t align);
خالی RPDEFVIS _ZdlPvSt11align_val_t(خالی *p, uint64_t a) {
  rpfree(p);
  (خالی)حجم(align);
}
خارجی خالی _ZdaPvSt11align_val_t(خالی *p, uint32_t align);
خالی RPDEFVIS _ZdaPvSt11align_val_t(خالی *p, uint64_t a) {
  rpfree(p);
  (خالی)حجم(align);
}
خارجی خالی _ZdlPvjSt11align_val_t(خالی *p, uint32_t سائز_2, uint32_t align);
خالی RPDEFVIS _ZdlPvjSt11align_val_t(خالی *p, uint64_t سائز_2, uint64_t align) {
  rpfree(p);
  (خالی)حجم(سائز_2);
  (خالی)حجم(a);
}
خارجی خالی _ZdaPvjSt11align_val_t(خالی *p, uint32_t سائز_2, uint32_t align);
خالی RPDEFVIS _ZdaPvjSt11align_val_t(خالی *p, uint64_t سائز_2, uint64_t align) {
  rpfree(p);
  (خالی)حجم(سائز_2);
  (خالی)حجم(a);
}
#اگر_ختم
#اگر_ختم
#اگر_ختم

#اگر USE_INTERPOSE || USE_ALIAS

ساکن خالی *rpmalloc_nothrow(size_t سائز_2, rp_nothrow_t t) {
  (خالی)حجم(t);
  واپس rpmalloc(سائز_2);
}
ساکن خالی *rpaligned_alloc_الٹ(size_t سائز_2, size_t align) {
  واپس rpaligned_alloc(align, سائز_2);
}
ساکن خالی *rpaligned_alloc_الٹ_nothrow(size_t سائز_2, size_t align,
                                             rp_nothrow_t t) {
  (خالی)حجم(t);
  واپس rpaligned_alloc(align, سائز_2);
}
ساکن خالی rpfree_سائز(خالی *p, size_t سائز_2) {
  (خالی)حجم(سائز_2);
  rpfree(p);
}
ساکن خالی rpfree_aligned(خالی *p, size_t align) {
  (خالی)حجم(align);
  rpfree(p);
}
ساکن خالی rpfree_سائز_aligned(خالی *p, size_t سائز_2, size_t align) {
  (خالی)حجم(سائز_2);
  (خالی)حجم(align);
  rpfree(p);
}

#اگر_ختم

#اگر USE_INTERPOSE

__attribute__((مستعمل)) ساکن ثابت interpose_t macinterpose_malloc[]
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
        MAC_INTERPOSE_PAIR(rpfree_سائز, _ZdlPvm),
        MAC_INTERPOSE_PAIR(rpfree_سائز, _ZdaPvm),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdlPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdaPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_سائز_aligned, _ZdlPvmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_سائز_aligned, _ZdaPvmSt11align_val_t),
        // libc entry points
        MAC_INTERPOSE_PAIR(rpmalloc, حافظہ_دو),
        MAC_INTERPOSE_PAIR(rpmalloc, صفر_حافظہ_دو),
        MAC_INTERPOSE_PAIR(rprealloc, realloc),
        MAC_INTERPOSE_PAIR(rprealloc, reallocf),
#اگر defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#اگر_ختم
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, آزاد), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_سائز, malloc_usable_سائز),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_سائز, malloc_سائز)};

#اگر_ختم

#اگر USE_ALIAS

#تعریف_کرو RPALIAS(fn) __attribute__((alias(#fn), مستعمل, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
خالی _ZdlPv(خالی *p) RPALIAS(rpfree) خالی _ZdaPv(خالی *p) RPALIAS(rpfree)

#اگر ARCH_64_ثنائی_ہندسہ
    // 64-bit operators new and new[], normal and aligned
    خالی *_Znwm(uint64_t سائز_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_سائز(1)
        RPALIAS(rpmalloc) خالی *_Znam(uint64_t سائز_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_سائز(1) RPALIAS(rpmalloc) خالی *_Znwmm(uint64_t سائز_2,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_الٹ) خالی *_Znamm(uint64_t سائز_2,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_الٹ) خالی *_ZnwmSt11align_val_t(
                size_t سائز_2, size_t align)
                RPALIAS(rpaligned_alloc_الٹ) خالی *_ZnamSt11align_val_t(
                    size_t سائز_2, size_t align)
                    RPALIAS(rpaligned_alloc_الٹ) خالی *_ZnwmRKSt9nothrow_t(
                        size_t سائز_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) خالی *_ZnamRKSt9nothrow_t(
                            size_t سائز_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) خالی
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t سائز_2,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_الٹ_nothrow) خالی
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t سائز_2, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_الٹ_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    خالی _ZdlPvm(خالی *p, size_t n) RPALIAS(rpfree_سائز) خالی _ZdaPvm(خالی *p,
                                                                      size_t n)
        RPALIAS(rpfree_سائز) خالی _ZdlPvSt11align_val_t(خالی *p, size_t a)
            RPALIAS(rpfree_aligned) خالی _ZdaPvSt11align_val_t(خالی *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) خالی _ZdlPvmSt11align_val_t(خالی *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_سائز_aligned) خالی _ZdaPvmSt11align_val_t(
                        خالی *p, size_t n, size_t a)
                        RPALIAS(rpfree_سائز_aligned)
#ورنہ
    // 32-bit operators new and new[], normal and aligned
    خالی *_Znwj(uint32_t سائز_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_سائز(1)
        RPALIAS(rpmalloc) خالی *_Znaj(uint32_t سائز_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_سائز(1) RPALIAS(rpmalloc) خالی *_Znwjj(uint32_t سائز_2,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_الٹ) خالی *_Znajj(uint32_t سائز_2,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_الٹ) خالی *_ZnwjSt11align_val_t(
                size_t سائز_2, size_t align)
                RPALIAS(rpaligned_alloc_الٹ) خالی *_ZnajSt11align_val_t(
                    size_t سائز_2, size_t align)
                    RPALIAS(rpaligned_alloc_الٹ) خالی *_ZnwjRKSt9nothrow_t(
                        size_t سائز_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) خالی *_ZnajRKSt9nothrow_t(
                            size_t سائز_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) خالی
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t سائز_2,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_الٹ_nothrow) خالی
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t سائز_2, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_الٹ_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    خالی _ZdlPvj(خالی *p, size_t n) RPALIAS(rpfree_سائز) خالی _ZdaPvj(خالی *p,
                                                                      size_t n)
        RPALIAS(rpfree_سائز) خالی _ZdlPvSt11align_val_t(خالی *p, size_t a)
            RPALIAS(rpfree_aligned) خالی _ZdaPvSt11align_val_t(خالی *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) خالی _ZdlPvjSt11align_val_t(خالی *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_سائز_aligned) خالی _ZdaPvjSt11align_val_t(
                        خالی *p, size_t n, size_t a)
                        RPALIAS(rpfree_سائز_aligned)
#اگر_ختم

                            خالی *حافظہ_دو(size_t سائز_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_سائز(1)
        RPALIAS(rpmalloc) خالی *صفر_حافظہ_دو(size_t گنتی, size_t سائز_2)
            RPALIAS(rpcalloc) خالی *realloc(خالی *ptr, size_t سائز_2)
                RPALIAS(rprealloc) خالی *reallocf(خالی *ptr, size_t سائز_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_سائز(2)
        RPALIAS(rprealloc) خالی *aligned_alloc(size_t alignment, size_t سائز_2)
            RPALIAS(rpaligned_alloc) خالی *memalign(
                size_t alignment, size_t سائز_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_سائز(2)
        RPALIAS(rpmemalign) صحیح posix_memalign(خالی **memptr, size_t alignment,
                                               size_t سائز_2)
            RPALIAS(rpposix_memalign) خالی آزاد(خالی *ptr)
                RPALIAS(rpfree) خالی cfree(خالی *ptr) RPALIAS(rpfree)
#اگر defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_سائز(ثابت خالی *ptr) RPALIAS(rpmalloc_usable_سائز)
#ورنہ
                    size_t
    malloc_usable_سائز(خالی *ptr) RPALIAS(rpmalloc_usable_سائز)
#اگر_ختم
        size_t malloc_سائز(خالی *ptr) RPALIAS(rpmalloc_usable_سائز)

#اگر_ختم

            ساکن اندرونی size_t rpmalloc_صفحہ_سائز(خالی) {
  واپس حافظہ_صفحہ_سائز;
}

خارجی خالی *RPMALLOC_CDECL reallocarray(خالی *ptr, size_t گنتی, size_t سائز_2);

خارجی خالی *RPMALLOC_CDECL reallocarray(خالی *ptr, size_t گنتی, size_t سائز_2) {
  size_t کل_2;
#اگر ENABLE_VALIDATE_دلائل
#اگر_تعریف_شدہ _MSC_VER
  صحیح err = سائز_TMult(گنتی, سائز_2, &کل_2);
  اگر ((err != S_OK) || (کل_2 >= MAX_ALLOC_سائز)) {
    errno = EINVAL;
    واپس 0;
  }
#ورنہ
  صحیح err = __builtin_umull_overflow(گنتی, سائز_2, &کل_2);
  اگر (err || (کل_2 >= MAX_ALLOC_سائز)) {
    errno = EINVAL;
    واپس 0;
  }
#اگر_ختم
#ورنہ
  کل_2 = گنتی * سائز_2;
#اگر_ختم
  واپس realloc(ptr, کل_2);
}

خارجی اندرونی خالی *RPMALLOC_CDECL valloc(size_t سائز_2) {
  get_thread_ڈھیر();
  واپس rpaligned_alloc(rpmalloc_صفحہ_سائز(), سائز_2);
}

خارجی اندرونی خالی *RPMALLOC_CDECL pvalloc(size_t سائز_2) {
  get_thread_ڈھیر();
  ثابت size_t صفحہ_سائز_2 = rpmalloc_صفحہ_سائز();
  ثابت size_t aligned_سائز = ((سائز_2 + صفحہ_سائز_2 - 1) / صفحہ_سائز_2) * صفحہ_سائز_2;
#اگر ENABLE_VALIDATE_دلائل
  اگر (aligned_سائز < سائز_2) {
    errno = EINVAL;
    واپس 0;
  }
#اگر_ختم
  واپس rpaligned_alloc(rpmalloc_صفحہ_سائز(), aligned_سائز);
}

#اگر_ختم // ENABLE_OVERRIDE

#اگر ENABLE_PRELOAD

#اگر_تعریف_شدہ _WIN32

#اگر defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

خارجی __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

خارجی __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (خالی)حجم(reserved);
  (خالی)حجم(instance);
  اگر (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  ورنہ اگر (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  ورنہ اگر (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  ورنہ اگر (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  واپس TRUE;
}

// end BUILD_DYNAMIC_LINK
#ورنہ

خارجی خالی _global_rpmalloc_init(خالی) {
  rpmalloc_سیٹ_main_thread();
  rpmalloc_initialize();
}

#اگر defined(__clang__) || defined(__GNUC__)

ساکن خالی __attribute__((constructor)) initializer(خالی) {
  _global_rpmalloc_init();
}

#وگرنہ_اگر defined(_MSC_VER)

ساکن صحیح _global_rpmalloc_xib(خالی) {
  _global_rpmalloc_init();
  واپس 0;
}

#ہدایت section(".CRT$XIB", اقرأ)
__declspec(allocate(".CRT$XIB")) خالی (*_rpmalloc_module_init)(خالی) =
    _global_rpmalloc_xib;
#اگر defined(_M_IX86) || defined(__i386__)
#ہدایت comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#ورنہ
#ہدایت comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#اگر_ختم

#اگر_ختم

// end !BUILD_DYNAMIC_LINK
#اگر_ختم

#ورنہ

#شامل <pthread.h>
#شامل <stdint.h>
#شامل <stdlib.h>
#شامل <unistd.h>

خارجی خالی rpmalloc_سیٹ_main_thread(خالی);

ساکن pthread_key_t destructor_کلید;

ساکن خالی thread_destructor(خالی *);

ساکن خالی __attribute__((constructor)) initializer(خالی) {
  rpmalloc_سیٹ_main_thread();
  rpmalloc_initialize();
  pthread_کلید_create(&destructor_کلید, thread_destructor);
}

ساکن خالی __attribute__((destructor)) finalizer(خالی) { rpmalloc_finalize(); }

قسمتعریف ساخت {
  خالی *(*real_آغاز)(خالی *);
  خالی *real_arg;
} thread_starter_arg;

ساکن خالی *thread_starter(خالی *argptr) {
  thread_starter_arg *arg = argptr;
  خالی *(*real_آغاز)(خالی *) = arg->real_آغاز;
  خالی *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_کلید, (خالی *)1);
  واپس (*real_آغاز)(real_arg);
}

ساکن خالی thread_destructor(خالی *قدر_2) {
  (خالی)حجم(قدر_2);
  rpmalloc_thread_finalize(1);
}

#اگر_تعریف_شدہ __APPLE__

ساکن صحیح pthread_create_proxy(pthread_t *thread, ثابت pthread_attr_t *attr,
                                خالی *(*آغاز_routine)(خالی *), خالی *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(حجم(thread_starter_arg));
  starter_arg->real_آغاز = آغاز_routine;
  starter_arg->real_arg = arg;
  واپس pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#ورنہ

#شامل <dlfcn.h>

صحیح pthread_create(pthread_t *thread, ثابت pthread_attr_t *attr,
                   خالی *(*آغاز_routine)(خالی *), خالی *arg) {
#اگر defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  حرف fname[] = "pthread_create";
#ورنہ
  حرف fname[] = "_pthread_create";
#اگر_ختم
  خالی *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(حجم(thread_starter_arg));
  starter_arg->real_آغاز = آغاز_routine;
  starter_arg->real_arg = arg;
  واپس (*(صحیح (*)(pthread_t *, ثابت pthread_attr_t *, خالی *(*)(خالی *),
                    خالی *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#اگر_ختم

#اگر_ختم

#اگر_ختم

#اگر ENABLE_OVERRIDE

#اگر defined(__GLIBC__) && defined(__linux__)

خالی *__libc_malloc(size_t سائز_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_سائز(1)
        RPALIAS(rpmalloc) خالی *__libc_calloc(size_t گنتی, size_t سائز_2)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) خالی *__libc_realloc(خالی *p, size_t سائز_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_سائز(2) RPALIAS(rprealloc) خالی __libc_free(خالی *p)
        RPALIAS(rpfree) خالی __libc_cfree(خالی *p)
            RPALIAS(rpfree) خالی *__libc_memalign(size_t align, size_t سائز_2)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_سائز(2)
        RPALIAS(rpmemalign) صحیح __posix_memalign(خالی **p, size_t align,
                                                 size_t سائز_2)
            RPALIAS(rpposix_memalign)

                خارجی خالی *__libc_valloc(size_t سائز_2);
خارجی خالی *__libc_pvalloc(size_t سائز_2);

خالی *__libc_valloc(size_t سائز_2) { واپس valloc(سائز_2); }

خالی *__libc_pvalloc(size_t سائز_2) { واپس pvalloc(سائز_2); }

#اگر_ختم

#اگر_ختم

#اگر (defined(__GNUC__) || defined(__clang__))
#ہدایت GCC visibility نکالو
#اگر_ختم
