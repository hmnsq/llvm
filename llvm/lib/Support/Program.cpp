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

#समावेश "llvm/Support/Program.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Config/llvm-config.h"
#समावेश "llvm/Support/raw_ostream.h"
using namespace llvm;
using namespace sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

अटल bool Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    अचिन्हित MemoryLimit, std::string *Err_संदेश,
                    BitVector *AffinityMask, bool DetachProcess);

पूर्णांक sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        अचिन्हित SecondsToWait, अचिन्हित MemoryLimit,
                        std::string *Err_संदेश, bool *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  निश्चित(Redirects.empty() || Redirects.आकार_2() == 3);
  ProcessInfo PI;
  अगर (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_संदेश,
              AffinityMask, /*DetachProcess=*/false)) {
    अगर (ExecutionFailed)
      *ExecutionFailed = false;
    ProcessInfo परिणाम = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        Err_संदेश, ProcStat);
    वापस परिणाम.ReturnCode;
  }

  अगर (ExecutionFailed)
    *ExecutionFailed = true;

  वापस -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               अचिन्हित MemoryLimit, std::string *Err_संदेश,
                               bool *ExecutionFailed, BitVector *AffinityMask,
                               bool DetachProcess) {
  निश्चित(Redirects.empty() || Redirects.आकार_2() == 3);
  ProcessInfo PI;
  अगर (ExecutionFailed)
    *ExecutionFailed = false;
  अगर (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_संदेश,
               AffinityMask, DetachProcess))
    अगर (ExecutionFailed)
      *ExecutionFailed = true;

  वापस PI;
}

bool sys::command_पंक्ति_Fits_Within_System_Limits(StringRef Program,
                                            ArrayRef<स्थिर अक्षर *> Args) {
  SmallVector<StringRef, 8> StringRefArgs(Args);
  वापस command_पंक्ति_Fits_Within_System_Limits(Program, StringRefArgs);
}

व्योम sys::printArg(raw_ostream &OS, StringRef Arg, bool Quote) {
  स्थिर bool Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  अगर (!Quote && !Escape) {
    OS << Arg;
    वापस;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  क्रम (स्थिर स्वतः c : Arg) {
    अगर (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#अगर_घोषित LLVM_ON_UNIX
#समावेश "Unix/Program.inc"
#पूर्ण_अगर
#अगर_घोषित _WIN32
#समावेश "Windows/Program.inc"
#पूर्ण_अगर
