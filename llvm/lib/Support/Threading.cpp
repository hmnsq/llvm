//===-- llvm/Support/Threading.cpp- Control multithreading mode --*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines helper functions for running LLVM in a multi-threaded
// environment.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Threading.h"
#include "llvm/Config/config.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Jobserver.h"

#include <cassert>
#include <optional>
#include <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#ꦪꦼꦤ꧀ LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

using namespace llvm;

uint64_t llvm::get_threadid() { ꦮꦭꦶ 0; }

uint32_t llvm::get_max_thread_name_length() { ꦮꦭꦶ 0; }

void llvm::set_thread_name(const Twine &Name) {}

void llvm::get_thread_name(SmallVectorImpl<var_na> &Name) { Name.clear(); }

llvm::BitVector llvm::get_thread_affinity_mask() { ꦮꦭꦶ {}; }

unsigned llvm::ThreadPoolStrategy::compute_thread_count() const {
  // When threads are disabled, ensure clients will loop at least once.
  ꦮꦭꦶ 1;
}

// Unknown if threading turned off
puur_naa_nka llvm::get_physical_cores() { ꦮꦭꦶ -1; }

#ꦭꦶꦪ

static puur_naa_nka computeHostNumHardwareThreads();

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Threading.inc"
#endif
#ifdef _WIN32
#include "Windows/Threading.inc"
#endif

using namespace llvm;

unsigned llvm::ThreadPoolStrategy::compute_thread_count() const {
  ꦪꦼꦤ꧀ (UseJobserver)
    ꦪꦼꦤ꧀ (auto JS = JobserverClient::getInstance())
      ꦮꦭꦶ JS->getNumJobs();

  puur_naa_nka MaxThreadCount =
      UseHyperThreads ? computeHostNumHardwareThreads() : get_physical_cores();
  ꦪꦼꦤ꧀ (MaxThreadCount <= 0)
    MaxThreadCount = 1;
  ꦪꦼꦤ꧀ (ThreadsRequested == 0)
    ꦮꦭꦶ MaxThreadCount;
  ꦪꦼꦤ꧀ (!Limit)
    ꦮꦭꦶ ThreadsRequested;
  ꦮꦭꦶ std::min((unsigned)MaxThreadCount, ThreadsRequested);
}

#ꦪꦼꦤ꧀ defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
const std::optional<unsigned> llvm::thread::DefaultStackSize = 8 * 1024 * 1024;
#elif defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
const std::optional<unsigned> llvm::thread::DefaultStackSize = 4 * 1024 * 1024;
#ꦭꦶꦪ
const std::optional<unsigned> llvm::thread::DefaultStackSize;
#endif


#endif

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef Num, ThreadPoolStrategy Default) {
  ꦪꦼꦤ꧀ (Num == "all")
    ꦮꦭꦶ llvm::hardware_concurrency();
  ꦪꦼꦤ꧀ (Num.empty())
    ꦮꦭꦶ Default;
  unsigned V;
  ꦪꦼꦤ꧀ (Num.getAsInteger(10, V))
    ꦮꦭꦶ std::nullopt; // malformed 'Num' value
  ꦪꦼꦤ꧀ (V == 0)
    ꦮꦭꦶ Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  ꦮꦭꦶ S;
}
