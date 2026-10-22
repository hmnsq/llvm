//===-- Atomic.cpp - Atomic Operations --------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements atomic operations.
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Atomic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"

using ନାମାକାଶ llvm;

#ଯଦି defined(_MSC_VER)
#ଅନ୍ତର୍ଭାଵ <intrin.h>

// We must include windows.h after intrin.h.
#ଅନ୍ତର୍ଭାଵ <windows.h>
#undef MemoryFence
#endif

#ଯଦି defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

ଶୂନ୍ୟ sys::MemoryFence() {
#ଯଦି LLVM_HAS_ATOMICS == 0
  ଫେରନ୍ତୁ;
#ଅନ୍ୟଥା
#  ଯଦି defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  ଅନ୍ୟଥା
# error No memory fence implementation ପାଇଁ your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(ଅସ୍ଥିର sys::cas_flag* ptr,
                                  sys::cas_flag new_ମୂଲ୍ୟ,
                                  sys::cas_flag old_ମୂଲ୍ୟ) {
#ଯଦି LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  ଯଦି (result == old_ମୂଲ୍ୟ)
    *ptr = new_ମୂଲ୍ୟ;
  ଫେରନ୍ତୁ result;
#ନଚେତଯଦି defined(GNU_ATOMICS)
  ଫେରନ୍ତୁ __sync_val_compare_and_swap(ptr, old_ମୂଲ୍ୟ, new_ମୂଲ୍ୟ);
#ନଚେତଯଦି defined(_MSC_VER)
  ଫେରନ୍ତୁ InterlockedCompareExchange(ptr, new_ମୂଲ୍ୟ, old_ମୂଲ୍ୟ);
#ଅନ୍ୟଥା
#  error No compare-and-swap implementation ପାଇଁ your platform!
#endif
}
