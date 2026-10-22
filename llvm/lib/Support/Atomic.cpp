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

#અન્તર્ભાવ "llvm/Support/Atomic.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h"

using નામાકાશ llvm;

#જો defined(_MSC_VER)
#અન્તર્ભાવ <intrin.h>

// We must include windows.h after intrin.h.
#અન્તર્ભાવ <windows.h>
#undef MemoryFence
#endif

#જો defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

શૂન્ય sys::MemoryFence() {
#જો LLVM_HAS_ATOMICS == 0
  પાછા;
#નહીંતર
#  જો defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  નહીંતર
# error No memory fence implementation માટે your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(અસ્થિર sys::cas_flag* ptr,
                                  sys::cas_flag new_કિંમત,
                                  sys::cas_flag old_કિંમત) {
#જો LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  જો (result == old_કિંમત)
    *ptr = new_કિંમત;
  પાછા result;
#અથવાજો defined(GNU_ATOMICS)
  પાછા __sync_val_compare_and_swap(ptr, old_કિંમત, new_કિંમત);
#અથવાજો defined(_MSC_VER)
  પાછા InterlockedCompareExchange(ptr, new_કિંમત, old_કિંમત);
#નહીંતર
#  error No compare-and-swap implementation માટે your platform!
#endif
}
