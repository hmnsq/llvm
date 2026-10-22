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

#अन्तर्भाव "llvm/Support/Threading.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"
#अन्तर्भाव "llvm/Support/Jobserver.h"

#अन्तर्भाव <cassert>
#अन्तर्भाव <optional>
#अन्तर्भाव <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#यदि LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

using नामाकाश llvm;

uint64_t llvm::get_threadid() { प्रत्यागम 0; }

uint32_t llvm::get_max_thread_name_length() { प्रत्यागम 0; }

शून्य llvm::set_thread_name(ध्रुव Twine &Name) {}

शून्य llvm::get_thread_name(SmallVectorImpl<वर्ण> &Name) { Name.clear(); }

llvm::BitVector llvm::get_thread_affinity_mask() { प्रत्यागम {}; }

अचिन्हित llvm::ThreadPoolStrategy::compute_thread_गिनती() ध्रुव {
  // When threads are disabled, ensure clients will loop at least once.
  प्रत्यागम 1;
}

// Unknown if threading turned off
पूर्णाङ्क llvm::get_physical_cores() { प्रत्यागम -1; }

#अन्यथा

स्थिर पूर्णाङ्क compute_Host_अंक_Hardware_Threads();

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#अन्तर्भाव "Unix/Threading.inc"
#endif
#ifdef _WIN32
#अन्तर्भाव "Windows/Threading.inc"
#endif

using नामाकाश llvm;

अचिन्हित llvm::ThreadPoolStrategy::compute_thread_गिनती() ध्रुव {
  यदि (UseJobserver)
    यदि (स्वतः JS = JobserverClient::getInstance())
      प्रत्यागम JS->get_अंक_Jobs();

  पूर्णाङ्क Max_Thread_गिनती =
      UseHyperThreads ? compute_Host_अंक_Hardware_Threads() : get_physical_cores();
  यदि (Max_Thread_गिनती <= 0)
    Max_Thread_गिनती = 1;
  यदि (ThreadsRequested == 0)
    प्रत्यागम Max_Thread_गिनती;
  यदि (!Limit)
    प्रत्यागम ThreadsRequested;
  प्रत्यागम std::min((अचिन्हित)Max_Thread_गिनती, ThreadsRequested);
}

#यदि defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ध्रुव std::optional<अचिन्हित> llvm::thread::Default_ढेर_Size = 8 * 1024 * 1024;
#नोचेद्यदि defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ध्रुव std::optional<अचिन्हित> llvm::thread::Default_ढेर_Size = 4 * 1024 * 1024;
#अन्यथा
ध्रुव std::optional<अचिन्हित> llvm::thread::Default_ढेर_Size;
#endif


#endif

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef अंक, ThreadPoolStrategy Default) {
  यदि (अंक == "all")
    प्रत्यागम llvm::hardware_concurrency();
  यदि (अंक.empty())
    प्रत्यागम Default;
  अचिन्हित V;
  यदि (अंक.getAsInteger(10, V))
    प्रत्यागम std::nullopt; // malformed 'Num' value
  यदि (V == 0)
    प्रत्यागम Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  प्रत्यागम S;
}
