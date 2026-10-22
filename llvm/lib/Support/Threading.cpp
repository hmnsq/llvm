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

#അന്തര്ഭാവ "llvm/Support/Threading.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h"
#അന്തര്ഭാവ "llvm/Support/Jobserver.h"

#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <optional>
#അന്തര്ഭാവ <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#എങ്കിൽ LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

using നാമാകാശ llvm;

uint64_t llvm::get_threadid() { തിരികെ 0; }

uint32_t llvm::get_max_thread_name_length() { തിരികെ 0; }

ശൂന്യം llvm::set_thread_name(സ്ഥിരം Twine &Name) {}

ശൂന്യം llvm::get_thread_name(SmallVectorImpl<അക്ഷരം> &Name) { Name.clear(); }

llvm::BitVector llvm::get_thread_affinity_mask() { തിരികെ {}; }

unsigned llvm::ThreadPoolStrategy::compute_thread_എണ്ണം() സ്ഥിരം {
  // When threads are disabled, ensure clients will loop at least once.
  തിരികെ 1;
}

// Unknown if threading turned off
പൂർണ്ണസംഖ്യ llvm::get_physical_cores() { തിരികെ -1; }

#അല്ലെങ്കിൽ

സ്ഥിതം പൂർണ്ണസംഖ്യ compute_Host_അക്കം_Hardware_Threads();

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#അന്തര്ഭാവ "Unix/Threading.inc"
#endif
#ifdef _WIN32
#അന്തര്ഭാവ "Windows/Threading.inc"
#endif

using നാമാകാശ llvm;

unsigned llvm::ThreadPoolStrategy::compute_thread_എണ്ണം() സ്ഥിരം {
  എങ്കിൽ (UseJobserver)
    എങ്കിൽ (auto JS = JobserverClient::getInstance())
      തിരികെ JS->get_അക്കം_Jobs();

  പൂർണ്ണസംഖ്യ Max_Thread_എണ്ണം =
      UseHyperThreads ? compute_Host_അക്കം_Hardware_Threads() : get_physical_cores();
  എങ്കിൽ (Max_Thread_എണ്ണം <= 0)
    Max_Thread_എണ്ണം = 1;
  എങ്കിൽ (ThreadsRequested == 0)
    തിരികെ Max_Thread_എണ്ണം;
  എങ്കിൽ (!Limit)
    തിരികെ ThreadsRequested;
  തിരികെ std::min((unsigned)Max_Thread_എണ്ണം, ThreadsRequested);
}

#എങ്കിൽ defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
സ്ഥിരം std::optional<unsigned> llvm::thread::Default_അട്ടി_Size = 8 * 1024 * 1024;
#അല്ലെങ്കിലെങ്കിൽ defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
സ്ഥിരം std::optional<unsigned> llvm::thread::Default_അട്ടി_Size = 4 * 1024 * 1024;
#അല്ലെങ്കിൽ
സ്ഥിരം std::optional<unsigned> llvm::thread::Default_അട്ടി_Size;
#endif


#endif

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef അക്കം, ThreadPoolStrategy Default) {
  എങ്കിൽ (അക്കം == "all")
    തിരികെ llvm::hardware_concurrency();
  എങ്കിൽ (അക്കം.empty())
    തിരികെ Default;
  unsigned V;
  എങ്കിൽ (അക്കം.getAsInteger(10, V))
    തിരികെ std::nullopt; // malformed 'Num' value
  എങ്കിൽ (V == 0)
    തിരികെ Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  തിരികെ S;
}
