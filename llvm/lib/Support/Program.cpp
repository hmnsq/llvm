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

#অন্তর্ভাৱ "llvm/Support/Program.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/Config/llvm-config.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
using নামাকাশ llvm;
using নামাকাশ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

স্থির তার্কিক Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *Err_বাৰ্তা,
                    BitVector *AffinityMask, তার্কিক DetachProcess);

পূর্ণাঙ্ক sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        unsigned SecondsToWait, unsigned MemoryLimit,
                        std::string *Err_বাৰ্তা, তার্কিক *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  প্রতিজ্ঞা(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  যদি (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_বাৰ্তা,
              AffinityMask, /*DetachProcess=*/মিছা)) {
    যদি (ExecutionFailed)
      *ExecutionFailed = মিছা;
    ProcessInfo Result = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        Err_বাৰ্তা, ProcStat);
    ঘূৰাই Result.ReturnCode;
  }

  যদি (ExecutionFailed)
    *ExecutionFailed = সঁচা;

  ঘূৰাই -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               unsigned MemoryLimit, std::string *Err_বাৰ্তা,
                               তার্কিক *ExecutionFailed, BitVector *AffinityMask,
                               তার্কিক DetachProcess) {
  প্রতিজ্ঞা(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  যদি (ExecutionFailed)
    *ExecutionFailed = মিছা;
  যদি (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_বাৰ্তা,
               AffinityMask, DetachProcess))
    যদি (ExecutionFailed)
      *ExecutionFailed = সঁচা;

  ঘূৰাই PI;
}

তার্কিক sys::command_সাৰি_Fits_Within_System_Limits(StringRef Program,
                                            ArrayRef<ধ্রুৱ ৱর্ণ *> Args) {
  SmallVector<StringRef, 8> StringRefArgs(Args);
  ঘূৰাই command_সাৰি_Fits_Within_System_Limits(Program, StringRefArgs);
}

শূন্য sys::printArg(raw_ostream &OS, StringRef Arg, তার্কিক Quote) {
  ধ্রুৱ তার্কিক Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  যদি (!Quote && !Escape) {
    OS << Arg;
    ঘূৰাই;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  বাবে (ধ্রুৱ auto c : Arg) {
    যদি (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#অন্তর্ভাৱ "Unix/Program.inc"
#endif
#ifdef _WIN32
#অন্তর্ভাৱ "Windows/Program.inc"
#endif
