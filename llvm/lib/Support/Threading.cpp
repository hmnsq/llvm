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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Threading.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Jobserver.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <optional>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#ਜੇ LLVM_ENABLE_ਧਾਗੇ == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

uint64_t llvm::get_threadid() { ਵਾਪਸ 0; }

uint32_t llvm::get_max_thread_name_ਲੰਮਾਈ() { ਵਾਪਸ 0; }

ਖਾਲੀ llvm::ਸੈੱਟ_thread_name(ਅਚਲ Twine &Name) {}

ਖਾਲੀ llvm::get_thread_name(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Name) { Name.clear(); }

llvm::ਦੋਆਧਾਰੀ_ਅੰਕ_ਸਦਿਸ਼ llvm::get_thread_affinity_ਨਕਾਬ() { ਵਾਪਸ {}; }

ਬਿਨਾਨਿਸ਼ਾਨੀ llvm::ThreadPoolStrategy::compute_thread_ਗਿਣਤੀ() ਅਚਲ {
  // When threads are disabled, ensure clients will loop at least once.
  ਵਾਪਸ 1;
}

// Unknown if threading turned off
ਪੂਰਨ llvm::get_physical_cores() { ਵਾਪਸ -1; }

#ਨਹੀਂਤਾਂ

ਸਥਿਰ ਪੂਰਨ compute_Host_ਅੰਕ_Hardware_ਧਾਗੇ();

// Include the platform-specific parts of this class.
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ON_UNIX
#ਅਨ੍ਤਰ੍ਭਾਵ "Unix/Threading.inc"
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਅਨ੍ਤਰ੍ਭਾਵ "Windows/Threading.inc"
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਬਿਨਾਨਿਸ਼ਾਨੀ llvm::ThreadPoolStrategy::compute_thread_ਗਿਣਤੀ() ਅਚਲ {
  ਜੇ (UseJobserver)
    ਜੇ (ਸਵੈਚਾਲਿਤ JS = JobserverClient::getInstance())
      ਵਾਪਸ JS->get_ਅੰਕ_Jobs();

  ਪੂਰਨ Max_Thread_ਗਿਣਤੀ =
      Use_Hyper_ਧਾਗੇ ? compute_Host_ਅੰਕ_Hardware_ਧਾਗੇ() : get_physical_cores();
  ਜੇ (Max_Thread_ਗਿਣਤੀ <= 0)
    Max_Thread_ਗਿਣਤੀ = 1;
  ਜੇ (ਧਾਗੇ_Requested == 0)
    ਵਾਪਸ Max_Thread_ਗਿਣਤੀ;
  ਜੇ (!ਹੱਦਬੰਦੀ_2)
    ਵਾਪਸ ਧਾਗੇ_Requested;
  ਵਾਪਸ std::min((ਬਿਨਾਨਿਸ਼ਾਨੀ)Max_Thread_ਗਿਣਤੀ, ਧਾਗੇ_Requested);
}

#ਜੇ defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ਅਚਲ std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ> llvm::thread::Default_ਢੇਰ_ਮਾਪ = 8 * 1024 * 1024;
#ਜਾਂਜੇ defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ਅਚਲ std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ> llvm::thread::Default_ਢੇਰ_ਮਾਪ = 4 * 1024 * 1024;
#ਨਹੀਂਤਾਂ
ਅਚਲ std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ> llvm::thread::Default_ਢੇਰ_ਮਾਪ;
#ਜੇ_ਅੰਤ


#ਜੇ_ਅੰਤ

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef ਅੰਕ, ThreadPoolStrategy Default) {
  ਜੇ (ਅੰਕ == "all")
    ਵਾਪਸ llvm::hardware_concurrency();
  ਜੇ (ਅੰਕ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ Default;
  ਬਿਨਾਨਿਸ਼ਾਨੀ V;
  ਜੇ (ਅੰਕ.getAsInteger(10, V))
    ਵਾਪਸ std::nullopt; // malformed 'Num' value
  ਜੇ (V == 0)
    ਵਾਪਸ Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ਧਾਗੇ_Requested = V;
  ਵਾਪਸ S;
}
