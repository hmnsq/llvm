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

#إذا LLVM_ENABLE_دھاگے == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

ورتوں نطاق llvm;

uint64_t llvm::get_threadid() { أرجع 0; }

uint32_t llvm::get_max_thread_name_لمائی() { أرجع 0; }

فراغ llvm::سیٹ_thread_name(ثابت Twine &Name) {}

فراغ llvm::get_thread_name(Small_سمتیہ_Impl<حرف> &Name) { Name.clear(); }

llvm::ثنائی_ہندسہ_سمتیہ llvm::get_thread_affinity_نقاب() { أرجع {}; }

بنانشانی llvm::ThreadPoolStrategy::compute_thread_گنتی() ثابت {
  // When threads are disabled, ensure clients will loop at least once.
  أرجع 1;
}

// Unknown if threading turned off
صحيح llvm::get_physical_cores() { أرجع -1; }

#وإلا

ساكن صحيح compute_Host_عدد_Hardware_دھاگے();

// Include the platform-specific parts of this class.
#جے_تعریف_شدہ LLVM_ON_UNIX
#ضمن "Unix/Threading.inc"
#جے_انت
#جے_تعریف_شدہ _WIN32
#ضمن "Windows/Threading.inc"
#جے_انت

ورتوں نطاق llvm;

بنانشانی llvm::ThreadPoolStrategy::compute_thread_گنتی() ثابت {
  إذا (UseJobserver)
    إذا (خودکار JS = JobserverClient::getInstance())
      أرجع JS->get_عدد_Jobs();

  صحيح Max_Thread_گنتی =
      Use_Hyper_دھاگے ? compute_Host_عدد_Hardware_دھاگے() : get_physical_cores();
  إذا (Max_Thread_گنتی <= 0)
    Max_Thread_گنتی = 1;
  إذا (دھاگے_Requested == 0)
    أرجع Max_Thread_گنتی;
  إذا (!حد_بندی_2)
    أرجع دھاگے_Requested;
  أرجع std::min((بنانشانی)Max_Thread_گنتی, دھاگے_Requested);
}

#إذا defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ثابت std::optional<بنانشانی> llvm::thread::Default_ڈھیر_ناپ = 8 * 1024 * 1024;
#وإلاإذا defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ثابت std::optional<بنانشانی> llvm::thread::Default_ڈھیر_ناپ = 4 * 1024 * 1024;
#وإلا
ثابت std::optional<بنانشانی> llvm::thread::Default_ڈھیر_ناپ;
#جے_انت


#جے_انت

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef عدد, ThreadPoolStrategy Default) {
  إذا (عدد == "all")
    أرجع llvm::hardware_concurrency();
  إذا (عدد.خالی_تھاواں())
    أرجع Default;
  بنانشانی V;
  إذا (عدد.getAsInteger(10, V))
    أرجع std::nullopt; // malformed 'Num' value
  إذا (V == 0)
    أرجع Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.دھاگے_Requested = V;
  أرجع S;
}
