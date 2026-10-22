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

#ᱡᱩᱫᱤ defined(_MSC_VER)
#include <intrin.h>

// We must include windows.h after intrin.h.
#include <windows.h>
#undef MemoryFence
#endif

#ᱡᱩᱫᱤ defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

void sys::MemoryFence() {
#ᱡᱩᱫᱤ LLVM_HAS_ATOMICS == 0
  ᱨᱩᱣᱟᱹᱲ;
#ᱵᱟᱝ
#  ᱡᱩᱫᱤ defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  ᱵᱟᱝ
# error No memory fence implementation for your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(ca_ncala sys::cas_flag* ptr,
                                  sys::cas_flag new_value,
                                  sys::cas_flag old_value) {
#ᱡᱩᱫᱤ LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  ᱡᱩᱫᱤ (result == old_value)
    *ptr = new_value;
  ᱨᱩᱣᱟᱹᱲ result;
#elif defined(GNU_ATOMICS)
  ᱨᱩᱣᱟᱹᱲ __sync_val_compare_and_swap(ptr, old_value, new_value);
#elif defined(_MSC_VER)
  ᱨᱩᱣᱟᱹᱲ InterlockedCompareExchange(ptr, new_value, old_value);
#ᱵᱟᱝ
#  error No compare-and-swap implementation for your platform!
#endif
}
