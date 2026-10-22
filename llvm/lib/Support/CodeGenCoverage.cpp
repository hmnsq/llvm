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

#अन्तर्भाव "llvm/Support/CodeGenCoverage.h"

#अन्तर्भाव "llvm/Support/Endian.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Mutex.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/ScopedPrinter.h"
#अन्तर्भाव "llvm/Support/ToolOutputFile.h"

using नामाकाश llvm;

CodeGenCoverage::CodeGenCoverage() = पूर्वनिर्धारित;

शून्य CodeGenCoverage::setCovered(uint64_t RuleID) {
  जर (RuleCoverage.size() <= RuleID)
    RuleCoverage.resize(RuleID + 1, असत्य);
  RuleCoverage[RuleID] = सत्य;
}

तार्किक CodeGenCoverage::isCovered(uint64_t RuleID) स्थिरांक {
  जर (RuleCoverage.size() <= RuleID)
    परत असत्य;
  परत RuleCoverage[RuleID];
}

iterator_range<CodeGenCoverage::const_covered_iterator>
CodeGenCoverage::covered() स्थिरांक {
  परत RuleCoverage.set_bits();
}

तार्किक CodeGenCoverage::parse(MemoryBuffer &Buffer, StringRef BackendName) {
  स्थिरांक अक्षर *CurPtr = Buffer.getBufferStart();

  पर्यंत (CurPtr != Buffer.getBufferEnd()) {
    // Read the backend name from the input.
    स्थिरांक अक्षर *LexedBackendName = CurPtr;
    पर्यंत (*CurPtr++ != 0)
      ;
    जर (CurPtr == Buffer.getBufferEnd())
      परत असत्य; // Data is invalid, expected rule id's to follow.

    तार्किक IsForThisBackend = BackendName == LexedBackendName;
    पर्यंत (CurPtr != Buffer.getBufferEnd()) {
      जर (std::distance(CurPtr, Buffer.getBufferEnd()) < 8)
        परत असत्य; // Data is invalid. Not enough bytes for another rule id.

      uint64_t RuleID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      जर (RuleID == ~0ull)
        थांबा;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      जर (IsForThisBackend)
        setCovered(RuleID);
    }
  }

  परत सत्य;
}

तार्किक CodeGenCoverage::emit(StringRef CoveragePrefix,
                           StringRef BackendName) स्थिरांक {
  जर (!CoveragePrefix.empty() && !RuleCoverage.empty()) {
    स्थिर sys::SmartMutex<सत्य> OutputMutex;
    sys::SmartScopedLock<सत्य> Lock(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::to_string(sys::Process::getProcessId());

    std::string CoverageFilename = (CoveragePrefix + Pid).str();

    std::error_code EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::OF_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    जर (EC)
      परत असत्य;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().लिख((स्थिरांक अक्षर *)&Zero, आकार(अचिन्हित अक्षर));
    साठी (uint64_t I : RuleCoverage.set_bits())
      CoverageFile->os().लिख((स्थिरांक अक्षर *)&I, आकार(uint64_t));
    CoverageFile->os().लिख((स्थिरांक अक्षर *)&InvZero, आकार(uint64_t));

    CoverageFile->keep();
  }

  परत सत्य;
}

शून्य CodeGenCoverage::reset() { RuleCoverage.resize(0); }
