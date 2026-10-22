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

শূন্য printBumpPtrAllocatorStats(unsigned অঙ্ক_Slabs, size_t যোগ_Memory) {
  errs() << "\nNumber of memory regions: " << অঙ্ক_Slabs << '\n'
         << "Bytes allocated: " << যোগ_Memory << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

শূন্য PrintRecyclerStats(size_t আকার_2,
                        size_t Align,
                        size_t Free_List_আকার) {
  errs() << "Recycler element size: " << আকার_2 << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << Free_List_আকার << '\n';
}

নামাকাশ parallel::detail {
unsigned claimPerThreadAllocatorId() {
  স্থির std::atomic<unsigned> গণক;
  ফেরত গণক.fetch_add(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
