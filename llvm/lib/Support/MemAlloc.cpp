//===- MemAlloc.cpp - Memory allocation functions -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/MemAlloc.h"
#ಅನ್ತರ್ಭಾವ <new>

// These are out of line to have __cpp_aligned_new not affect ABI.

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS ಶೂನ್ಯ *
llvm::allocate_buffer(size_t Size, size_t Alignment) {
  ಶೂನ್ಯ *Result = ::operator ನೂತನ(Size,
#ifdef __cpp_aligned_new
                                std::align_val_t(Alignment),
#endif
                                std::nothrow);
  ಯದಿ (Result == nullptr) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  ಪ್ರತ್ಯಾಗಮ Result;
}

ಶೂನ್ಯ llvm::deallocate_buffer(ಶೂನ್ಯ *Ptr, size_t Size, size_t Alignment) {
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
