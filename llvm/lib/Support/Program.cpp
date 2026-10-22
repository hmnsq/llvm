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

#ضمن "llvm/Support/Program.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/raw_ostream.h"
using نطاق llvm;
using نطاق sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ساكن منطقي Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *ErrMsg,
                    BitVector *AffinityMask, منطقي DetachProcess);

صحيح sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        unsigned SecondsToWait, unsigned MemoryLimit,
                        std::string *ErrMsg, منطقي *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  أكد(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  𐤀𐤌 (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, ErrMsg,
              AffinityMask, /*DetachProcess=*/خطأ)) {
    𐤀𐤌 (ExecutionFailed)
      *ExecutionFailed = خطأ;
    ProcessInfo Result = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        ErrMsg, ProcStat);
    أرجع Result.ReturnCode;
  }

  𐤀𐤌 (ExecutionFailed)
    *ExecutionFailed = true;

  أرجع -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               unsigned MemoryLimit, std::string *ErrMsg,
                               منطقي *ExecutionFailed, BitVector *AffinityMask,
                               منطقي DetachProcess) {
  أكد(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  𐤀𐤌 (ExecutionFailed)
    *ExecutionFailed = خطأ;
  𐤀𐤌 (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, ErrMsg,
               AffinityMask, DetachProcess))
    𐤀𐤌 (ExecutionFailed)
      *ExecutionFailed = true;

  أرجع PI;
}

منطقي sys::commandLineFitsWithinSystemLimits(StringRef Program,
                                            ArrayRef<ثابت حرف *> Args) {
  SmallVector<StringRef, 8> StringRefArgs(Args);
  أرجع commandLineFitsWithinSystemLimits(Program, StringRefArgs);
}

فراغ sys::printArg(raw_ostream &OS, StringRef Arg, منطقي Quote) {
  ثابت منطقي Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  𐤀𐤌 (!Quote && !Escape) {
    OS << Arg;
    أرجع;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  لكل (ثابت auto c : Arg) {
    𐤀𐤌 (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ضمن "Unix/Program.inc"
#endif
#ifdef _WIN32
#ضمن "Windows/Program.inc"
#endif
