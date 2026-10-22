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
ورتوں نطاق llvm;
ورتوں نطاق sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ساكن منطقي Execute(ProcessInfo &PI, StringRef پروگرام_2,
                    ArrayRef<StringRef> دلیلاں,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    بنانشانی حافظہ_حد_بندی, std::string *Err_پیغام,
                    ثنائی_ہندسہ_سمتیہ *Affinity_نقاب, منطقي DetachProcess);

صحيح sys::ExecuteAndWait(StringRef پروگرام_2, ArrayRef<StringRef> دلیلاں,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        بنانشانی Seconds_منزل_Wait, بنانشانی حافظہ_حد_بندی,
                        std::string *Err_پیغام, منطقي *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        ثنائی_ہندسہ_سمتیہ *Affinity_نقاب) {
  أكد(Redirects.خالی_تھاواں() || Redirects.ناپ_2() == 3);
  ProcessInfo PI;
  إذا (Execute(PI, پروگرام_2, دلیلاں, Env, Redirects, حافظہ_حد_بندی, Err_پیغام,
              Affinity_نقاب, /*DetachProcess=*/خطأ)) {
    إذا (ExecutionFailed)
      *ExecutionFailed = خطأ;
    ProcessInfo نتیجہ = Wait(
        PI, Seconds_منزل_Wait == 0 ? std::nullopt : std::optional(Seconds_منزل_Wait),
        Err_پیغام, ProcStat);
    أرجع نتیجہ.Return_کوڈ;
  }

  إذا (ExecutionFailed)
    *ExecutionFailed = سچ;

  أرجع -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef پروگرام_2, ArrayRef<StringRef> دلیلاں,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               بنانشانی حافظہ_حد_بندی, std::string *Err_پیغام,
                               منطقي *ExecutionFailed, ثنائی_ہندسہ_سمتیہ *Affinity_نقاب,
                               منطقي DetachProcess) {
  أكد(Redirects.خالی_تھاواں() || Redirects.ناپ_2() == 3);
  ProcessInfo PI;
  إذا (ExecutionFailed)
    *ExecutionFailed = خطأ;
  إذا (!Execute(PI, پروگرام_2, دلیلاں, Env, Redirects, حافظہ_حد_بندی, Err_پیغام,
               Affinity_نقاب, DetachProcess))
    إذا (ExecutionFailed)
      *ExecutionFailed = سچ;

  أرجع PI;
}

منطقي sys::command_سطر_سماندا_Within_System_Limits(StringRef پروگرام_2,
                                            ArrayRef<ثابت حرف *> دلیلاں) {
  Small_سمتیہ<StringRef, 8> String_Ref_دلیلاں(دلیلاں);
  أرجع command_سطر_سماندا_Within_System_Limits(پروگرام_2, String_Ref_دلیلاں);
}

فراغ sys::printArg(raw_ostream &OS, StringRef Arg, منطقي Quote) {
  ثابت منطقي Escape = Arg.لبھو_پہلا_دا(" \"\\$") != StringRef::npos;

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
#جے_تعریف_شدہ LLVM_ON_UNIX
#ضمن "Unix/Program.inc"
#جے_انت
#جے_تعریف_شدہ _WIN32
#ضمن "Windows/Program.inc"
#جے_انت
