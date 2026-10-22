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

#شامل "llvm/Support/Atomic.h"
#شامل "llvm/Config/llvm-config.h"

استعمال نامخلا llvm;

#اگر defined(_MSC_VER)
#شامل <intrin.h>

// We must include windows.h after intrin.h.
#شامل <windows.h>
#تعریف_منسوخ حافظہ_Fence
#اگر_ختم

#اگر defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#تعریف_کرو GNU_ATOMICS
#اگر_ختم

خالی sys::حافظہ_Fence() {
#اگر LLVM_HAS_ATOMICS == 0
  واپس;
#ورنہ
#  اگر defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  حافظہ_Barrier();
#  ورنہ
# error No حافظہ fence implementation برائے your platform!
#  endif
#اگر_ختم
}

sys::cas_flag sys::Compare_And_تبادلہ(متلون sys::cas_flag* ptr,
                                  sys::cas_flag new_قدر,
                                  sys::cas_flag old_قدر) {
#اگر LLVM_HAS_ATOMICS == 0
  sys::cas_flag نتیجہ_2 = *ptr;
  اگر (نتیجہ_2 == old_قدر)
    *ptr = new_قدر;
  واپس نتیجہ_2;
#وگرنہ_اگر defined(GNU_ATOMICS)
  واپس __sync_val_compare_and_swap(ptr, old_قدر, new_قدر);
#وگرنہ_اگر defined(_MSC_VER)
  واپس InterlockedCompareExchange(ptr, new_قدر, old_قدر);
#ورنہ
#  error No compare-and-تبادلہ implementation برائے your platform!
#اگر_ختم
}
