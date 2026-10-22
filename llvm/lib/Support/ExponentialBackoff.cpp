//===- llvm/Support/ExponentialBackoff.h ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/ExponentialBackoff.h"
#অন্তর্ভাব <thread>

using নামাকাশ llvm;

তার্কিক ExponentialBackoff::waitForNextAttempt() {
  auto Now = std::chrono::steady_clock::now();
  যদি (Now >= EndTime)
    প্রত্যাগম অসত্য;

  duration CurMaxWait = std::min(MinWait * CurrentMultiplier, MaxWait);
  std::uniform_int_distribution<uint64_t> Dist(MinWait.গুনতি(),
                                               CurMaxWait.গুনতি());
  // Use random_device directly instead of a PRNG as uniform_int_distribution
  // often only takes a few samples anyway.
  duration WaitDuration = std::min(duration(Dist(RandDev)), EndTime - Now);
  যদি (CurMaxWait < MaxWait)
    CurrentMultiplier *= 2;
  std::this_thread::sleep_for(WaitDuration);
  প্রত্যাগম সত্য;
}
