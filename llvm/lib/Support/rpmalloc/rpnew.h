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

#अगर_घोषित __cplusplus

#समावेश <new>
#समावेश <rpmalloc.h>

#अगर_अघोषित __CRTDECL
#घोषणा __CRTDECL
#पूर्ण_अगर

बाह्य व्योम __CRTDECL चालक delete(व्योम *p) noexcept { rpfree(p); }

बाह्य व्योम __CRTDECL चालक delete[](व्योम *p) noexcept { rpfree(p); }

बाह्य व्योम *__CRTDECL चालक new(std::माप_प्रकार आकार_2) noexcept(false) {
  वापस rpmalloc(आकार_2);
}

बाह्य व्योम *__CRTDECL चालक new[](std::माप_प्रकार आकार_2) noexcept(false) {
  वापस rpmalloc(आकार_2);
}

बाह्य व्योम *__CRTDECL चालक new(std::माप_प्रकार आकार_2,
                                    स्थिर std::nothrow_t &tag) noexcept {
  (व्योम)माप(tag);
  वापस rpmalloc(आकार_2);
}

बाह्य व्योम *__CRTDECL चालक new[](std::माप_प्रकार आकार_2,
                                      स्थिर std::nothrow_t &tag) noexcept {
  (व्योम)माप(tag);
  वापस rpmalloc(आकार_2);
}

#अगर (__cplusplus >= 201402L || _MSC_VER >= 1916)

बाह्य व्योम __CRTDECL चालक delete(व्योम *p, std::माप_प्रकार आकार_2) noexcept {
  (व्योम)माप(आकार_2);
  rpfree(p);
}

बाह्य व्योम __CRTDECL चालक delete[](व्योम *p, std::माप_प्रकार आकार_2) noexcept {
  (व्योम)माप(आकार_2);
  rpfree(p);
}

#पूर्ण_अगर

#अगर (__cplusplus > 201402L || defined(__cpp_aligned_new))

बाह्य व्योम __CRTDECL चालक delete(व्योम *p,
                                      std::align_val_t align) noexcept {
  (व्योम)माप(align);
  rpfree(p);
}

बाह्य व्योम __CRTDECL चालक delete[](व्योम *p,
                                        std::align_val_t align) noexcept {
  (व्योम)माप(align);
  rpfree(p);
}

बाह्य व्योम __CRTDECL चालक delete(व्योम *p, std::माप_प्रकार आकार_2,
                                      std::align_val_t align) noexcept {
  (व्योम)माप(आकार_2);
  (व्योम)माप(align);
  rpfree(p);
}

बाह्य व्योम __CRTDECL चालक delete[](व्योम *p, std::माप_प्रकार आकार_2,
                                        std::align_val_t align) noexcept {
  (व्योम)माप(आकार_2);
  (व्योम)माप(align);
  rpfree(p);
}

बाह्य व्योम *__CRTDECL चालक new(std::माप_प्रकार आकार_2,
                                    std::align_val_t align) noexcept(false) {
  वापस rpaligned_alloc(static_cast<माप_प्रकार>(align), आकार_2);
}

बाह्य व्योम *__CRTDECL चालक new[](std::माप_प्रकार आकार_2,
                                      std::align_val_t align) noexcept(false) {
  वापस rpaligned_alloc(static_cast<माप_प्रकार>(align), आकार_2);
}

बाह्य व्योम *__CRTDECL चालक new(std::माप_प्रकार आकार_2, std::align_val_t align,
                                    स्थिर std::nothrow_t &tag) noexcept {
  (व्योम)माप(tag);
  वापस rpaligned_alloc(static_cast<माप_प्रकार>(align), आकार_2);
}

बाह्य व्योम *__CRTDECL चालक new[](std::माप_प्रकार आकार_2, std::align_val_t align,
                                      स्थिर std::nothrow_t &tag) noexcept {
  (व्योम)माप(tag);
  वापस rpaligned_alloc(static_cast<माप_प्रकार>(align), आकार_2);
}

#पूर्ण_अगर

#पूर्ण_अगर
