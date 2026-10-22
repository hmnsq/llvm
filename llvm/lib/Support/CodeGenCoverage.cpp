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

उपयुज्य नामाकाश llvm;

संहिता_Gen_Coverage::संहिता_Gen_Coverage() = स्वभाव;

शून्य संहिता_Gen_Coverage::समुच्चयः_Covered(uint64_t नियमः_ID) {
  यदि (नियमः_Coverage.आकार_2() <= नियमः_ID)
    नियमः_Coverage.resize(नियमः_ID + 1, असत्य);
  नियमः_Coverage[नियमः_ID] = सत्य;
}

तार्किक संहिता_Gen_Coverage::isCovered(uint64_t नियमः_ID) ध्रुव {
  यदि (नियमः_Coverage.आकार_2() <= नियमः_ID)
    प्रत्यागम असत्य;
  प्रत्यागम नियमः_Coverage[नियमः_ID];
}

iterator_range<संहिता_Gen_Coverage::const_covered_iterator>
संहिता_Gen_Coverage::covered() ध्रुव {
  प्रत्यागम नियमः_Coverage.समुच्चयः_द्वयङ्काः_2();
}

तार्किक संहिता_Gen_Coverage::parse(स्मृतिः_बफर &बफर, StringRef BackendName) {
  ध्रुव वर्ण *CurPtr = बफर.get_बफर_Start();

  यावत् (CurPtr != बफर.get_बफर_अन्तः()) {
    // Read the backend name from the input.
    ध्रुव वर्ण *LexedBackendName = CurPtr;
    यावत् (*CurPtr++ != 0)
      ;
    यदि (CurPtr == बफर.get_बफर_अन्तः())
      प्रत्यागम असत्य; // Data is invalid, expected rule id's to follow.

    तार्किक IsForThisBackend = BackendName == LexedBackendName;
    यावत् (CurPtr != बफर.get_बफर_अन्तः()) {
      यदि (std::distance(CurPtr, बफर.get_बफर_अन्तः()) < 8)
        प्रत्यागम असत्य; // Data is invalid. Not enough bytes for another rule id.

      uint64_t नियमः_ID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      यदि (नियमः_ID == ~0ull)
        भङ्ग;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      यदि (IsForThisBackend)
        समुच्चयः_Covered(नियमः_ID);
    }
  }

  प्रत्यागम सत्य;
}

तार्किक संहिता_Gen_Coverage::emit(StringRef Coverage_उपसर्गः,
                           StringRef BackendName) ध्रुव {
  यदि (!Coverage_उपसर्गः.रिक्तानि() && !नियमः_Coverage.रिक्तानि()) {
    स्थिर sys::SmartMutex<सत्य> OutputMutex;
    sys::Smart_Scoped_तालकम्<सत्य> तालकम्_2(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::गन्तव्यम्_string(sys::Process::getProcessId());

    std::string CoverageFilename = (Coverage_उपसर्गः + Pid).str();

    std::error_संहिता EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::सम्बन्धि_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    यदि (EC)
      प्रत्यागम असत्य;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().लिख((ध्रुव वर्ण *)&Zero, परिमाण(अचिह्नित वर्ण));
    प्रति (uint64_t I : नियमः_Coverage.समुच्चयः_द्वयङ्काः_2())
      CoverageFile->os().लिख((ध्रुव वर्ण *)&I, परिमाण(uint64_t));
    CoverageFile->os().लिख((ध्रुव वर्ण *)&InvZero, परिमाण(uint64_t));

    CoverageFile->keep();
  }

  प्रत्यागम सत्य;
}

शून्य संहिता_Gen_Coverage::reset() { नियमः_Coverage.resize(0); }
