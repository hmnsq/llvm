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
llvm::allocate_ಬಫರ್(size_t ಗಾತ್ರ_2, size_t Alignment) {
  ಶೂನ್ಯ *ಫಲಿತಾಂಶ = ::ಕ್ರಿಯಾಕಾರಕ ಹೊಸ(ಗಾತ್ರ_2,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cpp_aligned_new
                                std::align_val_t(Alignment),
#ಆದರೆ_ಅಂತ್ಯ
                                std::nothrow);
  ಆದರೆ (ಫಲಿತಾಂಶ == ಶೂನ್ಯಸೂಚಕ) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ಶೂನ್ಯ llvm::deallocate_ಬಫರ್(ಶೂನ್ಯ *Ptr, size_t ಗಾತ್ರ_2, size_t Alignment) {
  ::ಕ್ರಿಯಾಕಾರಕ ಅಳಿಸು(Ptr
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cpp_sized_deallocation
                    ,
                    ಗಾತ್ರ_2
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#ಆದರೆ_ಅಂತ್ಯ
  );
}
