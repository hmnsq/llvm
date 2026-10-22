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

#اگر_تعریف_شدہ __cplusplus

#شامل <new>
#شامل <rpmalloc.h>

#اگر_غیر_تعریف_شدہ __CRTDECL
#تعریف_کرو __CRTDECL
#اگر_ختم

خارجی خالی __CRTDECL عامل delete(خالی *p) noexcept { rpfree(p); }

خارجی خالی __CRTDECL عامل delete[](خالی *p) noexcept { rpfree(p); }

خارجی خالی *__CRTDECL عامل new(std::size_t سائز_2) noexcept(false) {
  واپس rpmalloc(سائز_2);
}

خارجی خالی *__CRTDECL عامل new[](std::size_t سائز_2) noexcept(false) {
  واپس rpmalloc(سائز_2);
}

خارجی خالی *__CRTDECL عامل new(std::size_t سائز_2,
                                    ثابت std::nothrow_t &لیبل_2) noexcept {
  (خالی)حجم(لیبل_2);
  واپس rpmalloc(سائز_2);
}

خارجی خالی *__CRTDECL عامل new[](std::size_t سائز_2,
                                      ثابت std::nothrow_t &لیبل_2) noexcept {
  (خالی)حجم(لیبل_2);
  واپس rpmalloc(سائز_2);
}

#اگر (__cplusplus >= 201402L || _MSC_VER >= 1916)

خارجی خالی __CRTDECL عامل delete(خالی *p, std::size_t سائز_2) noexcept {
  (خالی)حجم(سائز_2);
  rpfree(p);
}

خارجی خالی __CRTDECL عامل delete[](خالی *p, std::size_t سائز_2) noexcept {
  (خالی)حجم(سائز_2);
  rpfree(p);
}

#اگر_ختم

#اگر (__cplusplus > 201402L || defined(__cpp_aligned_new))

خارجی خالی __CRTDECL عامل delete(خالی *p,
                                      std::align_val_t align) noexcept {
  (خالی)حجم(align);
  rpfree(p);
}

خارجی خالی __CRTDECL عامل delete[](خالی *p,
                                        std::align_val_t align) noexcept {
  (خالی)حجم(align);
  rpfree(p);
}

خارجی خالی __CRTDECL عامل delete(خالی *p, std::size_t سائز_2,
                                      std::align_val_t align) noexcept {
  (خالی)حجم(سائز_2);
  (خالی)حجم(align);
  rpfree(p);
}

خارجی خالی __CRTDECL عامل delete[](خالی *p, std::size_t سائز_2,
                                        std::align_val_t align) noexcept {
  (خالی)حجم(سائز_2);
  (خالی)حجم(align);
  rpfree(p);
}

خارجی خالی *__CRTDECL عامل new(std::size_t سائز_2,
                                    std::align_val_t align) noexcept(false) {
  واپس rpaligned_alloc(static_cast<size_t>(align), سائز_2);
}

خارجی خالی *__CRTDECL عامل new[](std::size_t سائز_2,
                                      std::align_val_t align) noexcept(false) {
  واپس rpaligned_alloc(static_cast<size_t>(align), سائز_2);
}

خارجی خالی *__CRTDECL عامل new(std::size_t سائز_2, std::align_val_t align,
                                    ثابت std::nothrow_t &لیبل_2) noexcept {
  (خالی)حجم(لیبل_2);
  واپس rpaligned_alloc(static_cast<size_t>(align), سائز_2);
}

خارجی خالی *__CRTDECL عامل new[](std::size_t سائز_2, std::align_val_t align,
                                      ثابت std::nothrow_t &لیبل_2) noexcept {
  (خالی)حجم(لیبل_2);
  واپس rpaligned_alloc(static_cast<size_t>(align), سائز_2);
}

#اگر_ختم

#اگر_ختم
