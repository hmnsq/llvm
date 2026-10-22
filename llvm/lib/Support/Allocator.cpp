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

शून्य printBumpPtrAllocatorStats(अचिह्नित अङ्कः_Slabs, size_t योग_स्मृतिः) {
  errs() << "\nNumber of memory regions: " << अङ्कः_Slabs << '\n'
         << "Bytes allocated: " << योग_स्मृतिः << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

शून्य PrintRecyclerStats(size_t आकार,
                        size_t Align,
                        size_t Free_List_आकार) {
  errs() << "Recycler element size: " << आकार << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << Free_List_आकार << '\n';
}

नामाकाश parallel::detail {
अचिह्नित claim_प्रत्येकम्_Thread_Allocator_Id() {
  स्थिर std::atomic<अचिह्नित> गणक;
  प्रत्यागम गणक.fetch_सङ्कलय(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
