//===- MemAlloc.cpp - Memory allocation functions -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/MemAlloc.h"
#అన్తర్భావ <new>

// These are out of line to have __cpp_aligned_new not affect ABI.

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS శూన్య *
llvm::allocate_బఫరు(size_t కొలత, size_t Alignment) {
  శూన్య *ఫలితం = ::క్రియాకారకం కొత్త(కొలత,
#నిర్వచితమైతే __cpp_aligned_new
                                std::align_val_t(Alignment),
#అయితే_ముగింపు
                                std::nothrow);
  ఒకవేళ (ఫలితం == శూన్యసూచిక) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  తిరిగి ఫలితం;
}

శూన్య llvm::deallocate_బఫరు(శూన్య *Ptr, size_t కొలత, size_t Alignment) {
  ::క్రియాకారకం తొలగించు(Ptr
#నిర్వచితమైతే __cpp_sized_deallocation
                    ,
                    కొలత
#అయితే_ముగింపు
#నిర్వచితమైతే __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#అయితే_ముగింపు
  );
}
