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

#అన్తర్భావ "llvm/Support/ManagedStatic.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/Threading.h"
#అన్తర్భావ <cassert>
#అన్తర్భావ <mutex>
ఉపయోగించి నామాకాశ llvm;

స్థిర స్థిరరాశి Managed_Static_పునాది *StaticList = శూన్యసూచిక;

స్థిర std::recursive_mutex *getManagedStaticMutex() {
  స్థిర std::recursive_mutex m;
  తిరిగి &m;
}

శూన్య Managed_Static_పునాది::రిజిస్టరు_Managed_Static(శూన్య *(*Creator)(),
                                              శూన్య (*Deleter)(శూన్య*)) స్థిరరాశి {
  ప్రతిజ్ఞా(Creator);
  ఒకవేళ (llvm_is_multithreaded()) {
    std::తాళం_guard<std::recursive_mutex> తాళం_3(*getManagedStaticMutex());

    ఒకవేళ (!Ptr.ఎక్కించు(std::memory_order_relaxed)) {
      శూన్య *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      తదుపరి_2 = StaticList;
      StaticList = ఇది;
    }
  } లేకపోతే {
    ప్రతిజ్ఞా(!Ptr && !DeleterFn && !తదుపరి_2 &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    తదుపరి_2 = StaticList;
    StaticList = ఇది;
  }
}

శూన్య Managed_Static_పునాది::destroy() స్థిరరాశి {
  ప్రతిజ్ఞా(DeleterFn && "ManagedStatic not initialized correctly!");
  ప్రతిజ్ఞా(StaticList == ఇది &&
         "Not destroyed in reverse order of construction?");
  // Unlink from list.
  StaticList = తదుపరి_2;
  తదుపరి_2 = శూన్యసూచిక;

  // Destroy memory.
  DeleterFn(Ptr);

  // Cleanup.
  Ptr = శూన్యసూచిక;
  DeleterFn = శూన్యసూచిక;
}

/// llvm_shutdown - Deallocate and destroy all ManagedStatic variables.
/// IMPORTANT: it's only safe to call llvm_shutdown() in single thread,
/// without any other threads executing LLVM APIs.
/// llvm_shutdown() should be the last use of LLVM APIs.
శూన్య llvm::llvm_shutdown() {
  అయితే (StaticList)
    StaticList->destroy();
}
