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

#અન્તર્ભાવ "llvm/Support/Allocator.h"
#અન્તર્ભાવ "llvm/Support/PerThreadBumpPtrAllocator.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

#અન્તર્ભાવ <atomic>

નામાકાશ llvm {

નામાકાશ detail {

શૂન્ય printBumpPtrAllocatorStats(unsigned અંક_Slabs, size_t TotalMemory) {
  errs() << "\nNumber of memory regions: " << અંક_Slabs << '\n'
         << "Bytes allocated: " << TotalMemory << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

શૂન્ય PrintRecyclerStats(size_t Size,
                        size_t Align,
                        size_t FreeListSize) {
  errs() << "Recycler element size: " << Size << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << FreeListSize << '\n';
}

નામાકાશ parallel::detail {
unsigned claimPerThreadAllocatorId() {
  સ્થિર std::atomic<unsigned> Counter;
  પાછા Counter.fetch_add(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
