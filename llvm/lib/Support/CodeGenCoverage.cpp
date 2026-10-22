//===- lib/Support/CodeGenCoverage.cpp -------------------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the CodeGenCoverage class.
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/CodeGenCoverage.h"

#ضمن "llvm/Support/Endian.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Mutex.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/ScopedPrinter.h"
#ضمن "llvm/Support/ToolOutputFile.h"

کارول نطاق llvm;

کوډ_Gen_Coverage::کوډ_Gen_Coverage() = افتراضي;

فراغ کوډ_Gen_Coverage::سېټ_Covered(uint64_t قاعده_ID) {
  إذا (قاعده_Coverage.اندازه_2() <= قاعده_ID)
    قاعده_Coverage.resize(قاعده_ID + 1, خطأ);
  قاعده_Coverage[قاعده_ID] = ریښتیا;
}

منطقي کوډ_Gen_Coverage::isCovered(uint64_t قاعده_ID) ثابت {
  إذا (قاعده_Coverage.اندازه_2() <= قاعده_ID)
    أرجع خطأ;
  أرجع قاعده_Coverage[قاعده_ID];
}

iterator_range<کوډ_Gen_Coverage::const_covered_iterator>
کوډ_Gen_Coverage::covered() ثابت {
  أرجع قاعده_Coverage.سېټ_بټونه_2();
}

منطقي کوډ_Gen_Coverage::parse(حافظه_بفر &بفر, StringRef BackendName) {
  ثابت حرف *CurPtr = بفر.get_بفر_پیل();

  طالما (CurPtr != بفر.get_بفر_پای_ټکی()) {
    // Read the backend name from the input.
    ثابت حرف *LexedBackendName = CurPtr;
    طالما (*CurPtr++ != 0)
      ;
    إذا (CurPtr == بفر.get_بفر_پای_ټکی())
      أرجع خطأ; // Data is invalid, expected rule id's to follow.

    منطقي IsForThisBackend = BackendName == LexedBackendName;
    طالما (CurPtr != بفر.get_بفر_پای_ټکی()) {
      إذا (std::واټن(CurPtr, بفر.get_بفر_پای_ټکی()) < 8)
        أرجع خطأ; // Data is invalid. Not enough bytes for another rule id.

      uint64_t قاعده_ID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      إذا (قاعده_ID == ~0ull)
        اكسر;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      إذا (IsForThisBackend)
        سېټ_Covered(قاعده_ID);
    }
  }

  أرجع ریښتیا;
}

منطقي کوډ_Gen_Coverage::emit(StringRef Coverage_مختاړی,
                           StringRef BackendName) ثابت {
  إذا (!Coverage_مختاړی.تشه() && !قاعده_Coverage.تشه()) {
    ساكن sys::SmartMutex<ریښتیا> OutputMutex;
    sys::Smart_Scoped_قلف<ریښتیا> قلف_2(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::منزل_string(sys::Process::getProcessId());

    std::string CoverageFilename = (Coverage_مختاړی + Pid).str();

    std::error_کوډ EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::د_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    إذا (EC)
      أرجع خطأ;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().اكتب((ثابت حرف *)&Zero, حجم(بینښه حرف));
    لكل (uint64_t I : قاعده_Coverage.سېټ_بټونه_2())
      CoverageFile->os().اكتب((ثابت حرف *)&I, حجم(uint64_t));
    CoverageFile->os().اكتب((ثابت حرف *)&InvZero, حجم(uint64_t));

    CoverageFile->keep();
  }

  أرجع ریښتیا;
}

فراغ کوډ_Gen_Coverage::reset() { قاعده_Coverage.resize(0); }
