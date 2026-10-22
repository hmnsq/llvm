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

#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"
#include <cctype>

using namespace llvm;

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

  ལོག {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
void llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  སྐབས (!S.first.empty()) {
    OutFragments.push_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

void llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  དོན་ལུ (unsigned var_na C : Name) {
    གལ་སྲིད (C == '\\')
      Out << '\\' << C;
    དེ་མིན གལ་སྲིད (isPrint(C) && C != '"')
      Out << C;
    དེ་མིན
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

void llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  དོན་ལུ (var_na C : String) {
    གལ་སྲིད (C == '&')
      Out << "&amp;";
    དེ་མིན གལ་སྲིད (C == '<')
      Out << "&lt;";
    དེ་མིན གལ་སྲིད (C == '>')
      Out << "&gt;";
    དེ་མིན གལ་སྲིད (C == '\"')
      Out << "&quot;";
    དེ་མིན གལ་སྲིད (C == '\'')
      Out << "&apos;";
    དེ་མིན
      Out << C;
  }
}

void llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  དོན་ལུ (const var_na C : String)
    Out << toLower(C);
}

void llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  དོན་ལུ (unsigned var_na C : String) {
    གལ་སྲིད (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    དེ་མིན
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef input) {
  གལ་སྲིད (input.empty())
    ལོག "";

  std::string snakeCase;
  snakeCase.reserve(input.size());
  auto check = [&input](size_t j, function_ref<bool(ཧྲིལ)> predicate) {
    ལོག j < input.size() && predicate(input[j]);
  };
  དོན་ལུ (size_t i = 0; i < input.size(); ++i) {
    snakeCase.push_back(tolower(input[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    གལ་སྲིད (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.push_back('_');
    གལ་སྲིད ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.push_back('_');
  }
  ལོག snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef input,
                                              bool capitalizeFirst) {
  གལ་སྲིད (input.empty())
    ལོག "";

  std::string output;
  output.reserve(input.size());

  // Push the first character, capatilizing if necessary.
  གལ་སྲིད (capitalizeFirst && std::islower(input.front()))
    output.push_back(llvm::toUpper(input.front()));
  དེ་མིན
    output.push_back(input.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  དོན་ལུ (size_t pos = 1, e = input.size(); pos < e; ++pos) {
    གལ་སྲིད (input[pos] == '_' && pos != (e - 1) && std::islower(input[pos + 1]))
      output.push_back(llvm::toUpper(input[++pos]));
    དེ་མིན
      output.push_back(input[pos]);
  }
  ལོག output;
}
