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

#જો defined(__TINYC__)
#અન્તર્ભાવ <sys/types.h>
#endif

#ifndef ARCH_64BIT
#જો defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#define ARCH_64BIT 1
_Static_assert(કદ(size_t) == 8, "Data type size mismatch");
_Static_assert(કદ(શૂન્ય *) == 8, "Data type size mismatch");
#નહીંતર
#define ARCH_64BIT 0
_Static_assert(કદ(size_t) == 4, "Data type size mismatch");
_Static_assert(કદ(શૂન્ય *) == 4, "Data type size mismatch");
#endif
#endif

#જો (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility ધકેલો(મૂળભૂત)
#endif

#define USE_IMPLEMENT 1
#define USE_INTERPOSE 0
#define USE_ALIAS 0

#જો defined(__APPLE__)
#undef USE_INTERPOSE
#define USE_INTERPOSE 1

પ્રકારવ્યાખ્યા સંરચના interpose_t {
  શૂન્ય *new_func;
  શૂન્ય *orig_func;
} interpose_t;

#define MAC_INTERPOSE_PAIR(newf, oldf) {(શૂન્ય *)newf, (શૂન્ય *)oldf}
#define MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((used)) સ્થિર અચલ interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#endif

#જો !defined(_WIN32) && !defined(__APPLE__)
#undef USE_IMPLEMENT
#undef USE_ALIAS
#define USE_IMPLEMENT 0
#define USE_ALIAS 1
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#undef malloc
#undef મુક્ત
#undef calloc
#define RPMALLOC_RESTRICT __declspec(restrict)
#નહીંતર
#define RPMALLOC_RESTRICT
#endif

#જો ENABLE_OVERRIDE

પ્રકારવ્યાખ્યા સંરચના rp_nothrow_t {
  પૂર્ણાંક __dummy;
} rp_nothrow_t;

#જો USE_IMPLEMENT

બાહ્ય અંતઃરેખ RPMALLOC_RESTRICT શૂન્ય *RPMALLOC_CDECL malloc(size_t size) {
  પાછા rpmalloc(size);
}
બાહ્ય અંતઃરેખ RPMALLOC_RESTRICT શૂન્ય *RPMALLOC_CDECL calloc(size_t ગણતરી,
                                                            size_t size) {
  પાછા rpcalloc(ગણતરી, size);
}
બાહ્ય અંતઃરેખ RPMALLOC_RESTRICT શૂન્ય *RPMALLOC_CDECL realloc(શૂન્ય *ptr,
                                                             size_t size) {
  પાછા rprealloc(ptr, size);
}
બાહ્ય અંતઃરેખ શૂન્ય *RPMALLOC_CDECL reallocf(શૂન્ય *ptr, size_t size) {
  પાછા rprealloc(ptr, size);
}
બાહ્ય અંતઃરેખ શૂન્ય *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t size) {
  પાછા rpaligned_alloc(alignment, size);
}
બાહ્ય અંતઃરેખ શૂન્ય *RPMALLOC_CDECL memalign(size_t alignment, size_t size) {
  પાછા rpmemalign(alignment, size);
}
બાહ્ય અંતઃરેખ પૂર્ણાંક RPMALLOC_CDECL posix_memalign(શૂન્ય **memptr, size_t alignment,
                                                size_t size) {
  પાછા rpposix_memalign(memptr, alignment, size);
}
બાહ્ય અંતઃરેખ શૂન્ય RPMALLOC_CDECL મુક્ત(શૂન્ય *ptr) { rpfree(ptr); }
બાહ્ય અંતઃરેખ શૂન્ય RPMALLOC_CDECL cfree(શૂન્ય *ptr) { rpfree(ptr); }
બાહ્ય અંતઃરેખ size_t RPMALLOC_CDECL malloc_usable_size(શૂન્ય *ptr) {
  પાછા rpmalloc_usable_size(ptr);
}
બાહ્ય અંતઃરેખ size_t RPMALLOC_CDECL malloc_size(શૂન્ય *ptr) {
  પાછા rpmalloc_usable_size(ptr);
}

#ifdef _WIN32
બાહ્ય અંતઃરેખ RPMALLOC_RESTRICT શૂન્ય *RPMALLOC_CDECL _malloc_base(size_t size) {
  પાછા rpmalloc(size);
}
બાહ્ય અંતઃરેખ શૂન્ય RPMALLOC_CDECL _free_base(શૂન્ય *ptr) { rpfree(ptr); }
બાહ્ય અંતઃરેખ RPMALLOC_RESTRICT શૂન્ય *RPMALLOC_CDECL _calloc_base(size_t ગણતરી,
                                                                  size_t size) {
  પાછા rpcalloc(ગણતરી, size);
}
બાહ્ય અંતઃરેખ size_t RPMALLOC_CDECL _msize(શૂન્ય *ptr) {
  પાછા rpmalloc_usable_size(ptr);
}
બાહ્ય અંતઃરેખ size_t RPMALLOC_CDECL _msize_base(શૂન્ય *ptr) {
  પાછા rpmalloc_usable_size(ptr);
}
બાહ્ય અંતઃરેખ RPMALLOC_RESTRICT શૂન્ય *RPMALLOC_CDECL
_realloc_base(શૂન્ય *ptr, size_t size) {
  પાછા rprealloc(ptr, size);
}
#endif

#ifdef _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#નહીંતર
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#define RPDEFVIS __attribute__((visibility("default")))
બાહ્ય શૂન્ય _ZdlPv(શૂન્ય *p);
શૂન્ય RPDEFVIS _ZdlPv(શૂન્ય *p) { rpfree(p); }
બાહ્ય શૂન્ય _ZdaPv(શૂન્ય *p);
શૂન્ય RPDEFVIS _ZdaPv(શૂન્ય *p) { rpfree(p); }
#જો ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
બાહ્ય શૂન્ય *_Znwm(uint64_t size);
શૂન્ય *RPDEFVIS _Znwm(uint64_t size) { પાછા rpmalloc(size); }
બાહ્ય શૂન્ય *_Znam(uint64_t size);
શૂન્ય *RPDEFVIS _Znam(uint64_t size) { પાછા rpmalloc(size); }
બાહ્ય શૂન્ય *_Znwmm(uint64_t size, uint64_t align);
શૂન્ય *RPDEFVIS _Znwmm(uint64_t size, uint64_t align) {
  પાછા rpaligned_alloc(align, size);
}
બાહ્ય શૂન્ય *_Znamm(uint64_t size, uint64_t align);
શૂન્ય *RPDEFVIS _Znamm(uint64_t size, uint64_t align) {
  પાછા rpaligned_alloc(align, size);
}
બાહ્ય શૂન્ય *_ZnwmSt11align_val_t(uint64_t size, uint64_t align);
શૂન્ય *RPDEFVIS _ZnwmSt11align_val_t(uint64_t size, uint64_t align) {
  પાછા rpaligned_alloc(align, size);
}
બાહ્ય શૂન્ય *_ZnamSt11align_val_t(uint64_t size, uint64_t align);
શૂન્ય *RPDEFVIS _ZnamSt11align_val_t(uint64_t size, uint64_t align) {
  પાછા rpaligned_alloc(align, size);
}
બાહ્ય શૂન્ય *_ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
શૂન્ય *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (શૂન્ય)કદ(t);
  પાછા rpmalloc(size);
}
બાહ્ય શૂન્ય *_ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
શૂન્ય *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (શૂન્ય)કદ(t);
  પાછા rpmalloc(size);
}
બાહ્ય શૂન્ય *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
શૂન્ય *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (શૂન્ય)કદ(t);
  પાછા rpaligned_alloc(align, size);
}
બાહ્ય શૂન્ય *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
શૂન્ય *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (શૂન્ય)કદ(t);
  પાછા rpaligned_alloc(align, size);
}
// 64-bit operators sized delete and delete[], normal and aligned
બાહ્ય શૂન્ય _ZdlPvm(શૂન્ય *p, uint64_t size);
શૂન્ય RPDEFVIS _ZdlPvm(શૂન્ય *p, uint64_t size) {
  rpfree(p);
  (શૂન્ય)કદ(size);
}
બાહ્ય શૂન્ય _ZdaPvm(શૂન્ય *p, uint64_t size);
શૂન્ય RPDEFVIS _ZdaPvm(શૂન્ય *p, uint64_t size) {
  rpfree(p);
  (શૂન્ય)કદ(size);
}
બાહ્ય શૂન્ય _ZdlPvSt11align_val_t(શૂન્ય *p, uint64_t align);
શૂન્ય RPDEFVIS _ZdlPvSt11align_val_t(શૂન્ય *p, uint64_t align) {
  rpfree(p);
  (શૂન્ય)કદ(align);
}
બાહ્ય શૂન્ય _ZdaPvSt11align_val_t(શૂન્ય *p, uint64_t align);
શૂન્ય RPDEFVIS _ZdaPvSt11align_val_t(શૂન્ય *p, uint64_t align) {
  rpfree(p);
  (શૂન્ય)કદ(align);
}
બાહ્ય શૂન્ય _ZdlPvmSt11align_val_t(શૂન્ય *p, uint64_t size, uint64_t align);
શૂન્ય RPDEFVIS _ZdlPvmSt11align_val_t(શૂન્ય *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (શૂન્ય)કદ(size);
  (શૂન્ય)કદ(align);
}
બાહ્ય શૂન્ય _ZdaPvmSt11align_val_t(શૂન્ય *p, uint64_t size, uint64_t align);
શૂન્ય RPDEFVIS _ZdaPvmSt11align_val_t(શૂન્ય *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (શૂન્ય)કદ(size);
  (શૂન્ય)કદ(align);
}
#નહીંતર
// 32-bit operators new and new[], normal and aligned
બાહ્ય શૂન્ય *_Znwj(uint32_t size);
શૂન્ય *RPDEFVIS _Znwj(uint32_t size) { પાછા rpmalloc(size); }
બાહ્ય શૂન્ય *_Znaj(uint32_t size);
શૂન્ય *RPDEFVIS _Znaj(uint32_t size) { પાછા rpmalloc(size); }
બાહ્ય શૂન્ય *_Znwjj(uint32_t size, uint32_t align);
શૂન્ય *RPDEFVIS _Znwjj(uint32_t size, uint32_t align) {
  પાછા rpaligned_alloc(align, size);
}
બાહ્ય શૂન્ય *_Znajj(uint32_t size, uint32_t align);
શૂન્ય *RPDEFVIS _Znajj(uint32_t size, uint32_t align) {
  પાછા rpaligned_alloc(align, size);
}
બાહ્ય શૂન્ય *_ZnwjSt11align_val_t(size_t size, size_t align);
શૂન્ય *RPDEFVIS _ZnwjSt11align_val_t(size_t size, size_t align) {
  પાછા rpaligned_alloc(align, size);
}
બાહ્ય શૂન્ય *_ZnajSt11align_val_t(size_t size, size_t align);
શૂન્ય *RPDEFVIS _ZnajSt11align_val_t(size_t size, size_t align) {
  પાછા rpaligned_alloc(align, size);
}
બાહ્ય શૂન્ય *_ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t);
શૂન્ય *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (શૂન્ય)કદ(t);
  પાછા rpmalloc(size);
}
બાહ્ય શૂન્ય *_ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t);
શૂન્ય *RPDEFVIS _ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (શૂન્ય)કદ(t);
  પાછા rpmalloc(size);
}
બાહ્ય શૂન્ય *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
શૂન્ય *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (શૂન્ય)કદ(t);
  પાછા rpaligned_alloc(align, size);
}
બાહ્ય શૂન્ય *_ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
શૂન્ય *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (શૂન્ય)કદ(t);
  પાછા rpaligned_alloc(align, size);
}
// 32-bit operators sized delete and delete[], normal and aligned
બાહ્ય શૂન્ય _ZdlPvj(શૂન્ય *p, uint64_t size);
શૂન્ય RPDEFVIS _ZdlPvj(શૂન્ય *p, uint64_t size) {
  rpfree(p);
  (શૂન્ય)કદ(size);
}
બાહ્ય શૂન્ય _ZdaPvj(શૂન્ય *p, uint64_t size);
શૂન્ય RPDEFVIS _ZdaPvj(શૂન્ય *p, uint64_t size) {
  rpfree(p);
  (શૂન્ય)કદ(size);
}
બાહ્ય શૂન્ય _ZdlPvSt11align_val_t(શૂન્ય *p, uint32_t align);
શૂન્ય RPDEFVIS _ZdlPvSt11align_val_t(શૂન્ય *p, uint64_t a) {
  rpfree(p);
  (શૂન્ય)કદ(align);
}
બાહ્ય શૂન્ય _ZdaPvSt11align_val_t(શૂન્ય *p, uint32_t align);
શૂન્ય RPDEFVIS _ZdaPvSt11align_val_t(શૂન્ય *p, uint64_t a) {
  rpfree(p);
  (શૂન્ય)કદ(align);
}
બાહ્ય શૂન્ય _ZdlPvjSt11align_val_t(શૂન્ય *p, uint32_t size, uint32_t align);
શૂન્ય RPDEFVIS _ZdlPvjSt11align_val_t(શૂન્ય *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (શૂન્ય)કદ(size);
  (શૂન્ય)કદ(a);
}
બાહ્ય શૂન્ય _ZdaPvjSt11align_val_t(શૂન્ય *p, uint32_t size, uint32_t align);
શૂન્ય RPDEFVIS _ZdaPvjSt11align_val_t(શૂન્ય *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (શૂન્ય)કદ(size);
  (શૂન્ય)કદ(a);
}
#endif
#endif
#endif

#જો USE_INTERPOSE || USE_ALIAS

સ્થિર શૂન્ય *rpmalloc_nothrow(size_t size, rp_nothrow_t t) {
  (શૂન્ય)કદ(t);
  પાછા rpmalloc(size);
}
સ્થિર શૂન્ય *rpaligned_alloc_reverse(size_t size, size_t align) {
  પાછા rpaligned_alloc(align, size);
}
સ્થિર શૂન્ય *rpaligned_alloc_reverse_nothrow(size_t size, size_t align,
                                             rp_nothrow_t t) {
  (શૂન્ય)કદ(t);
  પાછા rpaligned_alloc(align, size);
}
સ્થિર શૂન્ય rpfree_size(શૂન્ય *p, size_t size) {
  (શૂન્ય)કદ(size);
  rpfree(p);
}
સ્થિર શૂન્ય rpfree_aligned(શૂન્ય *p, size_t align) {
  (શૂન્ય)કદ(align);
  rpfree(p);
}
સ્થિર શૂન્ય rpfree_size_aligned(શૂન્ય *p, size_t size, size_t align) {
  (શૂન્ય)કદ(size);
  (શૂન્ય)કદ(align);
  rpfree(p);
}

#endif

#જો USE_INTERPOSE

__attribute__((used)) સ્થિર અચલ interpose_t macinterpose_malloc[]
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
#જો defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#endif
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, મુક્ત), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_usable_size),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_size)};

#endif

#જો USE_ALIAS

#define RPALIAS(fn) __attribute__((alias(#fn), used, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
શૂન્ય _ZdlPv(શૂન્ય *p) RPALIAS(rpfree) શૂન્ય _ZdaPv(શૂન્ય *p) RPALIAS(rpfree)

#જો ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    શૂન્ય *_Znwm(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) શૂન્ય *_Znam(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) શૂન્ય *_Znwmm(uint64_t size,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) શૂન્ય *_Znamm(uint64_t size,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) શૂન્ય *_ZnwmSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) શૂન્ય *_ZnamSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) શૂન્ય *_ZnwmRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) શૂન્ય *_ZnamRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) શૂન્ય
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) શૂન્ય
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    શૂન્ય _ZdlPvm(શૂન્ય *p, size_t n) RPALIAS(rpfree_size) શૂન્ય _ZdaPvm(શૂન્ય *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) શૂન્ય _ZdlPvSt11align_val_t(શૂન્ય *p, size_t a)
            RPALIAS(rpfree_aligned) શૂન્ય _ZdaPvSt11align_val_t(શૂન્ય *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) શૂન્ય _ZdlPvmSt11align_val_t(શૂન્ય *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) શૂન્ય _ZdaPvmSt11align_val_t(
                        શૂન્ય *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#નહીંતર
    // 32-bit operators new and new[], normal and aligned
    શૂન્ય *_Znwj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) શૂન્ય *_Znaj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) શૂન્ય *_Znwjj(uint32_t size,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) શૂન્ય *_Znajj(uint32_t size,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) શૂન્ય *_ZnwjSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) શૂન્ય *_ZnajSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) શૂન્ય *_ZnwjRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) શૂન્ય *_ZnajRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) શૂન્ય
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) શૂન્ય
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    શૂન્ય _ZdlPvj(શૂન્ય *p, size_t n) RPALIAS(rpfree_size) શૂન્ય _ZdaPvj(શૂન્ય *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) શૂન્ય _ZdlPvSt11align_val_t(શૂન્ય *p, size_t a)
            RPALIAS(rpfree_aligned) શૂન્ય _ZdaPvSt11align_val_t(શૂન્ય *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) શૂન્ય _ZdlPvjSt11align_val_t(શૂન્ય *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) શૂન્ય _ZdaPvjSt11align_val_t(
                        શૂન્ય *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#endif

                            શૂન્ય *malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) શૂન્ય *calloc(size_t ગણતરી, size_t size)
            RPALIAS(rpcalloc) શૂન્ય *realloc(શૂન્ય *ptr, size_t size)
                RPALIAS(rprealloc) શૂન્ય *reallocf(શૂન્ય *ptr, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rprealloc) શૂન્ય *aligned_alloc(size_t alignment, size_t size)
            RPALIAS(rpaligned_alloc) શૂન્ય *memalign(
                size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) પૂર્ણાંક posix_memalign(શૂન્ય **memptr, size_t alignment,
                                               size_t size)
            RPALIAS(rpposix_memalign) શૂન્ય મુક્ત(શૂન્ય *ptr)
                RPALIAS(rpfree) શૂન્ય cfree(શૂન્ય *ptr) RPALIAS(rpfree)
#જો defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_size(અચલ શૂન્ય *ptr) RPALIAS(rpmalloc_usable_size)
#નહીંતર
                    size_t
    malloc_usable_size(શૂન્ય *ptr) RPALIAS(rpmalloc_usable_size)
#endif
        size_t malloc_size(શૂન્ય *ptr) RPALIAS(rpmalloc_usable_size)

#endif

            સ્થિર અંતઃરેખ size_t _rpmalloc_page_size(શૂન્ય) {
  પાછા _memory_page_size;
}

બાહ્ય શૂન્ય *RPMALLOC_CDECL reallocarray(શૂન્ય *ptr, size_t ગણતરી, size_t size);

બાહ્ય શૂન્ય *RPMALLOC_CDECL reallocarray(શૂન્ય *ptr, size_t ગણતરી, size_t size) {
  size_t total;
#જો ENABLE_VALIDATE_ARGS
#ifdef _MSC_VER
  પૂર્ણાંક err = SizeTMult(ગણતરી, size, &total);
  જો ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    પાછા 0;
  }
#નહીંતર
  પૂર્ણાંક err = __builtin_umull_overflow(ગણતરી, size, &total);
  જો (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    પાછા 0;
  }
#endif
#નહીંતર
  total = ગણતરી * size;
#endif
  પાછા realloc(ptr, total);
}

બાહ્ય અંતઃરેખ શૂન્ય *RPMALLOC_CDECL valloc(size_t size) {
  get_thread_heap();
  પાછા rpaligned_alloc(_rpmalloc_page_size(), size);
}

બાહ્ય અંતઃરેખ શૂન્ય *RPMALLOC_CDECL pvalloc(size_t size) {
  get_thread_heap();
  અચલ size_t page_size = _rpmalloc_page_size();
  અચલ size_t aligned_size = ((size + page_size - 1) / page_size) * page_size;
#જો ENABLE_VALIDATE_ARGS
  જો (aligned_size < size) {
    errno = EINVAL;
    પાછા 0;
  }
#endif
  પાછા rpaligned_alloc(_rpmalloc_page_size(), aligned_size);
}

#endif // ENABLE_OVERRIDE

#જો ENABLE_PRELOAD

#ifdef _WIN32

#જો defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

બાહ્ય __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

બાહ્ય __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (શૂન્ય)કદ(reserved);
  (શૂન્ય)કદ(instance);
  જો (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  નહીંતર જો (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  નહીંતર જો (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  નહીંતર જો (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  પાછા TRUE;
}

// end BUILD_DYNAMIC_LINK
#નહીંતર

બાહ્ય શૂન્ય _global_rpmalloc_init(શૂન્ય) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
}

#જો defined(__clang__) || defined(__GNUC__)

સ્થિર શૂન્ય __attribute__((constructor)) initializer(શૂન્ય) {
  _global_rpmalloc_init();
}

#અથવાજો defined(_MSC_VER)

સ્થિર પૂર્ણાંક _global_rpmalloc_xib(શૂન્ય) {
  _global_rpmalloc_init();
  પાછા 0;
}

#pragma section(".CRT$XIB", પઠ)
__declspec(allocate(".CRT$XIB")) શૂન્ય (*_rpmalloc_module_init)(શૂન્ય) =
    _global_rpmalloc_xib;
#જો defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#નહીંતર
#pragma comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#endif

#endif

// end !BUILD_DYNAMIC_LINK
#endif

#નહીંતર

#અન્તર્ભાવ <pthread.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <unistd.h>

બાહ્ય શૂન્ય rpmalloc_set_main_thread(શૂન્ય);

સ્થિર pthread_key_t destructor_ચાવી;

સ્થિર શૂન્ય thread_destructor(શૂન્ય *);

સ્થિર શૂન્ય __attribute__((constructor)) initializer(શૂન્ય) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
  pthread_ચાવી_create(&destructor_ચાવી, thread_destructor);
}

સ્થિર શૂન્ય __attribute__((destructor)) finalizer(શૂન્ય) { rpmalloc_finalize(); }

પ્રકારવ્યાખ્યા સંરચના {
  શૂન્ય *(*real_start)(શૂન્ય *);
  શૂન્ય *real_arg;
} thread_starter_arg;

સ્થિર શૂન્ય *thread_starter(શૂન્ય *argptr) {
  thread_starter_arg *arg = argptr;
  શૂન્ય *(*real_start)(શૂન્ય *) = arg->real_start;
  શૂન્ય *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_ચાવી, (શૂન્ય *)1);
  પાછા (*real_start)(real_arg);
}

સ્થિર શૂન્ય thread_destructor(શૂન્ય *કિંમત_2) {
  (શૂન્ય)કદ(કિંમત_2);
  rpmalloc_thread_finalize(1);
}

#ifdef __APPLE__

સ્થિર પૂર્ણાંક pthread_create_proxy(pthread_t *thread, અચલ pthread_attr_t *attr,
                                શૂન્ય *(*start_routine)(શૂન્ય *), શૂન્ય *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(કદ(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  પાછા pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#નહીંતર

#અન્તર્ભાવ <dlfcn.h>

પૂર્ણાંક pthread_create(pthread_t *thread, અચલ pthread_attr_t *attr,
                   શૂન્ય *(*start_routine)(શૂન્ય *), શૂન્ય *arg) {
#જો defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  અક્ષર fname[] = "pthread_create";
#નહીંતર
  અક્ષર fname[] = "_pthread_create";
#endif
  શૂન્ય *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(કદ(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  પાછા (*(પૂર્ણાંક (*)(pthread_t *, અચલ pthread_attr_t *, શૂન્ય *(*)(શૂન્ય *),
                    શૂન્ય *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#endif

#endif

#endif

#જો ENABLE_OVERRIDE

#જો defined(__GLIBC__) && defined(__linux__)

શૂન્ય *__libc_malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) શૂન્ય *__libc_calloc(size_t ગણતરી, size_t size)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) શૂન્ય *__libc_realloc(શૂન્ય *p, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2) RPALIAS(rprealloc) શૂન્ય __libc_free(શૂન્ય *p)
        RPALIAS(rpfree) શૂન્ય __libc_cfree(શૂન્ય *p)
            RPALIAS(rpfree) શૂન્ય *__libc_memalign(size_t align, size_t size)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) પૂર્ણાંક __posix_memalign(શૂન્ય **p, size_t align,
                                                 size_t size)
            RPALIAS(rpposix_memalign)

                બાહ્ય શૂન્ય *__libc_valloc(size_t size);
બાહ્ય શૂન્ય *__libc_pvalloc(size_t size);

શૂન્ય *__libc_valloc(size_t size) { પાછા valloc(size); }

શૂન્ય *__libc_pvalloc(size_t size) { પાછા pvalloc(size); }

#endif

#endif

#જો (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility કાઢો
#endif
