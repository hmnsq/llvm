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

#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cctype>

using فضاینام llvm;

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

  بازگشت {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
تهی llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  وقتی (!S.first.empty()) {
    OutFragments.افزودن_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

تهی llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  برای (unsigned نویسه C : Name) {
    اگر (C == '\\')
      Out << '\\' << C;
    وگرنه اگر (isPrint(C) && C != '"')
      Out << C;
    وگرنه
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

تهی llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  برای (نویسه C : String) {
    اگر (C == '&')
      Out << "&amp;";
    وگرنه اگر (C == '<')
      Out << "&lt;";
    وگرنه اگر (C == '>')
      Out << "&gt;";
    وگرنه اگر (C == '\"')
      Out << "&quot;";
    وگرنه اگر (C == '\'')
      Out << "&apos;";
    وگرنه
      Out << C;
  }
}

تهی llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  برای (ثابت نویسه C : String)
    Out << toLower(C);
}

تهی llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  برای (unsigned نویسه C : String) {
    اگر (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    وگرنه
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef درون_داد_2) {
  اگر (درون_داد_2.empty())
    بازگشت "";

  std::string snakeCase;
  snakeCase.reserve(درون_داد_2.size());
  auto check = [&درون_داد_2](size_t j, function_ref<منطقی(صحیح)> predicate) {
    بازگشت j < درون_داد_2.size() && predicate(درون_داد_2[j]);
  };
  برای (size_t i = 0; i < درون_داد_2.size(); ++i) {
    snakeCase.افزودن_back(tolower(درون_داد_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    اگر (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.افزودن_back('_');
    اگر ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.افزودن_back('_');
  }
  بازگشت snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef درون_داد_2,
                                              منطقی capitalizeFirst) {
  اگر (درون_داد_2.empty())
    بازگشت "";

  std::string output;
  output.reserve(درون_داد_2.size());

  // Push the first character, capatilizing if necessary.
  اگر (capitalizeFirst && std::islower(درون_داد_2.front()))
    output.افزودن_back(llvm::toUpper(درون_داد_2.front()));
  وگرنه
    output.افزودن_back(درون_داد_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  برای (size_t pos = 1, e = درون_داد_2.size(); pos < e; ++pos) {
    اگر (درون_داد_2[pos] == '_' && pos != (e - 1) && std::islower(درون_داد_2[pos + 1]))
      output.افزودن_back(llvm::toUpper(درون_داد_2[++pos]));
    وگرنه
      output.افزودن_back(درون_داد_2[pos]);
  }
  بازگشت output;
}
