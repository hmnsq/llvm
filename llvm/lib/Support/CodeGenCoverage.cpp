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

استفاده نطاق llvm;

کد_Gen_Coverage::کد_Gen_Coverage() = افتراضي;

فراغ کد_Gen_Coverage::مجموعه_Covered(uint64_t قاعده_ID) {
  إذا (قاعده_Coverage.اندازه_2() <= قاعده_ID)
    قاعده_Coverage.resize(قاعده_ID + 1, خطأ);
  قاعده_Coverage[قاعده_ID] = درست;
}

منطقي کد_Gen_Coverage::isCovered(uint64_t قاعده_ID) ثابت {
  إذا (قاعده_Coverage.اندازه_2() <= قاعده_ID)
    أرجع خطأ;
  أرجع قاعده_Coverage[قاعده_ID];
}

iterator_range<کد_Gen_Coverage::const_covered_iterator>
کد_Gen_Coverage::covered() ثابت {
  أرجع قاعده_Coverage.مجموعه_بیتها_2();
}

منطقي کد_Gen_Coverage::parse(حافظه_میانگیر &میانگیر, StringRef BackendName) {
  ثابت حرف *CurPtr = میانگیر.get_میانگیر_شروع();

  طالما (CurPtr != میانگیر.get_میانگیر_انتها()) {
    // Read the backend name from the input.
    ثابت حرف *LexedBackendName = CurPtr;
    طالما (*CurPtr++ != 0)
      ;
    إذا (CurPtr == میانگیر.get_میانگیر_انتها())
      أرجع خطأ; // Data is invalid, expected rule id's to follow.

    منطقي IsForThisBackend = BackendName == LexedBackendName;
    طالما (CurPtr != میانگیر.get_میانگیر_انتها()) {
      إذا (std::فاصله(CurPtr, میانگیر.get_میانگیر_انتها()) < 8)
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
        مجموعه_Covered(قاعده_ID);
    }
  }

  أرجع درست;
}

منطقي کد_Gen_Coverage::emit(StringRef Coverage_پیشوند,
                           StringRef BackendName) ثابت {
  إذا (!Coverage_پیشوند.خالی() && !قاعده_Coverage.خالی()) {
    ساكن sys::SmartMutex<درست> OutputMutex;
    sys::Smart_Scoped_قفل<درست> قفل_3(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::مقصد_string(sys::Process::getProcessId());

    std::string CoverageFilename = (Coverage_پیشوند + Pid).str();

    std::error_کد EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::از_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    إذا (EC)
      أرجع خطأ;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().اكتب((ثابت حرف *)&Zero, حجم(بیعلامت حرف));
    لكل (uint64_t I : قاعده_Coverage.مجموعه_بیتها_2())
      CoverageFile->os().اكتب((ثابت حرف *)&I, حجم(uint64_t));
    CoverageFile->os().اكتب((ثابت حرف *)&InvZero, حجم(uint64_t));

    CoverageFile->keep();
  }

  أرجع درست;
}

فراغ کد_Gen_Coverage::reset() { قاعده_Coverage.resize(0); }
