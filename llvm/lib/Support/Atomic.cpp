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

#यदि defined(_MSC_VER)
#अन्तर्भाव <intrin.h>

// We must include windows.h after intrin.h.
#अन्तर्भाव <windows.h>
#undef MemoryFence
#endif

#यदि defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

शून्य sys::MemoryFence() {
#यदि LLVM_HAS_ATOMICS == 0
  फर्काऊ;
#अन्यथा
#  यदि defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  अन्यथा
# error No memory fence implementation लागि your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(चञ्चल sys::cas_flag* ptr,
                                  sys::cas_flag new_मान,
                                  sys::cas_flag old_मान) {
#यदि LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  यदि (result == old_मान)
    *ptr = new_मान;
  फर्काऊ result;
#अथवायदि defined(GNU_ATOMICS)
  फर्काऊ __sync_val_compare_and_swap(ptr, old_मान, new_मान);
#अथवायदि defined(_MSC_VER)
  फर्काऊ InterlockedCompareExchange(ptr, new_मान, old_मान);
#अन्यथा
#  error No compare-and-swap implementation लागि your platform!
#endif
}
