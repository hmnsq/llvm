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

  반환 {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
void llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  동안 (!S.first.empty()) {
    OutFragments.push_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

void llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  위해 (unsigned char C : Name) {
    만약 (C == '\\')
      Out << '\\' << C;
    아니면 만약 (isPrint(C) && C != '"')
      Out << C;
    아니면
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

void llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  위해 (char C : String) {
    만약 (C == '&')
      Out << "&amp;";
    아니면 만약 (C == '<')
      Out << "&lt;";
    아니면 만약 (C == '>')
      Out << "&gt;";
    아니면 만약 (C == '\"')
      Out << "&quot;";
    아니면 만약 (C == '\'')
      Out << "&apos;";
    아니면
      Out << C;
  }
}

void llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  위해 (const char C : String)
    Out << toLower(C);
}

void llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  위해 (unsigned char C : String) {
    만약 (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    아니면
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef input) {
  만약 (input.empty())
    반환 "";

  std::string snakeCase;
  snakeCase.reserve(input.size());
  auto check = [&input](size_t j, function_ref<bool(정수)> predicate) {
    반환 j < input.size() && predicate(input[j]);
  };
  위해 (size_t i = 0; i < input.size(); ++i) {
    snakeCase.push_back(tolower(input[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    만약 (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.push_back('_');
    만약 ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.push_back('_');
  }
  반환 snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef input,
                                              bool capitalizeFirst) {
  만약 (input.empty())
    반환 "";

  std::string output;
  output.reserve(input.size());

  // Push the first character, capatilizing if necessary.
  만약 (capitalizeFirst && std::islower(input.front()))
    output.push_back(llvm::toUpper(input.front()));
  아니면
    output.push_back(input.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  위해 (size_t pos = 1, e = input.size(); pos < e; ++pos) {
    만약 (input[pos] == '_' && pos != (e - 1) && std::islower(input[pos + 1]))
      output.push_back(llvm::toUpper(input[++pos]));
    아니면
      output.push_back(input[pos]);
  }
  반환 output;
}
