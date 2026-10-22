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

CodeGenCoverage::CodeGenCoverage() = स्वभाव;

शून्य CodeGenCoverage::setCovered(uint64_t RuleID) {
  यदि (RuleCoverage.size() <= RuleID)
    RuleCoverage.resize(RuleID + 1, असत्य);
  RuleCoverage[RuleID] = सत्य;
}

तार्किक CodeGenCoverage::isCovered(uint64_t RuleID) अचर {
  यदि (RuleCoverage.size() <= RuleID)
    फर्काऊ असत्य;
  फर्काऊ RuleCoverage[RuleID];
}

iterator_range<CodeGenCoverage::const_covered_iterator>
CodeGenCoverage::covered() अचर {
  फर्काऊ RuleCoverage.set_bits();
}

तार्किक CodeGenCoverage::parse(MemoryBuffer &Buffer, StringRef BackendName) {
  अचर वर्ण *CurPtr = Buffer.getBufferStart();

  जबसम्म (CurPtr != Buffer.getBufferEnd()) {
    // Read the backend name from the input.
    अचर वर्ण *LexedBackendName = CurPtr;
    जबसम्म (*CurPtr++ != 0)
      ;
    यदि (CurPtr == Buffer.getBufferEnd())
      फर्काऊ असत्य; // Data is invalid, expected rule id's to follow.

    तार्किक IsForThisBackend = BackendName == LexedBackendName;
    जबसम्म (CurPtr != Buffer.getBufferEnd()) {
      यदि (std::distance(CurPtr, Buffer.getBufferEnd()) < 8)
        फर्काऊ असत्य; // Data is invalid. Not enough bytes for another rule id.

      uint64_t RuleID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      यदि (RuleID == ~0ull)
        रोक;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      यदि (IsForThisBackend)
        setCovered(RuleID);
    }
  }

  फर्काऊ सत्य;
}

तार्किक CodeGenCoverage::emit(StringRef CoveragePrefix,
                           StringRef BackendName) अचर {
  यदि (!CoveragePrefix.empty() && !RuleCoverage.empty()) {
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
    यदि (EC)
      फर्काऊ असत्य;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().लिख((अचर वर्ण *)&Zero, परिमाण(अचिन्हित वर्ण));
    लागि (uint64_t I : RuleCoverage.set_bits())
      CoverageFile->os().लिख((अचर वर्ण *)&I, परिमाण(uint64_t));
    CoverageFile->os().लिख((अचर वर्ण *)&InvZero, परिमाण(uint64_t));

    CoverageFile->keep();
  }

  फर्काऊ सत्य;
}

शून्य CodeGenCoverage::reset() { RuleCoverage.resize(0); }
