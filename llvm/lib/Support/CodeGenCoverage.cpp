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

#அந்தர்பாவ "llvm/Support/CodeGenCoverage.h"

#அந்தர்பாவ "llvm/Support/Endian.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/Mutex.h"
#அந்தர்பாவ "llvm/Support/Process.h"
#அந்தர்பாவ "llvm/Support/ScopedPrinter.h"
#அந்தர்பாவ "llvm/Support/ToolOutputFile.h"

using நாமாகாஶ llvm;

CodeGenCoverage::CodeGenCoverage() = இயல்பு;

வெறுமை CodeGenCoverage::setCovered(uint64_t RuleID) {
  எனில் (RuleCoverage.size() <= RuleID)
    RuleCoverage.resize(RuleID + 1, பொய்);
  RuleCoverage[RuleID] = உண்மை;
}

தருக்க CodeGenCoverage::isCovered(uint64_t RuleID) மாறிலி {
  எனில் (RuleCoverage.size() <= RuleID)
    திரும்பு பொய்;
  திரும்பு RuleCoverage[RuleID];
}

iterator_range<CodeGenCoverage::const_covered_iterator>
CodeGenCoverage::covered() மாறிலி {
  திரும்பு RuleCoverage.set_bits();
}

தருக்க CodeGenCoverage::parse(MemoryBuffer &Buffer, StringRef BackendName) {
  மாறிலி எழுத்து *CurPtr = Buffer.getBufferStart();

  போது (CurPtr != Buffer.getBufferEnd()) {
    // Read the backend name from the input.
    மாறிலி எழுத்து *LexedBackendName = CurPtr;
    போது (*CurPtr++ != 0)
      ;
    எனில் (CurPtr == Buffer.getBufferEnd())
      திரும்பு பொய்; // Data is invalid, expected rule id's to follow.

    தருக்க IsForThisBackend = BackendName == LexedBackendName;
    போது (CurPtr != Buffer.getBufferEnd()) {
      எனில் (std::distance(CurPtr, Buffer.getBufferEnd()) < 8)
        திரும்பு பொய்; // Data is invalid. Not enough bytes for another rule id.

      uint64_t RuleID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      எனில் (RuleID == ~0ull)
        நிறுத்து;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      எனில் (IsForThisBackend)
        setCovered(RuleID);
    }
  }

  திரும்பு உண்மை;
}

தருக்க CodeGenCoverage::emit(StringRef CoveragePrefix,
                           StringRef BackendName) மாறிலி {
  எனில் (!CoveragePrefix.empty() && !RuleCoverage.empty()) {
    நிலையான sys::SmartMutex<உண்மை> OutputMutex;
    sys::SmartScopedLock<உண்மை> Lock(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::to_string(sys::Process::getProcessId());

    std::string CoverageFilename = (CoveragePrefix + Pid).str();

    std::error_code EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::OF_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    எனில் (EC)
      திரும்பு பொய்;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().லிக((மாறிலி எழுத்து *)&Zero, அளவு(unsigned எழுத்து));
    க்கு (uint64_t I : RuleCoverage.set_bits())
      CoverageFile->os().லிக((மாறிலி எழுத்து *)&I, அளவு(uint64_t));
    CoverageFile->os().லிக((மாறிலி எழுத்து *)&InvZero, அளவு(uint64_t));

    CoverageFile->keep();
  }

  திரும்பு உண்மை;
}

வெறுமை CodeGenCoverage::reset() { RuleCoverage.resize(0); }
