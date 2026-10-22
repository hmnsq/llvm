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

#அந்தர்பாவ <new>
#அந்தர்பாவ <rpmalloc.h>

#ifndef __CRTDECL
#define __CRTDECL
#endif

வெளி வெறுமை __CRTDECL operator delete(வெறுமை *p) noexcept { rpfree(p); }

வெளி வெறுமை __CRTDECL operator delete[](வெறுமை *p) noexcept { rpfree(p); }

வெளி வெறுமை *__CRTDECL operator new(std::size_t size) noexcept(false) {
  திரும்பு rpmalloc(size);
}

வெளி வெறுமை *__CRTDECL operator new[](std::size_t size) noexcept(false) {
  திரும்பு rpmalloc(size);
}

வெளி வெறுமை *__CRTDECL operator new(std::size_t size,
                                    மாறிலி std::nothrow_t &tag) noexcept {
  (வெறுமை)அளவு(tag);
  திரும்பு rpmalloc(size);
}

வெளி வெறுமை *__CRTDECL operator new[](std::size_t size,
                                      மாறிலி std::nothrow_t &tag) noexcept {
  (வெறுமை)அளவு(tag);
  திரும்பு rpmalloc(size);
}

#எனில் (__cplusplus >= 201402L || _MSC_VER >= 1916)

வெளி வெறுமை __CRTDECL operator delete(வெறுமை *p, std::size_t size) noexcept {
  (வெறுமை)அளவு(size);
  rpfree(p);
}

வெளி வெறுமை __CRTDECL operator delete[](வெறுமை *p, std::size_t size) noexcept {
  (வெறுமை)அளவு(size);
  rpfree(p);
}

#endif

#எனில் (__cplusplus > 201402L || defined(__cpp_aligned_new))

வெளி வெறுமை __CRTDECL operator delete(வெறுமை *p,
                                      std::align_val_t align) noexcept {
  (வெறுமை)அளவு(align);
  rpfree(p);
}

வெளி வெறுமை __CRTDECL operator delete[](வெறுமை *p,
                                        std::align_val_t align) noexcept {
  (வெறுமை)அளவு(align);
  rpfree(p);
}

வெளி வெறுமை __CRTDECL operator delete(வெறுமை *p, std::size_t size,
                                      std::align_val_t align) noexcept {
  (வெறுமை)அளவு(size);
  (வெறுமை)அளவு(align);
  rpfree(p);
}

வெளி வெறுமை __CRTDECL operator delete[](வெறுமை *p, std::size_t size,
                                        std::align_val_t align) noexcept {
  (வெறுமை)அளவு(size);
  (வெறுமை)அளவு(align);
  rpfree(p);
}

வெளி வெறுமை *__CRTDECL operator new(std::size_t size,
                                    std::align_val_t align) noexcept(false) {
  திரும்பு rpaligned_alloc(static_cast<size_t>(align), size);
}

வெளி வெறுமை *__CRTDECL operator new[](std::size_t size,
                                      std::align_val_t align) noexcept(false) {
  திரும்பு rpaligned_alloc(static_cast<size_t>(align), size);
}

வெளி வெறுமை *__CRTDECL operator new(std::size_t size, std::align_val_t align,
                                    மாறிலி std::nothrow_t &tag) noexcept {
  (வெறுமை)அளவு(tag);
  திரும்பு rpaligned_alloc(static_cast<size_t>(align), size);
}

வெளி வெறுமை *__CRTDECL operator new[](std::size_t size, std::align_val_t align,
                                      மாறிலி std::nothrow_t &tag) noexcept {
  (வெறுமை)அளவு(tag);
  திரும்பு rpaligned_alloc(static_cast<size_t>(align), size);
}

#endif

#endif
