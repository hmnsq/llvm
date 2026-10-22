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
llvm::allocate_میانگیر(size_t اندازه, size_t Alignment) {
  فراغ *نتیجه = ::عملگر جديد(اندازه,
#اگر_تعریف_شده __cpp_aligned_new
                                std::align_val_t(Alignment),
#پایان_اگر
                                std::nothrow);
  إذا (نتیجه == اشاره_تهی) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  أرجع نتیجه;
}

فراغ llvm::deallocate_میانگیر(فراغ *Ptr, size_t اندازه, size_t Alignment) {
  ::عملگر حذف(Ptr
#اگر_تعریف_شده __cpp_sized_deallocation
                    ,
                    اندازه
#پایان_اگر
#اگر_تعریف_شده __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#پایان_اگر
  );
}
