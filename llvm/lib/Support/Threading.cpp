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

#ضمن "llvm/Support/Threading.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/Jobserver.h"

#ضمن <cassert>
#ضمن <optional>
#ضمن <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#אם LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

using نطاق llvm;

uint64_t llvm::get_threadid() { החזר 0; }

uint32_t llvm::get_max_thread_name_length() { החזר 0; }

فراغ llvm::set_thread_name(ثابت Twine &Name) {}

فراغ llvm::get_thread_name(SmallVectorImpl<حرف> &Name) { Name.clear(); }

llvm::BitVector llvm::get_thread_affinity_mask() { החזר {}; }

unsigned llvm::ThreadPoolStrategy::compute_thread_count() ثابت {
  // When threads are disabled, ensure clients will loop at least once.
  החזר 1;
}

// Unknown if threading turned off
صحيح llvm::get_physical_cores() { החזר -1; }

#אחרת

ساكن صحيح computeHostNumHardwareThreads();

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ضمن "Unix/Threading.inc"
#endif
#ifdef _WIN32
#ضمن "Windows/Threading.inc"
#endif

using نطاق llvm;

unsigned llvm::ThreadPoolStrategy::compute_thread_count() ثابت {
  אם (UseJobserver)
    אם (auto JS = JobserverClient::getInstance())
      החזר JS->getNumJobs();

  صحيح MaxThreadCount =
      UseHyperThreads ? computeHostNumHardwareThreads() : get_physical_cores();
  אם (MaxThreadCount <= 0)
    MaxThreadCount = 1;
  אם (ThreadsRequested == 0)
    החזר MaxThreadCount;
  אם (!Limit)
    החזר ThreadsRequested;
  החזר std::min((unsigned)MaxThreadCount, ThreadsRequested);
}

#אם defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ثابت std::optional<unsigned> llvm::thread::DefaultStackSize = 8 * 1024 * 1024;
#وإلاإذا defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ثابت std::optional<unsigned> llvm::thread::DefaultStackSize = 4 * 1024 * 1024;
#אחרת
ثابت std::optional<unsigned> llvm::thread::DefaultStackSize;
#endif


#endif

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef Num, ThreadPoolStrategy Default) {
  אם (Num == "all")
    החזר llvm::hardware_concurrency();
  אם (Num.empty())
    החזר Default;
  unsigned V;
  אם (Num.getAsInteger(10, V))
    החזר std::nullopt; // malformed 'Num' value
  אם (V == 0)
    החזר Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  החזר S;
}
