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

#ប្រសិនបើ defined(_MSC_VER)
#include <intrin.h>

// We must include windows.h after intrin.h.
#include <windows.h>
#undef MemoryFence
#endif

#ប្រសិនបើ defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

void sys::MemoryFence() {
#ប្រសិនបើ LLVM_HAS_ATOMICS == 0
  ត្រឡប់;
#បើមិនដូច្នេះ
#  ប្រសិនបើ defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  បើមិនដូច្នេះ
# error No memory fence implementation សម្រាប់ your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(ca_ncala sys::cas_flag* ptr,
                                  sys::cas_flag new_value,
                                  sys::cas_flag old_value) {
#ប្រសិនបើ LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  ប្រសិនបើ (result == old_value)
    *ptr = new_value;
  ត្រឡប់ result;
#elif defined(GNU_ATOMICS)
  ត្រឡប់ __sync_val_compare_and_swap(ptr, old_value, new_value);
#elif defined(_MSC_VER)
  ត្រឡប់ InterlockedCompareExchange(ptr, new_value, old_value);
#បើមិនដូច្នេះ
#  error No compare-and-swap implementation សម្រាប់ your platform!
#endif
}
