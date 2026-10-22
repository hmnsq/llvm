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

#شامل <new>
#شامل <rpmalloc.h>

#ifndef __CRTDECL
#define __CRTDECL
#endif

بیرونی تهی __CRTDECL operator delete(تهی *p) noexcept { rpfree(p); }

بیرونی تهی __CRTDECL operator delete[](تهی *p) noexcept { rpfree(p); }

بیرونی تهی *__CRTDECL operator new(std::size_t size) noexcept(false) {
  بازگشت rpmalloc(size);
}

بیرونی تهی *__CRTDECL operator new[](std::size_t size) noexcept(false) {
  بازگشت rpmalloc(size);
}

بیرونی تهی *__CRTDECL operator new(std::size_t size,
                                    ثابت std::nothrow_t &tag) noexcept {
  (تهی)اندازه(tag);
  بازگشت rpmalloc(size);
}

بیرونی تهی *__CRTDECL operator new[](std::size_t size,
                                      ثابت std::nothrow_t &tag) noexcept {
  (تهی)اندازه(tag);
  بازگشت rpmalloc(size);
}

#اگر (__cplusplus >= 201402L || _MSC_VER >= 1916)

بیرونی تهی __CRTDECL operator delete(تهی *p, std::size_t size) noexcept {
  (تهی)اندازه(size);
  rpfree(p);
}

بیرونی تهی __CRTDECL operator delete[](تهی *p, std::size_t size) noexcept {
  (تهی)اندازه(size);
  rpfree(p);
}

#endif

#اگر (__cplusplus > 201402L || defined(__cpp_aligned_new))

بیرونی تهی __CRTDECL operator delete(تهی *p,
                                      std::align_val_t align) noexcept {
  (تهی)اندازه(align);
  rpfree(p);
}

بیرونی تهی __CRTDECL operator delete[](تهی *p,
                                        std::align_val_t align) noexcept {
  (تهی)اندازه(align);
  rpfree(p);
}

بیرونی تهی __CRTDECL operator delete(تهی *p, std::size_t size,
                                      std::align_val_t align) noexcept {
  (تهی)اندازه(size);
  (تهی)اندازه(align);
  rpfree(p);
}

بیرونی تهی __CRTDECL operator delete[](تهی *p, std::size_t size,
                                        std::align_val_t align) noexcept {
  (تهی)اندازه(size);
  (تهی)اندازه(align);
  rpfree(p);
}

بیرونی تهی *__CRTDECL operator new(std::size_t size,
                                    std::align_val_t align) noexcept(false) {
  بازگشت rpaligned_alloc(static_cast<size_t>(align), size);
}

بیرونی تهی *__CRTDECL operator new[](std::size_t size,
                                      std::align_val_t align) noexcept(false) {
  بازگشت rpaligned_alloc(static_cast<size_t>(align), size);
}

بیرونی تهی *__CRTDECL operator new(std::size_t size, std::align_val_t align,
                                    ثابت std::nothrow_t &tag) noexcept {
  (تهی)اندازه(tag);
  بازگشت rpaligned_alloc(static_cast<size_t>(align), size);
}

بیرونی تهی *__CRTDECL operator new[](std::size_t size, std::align_val_t align,
                                      ثابت std::nothrow_t &tag) noexcept {
  (تهی)اندازه(tag);
  بازگشت rpaligned_alloc(static_cast<size_t>(align), size);
}

#endif

#endif
