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
using ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ಧ್ರುವ ManagedStaticBase *StaticList = nullptr;

ಸ್ಥಿರ std::recursive_mutex *getManagedStaticMutex() {
  ಸ್ಥಿರ std::recursive_mutex m;
  ಪ್ರತ್ಯಾಗಮ &m;
}

ಶೂನ್ಯ ManagedStaticBase::RegisterManagedStatic(ಶೂನ್ಯ *(*Creator)(),
                                              ಶೂನ್ಯ (*Deleter)(ಶೂನ್ಯ*)) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(Creator);
  ಯದಿ (llvm_is_multithreaded()) {
    std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

    ಯದಿ (!Ptr.load(std::memory_order_relaxed)) {
      ಶೂನ್ಯ *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      Next = StaticList;
      StaticList = ಏತತ್;
    }
  } ಅನ್ಯಥಾ {
    ಪ್ರತಿಜ್ಞಾ(!Ptr && !DeleterFn && !Next &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    Next = StaticList;
    StaticList = ಏತತ್;
  }
}

ಶೂನ್ಯ ManagedStaticBase::destroy() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(DeleterFn && "ManagedStatic not initialized correctly!");
  ಪ್ರತಿಜ್ಞಾ(StaticList == ಏತತ್ &&
         "Not destroyed in reverse order of construction?");
  // Unlink from list.
  StaticList = Next;
  Next = nullptr;

  // Destroy memory.
  DeleterFn(Ptr);

  // Cleanup.
  Ptr = nullptr;
  DeleterFn = nullptr;
}

/// llvm_shutdown - Deallocate and destroy all ManagedStatic variables.
/// IMPORTANT: it's only safe to call llvm_shutdown() in single thread,
/// without any other threads executing LLVM APIs.
/// llvm_shutdown() should be the last use of LLVM APIs.
ಶೂನ್ಯ llvm::llvm_shutdown() {
  ಯಾವತ್ (StaticList)
    StaticList->destroy();
}
