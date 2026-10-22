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
#यदि_अन्तः

#यदि_अपरिभाषितम् ARCH_64BIT
#यदि defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#परिभाषय ARCH_64BIT 1
_Static_assert(परिमाण(size_t) == 8, "Data type size mismatch");
_Static_assert(परिमाण(शून्य *) == 8, "Data type size mismatch");
#अन्यथा
#परिभाषय ARCH_64BIT 0
_Static_assert(परिमाण(size_t) == 4, "Data type size mismatch");
_Static_assert(परिमाण(शून्य *) == 4, "Data type size mismatch");
#यदि_अन्तः
#यदि_अन्तः

#यदि (defined(__GNUC__) || defined(__clang__))
#निर्देशः GCC visibility प्रक्षिप(स्वभाव)
#यदि_अन्तः

#परिभाषय USE_IMPLEMENT 1
#परिभाषय USE_INTERPOSE 0
#परिभाषय USE_ALIAS 0

#यदि defined(__APPLE__)
#अपरिभाषय USE_INTERPOSE
#परिभाषय USE_INTERPOSE 1

प्रकारनिर्देश संरचना interpose_t {
  शून्य *new_func;
  शून्य *orig_func;
} interpose_t;

#परिभाषय MAC_INTERPOSE_PAIR(newf, oldf) {(शून्य *)newf, (शून्य *)oldf}
#परिभाषय MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((प्रयुक्तम्)) स्थिर ध्रुव interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#यदि_अन्तः

#यदि !defined(_WIN32) && !defined(__APPLE__)
#अपरिभाषय USE_IMPLEMENT
#अपरिभाषय USE_ALIAS
#परिभाषय USE_IMPLEMENT 0
#परिभाषय USE_ALIAS 1
#यदि_अन्तः

#यदि_परिभाषितम् _MSC_VER
#निर्देशः warning(disable : 4100)
#अपरिभाषय स्मृतिं_देहि
#अपरिभाषय मुक्त
#अपरिभाषय शून्यस्मृतिं_देहि
#परिभाषय RPMALLOC_RESTRICT __declspec(सीमित)
#अन्यथा
#परिभाषय RPMALLOC_RESTRICT
#यदि_अन्तः

#यदि ENABLE_OVERRIDE

प्रकारनिर्देश संरचना rp_nothrow_t {
  पूर्णाङ्क __dummy;
} rp_nothrow_t;

#यदि USE_IMPLEMENT

बाह्य अन्तःस्थ RPMALLOC_RESTRICT शून्य *RPMALLOC_CDECL स्मृतिं_देहि(size_t आकार_2) {
  प्रत्यागम rpmalloc(आकार_2);
}
बाह्य अन्तःस्थ RPMALLOC_RESTRICT शून्य *RPMALLOC_CDECL शून्यस्मृतिं_देहि(size_t गणनम्,
                                                            size_t आकार_2) {
  प्रत्यागम rpcalloc(गणनम्, आकार_2);
}
बाह्य अन्तःस्थ RPMALLOC_RESTRICT शून्य *RPMALLOC_CDECL realloc(शून्य *ptr,
                                                             size_t आकार_2) {
  प्रत्यागम rprealloc(ptr, आकार_2);
}
बाह्य अन्तःस्थ शून्य *RPMALLOC_CDECL reallocf(शून्य *ptr, size_t आकार_2) {
  प्रत्यागम rprealloc(ptr, आकार_2);
}
बाह्य अन्तःस्थ शून्य *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t आकार_2) {
  प्रत्यागम rpaligned_alloc(alignment, आकार_2);
}
बाह्य अन्तःस्थ शून्य *RPMALLOC_CDECL memalign(size_t alignment, size_t आकार_2) {
  प्रत्यागम rpmemalign(alignment, आकार_2);
}
बाह्य अन्तःस्थ पूर्णाङ्क RPMALLOC_CDECL posix_memalign(शून्य **memptr, size_t alignment,
                                                size_t आकार_2) {
  प्रत्यागम rpposix_memalign(memptr, alignment, आकार_2);
}
बाह्य अन्तःस्थ शून्य RPMALLOC_CDECL मुक्त(शून्य *ptr) { rpfree(ptr); }
बाह्य अन्तःस्थ शून्य RPMALLOC_CDECL cfree(शून्य *ptr) { rpfree(ptr); }
बाह्य अन्तःस्थ size_t RPMALLOC_CDECL malloc_usable_आकार(शून्य *ptr) {
  प्रत्यागम rpmalloc_usable_आकार(ptr);
}
बाह्य अन्तःस्थ size_t RPMALLOC_CDECL malloc_आकार(शून्य *ptr) {
  प्रत्यागम rpmalloc_usable_आकार(ptr);
}

#यदि_परिभाषितम् _WIN32
बाह्य अन्तःस्थ RPMALLOC_RESTRICT शून्य *RPMALLOC_CDECL malloc_आधारः(size_t आकार_2) {
  प्रत्यागम rpmalloc(आकार_2);
}
बाह्य अन्तःस्थ शून्य RPMALLOC_CDECL free_आधारः(शून्य *ptr) { rpfree(ptr); }
बाह्य अन्तःस्थ RPMALLOC_RESTRICT शून्य *RPMALLOC_CDECL calloc_आधारः(size_t गणनम्,
                                                                  size_t आकार_2) {
  प्रत्यागम rpcalloc(गणनम्, आकार_2);
}
बाह्य अन्तःस्थ size_t RPMALLOC_CDECL _msize(शून्य *ptr) {
  प्रत्यागम rpmalloc_usable_आकार(ptr);
}
बाह्य अन्तःस्थ size_t RPMALLOC_CDECL msize_आधारः(शून्य *ptr) {
  प्रत्यागम rpmalloc_usable_आकार(ptr);
}
बाह्य अन्तःस्थ RPMALLOC_RESTRICT शून्य *RPMALLOC_CDECL
realloc_आधारः(शून्य *ptr, size_t आकार_2) {
  प्रत्यागम rprealloc(ptr, आकार_2);
}
#यदि_अन्तः

#यदि_परिभाषितम् _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#अन्यथा
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#परिभाषय RPDEFVIS __attribute__((visibility("default")))
बाह्य शून्य _ZdlPv(शून्य *p);
शून्य RPDEFVIS _ZdlPv(शून्य *p) { rpfree(p); }
बाह्य शून्य _ZdaPv(शून्य *p);
शून्य RPDEFVIS _ZdaPv(शून्य *p) { rpfree(p); }
#यदि ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
बाह्य शून्य *_Znwm(uint64_t आकार_2);
शून्य *RPDEFVIS _Znwm(uint64_t आकार_2) { प्रत्यागम rpmalloc(आकार_2); }
बाह्य शून्य *_Znam(uint64_t आकार_2);
शून्य *RPDEFVIS _Znam(uint64_t आकार_2) { प्रत्यागम rpmalloc(आकार_2); }
बाह्य शून्य *_Znwmm(uint64_t आकार_2, uint64_t align);
शून्य *RPDEFVIS _Znwmm(uint64_t आकार_2, uint64_t align) {
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
बाह्य शून्य *_Znamm(uint64_t आकार_2, uint64_t align);
शून्य *RPDEFVIS _Znamm(uint64_t आकार_2, uint64_t align) {
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
बाह्य शून्य *_ZnwmSt11align_val_t(uint64_t आकार_2, uint64_t align);
शून्य *RPDEFVIS _ZnwmSt11align_val_t(uint64_t आकार_2, uint64_t align) {
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
बाह्य शून्य *_ZnamSt11align_val_t(uint64_t आकार_2, uint64_t align);
शून्य *RPDEFVIS _ZnamSt11align_val_t(uint64_t आकार_2, uint64_t align) {
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
बाह्य शून्य *_ZnwmRKSt9nothrow_t(uint64_t आकार_2, rp_nothrow_t t);
शून्य *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t आकार_2, rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpmalloc(आकार_2);
}
बाह्य शून्य *_ZnamRKSt9nothrow_t(uint64_t आकार_2, rp_nothrow_t t);
शून्य *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t आकार_2, rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpmalloc(आकार_2);
}
बाह्य शून्य *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t आकार_2, uint64_t align,
                                                rp_nothrow_t t);
शून्य *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t आकार_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
बाह्य शून्य *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t आकार_2, uint64_t align,
                                                rp_nothrow_t t);
शून्य *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t आकार_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
// 64-bit operators sized delete and delete[], normal and aligned
बाह्य शून्य _ZdlPvm(शून्य *p, uint64_t आकार_2);
शून्य RPDEFVIS _ZdlPvm(शून्य *p, uint64_t आकार_2) {
  rpfree(p);
  (शून्य)परिमाण(आकार_2);
}
बाह्य शून्य _ZdaPvm(शून्य *p, uint64_t आकार_2);
शून्य RPDEFVIS _ZdaPvm(शून्य *p, uint64_t आकार_2) {
  rpfree(p);
  (शून्य)परिमाण(आकार_2);
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
बाह्य शून्य _ZdlPvmSt11align_val_t(शून्य *p, uint64_t आकार_2, uint64_t align);
शून्य RPDEFVIS _ZdlPvmSt11align_val_t(शून्य *p, uint64_t आकार_2, uint64_t align) {
  rpfree(p);
  (शून्य)परिमाण(आकार_2);
  (शून्य)परिमाण(align);
}
बाह्य शून्य _ZdaPvmSt11align_val_t(शून्य *p, uint64_t आकार_2, uint64_t align);
शून्य RPDEFVIS _ZdaPvmSt11align_val_t(शून्य *p, uint64_t आकार_2, uint64_t align) {
  rpfree(p);
  (शून्य)परिमाण(आकार_2);
  (शून्य)परिमाण(align);
}
#अन्यथा
// 32-bit operators new and new[], normal and aligned
बाह्य शून्य *_Znwj(uint32_t आकार_2);
शून्य *RPDEFVIS _Znwj(uint32_t आकार_2) { प्रत्यागम rpmalloc(आकार_2); }
बाह्य शून्य *_Znaj(uint32_t आकार_2);
शून्य *RPDEFVIS _Znaj(uint32_t आकार_2) { प्रत्यागम rpmalloc(आकार_2); }
बाह्य शून्य *_Znwjj(uint32_t आकार_2, uint32_t align);
शून्य *RPDEFVIS _Znwjj(uint32_t आकार_2, uint32_t align) {
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
बाह्य शून्य *_Znajj(uint32_t आकार_2, uint32_t align);
शून्य *RPDEFVIS _Znajj(uint32_t आकार_2, uint32_t align) {
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
बाह्य शून्य *_ZnwjSt11align_val_t(size_t आकार_2, size_t align);
शून्य *RPDEFVIS _ZnwjSt11align_val_t(size_t आकार_2, size_t align) {
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
बाह्य शून्य *_ZnajSt11align_val_t(size_t आकार_2, size_t align);
शून्य *RPDEFVIS _ZnajSt11align_val_t(size_t आकार_2, size_t align) {
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
बाह्य शून्य *_ZnwjRKSt9nothrow_t(size_t आकार_2, rp_nothrow_t t);
शून्य *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t आकार_2, rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpmalloc(आकार_2);
}
बाह्य शून्य *_ZnajRKSt9nothrow_t(size_t आकार_2, rp_nothrow_t t);
शून्य *RPDEFVIS _ZnajRKSt9nothrow_t(size_t आकार_2, rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpmalloc(आकार_2);
}
बाह्य शून्य *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t आकार_2, size_t align,
                                                rp_nothrow_t t);
शून्य *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t आकार_2, size_t align,
                                                  rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
बाह्य शून्य *_ZnajSt11align_val_tRKSt9nothrow_t(size_t आकार_2, size_t align,
                                                rp_nothrow_t t);
शून्य *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t आकार_2, size_t align,
                                                  rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
// 32-bit operators sized delete and delete[], normal and aligned
बाह्य शून्य _ZdlPvj(शून्य *p, uint64_t आकार_2);
शून्य RPDEFVIS _ZdlPvj(शून्य *p, uint64_t आकार_2) {
  rpfree(p);
  (शून्य)परिमाण(आकार_2);
}
बाह्य शून्य _ZdaPvj(शून्य *p, uint64_t आकार_2);
शून्य RPDEFVIS _ZdaPvj(शून्य *p, uint64_t आकार_2) {
  rpfree(p);
  (शून्य)परिमाण(आकार_2);
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
बाह्य शून्य _ZdlPvjSt11align_val_t(शून्य *p, uint32_t आकार_2, uint32_t align);
शून्य RPDEFVIS _ZdlPvjSt11align_val_t(शून्य *p, uint64_t आकार_2, uint64_t align) {
  rpfree(p);
  (शून्य)परिमाण(आकार_2);
  (शून्य)परिमाण(a);
}
बाह्य शून्य _ZdaPvjSt11align_val_t(शून्य *p, uint32_t आकार_2, uint32_t align);
शून्य RPDEFVIS _ZdaPvjSt11align_val_t(शून्य *p, uint64_t आकार_2, uint64_t align) {
  rpfree(p);
  (शून्य)परिमाण(आकार_2);
  (शून्य)परिमाण(a);
}
#यदि_अन्तः
#यदि_अन्तः
#यदि_अन्तः

#यदि USE_INTERPOSE || USE_ALIAS

स्थिर शून्य *rpmalloc_nothrow(size_t आकार_2, rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpmalloc(आकार_2);
}
स्थिर शून्य *rpaligned_alloc_reverse(size_t आकार_2, size_t align) {
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
स्थिर शून्य *rpaligned_alloc_reverse_nothrow(size_t आकार_2, size_t align,
                                             rp_nothrow_t t) {
  (शून्य)परिमाण(t);
  प्रत्यागम rpaligned_alloc(align, आकार_2);
}
स्थिर शून्य rpfree_आकार(शून्य *p, size_t आकार_2) {
  (शून्य)परिमाण(आकार_2);
  rpfree(p);
}
स्थिर शून्य rpfree_aligned(शून्य *p, size_t align) {
  (शून्य)परिमाण(align);
  rpfree(p);
}
स्थिर शून्य rpfree_आकार_aligned(शून्य *p, size_t आकार_2, size_t align) {
  (शून्य)परिमाण(आकार_2);
  (शून्य)परिमाण(align);
  rpfree(p);
}

#यदि_अन्तः

#यदि USE_INTERPOSE

__attribute__((प्रयुक्तम्)) स्थिर ध्रुव interpose_t macinterpose_malloc[]
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
        MAC_INTERPOSE_PAIR(rpfree_आकार, _ZdlPvm),
        MAC_INTERPOSE_PAIR(rpfree_आकार, _ZdaPvm),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdlPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdaPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_आकार_aligned, _ZdlPvmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_आकार_aligned, _ZdaPvmSt11align_val_t),
        // libc entry points
        MAC_INTERPOSE_PAIR(rpmalloc, स्मृतिं_देहि),
        MAC_INTERPOSE_PAIR(rpmalloc, शून्यस्मृतिं_देहि),
        MAC_INTERPOSE_PAIR(rprealloc, realloc),
        MAC_INTERPOSE_PAIR(rprealloc, reallocf),
#यदि defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#यदि_अन्तः
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, मुक्त), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_आकार, malloc_usable_आकार),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_आकार, malloc_आकार)};

#यदि_अन्तः

#यदि USE_ALIAS

#परिभाषय RPALIAS(fn) __attribute__((alias(#fn), प्रयुक्तम्, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
शून्य _ZdlPv(शून्य *p) RPALIAS(rpfree) शून्य _ZdaPv(शून्य *p) RPALIAS(rpfree)

#यदि ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    शून्य *_Znwm(uint64_t आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(1)
        RPALIAS(rpmalloc) शून्य *_Znam(uint64_t आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(1) RPALIAS(rpmalloc) शून्य *_Znwmm(uint64_t आकार_2,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) शून्य *_Znamm(uint64_t आकार_2,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) शून्य *_ZnwmSt11align_val_t(
                size_t आकार_2, size_t align)
                RPALIAS(rpaligned_alloc_reverse) शून्य *_ZnamSt11align_val_t(
                    size_t आकार_2, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) शून्य *_ZnwmRKSt9nothrow_t(
                        size_t आकार_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) शून्य *_ZnamRKSt9nothrow_t(
                            size_t आकार_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) शून्य
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t आकार_2,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) शून्य
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t आकार_2, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    शून्य _ZdlPvm(शून्य *p, size_t n) RPALIAS(rpfree_आकार) शून्य _ZdaPvm(शून्य *p,
                                                                      size_t n)
        RPALIAS(rpfree_आकार) शून्य _ZdlPvSt11align_val_t(शून्य *p, size_t a)
            RPALIAS(rpfree_aligned) शून्य _ZdaPvSt11align_val_t(शून्य *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) शून्य _ZdlPvmSt11align_val_t(शून्य *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_आकार_aligned) शून्य _ZdaPvmSt11align_val_t(
                        शून्य *p, size_t n, size_t a)
                        RPALIAS(rpfree_आकार_aligned)
#अन्यथा
    // 32-bit operators new and new[], normal and aligned
    शून्य *_Znwj(uint32_t आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(1)
        RPALIAS(rpmalloc) शून्य *_Znaj(uint32_t आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(1) RPALIAS(rpmalloc) शून्य *_Znwjj(uint32_t आकार_2,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) शून्य *_Znajj(uint32_t आकार_2,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) शून्य *_ZnwjSt11align_val_t(
                size_t आकार_2, size_t align)
                RPALIAS(rpaligned_alloc_reverse) शून्य *_ZnajSt11align_val_t(
                    size_t आकार_2, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) शून्य *_ZnwjRKSt9nothrow_t(
                        size_t आकार_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) शून्य *_ZnajRKSt9nothrow_t(
                            size_t आकार_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) शून्य
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t आकार_2,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) शून्य
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t आकार_2, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    शून्य _ZdlPvj(शून्य *p, size_t n) RPALIAS(rpfree_आकार) शून्य _ZdaPvj(शून्य *p,
                                                                      size_t n)
        RPALIAS(rpfree_आकार) शून्य _ZdlPvSt11align_val_t(शून्य *p, size_t a)
            RPALIAS(rpfree_aligned) शून्य _ZdaPvSt11align_val_t(शून्य *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) शून्य _ZdlPvjSt11align_val_t(शून्य *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_आकार_aligned) शून्य _ZdaPvjSt11align_val_t(
                        शून्य *p, size_t n, size_t a)
                        RPALIAS(rpfree_आकार_aligned)
#यदि_अन्तः

                            शून्य *स्मृतिं_देहि(size_t आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(1)
        RPALIAS(rpmalloc) शून्य *शून्यस्मृतिं_देहि(size_t गणनम्, size_t आकार_2)
            RPALIAS(rpcalloc) शून्य *realloc(शून्य *ptr, size_t आकार_2)
                RPALIAS(rprealloc) शून्य *reallocf(शून्य *ptr, size_t आकार_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(2)
        RPALIAS(rprealloc) शून्य *aligned_alloc(size_t alignment, size_t आकार_2)
            RPALIAS(rpaligned_alloc) शून्य *memalign(
                size_t alignment, size_t आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(2)
        RPALIAS(rpmemalign) पूर्णाङ्क posix_memalign(शून्य **memptr, size_t alignment,
                                               size_t आकार_2)
            RPALIAS(rpposix_memalign) शून्य मुक्त(शून्य *ptr)
                RPALIAS(rpfree) शून्य cfree(शून्य *ptr) RPALIAS(rpfree)
#यदि defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_आकार(ध्रुव शून्य *ptr) RPALIAS(rpmalloc_usable_आकार)
#अन्यथा
                    size_t
    malloc_usable_आकार(शून्य *ptr) RPALIAS(rpmalloc_usable_आकार)
#यदि_अन्तः
        size_t malloc_आकार(शून्य *ptr) RPALIAS(rpmalloc_usable_आकार)

#यदि_अन्तः

            स्थिर अन्तःस्थ size_t rpmalloc_page_आकार(शून्य) {
  प्रत्यागम स्मृतिः_page_आकार;
}

बाह्य शून्य *RPMALLOC_CDECL reallocarray(शून्य *ptr, size_t गणनम्, size_t आकार_2);

बाह्य शून्य *RPMALLOC_CDECL reallocarray(शून्य *ptr, size_t गणनम्, size_t आकार_2) {
  size_t योग_3;
#यदि ENABLE_VALIDATE_तर्काः
#यदि_परिभाषितम् _MSC_VER
  पूर्णाङ्क err = आकार_TMult(गणनम्, आकार_2, &योग_3);
  यदि ((err != S_OK) || (योग_3 >= MAX_ALLOC_आकार)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#अन्यथा
  पूर्णाङ्क err = __builtin_umull_overflow(गणनम्, आकार_2, &योग_3);
  यदि (err || (योग_3 >= MAX_ALLOC_आकार)) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#यदि_अन्तः
#अन्यथा
  योग_3 = गणनम् * आकार_2;
#यदि_अन्तः
  प्रत्यागम realloc(ptr, योग_3);
}

बाह्य अन्तःस्थ शून्य *RPMALLOC_CDECL valloc(size_t आकार_2) {
  get_thread_heap();
  प्रत्यागम rpaligned_alloc(rpmalloc_page_आकार(), आकार_2);
}

बाह्य अन्तःस्थ शून्य *RPMALLOC_CDECL pvalloc(size_t आकार_2) {
  get_thread_heap();
  ध्रुव size_t page_आकार = rpmalloc_page_आकार();
  ध्रुव size_t aligned_आकार = ((आकार_2 + page_आकार - 1) / page_आकार) * page_आकार;
#यदि ENABLE_VALIDATE_तर्काः
  यदि (aligned_आकार < आकार_2) {
    errno = EINVAL;
    प्रत्यागम 0;
  }
#यदि_अन्तः
  प्रत्यागम rpaligned_alloc(rpmalloc_page_आकार(), aligned_आकार);
}

#यदि_अन्तः // ENABLE_OVERRIDE

#यदि ENABLE_PRELOAD

#यदि_परिभाषितम् _WIN32

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
  rpmalloc_समुच्चयः_main_thread();
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

#निर्देशः section(".CRT$XIB", पठ)
__declspec(allocate(".CRT$XIB")) शून्य (*_rpmalloc_module_init)(शून्य) =
    _global_rpmalloc_xib;
#यदि defined(_M_IX86) || defined(__i386__)
#निर्देशः comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#अन्यथा
#निर्देशः comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#यदि_अन्तः

#यदि_अन्तः

// end !BUILD_DYNAMIC_LINK
#यदि_अन्तः

#अन्यथा

#अन्तर्भाव <pthread.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <unistd.h>

बाह्य शून्य rpmalloc_समुच्चयः_main_thread(शून्य);

स्थिर pthread_key_t destructor_कुञ्चिका;

स्थिर शून्य thread_destructor(शून्य *);

स्थिर शून्य __attribute__((constructor)) initializer(शून्य) {
  rpmalloc_समुच्चयः_main_thread();
  rpmalloc_initialize();
  pthread_कुञ्चिका_create(&destructor_कुञ्चिका, thread_destructor);
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
  pthread_setspecific(destructor_कुञ्चिका, (शून्य *)1);
  प्रत्यागम (*real_start)(real_arg);
}

स्थिर शून्य thread_destructor(शून्य *मानम्_2) {
  (शून्य)परिमाण(मानम्_2);
  rpmalloc_thread_finalize(1);
}

#यदि_परिभाषितम् __APPLE__

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
#यदि_अन्तः
  शून्य *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(परिमाण(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  प्रत्यागम (*(पूर्णाङ्क (*)(pthread_t *, ध्रुव pthread_attr_t *, शून्य *(*)(शून्य *),
                    शून्य *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#यदि_अन्तः

#यदि_अन्तः

#यदि_अन्तः

#यदि ENABLE_OVERRIDE

#यदि defined(__GLIBC__) && defined(__linux__)

शून्य *__libc_malloc(size_t आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(1)
        RPALIAS(rpmalloc) शून्य *__libc_calloc(size_t गणनम्, size_t आकार_2)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) शून्य *__libc_realloc(शून्य *p, size_t आकार_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(2) RPALIAS(rprealloc) शून्य __libc_free(शून्य *p)
        RPALIAS(rpfree) शून्य __libc_cfree(शून्य *p)
            RPALIAS(rpfree) शून्य *__libc_memalign(size_t align, size_t आकार_2)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(2)
        RPALIAS(rpmemalign) पूर्णाङ्क __posix_memalign(शून्य **p, size_t align,
                                                 size_t आकार_2)
            RPALIAS(rpposix_memalign)

                बाह्य शून्य *__libc_valloc(size_t आकार_2);
बाह्य शून्य *__libc_pvalloc(size_t आकार_2);

शून्य *__libc_valloc(size_t आकार_2) { प्रत्यागम valloc(आकार_2); }

शून्य *__libc_pvalloc(size_t आकार_2) { प्रत्यागम pvalloc(आकार_2); }

#यदि_अन्तः

#यदि_अन्तः

#यदि (defined(__GNUC__) || defined(__clang__))
#निर्देशः GCC visibility निष्कासय
#यदि_अन्तः
