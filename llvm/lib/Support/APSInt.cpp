//===-- llvm/ADT/APSInt.cpp - Arbitrary Precision Signed Int ---*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the APSInt class, which is a simple class that
// represents an arbitrary sized integer that knows its signedness.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/APSInt.h"
#అన్తర్భావ "llvm/ADT/FoldingSet.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ <cassert>

ఉపయోగించి నామాకాశ llvm;

APSInt::APSInt(StringRef Str) {
  ప్రతిజ్ఞా(!Str.ఖాళీలు() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  గుర్తులేని అంకె_బిట్లు_2 = ((Str.కొలత_2() * 64) / 19) + 2;
  APInt Tmp(అంకె_బిట్లు_2, Str, /*radix=*/10);
  ఒకవేళ (Str[0] == '-') {
    గుర్తులేని Min_బిట్లు = Tmp.get_Significant_బిట్లు();
    ఒకవేళ (Min_బిట్లు < అంకె_బిట్లు_2)
      Tmp = Tmp.trunc(std::max<గుర్తులేని>(1, Min_బిట్లు));
    *ఇది = APSInt(Tmp, /*isUnsigned=*/అబద్ధం);
    తిరిగి;
  }
  గుర్తులేని చురుకైన_బిట్లు = Tmp.get_చురుకైన_బిట్లు();
  ఒకవేళ (చురుకైన_బిట్లు < అంకె_బిట్లు_2)
    Tmp = Tmp.trunc(std::max<గుర్తులేని>(1, చురుకైన_బిట్లు));
  *ఇది = APSInt(Tmp, /*isUnsigned=*/నిజం);
}

శూన్య APSInt::Profile(Folding_సమితి_కణుపు_ID& ID) స్థిరరాశి {
  ID.కూడు_Integer((గుర్తులేని) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
