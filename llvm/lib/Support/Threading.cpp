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

#অন্তর্ভাৱ "llvm/Support/Threading.h"
#অন্তর্ভাৱ "llvm/Config/config.h"
#অন্তর্ভাৱ "llvm/Config/llvm-config.h"
#অন্তর্ভাৱ "llvm/Support/Jobserver.h"

#অন্তর্ভাৱ <cassert>
#অন্তর্ভাৱ <optional>
#অন্তর্ভাৱ <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#যদি LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

using নামাকাশ llvm;

uint64_t llvm::get_threadid() { ঘূৰাই 0; }

uint32_t llvm::get_max_thread_name_length() { ঘূৰাই 0; }

শূন্য llvm::set_thread_name(ধ্রুৱ Twine &Name) {}

শূন্য llvm::get_thread_name(SmallVectorImpl<ৱর্ণ> &Name) { Name.clear(); }

llvm::BitVector llvm::get_thread_affinity_mask() { ঘূৰাই {}; }

unsigned llvm::ThreadPoolStrategy::compute_thread_গুনতি() ধ্রুৱ {
  // When threads are disabled, ensure clients will loop at least once.
  ঘূৰাই 1;
}

// Unknown if threading turned off
পূর্ণাঙ্ক llvm::get_physical_cores() { ঘূৰাই -1; }

#অন্যথা

স্থির পূর্ণাঙ্ক compute_Host_অঙ্ক_Hardware_Threads();

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#অন্তর্ভাৱ "Unix/Threading.inc"
#endif
#ifdef _WIN32
#অন্তর্ভাৱ "Windows/Threading.inc"
#endif

using নামাকাশ llvm;

unsigned llvm::ThreadPoolStrategy::compute_thread_গুনতি() ধ্রুৱ {
  যদি (UseJobserver)
    যদি (auto JS = JobserverClient::getInstance())
      ঘূৰাই JS->get_অঙ্ক_Jobs();

  পূর্ণাঙ্ক Max_Thread_গুনতি =
      UseHyperThreads ? compute_Host_অঙ্ক_Hardware_Threads() : get_physical_cores();
  যদি (Max_Thread_গুনতি <= 0)
    Max_Thread_গুনতি = 1;
  যদি (ThreadsRequested == 0)
    ঘূৰাই Max_Thread_গুনতি;
  যদি (!Limit)
    ঘূৰাই ThreadsRequested;
  ঘূৰাই std::min((unsigned)Max_Thread_গুনতি, ThreadsRequested);
}

#যদি defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ধ্রুৱ std::optional<unsigned> llvm::thread::Default_স্তূপ_Size = 8 * 1024 * 1024;
#নোচেদ্যদি defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ধ্রুৱ std::optional<unsigned> llvm::thread::Default_স্তূপ_Size = 4 * 1024 * 1024;
#অন্যথা
ধ্রুৱ std::optional<unsigned> llvm::thread::Default_স্তূপ_Size;
#endif


#endif

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef অঙ্ক, ThreadPoolStrategy Default) {
  যদি (অঙ্ক == "all")
    ঘূৰাই llvm::hardware_concurrency();
  যদি (অঙ্ক.empty())
    ঘূৰাই Default;
  unsigned V;
  যদি (অঙ্ক.getAsInteger(10, V))
    ঘূৰাই std::nullopt; // malformed 'Num' value
  যদি (V == 0)
    ঘূৰাই Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  ঘূৰাই S;
}
