//===- MemAlloc.cpp - Memory allocation functions -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/MemAlloc.h"
#ضمن <new>

// These are out of line to have __cpp_aligned_new not affect ABI.

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS فراغ *
llvm::allocate_بفر(size_t سائيز, size_t Alignment) {
  فراغ *نتيجو = ::عامل جديد(سائيز,
#جيڪڏهن_وصف_ٿيل __cpp_aligned_new
                                std::align_val_t(Alignment),
#جيڪڏهن_پورو
                                std::nothrow);
  إذا (نتيجو == خالي_اشارو) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  أرجع نتيجو;
}

فراغ llvm::deallocate_بفر(فراغ *Ptr, size_t سائيز, size_t Alignment) {
  ::عامل ميٽيو(Ptr
#جيڪڏهن_وصف_ٿيل __cpp_sized_deallocation
                    ,
                    سائيز
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#جيڪڏهن_پورو
  );
}
