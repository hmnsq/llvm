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

using نطاق llvm;

CodeGenCoverage::CodeGenCoverage() = افتراضي;

فراغ CodeGenCoverage::setCovered(uint64_t RuleID) {
  ܐܢ (RuleCoverage.size() <= RuleID)
    RuleCoverage.resize(RuleID + 1, ܕܓܠܐ);
  RuleCoverage[RuleID] = ܫܪܪܐ;
}

منطقي CodeGenCoverage::isCovered(uint64_t RuleID) ثابت {
  ܐܢ (RuleCoverage.size() <= RuleID)
    ܦܢܐ ܕܓܠܐ;
  ܦܢܐ RuleCoverage[RuleID];
}

iterator_range<CodeGenCoverage::const_covered_iterator>
CodeGenCoverage::covered() ثابت {
  ܦܢܐ RuleCoverage.set_bits();
}

منطقي CodeGenCoverage::parse(MemoryBuffer &Buffer, StringRef BackendName) {
  ثابت حرف *CurPtr = Buffer.getBufferStart();

  ܟܡܐ (CurPtr != Buffer.getBufferEnd()) {
    // Read the backend name from the input.
    ثابت حرف *LexedBackendName = CurPtr;
    ܟܡܐ (*CurPtr++ != 0)
      ;
    ܐܢ (CurPtr == Buffer.getBufferEnd())
      ܦܢܐ ܕܓܠܐ; // Data is invalid, expected rule id's to follow.

    منطقي IsForThisBackend = BackendName == LexedBackendName;
    ܟܡܐ (CurPtr != Buffer.getBufferEnd()) {
      ܐܢ (std::distance(CurPtr, Buffer.getBufferEnd()) < 8)
        ܦܢܐ ܕܓܠܐ; // Data is invalid. Not enough bytes for another rule id.

      uint64_t RuleID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      ܐܢ (RuleID == ~0ull)
        اكسر;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      ܐܢ (IsForThisBackend)
        setCovered(RuleID);
    }
  }

  ܦܢܐ ܫܪܪܐ;
}

منطقي CodeGenCoverage::emit(StringRef CoveragePrefix,
                           StringRef BackendName) ثابت {
  ܐܢ (!CoveragePrefix.empty() && !RuleCoverage.empty()) {
    ساكن sys::SmartMutex<ܫܪܪܐ> OutputMutex;
    sys::SmartScopedLock<ܫܪܪܐ> Lock(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::to_string(sys::Process::getProcessId());

    std::string CoverageFilename = (CoveragePrefix + Pid).str();

    std::error_code EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::OF_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    ܐܢ (EC)
      ܦܢܐ ܕܓܠܐ;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().اكتب((ثابت حرف *)&Zero, حجم(unsigned حرف));
    لكل (uint64_t I : RuleCoverage.set_bits())
      CoverageFile->os().اكتب((ثابت حرف *)&I, حجم(uint64_t));
    CoverageFile->os().اكتب((ثابت حرف *)&InvZero, حجم(uint64_t));

    CoverageFile->keep();
  }

  ܦܢܐ ܫܪܪܐ;
}

فراغ CodeGenCoverage::reset() { RuleCoverage.resize(0); }
