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

#شامل "llvm/Support/Threading.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/Jobserver.h"

#شامل <cassert>
#شامل <optional>
#شامل <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#اگر LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

using فضاینام llvm;

uint64_t llvm::get_threadid() { بازگشت 0; }

uint32_t llvm::get_max_thread_name_length() { بازگشت 0; }

تهی llvm::set_thread_name(ثابت Twine &Name) {}

تهی llvm::get_thread_name(SmallVectorImpl<نویسه> &Name) { Name.clear(); }

llvm::BitVector llvm::get_thread_affinity_mask() { بازگشت {}; }

unsigned llvm::ThreadPoolStrategy::compute_thread_شمار() ثابت {
  // When threads are disabled, ensure clients will loop at least once.
  بازگشت 1;
}

// Unknown if threading turned off
صحیح llvm::get_physical_cores() { بازگشت -1; }

#وگرنه

ایستا صحیح compute_Host_رقم_Hardware_Threads();

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#شامل "Unix/Threading.inc"
#endif
#ifdef _WIN32
#شامل "Windows/Threading.inc"
#endif

using فضاینام llvm;

unsigned llvm::ThreadPoolStrategy::compute_thread_شمار() ثابت {
  اگر (UseJobserver)
    اگر (auto JS = JobserverClient::getInstance())
      بازگشت JS->get_رقم_Jobs();

  صحیح Max_Thread_شمار =
      UseHyperThreads ? compute_Host_رقم_Hardware_Threads() : get_physical_cores();
  اگر (Max_Thread_شمار <= 0)
    Max_Thread_شمار = 1;
  اگر (ThreadsRequested == 0)
    بازگشت Max_Thread_شمار;
  اگر (!Limit)
    بازگشت ThreadsRequested;
  بازگشت std::min((unsigned)Max_Thread_شمار, ThreadsRequested);
}

#اگر defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ثابت std::optional<unsigned> llvm::thread::Default_پشته_Size = 8 * 1024 * 1024;
#وگرنهاگر defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ثابت std::optional<unsigned> llvm::thread::Default_پشته_Size = 4 * 1024 * 1024;
#وگرنه
ثابت std::optional<unsigned> llvm::thread::Default_پشته_Size;
#endif


#endif

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef رقم, ThreadPoolStrategy Default) {
  اگر (رقم == "all")
    بازگشت llvm::hardware_concurrency();
  اگر (رقم.empty())
    بازگشت Default;
  unsigned V;
  اگر (رقم.getAsInteger(10, V))
    بازگشت std::nullopt; // malformed 'Num' value
  اگر (V == 0)
    بازگشت Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  بازگشت S;
}
