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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Threading.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Jobserver.h"

#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <optional>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#ଯଦି LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

using ନାମାକାଶ llvm;

uint64_t llvm::get_threadid() { ଫେରନ୍ତୁ 0; }

uint32_t llvm::get_max_thread_name_length() { ଫେରନ୍ତୁ 0; }

ଶୂନ୍ୟ llvm::set_thread_name(ଅଚଳ Twine &Name) {}

ଶୂନ୍ୟ llvm::get_thread_name(SmallVectorImpl<ଅକ୍ଷର> &Name) { Name.clear(); }

llvm::BitVector llvm::get_thread_affinity_mask() { ଫେରନ୍ତୁ {}; }

unsigned llvm::ThreadPoolStrategy::compute_thread_ଗଣନା() ଅଚଳ {
  // When threads are disabled, ensure clients will loop at least once.
  ଫେରନ୍ତୁ 1;
}

// Unknown if threading turned off
ପୂର୍ଣ୍ଣସଂଖ୍ୟା llvm::get_physical_cores() { ଫେରନ୍ତୁ -1; }

#ଅନ୍ୟଥା

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା compute_Host_ଅଙ୍କ_Hardware_Threads();

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ଅନ୍ତର୍ଭାଵ "Unix/Threading.inc"
#endif
#ifdef _WIN32
#ଅନ୍ତର୍ଭାଵ "Windows/Threading.inc"
#endif

using ନାମାକାଶ llvm;

unsigned llvm::ThreadPoolStrategy::compute_thread_ଗଣନା() ଅଚଳ {
  ଯଦି (UseJobserver)
    ଯଦି (auto JS = JobserverClient::getInstance())
      ଫେରନ୍ତୁ JS->get_ଅଙ୍କ_Jobs();

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Max_Thread_ଗଣନା =
      UseHyperThreads ? compute_Host_ଅଙ୍କ_Hardware_Threads() : get_physical_cores();
  ଯଦି (Max_Thread_ଗଣନା <= 0)
    Max_Thread_ଗଣନା = 1;
  ଯଦି (ThreadsRequested == 0)
    ଫେରନ୍ତୁ Max_Thread_ଗଣନା;
  ଯଦି (!Limit)
    ଫେରନ୍ତୁ ThreadsRequested;
  ଫେରନ୍ତୁ std::min((unsigned)Max_Thread_ଗଣନା, ThreadsRequested);
}

#ଯଦି defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ଅଚଳ std::optional<unsigned> llvm::thread::Default_ଗଦା_Size = 8 * 1024 * 1024;
#ନଚେତଯଦି defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ଅଚଳ std::optional<unsigned> llvm::thread::Default_ଗଦା_Size = 4 * 1024 * 1024;
#ଅନ୍ୟଥା
ଅଚଳ std::optional<unsigned> llvm::thread::Default_ଗଦା_Size;
#endif


#endif

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef ଅଙ୍କ, ThreadPoolStrategy Default) {
  ଯଦି (ଅଙ୍କ == "all")
    ଫେରନ୍ତୁ llvm::hardware_concurrency();
  ଯଦି (ଅଙ୍କ.empty())
    ଫେରନ୍ତୁ Default;
  unsigned V;
  ଯଦି (ଅଙ୍କ.getAsInteger(10, V))
    ଫେରନ୍ତୁ std::nullopt; // malformed 'Num' value
  ଯଦି (V == 0)
    ଫେରନ୍ତୁ Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  ଫେରନ୍ତୁ S;
}
