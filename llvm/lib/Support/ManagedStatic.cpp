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

#अन्तर्भाव "llvm/Support/ManagedStatic.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/Threading.h"
#अन्तर्भाव <cassert>
#अन्तर्भाव <mutex>
using नामाकाश llvm;

स्थिर अचर ManagedStaticBase *StaticList = nullptr;

स्थिर std::recursive_mutex *getManagedStaticMutex() {
  स्थिर std::recursive_mutex m;
  फर्काऊ &m;
}

शून्य ManagedStaticBase::RegisterManagedStatic(शून्य *(*Creator)(),
                                              शून्य (*Deleter)(शून्य*)) अचर {
  प्रतिज्ञा(Creator);
  यदि (llvm_is_multithreaded()) {
    std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

    यदि (!Ptr.load(std::memory_order_relaxed)) {
      शून्य *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      Next = StaticList;
      StaticList = एतत्;
    }
  } अन्यथा {
    प्रतिज्ञा(!Ptr && !DeleterFn && !Next &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    Next = StaticList;
    StaticList = एतत्;
  }
}

शून्य ManagedStaticBase::destroy() अचर {
  प्रतिज्ञा(DeleterFn && "ManagedStatic not initialized correctly!");
  प्रतिज्ञा(StaticList == एतत् &&
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
शून्य llvm::llvm_shutdown() {
  जबसम्म (StaticList)
    StaticList->destroy();
}
