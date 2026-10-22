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

#ਜੇ defined(__TINYC__)
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ARCH_64_ਦੋਆਧਾਰੀ_ਅੰਕ
#ਜੇ defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ARCH_64_ਦੋਆਧਾਰੀ_ਅੰਕ 1
_Static_assert(ਆਕਾਰ(size_t) == 8, "Data type size mismatch");
_Static_assert(ਆਕਾਰ(ਖਾਲੀ *) == 8, "Data type size mismatch");
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ARCH_64_ਦੋਆਧਾਰੀ_ਅੰਕ 0
_Static_assert(ਆਕਾਰ(size_t) == 4, "Data type size mismatch");
_Static_assert(ਆਕਾਰ(ਖਾਲੀ *) == 4, "Data type size mismatch");
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ (defined(__GNUC__) || defined(__clang__))
#ਹਦਾਇਤ GCC visibility ਧੱਕੋ(ਮੂਲ)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_IMPLEMENT 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_INTERPOSE 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_ALIAS 0

#ਜੇ defined(__APPLE__)
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ USE_INTERPOSE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_INTERPOSE 1

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ interpose_t {
  ਖਾਲੀ *new_func;
  ਖਾਲੀ *orig_func;
} interpose_t;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAC_INTERPOSE_PAIR(newf, oldf) {(ਖਾਲੀ *)newf, (ਖਾਲੀ *)oldf}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((ਵਰਤਿਆ)) ਸਥਿਰ ਅਚਲ interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#ਜੇ_ਅੰਤ

#ਜੇ !defined(_WIN32) && !defined(__APPLE__)
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ USE_IMPLEMENT
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ USE_ALIAS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_IMPLEMENT 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_ALIAS 1
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
#ਹਦਾਇਤ warning(disable : 4100)
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਸਮ੍ਰਿਤੀ_ਦਿਓ
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਮੁਕਤ
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_RESTRICT __declspec(ਸੀਮਿਤ)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_RESTRICT
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_OVERRIDE

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ rp_nothrow_t {
  ਪੂਰਨ __dummy;
} rp_nothrow_t;

#ਜੇ USE_IMPLEMENT

ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_RESTRICT ਖਾਲੀ *RPMALLOC_CDECL ਸਮ੍ਰਿਤੀ_ਦਿਓ(size_t ਮਾਪ_2) {
  ਵਾਪਸ rpmalloc(ਮਾਪ_2);
}
ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_RESTRICT ਖਾਲੀ *RPMALLOC_CDECL ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(size_t ਗਿਣਤੀ_2,
                                                            size_t ਮਾਪ_2) {
  ਵਾਪਸ rpcalloc(ਗਿਣਤੀ_2, ਮਾਪ_2);
}
ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_RESTRICT ਖਾਲੀ *RPMALLOC_CDECL realloc(ਖਾਲੀ *ptr,
                                                             size_t ਮਾਪ_2) {
  ਵਾਪਸ rprealloc(ptr, ਮਾਪ_2);
}
ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ *RPMALLOC_CDECL reallocf(ਖਾਲੀ *ptr, size_t ਮਾਪ_2) {
  ਵਾਪਸ rprealloc(ptr, ਮਾਪ_2);
}
ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t ਮਾਪ_2) {
  ਵਾਪਸ rpaligned_alloc(alignment, ਮਾਪ_2);
}
ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ *RPMALLOC_CDECL memalign(size_t alignment, size_t ਮਾਪ_2) {
  ਵਾਪਸ rpmemalign(alignment, ਮਾਪ_2);
}
ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਪੂਰਨ RPMALLOC_CDECL posix_memalign(ਖਾਲੀ **memptr, size_t alignment,
                                                size_t ਮਾਪ_2) {
  ਵਾਪਸ rpposix_memalign(memptr, alignment, ਮਾਪ_2);
}
ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ RPMALLOC_CDECL ਮੁਕਤ(ਖਾਲੀ *ptr) { rpfree(ptr); }
ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ RPMALLOC_CDECL cfree(ਖਾਲੀ *ptr) { rpfree(ptr); }
ਬਾਹਰੀ ਸਤਰਅੰਦਰ size_t RPMALLOC_CDECL malloc_usable_ਮਾਪ(ਖਾਲੀ *ptr) {
  ਵਾਪਸ rpmalloc_usable_ਮਾਪ(ptr);
}
ਬਾਹਰੀ ਸਤਰਅੰਦਰ size_t RPMALLOC_CDECL malloc_ਮਾਪ(ਖਾਲੀ *ptr) {
  ਵਾਪਸ rpmalloc_usable_ਮਾਪ(ptr);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_RESTRICT ਖਾਲੀ *RPMALLOC_CDECL malloc_ਅਧਾਰ(size_t ਮਾਪ_2) {
  ਵਾਪਸ rpmalloc(ਮਾਪ_2);
}
ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ RPMALLOC_CDECL free_ਅਧਾਰ(ਖਾਲੀ *ptr) { rpfree(ptr); }
ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_RESTRICT ਖਾਲੀ *RPMALLOC_CDECL calloc_ਅਧਾਰ(size_t ਗਿਣਤੀ_2,
                                                                  size_t ਮਾਪ_2) {
  ਵਾਪਸ rpcalloc(ਗਿਣਤੀ_2, ਮਾਪ_2);
}
ਬਾਹਰੀ ਸਤਰਅੰਦਰ size_t RPMALLOC_CDECL _msize(ਖਾਲੀ *ptr) {
  ਵਾਪਸ rpmalloc_usable_ਮਾਪ(ptr);
}
ਬਾਹਰੀ ਸਤਰਅੰਦਰ size_t RPMALLOC_CDECL msize_ਅਧਾਰ(ਖਾਲੀ *ptr) {
  ਵਾਪਸ rpmalloc_usable_ਮਾਪ(ptr);
}
ਬਾਹਰੀ ਸਤਰਅੰਦਰ RPMALLOC_RESTRICT ਖਾਲੀ *RPMALLOC_CDECL
realloc_ਅਧਾਰ(ਖਾਲੀ *ptr, size_t ਮਾਪ_2) {
  ਵਾਪਸ rprealloc(ptr, ਮਾਪ_2);
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#ਨਹੀਂਤਾਂ
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPDEFVIS __attribute__((visibility("default")))
ਬਾਹਰੀ ਖਾਲੀ _ZdlPv(ਖਾਲੀ *p);
ਖਾਲੀ RPDEFVIS _ZdlPv(ਖਾਲੀ *p) { rpfree(p); }
ਬਾਹਰੀ ਖਾਲੀ _ZdaPv(ਖਾਲੀ *p);
ਖਾਲੀ RPDEFVIS _ZdaPv(ਖਾਲੀ *p) { rpfree(p); }
#ਜੇ ARCH_64_ਦੋਆਧਾਰੀ_ਅੰਕ
// 64-bit operators new and new[], normal and aligned
ਬਾਹਰੀ ਖਾਲੀ *_Znwm(uint64_t ਮਾਪ_2);
ਖਾਲੀ *RPDEFVIS _Znwm(uint64_t ਮਾਪ_2) { ਵਾਪਸ rpmalloc(ਮਾਪ_2); }
ਬਾਹਰੀ ਖਾਲੀ *_Znam(uint64_t ਮਾਪ_2);
ਖਾਲੀ *RPDEFVIS _Znam(uint64_t ਮਾਪ_2) { ਵਾਪਸ rpmalloc(ਮਾਪ_2); }
ਬਾਹਰੀ ਖਾਲੀ *_Znwmm(uint64_t ਮਾਪ_2, uint64_t align);
ਖਾਲੀ *RPDEFVIS _Znwmm(uint64_t ਮਾਪ_2, uint64_t align) {
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_Znamm(uint64_t ਮਾਪ_2, uint64_t align);
ਖਾਲੀ *RPDEFVIS _Znamm(uint64_t ਮਾਪ_2, uint64_t align) {
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_ZnwmSt11align_val_t(uint64_t ਮਾਪ_2, uint64_t align);
ਖਾਲੀ *RPDEFVIS _ZnwmSt11align_val_t(uint64_t ਮਾਪ_2, uint64_t align) {
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_ZnamSt11align_val_t(uint64_t ਮਾਪ_2, uint64_t align);
ਖਾਲੀ *RPDEFVIS _ZnamSt11align_val_t(uint64_t ਮਾਪ_2, uint64_t align) {
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_ZnwmRKSt9nothrow_t(uint64_t ਮਾਪ_2, rp_nothrow_t t);
ਖਾਲੀ *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t ਮਾਪ_2, rp_nothrow_t t) {
  (ਖਾਲੀ)ਆਕਾਰ(t);
  ਵਾਪਸ rpmalloc(ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_ZnamRKSt9nothrow_t(uint64_t ਮਾਪ_2, rp_nothrow_t t);
ਖਾਲੀ *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t ਮਾਪ_2, rp_nothrow_t t) {
  (ਖਾਲੀ)ਆਕਾਰ(t);
  ਵਾਪਸ rpmalloc(ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t ਮਾਪ_2, uint64_t align,
                                                rp_nothrow_t t);
ਖਾਲੀ *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t ਮਾਪ_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (ਖਾਲੀ)ਆਕਾਰ(t);
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t ਮਾਪ_2, uint64_t align,
                                                rp_nothrow_t t);
ਖਾਲੀ *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t ਮਾਪ_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (ਖਾਲੀ)ਆਕਾਰ(t);
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
// 64-bit operators sized delete and delete[], normal and aligned
ਬਾਹਰੀ ਖਾਲੀ _ZdlPvm(ਖਾਲੀ *p, uint64_t ਮਾਪ_2);
ਖਾਲੀ RPDEFVIS _ZdlPvm(ਖਾਲੀ *p, uint64_t ਮਾਪ_2) {
  rpfree(p);
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ _ZdaPvm(ਖਾਲੀ *p, uint64_t ਮਾਪ_2);
ਖਾਲੀ RPDEFVIS _ZdaPvm(ਖਾਲੀ *p, uint64_t ਮਾਪ_2) {
  rpfree(p);
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ _ZdlPvSt11align_val_t(ਖਾਲੀ *p, uint64_t align);
ਖਾਲੀ RPDEFVIS _ZdlPvSt11align_val_t(ਖਾਲੀ *p, uint64_t align) {
  rpfree(p);
  (ਖਾਲੀ)ਆਕਾਰ(align);
}
ਬਾਹਰੀ ਖਾਲੀ _ZdaPvSt11align_val_t(ਖਾਲੀ *p, uint64_t align);
ਖਾਲੀ RPDEFVIS _ZdaPvSt11align_val_t(ਖਾਲੀ *p, uint64_t align) {
  rpfree(p);
  (ਖਾਲੀ)ਆਕਾਰ(align);
}
ਬਾਹਰੀ ਖਾਲੀ _ZdlPvmSt11align_val_t(ਖਾਲੀ *p, uint64_t ਮਾਪ_2, uint64_t align);
ਖਾਲੀ RPDEFVIS _ZdlPvmSt11align_val_t(ਖਾਲੀ *p, uint64_t ਮਾਪ_2, uint64_t align) {
  rpfree(p);
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
  (ਖਾਲੀ)ਆਕਾਰ(align);
}
ਬਾਹਰੀ ਖਾਲੀ _ZdaPvmSt11align_val_t(ਖਾਲੀ *p, uint64_t ਮਾਪ_2, uint64_t align);
ਖਾਲੀ RPDEFVIS _ZdaPvmSt11align_val_t(ਖਾਲੀ *p, uint64_t ਮਾਪ_2, uint64_t align) {
  rpfree(p);
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
  (ਖਾਲੀ)ਆਕਾਰ(align);
}
#ਨਹੀਂਤਾਂ
// 32-bit operators new and new[], normal and aligned
ਬਾਹਰੀ ਖਾਲੀ *_Znwj(uint32_t ਮਾਪ_2);
ਖਾਲੀ *RPDEFVIS _Znwj(uint32_t ਮਾਪ_2) { ਵਾਪਸ rpmalloc(ਮਾਪ_2); }
ਬਾਹਰੀ ਖਾਲੀ *_Znaj(uint32_t ਮਾਪ_2);
ਖਾਲੀ *RPDEFVIS _Znaj(uint32_t ਮਾਪ_2) { ਵਾਪਸ rpmalloc(ਮਾਪ_2); }
ਬਾਹਰੀ ਖਾਲੀ *_Znwjj(uint32_t ਮਾਪ_2, uint32_t align);
ਖਾਲੀ *RPDEFVIS _Znwjj(uint32_t ਮਾਪ_2, uint32_t align) {
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_Znajj(uint32_t ਮਾਪ_2, uint32_t align);
ਖਾਲੀ *RPDEFVIS _Znajj(uint32_t ਮਾਪ_2, uint32_t align) {
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_ZnwjSt11align_val_t(size_t ਮਾਪ_2, size_t align);
ਖਾਲੀ *RPDEFVIS _ZnwjSt11align_val_t(size_t ਮਾਪ_2, size_t align) {
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_ZnajSt11align_val_t(size_t ਮਾਪ_2, size_t align);
ਖਾਲੀ *RPDEFVIS _ZnajSt11align_val_t(size_t ਮਾਪ_2, size_t align) {
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_ZnwjRKSt9nothrow_t(size_t ਮਾਪ_2, rp_nothrow_t t);
ਖਾਲੀ *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t ਮਾਪ_2, rp_nothrow_t t) {
  (ਖਾਲੀ)ਆਕਾਰ(t);
  ਵਾਪਸ rpmalloc(ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_ZnajRKSt9nothrow_t(size_t ਮਾਪ_2, rp_nothrow_t t);
ਖਾਲੀ *RPDEFVIS _ZnajRKSt9nothrow_t(size_t ਮਾਪ_2, rp_nothrow_t t) {
  (ਖਾਲੀ)ਆਕਾਰ(t);
  ਵਾਪਸ rpmalloc(ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t ਮਾਪ_2, size_t align,
                                                rp_nothrow_t t);
ਖਾਲੀ *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t ਮਾਪ_2, size_t align,
                                                  rp_nothrow_t t) {
  (ਖਾਲੀ)ਆਕਾਰ(t);
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ *_ZnajSt11align_val_tRKSt9nothrow_t(size_t ਮਾਪ_2, size_t align,
                                                rp_nothrow_t t);
ਖਾਲੀ *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t ਮਾਪ_2, size_t align,
                                                  rp_nothrow_t t) {
  (ਖਾਲੀ)ਆਕਾਰ(t);
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
// 32-bit operators sized delete and delete[], normal and aligned
ਬਾਹਰੀ ਖਾਲੀ _ZdlPvj(ਖਾਲੀ *p, uint64_t ਮਾਪ_2);
ਖਾਲੀ RPDEFVIS _ZdlPvj(ਖਾਲੀ *p, uint64_t ਮਾਪ_2) {
  rpfree(p);
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ _ZdaPvj(ਖਾਲੀ *p, uint64_t ਮਾਪ_2);
ਖਾਲੀ RPDEFVIS _ZdaPvj(ਖਾਲੀ *p, uint64_t ਮਾਪ_2) {
  rpfree(p);
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
}
ਬਾਹਰੀ ਖਾਲੀ _ZdlPvSt11align_val_t(ਖਾਲੀ *p, uint32_t align);
ਖਾਲੀ RPDEFVIS _ZdlPvSt11align_val_t(ਖਾਲੀ *p, uint64_t a) {
  rpfree(p);
  (ਖਾਲੀ)ਆਕਾਰ(align);
}
ਬਾਹਰੀ ਖਾਲੀ _ZdaPvSt11align_val_t(ਖਾਲੀ *p, uint32_t align);
ਖਾਲੀ RPDEFVIS _ZdaPvSt11align_val_t(ਖਾਲੀ *p, uint64_t a) {
  rpfree(p);
  (ਖਾਲੀ)ਆਕਾਰ(align);
}
ਬਾਹਰੀ ਖਾਲੀ _ZdlPvjSt11align_val_t(ਖਾਲੀ *p, uint32_t ਮਾਪ_2, uint32_t align);
ਖਾਲੀ RPDEFVIS _ZdlPvjSt11align_val_t(ਖਾਲੀ *p, uint64_t ਮਾਪ_2, uint64_t align) {
  rpfree(p);
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
  (ਖਾਲੀ)ਆਕਾਰ(a);
}
ਬਾਹਰੀ ਖਾਲੀ _ZdaPvjSt11align_val_t(ਖਾਲੀ *p, uint32_t ਮਾਪ_2, uint32_t align);
ਖਾਲੀ RPDEFVIS _ZdaPvjSt11align_val_t(ਖਾਲੀ *p, uint64_t ਮਾਪ_2, uint64_t align) {
  rpfree(p);
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
  (ਖਾਲੀ)ਆਕਾਰ(a);
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ USE_INTERPOSE || USE_ALIAS

ਸਥਿਰ ਖਾਲੀ *rpmalloc_nothrow(size_t ਮਾਪ_2, rp_nothrow_t t) {
  (ਖਾਲੀ)ਆਕਾਰ(t);
  ਵਾਪਸ rpmalloc(ਮਾਪ_2);
}
ਸਥਿਰ ਖਾਲੀ *rpaligned_alloc_ਉਲਟ(size_t ਮਾਪ_2, size_t align) {
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
ਸਥਿਰ ਖਾਲੀ *rpaligned_alloc_ਉਲਟ_nothrow(size_t ਮਾਪ_2, size_t align,
                                             rp_nothrow_t t) {
  (ਖਾਲੀ)ਆਕਾਰ(t);
  ਵਾਪਸ rpaligned_alloc(align, ਮਾਪ_2);
}
ਸਥਿਰ ਖਾਲੀ rpfree_ਮਾਪ(ਖਾਲੀ *p, size_t ਮਾਪ_2) {
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
  rpfree(p);
}
ਸਥਿਰ ਖਾਲੀ rpfree_aligned(ਖਾਲੀ *p, size_t align) {
  (ਖਾਲੀ)ਆਕਾਰ(align);
  rpfree(p);
}
ਸਥਿਰ ਖਾਲੀ rpfree_ਮਾਪ_aligned(ਖਾਲੀ *p, size_t ਮਾਪ_2, size_t align) {
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
  (ਖਾਲੀ)ਆਕਾਰ(align);
  rpfree(p);
}

#ਜੇ_ਅੰਤ

#ਜੇ USE_INTERPOSE

__attribute__((ਵਰਤਿਆ)) ਸਥਿਰ ਅਚਲ interpose_t macinterpose_malloc[]
    __attribute__((section("__DATA, __interpose"))) = {
        // new and new[]
        MAC_INTERPOSE_PAIR(rpmalloc, _Znwm),
        MAC_INTERPOSE_PAIR(rpmalloc, _Znam),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_ਉਲਟ, _Znwmm),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_ਉਲਟ, _Znamm),
        MAC_INTERPOSE_PAIR(rpmalloc_nothrow, _ZnwmRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpmalloc_nothrow, _ZnamRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_ਉਲਟ, _ZnwmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_ਉਲਟ, _ZnamSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_ਉਲਟ_nothrow,
                           _ZnwmSt11align_val_tRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_ਉਲਟ_nothrow,
                           _ZnamSt11align_val_tRKSt9nothrow_t),
        // delete and delete[]
        MAC_INTERPOSE_PAIR(rpfree, _ZdlPv), MAC_INTERPOSE_PAIR(rpfree, _ZdaPv),
        MAC_INTERPOSE_PAIR(rpfree_ਮਾਪ, _ZdlPvm),
        MAC_INTERPOSE_PAIR(rpfree_ਮਾਪ, _ZdaPvm),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdlPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdaPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_ਮਾਪ_aligned, _ZdlPvmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_ਮਾਪ_aligned, _ZdaPvmSt11align_val_t),
        // libc entry points
        MAC_INTERPOSE_PAIR(rpmalloc, ਸਮ੍ਰਿਤੀ_ਦਿਓ),
        MAC_INTERPOSE_PAIR(rpmalloc, ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ),
        MAC_INTERPOSE_PAIR(rprealloc, realloc),
        MAC_INTERPOSE_PAIR(rprealloc, reallocf),
#ਜੇ defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#ਜੇ_ਅੰਤ
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, ਮੁਕਤ), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_ਮਾਪ, malloc_usable_ਮਾਪ),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_ਮਾਪ, malloc_ਮਾਪ)};

#ਜੇ_ਅੰਤ

#ਜੇ USE_ALIAS

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPALIAS(fn) __attribute__((alias(#fn), ਵਰਤਿਆ, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
ਖਾਲੀ _ZdlPv(ਖਾਲੀ *p) RPALIAS(rpfree) ਖਾਲੀ _ZdaPv(ਖਾਲੀ *p) RPALIAS(rpfree)

#ਜੇ ARCH_64_ਦੋਆਧਾਰੀ_ਅੰਕ
    // 64-bit operators new and new[], normal and aligned
    ਖਾਲੀ *_Znwm(uint64_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(1)
        RPALIAS(rpmalloc) ਖਾਲੀ *_Znam(uint64_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(1) RPALIAS(rpmalloc) ਖਾਲੀ *_Znwmm(uint64_t ਮਾਪ_2,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_ਉਲਟ) ਖਾਲੀ *_Znamm(uint64_t ਮਾਪ_2,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_ਉਲਟ) ਖਾਲੀ *_ZnwmSt11align_val_t(
                size_t ਮਾਪ_2, size_t align)
                RPALIAS(rpaligned_alloc_ਉਲਟ) ਖਾਲੀ *_ZnamSt11align_val_t(
                    size_t ਮਾਪ_2, size_t align)
                    RPALIAS(rpaligned_alloc_ਉਲਟ) ਖਾਲੀ *_ZnwmRKSt9nothrow_t(
                        size_t ਮਾਪ_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) ਖਾਲੀ *_ZnamRKSt9nothrow_t(
                            size_t ਮਾਪ_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) ਖਾਲੀ
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t ਮਾਪ_2,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_ਉਲਟ_nothrow) ਖਾਲੀ
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t ਮਾਪ_2, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_ਉਲਟ_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    ਖਾਲੀ _ZdlPvm(ਖਾਲੀ *p, size_t n) RPALIAS(rpfree_ਮਾਪ) ਖਾਲੀ _ZdaPvm(ਖਾਲੀ *p,
                                                                      size_t n)
        RPALIAS(rpfree_ਮਾਪ) ਖਾਲੀ _ZdlPvSt11align_val_t(ਖਾਲੀ *p, size_t a)
            RPALIAS(rpfree_aligned) ਖਾਲੀ _ZdaPvSt11align_val_t(ਖਾਲੀ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) ਖਾਲੀ _ZdlPvmSt11align_val_t(ਖਾਲੀ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_ਮਾਪ_aligned) ਖਾਲੀ _ZdaPvmSt11align_val_t(
                        ਖਾਲੀ *p, size_t n, size_t a)
                        RPALIAS(rpfree_ਮਾਪ_aligned)
#ਨਹੀਂਤਾਂ
    // 32-bit operators new and new[], normal and aligned
    ਖਾਲੀ *_Znwj(uint32_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(1)
        RPALIAS(rpmalloc) ਖਾਲੀ *_Znaj(uint32_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(1) RPALIAS(rpmalloc) ਖਾਲੀ *_Znwjj(uint32_t ਮਾਪ_2,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_ਉਲਟ) ਖਾਲੀ *_Znajj(uint32_t ਮਾਪ_2,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_ਉਲਟ) ਖਾਲੀ *_ZnwjSt11align_val_t(
                size_t ਮਾਪ_2, size_t align)
                RPALIAS(rpaligned_alloc_ਉਲਟ) ਖਾਲੀ *_ZnajSt11align_val_t(
                    size_t ਮਾਪ_2, size_t align)
                    RPALIAS(rpaligned_alloc_ਉਲਟ) ਖਾਲੀ *_ZnwjRKSt9nothrow_t(
                        size_t ਮਾਪ_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) ਖਾਲੀ *_ZnajRKSt9nothrow_t(
                            size_t ਮਾਪ_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) ਖਾਲੀ
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t ਮਾਪ_2,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_ਉਲਟ_nothrow) ਖਾਲੀ
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t ਮਾਪ_2, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_ਉਲਟ_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    ਖਾਲੀ _ZdlPvj(ਖਾਲੀ *p, size_t n) RPALIAS(rpfree_ਮਾਪ) ਖਾਲੀ _ZdaPvj(ਖਾਲੀ *p,
                                                                      size_t n)
        RPALIAS(rpfree_ਮਾਪ) ਖਾਲੀ _ZdlPvSt11align_val_t(ਖਾਲੀ *p, size_t a)
            RPALIAS(rpfree_aligned) ਖਾਲੀ _ZdaPvSt11align_val_t(ਖਾਲੀ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) ਖਾਲੀ _ZdlPvjSt11align_val_t(ਖਾਲੀ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_ਮਾਪ_aligned) ਖਾਲੀ _ZdaPvjSt11align_val_t(
                        ਖਾਲੀ *p, size_t n, size_t a)
                        RPALIAS(rpfree_ਮਾਪ_aligned)
#ਜੇ_ਅੰਤ

                            ਖਾਲੀ *ਸਮ੍ਰਿਤੀ_ਦਿਓ(size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(1)
        RPALIAS(rpmalloc) ਖਾਲੀ *ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(size_t ਗਿਣਤੀ_2, size_t ਮਾਪ_2)
            RPALIAS(rpcalloc) ਖਾਲੀ *realloc(ਖਾਲੀ *ptr, size_t ਮਾਪ_2)
                RPALIAS(rprealloc) ਖਾਲੀ *reallocf(ਖਾਲੀ *ptr, size_t ਮਾਪ_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(2)
        RPALIAS(rprealloc) ਖਾਲੀ *aligned_alloc(size_t alignment, size_t ਮਾਪ_2)
            RPALIAS(rpaligned_alloc) ਖਾਲੀ *memalign(
                size_t alignment, size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(2)
        RPALIAS(rpmemalign) ਪੂਰਨ posix_memalign(ਖਾਲੀ **memptr, size_t alignment,
                                               size_t ਮਾਪ_2)
            RPALIAS(rpposix_memalign) ਖਾਲੀ ਮੁਕਤ(ਖਾਲੀ *ptr)
                RPALIAS(rpfree) ਖਾਲੀ cfree(ਖਾਲੀ *ptr) RPALIAS(rpfree)
#ਜੇ defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_ਮਾਪ(ਅਚਲ ਖਾਲੀ *ptr) RPALIAS(rpmalloc_usable_ਮਾਪ)
#ਨਹੀਂਤਾਂ
                    size_t
    malloc_usable_ਮਾਪ(ਖਾਲੀ *ptr) RPALIAS(rpmalloc_usable_ਮਾਪ)
#ਜੇ_ਅੰਤ
        size_t malloc_ਮਾਪ(ਖਾਲੀ *ptr) RPALIAS(rpmalloc_usable_ਮਾਪ)

#ਜੇ_ਅੰਤ

            ਸਥਿਰ ਸਤਰਅੰਦਰ size_t rpmalloc_ਪੰਨਾ_ਮਾਪ(ਖਾਲੀ) {
  ਵਾਪਸ ਸਮ੍ਰਿਤੀ_ਪੰਨਾ_ਮਾਪ;
}

ਬਾਹਰੀ ਖਾਲੀ *RPMALLOC_CDECL reallocarray(ਖਾਲੀ *ptr, size_t ਗਿਣਤੀ_2, size_t ਮਾਪ_2);

ਬਾਹਰੀ ਖਾਲੀ *RPMALLOC_CDECL reallocarray(ਖਾਲੀ *ptr, size_t ਗਿਣਤੀ_2, size_t ਮਾਪ_2) {
  size_t ਕੁੱਲ_2;
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
  ਪੂਰਨ err = ਮਾਪ_TMult(ਗਿਣਤੀ_2, ਮਾਪ_2, &ਕੁੱਲ_2);
  ਜੇ ((err != S_OK) || (ਕੁੱਲ_2 >= MAX_ALLOC_ਮਾਪ)) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ err = __builtin_umull_overflow(ਗਿਣਤੀ_2, ਮਾਪ_2, &ਕੁੱਲ_2);
  ਜੇ (err || (ਕੁੱਲ_2 >= MAX_ALLOC_ਮਾਪ)) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
  ਕੁੱਲ_2 = ਗਿਣਤੀ_2 * ਮਾਪ_2;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ realloc(ptr, ਕੁੱਲ_2);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ *RPMALLOC_CDECL valloc(size_t ਮਾਪ_2) {
  get_thread_ਢੇਰ();
  ਵਾਪਸ rpaligned_alloc(rpmalloc_ਪੰਨਾ_ਮਾਪ(), ਮਾਪ_2);
}

ਬਾਹਰੀ ਸਤਰਅੰਦਰ ਖਾਲੀ *RPMALLOC_CDECL pvalloc(size_t ਮਾਪ_2) {
  get_thread_ਢੇਰ();
  ਅਚਲ size_t ਪੰਨਾ_ਮਾਪ_2 = rpmalloc_ਪੰਨਾ_ਮਾਪ();
  ਅਚਲ size_t aligned_ਮਾਪ = ((ਮਾਪ_2 + ਪੰਨਾ_ਮਾਪ_2 - 1) / ਪੰਨਾ_ਮਾਪ_2) * ਪੰਨਾ_ਮਾਪ_2;
#ਜੇ ENABLE_VALIDATE_ਦਲੀਲਾਂ
  ਜੇ (aligned_ਮਾਪ < ਮਾਪ_2) {
    errno = EINVAL;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ rpaligned_alloc(rpmalloc_ਪੰਨਾ_ਮਾਪ(), aligned_ਮਾਪ);
}

#ਜੇ_ਅੰਤ // ENABLE_OVERRIDE

#ਜੇ ENABLE_PRELOAD

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32

#ਜੇ defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

ਬਾਹਰੀ __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

ਬਾਹਰੀ __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (ਖਾਲੀ)ਆਕਾਰ(reserved);
  (ਖਾਲੀ)ਆਕਾਰ(instance);
  ਜੇ (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  ਨਹੀਂਤਾਂ ਜੇ (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  ਨਹੀਂਤਾਂ ਜੇ (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  ਨਹੀਂਤਾਂ ਜੇ (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  ਵਾਪਸ TRUE;
}

// end BUILD_DYNAMIC_LINK
#ਨਹੀਂਤਾਂ

ਬਾਹਰੀ ਖਾਲੀ _global_rpmalloc_init(ਖਾਲੀ) {
  rpmalloc_ਸੈੱਟ_main_thread();
  rpmalloc_initialize();
}

#ਜੇ defined(__clang__) || defined(__GNUC__)

ਸਥਿਰ ਖਾਲੀ __attribute__((constructor)) initializer(ਖਾਲੀ) {
  _global_rpmalloc_init();
}

#ਜਾਂਜੇ defined(_MSC_VER)

ਸਥਿਰ ਪੂਰਨ _global_rpmalloc_xib(ਖਾਲੀ) {
  _global_rpmalloc_init();
  ਵਾਪਸ 0;
}

#ਹਦਾਇਤ section(".CRT$XIB", ਪਠ)
__declspec(allocate(".CRT$XIB")) ਖਾਲੀ (*_rpmalloc_module_init)(ਖਾਲੀ) =
    _global_rpmalloc_xib;
#ਜੇ defined(_M_IX86) || defined(__i386__)
#ਹਦਾਇਤ comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#ਨਹੀਂਤਾਂ
#ਹਦਾਇਤ comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ

// end !BUILD_DYNAMIC_LINK
#ਜੇ_ਅੰਤ

#ਨਹੀਂਤਾਂ

#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

ਬਾਹਰੀ ਖਾਲੀ rpmalloc_ਸੈੱਟ_main_thread(ਖਾਲੀ);

ਸਥਿਰ pthread_key_t destructor_ਕੁੰਜੀ;

ਸਥਿਰ ਖਾਲੀ thread_destructor(ਖਾਲੀ *);

ਸਥਿਰ ਖਾਲੀ __attribute__((constructor)) initializer(ਖਾਲੀ) {
  rpmalloc_ਸੈੱਟ_main_thread();
  rpmalloc_initialize();
  pthread_ਕੁੰਜੀ_create(&destructor_ਕੁੰਜੀ, thread_destructor);
}

ਸਥਿਰ ਖਾਲੀ __attribute__((destructor)) finalizer(ਖਾਲੀ) { rpmalloc_finalize(); }

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  ਖਾਲੀ *(*real_ਸ਼ੁਰੂ)(ਖਾਲੀ *);
  ਖਾਲੀ *real_arg;
} thread_starter_arg;

ਸਥਿਰ ਖਾਲੀ *thread_starter(ਖਾਲੀ *argptr) {
  thread_starter_arg *arg = argptr;
  ਖਾਲੀ *(*real_ਸ਼ੁਰੂ)(ਖਾਲੀ *) = arg->real_ਸ਼ੁਰੂ;
  ਖਾਲੀ *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_ਕੁੰਜੀ, (ਖਾਲੀ *)1);
  ਵਾਪਸ (*real_ਸ਼ੁਰੂ)(real_arg);
}

ਸਥਿਰ ਖਾਲੀ thread_destructor(ਖਾਲੀ *ਮੁੱਲ_2) {
  (ਖਾਲੀ)ਆਕਾਰ(ਮੁੱਲ_2);
  rpmalloc_thread_finalize(1);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__

ਸਥਿਰ ਪੂਰਨ pthread_create_proxy(pthread_t *thread, ਅਚਲ pthread_attr_t *attr,
                                ਖਾਲੀ *(*ਸ਼ੁਰੂ_routine)(ਖਾਲੀ *), ਖਾਲੀ *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(ਆਕਾਰ(thread_starter_arg));
  starter_arg->real_ਸ਼ੁਰੂ = ਸ਼ੁਰੂ_routine;
  starter_arg->real_arg = arg;
  ਵਾਪਸ pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#ਨਹੀਂਤਾਂ

#ਅਨ੍ਤਰ੍ਭਾਵ <dlfcn.h>

ਪੂਰਨ pthread_create(pthread_t *thread, ਅਚਲ pthread_attr_t *attr,
                   ਖਾਲੀ *(*ਸ਼ੁਰੂ_routine)(ਖਾਲੀ *), ਖਾਲੀ *arg) {
#ਜੇ defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  ਅੱਖਰ fname[] = "pthread_create";
#ਨਹੀਂਤਾਂ
  ਅੱਖਰ fname[] = "_pthread_create";
#ਜੇ_ਅੰਤ
  ਖਾਲੀ *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(ਆਕਾਰ(thread_starter_arg));
  starter_arg->real_ਸ਼ੁਰੂ = ਸ਼ੁਰੂ_routine;
  starter_arg->real_arg = arg;
  ਵਾਪਸ (*(ਪੂਰਨ (*)(pthread_t *, ਅਚਲ pthread_attr_t *, ਖਾਲੀ *(*)(ਖਾਲੀ *),
                    ਖਾਲੀ *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_OVERRIDE

#ਜੇ defined(__GLIBC__) && defined(__linux__)

ਖਾਲੀ *__libc_malloc(size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(1)
        RPALIAS(rpmalloc) ਖਾਲੀ *__libc_calloc(size_t ਗਿਣਤੀ_2, size_t ਮਾਪ_2)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) ਖਾਲੀ *__libc_realloc(ਖਾਲੀ *p, size_t ਮਾਪ_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(2) RPALIAS(rprealloc) ਖਾਲੀ __libc_free(ਖਾਲੀ *p)
        RPALIAS(rpfree) ਖਾਲੀ __libc_cfree(ਖਾਲੀ *p)
            RPALIAS(rpfree) ਖਾਲੀ *__libc_memalign(size_t align, size_t ਮਾਪ_2)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(2)
        RPALIAS(rpmemalign) ਪੂਰਨ __posix_memalign(ਖਾਲੀ **p, size_t align,
                                                 size_t ਮਾਪ_2)
            RPALIAS(rpposix_memalign)

                ਬਾਹਰੀ ਖਾਲੀ *__libc_valloc(size_t ਮਾਪ_2);
ਬਾਹਰੀ ਖਾਲੀ *__libc_pvalloc(size_t ਮਾਪ_2);

ਖਾਲੀ *__libc_valloc(size_t ਮਾਪ_2) { ਵਾਪਸ valloc(ਮਾਪ_2); }

ਖਾਲੀ *__libc_pvalloc(size_t ਮਾਪ_2) { ਵਾਪਸ pvalloc(ਮਾਪ_2); }

#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ

#ਜੇ (defined(__GNUC__) || defined(__clang__))
#ਹਦਾਇਤ GCC visibility ਕੱਢੋ
#ਜੇ_ਅੰਤ
