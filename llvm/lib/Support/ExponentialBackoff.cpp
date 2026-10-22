//===- llvm/Support/ExponentialBackoff.h ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/ExponentialBackoff.h"
#अन्तर्भाव <thread>

using नामाकाश llvm;

तार्किक ExponentialBackoff::waitForNextAttempt() {
  auto Now = std::chrono::steady_clock::now();
  सचे (Now >= EndTime)
    पटिनिवत्त मिच्छा;

  duration CurMaxWait = std::min(MinWait * CurrentMultiplier, MaxWait);
  std::uniform_int_distribution<uint64_t> Dist(MinWait.गणनम्(),
                                               CurMaxWait.गणनम्());
  // Use random_device directly instead of a PRNG as uniform_int_distribution
  // often only takes a few samples anyway.
  duration WaitDuration = std::min(duration(Dist(RandDev)), EndTime - Now);
  सचे (CurMaxWait < MaxWait)
    CurrentMultiplier *= 2;
  std::this_thread::sleep_for(WaitDuration);
  पटिनिवत्त सच्च;
}
