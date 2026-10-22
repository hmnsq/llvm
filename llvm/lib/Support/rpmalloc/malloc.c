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

#यदि defined(__TINYC__)
#अन्तर्भाव <sys/types.h>
#endif

#ifndef ARCH_64BIT
#यदि defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#define ARCH_64BIT 1
_Static_assert(परिमाण(size_t) == 8, "Data type size mismatch");
_Static_assert(परिमाण(शून्य *) == 8, "Data type size mismatch");
#अन्यथा
#define ARCH_64BIT 0
_Static_assert(परिमाण(size_t) == 4, "Data type size mismatch");
_Static_assert(परिमाण(शून्य *) == 4, "Data type size mismatch");
#endif
#endif

#यदि (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility धकेलो(स्वभाव)
#endif

#define USE_IMPLEMENT 1
#define USE_INTERPOSE 0
#define USE_ALIAS 0

#यदि defined(__APPLE__)
#undef USE_INTERPOSE
#define USE_INTERPOSE 1

प्रकारनिर्देश संरचना interpose_t {
  शून्य *new_func;
  शून्य *orig_func;
} interpose_t;

#define MAC_INTERPOSE_PAIR(newf, oldf) {(शून्य *)newf, (शून्य *)oldf}
#define MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((used)) स्थिर ध्रुव interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#endif

#यदि !defined(_WIN32) && !defined(__APPLE__)
#undef USE_IMPLEMENT
#undef USE_ALIAS
#define USE_IMPLEMENT 0
#define USE_ALIAS 1
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#undef malloc
#undef मुक्त
#undef calloc
#define RPMALLOC_RESTRICT __declspec(restrict)
#अन्यथा
#define RPMALLOC_RESTRICT
#endif

#यदि ENABLE_OVERRIDE

प्रकारनिर्देश संरचना rp_nothrow_t {
  पूर्णाङ्क __dummy;
} rp_nothrow_t;

#यदि USE_IMPLEMENT

बाह्य अन्तःस्थ RPMALLOC_RESTRICT शून्य *RPMALLOC_CDECL malloc(size_t size) {
  प्रत्यागम rpmalloc(size);
}
बाह्य अन्तःस्थ RPMALLOC_RESTRICT शून्य *RPMALLOC_CDECL calloc(size_t गिनती,
                                                            size_t size) {
  प्रत्यागम rpcalloc(गिनती, size);
}
बाह्य अन्तःस्थ RPMALLOC_RESTRICT शून्य *RPMALLOC_CDECL realloc(शून्य *ptr,
                                                             size_t size) {
  प्रत्यागम rprealloc(ptr, size);
}
बाह्य अन्तःस्थ शून्य *RPMALLOC_CDECL reallocf(शून्य *ptr, size_t size) {
  प्रत्यागम rprealloc(ptr, size);
}
बाह्य अन्तःस्थ शून्य *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t size) {
  प्रत्यागम rpaligned_alloc(alignment, size);
}
बाह्य अन्तःस्थ शून्य *RPMALLOC_CDECL memalign(size_t alignment, size_t size) {
  प्रत्यागम rpmemalign(alignment, size);
}
बाह्य अन्तःस्थ पूर्णाङ्क RPMALLOC_CDECL posix_memalign(शून्य **memptr, size_t alignment,
                                                size_t size) {
  प्रत्यागम rpposix_memalign(memptr, alignment, size);
}
बाह्य अन्तःस्थ शून्य RPMALLOC_CDECL मुक्त(शून्य *ptr) { rpfree(ptr); }
बाह्य अन्तःस्थ शून्य RPMALLOC_CDECL cfree(शून्य *ptr) { rpfree(ptr); }
बाह्य अन्तःस्थ size_t RPMALLOC_CDECL malloc_usable_size(शून्य *ptr) {
  प्रत्यागम rpmalloc_usable_size(ptr);
}
बाह्य अन्तःस्थ size_t RPMALLOC_CDECL malloc_size(शून्य *ptr) {
  प्रत्यागम rpmalloc_usable_size(ptr);
}

#ifdef _WIN32
बाह्य अन्तःस्थ RPMALLOC_RESTRICT शून्य *RPMALLOC_CDECL _malloc_base(size_t size) {
  प्रत्यागम rpmalloc(size);
}
बाह्य अन्तःस्थ शून्य RPMALLOC_CDECL _free_base(शून्य *ptr) { rpfree(ptr); }
बाह्य अन्तःस्थ RPMALLOC_RESTRICT शून्य *RPMALLOC_CDECL _calloc_base(size_t गिनती,
                                                                  size_t size) {
  प्रत्यागम rpcalloc(गिनती, size);
}
बाह्य अन्तःस्थ size_t RPMALLOC_CDECL _msize(शून्य *ptr) {
  प्रत्यागम rpmalloc_usable_size(ptr);
}
बाह्य अन्तःस्थ size_t RPMALLOC_CDECL _msize_base(शून्य *ptr) {
  प्रत्यागम rpmalloc_usable_size(ptr);
}
बाह्य अन्तःस्थ RPMALLOC_RESTRICT शून्य *RPMALLOC_CDECL
_realloc_base(शून्य *ptr, size_t size) {
  प्रत्यागम rprealloc(ptr, size);
}
#endif

#ifdef _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#अन्यथा
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#define RPDEFVIS __attribute__((visibility("default")))
बाह्य शून्य _ZdlPv(शून्य *p);
शून्य RPDEFVIS _ZdlPv(शून्य *p) { rpfree(p); }
बाह्य शून्य _ZdaPv(शून्य *p);
शून्य RPDEFVIS _ZdaPv(शून्य *p) { rpfree(p); }
#यदि ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
बाह्य शून्य *_Znwm(uint64_t size);
शून्य *RPDEFVIS _Znwm(uint64_t size) { प्रत्यागम rpmalloc(size); }
बाह्य शून्य *_Znam(uint64_t size);
शून्य *RPDEFVIS _Znam(uint64_t size) { प्रत्यागम rpmalloc(size); }
बाह्य शून्य *_Znwmm(uint64_t size, uint64_t align);
शून्य *RPDEFVIS _Znwmm(uint64_t size, uint64_t align) {
  प्रत्यागम rpaligned_alloc(align, size);
}
बाह्य शून्य *_Znamm(uint64_t size, uint64_t align);
शून्य *RPDEFVIS _Znamm(uint64_t size, uint64_t align) {
  प्रत्यागम rpaligned_alloc(align, size);
}
बाह्य शून्य *_ZnwmSt11align_val_t(uint64_t size, uint64_t align);
शून्य *RPDEFVIS _ZnwmSt11align_val_t(uint64_t size, uint64_t align) {
  प्रत्यागम rpaligned_alloc(align, size);
}
बाह्य शून्य *_ZnamSt11align_val_t(uint64_t size, uint64_t align);
शून्य *RPDEFVIS _ZnamSt11align_val_t(uint64_t size, uint64_t align) {
  प्रत्यागम rpaligned_alloc(align, size);
}
बाह्य शून्य *_ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
शून्य *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpmalloc(size);
}
बाह्य शून्य *_ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
शून्य *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpmalloc(size);
}
बाह्य शून्य *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
शून्य *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpaligned_alloc(align, size);
}
बाह्य शून्य *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
शून्य *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpaligned_alloc(align, size);
}
// 64-bit operators sized delete and delete[], normal and aligned
बाह्य शून्य _ZdlPvm(शून्य *p, uint64_t size);
शून्य RPDEFVIS _ZdlPvm(शून्य *p, uint64_t size) {
  rpfree(p);
  (शून्य)परिमाण(size);
}
बाह्य शून्य _ZdaPvm(शून्य *p, uint64_t size);
शून्य RPDEFVIS _ZdaPvm(शून्य *p, uint64_t size) {
  rpfree(p);
  (शून्य)परिमाण(size);
}
बाह्य शून्य _ZdlPvSt11align_val_t(शून्य *p, uint64_t align);
शून्य RPDEFVIS _ZdlPvSt11align_val_t(शून्य *p, uint64_t align) {
  rpfree(p);
  (शून्य)परिमाण(align);
}
बाह्य शून्य _ZdaPvSt11align_val_t(शून्य *p, uint64_t align);
शून्य RPDEFVIS _ZdaPvSt11align_val_t(शून्य *p, uint64_t align) {
  rpfree(p);
  (शून्य)परिमाण(align);
}
बाह्य शून्य _ZdlPvmSt11align_val_t(शून्य *p, uint64_t size, uint64_t align);
शून्य RPDEFVIS _ZdlPvmSt11align_val_t(शून्य *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (शून्य)परिमाण(size);
  (शून्य)परिमाण(align);
}
बाह्य शून्य _ZdaPvmSt11align_val_t(शून्य *p, uint64_t size, uint64_t align);
शून्य RPDEFVIS _ZdaPvmSt11align_val_t(शून्य *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (शून्य)परिमाण(size);
  (शून्य)परिमाण(align);
}
#अन्यथा
// 32-bit operators new and new[], normal and aligned
बाह्य शून्य *_Znwj(uint32_t size);
शून्य *RPDEFVIS _Znwj(uint32_t size) { प्रत्यागम rpmalloc(size); }
बाह्य शून्य *_Znaj(uint32_t size);
शून्य *RPDEFVIS _Znaj(uint32_t size) { प्रत्यागम rpmalloc(size); }
बाह्य शून्य *_Znwjj(uint32_t size, uint32_t align);
शून्य *RPDEFVIS _Znwjj(uint32_t size, uint32_t align) {
  प्रत्यागम rpaligned_alloc(align, size);
}
बाह्य शून्य *_Znajj(uint32_t size, uint32_t align);
शून्य *RPDEFVIS _Znajj(uint32_t size, uint32_t align) {
  प्रत्यागम rpaligned_alloc(align, size);
}
बाह्य शून्य *_ZnwjSt11align_val_t(size_t size, size_t align);
शून्य *RPDEFVIS _ZnwjSt11align_val_t(size_t size, size_t align) {
  प्रत्यागम rpaligned_alloc(align, size);
}
बाह्य शून्य *_ZnajSt11align_val_t(size_t size, size_t align);
शून्य *RPDEFVIS _ZnajSt11align_val_t(size_t size, size_t align) {
  प्रत्यागम rpaligned_alloc(align, size);
}
बाह्य शून्य *_ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t);
शून्य *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpmalloc(size);
}
बाह्य शून्य *_ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t);
शून्य *RPDEFVIS _ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpmalloc(size);
}
बाह्य शून्य *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
शून्य *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpaligned_alloc(align, size);
}
बाह्य शून्य *_ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
शून्य *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpaligned_alloc(align, size);
}
// 32-bit operators sized delete and delete[], normal and aligned
बाह्य शून्य _ZdlPvj(शून्य *p, uint64_t size);
शून्य RPDEFVIS _ZdlPvj(शून्य *p, uint64_t size) {
  rpfree(p);
  (शून्य)परिमाण(size);
}
बाह्य शून्य _ZdaPvj(शून्य *p, uint64_t size);
शून्य RPDEFVIS _ZdaPvj(शून्य *p, uint64_t size) {
  rpfree(p);
  (शून्य)परिमाण(size);
}
बाह्य शून्य _ZdlPvSt11align_val_t(शून्य *p, uint32_t align);
शून्य RPDEFVIS _ZdlPvSt11align_val_t(शून्य *p, uint64_t a) {
  rpfree(p);
  (शून्य)परिमाण(align);
}
बाह्य शून्य _ZdaPvSt11align_val_t(शून्य *p, uint32_t align);
शून्य RPDEFVIS _ZdaPvSt11align_val_t(शून्य *p, uint64_t a) {
  rpfree(p);
  (शून्य)परिमाण(align);
}
बाह्य शून्य _ZdlPvjSt11align_val_t(शून्य *p, uint32_t size, uint32_t align);
शून्य RPDEFVIS _ZdlPvjSt11align_val_t(शून्य *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (शून्य)परिमाण(size);
  (शून्य)परिमाण(a);
}
बाह्य शून्य _ZdaPvjSt11align_val_t(शून्य *p, uint32_t size, uint32_t align);
शून्य RPDEFVIS _ZdaPvjSt11align_val_t(शून्य *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (शून्य)परिमाण(size);
  (शून्य)परिमाण(a);
}
#endif
#endif
#endif

#यदि USE_INTERPOSE || USE_ALIAS

स्थिर शून्य *rpmalloc_nothrow(size_t size, rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpmalloc(size);
}
स्थिर शून्य *rpaligned_alloc_reverse(size_t size, size_t align) {
  प्रत्यागम rpaligned_alloc(align, size);
}
स्थिर शून्य *rpaligned_alloc_reverse_nothrow(size_t size, size_t align,
                                             rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpaligned_alloc(align, size);
}
स्थिर शून्य rpfree_size(शून्य *p, size_t size) {
  (शून्य)परिमाण(size);
  rpfree(p);
}
स्थिर शून्य rpfree_aligned(शून्य *p, size_t align) {
  (शून्य)परिमाण(align);
  rpfree(p);
}
स्थिर शून्य rpfree_size_aligned(शून्य *p, size_t size, size_t align) {
  (शून्य)परिमाण(size);
  (शून्य)परिमाण(align);
  rpfree(p);
}

#endif

#यदि USE_INTERPOSE

__attribute__((used)) स्थिर ध्रुव interpose_t macinterpose_malloc[]
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
#यदि defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#endif
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, मुक्त), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_usable_size),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_size)};

#endif

#यदि USE_ALIAS

#define RPALIAS(fn) __attribute__((alias(#fn), used, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
शून्य _ZdlPv(शून्य *p) RPALIAS(rpfree) शून्य _ZdaPv(शून्य *p) RPALIAS(rpfree)

#यदि ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    शून्य *_Znwm(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) शून्य *_Znam(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) शून्य *_Znwmm(uint64_t size,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) शून्य *_Znamm(uint64_t size,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) शून्य *_ZnwmSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) शून्य *_ZnamSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) शून्य *_ZnwmRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) शून्य *_ZnamRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) शून्य
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) शून्य
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    शून्य _ZdlPvm(शून्य *p, size_t n) RPALIAS(rpfree_size) शून्य _ZdaPvm(शून्य *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) शून्य _ZdlPvSt11align_val_t(शून्य *p, size_t a)
            RPALIAS(rpfree_aligned) शून्य _ZdaPvSt11align_val_t(शून्य *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) शून्य _ZdlPvmSt11align_val_t(शून्य *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) शून्य _ZdaPvmSt11align_val_t(
                        शून्य *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#अन्यथा
    // 32-bit operators new and new[], normal and aligned
    शून्य *_Znwj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) शून्य *_Znaj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) शून्य *_Znwjj(uint32_t size,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) शून्य *_Znajj(uint32_t size,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) शून्य *_ZnwjSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) शून्य *_ZnajSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) शून्य *_ZnwjRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) शून्य *_ZnajRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) शून्य
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) शून्य
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    शून्य _ZdlPvj(शून्य *p, size_t n) RPALIAS(rpfree_size) शून्य _ZdaPvj(शून्य *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) शून्य _ZdlPvSt11align_val_t(शून्य *p, size_t a)
            RPALIAS(rpfree_aligned) शून्य _ZdaPvSt11align_val_t(शून्य *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) शून्य _ZdlPvjSt11align_val_t(शून्य *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) शून्य _ZdaPvjSt11align_val_t(
                        शून्य *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#endif

                            शून्य *malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) शून्य *calloc(size_t गिनती, size_t size)
            RPALIAS(rpcalloc) शून्य *realloc(शून्य *ptr, size_t size)
                RPALIAS(rprealloc) शून्य *reallocf(शून्य *ptr, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rprealloc) शून्य *aligned_alloc(size_t alignment, size_t size)
            RPALIAS(rpaligned_alloc) शून्य *memalign(
                size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) पूर्णाङ्क posix_memalign(शून्य **memptr, size_t alignment,
                                               size_t size)
            RPALIAS(rpposix_memalign) शून्य मुक्त(शून्य *ptr)
                RPALIAS(rpfree) शून्य cfree(शून्य *ptr) RPALIAS(rpfree)
#यदि defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_size(ध्रुव शून्य *ptr) RPALIAS(rpmalloc_usable_size)
#अन्यथा
                    size_t
    malloc_usable_size(शून्य *ptr) RPALIAS(rpmalloc_usable_size)
#endif
        size_t malloc_size(शून्य *ptr) RPALIAS(rpmalloc_usable_size)

#endif

            स्थिर अन्तःस्थ size_t _rpmalloc_page_size(शून्य) {
  प्रत्यागम _memory_page_size;
}

बाह्य शून्य *RPMALLOC_CDECL reallocarray(शून्य *ptr, size_t गिनती, size_t size);

बाह्य शून्य *RPMALLOC_CDECL reallocarray(शून्य *ptr, size_t गिनती, size_t size) {
  size_t total;
#यदि ENABLE_VALIDATE_ARGS
#ifdef _MSC_VER
  पूर्णाङ्क err = SizeTMult(गिनती, size, &total);
  यदि ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#अन्यथा
  पूर्णाङ्क err = __builtin_umull_overflow(गिनती, size, &total);
  यदि (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#endif
#अन्यथा
  total = गिनती * size;
#endif
  प्रत्यागम realloc(ptr, total);
}

बाह्य अन्तःस्थ शून्य *RPMALLOC_CDECL valloc(size_t size) {
  get_thread_heap();
  प्रत्यागम rpaligned_alloc(_rpmalloc_page_size(), size);
}

बाह्य अन्तःस्थ शून्य *RPMALLOC_CDECL pvalloc(size_t size) {
  get_thread_heap();
  ध्रुव size_t page_size = _rpmalloc_page_size();
  ध्रुव size_t aligned_size = ((size + page_size - 1) / page_size) * page_size;
#यदि ENABLE_VALIDATE_ARGS
  यदि (aligned_size < size) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#endif
  प्रत्यागम rpaligned_alloc(_rpmalloc_page_size(), aligned_size);
}

#endif // ENABLE_OVERRIDE

#यदि ENABLE_PRELOAD

#ifdef _WIN32

#यदि defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

बाह्य __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

बाह्य __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (शून्य)परिमाण(reserved);
  (शून्य)परिमाण(instance);
  यदि (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  अन्यथा यदि (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  अन्यथा यदि (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  अन्यथा यदि (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  प्रत्यागम TRUE;
}

// end BUILD_DYNAMIC_LINK
#अन्यथा

बाह्य शून्य _global_rpmalloc_init(शून्य) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
}

#यदि defined(__clang__) || defined(__GNUC__)

स्थिर शून्य __attribute__((constructor)) initializer(शून्य) {
  _global_rpmalloc_init();
}

#नोचेद्यदि defined(_MSC_VER)

स्थिर पूर्णाङ्क _global_rpmalloc_xib(शून्य) {
  _global_rpmalloc_init();
  प्रत्यागम 0;
}

#pragma section(".CRT$XIB", पठ)
__declspec(allocate(".CRT$XIB")) शून्य (*_rpmalloc_module_init)(शून्य) =
    _global_rpmalloc_xib;
#यदि defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#अन्यथा
#pragma comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#endif

#endif

// end !BUILD_DYNAMIC_LINK
#endif

#अन्यथा

#अन्तर्भाव <pthread.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <unistd.h>

बाह्य शून्य rpmalloc_set_main_thread(शून्य);

स्थिर pthread_key_t destructor_कुंजी;

स्थिर शून्य thread_destructor(शून्य *);

स्थिर शून्य __attribute__((constructor)) initializer(शून्य) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
  pthread_कुंजी_create(&destructor_कुंजी, thread_destructor);
}

स्थिर शून्य __attribute__((destructor)) finalizer(शून्य) { rpmalloc_finalize(); }

प्रकारनिर्देश संरचना {
  शून्य *(*real_start)(शून्य *);
  शून्य *real_arg;
} thread_starter_arg;

स्थिर शून्य *thread_starter(शून्य *argptr) {
  thread_starter_arg *arg = argptr;
  शून्य *(*real_start)(शून्य *) = arg->real_start;
  शून्य *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_कुंजी, (शून्य *)1);
  प्रत्यागम (*real_start)(real_arg);
}

स्थिर शून्य thread_destructor(शून्य *मूल्य_2) {
  (शून्य)परिमाण(मूल्य_2);
  rpmalloc_thread_finalize(1);
}

#ifdef __APPLE__

स्थिर पूर्णाङ्क pthread_create_proxy(pthread_t *thread, ध्रुव pthread_attr_t *attr,
                                शून्य *(*start_routine)(शून्य *), शून्य *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(परिमाण(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  प्रत्यागम pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#अन्यथा

#अन्तर्भाव <dlfcn.h>

पूर्णाङ्क pthread_create(pthread_t *thread, ध्रुव pthread_attr_t *attr,
                   शून्य *(*start_routine)(शून्य *), शून्य *arg) {
#यदि defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  वर्ण fname[] = "pthread_create";
#अन्यथा
  वर्ण fname[] = "_pthread_create";
#endif
  शून्य *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(परिमाण(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  प्रत्यागम (*(पूर्णाङ्क (*)(pthread_t *, ध्रुव pthread_attr_t *, शून्य *(*)(शून्य *),
                    शून्य *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#endif

#endif

#endif

#यदि ENABLE_OVERRIDE

#यदि defined(__GLIBC__) && defined(__linux__)

शून्य *__libc_malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) शून्य *__libc_calloc(size_t गिनती, size_t size)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) शून्य *__libc_realloc(शून्य *p, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2) RPALIAS(rprealloc) शून्य __libc_free(शून्य *p)
        RPALIAS(rpfree) शून्य __libc_cfree(शून्य *p)
            RPALIAS(rpfree) शून्य *__libc_memalign(size_t align, size_t size)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) पूर्णाङ्क __posix_memalign(शून्य **p, size_t align,
                                                 size_t size)
            RPALIAS(rpposix_memalign)

                बाह्य शून्य *__libc_valloc(size_t size);
बाह्य शून्य *__libc_pvalloc(size_t size);

शून्य *__libc_valloc(size_t size) { प्रत्यागम valloc(size); }

शून्य *__libc_pvalloc(size_t size) { प्रत्यागम pvalloc(size); }

#endif

#endif

#यदि (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility निकालो
#endif
