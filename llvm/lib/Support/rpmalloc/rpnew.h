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

#جے_تعریف_شدہ __cplusplus

#ضمن <new>
#ضمن <rpmalloc.h>

#جے_بنا_تعریف __CRTDECL
#تعریف_کرو __CRTDECL
#جے_انت

خارجي فراغ __CRTDECL عامل delete(فراغ *p) noexcept { rpfree(p); }

خارجي فراغ __CRTDECL عامل delete[](فراغ *p) noexcept { rpfree(p); }

خارجي فراغ *__CRTDECL عامل new(std::size_t ناپ_2) noexcept(false) {
  أرجع rpmalloc(ناپ_2);
}

خارجي فراغ *__CRTDECL عامل new[](std::size_t ناپ_2) noexcept(false) {
  أرجع rpmalloc(ناپ_2);
}

خارجي فراغ *__CRTDECL عامل new(std::size_t ناپ_2,
                                    ثابت std::nothrow_t &ٹیگ_2) noexcept {
  (فراغ)حجم(ٹیگ_2);
  أرجع rpmalloc(ناپ_2);
}

خارجي فراغ *__CRTDECL عامل new[](std::size_t ناپ_2,
                                      ثابت std::nothrow_t &ٹیگ_2) noexcept {
  (فراغ)حجم(ٹیگ_2);
  أرجع rpmalloc(ناپ_2);
}

#إذا (__cplusplus >= 201402L || _MSC_VER >= 1916)

خارجي فراغ __CRTDECL عامل delete(فراغ *p, std::size_t ناپ_2) noexcept {
  (فراغ)حجم(ناپ_2);
  rpfree(p);
}

خارجي فراغ __CRTDECL عامل delete[](فراغ *p, std::size_t ناپ_2) noexcept {
  (فراغ)حجم(ناپ_2);
  rpfree(p);
}

#جے_انت

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

خارجي فراغ __CRTDECL عامل delete(فراغ *p, std::size_t ناپ_2,
                                      std::align_val_t align) noexcept {
  (فراغ)حجم(ناپ_2);
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ __CRTDECL عامل delete[](فراغ *p, std::size_t ناپ_2,
                                        std::align_val_t align) noexcept {
  (فراغ)حجم(ناپ_2);
  (فراغ)حجم(align);
  rpfree(p);
}

خارجي فراغ *__CRTDECL عامل new(std::size_t ناپ_2,
                                    std::align_val_t align) noexcept(false) {
  أرجع rpaligned_alloc(static_cast<size_t>(align), ناپ_2);
}

خارجي فراغ *__CRTDECL عامل new[](std::size_t ناپ_2,
                                      std::align_val_t align) noexcept(false) {
  أرجع rpaligned_alloc(static_cast<size_t>(align), ناپ_2);
}

خارجي فراغ *__CRTDECL عامل new(std::size_t ناپ_2, std::align_val_t align,
                                    ثابت std::nothrow_t &ٹیگ_2) noexcept {
  (فراغ)حجم(ٹیگ_2);
  أرجع rpaligned_alloc(static_cast<size_t>(align), ناپ_2);
}

خارجي فراغ *__CRTDECL عامل new[](std::size_t ناپ_2, std::align_val_t align,
                                      ثابت std::nothrow_t &ٹیگ_2) noexcept {
  (فراغ)حجم(ٹیگ_2);
  أرجع rpaligned_alloc(static_cast<size_t>(align), ناپ_2);
}

#جے_انت

#جے_انت
