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

#شامل "llvm/Support/CodeGenCoverage.h"

#شامل "llvm/Support/Endian.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Mutex.h"
#شامل "llvm/Support/Process.h"
#شامل "llvm/Support/ScopedPrinter.h"
#شامل "llvm/Support/ToolOutputFile.h"

استعمال نامخلا llvm;

رمز_Gen_Coverage::رمز_Gen_Coverage() = طےشدہ;

خالی رمز_Gen_Coverage::سیٹ_Covered(uint64_t قاعدہ_ID) {
  اگر (قاعدہ_Coverage.سائز_2() <= قاعدہ_ID)
    قاعدہ_Coverage.resize(قاعدہ_ID + 1, جھوٹ);
  قاعدہ_Coverage[قاعدہ_ID] = سچ;
}

منطقی رمز_Gen_Coverage::isCovered(uint64_t قاعدہ_ID) ثابت {
  اگر (قاعدہ_Coverage.سائز_2() <= قاعدہ_ID)
    واپس جھوٹ;
  واپس قاعدہ_Coverage[قاعدہ_ID];
}

iterator_range<رمز_Gen_Coverage::const_covered_iterator>
رمز_Gen_Coverage::covered() ثابت {
  واپس قاعدہ_Coverage.سیٹ_بٹ_2();
}

منطقی رمز_Gen_Coverage::parse(حافظہ_بفر &بفر, StringRef BackendName) {
  ثابت حرف *CurPtr = بفر.get_بفر_آغاز();

  جب (CurPtr != بفر.get_بفر_انتہا()) {
    // Read the backend name from the input.
    ثابت حرف *LexedBackendName = CurPtr;
    جب (*CurPtr++ != 0)
      ;
    اگر (CurPtr == بفر.get_بفر_انتہا())
      واپس جھوٹ; // Data is invalid, expected rule id's to follow.

    منطقی IsForThisBackend = BackendName == LexedBackendName;
    جب (CurPtr != بفر.get_بفر_انتہا()) {
      اگر (std::فاصلہ(CurPtr, بفر.get_بفر_انتہا()) < 8)
        واپس جھوٹ; // Data is invalid. Not enough bytes for another rule id.

      uint64_t قاعدہ_ID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      اگر (قاعدہ_ID == ~0ull)
        توڑو;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      اگر (IsForThisBackend)
        سیٹ_Covered(قاعدہ_ID);
    }
  }

  واپس سچ;
}

منطقی رمز_Gen_Coverage::emit(StringRef Coverage_سابقہ,
                           StringRef BackendName) ثابت {
  اگر (!Coverage_سابقہ.خالی_خانے() && !قاعدہ_Coverage.خالی_خانے()) {
    ساکن sys::SmartMutex<سچ> OutputMutex;
    sys::Smart_Scoped_تالا<سچ> تالا_3(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::مقصد_string(sys::Process::getProcessId());

    std::string CoverageFilename = (Coverage_سابقہ + Pid).str();

    std::error_رمز EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::کا_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    اگر (EC)
      واپس جھوٹ;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().اكتب((ثابت حرف *)&Zero, حجم(غیرعلامتی حرف));
    برائے (uint64_t I : قاعدہ_Coverage.سیٹ_بٹ_2())
      CoverageFile->os().اكتب((ثابت حرف *)&I, حجم(uint64_t));
    CoverageFile->os().اكتب((ثابت حرف *)&InvZero, حجم(uint64_t));

    CoverageFile->keep();
  }

  واپس سچ;
}

خالی رمز_Gen_Coverage::reset() { قاعدہ_Coverage.resize(0); }
