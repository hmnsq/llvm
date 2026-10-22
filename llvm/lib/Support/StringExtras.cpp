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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cctype>

using ನಾಮಾಕಾಶ llvm;

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

  ಪ್ರತ್ಯಾಗಮ {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
ಶೂನ್ಯ llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  ಯಾವತ್ (!S.first.empty()) {
    OutFragments.ತಳ್ಳು_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

ಶೂನ್ಯ llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  ಪ್ರತಿ (unsigned ವರ್ಣ C : Name) {
    ಯದಿ (C == '\\')
      Out << '\\' << C;
    ಅನ್ಯಥಾ ಯದಿ (isPrint(C) && C != '"')
      Out << C;
    ಅನ್ಯಥಾ
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

ಶೂನ್ಯ llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  ಪ್ರತಿ (ವರ್ಣ C : String) {
    ಯದಿ (C == '&')
      Out << "&amp;";
    ಅನ್ಯಥಾ ಯದಿ (C == '<')
      Out << "&lt;";
    ಅನ್ಯಥಾ ಯದಿ (C == '>')
      Out << "&gt;";
    ಅನ್ಯಥಾ ಯದಿ (C == '\"')
      Out << "&quot;";
    ಅನ್ಯಥಾ ಯದಿ (C == '\'')
      Out << "&apos;";
    ಅನ್ಯಥಾ
      Out << C;
  }
}

ಶೂನ್ಯ llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  ಪ್ರತಿ (ಧ್ರುವ ವರ್ಣ C : String)
    Out << toLower(C);
}

ಶೂನ್ಯ llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  ಪ್ರತಿ (unsigned ವರ್ಣ C : String) {
    ಯದಿ (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    ಅನ್ಯಥಾ
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef ಒಳಹರಿವು_2) {
  ಯದಿ (ಒಳಹರಿವು_2.empty())
    ಪ್ರತ್ಯಾಗಮ "";

  std::string snakeCase;
  snakeCase.reserve(ಒಳಹರಿವು_2.size());
  auto check = [&ಒಳಹರಿವು_2](size_t j, function_ref<ತಾರ್ಕಿಕ(ಪೂರ್ಣಾಙ್ಕ)> predicate) {
    ಪ್ರತ್ಯಾಗಮ j < ಒಳಹರಿವು_2.size() && predicate(ಒಳಹರಿವು_2[j]);
  };
  ಪ್ರತಿ (size_t i = 0; i < ಒಳಹರಿವು_2.size(); ++i) {
    snakeCase.ತಳ್ಳು_back(tolower(ಒಳಹರಿವು_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    ಯದಿ (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.ತಳ್ಳು_back('_');
    ಯದಿ ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.ತಳ್ಳು_back('_');
  }
  ಪ್ರತ್ಯಾಗಮ snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef ಒಳಹರಿವು_2,
                                              ತಾರ್ಕಿಕ capitalizeFirst) {
  ಯದಿ (ಒಳಹರಿವು_2.empty())
    ಪ್ರತ್ಯಾಗಮ "";

  std::string output;
  output.reserve(ಒಳಹರಿವು_2.size());

  // Push the first character, capatilizing if necessary.
  ಯದಿ (capitalizeFirst && std::islower(ಒಳಹರಿವು_2.front()))
    output.ತಳ್ಳು_back(llvm::toUpper(ಒಳಹರಿವು_2.front()));
  ಅನ್ಯಥಾ
    output.ತಳ್ಳು_back(ಒಳಹರಿವು_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  ಪ್ರತಿ (size_t pos = 1, e = ಒಳಹರಿವು_2.size(); pos < e; ++pos) {
    ಯದಿ (ಒಳಹರಿವು_2[pos] == '_' && pos != (e - 1) && std::islower(ಒಳಹರಿವು_2[pos + 1]))
      output.ತಳ್ಳು_back(llvm::toUpper(ಒಳಹರಿವು_2[++pos]));
    ಅನ್ಯಥಾ
      output.ತಳ್ಳು_back(ಒಳಹರಿವು_2[pos]);
  }
  ಪ್ರತ್ಯಾಗಮ output;
}
