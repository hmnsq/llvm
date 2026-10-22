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
استفاده نطاق llvm;
استفاده نطاق sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ساكن منطقي Execute(ProcessInfo &PI, StringRef برنامه_2,
                    ArrayRef<StringRef> آرگومانها,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    بیعلامت حافظه_حد, std::string *Err_پیام,
                    بیت_وکتور *Affinity_ماسک, منطقي DetachProcess);

صحيح sys::ExecuteAndWait(StringRef برنامه_2, ArrayRef<StringRef> آرگومانها,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        بیعلامت Seconds_مقصد_Wait, بیعلامت حافظه_حد,
                        std::string *Err_پیام, منطقي *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        بیت_وکتور *Affinity_ماسک) {
  أكد(Redirects.خالی() || Redirects.اندازه_2() == 3);
  ProcessInfo PI;
  إذا (Execute(PI, برنامه_2, آرگومانها, Env, Redirects, حافظه_حد, Err_پیام,
              Affinity_ماسک, /*DetachProcess=*/خطأ)) {
    إذا (ExecutionFailed)
      *ExecutionFailed = خطأ;
    ProcessInfo نتیجه = Wait(
        PI, Seconds_مقصد_Wait == 0 ? std::nullopt : std::optional(Seconds_مقصد_Wait),
        Err_پیام, ProcStat);
    أرجع نتیجه.Return_کد;
  }

  إذا (ExecutionFailed)
    *ExecutionFailed = درست;

  أرجع -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef برنامه_2, ArrayRef<StringRef> آرگومانها,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               بیعلامت حافظه_حد, std::string *Err_پیام,
                               منطقي *ExecutionFailed, بیت_وکتور *Affinity_ماسک,
                               منطقي DetachProcess) {
  أكد(Redirects.خالی() || Redirects.اندازه_2() == 3);
  ProcessInfo PI;
  إذا (ExecutionFailed)
    *ExecutionFailed = خطأ;
  إذا (!Execute(PI, برنامه_2, آرگومانها, Env, Redirects, حافظه_حد, Err_پیام,
               Affinity_ماسک, DetachProcess))
    إذا (ExecutionFailed)
      *ExecutionFailed = درست;

  أرجع PI;
}

منطقي sys::command_سطر_میگنجد_Within_System_Limits(StringRef برنامه_2,
                                            ArrayRef<ثابت حرف *> آرگومانها) {
  Small_وکتور<StringRef, 8> String_Ref_آرگومانها(آرگومانها);
  أرجع command_سطر_میگنجد_Within_System_Limits(برنامه_2, String_Ref_آرگومانها);
}

فراغ sys::printArg(raw_ostream &OS, StringRef Arg, منطقي Quote) {
  ثابت منطقي Escape = Arg.پیدا_کن_اولی_از(" \"\\$") != StringRef::npos;

  إذا (!Quote && !Escape) {
    OS << Arg;
    أرجع;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  لكل (ثابت خودکار c : Arg) {
    إذا (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#اگر_تعریف_شده LLVM_ON_UNIX
#ضمن "Unix/Program.inc"
#پایان_اگر
#اگر_تعریف_شده _WIN32
#ضمن "Windows/Program.inc"
#پایان_اگر
