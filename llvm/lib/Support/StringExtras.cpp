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

#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <cctype>

using નામાકાશ llvm;

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<StringRef, StringRef> llvm::getToken(StringRef Source,
                                               StringRef Delimiters) {
  // Figure out where the token starts.
  StringRef::size_type Start = Source.find_first_not_of(Delimiters);

  // Find the next occurrence of the delimiter.
  StringRef::size_type End = Source.find_first_of(Delimiters, Start);

  પાછા {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
શૂન્ય llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  જ્યારે (!S.first.empty()) {
    OutFragments.ધકેલો_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

શૂન્ય llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  માટે (unsigned અક્ષર C : Name) {
    જો (C == '\\')
      Out << '\\' << C;
    નહીંતર જો (isPrint(C) && C != '"')
      Out << C;
    નહીંતર
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

શૂન્ય llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  માટે (અક્ષર C : String) {
    જો (C == '&')
      Out << "&amp;";
    નહીંતર જો (C == '<')
      Out << "&lt;";
    નહીંતર જો (C == '>')
      Out << "&gt;";
    નહીંતર જો (C == '\"')
      Out << "&quot;";
    નહીંતર જો (C == '\'')
      Out << "&apos;";
    નહીંતર
      Out << C;
  }
}

શૂન્ય llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  માટે (અચલ અક્ષર C : String)
    Out << toLower(C);
}

શૂન્ય llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  માટે (unsigned અક્ષર C : String) {
    જો (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    નહીંતર
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef આગત_2) {
  જો (આગત_2.empty())
    પાછા "";

  std::string snakeCase;
  snakeCase.reserve(આગત_2.size());
  auto check = [&આગત_2](size_t j, function_ref<તાર્કિક(પૂર્ણાંક)> predicate) {
    પાછા j < આગત_2.size() && predicate(આગત_2[j]);
  };
  માટે (size_t i = 0; i < આગત_2.size(); ++i) {
    snakeCase.ધકેલો_back(tolower(આગત_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    જો (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.ધકેલો_back('_');
    જો ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.ધકેલો_back('_');
  }
  પાછા snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef આગત_2,
                                              તાર્કિક capitalizeFirst) {
  જો (આગત_2.empty())
    પાછા "";

  std::string output;
  output.reserve(આગત_2.size());

  // Push the first character, capatilizing if necessary.
  જો (capitalizeFirst && std::islower(આગત_2.front()))
    output.ધકેલો_back(llvm::toUpper(આગત_2.front()));
  નહીંતર
    output.ધકેલો_back(આગત_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  માટે (size_t pos = 1, e = આગત_2.size(); pos < e; ++pos) {
    જો (આગત_2[pos] == '_' && pos != (e - 1) && std::islower(આગત_2[pos + 1]))
      output.ધકેલો_back(llvm::toUpper(આગત_2[++pos]));
    નહીંતર
      output.ધકેલો_back(આગત_2[pos]);
  }
  પાછા output;
}
