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

#അന്തര്ഭാവ "llvm/Support/Allocator.h"
#അന്തര്ഭാവ "llvm/Support/PerThreadBumpPtrAllocator.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

#അന്തര്ഭാവ <atomic>

നാമാകാശ llvm {

നാമാകാശ detail {

ശൂന്യം printBumpPtrAllocatorStats(unsigned അക്കം_Slabs, size_t TotalMemory) {
  errs() << "\nNumber of memory regions: " << അക്കം_Slabs << '\n'
         << "Bytes allocated: " << TotalMemory << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

ശൂന്യം PrintRecyclerStats(size_t Size,
                        size_t Align,
                        size_t FreeListSize) {
  errs() << "Recycler element size: " << Size << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << FreeListSize << '\n';
}

നാമാകാശ parallel::detail {
unsigned claimPerThreadAllocatorId() {
  സ്ഥിതം std::atomic<unsigned> Counter;
  തിരികെ Counter.fetch_add(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
