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

#ضمن "llvm/Support/Allocator.h"
#ضمن "llvm/Support/PerThreadBumpPtrAllocator.h"
#ضمن "llvm/Support/raw_ostream.h"

#ضمن <atomic>

نطاق llvm {

نطاق detail {

فراغ printBumpPtrAllocatorStats(unsigned عدد_Slabs, size_t TotalMemory) {
  errs() << "\nNumber of memory regions: " << عدد_Slabs << '\n'
         << "Bytes allocated: " << TotalMemory << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

فراغ PrintRecyclerStats(size_t Size,
                        size_t Align,
                        size_t FreeListSize) {
  errs() << "Recycler element size: " << Size << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << FreeListSize << '\n';
}

نطاق parallel::detail {
unsigned claimPerThreadAllocatorId() {
  ساكن std::atomic<unsigned> Counter;
  أرجع Counter.fetch_add(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
