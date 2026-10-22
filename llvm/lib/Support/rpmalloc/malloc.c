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

#ಆದರೆ defined(__TINYC__)
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ARCH_64_ಬಿಟ್
#ಆದರೆ defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#ವ್ಯಾಖ್ಯಾನಿಸು ARCH_64_ಬಿಟ್ 1
_Static_assert(ಗಾತ್ರ(size_t) == 8, "Data type size mismatch");
_Static_assert(ಗಾತ್ರ(ಶೂನ್ಯ *) == 8, "Data type size mismatch");
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು ARCH_64_ಬಿಟ್ 0
_Static_assert(ಗಾತ್ರ(size_t) == 4, "Data type size mismatch");
_Static_assert(ಗಾತ್ರ(ಶೂನ್ಯ *) == 4, "Data type size mismatch");
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ (defined(__GNUC__) || defined(__clang__))
#ಸೂಚನೆ GCC visibility ತಳ್ಳು(ಪೂರ್ವನಿಯೋಜಿತ)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು USE_IMPLEMENT 1
#ವ್ಯಾಖ್ಯಾನಿಸು USE_INTERPOSE 0
#ವ್ಯಾಖ್ಯಾನಿಸು USE_ALIAS 0

#ಆದರೆ defined(__APPLE__)
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು USE_INTERPOSE
#ವ್ಯಾಖ್ಯಾನಿಸು USE_INTERPOSE 1

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ interpose_t {
  ಶೂನ್ಯ *new_func;
  ಶೂನ್ಯ *orig_func;
} interpose_t;

#ವ್ಯಾಖ್ಯಾನಿಸು MAC_INTERPOSE_PAIR(newf, oldf) {(ಶೂನ್ಯ *)newf, (ಶೂನ್ಯ *)oldf}
#ವ್ಯಾಖ್ಯಾನಿಸು MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((ಬಳಸಿದ)) ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(_WIN32) && !defined(__APPLE__)
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು USE_IMPLEMENT
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು USE_ALIAS
#ವ್ಯಾಖ್ಯಾನಿಸು USE_IMPLEMENT 0
#ವ್ಯಾಖ್ಯಾನಿಸು USE_ALIAS 1
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
#ಸೂಚನೆ warning(disable : 4100)
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಸ್ಮರಣೆ_ಕೊಡು
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಬಿಡುಗಡೆ
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು
#ವ್ಯಾಖ್ಯಾನಿಸು RPMALLOC_RESTRICT __declspec(ಸೀಮಿತ)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು RPMALLOC_RESTRICT
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_OVERRIDE

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ rp_nothrow_t {
  ಪೂರ್ಣಾಂಕ __dummy;
} rp_nothrow_t;

#ಆದರೆ USE_IMPLEMENT

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_RESTRICT ಶೂನ್ಯ *RPMALLOC_CDECL ಸ್ಮರಣೆ_ಕೊಡು(size_t ಗಾತ್ರ_3) {
  ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_RESTRICT ಶೂನ್ಯ *RPMALLOC_CDECL ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(size_t ಎಣಿಕೆ_2,
                                                            size_t ಗಾತ್ರ_3) {
  ಹಿಂತಿರುಗಿ rpcalloc(ಎಣಿಕೆ_2, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_RESTRICT ಶೂನ್ಯ *RPMALLOC_CDECL realloc(ಶೂನ್ಯ *ptr,
                                                             size_t ಗಾತ್ರ_3) {
  ಹಿಂತಿರುಗಿ rprealloc(ptr, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *RPMALLOC_CDECL reallocf(ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_3) {
  ಹಿಂತಿರುಗಿ rprealloc(ptr, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t ಗಾತ್ರ_3) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(alignment, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *RPMALLOC_CDECL memalign(size_t alignment, size_t ಗಾತ್ರ_3) {
  ಹಿಂತಿರುಗಿ rpmemalign(alignment, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ RPMALLOC_CDECL posix_memalign(ಶೂನ್ಯ **memptr, size_t alignment,
                                                size_t ಗಾತ್ರ_3) {
  ಹಿಂತಿರುಗಿ rpposix_memalign(memptr, alignment, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ RPMALLOC_CDECL ಬಿಡುಗಡೆ(ಶೂನ್ಯ *ptr) { rpfree(ptr); }
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ RPMALLOC_CDECL cfree(ಶೂನ್ಯ *ptr) { rpfree(ptr); }
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ size_t RPMALLOC_CDECL malloc_usable_ಗಾತ್ರ(ಶೂನ್ಯ *ptr) {
  ಹಿಂತಿರುಗಿ rpmalloc_usable_ಗಾತ್ರ(ptr);
}
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ size_t RPMALLOC_CDECL malloc_ಗಾತ್ರ(ಶೂನ್ಯ *ptr) {
  ಹಿಂತಿರುಗಿ rpmalloc_usable_ಗಾತ್ರ(ptr);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_RESTRICT ಶೂನ್ಯ *RPMALLOC_CDECL malloc_ತಳ(size_t ಗಾತ್ರ_3) {
  ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ RPMALLOC_CDECL free_ತಳ(ಶೂನ್ಯ *ptr) { rpfree(ptr); }
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_RESTRICT ಶೂನ್ಯ *RPMALLOC_CDECL calloc_ತಳ(size_t ಎಣಿಕೆ_2,
                                                                  size_t ಗಾತ್ರ_3) {
  ಹಿಂತಿರುಗಿ rpcalloc(ಎಣಿಕೆ_2, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ size_t RPMALLOC_CDECL _msize(ಶೂನ್ಯ *ptr) {
  ಹಿಂತಿರುಗಿ rpmalloc_usable_ಗಾತ್ರ(ptr);
}
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ size_t RPMALLOC_CDECL msize_ತಳ(ಶೂನ್ಯ *ptr) {
  ಹಿಂತಿರುಗಿ rpmalloc_usable_ಗಾತ್ರ(ptr);
}
ಬಾಹ್ಯ ಸಾಲೊಳಗೆ RPMALLOC_RESTRICT ಶೂನ್ಯ *RPMALLOC_CDECL
realloc_ತಳ(ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_3) {
  ಹಿಂತಿರುಗಿ rprealloc(ptr, ಗಾತ್ರ_3);
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#ಇಲ್ಲದಿದ್ದರೆ
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#ವ್ಯಾಖ್ಯಾನಿಸು RPDEFVIS __attribute__((visibility("default")))
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPv(ಶೂನ್ಯ *p);
ಶೂನ್ಯ RPDEFVIS _ZdlPv(ಶೂನ್ಯ *p) { rpfree(p); }
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPv(ಶೂನ್ಯ *p);
ಶೂನ್ಯ RPDEFVIS _ZdaPv(ಶೂನ್ಯ *p) { rpfree(p); }
#ಆದರೆ ARCH_64_ಬಿಟ್
// 64-bit operators new and new[], normal and aligned
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znwm(uint64_t ಗಾತ್ರ_3);
ಶೂನ್ಯ *RPDEFVIS _Znwm(uint64_t ಗಾತ್ರ_3) { ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3); }
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znam(uint64_t ಗಾತ್ರ_3);
ಶೂನ್ಯ *RPDEFVIS _Znam(uint64_t ಗಾತ್ರ_3) { ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3); }
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znwmm(uint64_t ಗಾತ್ರ_3, uint64_t align);
ಶೂನ್ಯ *RPDEFVIS _Znwmm(uint64_t ಗಾತ್ರ_3, uint64_t align) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znamm(uint64_t ಗಾತ್ರ_3, uint64_t align);
ಶೂನ್ಯ *RPDEFVIS _Znamm(uint64_t ಗಾತ್ರ_3, uint64_t align) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnwmSt11align_val_t(uint64_t ಗಾತ್ರ_3, uint64_t align);
ಶೂನ್ಯ *RPDEFVIS _ZnwmSt11align_val_t(uint64_t ಗಾತ್ರ_3, uint64_t align) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnamSt11align_val_t(uint64_t ಗಾತ್ರ_3, uint64_t align);
ಶೂನ್ಯ *RPDEFVIS _ZnamSt11align_val_t(uint64_t ಗಾತ್ರ_3, uint64_t align) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnwmRKSt9nothrow_t(uint64_t ಗಾತ್ರ_3, rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t ಗಾತ್ರ_3, rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(t);
  ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnamRKSt9nothrow_t(uint64_t ಗಾತ್ರ_3, rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t ಗಾತ್ರ_3, rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(t);
  ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t ಗಾತ್ರ_3, uint64_t align,
                                                rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t ಗಾತ್ರ_3, uint64_t align,
                                                  rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(t);
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t ಗಾತ್ರ_3, uint64_t align,
                                                rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t ಗಾತ್ರ_3, uint64_t align,
                                                  rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(t);
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
// 64-bit operators sized delete and delete[], normal and aligned
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPvm(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3);
ಶೂನ್ಯ RPDEFVIS _ZdlPvm(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3) {
  rpfree(p);
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPvm(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3);
ಶೂನ್ಯ RPDEFVIS _ZdaPvm(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3) {
  rpfree(p);
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPvSt11align_val_t(ಶೂನ್ಯ *p, uint64_t align);
ಶೂನ್ಯ RPDEFVIS _ZdlPvSt11align_val_t(ಶೂನ್ಯ *p, uint64_t align) {
  rpfree(p);
  (ಶೂನ್ಯ)ಗಾತ್ರ(align);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPvSt11align_val_t(ಶೂನ್ಯ *p, uint64_t align);
ಶೂನ್ಯ RPDEFVIS _ZdaPvSt11align_val_t(ಶೂನ್ಯ *p, uint64_t align) {
  rpfree(p);
  (ಶೂನ್ಯ)ಗಾತ್ರ(align);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPvmSt11align_val_t(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3, uint64_t align);
ಶೂನ್ಯ RPDEFVIS _ZdlPvmSt11align_val_t(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3, uint64_t align) {
  rpfree(p);
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
  (ಶೂನ್ಯ)ಗಾತ್ರ(align);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPvmSt11align_val_t(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3, uint64_t align);
ಶೂನ್ಯ RPDEFVIS _ZdaPvmSt11align_val_t(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3, uint64_t align) {
  rpfree(p);
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
  (ಶೂನ್ಯ)ಗಾತ್ರ(align);
}
#ಇಲ್ಲದಿದ್ದರೆ
// 32-bit operators new and new[], normal and aligned
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znwj(uint32_t ಗಾತ್ರ_3);
ಶೂನ್ಯ *RPDEFVIS _Znwj(uint32_t ಗಾತ್ರ_3) { ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3); }
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znaj(uint32_t ಗಾತ್ರ_3);
ಶೂನ್ಯ *RPDEFVIS _Znaj(uint32_t ಗಾತ್ರ_3) { ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3); }
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znwjj(uint32_t ಗಾತ್ರ_3, uint32_t align);
ಶೂನ್ಯ *RPDEFVIS _Znwjj(uint32_t ಗಾತ್ರ_3, uint32_t align) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_Znajj(uint32_t ಗಾತ್ರ_3, uint32_t align);
ಶೂನ್ಯ *RPDEFVIS _Znajj(uint32_t ಗಾತ್ರ_3, uint32_t align) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnwjSt11align_val_t(size_t ಗಾತ್ರ_3, size_t align);
ಶೂನ್ಯ *RPDEFVIS _ZnwjSt11align_val_t(size_t ಗಾತ್ರ_3, size_t align) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnajSt11align_val_t(size_t ಗಾತ್ರ_3, size_t align);
ಶೂನ್ಯ *RPDEFVIS _ZnajSt11align_val_t(size_t ಗಾತ್ರ_3, size_t align) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnwjRKSt9nothrow_t(size_t ಗಾತ್ರ_3, rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t ಗಾತ್ರ_3, rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(t);
  ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnajRKSt9nothrow_t(size_t ಗಾತ್ರ_3, rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnajRKSt9nothrow_t(size_t ಗಾತ್ರ_3, rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(t);
  ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t ಗಾತ್ರ_3, size_t align,
                                                rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t ಗಾತ್ರ_3, size_t align,
                                                  rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(t);
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ *_ZnajSt11align_val_tRKSt9nothrow_t(size_t ಗಾತ್ರ_3, size_t align,
                                                rp_nothrow_t t);
ಶೂನ್ಯ *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t ಗಾತ್ರ_3, size_t align,
                                                  rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(t);
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
// 32-bit operators sized delete and delete[], normal and aligned
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPvj(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3);
ಶೂನ್ಯ RPDEFVIS _ZdlPvj(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3) {
  rpfree(p);
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPvj(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3);
ಶೂನ್ಯ RPDEFVIS _ZdaPvj(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3) {
  rpfree(p);
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPvSt11align_val_t(ಶೂನ್ಯ *p, uint32_t align);
ಶೂನ್ಯ RPDEFVIS _ZdlPvSt11align_val_t(ಶೂನ್ಯ *p, uint64_t a) {
  rpfree(p);
  (ಶೂನ್ಯ)ಗಾತ್ರ(align);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPvSt11align_val_t(ಶೂನ್ಯ *p, uint32_t align);
ಶೂನ್ಯ RPDEFVIS _ZdaPvSt11align_val_t(ಶೂನ್ಯ *p, uint64_t a) {
  rpfree(p);
  (ಶೂನ್ಯ)ಗಾತ್ರ(align);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdlPvjSt11align_val_t(ಶೂನ್ಯ *p, uint32_t ಗಾತ್ರ_3, uint32_t align);
ಶೂನ್ಯ RPDEFVIS _ZdlPvjSt11align_val_t(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3, uint64_t align) {
  rpfree(p);
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
  (ಶೂನ್ಯ)ಗಾತ್ರ(a);
}
ಬಾಹ್ಯ ಶೂನ್ಯ _ZdaPvjSt11align_val_t(ಶೂನ್ಯ *p, uint32_t ಗಾತ್ರ_3, uint32_t align);
ಶೂನ್ಯ RPDEFVIS _ZdaPvjSt11align_val_t(ಶೂನ್ಯ *p, uint64_t ಗಾತ್ರ_3, uint64_t align) {
  rpfree(p);
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
  (ಶೂನ್ಯ)ಗಾತ್ರ(a);
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ USE_INTERPOSE || USE_ALIAS

ಸ್ಥಿರ ಶೂನ್ಯ *rpmalloc_nothrow(size_t ಗಾತ್ರ_3, rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(t);
  ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3);
}
ಸ್ಥಿರ ಶೂನ್ಯ *rpaligned_alloc_ಹಿಮ್ಮುಖ(size_t ಗಾತ್ರ_3, size_t align) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
ಸ್ಥಿರ ಶೂನ್ಯ *rpaligned_alloc_ಹಿಮ್ಮುಖ_nothrow(size_t ಗಾತ್ರ_3, size_t align,
                                             rp_nothrow_t t) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(t);
  ಹಿಂತಿರುಗಿ rpaligned_alloc(align, ಗಾತ್ರ_3);
}
ಸ್ಥಿರ ಶೂನ್ಯ rpfree_ಗಾತ್ರ(ಶೂನ್ಯ *p, size_t ಗಾತ್ರ_3) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
  rpfree(p);
}
ಸ್ಥಿರ ಶೂನ್ಯ rpfree_aligned(ಶೂನ್ಯ *p, size_t align) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(align);
  rpfree(p);
}
ಸ್ಥಿರ ಶೂನ್ಯ rpfree_ಗಾತ್ರ_aligned(ಶೂನ್ಯ *p, size_t ಗಾತ್ರ_3, size_t align) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
  (ಶೂನ್ಯ)ಗಾತ್ರ(align);
  rpfree(p);
}

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ USE_INTERPOSE

__attribute__((ಬಳಸಿದ)) ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ interpose_t macinterpose_malloc[]
    __attribute__((section("__DATA, __interpose"))) = {
        // new and new[]
        MAC_INTERPOSE_PAIR(rpmalloc, _Znwm),
        MAC_INTERPOSE_PAIR(rpmalloc, _Znam),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_ಹಿಮ್ಮುಖ, _Znwmm),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_ಹಿಮ್ಮುಖ, _Znamm),
        MAC_INTERPOSE_PAIR(rpmalloc_nothrow, _ZnwmRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpmalloc_nothrow, _ZnamRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_ಹಿಮ್ಮುಖ, _ZnwmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_ಹಿಮ್ಮುಖ, _ZnamSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_ಹಿಮ್ಮುಖ_nothrow,
                           _ZnwmSt11align_val_tRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_ಹಿಮ್ಮುಖ_nothrow,
                           _ZnamSt11align_val_tRKSt9nothrow_t),
        // delete and delete[]
        MAC_INTERPOSE_PAIR(rpfree, _ZdlPv), MAC_INTERPOSE_PAIR(rpfree, _ZdaPv),
        MAC_INTERPOSE_PAIR(rpfree_ಗಾತ್ರ, _ZdlPvm),
        MAC_INTERPOSE_PAIR(rpfree_ಗಾತ್ರ, _ZdaPvm),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdlPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdaPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_ಗಾತ್ರ_aligned, _ZdlPvmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_ಗಾತ್ರ_aligned, _ZdaPvmSt11align_val_t),
        // libc entry points
        MAC_INTERPOSE_PAIR(rpmalloc, ಸ್ಮರಣೆ_ಕೊಡು),
        MAC_INTERPOSE_PAIR(rpmalloc, ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು),
        MAC_INTERPOSE_PAIR(rprealloc, realloc),
        MAC_INTERPOSE_PAIR(rprealloc, reallocf),
#ಆದರೆ defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#ಆದರೆ_ಅಂತ್ಯ
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, ಬಿಡುಗಡೆ), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_ಗಾತ್ರ, malloc_usable_ಗಾತ್ರ),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_ಗಾತ್ರ, malloc_ಗಾತ್ರ)};

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ USE_ALIAS

#ವ್ಯಾಖ್ಯಾನಿಸು RPALIAS(fn) __attribute__((alias(#fn), ಬಳಸಿದ, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
ಶೂನ್ಯ _ZdlPv(ಶೂನ್ಯ *p) RPALIAS(rpfree) ಶೂನ್ಯ _ZdaPv(ಶೂನ್ಯ *p) RPALIAS(rpfree)

#ಆದರೆ ARCH_64_ಬಿಟ್
    // 64-bit operators new and new[], normal and aligned
    ಶೂನ್ಯ *_Znwm(uint64_t ಗಾತ್ರ_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ಗಾತ್ರ(1)
        RPALIAS(rpmalloc) ಶೂನ್ಯ *_Znam(uint64_t ಗಾತ್ರ_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ಗಾತ್ರ(1) RPALIAS(rpmalloc) ಶೂನ್ಯ *_Znwmm(uint64_t ಗಾತ್ರ_3,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_ಹಿಮ್ಮುಖ) ಶೂನ್ಯ *_Znamm(uint64_t ಗಾತ್ರ_3,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_ಹಿಮ್ಮುಖ) ಶೂನ್ಯ *_ZnwmSt11align_val_t(
                size_t ಗಾತ್ರ_3, size_t align)
                RPALIAS(rpaligned_alloc_ಹಿಮ್ಮುಖ) ಶೂನ್ಯ *_ZnamSt11align_val_t(
                    size_t ಗಾತ್ರ_3, size_t align)
                    RPALIAS(rpaligned_alloc_ಹಿಮ್ಮುಖ) ಶೂನ್ಯ *_ZnwmRKSt9nothrow_t(
                        size_t ಗಾತ್ರ_3, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) ಶೂನ್ಯ *_ZnamRKSt9nothrow_t(
                            size_t ಗಾತ್ರ_3,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) ಶೂನ್ಯ
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t ಗಾತ್ರ_3,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_ಹಿಮ್ಮುಖ_nothrow) ಶೂನ್ಯ
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t ಗಾತ್ರ_3, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_ಹಿಮ್ಮುಖ_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    ಶೂನ್ಯ _ZdlPvm(ಶೂನ್ಯ *p, size_t n) RPALIAS(rpfree_ಗಾತ್ರ) ಶೂನ್ಯ _ZdaPvm(ಶೂನ್ಯ *p,
                                                                      size_t n)
        RPALIAS(rpfree_ಗಾತ್ರ) ಶೂನ್ಯ _ZdlPvSt11align_val_t(ಶೂನ್ಯ *p, size_t a)
            RPALIAS(rpfree_aligned) ಶೂನ್ಯ _ZdaPvSt11align_val_t(ಶೂನ್ಯ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) ಶೂನ್ಯ _ZdlPvmSt11align_val_t(ಶೂನ್ಯ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_ಗಾತ್ರ_aligned) ಶೂನ್ಯ _ZdaPvmSt11align_val_t(
                        ಶೂನ್ಯ *p, size_t n, size_t a)
                        RPALIAS(rpfree_ಗಾತ್ರ_aligned)
#ಇಲ್ಲದಿದ್ದರೆ
    // 32-bit operators new and new[], normal and aligned
    ಶೂನ್ಯ *_Znwj(uint32_t ಗಾತ್ರ_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ಗಾತ್ರ(1)
        RPALIAS(rpmalloc) ಶೂನ್ಯ *_Znaj(uint32_t ಗಾತ್ರ_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ಗಾತ್ರ(1) RPALIAS(rpmalloc) ಶೂನ್ಯ *_Znwjj(uint32_t ಗಾತ್ರ_3,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_ಹಿಮ್ಮುಖ) ಶೂನ್ಯ *_Znajj(uint32_t ಗಾತ್ರ_3,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_ಹಿಮ್ಮುಖ) ಶೂನ್ಯ *_ZnwjSt11align_val_t(
                size_t ಗಾತ್ರ_3, size_t align)
                RPALIAS(rpaligned_alloc_ಹಿಮ್ಮುಖ) ಶೂನ್ಯ *_ZnajSt11align_val_t(
                    size_t ಗಾತ್ರ_3, size_t align)
                    RPALIAS(rpaligned_alloc_ಹಿಮ್ಮುಖ) ಶೂನ್ಯ *_ZnwjRKSt9nothrow_t(
                        size_t ಗಾತ್ರ_3, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) ಶೂನ್ಯ *_ZnajRKSt9nothrow_t(
                            size_t ಗಾತ್ರ_3,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) ಶೂನ್ಯ
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t ಗಾತ್ರ_3,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_ಹಿಮ್ಮುಖ_nothrow) ಶೂನ್ಯ
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t ಗಾತ್ರ_3, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_ಹಿಮ್ಮುಖ_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    ಶೂನ್ಯ _ZdlPvj(ಶೂನ್ಯ *p, size_t n) RPALIAS(rpfree_ಗಾತ್ರ) ಶೂನ್ಯ _ZdaPvj(ಶೂನ್ಯ *p,
                                                                      size_t n)
        RPALIAS(rpfree_ಗಾತ್ರ) ಶೂನ್ಯ _ZdlPvSt11align_val_t(ಶೂನ್ಯ *p, size_t a)
            RPALIAS(rpfree_aligned) ಶೂನ್ಯ _ZdaPvSt11align_val_t(ಶೂನ್ಯ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) ಶೂನ್ಯ _ZdlPvjSt11align_val_t(ಶೂನ್ಯ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_ಗಾತ್ರ_aligned) ಶೂನ್ಯ _ZdaPvjSt11align_val_t(
                        ಶೂನ್ಯ *p, size_t n, size_t a)
                        RPALIAS(rpfree_ಗಾತ್ರ_aligned)
#ಆದರೆ_ಅಂತ್ಯ

                            ಶೂನ್ಯ *ಸ್ಮರಣೆ_ಕೊಡು(size_t ಗಾತ್ರ_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ಗಾತ್ರ(1)
        RPALIAS(rpmalloc) ಶೂನ್ಯ *ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(size_t ಎಣಿಕೆ_2, size_t ಗಾತ್ರ_3)
            RPALIAS(rpcalloc) ಶೂನ್ಯ *realloc(ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_3)
                RPALIAS(rprealloc) ಶೂನ್ಯ *reallocf(ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_3)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ಗಾತ್ರ(2)
        RPALIAS(rprealloc) ಶೂನ್ಯ *aligned_alloc(size_t alignment, size_t ಗಾತ್ರ_3)
            RPALIAS(rpaligned_alloc) ಶೂನ್ಯ *memalign(
                size_t alignment, size_t ಗಾತ್ರ_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ಗಾತ್ರ(2)
        RPALIAS(rpmemalign) ಪೂರ್ಣಾಂಕ posix_memalign(ಶೂನ್ಯ **memptr, size_t alignment,
                                               size_t ಗಾತ್ರ_3)
            RPALIAS(rpposix_memalign) ಶೂನ್ಯ ಬಿಡುಗಡೆ(ಶೂನ್ಯ *ptr)
                RPALIAS(rpfree) ಶೂನ್ಯ cfree(ಶೂನ್ಯ *ptr) RPALIAS(rpfree)
#ಆದರೆ defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ptr) RPALIAS(rpmalloc_usable_ಗಾತ್ರ)
#ಇಲ್ಲದಿದ್ದರೆ
                    size_t
    malloc_usable_ಗಾತ್ರ(ಶೂನ್ಯ *ptr) RPALIAS(rpmalloc_usable_ಗಾತ್ರ)
#ಆದರೆ_ಅಂತ್ಯ
        size_t malloc_ಗಾತ್ರ(ಶೂನ್ಯ *ptr) RPALIAS(rpmalloc_usable_ಗಾತ್ರ)

#ಆದರೆ_ಅಂತ್ಯ

            ಸ್ಥಿರ ಸಾಲೊಳಗೆ size_t rpmalloc_ಪುಟ_ಗಾತ್ರ(ಶೂನ್ಯ) {
  ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_ಪುಟ_ಗಾತ್ರ;
}

ಬಾಹ್ಯ ಶೂನ್ಯ *RPMALLOC_CDECL reallocarray(ಶೂನ್ಯ *ptr, size_t ಎಣಿಕೆ_2, size_t ಗಾತ್ರ_3);

ಬಾಹ್ಯ ಶೂನ್ಯ *RPMALLOC_CDECL reallocarray(ಶೂನ್ಯ *ptr, size_t ಎಣಿಕೆ_2, size_t ಗಾತ್ರ_3) {
  size_t ಒಟ್ಟು_2;
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
  ಪೂರ್ಣಾಂಕ err = ಗಾತ್ರ_TMult(ಎಣಿಕೆ_2, ಗಾತ್ರ_3, &ಒಟ್ಟು_2);
  ಆದರೆ ((err != S_OK) || (ಒಟ್ಟು_2 >= MAX_ALLOC_ಗಾತ್ರ)) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ err = __builtin_umull_overflow(ಎಣಿಕೆ_2, ಗಾತ್ರ_3, &ಒಟ್ಟು_2);
  ಆದರೆ (err || (ಒಟ್ಟು_2 >= MAX_ALLOC_ಗಾತ್ರ)) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
  ಒಟ್ಟು_2 = ಎಣಿಕೆ_2 * ಗಾತ್ರ_3;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ realloc(ptr, ಒಟ್ಟು_2);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *RPMALLOC_CDECL valloc(size_t ಗಾತ್ರ_3) {
  get_thread_ರಾಶಿ();
  ಹಿಂತಿರುಗಿ rpaligned_alloc(rpmalloc_ಪುಟ_ಗಾತ್ರ(), ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *RPMALLOC_CDECL pvalloc(size_t ಗಾತ್ರ_3) {
  get_thread_ರಾಶಿ();
  ಸ್ಥಿರಾಂಕ size_t ಪುಟ_ಗಾತ್ರ_2 = rpmalloc_ಪುಟ_ಗಾತ್ರ();
  ಸ್ಥಿರಾಂಕ size_t aligned_ಗಾತ್ರ = ((ಗಾತ್ರ_3 + ಪುಟ_ಗಾತ್ರ_2 - 1) / ಪುಟ_ಗಾತ್ರ_2) * ಪುಟ_ಗಾತ್ರ_2;
#ಆದರೆ ENABLE_VALIDATE_ವಾದಗಳು
  ಆದರೆ (aligned_ಗಾತ್ರ < ಗಾತ್ರ_3) {
    errno = EINVAL;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ rpaligned_alloc(rpmalloc_ಪುಟ_ಗಾತ್ರ(), aligned_ಗಾತ್ರ);
}

#ಆದರೆ_ಅಂತ್ಯ // ENABLE_OVERRIDE

#ಆದರೆ ENABLE_PRELOAD

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32

#ಆದರೆ defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

ಬಾಹ್ಯ __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

ಬಾಹ್ಯ __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(reserved);
  (ಶೂನ್ಯ)ಗಾತ್ರ(instance);
  ಆದರೆ (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  ಹಿಂತಿರುಗಿ TRUE;
}

// end BUILD_DYNAMIC_LINK
#ಇಲ್ಲದಿದ್ದರೆ

ಬಾಹ್ಯ ಶೂನ್ಯ _global_rpmalloc_init(ಶೂನ್ಯ) {
  rpmalloc_ಗಣ_main_thread();
  rpmalloc_initialize();
}

#ಆದರೆ defined(__clang__) || defined(__GNUC__)

ಸ್ಥಿರ ಶೂನ್ಯ __attribute__((constructor)) initializer(ಶೂನ್ಯ) {
  _global_rpmalloc_init();
}

#ಅಥವಾಆದರೆ defined(_MSC_VER)

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ _global_rpmalloc_xib(ಶೂನ್ಯ) {
  _global_rpmalloc_init();
  ಹಿಂತಿರುಗಿ 0;
}

#ಸೂಚನೆ section(".CRT$XIB", ಪಠ)
__declspec(allocate(".CRT$XIB")) ಶೂನ್ಯ (*_rpmalloc_module_init)(ಶೂನ್ಯ) =
    _global_rpmalloc_xib;
#ಆದರೆ defined(_M_IX86) || defined(__i386__)
#ಸೂಚನೆ comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#ಇಲ್ಲದಿದ್ದರೆ
#ಸೂಚನೆ comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ

// end !BUILD_DYNAMIC_LINK
#ಆದರೆ_ಅಂತ್ಯ

#ಇಲ್ಲದಿದ್ದರೆ

#ಅನ್ತರ್ಭಾವ <pthread.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

ಬಾಹ್ಯ ಶೂನ್ಯ rpmalloc_ಗಣ_main_thread(ಶೂನ್ಯ);

ಸ್ಥಿರ pthread_key_t destructor_ಕೀಲಿ;

ಸ್ಥಿರ ಶೂನ್ಯ thread_destructor(ಶೂನ್ಯ *);

ಸ್ಥಿರ ಶೂನ್ಯ __attribute__((constructor)) initializer(ಶೂನ್ಯ) {
  rpmalloc_ಗಣ_main_thread();
  rpmalloc_initialize();
  pthread_ಕೀಲಿ_create(&destructor_ಕೀಲಿ, thread_destructor);
}

ಸ್ಥಿರ ಶೂನ್ಯ __attribute__((destructor)) finalizer(ಶೂನ್ಯ) { rpmalloc_finalize(); }

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
  ಶೂನ್ಯ *(*real_ಆರಂಭ)(ಶೂನ್ಯ *);
  ಶೂನ್ಯ *real_arg;
} thread_starter_arg;

ಸ್ಥಿರ ಶೂನ್ಯ *thread_starter(ಶೂನ್ಯ *argptr) {
  thread_starter_arg *arg = argptr;
  ಶೂನ್ಯ *(*real_ಆರಂಭ)(ಶೂನ್ಯ *) = arg->real_ಆರಂಭ;
  ಶೂನ್ಯ *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_ಕೀಲಿ, (ಶೂನ್ಯ *)1);
  ಹಿಂತಿರುಗಿ (*real_ಆರಂಭ)(real_arg);
}

ಸ್ಥಿರ ಶೂನ್ಯ thread_destructor(ಶೂನ್ಯ *ಮೌಲ್ಯ_2) {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಮೌಲ್ಯ_2);
  rpmalloc_thread_finalize(1);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pthread_create_proxy(pthread_t *thread, ಸ್ಥಿರಾಂಕ pthread_attr_t *attr,
                                ಶೂನ್ಯ *(*ಆರಂಭ_routine)(ಶೂನ್ಯ *), ಶೂನ್ಯ *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(ಗಾತ್ರ(thread_starter_arg));
  starter_arg->real_ಆರಂಭ = ಆರಂಭ_routine;
  starter_arg->real_arg = arg;
  ಹಿಂತಿರುಗಿ pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#ಇಲ್ಲದಿದ್ದರೆ

#ಅನ್ತರ್ಭಾವ <dlfcn.h>

ಪೂರ್ಣಾಂಕ pthread_create(pthread_t *thread, ಸ್ಥಿರಾಂಕ pthread_attr_t *attr,
                   ಶೂನ್ಯ *(*ಆರಂಭ_routine)(ಶೂನ್ಯ *), ಶೂನ್ಯ *arg) {
#ಆದರೆ defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  ಅಕ್ಷರ fname[] = "pthread_create";
#ಇಲ್ಲದಿದ್ದರೆ
  ಅಕ್ಷರ fname[] = "_pthread_create";
#ಆದರೆ_ಅಂತ್ಯ
  ಶೂನ್ಯ *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(ಗಾತ್ರ(thread_starter_arg));
  starter_arg->real_ಆರಂಭ = ಆರಂಭ_routine;
  starter_arg->real_arg = arg;
  ಹಿಂತಿರುಗಿ (*(ಪೂರ್ಣಾಂಕ (*)(pthread_t *, ಸ್ಥಿರಾಂಕ pthread_attr_t *, ಶೂನ್ಯ *(*)(ಶೂನ್ಯ *),
                    ಶೂನ್ಯ *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_OVERRIDE

#ಆದರೆ defined(__GLIBC__) && defined(__linux__)

ಶೂನ್ಯ *__libc_malloc(size_t ಗಾತ್ರ_3) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ಗಾತ್ರ(1)
        RPALIAS(rpmalloc) ಶೂನ್ಯ *__libc_calloc(size_t ಎಣಿಕೆ_2, size_t ಗಾತ್ರ_3)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) ಶೂನ್ಯ *__libc_realloc(ಶೂನ್ಯ *p, size_t ಗಾತ್ರ_3)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ಗಾತ್ರ(2) RPALIAS(rprealloc) ಶೂನ್ಯ __libc_free(ಶೂನ್ಯ *p)
        RPALIAS(rpfree) ಶೂನ್ಯ __libc_cfree(ಶೂನ್ಯ *p)
            RPALIAS(rpfree) ಶೂನ್ಯ *__libc_memalign(size_t align, size_t ಗಾತ್ರ_3)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ಗಾತ್ರ(2)
        RPALIAS(rpmemalign) ಪೂರ್ಣಾಂಕ __posix_memalign(ಶೂನ್ಯ **p, size_t align,
                                                 size_t ಗಾತ್ರ_3)
            RPALIAS(rpposix_memalign)

                ಬಾಹ್ಯ ಶೂನ್ಯ *__libc_valloc(size_t ಗಾತ್ರ_3);
ಬಾಹ್ಯ ಶೂನ್ಯ *__libc_pvalloc(size_t ಗಾತ್ರ_3);

ಶೂನ್ಯ *__libc_valloc(size_t ಗಾತ್ರ_3) { ಹಿಂತಿರುಗಿ valloc(ಗಾತ್ರ_3); }

ಶೂನ್ಯ *__libc_pvalloc(size_t ಗಾತ್ರ_3) { ಹಿಂತಿರುಗಿ pvalloc(ಗಾತ್ರ_3); }

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ (defined(__GNUC__) || defined(__clang__))
#ಸೂಚನೆ GCC visibility ತೆಗೆ
#ಆದರೆ_ಅಂತ್ಯ
