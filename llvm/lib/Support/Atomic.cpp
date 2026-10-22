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

#もし defined(_MSC_VER)
#include <intrin.h>

// We must include windows.h after intrin.h.
#include <windows.h>
#undef MemoryFence
#endif

#もし defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

void sys::MemoryFence() {
#もし LLVM_HAS_ATOMICS == 0
  戻る;
#否则
#  もし defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  否则
# error No memory fence implementation ために your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(volatile sys::cas_flag* ptr,
                                  sys::cas_flag new_value,
                                  sys::cas_flag old_value) {
#もし LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  もし (result == old_value)
    *ptr = new_value;
  戻る result;
#elif defined(GNU_ATOMICS)
  戻る __sync_val_compare_and_swap(ptr, old_value, new_value);
#elif defined(_MSC_VER)
  戻る InterlockedCompareExchange(ptr, new_value, old_value);
#否则
#  error No compare-and-swap implementation ために your platform!
#endif
}
