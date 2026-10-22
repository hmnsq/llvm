//===- MemAlloc.cpp - Memory allocation functions -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/MemAlloc.h"
#अन्तर्भाव <new>

// These are out of line to have __cpp_aligned_new not affect ABI.

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS शून्य *
llvm::allocate_buffer(माप_प्रकार Size, माप_प्रकार Alignment) {
  शून्य *Result = ::चालक नवीन(Size,
#ifdef __cpp_aligned_new
                                std::align_val_t(Alignment),
#endif
                                std::nothrow);
  जर (Result == nullptr) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  परत Result;
}

शून्य llvm::deallocate_buffer(शून्य *Ptr, माप_प्रकार Size, माप_प्रकार Alignment) {
  ::चालक delete(Ptr
#ifdef __cpp_sized_deallocation
                    ,
                    Size
#endif
#ifdef __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#endif
  );
}
