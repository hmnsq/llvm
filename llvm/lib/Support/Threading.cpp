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

#إذا LLVM_ENABLE_نخها == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

استفاده نطاق llvm;

uint64_t llvm::get_threadid() { أرجع 0; }

uint32_t llvm::get_max_thread_name_درازی() { أرجع 0; }

فراغ llvm::مجموعه_thread_name(ثابت Twine &Name) {}

فراغ llvm::get_thread_name(Small_وکتور_Impl<حرف> &Name) { Name.clear(); }

llvm::بیت_وکتور llvm::get_thread_affinity_ماسک() { أرجع {}; }

بیعلامت llvm::ThreadPoolStrategy::compute_thread_شمار() ثابت {
  // When threads are disabled, ensure clients will loop at least once.
  أرجع 1;
}

// Unknown if threading turned off
صحيح llvm::get_physical_cores() { أرجع -1; }

#وإلا

ساكن صحيح compute_Host_رقم_Hardware_نخها();

// Include the platform-specific parts of this class.
#اگر_تعریف_شده LLVM_ON_UNIX
#ضمن "Unix/Threading.inc"
#پایان_اگر
#اگر_تعریف_شده _WIN32
#ضمن "Windows/Threading.inc"
#پایان_اگر

استفاده نطاق llvm;

بیعلامت llvm::ThreadPoolStrategy::compute_thread_شمار() ثابت {
  إذا (UseJobserver)
    إذا (خودکار JS = JobserverClient::getInstance())
      أرجع JS->get_رقم_Jobs();

  صحيح Max_Thread_شمار =
      Use_Hyper_نخها ? compute_Host_رقم_Hardware_نخها() : get_physical_cores();
  إذا (Max_Thread_شمار <= 0)
    Max_Thread_شمار = 1;
  إذا (نخها_Requested == 0)
    أرجع Max_Thread_شمار;
  إذا (!حد_2)
    أرجع نخها_Requested;
  أرجع std::min((بیعلامت)Max_Thread_شمار, نخها_Requested);
}

#إذا defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ثابت std::optional<بیعلامت> llvm::thread::Default_پشته_اندازه = 8 * 1024 * 1024;
#وإلاإذا defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ثابت std::optional<بیعلامت> llvm::thread::Default_پشته_اندازه = 4 * 1024 * 1024;
#وإلا
ثابت std::optional<بیعلامت> llvm::thread::Default_پشته_اندازه;
#پایان_اگر


#پایان_اگر

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef رقم, ThreadPoolStrategy Default) {
  إذا (رقم == "all")
    أرجع llvm::hardware_concurrency();
  إذا (رقم.خالی())
    أرجع Default;
  بیعلامت V;
  إذا (رقم.getAsInteger(10, V))
    أرجع std::nullopt; // malformed 'Num' value
  إذا (V == 0)
    أرجع Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.نخها_Requested = V;
  أرجع S;
}
