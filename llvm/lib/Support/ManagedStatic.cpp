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

#অন্তর্ভাব "llvm/Support/ManagedStatic.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Support/Threading.h"
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <mutex>
using নামাকাশ llvm;

স্থির ধ্রুবক ManagedStaticBase *StaticList = nullptr;

স্থির std::recursive_mutex *getManagedStaticMutex() {
  স্থির std::recursive_mutex m;
  ফেরত &m;
}

শূন্য ManagedStaticBase::RegisterManagedStatic(শূন্য *(*Creator)(),
                                              শূন্য (*Deleter)(শূন্য*)) ধ্রুবক {
  প্রতিজ্ঞা(Creator);
  যদি (llvm_is_multithreaded()) {
    std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

    যদি (!Ptr.load(std::memory_order_relaxed)) {
      শূন্য *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      পরবর্তী_2 = StaticList;
      StaticList = এই;
    }
  } অন্যথা {
    প্রতিজ্ঞা(!Ptr && !DeleterFn && !পরবর্তী_2 &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    পরবর্তী_2 = StaticList;
    StaticList = এই;
  }
}

শূন্য ManagedStaticBase::destroy() ধ্রুবক {
  প্রতিজ্ঞা(DeleterFn && "ManagedStatic not initialized correctly!");
  প্রতিজ্ঞা(StaticList == এই &&
         "Not destroyed in reverse order of construction?");
  // Unlink from list.
  StaticList = পরবর্তী_2;
  পরবর্তী_2 = nullptr;

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
শূন্য llvm::llvm_shutdown() {
  যখন (StaticList)
    StaticList->destroy();
}
