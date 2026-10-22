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

#અન્તર્ભાવ "llvm/Support/Threading.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h"
#અન્તર્ભાવ "llvm/Support/Jobserver.h"

#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <optional>
#અન્તર્ભાવ <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#જો LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

using નામાકાશ llvm;

uint64_t llvm::get_threadid() { પાછા 0; }

uint32_t llvm::get_max_thread_name_length() { પાછા 0; }

શૂન્ય llvm::set_thread_name(અચલ Twine &Name) {}

શૂન્ય llvm::get_thread_name(SmallVectorImpl<અક્ષર> &Name) { Name.clear(); }

llvm::BitVector llvm::get_thread_affinity_mask() { પાછા {}; }

unsigned llvm::ThreadPoolStrategy::compute_thread_ગણતરી() અચલ {
  // When threads are disabled, ensure clients will loop at least once.
  પાછા 1;
}

// Unknown if threading turned off
પૂર્ણાંક llvm::get_physical_cores() { પાછા -1; }

#નહીંતર

સ્થિર પૂર્ણાંક compute_Host_અંક_Hardware_Threads();

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#અન્તર્ભાવ "Unix/Threading.inc"
#endif
#ifdef _WIN32
#અન્તર્ભાવ "Windows/Threading.inc"
#endif

using નામાકાશ llvm;

unsigned llvm::ThreadPoolStrategy::compute_thread_ગણતરી() અચલ {
  જો (UseJobserver)
    જો (auto JS = JobserverClient::getInstance())
      પાછા JS->get_અંક_Jobs();

  પૂર્ણાંક Max_Thread_ગણતરી =
      UseHyperThreads ? compute_Host_અંક_Hardware_Threads() : get_physical_cores();
  જો (Max_Thread_ગણતરી <= 0)
    Max_Thread_ગણતરી = 1;
  જો (ThreadsRequested == 0)
    પાછા Max_Thread_ગણતરી;
  જો (!Limit)
    પાછા ThreadsRequested;
  પાછા std::min((unsigned)Max_Thread_ગણતરી, ThreadsRequested);
}

#જો defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
અચલ std::optional<unsigned> llvm::thread::Default_ઢગલો_Size = 8 * 1024 * 1024;
#અથવાજો defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
અચલ std::optional<unsigned> llvm::thread::Default_ઢગલો_Size = 4 * 1024 * 1024;
#નહીંતર
અચલ std::optional<unsigned> llvm::thread::Default_ઢગલો_Size;
#endif


#endif

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef અંક, ThreadPoolStrategy Default) {
  જો (અંક == "all")
    પાછા llvm::hardware_concurrency();
  જો (અંક.empty())
    પાછા Default;
  unsigned V;
  જો (અંક.getAsInteger(10, V))
    પાછા std::nullopt; // malformed 'Num' value
  જો (V == 0)
    પાછા Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  પાછા S;
}
