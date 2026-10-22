//===- llvm/Support/ExponentialBackoff.h ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/ExponentialBackoff.h"
#समावेश <thread>

using namespace llvm;

bool ExponentialBackoff::wait_For_अग्र_Attempt() {
  स्वतः Now = std::chrono::steady_clock::now();
  अगर (Now >= EndTime)
    वापस false;

  duration CurMaxWait = std::min(MinWait * CurrentMultiplier, MaxWait);
  std::uniform_int_distribution<uint64_t> Dist(MinWait.गिनती(),
                                               CurMaxWait.गिनती());
  // Use random_device directly instead of a PRNG as uniform_int_distribution
  // often only takes a few samples anyway.
  duration WaitDuration = std::min(duration(Dist(RandDev)), EndTime - Now);
  अगर (CurMaxWait < MaxWait)
    CurrentMultiplier *= 2;
  std::this_thread::sleep_for(WaitDuration);
  वापस true;
}
