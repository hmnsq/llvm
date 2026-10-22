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

#如果 defined(_MSC_VER)
#include <intrin.h>

// We must include windows.h after intrin.h.
#include <windows.h>
#undef MemoryFence
#endif

#如果 defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

void sys::MemoryFence() {
#如果 LLVM_HAS_ATOMICS == 0
  返回;
#否则
#  如果 defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  否则
# error No memory fence implementation 为 your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(volatile sys::cas_flag* ptr,
                                  sys::cas_flag new_value,
                                  sys::cas_flag old_value) {
#如果 LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  如果 (result == old_value)
    *ptr = new_value;
  返回 result;
#elif defined(GNU_ATOMICS)
  返回 __sync_val_compare_and_swap(ptr, old_value, new_value);
#elif defined(_MSC_VER)
  返回 InterlockedCompareExchange(ptr, new_value, old_value);
#否则
#  error No compare-and-swap implementation 为 your platform!
#endif
}
