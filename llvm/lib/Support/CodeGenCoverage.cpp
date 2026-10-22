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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CodeGenCoverage.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Endian.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Mutex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ScopedPrinter.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ToolOutputFile.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਕੋਡ_Gen_Coverage::ਕੋਡ_Gen_Coverage() = ਮੂਲ;

ਖਾਲੀ ਕੋਡ_Gen_Coverage::ਸੈੱਟ_Covered(uint64_t ਨਿਯਮ_ID) {
  ਜੇ (ਨਿਯਮ_Coverage.ਮਾਪ_2() <= ਨਿਯਮ_ID)
    ਨਿਯਮ_Coverage.resize(ਨਿਯਮ_ID + 1, ਝੂਠ);
  ਨਿਯਮ_Coverage[ਨਿਯਮ_ID] = ਸੱਚ;
}

ਤਾਰਕਿਕ ਕੋਡ_Gen_Coverage::isCovered(uint64_t ਨਿਯਮ_ID) ਅਚਲ {
  ਜੇ (ਨਿਯਮ_Coverage.ਮਾਪ_2() <= ਨਿਯਮ_ID)
    ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ ਨਿਯਮ_Coverage[ਨਿਯਮ_ID];
}

iterator_range<ਕੋਡ_Gen_Coverage::const_covered_iterator>
ਕੋਡ_Gen_Coverage::covered() ਅਚਲ {
  ਵਾਪਸ ਨਿਯਮ_Coverage.ਸੈੱਟ_ਬਿੱਟ_2();
}

ਤਾਰਕਿਕ ਕੋਡ_Gen_Coverage::parse(ਸਮ੍ਰਿਤੀ_ਬਫਰ &ਬਫਰ, StringRef BackendName) {
  ਅਚਲ ਅੱਖਰ *CurPtr = ਬਫਰ.get_ਬਫਰ_ਸ਼ੁਰੂ();

  ਜਦੋਂ (CurPtr != ਬਫਰ.get_ਬਫਰ_ਸਿਰਾ()) {
    // Read the backend name from the input.
    ਅਚਲ ਅੱਖਰ *LexedBackendName = CurPtr;
    ਜਦੋਂ (*CurPtr++ != 0)
      ;
    ਜੇ (CurPtr == ਬਫਰ.get_ਬਫਰ_ਸਿਰਾ())
      ਵਾਪਸ ਝੂਠ; // Data is invalid, expected rule id's to follow.

    ਤਾਰਕਿਕ IsForThisBackend = BackendName == LexedBackendName;
    ਜਦੋਂ (CurPtr != ਬਫਰ.get_ਬਫਰ_ਸਿਰਾ()) {
      ਜੇ (std::ਦੂਰੀ(CurPtr, ਬਫਰ.get_ਬਫਰ_ਸਿਰਾ()) < 8)
        ਵਾਪਸ ਝੂਠ; // Data is invalid. Not enough bytes for another rule id.

      uint64_t ਨਿਯਮ_ID =
          support::endian::read64(CurPtr, llvm::endianness::native);
      CurPtr += 8;

      // ~0ull terminates the rule id list.
      ਜੇ (ਨਿਯਮ_ID == ~0ull)
        ਰੋਕੋ;

      // Anything else, is recorded or ignored depending on whether it's
      // intended for the backend we're interested in.
      ਜੇ (IsForThisBackend)
        ਸੈੱਟ_Covered(ਨਿਯਮ_ID);
    }
  }

  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ ਕੋਡ_Gen_Coverage::emit(StringRef Coverage_ਅਗੇਤਰ,
                           StringRef BackendName) ਅਚਲ {
  ਜੇ (!Coverage_ਅਗੇਤਰ.ਖਾਲੀ_ਥਾਵਾਂ() && !ਨਿਯਮ_Coverage.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਸਥਿਰ sys::SmartMutex<ਸੱਚ> OutputMutex;
    sys::Smart_Scoped_ਤਾਲਾ<ਸੱਚ> ਤਾਲਾ_2(OutputMutex);

    // We can handle locking within a process easily enough but we don't want to
    // manage it between multiple processes. Use the process ID to ensure no
    // more than one process is ever writing to the same file at the same time.
    std::string Pid = llvm::ਮੰਜ਼ਿਲ_string(sys::Process::getProcessId());

    std::string CoverageFilename = (Coverage_ਅਗੇਤਰ + Pid).str();

    std::error_ਕੋਡ EC;
    sys::fs::OpenFlags OpenFlags = sys::fs::ਦਾ_Append;
    std::unique_ptr<ToolOutputFile> CoverageFile =
        std::make_unique<ToolOutputFile>(CoverageFilename, EC, OpenFlags);
    ਜੇ (EC)
      ਵਾਪਸ ਝੂਠ;

    uint64_t Zero = 0;
    uint64_t InvZero = ~0ull;
    CoverageFile->os() << BackendName;
    CoverageFile->os().ਲਿਖ((ਅਚਲ ਅੱਖਰ *)&Zero, ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ));
    ਲਈ (uint64_t I : ਨਿਯਮ_Coverage.ਸੈੱਟ_ਬਿੱਟ_2())
      CoverageFile->os().ਲਿਖ((ਅਚਲ ਅੱਖਰ *)&I, ਆਕਾਰ(uint64_t));
    CoverageFile->os().ਲਿਖ((ਅਚਲ ਅੱਖਰ *)&InvZero, ਆਕਾਰ(uint64_t));

    CoverageFile->keep();
  }

  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ ਕੋਡ_Gen_Coverage::reset() { ਨਿਯਮ_Coverage.resize(0); }
