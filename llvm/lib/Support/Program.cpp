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

#ಅನ್ತರ್ಭಾವ "llvm/Support/Program.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ಸ್ಥಿರ ತಾರ್ಕಿಕ Execute(ProcessInfo &PI, StringRef ಕಾರ್ಯಕ್ರಮ_2,
                    ArrayRef<StringRef> ವಾದಗಳು,
                    std::optional<ArrayRef<StringRef>> Env,
                    ArrayRef<std::optional<StringRef>> Redirects,
                    ಚಿಹ್ನೆರಹಿತ ಸ್ಮರಣೆ_ಎಲ್ಲೆ, std::string *Err_ಸಂದೇಶ,
                    ಬಿಟ್_ಸದಿಶ *Affinity_ಮುಖವಾಡ, ತಾರ್ಕಿಕ DetachProcess);

ಪೂರ್ಣಾಂಕ sys::ExecuteAndWait(StringRef ಕಾರ್ಯಕ್ರಮ_2, ArrayRef<StringRef> ವಾದಗಳು,
                        std::optional<ArrayRef<StringRef>> Env,
                        ArrayRef<std::optional<StringRef>> Redirects,
                        ಚಿಹ್ನೆರಹಿತ Seconds_ಗಮ್ಯ_Wait, ಚಿಹ್ನೆರಹಿತ ಸ್ಮರಣೆ_ಎಲ್ಲೆ,
                        std::string *Err_ಸಂದೇಶ, ತಾರ್ಕಿಕ *ExecutionFailed,
                        std::optional<ProcessStatistics> *ProcStat,
                        ಬಿಟ್_ಸದಿಶ *Affinity_ಮುಖವಾಡ) {
  ಪ್ರತಿಜ್ಞಾ(Redirects.ಖಾಲಿಗಳು() || Redirects.ಗಾತ್ರ_3() == 3);
  ProcessInfo PI;
  ಆದರೆ (Execute(PI, ಕಾರ್ಯಕ್ರಮ_2, ವಾದಗಳು, Env, Redirects, ಸ್ಮರಣೆ_ಎಲ್ಲೆ, Err_ಸಂದೇಶ,
              Affinity_ಮುಖವಾಡ, /*DetachProcess=*/ಸುಳ್ಳು)) {
    ಆದರೆ (ExecutionFailed)
      *ExecutionFailed = ಸುಳ್ಳು;
    ProcessInfo ಫಲಿತಾಂಶ = Wait(
        PI, Seconds_ಗಮ್ಯ_Wait == 0 ? std::nullopt : std::optional(Seconds_ಗಮ್ಯ_Wait),
        Err_ಸಂದೇಶ, ProcStat);
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ.Return_ಸಂಕೇತ;
  }

  ಆದರೆ (ExecutionFailed)
    *ExecutionFailed = ನಿಜ;

  ಹಿಂತಿರುಗಿ -1;
}

ProcessInfo sys::ExecuteNoWait(StringRef ಕಾರ್ಯಕ್ರಮ_2, ArrayRef<StringRef> ವಾದಗಳು,
                               std::optional<ArrayRef<StringRef>> Env,
                               ArrayRef<std::optional<StringRef>> Redirects,
                               ಚಿಹ್ನೆರಹಿತ ಸ್ಮರಣೆ_ಎಲ್ಲೆ, std::string *Err_ಸಂದೇಶ,
                               ತಾರ್ಕಿಕ *ExecutionFailed, ಬಿಟ್_ಸದಿಶ *Affinity_ಮುಖವಾಡ,
                               ತಾರ್ಕಿಕ DetachProcess) {
  ಪ್ರತಿಜ್ಞಾ(Redirects.ಖಾಲಿಗಳು() || Redirects.ಗಾತ್ರ_3() == 3);
  ProcessInfo PI;
  ಆದರೆ (ExecutionFailed)
    *ExecutionFailed = ಸುಳ್ಳು;
  ಆದರೆ (!Execute(PI, ಕಾರ್ಯಕ್ರಮ_2, ವಾದಗಳು, Env, Redirects, ಸ್ಮರಣೆ_ಎಲ್ಲೆ, Err_ಸಂದೇಶ,
               Affinity_ಮುಖವಾಡ, DetachProcess))
    ಆದರೆ (ExecutionFailed)
      *ExecutionFailed = ನಿಜ;

  ಹಿಂತಿರುಗಿ PI;
}

ತಾರ್ಕಿಕ sys::command_ಸಾಲು_ಹೊಂದುತ್ತದೆ_Within_System_Limits(StringRef ಕಾರ್ಯಕ್ರಮ_2,
                                            ArrayRef<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> ವಾದಗಳು) {
  Small_ಸದಿಶ<StringRef, 8> String_Ref_ವಾದಗಳು(ವಾದಗಳು);
  ಹಿಂತಿರುಗಿ command_ಸಾಲು_ಹೊಂದುತ್ತದೆ_Within_System_Limits(ಕಾರ್ಯಕ್ರಮ_2, String_Ref_ವಾದಗಳು);
}

ಶೂನ್ಯ sys::printArg(raw_ostream &OS, StringRef Arg, ತಾರ್ಕಿಕ Quote) {
  ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Escape = Arg.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(" \"\\$") != StringRef::npos;

  ಆದರೆ (!Quote && !Escape) {
    OS << Arg;
    ಹಿಂತಿರುಗಿ;
  }

  // Quote and escape. This isn't really complete, but good enough.
  OS << '"';
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ c : Arg) {
    ಆದರೆ (c == '"' || c == '\\' || c == '$')
      OS << '\\';
    OS << c;
  }
  OS << '"';
}

// Include the platform-specific parts of this class.
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_ON_UNIX
#ಅನ್ತರ್ಭಾವ "Unix/Program.inc"
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#ಅನ್ತರ್ಭಾವ "Windows/Program.inc"
#ಆದರೆ_ಅಂತ್ಯ
