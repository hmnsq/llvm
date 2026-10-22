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

#ضمن "llvm/Support/Atomic.h"
#ضمن "llvm/Config/llvm-config.h"

using نطاق llvm;

#𐤀𐤌 defined(_MSC_VER)
#ضمن <intrin.h>

// We must include windows.h after intrin.h.
#ضمن <windows.h>
#undef MemoryFence
#endif

#𐤀𐤌 defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

فراغ sys::MemoryFence() {
#𐤀𐤌 LLVM_HAS_ATOMICS == 0
  أرجع;
#وإلا
#  𐤀𐤌 defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  وإلا
# error No memory fence implementation لكل your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(متطاير sys::cas_flag* ptr,
                                  sys::cas_flag new_value,
                                  sys::cas_flag old_value) {
#𐤀𐤌 LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  𐤀𐤌 (result == old_value)
    *ptr = new_value;
  أرجع result;
#وإلاإذا defined(GNU_ATOMICS)
  أرجع __sync_val_compare_and_swap(ptr, old_value, new_value);
#وإلاإذا defined(_MSC_VER)
  أرجع InterlockedCompareExchange(ptr, new_value, old_value);
#وإلا
#  error No compare-and-swap implementation لكل your platform!
#endif
}
