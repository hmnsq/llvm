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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallVector.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <cctype>

using ନାମାକାଶ llvm;

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

  ଫେରନ୍ତୁ {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
ଶୂନ୍ୟ llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  ଯେତେବେଳେ (!S.first.empty()) {
    OutFragments.ଠେଲ_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

ଶୂନ୍ୟ llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  ପାଇଁ (unsigned ଅକ୍ଷର C : Name) {
    ଯଦି (C == '\\')
      Out << '\\' << C;
    ଅନ୍ୟଥା ଯଦି (isPrint(C) && C != '"')
      Out << C;
    ଅନ୍ୟଥା
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

ଶୂନ୍ୟ llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  ପାଇଁ (ଅକ୍ଷର C : String) {
    ଯଦି (C == '&')
      Out << "&amp;";
    ଅନ୍ୟଥା ଯଦି (C == '<')
      Out << "&lt;";
    ଅନ୍ୟଥା ଯଦି (C == '>')
      Out << "&gt;";
    ଅନ୍ୟଥା ଯଦି (C == '\"')
      Out << "&quot;";
    ଅନ୍ୟଥା ଯଦି (C == '\'')
      Out << "&apos;";
    ଅନ୍ୟଥା
      Out << C;
  }
}

ଶୂନ୍ୟ llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  ପାଇଁ (ଅଚଳ ଅକ୍ଷର C : String)
    Out << toLower(C);
}

ଶୂନ୍ୟ llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  ପାଇଁ (unsigned ଅକ୍ଷର C : String) {
    ଯଦି (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    ଅନ୍ୟଥା
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef ନିବେଶ_2) {
  ଯଦି (ନିବେଶ_2.empty())
    ଫେରନ୍ତୁ "";

  std::string snakeCase;
  snakeCase.reserve(ନିବେଶ_2.size());
  auto check = [&ନିବେଶ_2](size_t j, function_ref<ତାର୍କିକ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)> predicate) {
    ଫେରନ୍ତୁ j < ନିବେଶ_2.size() && predicate(ନିବେଶ_2[j]);
  };
  ପାଇଁ (size_t i = 0; i < ନିବେଶ_2.size(); ++i) {
    snakeCase.ଠେଲ_back(tolower(ନିବେଶ_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    ଯଦି (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.ଠେଲ_back('_');
    ଯଦି ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.ଠେଲ_back('_');
  }
  ଫେରନ୍ତୁ snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef ନିବେଶ_2,
                                              ତାର୍କିକ capitalizeFirst) {
  ଯଦି (ନିବେଶ_2.empty())
    ଫେରନ୍ତୁ "";

  std::string output;
  output.reserve(ନିବେଶ_2.size());

  // Push the first character, capatilizing if necessary.
  ଯଦି (capitalizeFirst && std::islower(ନିବେଶ_2.front()))
    output.ଠେଲ_back(llvm::toUpper(ନିବେଶ_2.front()));
  ଅନ୍ୟଥା
    output.ଠେଲ_back(ନିବେଶ_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  ପାଇଁ (size_t pos = 1, e = ନିବେଶ_2.size(); pos < e; ++pos) {
    ଯଦି (ନିବେଶ_2[pos] == '_' && pos != (e - 1) && std::islower(ନିବେଶ_2[pos + 1]))
      output.ଠେଲ_back(llvm::toUpper(ନିବେଶ_2[++pos]));
    ଅନ୍ୟଥା
      output.ଠେଲ_back(ନିବେଶ_2[pos]);
  }
  ଫେରନ୍ତୁ output;
}
