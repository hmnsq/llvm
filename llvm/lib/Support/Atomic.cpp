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

#إذا defined(_MSC_VER)
#ضمن <intrin.h>

// We must include windows.h after intrin.h.
#ضمن <windows.h>
#undef MemoryFence
#endif

#إذا defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

فراغ sys::MemoryFence() {
#إذا LLVM_HAS_ATOMICS == 0
  أرجع;
#وإلا
#  إذا defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  وإلا
# error No memory fence implementation لكل your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(متطاير sys::cas_flag* ptr,
                                  sys::cas_flag new_قىممەت,
                                  sys::cas_flag old_قىممەت) {
#إذا LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  إذا (result == old_قىممەت)
    *ptr = new_قىممەت;
  أرجع result;
#وإلاإذا defined(GNU_ATOMICS)
  أرجع __sync_val_compare_and_swap(ptr, old_قىممەت, new_قىممەت);
#وإلاإذا defined(_MSC_VER)
  أرجع InterlockedCompareExchange(ptr, new_قىممەت, old_قىممەت);
#وإلا
#  error No compare-and-swap implementation لكل your platform!
#endif
}
