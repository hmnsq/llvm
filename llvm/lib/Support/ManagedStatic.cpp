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

#شامل "llvm/Support/ManagedStatic.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/Threading.h"
#شامل <cassert>
#شامل <mutex>
استعمال نامخلا llvm;

ساکن ثابت Managed_Static_بنیاد *StaticList = خالی_اشاریہ;

ساکن std::recursive_mutex *getManagedStaticMutex() {
  ساکن std::recursive_mutex m;
  واپس &m;
}

خالی Managed_Static_بنیاد::رجسٹر_Managed_Static(خالی *(*Creator)(),
                                              خالی (*Deleter)(خالی*)) ثابت {
  أكد(Creator);
  اگر (llvm_is_multithreaded()) {
    std::تالا_guard<std::recursive_mutex> تالا_3(*getManagedStaticMutex());

    اگر (!Ptr.بھرو(std::memory_order_relaxed)) {
      خالی *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      اگلا_2 = StaticList;
      StaticList = یہ;
    }
  } ورنہ {
    أكد(!Ptr && !DeleterFn && !اگلا_2 &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    اگلا_2 = StaticList;
    StaticList = یہ;
  }
}

خالی Managed_Static_بنیاد::destroy() ثابت {
  أكد(DeleterFn && "ManagedStatic not initialized correctly!");
  أكد(StaticList == یہ &&
         "Not destroyed in reverse order of construction?");
  // Unlink from list.
  StaticList = اگلا_2;
  اگلا_2 = خالی_اشاریہ;

  // Destroy memory.
  DeleterFn(Ptr);

  // Cleanup.
  Ptr = خالی_اشاریہ;
  DeleterFn = خالی_اشاریہ;
}

/// llvm_shutdown - Deallocate and destroy all ManagedStatic variables.
/// IMPORTANT: it's only safe to call llvm_shutdown() in single thread,
/// without any other threads executing LLVM APIs.
/// llvm_shutdown() should be the last use of LLVM APIs.
خالی llvm::llvm_shutdown() {
  جب (StaticList)
    StaticList->destroy();
}
