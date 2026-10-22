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

#ಯದಿ defined(__TINYC__)
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#endif

#ifndef ARCH_64BIT
#ಯದಿ defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#define ARCH_64BIT 1
_Static_assert(ಪರಿಮಾಣ(size_t) == 8, "Data type size mismatch");
_Static_assert(ಪರಿಮಾಣ(ಶೂನ್ಯ *) == 8, "Data type size mismatch");
#ಅನ್ಯಥಾ
#define ARCH_64BIT 0
_Static_assert(ಪರಿಮಾಣ(size_t) == 4, "Data type size mismatch");
_Static_assert(ಪರಿಮಾಣ(ಶೂನ್ಯ *) == 4, "Data type size mismatch");
#endif
#endif

#ಯದಿ (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility ತಳ್ಳು(ಸ್ವಭಾವ)
#endif

#define USE_IMPLEMENT 1
#define USE_INTERPOSE 0
#define USE_ALIAS 0

#ಯದಿ defined(__APPLE__)
#undef USE_INTERPOSE
#define USE_INTERPOSE 1

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ interpose_t {
  ಶೂನ್ಯ *new_func;
  ಶೂನ್ಯ *orig_func;
} interpose_t;

#define MAC_INTERPOSE_PAIR(newf, oldf) {(ಶೂನ್ಯ *)newf, (ಶೂನ್ಯ *)oldf}
#define MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((used)) ಸ್ಥಿರ ಧ್ರುವ interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#endif

#ಯದಿ !defined(_WIN32) && !defined(__APPLE__)
#undef USE_IMPLEMENT
#undef USE_ALIAS
#define USE_IMPLEMENT 0
#define USE_ALIAS 1
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#undef malloc
#undef ಮುಕ್ತ
#undef calloc
#define RPMALLOC_RESTRICT __declspec(restrict)
#ಅನ್ಯಥಾ
#define RPMALLOC_RESTRICT
#endif

#ಯದಿ ENABLE_OVERRIDE

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ rp_nothrow_t {
  ಪೂರ್ಣಾಙ್ಕ __dummy;
} rp_nothrow_t;

#ಯದಿ USE_IMPLEMENT

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_RESTRICT ಶೂನ್ಯ *RPMALLOC_CDECL malloc(size_t size) {
  ಪ್ರತ್ಯಾಗಮ rpmalloc(size);
}
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_RESTRICT ಶೂನ್ಯ *RPMALLOC_CDECL calloc(size_t ಎಣಿಕೆ,
                                                            size_t size) {
  ಪ್ರತ್ಯಾಗಮ rpcalloc(ಎಣಿಕೆ, size);
}
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_RESTRICT ಶೂನ್ಯ *RPMALLOC_CDECL realloc(ಶೂನ್ಯ *ptr,
                                                             size_t size) {
  ಪ್ರತ್ಯಾಗಮ rprealloc(ptr, size);
}
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *RPMALLOC_CDECL reallocf(ಶೂನ್ಯ *ptr, size_t size) {
  ಪ್ರತ್ಯಾಗಮ rprealloc(ptr, size);
}
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t size) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(alignment, size);
}
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *RPMALLOC_CDECL memalign(size_t alignment, size_t size) {
  ಪ್ರತ್ಯಾಗಮ rpmemalign(alignment, size);
}
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ RPMALLOC_CDECL posix_memalign(ಶೂನ್ಯ **memptr, size_t alignment,
                                                size_t size) {
  ಪ್ರತ್ಯಾಗಮ rpposix_memalign(memptr, alignment, size);
}
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ RPMALLOC_CDECL ಮುಕ್ತ(ಶೂನ್ಯ *ptr) { rpfree(ptr); }
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ RPMALLOC_CDECL cfree(ಶೂನ್ಯ *ptr) { rpfree(ptr); }
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ size_t RPMALLOC_CDECL malloc_usable_size(ಶೂನ್ಯ *ptr) {
  ಪ್ರತ್ಯಾಗಮ rpmalloc_usable_size(ptr);
}
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ size_t RPMALLOC_CDECL malloc_size(ಶೂನ್ಯ *ptr) {
  ಪ್ರತ್ಯಾಗಮ rpmalloc_usable_size(ptr);
}

#ifdef _WIN32
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_RESTRICT ಶೂನ್ಯ *RPMALLOC_CDECL _malloc_base(size_t size) {
  ಪ್ರತ್ಯಾಗಮ rpmalloc(size);
}
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ RPMALLOC_CDECL _free_base(ಶೂನ್ಯ *ptr) { rpfree(ptr); }
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_RESTRICT ಶೂನ್ಯ *RPMALLOC_CDECL _calloc_base(size_t ಎಣಿಕೆ,
                                                                  size_t size) {
  ಪ್ರತ್ಯಾಗಮ rpcalloc(ಎಣಿಕೆ, size);
}
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ size_t RPMALLOC_CDECL _msize(ಶೂನ್ಯ *ptr) {
  ಪ್ರತ್ಯಾಗಮ rpmalloc_usable_size(ptr);
}
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ size_t RPMALLOC_CDECL _msize_base(ಶೂನ್ಯ *ptr) {
  ಪ್ರತ್ಯಾಗಮ rpmalloc_usable_size(ptr);
}
ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ RPMALLOC_RESTRICT ಶೂನ್ಯ *RPMALLOC_CDECL
_realloc_base(ಶೂನ್ಯ *ptr, size_t size) {
  ಪ್ರತ್ಯಾಗಮ rprealloc(ptr, size);
}
#endif

#ifdef _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#ಅನ್ಯಥಾ
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#define RPDEFVIS __attribute__((visibility("default")))
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPv(ಶೂನ್ಯ *p);
ಶೂನ್ಯ RPDEFVIS _ZdlPv(ಶೂನ್ಯ *p) { rpfree(p); }
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPv(ಶೂನ್ಯ *p);
ಶೂನ್ಯ RPDEFVIS _ZdaPv(ಶೂನ್ಯ *p) { rpfree(p); }
#ಯದಿ ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znwm(uint64_t size);
ಶೂನ್ಯ *RPDEFVIS _Znwm(uint64_t size) { ಪ್ರತ್ಯಾಗಮ rpmalloc(size); }
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znam(uint64_t size);
ಶೂನ್ಯ *RPDEFVIS _Znam(uint64_t size) { ಪ್ರತ್ಯಾಗಮ rpmalloc(size); }
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znwmm(uint64_t size, uint64_t align);
ಶೂನ್ಯ *RPDEFVIS _Znwmm(uint64_t size, uint64_t align) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znamm(uint64_t size, uint64_t align);
ಶೂನ್ಯ *RPDEFVIS _Znamm(uint64_t size, uint64_t align) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnwmSt11align_val_t(uint64_t size, uint64_t align);
ಶೂನ್ಯ *RPDEFVIS _ZnwmSt11align_val_t(uint64_t size, uint64_t align) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnamSt11align_val_t(uint64_t size, uint64_t align);
ಶೂನ್ಯ *RPDEFVIS _ZnamSt11align_val_t(uint64_t size, uint64_t align) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(t);
  ಪ್ರತ್ಯಾಗಮ rpmalloc(size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(t);
  ಪ್ರತ್ಯಾಗಮ rpmalloc(size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(t);
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(t);
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
// 64-bit operators sized delete and delete[], normal and aligned
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPvm(ಶೂನ್ಯ *p, uint64_t size);
ಶೂನ್ಯ RPDEFVIS _ZdlPvm(ಶೂನ್ಯ *p, uint64_t size) {
  rpfree(p);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPvm(ಶೂನ್ಯ *p, uint64_t size);
ಶೂನ್ಯ RPDEFVIS _ZdaPvm(ಶೂನ್ಯ *p, uint64_t size) {
  rpfree(p);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPvSt11align_val_t(ಶೂನ್ಯ *p, uint64_t align);
ಶೂನ್ಯ RPDEFVIS _ZdlPvSt11align_val_t(ಶೂನ್ಯ *p, uint64_t align) {
  rpfree(p);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(align);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPvSt11align_val_t(ಶೂನ್ಯ *p, uint64_t align);
ಶೂನ್ಯ RPDEFVIS _ZdaPvSt11align_val_t(ಶೂನ್ಯ *p, uint64_t align) {
  rpfree(p);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(align);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPvmSt11align_val_t(ಶೂನ್ಯ *p, uint64_t size, uint64_t align);
ಶೂನ್ಯ RPDEFVIS _ZdlPvmSt11align_val_t(ಶೂನ್ಯ *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(align);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPvmSt11align_val_t(ಶೂನ್ಯ *p, uint64_t size, uint64_t align);
ಶೂನ್ಯ RPDEFVIS _ZdaPvmSt11align_val_t(ಶೂನ್ಯ *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(align);
}
#ಅನ್ಯಥಾ
// 32-bit operators new and new[], normal and aligned
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znwj(uint32_t size);
ಶೂನ್ಯ *RPDEFVIS _Znwj(uint32_t size) { ಪ್ರತ್ಯಾಗಮ rpmalloc(size); }
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znaj(uint32_t size);
ಶೂನ್ಯ *RPDEFVIS _Znaj(uint32_t size) { ಪ್ರತ್ಯಾಗಮ rpmalloc(size); }
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znwjj(uint32_t size, uint32_t align);
ಶೂನ್ಯ *RPDEFVIS _Znwjj(uint32_t size, uint32_t align) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znajj(uint32_t size, uint32_t align);
ಶೂನ್ಯ *RPDEFVIS _Znajj(uint32_t size, uint32_t align) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnwjSt11align_val_t(size_t size, size_t align);
ಶೂನ್ಯ *RPDEFVIS _ZnwjSt11align_val_t(size_t size, size_t align) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnajSt11align_val_t(size_t size, size_t align);
ಶೂನ್ಯ *RPDEFVIS _ZnajSt11align_val_t(size_t size, size_t align) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(t);
  ಪ್ರತ್ಯಾಗಮ rpmalloc(size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(t);
  ಪ್ರತ್ಯಾಗಮ rpmalloc(size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(t);
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(t);
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
// 32-bit operators sized delete and delete[], normal and aligned
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPvj(ಶೂನ್ಯ *p, uint64_t size);
ಶೂನ್ಯ RPDEFVIS _ZdlPvj(ಶೂನ್ಯ *p, uint64_t size) {
  rpfree(p);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPvj(ಶೂನ್ಯ *p, uint64_t size);
ಶೂನ್ಯ RPDEFVIS _ZdaPvj(ಶೂನ್ಯ *p, uint64_t size) {
  rpfree(p);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPvSt11align_val_t(ಶೂನ್ಯ *p, uint32_t align);
ಶೂನ್ಯ RPDEFVIS _ZdlPvSt11align_val_t(ಶೂನ್ಯ *p, uint64_t a) {
  rpfree(p);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(align);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPvSt11align_val_t(ಶೂನ್ಯ *p, uint32_t align);
ಶೂನ್ಯ RPDEFVIS _ZdaPvSt11align_val_t(ಶೂನ್ಯ *p, uint64_t a) {
  rpfree(p);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(align);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPvjSt11align_val_t(ಶೂನ್ಯ *p, uint32_t size, uint32_t align);
ಶೂನ್ಯ RPDEFVIS _ZdlPvjSt11align_val_t(ಶೂನ್ಯ *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(a);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPvjSt11align_val_t(ಶೂನ್ಯ *p, uint32_t size, uint32_t align);
ಶೂನ್ಯ RPDEFVIS _ZdaPvjSt11align_val_t(ಶೂನ್ಯ *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(a);
}
#endif
#endif
#endif

#ಯದಿ USE_INTERPOSE || USE_ALIAS

ಸ್ಥಿರ ಶೂನ್ಯ *rpmalloc_nothrow(size_t size, rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(t);
  ಪ್ರತ್ಯಾಗಮ rpmalloc(size);
}
ಸ್ಥಿರ ಶೂನ್ಯ *rpaligned_alloc_reverse(size_t size, size_t align) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
ಸ್ಥಿರ ಶೂನ್ಯ *rpaligned_alloc_reverse_nothrow(size_t size, size_t align,
                                             rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(t);
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(align, size);
}
ಸ್ಥಿರ ಶೂನ್ಯ rpfree_size(ಶೂನ್ಯ *p, size_t size) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
  rpfree(p);
}
ಸ್ಥಿರ ಶೂನ್ಯ rpfree_aligned(ಶೂನ್ಯ *p, size_t align) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(align);
  rpfree(p);
}
ಸ್ಥಿರ ಶೂನ್ಯ rpfree_size_aligned(ಶೂನ್ಯ *p, size_t size, size_t align) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(align);
  rpfree(p);
}

#endif

#ಯದಿ USE_INTERPOSE

__attribute__((used)) ಸ್ಥಿರ ಧ್ರುವ interpose_t macinterpose_malloc[]
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
#ಯದಿ defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#endif
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, ಮುಕ್ತ), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_usable_size),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_size)};

#endif

#ಯದಿ USE_ALIAS

#define RPALIAS(fn) __attribute__((alias(#fn), used, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
ಶೂನ್ಯ _ZdlPv(ಶೂನ್ಯ *p) RPALIAS(rpfree) ಶೂನ್ಯ _ZdaPv(ಶೂನ್ಯ *p) RPALIAS(rpfree)

#ಯದಿ ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    ಶೂನ್ಯ *_Znwm(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) ಶೂನ್ಯ *_Znam(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) ಶೂನ್ಯ *_Znwmm(uint64_t size,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) ಶೂನ್ಯ *_Znamm(uint64_t size,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) ಶೂನ್ಯ *_ZnwmSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) ಶೂನ್ಯ *_ZnamSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) ಶೂನ್ಯ *_ZnwmRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) ಶೂನ್ಯ *_ZnamRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) ಶೂನ್ಯ
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) ಶೂನ್ಯ
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    ಶೂನ್ಯ _ZdlPvm(ಶೂನ್ಯ *p, size_t n) RPALIAS(rpfree_size) ಶೂನ್ಯ _ZdaPvm(ಶೂನ್ಯ *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) ಶೂನ್ಯ _ZdlPvSt11align_val_t(ಶೂನ್ಯ *p, size_t a)
            RPALIAS(rpfree_aligned) ಶೂನ್ಯ _ZdaPvSt11align_val_t(ಶೂನ್ಯ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) ಶೂನ್ಯ _ZdlPvmSt11align_val_t(ಶೂನ್ಯ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) ಶೂನ್ಯ _ZdaPvmSt11align_val_t(
                        ಶೂನ್ಯ *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#ಅನ್ಯಥಾ
    // 32-bit operators new and new[], normal and aligned
    ಶೂನ್ಯ *_Znwj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) ಶೂನ್ಯ *_Znaj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) ಶೂನ್ಯ *_Znwjj(uint32_t size,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) ಶೂನ್ಯ *_Znajj(uint32_t size,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) ಶೂನ್ಯ *_ZnwjSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) ಶೂನ್ಯ *_ZnajSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) ಶೂನ್ಯ *_ZnwjRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) ಶೂನ್ಯ *_ZnajRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) ಶೂನ್ಯ
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) ಶೂನ್ಯ
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    ಶೂನ್ಯ _ZdlPvj(ಶೂನ್ಯ *p, size_t n) RPALIAS(rpfree_size) ಶೂನ್ಯ _ZdaPvj(ಶೂನ್ಯ *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) ಶೂನ್ಯ _ZdlPvSt11align_val_t(ಶೂನ್ಯ *p, size_t a)
            RPALIAS(rpfree_aligned) ಶೂನ್ಯ _ZdaPvSt11align_val_t(ಶೂನ್ಯ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) ಶೂನ್ಯ _ZdlPvjSt11align_val_t(ಶೂನ್ಯ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) ಶೂನ್ಯ _ZdaPvjSt11align_val_t(
                        ಶೂನ್ಯ *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#endif

                            ಶೂನ್ಯ *malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) ಶೂನ್ಯ *calloc(size_t ಎಣಿಕೆ, size_t size)
            RPALIAS(rpcalloc) ಶೂನ್ಯ *realloc(ಶೂನ್ಯ *ptr, size_t size)
                RPALIAS(rprealloc) ಶೂನ್ಯ *reallocf(ಶೂನ್ಯ *ptr, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rprealloc) ಶೂನ್ಯ *aligned_alloc(size_t alignment, size_t size)
            RPALIAS(rpaligned_alloc) ಶೂನ್ಯ *memalign(
                size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) ಪೂರ್ಣಾಙ್ಕ posix_memalign(ಶೂನ್ಯ **memptr, size_t alignment,
                                               size_t size)
            RPALIAS(rpposix_memalign) ಶೂನ್ಯ ಮುಕ್ತ(ಶೂನ್ಯ *ptr)
                RPALIAS(rpfree) ಶೂನ್ಯ cfree(ಶೂನ್ಯ *ptr) RPALIAS(rpfree)
#ಯದಿ defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_size(ಧ್ರುವ ಶೂನ್ಯ *ptr) RPALIAS(rpmalloc_usable_size)
#ಅನ್ಯಥಾ
                    size_t
    malloc_usable_size(ಶೂನ್ಯ *ptr) RPALIAS(rpmalloc_usable_size)
#endif
        size_t malloc_size(ಶೂನ್ಯ *ptr) RPALIAS(rpmalloc_usable_size)

#endif

            ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ size_t _rpmalloc_page_size(ಶೂನ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ _memory_page_size;
}

ಬಾಹ್ಯ ಶೂನ್ಯ *RPMALLOC_CDECL reallocarray(ಶೂನ್ಯ *ptr, size_t ಎಣಿಕೆ, size_t size);

ಬಾಹ್ಯ ಶೂನ್ಯ *RPMALLOC_CDECL reallocarray(ಶೂನ್ಯ *ptr, size_t ಎಣಿಕೆ, size_t size) {
  size_t total;
#ಯದಿ ENABLE_VALIDATE_ARGS
#ifdef _MSC_VER
  ಪೂರ್ಣಾಙ್ಕ err = SizeTMult(ಎಣಿಕೆ, size, &total);
  ಯದಿ ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#ಅನ್ಯಥಾ
  ಪೂರ್ಣಾಙ್ಕ err = __builtin_umull_overflow(ಎಣಿಕೆ, size, &total);
  ಯದಿ (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
#ಅನ್ಯಥಾ
  total = ಎಣಿಕೆ * size;
#endif
  ಪ್ರತ್ಯಾಗಮ realloc(ptr, total);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *RPMALLOC_CDECL valloc(size_t size) {
  get_thread_heap();
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(_rpmalloc_page_size(), size);
}

ಬಾಹ್ಯ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *RPMALLOC_CDECL pvalloc(size_t size) {
  get_thread_heap();
  ಧ್ರುವ size_t page_size = _rpmalloc_page_size();
  ಧ್ರುವ size_t aligned_size = ((size + page_size - 1) / page_size) * page_size;
#ಯದಿ ENABLE_VALIDATE_ARGS
  ಯದಿ (aligned_size < size) {
    errno = EINVAL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(_rpmalloc_page_size(), aligned_size);
}

#endif // ENABLE_OVERRIDE

#ಯದಿ ENABLE_PRELOAD

#ifdef _WIN32

#ಯದಿ defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

ಬಾಹ್ಯ __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

ಬಾಹ್ಯ __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(reserved);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(instance);
  ಯದಿ (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  ಅನ್ಯಥಾ ಯದಿ (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  ಅನ್ಯಥಾ ಯದಿ (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  ಅನ್ಯಥಾ ಯದಿ (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  ಪ್ರತ್ಯಾಗಮ TRUE;
}

// end BUILD_DYNAMIC_LINK
#ಅನ್ಯಥಾ

ಬಾಹ್ಯ ಶೂನ್ಯ _global_rpmalloc_init(ಶೂನ್ಯ) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
}

#ಯದಿ defined(__clang__) || defined(__GNUC__)

ಸ್ಥಿರ ಶೂನ್ಯ __attribute__((constructor)) initializer(ಶೂನ್ಯ) {
  _global_rpmalloc_init();
}

#ನೋಚೇದ್ಯದಿ defined(_MSC_VER)

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ _global_rpmalloc_xib(ಶೂನ್ಯ) {
  _global_rpmalloc_init();
  ಪ್ರತ್ಯಾಗಮ 0;
}

#pragma section(".CRT$XIB", ಪಠ)
__declspec(allocate(".CRT$XIB")) ಶೂನ್ಯ (*_rpmalloc_module_init)(ಶೂನ್ಯ) =
    _global_rpmalloc_xib;
#ಯದಿ defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#ಅನ್ಯಥಾ
#pragma comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#endif

#endif

// end !BUILD_DYNAMIC_LINK
#endif

#ಅನ್ಯಥಾ

#ಅನ್ತರ್ಭಾವ <pthread.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

ಬಾಹ್ಯ ಶೂನ್ಯ rpmalloc_set_main_thread(ಶೂನ್ಯ);

ಸ್ಥಿರ pthread_key_t destructor_ಕೀಲಿ;

ಸ್ಥಿರ ಶೂನ್ಯ thread_destructor(ಶೂನ್ಯ *);

ಸ್ಥಿರ ಶೂನ್ಯ __attribute__((constructor)) initializer(ಶೂನ್ಯ) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
  pthread_ಕೀಲಿ_create(&destructor_ಕೀಲಿ, thread_destructor);
}

ಸ್ಥಿರ ಶೂನ್ಯ __attribute__((destructor)) finalizer(ಶೂನ್ಯ) { rpmalloc_finalize(); }

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
  ಶೂನ್ಯ *(*real_start)(ಶೂನ್ಯ *);
  ಶೂನ್ಯ *real_arg;
} thread_starter_arg;

ಸ್ಥಿರ ಶೂನ್ಯ *thread_starter(ಶೂನ್ಯ *argptr) {
  thread_starter_arg *arg = argptr;
  ಶೂನ್ಯ *(*real_start)(ಶೂನ್ಯ *) = arg->real_start;
  ಶೂನ್ಯ *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_ಕೀಲಿ, (ಶೂನ್ಯ *)1);
  ಪ್ರತ್ಯಾಗಮ (*real_start)(real_arg);
}

ಸ್ಥಿರ ಶೂನ್ಯ thread_destructor(ಶೂನ್ಯ *ಮೌಲ್ಯ_2) {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(ಮೌಲ್ಯ_2);
  rpmalloc_thread_finalize(1);
}

#ifdef __APPLE__

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pthread_create_proxy(pthread_t *thread, ಧ್ರುವ pthread_attr_t *attr,
                                ಶೂನ್ಯ *(*start_routine)(ಶೂನ್ಯ *), ಶೂನ್ಯ *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(ಪರಿಮಾಣ(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  ಪ್ರತ್ಯಾಗಮ pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#ಅನ್ಯಥಾ

#ಅನ್ತರ್ಭಾವ <dlfcn.h>

ಪೂರ್ಣಾಙ್ಕ pthread_create(pthread_t *thread, ಧ್ರುವ pthread_attr_t *attr,
                   ಶೂನ್ಯ *(*start_routine)(ಶೂನ್ಯ *), ಶೂನ್ಯ *arg) {
#ಯದಿ defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  ವರ್ಣ fname[] = "pthread_create";
#ಅನ್ಯಥಾ
  ವರ್ಣ fname[] = "_pthread_create";
#endif
  ಶೂನ್ಯ *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(ಪರಿಮಾಣ(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  ಪ್ರತ್ಯಾಗಮ (*(ಪೂರ್ಣಾಙ್ಕ (*)(pthread_t *, ಧ್ರುವ pthread_attr_t *, ಶೂನ್ಯ *(*)(ಶೂನ್ಯ *),
                    ಶೂನ್ಯ *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#endif

#endif

#endif

#ಯದಿ ENABLE_OVERRIDE

#ಯದಿ defined(__GLIBC__) && defined(__linux__)

ಶೂನ್ಯ *__libc_malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) ಶೂನ್ಯ *__libc_calloc(size_t ಎಣಿಕೆ, size_t size)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) ಶೂನ್ಯ *__libc_realloc(ಶೂನ್ಯ *p, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2) RPALIAS(rprealloc) ಶೂನ್ಯ __libc_free(ಶೂನ್ಯ *p)
        RPALIAS(rpfree) ಶೂನ್ಯ __libc_cfree(ಶೂನ್ಯ *p)
            RPALIAS(rpfree) ಶೂನ್ಯ *__libc_memalign(size_t align, size_t size)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) ಪೂರ್ಣಾಙ್ಕ __posix_memalign(ಶೂನ್ಯ **p, size_t align,
                                                 size_t size)
            RPALIAS(rpposix_memalign)

                ಬಾಹ್ಯ ಶೂನ್ಯ *__libc_valloc(size_t size);
ಬಾಹ್ಯ ಶೂನ್ಯ *__libc_pvalloc(size_t size);

ಶೂನ್ಯ *__libc_valloc(size_t size) { ಪ್ರತ್ಯಾಗಮ valloc(size); }

ಶೂನ್ಯ *__libc_pvalloc(size_t size) { ಪ್ರತ್ಯಾಗಮ pvalloc(size); }

#endif

#endif

#ಯದಿ (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility ತೆಗೆ
#endif
