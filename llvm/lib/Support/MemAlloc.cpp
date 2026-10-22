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
llvm::allocate_بفر(size_t ناپ, size_t Alignment) {
  فراغ *نتیجہ = ::عامل جديد(ناپ,
#جے_تعریف_شدہ __cpp_aligned_new
                                std::align_val_t(Alignment),
#جے_انت
                                std::nothrow);
  إذا (نتیجہ == خالی_سوچک) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  أرجع نتیجہ;
}

فراغ llvm::deallocate_بفر(فراغ *Ptr, size_t ناپ, size_t Alignment) {
  ::عامل مٹاؤ(Ptr
#جے_تعریف_شدہ __cpp_sized_deallocation
                    ,
                    ناپ
#جے_انت
#جے_تعریف_شدہ __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#جے_انت
  );
}
