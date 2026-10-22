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

#اگر_تعریف_شده __cplusplus

#ضمن <new>
#ضمن <rpmalloc.h>

#اگر_تعریف_نشده __CRTDECL
#تعریف_کن __CRTDECL
#پایان_اگر

خارجي فراغ __CRTDECL عملگر delete(فراغ *p) noexcept { rpfree(p); }

خارجي فراغ __CRTDECL عملگر delete[](فراغ *p) noexcept { rpfree(p); }

خارجي فراغ *__CRTDECL عملگر new(std::size_t اندازه_2) noexcept(false) {
  أرجع rpmalloc(اندازه_2);
}

خارجي فراغ *__CRTDECL عملگر new[](std::size_t اندازه_2) noexcept(false) {
  أرجع rpmalloc(اندازه_2);
}

خارجي فراغ *__CRTDECL عملگر new(std::size_t اندازه_2,
                                    ثابت std::nothrow_t &برچسب_2) noexcept {
  (فراغ)حجم(برچسب_2);
  أرجع rpmalloc(اندازه_2);
}

خارجي فراغ *__CRTDECL عملگر new[](std::size_t اندازه_2,
                                      ثابت std::nothrow_t &برچسب_2) noexcept {
  (فراغ)حجم(برچسب_2);
  أرجع rpmalloc(اندازه_2);
}

#إذا (__cplusplus >= 201402L || _MSC_VER >= 1916)

خارجي فراغ __CRTDECL عملگر delete(فراغ *p, std::size_t اندازه_2) noexcept {
  (فراغ)حجم(اندازه_2);
  rpfree(p);
}

خارجي فراغ __CRTDECL عملگر delete[](فراغ *p, std::size_t اندازه_2) noexcept {
  (فراغ)حجم(اندازه_2);
  rpfree(p);
}

#پایان_اگر

#إذا (__cplusplus > 201402L || defined(__cpp_aligned_new))

خارجي فراغ __CRTDECL عملگر delete(فراغ *p,
                                      std::align_val_t align) noexcept {
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ __CRTDECL عملگر delete[](فراغ *p,
                                        std::align_val_t align) noexcept {
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ __CRTDECL عملگر delete(فراغ *p, std::size_t اندازه_2,
                                      std::align_val_t align) noexcept {
  (فراغ)حجم(اندازه_2);
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ __CRTDECL عملگر delete[](فراغ *p, std::size_t اندازه_2,
                                        std::align_val_t align) noexcept {
  (فراغ)حجم(اندازه_2);
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ *__CRTDECL عملگر new(std::size_t اندازه_2,
                                    std::align_val_t align) noexcept(false) {
  أرجع rpaligned_alloc(static_cast<size_t>(align), اندازه_2);
}

خارجي فراغ *__CRTDECL عملگر new[](std::size_t اندازه_2,
                                      std::align_val_t align) noexcept(false) {
  أرجع rpaligned_alloc(static_cast<size_t>(align), اندازه_2);
}

خارجي فراغ *__CRTDECL عملگر new(std::size_t اندازه_2, std::align_val_t align,
                                    ثابت std::nothrow_t &برچسب_2) noexcept {
  (فراغ)حجم(برچسب_2);
  أرجع rpaligned_alloc(static_cast<size_t>(align), اندازه_2);
}

خارجي فراغ *__CRTDECL عملگر new[](std::size_t اندازه_2, std::align_val_t align,
                                      ثابت std::nothrow_t &برچسب_2) noexcept {
  (فراغ)حجم(برچسب_2);
  أرجع rpaligned_alloc(static_cast<size_t>(align), اندازه_2);
}

#پایان_اگر

#پایان_اگر
