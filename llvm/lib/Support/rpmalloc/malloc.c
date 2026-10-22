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

#ଯଦି defined(__TINYC__)
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#endif

#ifndef ARCH_64BIT
#ଯଦି defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#define ARCH_64BIT 1
_Static_assert(ଆକାର(size_t) == 8, "Data type size mismatch");
_Static_assert(ଆକାର(ଶୂନ୍ୟ *) == 8, "Data type size mismatch");
#ଅନ୍ୟଥା
#define ARCH_64BIT 0
_Static_assert(ଆକାର(size_t) == 4, "Data type size mismatch");
_Static_assert(ଆକାର(ଶୂନ୍ୟ *) == 4, "Data type size mismatch");
#endif
#endif

#ଯଦି (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility ଠେଲ(ପୂର୍ବନିର୍ଦ୍ଧାରିତ)
#endif

#define USE_IMPLEMENT 1
#define USE_INTERPOSE 0
#define USE_ALIAS 0

#ଯଦି defined(__APPLE__)
#undef USE_INTERPOSE
#define USE_INTERPOSE 1

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା interpose_t {
  ଶୂନ୍ୟ *new_func;
  ଶୂନ୍ୟ *orig_func;
} interpose_t;

#define MAC_INTERPOSE_PAIR(newf, oldf) {(ଶୂନ୍ୟ *)newf, (ଶୂନ୍ୟ *)oldf}
#define MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((used)) ସ୍ଥିର ଅଚଳ interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#endif

#ଯଦି !defined(_WIN32) && !defined(__APPLE__)
#undef USE_IMPLEMENT
#undef USE_ALIAS
#define USE_IMPLEMENT 0
#define USE_ALIAS 1
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#undef malloc
#undef ମୁକ୍ତ
#undef calloc
#define RPMALLOC_RESTRICT __declspec(restrict)
#ଅନ୍ୟଥା
#define RPMALLOC_RESTRICT
#endif

#ଯଦି ENABLE_OVERRIDE

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା rp_nothrow_t {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା __dummy;
} rp_nothrow_t;

#ଯଦି USE_IMPLEMENT

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_RESTRICT ଶୂନ୍ୟ *RPMALLOC_CDECL malloc(size_t size) {
  ଫେରନ୍ତୁ rpmalloc(size);
}
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_RESTRICT ଶୂନ୍ୟ *RPMALLOC_CDECL calloc(size_t ଗଣନା_2,
                                                            size_t size) {
  ଫେରନ୍ତୁ rpcalloc(ଗଣନା_2, size);
}
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_RESTRICT ଶୂନ୍ୟ *RPMALLOC_CDECL realloc(ଶୂନ୍ୟ *ptr,
                                                             size_t size) {
  ଫେରନ୍ତୁ rprealloc(ptr, size);
}
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *RPMALLOC_CDECL reallocf(ଶୂନ୍ୟ *ptr, size_t size) {
  ଫେରନ୍ତୁ rprealloc(ptr, size);
}
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t size) {
  ଫେରନ୍ତୁ rpaligned_alloc(alignment, size);
}
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *RPMALLOC_CDECL memalign(size_t alignment, size_t size) {
  ଫେରନ୍ତୁ rpmemalign(alignment, size);
}
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା RPMALLOC_CDECL posix_memalign(ଶୂନ୍ୟ **memptr, size_t alignment,
                                                size_t size) {
  ଫେରନ୍ତୁ rpposix_memalign(memptr, alignment, size);
}
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ RPMALLOC_CDECL ମୁକ୍ତ(ଶୂନ୍ୟ *ptr) { rpfree(ptr); }
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ RPMALLOC_CDECL cfree(ଶୂନ୍ୟ *ptr) { rpfree(ptr); }
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ size_t RPMALLOC_CDECL malloc_usable_size(ଶୂନ୍ୟ *ptr) {
  ଫେରନ୍ତୁ rpmalloc_usable_size(ptr);
}
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ size_t RPMALLOC_CDECL malloc_size(ଶୂନ୍ୟ *ptr) {
  ଫେରନ୍ତୁ rpmalloc_usable_size(ptr);
}

#ifdef _WIN32
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_RESTRICT ଶୂନ୍ୟ *RPMALLOC_CDECL _malloc_base(size_t size) {
  ଫେରନ୍ତୁ rpmalloc(size);
}
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ RPMALLOC_CDECL _free_base(ଶୂନ୍ୟ *ptr) { rpfree(ptr); }
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_RESTRICT ଶୂନ୍ୟ *RPMALLOC_CDECL _calloc_base(size_t ଗଣନା_2,
                                                                  size_t size) {
  ଫେରନ୍ତୁ rpcalloc(ଗଣନା_2, size);
}
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ size_t RPMALLOC_CDECL _msize(ଶୂନ୍ୟ *ptr) {
  ଫେରନ୍ତୁ rpmalloc_usable_size(ptr);
}
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ size_t RPMALLOC_CDECL _msize_base(ଶୂନ୍ୟ *ptr) {
  ଫେରନ୍ତୁ rpmalloc_usable_size(ptr);
}
ବାହ୍ୟ ଧାଡ଼ିଭିତରେ RPMALLOC_RESTRICT ଶୂନ୍ୟ *RPMALLOC_CDECL
_realloc_base(ଶୂନ୍ୟ *ptr, size_t size) {
  ଫେରନ୍ତୁ rprealloc(ptr, size);
}
#endif

#ifdef _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#ଅନ୍ୟଥା
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#define RPDEFVIS __attribute__((visibility("default")))
ବାହ୍ୟ ଶୂନ୍ୟ _ZdlPv(ଶୂନ୍ୟ *p);
ଶୂନ୍ୟ RPDEFVIS _ZdlPv(ଶୂନ୍ୟ *p) { rpfree(p); }
ବାହ୍ୟ ଶୂନ୍ୟ _ZdaPv(ଶୂନ୍ୟ *p);
ଶୂନ୍ୟ RPDEFVIS _ZdaPv(ଶୂନ୍ୟ *p) { rpfree(p); }
#ଯଦି ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
ବାହ୍ୟ ଶୂନ୍ୟ *_Znwm(uint64_t size);
ଶୂନ୍ୟ *RPDEFVIS _Znwm(uint64_t size) { ଫେରନ୍ତୁ rpmalloc(size); }
ବାହ୍ୟ ଶୂନ୍ୟ *_Znam(uint64_t size);
ଶୂନ୍ୟ *RPDEFVIS _Znam(uint64_t size) { ଫେରନ୍ତୁ rpmalloc(size); }
ବାହ୍ୟ ଶୂନ୍ୟ *_Znwmm(uint64_t size, uint64_t align);
ଶୂନ୍ୟ *RPDEFVIS _Znwmm(uint64_t size, uint64_t align) {
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_Znamm(uint64_t size, uint64_t align);
ଶୂନ୍ୟ *RPDEFVIS _Znamm(uint64_t size, uint64_t align) {
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_ZnwmSt11align_val_t(uint64_t size, uint64_t align);
ଶୂନ୍ୟ *RPDEFVIS _ZnwmSt11align_val_t(uint64_t size, uint64_t align) {
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_ZnamSt11align_val_t(uint64_t size, uint64_t align);
ଶୂନ୍ୟ *RPDEFVIS _ZnamSt11align_val_t(uint64_t size, uint64_t align) {
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
ଶୂନ୍ୟ *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (ଶୂନ୍ୟ)ଆକାର(t);
  ଫେରନ୍ତୁ rpmalloc(size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
ଶୂନ୍ୟ *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (ଶୂନ୍ୟ)ଆକାର(t);
  ଫେରନ୍ତୁ rpmalloc(size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
ଶୂନ୍ୟ *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (ଶୂନ୍ୟ)ଆକାର(t);
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
ଶୂନ୍ୟ *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (ଶୂନ୍ୟ)ଆକାର(t);
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
// 64-bit operators sized delete and delete[], normal and aligned
ବାହ୍ୟ ଶୂନ୍ୟ _ZdlPvm(ଶୂନ୍ୟ *p, uint64_t size);
ଶୂନ୍ୟ RPDEFVIS _ZdlPvm(ଶୂନ୍ୟ *p, uint64_t size) {
  rpfree(p);
  (ଶୂନ୍ୟ)ଆକାର(size);
}
ବାହ୍ୟ ଶୂନ୍ୟ _ZdaPvm(ଶୂନ୍ୟ *p, uint64_t size);
ଶୂନ୍ୟ RPDEFVIS _ZdaPvm(ଶୂନ୍ୟ *p, uint64_t size) {
  rpfree(p);
  (ଶୂନ୍ୟ)ଆକାର(size);
}
ବାହ୍ୟ ଶୂନ୍ୟ _ZdlPvSt11align_val_t(ଶୂନ୍ୟ *p, uint64_t align);
ଶୂନ୍ୟ RPDEFVIS _ZdlPvSt11align_val_t(ଶୂନ୍ୟ *p, uint64_t align) {
  rpfree(p);
  (ଶୂନ୍ୟ)ଆକାର(align);
}
ବାହ୍ୟ ଶୂନ୍ୟ _ZdaPvSt11align_val_t(ଶୂନ୍ୟ *p, uint64_t align);
ଶୂନ୍ୟ RPDEFVIS _ZdaPvSt11align_val_t(ଶୂନ୍ୟ *p, uint64_t align) {
  rpfree(p);
  (ଶୂନ୍ୟ)ଆକାର(align);
}
ବାହ୍ୟ ଶୂନ୍ୟ _ZdlPvmSt11align_val_t(ଶୂନ୍ୟ *p, uint64_t size, uint64_t align);
ଶୂନ୍ୟ RPDEFVIS _ZdlPvmSt11align_val_t(ଶୂନ୍ୟ *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (ଶୂନ୍ୟ)ଆକାର(size);
  (ଶୂନ୍ୟ)ଆକାର(align);
}
ବାହ୍ୟ ଶୂନ୍ୟ _ZdaPvmSt11align_val_t(ଶୂନ୍ୟ *p, uint64_t size, uint64_t align);
ଶୂନ୍ୟ RPDEFVIS _ZdaPvmSt11align_val_t(ଶୂନ୍ୟ *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (ଶୂନ୍ୟ)ଆକାର(size);
  (ଶୂନ୍ୟ)ଆକାର(align);
}
#ଅନ୍ୟଥା
// 32-bit operators new and new[], normal and aligned
ବାହ୍ୟ ଶୂନ୍ୟ *_Znwj(uint32_t size);
ଶୂନ୍ୟ *RPDEFVIS _Znwj(uint32_t size) { ଫେରନ୍ତୁ rpmalloc(size); }
ବାହ୍ୟ ଶୂନ୍ୟ *_Znaj(uint32_t size);
ଶୂନ୍ୟ *RPDEFVIS _Znaj(uint32_t size) { ଫେରନ୍ତୁ rpmalloc(size); }
ବାହ୍ୟ ଶୂନ୍ୟ *_Znwjj(uint32_t size, uint32_t align);
ଶୂନ୍ୟ *RPDEFVIS _Znwjj(uint32_t size, uint32_t align) {
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_Znajj(uint32_t size, uint32_t align);
ଶୂନ୍ୟ *RPDEFVIS _Znajj(uint32_t size, uint32_t align) {
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_ZnwjSt11align_val_t(size_t size, size_t align);
ଶୂନ୍ୟ *RPDEFVIS _ZnwjSt11align_val_t(size_t size, size_t align) {
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_ZnajSt11align_val_t(size_t size, size_t align);
ଶୂନ୍ୟ *RPDEFVIS _ZnajSt11align_val_t(size_t size, size_t align) {
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t);
ଶୂନ୍ୟ *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (ଶୂନ୍ୟ)ଆକାର(t);
  ଫେରନ୍ତୁ rpmalloc(size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t);
ଶୂନ୍ୟ *RPDEFVIS _ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (ଶୂନ୍ୟ)ଆକାର(t);
  ଫେରନ୍ତୁ rpmalloc(size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
ଶୂନ୍ୟ *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (ଶୂନ୍ୟ)ଆକାର(t);
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
ବାହ୍ୟ ଶୂନ୍ୟ *_ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
ଶୂନ୍ୟ *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (ଶୂନ୍ୟ)ଆକାର(t);
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
// 32-bit operators sized delete and delete[], normal and aligned
ବାହ୍ୟ ଶୂନ୍ୟ _ZdlPvj(ଶୂନ୍ୟ *p, uint64_t size);
ଶୂନ୍ୟ RPDEFVIS _ZdlPvj(ଶୂନ୍ୟ *p, uint64_t size) {
  rpfree(p);
  (ଶୂନ୍ୟ)ଆକାର(size);
}
ବାହ୍ୟ ଶୂନ୍ୟ _ZdaPvj(ଶୂନ୍ୟ *p, uint64_t size);
ଶୂନ୍ୟ RPDEFVIS _ZdaPvj(ଶୂନ୍ୟ *p, uint64_t size) {
  rpfree(p);
  (ଶୂନ୍ୟ)ଆକାର(size);
}
ବାହ୍ୟ ଶୂନ୍ୟ _ZdlPvSt11align_val_t(ଶୂନ୍ୟ *p, uint32_t align);
ଶୂନ୍ୟ RPDEFVIS _ZdlPvSt11align_val_t(ଶୂନ୍ୟ *p, uint64_t a) {
  rpfree(p);
  (ଶୂନ୍ୟ)ଆକାର(align);
}
ବାହ୍ୟ ଶୂନ୍ୟ _ZdaPvSt11align_val_t(ଶୂନ୍ୟ *p, uint32_t align);
ଶୂନ୍ୟ RPDEFVIS _ZdaPvSt11align_val_t(ଶୂନ୍ୟ *p, uint64_t a) {
  rpfree(p);
  (ଶୂନ୍ୟ)ଆକାର(align);
}
ବାହ୍ୟ ଶୂନ୍ୟ _ZdlPvjSt11align_val_t(ଶୂନ୍ୟ *p, uint32_t size, uint32_t align);
ଶୂନ୍ୟ RPDEFVIS _ZdlPvjSt11align_val_t(ଶୂନ୍ୟ *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (ଶୂନ୍ୟ)ଆକାର(size);
  (ଶୂନ୍ୟ)ଆକାର(a);
}
ବାହ୍ୟ ଶୂନ୍ୟ _ZdaPvjSt11align_val_t(ଶୂନ୍ୟ *p, uint32_t size, uint32_t align);
ଶୂନ୍ୟ RPDEFVIS _ZdaPvjSt11align_val_t(ଶୂନ୍ୟ *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (ଶୂନ୍ୟ)ଆକାର(size);
  (ଶୂନ୍ୟ)ଆକାର(a);
}
#endif
#endif
#endif

#ଯଦି USE_INTERPOSE || USE_ALIAS

ସ୍ଥିର ଶୂନ୍ୟ *rpmalloc_nothrow(size_t size, rp_nothrow_t t) {
  (ଶୂନ୍ୟ)ଆକାର(t);
  ଫେରନ୍ତୁ rpmalloc(size);
}
ସ୍ଥିର ଶୂନ୍ୟ *rpaligned_alloc_reverse(size_t size, size_t align) {
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
ସ୍ଥିର ଶୂନ୍ୟ *rpaligned_alloc_reverse_nothrow(size_t size, size_t align,
                                             rp_nothrow_t t) {
  (ଶୂନ୍ୟ)ଆକାର(t);
  ଫେରନ୍ତୁ rpaligned_alloc(align, size);
}
ସ୍ଥିର ଶୂନ୍ୟ rpfree_size(ଶୂନ୍ୟ *p, size_t size) {
  (ଶୂନ୍ୟ)ଆକାର(size);
  rpfree(p);
}
ସ୍ଥିର ଶୂନ୍ୟ rpfree_aligned(ଶୂନ୍ୟ *p, size_t align) {
  (ଶୂନ୍ୟ)ଆକାର(align);
  rpfree(p);
}
ସ୍ଥିର ଶୂନ୍ୟ rpfree_size_aligned(ଶୂନ୍ୟ *p, size_t size, size_t align) {
  (ଶୂନ୍ୟ)ଆକାର(size);
  (ଶୂନ୍ୟ)ଆକାର(align);
  rpfree(p);
}

#endif

#ଯଦି USE_INTERPOSE

__attribute__((used)) ସ୍ଥିର ଅଚଳ interpose_t macinterpose_malloc[]
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
#ଯଦି defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#endif
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, ମୁକ୍ତ), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_usable_size),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_size)};

#endif

#ଯଦି USE_ALIAS

#define RPALIAS(fn) __attribute__((alias(#fn), used, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
ଶୂନ୍ୟ _ZdlPv(ଶୂନ୍ୟ *p) RPALIAS(rpfree) ଶୂନ୍ୟ _ZdaPv(ଶୂନ୍ୟ *p) RPALIAS(rpfree)

#ଯଦି ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    ଶୂନ୍ୟ *_Znwm(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) ଶୂନ୍ୟ *_Znam(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) ଶୂନ୍ୟ *_Znwmm(uint64_t size,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) ଶୂନ୍ୟ *_Znamm(uint64_t size,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) ଶୂନ୍ୟ *_ZnwmSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) ଶୂନ୍ୟ *_ZnamSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) ଶୂନ୍ୟ *_ZnwmRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) ଶୂନ୍ୟ *_ZnamRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) ଶୂନ୍ୟ
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) ଶୂନ୍ୟ
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    ଶୂନ୍ୟ _ZdlPvm(ଶୂନ୍ୟ *p, size_t n) RPALIAS(rpfree_size) ଶୂନ୍ୟ _ZdaPvm(ଶୂନ୍ୟ *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) ଶୂନ୍ୟ _ZdlPvSt11align_val_t(ଶୂନ୍ୟ *p, size_t a)
            RPALIAS(rpfree_aligned) ଶୂନ୍ୟ _ZdaPvSt11align_val_t(ଶୂନ୍ୟ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) ଶୂନ୍ୟ _ZdlPvmSt11align_val_t(ଶୂନ୍ୟ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) ଶୂନ୍ୟ _ZdaPvmSt11align_val_t(
                        ଶୂନ୍ୟ *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#ଅନ୍ୟଥା
    // 32-bit operators new and new[], normal and aligned
    ଶୂନ୍ୟ *_Znwj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) ଶୂନ୍ୟ *_Znaj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) ଶୂନ୍ୟ *_Znwjj(uint32_t size,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) ଶୂନ୍ୟ *_Znajj(uint32_t size,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) ଶୂନ୍ୟ *_ZnwjSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) ଶୂନ୍ୟ *_ZnajSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) ଶୂନ୍ୟ *_ZnwjRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) ଶୂନ୍ୟ *_ZnajRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) ଶୂନ୍ୟ
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) ଶୂନ୍ୟ
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    ଶୂନ୍ୟ _ZdlPvj(ଶୂନ୍ୟ *p, size_t n) RPALIAS(rpfree_size) ଶୂନ୍ୟ _ZdaPvj(ଶୂନ୍ୟ *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) ଶୂନ୍ୟ _ZdlPvSt11align_val_t(ଶୂନ୍ୟ *p, size_t a)
            RPALIAS(rpfree_aligned) ଶୂନ୍ୟ _ZdaPvSt11align_val_t(ଶୂନ୍ୟ *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) ଶୂନ୍ୟ _ZdlPvjSt11align_val_t(ଶୂନ୍ୟ *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) ଶୂନ୍ୟ _ZdaPvjSt11align_val_t(
                        ଶୂନ୍ୟ *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#endif

                            ଶୂନ୍ୟ *malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) ଶୂନ୍ୟ *calloc(size_t ଗଣନା_2, size_t size)
            RPALIAS(rpcalloc) ଶୂନ୍ୟ *realloc(ଶୂନ୍ୟ *ptr, size_t size)
                RPALIAS(rprealloc) ଶୂନ୍ୟ *reallocf(ଶୂନ୍ୟ *ptr, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rprealloc) ଶୂନ୍ୟ *aligned_alloc(size_t alignment, size_t size)
            RPALIAS(rpaligned_alloc) ଶୂନ୍ୟ *memalign(
                size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) ପୂର୍ଣ୍ଣସଂଖ୍ୟା posix_memalign(ଶୂନ୍ୟ **memptr, size_t alignment,
                                               size_t size)
            RPALIAS(rpposix_memalign) ଶୂନ୍ୟ ମୁକ୍ତ(ଶୂନ୍ୟ *ptr)
                RPALIAS(rpfree) ଶୂନ୍ୟ cfree(ଶୂନ୍ୟ *ptr) RPALIAS(rpfree)
#ଯଦି defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_size(ଅଚଳ ଶୂନ୍ୟ *ptr) RPALIAS(rpmalloc_usable_size)
#ଅନ୍ୟଥା
                    size_t
    malloc_usable_size(ଶୂନ୍ୟ *ptr) RPALIAS(rpmalloc_usable_size)
#endif
        size_t malloc_size(ଶୂନ୍ୟ *ptr) RPALIAS(rpmalloc_usable_size)

#endif

            ସ୍ଥିର ଧାଡ଼ିଭିତରେ size_t _rpmalloc_page_size(ଶୂନ୍ୟ) {
  ଫେରନ୍ତୁ _memory_page_size;
}

ବାହ୍ୟ ଶୂନ୍ୟ *RPMALLOC_CDECL reallocarray(ଶୂନ୍ୟ *ptr, size_t ଗଣନା_2, size_t size);

ବାହ୍ୟ ଶୂନ୍ୟ *RPMALLOC_CDECL reallocarray(ଶୂନ୍ୟ *ptr, size_t ଗଣନା_2, size_t size) {
  size_t total;
#ଯଦି ENABLE_VALIDATE_ARGS
#ifdef _MSC_VER
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = SizeTMult(ଗଣନା_2, size, &total);
  ଯଦି ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#ଅନ୍ୟଥା
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = __builtin_umull_overflow(ଗଣନା_2, size, &total);
  ଯଦି (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#endif
#ଅନ୍ୟଥା
  total = ଗଣନା_2 * size;
#endif
  ଫେରନ୍ତୁ realloc(ptr, total);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *RPMALLOC_CDECL valloc(size_t size) {
  get_thread_heap();
  ଫେରନ୍ତୁ rpaligned_alloc(_rpmalloc_page_size(), size);
}

ବାହ୍ୟ ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *RPMALLOC_CDECL pvalloc(size_t size) {
  get_thread_heap();
  ଅଚଳ size_t page_size = _rpmalloc_page_size();
  ଅଚଳ size_t aligned_size = ((size + page_size - 1) / page_size) * page_size;
#ଯଦି ENABLE_VALIDATE_ARGS
  ଯଦି (aligned_size < size) {
    errno = EINVAL;
    ଫେରନ୍ତୁ 0;
  }
#endif
  ଫେରନ୍ତୁ rpaligned_alloc(_rpmalloc_page_size(), aligned_size);
}

#endif // ENABLE_OVERRIDE

#ଯଦି ENABLE_PRELOAD

#ifdef _WIN32

#ଯଦି defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

ବାହ୍ୟ __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

ବାହ୍ୟ __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (ଶୂନ୍ୟ)ଆକାର(reserved);
  (ଶୂନ୍ୟ)ଆକାର(instance);
  ଯଦି (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  ଅନ୍ୟଥା ଯଦି (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  ଅନ୍ୟଥା ଯଦି (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  ଅନ୍ୟଥା ଯଦି (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  ଫେରନ୍ତୁ TRUE;
}

// end BUILD_DYNAMIC_LINK
#ଅନ୍ୟଥା

ବାହ୍ୟ ଶୂନ୍ୟ _global_rpmalloc_init(ଶୂନ୍ୟ) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
}

#ଯଦି defined(__clang__) || defined(__GNUC__)

ସ୍ଥିର ଶୂନ୍ୟ __attribute__((constructor)) initializer(ଶୂନ୍ୟ) {
  _global_rpmalloc_init();
}

#ନଚେତଯଦି defined(_MSC_VER)

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା _global_rpmalloc_xib(ଶୂନ୍ୟ) {
  _global_rpmalloc_init();
  ଫେରନ୍ତୁ 0;
}

#pragma section(".CRT$XIB", ପଠ)
__declspec(allocate(".CRT$XIB")) ଶୂନ୍ୟ (*_rpmalloc_module_init)(ଶୂନ୍ୟ) =
    _global_rpmalloc_xib;
#ଯଦି defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#ଅନ୍ୟଥା
#pragma comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#endif

#endif

// end !BUILD_DYNAMIC_LINK
#endif

#ଅନ୍ୟଥା

#ଅନ୍ତର୍ଭାଵ <pthread.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

ବାହ୍ୟ ଶୂନ୍ୟ rpmalloc_set_main_thread(ଶୂନ୍ୟ);

ସ୍ଥିର pthread_key_t destructor_ଚାବି;

ସ୍ଥିର ଶୂନ୍ୟ thread_destructor(ଶୂନ୍ୟ *);

ସ୍ଥିର ଶୂନ୍ୟ __attribute__((constructor)) initializer(ଶୂନ୍ୟ) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
  pthread_ଚାବି_create(&destructor_ଚାବି, thread_destructor);
}

ସ୍ଥିର ଶୂନ୍ୟ __attribute__((destructor)) finalizer(ଶୂନ୍ୟ) { rpmalloc_finalize(); }

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
  ଶୂନ୍ୟ *(*real_start)(ଶୂନ୍ୟ *);
  ଶୂନ୍ୟ *real_arg;
} thread_starter_arg;

ସ୍ଥିର ଶୂନ୍ୟ *thread_starter(ଶୂନ୍ୟ *argptr) {
  thread_starter_arg *arg = argptr;
  ଶୂନ୍ୟ *(*real_start)(ଶୂନ୍ୟ *) = arg->real_start;
  ଶୂନ୍ୟ *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_ଚାବି, (ଶୂନ୍ୟ *)1);
  ଫେରନ୍ତୁ (*real_start)(real_arg);
}

ସ୍ଥିର ଶୂନ୍ୟ thread_destructor(ଶୂନ୍ୟ *ମୂଲ୍ୟ_2) {
  (ଶୂନ୍ୟ)ଆକାର(ମୂଲ୍ୟ_2);
  rpmalloc_thread_finalize(1);
}

#ifdef __APPLE__

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା pthread_create_proxy(pthread_t *thread, ଅଚଳ pthread_attr_t *attr,
                                ଶୂନ୍ୟ *(*start_routine)(ଶୂନ୍ୟ *), ଶୂନ୍ୟ *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(ଆକାର(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  ଫେରନ୍ତୁ pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#ଅନ୍ୟଥା

#ଅନ୍ତର୍ଭାଵ <dlfcn.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା pthread_create(pthread_t *thread, ଅଚଳ pthread_attr_t *attr,
                   ଶୂନ୍ୟ *(*start_routine)(ଶୂନ୍ୟ *), ଶୂନ୍ୟ *arg) {
#ଯଦି defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  ଅକ୍ଷର fname[] = "pthread_create";
#ଅନ୍ୟଥା
  ଅକ୍ଷର fname[] = "_pthread_create";
#endif
  ଶୂନ୍ୟ *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(ଆକାର(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  ଫେରନ୍ତୁ (*(ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*)(pthread_t *, ଅଚଳ pthread_attr_t *, ଶୂନ୍ୟ *(*)(ଶୂନ୍ୟ *),
                    ଶୂନ୍ୟ *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#endif

#endif

#endif

#ଯଦି ENABLE_OVERRIDE

#ଯଦି defined(__GLIBC__) && defined(__linux__)

ଶୂନ୍ୟ *__libc_malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) ଶୂନ୍ୟ *__libc_calloc(size_t ଗଣନା_2, size_t size)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) ଶୂନ୍ୟ *__libc_realloc(ଶୂନ୍ୟ *p, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2) RPALIAS(rprealloc) ଶୂନ୍ୟ __libc_free(ଶୂନ୍ୟ *p)
        RPALIAS(rpfree) ଶୂନ୍ୟ __libc_cfree(ଶୂନ୍ୟ *p)
            RPALIAS(rpfree) ଶୂନ୍ୟ *__libc_memalign(size_t align, size_t size)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) ପୂର୍ଣ୍ଣସଂଖ୍ୟା __posix_memalign(ଶୂନ୍ୟ **p, size_t align,
                                                 size_t size)
            RPALIAS(rpposix_memalign)

                ବାହ୍ୟ ଶୂନ୍ୟ *__libc_valloc(size_t size);
ବାହ୍ୟ ଶୂନ୍ୟ *__libc_pvalloc(size_t size);

ଶୂନ୍ୟ *__libc_valloc(size_t size) { ଫେରନ୍ତୁ valloc(size); }

ଶୂନ୍ୟ *__libc_pvalloc(size_t size) { ଫେରନ୍ତୁ pvalloc(size); }

#endif

#endif

#ଯଦି (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility କାଢ଼
#endif
