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

#यदि LLVM_ENABLE_तन्तवः == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

उपयुज्य नामाकाश llvm;

uint64_t llvm::get_threadid() { प्रत्यागम 0; }

uint32_t llvm::get_max_thread_name_दीर्घता() { प्रत्यागम 0; }

शून्य llvm::समुच्चयः_thread_name(ध्रुव Twine &Name) {}

शून्य llvm::get_thread_name(Small_सदिशम्_Impl<वर्ण> &Name) { Name.clear(); }

llvm::Bit_सदिशम् llvm::get_thread_affinity_आवरणम्() { प्रत्यागम {}; }

अचिह्नित llvm::ThreadPoolStrategy::compute_thread_गणनम्() ध्रुव {
  // When threads are disabled, ensure clients will loop at least once.
  प्रत्यागम 1;
}

// Unknown if threading turned off
पूर्णाङ्क llvm::get_physical_cores() { प्रत्यागम -1; }

#अन्यथा

स्थिर पूर्णाङ्क compute_Host_अङ्कः_Hardware_तन्तवः();

// Include the platform-specific parts of this class.
#यदि_परिभाषितम् LLVM_ON_UNIX
#अन्तर्भाव "Unix/Threading.inc"
#यदि_अन्तः
#यदि_परिभाषितम् _WIN32
#अन्तर्भाव "Windows/Threading.inc"
#यदि_अन्तः

उपयुज्य नामाकाश llvm;

अचिह्नित llvm::ThreadPoolStrategy::compute_thread_गणनम्() ध्रुव {
  यदि (UseJobserver)
    यदि (स्वचालित JS = JobserverClient::getInstance())
      प्रत्यागम JS->get_अङ्कः_Jobs();

  पूर्णाङ्क Max_Thread_गणनम् =
      Use_Hyper_तन्तवः ? compute_Host_अङ्कः_Hardware_तन्तवः() : get_physical_cores();
  यदि (Max_Thread_गणनम् <= 0)
    Max_Thread_गणनम् = 1;
  यदि (तन्तवः_Requested == 0)
    प्रत्यागम Max_Thread_गणनम्;
  यदि (!सीमा_2)
    प्रत्यागम तन्तवः_Requested;
  प्रत्यागम std::min((अचिह्नित)Max_Thread_गणनम्, तन्तवः_Requested);
}

#यदि defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ध्रुव std::optional<अचिह्नित> llvm::thread::Default_राशिः_आकार = 8 * 1024 * 1024;
#नोचेद्यदि defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ध्रुव std::optional<अचिह्नित> llvm::thread::Default_राशिः_आकार = 4 * 1024 * 1024;
#अन्यथा
ध्रुव std::optional<अचिह्नित> llvm::thread::Default_राशिः_आकार;
#यदि_अन्तः


#यदि_अन्तः

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef अङ्कः, ThreadPoolStrategy Default) {
  यदि (अङ्कः == "all")
    प्रत्यागम llvm::hardware_concurrency();
  यदि (अङ्कः.रिक्तानि())
    प्रत्यागम Default;
  अचिह्नित V;
  यदि (अङ्कः.getAsInteger(10, V))
    प्रत्यागम std::nullopt; // malformed 'Num' value
  यदि (V == 0)
    प्रत्यागम Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.तन्तवः_Requested = V;
  प्रत्यागम S;
}
