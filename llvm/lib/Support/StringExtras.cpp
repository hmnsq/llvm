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

#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/ADT/SmallVector.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cctype>

using நாமாகாஶ llvm;

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

  திரும்பு {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
வெறுமை llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  போது (!S.first.empty()) {
    OutFragments.தள்ளு_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

வெறுமை llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  க்கு (unsigned எழுத்து C : Name) {
    எனில் (C == '\\')
      Out << '\\' << C;
    இல்லையெனில் எனில் (isPrint(C) && C != '"')
      Out << C;
    இல்லையெனில்
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

வெறுமை llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  க்கு (எழுத்து C : String) {
    எனில் (C == '&')
      Out << "&amp;";
    இல்லையெனில் எனில் (C == '<')
      Out << "&lt;";
    இல்லையெனில் எனில் (C == '>')
      Out << "&gt;";
    இல்லையெனில் எனில் (C == '\"')
      Out << "&quot;";
    இல்லையெனில் எனில் (C == '\'')
      Out << "&apos;";
    இல்லையெனில்
      Out << C;
  }
}

வெறுமை llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  க்கு (மாறிலி எழுத்து C : String)
    Out << toLower(C);
}

வெறுமை llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  க்கு (unsigned எழுத்து C : String) {
    எனில் (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    இல்லையெனில்
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef உள்ளீடு_2) {
  எனில் (உள்ளீடு_2.empty())
    திரும்பு "";

  std::string snakeCase;
  snakeCase.reserve(உள்ளீடு_2.size());
  auto check = [&உள்ளீடு_2](size_t j, function_ref<தருக்க(முழு)> predicate) {
    திரும்பு j < உள்ளீடு_2.size() && predicate(உள்ளீடு_2[j]);
  };
  க்கு (size_t i = 0; i < உள்ளீடு_2.size(); ++i) {
    snakeCase.தள்ளு_back(tolower(உள்ளீடு_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    எனில் (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.தள்ளு_back('_');
    எனில் ((check(i, islower) || check(i, isdigit)) && check(i + 1, isupper))
      snakeCase.தள்ளு_back('_');
  }
  திரும்பு snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef உள்ளீடு_2,
                                              தருக்க capitalizeFirst) {
  எனில் (உள்ளீடு_2.empty())
    திரும்பு "";

  std::string output;
  output.reserve(உள்ளீடு_2.size());

  // Push the first character, capatilizing if necessary.
  எனில் (capitalizeFirst && std::islower(உள்ளீடு_2.front()))
    output.தள்ளு_back(llvm::toUpper(உள்ளீடு_2.front()));
  இல்லையெனில்
    output.தள்ளு_back(உள்ளீடு_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  க்கு (size_t pos = 1, e = உள்ளீடு_2.size(); pos < e; ++pos) {
    எனில் (உள்ளீடு_2[pos] == '_' && pos != (e - 1) && std::islower(உள்ளீடு_2[pos + 1]))
      output.தள்ளு_back(llvm::toUpper(உள்ளீடு_2[++pos]));
    இல்லையெனில்
      output.தள்ளு_back(உள்ளீடு_2[pos]);
  }
  திரும்பு output;
}
