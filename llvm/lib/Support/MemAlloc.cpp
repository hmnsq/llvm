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
llvm::allocate_बफर(size_t आकार, size_t Alignment) {
  शून्य *परिणाम = ::संक्रियकः नूतन(आकार,
#यदि_परिभाषितम् __cpp_aligned_new
                                std::align_val_t(Alignment),
#यदि_अन्तः
                                std::nothrow);
  यदि (परिणाम == शून्यसूचकः) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  प्रत्यागम परिणाम;
}

शून्य llvm::deallocate_बफर(शून्य *Ptr, size_t आकार, size_t Alignment) {
  ::संक्रियकः विलोपय(Ptr
#यदि_परिभाषितम् __cpp_sized_deallocation
                    ,
                    आकार
#यदि_अन्तः
#यदि_परिभाषितम् __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#यदि_अन्तः
  );
}
