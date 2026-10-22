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

#समावेश "llvm/Support/Atomic.h"
#समावेश "llvm/Config/llvm-config.h"

using namespace llvm;

#अगर defined(_MSC_VER)
#समावेश <intrin.h>

// We must include windows.h after intrin.h.
#समावेश <windows.h>
#अघोषित MemoryFence
#पूर्ण_अगर

#अगर defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#घोषणा GNU_ATOMICS
#पूर्ण_अगर

व्योम sys::MemoryFence() {
#अगर LLVM_HAS_ATOMICS == 0
  वापस;
#अन्यथा
#  अगर defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  अन्यथा
# error No memory fence implementation क्रम your platform!
#  endif
#पूर्ण_अगर
}

sys::cas_flag sys::CompareAndSwap(अस्थिर sys::cas_flag* ptr,
                                  sys::cas_flag new_मूल्य,
                                  sys::cas_flag old_मूल्य) {
#अगर LLVM_HAS_ATOMICS == 0
  sys::cas_flag परिणाम_2 = *ptr;
  अगर (परिणाम_2 == old_मूल्य)
    *ptr = new_मूल्य;
  वापस परिणाम_2;
#या_अगर defined(GNU_ATOMICS)
  वापस __sync_val_compare_and_swap(ptr, old_मूल्य, new_मूल्य);
#या_अगर defined(_MSC_VER)
  वापस InterlockedCompareExchange(ptr, new_मूल्य, old_मूल्य);
#अन्यथा
#  error No compare-and-swap implementation क्रम your platform!
#पूर्ण_अगर
}
