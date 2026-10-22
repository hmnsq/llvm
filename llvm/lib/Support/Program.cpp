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

#شامل "llvm/Support/Program.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/raw_ostream.h"
استعمال نامخلا llvm;
استعمال نامخلا sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ساکن منطقی Execute(ProcessInfo &PI, StringRef پروگرام_2,
                    ArrayRef<StringRef> دلائل,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    غیرعلامتی حافظہ_حد_بندی, std::string *Err_پیغام,
                    ثنائی_ہندسہ_سمتیہ *Affinity_نقاب, منطقی DetachProcess);

صحیح sys::ExecuteAndWait(StringRef پروگرام_2, ArrayRef<StringRef> دلائل,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        غیرعلامتی Seconds_مقصد_Wait, غیرعلامتی حافظہ_حد_بندی,
                        std::string *Err_پیغام, منطقی *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        ثنائی_ہندسہ_سمتیہ *Affinity_نقاب) {
  أكد(Redirects.خالی_خانے() || Redirects.سائز_2() == 3);
  ProcessInfo PI;
  اگر (Execute(PI, پروگرام_2, دلائل, Env, Redirects, حافظہ_حد_بندی, Err_پیغام,
              Affinity_نقاب, /*DetachProcess=*/جھوٹ)) {
    اگر (ExecutionFailed)
      *ExecutionFailed = جھوٹ;
    ProcessInfo نتیجہ = Wait(
        PI, Seconds_مقصد_Wait == 0 ? std::nullopt : std::optional(Seconds_مقصد_Wait),
        Err_پیغام, ProcStat);
    واپس نتیجہ.Return_رمز;
  }

  اگر (ExecutionFailed)
    *ExecutionFailed = سچ;

  واپس -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef پروگرام_2, ArrayRef<StringRef> دلائل,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               غیرعلامتی حافظہ_حد_بندی, std::string *Err_پیغام,
                               منطقی *ExecutionFailed, ثنائی_ہندسہ_سمتیہ *Affinity_نقاب,
                               منطقی DetachProcess) {
  أكد(Redirects.خالی_خانے() || Redirects.سائز_2() == 3);
  ProcessInfo PI;
  اگر (ExecutionFailed)
    *ExecutionFailed = جھوٹ;
  اگر (!Execute(PI, پروگرام_2, دلائل, Env, Redirects, حافظہ_حد_بندی, Err_پیغام,
               Affinity_نقاب, DetachProcess))
    اگر (ExecutionFailed)
      *ExecutionFailed = سچ;

  واپس PI;
}

منطقی sys::command_سطر_سماتا_Within_System_Limits(StringRef پروگرام_2,
                                            ArrayRef<ثابت حرف *> دلائل) {
  Small_سمتیہ<StringRef, 8> String_Ref_دلائل(دلائل);
  واپس command_سطر_سماتا_Within_System_Limits(پروگرام_2, String_Ref_دلائل);
}

خالی sys::printArg(raw_ostream &OS, StringRef Arg, منطقی Quote) {
  ثابت منطقی Escape = Arg.ڈھونڈو_پہلا_کا(" \"\\$") != StringRef::npos;

  اگر (!Quote && !Escape) {
    OS << Arg;
    واپس;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  برائے (ثابت خودکار c : Arg) {
    اگر (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#اگر_تعریف_شدہ LLVM_ON_UNIX
#شامل "Unix/Program.inc"
#اگر_ختم
#اگر_تعریف_شدہ _WIN32
#شامل "Windows/Program.inc"
#اگر_ختم
