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

#അന്തര്ഭാവ "llvm/Support/CodeGenCoverage.h"

#അന്തര്ഭാവ "llvm/Support/Endian.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/Support/Mutex.h"
#അന്തര്ഭാവ "llvm/Support/Process.h"
#അന്തര്ഭാവ "llvm/Support/ScopedPrinter.h"
#അന്തര്ഭാവ "llvm/Support/ToolOutputFile.h"

using നാമാകാശ llvm;

CodeGenCoverage::CodeGenCoverage() = സ്ഥിരസ്ഥിതി;

ശൂന്യം CodeGenCoverage::setCovered(uint64_t RuleID) {
  എങ്കിൽ (RuleCoverage.size() <= RuleID)
    RuleCoverage.resize(RuleID + 1, അസത്യം);
  RuleCoverage[RuleID] = സത്യം;
}

തർക്കം CodeGenCoverage::isCovered(uint64_t RuleID) സ്ഥിരം {
  എങ്കിൽ (RuleCoverage.size() <= RuleID)
    തിരികെ അസത്യം;
  തിരികെ RuleCoverage[RuleID];
}

iterator_range<CodeGenCoverage::const_covered_iterator>
CodeGenCoverage::covered() സ്ഥിരം {
  തിരികെ RuleCoverage.set_bits();
}

തർക്കം CodeGenCoverage::parse(MemoryBuffer &Buffer, StringRef BackendName) {
  സ്ഥിരം അക്ഷരം *CurPtr = Buffer.getBufferStart();

  ആകുമ്പോൾ (CurPtr != Buffer.getBufferEnd()) {
    // Read the backend name from the input.
    സ്ഥിരം അക്ഷരം *LexedBackendName = CurPtr;
    ആകുമ്പോൾ (*CurPtr++ != 0)
      ;
    എങ്കിൽ (CurPtr == Buffer.getBufferEnd())
      തിരികെ അസത്യം; // Data is invalid, expected rule id's to follow.

    തർക്കം IsForThisBackend = BackendName == LexedBackendName;
    ആകുമ്പോൾ (CurPtr != Buffer.getBufferEnd()) {
      എങ്കിൽ (std::distance(CurPtr, Buffer.getBufferEnd()) < 8)
        തിരികെ അസത്യം; // Data is invalid. Not enough bytes for another rule id.

      uint64_t RuleID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      എങ്കിൽ (RuleID == ~0ull)
        നിർത്തുക;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      എങ്കിൽ (IsForThisBackend)
        setCovered(RuleID);
    }
  }

  തിരികെ സത്യം;
}

തർക്കം CodeGenCoverage::emit(StringRef CoveragePrefix,
                           StringRef BackendName) സ്ഥിരം {
  എങ്കിൽ (!CoveragePrefix.empty() && !RuleCoverage.empty()) {
    സ്ഥിതം sys::SmartMutex<സത്യം> OutputMutex;
    sys::SmartScopedLock<സത്യം> Lock(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::to_string(sys::Process::getProcessId());

    std::string CoverageFilename = (CoveragePrefix + Pid).str();

    std::error_code EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::OF_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    എങ്കിൽ (EC)
      തിരികെ അസത്യം;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().ലിഖ((സ്ഥിരം അക്ഷരം *)&Zero, വലുപ്പം(unsigned അക്ഷരം));
    വേണ്ടി (uint64_t I : RuleCoverage.set_bits())
      CoverageFile->os().ലിഖ((സ്ഥിരം അക്ഷരം *)&I, വലുപ്പം(uint64_t));
    CoverageFile->os().ലിഖ((സ്ഥിരം അക്ഷരം *)&InvZero, വലുപ്പം(uint64_t));

    CoverageFile->keep();
  }

  തിരികെ സത്യം;
}

ശൂന്യം CodeGenCoverage::reset() { RuleCoverage.resize(0); }
