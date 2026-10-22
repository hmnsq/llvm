//===-- StringExtras.cpp - Implement the StringExtras header --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the StringExtras.h header
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <cctype>

ఉపయోగించి నామాకాశ llvm;

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<StringRef, StringRef> llvm::getToken(StringRef ఆధారం,
                                               StringRef Delimiters) {
  // Figure out where the token starts.
  StringRef::కొలత_type ప్రారంభం = ఆధారం.కనుగొను_మొదటి_not_యొక్క(Delimiters);

  // Find the next occurrence of the delimiter.
  StringRef::కొలత_type చివర_2 = ఆధారం.కనుగొను_మొదటి_యొక్క(Delimiters, ప్రారంభం);

  తిరిగి {ఆధారం.ముక్క(ప్రారంభం, చివర_2), ఆధారం.substr(చివర_2)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
శూన్య llvm::విభజించు_String(StringRef ఆధారం,
                       Small_సదిశ_Impl<StringRef> &బహిర్గతం_Fragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(ఆధారం, Delimiters);
  అయితే (!S.మొదటి_2.ఖాళీలు()) {
    బహిర్గతం_Fragments.నెట్టు_వెనుక(S.మొదటి_2);
    S = getToken(S.రెండవ_2, Delimiters);
  }
}

శూన్య llvm::printEscapedString(StringRef Name, raw_ostream &బహిర్గతం) {
  కొరకు (గుర్తులేని అక్షరం C : Name) {
    ఒకవేళ (C == '\\')
      బహిర్గతం << '\\' << C;
    లేకపోతే ఒకవేళ (isPrint(C) && C != '"')
      బహిర్గతం << C;
    లేకపోతే
      బహిర్గతం << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

శూన్య llvm::printHTMLEscaped(StringRef String, raw_ostream &బహిర్గతం) {
  కొరకు (అక్షరం C : String) {
    ఒకవేళ (C == '&')
      బహిర్గతం << "&amp;";
    లేకపోతే ఒకవేళ (C == '<')
      బహిర్గతం << "&lt;";
    లేకపోతే ఒకవేళ (C == '>')
      బహిర్గతం << "&gt;";
    లేకపోతే ఒకవేళ (C == '\"')
      బహిర్గతం << "&quot;";
    లేకపోతే ఒకవేళ (C == '\'')
      బహిర్గతం << "&apos;";
    లేకపోతే
      బహిర్గతం << C;
  }
}

శూన్య llvm::print_క్రింది_Case(StringRef String, raw_ostream &బహిర్గతం) {
  కొరకు (స్థిరరాశి అక్షరం C : String)
    బహిర్గతం << గమ్యం_క్రింది(C);
}

శూన్య llvm::printPercentEncoded(StringRef String, raw_ostream &బహిర్గతం) {
  కొరకు (గుర్తులేని అక్షరం C : String) {
    ఒకవేళ (isAlnum(C) || StringRef("-_.~").కలిగియుంది(C))
      బహిర్గతం << C;
    లేకపోతే
      బహిర్గతం << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convert_గమ్యం_Snake_From_Camel_Case(StringRef ఆదానం_2) {
  ఒకవేళ (ఆదానం_2.ఖాళీలు())
    తిరిగి "";

  std::string snakeCase;
  snakeCase.reserve(ఆదానం_2.కొలత_2());
  స్వయంచాలక check = [&ఆదానం_2](size_t j, function_ref<తార్కిక(పూర్ణాంకం)> predicate) {
    తిరిగి j < ఆదానం_2.కొలత_2() && predicate(ఆదానం_2[j]);
  };
  కొరకు (size_t i = 0; i < ఆదానం_2.కొలత_2(); ++i) {
    snakeCase.నెట్టు_వెనుక(tolower(ఆదానం_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    ఒకవేళ (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.నెట్టు_వెనుక('_');
    ఒకవేళ ((check(i, islower) || check(i, అంకెనా)) && check(i + 1, isupper))
      snakeCase.నెట్టు_వెనుక('_');
  }
  తిరిగి snakeCase;
}

std::string llvm::convert_గమ్యం_Camel_From_Snake_Case(StringRef ఆదానం_2,
                                              తార్కిక capitalize_మొదటి) {
  ఒకవేళ (ఆదానం_2.ఖాళీలు())
    తిరిగి "";

  std::string output;
  output.reserve(ఆదానం_2.కొలత_2());

  // Push the first character, capatilizing if necessary.
  ఒకవేళ (capitalize_మొదటి && std::islower(ఆదానం_2.front()))
    output.నెట్టు_వెనుక(llvm::గమ్యం_పై(ఆదానం_2.front()));
  లేకపోతే
    output.నెట్టు_వెనుక(ఆదానం_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  కొరకు (size_t pos = 1, e = ఆదానం_2.కొలత_2(); pos < e; ++pos) {
    ఒకవేళ (ఆదానం_2[pos] == '_' && pos != (e - 1) && std::islower(ఆదానం_2[pos + 1]))
      output.నెట్టు_వెనుక(llvm::గమ్యం_పై(ఆదానం_2[++pos]));
    లేకపోతే
      output.నెట్టు_వెనుక(ఆదానం_2[pos]);
  }
  తిరిగి output;
}
