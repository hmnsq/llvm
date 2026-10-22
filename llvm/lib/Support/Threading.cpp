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

#అన్తర్భావ "llvm/Support/Threading.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Config/llvm-config.h"
#అన్తర్భావ "llvm/Support/Jobserver.h"

#అన్తర్భావ <cassert>
#అన్తర్భావ <optional>
#అన్తర్భావ <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#ఒకవేళ LLVM_ENABLE_దారాలు == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

ఉపయోగించి నామాకాశ llvm;

uint64_t llvm::get_threadid() { తిరిగి 0; }

uint32_t llvm::get_max_thread_name_నిడివి() { తిరిగి 0; }

శూన్య llvm::సమితి_thread_name(స్థిరరాశి Twine &Name) {}

శూన్య llvm::get_thread_name(Small_సదిశ_Impl<అక్షరం> &Name) { Name.clear(); }

llvm::బిట్టు_సదిశ llvm::get_thread_affinity_ముసుగు() { తిరిగి {}; }

గుర్తులేని llvm::ThreadPoolStrategy::compute_thread_లెక్క() స్థిరరాశి {
  // When threads are disabled, ensure clients will loop at least once.
  తిరిగి 1;
}

// Unknown if threading turned off
పూర్ణాంకం llvm::get_physical_cores() { తిరిగి -1; }

#లేకపోతే

స్థిర పూర్ణాంకం compute_Host_అంకె_Hardware_దారాలు();

// Include the platform-specific parts of this class.
#నిర్వచితమైతే LLVM_ON_UNIX
#అన్తర్భావ "Unix/Threading.inc"
#అయితే_ముగింపు
#నిర్వచితమైతే _WIN32
#అన్తర్భావ "Windows/Threading.inc"
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

గుర్తులేని llvm::ThreadPoolStrategy::compute_thread_లెక్క() స్థిరరాశి {
  ఒకవేళ (UseJobserver)
    ఒకవేళ (స్వయంచాలక JS = JobserverClient::getInstance())
      తిరిగి JS->get_అంకె_Jobs();

  పూర్ణాంకం Max_Thread_లెక్క =
      Use_Hyper_దారాలు ? compute_Host_అంకె_Hardware_దారాలు() : get_physical_cores();
  ఒకవేళ (Max_Thread_లెక్క <= 0)
    Max_Thread_లెక్క = 1;
  ఒకవేళ (దారాలు_Requested == 0)
    తిరిగి Max_Thread_లెక్క;
  ఒకవేళ (!హద్దు_2)
    తిరిగి దారాలు_Requested;
  తిరిగి std::min((గుర్తులేని)Max_Thread_లెక్క, దారాలు_Requested);
}

#ఒకవేళ defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
స్థిరరాశి std::optional<గుర్తులేని> llvm::thread::Default_దొంతర_కొలత = 8 * 1024 * 1024;
#లేదావేళ defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
స్థిరరాశి std::optional<గుర్తులేని> llvm::thread::Default_దొంతర_కొలత = 4 * 1024 * 1024;
#లేకపోతే
స్థిరరాశి std::optional<గుర్తులేని> llvm::thread::Default_దొంతర_కొలత;
#అయితే_ముగింపు


#అయితే_ముగింపు

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef అంకె, ThreadPoolStrategy Default) {
  ఒకవేళ (అంకె == "all")
    తిరిగి llvm::hardware_concurrency();
  ఒకవేళ (అంకె.ఖాళీలు())
    తిరిగి Default;
  గుర్తులేని V;
  ఒకవేళ (అంకె.getAsInteger(10, V))
    తిరిగి std::nullopt; // malformed 'Num' value
  ఒకవేళ (V == 0)
    తిరిగి Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.దారాలు_Requested = V;
  తిరిగి S;
}
