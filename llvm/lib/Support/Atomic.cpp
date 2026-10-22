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

#सचे defined(_MSC_VER)
#अन्तर्भाव <intrin.h>

// We must include windows.h after intrin.h.
#अन्तर्भाव <windows.h>
#undef MemoryFence
#endif

#सचे defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

शून्य sys::MemoryFence() {
#सचे LLVM_HAS_ATOMICS == 0
  पटिनिवत्त;
#अञ्ञथा
#  सचे defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  अञ्ञथा
# error No memory fence implementation अत्थाय your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(चञ्चल sys::cas_flag* ptr,
                                  sys::cas_flag new_मानम्,
                                  sys::cas_flag old_मानम्) {
#सचे LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  सचे (result == old_मानम्)
    *ptr = new_मानम्;
  पटिनिवत्त result;
#नोचेद्यदि defined(GNU_ATOMICS)
  पटिनिवत्त __sync_val_compare_and_swap(ptr, old_मानम्, new_मानम्);
#नोचेद्यदि defined(_MSC_VER)
  पटिनिवत्त InterlockedCompareExchange(ptr, new_मानम्, old_मानम्);
#अञ्ञथा
#  error No compare-and-swap implementation अत्थाय your platform!
#endif
}
