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

#अन्तर्भाव "llvm/Support/Allocator.h"
#अन्तर्भाव "llvm/Support/PerThreadBumpPtrAllocator.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

#अन्तर्भाव <atomic>

नामाकाश llvm {

नामाकाश detail {

शून्य printBumpPtrAllocatorStats(unsigned अङ्कः_Slabs, size_t TotalMemory) {
  errs() << "\nNumber of memory regions: " << अङ्कः_Slabs << '\n'
         << "Bytes allocated: " << TotalMemory << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

शून्य PrintRecyclerStats(size_t Size,
                        size_t Align,
                        size_t FreeListSize) {
  errs() << "Recycler element size: " << Size << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << FreeListSize << '\n';
}

नामाकाश parallel::detail {
unsigned claimPerThreadAllocatorId() {
  स्थिर std::atomic<unsigned> Counter;
  प्रत्यागम Counter.fetch_add(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
