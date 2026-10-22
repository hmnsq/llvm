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

#অন্তর্ভাৱ <new>
#অন্তর্ভাৱ <rpmalloc.h>

#ifndef __CRTDECL
#define __CRTDECL
#endif

বাহ্য শূন্য __CRTDECL operator delete(শূন্য *p) noexcept { rpfree(p); }

বাহ্য শূন্য __CRTDECL operator delete[](শূন্য *p) noexcept { rpfree(p); }

বাহ্য শূন্য *__CRTDECL operator new(std::size_t size) noexcept(false) {
  ঘূৰাই rpmalloc(size);
}

বাহ্য শূন্য *__CRTDECL operator new[](std::size_t size) noexcept(false) {
  ঘূৰাই rpmalloc(size);
}

বাহ্য শূন্য *__CRTDECL operator new(std::size_t size,
                                    ধ্রুৱ std::nothrow_t &tag) noexcept {
  (শূন্য)পরিমাণ(tag);
  ঘূৰাই rpmalloc(size);
}

বাহ্য শূন্য *__CRTDECL operator new[](std::size_t size,
                                      ধ্রুৱ std::nothrow_t &tag) noexcept {
  (শূন্য)পরিমাণ(tag);
  ঘূৰাই rpmalloc(size);
}

#যদি (__cplusplus >= 201402L || _MSC_VER >= 1916)

বাহ্য শূন্য __CRTDECL operator delete(শূন্য *p, std::size_t size) noexcept {
  (শূন্য)পরিমাণ(size);
  rpfree(p);
}

বাহ্য শূন্য __CRTDECL operator delete[](শূন্য *p, std::size_t size) noexcept {
  (শূন্য)পরিমাণ(size);
  rpfree(p);
}

#endif

#যদি (__cplusplus > 201402L || defined(__cpp_aligned_new))

বাহ্য শূন্য __CRTDECL operator delete(শূন্য *p,
                                      std::align_val_t align) noexcept {
  (শূন্য)পরিমাণ(align);
  rpfree(p);
}

বাহ্য শূন্য __CRTDECL operator delete[](শূন্য *p,
                                        std::align_val_t align) noexcept {
  (শূন্য)পরিমাণ(align);
  rpfree(p);
}

বাহ্য শূন্য __CRTDECL operator delete(শূন্য *p, std::size_t size,
                                      std::align_val_t align) noexcept {
  (শূন্য)পরিমাণ(size);
  (শূন্য)পরিমাণ(align);
  rpfree(p);
}

বাহ্য শূন্য __CRTDECL operator delete[](শূন্য *p, std::size_t size,
                                        std::align_val_t align) noexcept {
  (শূন্য)পরিমাণ(size);
  (শূন্য)পরিমাণ(align);
  rpfree(p);
}

বাহ্য শূন্য *__CRTDECL operator new(std::size_t size,
                                    std::align_val_t align) noexcept(false) {
  ঘূৰাই rpaligned_alloc(static_cast<size_t>(align), size);
}

বাহ্য শূন্য *__CRTDECL operator new[](std::size_t size,
                                      std::align_val_t align) noexcept(false) {
  ঘূৰাই rpaligned_alloc(static_cast<size_t>(align), size);
}

বাহ্য শূন্য *__CRTDECL operator new(std::size_t size, std::align_val_t align,
                                    ধ্রুৱ std::nothrow_t &tag) noexcept {
  (শূন্য)পরিমাণ(tag);
  ঘূৰাই rpaligned_alloc(static_cast<size_t>(align), size);
}

বাহ্য শূন্য *__CRTDECL operator new[](std::size_t size, std::align_val_t align,
                                      ধ্রুৱ std::nothrow_t &tag) noexcept {
  (শূন্য)পরিমাণ(tag);
  ঘূৰাই rpaligned_alloc(static_cast<size_t>(align), size);
}

#endif

#endif
