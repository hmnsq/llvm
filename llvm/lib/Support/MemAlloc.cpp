//===- MemAlloc.cpp - Memory allocation functions -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/MemAlloc.h"
#समावेश <new>

// These are out of line to have __cpp_aligned_new not affect ABI.

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS व्योम *
llvm::allocate_बफर(माप_प्रकार आकार, माप_प्रकार Alignment) {
  व्योम *परिणाम = ::चालक नया(आकार,
#अगर_घोषित __cpp_aligned_new
                                std::align_val_t(Alignment),
#पूर्ण_अगर
                                std::nothrow);
  अगर (परिणाम == nullptr) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  वापस परिणाम;
}

व्योम llvm::deallocate_बफर(व्योम *Ptr, माप_प्रकार आकार, माप_प्रकार Alignment) {
  ::चालक मिटाओ(Ptr
#अगर_घोषित __cpp_sized_deallocation
                    ,
                    आकार
#पूर्ण_अगर
#अगर_घोषित __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#पूर्ण_अगर
  );
}
