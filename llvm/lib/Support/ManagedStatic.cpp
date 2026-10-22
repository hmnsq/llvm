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

#અન્તર્ભાવ "llvm/Support/ManagedStatic.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/Threading.h"
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <mutex>
using નામાકાશ llvm;

સ્થિર અચલ ManagedStaticBase *StaticList = nullptr;

સ્થિર std::recursive_mutex *getManagedStaticMutex() {
  સ્થિર std::recursive_mutex m;
  પાછા &m;
}

શૂન્ય ManagedStaticBase::RegisterManagedStatic(શૂન્ય *(*Creator)(),
                                              શૂન્ય (*Deleter)(શૂન્ય*)) અચલ {
  પ્રતિજ્ઞા(Creator);
  જો (llvm_is_multithreaded()) {
    std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

    જો (!Ptr.load(std::memory_order_relaxed)) {
      શૂન્ય *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      Next = StaticList;
      StaticList = આ;
    }
  } નહીંતર {
    પ્રતિજ્ઞા(!Ptr && !DeleterFn && !Next &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    Next = StaticList;
    StaticList = આ;
  }
}

શૂન્ય ManagedStaticBase::destroy() અચલ {
  પ્રતિજ્ઞા(DeleterFn && "ManagedStatic not initialized correctly!");
  પ્રતિજ્ઞા(StaticList == આ &&
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
શૂન્ય llvm::llvm_shutdown() {
  જ્યારે (StaticList)
    StaticList->destroy();
}
