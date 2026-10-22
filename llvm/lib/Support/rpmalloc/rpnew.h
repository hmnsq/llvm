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

#અન્તર્ભાવ <new>
#અન્તર્ભાવ <rpmalloc.h>

#ifndef __CRTDECL
#define __CRTDECL
#endif

બાહ્ય શૂન્ય __CRTDECL operator delete(શૂન્ય *p) noexcept { rpfree(p); }

બાહ્ય શૂન્ય __CRTDECL operator delete[](શૂન્ય *p) noexcept { rpfree(p); }

બાહ્ય શૂન્ય *__CRTDECL operator new(std::size_t size) noexcept(false) {
  પાછા rpmalloc(size);
}

બાહ્ય શૂન્ય *__CRTDECL operator new[](std::size_t size) noexcept(false) {
  પાછા rpmalloc(size);
}

બાહ્ય શૂન્ય *__CRTDECL operator new(std::size_t size,
                                    અચલ std::nothrow_t &tag) noexcept {
  (શૂન્ય)કદ(tag);
  પાછા rpmalloc(size);
}

બાહ્ય શૂન્ય *__CRTDECL operator new[](std::size_t size,
                                      અચલ std::nothrow_t &tag) noexcept {
  (શૂન્ય)કદ(tag);
  પાછા rpmalloc(size);
}

#જો (__cplusplus >= 201402L || _MSC_VER >= 1916)

બાહ્ય શૂન્ય __CRTDECL operator delete(શૂન્ય *p, std::size_t size) noexcept {
  (શૂન્ય)કદ(size);
  rpfree(p);
}

બાહ્ય શૂન્ય __CRTDECL operator delete[](શૂન્ય *p, std::size_t size) noexcept {
  (શૂન્ય)કદ(size);
  rpfree(p);
}

#endif

#જો (__cplusplus > 201402L || defined(__cpp_aligned_new))

બાહ્ય શૂન્ય __CRTDECL operator delete(શૂન્ય *p,
                                      std::align_val_t align) noexcept {
  (શૂન્ય)કદ(align);
  rpfree(p);
}

બાહ્ય શૂન્ય __CRTDECL operator delete[](શૂન્ય *p,
                                        std::align_val_t align) noexcept {
  (શૂન્ય)કદ(align);
  rpfree(p);
}

બાહ્ય શૂન્ય __CRTDECL operator delete(શૂન્ય *p, std::size_t size,
                                      std::align_val_t align) noexcept {
  (શૂન્ય)કદ(size);
  (શૂન્ય)કદ(align);
  rpfree(p);
}

બાહ્ય શૂન્ય __CRTDECL operator delete[](શૂન્ય *p, std::size_t size,
                                        std::align_val_t align) noexcept {
  (શૂન્ય)કદ(size);
  (શૂન્ય)કદ(align);
  rpfree(p);
}

બાહ્ય શૂન્ય *__CRTDECL operator new(std::size_t size,
                                    std::align_val_t align) noexcept(false) {
  પાછા rpaligned_alloc(static_cast<size_t>(align), size);
}

બાહ્ય શૂન્ય *__CRTDECL operator new[](std::size_t size,
                                      std::align_val_t align) noexcept(false) {
  પાછા rpaligned_alloc(static_cast<size_t>(align), size);
}

બાહ્ય શૂન્ય *__CRTDECL operator new(std::size_t size, std::align_val_t align,
                                    અચલ std::nothrow_t &tag) noexcept {
  (શૂન્ય)કદ(tag);
  પાછા rpaligned_alloc(static_cast<size_t>(align), size);
}

બાહ્ય શૂન્ય *__CRTDECL operator new[](std::size_t size, std::align_val_t align,
                                      અચલ std::nothrow_t &tag) noexcept {
  (શૂન્ય)કદ(tag);
  પાછા rpaligned_alloc(static_cast<size_t>(align), size);
}

#endif

#endif
