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

#ಅನ್ತರ್ಭಾವ "llvm/Support/Threading.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Jobserver.h"

#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <optional>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#ಯದಿ LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

using ನಾಮಾಕಾಶ llvm;

uint64_t llvm::get_threadid() { ಪ್ರತ್ಯಾಗಮ 0; }

uint32_t llvm::get_max_thread_name_length() { ಪ್ರತ್ಯಾಗಮ 0; }

ಶೂನ್ಯ llvm::set_thread_name(ಧ್ರುವ Twine &Name) {}

ಶೂನ್ಯ llvm::get_thread_name(SmallVectorImpl<ವರ್ಣ> &Name) { Name.clear(); }

llvm::BitVector llvm::get_thread_affinity_mask() { ಪ್ರತ್ಯಾಗಮ {}; }

unsigned llvm::ThreadPoolStrategy::compute_thread_ಎಣಿಕೆ() ಧ್ರುವ {
  // When threads are disabled, ensure clients will loop at least once.
  ಪ್ರತ್ಯಾಗಮ 1;
}

// Unknown if threading turned off
ಪೂರ್ಣಾಙ್ಕ llvm::get_physical_cores() { ಪ್ರತ್ಯಾಗಮ -1; }

#ಅನ್ಯಥಾ

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ compute_Host_ಅಂಕೆ_Hardware_Threads();

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ಅನ್ತರ್ಭಾವ "Unix/Threading.inc"
#endif
#ifdef _WIN32
#ಅನ್ತರ್ಭಾವ "Windows/Threading.inc"
#endif

using ನಾಮಾಕಾಶ llvm;

unsigned llvm::ThreadPoolStrategy::compute_thread_ಎಣಿಕೆ() ಧ್ರುವ {
  ಯದಿ (UseJobserver)
    ಯದಿ (auto JS = JobserverClient::getInstance())
      ಪ್ರತ್ಯಾಗಮ JS->get_ಅಂಕೆ_Jobs();

  ಪೂರ್ಣಾಙ್ಕ Max_Thread_ಎಣಿಕೆ =
      UseHyperThreads ? compute_Host_ಅಂಕೆ_Hardware_Threads() : get_physical_cores();
  ಯದಿ (Max_Thread_ಎಣಿಕೆ <= 0)
    Max_Thread_ಎಣಿಕೆ = 1;
  ಯದಿ (ThreadsRequested == 0)
    ಪ್ರತ್ಯಾಗಮ Max_Thread_ಎಣಿಕೆ;
  ಯದಿ (!Limit)
    ಪ್ರತ್ಯಾಗಮ ThreadsRequested;
  ಪ್ರತ್ಯಾಗಮ std::min((unsigned)Max_Thread_ಎಣಿಕೆ, ThreadsRequested);
}

#ಯದಿ defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ಧ್ರುವ std::optional<unsigned> llvm::thread::Default_ರಾಶಿ_Size = 8 * 1024 * 1024;
#ನೋಚೇದ್ಯದಿ defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ಧ್ರುವ std::optional<unsigned> llvm::thread::Default_ರಾಶಿ_Size = 4 * 1024 * 1024;
#ಅನ್ಯಥಾ
ಧ್ರುವ std::optional<unsigned> llvm::thread::Default_ರಾಶಿ_Size;
#endif


#endif

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef ಅಂಕೆ, ThreadPoolStrategy Default) {
  ಯದಿ (ಅಂಕೆ == "all")
    ಪ್ರತ್ಯಾಗಮ llvm::hardware_concurrency();
  ಯದಿ (ಅಂಕೆ.empty())
    ಪ್ರತ್ಯಾಗಮ Default;
  unsigned V;
  ಯದಿ (ಅಂಕೆ.getAsInteger(10, V))
    ಪ್ರತ್ಯಾಗಮ std::nullopt; // malformed 'Num' value
  ಯದಿ (V == 0)
    ಪ್ರತ್ಯಾಗಮ Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  ಪ್ರತ್ಯಾಗಮ S;
}
