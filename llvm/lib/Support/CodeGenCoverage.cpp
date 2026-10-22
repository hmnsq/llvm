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

using فضاینام llvm;

CodeGenCoverage::CodeGenCoverage() = پیشفرض;

تهی CodeGenCoverage::setCovered(uint64_t RuleID) {
  اگر (RuleCoverage.size() <= RuleID)
    RuleCoverage.resize(RuleID + 1, نادرست);
  RuleCoverage[RuleID] = درست;
}

منطقی CodeGenCoverage::isCovered(uint64_t RuleID) ثابت {
  اگر (RuleCoverage.size() <= RuleID)
    بازگشت نادرست;
  بازگشت RuleCoverage[RuleID];
}

iterator_range<CodeGenCoverage::const_covered_iterator>
CodeGenCoverage::covered() ثابت {
  بازگشت RuleCoverage.set_bits();
}

منطقی CodeGenCoverage::parse(MemoryBuffer &Buffer, StringRef BackendName) {
  ثابت نویسه *CurPtr = Buffer.getBufferStart();

  وقتی (CurPtr != Buffer.getBufferEnd()) {
    // Read the backend name from the input.
    ثابت نویسه *LexedBackendName = CurPtr;
    وقتی (*CurPtr++ != 0)
      ;
    اگر (CurPtr == Buffer.getBufferEnd())
      بازگشت نادرست; // Data is invalid, expected rule id's to follow.

    منطقی IsForThisBackend = BackendName == LexedBackendName;
    وقتی (CurPtr != Buffer.getBufferEnd()) {
      اگر (std::distance(CurPtr, Buffer.getBufferEnd()) < 8)
        بازگشت نادرست; // Data is invalid. Not enough bytes for another rule id.

      uint64_t RuleID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      اگر (RuleID == ~0ull)
        بشکن;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      اگر (IsForThisBackend)
        setCovered(RuleID);
    }
  }

  بازگشت درست;
}

منطقی CodeGenCoverage::emit(StringRef CoveragePrefix,
                           StringRef BackendName) ثابت {
  اگر (!CoveragePrefix.empty() && !RuleCoverage.empty()) {
    ایستا sys::SmartMutex<درست> OutputMutex;
    sys::SmartScopedLock<درست> Lock(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::to_string(sys::Process::getProcessId());

    std::string CoverageFilename = (CoveragePrefix + Pid).str();

    std::error_code EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::OF_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    اگر (EC)
      بازگشت نادرست;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().اكتب((ثابت نویسه *)&Zero, اندازه(unsigned نویسه));
    برای (uint64_t I : RuleCoverage.set_bits())
      CoverageFile->os().اكتب((ثابت نویسه *)&I, اندازه(uint64_t));
    CoverageFile->os().اكتب((ثابت نویسه *)&InvZero, اندازه(uint64_t));

    CoverageFile->keep();
  }

  بازگشت درست;
}

تهی CodeGenCoverage::reset() { RuleCoverage.resize(0); }
