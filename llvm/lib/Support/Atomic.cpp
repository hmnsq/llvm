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

#అన్తర్భావ "llvm/Support/Atomic.h"
#అన్తర్భావ "llvm/Config/llvm-config.h"

ఉపయోగించి నామాకాశ llvm;

#ఒకవేళ defined(_MSC_VER)
#అన్తర్భావ <intrin.h>

// We must include windows.h after intrin.h.
#అన్తర్భావ <windows.h>
#నిర్వచనం_రద్దు జ్ఞాపకం_Fence
#అయితే_ముగింపు

#ఒకవేళ defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#నిర్వచించు GNU_ATOMICS
#అయితే_ముగింపు

శూన్య sys::జ్ఞాపకం_Fence() {
#ఒకవేళ LLVM_HAS_ATOMICS == 0
  తిరిగి;
#లేకపోతే
#  ఒకవేళ defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  జ్ఞాపకం_Barrier();
#  లేకపోతే
# error No జ్ఞాపకం fence implementation కొరకు your platform!
#  endif
#అయితే_ముగింపు
}

sys::cas_flag sys::Compare_And_మార్పిడి(అస్థిర sys::cas_flag* ptr,
                                  sys::cas_flag new_విలువ,
                                  sys::cas_flag old_విలువ) {
#ఒకవేళ LLVM_HAS_ATOMICS == 0
  sys::cas_flag ఫలితం_2 = *ptr;
  ఒకవేళ (ఫలితం_2 == old_విలువ)
    *ptr = new_విలువ;
  తిరిగి ఫలితం_2;
#లేదావేళ defined(GNU_ATOMICS)
  తిరిగి __sync_val_compare_and_swap(ptr, old_విలువ, new_విలువ);
#లేదావేళ defined(_MSC_VER)
  తిరిగి InterlockedCompareExchange(ptr, new_విలువ, old_విలువ);
#లేకపోతే
#  error No compare-and-మార్పిడి implementation కొరకు your platform!
#అయితే_ముగింపు
}
