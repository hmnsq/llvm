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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Program.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
using ନାମାକାଶ llvm;
using ନାମାକାଶ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ସ୍ଥିର ତାର୍କିକ Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *Err_ସନ୍ଦେଶ,
                    BitVector *AffinityMask, ତାର୍କିକ DetachProcess);

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        unsigned SecondsToWait, unsigned MemoryLimit,
                        std::string *Err_ସନ୍ଦେଶ, ତାର୍କିକ *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  ପ୍ରତିଜ୍ଞା(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  ଯଦି (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_ସନ୍ଦେଶ,
              AffinityMask, /*DetachProcess=*/ମିଥ୍ୟା)) {
    ଯଦି (ExecutionFailed)
      *ExecutionFailed = ମିଥ୍ୟା;
    ProcessInfo Result = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        Err_ସନ୍ଦେଶ, ProcStat);
    ଫେରନ୍ତୁ Result.ReturnCode;
  }

  ଯଦି (ExecutionFailed)
    *ExecutionFailed = ସତ୍ୟ;

  ଫେରନ୍ତୁ -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               unsigned MemoryLimit, std::string *Err_ସନ୍ଦେଶ,
                               ତାର୍କିକ *ExecutionFailed, BitVector *AffinityMask,
                               ତାର୍କିକ DetachProcess) {
  ପ୍ରତିଜ୍ଞା(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  ଯଦି (ExecutionFailed)
    *ExecutionFailed = ମିଥ୍ୟା;
  ଯଦି (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_ସନ୍ଦେଶ,
               AffinityMask, DetachProcess))
    ଯଦି (ExecutionFailed)
      *ExecutionFailed = ସତ୍ୟ;

  ଫେରନ୍ତୁ PI;
}

ତାର୍କିକ sys::command_ଧାଡ଼ି_Fits_Within_System_Limits(StringRef Program,
                                            ArrayRef<ଅଚଳ ଅକ୍ଷର *> Args) {
  SmallVector<StringRef, 8> StringRefArgs(Args);
  ଫେରନ୍ତୁ command_ଧାଡ଼ି_Fits_Within_System_Limits(Program, StringRefArgs);
}

ଶୂନ୍ୟ sys::printArg(raw_ostream &OS, StringRef Arg, ତାର୍କିକ Quote) {
  ଅଚଳ ତାର୍କିକ Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  ଯଦି (!Quote && !Escape) {
    OS << Arg;
    ଫେରନ୍ତୁ;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  ପାଇଁ (ଅଚଳ auto c : Arg) {
    ଯଦି (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ଅନ୍ତର୍ଭାଵ "Unix/Program.inc"
#endif
#ifdef _WIN32
#ଅନ୍ତର୍ଭାଵ "Windows/Program.inc"
#endif
