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

#ಅನ್ತರ್ಭಾವ "llvm/Support/Atomic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"

using ನಾಮಾಕಾಶ llvm;

#ಯದಿ defined(_MSC_VER)
#ಅನ್ತರ್ಭಾವ <intrin.h>

// We must include windows.h after intrin.h.
#ಅನ್ತರ್ಭಾವ <windows.h>
#undef MemoryFence
#endif

#ಯದಿ defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

ಶೂನ್ಯ sys::MemoryFence() {
#ಯದಿ LLVM_HAS_ATOMICS == 0
  ಪ್ರತ್ಯಾಗಮ;
#ಅನ್ಯಥಾ
#  ಯದಿ defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  ಅನ್ಯಥಾ
# error No memory fence implementation ಪ್ರತಿ your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(ಚಞ್ಚಲ sys::cas_flag* ptr,
                                  sys::cas_flag new_ಮೌಲ್ಯ,
                                  sys::cas_flag old_ಮೌಲ್ಯ) {
#ಯದಿ LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  ಯದಿ (result == old_ಮೌಲ್ಯ)
    *ptr = new_ಮೌಲ್ಯ;
  ಪ್ರತ್ಯಾಗಮ result;
#ನೋಚೇದ್ಯದಿ defined(GNU_ATOMICS)
  ಪ್ರತ್ಯಾಗಮ __sync_val_compare_and_swap(ptr, old_ಮೌಲ್ಯ, new_ಮೌಲ್ಯ);
#ನೋಚೇದ್ಯದಿ defined(_MSC_VER)
  ಪ್ರತ್ಯಾಗಮ InterlockedCompareExchange(ptr, new_ಮೌಲ್ಯ, old_ಮೌಲ್ಯ);
#ಅನ್ಯಥಾ
#  error No compare-and-swap implementation ಪ್ರತಿ your platform!
#endif
}
