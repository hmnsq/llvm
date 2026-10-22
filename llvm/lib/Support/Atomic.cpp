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

#അന്തര്ഭാവ "llvm/Support/Atomic.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h"

using നാമാകാശ llvm;

#എങ്കിൽ defined(_MSC_VER)
#അന്തര്ഭാവ <intrin.h>

// We must include windows.h after intrin.h.
#അന്തര്ഭാവ <windows.h>
#undef MemoryFence
#endif

#എങ്കിൽ defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

ശൂന്യം sys::MemoryFence() {
#എങ്കിൽ LLVM_HAS_ATOMICS == 0
  തിരികെ;
#അല്ലെങ്കിൽ
#  എങ്കിൽ defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  അല്ലെങ്കിൽ
# error No memory fence implementation വേണ്ടി your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(അസ്ഥിര sys::cas_flag* ptr,
                                  sys::cas_flag new_മൂല്യം,
                                  sys::cas_flag old_മൂല്യം) {
#എങ്കിൽ LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  എങ്കിൽ (result == old_മൂല്യം)
    *ptr = new_മൂല്യം;
  തിരികെ result;
#അല്ലെങ്കിലെങ്കിൽ defined(GNU_ATOMICS)
  തിരികെ __sync_val_compare_and_swap(ptr, old_മൂല്യം, new_മൂല്യം);
#അല്ലെങ്കിലെങ്കിൽ defined(_MSC_VER)
  തിരികെ InterlockedCompareExchange(ptr, new_മൂല്യം, old_മൂല്യം);
#അല്ലെങ്കിൽ
#  error No compare-and-swap implementation വേണ്ടി your platform!
#endif
}
