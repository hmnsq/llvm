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

#ଅନ୍ତର୍ଭାଵ <new>
#ଅନ୍ତର୍ଭାଵ <rpmalloc.h>

#ifndef __CRTDECL
#define __CRTDECL
#endif

ବାହ୍ୟ ଶୂନ୍ୟ __CRTDECL operator delete(ଶୂନ୍ୟ *p) noexcept { rpfree(p); }

ବାହ୍ୟ ଶୂନ୍ୟ __CRTDECL operator delete[](ଶୂନ୍ୟ *p) noexcept { rpfree(p); }

ବାହ୍ୟ ଶୂନ୍ୟ *__CRTDECL operator new(std::size_t size) noexcept(false) {
  ଫେରନ୍ତୁ rpmalloc(size);
}

ବାହ୍ୟ ଶୂନ୍ୟ *__CRTDECL operator new[](std::size_t size) noexcept(false) {
  ଫେରନ୍ତୁ rpmalloc(size);
}

ବାହ୍ୟ ଶୂନ୍ୟ *__CRTDECL operator new(std::size_t size,
                                    ଅଚଳ std::nothrow_t &tag) noexcept {
  (ଶୂନ୍ୟ)ଆକାର(tag);
  ଫେରନ୍ତୁ rpmalloc(size);
}

ବାହ୍ୟ ଶୂନ୍ୟ *__CRTDECL operator new[](std::size_t size,
                                      ଅଚଳ std::nothrow_t &tag) noexcept {
  (ଶୂନ୍ୟ)ଆକାର(tag);
  ଫେରନ୍ତୁ rpmalloc(size);
}

#ଯଦି (__cplusplus >= 201402L || _MSC_VER >= 1916)

ବାହ୍ୟ ଶୂନ୍ୟ __CRTDECL operator delete(ଶୂନ୍ୟ *p, std::size_t size) noexcept {
  (ଶୂନ୍ୟ)ଆକାର(size);
  rpfree(p);
}

ବାହ୍ୟ ଶୂନ୍ୟ __CRTDECL operator delete[](ଶୂନ୍ୟ *p, std::size_t size) noexcept {
  (ଶୂନ୍ୟ)ଆକାର(size);
  rpfree(p);
}

#endif

#ଯଦି (__cplusplus > 201402L || defined(__cpp_aligned_new))

ବାହ୍ୟ ଶୂନ୍ୟ __CRTDECL operator delete(ଶୂନ୍ୟ *p,
                                      std::align_val_t align) noexcept {
  (ଶୂନ୍ୟ)ଆକାର(align);
  rpfree(p);
}

ବାହ୍ୟ ଶୂନ୍ୟ __CRTDECL operator delete[](ଶୂନ୍ୟ *p,
                                        std::align_val_t align) noexcept {
  (ଶୂନ୍ୟ)ଆକାର(align);
  rpfree(p);
}

ବାହ୍ୟ ଶୂନ୍ୟ __CRTDECL operator delete(ଶୂନ୍ୟ *p, std::size_t size,
                                      std::align_val_t align) noexcept {
  (ଶୂନ୍ୟ)ଆକାର(size);
  (ଶୂନ୍ୟ)ଆକାର(align);
  rpfree(p);
}

ବାହ୍ୟ ଶୂନ୍ୟ __CRTDECL operator delete[](ଶୂନ୍ୟ *p, std::size_t size,
                                        std::align_val_t align) noexcept {
  (ଶୂନ୍ୟ)ଆକାର(size);
  (ଶୂନ୍ୟ)ଆକାର(align);
  rpfree(p);
}

ବାହ୍ୟ ଶୂନ୍ୟ *__CRTDECL operator new(std::size_t size,
                                    std::align_val_t align) noexcept(false) {
  ଫେରନ୍ତୁ rpaligned_alloc(static_cast<size_t>(align), size);
}

ବାହ୍ୟ ଶୂନ୍ୟ *__CRTDECL operator new[](std::size_t size,
                                      std::align_val_t align) noexcept(false) {
  ଫେରନ୍ତୁ rpaligned_alloc(static_cast<size_t>(align), size);
}

ବାହ୍ୟ ଶୂନ୍ୟ *__CRTDECL operator new(std::size_t size, std::align_val_t align,
                                    ଅଚଳ std::nothrow_t &tag) noexcept {
  (ଶୂନ୍ୟ)ଆକାର(tag);
  ଫେରନ୍ତୁ rpaligned_alloc(static_cast<size_t>(align), size);
}

ବାହ୍ୟ ଶୂନ୍ୟ *__CRTDECL operator new[](std::size_t size, std::align_val_t align,
                                      ଅଚଳ std::nothrow_t &tag) noexcept {
  (ଶୂନ୍ୟ)ଆକାର(tag);
  ଫେରନ୍ତୁ rpaligned_alloc(static_cast<size_t>(align), size);
}

#endif

#endif
