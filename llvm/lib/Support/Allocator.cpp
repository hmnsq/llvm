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

#شامل "llvm/Support/Allocator.h"
#شامل "llvm/Support/PerThreadBumpPtrAllocator.h"
#شامل "llvm/Support/raw_ostream.h"

#شامل <atomic>

فضاینام llvm {

فضاینام detail {

تهی printBumpPtrAllocatorStats(unsigned رقم_Slabs, size_t TotalMemory) {
  errs() << "\nNumber of memory regions: " << رقم_Slabs << '\n'
         << "Bytes allocated: " << TotalMemory << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

تهی PrintRecyclerStats(size_t Size,
                        size_t Align,
                        size_t FreeListSize) {
  errs() << "Recycler element size: " << Size << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << FreeListSize << '\n';
}

فضاینام parallel::detail {
unsigned claimPerThreadAllocatorId() {
  ایستا std::atomic<unsigned> Counter;
  بازگشت Counter.fetch_add(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
