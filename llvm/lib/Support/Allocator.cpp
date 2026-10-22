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

فراغ printBumpPtrAllocatorStats(بینښه عدد_Slabs, size_t ټول_حافظه) {
  errs() << "\nNumber of memory regions: " << عدد_Slabs << '\n'
         << "Bytes allocated: " << ټول_حافظه << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

فراغ PrintRecyclerStats(size_t اندازه,
                        size_t Align,
                        size_t Free_List_اندازه) {
  errs() << "Recycler element size: " << اندازه << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << Free_List_اندازه << '\n';
}

نطاق parallel::detail {
بینښه claim_فی_Thread_Allocator_Id() {
  ساكن std::atomic<بینښه> Counter;
  أرجع Counter.fetch_جمع_کړه(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
