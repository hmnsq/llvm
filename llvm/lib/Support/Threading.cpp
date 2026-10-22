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

#समावेश "llvm/Support/Threading.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Config/llvm-config.h"
#समावेश "llvm/Support/Jobserver.h"

#समावेश <cassert>
#समावेश <optional>
#समावेश <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#अगर LLVM_ENABLE_THREADS == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

using namespace llvm;

uint64_t llvm::get_threadid() { वापस 0; }

uint32_t llvm::get_max_thread_name_लंबाई() { वापस 0; }

व्योम llvm::set_thread_name(स्थिर Twine &Name) {}

व्योम llvm::get_thread_name(SmallVectorImpl<अक्षर> &Name) { Name.सफा(); }

llvm::BitVector llvm::get_thread_affinity_mask() { वापस {}; }

अचिन्हित llvm::ThreadPoolStrategy::compute_thread_गिनती() स्थिर {
  // When threads are disabled, ensure clients will loop at least once.
  वापस 1;
}

// Unknown if threading turned off
पूर्णांक llvm::get_physical_cores() { वापस -1; }

#अन्यथा

अटल पूर्णांक compute_Host_अंक_Hardware_Threads();

// Include the platform-specific parts of this class.
#अगर_घोषित LLVM_ON_UNIX
#समावेश "Unix/Threading.inc"
#पूर्ण_अगर
#अगर_घोषित _WIN32
#समावेश "Windows/Threading.inc"
#पूर्ण_अगर

using namespace llvm;

अचिन्हित llvm::ThreadPoolStrategy::compute_thread_गिनती() स्थिर {
  अगर (UseJobserver)
    अगर (स्वतः JS = JobserverClient::getInstance())
      वापस JS->get_अंक_Jobs();

  पूर्णांक Max_Thread_गिनती =
      UseHyperThreads ? compute_Host_अंक_Hardware_Threads() : get_physical_cores();
  अगर (Max_Thread_गिनती <= 0)
    Max_Thread_गिनती = 1;
  अगर (ThreadsRequested == 0)
    वापस Max_Thread_गिनती;
  अगर (!Limit)
    वापस ThreadsRequested;
  वापस std::min((अचिन्हित)Max_Thread_गिनती, ThreadsRequested);
}

#अगर defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
स्थिर std::optional<अचिन्हित> llvm::thread::Default_ढेर_आकार = 8 * 1024 * 1024;
#या_अगर defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
स्थिर std::optional<अचिन्हित> llvm::thread::Default_ढेर_आकार = 4 * 1024 * 1024;
#अन्यथा
स्थिर std::optional<अचिन्हित> llvm::thread::Default_ढेर_आकार;
#पूर्ण_अगर


#पूर्ण_अगर

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef अंक, ThreadPoolStrategy Default) {
  अगर (अंक == "all")
    वापस llvm::hardware_concurrency();
  अगर (अंक.empty())
    वापस Default;
  अचिन्हित V;
  अगर (अंक.getAsInteger(10, V))
    वापस std::nullopt; // malformed 'Num' value
  अगर (V == 0)
    वापस Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ThreadsRequested = V;
  वापस S;
}
