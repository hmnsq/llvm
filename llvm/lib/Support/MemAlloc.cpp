//===- MemAlloc.cpp - Memory allocation functions -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/MemAlloc.h"
#അന്തര്ഭാവ <new>

// These are out of line to have __cpp_aligned_new not affect ABI.

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS ശൂന്യം *
llvm::allocate_buffer(size_t Size, size_t Alignment) {
  ശൂന്യം *Result = ::operator പുതിയ(Size,
#ifdef __cpp_aligned_new
                                std::align_val_t(Alignment),
#endif
                                std::nothrow);
  എങ്കിൽ (Result == nullptr) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  തിരികെ Result;
}

ശൂന്യം llvm::deallocate_buffer(ശൂന്യം *Ptr, size_t Size, size_t Alignment) {
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
