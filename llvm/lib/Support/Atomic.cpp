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

کارول نطاق llvm;

#إذا defined(_MSC_VER)
#ضمن <intrin.h>

// We must include windows.h after intrin.h.
#ضمن <windows.h>
#تعریف_لغوه حافظه_Fence
#د_که_پای

#إذا defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#تعریف_کړه GNU_ATOMICS
#د_که_پای

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
#د_که_پای
}

sys::cas_flag sys::Compare_And_بدلون(متطاير sys::cas_flag* ptr,
                                  sys::cas_flag new_ارزښت,
                                  sys::cas_flag old_ارزښت) {
#إذا LLVM_HAS_ATOMICS == 0
  sys::cas_flag پایله_2 = *ptr;
  إذا (پایله_2 == old_ارزښت)
    *ptr = new_ارزښت;
  أرجع پایله_2;
#وإلاإذا defined(GNU_ATOMICS)
  أرجع __sync_val_compare_and_swap(ptr, old_ارزښت, new_ارزښت);
#وإلاإذا defined(_MSC_VER)
  أرجع InterlockedCompareExchange(ptr, new_ارزښت, old_ارزښت);
#وإلا
#  error No compare-and-بدلون implementation لكل your platform!
#د_که_پای
}
