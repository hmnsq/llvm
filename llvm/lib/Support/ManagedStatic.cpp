//===-- ManagedStatic.cpp - Static Global wrapper -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the ManagedStatic class and llvm_shutdown().
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/ManagedStatic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Threading.h"
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <mutex>
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Managed_Static_ತಳ *StaticList = ಶೂನ್ಯಸೂಚಕ;

ಸ್ಥಿರ std::recursive_mutex *getManagedStaticMutex() {
  ಸ್ಥಿರ std::recursive_mutex m;
  ಹಿಂತಿರುಗಿ &m;
}

ಶೂನ್ಯ Managed_Static_ತಳ::ರಿಜಿಸ್ಟರ್_Managed_Static(ಶೂನ್ಯ *(*Creator)(),
                                              ಶೂನ್ಯ (*Deleter)(ಶೂನ್ಯ*)) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(Creator);
  ಆದರೆ (llvm_is_multithreaded()) {
    std::ಬೀಗ_guard<std::recursive_mutex> ಬೀಗ_3(*getManagedStaticMutex());

    ಆದರೆ (!Ptr.ತುಂಬು(std::memory_order_relaxed)) {
      ಶೂನ್ಯ *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      ಮುಂದಿನ_2 = StaticList;
      StaticList = ಇದು;
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಪ್ರತಿಜ್ಞಾ(!Ptr && !DeleterFn && !ಮುಂದಿನ_2 &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    ಮುಂದಿನ_2 = StaticList;
    StaticList = ಇದು;
  }
}

ಶೂನ್ಯ Managed_Static_ತಳ::destroy() ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(DeleterFn && "ManagedStatic not initialized correctly!");
  ಪ್ರತಿಜ್ಞಾ(StaticList == ಇದು &&
         "Not destroyed in reverse order of construction?");
  // Unlink from list.
  StaticList = ಮುಂದಿನ_2;
  ಮುಂದಿನ_2 = ಶೂನ್ಯಸೂಚಕ;

  // Destroy memory.
  DeleterFn(Ptr);

  // Cleanup.
  Ptr = ಶೂನ್ಯಸೂಚಕ;
  DeleterFn = ಶೂನ್ಯಸೂಚಕ;
}

/// llvm_shutdown - Deallocate and destroy all ManagedStatic variables.
/// IMPORTANT: it's only safe to call llvm_shutdown() in single thread,
/// without any other threads executing LLVM APIs.
/// llvm_shutdown() should be the last use of LLVM APIs.
ಶೂನ್ಯ llvm::llvm_shutdown() {
  ಆಗ (StaticList)
    StaticList->destroy();
}
