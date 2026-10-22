//===- llvm/Support/ExponentialBackoff.h ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/ExponentialBackoff.h"
#شامل <thread>

استعمال نامخلا llvm;

منطقی قوت_نمائی_Backoff::wait_For_اگلا_Attempt() {
  خودکار Now = std::chrono::steady_clock::now();
  اگر (Now >= انتہا_وقت)
    واپس جھوٹ;

  duration CurMaxWait = std::min(MinWait * CurrentMultiplier, MaxWait);
  std::یکساں_int_distribution<uint64_t> Dist(MinWait.گنتی(),
                                               CurMaxWait.گنتی());
  // Use random_device directly instead of a PRNG as uniform_int_distribution
  // often only takes a few samples anyway.
  duration WaitDuration = std::min(duration(Dist(RandDev)), انتہا_وقت - Now);
  اگر (CurMaxWait < MaxWait)
    CurrentMultiplier *= 2;
  std::this_thread::sleep_for(WaitDuration);
  واپس سچ;
}
