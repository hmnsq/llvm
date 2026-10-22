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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Program.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ਸਥਿਰ ਤਾਰਕਿਕ Execute(ProcessInfo &PI, StringRef ਪ੍ਰੋਗਰਾਮ_2,
                    ArrayRef<StringRef> ਦਲੀਲਾਂ,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਮ੍ਰਿਤੀ_ਹੱਦਬੰਦੀ, std::string *Err_ਸੁਨੇਹਾ,
                    ਦੋਆਧਾਰੀ_ਅੰਕ_ਸਦਿਸ਼ *Affinity_ਨਕਾਬ, ਤਾਰਕਿਕ DetachProcess);

ਪੂਰਨ sys::ExecuteAndWait(StringRef ਪ੍ਰੋਗਰਾਮ_2, ArrayRef<StringRef> ਦਲੀਲਾਂ,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        ਬਿਨਾਨਿਸ਼ਾਨੀ Seconds_ਮੰਜ਼ਿਲ_Wait, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਮ੍ਰਿਤੀ_ਹੱਦਬੰਦੀ,
                        std::string *Err_ਸੁਨੇਹਾ, ਤਾਰਕਿਕ *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        ਦੋਆਧਾਰੀ_ਅੰਕ_ਸਦਿਸ਼ *Affinity_ਨਕਾਬ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Redirects.ਖਾਲੀ_ਥਾਵਾਂ() || Redirects.ਮਾਪ_2() == 3);
  ProcessInfo PI;
  ਜੇ (Execute(PI, ਪ੍ਰੋਗਰਾਮ_2, ਦਲੀਲਾਂ, Env, Redirects, ਸਮ੍ਰਿਤੀ_ਹੱਦਬੰਦੀ, Err_ਸੁਨੇਹਾ,
              Affinity_ਨਕਾਬ, /*DetachProcess=*/ਝੂਠ)) {
    ਜੇ (ExecutionFailed)
      *ExecutionFailed = ਝੂਠ;
    ProcessInfo ਨਤੀਜਾ = Wait(
        PI, Seconds_ਮੰਜ਼ਿਲ_Wait == 0 ? std::nullopt : std::optional(Seconds_ਮੰਜ਼ਿਲ_Wait),
        Err_ਸੁਨੇਹਾ, ProcStat);
    ਵਾਪਸ ਨਤੀਜਾ.Return_ਕੋਡ;
  }

  ਜੇ (ExecutionFailed)
    *ExecutionFailed = ਸੱਚ;

  ਵਾਪਸ -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef ਪ੍ਰੋਗਰਾਮ_2, ArrayRef<StringRef> ਦਲੀਲਾਂ,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਮ੍ਰਿਤੀ_ਹੱਦਬੰਦੀ, std::string *Err_ਸੁਨੇਹਾ,
                               ਤਾਰਕਿਕ *ExecutionFailed, ਦੋਆਧਾਰੀ_ਅੰਕ_ਸਦਿਸ਼ *Affinity_ਨਕਾਬ,
                               ਤਾਰਕਿਕ DetachProcess) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Redirects.ਖਾਲੀ_ਥਾਵਾਂ() || Redirects.ਮਾਪ_2() == 3);
  ProcessInfo PI;
  ਜੇ (ExecutionFailed)
    *ExecutionFailed = ਝੂਠ;
  ਜੇ (!Execute(PI, ਪ੍ਰੋਗਰਾਮ_2, ਦਲੀਲਾਂ, Env, Redirects, ਸਮ੍ਰਿਤੀ_ਹੱਦਬੰਦੀ, Err_ਸੁਨੇਹਾ,
               Affinity_ਨਕਾਬ, DetachProcess))
    ਜੇ (ExecutionFailed)
      *ExecutionFailed = ਸੱਚ;

  ਵਾਪਸ PI;
}

ਤਾਰਕਿਕ sys::command_ਸਤਰ_ਸਮਾਉਂਦਾ_Within_System_Limits(StringRef ਪ੍ਰੋਗਰਾਮ_2,
                                            ArrayRef<ਅਚਲ ਅੱਖਰ *> ਦਲੀਲਾਂ) {
  Small_ਸਦਿਸ਼<StringRef, 8> String_Ref_ਦਲੀਲਾਂ(ਦਲੀਲਾਂ);
  ਵਾਪਸ command_ਸਤਰ_ਸਮਾਉਂਦਾ_Within_System_Limits(ਪ੍ਰੋਗਰਾਮ_2, String_Ref_ਦਲੀਲਾਂ);
}

ਖਾਲੀ sys::printArg(raw_ostream &OS, StringRef Arg, ਤਾਰਕਿਕ Quote) {
  ਅਚਲ ਤਾਰਕਿਕ Escape = Arg.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(" \"\\$") != StringRef::npos;

  ਜੇ (!Quote && !Escape) {
    OS << Arg;
    ਵਾਪਸ;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ c : Arg) {
    ਜੇ (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ON_UNIX
#ਅਨ੍ਤਰ੍ਭਾਵ "Unix/Program.inc"
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਅਨ੍ਤਰ੍ਭਾਵ "Windows/Program.inc"
#ਜੇ_ਅੰਤ
