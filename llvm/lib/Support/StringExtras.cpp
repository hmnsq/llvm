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

#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cctype>

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
  StringRef::आकार_type Start = Source.find_first_not_of(Delimiters);

  // Find the next occurrence of the delimiter.
  StringRef::आकार_type End = Source.find_first_of(Delimiters, Start);

  वापस {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
व्योम llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  जबतक (!S.first.empty()) {
    OutFragments.धकेलो_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

व्योम llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  क्रम (अचिन्हित अक्षर C : Name) {
    अगर (C == '\\')
      Out << '\\' << C;
    अन्यथा अगर (isPrint(C) && C != '"')
      Out << C;
    अन्यथा
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

व्योम llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  क्रम (अक्षर C : String) {
    अगर (C == '&')
      Out << "&amp;";
    अन्यथा अगर (C == '<')
      Out << "&lt;";
    अन्यथा अगर (C == '>')
      Out << "&gt;";
    अन्यथा अगर (C == '\"')
      Out << "&quot;";
    अन्यथा अगर (C == '\'')
      Out << "&apos;";
    अन्यथा
      Out << C;
  }
}

व्योम llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  क्रम (स्थिर अक्षर C : String)
    Out << toLower(C);
}

व्योम llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  क्रम (अचिन्हित अक्षर C : String) {
    अगर (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    अन्यथा
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef निवेश_2) {
  अगर (निवेश_2.empty())
    वापस "";

  std::string snakeCase;
  snakeCase.reserve(निवेश_2.आकार_2());
  स्वतः check = [&निवेश_2](माप_प्रकार j, function_ref<bool(पूर्णांक)> predicate) {
    वापस j < निवेश_2.आकार_2() && predicate(निवेश_2[j]);
  };
  क्रम (माप_प्रकार i = 0; i < निवेश_2.आकार_2(); ++i) {
    snakeCase.धकेलो_back(छोटे(निवेश_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    अगर (check(i, है_बड़ा) && check(i + 1, है_बड़ा) && check(i + 2, है_छोटा))
      snakeCase.धकेलो_back('_');
    अगर ((check(i, है_छोटा) || check(i, है_अंक)) && check(i + 1, है_बड़ा))
      snakeCase.धकेलो_back('_');
  }
  वापस snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef निवेश_2,
                                              bool capitalizeFirst) {
  अगर (निवेश_2.empty())
    वापस "";

  std::string output;
  output.reserve(निवेश_2.आकार_2());

  // Push the first character, capatilizing if necessary.
  अगर (capitalizeFirst && std::है_छोटा(निवेश_2.front()))
    output.धकेलो_back(llvm::toUpper(निवेश_2.front()));
  अन्यथा
    output.धकेलो_back(निवेश_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  क्रम (माप_प्रकार pos = 1, e = निवेश_2.आकार_2(); pos < e; ++pos) {
    अगर (निवेश_2[pos] == '_' && pos != (e - 1) && std::है_छोटा(निवेश_2[pos + 1]))
      output.धकेलो_back(llvm::toUpper(निवेश_2[++pos]));
    अन्यथा
      output.धकेलो_back(निवेश_2[pos]);
  }
  वापस output;
}
