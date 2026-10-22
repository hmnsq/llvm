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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

#ಆದರೆ defined(_MSC_VER)
#ಅನ್ತರ್ಭಾವ <intrin.h>

// We must include windows.h after intrin.h.
#ಅನ್ತರ್ಭಾವ <windows.h>
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಸ್ಮರಣೆ_Fence
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#ವ್ಯಾಖ್ಯಾನಿಸು GNU_ATOMICS
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ sys::ಸ್ಮರಣೆ_Fence() {
#ಆದರೆ LLVM_HAS_ATOMICS == 0
  ಹಿಂತಿರುಗಿ;
#ಇಲ್ಲದಿದ್ದರೆ
#  ಆದರೆ defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  ಸ್ಮರಣೆ_Barrier();
#  ಇಲ್ಲದಿದ್ದರೆ
# error No ಸ್ಮರಣೆ fence implementation ಗಾಗಿ your platform!
#  endif
#ಆದರೆ_ಅಂತ್ಯ
}

sys::cas_flag sys::Compare_And_ಅದಲುಬದಲು(ಅಸ್ಥಿರ sys::cas_flag* ptr,
                                  sys::cas_flag new_ಮೌಲ್ಯ,
                                  sys::cas_flag old_ಮೌಲ್ಯ) {
#ಆದರೆ LLVM_HAS_ATOMICS == 0
  sys::cas_flag ಫಲಿತಾಂಶ_2 = *ptr;
  ಆದರೆ (ಫಲಿತಾಂಶ_2 == old_ಮೌಲ್ಯ)
    *ptr = new_ಮೌಲ್ಯ;
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
#ಅಥವಾಆದರೆ defined(GNU_ATOMICS)
  ಹಿಂತಿರುಗಿ __sync_val_compare_and_swap(ptr, old_ಮೌಲ್ಯ, new_ಮೌಲ್ಯ);
#ಅಥವಾಆದರೆ defined(_MSC_VER)
  ಹಿಂತಿರುಗಿ InterlockedCompareExchange(ptr, new_ಮೌಲ್ಯ, old_ಮೌಲ್ಯ);
#ಇಲ್ಲದಿದ್ದರೆ
#  error No compare-and-ಅದಲುಬದಲು implementation ಗಾಗಿ your platform!
#ಆದರೆ_ಅಂತ್ಯ
}
