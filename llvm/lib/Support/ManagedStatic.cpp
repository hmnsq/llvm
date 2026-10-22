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

#समावेश "llvm/Support/ManagedStatic.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/Threading.h"
#समावेश <cassert>
#समावेश <mutex>
using namespace llvm;

अटल स्थिर ManagedStaticBase *StaticList = nullptr;

अटल std::recursive_mutex *getManagedStaticMutex() {
  अटल std::recursive_mutex m;
  वापस &m;
}

व्योम ManagedStaticBase::RegisterManagedStatic(व्योम *(*Creator)(),
                                              व्योम (*Deleter)(व्योम*)) स्थिर {
  निश्चित(Creator);
  अगर (llvm_is_multithreaded()) {
    std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

    अगर (!Ptr.load(std::memory_order_relaxed)) {
      व्योम *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      अग्र_2 = StaticList;
      StaticList = यह;
    }
  } अन्यथा {
    निश्चित(!Ptr && !DeleterFn && !अग्र_2 &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    अग्र_2 = StaticList;
    StaticList = यह;
  }
}

व्योम ManagedStaticBase::destroy() स्थिर {
  निश्चित(DeleterFn && "ManagedStatic not initialized correctly!");
  निश्चित(StaticList == यह &&
         "Not destroyed in reverse order of construction?");
  // Unlink from list.
  StaticList = अग्र_2;
  अग्र_2 = nullptr;

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
व्योम llvm::llvm_shutdown() {
  जबतक (StaticList)
    StaticList->destroy();
}
