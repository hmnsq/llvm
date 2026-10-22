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

#यदि_परिभाषितम् __cplusplus

#अन्तर्भाव <new>
#अन्तर्भाव <rpmalloc.h>

#यदि_अपरिभाषितम् __CRTDECL
#परिभाषय __CRTDECL
#यदि_अन्तः

बाह्य शून्य __CRTDECL संक्रियकः delete(शून्य *p) noexcept { rpfree(p); }

बाह्य शून्य __CRTDECL संक्रियकः delete[](शून्य *p) noexcept { rpfree(p); }

बाह्य शून्य *__CRTDECL संक्रियकः new(std::size_t आकार_2) noexcept(false) {
  प्रत्यागम rpmalloc(आकार_2);
}

बाह्य शून्य *__CRTDECL संक्रियकः new[](std::size_t आकार_2) noexcept(false) {
  प्रत्यागम rpmalloc(आकार_2);
}

बाह्य शून्य *__CRTDECL संक्रियकः new(std::size_t आकार_2,
                                    ध्रुव std::nothrow_t &चिह्नकम्_2) noexcept {
  (शून्य)परिमाण(चिह्नकम्_2);
  प्रत्यागम rpmalloc(आकार_2);
}

बाह्य शून्य *__CRTDECL संक्रियकः new[](std::size_t आकार_2,
                                      ध्रुव std::nothrow_t &चिह्नकम्_2) noexcept {
  (शून्य)परिमाण(चिह्नकम्_2);
  प्रत्यागम rpmalloc(आकार_2);
}

#यदि (__cplusplus >= 201402L || _MSC_VER >= 1916)

बाह्य शून्य __CRTDECL संक्रियकः delete(शून्य *p, std::size_t आकार_2) noexcept {
  (शून्य)परिमाण(आकार_2);
  rpfree(p);
}

बाह्य शून्य __CRTDECL संक्रियकः delete[](शून्य *p, std::size_t आकार_2) noexcept {
  (शून्य)परिमाण(आकार_2);
  rpfree(p);
}

#यदि_अन्तः

#यदि (__cplusplus > 201402L || defined(__cpp_aligned_new))

बाह्य शून्य __CRTDECL संक्रियकः delete(शून्य *p,
                                      std::align_val_t align) noexcept {
  (शून्य)परिमाण(align);
  rpfree(p);
}

बाह्य शून्य __CRTDECL संक्रियकः delete[](शून्य *p,
                                        std::align_val_t align) noexcept {
  (शून्य)परिमाण(align);
  rpfree(p);
}

बाह्य शून्य __CRTDECL संक्रियकः delete(शून्य *p, std::size_t आकार_2,
                                      std::align_val_t align) noexcept {
  (शून्य)परिमाण(आकार_2);
  (शून्य)परिमाण(align);
  rpfree(p);
}

बाह्य शून्य __CRTDECL संक्रियकः delete[](शून्य *p, std::size_t आकार_2,
                                        std::align_val_t align) noexcept {
  (शून्य)परिमाण(आकार_2);
  (शून्य)परिमाण(align);
  rpfree(p);
}

बाह्य शून्य *__CRTDECL संक्रियकः new(std::size_t आकार_2,
                                    std::align_val_t align) noexcept(false) {
  प्रत्यागम rpaligned_alloc(static_cast<size_t>(align), आकार_2);
}

बाह्य शून्य *__CRTDECL संक्रियकः new[](std::size_t आकार_2,
                                      std::align_val_t align) noexcept(false) {
  प्रत्यागम rpaligned_alloc(static_cast<size_t>(align), आकार_2);
}

बाह्य शून्य *__CRTDECL संक्रियकः new(std::size_t आकार_2, std::align_val_t align,
                                    ध्रुव std::nothrow_t &चिह्नकम्_2) noexcept {
  (शून्य)परिमाण(चिह्नकम्_2);
  प्रत्यागम rpaligned_alloc(static_cast<size_t>(align), आकार_2);
}

बाह्य शून्य *__CRTDECL संक्रियकः new[](std::size_t आकार_2, std::align_val_t align,
                                      ध्रुव std::nothrow_t &चिह्नकम्_2) noexcept {
  (शून्य)परिमाण(चिह्नकम्_2);
  प्रत्यागम rpaligned_alloc(static_cast<size_t>(align), आकार_2);
}

#यदि_अन्तः

#यदि_अन्तः
