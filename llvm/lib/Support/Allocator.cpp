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

#అన్తర్భావ "llvm/Support/Allocator.h"
#అన్తర్భావ "llvm/Support/PerThreadBumpPtrAllocator.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"

#అన్తర్భావ <atomic>

నామాకాశ llvm {

నామాకాశ detail {

శూన్య printBumpPtrAllocatorStats(గుర్తులేని అంకె_Slabs, size_t మొత్తం_జ్ఞాపకం) {
  errs() << "\nNumber of memory regions: " << అంకె_Slabs << '\n'
         << "Bytes allocated: " << మొత్తం_జ్ఞాపకం << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

శూన్య PrintRecyclerStats(size_t కొలత,
                        size_t Align,
                        size_t Free_List_కొలత) {
  errs() << "Recycler element size: " << కొలత << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << Free_List_కొలత << '\n';
}

నామాకాశ parallel::detail {
గుర్తులేని claim_చొప్పున_Thread_Allocator_Id() {
  స్థిర std::atomic<గుర్తులేని> Counter;
  తిరిగి Counter.fetch_కూడు(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
