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

استفاده نطاق llvm;

#إذا defined(_MSC_VER)
#ضمن <intrin.h>

// We must include windows.h after intrin.h.
#ضمن <windows.h>
#لغو_تعریف حافظه_Fence
#پایان_اگر

#إذا defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#تعریف_کن GNU_ATOMICS
#پایان_اگر

فراغ sys::حافظه_Fence() {
#إذا LLVM_HAS_ATOMICS == 0
  أرجع;
#وإلا
#  إذا defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  حافظه_Barrier();
#  وإلا
# error No حافظه fence implementation لكل your platform!
#  endif
#پایان_اگر
}

sys::cas_flag sys::Compare_And_جابجا(متطاير sys::cas_flag* ptr,
                                  sys::cas_flag new_مقدار,
                                  sys::cas_flag old_مقدار) {
#إذا LLVM_HAS_ATOMICS == 0
  sys::cas_flag نتیجه_2 = *ptr;
  إذا (نتیجه_2 == old_مقدار)
    *ptr = new_مقدار;
  أرجع نتیجه_2;
#وإلاإذا defined(GNU_ATOMICS)
  أرجع __sync_val_compare_and_swap(ptr, old_مقدار, new_مقدار);
#وإلاإذا defined(_MSC_VER)
  أرجع InterlockedCompareExchange(ptr, new_مقدار, old_مقدار);
#وإلا
#  error No compare-and-جابجا implementation لكل your platform!
#پایان_اگر
}
