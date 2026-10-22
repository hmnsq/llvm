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

#अन्तर्भाव "llvm/Support/Atomic.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"

उपयुज्य नामाकाश llvm;

#यदि defined(_MSC_VER)
#अन्तर्भाव <intrin.h>

// We must include windows.h after intrin.h.
#अन्तर्भाव <windows.h>
#अपरिभाषय स्मृतिः_Fence
#यदि_अन्तः

#यदि defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#परिभाषय GNU_ATOMICS
#यदि_अन्तः

शून्य sys::स्मृतिः_Fence() {
#यदि LLVM_HAS_ATOMICS == 0
  प्रत्यागम;
#अन्यथा
#  यदि defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  स्मृतिः_Barrier();
#  अन्यथा
# error No स्मृतिः fence implementation प्रति your platform!
#  endif
#यदि_अन्तः
}

sys::cas_flag sys::CompareAndSwap(चञ्चल sys::cas_flag* ptr,
                                  sys::cas_flag new_मानम्,
                                  sys::cas_flag old_मानम्) {
#यदि LLVM_HAS_ATOMICS == 0
  sys::cas_flag परिणाम_2 = *ptr;
  यदि (परिणाम_2 == old_मानम्)
    *ptr = new_मानम्;
  प्रत्यागम परिणाम_2;
#नोचेद्यदि defined(GNU_ATOMICS)
  प्रत्यागम __sync_val_compare_and_swap(ptr, old_मानम्, new_मानम्);
#नोचेद्यदि defined(_MSC_VER)
  प्रत्यागम InterlockedCompareExchange(ptr, new_मानम्, old_मानम्);
#अन्यथा
#  error No compare-and-swap implementation प्रति your platform!
#यदि_अन्तः
}
