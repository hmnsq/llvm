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

#ಅನ್ತರ್ಭಾವ "llvm/Support/CodeGenCoverage.h"

#ಅನ್ತರ್ಭಾವ "llvm/Support/Endian.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Mutex.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Process.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ScopedPrinter.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ToolOutputFile.h"

using ನಾಮಾಕಾಶ llvm;

CodeGenCoverage::CodeGenCoverage() = ಸ್ವಭಾವ;

ಶೂನ್ಯ CodeGenCoverage::setCovered(uint64_t RuleID) {
  ಯದಿ (RuleCoverage.size() <= RuleID)
    RuleCoverage.resize(RuleID + 1, ಅಸತ್ಯ);
  RuleCoverage[RuleID] = ಸತ್ಯ;
}

ತಾರ್ಕಿಕ CodeGenCoverage::isCovered(uint64_t RuleID) ಧ್ರುವ {
  ಯದಿ (RuleCoverage.size() <= RuleID)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ RuleCoverage[RuleID];
}

iterator_range<CodeGenCoverage::const_covered_iterator>
CodeGenCoverage::covered() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ RuleCoverage.set_bits();
}

ತಾರ್ಕಿಕ CodeGenCoverage::parse(MemoryBuffer &Buffer, StringRef BackendName) {
  ಧ್ರುವ ವರ್ಣ *CurPtr = Buffer.getBufferStart();

  ಯಾವತ್ (CurPtr != Buffer.getBufferEnd()) {
    // Read the backend name from the input.
    ಧ್ರುವ ವರ್ಣ *LexedBackendName = CurPtr;
    ಯಾವತ್ (*CurPtr++ != 0)
      ;
    ಯದಿ (CurPtr == Buffer.getBufferEnd())
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ; // Data is invalid, expected rule id's to follow.

    ತಾರ್ಕಿಕ IsForThisBackend = BackendName == LexedBackendName;
    ಯಾವತ್ (CurPtr != Buffer.getBufferEnd()) {
      ಯದಿ (std::distance(CurPtr, Buffer.getBufferEnd()) < 8)
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ; // Data is invalid. Not enough bytes for another rule id.

      uint64_t RuleID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      ಯದಿ (RuleID == ~0ull)
        ಭಙ್ಗ;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      ಯದಿ (IsForThisBackend)
        setCovered(RuleID);
    }
  }

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ CodeGenCoverage::emit(StringRef CoveragePrefix,
                           StringRef BackendName) ಧ್ರುವ {
  ಯದಿ (!CoveragePrefix.empty() && !RuleCoverage.empty()) {
    ಸ್ಥಿರ sys::SmartMutex<ಸತ್ಯ> OutputMutex;
    sys::SmartScopedLock<ಸತ್ಯ> Lock(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::to_string(sys::Process::getProcessId());

    std::string CoverageFilename = (CoveragePrefix + Pid).str();

    std::error_code EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::OF_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    ಯದಿ (EC)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().ಲಿಖ((ಧ್ರುವ ವರ್ಣ *)&Zero, ಪರಿಮಾಣ(unsigned ವರ್ಣ));
    ಪ್ರತಿ (uint64_t I : RuleCoverage.set_bits())
      CoverageFile->os().ಲಿಖ((ಧ್ರುವ ವರ್ಣ *)&I, ಪರಿಮಾಣ(uint64_t));
    CoverageFile->os().ಲಿಖ((ಧ್ರುವ ವರ್ಣ *)&InvZero, ಪರಿಮಾಣ(uint64_t));

    CoverageFile->keep();
  }

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಶೂನ್ಯ CodeGenCoverage::reset() { RuleCoverage.resize(0); }
