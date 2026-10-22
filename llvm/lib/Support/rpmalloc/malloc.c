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

#अगर defined(__TINYC__)
#समावेश <sys/types.h>
#पूर्ण_अगर

#अगर_अघोषित ARCH_64BIT
#अगर defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#घोषणा ARCH_64BIT 1
_Static_assert(माप(माप_प्रकार) == 8, "Data type size mismatch");
_Static_assert(माप(व्योम *) == 8, "Data type size mismatch");
#अन्यथा
#घोषणा ARCH_64BIT 0
_Static_assert(माप(माप_प्रकार) == 4, "Data type size mismatch");
_Static_assert(माप(व्योम *) == 4, "Data type size mismatch");
#पूर्ण_अगर
#पूर्ण_अगर

#अगर (defined(__GNUC__) || defined(__clang__))
#आशय GCC visibility धकेलो(शेष)
#पूर्ण_अगर

#घोषणा USE_IMPLEMENT 1
#घोषणा USE_INTERPOSE 0
#घोषणा USE_ALIAS 0

#अगर defined(__APPLE__)
#अघोषित USE_INTERPOSE
#घोषणा USE_INTERPOSE 1

प्रकार काष्ठा interpose_t {
  व्योम *new_func;
  व्योम *orig_func;
} interpose_t;

#घोषणा MAC_INTERPOSE_PAIR(newf, oldf) {(व्योम *)newf, (व्योम *)oldf}
#घोषणा MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((used)) अटल स्थिर interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#पूर्ण_अगर

#अगर !defined(_WIN32) && !defined(__APPLE__)
#अघोषित USE_IMPLEMENT
#अघोषित USE_ALIAS
#घोषणा USE_IMPLEMENT 0
#घोषणा USE_ALIAS 1
#पूर्ण_अगर

#अगर_घोषित _MSC_VER
#आशय warning(disable : 4100)
#अघोषित दो_स्मृति
#अघोषित मुक्त
#अघोषित सुस्मृति
#घोषणा RPMALLOC_RESTRICT __declspec(restrict)
#अन्यथा
#घोषणा RPMALLOC_RESTRICT
#पूर्ण_अगर

#अगर ENABLE_OVERRIDE

प्रकार काष्ठा rp_nothrow_t {
  पूर्णांक __dummy;
} rp_nothrow_t;

#अगर USE_IMPLEMENT

बाह्य अंतरभूत RPMALLOC_RESTRICT व्योम *RPMALLOC_CDECL दो_स्मृति(माप_प्रकार आकार_2) {
  वापस rpmalloc(आकार_2);
}
बाह्य अंतरभूत RPMALLOC_RESTRICT व्योम *RPMALLOC_CDECL सुस्मृति(माप_प्रकार गिनती,
                                                            माप_प्रकार आकार_2) {
  वापस rpcalloc(गिनती, आकार_2);
}
बाह्य अंतरभूत RPMALLOC_RESTRICT व्योम *RPMALLOC_CDECL पुनः_स्मृति(व्योम *ptr,
                                                             माप_प्रकार आकार_2) {
  वापस rprealloc(ptr, आकार_2);
}
बाह्य अंतरभूत व्योम *RPMALLOC_CDECL reallocf(व्योम *ptr, माप_प्रकार आकार_2) {
  वापस rprealloc(ptr, आकार_2);
}
बाह्य अंतरभूत व्योम *RPMALLOC_CDECL aligned_alloc(माप_प्रकार alignment,
                                                 माप_प्रकार आकार_2) {
  वापस rpaligned_alloc(alignment, आकार_2);
}
बाह्य अंतरभूत व्योम *RPMALLOC_CDECL memalign(माप_प्रकार alignment, माप_प्रकार आकार_2) {
  वापस rpmemalign(alignment, आकार_2);
}
बाह्य अंतरभूत पूर्णांक RPMALLOC_CDECL posix_memalign(व्योम **memptr, माप_प्रकार alignment,
                                                माप_प्रकार आकार_2) {
  वापस rpposix_memalign(memptr, alignment, आकार_2);
}
बाह्य अंतरभूत व्योम RPMALLOC_CDECL मुक्त(व्योम *ptr) { rpfree(ptr); }
बाह्य अंतरभूत व्योम RPMALLOC_CDECL cfree(व्योम *ptr) { rpfree(ptr); }
बाह्य अंतरभूत माप_प्रकार RPMALLOC_CDECL malloc_usable_आकार(व्योम *ptr) {
  वापस rpmalloc_usable_आकार(ptr);
}
बाह्य अंतरभूत माप_प्रकार RPMALLOC_CDECL malloc_आकार(व्योम *ptr) {
  वापस rpmalloc_usable_आकार(ptr);
}

#अगर_घोषित _WIN32
बाह्य अंतरभूत RPMALLOC_RESTRICT व्योम *RPMALLOC_CDECL _malloc_base(माप_प्रकार आकार_2) {
  वापस rpmalloc(आकार_2);
}
बाह्य अंतरभूत व्योम RPMALLOC_CDECL _free_base(व्योम *ptr) { rpfree(ptr); }
बाह्य अंतरभूत RPMALLOC_RESTRICT व्योम *RPMALLOC_CDECL _calloc_base(माप_प्रकार गिनती,
                                                                  माप_प्रकार आकार_2) {
  वापस rpcalloc(गिनती, आकार_2);
}
बाह्य अंतरभूत माप_प्रकार RPMALLOC_CDECL _msize(व्योम *ptr) {
  वापस rpmalloc_usable_आकार(ptr);
}
बाह्य अंतरभूत माप_प्रकार RPMALLOC_CDECL _msize_base(व्योम *ptr) {
  वापस rpmalloc_usable_आकार(ptr);
}
बाह्य अंतरभूत RPMALLOC_RESTRICT व्योम *RPMALLOC_CDECL
_realloc_base(व्योम *ptr, माप_प्रकार आकार_2) {
  वापस rprealloc(ptr, आकार_2);
}
#पूर्ण_अगर

#अगर_घोषित _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#अन्यथा
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#घोषणा RPDEFVIS __attribute__((visibility("default")))
बाह्य व्योम _ZdlPv(व्योम *p);
व्योम RPDEFVIS _ZdlPv(व्योम *p) { rpfree(p); }
बाह्य व्योम _ZdaPv(व्योम *p);
व्योम RPDEFVIS _ZdaPv(व्योम *p) { rpfree(p); }
#अगर ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
बाह्य व्योम *_Znwm(uint64_t आकार_2);
व्योम *RPDEFVIS _Znwm(uint64_t आकार_2) { वापस rpmalloc(आकार_2); }
बाह्य व्योम *_Znam(uint64_t आकार_2);
व्योम *RPDEFVIS _Znam(uint64_t आकार_2) { वापस rpmalloc(आकार_2); }
बाह्य व्योम *_Znwmm(uint64_t आकार_2, uint64_t align);
व्योम *RPDEFVIS _Znwmm(uint64_t आकार_2, uint64_t align) {
  वापस rpaligned_alloc(align, आकार_2);
}
बाह्य व्योम *_Znamm(uint64_t आकार_2, uint64_t align);
व्योम *RPDEFVIS _Znamm(uint64_t आकार_2, uint64_t align) {
  वापस rpaligned_alloc(align, आकार_2);
}
बाह्य व्योम *_ZnwmSt11align_val_t(uint64_t आकार_2, uint64_t align);
व्योम *RPDEFVIS _ZnwmSt11align_val_t(uint64_t आकार_2, uint64_t align) {
  वापस rpaligned_alloc(align, आकार_2);
}
बाह्य व्योम *_ZnamSt11align_val_t(uint64_t आकार_2, uint64_t align);
व्योम *RPDEFVIS _ZnamSt11align_val_t(uint64_t आकार_2, uint64_t align) {
  वापस rpaligned_alloc(align, आकार_2);
}
बाह्य व्योम *_ZnwmRKSt9nothrow_t(uint64_t आकार_2, rp_nothrow_t t);
व्योम *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t आकार_2, rp_nothrow_t t) {
  (व्योम)माप(t);
  वापस rpmalloc(आकार_2);
}
बाह्य व्योम *_ZnamRKSt9nothrow_t(uint64_t आकार_2, rp_nothrow_t t);
व्योम *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t आकार_2, rp_nothrow_t t) {
  (व्योम)माप(t);
  वापस rpmalloc(आकार_2);
}
बाह्य व्योम *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t आकार_2, uint64_t align,
                                                rp_nothrow_t t);
व्योम *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t आकार_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (व्योम)माप(t);
  वापस rpaligned_alloc(align, आकार_2);
}
बाह्य व्योम *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t आकार_2, uint64_t align,
                                                rp_nothrow_t t);
व्योम *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t आकार_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (व्योम)माप(t);
  वापस rpaligned_alloc(align, आकार_2);
}
// 64-bit operators sized delete and delete[], normal and aligned
बाह्य व्योम _ZdlPvm(व्योम *p, uint64_t आकार_2);
व्योम RPDEFVIS _ZdlPvm(व्योम *p, uint64_t आकार_2) {
  rpfree(p);
  (व्योम)माप(आकार_2);
}
बाह्य व्योम _ZdaPvm(व्योम *p, uint64_t आकार_2);
व्योम RPDEFVIS _ZdaPvm(व्योम *p, uint64_t आकार_2) {
  rpfree(p);
  (व्योम)माप(आकार_2);
}
बाह्य व्योम _ZdlPvSt11align_val_t(व्योम *p, uint64_t align);
व्योम RPDEFVIS _ZdlPvSt11align_val_t(व्योम *p, uint64_t align) {
  rpfree(p);
  (व्योम)माप(align);
}
बाह्य व्योम _ZdaPvSt11align_val_t(व्योम *p, uint64_t align);
व्योम RPDEFVIS _ZdaPvSt11align_val_t(व्योम *p, uint64_t align) {
  rpfree(p);
  (व्योम)माप(align);
}
बाह्य व्योम _ZdlPvmSt11align_val_t(व्योम *p, uint64_t आकार_2, uint64_t align);
व्योम RPDEFVIS _ZdlPvmSt11align_val_t(व्योम *p, uint64_t आकार_2, uint64_t align) {
  rpfree(p);
  (व्योम)माप(आकार_2);
  (व्योम)माप(align);
}
बाह्य व्योम _ZdaPvmSt11align_val_t(व्योम *p, uint64_t आकार_2, uint64_t align);
व्योम RPDEFVIS _ZdaPvmSt11align_val_t(व्योम *p, uint64_t आकार_2, uint64_t align) {
  rpfree(p);
  (व्योम)माप(आकार_2);
  (व्योम)माप(align);
}
#अन्यथा
// 32-bit operators new and new[], normal and aligned
बाह्य व्योम *_Znwj(uint32_t आकार_2);
व्योम *RPDEFVIS _Znwj(uint32_t आकार_2) { वापस rpmalloc(आकार_2); }
बाह्य व्योम *_Znaj(uint32_t आकार_2);
व्योम *RPDEFVIS _Znaj(uint32_t आकार_2) { वापस rpmalloc(आकार_2); }
बाह्य व्योम *_Znwjj(uint32_t आकार_2, uint32_t align);
व्योम *RPDEFVIS _Znwjj(uint32_t आकार_2, uint32_t align) {
  वापस rpaligned_alloc(align, आकार_2);
}
बाह्य व्योम *_Znajj(uint32_t आकार_2, uint32_t align);
व्योम *RPDEFVIS _Znajj(uint32_t आकार_2, uint32_t align) {
  वापस rpaligned_alloc(align, आकार_2);
}
बाह्य व्योम *_ZnwjSt11align_val_t(माप_प्रकार आकार_2, माप_प्रकार align);
व्योम *RPDEFVIS _ZnwjSt11align_val_t(माप_प्रकार आकार_2, माप_प्रकार align) {
  वापस rpaligned_alloc(align, आकार_2);
}
बाह्य व्योम *_ZnajSt11align_val_t(माप_प्रकार आकार_2, माप_प्रकार align);
व्योम *RPDEFVIS _ZnajSt11align_val_t(माप_प्रकार आकार_2, माप_प्रकार align) {
  वापस rpaligned_alloc(align, आकार_2);
}
बाह्य व्योम *_ZnwjRKSt9nothrow_t(माप_प्रकार आकार_2, rp_nothrow_t t);
व्योम *RPDEFVIS _ZnwjRKSt9nothrow_t(माप_प्रकार आकार_2, rp_nothrow_t t) {
  (व्योम)माप(t);
  वापस rpmalloc(आकार_2);
}
बाह्य व्योम *_ZnajRKSt9nothrow_t(माप_प्रकार आकार_2, rp_nothrow_t t);
व्योम *RPDEFVIS _ZnajRKSt9nothrow_t(माप_प्रकार आकार_2, rp_nothrow_t t) {
  (व्योम)माप(t);
  वापस rpmalloc(आकार_2);
}
बाह्य व्योम *_ZnwjSt11align_val_tRKSt9nothrow_t(माप_प्रकार आकार_2, माप_प्रकार align,
                                                rp_nothrow_t t);
व्योम *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(माप_प्रकार आकार_2, माप_प्रकार align,
                                                  rp_nothrow_t t) {
  (व्योम)माप(t);
  वापस rpaligned_alloc(align, आकार_2);
}
बाह्य व्योम *_ZnajSt11align_val_tRKSt9nothrow_t(माप_प्रकार आकार_2, माप_प्रकार align,
                                                rp_nothrow_t t);
व्योम *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(माप_प्रकार आकार_2, माप_प्रकार align,
                                                  rp_nothrow_t t) {
  (व्योम)माप(t);
  वापस rpaligned_alloc(align, आकार_2);
}
// 32-bit operators sized delete and delete[], normal and aligned
बाह्य व्योम _ZdlPvj(व्योम *p, uint64_t आकार_2);
व्योम RPDEFVIS _ZdlPvj(व्योम *p, uint64_t आकार_2) {
  rpfree(p);
  (व्योम)माप(आकार_2);
}
बाह्य व्योम _ZdaPvj(व्योम *p, uint64_t आकार_2);
व्योम RPDEFVIS _ZdaPvj(व्योम *p, uint64_t आकार_2) {
  rpfree(p);
  (व्योम)माप(आकार_2);
}
बाह्य व्योम _ZdlPvSt11align_val_t(व्योम *p, uint32_t align);
व्योम RPDEFVIS _ZdlPvSt11align_val_t(व्योम *p, uint64_t a) {
  rpfree(p);
  (व्योम)माप(align);
}
बाह्य व्योम _ZdaPvSt11align_val_t(व्योम *p, uint32_t align);
व्योम RPDEFVIS _ZdaPvSt11align_val_t(व्योम *p, uint64_t a) {
  rpfree(p);
  (व्योम)माप(align);
}
बाह्य व्योम _ZdlPvjSt11align_val_t(व्योम *p, uint32_t आकार_2, uint32_t align);
व्योम RPDEFVIS _ZdlPvjSt11align_val_t(व्योम *p, uint64_t आकार_2, uint64_t align) {
  rpfree(p);
  (व्योम)माप(आकार_2);
  (व्योम)माप(a);
}
बाह्य व्योम _ZdaPvjSt11align_val_t(व्योम *p, uint32_t आकार_2, uint32_t align);
व्योम RPDEFVIS _ZdaPvjSt11align_val_t(व्योम *p, uint64_t आकार_2, uint64_t align) {
  rpfree(p);
  (व्योम)माप(आकार_2);
  (व्योम)माप(a);
}
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर

#अगर USE_INTERPOSE || USE_ALIAS

अटल व्योम *rpmalloc_nothrow(माप_प्रकार आकार_2, rp_nothrow_t t) {
  (व्योम)माप(t);
  वापस rpmalloc(आकार_2);
}
अटल व्योम *rpaligned_alloc_reverse(माप_प्रकार आकार_2, माप_प्रकार align) {
  वापस rpaligned_alloc(align, आकार_2);
}
अटल व्योम *rpaligned_alloc_reverse_nothrow(माप_प्रकार आकार_2, माप_प्रकार align,
                                             rp_nothrow_t t) {
  (व्योम)माप(t);
  वापस rpaligned_alloc(align, आकार_2);
}
अटल व्योम rpfree_आकार(व्योम *p, माप_प्रकार आकार_2) {
  (व्योम)माप(आकार_2);
  rpfree(p);
}
अटल व्योम rpfree_aligned(व्योम *p, माप_प्रकार align) {
  (व्योम)माप(align);
  rpfree(p);
}
अटल व्योम rpfree_आकार_aligned(व्योम *p, माप_प्रकार आकार_2, माप_प्रकार align) {
  (व्योम)माप(आकार_2);
  (व्योम)माप(align);
  rpfree(p);
}

#पूर्ण_अगर

#अगर USE_INTERPOSE

__attribute__((used)) अटल स्थिर interpose_t macinterpose_malloc[]
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
        MAC_INTERPOSE_PAIR(rpmalloc, दो_स्मृति),
        MAC_INTERPOSE_PAIR(rpmalloc, सुस्मृति),
        MAC_INTERPOSE_PAIR(rprealloc, पुनः_स्मृति),
        MAC_INTERPOSE_PAIR(rprealloc, reallocf),
#अगर defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#पूर्ण_अगर
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, मुक्त), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_आकार, malloc_usable_आकार),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_आकार, malloc_आकार)};

#पूर्ण_अगर

#अगर USE_ALIAS

#घोषणा RPALIAS(fn) __attribute__((alias(#fn), used, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
व्योम _ZdlPv(व्योम *p) RPALIAS(rpfree) व्योम _ZdaPv(व्योम *p) RPALIAS(rpfree)

#अगर ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    व्योम *_Znwm(uint64_t आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(1)
        RPALIAS(rpmalloc) व्योम *_Znam(uint64_t आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(1) RPALIAS(rpmalloc) व्योम *_Znwmm(uint64_t आकार_2,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) व्योम *_Znamm(uint64_t आकार_2,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) व्योम *_ZnwmSt11align_val_t(
                माप_प्रकार आकार_2, माप_प्रकार align)
                RPALIAS(rpaligned_alloc_reverse) व्योम *_ZnamSt11align_val_t(
                    माप_प्रकार आकार_2, माप_प्रकार align)
                    RPALIAS(rpaligned_alloc_reverse) व्योम *_ZnwmRKSt9nothrow_t(
                        माप_प्रकार आकार_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) व्योम *_ZnamRKSt9nothrow_t(
                            माप_प्रकार आकार_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) व्योम
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(माप_प्रकार आकार_2,
                                                                माप_प्रकार align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) व्योम
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        माप_प्रकार आकार_2, माप_प्रकार align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    व्योम _ZdlPvm(व्योम *p, माप_प्रकार n) RPALIAS(rpfree_आकार) व्योम _ZdaPvm(व्योम *p,
                                                                      माप_प्रकार n)
        RPALIAS(rpfree_आकार) व्योम _ZdlPvSt11align_val_t(व्योम *p, माप_प्रकार a)
            RPALIAS(rpfree_aligned) व्योम _ZdaPvSt11align_val_t(व्योम *p,
                                                               माप_प्रकार a)
                RPALIAS(rpfree_aligned) व्योम _ZdlPvmSt11align_val_t(व्योम *p,
                                                                    माप_प्रकार n,
                                                                    माप_प्रकार a)
                    RPALIAS(rpfree_आकार_aligned) व्योम _ZdaPvmSt11align_val_t(
                        व्योम *p, माप_प्रकार n, माप_प्रकार a)
                        RPALIAS(rpfree_आकार_aligned)
#अन्यथा
    // 32-bit operators new and new[], normal and aligned
    व्योम *_Znwj(uint32_t आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(1)
        RPALIAS(rpmalloc) व्योम *_Znaj(uint32_t आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(1) RPALIAS(rpmalloc) व्योम *_Znwjj(uint32_t आकार_2,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) व्योम *_Znajj(uint32_t आकार_2,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) व्योम *_ZnwjSt11align_val_t(
                माप_प्रकार आकार_2, माप_प्रकार align)
                RPALIAS(rpaligned_alloc_reverse) व्योम *_ZnajSt11align_val_t(
                    माप_प्रकार आकार_2, माप_प्रकार align)
                    RPALIAS(rpaligned_alloc_reverse) व्योम *_ZnwjRKSt9nothrow_t(
                        माप_प्रकार आकार_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) व्योम *_ZnajRKSt9nothrow_t(
                            माप_प्रकार आकार_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) व्योम
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(माप_प्रकार आकार_2,
                                                                माप_प्रकार align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) व्योम
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        माप_प्रकार आकार_2, माप_प्रकार align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    व्योम _ZdlPvj(व्योम *p, माप_प्रकार n) RPALIAS(rpfree_आकार) व्योम _ZdaPvj(व्योम *p,
                                                                      माप_प्रकार n)
        RPALIAS(rpfree_आकार) व्योम _ZdlPvSt11align_val_t(व्योम *p, माप_प्रकार a)
            RPALIAS(rpfree_aligned) व्योम _ZdaPvSt11align_val_t(व्योम *p,
                                                               माप_प्रकार a)
                RPALIAS(rpfree_aligned) व्योम _ZdlPvjSt11align_val_t(व्योम *p,
                                                                    माप_प्रकार n,
                                                                    माप_प्रकार a)
                    RPALIAS(rpfree_आकार_aligned) व्योम _ZdaPvjSt11align_val_t(
                        व्योम *p, माप_प्रकार n, माप_प्रकार a)
                        RPALIAS(rpfree_आकार_aligned)
#पूर्ण_अगर

                            व्योम *दो_स्मृति(माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(1)
        RPALIAS(rpmalloc) व्योम *सुस्मृति(माप_प्रकार गिनती, माप_प्रकार आकार_2)
            RPALIAS(rpcalloc) व्योम *पुनः_स्मृति(व्योम *ptr, माप_प्रकार आकार_2)
                RPALIAS(rprealloc) व्योम *reallocf(व्योम *ptr, माप_प्रकार आकार_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(2)
        RPALIAS(rprealloc) व्योम *aligned_alloc(माप_प्रकार alignment, माप_प्रकार आकार_2)
            RPALIAS(rpaligned_alloc) व्योम *memalign(
                माप_प्रकार alignment, माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(2)
        RPALIAS(rpmemalign) पूर्णांक posix_memalign(व्योम **memptr, माप_प्रकार alignment,
                                               माप_प्रकार आकार_2)
            RPALIAS(rpposix_memalign) व्योम मुक्त(व्योम *ptr)
                RPALIAS(rpfree) व्योम cfree(व्योम *ptr) RPALIAS(rpfree)
#अगर defined(__ANDROID__) || defined(__FreeBSD__)
                    माप_प्रकार
    malloc_usable_आकार(स्थिर व्योम *ptr) RPALIAS(rpmalloc_usable_आकार)
#अन्यथा
                    माप_प्रकार
    malloc_usable_आकार(व्योम *ptr) RPALIAS(rpmalloc_usable_आकार)
#पूर्ण_अगर
        माप_प्रकार malloc_आकार(व्योम *ptr) RPALIAS(rpmalloc_usable_आकार)

#पूर्ण_अगर

            अटल अंतरभूत माप_प्रकार rpmalloc_page_आकार(व्योम) {
  वापस memory_page_आकार;
}

बाह्य व्योम *RPMALLOC_CDECL reallocarray(व्योम *ptr, माप_प्रकार गिनती, माप_प्रकार आकार_2);

बाह्य व्योम *RPMALLOC_CDECL reallocarray(व्योम *ptr, माप_प्रकार गिनती, माप_प्रकार आकार_2) {
  माप_प्रकार योग_3;
#अगर ENABLE_VALIDATE_ARGS
#अगर_घोषित _MSC_VER
  पूर्णांक err = आकार_TMult(गिनती, आकार_2, &योग_3);
  अगर ((err != S_OK) || (योग_3 >= MAX_ALLOC_आकार)) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#अन्यथा
  पूर्णांक err = __builtin_umull_overflow(गिनती, आकार_2, &योग_3);
  अगर (err || (योग_3 >= MAX_ALLOC_आकार)) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#पूर्ण_अगर
#अन्यथा
  योग_3 = गिनती * आकार_2;
#पूर्ण_अगर
  वापस पुनः_स्मृति(ptr, योग_3);
}

बाह्य अंतरभूत व्योम *RPMALLOC_CDECL valloc(माप_प्रकार आकार_2) {
  get_thread_heap();
  वापस rpaligned_alloc(rpmalloc_page_आकार(), आकार_2);
}

बाह्य अंतरभूत व्योम *RPMALLOC_CDECL pvalloc(माप_प्रकार आकार_2) {
  get_thread_heap();
  स्थिर माप_प्रकार page_आकार = rpmalloc_page_आकार();
  स्थिर माप_प्रकार aligned_आकार = ((आकार_2 + page_आकार - 1) / page_आकार) * page_आकार;
#अगर ENABLE_VALIDATE_ARGS
  अगर (aligned_आकार < आकार_2) {
    त्रुटि_सं = EINVAL;
    वापस 0;
  }
#पूर्ण_अगर
  वापस rpaligned_alloc(rpmalloc_page_आकार(), aligned_आकार);
}

#पूर्ण_अगर // ENABLE_OVERRIDE

#अगर ENABLE_PRELOAD

#अगर_घोषित _WIN32

#अगर defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

बाह्य __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

बाह्य __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (व्योम)माप(reserved);
  (व्योम)माप(instance);
  अगर (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  अन्यथा अगर (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  अन्यथा अगर (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  अन्यथा अगर (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  वापस TRUE;
}

// end BUILD_DYNAMIC_LINK
#अन्यथा

बाह्य व्योम _global_rpmalloc_init(व्योम) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
}

#अगर defined(__clang__) || defined(__GNUC__)

अटल व्योम __attribute__((constructor)) initializer(व्योम) {
  _global_rpmalloc_init();
}

#या_अगर defined(_MSC_VER)

अटल पूर्णांक _global_rpmalloc_xib(व्योम) {
  _global_rpmalloc_init();
  वापस 0;
}

#आशय section(".CRT$XIB", पढ़ो)
__declspec(allocate(".CRT$XIB")) व्योम (*_rpmalloc_module_init)(व्योम) =
    _global_rpmalloc_xib;
#अगर defined(_M_IX86) || defined(__i386__)
#आशय comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#अन्यथा
#आशय comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#पूर्ण_अगर

#पूर्ण_अगर

// end !BUILD_DYNAMIC_LINK
#पूर्ण_अगर

#अन्यथा

#समावेश <pthread.h>
#समावेश <stdint.h>
#समावेश <stdlib.h>
#समावेश <unistd.h>

बाह्य व्योम rpmalloc_set_main_thread(व्योम);

अटल pthread_key_t destructor_कुंजी;

अटल व्योम thread_destructor(व्योम *);

अटल व्योम __attribute__((constructor)) initializer(व्योम) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
  pthread_कुंजी_create(&destructor_कुंजी, thread_destructor);
}

अटल व्योम __attribute__((destructor)) finalizer(व्योम) { rpmalloc_finalize(); }

प्रकार काष्ठा {
  व्योम *(*real_start)(व्योम *);
  व्योम *real_arg;
} thread_starter_arg;

अटल व्योम *thread_starter(व्योम *argptr) {
  thread_starter_arg *arg = argptr;
  व्योम *(*real_start)(व्योम *) = arg->real_start;
  व्योम *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_कुंजी, (व्योम *)1);
  वापस (*real_start)(real_arg);
}

अटल व्योम thread_destructor(व्योम *मूल्य_2) {
  (व्योम)माप(मूल्य_2);
  rpmalloc_thread_finalize(1);
}

#अगर_घोषित __APPLE__

अटल पूर्णांक pthread_create_proxy(pthread_t *thread, स्थिर pthread_attr_t *attr,
                                व्योम *(*start_routine)(व्योम *), व्योम *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(माप(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  वापस pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#अन्यथा

#समावेश <dlfcn.h>

पूर्णांक pthread_create(pthread_t *thread, स्थिर pthread_attr_t *attr,
                   व्योम *(*start_routine)(व्योम *), व्योम *arg) {
#अगर defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  अक्षर fname[] = "pthread_create";
#अन्यथा
  अक्षर fname[] = "_pthread_create";
#पूर्ण_अगर
  व्योम *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(माप(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  वापस (*(पूर्णांक (*)(pthread_t *, स्थिर pthread_attr_t *, व्योम *(*)(व्योम *),
                    व्योम *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#पूर्ण_अगर

#पूर्ण_अगर

#पूर्ण_अगर

#अगर ENABLE_OVERRIDE

#अगर defined(__GLIBC__) && defined(__linux__)

व्योम *__libc_malloc(माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(1)
        RPALIAS(rpmalloc) व्योम *__libc_calloc(माप_प्रकार गिनती, माप_प्रकार आकार_2)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) व्योम *__libc_realloc(व्योम *p, माप_प्रकार आकार_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(2) RPALIAS(rprealloc) व्योम __libc_free(व्योम *p)
        RPALIAS(rpfree) व्योम __libc_cfree(व्योम *p)
            RPALIAS(rpfree) व्योम *__libc_memalign(माप_प्रकार align, माप_प्रकार आकार_2)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(2)
        RPALIAS(rpmemalign) पूर्णांक __posix_memalign(व्योम **p, माप_प्रकार align,
                                                 माप_प्रकार आकार_2)
            RPALIAS(rpposix_memalign)

                बाह्य व्योम *__libc_valloc(माप_प्रकार आकार_2);
बाह्य व्योम *__libc_pvalloc(माप_प्रकार आकार_2);

व्योम *__libc_valloc(माप_प्रकार आकार_2) { वापस valloc(आकार_2); }

व्योम *__libc_pvalloc(माप_प्रकार आकार_2) { वापस pvalloc(आकार_2); }

#पूर्ण_अगर

#पूर्ण_अगर

#अगर (defined(__GNUC__) || defined(__clang__))
#आशय GCC visibility निकालो
#पूर्ण_अगर
