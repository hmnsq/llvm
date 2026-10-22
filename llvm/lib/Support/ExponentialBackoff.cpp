//===- llvm/Support/ExponentialBackoff.h ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/ExponentialBackoff.h"
#അന്തര്ഭാവ <thread>

using നാമാകാശ llvm;

തർക്കം ExponentialBackoff::waitForNextAttempt() {
  auto Now = std::chrono::steady_clock::now();
  എങ്കിൽ (Now >= EndTime)
    തിരികെ അസത്യം;

  duration CurMaxWait = std::min(MinWait * CurrentMultiplier, MaxWait);
  std::uniform_int_distribution<uint64_t> Dist(MinWait.എണ്ണം(),
                                               CurMaxWait.എണ്ണം());
  // Use random_device directly instead of a PRNG as uniform_int_distribution
  // often only takes a few samples anyway.
  duration WaitDuration = std::min(duration(Dist(RandDev)), EndTime - Now);
  എങ്കിൽ (CurMaxWait < MaxWait)
    CurrentMultiplier *= 2;
  std::this_thread::sleep_for(WaitDuration);
  തിരികെ സത്യം;
}
