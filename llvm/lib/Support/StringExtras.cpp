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

#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cctype>

using नामाकाश llvm;

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

  प्रत्यागम {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
शून्य llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  जाव (!S.first.empty()) {
    OutFragments.प्रक्षिप_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

शून्य llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  प्रति (unsigned वर्ण C : Name) {
    जइ (C == '\\')
      Out << '\\' << C;
    अण्णहा जइ (isPrint(C) && C != '"')
      Out << C;
    अण्णहा
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

शून्य llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  प्रति (वर्ण C : String) {
    जइ (C == '&')
      Out << "&amp;";
    अण्णहा जइ (C == '<')
      Out << "&lt;";
    अण्णहा जइ (C == '>')
      Out << "&gt;";
    अण्णहा जइ (C == '\"')
      Out << "&quot;";
    अण्णहा जइ (C == '\'')
      Out << "&apos;";
    अण्णहा
      Out << C;
  }
}

शून्य llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  प्रति (ध्रुव वर्ण C : String)
    Out << toLower(C);
}

शून्य llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  प्रति (unsigned वर्ण C : String) {
    जइ (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    अण्णहा
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef निवेशः_2) {
  जइ (निवेशः_2.empty())
    प्रत्यागम "";

  std::string snakeCase;
  snakeCase.reserve(निवेशः_2.size());
  auto check = [&निवेशः_2](size_t j, function_ref<तार्किक(पूर्णाङ्क)> predicate) {
    प्रत्यागम j < निवेशः_2.size() && predicate(निवेशः_2[j]);
  };
  प्रति (size_t i = 0; i < निवेशः_2.size(); ++i) {
    snakeCase.प्रक्षिप_back(tolower(निवेशः_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    जइ (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.प्रक्षिप_back('_');
    जइ ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.प्रक्षिप_back('_');
  }
  प्रत्यागम snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef निवेशः_2,
                                              तार्किक capitalizeFirst) {
  जइ (निवेशः_2.empty())
    प्रत्यागम "";

  std::string output;
  output.reserve(निवेशः_2.size());

  // Push the first character, capatilizing if necessary.
  जइ (capitalizeFirst && std::islower(निवेशः_2.front()))
    output.प्रक्षिप_back(llvm::toUpper(निवेशः_2.front()));
  अण्णहा
    output.प्रक्षिप_back(निवेशः_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  प्रति (size_t pos = 1, e = निवेशः_2.size(); pos < e; ++pos) {
    जइ (निवेशः_2[pos] == '_' && pos != (e - 1) && std::islower(निवेशः_2[pos + 1]))
      output.प्रक्षिप_back(llvm::toUpper(निवेशः_2[++pos]));
    अण्णहा
      output.प्रक्षिप_back(निवेशः_2[pos]);
  }
  प्रत्यागम output;
}
