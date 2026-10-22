//===- llvm/Support/ExponentialBackoff.h ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ExponentialBackoff.h"
#ضمن <thread>

استعمال نطاق llvm;

منطقي قوت_نمائي_Backoff::wait_For_ايندڙ_Attempt() {
  خودڪار Now = std::chrono::steady_clock::now();
  إذا (Now >= پڇاڙي_وقت)
    أرجع خطأ;

  duration CurMaxWait = std::min(MinWait * CurrentMultiplier, MaxWait);
  std::هڪجهڙو_int_distribution<uint64_t> Dist(MinWait.ڳڻپ(),
                                               CurMaxWait.ڳڻپ());
  // Use random_device directly instead of a PRNG as uniform_int_distribution
  // often only takes a few samples anyway.
  duration WaitDuration = std::min(duration(Dist(RandDev)), پڇاڙي_وقت - Now);
  إذا (CurMaxWait < MaxWait)
    CurrentMultiplier *= 2;
  std::this_thread::sleep_for(WaitDuration);
  أرجع سچ;
}
