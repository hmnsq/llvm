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

#నిర్వచితమైతే __cplusplus

#అన్తర్భావ <new>
#అన్తర్భావ <rpmalloc.h>

#నిర్వచితం_కాకపోతే __CRTDECL
#నిర్వచించు __CRTDECL
#అయితే_ముగింపు

బాహ్య శూన్య __CRTDECL క్రియాకారకం delete(శూన్య *p) noexcept { rpfree(p); }

బాహ్య శూన్య __CRTDECL క్రియాకారకం delete[](శూన్య *p) noexcept { rpfree(p); }

బాహ్య శూన్య *__CRTDECL క్రియాకారకం new(std::size_t కొలత_2) noexcept(false) {
  తిరిగి rpmalloc(కొలత_2);
}

బాహ్య శూన్య *__CRTDECL క్రియాకారకం new[](std::size_t కొలత_2) noexcept(false) {
  తిరిగి rpmalloc(కొలత_2);
}

బాహ్య శూన్య *__CRTDECL క్రియాకారకం new(std::size_t కొలత_2,
                                    స్థిరరాశి std::nothrow_t &ట్యాగ్_2) noexcept {
  (శూన్య)పరిమాణం(ట్యాగ్_2);
  తిరిగి rpmalloc(కొలత_2);
}

బాహ్య శూన్య *__CRTDECL క్రియాకారకం new[](std::size_t కొలత_2,
                                      స్థిరరాశి std::nothrow_t &ట్యాగ్_2) noexcept {
  (శూన్య)పరిమాణం(ట్యాగ్_2);
  తిరిగి rpmalloc(కొలత_2);
}

#ఒకవేళ (__cplusplus >= 201402L || _MSC_VER >= 1916)

బాహ్య శూన్య __CRTDECL క్రియాకారకం delete(శూన్య *p, std::size_t కొలత_2) noexcept {
  (శూన్య)పరిమాణం(కొలత_2);
  rpfree(p);
}

బాహ్య శూన్య __CRTDECL క్రియాకారకం delete[](శూన్య *p, std::size_t కొలత_2) noexcept {
  (శూన్య)పరిమాణం(కొలత_2);
  rpfree(p);
}

#అయితే_ముగింపు

#ఒకవేళ (__cplusplus > 201402L || defined(__cpp_aligned_new))

బాహ్య శూన్య __CRTDECL క్రియాకారకం delete(శూన్య *p,
                                      std::align_val_t align) noexcept {
  (శూన్య)పరిమాణం(align);
  rpfree(p);
}

బాహ్య శూన్య __CRTDECL క్రియాకారకం delete[](శూన్య *p,
                                        std::align_val_t align) noexcept {
  (శూన్య)పరిమాణం(align);
  rpfree(p);
}

బాహ్య శూన్య __CRTDECL క్రియాకారకం delete(శూన్య *p, std::size_t కొలత_2,
                                      std::align_val_t align) noexcept {
  (శూన్య)పరిమాణం(కొలత_2);
  (శూన్య)పరిమాణం(align);
  rpfree(p);
}

బాహ్య శూన్య __CRTDECL క్రియాకారకం delete[](శూన్య *p, std::size_t కొలత_2,
                                        std::align_val_t align) noexcept {
  (శూన్య)పరిమాణం(కొలత_2);
  (శూన్య)పరిమాణం(align);
  rpfree(p);
}

బాహ్య శూన్య *__CRTDECL క్రియాకారకం new(std::size_t కొలత_2,
                                    std::align_val_t align) noexcept(false) {
  తిరిగి rpaligned_alloc(static_cast<size_t>(align), కొలత_2);
}

బాహ్య శూన్య *__CRTDECL క్రియాకారకం new[](std::size_t కొలత_2,
                                      std::align_val_t align) noexcept(false) {
  తిరిగి rpaligned_alloc(static_cast<size_t>(align), కొలత_2);
}

బాహ్య శూన్య *__CRTDECL క్రియాకారకం new(std::size_t కొలత_2, std::align_val_t align,
                                    స్థిరరాశి std::nothrow_t &ట్యాగ్_2) noexcept {
  (శూన్య)పరిమాణం(ట్యాగ్_2);
  తిరిగి rpaligned_alloc(static_cast<size_t>(align), కొలత_2);
}

బాహ్య శూన్య *__CRTDECL క్రియాకారకం new[](std::size_t కొలత_2, std::align_val_t align,
                                      స్థిరరాశి std::nothrow_t &ట్యాగ్_2) noexcept {
  (శూన్య)పరిమాణం(ట్యాగ్_2);
  తిరిగి rpaligned_alloc(static_cast<size_t>(align), కొలత_2);
}

#అయితే_ముగింపు

#అయితే_ముగింపు
