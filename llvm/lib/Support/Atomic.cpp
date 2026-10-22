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

#অন্তর্ভাব "llvm/Support/Atomic.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h"

using নামাকাশ llvm;

#যদি defined(_MSC_VER)
#অন্তর্ভাব <intrin.h>

// We must include windows.h after intrin.h.
#অন্তর্ভাব <windows.h>
#undef MemoryFence
#endif

#যদি defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

শূন্য sys::MemoryFence() {
#যদি LLVM_HAS_ATOMICS == 0
  ফেরত;
#অন্যথা
#  যদি defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  অন্যথা
# error No memory fence implementation জন্য your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(অস্থির sys::cas_flag* ptr,
                                  sys::cas_flag new_মান,
                                  sys::cas_flag old_মান) {
#যদি LLVM_HAS_ATOMICS == 0
  sys::cas_flag ফলাফল_2 = *ptr;
  যদি (ফলাফল_2 == old_মান)
    *ptr = new_মান;
  ফেরত ফলাফল_2;
#নতুবা_যদি defined(GNU_ATOMICS)
  ফেরত __sync_val_compare_and_swap(ptr, old_মান, new_মান);
#নতুবা_যদি defined(_MSC_VER)
  ফেরত InterlockedCompareExchange(ptr, new_মান, old_মান);
#অন্যথা
#  error No compare-and-swap implementation জন্য your platform!
#endif
}
