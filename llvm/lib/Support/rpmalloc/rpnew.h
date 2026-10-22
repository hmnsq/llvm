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

#جيڪڏهن_وصف_ٿيل __cplusplus

#ضمن <new>
#ضمن <rpmalloc.h>

#جيڪڏهن_وصف_نه_ٿيل __CRTDECL
#وصف_ڪريو __CRTDECL
#جيڪڏهن_پورو

خارجي فراغ __CRTDECL عامل delete(فراغ *p) noexcept { rpfree(p); }

خارجي فراغ __CRTDECL عامل delete[](فراغ *p) noexcept { rpfree(p); }

خارجي فراغ *__CRTDECL عامل new(std::size_t سائيز_2) noexcept(false) {
  أرجع rpmalloc(سائيز_2);
}

خارجي فراغ *__CRTDECL عامل new[](std::size_t سائيز_2) noexcept(false) {
  أرجع rpmalloc(سائيز_2);
}

خارجي فراغ *__CRTDECL عامل new(std::size_t سائيز_2,
                                    ثابت std::nothrow_t &ٽيگ_2) noexcept {
  (فراغ)حجم(ٽيگ_2);
  أرجع rpmalloc(سائيز_2);
}

خارجي فراغ *__CRTDECL عامل new[](std::size_t سائيز_2,
                                      ثابت std::nothrow_t &ٽيگ_2) noexcept {
  (فراغ)حجم(ٽيگ_2);
  أرجع rpmalloc(سائيز_2);
}

#إذا (__cplusplus >= 201402L || _MSC_VER >= 1916)

خارجي فراغ __CRTDECL عامل delete(فراغ *p, std::size_t سائيز_2) noexcept {
  (فراغ)حجم(سائيز_2);
  rpfree(p);
}

خارجي فراغ __CRTDECL عامل delete[](فراغ *p, std::size_t سائيز_2) noexcept {
  (فراغ)حجم(سائيز_2);
  rpfree(p);
}

#جيڪڏهن_پورو

#إذا (__cplusplus > 201402L || defined(__cpp_aligned_new))

خارجي فراغ __CRTDECL عامل delete(فراغ *p,
                                      std::align_val_t align) noexcept {
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ __CRTDECL عامل delete[](فراغ *p,
                                        std::align_val_t align) noexcept {
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ __CRTDECL عامل delete(فراغ *p, std::size_t سائيز_2,
                                      std::align_val_t align) noexcept {
  (فراغ)حجم(سائيز_2);
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ __CRTDECL عامل delete[](فراغ *p, std::size_t سائيز_2,
                                        std::align_val_t align) noexcept {
  (فراغ)حجم(سائيز_2);
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ *__CRTDECL عامل new(std::size_t سائيز_2,
                                    std::align_val_t align) noexcept(false) {
  أرجع rpaligned_alloc(static_cast<size_t>(align), سائيز_2);
}

خارجي فراغ *__CRTDECL عامل new[](std::size_t سائيز_2,
                                      std::align_val_t align) noexcept(false) {
  أرجع rpaligned_alloc(static_cast<size_t>(align), سائيز_2);
}

خارجي فراغ *__CRTDECL عامل new(std::size_t سائيز_2, std::align_val_t align,
                                    ثابت std::nothrow_t &ٽيگ_2) noexcept {
  (فراغ)حجم(ٽيگ_2);
  أرجع rpaligned_alloc(static_cast<size_t>(align), سائيز_2);
}

خارجي فراغ *__CRTDECL عامل new[](std::size_t سائيز_2, std::align_val_t align,
                                      ثابت std::nothrow_t &ٽيگ_2) noexcept {
  (فراغ)حجم(ٽيگ_2);
  أرجع rpaligned_alloc(static_cast<size_t>(align), سائيز_2);
}

#جيڪڏهن_پورو

#جيڪڏهن_پورو
