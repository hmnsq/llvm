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

#అన్తర్భావ "llvm/Support/Program.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Config/llvm-config.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

స్థిర తార్కిక Execute(ProcessInfo &PI, StringRef కార్యక్రమం_2,
                    ArrayRef<StringRef> వాదనలు,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    గుర్తులేని జ్ఞాపకం_హద్దు, std::string *Err_సందేశం,
                    బిట్టు_సదిశ *Affinity_ముసుగు, తార్కిక DetachProcess);

పూర్ణాంకం sys::ExecuteAndWait(StringRef కార్యక్రమం_2, ArrayRef<StringRef> వాదనలు,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        గుర్తులేని Seconds_గమ్యం_Wait, గుర్తులేని జ్ఞాపకం_హద్దు,
                        std::string *Err_సందేశం, తార్కిక *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        బిట్టు_సదిశ *Affinity_ముసుగు) {
  ప్రతిజ్ఞా(Redirects.ఖాళీలు() || Redirects.కొలత_2() == 3);
  ProcessInfo PI;
  ఒకవేళ (Execute(PI, కార్యక్రమం_2, వాదనలు, Env, Redirects, జ్ఞాపకం_హద్దు, Err_సందేశం,
              Affinity_ముసుగు, /*DetachProcess=*/అబద్ధం)) {
    ఒకవేళ (ExecutionFailed)
      *ExecutionFailed = అబద్ధం;
    ProcessInfo ఫలితం = Wait(
        PI, Seconds_గమ్యం_Wait == 0 ? std::nullopt : std::optional(Seconds_గమ్యం_Wait),
        Err_సందేశం, ProcStat);
    తిరిగి ఫలితం.Return_సంకేతం;
  }

  ఒకవేళ (ExecutionFailed)
    *ExecutionFailed = నిజం;

  తిరిగి -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef కార్యక్రమం_2, ArrayRef<StringRef> వాదనలు,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               గుర్తులేని జ్ఞాపకం_హద్దు, std::string *Err_సందేశం,
                               తార్కిక *ExecutionFailed, బిట్టు_సదిశ *Affinity_ముసుగు,
                               తార్కిక DetachProcess) {
  ప్రతిజ్ఞా(Redirects.ఖాళీలు() || Redirects.కొలత_2() == 3);
  ProcessInfo PI;
  ఒకవేళ (ExecutionFailed)
    *ExecutionFailed = అబద్ధం;
  ఒకవేళ (!Execute(PI, కార్యక్రమం_2, వాదనలు, Env, Redirects, జ్ఞాపకం_హద్దు, Err_సందేశం,
               Affinity_ముసుగు, DetachProcess))
    ఒకవేళ (ExecutionFailed)
      *ExecutionFailed = నిజం;

  తిరిగి PI;
}

తార్కిక sys::command_పంక్తి_సరిపోతుంది_Within_System_Limits(StringRef కార్యక్రమం_2,
                                            ArrayRef<స్థిరరాశి అక్షరం *> వాదనలు) {
  Small_సదిశ<StringRef, 8> String_Ref_వాదనలు(వాదనలు);
  తిరిగి command_పంక్తి_సరిపోతుంది_Within_System_Limits(కార్యక్రమం_2, String_Ref_వాదనలు);
}

శూన్య sys::printArg(raw_ostream &OS, StringRef Arg, తార్కిక Quote) {
  స్థిరరాశి తార్కిక Escape = Arg.కనుగొను_మొదటి_యొక్క(" \"\\$") != StringRef::npos;

  ఒకవేళ (!Quote && !Escape) {
    OS << Arg;
    తిరిగి;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  కొరకు (స్థిరరాశి స్వయంచాలక c : Arg) {
    ఒకవేళ (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#నిర్వచితమైతే LLVM_ON_UNIX
#అన్తర్భావ "Unix/Program.inc"
#అయితే_ముగింపు
#నిర్వచితమైతే _WIN32
#అన్తర్భావ "Windows/Program.inc"
#అయితే_ముగింపు
