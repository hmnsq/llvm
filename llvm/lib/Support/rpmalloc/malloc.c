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

#ఒకవేళ defined(__TINYC__)
#అన్తర్భావ <sys/types.h>
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే ARCH_64_బిట్టు
#ఒకవేళ defined(__LLP64__) || defined(__LP64__) || defined(_WIN64)
#నిర్వచించు ARCH_64_బిట్టు 1
_Static_assert(పరిమాణం(size_t) == 8, "Data type size mismatch");
_Static_assert(పరిమాణం(శూన్య *) == 8, "Data type size mismatch");
#లేకపోతే
#నిర్వచించు ARCH_64_బిట్టు 0
_Static_assert(పరిమాణం(size_t) == 4, "Data type size mismatch");
_Static_assert(పరిమాణం(శూన్య *) == 4, "Data type size mismatch");
#అయితే_ముగింపు
#అయితే_ముగింపు

#ఒకవేళ (defined(__GNUC__) || defined(__clang__))
#సూచన GCC visibility నెట్టు(అప్రమేయ)
#అయితే_ముగింపు

#నిర్వచించు USE_IMPLEMENT 1
#నిర్వచించు USE_INTERPOSE 0
#నిర్వచించు USE_ALIAS 0

#ఒకవేళ defined(__APPLE__)
#నిర్వచనం_రద్దు USE_INTERPOSE
#నిర్వచించు USE_INTERPOSE 1

రకనిర్వచనం నిర్మాణం interpose_t {
  శూన్య *new_func;
  శూన్య *orig_func;
} interpose_t;

#నిర్వచించు MAC_INTERPOSE_PAIR(newf, oldf) {(శూన్య *)newf, (శూన్య *)oldf}
#నిర్వచించు MAC_INTERPOSE_SINGLE(newf, oldf)                                       \
  __attribute__((వాడిన)) స్థిర స్థిరరాశి interpose_t macinterpose##newf##oldf      \
      __attribute__((section("__DATA, __interpose"))) =                        \
          MAC_INTERPOSE_PAIR(newf, oldf)

#అయితే_ముగింపు

#ఒకవేళ !defined(_WIN32) && !defined(__APPLE__)
#నిర్వచనం_రద్దు USE_IMPLEMENT
#నిర్వచనం_రద్దు USE_ALIAS
#నిర్వచించు USE_IMPLEMENT 0
#నిర్వచించు USE_ALIAS 1
#అయితే_ముగింపు

#నిర్వచితమైతే _MSC_VER
#సూచన warning(disable : 4100)
#నిర్వచనం_రద్దు జ్ఞాపకం_ఇవ్వు
#నిర్వచనం_రద్దు విడుదల
#నిర్వచనం_రద్దు సున్నా_జ్ఞాపకం_ఇవ్వు
#నిర్వచించు RPMALLOC_RESTRICT __declspec(పరిమిత)
#లేకపోతే
#నిర్వచించు RPMALLOC_RESTRICT
#అయితే_ముగింపు

#ఒకవేళ ENABLE_OVERRIDE

రకనిర్వచనం నిర్మాణం rp_nothrow_t {
  పూర్ణాంకం __dummy;
} rp_nothrow_t;

#ఒకవేళ USE_IMPLEMENT

బాహ్య అంతఃవరుస RPMALLOC_RESTRICT శూన్య *RPMALLOC_CDECL జ్ఞాపకం_ఇవ్వు(size_t కొలత_2) {
  తిరిగి rpmalloc(కొలత_2);
}
బాహ్య అంతఃవరుస RPMALLOC_RESTRICT శూన్య *RPMALLOC_CDECL సున్నా_జ్ఞాపకం_ఇవ్వు(size_t లెక్క,
                                                            size_t కొలత_2) {
  తిరిగి rpcalloc(లెక్క, కొలత_2);
}
బాహ్య అంతఃవరుస RPMALLOC_RESTRICT శూన్య *RPMALLOC_CDECL realloc(శూన్య *ptr,
                                                             size_t కొలత_2) {
  తిరిగి rprealloc(ptr, కొలత_2);
}
బాహ్య అంతఃవరుస శూన్య *RPMALLOC_CDECL reallocf(శూన్య *ptr, size_t కొలత_2) {
  తిరిగి rprealloc(ptr, కొలత_2);
}
బాహ్య అంతఃవరుస శూన్య *RPMALLOC_CDECL aligned_alloc(size_t alignment,
                                                 size_t కొలత_2) {
  తిరిగి rpaligned_alloc(alignment, కొలత_2);
}
బాహ్య అంతఃవరుస శూన్య *RPMALLOC_CDECL memalign(size_t alignment, size_t కొలత_2) {
  తిరిగి rpmemalign(alignment, కొలత_2);
}
బాహ్య అంతఃవరుస పూర్ణాంకం RPMALLOC_CDECL posix_memalign(శూన్య **memptr, size_t alignment,
                                                size_t కొలత_2) {
  తిరిగి rpposix_memalign(memptr, alignment, కొలత_2);
}
బాహ్య అంతఃవరుస శూన్య RPMALLOC_CDECL విడుదల(శూన్య *ptr) { rpfree(ptr); }
బాహ్య అంతఃవరుస శూన్య RPMALLOC_CDECL cfree(శూన్య *ptr) { rpfree(ptr); }
బాహ్య అంతఃవరుస size_t RPMALLOC_CDECL malloc_usable_కొలత(శూన్య *ptr) {
  తిరిగి rpmalloc_usable_కొలత(ptr);
}
బాహ్య అంతఃవరుస size_t RPMALLOC_CDECL malloc_కొలత(శూన్య *ptr) {
  తిరిగి rpmalloc_usable_కొలత(ptr);
}

#నిర్వచితమైతే _WIN32
బాహ్య అంతఃవరుస RPMALLOC_RESTRICT శూన్య *RPMALLOC_CDECL malloc_పునాది(size_t కొలత_2) {
  తిరిగి rpmalloc(కొలత_2);
}
బాహ్య అంతఃవరుస శూన్య RPMALLOC_CDECL free_పునాది(శూన్య *ptr) { rpfree(ptr); }
బాహ్య అంతఃవరుస RPMALLOC_RESTRICT శూన్య *RPMALLOC_CDECL calloc_పునాది(size_t లెక్క,
                                                                  size_t కొలత_2) {
  తిరిగి rpcalloc(లెక్క, కొలత_2);
}
బాహ్య అంతఃవరుస size_t RPMALLOC_CDECL _msize(శూన్య *ptr) {
  తిరిగి rpmalloc_usable_కొలత(ptr);
}
బాహ్య అంతఃవరుస size_t RPMALLOC_CDECL msize_పునాది(శూన్య *ptr) {
  తిరిగి rpmalloc_usable_కొలత(ptr);
}
బాహ్య అంతఃవరుస RPMALLOC_RESTRICT శూన్య *RPMALLOC_CDECL
realloc_పునాది(శూన్య *ptr, size_t కొలత_2) {
  తిరిగి rprealloc(ptr, కొలత_2);
}
#అయితే_ముగింపు

#నిర్వచితమైతే _WIN32
// For Windows, #include <rpnew.h> in one source file to get the C++ operator
// overrides implemented in your module
#లేకపోతే
// Overload the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) operators
// delete and delete[]
#నిర్వచించు RPDEFVIS __attribute__((visibility("default")))
బాహ్య శూన్య _ZdlPv(శూన్య *p);
శూన్య RPDEFVIS _ZdlPv(శూన్య *p) { rpfree(p); }
బాహ్య శూన్య _ZdaPv(శూన్య *p);
శూన్య RPDEFVIS _ZdaPv(శూన్య *p) { rpfree(p); }
#ఒకవేళ ARCH_64_బిట్టు
// 64-bit operators new and new[], normal and aligned
బాహ్య శూన్య *_Znwm(uint64_t కొలత_2);
శూన్య *RPDEFVIS _Znwm(uint64_t కొలత_2) { తిరిగి rpmalloc(కొలత_2); }
బాహ్య శూన్య *_Znam(uint64_t కొలత_2);
శూన్య *RPDEFVIS _Znam(uint64_t కొలత_2) { తిరిగి rpmalloc(కొలత_2); }
బాహ్య శూన్య *_Znwmm(uint64_t కొలత_2, uint64_t align);
శూన్య *RPDEFVIS _Znwmm(uint64_t కొలత_2, uint64_t align) {
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
బాహ్య శూన్య *_Znamm(uint64_t కొలత_2, uint64_t align);
శూన్య *RPDEFVIS _Znamm(uint64_t కొలత_2, uint64_t align) {
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
బాహ్య శూన్య *_ZnwmSt11align_val_t(uint64_t కొలత_2, uint64_t align);
శూన్య *RPDEFVIS _ZnwmSt11align_val_t(uint64_t కొలత_2, uint64_t align) {
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
బాహ్య శూన్య *_ZnamSt11align_val_t(uint64_t కొలత_2, uint64_t align);
శూన్య *RPDEFVIS _ZnamSt11align_val_t(uint64_t కొలత_2, uint64_t align) {
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
బాహ్య శూన్య *_ZnwmRKSt9nothrow_t(uint64_t కొలత_2, rp_nothrow_t t);
శూన్య *RPDEFVIS _ZnwmRKSt9nothrow_t(uint64_t కొలత_2, rp_nothrow_t t) {
  (శూన్య)పరిమాణం(t);
  తిరిగి rpmalloc(కొలత_2);
}
బాహ్య శూన్య *_ZnamRKSt9nothrow_t(uint64_t కొలత_2, rp_nothrow_t t);
శూన్య *RPDEFVIS _ZnamRKSt9nothrow_t(uint64_t కొలత_2, rp_nothrow_t t) {
  (శూన్య)పరిమాణం(t);
  తిరిగి rpmalloc(కొలత_2);
}
బాహ్య శూన్య *_ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t కొలత_2, uint64_t align,
                                                rp_nothrow_t t);
శూన్య *RPDEFVIS _ZnwmSt11align_val_tRKSt9nothrow_t(uint64_t కొలత_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (శూన్య)పరిమాణం(t);
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
బాహ్య శూన్య *_ZnamSt11align_val_tRKSt9nothrow_t(uint64_t కొలత_2, uint64_t align,
                                                rp_nothrow_t t);
శూన్య *RPDEFVIS _ZnamSt11align_val_tRKSt9nothrow_t(uint64_t కొలత_2, uint64_t align,
                                                  rp_nothrow_t t) {
  (శూన్య)పరిమాణం(t);
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
// 64-bit operators sized delete and delete[], normal and aligned
బాహ్య శూన్య _ZdlPvm(శూన్య *p, uint64_t కొలత_2);
శూన్య RPDEFVIS _ZdlPvm(శూన్య *p, uint64_t కొలత_2) {
  rpfree(p);
  (శూన్య)పరిమాణం(కొలత_2);
}
బాహ్య శూన్య _ZdaPvm(శూన్య *p, uint64_t కొలత_2);
శూన్య RPDEFVIS _ZdaPvm(శూన్య *p, uint64_t కొలత_2) {
  rpfree(p);
  (శూన్య)పరిమాణం(కొలత_2);
}
బాహ్య శూన్య _ZdlPvSt11align_val_t(శూన్య *p, uint64_t align);
శూన్య RPDEFVIS _ZdlPvSt11align_val_t(శూన్య *p, uint64_t align) {
  rpfree(p);
  (శూన్య)పరిమాణం(align);
}
బాహ్య శూన్య _ZdaPvSt11align_val_t(శూన్య *p, uint64_t align);
శూన్య RPDEFVIS _ZdaPvSt11align_val_t(శూన్య *p, uint64_t align) {
  rpfree(p);
  (శూన్య)పరిమాణం(align);
}
బాహ్య శూన్య _ZdlPvmSt11align_val_t(శూన్య *p, uint64_t కొలత_2, uint64_t align);
శూన్య RPDEFVIS _ZdlPvmSt11align_val_t(శూన్య *p, uint64_t కొలత_2, uint64_t align) {
  rpfree(p);
  (శూన్య)పరిమాణం(కొలత_2);
  (శూన్య)పరిమాణం(align);
}
బాహ్య శూన్య _ZdaPvmSt11align_val_t(శూన్య *p, uint64_t కొలత_2, uint64_t align);
శూన్య RPDEFVIS _ZdaPvmSt11align_val_t(శూన్య *p, uint64_t కొలత_2, uint64_t align) {
  rpfree(p);
  (శూన్య)పరిమాణం(కొలత_2);
  (శూన్య)పరిమాణం(align);
}
#లేకపోతే
// 32-bit operators new and new[], normal and aligned
బాహ్య శూన్య *_Znwj(uint32_t కొలత_2);
శూన్య *RPDEFVIS _Znwj(uint32_t కొలత_2) { తిరిగి rpmalloc(కొలత_2); }
బాహ్య శూన్య *_Znaj(uint32_t కొలత_2);
శూన్య *RPDEFVIS _Znaj(uint32_t కొలత_2) { తిరిగి rpmalloc(కొలత_2); }
బాహ్య శూన్య *_Znwjj(uint32_t కొలత_2, uint32_t align);
శూన్య *RPDEFVIS _Znwjj(uint32_t కొలత_2, uint32_t align) {
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
బాహ్య శూన్య *_Znajj(uint32_t కొలత_2, uint32_t align);
శూన్య *RPDEFVIS _Znajj(uint32_t కొలత_2, uint32_t align) {
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
బాహ్య శూన్య *_ZnwjSt11align_val_t(size_t కొలత_2, size_t align);
శూన్య *RPDEFVIS _ZnwjSt11align_val_t(size_t కొలత_2, size_t align) {
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
బాహ్య శూన్య *_ZnajSt11align_val_t(size_t కొలత_2, size_t align);
శూన్య *RPDEFVIS _ZnajSt11align_val_t(size_t కొలత_2, size_t align) {
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
బాహ్య శూన్య *_ZnwjRKSt9nothrow_t(size_t కొలత_2, rp_nothrow_t t);
శూన్య *RPDEFVIS _ZnwjRKSt9nothrow_t(size_t కొలత_2, rp_nothrow_t t) {
  (శూన్య)పరిమాణం(t);
  తిరిగి rpmalloc(కొలత_2);
}
బాహ్య శూన్య *_ZnajRKSt9nothrow_t(size_t కొలత_2, rp_nothrow_t t);
శూన్య *RPDEFVIS _ZnajRKSt9nothrow_t(size_t కొలత_2, rp_nothrow_t t) {
  (శూన్య)పరిమాణం(t);
  తిరిగి rpmalloc(కొలత_2);
}
బాహ్య శూన్య *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t కొలత_2, size_t align,
                                                rp_nothrow_t t);
శూన్య *RPDEFVIS _ZnwjSt11align_val_tRKSt9nothrow_t(size_t కొలత_2, size_t align,
                                                  rp_nothrow_t t) {
  (శూన్య)పరిమాణం(t);
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
బాహ్య శూన్య *_ZnajSt11align_val_tRKSt9nothrow_t(size_t కొలత_2, size_t align,
                                                rp_nothrow_t t);
శూన్య *RPDEFVIS _ZnajSt11align_val_tRKSt9nothrow_t(size_t కొలత_2, size_t align,
                                                  rp_nothrow_t t) {
  (శూన్య)పరిమాణం(t);
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
// 32-bit operators sized delete and delete[], normal and aligned
బాహ్య శూన్య _ZdlPvj(శూన్య *p, uint64_t కొలత_2);
శూన్య RPDEFVIS _ZdlPvj(శూన్య *p, uint64_t కొలత_2) {
  rpfree(p);
  (శూన్య)పరిమాణం(కొలత_2);
}
బాహ్య శూన్య _ZdaPvj(శూన్య *p, uint64_t కొలత_2);
శూన్య RPDEFVIS _ZdaPvj(శూన్య *p, uint64_t కొలత_2) {
  rpfree(p);
  (శూన్య)పరిమాణం(కొలత_2);
}
బాహ్య శూన్య _ZdlPvSt11align_val_t(శూన్య *p, uint32_t align);
శూన్య RPDEFVIS _ZdlPvSt11align_val_t(శూన్య *p, uint64_t a) {
  rpfree(p);
  (శూన్య)పరిమాణం(align);
}
బాహ్య శూన్య _ZdaPvSt11align_val_t(శూన్య *p, uint32_t align);
శూన్య RPDEFVIS _ZdaPvSt11align_val_t(శూన్య *p, uint64_t a) {
  rpfree(p);
  (శూన్య)పరిమాణం(align);
}
బాహ్య శూన్య _ZdlPvjSt11align_val_t(శూన్య *p, uint32_t కొలత_2, uint32_t align);
శూన్య RPDEFVIS _ZdlPvjSt11align_val_t(శూన్య *p, uint64_t కొలత_2, uint64_t align) {
  rpfree(p);
  (శూన్య)పరిమాణం(కొలత_2);
  (శూన్య)పరిమాణం(a);
}
బాహ్య శూన్య _ZdaPvjSt11align_val_t(శూన్య *p, uint32_t కొలత_2, uint32_t align);
శూన్య RPDEFVIS _ZdaPvjSt11align_val_t(శూన్య *p, uint64_t కొలత_2, uint64_t align) {
  rpfree(p);
  (శూన్య)పరిమాణం(కొలత_2);
  (శూన్య)పరిమాణం(a);
}
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు

#ఒకవేళ USE_INTERPOSE || USE_ALIAS

స్థిర శూన్య *rpmalloc_nothrow(size_t కొలత_2, rp_nothrow_t t) {
  (శూన్య)పరిమాణం(t);
  తిరిగి rpmalloc(కొలత_2);
}
స్థిర శూన్య *rpaligned_alloc_తిరగేయి(size_t కొలత_2, size_t align) {
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
స్థిర శూన్య *rpaligned_alloc_తిరగేయి_nothrow(size_t కొలత_2, size_t align,
                                             rp_nothrow_t t) {
  (శూన్య)పరిమాణం(t);
  తిరిగి rpaligned_alloc(align, కొలత_2);
}
స్థిర శూన్య rpfree_కొలత(శూన్య *p, size_t కొలత_2) {
  (శూన్య)పరిమాణం(కొలత_2);
  rpfree(p);
}
స్థిర శూన్య rpfree_aligned(శూన్య *p, size_t align) {
  (శూన్య)పరిమాణం(align);
  rpfree(p);
}
స్థిర శూన్య rpfree_కొలత_aligned(శూన్య *p, size_t కొలత_2, size_t align) {
  (శూన్య)పరిమాణం(కొలత_2);
  (శూన్య)పరిమాణం(align);
  rpfree(p);
}

#అయితే_ముగింపు

#ఒకవేళ USE_INTERPOSE

__attribute__((వాడిన)) స్థిర స్థిరరాశి interpose_t macinterpose_malloc[]
    __attribute__((section("__DATA, __interpose"))) = {
        // new and new[]
        MAC_INTERPOSE_PAIR(rpmalloc, _Znwm),
        MAC_INTERPOSE_PAIR(rpmalloc, _Znam),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_తిరగేయి, _Znwmm),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_తిరగేయి, _Znamm),
        MAC_INTERPOSE_PAIR(rpmalloc_nothrow, _ZnwmRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpmalloc_nothrow, _ZnamRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_తిరగేయి, _ZnwmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_తిరగేయి, _ZnamSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_తిరగేయి_nothrow,
                           _ZnwmSt11align_val_tRKSt9nothrow_t),
        MAC_INTERPOSE_PAIR(rpaligned_alloc_తిరగేయి_nothrow,
                           _ZnamSt11align_val_tRKSt9nothrow_t),
        // delete and delete[]
        MAC_INTERPOSE_PAIR(rpfree, _ZdlPv), MAC_INTERPOSE_PAIR(rpfree, _ZdaPv),
        MAC_INTERPOSE_PAIR(rpfree_కొలత, _ZdlPvm),
        MAC_INTERPOSE_PAIR(rpfree_కొలత, _ZdaPvm),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdlPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_aligned, _ZdaPvSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_కొలత_aligned, _ZdlPvmSt11align_val_t),
        MAC_INTERPOSE_PAIR(rpfree_కొలత_aligned, _ZdaPvmSt11align_val_t),
        // libc entry points
        MAC_INTERPOSE_PAIR(rpmalloc, జ్ఞాపకం_ఇవ్వు),
        MAC_INTERPOSE_PAIR(rpmalloc, సున్నా_జ్ఞాపకం_ఇవ్వు),
        MAC_INTERPOSE_PAIR(rprealloc, realloc),
        MAC_INTERPOSE_PAIR(rprealloc, reallocf),
#ఒకవేళ defined(__MAC_10_15) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
        MAC_INTERPOSE_PAIR(rpaligned_alloc, aligned_alloc),
#అయితే_ముగింపు
        MAC_INTERPOSE_PAIR(rpmemalign, memalign),
        MAC_INTERPOSE_PAIR(rpposix_memalign, posix_memalign),
        MAC_INTERPOSE_PAIR(rpfree, విడుదల), MAC_INTERPOSE_PAIR(rpfree, cfree),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_కొలత, malloc_usable_కొలత),
        MAC_INTERPOSE_PAIR(rpmalloc_usable_కొలత, malloc_కొలత)};

#అయితే_ముగింపు

#ఒకవేళ USE_ALIAS

#నిర్వచించు RPALIAS(fn) __attribute__((alias(#fn), వాడిన, visibility("default")));

// Alias the C++ operators using the mangled names
// (https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)

// operators delete and delete[]
శూన్య _ZdlPv(శూన్య *p) RPALIAS(rpfree) శూన్య _ZdaPv(శూన్య *p) RPALIAS(rpfree)

#ఒకవేళ ARCH_64_బిట్టు
    // 64-bit operators new and new[], normal and aligned
    శూన్య *_Znwm(uint64_t కొలత_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_కొలత(1)
        RPALIAS(rpmalloc) శూన్య *_Znam(uint64_t కొలత_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_కొలత(1) RPALIAS(rpmalloc) శూన్య *_Znwmm(uint64_t కొలత_2,
                                                                 uint64_t align)
        RPALIAS(rpaligned_alloc_తిరగేయి) శూన్య *_Znamm(uint64_t కొలత_2,
                                                      uint64_t align)
            RPALIAS(rpaligned_alloc_తిరగేయి) శూన్య *_ZnwmSt11align_val_t(
                size_t కొలత_2, size_t align)
                RPALIAS(rpaligned_alloc_తిరగేయి) శూన్య *_ZnamSt11align_val_t(
                    size_t కొలత_2, size_t align)
                    RPALIAS(rpaligned_alloc_తిరగేయి) శూన్య *_ZnwmRKSt9nothrow_t(
                        size_t కొలత_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) శూన్య *_ZnamRKSt9nothrow_t(
                            size_t కొలత_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) శూన్య
                            *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t కొలత_2,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_తిరగేయి_nothrow) శూన్య
                                    *_ZnamSt11align_val_tRKSt9nothrow_t(
                                        size_t కొలత_2, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_తిరగేయి_nothrow)
    // 64-bit operators delete and delete[], sized and aligned
    శూన్య _ZdlPvm(శూన్య *p, size_t n) RPALIAS(rpfree_కొలత) శూన్య _ZdaPvm(శూన్య *p,
                                                                      size_t n)
        RPALIAS(rpfree_కొలత) శూన్య _ZdlPvSt11align_val_t(శూన్య *p, size_t a)
            RPALIAS(rpfree_aligned) శూన్య _ZdaPvSt11align_val_t(శూన్య *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) శూన్య _ZdlPvmSt11align_val_t(శూన్య *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_కొలత_aligned) శూన్య _ZdaPvmSt11align_val_t(
                        శూన్య *p, size_t n, size_t a)
                        RPALIAS(rpfree_కొలత_aligned)
#లేకపోతే
    // 32-bit operators new and new[], normal and aligned
    శూన్య *_Znwj(uint32_t కొలత_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_కొలత(1)
        RPALIAS(rpmalloc) శూన్య *_Znaj(uint32_t కొలత_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_కొలత(1) RPALIAS(rpmalloc) శూన్య *_Znwjj(uint32_t కొలత_2,
                                                                 uint32_t align)
        RPALIAS(rpaligned_alloc_తిరగేయి) శూన్య *_Znajj(uint32_t కొలత_2,
                                                      uint32_t align)
            RPALIAS(rpaligned_alloc_తిరగేయి) శూన్య *_ZnwjSt11align_val_t(
                size_t కొలత_2, size_t align)
                RPALIAS(rpaligned_alloc_తిరగేయి) శూన్య *_ZnajSt11align_val_t(
                    size_t కొలత_2, size_t align)
                    RPALIAS(rpaligned_alloc_తిరగేయి) శూన్య *_ZnwjRKSt9nothrow_t(
                        size_t కొలత_2, rp_nothrow_t t)
                        RPALIAS(rpmalloc_nothrow) శూన్య *_ZnajRKSt9nothrow_t(
                            size_t కొలత_2,
                            rp_nothrow_t t) RPALIAS(rpmalloc_nothrow) శూన్య
                            *_ZnwjSt11align_val_tRKSt9nothrow_t(size_t కొలత_2,
                                                                size_t align,
                                                                rp_nothrow_t t)
                                RPALIAS(rpaligned_alloc_తిరగేయి_nothrow) శూన్య
                                    *_ZnajSt11align_val_tRKSt9nothrow_t(
                                        size_t కొలత_2, size_t align,
                                        rp_nothrow_t t)
                                        RPALIAS(rpaligned_alloc_తిరగేయి_nothrow)
    // 32-bit operators delete and delete[], sized and aligned
    శూన్య _ZdlPvj(శూన్య *p, size_t n) RPALIAS(rpfree_కొలత) శూన్య _ZdaPvj(శూన్య *p,
                                                                      size_t n)
        RPALIAS(rpfree_కొలత) శూన్య _ZdlPvSt11align_val_t(శూన్య *p, size_t a)
            RPALIAS(rpfree_aligned) శూన్య _ZdaPvSt11align_val_t(శూన్య *p,
                                                               size_t a)
                RPALIAS(rpfree_aligned) శూన్య _ZdlPvjSt11align_val_t(శూన్య *p,
                                                                    size_t n,
                                                                    size_t a)
                    RPALIAS(rpfree_కొలత_aligned) శూన్య _ZdaPvjSt11align_val_t(
                        శూన్య *p, size_t n, size_t a)
                        RPALIAS(rpfree_కొలత_aligned)
#అయితే_ముగింపు

                            శూన్య *జ్ఞాపకం_ఇవ్వు(size_t కొలత_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_కొలత(1)
        RPALIAS(rpmalloc) శూన్య *సున్నా_జ్ఞాపకం_ఇవ్వు(size_t లెక్క, size_t కొలత_2)
            RPALIAS(rpcalloc) శూన్య *realloc(శూన్య *ptr, size_t కొలత_2)
                RPALIAS(rprealloc) శూన్య *reallocf(శూన్య *ptr, size_t కొలత_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_కొలత(2)
        RPALIAS(rprealloc) శూన్య *aligned_alloc(size_t alignment, size_t కొలత_2)
            RPALIAS(rpaligned_alloc) శూన్య *memalign(
                size_t alignment, size_t కొలత_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_కొలత(2)
        RPALIAS(rpmemalign) పూర్ణాంకం posix_memalign(శూన్య **memptr, size_t alignment,
                                               size_t కొలత_2)
            RPALIAS(rpposix_memalign) శూన్య విడుదల(శూన్య *ptr)
                RPALIAS(rpfree) శూన్య cfree(శూన్య *ptr) RPALIAS(rpfree)
#ఒకవేళ defined(__ANDROID__) || defined(__FreeBSD__)
                    size_t
    malloc_usable_కొలత(స్థిరరాశి శూన్య *ptr) RPALIAS(rpmalloc_usable_కొలత)
#లేకపోతే
                    size_t
    malloc_usable_కొలత(శూన్య *ptr) RPALIAS(rpmalloc_usable_కొలత)
#అయితే_ముగింపు
        size_t malloc_కొలత(శూన్య *ptr) RPALIAS(rpmalloc_usable_కొలత)

#అయితే_ముగింపు

            స్థిర అంతఃవరుస size_t rpmalloc_పుట_కొలత(శూన్య) {
  తిరిగి జ్ఞాపకం_పుట_కొలత;
}

బాహ్య శూన్య *RPMALLOC_CDECL reallocarray(శూన్య *ptr, size_t లెక్క, size_t కొలత_2);

బాహ్య శూన్య *RPMALLOC_CDECL reallocarray(శూన్య *ptr, size_t లెక్క, size_t కొలత_2) {
  size_t మొత్తం_2;
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
#నిర్వచితమైతే _MSC_VER
  పూర్ణాంకం err = కొలత_TMult(లెక్క, కొలత_2, &మొత్తం_2);
  ఒకవేళ ((err != S_OK) || (మొత్తం_2 >= MAX_ALLOC_కొలత)) {
    errno = EINVAL;
    తిరిగి 0;
  }
#లేకపోతే
  పూర్ణాంకం err = __builtin_umull_overflow(లెక్క, కొలత_2, &మొత్తం_2);
  ఒకవేళ (err || (మొత్తం_2 >= MAX_ALLOC_కొలత)) {
    errno = EINVAL;
    తిరిగి 0;
  }
#అయితే_ముగింపు
#లేకపోతే
  మొత్తం_2 = లెక్క * కొలత_2;
#అయితే_ముగింపు
  తిరిగి realloc(ptr, మొత్తం_2);
}

బాహ్య అంతఃవరుస శూన్య *RPMALLOC_CDECL valloc(size_t కొలత_2) {
  get_thread_కుప్ప();
  తిరిగి rpaligned_alloc(rpmalloc_పుట_కొలత(), కొలత_2);
}

బాహ్య అంతఃవరుస శూన్య *RPMALLOC_CDECL pvalloc(size_t కొలత_2) {
  get_thread_కుప్ప();
  స్థిరరాశి size_t పుట_కొలత_2 = rpmalloc_పుట_కొలత();
  స్థిరరాశి size_t aligned_కొలత = ((కొలత_2 + పుట_కొలత_2 - 1) / పుట_కొలత_2) * పుట_కొలత_2;
#ఒకవేళ ENABLE_VALIDATE_వాదనలు
  ఒకవేళ (aligned_కొలత < కొలత_2) {
    errno = EINVAL;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  తిరిగి rpaligned_alloc(rpmalloc_పుట_కొలత(), aligned_కొలత);
}

#అయితే_ముగింపు // ENABLE_OVERRIDE

#ఒకవేళ ENABLE_PRELOAD

#నిర్వచితమైతే _WIN32

#ఒకవేళ defined(BUILD_DYNAMIC_LINK) && BUILD_DYNAMIC_LINK

బాహ్య __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason, LPVOID reserved);

బాహ్య __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE instance,
                                                 DWORD reason,
                                                 LPVOID reserved) {
  (శూన్య)పరిమాణం(reserved);
  (శూన్య)పరిమాణం(instance);
  ఒకవేళ (reason == DLL_PROCESS_ATTACH)
    rpmalloc_initialize();
  లేకపోతే ఒకవేళ (reason == DLL_PROCESS_DETACH)
    rpmalloc_finalize();
  లేకపోతే ఒకవేళ (reason == DLL_THREAD_ATTACH)
    rpmalloc_thread_initialize();
  లేకపోతే ఒకవేళ (reason == DLL_THREAD_DETACH)
    rpmalloc_thread_finalize(1);
  తిరిగి TRUE;
}

// end BUILD_DYNAMIC_LINK
#లేకపోతే

బాహ్య శూన్య _global_rpmalloc_init(శూన్య) {
  rpmalloc_సమితి_main_thread();
  rpmalloc_initialize();
}

#ఒకవేళ defined(__clang__) || defined(__GNUC__)

స్థిర శూన్య __attribute__((constructor)) initializer(శూన్య) {
  _global_rpmalloc_init();
}

#లేదావేళ defined(_MSC_VER)

స్థిర పూర్ణాంకం _global_rpmalloc_xib(శూన్య) {
  _global_rpmalloc_init();
  తిరిగి 0;
}

#సూచన section(".CRT$XIB", పఠ)
__declspec(allocate(".CRT$XIB")) శూన్య (*_rpmalloc_module_init)(శూన్య) =
    _global_rpmalloc_xib;
#ఒకవేళ defined(_M_IX86) || defined(__i386__)
#సూచన comment(linker, "/include:"                                            \
                        "__rpmalloc_module_init")
#లేకపోతే
#సూచన comment(linker, "/include:"                                            \
                        "_rpmalloc_module_init")
#అయితే_ముగింపు

#అయితే_ముగింపు

// end !BUILD_DYNAMIC_LINK
#అయితే_ముగింపు

#లేకపోతే

#అన్తర్భావ <pthread.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <unistd.h>

బాహ్య శూన్య rpmalloc_సమితి_main_thread(శూన్య);

స్థిర pthread_key_t destructor_కీలకం;

స్థిర శూన్య thread_destructor(శూన్య *);

స్థిర శూన్య __attribute__((constructor)) initializer(శూన్య) {
  rpmalloc_సమితి_main_thread();
  rpmalloc_initialize();
  pthread_కీలకం_create(&destructor_కీలకం, thread_destructor);
}

స్థిర శూన్య __attribute__((destructor)) finalizer(శూన్య) { rpmalloc_finalize(); }

రకనిర్వచనం నిర్మాణం {
  శూన్య *(*real_ప్రారంభం)(శూన్య *);
  శూన్య *real_arg;
} thread_starter_arg;

స్థిర శూన్య *thread_starter(శూన్య *argptr) {
  thread_starter_arg *arg = argptr;
  శూన్య *(*real_ప్రారంభం)(శూన్య *) = arg->real_ప్రారంభం;
  శూన్య *real_arg = arg->real_arg;
  rpmalloc_thread_initialize();
  rpfree(argptr);
  pthread_setspecific(destructor_కీలకం, (శూన్య *)1);
  తిరిగి (*real_ప్రారంభం)(real_arg);
}

స్థిర శూన్య thread_destructor(శూన్య *విలువ_2) {
  (శూన్య)పరిమాణం(విలువ_2);
  rpmalloc_thread_finalize(1);
}

#నిర్వచితమైతే __APPLE__

స్థిర పూర్ణాంకం pthread_create_proxy(pthread_t *thread, స్థిరరాశి pthread_attr_t *attr,
                                శూన్య *(*ప్రారంభం_routine)(శూన్య *), శూన్య *arg) {
  rpmalloc_initialize();
  thread_starter_arg *starter_arg = rpmalloc(పరిమాణం(thread_starter_arg));
  starter_arg->real_ప్రారంభం = ప్రారంభం_routine;
  starter_arg->real_arg = arg;
  తిరిగి pthread_create(thread, attr, thread_starter, starter_arg);
}

MAC_INTERPOSE_SINGLE(pthread_create_proxy, pthread_create);

#లేకపోతే

#అన్తర్భావ <dlfcn.h>

పూర్ణాంకం pthread_create(pthread_t *thread, స్థిరరాశి pthread_attr_t *attr,
                   శూన్య *(*ప్రారంభం_routine)(శూన్య *), శూన్య *arg) {
#ఒకవేళ defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) ||     \
    defined(__HAIKU__)
  అక్షరం fname[] = "pthread_create";
#లేకపోతే
  అక్షరం fname[] = "_pthread_create";
#అయితే_ముగింపు
  శూన్య *real_pthread_create = dlsym(RTLD_NEXT, fname);
  rpmalloc_thread_initialize();
  thread_starter_arg *starter_arg = rpmalloc(పరిమాణం(thread_starter_arg));
  starter_arg->real_ప్రారంభం = ప్రారంభం_routine;
  starter_arg->real_arg = arg;
  తిరిగి (*(పూర్ణాంకం (*)(pthread_t *, స్థిరరాశి pthread_attr_t *, శూన్య *(*)(శూన్య *),
                    శూన్య *))real_pthread_create)(thread, attr, thread_starter,
                                                 starter_arg);
}

#అయితే_ముగింపు

#అయితే_ముగింపు

#అయితే_ముగింపు

#ఒకవేళ ENABLE_OVERRIDE

#ఒకవేళ defined(__GLIBC__) && defined(__linux__)

శూన్య *__libc_malloc(size_t కొలత_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_కొలత(1)
        RPALIAS(rpmalloc) శూన్య *__libc_calloc(size_t లెక్క, size_t కొలత_2)
            RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2)
                RPALIAS(rpcalloc) శూన్య *__libc_realloc(శూన్య *p, size_t కొలత_2)
                    RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_కొలత(2) RPALIAS(rprealloc) శూన్య __libc_free(శూన్య *p)
        RPALIAS(rpfree) శూన్య __libc_cfree(శూన్య *p)
            RPALIAS(rpfree) శూన్య *__libc_memalign(size_t align, size_t కొలత_2)
                RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_కొలత(2)
        RPALIAS(rpmemalign) పూర్ణాంకం __posix_memalign(శూన్య **p, size_t align,
                                                 size_t కొలత_2)
            RPALIAS(rpposix_memalign)

                బాహ్య శూన్య *__libc_valloc(size_t కొలత_2);
బాహ్య శూన్య *__libc_pvalloc(size_t కొలత_2);

శూన్య *__libc_valloc(size_t కొలత_2) { తిరిగి valloc(కొలత_2); }

శూన్య *__libc_pvalloc(size_t కొలత_2) { తిరిగి pvalloc(కొలత_2); }

#అయితే_ముగింపు

#అయితే_ముగింపు

#ఒకవేళ (defined(__GNUC__) || defined(__clang__))
#సూచన GCC visibility తీయి
#అయితే_ముగింపు
