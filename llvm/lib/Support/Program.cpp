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
                    ArrayRef<StringRef> मगा_कातविरान,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *Err_मेनसाहे,
                    BitVector *AffinityMask, bool DetachProcess);

int sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> मगा_कातविरान,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        unsigned SecondsToWait, unsigned MemoryLimit,
                        std::string *Err_मेनसाहे, bool *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  assert(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  if (Execute(PI, Program, मगा_कातविरान, Env, Redirects, MemoryLimit, Err_मेनसाहे,
              AffinityMask, /*DetachProcess=*/false)) {
    if (ExecutionFailed)
      *ExecutionFailed = false;
    ProcessInfo Result = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        Err_मेनसाहे, ProcStat);
    return Result.ReturnCode;
  }

  if (ExecutionFailed)
    *ExecutionFailed = true;

  return -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> मगा_कातविरान,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               unsigned MemoryLimit, std::string *Err_मेनसाहे,
                               bool *ExecutionFailed, BitVector *AffinityMask,
                               bool DetachProcess) {
  assert(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  if (ExecutionFailed)
    *ExecutionFailed = false;
  if (!Execute(PI, Program, मगा_कातविरान, Env, Redirects, MemoryLimit, Err_मेनसाहे,
               AffinityMask, DetachProcess))
    if (ExecutionFailed)
      *ExecutionFailed = true;

  return PI;
}

bool sys::command_लिन्या_Fits_Within_System_Limits(StringRef Program,
                                            ArrayRef<const char *> मगा_कातविरान) {
  SmallVector<StringRef, 8> String_Ref_मगा_कातविरान(मगा_कातविरान);
  return command_लिन्या_Fits_Within_System_Limits(Program, String_Ref_मगा_कातविरान);
}

void sys::इलिम्बाग_Arg_2(raw_ostream &OS, StringRef Arg, bool Quote) {
  const bool Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  if (!Quote && !Escape) {
    OS << Arg;
    return;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  for (const auto c : Arg) {
    if (c == '"' || c == '\\' || c == '$')
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
