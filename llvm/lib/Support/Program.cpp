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
उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

स्थिर तार्किक Execute(ProcessInfo &PI, StringRef क्रमादेशः_2,
                    ArrayRef<StringRef> तर्काः,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    अचिह्नित स्मृतिः_सीमा, std::string *Err_सन्देशः,
                    Bit_सदिशम् *Affinity_आवरणम्, तार्किक DetachProcess);

पूर्णाङ्क sys::ExecuteAndWait(StringRef क्रमादेशः_2, ArrayRef<StringRef> तर्काः,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        अचिह्नित Seconds_गन्तव्यम्_Wait, अचिह्नित स्मृतिः_सीमा,
                        std::string *Err_सन्देशः, तार्किक *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        Bit_सदिशम् *Affinity_आवरणम्) {
  प्रतिज्ञा(Redirects.रिक्तानि() || Redirects.आकार_2() == 3);
  ProcessInfo PI;
  यदि (Execute(PI, क्रमादेशः_2, तर्काः, Env, Redirects, स्मृतिः_सीमा, Err_सन्देशः,
              Affinity_आवरणम्, /*DetachProcess=*/असत्य)) {
    यदि (ExecutionFailed)
      *ExecutionFailed = असत्य;
    ProcessInfo परिणाम = Wait(
        PI, Seconds_गन्तव्यम्_Wait == 0 ? std::nullopt : std::optional(Seconds_गन्तव्यम्_Wait),
        Err_सन्देशः, ProcStat);
    प्रत्यागम परिणाम.Return_संहिता;
  }

  यदि (ExecutionFailed)
    *ExecutionFailed = सत्य;

  प्रत्यागम -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef क्रमादेशः_2, ArrayRef<StringRef> तर्काः,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               अचिह्नित स्मृतिः_सीमा, std::string *Err_सन्देशः,
                               तार्किक *ExecutionFailed, Bit_सदिशम् *Affinity_आवरणम्,
                               तार्किक DetachProcess) {
  प्रतिज्ञा(Redirects.रिक्तानि() || Redirects.आकार_2() == 3);
  ProcessInfo PI;
  यदि (ExecutionFailed)
    *ExecutionFailed = असत्य;
  यदि (!Execute(PI, क्रमादेशः_2, तर्काः, Env, Redirects, स्मृतिः_सीमा, Err_सन्देशः,
               Affinity_आवरणम्, DetachProcess))
    यदि (ExecutionFailed)
      *ExecutionFailed = सत्य;

  प्रत्यागम PI;
}

तार्किक sys::command_पङ्क्तिः_माति_Within_System_Limits(StringRef क्रमादेशः_2,
                                            ArrayRef<ध्रुव वर्ण *> तर्काः) {
  Small_सदिशम्<StringRef, 8> String_Ref_तर्काः(तर्काः);
  प्रत्यागम command_पङ्क्तिः_माति_Within_System_Limits(क्रमादेशः_2, String_Ref_तर्काः);
}

शून्य sys::printArg(raw_ostream &OS, StringRef Arg, तार्किक Quote) {
  ध्रुव तार्किक Escape = Arg.find_first_सम्बन्धि(" \"\\$") != StringRef::npos;

  यदि (!Quote && !Escape) {
    OS << Arg;
    प्रत्यागम;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  प्रति (ध्रुव स्वचालित c : Arg) {
    यदि (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#यदि_परिभाषितम् LLVM_ON_UNIX
#अन्तर्भाव "Unix/Program.inc"
#यदि_अन्तः
#यदि_परिभाषितम् _WIN32
#अन्तर्भाव "Windows/Program.inc"
#यदि_अन्तः
