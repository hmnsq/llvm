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

  परत {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
शून्य llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  पर्यंत (!S.first.empty()) {
    OutFragments.ढकला_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

शून्य llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  साठी (अचिन्हित अक्षर C : Name) {
    जर (C == '\\')
      Out << '\\' << C;
    अन्यथा जर (isPrint(C) && C != '"')
      Out << C;
    अन्यथा
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

शून्य llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  साठी (अक्षर C : String) {
    जर (C == '&')
      Out << "&amp;";
    अन्यथा जर (C == '<')
      Out << "&lt;";
    अन्यथा जर (C == '>')
      Out << "&gt;";
    अन्यथा जर (C == '\"')
      Out << "&quot;";
    अन्यथा जर (C == '\'')
      Out << "&apos;";
    अन्यथा
      Out << C;
  }
}

शून्य llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  साठी (स्थिरांक अक्षर C : String)
    Out << toLower(C);
}

शून्य llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  साठी (अचिन्हित अक्षर C : String) {
    जर (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    अन्यथा
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef आदान_2) {
  जर (आदान_2.empty())
    परत "";

  std::string snakeCase;
  snakeCase.reserve(आदान_2.size());
  स्वतः check = [&आदान_2](माप_प्रकार j, function_ref<तार्किक(पूर्णांक)> predicate) {
    परत j < आदान_2.size() && predicate(आदान_2[j]);
  };
  साठी (माप_प्रकार i = 0; i < आदान_2.size(); ++i) {
    snakeCase.ढकला_back(छोटे(आदान_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    जर (check(i, है_बड़ा) && check(i + 1, है_बड़ा) && check(i + 2, है_छोटा))
      snakeCase.ढकला_back('_');
    जर ((check(i, है_छोटा) || check(i, है_अंक)) && check(i + 1, है_बड़ा))
      snakeCase.ढकला_back('_');
  }
  परत snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef आदान_2,
                                              तार्किक capitalizeFirst) {
  जर (आदान_2.empty())
    परत "";

  std::string output;
  output.reserve(आदान_2.size());

  // Push the first character, capatilizing if necessary.
  जर (capitalizeFirst && std::है_छोटा(आदान_2.front()))
    output.ढकला_back(llvm::toUpper(आदान_2.front()));
  अन्यथा
    output.ढकला_back(आदान_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  साठी (माप_प्रकार pos = 1, e = आदान_2.size(); pos < e; ++pos) {
    जर (आदान_2[pos] == '_' && pos != (e - 1) && std::है_छोटा(आदान_2[pos + 1]))
      output.ढकला_back(llvm::toUpper(आदान_2[++pos]));
    अन्यथा
      output.ढकला_back(आदान_2[pos]);
  }
  परत output;
}
