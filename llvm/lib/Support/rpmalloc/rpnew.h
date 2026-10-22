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

#ضمن <new>
#ضمن <rpmalloc.h>

#ifndef __CRTDECL
#define __CRTDECL
#endif

خارجي فراغ __CRTDECL operator delete(فراغ *p) noexcept { rpfree(p); }

خارجي فراغ __CRTDECL operator delete[](فراغ *p) noexcept { rpfree(p); }

خارجي فراغ *__CRTDECL operator new(std::size_t size) noexcept(false) {
  إرجاع rpmalloc(size);
}

خارجي فراغ *__CRTDECL operator new[](std::size_t size) noexcept(false) {
  إرجاع rpmalloc(size);
}

خارجي فراغ *__CRTDECL operator new(std::size_t size,
                                    ثابت std::nothrow_t &tag) noexcept {
  (فراغ)حجم(tag);
  إرجاع rpmalloc(size);
}

خارجي فراغ *__CRTDECL operator new[](std::size_t size,
                                      ثابت std::nothrow_t &tag) noexcept {
  (فراغ)حجم(tag);
  إرجاع rpmalloc(size);
}

#إذا (__cplusplus >= 201402L || _MSC_VER >= 1916)

خارجي فراغ __CRTDECL operator delete(فراغ *p, std::size_t size) noexcept {
  (فراغ)حجم(size);
  rpfree(p);
}

خارجي فراغ __CRTDECL operator delete[](فراغ *p, std::size_t size) noexcept {
  (فراغ)حجم(size);
  rpfree(p);
}

#endif

#إذا (__cplusplus > 201402L || defined(__cpp_aligned_new))

خارجي فراغ __CRTDECL operator delete(فراغ *p,
                                      std::align_val_t align) noexcept {
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ __CRTDECL operator delete[](فراغ *p,
                                        std::align_val_t align) noexcept {
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ __CRTDECL operator delete(فراغ *p, std::size_t size,
                                      std::align_val_t align) noexcept {
  (فراغ)حجم(size);
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ __CRTDECL operator delete[](فراغ *p, std::size_t size,
                                        std::align_val_t align) noexcept {
  (فراغ)حجم(size);
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ *__CRTDECL operator new(std::size_t size,
                                    std::align_val_t align) noexcept(false) {
  إرجاع rpaligned_alloc(static_cast<size_t>(align), size);
}

خارجي فراغ *__CRTDECL operator new[](std::size_t size,
                                      std::align_val_t align) noexcept(false) {
  إرجاع rpaligned_alloc(static_cast<size_t>(align), size);
}

خارجي فراغ *__CRTDECL operator new(std::size_t size, std::align_val_t align,
                                    ثابت std::nothrow_t &tag) noexcept {
  (فراغ)حجم(tag);
  إرجاع rpaligned_alloc(static_cast<size_t>(align), size);
}

خارجي فراغ *__CRTDECL operator new[](std::size_t size, std::align_val_t align,
                                      ثابت std::nothrow_t &tag) noexcept {
  (فراغ)حجم(tag);
  إرجاع rpaligned_alloc(static_cast<size_t>(align), size);
}

#endif

#endif
