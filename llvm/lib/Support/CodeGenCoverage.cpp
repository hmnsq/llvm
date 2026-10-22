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

#অন্তর্ভাব "llvm/Support/CodeGenCoverage.h"

#অন্তর্ভাব "llvm/Support/Endian.h"
#অন্তর্ভাব "llvm/Support/FileSystem.h"
#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাব "llvm/Support/Mutex.h"
#অন্তর্ভাব "llvm/Support/Process.h"
#অন্তর্ভাব "llvm/Support/ScopedPrinter.h"
#অন্তর্ভাব "llvm/Support/ToolOutputFile.h"

using নামাকাশ llvm;

CodeGenCoverage::CodeGenCoverage() = পূর্বনির্ধারিত;

শূন্য CodeGenCoverage::setCovered(uint64_t RuleID) {
  যদি (RuleCoverage.আকার_3() <= RuleID)
    RuleCoverage.resize(RuleID + 1, মিথ্যা);
  RuleCoverage[RuleID] = সত্য;
}

যৌক্তিক CodeGenCoverage::isCovered(uint64_t RuleID) ধ্রুবক {
  যদি (RuleCoverage.আকার_3() <= RuleID)
    ফেরত মিথ্যা;
  ফেরত RuleCoverage[RuleID];
}

iterator_range<CodeGenCoverage::const_covered_iterator>
CodeGenCoverage::covered() ধ্রুবক {
  ফেরত RuleCoverage.set_bits();
}

যৌক্তিক CodeGenCoverage::parse(Memory_বাফার &বাফার, StringRef BackendName) {
  ধ্রুবক অক্ষর *CurPtr = বাফার.get_বাফার_Start();

  যখন (CurPtr != বাফার.get_বাফার_End()) {
    // Read the backend name from the input.
    ধ্রুবক অক্ষর *LexedBackendName = CurPtr;
    যখন (*CurPtr++ != 0)
      ;
    যদি (CurPtr == বাফার.get_বাফার_End())
      ফেরত মিথ্যা; // Data is invalid, expected rule id's to follow.

    যৌক্তিক IsForThisBackend = BackendName == LexedBackendName;
    যখন (CurPtr != বাফার.get_বাফার_End()) {
      যদি (std::distance(CurPtr, বাফার.get_বাফার_End()) < 8)
        ফেরত মিথ্যা; // Data is invalid. Not enough bytes for another rule id.

      uint64_t RuleID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      যদি (RuleID == ~0ull)
        বিরতি;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      যদি (IsForThisBackend)
        setCovered(RuleID);
    }
  }

  ফেরত সত্য;
}

যৌক্তিক CodeGenCoverage::emit(StringRef CoveragePrefix,
                           StringRef BackendName) ধ্রুবক {
  যদি (!CoveragePrefix.empty() && !RuleCoverage.empty()) {
    স্থির sys::SmartMutex<সত্য> OutputMutex;
    sys::SmartScopedLock<সত্য> Lock(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::to_string(sys::Process::getProcessId());

    std::string CoverageFilename = (CoveragePrefix + Pid).str();

    std::error_code EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::OF_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    যদি (EC)
      ফেরত মিথ্যা;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().লিখ((ধ্রুবক অক্ষর *)&Zero, আকার(unsigned অক্ষর));
    জন্য (uint64_t I : RuleCoverage.set_bits())
      CoverageFile->os().লিখ((ধ্রুবক অক্ষর *)&I, আকার(uint64_t));
    CoverageFile->os().লিখ((ধ্রুবক অক্ষর *)&InvZero, আকার(uint64_t));

    CoverageFile->keep();
  }

  ফেরত সত্য;
}

শূন্য CodeGenCoverage::reset() { RuleCoverage.resize(0); }
