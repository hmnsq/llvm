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
उपयुज्य नामाकाश llvm;

स्थिर ध्रुव Managed_Static_आधारः *StaticList = शून्यसूचकः;

स्थिर std::recursive_mutex *getManagedStaticMutex() {
  स्थिर std::recursive_mutex m;
  प्रत्यागम &m;
}

शून्य Managed_Static_आधारः::पञ्जिकाकोशः_Managed_Static(शून्य *(*Creator)(),
                                              शून्य (*Deleter)(शून्य*)) ध्रुव {
  प्रतिज्ञा(Creator);
  यदि (llvm_is_multithreaded()) {
    std::तालकम्_guard<std::recursive_mutex> तालकम्_2(*getManagedStaticMutex());

    यदि (!Ptr.आरोपय(std::memory_order_relaxed)) {
      शून्य *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      अग्र_2 = StaticList;
      StaticList = एतत्;
    }
  } अन्यथा {
    प्रतिज्ञा(!Ptr && !DeleterFn && !अग्र_2 &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    अग्र_2 = StaticList;
    StaticList = एतत्;
  }
}

शून्य Managed_Static_आधारः::destroy() ध्रुव {
  प्रतिज्ञा(DeleterFn && "ManagedStatic not initialized correctly!");
  प्रतिज्ञा(StaticList == एतत् &&
         "Not destroyed in reverse order of construction?");
  // Unlink from list.
  StaticList = अग्र_2;
  अग्र_2 = शून्यसूचकः;

  // Destroy memory.
  DeleterFn(Ptr);

  // Cleanup.
  Ptr = शून्यसूचकः;
  DeleterFn = शून्यसूचकः;
}

/// llvm_shutdown - Deallocate and destroy all ManagedStatic variables.
/// IMPORTANT: it's only safe to call llvm_shutdown() in single thread,
/// without any other threads executing LLVM APIs.
/// llvm_shutdown() should be the last use of LLVM APIs.
शून्य llvm::llvm_shutdown() {
  यावत् (StaticList)
    StaticList->destroy();
}
