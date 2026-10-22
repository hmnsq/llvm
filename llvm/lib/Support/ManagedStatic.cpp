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
using نطاق llvm;

ساكن ثابت ManagedStaticBase *StaticList = nullptr;

ساكن std::recursive_mutex *getManagedStaticMutex() {
  ساكن std::recursive_mutex m;
  החזר &m;
}

فراغ ManagedStaticBase::RegisterManagedStatic(فراغ *(*Creator)(),
                                              فراغ (*Deleter)(فراغ*)) ثابت {
  أكد(Creator);
  אם (llvm_is_multithreaded()) {
    std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

    אם (!Ptr.load(std::memory_order_relaxed)) {
      فراغ *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      Next = StaticList;
      StaticList = هذا;
    }
  } אחרת {
    أكد(!Ptr && !DeleterFn && !Next &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    Next = StaticList;
    StaticList = هذا;
  }
}

فراغ ManagedStaticBase::destroy() ثابت {
  أكد(DeleterFn && "ManagedStatic not initialized correctly!");
  أكد(StaticList == هذا &&
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
فراغ llvm::llvm_shutdown() {
  כלעוד (StaticList)
    StaticList->destroy();
}
