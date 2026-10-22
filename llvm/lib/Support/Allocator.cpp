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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Allocator.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/PerThreadBumpPtrAllocator.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

#ଅନ୍ତର୍ଭାଵ <atomic>

ନାମାକାଶ llvm {

ନାମାକାଶ detail {

ଶୂନ୍ୟ printBumpPtrAllocatorStats(unsigned ଅଙ୍କ_Slabs, size_t TotalMemory) {
  errs() << "\nNumber of memory regions: " << ଅଙ୍କ_Slabs << '\n'
         << "Bytes allocated: " << TotalMemory << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

ଶୂନ୍ୟ PrintRecyclerStats(size_t Size,
                        size_t Align,
                        size_t FreeListSize) {
  errs() << "Recycler element size: " << Size << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << FreeListSize << '\n';
}

ନାମାକାଶ parallel::detail {
unsigned claimPerThreadAllocatorId() {
  ସ୍ଥିର std::atomic<unsigned> Counter;
  ଫେରନ୍ତୁ Counter.fetch_add(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
