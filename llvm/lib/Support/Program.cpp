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

#અન્તર્ભાવ "llvm/Support/Program.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
using નામાકાશ llvm;
using નામાકાશ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

સ્થિર તાર્કિક Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *Err_સંદેશ,
                    BitVector *AffinityMask, તાર્કિક DetachProcess);

પૂર્ણાંક sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        unsigned SecondsToWait, unsigned MemoryLimit,
                        std::string *Err_સંદેશ, તાર્કિક *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  પ્રતિજ્ઞા(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  જો (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_સંદેશ,
              AffinityMask, /*DetachProcess=*/ખોટું)) {
    જો (ExecutionFailed)
      *ExecutionFailed = ખોટું;
    ProcessInfo Result = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        Err_સંદેશ, ProcStat);
    પાછા Result.ReturnCode;
  }

  જો (ExecutionFailed)
    *ExecutionFailed = સાચું;

  પાછા -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               unsigned MemoryLimit, std::string *Err_સંદેશ,
                               તાર્કિક *ExecutionFailed, BitVector *AffinityMask,
                               તાર્કિક DetachProcess) {
  પ્રતિજ્ઞા(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  જો (ExecutionFailed)
    *ExecutionFailed = ખોટું;
  જો (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_સંદેશ,
               AffinityMask, DetachProcess))
    જો (ExecutionFailed)
      *ExecutionFailed = સાચું;

  પાછા PI;
}

તાર્કિક sys::command_લીટી_Fits_Within_System_Limits(StringRef Program,
                                            ArrayRef<અચલ અક્ષર *> Args) {
  SmallVector<StringRef, 8> StringRefArgs(Args);
  પાછા command_લીટી_Fits_Within_System_Limits(Program, StringRefArgs);
}

શૂન્ય sys::printArg(raw_ostream &OS, StringRef Arg, તાર્કિક Quote) {
  અચલ તાર્કિક Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  જો (!Quote && !Escape) {
    OS << Arg;
    પાછા;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  માટે (અચલ auto c : Arg) {
    જો (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#અન્તર્ભાવ "Unix/Program.inc"
#endif
#ifdef _WIN32
#અન્તર્ભાવ "Windows/Program.inc"
#endif
