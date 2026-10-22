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

#అన్తర్భావ "llvm/Support/CodeGenCoverage.h"

#అన్తర్భావ "llvm/Support/Endian.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/Support/Mutex.h"
#అన్తర్భావ "llvm/Support/Process.h"
#అన్తర్భావ "llvm/Support/ScopedPrinter.h"
#అన్తర్భావ "llvm/Support/ToolOutputFile.h"

ఉపయోగించి నామాకాశ llvm;

సంకేతం_Gen_Coverage::సంకేతం_Gen_Coverage() = అప్రమేయ;

శూన్య సంకేతం_Gen_Coverage::సమితి_Covered(uint64_t నియమం_ID) {
  ఒకవేళ (నియమం_Coverage.కొలత_2() <= నియమం_ID)
    నియమం_Coverage.resize(నియమం_ID + 1, అబద్ధం);
  నియమం_Coverage[నియమం_ID] = నిజం;
}

తార్కిక సంకేతం_Gen_Coverage::isCovered(uint64_t నియమం_ID) స్థిరరాశి {
  ఒకవేళ (నియమం_Coverage.కొలత_2() <= నియమం_ID)
    తిరిగి అబద్ధం;
  తిరిగి నియమం_Coverage[నియమం_ID];
}

iterator_range<సంకేతం_Gen_Coverage::const_covered_iterator>
సంకేతం_Gen_Coverage::covered() స్థిరరాశి {
  తిరిగి నియమం_Coverage.సమితి_బిట్లు_2();
}

తార్కిక సంకేతం_Gen_Coverage::parse(జ్ఞాపకం_బఫరు &బఫరు, StringRef BackendName) {
  స్థిరరాశి అక్షరం *CurPtr = బఫరు.get_బఫరు_ప్రారంభం();

  అయితే (CurPtr != బఫరు.get_బఫరు_చివర()) {
    // Read the backend name from the input.
    స్థిరరాశి అక్షరం *LexedBackendName = CurPtr;
    అయితే (*CurPtr++ != 0)
      ;
    ఒకవేళ (CurPtr == బఫరు.get_బఫరు_చివర())
      తిరిగి అబద్ధం; // Data is invalid, expected rule id's to follow.

    తార్కిక IsForThisBackend = BackendName == LexedBackendName;
    అయితే (CurPtr != బఫరు.get_బఫరు_చివర()) {
      ఒకవేళ (std::దూరం(CurPtr, బఫరు.get_బఫరు_చివర()) < 8)
        తిరిగి అబద్ధం; // Data is invalid. Not enough bytes for another rule id.

      uint64_t నియమం_ID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      ఒకవేళ (నియమం_ID == ~0ull)
        ఆపు;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      ఒకవేళ (IsForThisBackend)
        సమితి_Covered(నియమం_ID);
    }
  }

  తిరిగి నిజం;
}

తార్కిక సంకేతం_Gen_Coverage::emit(StringRef Coverage_ఉపసర్గ,
                           StringRef BackendName) స్థిరరాశి {
  ఒకవేళ (!Coverage_ఉపసర్గ.ఖాళీలు() && !నియమం_Coverage.ఖాళీలు()) {
    స్థిర sys::SmartMutex<నిజం> OutputMutex;
    sys::Smart_Scoped_తాళం<నిజం> తాళం_3(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::గమ్యం_string(sys::Process::getProcessId());

    std::string CoverageFilename = (Coverage_ఉపసర్గ + Pid).str();

    std::error_సంకేతం EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::యొక్క_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    ఒకవేళ (EC)
      తిరిగి అబద్ధం;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().లిఖ((స్థిరరాశి అక్షరం *)&Zero, పరిమాణం(గుర్తులేని అక్షరం));
    కొరకు (uint64_t I : నియమం_Coverage.సమితి_బిట్లు_2())
      CoverageFile->os().లిఖ((స్థిరరాశి అక్షరం *)&I, పరిమాణం(uint64_t));
    CoverageFile->os().లిఖ((స్థిరరాశి అక్షరం *)&InvZero, పరిమాణం(uint64_t));

    CoverageFile->keep();
  }

  తిరిగి నిజం;
}

శూన్య సంకేతం_Gen_Coverage::reset() { నియమం_Coverage.resize(0); }
