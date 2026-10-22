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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Allocator.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/PerThreadBumpPtrAllocator.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <atomic>

ਨਾਮਾਕਾਸ਼ llvm {

ਨਾਮਾਕਾਸ਼ detail {

ਖਾਲੀ printBumpPtrAllocatorStats(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Slabs, size_t ਕੁੱਲ_ਸਮ੍ਰਿਤੀ) {
  errs() << "\nNumber of memory regions: " << ਅੰਕ_Slabs << '\n'
         << "Bytes allocated: " << ਕੁੱਲ_ਸਮ੍ਰਿਤੀ << '\n'
         << " (includes alignment, etc)\n";
}

} // namespace detail

ਖਾਲੀ PrintRecyclerStats(size_t ਮਾਪ,
                        size_t Align,
                        size_t Free_List_ਮਾਪ) {
  errs() << "Recycler element size: " << ਮਾਪ << '\n'
         << "Recycler element alignment: " << Align << '\n'
         << "Number of elements free for recycling: " << Free_List_ਮਾਪ << '\n';
}

ਨਾਮਾਕਾਸ਼ parallel::detail {
ਬਿਨਾਨਿਸ਼ਾਨੀ claim_ਪ੍ਰਤੀ_Thread_Allocator_Id() {
  ਸਥਿਰ std::atomic<ਬਿਨਾਨਿਸ਼ਾਨੀ> Counter;
  ਵਾਪਸ Counter.fetch_ਜੋੜੋ(1, std::memory_order_relaxed);
}
} // namespace parallel::detail

} // namespace llvm
