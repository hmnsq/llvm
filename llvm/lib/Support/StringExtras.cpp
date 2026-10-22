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

#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/ADT/SmallVector.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <cctype>

using নামাকাশ llvm;

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

  প্রত্যাগম {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
শূন্য llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  যাবত্ (!S.first.empty()) {
    OutFragments.ঠেলো_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

শূন্য llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  প্রতি (unsigned বর্ণ C : Name) {
    যদি (C == '\\')
      Out << '\\' << C;
    অন্যথা যদি (isPrint(C) && C != '"')
      Out << C;
    অন্যথা
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

শূন্য llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  প্রতি (বর্ণ C : String) {
    যদি (C == '&')
      Out << "&amp;";
    অন্যথা যদি (C == '<')
      Out << "&lt;";
    অন্যথা যদি (C == '>')
      Out << "&gt;";
    অন্যথা যদি (C == '\"')
      Out << "&quot;";
    অন্যথা যদি (C == '\'')
      Out << "&apos;";
    অন্যথা
      Out << C;
  }
}

শূন্য llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  প্রতি (ধ্রুব বর্ণ C : String)
    Out << toLower(C);
}

শূন্য llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  প্রতি (unsigned বর্ণ C : String) {
    যদি (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    অন্যথা
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef প্রবেশ_2) {
  যদি (প্রবেশ_2.empty())
    প্রত্যাগম "";

  std::string snakeCase;
  snakeCase.reserve(প্রবেশ_2.size());
  auto check = [&প্রবেশ_2](size_t j, function_ref<তার্কিক(পূর্ণাঙ্ক)> predicate) {
    প্রত্যাগম j < প্রবেশ_2.size() && predicate(প্রবেশ_2[j]);
  };
  প্রতি (size_t i = 0; i < প্রবেশ_2.size(); ++i) {
    snakeCase.ঠেলো_back(tolower(প্রবেশ_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    যদি (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.ঠেলো_back('_');
    যদি ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.ঠেলো_back('_');
  }
  প্রত্যাগম snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef প্রবেশ_2,
                                              তার্কিক capitalizeFirst) {
  যদি (প্রবেশ_2.empty())
    প্রত্যাগম "";

  std::string output;
  output.reserve(প্রবেশ_2.size());

  // Push the first character, capatilizing if necessary.
  যদি (capitalizeFirst && std::islower(প্রবেশ_2.front()))
    output.ঠেলো_back(llvm::toUpper(প্রবেশ_2.front()));
  অন্যথা
    output.ঠেলো_back(প্রবেশ_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  প্রতি (size_t pos = 1, e = প্রবেশ_2.size(); pos < e; ++pos) {
    যদি (প্রবেশ_2[pos] == '_' && pos != (e - 1) && std::islower(প্রবেশ_2[pos + 1]))
      output.ঠেলো_back(llvm::toUpper(প্রবেশ_2[++pos]));
    অন্যথা
      output.ঠেলো_back(প্রবেশ_2[pos]);
  }
  প্রত্যাগম output;
}
