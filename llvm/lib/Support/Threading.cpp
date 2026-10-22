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

#जर LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

using नामाकाश llvm;

uint64_t llvm::get_threadid() { परत 0; }

uint32_t llvm::get_max_thread_name_length() { परत 0; }

शून्य llvm::set_thread_name(स्थिरांक Twine &Name) {}

शून्य llvm::get_thread_name(SmallVectorImpl<अक्षर> &Name) { Name.clear(); }

llvm::BitVector llvm::get_thread_affinity_mask() { परत {}; }

अचिन्हित llvm::ThreadPoolStrategy::compute_thread_मोजणी() स्थिरांक {
  // When threads are disabled, ensure clients will loop at least once.
  परत 1;
}

// Unknown if threading turned off
पूर्णांक llvm::get_physical_cores() { परत -1; }

#अन्यथा

स्थिर पूर्णांक compute_Host_अंक_Hardware_Threads();

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#अन्तर्भाव "Unix/Threading.inc"
#endif
#ifdef _WIN32
#अन्तर्भाव "Windows/Threading.inc"
#endif

using नामाकाश llvm;

अचिन्हित llvm::ThreadPoolStrategy::compute_thread_मोजणी() स्थिरांक {
  जर (UseJobserver)
    जर (स्वतः JS = JobserverClient::getInstance())
      परत JS->get_अंक_Jobs();

  पूर्णांक Max_Thread_मोजणी =
      UseHyperThreads ? compute_Host_अंक_Hardware_Threads() : get_physical_cores();
  जर (Max_Thread_मोजणी <= 0)
    Max_Thread_मोजणी = 1;
  जर (ThreadsRequested == 0)
    परत Max_Thread_मोजणी;
  जर (!Limit)
    परत ThreadsRequested;
  परत std::min((अचिन्हित)Max_Thread_मोजणी, ThreadsRequested);
}

#जर defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
स्थिरांक std::optional<अचिन्हित> llvm::thread::Default_चळत_Size = 8 * 1024 * 1024;
#किंवाजर defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
स्थिरांक std::optional<अचिन्हित> llvm::thread::Default_चळत_Size = 4 * 1024 * 1024;
#अन्यथा
स्थिरांक std::optional<अचिन्हित> llvm::thread::Default_चळत_Size;
#endif


#endif

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef अंक, ThreadPoolStrategy Default) {
  जर (अंक == "all")
    परत llvm::hardware_concurrency();
  जर (अंक.empty())
    परत Default;
  अचिन्हित V;
  जर (अंक.getAsInteger(10, V))
    परत std::nullopt; // malformed 'Num' value
  जर (V == 0)
    परत Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  परत S;
}
