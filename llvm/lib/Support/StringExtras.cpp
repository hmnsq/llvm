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

#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/ADT/SmallVector.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <cctype>

using നാമാകാശ llvm;

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

  തിരികെ {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
ശൂന്യം llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  ആകുമ്പോൾ (!S.first.empty()) {
    OutFragments.തള്ളുക_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

ശൂന്യം llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  വേണ്ടി (unsigned അക്ഷരം C : Name) {
    എങ്കിൽ (C == '\\')
      Out << '\\' << C;
    അല്ലെങ്കിൽ എങ്കിൽ (isPrint(C) && C != '"')
      Out << C;
    അല്ലെങ്കിൽ
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

ശൂന്യം llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  വേണ്ടി (അക്ഷരം C : String) {
    എങ്കിൽ (C == '&')
      Out << "&amp;";
    അല്ലെങ്കിൽ എങ്കിൽ (C == '<')
      Out << "&lt;";
    അല്ലെങ്കിൽ എങ്കിൽ (C == '>')
      Out << "&gt;";
    അല്ലെങ്കിൽ എങ്കിൽ (C == '\"')
      Out << "&quot;";
    അല്ലെങ്കിൽ എങ്കിൽ (C == '\'')
      Out << "&apos;";
    അല്ലെങ്കിൽ
      Out << C;
  }
}

ശൂന്യം llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  വേണ്ടി (സ്ഥിരം അക്ഷരം C : String)
    Out << toLower(C);
}

ശൂന്യം llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  വേണ്ടി (unsigned അക്ഷരം C : String) {
    എങ്കിൽ (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    അല്ലെങ്കിൽ
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef നിവേശം_2) {
  എങ്കിൽ (നിവേശം_2.empty())
    തിരികെ "";

  std::string snakeCase;
  snakeCase.reserve(നിവേശം_2.size());
  auto check = [&നിവേശം_2](size_t j, function_ref<തർക്കം(പൂർണ്ണസംഖ്യ)> predicate) {
    തിരികെ j < നിവേശം_2.size() && predicate(നിവേശം_2[j]);
  };
  വേണ്ടി (size_t i = 0; i < നിവേശം_2.size(); ++i) {
    snakeCase.തള്ളുക_back(tolower(നിവേശം_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    എങ്കിൽ (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.തള്ളുക_back('_');
    എങ്കിൽ ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.തള്ളുക_back('_');
  }
  തിരികെ snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef നിവേശം_2,
                                              തർക്കം capitalizeFirst) {
  എങ്കിൽ (നിവേശം_2.empty())
    തിരികെ "";

  std::string output;
  output.reserve(നിവേശം_2.size());

  // Push the first character, capatilizing if necessary.
  എങ്കിൽ (capitalizeFirst && std::islower(നിവേശം_2.front()))
    output.തള്ളുക_back(llvm::toUpper(നിവേശം_2.front()));
  അല്ലെങ്കിൽ
    output.തള്ളുക_back(നിവേശം_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  വേണ്ടി (size_t pos = 1, e = നിവേശം_2.size(); pos < e; ++pos) {
    എങ്കിൽ (നിവേശം_2[pos] == '_' && pos != (e - 1) && std::islower(നിവേശം_2[pos + 1]))
      output.തള്ളുക_back(llvm::toUpper(നിവേശം_2[++pos]));
    അല്ലെങ്കിൽ
      output.തള്ളുക_back(നിവേശം_2[pos]);
  }
  തിരികെ output;
}
