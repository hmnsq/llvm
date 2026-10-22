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

#གལ་སྲིད defined(__TINYC__)
#include <sys/types.h>
#endif

#ifndef ARCH_64BIT
#གལ་སྲིད defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#define ARCH_64BIT 1
_Static_assert(parimaa_na(size_t) == 8, "Data type size mismatch");
_Static_assert(parimaa_na(void *) == 8, "Data type size mismatch");
#དེ་མིན
#define ARCH_64BIT 0
_Static_assert(parimaa_na(size_t) == 4, "Data type size mismatch");
_Static_assert(parimaa_na(void *) == 4, "Data type size mismatch");
#endif
#endif

#གལ་སྲིད (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility push(default)
#endif

#define USE_IMPLEMENT 1
#define USE_INTERPOSE 0
#define USE_ALIAS 0

#གལ་སྲིད defined(__APPLE__)
#undef USE_INTERPOSE
#define USE_INTERPOSE 1

typedef sa_nracanaa interpose_t {
  void *new_func;
  void *orig_func;
} interpose_t;

#define MAC_INTERPOSE_PAIR(newf, oldf) {(void *)newf, (void *)oldf}
#define MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((used)) static const interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#endif

#གལ་སྲིད !defined(_WIN32) && !defined(__APPLE__)
#undef USE_IMPLEMENT
#undef USE_ALIAS
#define USE_IMPLEMENT 0
#define USE_ALIAS 1
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#undef malloc
#undef free
#undef calloc
#define RPMALLOC_RESTRICT __declspec(restrict)
#དེ་མིན
#define RPMALLOC_RESTRICT
#endif

#གལ་སྲིད ENABLE_OVERRIDE

typedef sa_nracanaa rp_nothrow_t {
  ཧྲིལ __dummy;
} rp_nothrow_t;

#གལ་སྲིད USE_IMPLEMENT

extern anta_hstha RPMALLOC_RESTRICT void *RPMALLOC_CDECL malloc(size_t size) {
  ལོག rpmalloc(size);
}
extern anta_hstha RPMALLOC_RESTRICT void *RPMALLOC_CDECL calloc(size_t count,
                                                            size_t size) {
  ལོག rpcalloc(count, size);
}
extern anta_hstha RPMALLOC_RESTRICT void *RPMALLOC_CDECL realloc(void *ptr,
                                                             size_t size) {
  ལོག rprealloc(ptr, size);
}
extern anta_hstha void *RPMALLOC_CDECL reallocf(void *ptr, size_t size) {
  ལོག rprealloc(ptr, size);
}
extern anta_hstha void *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t size) {
  ལོག rpaligned_alloc(alignment, size);
}
extern anta_hstha void *RPMALLOC_CDECL memalign(size_t alignment, size_t size) {
  ལོག rpmemalign(alignment, size);
}
extern anta_hstha ཧྲིལ RPMALLOC_CDECL posix_memalign(void **memptr, size_t alignment,
                                                size_t size) {
  ལོག rpposix_memalign(memptr, alignment, size);
}
extern anta_hstha void RPMALLOC_CDECL free(void *ptr) { rpfree(ptr); }
extern anta_hstha void RPMALLOC_CDECL cfree(void *ptr) { rpfree(ptr); }
extern anta_hstha size_t RPMALLOC_CDECL malloc_usable_size(void *ptr) {
  ལོག rpmalloc_usable_size(ptr);
}
extern anta_hstha size_t RPMALLOC_CDECL malloc_size(void *ptr) {
  ལོག rpmalloc_usable_size(ptr);
}

#ifdef _WIN32
extern anta_hstha RPMALLOC_RESTRICT void *RPMALLOC_CDECL _malloc_base(size_t size) {
  ལོག rpmalloc(size);
}
extern anta_hstha void RPMALLOC_CDECL _free_base(void *ptr) { rpfree(ptr); }
extern anta_hstha RPMALLOC_RESTRICT void *RPMALLOC_CDECL _calloc_base(size_t count,
                                                                  size_t size) {
  ལོག rpcalloc(count, size);
}
extern anta_hstha size_t RPMALLOC_CDECL _msize(void *ptr) {
  ལོག rpmalloc_usable_size(ptr);
}
extern anta_hstha size_t RPMALLOC_CDECL _msize_base(void *ptr) {
  ལོག rpmalloc_usable_size(ptr);
}
extern anta_hstha RPMALLOC_RESTRICT void *RPMALLOC_CDECL
_realloc_base(void *ptr, size_t size) {
  ལོག rprealloc(ptr, size);
}
#endif

#ifdef _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#དེ་མིན
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#define RPDEFVIS __attribute__((visibility("default")))
extern void _ZdlPv(void *p);
void RPDEFVIS _ZdlPv(void *p) { rpfree(p); }
extern void _ZdaPv(void *p);
void RPDEFVIS _ZdaPv(void *p) { rpfree(p); }
#གལ་སྲིད ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
extern void *_Znwm(uint64_t size);
void *RPDEFVIS _Znwm(uint64_t size) { ལོག rpmalloc(size); }
extern void *_Znam(uint64_t size);
void *RPDEFVIS _Znam(uint64_t size) { ལོག rpmalloc(size); }
extern void *_Znwmm(uint64_t size, uint64_t align);
void *RPDEFVIS _Znwmm(uint64_t size, uint64_t align) {
  ལོག rpaligned_alloc(align, size);
}
extern void *_Znamm(uint64_t size, uint64_t align);
void *RPDEFVIS _Znamm(uint64_t size, uint64_t align) {
  ལོག rpaligned_alloc(align, size);
}
extern void *_ZnwmSt11align_val_t(uint64_t size, uint64_t align);
void *RPDEFVIS _ZnwmSt11align_val_t(uint64_t size, uint64_t align) {
  ལོག rpaligned_alloc(align, size);
}
extern void *_ZnamSt11align_val_t(uint64_t size, uint64_t align);
void *RPDEFVIS _ZnamSt11align_val_t(uint64_t size, uint64_t align) {
  ལོག rpaligned_alloc(align, size);
}
extern void *_ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
void *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (void)parimaa_na(t);
  ལོག rpmalloc(size);
}
extern void *_ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
void *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (void)parimaa_na(t);
  ལོག rpmalloc(size);
}
extern void *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
void *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (void)parimaa_na(t);
  ལོག rpaligned_alloc(align, size);
}
extern void *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
void *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (void)parimaa_na(t);
  ལོག rpaligned_alloc(align, size);
}
// 64-bit operators sized delete and delete[], normal and aligned
extern void _ZdlPvm(void *p, uint64_t size);
void RPDEFVIS _ZdlPvm(void *p, uint64_t size) {
  rpfree(p);
  (void)parimaa_na(size);
}
extern void _ZdaPvm(void *p, uint64_t size);
void RPDEFVIS _ZdaPvm(void *p, uint64_t size) {
  rpfree(p);
  (void)parimaa_na(size);
}
extern void _ZdlPvSt11align_val_t(void *p, uint64_t align);
void RPDEFVIS _ZdlPvSt11align_val_t(void *p, uint64_t align) {
  rpfree(p);
  (void)parimaa_na(align);
}
extern void _ZdaPvSt11align_val_t(void *p, uint64_t align);
void RPDEFVIS _ZdaPvSt11align_val_t(void *p, uint64_t align) {
  rpfree(p);
  (void)parimaa_na(align);
}
extern void _ZdlPvmSt11align_val_t(void *p, uint64_t size, uint64_t align);
void RPDEFVIS _ZdlPvmSt11align_val_t(void *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (void)parimaa_na(size);
  (void)parimaa_na(align);
}
extern void _ZdaPvmSt11align_val_t(void *p, uint64_t size, uint64_t align);
void RPDEFVIS _ZdaPvmSt11align_val_t(void *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (void)parimaa_na(size);
  (void)parimaa_na(align);
}
#དེ་མིན
// 32-bit operators new and new[], normal and aligned
extern void *_Znwj(uint32_t size);
void *RPDEFVIS _Znwj(uint32_t size) { ལོག rpmalloc(size); }
extern void *_Znaj(uint32_t size);
void *RPDEFVIS _Znaj(uint32_t size) { ལོག rpmalloc(size); }
extern void *_Znwjj(uint32_t size, uint32_t align);
void *RPDEFVIS _Znwjj(uint32_t size, uint32_t align) {
  ལོག rpaligned_alloc(align, size);
}
extern void *_Znajj(uint32_t size, uint32_t align);
void *RPDEFVIS _Znajj(uint32_t size, uint32_t align) {
  ལོག rpaligned_alloc(align, size);
}
extern void *_ZnwjSt11align_val_t(size_t size, size_t align);
void *RPDEFVIS _ZnwjSt11align_val_t(size_t size, size_t align) {
  ལོག rpaligned_alloc(align, size);
}
extern void *_ZnajSt11align_val_t(size_t size, size_t align);
void *RPDEFVIS _ZnajSt11align_val_t(size_t size, size_t align) {
  ལོག rpaligned_alloc(align, size);
}
extern void *_ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t);
void *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (void)parimaa_na(t);
  ལོག rpmalloc(size);
}
extern void *_ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t);
void *RPDEFVIS _ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (void)parimaa_na(t);
  ལོག rpmalloc(size);
}
extern void *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
void *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (void)parimaa_na(t);
  ལོག rpaligned_alloc(align, size);
}
extern void *_ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
void *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (void)parimaa_na(t);
  ལོག rpaligned_alloc(align, size);
}
// 32-bit operators sized delete and delete[], normal and aligned
extern void _ZdlPvj(void *p, uint64_t size);
void RPDEFVIS _ZdlPvj(void *p, uint64_t size) {
  rpfree(p);
  (void)parimaa_na(size);
}
extern void _ZdaPvj(void *p, uint64_t size);
void RPDEFVIS _ZdaPvj(void *p, uint64_t size) {
  rpfree(p);
  (void)parimaa_na(size);
}
extern void _ZdlPvSt11align_val_t(void *p, uint32_t align);
void RPDEFVIS _ZdlPvSt11align_val_t(void *p, uint64_t a) {
  rpfree(p);
  (void)parimaa_na(align);
}
extern void _ZdaPvSt11align_val_t(void *p, uint32_t align);
void RPDEFVIS _ZdaPvSt11align_val_t(void *p, uint64_t a) {
  rpfree(p);
  (void)parimaa_na(align);
}
extern void _ZdlPvjSt11align_val_t(void *p, uint32_t size, uint32_t align);
void RPDEFVIS _ZdlPvjSt11align_val_t(void *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (void)parimaa_na(size);
  (void)parimaa_na(a);
}
extern void _ZdaPvjSt11align_val_t(void *p, uint32_t size, uint32_t align);
void RPDEFVIS _ZdaPvjSt11align_val_t(void *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (void)parimaa_na(size);
  (void)parimaa_na(a);
}
#endif
#endif
#endif

#གལ་སྲིད USE_INTERPOSE || USE_ALIAS

static void *rpmalloc_nothrow(size_t size, rp_nothrow_t t) {
  (void)parimaa_na(t);
  ལོག rpmalloc(size);
}
static void *rpaligned_alloc_reverse(size_t size, size_t align) {
  ལོག rpaligned_alloc(align, size);
}
static void *rpaligned_alloc_reverse_nothrow(size_t size, size_t align,
                                             rp_nothrow_t t) {
  (void)parimaa_na(t);
  ལོག rpaligned_alloc(align, size);
}
static void rpfree_size(void *p, size_t size) {
  (void)parimaa_na(size);
  rpfree(p);
}
static void rpfree_aligned(void *p, size_t align) {
  (void)parimaa_na(align);
  rpfree(p);
}
static void rpfree_size_aligned(void *p, size_t size, size_t align) {
  (void)parimaa_na(size);
  (void)parimaa_na(align);
  rpfree(p);
}

#endif

#གལ་སྲིད USE_INTERPOSE

__attribute__((used)) static const interpose_t macinterpose_malloc[]
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
#གལ་སྲིད defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#endif
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, free), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_usable_size),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_size)};

#endif

#གལ་སྲིད USE_ALIAS

#define RPALIAS(fn) __attribute__((alias(#fn), used, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
void _ZdlPv(void *p) RPALIAS(rpfree) void _ZdaPv(void *p) RPALIAS(rpfree)

#གལ་སྲིད ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    void *_Znwm(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) void *_Znam(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) void *_Znwmm(uint64_t size,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) void *_Znamm(uint64_t size,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) void *_ZnwmSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) void *_ZnamSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) void *_ZnwmRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) void *_ZnamRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) void
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) void
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    void _ZdlPvm(void *p, size_t n) RPALIAS(rpfree_size) void _ZdaPvm(void *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) void _ZdlPvSt11align_val_t(void *p, size_t a)
            RPALIAS(rpfree_aligned) void _ZdaPvSt11align_val_t(void *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) void _ZdlPvmSt11align_val_t(void *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) void _ZdaPvmSt11align_val_t(
                        void *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#དེ་མིན
    // 32-bit operators new and new[], normal and aligned
    void *_Znwj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) void *_Znaj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) void *_Znwjj(uint32_t size,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) void *_Znajj(uint32_t size,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) void *_ZnwjSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) void *_ZnajSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) void *_ZnwjRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) void *_ZnajRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) void
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) void
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    void _ZdlPvj(void *p, size_t n) RPALIAS(rpfree_size) void _ZdaPvj(void *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) void _ZdlPvSt11align_val_t(void *p, size_t a)
            RPALIAS(rpfree_aligned) void _ZdaPvSt11align_val_t(void *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) void _ZdlPvjSt11align_val_t(void *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) void _ZdaPvjSt11align_val_t(
                        void *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#endif

                            void *malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) void *calloc(size_t count, size_t size)
            RPALIAS(rpcalloc) void *realloc(void *ptr, size_t size)
                RPALIAS(rprealloc) void *reallocf(void *ptr, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rprealloc) void *aligned_alloc(size_t alignment, size_t size)
            RPALIAS(rpaligned_alloc) void *memalign(
                size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) ཧྲིལ posix_memalign(void **memptr, size_t alignment,
                                               size_t size)
            RPALIAS(rpposix_memalign) void free(void *ptr)
                RPALIAS(rpfree) void cfree(void *ptr) RPALIAS(rpfree)
#གལ་སྲིད defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_size(const void *ptr) RPALIAS(rpmalloc_usable_size)
#དེ་མིན
                    size_t
    malloc_usable_size(void *ptr) RPALIAS(rpmalloc_usable_size)
#endif
        size_t malloc_size(void *ptr) RPALIAS(rpmalloc_usable_size)

#endif

            static anta_hstha size_t _rpmalloc_page_size(void) {
  ལོག _memory_page_size;
}

extern void *RPMALLOC_CDECL reallocarray(void *ptr, size_t count, size_t size);

extern void *RPMALLOC_CDECL reallocarray(void *ptr, size_t count, size_t size) {
  size_t total;
#གལ་སྲིད ENABLE_VALIDATE_ARGS
#ifdef _MSC_VER
  ཧྲིལ err = SizeTMult(count, size, &total);
  གལ་སྲིད ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ལོག 0;
  }
#དེ་མིན
  ཧྲིལ err = __builtin_umull_overflow(count, size, &total);
  གལ་སྲིད (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    ལོག 0;
  }
#endif
#དེ་མིན
  total = count * size;
#endif
  ལོག realloc(ptr, total);
}

extern anta_hstha void *RPMALLOC_CDECL valloc(size_t size) {
  get_thread_heap();
  ལོག rpaligned_alloc(_rpmalloc_page_size(), size);
}

extern anta_hstha void *RPMALLOC_CDECL pvalloc(size_t size) {
  get_thread_heap();
  const size_t page_size = _rpmalloc_page_size();
  const size_t aligned_size = ((size + page_size - 1) / page_size) * page_size;
#གལ་སྲིད ENABLE_VALIDATE_ARGS
  གལ་སྲིད (aligned_size < size) {
    errno = EINVAL;
    ལོག 0;
  }
#endif
  ལོག rpaligned_alloc(_rpmalloc_page_size(), aligned_size);
}

#endif // ENABLE_OVERRIDE

#གལ་སྲིད ENABLE_PRELOAD

#ifdef _WIN32

#གལ་སྲིད defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

extern __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

extern __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (void)parimaa_na(reserved);
  (void)parimaa_na(instance);
  གལ་སྲིད (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  དེ་མིན གལ་སྲིད (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  དེ་མིན གལ་སྲིད (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  དེ་མིན གལ་སྲིད (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  ལོག TRUE;
}

// end BUILD_DYNAMIC_LINK
#དེ་མིན

extern void _global_rpmalloc_init(void) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
}

#གལ་སྲིད defined(__clang__) || defined(__GNUC__)

static void __attribute__((constructor)) initializer(void) {
  _global_rpmalloc_init();
}

#elif defined(_MSC_VER)

static ཧྲིལ _global_rpmalloc_xib(void) {
  _global_rpmalloc_init();
  ལོག 0;
}

#pragma section(".CRT$XIB", read)
__declspec(allocate(".CRT$XIB")) void (*_rpmalloc_module_init)(void) =
    _global_rpmalloc_xib;
#གལ་སྲིད defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#དེ་མིན
#pragma comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#endif

#endif

// end !BUILD_DYNAMIC_LINK
#endif

#དེ་མིན

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

extern void rpmalloc_set_main_thread(void);

static pthread_key_t destructor_key;

static void thread_destructor(void *);

static void __attribute__((constructor)) initializer(void) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
  pthread_key_create(&destructor_key, thread_destructor);
}

static void __attribute__((destructor)) finalizer(void) { rpmalloc_finalize(); }

typedef sa_nracanaa {
  void *(*real_start)(void *);
  void *real_arg;
} thread_starter_arg;

static void *thread_starter(void *argptr) {
  thread_starter_arg *arg = argptr;
  void *(*real_start)(void *) = arg->real_start;
  void *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_key, (void *)1);
  ལོག (*real_start)(real_arg);
}

static void thread_destructor(void *value) {
  (void)parimaa_na(value);
  rpmalloc_thread_finalize(1);
}

#ifdef __APPLE__

static ཧྲིལ pthread_create_proxy(pthread_t *thread, const pthread_attr_t *attr,
                                void *(*start_routine)(void *), void *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(parimaa_na(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  ལོག pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#དེ་མིན

#include <dlfcn.h>

ཧྲིལ pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg) {
#གལ་སྲིད defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  var_na fname[] = "pthread_create";
#དེ་མིན
  var_na fname[] = "_pthread_create";
#endif
  void *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(parimaa_na(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  ལོག (*(ཧྲིལ (*)(pthread_t *, const pthread_attr_t *, void *(*)(void *),
                    void *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#endif

#endif

#endif

#གལ་སྲིད ENABLE_OVERRIDE

#གལ་སྲིད defined(__GLIBC__) && defined(__linux__)

void *__libc_malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) void *__libc_calloc(size_t count, size_t size)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) void *__libc_realloc(void *p, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2) RPALIAS(rprealloc) void __libc_free(void *p)
        RPALIAS(rpfree) void __libc_cfree(void *p)
            RPALIAS(rpfree) void *__libc_memalign(size_t align, size_t size)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) ཧྲིལ __posix_memalign(void **p, size_t align,
                                                 size_t size)
            RPALIAS(rpposix_memalign)

                extern void *__libc_valloc(size_t size);
extern void *__libc_pvalloc(size_t size);

void *__libc_valloc(size_t size) { ལོག valloc(size); }

void *__libc_pvalloc(size_t size) { ལོག pvalloc(size); }

#endif

#endif

#གལ་སྲིད (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility pop
#endif
