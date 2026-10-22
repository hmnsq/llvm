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
استعمال نطاق llvm;
استعمال نطاق sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ساكن منطقي Execute(ProcessInfo &PI, StringRef پروگرام_2,
                    ArrayRef<StringRef> دليلون,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    بنانشان يادگيري_حد_بندي, std::string *Err_نياپو,
                    ٻٽو_انگ_ويڪٽر *Affinity_نقاب, منطقي DetachProcess);

صحيح sys::ExecuteAndWait(StringRef پروگرام_2, ArrayRef<StringRef> دليلون,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        بنانشان Seconds_منزل_Wait, بنانشان يادگيري_حد_بندي,
                        std::string *Err_نياپو, منطقي *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        ٻٽو_انگ_ويڪٽر *Affinity_نقاب) {
  أكد(Redirects.خالي_جايون() || Redirects.سائيز_2() == 3);
  ProcessInfo PI;
  إذا (Execute(PI, پروگرام_2, دليلون, Env, Redirects, يادگيري_حد_بندي, Err_نياپو,
              Affinity_نقاب, /*DetachProcess=*/خطأ)) {
    إذا (ExecutionFailed)
      *ExecutionFailed = خطأ;
    ProcessInfo نتيجو = Wait(
        PI, Seconds_منزل_Wait == 0 ? std::nullopt : std::optional(Seconds_منزل_Wait),
        Err_نياپو, ProcStat);
    أرجع نتيجو.Return_ڪوڊ;
  }

  إذا (ExecutionFailed)
    *ExecutionFailed = سچ;

  أرجع -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef پروگرام_2, ArrayRef<StringRef> دليلون,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               بنانشان يادگيري_حد_بندي, std::string *Err_نياپو,
                               منطقي *ExecutionFailed, ٻٽو_انگ_ويڪٽر *Affinity_نقاب,
                               منطقي DetachProcess) {
  أكد(Redirects.خالي_جايون() || Redirects.سائيز_2() == 3);
  ProcessInfo PI;
  إذا (ExecutionFailed)
    *ExecutionFailed = خطأ;
  إذا (!Execute(PI, پروگرام_2, دليلون, Env, Redirects, يادگيري_حد_بندي, Err_نياپو,
               Affinity_نقاب, DetachProcess))
    إذا (ExecutionFailed)
      *ExecutionFailed = سچ;

  أرجع PI;
}

منطقي sys::command_سٽ_سمائي_Within_System_Limits(StringRef پروگرام_2,
                                            ArrayRef<ثابت حرف *> دليلون) {
  Small_ويڪٽر<StringRef, 8> String_Ref_دليلون(دليلون);
  أرجع command_سٽ_سمائي_Within_System_Limits(پروگرام_2, String_Ref_دليلون);
}

فراغ sys::printArg(raw_ostream &OS, StringRef Arg, منطقي Quote) {
  ثابت منطقي Escape = Arg.ڳوليو_پهريون_جو(" \"\\$") != StringRef::npos;

  إذا (!Quote && !Escape) {
    OS << Arg;
    أرجع;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  لكل (ثابت خودڪار c : Arg) {
    إذا (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#جيڪڏهن_وصف_ٿيل LLVM_ON_UNIX
#ضمن "Unix/Program.inc"
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل _WIN32
#ضمن "Windows/Program.inc"
#جيڪڏهن_پورو
