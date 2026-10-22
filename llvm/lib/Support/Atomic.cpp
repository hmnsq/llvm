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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Atomic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

#ਜੇ defined(_MSC_VER)
#ਅਨ੍ਤਰ੍ਭਾਵ <intrin.h>

// We must include windows.h after intrin.h.
#ਅਨ੍ਤਰ੍ਭਾਵ <windows.h>
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਸਮ੍ਰਿਤੀ_Fence
#ਜੇ_ਅੰਤ

#ਜੇ defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GNU_ATOMICS
#ਜੇ_ਅੰਤ

ਖਾਲੀ sys::ਸਮ੍ਰਿਤੀ_Fence() {
#ਜੇ LLVM_HAS_ATOMICS == 0
  ਵਾਪਸ;
#ਨਹੀਂਤਾਂ
#  ਜੇ defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  ਸਮ੍ਰਿਤੀ_Barrier();
#  ਨਹੀਂਤਾਂ
# error No ਸਮ੍ਰਿਤੀ fence implementation ਲਈ your platform!
#  endif
#ਜੇ_ਅੰਤ
}

sys::cas_flag sys::Compare_And_ਅਦਲਾ_ਬਦਲੀ(ਅਸਥਿਰ sys::cas_flag* ptr,
                                  sys::cas_flag new_ਮੁੱਲ,
                                  sys::cas_flag old_ਮੁੱਲ) {
#ਜੇ LLVM_HAS_ATOMICS == 0
  sys::cas_flag ਨਤੀਜਾ_2 = *ptr;
  ਜੇ (ਨਤੀਜਾ_2 == old_ਮੁੱਲ)
    *ptr = new_ਮੁੱਲ;
  ਵਾਪਸ ਨਤੀਜਾ_2;
#ਜਾਂਜੇ defined(GNU_ATOMICS)
  ਵਾਪਸ __sync_val_compare_and_swap(ptr, old_ਮੁੱਲ, new_ਮੁੱਲ);
#ਜਾਂਜੇ defined(_MSC_VER)
  ਵਾਪਸ InterlockedCompareExchange(ptr, new_ਮੁੱਲ, old_ਮੁੱਲ);
#ਨਹੀਂਤਾਂ
#  error No compare-and-ਅਦਲਾ_ਬਦਲੀ implementation ਲਈ your platform!
#ਜੇ_ਅੰਤ
}
