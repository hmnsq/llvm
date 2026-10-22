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

#எனில் defined(__TINYC__)
#அந்தர்பாவ <sys/types.h>
#endif

#ifndef ARCH_64BIT
#எனில் defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#define ARCH_64BIT 1
_Static_assert(அளவு(size_t) == 8, "Data type size mismatch");
_Static_assert(அளவு(வெறுமை *) == 8, "Data type size mismatch");
#இல்லையெனில்
#define ARCH_64BIT 0
_Static_assert(அளவு(size_t) == 4, "Data type size mismatch");
_Static_assert(அளவு(வெறுமை *) == 4, "Data type size mismatch");
#endif
#endif

#எனில் (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility தள்ளு(இயல்பு)
#endif

#define USE_IMPLEMENT 1
#define USE_INTERPOSE 0
#define USE_ALIAS 0

#எனில் defined(__APPLE__)
#undef USE_INTERPOSE
#define USE_INTERPOSE 1

வகைவரை கட்டமைப்பு interpose_t {
  வெறுமை *new_func;
  வெறுமை *orig_func;
} interpose_t;

#define MAC_INTERPOSE_PAIR(newf, oldf) {(வெறுமை *)newf, (வெறுமை *)oldf}
#define MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((used)) நிலையான மாறிலி interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#endif

#எனில் !defined(_WIN32) && !defined(__APPLE__)
#undef USE_IMPLEMENT
#undef USE_ALIAS
#define USE_IMPLEMENT 0
#define USE_ALIAS 1
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#undef malloc
#undef விடுவி
#undef calloc
#define RPMALLOC_RESTRICT __declspec(restrict)
#இல்லையெனில்
#define RPMALLOC_RESTRICT
#endif

#எனில் ENABLE_OVERRIDE

வகைவரை கட்டமைப்பு rp_nothrow_t {
  முழு __dummy;
} rp_nothrow_t;

#எனில் USE_IMPLEMENT

வெளி உள்வரி RPMALLOC_RESTRICT வெறுமை *RPMALLOC_CDECL malloc(size_t size) {
  திரும்பு rpmalloc(size);
}
வெளி உள்வரி RPMALLOC_RESTRICT வெறுமை *RPMALLOC_CDECL calloc(size_t மொத்தம்,
                                                            size_t size) {
  திரும்பு rpcalloc(மொத்தம், size);
}
வெளி உள்வரி RPMALLOC_RESTRICT வெறுமை *RPMALLOC_CDECL realloc(வெறுமை *ptr,
                                                             size_t size) {
  திரும்பு rprealloc(ptr, size);
}
வெளி உள்வரி வெறுமை *RPMALLOC_CDECL reallocf(வெறுமை *ptr, size_t size) {
  திரும்பு rprealloc(ptr, size);
}
வெளி உள்வரி வெறுமை *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t size) {
  திரும்பு rpaligned_alloc(alignment, size);
}
வெளி உள்வரி வெறுமை *RPMALLOC_CDECL memalign(size_t alignment, size_t size) {
  திரும்பு rpmemalign(alignment, size);
}
வெளி உள்வரி முழு RPMALLOC_CDECL posix_memalign(வெறுமை **memptr, size_t alignment,
                                                size_t size) {
  திரும்பு rpposix_memalign(memptr, alignment, size);
}
வெளி உள்வரி வெறுமை RPMALLOC_CDECL விடுவி(வெறுமை *ptr) { rpfree(ptr); }
வெளி உள்வரி வெறுமை RPMALLOC_CDECL cfree(வெறுமை *ptr) { rpfree(ptr); }
வெளி உள்வரி size_t RPMALLOC_CDECL malloc_usable_size(வெறுமை *ptr) {
  திரும்பு rpmalloc_usable_size(ptr);
}
வெளி உள்வரி size_t RPMALLOC_CDECL malloc_size(வெறுமை *ptr) {
  திரும்பு rpmalloc_usable_size(ptr);
}

#ifdef _WIN32
வெளி உள்வரி RPMALLOC_RESTRICT வெறுமை *RPMALLOC_CDECL _malloc_base(size_t size) {
  திரும்பு rpmalloc(size);
}
வெளி உள்வரி வெறுமை RPMALLOC_CDECL _free_base(வெறுமை *ptr) { rpfree(ptr); }
வெளி உள்வரி RPMALLOC_RESTRICT வெறுமை *RPMALLOC_CDECL _calloc_base(size_t மொத்தம்,
                                                                  size_t size) {
  திரும்பு rpcalloc(மொத்தம், size);
}
வெளி உள்வரி size_t RPMALLOC_CDECL _msize(வெறுமை *ptr) {
  திரும்பு rpmalloc_usable_size(ptr);
}
வெளி உள்வரி size_t RPMALLOC_CDECL _msize_base(வெறுமை *ptr) {
  திரும்பு rpmalloc_usable_size(ptr);
}
வெளி உள்வரி RPMALLOC_RESTRICT வெறுமை *RPMALLOC_CDECL
_realloc_base(வெறுமை *ptr, size_t size) {
  திரும்பு rprealloc(ptr, size);
}
#endif

#ifdef _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#இல்லையெனில்
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#define RPDEFVIS __attribute__((visibility("default")))
வெளி வெறுமை _ZdlPv(வெறுமை *p);
வெறுமை RPDEFVIS _ZdlPv(வெறுமை *p) { rpfree(p); }
வெளி வெறுமை _ZdaPv(வெறுமை *p);
வெறுமை RPDEFVIS _ZdaPv(வெறுமை *p) { rpfree(p); }
#எனில் ARCH_64BIT
// 64-bit operators new and new[], normal and aligned
வெளி வெறுமை *_Znwm(uint64_t size);
வெறுமை *RPDEFVIS _Znwm(uint64_t size) { திரும்பு rpmalloc(size); }
வெளி வெறுமை *_Znam(uint64_t size);
வெறுமை *RPDEFVIS _Znam(uint64_t size) { திரும்பு rpmalloc(size); }
வெளி வெறுமை *_Znwmm(uint64_t size, uint64_t align);
வெறுமை *RPDEFVIS _Znwmm(uint64_t size, uint64_t align) {
  திரும்பு rpaligned_alloc(align, size);
}
வெளி வெறுமை *_Znamm(uint64_t size, uint64_t align);
வெறுமை *RPDEFVIS _Znamm(uint64_t size, uint64_t align) {
  திரும்பு rpaligned_alloc(align, size);
}
வெளி வெறுமை *_ZnwmSt11align_val_t(uint64_t size, uint64_t align);
வெறுமை *RPDEFVIS _ZnwmSt11align_val_t(uint64_t size, uint64_t align) {
  திரும்பு rpaligned_alloc(align, size);
}
வெளி வெறுமை *_ZnamSt11align_val_t(uint64_t size, uint64_t align);
வெறுமை *RPDEFVIS _ZnamSt11align_val_t(uint64_t size, uint64_t align) {
  திரும்பு rpaligned_alloc(align, size);
}
வெளி வெறுமை *_ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
வெறுமை *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (வெறுமை)அளவு(t);
  திரும்பு rpmalloc(size);
}
வெளி வெறுமை *_ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t);
வெறுமை *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t size, rp_nothrow_t t) {
  (வெறுமை)அளவு(t);
  திரும்பு rpmalloc(size);
}
வெளி வெறுமை *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
வெறுமை *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (வெறுமை)அளவு(t);
  திரும்பு rpaligned_alloc(align, size);
}
வெளி வெறுமை *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                rp_nothrow_t t);
வெறுமை *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t size, uint64_t align,
                                                  rp_nothrow_t t) {
  (வெறுமை)அளவு(t);
  திரும்பு rpaligned_alloc(align, size);
}
// 64-bit operators sized delete and delete[], normal and aligned
வெளி வெறுமை _ZdlPvm(வெறுமை *p, uint64_t size);
வெறுமை RPDEFVIS _ZdlPvm(வெறுமை *p, uint64_t size) {
  rpfree(p);
  (வெறுமை)அளவு(size);
}
வெளி வெறுமை _ZdaPvm(வெறுமை *p, uint64_t size);
வெறுமை RPDEFVIS _ZdaPvm(வெறுமை *p, uint64_t size) {
  rpfree(p);
  (வெறுமை)அளவு(size);
}
வெளி வெறுமை _ZdlPvSt11align_val_t(வெறுமை *p, uint64_t align);
வெறுமை RPDEFVIS _ZdlPvSt11align_val_t(வெறுமை *p, uint64_t align) {
  rpfree(p);
  (வெறுமை)அளவு(align);
}
வெளி வெறுமை _ZdaPvSt11align_val_t(வெறுமை *p, uint64_t align);
வெறுமை RPDEFVIS _ZdaPvSt11align_val_t(வெறுமை *p, uint64_t align) {
  rpfree(p);
  (வெறுமை)அளவு(align);
}
வெளி வெறுமை _ZdlPvmSt11align_val_t(வெறுமை *p, uint64_t size, uint64_t align);
வெறுமை RPDEFVIS _ZdlPvmSt11align_val_t(வெறுமை *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (வெறுமை)அளவு(size);
  (வெறுமை)அளவு(align);
}
வெளி வெறுமை _ZdaPvmSt11align_val_t(வெறுமை *p, uint64_t size, uint64_t align);
வெறுமை RPDEFVIS _ZdaPvmSt11align_val_t(வெறுமை *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (வெறுமை)அளவு(size);
  (வெறுமை)அளவு(align);
}
#இல்லையெனில்
// 32-bit operators new and new[], normal and aligned
வெளி வெறுமை *_Znwj(uint32_t size);
வெறுமை *RPDEFVIS _Znwj(uint32_t size) { திரும்பு rpmalloc(size); }
வெளி வெறுமை *_Znaj(uint32_t size);
வெறுமை *RPDEFVIS _Znaj(uint32_t size) { திரும்பு rpmalloc(size); }
வெளி வெறுமை *_Znwjj(uint32_t size, uint32_t align);
வெறுமை *RPDEFVIS _Znwjj(uint32_t size, uint32_t align) {
  திரும்பு rpaligned_alloc(align, size);
}
வெளி வெறுமை *_Znajj(uint32_t size, uint32_t align);
வெறுமை *RPDEFVIS _Znajj(uint32_t size, uint32_t align) {
  திரும்பு rpaligned_alloc(align, size);
}
வெளி வெறுமை *_ZnwjSt11align_val_t(size_t size, size_t align);
வெறுமை *RPDEFVIS _ZnwjSt11align_val_t(size_t size, size_t align) {
  திரும்பு rpaligned_alloc(align, size);
}
வெளி வெறுமை *_ZnajSt11align_val_t(size_t size, size_t align);
வெறுமை *RPDEFVIS _ZnajSt11align_val_t(size_t size, size_t align) {
  திரும்பு rpaligned_alloc(align, size);
}
வெளி வெறுமை *_ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t);
வெறுமை *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (வெறுமை)அளவு(t);
  திரும்பு rpmalloc(size);
}
வெளி வெறுமை *_ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t);
வெறுமை *RPDEFVIS _ZnajRKSt9nothrow_t(size_t size, rp_nothrow_t t) {
  (வெறுமை)அளவு(t);
  திரும்பு rpmalloc(size);
}
வெளி வெறுமை *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
வெறுமை *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (வெறுமை)அளவு(t);
  திரும்பு rpaligned_alloc(align, size);
}
வெளி வெறுமை *_ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                rp_nothrow_t t);
வெறுமை *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t size, size_t align,
                                                  rp_nothrow_t t) {
  (வெறுமை)அளவு(t);
  திரும்பு rpaligned_alloc(align, size);
}
// 32-bit operators sized delete and delete[], normal and aligned
வெளி வெறுமை _ZdlPvj(வெறுமை *p, uint64_t size);
வெறுமை RPDEFVIS _ZdlPvj(வெறுமை *p, uint64_t size) {
  rpfree(p);
  (வெறுமை)அளவு(size);
}
வெளி வெறுமை _ZdaPvj(வெறுமை *p, uint64_t size);
வெறுமை RPDEFVIS _ZdaPvj(வெறுமை *p, uint64_t size) {
  rpfree(p);
  (வெறுமை)அளவு(size);
}
வெளி வெறுமை _ZdlPvSt11align_val_t(வெறுமை *p, uint32_t align);
வெறுமை RPDEFVIS _ZdlPvSt11align_val_t(வெறுமை *p, uint64_t a) {
  rpfree(p);
  (வெறுமை)அளவு(align);
}
வெளி வெறுமை _ZdaPvSt11align_val_t(வெறுமை *p, uint32_t align);
வெறுமை RPDEFVIS _ZdaPvSt11align_val_t(வெறுமை *p, uint64_t a) {
  rpfree(p);
  (வெறுமை)அளவு(align);
}
வெளி வெறுமை _ZdlPvjSt11align_val_t(வெறுமை *p, uint32_t size, uint32_t align);
வெறுமை RPDEFVIS _ZdlPvjSt11align_val_t(வெறுமை *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (வெறுமை)அளவு(size);
  (வெறுமை)அளவு(a);
}
வெளி வெறுமை _ZdaPvjSt11align_val_t(வெறுமை *p, uint32_t size, uint32_t align);
வெறுமை RPDEFVIS _ZdaPvjSt11align_val_t(வெறுமை *p, uint64_t size, uint64_t align) {
  rpfree(p);
  (வெறுமை)அளவு(size);
  (வெறுமை)அளவு(a);
}
#endif
#endif
#endif

#எனில் USE_INTERPOSE || USE_ALIAS

நிலையான வெறுமை *rpmalloc_nothrow(size_t size, rp_nothrow_t t) {
  (வெறுமை)அளவு(t);
  திரும்பு rpmalloc(size);
}
நிலையான வெறுமை *rpaligned_alloc_reverse(size_t size, size_t align) {
  திரும்பு rpaligned_alloc(align, size);
}
நிலையான வெறுமை *rpaligned_alloc_reverse_nothrow(size_t size, size_t align,
                                             rp_nothrow_t t) {
  (வெறுமை)அளவு(t);
  திரும்பு rpaligned_alloc(align, size);
}
நிலையான வெறுமை rpfree_size(வெறுமை *p, size_t size) {
  (வெறுமை)அளவு(size);
  rpfree(p);
}
நிலையான வெறுமை rpfree_aligned(வெறுமை *p, size_t align) {
  (வெறுமை)அளவு(align);
  rpfree(p);
}
நிலையான வெறுமை rpfree_size_aligned(வெறுமை *p, size_t size, size_t align) {
  (வெறுமை)அளவு(size);
  (வெறுமை)அளவு(align);
  rpfree(p);
}

#endif

#எனில் USE_INTERPOSE

__attribute__((used)) நிலையான மாறிலி interpose_t macinterpose_malloc[]
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
#எனில் defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#endif
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, விடுவி), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_usable_size),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_size, malloc_size)};

#endif

#எனில் USE_ALIAS

#define RPALIAS(fn) __attribute__((alias(#fn), used, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
வெறுமை _ZdlPv(வெறுமை *p) RPALIAS(rpfree) வெறுமை _ZdaPv(வெறுமை *p) RPALIAS(rpfree)

#எனில் ARCH_64BIT
    // 64-bit operators new and new[], normal and aligned
    வெறுமை *_Znwm(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) வெறுமை *_Znam(uint64_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) வெறுமை *_Znwmm(uint64_t size,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_reverse) வெறுமை *_Znamm(uint64_t size,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_reverse) வெறுமை *_ZnwmSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) வெறுமை *_ZnamSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) வெறுமை *_ZnwmRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) வெறுமை *_ZnamRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) வெறுமை
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) வெறுமை
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    வெறுமை _ZdlPvm(வெறுமை *p, size_t n) RPALIAS(rpfree_size) வெறுமை _ZdaPvm(வெறுமை *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) வெறுமை _ZdlPvSt11align_val_t(வெறுமை *p, size_t a)
            RPALIAS(rpfree_aligned) வெறுமை _ZdaPvSt11align_val_t(வெறுமை *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) வெறுமை _ZdlPvmSt11align_val_t(வெறுமை *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) வெறுமை _ZdaPvmSt11align_val_t(
                        வெறுமை *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#இல்லையெனில்
    // 32-bit operators new and new[], normal and aligned
    வெறுமை *_Znwj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) வெறுமை *_Znaj(uint32_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1) RPALIAS(rpmalloc) வெறுமை *_Znwjj(uint32_t size,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_reverse) வெறுமை *_Znajj(uint32_t size,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_reverse) வெறுமை *_ZnwjSt11align_val_t(
                size_t size, size_t align)
                RPALIAS(rpaligned_alloc_reverse) வெறுமை *_ZnajSt11align_val_t(
                    size_t size, size_t align)
                    RPALIAS(rpaligned_alloc_reverse) வெறுமை *_ZnwjRKSt9nothrow_t(
                        size_t size, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) வெறுமை *_ZnajRKSt9nothrow_t(
                            size_t size,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) வெறுமை
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t size,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_reverse_nothrow) வெறுமை
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t size, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_reverse_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    வெறுமை _ZdlPvj(வெறுமை *p, size_t n) RPALIAS(rpfree_size) வெறுமை _ZdaPvj(வெறுமை *p,
                                                                      size_t n)
        RPALIAS(rpfree_size) வெறுமை _ZdlPvSt11align_val_t(வெறுமை *p, size_t a)
            RPALIAS(rpfree_aligned) வெறுமை _ZdaPvSt11align_val_t(வெறுமை *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) வெறுமை _ZdlPvjSt11align_val_t(வெறுமை *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_size_aligned) வெறுமை _ZdaPvjSt11align_val_t(
                        வெறுமை *p, size_t n, size_t a)
                        RPALIAS(rpfree_size_aligned)
#endif

                            வெறுமை *malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) வெறுமை *calloc(size_t மொத்தம், size_t size)
            RPALIAS(rpcalloc) வெறுமை *realloc(வெறுமை *ptr, size_t size)
                RPALIAS(rprealloc) வெறுமை *reallocf(வெறுமை *ptr, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rprealloc) வெறுமை *aligned_alloc(size_t alignment, size_t size)
            RPALIAS(rpaligned_alloc) வெறுமை *memalign(
                size_t alignment, size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) முழு posix_memalign(வெறுமை **memptr, size_t alignment,
                                               size_t size)
            RPALIAS(rpposix_memalign) வெறுமை விடுவி(வெறுமை *ptr)
                RPALIAS(rpfree) வெறுமை cfree(வெறுமை *ptr) RPALIAS(rpfree)
#எனில் defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_size(மாறிலி வெறுமை *ptr) RPALIAS(rpmalloc_usable_size)
#இல்லையெனில்
                    size_t
    malloc_usable_size(வெறுமை *ptr) RPALIAS(rpmalloc_usable_size)
#endif
        size_t malloc_size(வெறுமை *ptr) RPALIAS(rpmalloc_usable_size)

#endif

            நிலையான உள்வரி size_t _rpmalloc_page_size(வெறுமை) {
  திரும்பு _memory_page_size;
}

வெளி வெறுமை *RPMALLOC_CDECL reallocarray(வெறுமை *ptr, size_t மொத்தம், size_t size);

வெளி வெறுமை *RPMALLOC_CDECL reallocarray(வெறுமை *ptr, size_t மொத்தம், size_t size) {
  size_t total;
#எனில் ENABLE_VALIDATE_ARGS
#ifdef _MSC_VER
  முழு err = SizeTMult(மொத்தம், size, &total);
  எனில் ((err != S_OK) || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    திரும்பு 0;
  }
#இல்லையெனில்
  முழு err = __builtin_umull_overflow(மொத்தம், size, &total);
  எனில் (err || (total >= MAX_ALLOC_SIZE)) {
    errno = EINVAL;
    திரும்பு 0;
  }
#endif
#இல்லையெனில்
  total = மொத்தம் * size;
#endif
  திரும்பு realloc(ptr, total);
}

வெளி உள்வரி வெறுமை *RPMALLOC_CDECL valloc(size_t size) {
  get_thread_heap();
  திரும்பு rpaligned_alloc(_rpmalloc_page_size(), size);
}

வெளி உள்வரி வெறுமை *RPMALLOC_CDECL pvalloc(size_t size) {
  get_thread_heap();
  மாறிலி size_t page_size = _rpmalloc_page_size();
  மாறிலி size_t aligned_size = ((size + page_size - 1) / page_size) * page_size;
#எனில் ENABLE_VALIDATE_ARGS
  எனில் (aligned_size < size) {
    errno = EINVAL;
    திரும்பு 0;
  }
#endif
  திரும்பு rpaligned_alloc(_rpmalloc_page_size(), aligned_size);
}

#endif // ENABLE_OVERRIDE

#எனில் ENABLE_PRELOAD

#ifdef _WIN32

#எனில் defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

வெளி __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

வெளி __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (வெறுமை)அளவு(reserved);
  (வெறுமை)அளவு(instance);
  எனில் (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  இல்லையெனில் எனில் (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  இல்லையெனில் எனில் (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  இல்லையெனில் எனில் (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  திரும்பு TRUE;
}

// end BUILD_DYNAMIC_LINK
#இல்லையெனில்

வெளி வெறுமை _global_rpmalloc_init(வெறுமை) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
}

#எனில் defined(__clang__) || defined(__GNUC__)

நிலையான வெறுமை __attribute__((constructor)) initializer(வெறுமை) {
  _global_rpmalloc_init();
}

#அல்லதெனில் defined(_MSC_VER)

நிலையான முழு _global_rpmalloc_xib(வெறுமை) {
  _global_rpmalloc_init();
  திரும்பு 0;
}

#pragma section(".CRT$XIB", பட)
__declspec(allocate(".CRT$XIB")) வெறுமை (*_rpmalloc_module_init)(வெறுமை) =
    _global_rpmalloc_xib;
#எனில் defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#இல்லையெனில்
#pragma comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#endif

#endif

// end !BUILD_DYNAMIC_LINK
#endif

#இல்லையெனில்

#அந்தர்பாவ <pthread.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <unistd.h>

வெளி வெறுமை rpmalloc_set_main_thread(வெறுமை);

நிலையான pthread_key_t destructor_சாவி;

நிலையான வெறுமை thread_destructor(வெறுமை *);

நிலையான வெறுமை __attribute__((constructor)) initializer(வெறுமை) {
  rpmalloc_set_main_thread();
  rpmalloc_initialize();
  pthread_சாவி_create(&destructor_சாவி, thread_destructor);
}

நிலையான வெறுமை __attribute__((destructor)) finalizer(வெறுமை) { rpmalloc_finalize(); }

வகைவரை கட்டமைப்பு {
  வெறுமை *(*real_start)(வெறுமை *);
  வெறுமை *real_arg;
} thread_starter_arg;

நிலையான வெறுமை *thread_starter(வெறுமை *argptr) {
  thread_starter_arg *arg = argptr;
  வெறுமை *(*real_start)(வெறுமை *) = arg->real_start;
  வெறுமை *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_சாவி, (வெறுமை *)1);
  திரும்பு (*real_start)(real_arg);
}

நிலையான வெறுமை thread_destructor(வெறுமை *மதிப்பு_2) {
  (வெறுமை)அளவு(மதிப்பு_2);
  rpmalloc_thread_finalize(1);
}

#ifdef __APPLE__

நிலையான முழு pthread_create_proxy(pthread_t *thread, மாறிலி pthread_attr_t *attr,
                                வெறுமை *(*start_routine)(வெறுமை *), வெறுமை *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(அளவு(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  திரும்பு pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#இல்லையெனில்

#அந்தர்பாவ <dlfcn.h>

முழு pthread_create(pthread_t *thread, மாறிலி pthread_attr_t *attr,
                   வெறுமை *(*start_routine)(வெறுமை *), வெறுமை *arg) {
#எனில் defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  எழுத்து fname[] = "pthread_create";
#இல்லையெனில்
  எழுத்து fname[] = "_pthread_create";
#endif
  வெறுமை *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(அளவு(thread_starter_arg));
  starter_arg->real_start = start_routine;
  starter_arg->real_arg = arg;
  திரும்பு (*(முழு (*)(pthread_t *, மாறிலி pthread_attr_t *, வெறுமை *(*)(வெறுமை *),
                    வெறுமை *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#endif

#endif

#endif

#எனில் ENABLE_OVERRIDE

#எனில் defined(__GLIBC__) && defined(__linux__)

வெறுமை *__libc_malloc(size_t size) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(1)
        RPALIAS(rpmalloc) வெறுமை *__libc_calloc(size_t மொத்தம், size_t size)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) வெறுமை *__libc_realloc(வெறுமை *p, size_t size)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2) RPALIAS(rprealloc) வெறுமை __libc_free(வெறுமை *p)
        RPALIAS(rpfree) வெறுமை __libc_cfree(வெறுமை *p)
            RPALIAS(rpfree) வெறுமை *__libc_memalign(size_t align, size_t size)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE(2)
        RPALIAS(rpmemalign) முழு __posix_memalign(வெறுமை **p, size_t align,
                                                 size_t size)
            RPALIAS(rpposix_memalign)

                வெளி வெறுமை *__libc_valloc(size_t size);
வெளி வெறுமை *__libc_pvalloc(size_t size);

வெறுமை *__libc_valloc(size_t size) { திரும்பு valloc(size); }

வெறுமை *__libc_pvalloc(size_t size) { திரும்பு pvalloc(size); }

#endif

#endif

#எனில் (defined(__GNUC__) || defined(__clang__))
#pragma GCC visibility எடு
#endif
