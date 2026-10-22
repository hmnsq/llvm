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
llvm::allocate_buffer(size_t Size, size_t Alignment) {
  فراغ *Result = ::operator جديد(Size,
#ifdef __cpp_aligned_new
                                std::align_val_t(Alignment),
#endif
                                std::nothrow);
  𐤀𐤌 (Result == nullptr) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  أرجع Result;
}

فراغ llvm::deallocate_buffer(فراغ *Ptr, size_t Size, size_t Alignment) {
  ::operator delete(Ptr
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
