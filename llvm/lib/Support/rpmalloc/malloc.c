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

#എങ്കിൽ defined(__TINYC__)
#അന്തര്ഭാവ <sys/types.h>
#endif

#ifndef ARCH_64BIT
#എങ്കിൽ defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#define ARCH_64BIT 1
_Static_assert(വലുപ്പം(size_t) == 8, "Data type size mismatch");
_Static_assert(വലുപ്പം(ശൂന്യം *) == 8, "Data type size mismatch");
#അല്ലെങ്കിൽ
#define ARCH_64BIT 0
_Static_assert(വലുപ്പം(size_t) == 4, "Data type size mismatch");
_Static_assert(വലുപ്പം(ശൂന്യം *) == 4, "Data type size mismatch");
#endif
#endif

#എങ്കിൽ (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility തള്ളുക(സ്ഥിരസ്ഥിതി)
#endif

#define USE_IMPLEMENT 1
#define USE_INTERPOSE 0
#define USE_ALIAS 0

#എങ്കിൽ defined(__APPLE__)
#undef USE_INTERPOSE
#define USE_INTERPOSE 1

തരനിർവചനം ഘടന interpose_t {
  ശൂന്യം *new_func;
  ശൂന്യം *orig_func;
} interpose_t;

#define MAC_INTERPOSE_PAIR(newf, oldf) {(ശൂന്യം *)newf, (ശൂന്യം *)oldf}
#define MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((used)) സ്ഥിതം സ്ഥിരം interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#endif

#എങ്കിൽ !defined(_WIN32) && !defined(__APPLE__)
#undef USE_IMPLEMENT
#undef USE_ALIAS
#define USE_IMPLEMENT 0
#define USE_ALIAS 1
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#undef malloc
#undef സ്വതന്ത്രം
#undef calloc
#define RPMALLOC_RESTRICT __declspec(restrict)
#അല്ലെങ്കിൽ
#define RPMALLOC_RESTRICT
#endif

#എങ്കിൽ ENABLE_OVERRIDE

തരനിർവചനം ഘടന rp_nothrow_t {
  പൂർണ്ണസംഖ്യ __dummy;
} rp_nothrow_t;

#എങ്കിൽ USE_IMPLEMENT

ബാഹ്യ വരിയിൽ RPMALLOC_RESTRICT ശൂന്യം *RPMALLOC_CDECL malloc(size_t size) {
  തിരികെ rpmalloc(size);
}
ബാഹ്യ വരിയിൽ RPMALLOC_RESTRICT ശൂന്യം *RPMALLOC_CDECL calloc(size_t എണ്ണം,
                                                            size_t size) {
  തിരികെ rpcalloc(എണ്ണം, size);
}
ബാഹ്യ വരിയിൽ RPMALLOC_RESTRICT ശൂന്യം *RPMALLOC_CDECL realloc(ശൂന്യം *ptr,
                                                             size_t size) {
  തിരികെ rprealloc(ptr, size);
}
ബാഹ്യ വരിയിൽ ശൂന്യം *RPMALLOC_CDECL reallocf(ശൂന്യം *ptr, size_t size) {
  തിരികെ rprealloc(ptr, size);
}
ബാഹ്യ വരിയിൽ ശൂന്യം *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t size) {
  തിരികെ rpaligned_alloc(alignment, size);
}
ബാഹ്യ വരിയിൽ ശൂന്യം *RPMALLOC_CDECL memalign(size_t alignment, size_t size) {
  തിരികെ rpmemalign(alignment, size);
}
ബാഹ്യ വരിയിൽ പൂർണ്ണസംഖ്യ RPMALLOC_CDECL posix_memalign(ശൂന്യം **memptr, size_t alignment,
                                                size_t size) {
  തിരികെ rpposix_memalign(memptr, alignment, size);
}
ബാഹ്യ വരിയിൽ ശൂന്യം RPMALLOC_CDECL സ്വതന്ത്രം(ശൂന്യം *ptr) { rpfree(ptr); }
ബാഹ്യ വരിയിൽ ശൂന്യം RPMALLOC_CDECL cfree(ശൂന്യം *ptr) { rpfree(ptr); }
ബാഹ്യ വരിയിൽ size_t RPMALLOC_CDECL malloc_usable_size(ശൂന്യം *ptr) {
  തിരികെ rpmalloc_usable_size(ptr);
}
ബാഹ്യ വരിയിൽ size_t RPMALLOC_CDECL malloc_size(ശൂന്യം *ptr) {
  തിരികെ rpmalloc_usable_size(ptr);
}

#ifdef _WIN32
ബാഹ്യ വരിയിൽ RPMALLOC_RESTRICT ശൂന്യം *RPMALLOC_CDECL _malloc_base(size_t size) {
  തിരികെ rpmalloc(size);
}
ബാഹ്യ വരിയിൽ ശൂന്യം RPMALLOC_CDECL _free_base(ശൂന്യം *ptr) { rpfree(ptr); }
ബാഹ്യ വരിയിൽ RPMALLOC_RESTRICT ശൂന്യം *RPMALLOC_CDECL _calloc_base(size_t എണ്ണം,
                                                                  size_t size) {
  തിരികെ rpcalloc(എണ്ണം, size);
}
ബാഹ്യ വരിയിൽ size_t RPMALLOC_CDECL _msize(ശൂന്യം *ptr) {
  തിരികെ rpmalloc_usable_size(ptr);
}
ബാഹ്യ വരിയിൽ size_t RPMALLOC_CDECL _msize_base(ശൂന്യം *ptr) {
  തിരികെ rpmalloc_usable_size(ptr);
}
ബാഹ്യ വരിയിൽ RPMALLOC_RESTRICT ശൂന്യം *RPMALLOC_CDECL
_realloc_base(ശൂന്യം *ptr, size_t size) {
  തിരികെ rprealloc(ptr, size);
}
#endif

#ifdef _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#അല്ലെങ്കിൽ
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#define RPDEFVIS __attribute__((visibility("default")))
ബാഹ്യ ശൂന്യം _ZdlPv(ശൂന്യം *p);
ശൂന്യം RPDEFVIS _ZdlPv(ശൂന്യം *p) { rpfree(p); }
ബാഹ്യ ശൂന്യം _ZdaPv(ശൂന്യം *p);
ശൂന്യം RPDEFVIS _ZdaPv(ശൂന്യം *p) { rpfree(p); }
#എങ്കിൽ ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
ബാഹ്യ ശൂന്യം *_Znwm(uint64_t size);
ശൂന്യം *RPDEFVIS _Znwm(uint64_t size) { തിരികെ rpmalloc(size); }
ബാഹ്യ ശൂന്യം *_Znam(uint64_t size);
ശൂന്യം *RPDEFVIS _Znam(uint64_t size) { തിരികെ rpmalloc(size); }
ബാഹ്യ ശൂന്യം *_Znwmm(uint64_t size, uint64_t align);
ശൂന്യം *RPDEFVIS _Znwmm(uint64_t size, uint64_t align) {
  തിരികെ rpaligned_alloc(align, size);
}
ബാഹ്യ ശൂന്യം *_Znamm(uint64_t size, uint64_t align);
ശൂന്യം *RPDEFVIS _Znamm(uint64_t size, uint64_t align) {
  തിരികെ rpaligned_alloc(align, size);
}
ബാഹ്യ ശൂന്യം *_ZnwmSt11align_val_t(uint64_t size, uint64_t align);
ശൂന്യം *RPDEFVIS _ZnwmSt11align_val_t(uint64_t size, uint64_t align) {
  തിരികെ rpaligned_alloc(align, size);
}
ബാഹ്യ ശൂന്യം *_ZnamSt11align_val_t(uint64_t size, uint64_t align);
ശൂന്യം *RPDEFVIS _ZnamSt11align_val_t(uint64_t size, uint64_t align) {
  തിരികെ rpaligned_alloc(align, size);
}
ബാഹ്യ ശൂന്യം *_ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
ശൂന്യം *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (ശൂന്യം)വലുപ്പം(t);
  തിരികെ rpmalloc(size);
}
ബാഹ്യ ശൂന്യം *_ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
ശൂന്യം *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (ശൂന്യം)വലുപ്പം(t);
  തിരികെ rpmalloc(size);
}
ബാഹ്യ ശൂന്യം *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
ശൂന്യം *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (ശൂന്യം)വലുപ്പം(t);
  തിരികെ rpaligned_alloc(align, size);
}
ബാഹ്യ ശൂന്യം *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
ശൂന്യം *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (ശൂന്യം)വലുപ്പം(t);
  തിരികെ rpaligned_alloc(align, size);
}
// 64-bit operators sized delete and delete[], normal and aligned
ബാഹ്യ ശൂന്യം _ZdlPvm(ശൂന്യം *p, uint64_t size);
ശൂന്യം RPDEFVIS _ZdlPvm(ശൂന്യം *p, uint64_t size) {
  rpfree(p);
  (ശൂന്യം)വലുപ്പം(size);
}
ബാഹ്യ ശൂന്യം _ZdaPvm(ശൂന്യം *p, uint64_t size);
ശൂന്യം RPDEFVIS _ZdaPvm(ശൂന്യം *p, uint64_t size) {
  rpfree(p);
  (ശൂന്യം)വലുപ്പം(size);
}
ബാഹ്യ ശൂന്യം _ZdlPvSt11align_val_t(ശൂന്യം *p, uint64_t align);
ശൂന്യം RPDEFVIS _ZdlPvSt11align_val_t(ശൂന്യം *p, uint64_t align) {
  rpfree(p);
  (ശൂന്യം)വലുപ്പം(align);
}
ബാഹ്യ ശൂന്യം _ZdaPvSt11align_val_t(ശൂന്യം *p, uint64_t align);
ശൂന്യം RPDEFVIS _ZdaPvSt11align_val_t(ശൂന്യം *p, uint64_t align) {
  rpfree(p);
  (ശൂന്യം)വലുപ്പം(align);
}
ബാഹ്യ ശൂന്യം _ZdlPvmSt11align_val_t(ശൂന്യം *p, uint64_t size, uint64_t align);
ശൂന്യം RPDEFVIS _ZdlPvmSt11align_val_t(ശൂന്യം *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (ശൂന്യം)വലുപ്പം(size);
  (ശൂന്യം)വലുപ്പം(align);
}
ബാഹ്യ ശൂന്യം _ZdaPvmSt11align_val_t(ശൂന്യം *p, uint64_t size, uint64_t align);
ശൂന്യം RPDEFVIS _ZdaPvmSt11align_val_t(ശൂന്യം *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (ശൂന്യം)വലുപ്പം(size);
  (ശൂന്യം)വലുപ്പം(align);
}
#അല്ലെങ്കിൽ
// 32-bit operators new and new[], normal and aligned
ബാഹ്യ ശൂന്യം *_Znwj(uint32_t size);
ശൂന്യം *RPDEFVIS _Znwj(uint32_t size) { തിരികെ rpmalloc(size); }
ബാഹ്യ ശൂന്യം *_Znaj(uint32_t size);
ശൂന്യം *RPDEFVIS _Znaj(uint32_t size) { തിരികെ rpmalloc(size); }
ബാഹ്യ ശൂന്യം *_Znwjj(uint32_t size, uint32_t align);
ശൂന്യം *RPDEFVIS _Znwjj(uint32_t size, uint32_t align) {
  തിരികെ rpaligned_alloc(align, size);
}
ബാഹ്യ ശൂന്യം *_Znajj(uint32_t size, uint32_t align);
ശൂന്യം *RPDEFVIS _Znajj(uint32_t size, uint32_t align) {
  തിരികെ rpaligned_alloc(align, size);
}
ബാഹ്യ ശൂന്യം *_ZnwjSt11align_val_t(size_t size, size_t align);
ശൂന്യം *RPDEFVIS _ZnwjSt11align_val_t(size_t size, size_t align) {
  തിരികെ rpaligned_alloc(align, size);
}
ബാഹ്യ ശൂന്യം *_ZnajSt11align_val_t(size_t size, size_t align);
ശൂന്യം *RPDEFVIS _ZnajSt11align_val_t(size_t size, size_t align) {
  തിരികെ rpaligned_alloc(align, size);
}
ബാഹ്യ ശൂന്യം *_ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t);
ശൂന്യം *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (ശൂന്യം)വലുപ്പം(t);
  തിരികെ rpmalloc(size);
}
ബാഹ്യ ശൂന്യം *_ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t);
ശൂന്യം *RPDEFVIS _ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (ശൂന്യം)വലുപ്പം(t);
  തിരികെ rpmalloc(size);
}
ബാഹ്യ ശൂന്യം *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
ശൂന്യം *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (ശൂന്യം)വലുപ്പം(t);
  തിരികെ rpaligned_alloc(align, size);
}
ബാഹ്യ ശൂന്യം *_ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
ശൂന്യം *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (ശൂന്യം)വലുപ്പം(t);
  തിരികെ rpaligned_alloc(align, size);
}
// 32-bit operators sized delete and delete[], normal and aligned
ബാഹ്യ ശൂന്യം _ZdlPvj(ശൂന്യം *p, uint64_t size);
ശൂന്യം RPDEFVIS _ZdlPvj(ശൂന്യം *p, uint64_t size) {
  rpfree(p);
  (ശൂന്യം)വലുപ്പം(size);
}
ബാഹ്യ ശൂന്യം _ZdaPvj(ശൂന്യം *p, uint64_t size);
ശൂന്യം RPDEFVIS _ZdaPvj(ശൂന്യം *p, uint64_t size) {
  rpfree(p);
  (ശൂന്യം)വലുപ്പം(size);
}
ബാഹ്യ ശൂന്യം _ZdlPvSt11align_val_t(ശൂന്യം *p, uint32_t align);
ശൂന്യം RPDEFVIS _ZdlPvSt11align_val_t(ശൂന്യം *p, uint64_t a) {
  rpfree(p);
  (ശൂന്യം)വലുപ്പം(align);
}
ബാഹ്യ ശൂന്യം _ZdaPvSt11align_val_t(ശൂന്യം *p, uint32_t align);
ശൂന്യം RPDEFVIS _ZdaPvSt11align_val_t(ശൂന്യം *p, uint64_t a) {
  rpfree(p);
  (ശൂന്യം)വലുപ്പം(align);
}
ബാഹ്യ ശൂന്യം _ZdlPvjSt11align_val_t(ശൂന്യം *p, uint32_t size, uint32_t align);
ശൂന്യം RPDEFVIS _ZdlPvjSt11align_val_t(ശൂന്യം *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (ശൂന്യം)വലുപ്പം(size);
  (ശൂന്യം)വലുപ്പം(a);
}
ബാഹ്യ ശൂന്യം _ZdaPvjSt11align_val_t(ശൂന്യം *p, uint32_t size, uint32_t align);
ശൂന്യം RPDEFVIS _ZdaPvjSt11align_val_t(ശൂന്യം *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (ശൂന്യം)വലുപ്പം(size);
  (ശൂന്യം)വലുപ്പം(a);
}
#endif
#endif
#endif

#എങ്കിൽ USE_INTERPOSE || USE_ALIAS

സ്ഥിതം ശൂന്യം *rpmalloc_nothrow(size_t size, rp_nothrow_t t) {
  (ശൂന്യം)വലുപ്പം(t);
  തിരികെ rpmalloc(size);
}
സ്ഥിതം ശൂന്യം *rpaligned_alloc_reverse(size_t size, size_t align) {
  തിരികെ rpaligned_alloc(align, size);
}
സ്ഥിതം ശൂന്യം *rpaligned_alloc_reverse_nothrow(size_t size, size_t align,
                                             rp_nothrow_t t) {
  (ശൂന്യം)വലുപ്പം(t);
  തിരികെ rpaligned_alloc(align, size);
}
സ്ഥിതം ശൂന്യം rpfree_size(ശൂന്യം *p, size_t size) {
  (ശൂന്യം)വലുപ്പം(size);
  rpfree(p);
}
സ്ഥിതം ശൂന്യം rpfree_aligned(ശൂന്യം *p, size_t align) {
  (ശൂന്യം)വലുപ്പം(align);
  rpfree(p);
}
സ്ഥിതം ശൂന്യം rpfree_size_aligned(ശൂന്യം *p, size_t size, size_t align) {
  (ശൂന്യം)വലുപ്പം(size);
  (ശൂന്യം)വലുപ്പം(align);
  rpfree(p);
}

#endif

#എങ്കിൽ USE_INTERPOSE

__attribute__((used)) സ്ഥിതം സ്ഥിരം interpose_t macinterpose_malloc[]
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
#എങ്കിൽ defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#endif
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, സ്വതന്ത്രം), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_usable_size),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_size)};

#endif

#എങ്കിൽ USE_ALIAS

#define RPALIAS(fn) __attribute__((alias(#fn), used, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
ശൂന്യം _ZdlPv(ശൂന്യം *p) RPALIAS(rpfree) ശൂന്യം _ZdaPv(ശൂന്യം *p) RPALIAS(rpfree)

#എങ്കിൽ ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    ശൂന്യം *_Znwm(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) ശൂന്യം *_Znam(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) ശൂന്യം *_Znwmm(uint64_t size,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) ശൂന്യം *_Znamm(uint64_t size,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) ശൂന്യം *_ZnwmSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) ശൂന്യം *_ZnamSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) ശൂന്യം *_ZnwmRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) ശൂന്യം *_ZnamRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) ശൂന്യം
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) ശൂന്യം
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    ശൂന്യം _ZdlPvm(ശൂന്യം *p, size_t n) RPALIAS(rpfree_size) ശൂന്യം _ZdaPvm(ശൂന്യം *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) ശൂന്യം _ZdlPvSt11align_val_t(ശൂന്യം *p, size_t a)
            RPALIAS(rpfree_aligned) ശൂന്യം _ZdaPvSt11align_val_t(ശൂന്യം *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) ശൂന്യം _ZdlPvmSt11align_val_t(ശൂന്യം *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) ശൂന്യം _ZdaPvmSt11align_val_t(
                        ശൂന്യം *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#അല്ലെങ്കിൽ
    // 32-bit operators new and new[], normal and aligned
    ശൂന്യം *_Znwj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) ശൂന്യം *_Znaj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) ശൂന്യം *_Znwjj(uint32_t size,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) ശൂന്യം *_Znajj(uint32_t size,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) ശൂന്യം *_ZnwjSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) ശൂന്യം *_ZnajSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) ശൂന്യം *_ZnwjRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) ശൂന്യം *_ZnajRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) ശൂന്യം
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) ശൂന്യം
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    ശൂന്യം _ZdlPvj(ശൂന്യം *p, size_t n) RPALIAS(rpfree_size) ശൂന്യം _ZdaPvj(ശൂന്യം *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) ശൂന്യം _ZdlPvSt11align_val_t(ശൂന്യം *p, size_t a)
            RPALIAS(rpfree_aligned) ശൂന്യം _ZdaPvSt11align_val_t(ശൂന്യം *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) ശൂന്യം _ZdlPvjSt11align_val_t(ശൂന്യം *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) ശൂന്യം _ZdaPvjSt11align_val_t(
                        ശൂന്യം *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#endif

                            ശൂന്യം *malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) ശൂന്യം *calloc(size_t എണ്ണം, size_t size)
            RPALIAS(rpcalloc) ശൂന്യം *realloc(ശൂന്യം *ptr, size_t size)
                RPALIAS(rprealloc) ശൂന്യം *reallocf(ശൂന്യം *ptr, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rprealloc) ശൂന്യം *aligned_alloc(size_t alignment, size_t size)
            RPALIAS(rpaligned_alloc) ശൂന്യം *memalign(
                size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) പൂർണ്ണസംഖ്യ posix_memalign(ശൂന്യം **memptr, size_t alignment,
                                               size_t size)
            RPALIAS(rpposix_memalign) ശൂന്യം സ്വതന്ത്രം(ശൂന്യം *ptr)
                RPALIAS(rpfree) ശൂന്യം cfree(ശൂന്യം *ptr) RPALIAS(rpfree)
#എങ്കിൽ defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_size(സ്ഥിരം ശൂന്യം *ptr) RPALIAS(rpmalloc_usable_size)
#അല്ലെങ്കിൽ
                    size_t
    malloc_usable_size(ശൂന്യം *ptr) RPALIAS(rpmalloc_usable_size)
#endif
        size_t malloc_size(ശൂന്യം *ptr) RPALIAS(rpmalloc_usable_size)

#endif

            സ്ഥിതം വരിയിൽ size_t _rpmalloc_page_size(ശൂന്യം) {
  തിരികെ _memory_page_size;
}

ബാഹ്യ ശൂന്യം *RPMALLOC_CDECL reallocarray(ശൂന്യം *ptr, size_t എണ്ണം, size_t size);

ബാഹ്യ ശൂന്യം *RPMALLOC_CDECL reallocarray(ശൂന്യം *ptr, size_t എണ്ണം, size_t size) {
  size_t total;
#എങ്കിൽ ENABLE_VALIDATE_ARGS
#ifdef _MSC_VER
  പൂർണ്ണസംഖ്യ err = SizeTMult(എണ്ണം, size, &total);
  എങ്കിൽ ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    തിരികെ 0;
  }
#അല്ലെങ്കിൽ
  പൂർണ്ണസംഖ്യ err = __builtin_umull_overflow(എണ്ണം, size, &total);
  എങ്കിൽ (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    തിരികെ 0;
  }
#endif
#അല്ലെങ്കിൽ
  total = എണ്ണം * size;
#endif
  തിരികെ realloc(ptr, total);
}

ബാഹ്യ വരിയിൽ ശൂന്യം *RPMALLOC_CDECL valloc(size_t size) {
  get_thread_heap();
  തിരികെ rpaligned_alloc(_rpmalloc_page_size(), size);
}

ബാഹ്യ വരിയിൽ ശൂന്യം *RPMALLOC_CDECL pvalloc(size_t size) {
  get_thread_heap();
  സ്ഥിരം size_t page_size = _rpmalloc_page_size();
  സ്ഥിരം size_t aligned_size = ((size + page_size - 1) / page_size) * page_size;
#എങ്കിൽ ENABLE_VALIDATE_ARGS
  എങ്കിൽ (aligned_size < size) {
    errno = EINVAL;
    തിരികെ 0;
  }
#endif
  തിരികെ rpaligned_alloc(_rpmalloc_page_size(), aligned_size);
}

#endif // ENABLE_OVERRIDE

#എങ്കിൽ ENABLE_PRELOAD

#ifdef _WIN32

#എങ്കിൽ defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

ബാഹ്യ __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

ബാഹ്യ __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (ശൂന്യം)വലുപ്പം(reserved);
  (ശൂന്യം)വലുപ്പം(instance);
  എങ്കിൽ (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  അല്ലെങ്കിൽ എങ്കിൽ (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  അല്ലെങ്കിൽ എങ്കിൽ (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  അല്ലെങ്കിൽ എങ്കിൽ (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  തിരികെ TRUE;
}

// end BUILD_DYNAMIC_LINK
#അല്ലെങ്കിൽ

ബാഹ്യ ശൂന്യം _global_rpmalloc_init(ശൂന്യം) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
}

#എങ്കിൽ defined(__clang__) || defined(__GNUC__)

സ്ഥിതം ശൂന്യം __attribute__((constructor)) initializer(ശൂന്യം) {
  _global_rpmalloc_init();
}

#അല്ലെങ്കിലെങ്കിൽ defined(_MSC_VER)

സ്ഥിതം പൂർണ്ണസംഖ്യ _global_rpmalloc_xib(ശൂന്യം) {
  _global_rpmalloc_init();
  തിരികെ 0;
}

#pragma section(".CRT$XIB", പഠ)
__declspec(allocate(".CRT$XIB")) ശൂന്യം (*_rpmalloc_module_init)(ശൂന്യം) =
    _global_rpmalloc_xib;
#എങ്കിൽ defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#അല്ലെങ്കിൽ
#pragma comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#endif

#endif

// end !BUILD_DYNAMIC_LINK
#endif

#അല്ലെങ്കിൽ

#അന്തര്ഭാവ <pthread.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <unistd.h>

ബാഹ്യ ശൂന്യം rpmalloc_set_main_thread(ശൂന്യം);

സ്ഥിതം pthread_key_t destructor_താക്കോൽ;

സ്ഥിതം ശൂന്യം thread_destructor(ശൂന്യം *);

സ്ഥിതം ശൂന്യം __attribute__((constructor)) initializer(ശൂന്യം) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
  pthread_താക്കോൽ_create(&destructor_താക്കോൽ, thread_destructor);
}

സ്ഥിതം ശൂന്യം __attribute__((destructor)) finalizer(ശൂന്യം) { rpmalloc_finalize(); }

തരനിർവചനം ഘടന {
  ശൂന്യം *(*real_start)(ശൂന്യം *);
  ശൂന്യം *real_arg;
} thread_starter_arg;

സ്ഥിതം ശൂന്യം *thread_starter(ശൂന്യം *argptr) {
  thread_starter_arg *arg = argptr;
  ശൂന്യം *(*real_start)(ശൂന്യം *) = arg->real_start;
  ശൂന്യം *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_താക്കോൽ, (ശൂന്യം *)1);
  തിരികെ (*real_start)(real_arg);
}

സ്ഥിതം ശൂന്യം thread_destructor(ശൂന്യം *മൂല്യം_2) {
  (ശൂന്യം)വലുപ്പം(മൂല്യം_2);
  rpmalloc_thread_finalize(1);
}

#ifdef __APPLE__

സ്ഥിതം പൂർണ്ണസംഖ്യ pthread_create_proxy(pthread_t *thread, സ്ഥിരം pthread_attr_t *attr,
                                ശൂന്യം *(*start_routine)(ശൂന്യം *), ശൂന്യം *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(വലുപ്പം(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  തിരികെ pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#അല്ലെങ്കിൽ

#അന്തര്ഭാവ <dlfcn.h>

പൂർണ്ണസംഖ്യ pthread_create(pthread_t *thread, സ്ഥിരം pthread_attr_t *attr,
                   ശൂന്യം *(*start_routine)(ശൂന്യം *), ശൂന്യം *arg) {
#എങ്കിൽ defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  അക്ഷരം fname[] = "pthread_create";
#അല്ലെങ്കിൽ
  അക്ഷരം fname[] = "_pthread_create";
#endif
  ശൂന്യം *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(വലുപ്പം(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  തിരികെ (*(പൂർണ്ണസംഖ്യ (*)(pthread_t *, സ്ഥിരം pthread_attr_t *, ശൂന്യം *(*)(ശൂന്യം *),
                    ശൂന്യം *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#endif

#endif

#endif

#എങ്കിൽ ENABLE_OVERRIDE

#എങ്കിൽ defined(__GLIBC__) && defined(__linux__)

ശൂന്യം *__libc_malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) ശൂന്യം *__libc_calloc(size_t എണ്ണം, size_t size)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) ശൂന്യം *__libc_realloc(ശൂന്യം *p, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2) RPALIAS(rprealloc) ശൂന്യം __libc_free(ശൂന്യം *p)
        RPALIAS(rpfree) ശൂന്യം __libc_cfree(ശൂന്യം *p)
            RPALIAS(rpfree) ശൂന്യം *__libc_memalign(size_t align, size_t size)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) പൂർണ്ണസംഖ്യ __posix_memalign(ശൂന്യം **p, size_t align,
                                                 size_t size)
            RPALIAS(rpposix_memalign)

                ബാഹ്യ ശൂന്യം *__libc_valloc(size_t size);
ബാഹ്യ ശൂന്യം *__libc_pvalloc(size_t size);

ശൂന്യം *__libc_valloc(size_t size) { തിരികെ valloc(size); }

ശൂന്യം *__libc_pvalloc(size_t size) { തിരികെ pvalloc(size); }

#endif

#endif

#എങ്കിൽ (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility എടുക്കുക
#endif
