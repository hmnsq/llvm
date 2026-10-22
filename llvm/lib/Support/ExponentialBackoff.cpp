//===- llvm/Support/ExponentialBackoff.h ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/ExponentialBackoff.h"
#અન્તર્ભાવ <thread>

using નામાકાશ llvm;

તાર્કિક ExponentialBackoff::waitForNextAttempt() {
  auto Now = std::chrono::steady_clock::now();
  જો (Now >= EndTime)
    પાછા ખોટું;

  duration CurMaxWait = std::min(MinWait * CurrentMultiplier, MaxWait);
  std::uniform_int_distribution<uint64_t> Dist(MinWait.ગણતરી(),
                                               CurMaxWait.ગણતરી());
  // Use random_device directly instead of a PRNG as uniform_int_distribution
  // often only takes a few samples anyway.
  duration WaitDuration = std::min(duration(Dist(RandDev)), EndTime - Now);
  જો (CurMaxWait < MaxWait)
    CurrentMultiplier *= 2;
  std::this_thread::sleep_for(WaitDuration);
  પાછા સાચું;
}
