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

#ضمن "llvm/Support/ManagedStatic.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Threading.h"
#ضمن <cassert>
#ضمن <mutex>
ورتوں نطاق llvm;

ساكن ثابت Managed_Static_ادھار *StaticList = خالی_سوچک;

ساكن std::recursive_mutex *getManagedStaticMutex() {
  ساكن std::recursive_mutex m;
  أرجع &m;
}

فراغ Managed_Static_ادھار::رجسٹر_Managed_Static(فراغ *(*Creator)(),
                                              فراغ (*Deleter)(فراغ*)) ثابت {
  أكد(Creator);
  إذا (llvm_is_multithreaded()) {
    std::جندرا_guard<std::recursive_mutex> جندرا_2(*getManagedStaticMutex());

    إذا (!Ptr.لدو(std::memory_order_relaxed)) {
      فراغ *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      اگلا_2 = StaticList;
      StaticList = هذا;
    }
  } وإلا {
    أكد(!Ptr && !DeleterFn && !اگلا_2 &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    اگلا_2 = StaticList;
    StaticList = هذا;
  }
}

فراغ Managed_Static_ادھار::destroy() ثابت {
  أكد(DeleterFn && "ManagedStatic not initialized correctly!");
  أكد(StaticList == هذا &&
         "Not destroyed in reverse order of construction?");
  // Unlink from list.
  StaticList = اگلا_2;
  اگلا_2 = خالی_سوچک;

  // Destroy memory.
  DeleterFn(Ptr);

  // Cleanup.
  Ptr = خالی_سوچک;
  DeleterFn = خالی_سوچک;
}

/// llvm_shutdown - Deallocate and destroy all ManagedStatic variables.
/// IMPORTANT: it's only safe to call llvm_shutdown() in single thread,
/// without any other threads executing LLVM APIs.
/// llvm_shutdown() should be the last use of LLVM APIs.
فراغ llvm::llvm_shutdown() {
  طالما (StaticList)
    StaticList->destroy();
}
