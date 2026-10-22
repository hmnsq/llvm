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

using فضاینام llvm;

#اگر defined(_MSC_VER)
#شامل <intrin.h>

// We must include windows.h after intrin.h.
#شامل <windows.h>
#undef MemoryFence
#endif

#اگر defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

تهی sys::MemoryFence() {
#اگر LLVM_HAS_ATOMICS == 0
  بازگشت;
#وگرنه
#  اگر defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  وگرنه
# error No memory fence implementation برای your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(فرار sys::cas_flag* ptr,
                                  sys::cas_flag new_مقدار,
                                  sys::cas_flag old_مقدار) {
#اگر LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  اگر (result == old_مقدار)
    *ptr = new_مقدار;
  بازگشت result;
#وگرنهاگر defined(GNU_ATOMICS)
  بازگشت __sync_val_compare_and_swap(ptr, old_مقدار, new_مقدار);
#وگرنهاگر defined(_MSC_VER)
  بازگشت InterlockedCompareExchange(ptr, new_مقدار, old_مقدار);
#وگرنه
#  error No compare-and-swap implementation برای your platform!
#endif
}
