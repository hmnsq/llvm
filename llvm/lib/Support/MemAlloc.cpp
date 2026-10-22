//===- MemAlloc.cpp - Memory allocation functions -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/MemAlloc.h"
#شامل <new>

// These are out of line to have __cpp_aligned_new not affect ABI.

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS خالی *
llvm::allocate_بفر(size_t سائز, size_t Alignment) {
  خالی *نتیجہ = ::عامل نیا(سائز,
#اگر_تعریف_شدہ __cpp_aligned_new
                                std::align_val_t(Alignment),
#اگر_ختم
                                std::nothrow);
  اگر (نتیجہ == خالی_اشاریہ) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  واپس نتیجہ;
}

خالی llvm::deallocate_بفر(خالی *Ptr, size_t سائز, size_t Alignment) {
  ::عامل حذف(Ptr
#اگر_تعریف_شدہ __cpp_sized_deallocation
                    ,
                    سائز
#اگر_ختم
#اگر_تعریف_شدہ __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#اگر_ختم
  );
}
