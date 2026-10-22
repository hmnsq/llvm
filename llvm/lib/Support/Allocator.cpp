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

#அந்தர்பாவ "llvm/Support/Allocator.h"
#அந்தர்பாவ "llvm/Support/PerThreadBumpPtrAllocator.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

#அந்தர்பாவ <atomic>

நாமாகாஶ llvm {

நாமாகாஶ detail {

வெறுமை printBumpPtrAllocatorStats(unsigned இலக்கம்_Slabs, size_t TotalMemory) {
  errs() << "\nNumber of memory regions: " << இலக்கம்_Slabs << '\n'
         << "Bytes allocated: " << TotalMemory << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

வெறுமை PrintRecyclerStats(size_t Size,
                        size_t Align,
                        size_t FreeListSize) {
  errs() << "Recycler element size: " << Size << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << FreeListSize << '\n';
}

நாமாகாஶ parallel::detail {
unsigned claimPerThreadAllocatorId() {
  நிலையான std::atomic<unsigned> Counter;
  திரும்பு Counter.fetch_add(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
