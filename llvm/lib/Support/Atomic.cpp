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

using नामाकाश llvm;

#जर defined(_MSC_VER)
#अन्तर्भाव <intrin.h>

// We must include windows.h after intrin.h.
#अन्तर्भाव <windows.h>
#undef MemoryFence
#endif

#जर defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

शून्य sys::MemoryFence() {
#जर LLVM_HAS_ATOMICS == 0
  परत;
#अन्यथा
#  जर defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  अन्यथा
# error No memory fence implementation साठी your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(अस्थिर sys::cas_flag* ptr,
                                  sys::cas_flag new_मूल्य,
                                  sys::cas_flag old_मूल्य) {
#जर LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  जर (result == old_मूल्य)
    *ptr = new_मूल्य;
  परत result;
#किंवाजर defined(GNU_ATOMICS)
  परत __sync_val_compare_and_swap(ptr, old_मूल्य, new_मूल्य);
#किंवाजर defined(_MSC_VER)
  परत InterlockedCompareExchange(ptr, new_मूल्य, old_मूल्य);
#अन्यथा
#  error No compare-and-swap implementation साठी your platform!
#endif
}
