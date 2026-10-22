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

#include "llvm/Support/Atomic.h"
#include "llvm/Config/llvm-config.h"

using namespace llvm;

#ᨀᨘ defined(_MSC_VER)
#include <intrin.h>

// We must include windows.h after intrin.h.
#include <windows.h>
#undef MemoryFence
#endif

#ᨀᨘ defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

void sys::MemoryFence() {
#ᨀᨘ LLVM_HAS_ATOMICS == 0
  ᨀᨘᨒ;
#ᨀᨘᨁ
#  ᨀᨘ defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  ᨀᨘᨁ
# error No memory fence implementation for your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(ca_ncala sys::cas_flag* ptr,
                                  sys::cas_flag new_value,
                                  sys::cas_flag old_value) {
#ᨀᨘ LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  ᨀᨘ (result == old_value)
    *ptr = new_value;
  ᨀᨘᨒ result;
#elif defined(GNU_ATOMICS)
  ᨀᨘᨒ __sync_val_compare_and_swap(ptr, old_value, new_value);
#elif defined(_MSC_VER)
  ᨀᨘᨒ InterlockedCompareExchange(ptr, new_value, old_value);
#ᨀᨘᨁ
#  error No compare-and-swap implementation for your platform!
#endif
}
