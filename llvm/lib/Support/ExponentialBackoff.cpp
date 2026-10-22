//===- llvm/Support/ExponentialBackoff.h ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/ExponentialBackoff.h"
#அந்தர்பாவ <thread>

using நாமாகாஶ llvm;

தருக்க ExponentialBackoff::waitForNextAttempt() {
  auto Now = std::chrono::steady_clock::now();
  எனில் (Now >= EndTime)
    திரும்பு பொய்;

  duration CurMaxWait = std::min(MinWait * CurrentMultiplier, MaxWait);
  std::uniform_int_distribution<uint64_t> Dist(MinWait.மொத்தம்(),
                                               CurMaxWait.மொத்தம்());
  // Use random_device directly instead of a PRNG as uniform_int_distribution
  // often only takes a few samples anyway.
  duration WaitDuration = std::min(duration(Dist(RandDev)), EndTime - Now);
  எனில் (CurMaxWait < MaxWait)
    CurrentMultiplier *= 2;
  std::this_thread::sleep_for(WaitDuration);
  திரும்பு உண்மை;
}
