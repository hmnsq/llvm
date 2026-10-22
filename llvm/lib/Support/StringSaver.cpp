//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/StringSaver.h"

#అన్తర్భావ "llvm/ADT/SmallString.h"

ఉపయోగించి నామాకాశ llvm;

StringRef StringSaver::save(StringRef S) {
  అక్షరం *P = Alloc.Allocate<అక్షరం>(S.కొలత_2() + 1);
  ఒకవేళ (!S.ఖాళీలు())
    జ్ఞాపకం_నకలు(P, S.దత్తాంశం(), S.కొలత_2());
  P[S.కొలత_2()] = '\0';
  తిరిగి StringRef(P, S.కొలత_2());
}

StringRef StringSaver::save(స్థిరరాశి Twine &S) {
  SmallString<128> Storage;
  తిరిగి save(S.గమ్యం_String_Ref(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  స్వయంచాలక R = Unique.చొప్పించు(S);
  ఒకవేళ (R.రెండవ_2)                 // cache miss, need to actually save the string
    *R.మొదటి_2 = అక్షరశ్రేణులు_2.save(S); // safe replacement with equal value
  తిరిగి *R.మొదటి_2;
}

StringRef UniqueStringSaver::save(స్థిరరాశి Twine &S) {
  SmallString<128> Storage;
  తిరిగి save(S.గమ్యం_String_Ref(Storage));
}
