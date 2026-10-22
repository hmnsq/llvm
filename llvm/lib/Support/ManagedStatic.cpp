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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/ManagedStatic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Threading.h"
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <mutex>
using ନାମାକାଶ llvm;

ସ୍ଥିର ଅଚଳ ManagedStaticBase *StaticList = nullptr;

ସ୍ଥିର std::recursive_mutex *getManagedStaticMutex() {
  ସ୍ଥିର std::recursive_mutex m;
  ଫେରନ୍ତୁ &m;
}

ଶୂନ୍ୟ ManagedStaticBase::RegisterManagedStatic(ଶୂନ୍ୟ *(*Creator)(),
                                              ଶୂନ୍ୟ (*Deleter)(ଶୂନ୍ୟ*)) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(Creator);
  ଯଦି (llvm_is_multithreaded()) {
    std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

    ଯଦି (!Ptr.load(std::memory_order_relaxed)) {
      ଶୂନ୍ୟ *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      Next = StaticList;
      StaticList = ଏହି;
    }
  } ଅନ୍ୟଥା {
    ପ୍ରତିଜ୍ଞା(!Ptr && !DeleterFn && !Next &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    Next = StaticList;
    StaticList = ଏହି;
  }
}

ଶୂନ୍ୟ ManagedStaticBase::destroy() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(DeleterFn && "ManagedStatic not initialized correctly!");
  ପ୍ରତିଜ୍ଞା(StaticList == ଏହି &&
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
ଶୂନ୍ୟ llvm::llvm_shutdown() {
  ଯେତେବେଳେ (StaticList)
    StaticList->destroy();
}
