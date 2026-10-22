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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ManagedStatic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Threading.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <mutex>
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਥਿਰ ਅਚਲ Managed_Static_ਅਧਾਰ *StaticList = ਖਾਲੀ_ਸੂਚਕ;

ਸਥਿਰ std::recursive_mutex *getManagedStaticMutex() {
  ਸਥਿਰ std::recursive_mutex m;
  ਵਾਪਸ &m;
}

ਖਾਲੀ Managed_Static_ਅਧਾਰ::ਰਜਿਸਟਰ_Managed_Static(ਖਾਲੀ *(*Creator)(),
                                              ਖਾਲੀ (*Deleter)(ਖਾਲੀ*)) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(Creator);
  ਜੇ (llvm_is_multithreaded()) {
    std::ਤਾਲਾ_guard<std::recursive_mutex> ਤਾਲਾ_2(*getManagedStaticMutex());

    ਜੇ (!Ptr.ਲੱਦੋ(std::memory_order_relaxed)) {
      ਖਾਲੀ *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      ਅਗਲਾ_2 = StaticList;
      StaticList = ਇਹ;
    }
  } ਨਹੀਂਤਾਂ {
    ਪ੍ਰਤਿਜ੍ਞਾ(!Ptr && !DeleterFn && !ਅਗਲਾ_2 &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    ਅਗਲਾ_2 = StaticList;
    StaticList = ਇਹ;
  }
}

ਖਾਲੀ Managed_Static_ਅਧਾਰ::destroy() ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(DeleterFn && "ManagedStatic not initialized correctly!");
  ਪ੍ਰਤਿਜ੍ਞਾ(StaticList == ਇਹ &&
         "Not destroyed in reverse order of construction?");
  // Unlink from list.
  StaticList = ਅਗਲਾ_2;
  ਅਗਲਾ_2 = ਖਾਲੀ_ਸੂਚਕ;

  // Destroy memory.
  DeleterFn(Ptr);

  // Cleanup.
  Ptr = ਖਾਲੀ_ਸੂਚਕ;
  DeleterFn = ਖਾਲੀ_ਸੂਚਕ;
}

/// llvm_shutdown - Deallocate and destroy all ManagedStatic variables.
/// IMPORTANT: it's only safe to call llvm_shutdown() in single thread,
/// without any other threads executing LLVM APIs.
/// llvm_shutdown() should be the last use of LLVM APIs.
ਖਾਲੀ llvm::llvm_shutdown() {
  ਜਦੋਂ (StaticList)
    StaticList->destroy();
}
