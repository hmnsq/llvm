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

#समावेश "llvm/Support/CodeGenCoverage.h"

#समावेश "llvm/Support/Endian.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/Mutex.h"
#समावेश "llvm/Support/Process.h"
#समावेश "llvm/Support/ScopedPrinter.h"
#समावेश "llvm/Support/ToolOutputFile.h"

using namespace llvm;

CodeGenCoverage::CodeGenCoverage() = शेष;

व्योम CodeGenCoverage::setCovered(uint64_t RuleID) {
  अगर (RuleCoverage.आकार_2() <= RuleID)
    RuleCoverage.resize(RuleID + 1, false);
  RuleCoverage[RuleID] = true;
}

bool CodeGenCoverage::isCovered(uint64_t RuleID) स्थिर {
  अगर (RuleCoverage.आकार_2() <= RuleID)
    वापस false;
  वापस RuleCoverage[RuleID];
}

iterator_range<CodeGenCoverage::const_covered_iterator>
CodeGenCoverage::covered() स्थिर {
  वापस RuleCoverage.set_bits();
}

bool CodeGenCoverage::parse(Memory_बफर &बफर, StringRef BackendName) {
  स्थिर अक्षर *CurPtr = बफर.get_बफर_Start();

  जबतक (CurPtr != बफर.get_बफर_End()) {
    // Read the backend name from the input.
    स्थिर अक्षर *LexedBackendName = CurPtr;
    जबतक (*CurPtr++ != 0)
      ;
    अगर (CurPtr == बफर.get_बफर_End())
      वापस false; // Data is invalid, expected rule id's to follow.

    bool IsForThisBackend = BackendName == LexedBackendName;
    जबतक (CurPtr != बफर.get_बफर_End()) {
      अगर (std::distance(CurPtr, बफर.get_बफर_End()) < 8)
        वापस false; // Data is invalid. Not enough bytes for another rule id.

      uint64_t RuleID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      अगर (RuleID == ~0ull)
        अवरोध;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      अगर (IsForThisBackend)
        setCovered(RuleID);
    }
  }

  वापस true;
}

bool CodeGenCoverage::emit(StringRef CoveragePrefix,
                           StringRef BackendName) स्थिर {
  अगर (!CoveragePrefix.empty() && !RuleCoverage.empty()) {
    अटल sys::SmartMutex<true> OutputMutex;
    sys::SmartScopedLock<true> Lock(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::to_string(sys::Process::getProcessId());

    std::string CoverageFilename = (CoveragePrefix + Pid).str();

    std::error_code EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::OF_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    अगर (EC)
      वापस false;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().ग_लिखो((स्थिर अक्षर *)&Zero, माप(अचिन्हित अक्षर));
    क्रम (uint64_t I : RuleCoverage.set_bits())
      CoverageFile->os().ग_लिखो((स्थिर अक्षर *)&I, माप(uint64_t));
    CoverageFile->os().ग_लिखो((स्थिर अक्षर *)&InvZero, माप(uint64_t));

    CoverageFile->keep();
  }

  वापस true;
}

व्योम CodeGenCoverage::reset() { RuleCoverage.resize(0); }
