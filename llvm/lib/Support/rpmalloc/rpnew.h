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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus

#ಅನ್ತರ್ಭಾವ <new>
#ಅನ್ತರ್ಭಾವ <rpmalloc.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __CRTDECL
#ವ್ಯಾಖ್ಯಾನಿಸು __CRTDECL
#ಆದರೆ_ಅಂತ್ಯ

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL ಕ್ರಿಯಾಕಾರಕ delete(ಶೂನ್ಯ *p) noexcept { rpfree(p); }

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL ಕ್ರಿಯಾಕಾರಕ delete[](ಶೂನ್ಯ *p) noexcept { rpfree(p); }

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL ಕ್ರಿಯಾಕಾರಕ new(std::size_t ಗಾತ್ರ_3) noexcept(false) {
  ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL ಕ್ರಿಯಾಕಾರಕ new[](std::size_t ಗಾತ್ರ_3) noexcept(false) {
  ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL ಕ್ರಿಯಾಕಾರಕ new(std::size_t ಗಾತ್ರ_3,
                                    ಸ್ಥಿರಾಂಕ std::nothrow_t &ಟ್ಯಾಗ್_2) noexcept {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಟ್ಯಾಗ್_2);
  ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL ಕ್ರಿಯಾಕಾರಕ new[](std::size_t ಗಾತ್ರ_3,
                                      ಸ್ಥಿರಾಂಕ std::nothrow_t &ಟ್ಯಾಗ್_2) noexcept {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಟ್ಯಾಗ್_2);
  ಹಿಂತಿರುಗಿ rpmalloc(ಗಾತ್ರ_3);
}

#ಆದರೆ (__cplusplus >= 201402L || _MSC_VER >= 1916)

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL ಕ್ರಿಯಾಕಾರಕ delete(ಶೂನ್ಯ *p, std::size_t ಗಾತ್ರ_3) noexcept {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
  rpfree(p);
}

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL ಕ್ರಿಯಾಕಾರಕ delete[](ಶೂನ್ಯ *p, std::size_t ಗಾತ್ರ_3) noexcept {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
  rpfree(p);
}

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ (__cplusplus > 201402L || defined(__cpp_aligned_new))

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL ಕ್ರಿಯಾಕಾರಕ delete(ಶೂನ್ಯ *p,
                                      std::align_val_t align) noexcept {
  (ಶೂನ್ಯ)ಗಾತ್ರ(align);
  rpfree(p);
}

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL ಕ್ರಿಯಾಕಾರಕ delete[](ಶೂನ್ಯ *p,
                                        std::align_val_t align) noexcept {
  (ಶೂನ್ಯ)ಗಾತ್ರ(align);
  rpfree(p);
}

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL ಕ್ರಿಯಾಕಾರಕ delete(ಶೂನ್ಯ *p, std::size_t ಗಾತ್ರ_3,
                                      std::align_val_t align) noexcept {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
  (ಶೂನ್ಯ)ಗಾತ್ರ(align);
  rpfree(p);
}

ಬಾಹ್ಯ ಶೂನ್ಯ __CRTDECL ಕ್ರಿಯಾಕಾರಕ delete[](ಶೂನ್ಯ *p, std::size_t ಗಾತ್ರ_3,
                                        std::align_val_t align) noexcept {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಗಾತ್ರ_3);
  (ಶೂನ್ಯ)ಗಾತ್ರ(align);
  rpfree(p);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL ಕ್ರಿಯಾಕಾರಕ new(std::size_t ಗಾತ್ರ_3,
                                    std::align_val_t align) noexcept(false) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(static_cast<size_t>(align), ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL ಕ್ರಿಯಾಕಾರಕ new[](std::size_t ಗಾತ್ರ_3,
                                      std::align_val_t align) noexcept(false) {
  ಹಿಂತಿರುಗಿ rpaligned_alloc(static_cast<size_t>(align), ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL ಕ್ರಿಯಾಕಾರಕ new(std::size_t ಗಾತ್ರ_3, std::align_val_t align,
                                    ಸ್ಥಿರಾಂಕ std::nothrow_t &ಟ್ಯಾಗ್_2) noexcept {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಟ್ಯಾಗ್_2);
  ಹಿಂತಿರುಗಿ rpaligned_alloc(static_cast<size_t>(align), ಗಾತ್ರ_3);
}

ಬಾಹ್ಯ ಶೂನ್ಯ *__CRTDECL ಕ್ರಿಯಾಕಾರಕ new[](std::size_t ಗಾತ್ರ_3, std::align_val_t align,
                                      ಸ್ಥಿರಾಂಕ std::nothrow_t &ಟ್ಯಾಗ್_2) noexcept {
  (ಶೂನ್ಯ)ಗಾತ್ರ(ಟ್ಯಾಗ್_2);
  ಹಿಂತಿರುಗಿ rpaligned_alloc(static_cast<size_t>(align), ಗಾತ್ರ_3);
}

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ
