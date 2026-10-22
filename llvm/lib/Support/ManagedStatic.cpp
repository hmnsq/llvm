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
using فضاینام llvm;

ایستا ثابت ManagedStaticBase *StaticList = nullptr;

ایستا std::recursive_mutex *getManagedStaticMutex() {
  ایستا std::recursive_mutex m;
  بازگشت &m;
}

تهی ManagedStaticBase::RegisterManagedStatic(تهی *(*Creator)(),
                                              تهی (*Deleter)(تهی*)) ثابت {
  أكد(Creator);
  اگر (llvm_is_multithreaded()) {
    std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

    اگر (!Ptr.load(std::memory_order_relaxed)) {
      تهی *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      Next = StaticList;
      StaticList = این;
    }
  } وگرنه {
    أكد(!Ptr && !DeleterFn && !Next &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    Next = StaticList;
    StaticList = این;
  }
}

تهی ManagedStaticBase::destroy() ثابت {
  أكد(DeleterFn && "ManagedStatic not initialized correctly!");
  أكد(StaticList == این &&
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
تهی llvm::llvm_shutdown() {
  وقتی (StaticList)
    StaticList->destroy();
}
