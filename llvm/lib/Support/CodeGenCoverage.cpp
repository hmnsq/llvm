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

ورتوں نطاق llvm;

کوڈ_Gen_Coverage::کوڈ_Gen_Coverage() = افتراضي;

فراغ کوڈ_Gen_Coverage::سیٹ_Covered(uint64_t قاعدہ_ID) {
  إذا (قاعدہ_Coverage.ناپ_2() <= قاعدہ_ID)
    قاعدہ_Coverage.resize(قاعدہ_ID + 1, خطأ);
  قاعدہ_Coverage[قاعدہ_ID] = سچ;
}

منطقي کوڈ_Gen_Coverage::isCovered(uint64_t قاعدہ_ID) ثابت {
  إذا (قاعدہ_Coverage.ناپ_2() <= قاعدہ_ID)
    أرجع خطأ;
  أرجع قاعدہ_Coverage[قاعدہ_ID];
}

iterator_range<کوڈ_Gen_Coverage::const_covered_iterator>
کوڈ_Gen_Coverage::covered() ثابت {
  أرجع قاعدہ_Coverage.سیٹ_بٹ_2();
}

منطقي کوڈ_Gen_Coverage::parse(حافظہ_بفر &بفر, StringRef BackendName) {
  ثابت حرف *CurPtr = بفر.get_بفر_شروع();

  طالما (CurPtr != بفر.get_بفر_سرا()) {
    // Read the backend name from the input.
    ثابت حرف *LexedBackendName = CurPtr;
    طالما (*CurPtr++ != 0)
      ;
    إذا (CurPtr == بفر.get_بفر_سرا())
      أرجع خطأ; // Data is invalid, expected rule id's to follow.

    منطقي IsForThisBackend = BackendName == LexedBackendName;
    طالما (CurPtr != بفر.get_بفر_سرا()) {
      إذا (std::فاصلہ(CurPtr, بفر.get_بفر_سرا()) < 8)
        أرجع خطأ; // Data is invalid. Not enough bytes for another rule id.

      uint64_t قاعدہ_ID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      إذا (قاعدہ_ID == ~0ull)
        اكسر;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      إذا (IsForThisBackend)
        سیٹ_Covered(قاعدہ_ID);
    }
  }

  أرجع سچ;
}

منطقي کوڈ_Gen_Coverage::emit(StringRef Coverage_اگیتر,
                           StringRef BackendName) ثابت {
  إذا (!Coverage_اگیتر.خالی_تھاواں() && !قاعدہ_Coverage.خالی_تھاواں()) {
    ساكن sys::SmartMutex<سچ> OutputMutex;
    sys::Smart_Scoped_جندرا<سچ> جندرا_2(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::منزل_string(sys::Process::getProcessId());

    std::string CoverageFilename = (Coverage_اگیتر + Pid).str();

    std::error_کوڈ EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::دا_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    إذا (EC)
      أرجع خطأ;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().اكتب((ثابت حرف *)&Zero, حجم(بنانشانی حرف));
    لكل (uint64_t I : قاعدہ_Coverage.سیٹ_بٹ_2())
      CoverageFile->os().اكتب((ثابت حرف *)&I, حجم(uint64_t));
    CoverageFile->os().اكتب((ثابت حرف *)&InvZero, حجم(uint64_t));

    CoverageFile->keep();
  }

  أرجع سچ;
}

فراغ کوڈ_Gen_Coverage::reset() { قاعدہ_Coverage.resize(0); }
