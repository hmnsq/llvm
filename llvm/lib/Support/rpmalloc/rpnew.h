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

#अन्तर्भाव <new>
#अन्तर्भाव <rpmalloc.h>

#ifndef __CRTDECL
#define __CRTDECL
#endif

बाह्य शून्य __CRTDECL चालक delete(शून्य *p) noexcept { rpfree(p); }

बाह्य शून्य __CRTDECL चालक delete[](शून्य *p) noexcept { rpfree(p); }

बाह्य शून्य *__CRTDECL चालक new(std::माप_प्रकार size) noexcept(false) {
  प्रत्यागम rpmalloc(size);
}

बाह्य शून्य *__CRTDECL चालक new[](std::माप_प्रकार size) noexcept(false) {
  प्रत्यागम rpmalloc(size);
}

बाह्य शून्य *__CRTDECL चालक new(std::माप_प्रकार size,
                                    ध्रुव std::nothrow_t &tag) noexcept {
  (शून्य)परिमाण(tag);
  प्रत्यागम rpmalloc(size);
}

बाह्य शून्य *__CRTDECL चालक new[](std::माप_प्रकार size,
                                      ध्रुव std::nothrow_t &tag) noexcept {
  (शून्य)परिमाण(tag);
  प्रत्यागम rpmalloc(size);
}

#यदि (__cplusplus >= 201402L || _MSC_VER >= 1916)

बाह्य शून्य __CRTDECL चालक delete(शून्य *p, std::माप_प्रकार size) noexcept {
  (शून्य)परिमाण(size);
  rpfree(p);
}

बाह्य शून्य __CRTDECL चालक delete[](शून्य *p, std::माप_प्रकार size) noexcept {
  (शून्य)परिमाण(size);
  rpfree(p);
}

#endif

#यदि (__cplusplus > 201402L || defined(__cpp_aligned_new))

बाह्य शून्य __CRTDECL चालक delete(शून्य *p,
                                      std::align_val_t align) noexcept {
  (शून्य)परिमाण(align);
  rpfree(p);
}

बाह्य शून्य __CRTDECL चालक delete[](शून्य *p,
                                        std::align_val_t align) noexcept {
  (शून्य)परिमाण(align);
  rpfree(p);
}

बाह्य शून्य __CRTDECL चालक delete(शून्य *p, std::माप_प्रकार size,
                                      std::align_val_t align) noexcept {
  (शून्य)परिमाण(size);
  (शून्य)परिमाण(align);
  rpfree(p);
}

बाह्य शून्य __CRTDECL चालक delete[](शून्य *p, std::माप_प्रकार size,
                                        std::align_val_t align) noexcept {
  (शून्य)परिमाण(size);
  (शून्य)परिमाण(align);
  rpfree(p);
}

बाह्य शून्य *__CRTDECL चालक new(std::माप_प्रकार size,
                                    std::align_val_t align) noexcept(false) {
  प्रत्यागम rpaligned_alloc(static_cast<माप_प्रकार>(align), size);
}

बाह्य शून्य *__CRTDECL चालक new[](std::माप_प्रकार size,
                                      std::align_val_t align) noexcept(false) {
  प्रत्यागम rpaligned_alloc(static_cast<माप_प्रकार>(align), size);
}

बाह्य शून्य *__CRTDECL चालक new(std::माप_प्रकार size, std::align_val_t align,
                                    ध्रुव std::nothrow_t &tag) noexcept {
  (शून्य)परिमाण(tag);
  प्रत्यागम rpaligned_alloc(static_cast<माप_प्रकार>(align), size);
}

बाह्य शून्य *__CRTDECL चालक new[](std::माप_प्रकार size, std::align_val_t align,
                                      ध्रुव std::nothrow_t &tag) noexcept {
  (शून्य)परिमाण(tag);
  प्रत्यागम rpaligned_alloc(static_cast<माप_प्रकार>(align), size);
}

#endif

#endif
