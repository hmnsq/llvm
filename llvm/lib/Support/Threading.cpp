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

#ಅನ್ತರ್ಭಾವ "llvm/Support/Threading.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Jobserver.h"

#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <optional>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು == 0 ||                                                \
    (!defined(_WIN32) && !defined(HAVE_PTHREAD_H))

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

uint64_t llvm::get_threadid() { ಹಿಂತಿರುಗಿ 0; }

uint32_t llvm::get_max_thread_name_ಉದ್ದಳತೆ() { ಹಿಂತಿರುಗಿ 0; }

ಶೂನ್ಯ llvm::ಗಣ_thread_name(ಸ್ಥಿರಾಂಕ Twine &Name) {}

ಶೂನ್ಯ llvm::get_thread_name(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Name) { Name.clear(); }

llvm::ಬಿಟ್_ಸದಿಶ llvm::get_thread_affinity_ಮುಖವಾಡ() { ಹಿಂತಿರುಗಿ {}; }

ಚಿಹ್ನೆರಹಿತ llvm::ThreadPoolStrategy::compute_thread_ಎಣಿಕೆ() ಸ್ಥಿರಾಂಕ {
  // When threads are disabled, ensure clients will loop at least once.
  ಹಿಂತಿರುಗಿ 1;
}

// Unknown if threading turned off
ಪೂರ್ಣಾಂಕ llvm::get_physical_cores() { ಹಿಂತಿರುಗಿ -1; }

#ಇಲ್ಲದಿದ್ದರೆ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ compute_Host_ಅಂಕೆ_Hardware_ಎಳೆಗಳು();

// Include the platform-specific parts of this class.
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_ON_UNIX
#ಅನ್ತರ್ಭಾವ "Unix/Threading.inc"
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#ಅನ್ತರ್ಭಾವ "Windows/Threading.inc"
#ಆದರೆ_ಅಂತ್ಯ

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಚಿಹ್ನೆರಹಿತ llvm::ThreadPoolStrategy::compute_thread_ಎಣಿಕೆ() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (UseJobserver)
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ JS = JobserverClient::getInstance())
      ಹಿಂತಿರುಗಿ JS->get_ಅಂಕೆ_Jobs();

  ಪೂರ್ಣಾಂಕ Max_Thread_ಎಣಿಕೆ =
      Use_Hyper_ಎಳೆಗಳು ? compute_Host_ಅಂಕೆ_Hardware_ಎಳೆಗಳು() : get_physical_cores();
  ಆದರೆ (Max_Thread_ಎಣಿಕೆ <= 0)
    Max_Thread_ಎಣಿಕೆ = 1;
  ಆದರೆ (ಎಳೆಗಳು_Requested == 0)
    ಹಿಂತಿರುಗಿ Max_Thread_ಎಣಿಕೆ;
  ಆದರೆ (!ಎಲ್ಲೆ_2)
    ಹಿಂತಿರುಗಿ ಎಳೆಗಳು_Requested;
  ಹಿಂತಿರುಗಿ std::min((ಚಿಹ್ನೆರಹಿತ)Max_Thread_ಎಣಿಕೆ, ಎಳೆಗಳು_Requested);
}

#ಆದರೆ defined(__APPLE__)
  // Darwin's default stack size for threads except the main one is only 512KB,
  // which is not enough for some/many normal LLVM compilations. This implements
  // the same interface as std::thread but requests the same stack size as the
  // main thread (8MB) before creation.
ಸ್ಥಿರಾಂಕ std::optional<ಚಿಹ್ನೆರಹಿತ> llvm::thread::Default_ರಾಶಿ_ಗಾತ್ರ = 8 * 1024 * 1024;
#ಅಥವಾಆದರೆ defined(_AIX)
  // On AIX, the default pthread stack size limit is ~192k for 64-bit programs.
  // This limit is easily reached when doing link-time thinLTO. AIX library
  // developers have used 4MB, so we'll do the same.
ಸ್ಥಿರಾಂಕ std::optional<ಚಿಹ್ನೆರಹಿತ> llvm::thread::Default_ರಾಶಿ_ಗಾತ್ರ = 4 * 1024 * 1024;
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರಾಂಕ std::optional<ಚಿಹ್ನೆರಹಿತ> llvm::thread::Default_ರಾಶಿ_ಗಾತ್ರ;
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ_ಅಂತ್ಯ

std::optional<ThreadPoolStrategy>
llvm::get_threadpool_strategy(StringRef ಅಂಕೆ, ThreadPoolStrategy Default) {
  ಆದರೆ (ಅಂಕೆ == "all")
    ಹಿಂತಿರುಗಿ llvm::hardware_concurrency();
  ಆದರೆ (ಅಂಕೆ.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ Default;
  ಚಿಹ್ನೆರಹಿತ V;
  ಆದರೆ (ಅಂಕೆ.getAsInteger(10, V))
    ಹಿಂತಿರುಗಿ std::nullopt; // malformed 'Num' value
  ಆದರೆ (V == 0)
    ಹಿಂತಿರುಗಿ Default;

  // Do not take the Default into account. This effectively disables
  // heavyweight_hardware_concurrency() if the user asks for any number of
  // threads on the cmd-line.
  ThreadPoolStrategy S = llvm::hardware_concurrency();
  S.ಎಳೆಗಳು_Requested = V;
  ಹಿಂತಿರುಗಿ S;
}
