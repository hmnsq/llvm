//===- MemAlloc.cpp - Memory allocation functions -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/MemAlloc.h"
#અન્તર્ભાવ <new>

// These are out of line to have __cpp_aligned_new not affect ABI.

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS શૂન્ય *
llvm::allocate_buffer(size_t Size, size_t Alignment) {
  શૂન્ય *Result = ::operator નવું(Size,
#ifdef __cpp_aligned_new
                                std::align_val_t(Alignment),
#endif
                                std::nothrow);
  જો (Result == nullptr) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  પાછા Result;
}

શૂન્ય llvm::deallocate_buffer(શૂન્ય *Ptr, size_t Size, size_t Alignment) {
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
