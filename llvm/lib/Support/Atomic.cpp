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

#அந்தர்பாவ "llvm/Support/Atomic.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"

using நாமாகாஶ llvm;

#எனில் defined(_MSC_VER)
#அந்தர்பாவ <intrin.h>

// We must include windows.h after intrin.h.
#அந்தர்பாவ <windows.h>
#undef MemoryFence
#endif

#எனில் defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

வெறுமை sys::MemoryFence() {
#எனில் LLVM_HAS_ATOMICS == 0
  திரும்பு;
#இல்லையெனில்
#  எனில் defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  இல்லையெனில்
# error No memory fence implementation க்கு your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(நிலையற்ற sys::cas_flag* ptr,
                                  sys::cas_flag new_மதிப்பு,
                                  sys::cas_flag old_மதிப்பு) {
#எனில் LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  எனில் (result == old_மதிப்பு)
    *ptr = new_மதிப்பு;
  திரும்பு result;
#அல்லதெனில் defined(GNU_ATOMICS)
  திரும்பு __sync_val_compare_and_swap(ptr, old_மதிப்பு, new_மதிப்பு);
#அல்லதெனில் defined(_MSC_VER)
  திரும்பு InterlockedCompareExchange(ptr, new_மதிப்பு, old_மதிப்பு);
#இல்லையெனில்
#  error No compare-and-swap implementation க்கு your platform!
#endif
}
