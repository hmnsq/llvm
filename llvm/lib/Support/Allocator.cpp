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

#समावेश "llvm/Support/Allocator.h"
#समावेश "llvm/Support/PerThreadBumpPtrAllocator.h"
#समावेश "llvm/Support/raw_ostream.h"

#समावेश <atomic>

namespace llvm {

namespace detail {

व्योम printBumpPtrAllocatorStats(अचिन्हित अंक_Slabs, माप_प्रकार योग_Memory) {
  errs() << "\nNumber of memory regions: " << अंक_Slabs << '\n'
         << "Bytes allocated: " << योग_Memory << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

व्योम PrintRecyclerStats(माप_प्रकार आकार,
                        माप_प्रकार Align,
                        माप_प्रकार Free_List_आकार) {
  errs() << "Recycler element size: " << आकार << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << Free_List_आकार << '\n';
}

namespace parallel::detail {
अचिन्हित claimPerThreadAllocatorId() {
  अटल std::atomic<अचिन्हित> गणक;
  वापस गणक.fetch_add(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
