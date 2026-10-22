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

#ضمن "llvm/Support/Threading.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/Jobserver.h"

#ضمن <cassert>
#ضمن <optional>
#ضمن <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#إذا LLVM_ENABLE_ڌاڳا == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

استعمال نطاق llvm;

uint64_t llvm::get_threadid() { أرجع 0; }

uint32_t llvm::get_max_thread_name_ڊيگهائي() { أرجع 0; }

فراغ llvm::سيٽ_thread_name(ثابت Twine &Name) {}

فراغ llvm::get_thread_name(Small_ويڪٽر_Impl<حرف> &Name) { Name.clear(); }

llvm::ٻٽو_انگ_ويڪٽر llvm::get_thread_affinity_نقاب() { أرجع {}; }

بنانشان llvm::ThreadPoolStrategy::compute_thread_ڳڻپ() ثابت {
  // When threads are disabled, ensure clients will loop at least once.
  أرجع 1;
}

// Unknown if threading turned off
صحيح llvm::get_physical_cores() { أرجع -1; }

#وإلا

ساكن صحيح compute_Host_انگ_Hardware_ڌاڳا();

// Include the platform-specific parts of this class.
#جيڪڏهن_وصف_ٿيل LLVM_ON_UNIX
#ضمن "Unix/Threading.inc"
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل _WIN32
#ضمن "Windows/Threading.inc"
#جيڪڏهن_پورو

استعمال نطاق llvm;

بنانشان llvm::ThreadPoolStrategy::compute_thread_ڳڻپ() ثابت {
  إذا (UseJobserver)
    إذا (خودڪار JS = JobserverClient::getInstance())
      أرجع JS->get_انگ_Jobs();

  صحيح Max_Thread_ڳڻپ =
      Use_Hyper_ڌاڳا ? compute_Host_انگ_Hardware_ڌاڳا() : get_physical_cores();
  إذا (Max_Thread_ڳڻپ <= 0)
    Max_Thread_ڳڻپ = 1;
  إذا (ڌاڳا_Requested == 0)
    أرجع Max_Thread_ڳڻپ;
  إذا (!حد_بندي_2)
    أرجع ڌاڳا_Requested;
  أرجع std::min((بنانشان)Max_Thread_ڳڻپ, ڌاڳا_Requested);
}

#إذا defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ثابت std::optional<بنانشان> llvm::thread::Default_ڍير_سائيز = 8 * 1024 * 1024;
#وإلاإذا defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ثابت std::optional<بنانشان> llvm::thread::Default_ڍير_سائيز = 4 * 1024 * 1024;
#وإلا
ثابت std::optional<بنانشان> llvm::thread::Default_ڍير_سائيز;
#جيڪڏهن_پورو


#جيڪڏهن_پورو

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef انگ, ThreadPoolStrategy Default) {
  إذا (انگ == "all")
    أرجع llvm::hardware_concurrency();
  إذا (انگ.خالي_جايون())
    أرجع Default;
  بنانشان V;
  إذا (انگ.getAsInteger(10, V))
    أرجع std::nullopt; // malformed 'Num' value
  إذا (V == 0)
    أرجع Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ڌاڳا_Requested = V;
  أرجع S;
}
