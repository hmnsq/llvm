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

#അന്തര്ഭാവ "llvm/Support/ManagedStatic.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/Threading.h"
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <mutex>
using നാമാകാശ llvm;

സ്ഥിതം സ്ഥിരം ManagedStaticBase *StaticList = nullptr;

സ്ഥിതം std::recursive_mutex *getManagedStaticMutex() {
  സ്ഥിതം std::recursive_mutex m;
  തിരികെ &m;
}

ശൂന്യം ManagedStaticBase::RegisterManagedStatic(ശൂന്യം *(*Creator)(),
                                              ശൂന്യം (*Deleter)(ശൂന്യം*)) സ്ഥിരം {
  പ്രതിജ്ഞാ(Creator);
  എങ്കിൽ (llvm_is_multithreaded()) {
    std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

    എങ്കിൽ (!Ptr.load(std::memory_order_relaxed)) {
      ശൂന്യം *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      Next = StaticList;
      StaticList = ഇത്;
    }
  } അല്ലെങ്കിൽ {
    പ്രതിജ്ഞാ(!Ptr && !DeleterFn && !Next &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    Next = StaticList;
    StaticList = ഇത്;
  }
}

ശൂന്യം ManagedStaticBase::destroy() സ്ഥിരം {
  പ്രതിജ്ഞാ(DeleterFn && "ManagedStatic not initialized correctly!");
  പ്രതിജ്ഞാ(StaticList == ഇത് &&
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
ശൂന്യം llvm::llvm_shutdown() {
  ആകുമ്പോൾ (StaticList)
    StaticList->destroy();
}
