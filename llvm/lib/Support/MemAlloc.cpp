//===- MemAlloc.cpp - Memory allocation functions -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemAlloc.h"
#ଅନ୍ତର୍ଭାଵ <new>

// These are out of line to have __cpp_aligned_new not affect ABI.

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS ଶୂନ୍ୟ *
llvm::allocate_buffer(size_t Size, size_t Alignment) {
  ଶୂନ୍ୟ *Result = ::operator ନୂତନ(Size,
#ifdef __cpp_aligned_new
                                std::align_val_t(Alignment),
#endif
                                std::nothrow);
  ଯଦି (Result == nullptr) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  ଫେରନ୍ତୁ Result;
}

ଶୂନ୍ୟ llvm::deallocate_buffer(ଶୂନ୍ୟ *Ptr, size_t Size, size_t Alignment) {
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
