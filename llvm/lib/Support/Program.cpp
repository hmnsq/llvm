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

#شامل "llvm/Support/Program.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/raw_ostream.h"
using فضاینام llvm;
using فضاینام sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ایستا منطقی Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *Err_پیام,
                    BitVector *AffinityMask, منطقی DetachProcess);

صحیح sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        unsigned SecondsToWait, unsigned MemoryLimit,
                        std::string *Err_پیام, منطقی *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  أكد(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  اگر (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_پیام,
              AffinityMask, /*DetachProcess=*/نادرست)) {
    اگر (ExecutionFailed)
      *ExecutionFailed = نادرست;
    ProcessInfo Result = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        Err_پیام, ProcStat);
    بازگشت Result.ReturnCode;
  }

  اگر (ExecutionFailed)
    *ExecutionFailed = درست;

  بازگشت -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               unsigned MemoryLimit, std::string *Err_پیام,
                               منطقی *ExecutionFailed, BitVector *AffinityMask,
                               منطقی DetachProcess) {
  أكد(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  اگر (ExecutionFailed)
    *ExecutionFailed = نادرست;
  اگر (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_پیام,
               AffinityMask, DetachProcess))
    اگر (ExecutionFailed)
      *ExecutionFailed = درست;

  بازگشت PI;
}

منطقی sys::command_سطر_Fits_Within_System_Limits(StringRef Program,
                                            ArrayRef<ثابت نویسه *> Args) {
  SmallVector<StringRef, 8> StringRefArgs(Args);
  بازگشت command_سطر_Fits_Within_System_Limits(Program, StringRefArgs);
}

تهی sys::printArg(raw_ostream &OS, StringRef Arg, منطقی Quote) {
  ثابت منطقی Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  اگر (!Quote && !Escape) {
    OS << Arg;
    بازگشت;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  برای (ثابت auto c : Arg) {
    اگر (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#شامل "Unix/Program.inc"
#endif
#ifdef _WIN32
#شامل "Windows/Program.inc"
#endif
