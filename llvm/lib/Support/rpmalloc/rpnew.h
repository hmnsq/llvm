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

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus

#ਅਨ੍ਤਰ੍ਭਾਵ <new>
#ਅਨ੍ਤਰ੍ਭਾਵ <rpmalloc.h>

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __CRTDECL
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __CRTDECL
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ ਖਾਲੀ __CRTDECL ਸੰਚਾਲਕ delete(ਖਾਲੀ *p) noexcept { rpfree(p); }

ਬਾਹਰੀ ਖਾਲੀ __CRTDECL ਸੰਚਾਲਕ delete[](ਖਾਲੀ *p) noexcept { rpfree(p); }

ਬਾਹਰੀ ਖਾਲੀ *__CRTDECL ਸੰਚਾਲਕ new(std::size_t ਮਾਪ_2) noexcept(false) {
  ਵਾਪਸ rpmalloc(ਮਾਪ_2);
}

ਬਾਹਰੀ ਖਾਲੀ *__CRTDECL ਸੰਚਾਲਕ new[](std::size_t ਮਾਪ_2) noexcept(false) {
  ਵਾਪਸ rpmalloc(ਮਾਪ_2);
}

ਬਾਹਰੀ ਖਾਲੀ *__CRTDECL ਸੰਚਾਲਕ new(std::size_t ਮਾਪ_2,
                                    ਅਚਲ std::nothrow_t &ਟੈਗ_2) noexcept {
  (ਖਾਲੀ)ਆਕਾਰ(ਟੈਗ_2);
  ਵਾਪਸ rpmalloc(ਮਾਪ_2);
}

ਬਾਹਰੀ ਖਾਲੀ *__CRTDECL ਸੰਚਾਲਕ new[](std::size_t ਮਾਪ_2,
                                      ਅਚਲ std::nothrow_t &ਟੈਗ_2) noexcept {
  (ਖਾਲੀ)ਆਕਾਰ(ਟੈਗ_2);
  ਵਾਪਸ rpmalloc(ਮਾਪ_2);
}

#ਜੇ (__cplusplus >= 201402L || _MSC_VER >= 1916)

ਬਾਹਰੀ ਖਾਲੀ __CRTDECL ਸੰਚਾਲਕ delete(ਖਾਲੀ *p, std::size_t ਮਾਪ_2) noexcept {
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
  rpfree(p);
}

ਬਾਹਰੀ ਖਾਲੀ __CRTDECL ਸੰਚਾਲਕ delete[](ਖਾਲੀ *p, std::size_t ਮਾਪ_2) noexcept {
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
  rpfree(p);
}

#ਜੇ_ਅੰਤ

#ਜੇ (__cplusplus > 201402L || defined(__cpp_aligned_new))

ਬਾਹਰੀ ਖਾਲੀ __CRTDECL ਸੰਚਾਲਕ delete(ਖਾਲੀ *p,
                                      std::align_val_t align) noexcept {
  (ਖਾਲੀ)ਆਕਾਰ(align);
  rpfree(p);
}

ਬਾਹਰੀ ਖਾਲੀ __CRTDECL ਸੰਚਾਲਕ delete[](ਖਾਲੀ *p,
                                        std::align_val_t align) noexcept {
  (ਖਾਲੀ)ਆਕਾਰ(align);
  rpfree(p);
}

ਬਾਹਰੀ ਖਾਲੀ __CRTDECL ਸੰਚਾਲਕ delete(ਖਾਲੀ *p, std::size_t ਮਾਪ_2,
                                      std::align_val_t align) noexcept {
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
  (ਖਾਲੀ)ਆਕਾਰ(align);
  rpfree(p);
}

ਬਾਹਰੀ ਖਾਲੀ __CRTDECL ਸੰਚਾਲਕ delete[](ਖਾਲੀ *p, std::size_t ਮਾਪ_2,
                                        std::align_val_t align) noexcept {
  (ਖਾਲੀ)ਆਕਾਰ(ਮਾਪ_2);
  (ਖਾਲੀ)ਆਕਾਰ(align);
  rpfree(p);
}

ਬਾਹਰੀ ਖਾਲੀ *__CRTDECL ਸੰਚਾਲਕ new(std::size_t ਮਾਪ_2,
                                    std::align_val_t align) noexcept(false) {
  ਵਾਪਸ rpaligned_alloc(static_cast<size_t>(align), ਮਾਪ_2);
}

ਬਾਹਰੀ ਖਾਲੀ *__CRTDECL ਸੰਚਾਲਕ new[](std::size_t ਮਾਪ_2,
                                      std::align_val_t align) noexcept(false) {
  ਵਾਪਸ rpaligned_alloc(static_cast<size_t>(align), ਮਾਪ_2);
}

ਬਾਹਰੀ ਖਾਲੀ *__CRTDECL ਸੰਚਾਲਕ new(std::size_t ਮਾਪ_2, std::align_val_t align,
                                    ਅਚਲ std::nothrow_t &ਟੈਗ_2) noexcept {
  (ਖਾਲੀ)ਆਕਾਰ(ਟੈਗ_2);
  ਵਾਪਸ rpaligned_alloc(static_cast<size_t>(align), ਮਾਪ_2);
}

ਬਾਹਰੀ ਖਾਲੀ *__CRTDECL ਸੰਚਾਲਕ new[](std::size_t ਮਾਪ_2, std::align_val_t align,
                                      ਅਚਲ std::nothrow_t &ਟੈਗ_2) noexcept {
  (ਖਾਲੀ)ਆਕਾਰ(ਟੈਗ_2);
  ਵਾਪਸ rpaligned_alloc(static_cast<size_t>(align), ਮਾਪ_2);
}

#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ
