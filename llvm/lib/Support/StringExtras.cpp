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

#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cctype>

using نطاق llvm;

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

  أرجع {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
فراغ llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  طالما (!S.first.empty()) {
    OutFragments.push_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

فراغ llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  لكل (unsigned حرف C : Name) {
    𐡄𐡍 (C == '\\')
      Out << '\\' << C;
    وإلا 𐡄𐡍 (isPrint(C) && C != '"')
      Out << C;
    وإلا
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

فراغ llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  لكل (حرف C : String) {
    𐡄𐡍 (C == '&')
      Out << "&amp;";
    وإلا 𐡄𐡍 (C == '<')
      Out << "&lt;";
    وإلا 𐡄𐡍 (C == '>')
      Out << "&gt;";
    وإلا 𐡄𐡍 (C == '\"')
      Out << "&quot;";
    وإلا 𐡄𐡍 (C == '\'')
      Out << "&apos;";
    وإلا
      Out << C;
  }
}

فراغ llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  لكل (ثابت حرف C : String)
    Out << toLower(C);
}

فراغ llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  لكل (unsigned حرف C : String) {
    𐡄𐡍 (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    وإلا
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef input) {
  𐡄𐡍 (input.empty())
    أرجع "";

  std::string snakeCase;
  snakeCase.reserve(input.size());
  auto check = [&input](size_t j, function_ref<منطقي(صحيح)> predicate) {
    أرجع j < input.size() && predicate(input[j]);
  };
  لكل (size_t i = 0; i < input.size(); ++i) {
    snakeCase.push_back(tolower(input[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    𐡄𐡍 (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.push_back('_');
    𐡄𐡍 ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.push_back('_');
  }
  أرجع snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef input,
                                              منطقي capitalizeFirst) {
  𐡄𐡍 (input.empty())
    أرجع "";

  std::string output;
  output.reserve(input.size());

  // Push the first character, capatilizing if necessary.
  𐡄𐡍 (capitalizeFirst && std::islower(input.front()))
    output.push_back(llvm::toUpper(input.front()));
  وإلا
    output.push_back(input.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  لكل (size_t pos = 1, e = input.size(); pos < e; ++pos) {
    𐡄𐡍 (input[pos] == '_' && pos != (e - 1) && std::islower(input[pos + 1]))
      output.push_back(llvm::toUpper(input[++pos]));
    وإلا
      output.push_back(input[pos]);
  }
  أرجع output;
}
