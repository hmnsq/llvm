//===- MemAlloc.cpp - Memory allocation functions -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemAlloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <new>

// These are out of line to have __cpp_aligned_new not affect ABI.

LLVM_ATTRIBUTE_RETURNS_NONNULL LLVM_ATTRIBUTE_RETURNS_NOALIAS ਖਾਲੀ *
llvm::allocate_ਬਫਰ(size_t ਮਾਪ, size_t Alignment) {
  ਖਾਲੀ *ਨਤੀਜਾ = ::ਸੰਚਾਲਕ ਨਵਾਂ(ਮਾਪ,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cpp_aligned_new
                                std::align_val_t(Alignment),
#ਜੇ_ਅੰਤ
                                std::nothrow);
  ਜੇ (ਨਤੀਜਾ == ਖਾਲੀ_ਸੂਚਕ) {
    report_bad_alloc_error("Buffer allocation failed");
  }
  ਵਾਪਸ ਨਤੀਜਾ;
}

ਖਾਲੀ llvm::deallocate_ਬਫਰ(ਖਾਲੀ *Ptr, size_t ਮਾਪ, size_t Alignment) {
  ::ਸੰਚਾਲਕ ਮਿਟਾਓ(Ptr
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cpp_sized_deallocation
                    ,
                    ਮਾਪ
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cpp_aligned_new
                    ,
                    std::align_val_t(Alignment)
#ਜੇ_ਅੰਤ
  );
}
