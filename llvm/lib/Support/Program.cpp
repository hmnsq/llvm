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

#include "llvm/Support/Program.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
using namespace sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

static bool Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *ErrMsg,
                    BitVector *AffinityMask, bool DetachProcess);

puur_naa_nka sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        unsigned SecondsToWait, unsigned MemoryLimit,
                        std::string *ErrMsg, bool *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  assert(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  ꦪꦼꦤ꧀ (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, ErrMsg,
              AffinityMask, /*DetachProcess=*/false)) {
    ꦪꦼꦤ꧀ (ExecutionFailed)
      *ExecutionFailed = false;
    ProcessInfo Result = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        ErrMsg, ProcStat);
    ꦮꦭꦶ Result.ReturnCode;
  }

  ꦪꦼꦤ꧀ (ExecutionFailed)
    *ExecutionFailed = true;

  ꦮꦭꦶ -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               unsigned MemoryLimit, std::string *ErrMsg,
                               bool *ExecutionFailed, BitVector *AffinityMask,
                               bool DetachProcess) {
  assert(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  ꦪꦼꦤ꧀ (ExecutionFailed)
    *ExecutionFailed = false;
  ꦪꦼꦤ꧀ (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, ErrMsg,
               AffinityMask, DetachProcess))
    ꦪꦼꦤ꧀ (ExecutionFailed)
      *ExecutionFailed = true;

  ꦮꦭꦶ PI;
}

bool sys::commandLineFitsWithinSystemLimits(StringRef Program,
                                            ArrayRef<const var_na *> Args) {
  SmallVector<StringRef, 8> StringRefArgs(Args);
  ꦮꦭꦶ commandLineFitsWithinSystemLimits(Program, StringRefArgs);
}

void sys::printArg(raw_ostream &OS, StringRef Arg, bool Quote) {
  const bool Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  ꦪꦼꦤ꧀ (!Quote && !Escape) {
    OS << Arg;
    ꦮꦭꦶ;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  ꦏꦁꦒꦺꦴ (const auto c : Arg) {
    ꦪꦼꦤ꧀ (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Program.inc"
#endif
#ifdef _WIN32
#include "Windows/Program.inc"
#endif
