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

استعمال نطاق llvm;

ڪوڊ_Gen_Coverage::ڪوڊ_Gen_Coverage() = افتراضي;

فراغ ڪوڊ_Gen_Coverage::سيٽ_Covered(uint64_t قاعدو_ID) {
  إذا (قاعدو_Coverage.سائيز_2() <= قاعدو_ID)
    قاعدو_Coverage.resize(قاعدو_ID + 1, خطأ);
  قاعدو_Coverage[قاعدو_ID] = سچ;
}

منطقي ڪوڊ_Gen_Coverage::isCovered(uint64_t قاعدو_ID) ثابت {
  إذا (قاعدو_Coverage.سائيز_2() <= قاعدو_ID)
    أرجع خطأ;
  أرجع قاعدو_Coverage[قاعدو_ID];
}

iterator_range<ڪوڊ_Gen_Coverage::const_covered_iterator>
ڪوڊ_Gen_Coverage::covered() ثابت {
  أرجع قاعدو_Coverage.سيٽ_بٽ_2();
}

منطقي ڪوڊ_Gen_Coverage::parse(يادگيري_بفر &بفر, StringRef BackendName) {
  ثابت حرف *CurPtr = بفر.get_بفر_شروعات();

  طالما (CurPtr != بفر.get_بفر_پڇاڙي()) {
    // Read the backend name from the input.
    ثابت حرف *LexedBackendName = CurPtr;
    طالما (*CurPtr++ != 0)
      ;
    إذا (CurPtr == بفر.get_بفر_پڇاڙي())
      أرجع خطأ; // Data is invalid, expected rule id's to follow.

    منطقي IsForThisBackend = BackendName == LexedBackendName;
    طالما (CurPtr != بفر.get_بفر_پڇاڙي()) {
      إذا (std::مفاصلو(CurPtr, بفر.get_بفر_پڇاڙي()) < 8)
        أرجع خطأ; // Data is invalid. Not enough bytes for another rule id.

      uint64_t قاعدو_ID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      إذا (قاعدو_ID == ~0ull)
        اكسر;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      إذا (IsForThisBackend)
        سيٽ_Covered(قاعدو_ID);
    }
  }

  أرجع سچ;
}

منطقي ڪوڊ_Gen_Coverage::emit(StringRef Coverage_اڳياڙي,
                           StringRef BackendName) ثابت {
  إذا (!Coverage_اڳياڙي.خالي_جايون() && !قاعدو_Coverage.خالي_جايون()) {
    ساكن sys::SmartMutex<سچ> OutputMutex;
    sys::Smart_Scoped_تالو<سچ> تالو_2(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::منزل_string(sys::Process::getProcessId());

    std::string CoverageFilename = (Coverage_اڳياڙي + Pid).str();

    std::error_ڪوڊ EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::جو_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    إذا (EC)
      أرجع خطأ;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().اكتب((ثابت حرف *)&Zero, حجم(بنانشان حرف));
    لكل (uint64_t I : قاعدو_Coverage.سيٽ_بٽ_2())
      CoverageFile->os().اكتب((ثابت حرف *)&I, حجم(uint64_t));
    CoverageFile->os().اكتب((ثابت حرف *)&InvZero, حجم(uint64_t));

    CoverageFile->keep();
  }

  أرجع سچ;
}

فراغ ڪوڊ_Gen_Coverage::reset() { قاعدو_Coverage.resize(0); }
