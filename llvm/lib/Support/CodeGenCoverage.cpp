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

#અન્તર્ભાવ "llvm/Support/CodeGenCoverage.h"

#અન્તર્ભાવ "llvm/Support/Endian.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"
#અન્તર્ભાવ "llvm/Support/Mutex.h"
#અન્તર્ભાવ "llvm/Support/Process.h"
#અન્તર્ભાવ "llvm/Support/ScopedPrinter.h"
#અન્તર્ભાવ "llvm/Support/ToolOutputFile.h"

using નામાકાશ llvm;

CodeGenCoverage::CodeGenCoverage() = મૂળભૂત;

શૂન્ય CodeGenCoverage::setCovered(uint64_t RuleID) {
  જો (RuleCoverage.size() <= RuleID)
    RuleCoverage.resize(RuleID + 1, ખોટું);
  RuleCoverage[RuleID] = સાચું;
}

તાર્કિક CodeGenCoverage::isCovered(uint64_t RuleID) અચલ {
  જો (RuleCoverage.size() <= RuleID)
    પાછા ખોટું;
  પાછા RuleCoverage[RuleID];
}

iterator_range<CodeGenCoverage::const_covered_iterator>
CodeGenCoverage::covered() અચલ {
  પાછા RuleCoverage.set_bits();
}

તાર્કિક CodeGenCoverage::parse(MemoryBuffer &Buffer, StringRef BackendName) {
  અચલ અક્ષર *CurPtr = Buffer.getBufferStart();

  જ્યારે (CurPtr != Buffer.getBufferEnd()) {
    // Read the backend name from the input.
    અચલ અક્ષર *LexedBackendName = CurPtr;
    જ્યારે (*CurPtr++ != 0)
      ;
    જો (CurPtr == Buffer.getBufferEnd())
      પાછા ખોટું; // Data is invalid, expected rule id's to follow.

    તાર્કિક IsForThisBackend = BackendName == LexedBackendName;
    જ્યારે (CurPtr != Buffer.getBufferEnd()) {
      જો (std::distance(CurPtr, Buffer.getBufferEnd()) < 8)
        પાછા ખોટું; // Data is invalid. Not enough bytes for another rule id.

      uint64_t RuleID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      જો (RuleID == ~0ull)
        તોડો;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      જો (IsForThisBackend)
        setCovered(RuleID);
    }
  }

  પાછા સાચું;
}

તાર્કિક CodeGenCoverage::emit(StringRef CoveragePrefix,
                           StringRef BackendName) અચલ {
  જો (!CoveragePrefix.empty() && !RuleCoverage.empty()) {
    સ્થિર sys::SmartMutex<સાચું> OutputMutex;
    sys::SmartScopedLock<સાચું> Lock(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::to_string(sys::Process::getProcessId());

    std::string CoverageFilename = (CoveragePrefix + Pid).str();

    std::error_code EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::OF_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    જો (EC)
      પાછા ખોટું;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().લિખ((અચલ અક્ષર *)&Zero, કદ(unsigned અક્ષર));
    માટે (uint64_t I : RuleCoverage.set_bits())
      CoverageFile->os().લિખ((અચલ અક્ષર *)&I, કદ(uint64_t));
    CoverageFile->os().લિખ((અચલ અક્ષર *)&InvZero, કદ(uint64_t));

    CoverageFile->keep();
  }

  પાછા સાચું;
}

શૂન્ય CodeGenCoverage::reset() { RuleCoverage.resize(0); }
