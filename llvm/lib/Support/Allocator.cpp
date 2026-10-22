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

نامخلا llvm {

نامخلا detail {

خالی printBumpPtrAllocatorStats(غیرعلامتی عدد_Slabs, size_t کل_حافظہ) {
  errs() << "\nNumber of memory regions: " << عدد_Slabs << '\n'
         << "Bytes allocated: " << کل_حافظہ << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

خالی PrintRecyclerStats(size_t سائز,
                        size_t Align,
                        size_t Free_List_سائز) {
  errs() << "Recycler element size: " << سائز << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << Free_List_سائز << '\n';
}

نامخلا parallel::detail {
غیرعلامتی claim_فی_Thread_Allocator_Id() {
  ساکن std::atomic<غیرعلامتی> شمار_2;
  واپس شمار_2.fetch_جوڑو(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
