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

#அந்தர்பாவ "llvm/Support/Threading.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"
#அந்தர்பாவ "llvm/Support/Jobserver.h"

#அந்தர்பாவ <cassert>
#அந்தர்பாவ <optional>
#அந்தர்பாவ <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#எனில் LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

using நாமாகாஶ llvm;

uint64_t llvm::get_threadid() { திரும்பு 0; }

uint32_t llvm::get_max_thread_name_length() { திரும்பு 0; }

வெறுமை llvm::set_thread_name(மாறிலி Twine &Name) {}

வெறுமை llvm::get_thread_name(SmallVectorImpl<எழுத்து> &Name) { Name.clear(); }

llvm::BitVector llvm::get_thread_affinity_mask() { திரும்பு {}; }

unsigned llvm::ThreadPoolStrategy::compute_thread_மொத்தம்() மாறிலி {
  // When threads are disabled, ensure clients will loop at least once.
  திரும்பு 1;
}

// Unknown if threading turned off
முழு llvm::get_physical_cores() { திரும்பு -1; }

#இல்லையெனில்

நிலையான முழு compute_Host_இலக்கம்_Hardware_Threads();

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#அந்தர்பாவ "Unix/Threading.inc"
#endif
#ifdef _WIN32
#அந்தர்பாவ "Windows/Threading.inc"
#endif

using நாமாகாஶ llvm;

unsigned llvm::ThreadPoolStrategy::compute_thread_மொத்தம்() மாறிலி {
  எனில் (UseJobserver)
    எனில் (auto JS = JobserverClient::getInstance())
      திரும்பு JS->get_இலக்கம்_Jobs();

  முழு Max_Thread_மொத்தம் =
      UseHyperThreads ? compute_Host_இலக்கம்_Hardware_Threads() : get_physical_cores();
  எனில் (Max_Thread_மொத்தம் <= 0)
    Max_Thread_மொத்தம் = 1;
  எனில் (ThreadsRequested == 0)
    திரும்பு Max_Thread_மொத்தம்;
  எனில் (!Limit)
    திரும்பு ThreadsRequested;
  திரும்பு std::min((unsigned)Max_Thread_மொத்தம், ThreadsRequested);
}

#எனில் defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
மாறிலி std::optional<unsigned> llvm::thread::Default_அடுக்கு_Size = 8 * 1024 * 1024;
#அல்லதெனில் defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
மாறிலி std::optional<unsigned> llvm::thread::Default_அடுக்கு_Size = 4 * 1024 * 1024;
#இல்லையெனில்
மாறிலி std::optional<unsigned> llvm::thread::Default_அடுக்கு_Size;
#endif


#endif

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef இலக்கம், ThreadPoolStrategy Default) {
  எனில் (இலக்கம் == "all")
    திரும்பு llvm::hardware_concurrency();
  எனில் (இலக்கம்.empty())
    திரும்பு Default;
  unsigned V;
  எனில் (இலக்கம்.getAsInteger(10, V))
    திரும்பு std::nullopt; // malformed 'Num' value
  எனில் (V == 0)
    திரும்பு Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  திரும்பு S;
}
