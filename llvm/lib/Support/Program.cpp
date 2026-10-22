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
کارول نطاق llvm;
کارول نطاق sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ساكن منطقي Execute(ProcessInfo &PI, StringRef پروګرام_2,
                    ArrayRef<StringRef> دلیلونه,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    بینښه حافظه_حد, std::string *Err_پیغام,
                    بټ_ویکټور *Affinity_ماسک, منطقي DetachProcess);

صحيح sys::ExecuteAndWait(StringRef پروګرام_2, ArrayRef<StringRef> دلیلونه,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        بینښه Seconds_منزل_Wait, بینښه حافظه_حد,
                        std::string *Err_پیغام, منطقي *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        بټ_ویکټور *Affinity_ماسک) {
  أكد(Redirects.تشه() || Redirects.اندازه_2() == 3);
  ProcessInfo PI;
  إذا (Execute(PI, پروګرام_2, دلیلونه, Env, Redirects, حافظه_حد, Err_پیغام,
              Affinity_ماسک, /*DetachProcess=*/خطأ)) {
    إذا (ExecutionFailed)
      *ExecutionFailed = خطأ;
    ProcessInfo پایله = Wait(
        PI, Seconds_منزل_Wait == 0 ? std::nullopt : std::optional(Seconds_منزل_Wait),
        Err_پیغام, ProcStat);
    أرجع پایله.Return_کوډ;
  }

  إذا (ExecutionFailed)
    *ExecutionFailed = ریښتیا;

  أرجع -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef پروګرام_2, ArrayRef<StringRef> دلیلونه,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               بینښه حافظه_حد, std::string *Err_پیغام,
                               منطقي *ExecutionFailed, بټ_ویکټور *Affinity_ماسک,
                               منطقي DetachProcess) {
  أكد(Redirects.تشه() || Redirects.اندازه_2() == 3);
  ProcessInfo PI;
  إذا (ExecutionFailed)
    *ExecutionFailed = خطأ;
  إذا (!Execute(PI, پروګرام_2, دلیلونه, Env, Redirects, حافظه_حد, Err_پیغام,
               Affinity_ماسک, DetachProcess))
    إذا (ExecutionFailed)
      *ExecutionFailed = ریښتیا;

  أرجع PI;
}

منطقي sys::command_کرښه_ځاییږي_Within_System_Limits(StringRef پروګرام_2,
                                            ArrayRef<ثابت حرف *> دلیلونه) {
  Small_ویکټور<StringRef, 8> String_Ref_دلیلونه(دلیلونه);
  أرجع command_کرښه_ځاییږي_Within_System_Limits(پروګرام_2, String_Ref_دلیلونه);
}

فراغ sys::printArg(raw_ostream &OS, StringRef Arg, منطقي Quote) {
  ثابت منطقي Escape = Arg.پیدا_کړه_لومړی_د(" \"\\$") != StringRef::npos;

  إذا (!Quote && !Escape) {
    OS << Arg;
    أرجع;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  لكل (ثابت خپلکاری c : Arg) {
    إذا (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#که_تعریف_شوی LLVM_ON_UNIX
#ضمن "Unix/Program.inc"
#د_که_پای
#که_تعریف_شوی _WIN32
#ضمن "Windows/Program.inc"
#د_که_پای
