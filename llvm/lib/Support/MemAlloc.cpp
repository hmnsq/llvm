//===- MemAlloc.cpp - Memory allocation functions -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/MemAlloc.h"
#অন্তর্ভাব <new>

// These are out of line to have __cpp_aligned_new not affect ABI.

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS শূন্য *
llvm::allocate_বাফার(size_t আকার_2, size_t Alignment) {
  শূন্য *ফলাফল = ::operator নতুন(আকার_2,
#ifdef __cpp_aligned_new
                                std::align_val_t(Alignment),
#endif
                                std::nothrow);
  যদি (ফলাফল == nullptr) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  ফেরত ফলাফল;
}

শূন্য llvm::deallocate_বাফার(শূন্য *Ptr, size_t আকার_2, size_t Alignment) {
  ::operator delete(Ptr
#ifdef __cpp_sized_deallocation
                    ,
                    আকার_2
#endif
#ifdef __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#endif
  );
}
