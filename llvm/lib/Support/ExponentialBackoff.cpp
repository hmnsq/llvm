//===- llvm/Support/ExponentialBackoff.h ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ExponentialBackoff.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <thread>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਤਾਰਕਿਕ ਘਾਤਾਂਕੀ_Backoff::wait_For_ਅਗਲਾ_Attempt() {
  ਸਵੈਚਾਲਿਤ Now = std::chrono::steady_clock::now();
  ਜੇ (Now >= ਸਿਰਾ_ਸਮਾਂ)
    ਵਾਪਸ ਝੂਠ;

  duration CurMaxWait = std::min(MinWait * CurrentMultiplier, MaxWait);
  std::ਇਕਸਾਰ_int_distribution<uint64_t> Dist(MinWait.ਗਿਣਤੀ_2(),
                                               CurMaxWait.ਗਿਣਤੀ_2());
  // Use random_device directly instead of a PRNG as uniform_int_distribution
  // often only takes a few samples anyway.
  duration WaitDuration = std::min(duration(Dist(RandDev)), ਸਿਰਾ_ਸਮਾਂ - Now);
  ਜੇ (CurMaxWait < MaxWait)
    CurrentMultiplier *= 2;
  std::this_thread::sleep_for(WaitDuration);
  ਵਾਪਸ ਸੱਚ;
}
