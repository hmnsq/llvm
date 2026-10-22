//===- llvm/Support/ExponentialBackoff.h ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/ExponentialBackoff.h"
#ଅନ୍ତର୍ଭାଵ <thread>

using ନାମାକାଶ llvm;

ତାର୍କିକ ExponentialBackoff::waitForNextAttempt() {
  auto Now = std::chrono::steady_clock::now();
  ଯଦି (Now >= EndTime)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  duration CurMaxWait = std::min(MinWait * CurrentMultiplier, MaxWait);
  std::uniform_int_distribution<uint64_t> Dist(MinWait.ଗଣନା_2(),
                                               CurMaxWait.ଗଣନା_2());
  // Use random_device directly instead of a PRNG as uniform_int_distribution
  // often only takes a few samples anyway.
  duration WaitDuration = std::min(duration(Dist(RandDev)), EndTime - Now);
  ଯଦି (CurMaxWait < MaxWait)
    CurrentMultiplier *= 2;
  std::this_thread::sleep_for(WaitDuration);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}
