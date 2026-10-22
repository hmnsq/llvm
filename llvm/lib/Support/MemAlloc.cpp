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
llvm::allocate_بفر(size_t اندازه, size_t Alignment) {
  فراغ *پایله = ::چلوونکی جديد(اندازه,
#که_تعریف_شوی __cpp_aligned_new
                                std::align_val_t(Alignment),
#د_که_پای
                                std::nothrow);
  إذا (پایله == تش_اشاره) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  أرجع پایله;
}

فراغ llvm::deallocate_بفر(فراغ *Ptr, size_t اندازه, size_t Alignment) {
  ::چلوونکی ړنګول(Ptr
#که_تعریف_شوی __cpp_sized_deallocation
                    ,
                    اندازه
#د_که_پای
#که_تعریف_شوی __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#د_که_پای
  );
}
