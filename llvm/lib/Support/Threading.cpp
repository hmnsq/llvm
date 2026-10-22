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

uint64_t llvm::get_threadid() { फर्काऊ 0; }

uint32_t llvm::get_max_thread_name_length() { फर्काऊ 0; }

शून्य llvm::set_thread_name(अचर Twine &Name) {}

शून्य llvm::get_thread_name(SmallVectorImpl<वर्ण> &Name) { Name.clear(); }

llvm::BitVector llvm::get_thread_affinity_mask() { फर्काऊ {}; }

अचिन्हित llvm::ThreadPoolStrategy::compute_thread_गन्ती() अचर {
  // When threads are disabled, ensure clients will loop at least once.
  फर्काऊ 1;
}

// Unknown if threading turned off
पूर्णाङ्क llvm::get_physical_cores() { फर्काऊ -1; }

#अन्यथा

स्थिर पूर्णाङ्क compute_Host_अङ्क_Hardware_Threads();

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#अन्तर्भाव "Unix/Threading.inc"
#endif
#ifdef _WIN32
#अन्तर्भाव "Windows/Threading.inc"
#endif

using नामाकाश llvm;

अचिन्हित llvm::ThreadPoolStrategy::compute_thread_गन्ती() अचर {
  यदि (UseJobserver)
    यदि (स्वतः JS = JobserverClient::getInstance())
      फर्काऊ JS->get_अङ्क_Jobs();

  पूर्णाङ्क Max_Thread_गन्ती =
      UseHyperThreads ? compute_Host_अङ्क_Hardware_Threads() : get_physical_cores();
  यदि (Max_Thread_गन्ती <= 0)
    Max_Thread_गन्ती = 1;
  यदि (ThreadsRequested == 0)
    फर्काऊ Max_Thread_गन्ती;
  यदि (!Limit)
    फर्काऊ ThreadsRequested;
  फर्काऊ std::min((अचिन्हित)Max_Thread_गन्ती, ThreadsRequested);
}

#यदि defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
अचर std::optional<अचिन्हित> llvm::thread::Default_थाक_Size = 8 * 1024 * 1024;
#अथवायदि defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
अचर std::optional<अचिन्हित> llvm::thread::Default_थाक_Size = 4 * 1024 * 1024;
#अन्यथा
अचर std::optional<अचिन्हित> llvm::thread::Default_थाक_Size;
#endif


#endif

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef अङ्क, ThreadPoolStrategy Default) {
  यदि (अङ्क == "all")
    फर्काऊ llvm::hardware_concurrency();
  यदि (अङ्क.empty())
    फर्काऊ Default;
  अचिन्हित V;
  यदि (अङ्क.getAsInteger(10, V))
    फर्काऊ std::nullopt; // malformed 'Num' value
  यदि (V == 0)
    फर्काऊ Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  फर्काऊ S;
}
