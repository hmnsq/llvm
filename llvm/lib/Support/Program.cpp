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

#அந்தர்பாவ "llvm/Support/Program.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
using நாமாகாஶ llvm;
using நாமாகாஶ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

நிலையான தருக்க Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *Err_செய்தி,
                    BitVector *AffinityMask, தருக்க DetachProcess);

முழு sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        unsigned SecondsToWait, unsigned MemoryLimit,
                        std::string *Err_செய்தி, தருக்க *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  ப்ரதிஜ்ஞா(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  எனில் (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_செய்தி,
              AffinityMask, /*DetachProcess=*/பொய்)) {
    எனில் (ExecutionFailed)
      *ExecutionFailed = பொய்;
    ProcessInfo Result = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        Err_செய்தி, ProcStat);
    திரும்பு Result.ReturnCode;
  }

  எனில் (ExecutionFailed)
    *ExecutionFailed = உண்மை;

  திரும்பு -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               unsigned MemoryLimit, std::string *Err_செய்தி,
                               தருக்க *ExecutionFailed, BitVector *AffinityMask,
                               தருக்க DetachProcess) {
  ப்ரதிஜ்ஞா(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  எனில் (ExecutionFailed)
    *ExecutionFailed = பொய்;
  எனில் (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_செய்தி,
               AffinityMask, DetachProcess))
    எனில் (ExecutionFailed)
      *ExecutionFailed = உண்மை;

  திரும்பு PI;
}

தருக்க sys::command_வரி_Fits_Within_System_Limits(StringRef Program,
                                            ArrayRef<மாறிலி எழுத்து *> Args) {
  SmallVector<StringRef, 8> StringRefArgs(Args);
  திரும்பு command_வரி_Fits_Within_System_Limits(Program, StringRefArgs);
}

வெறுமை sys::printArg(raw_ostream &OS, StringRef Arg, தருக்க Quote) {
  மாறிலி தருக்க Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  எனில் (!Quote && !Escape) {
    OS << Arg;
    திரும்பு;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  க்கு (மாறிலி auto c : Arg) {
    எனில் (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#அந்தர்பாவ "Unix/Program.inc"
#endif
#ifdef _WIN32
#அந்தர்பாவ "Windows/Program.inc"
#endif
