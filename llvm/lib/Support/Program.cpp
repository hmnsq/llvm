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

#अन्तर्भाव "llvm/Support/Program.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
using नामाकाश llvm;
using नामाकाश sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

स्थिर तार्किक Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    अचिन्हित MemoryLimit, std::string *Err_संदेश,
                    BitVector *AffinityMask, तार्किक DetachProcess);

पूर्णाङ्क sys::ExecuteAndWait(StringRef Program, ArrayRef<StringRef> Args,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        अचिन्हित SecondsToWait, अचिन्हित MemoryLimit,
                        std::string *Err_संदेश, तार्किक *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        BitVector *AffinityMask) {
  प्रतिज्ञा(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  यदि (Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_संदेश,
              AffinityMask, /*DetachProcess=*/असत्य)) {
    यदि (ExecutionFailed)
      *ExecutionFailed = असत्य;
    ProcessInfo Result = Wait(
        PI, SecondsToWait == 0 ? std::nullopt : std::optional(SecondsToWait),
        Err_संदेश, ProcStat);
    प्रत्यागम Result.ReturnCode;
  }

  यदि (ExecutionFailed)
    *ExecutionFailed = सत्य;

  प्रत्यागम -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef Program, ArrayRef<StringRef> Args,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               अचिन्हित MemoryLimit, std::string *Err_संदेश,
                               तार्किक *ExecutionFailed, BitVector *AffinityMask,
                               तार्किक DetachProcess) {
  प्रतिज्ञा(Redirects.empty() || Redirects.size() == 3);
  ProcessInfo PI;
  यदि (ExecutionFailed)
    *ExecutionFailed = असत्य;
  यदि (!Execute(PI, Program, Args, Env, Redirects, MemoryLimit, Err_संदेश,
               AffinityMask, DetachProcess))
    यदि (ExecutionFailed)
      *ExecutionFailed = सत्य;

  प्रत्यागम PI;
}

तार्किक sys::command_पंक्ति_Fits_Within_System_Limits(StringRef Program,
                                            ArrayRef<ध्रुव वर्ण *> Args) {
  SmallVector<StringRef, 8> StringRefArgs(Args);
  प्रत्यागम command_पंक्ति_Fits_Within_System_Limits(Program, StringRefArgs);
}

शून्य sys::printArg(raw_ostream &OS, StringRef Arg, तार्किक Quote) {
  ध्रुव तार्किक Escape = Arg.find_first_of(" \"\\$") != StringRef::npos;

  यदि (!Quote && !Escape) {
    OS << Arg;
    प्रत्यागम;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  प्रति (ध्रुव स्वतः c : Arg) {
    यदि (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#अन्तर्भाव "Unix/Program.inc"
#endif
#ifdef _WIN32
#अन्तर्भाव "Windows/Program.inc"
#endif
