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

#ಅನ್ತರ್ಭಾವ <new>
#ಅನ್ತರ್ಭಾವ <rpmalloc.h>

#ifndef __CRTDECL
#define __CRTDECL
#endif

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL operator delete(ಶೂನ್ಯ *p) noexcept { rpfree(p); }

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL operator delete[](ಶೂನ್ಯ *p) noexcept { rpfree(p); }

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL operator new(std::size_t size) noexcept(false) {
  ಪ್ರತ್ಯಾಗಮ rpmalloc(size);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL operator new[](std::size_t size) noexcept(false) {
  ಪ್ರತ್ಯಾಗಮ rpmalloc(size);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL operator new(std::size_t size,
                                    ಧ್ರುವ std::nothrow_t &tag) noexcept {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(tag);
  ಪ್ರತ್ಯಾಗಮ rpmalloc(size);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL operator new[](std::size_t size,
                                      ಧ್ರುವ std::nothrow_t &tag) noexcept {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(tag);
  ಪ್ರತ್ಯಾಗಮ rpmalloc(size);
}

#ಯದಿ (__cplusplus >= 201402L || _MSC_VER >= 1916)

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL operator delete(ಶೂನ್ಯ *p, std::size_t size) noexcept {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
  rpfree(p);
}

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL operator delete[](ಶೂನ್ಯ *p, std::size_t size) noexcept {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
  rpfree(p);
}

#endif

#ಯದಿ (__cplusplus > 201402L || defined(__cpp_aligned_new))

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL operator delete(ಶೂನ್ಯ *p,
                                      std::align_val_t align) noexcept {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(align);
  rpfree(p);
}

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL operator delete[](ಶೂನ್ಯ *p,
                                        std::align_val_t align) noexcept {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(align);
  rpfree(p);
}

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL operator delete(ಶೂನ್ಯ *p, std::size_t size,
                                      std::align_val_t align) noexcept {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(align);
  rpfree(p);
}

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL operator delete[](ಶೂನ್ಯ *p, std::size_t size,
                                        std::align_val_t align) noexcept {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(size);
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(align);
  rpfree(p);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL operator new(std::size_t size,
                                    std::align_val_t align) noexcept(false) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(static_cast<size_t>(align), size);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL operator new[](std::size_t size,
                                      std::align_val_t align) noexcept(false) {
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(static_cast<size_t>(align), size);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL operator new(std::size_t size, std::align_val_t align,
                                    ಧ್ರುವ std::nothrow_t &tag) noexcept {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(tag);
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(static_cast<size_t>(align), size);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL operator new[](std::size_t size, std::align_val_t align,
                                      ಧ್ರುವ std::nothrow_t &tag) noexcept {
  (ಶೂನ್ಯ)ಪರಿಮಾಣ(tag);
  ಪ್ರತ್ಯಾಗಮ rpaligned_alloc(static_cast<size_t>(align), size);
}

#endif

#endif
