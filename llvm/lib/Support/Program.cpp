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

#അന്തര്ഭാവ "llvm/Support/Program.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
using നാമാകാശ llvm;
using നാമാകാശ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

സ്ഥിതം തർക്കം Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *Err_സന്ദേശം,
                    BitVector *AffinityMask, തർക്കം DetachProcess);

പൂർണ്ണസംഖ്യ sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        unsigned SecondsToWait, unsigned MemoryLimit,
                        std::string *Err_സന്ദേശം, തർക്കം *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  പ്രതിജ്ഞാ(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  എങ്കിൽ (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_സന്ദേശം,
              AffinityMask, /*DetachProcess=*/അസത്യം)) {
    എങ്കിൽ (ExecutionFailed)
      *ExecutionFailed = അസത്യം;
    ProcessInfo Result = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        Err_സന്ദേശം, ProcStat);
    തിരികെ Result.ReturnCode;
  }

  എങ്കിൽ (ExecutionFailed)
    *ExecutionFailed = സത്യം;

  തിരികെ -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               unsigned MemoryLimit, std::string *Err_സന്ദേശം,
                               തർക്കം *ExecutionFailed, BitVector *AffinityMask,
                               തർക്കം DetachProcess) {
  പ്രതിജ്ഞാ(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  എങ്കിൽ (ExecutionFailed)
    *ExecutionFailed = അസത്യം;
  എങ്കിൽ (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_സന്ദേശം,
               AffinityMask, DetachProcess))
    എങ്കിൽ (ExecutionFailed)
      *ExecutionFailed = സത്യം;

  തിരികെ PI;
}

തർക്കം sys::command_വരി_Fits_Within_System_Limits(StringRef Program,
                                            ArrayRef<സ്ഥിരം അക്ഷരം *> Args) {
  SmallVector<StringRef, 8> StringRefArgs(Args);
  തിരികെ command_വരി_Fits_Within_System_Limits(Program, StringRefArgs);
}

ശൂന്യം sys::printArg(raw_ostream &OS, StringRef Arg, തർക്കം Quote) {
  സ്ഥിരം തർക്കം Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  എങ്കിൽ (!Quote && !Escape) {
    OS << Arg;
    തിരികെ;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  വേണ്ടി (സ്ഥിരം auto c : Arg) {
    എങ്കിൽ (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#അന്തര്ഭാവ "Unix/Program.inc"
#endif
#ifdef _WIN32
#അന്തര്ഭാവ "Windows/Program.inc"
#endif
