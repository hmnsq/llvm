//===-------------------------- rpnew.h -----------------*- C -*-=============//
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
//===----------------------------------------------------------------------===//

#ifdef __cplusplus

#অন্তর্ভাব <new>
#অন্তর্ভাব <rpmalloc.h>

#ifndef __CRTDECL
#define __CRTDECL
#endif

বাহ্যিক শূন্য __CRTDECL operator delete(শূন্য *p) noexcept { rpfree(p); }

বাহ্যিক শূন্য __CRTDECL operator delete[](শূন্য *p) noexcept { rpfree(p); }

বাহ্যিক শূন্য *__CRTDECL operator new(std::size_t আকার_3) noexcept(false) {
  ফেরত rpmalloc(আকার_3);
}

বাহ্যিক শূন্য *__CRTDECL operator new[](std::size_t আকার_3) noexcept(false) {
  ফেরত rpmalloc(আকার_3);
}

বাহ্যিক শূন্য *__CRTDECL operator new(std::size_t আকার_3,
                                    ধ্রুবক std::nothrow_t &tag) noexcept {
  (শূন্য)আকার(tag);
  ফেরত rpmalloc(আকার_3);
}

বাহ্যিক শূন্য *__CRTDECL operator new[](std::size_t আকার_3,
                                      ধ্রুবক std::nothrow_t &tag) noexcept {
  (শূন্য)আকার(tag);
  ফেরত rpmalloc(আকার_3);
}

#যদি (__cplusplus >= 201402L || _MSC_VER >= 1916)

বাহ্যিক শূন্য __CRTDECL operator delete(শূন্য *p, std::size_t আকার_3) noexcept {
  (শূন্য)আকার(আকার_3);
  rpfree(p);
}

বাহ্যিক শূন্য __CRTDECL operator delete[](শূন্য *p, std::size_t আকার_3) noexcept {
  (শূন্য)আকার(আকার_3);
  rpfree(p);
}

#endif

#যদি (__cplusplus > 201402L || defined(__cpp_aligned_new))

বাহ্যিক শূন্য __CRTDECL operator delete(শূন্য *p,
                                      std::align_val_t align) noexcept {
  (শূন্য)আকার(align);
  rpfree(p);
}

বাহ্যিক শূন্য __CRTDECL operator delete[](শূন্য *p,
                                        std::align_val_t align) noexcept {
  (শূন্য)আকার(align);
  rpfree(p);
}

বাহ্যিক শূন্য __CRTDECL operator delete(শূন্য *p, std::size_t আকার_3,
                                      std::align_val_t align) noexcept {
  (শূন্য)আকার(আকার_3);
  (শূন্য)আকার(align);
  rpfree(p);
}

বাহ্যিক শূন্য __CRTDECL operator delete[](শূন্য *p, std::size_t আকার_3,
                                        std::align_val_t align) noexcept {
  (শূন্য)আকার(আকার_3);
  (শূন্য)আকার(align);
  rpfree(p);
}

বাহ্যিক শূন্য *__CRTDECL operator new(std::size_t আকার_3,
                                    std::align_val_t align) noexcept(false) {
  ফেরত rpaligned_alloc(static_cast<size_t>(align), আকার_3);
}

বাহ্যিক শূন্য *__CRTDECL operator new[](std::size_t আকার_3,
                                      std::align_val_t align) noexcept(false) {
  ফেরত rpaligned_alloc(static_cast<size_t>(align), আকার_3);
}

বাহ্যিক শূন্য *__CRTDECL operator new(std::size_t আকার_3, std::align_val_t align,
                                    ধ্রুবক std::nothrow_t &tag) noexcept {
  (শূন্য)আকার(tag);
  ফেরত rpaligned_alloc(static_cast<size_t>(align), আকার_3);
}

বাহ্যিক শূন্য *__CRTDECL operator new[](std::size_t আকার_3, std::align_val_t align,
                                      ধ্রুবক std::nothrow_t &tag) noexcept {
  (শূন্য)আকার(tag);
  ফেরত rpaligned_alloc(static_cast<size_t>(align), আকার_3);
}

#endif

#endif
