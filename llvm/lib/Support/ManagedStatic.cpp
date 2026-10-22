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

#அந்தர்பாவ "llvm/Support/ManagedStatic.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Support/Threading.h"
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <mutex>
using நாமாகாஶ llvm;

நிலையான மாறிலி ManagedStaticBase *StaticList = nullptr;

நிலையான std::recursive_mutex *getManagedStaticMutex() {
  நிலையான std::recursive_mutex m;
  திரும்பு &m;
}

வெறுமை ManagedStaticBase::RegisterManagedStatic(வெறுமை *(*Creator)(),
                                              வெறுமை (*Deleter)(வெறுமை*)) மாறிலி {
  ப்ரதிஜ்ஞா(Creator);
  எனில் (llvm_is_multithreaded()) {
    std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

    எனில் (!Ptr.load(std::memory_order_relaxed)) {
      வெறுமை *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      Next = StaticList;
      StaticList = இது;
    }
  } இல்லையெனில் {
    ப்ரதிஜ்ஞா(!Ptr && !DeleterFn && !Next &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    Next = StaticList;
    StaticList = இது;
  }
}

வெறுமை ManagedStaticBase::destroy() மாறிலி {
  ப்ரதிஜ்ஞா(DeleterFn && "ManagedStatic not initialized correctly!");
  ப்ரதிஜ்ஞா(StaticList == இது &&
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
வெறுமை llvm::llvm_shutdown() {
  போது (StaticList)
    StaticList->destroy();
}
