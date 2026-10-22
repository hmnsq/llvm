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

#অন্তর্ভাব "llvm/Support/Program.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
using নামাকাশ llvm;
using নামাকাশ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

স্থির যৌক্তিক Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *Err_বার্তা,
                    BitVector *AffinityMask, যৌক্তিক DetachProcess);

পূর্ণসংখ্যা sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        unsigned SecondsToWait, unsigned MemoryLimit,
                        std::string *Err_বার্তা, যৌক্তিক *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  প্রতিজ্ঞা(Redirects.empty() || Redirects.আকার_3() == 3);
  ProcessInfo PI;
  যদি (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_বার্তা,
              AffinityMask, /*DetachProcess=*/মিথ্যা)) {
    যদি (ExecutionFailed)
      *ExecutionFailed = মিথ্যা;
    ProcessInfo ফলাফল = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        Err_বার্তা, ProcStat);
    ফেরত ফলাফল.ReturnCode;
  }

  যদি (ExecutionFailed)
    *ExecutionFailed = সত্য;

  ফেরত -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               unsigned MemoryLimit, std::string *Err_বার্তা,
                               যৌক্তিক *ExecutionFailed, BitVector *AffinityMask,
                               যৌক্তিক DetachProcess) {
  প্রতিজ্ঞা(Redirects.empty() || Redirects.আকার_3() == 3);
  ProcessInfo PI;
  যদি (ExecutionFailed)
    *ExecutionFailed = মিথ্যা;
  যদি (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_বার্তা,
               AffinityMask, DetachProcess))
    যদি (ExecutionFailed)
      *ExecutionFailed = সত্য;

  ফেরত PI;
}

যৌক্তিক sys::command_সারি_Fits_Within_System_Limits(StringRef Program,
                                            ArrayRef<ধ্রুবক অক্ষর *> Args) {
  SmallVector<StringRef, 8> StringRefArgs(Args);
  ফেরত command_সারি_Fits_Within_System_Limits(Program, StringRefArgs);
}

শূন্য sys::printArg(raw_ostream &OS, StringRef Arg, যৌক্তিক Quote) {
  ধ্রুবক যৌক্তিক Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  যদি (!Quote && !Escape) {
    OS << Arg;
    ফেরত;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  জন্য (ধ্রুবক auto c : Arg) {
    যদি (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#অন্তর্ভাব "Unix/Program.inc"
#endif
#ifdef _WIN32
#অন্তর্ভাব "Windows/Program.inc"
#endif
