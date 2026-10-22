//===--- Allocator.cpp - Simple memory allocation abstraction -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the BumpPtrAllocator interface.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/Allocator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/PerThreadBumpPtrAllocator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

#ಅನ್ತರ್ಭಾವ <atomic>

ನಾಮಾಕಾಶ llvm {

ನಾಮಾಕಾಶ detail {

ಶೂನ್ಯ printBumpPtrAllocatorStats(unsigned ಅಂಕೆ_Slabs, size_t TotalMemory) {
  errs() << "\nNumber of memory regions: " << ಅಂಕೆ_Slabs << '\n'
         << "Bytes allocated: " << TotalMemory << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

ಶೂನ್ಯ PrintRecyclerStats(size_t Size,
                        size_t Align,
                        size_t FreeListSize) {
  errs() << "Recycler element size: " << Size << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << FreeListSize << '\n';
}

ನಾಮಾಕಾಶ parallel::detail {
unsigned claimPerThreadAllocatorId() {
  ಸ್ಥಿರ std::atomic<unsigned> Counter;
  ಪ್ರತ್ಯಾಗಮ Counter.fetch_add(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
