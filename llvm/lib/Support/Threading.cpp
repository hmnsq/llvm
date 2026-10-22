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

#اگر LLVM_ENABLE_دھاگے == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

استعمال نامخلا llvm;

uint64_t llvm::get_threadid() { واپس 0; }

uint32_t llvm::get_max_thread_name_لمبائی() { واپس 0; }

خالی llvm::سیٹ_thread_name(ثابت Twine &Name) {}

خالی llvm::get_thread_name(Small_سمتیہ_Impl<حرف> &Name) { Name.clear(); }

llvm::ثنائی_ہندسہ_سمتیہ llvm::get_thread_affinity_نقاب() { واپس {}; }

غیرعلامتی llvm::ThreadPoolStrategy::compute_thread_گنتی() ثابت {
  // When threads are disabled, ensure clients will loop at least once.
  واپس 1;
}

// Unknown if threading turned off
صحیح llvm::get_physical_cores() { واپس -1; }

#ورنہ

ساکن صحیح compute_Host_عدد_Hardware_دھاگے();

// Include the platform-specific parts of this class.
#اگر_تعریف_شدہ LLVM_ON_UNIX
#شامل "Unix/Threading.inc"
#اگر_ختم
#اگر_تعریف_شدہ _WIN32
#شامل "Windows/Threading.inc"
#اگر_ختم

استعمال نامخلا llvm;

غیرعلامتی llvm::ThreadPoolStrategy::compute_thread_گنتی() ثابت {
  اگر (UseJobserver)
    اگر (خودکار JS = JobserverClient::getInstance())
      واپس JS->get_عدد_Jobs();

  صحیح Max_Thread_گنتی =
      Use_Hyper_دھاگے ? compute_Host_عدد_Hardware_دھاگے() : get_physical_cores();
  اگر (Max_Thread_گنتی <= 0)
    Max_Thread_گنتی = 1;
  اگر (دھاگے_Requested == 0)
    واپس Max_Thread_گنتی;
  اگر (!حد_بندی_2)
    واپس دھاگے_Requested;
  واپس std::min((غیرعلامتی)Max_Thread_گنتی, دھاگے_Requested);
}

#اگر defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ثابت std::optional<غیرعلامتی> llvm::thread::Default_ڈھیر_سائز = 8 * 1024 * 1024;
#وگرنہ_اگر defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ثابت std::optional<غیرعلامتی> llvm::thread::Default_ڈھیر_سائز = 4 * 1024 * 1024;
#ورنہ
ثابت std::optional<غیرعلامتی> llvm::thread::Default_ڈھیر_سائز;
#اگر_ختم


#اگر_ختم

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef عدد, ThreadPoolStrategy Default) {
  اگر (عدد == "all")
    واپس llvm::hardware_concurrency();
  اگر (عدد.خالی_خانے())
    واپس Default;
  غیرعلامتی V;
  اگر (عدد.getAsInteger(10, V))
    واپس std::nullopt; // malformed 'Num' value
  اگر (V == 0)
    واپس Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.دھاگے_Requested = V;
  واپس S;
}
