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

#অন্তর্ভাব "llvm/Support/Allocator.h"
#অন্তর্ভাব "llvm/Support/PerThreadBumpPtrAllocator.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"

#অন্তর্ভাব <atomic>

নামাকাশ llvm {

নামাকাশ detail {

শূন্য printBumpPtrAllocatorStats(unsigned অঙ্ক_Slabs, size_t TotalMemory) {
  errs() << "\nNumber of memory regions: " << অঙ্ক_Slabs << '\n'
         << "Bytes allocated: " << TotalMemory << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

শূন্য PrintRecyclerStats(size_t Size,
                        size_t Align,
                        size_t FreeListSize) {
  errs() << "Recycler element size: " << Size << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << FreeListSize << '\n';
}

নামাকাশ parallel::detail {
unsigned claimPerThreadAllocatorId() {
  স্থির std::atomic<unsigned> Counter;
  প্রত্যাগম Counter.fetch_add(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
