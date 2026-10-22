//===- llvm/Support/ExponentialBackoff.h ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/ExponentialBackoff.h"
#అన్తర్భావ <thread>

ఉపయోగించి నామాకాశ llvm;

తార్కిక ఘాతాంక_Backoff::wait_For_తదుపరి_Attempt() {
  స్వయంచాలక Now = std::chrono::steady_clock::now();
  ఒకవేళ (Now >= చివర_సమయం)
    తిరిగి అబద్ధం;

  duration CurMaxWait = std::min(MinWait * CurrentMultiplier, MaxWait);
  std::ఏకరీతి_int_distribution<uint64_t> Dist(MinWait.లెక్క(),
                                               CurMaxWait.లెక్క());
  // Use random_device directly instead of a PRNG as uniform_int_distribution
  // often only takes a few samples anyway.
  duration WaitDuration = std::min(duration(Dist(RandDev)), చివర_సమయం - Now);
  ఒకవేళ (CurMaxWait < MaxWait)
    CurrentMultiplier *= 2;
  std::this_thread::sleep_for(WaitDuration);
  తిరిగి నిజం;
}
