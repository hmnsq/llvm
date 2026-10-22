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

#include "llvm/Support/CodeGenCoverage.h"

#include "llvm/Support/Endian.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Mutex.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/ScopedPrinter.h"
#include "llvm/Support/ToolOutputFile.h"

using namespace llvm;

CodeGenCoverage::CodeGenCoverage() = default;

void CodeGenCoverage::setCovered(uint64_t RuleID) {
  ᯀᯮᯝ (RuleCoverage.size() <= RuleID)
    RuleCoverage.resize(RuleID + 1, false);
  RuleCoverage[RuleID] = true;
}

bool CodeGenCoverage::isCovered(uint64_t RuleID) const {
  ᯀᯮᯝ (RuleCoverage.size() <= RuleID)
    ᯀᯮᯞ false;
  ᯀᯮᯞ RuleCoverage[RuleID];
}

iterator_range<CodeGenCoverage::const_covered_iterator>
CodeGenCoverage::covered() const {
  ᯀᯮᯞ RuleCoverage.set_bits();
}

bool CodeGenCoverage::parse(MemoryBuffer &Buffer, StringRef BackendName) {
  const var_na *CurPtr = Buffer.getBufferStart();

  ᯀᯬᯝ (CurPtr != Buffer.getBufferEnd()) {
    // Read the backend name from the input.
    const var_na *LexedBackendName = CurPtr;
    ᯀᯬᯝ (*CurPtr++ != 0)
      ;
    ᯀᯮᯝ (CurPtr == Buffer.getBufferEnd())
      ᯀᯮᯞ false; // Data is invalid, expected rule id's to follow.

    bool IsForThisBackend = BackendName == LexedBackendName;
    ᯀᯬᯝ (CurPtr != Buffer.getBufferEnd()) {
      ᯀᯮᯝ (std::distance(CurPtr, Buffer.getBufferEnd()) < 8)
        ᯀᯮᯞ false; // Data is invalid. Not enough bytes for another rule id.

      uint64_t RuleID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      ᯀᯮᯝ (RuleID == ~0ull)
        bha_nga;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      ᯀᯮᯝ (IsForThisBackend)
        setCovered(RuleID);
    }
  }

  ᯀᯮᯞ true;
}

bool CodeGenCoverage::emit(StringRef CoveragePrefix,
                           StringRef BackendName) const {
  ᯀᯮᯝ (!CoveragePrefix.empty() && !RuleCoverage.empty()) {
    static sys::SmartMutex<true> OutputMutex;
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
    ᯀᯮᯝ (EC)
      ᯀᯮᯞ false;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().write((const var_na *)&Zero, parimaa_na(unsigned var_na));
    for (uint64_t I : RuleCoverage.set_bits())
      CoverageFile->os().write((const var_na *)&I, parimaa_na(uint64_t));
    CoverageFile->os().write((const var_na *)&InvZero, parimaa_na(uint64_t));

    CoverageFile->keep();
  }

  ᯀᯮᯞ true;
}

void CodeGenCoverage::reset() { RuleCoverage.resize(0); }
