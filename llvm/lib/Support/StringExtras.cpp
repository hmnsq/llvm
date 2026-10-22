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
    OutFragments.تور_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

فراغ llvm::بغ_Escaped_String(StringRef Name, raw_ostream &Out) {
  لكل (unsigned حرف C : Name) {
    إذا (C == '\\')
      Out << '\\' << C;
    وإلا إذا (is_بغ(C) && C != '"')
      Out << C;
    وإلا
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

فراغ llvm::بغ_HTMLEscaped(StringRef String, raw_ostream &Out) {
  لكل (حرف C : String) {
    إذا (C == '&')
      Out << "&amp;";
    وإلا إذا (C == '<')
      Out << "&lt;";
    وإلا إذا (C == '>')
      Out << "&gt;";
    وإلا إذا (C == '\"')
      Out << "&quot;";
    وإلا إذا (C == '\'')
      Out << "&apos;";
    وإلا
      Out << C;
  }
}

فراغ llvm::بغ_Lower_Case(StringRef String, raw_ostream &Out) {
  لكل (ثابت حرف C : String)
    Out << toLower(C);
}

فراغ llvm::بغ_Percent_Encoded(StringRef String, raw_ostream &Out) {
  لكل (unsigned حرف C : String) {
    إذا (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    وإلا
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef شغرو_2) {
  إذا (شغرو_2.empty())
    أرجع "";

  std::string snakeCase;
  snakeCase.reserve(شغرو_2.size());
  auto check = [&شغرو_2](size_t j, function_ref<منطقي(صحيح)> predicate) {
    أرجع j < شغرو_2.size() && predicate(شغرو_2[j]);
  };
  لكل (size_t i = 0; i < شغرو_2.size(); ++i) {
    snakeCase.تور_back(tolower(شغرو_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    إذا (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.تور_back('_');
    إذا ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.تور_back('_');
  }
  أرجع snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef شغرو_2,
                                              منطقي capitalizeFirst) {
  إذا (شغرو_2.empty())
    أرجع "";

  std::string output;
  output.reserve(شغرو_2.size());

  // Push the first character, capatilizing if necessary.
  إذا (capitalizeFirst && std::islower(شغرو_2.front()))
    output.تور_back(llvm::toUpper(شغرو_2.front()));
  وإلا
    output.تور_back(شغرو_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  لكل (size_t pos = 1, e = شغرو_2.size(); pos < e; ++pos) {
    إذا (شغرو_2[pos] == '_' && pos != (e - 1) && std::islower(شغرو_2[pos + 1]))
      output.تور_back(llvm::toUpper(شغرو_2[++pos]));
    وإلا
      output.تور_back(شغرو_2[pos]);
  }
  أرجع output;
}
