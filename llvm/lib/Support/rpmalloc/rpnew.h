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

#അന്തര്ഭാവ <new>
#അന്തര്ഭാവ <rpmalloc.h>

#ifndef __CRTDECL
#define __CRTDECL
#endif

ബാഹ്യ ശൂന്യം __CRTDECL operator delete(ശൂന്യം *p) noexcept { rpfree(p); }

ബാഹ്യ ശൂന്യം __CRTDECL operator delete[](ശൂന്യം *p) noexcept { rpfree(p); }

ബാഹ്യ ശൂന്യം *__CRTDECL operator new(std::size_t size) noexcept(false) {
  തിരികെ rpmalloc(size);
}

ബാഹ്യ ശൂന്യം *__CRTDECL operator new[](std::size_t size) noexcept(false) {
  തിരികെ rpmalloc(size);
}

ബാഹ്യ ശൂന്യം *__CRTDECL operator new(std::size_t size,
                                    സ്ഥിരം std::nothrow_t &tag) noexcept {
  (ശൂന്യം)വലുപ്പം(tag);
  തിരികെ rpmalloc(size);
}

ബാഹ്യ ശൂന്യം *__CRTDECL operator new[](std::size_t size,
                                      സ്ഥിരം std::nothrow_t &tag) noexcept {
  (ശൂന്യം)വലുപ്പം(tag);
  തിരികെ rpmalloc(size);
}

#എങ്കിൽ (__cplusplus >= 201402L || _MSC_VER >= 1916)

ബാഹ്യ ശൂന്യം __CRTDECL operator delete(ശൂന്യം *p, std::size_t size) noexcept {
  (ശൂന്യം)വലുപ്പം(size);
  rpfree(p);
}

ബാഹ്യ ശൂന്യം __CRTDECL operator delete[](ശൂന്യം *p, std::size_t size) noexcept {
  (ശൂന്യം)വലുപ്പം(size);
  rpfree(p);
}

#endif

#എങ്കിൽ (__cplusplus > 201402L || defined(__cpp_aligned_new))

ബാഹ്യ ശൂന്യം __CRTDECL operator delete(ശൂന്യം *p,
                                      std::align_val_t align) noexcept {
  (ശൂന്യം)വലുപ്പം(align);
  rpfree(p);
}

ബാഹ്യ ശൂന്യം __CRTDECL operator delete[](ശൂന്യം *p,
                                        std::align_val_t align) noexcept {
  (ശൂന്യം)വലുപ്പം(align);
  rpfree(p);
}

ബാഹ്യ ശൂന്യം __CRTDECL operator delete(ശൂന്യം *p, std::size_t size,
                                      std::align_val_t align) noexcept {
  (ശൂന്യം)വലുപ്പം(size);
  (ശൂന്യം)വലുപ്പം(align);
  rpfree(p);
}

ബാഹ്യ ശൂന്യം __CRTDECL operator delete[](ശൂന്യം *p, std::size_t size,
                                        std::align_val_t align) noexcept {
  (ശൂന്യം)വലുപ്പം(size);
  (ശൂന്യം)വലുപ്പം(align);
  rpfree(p);
}

ബാഹ്യ ശൂന്യം *__CRTDECL operator new(std::size_t size,
                                    std::align_val_t align) noexcept(false) {
  തിരികെ rpaligned_alloc(static_cast<size_t>(align), size);
}

ബാഹ്യ ശൂന്യം *__CRTDECL operator new[](std::size_t size,
                                      std::align_val_t align) noexcept(false) {
  തിരികെ rpaligned_alloc(static_cast<size_t>(align), size);
}

ബാഹ്യ ശൂന്യം *__CRTDECL operator new(std::size_t size, std::align_val_t align,
                                    സ്ഥിരം std::nothrow_t &tag) noexcept {
  (ശൂന്യം)വലുപ്പം(tag);
  തിരികെ rpaligned_alloc(static_cast<size_t>(align), size);
}

ബാഹ്യ ശൂന്യം *__CRTDECL operator new[](std::size_t size, std::align_val_t align,
                                      സ്ഥിരം std::nothrow_t &tag) noexcept {
  (ശൂന്യം)വലുപ്പം(tag);
  തിരികെ rpaligned_alloc(static_cast<size_t>(align), size);
}

#endif

#endif
