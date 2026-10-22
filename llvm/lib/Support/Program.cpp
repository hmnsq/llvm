//===-- Program.cpp - Implement OS Program Concept --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the operating system Program concept.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/Program.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ಸ್ಥಿರ ತಾರ್ಕಿಕ Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *Err_ಸಂದೇಶ,
                    BitVector *AffinityMask, ತಾರ್ಕಿಕ DetachProcess);

ಪೂರ್ಣಾಙ್ಕ sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        unsigned SecondsToWait, unsigned MemoryLimit,
                        std::string *Err_ಸಂದೇಶ, ತಾರ್ಕಿಕ *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  ಪ್ರತಿಜ್ಞಾ(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  ಯದಿ (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_ಸಂದೇಶ,
              AffinityMask, /*DetachProcess=*/ಅಸತ್ಯ)) {
    ಯದಿ (ExecutionFailed)
      *ExecutionFailed = ಅಸತ್ಯ;
    ProcessInfo Result = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        Err_ಸಂದೇಶ, ProcStat);
    ಪ್ರತ್ಯಾಗಮ Result.ReturnCode;
  }

  ಯದಿ (ExecutionFailed)
    *ExecutionFailed = ಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               unsigned MemoryLimit, std::string *Err_ಸಂದೇಶ,
                               ತಾರ್ಕಿಕ *ExecutionFailed, BitVector *AffinityMask,
                               ತಾರ್ಕಿಕ DetachProcess) {
  ಪ್ರತಿಜ್ಞಾ(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  ಯದಿ (ExecutionFailed)
    *ExecutionFailed = ಅಸತ್ಯ;
  ಯದಿ (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_ಸಂದೇಶ,
               AffinityMask, DetachProcess))
    ಯದಿ (ExecutionFailed)
      *ExecutionFailed = ಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ PI;
}

ತಾರ್ಕಿಕ sys::command_ಸಾಲು_Fits_Within_System_Limits(StringRef Program,
                                            ArrayRef<ಧ್ರುವ ವರ್ಣ *> Args) {
  SmallVector<StringRef, 8> StringRefArgs(Args);
  ಪ್ರತ್ಯಾಗಮ command_ಸಾಲು_Fits_Within_System_Limits(Program, StringRefArgs);
}

ಶೂನ್ಯ sys::printArg(raw_ostream &OS, StringRef Arg, ತಾರ್ಕಿಕ Quote) {
  ಧ್ರುವ ತಾರ್ಕಿಕ Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  ಯದಿ (!Quote && !Escape) {
    OS << Arg;
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  ಪ್ರತಿ (ಧ್ರುವ auto c : Arg) {
    ಯದಿ (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ಅನ್ತರ್ಭಾವ "Unix/Program.inc"
#endif
#ifdef _WIN32
#ಅನ್ತರ್ಭಾವ "Windows/Program.inc"
#endif
