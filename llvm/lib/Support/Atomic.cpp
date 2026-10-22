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

استعمال نطاق llvm;

#إذا defined(_MSC_VER)
#ضمن <intrin.h>

// We must include windows.h after intrin.h.
#ضمن <windows.h>
#وصف_رد يادگيري_Fence
#جيڪڏهن_پورو

#إذا defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#وصف_ڪريو GNU_ATOMICS
#جيڪڏهن_پورو

فراغ sys::يادگيري_Fence() {
#إذا LLVM_HAS_ATOMICS == 0
  أرجع;
#وإلا
#  إذا defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  يادگيري_Barrier();
#  وإلا
# error No يادگيري fence implementation لكل your platform!
#  endif
#جيڪڏهن_پورو
}

sys::cas_flag sys::Compare_And_مٽا_سٽا(متطاير sys::cas_flag* ptr,
                                  sys::cas_flag new_قدر,
                                  sys::cas_flag old_قدر) {
#إذا LLVM_HAS_ATOMICS == 0
  sys::cas_flag نتيجو_2 = *ptr;
  إذا (نتيجو_2 == old_قدر)
    *ptr = new_قدر;
  أرجع نتيجو_2;
#وإلاإذا defined(GNU_ATOMICS)
  أرجع __sync_val_compare_and_swap(ptr, old_قدر, new_قدر);
#وإلاإذا defined(_MSC_VER)
  أرجع InterlockedCompareExchange(ptr, new_قدر, old_قدر);
#وإلا
#  error No compare-and-مٽا_سٽا implementation لكل your platform!
#جيڪڏهن_پورو
}
