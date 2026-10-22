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

  फर्काऊ {Source.slice(Start, End), Source.substr(End)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
शून्य llvm::SplitString(StringRef Source,
                       SmallVectorImpl<StringRef> &OutFragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  जबसम्म (!S.first.empty()) {
    OutFragments.थप_back(S.first);
    S = getToken(S.second, Delimiters);
  }
}

शून्य llvm::printEscapedString(StringRef Name, raw_ostream &Out) {
  लागि (अचिन्हित वर्ण C : Name) {
    यदि (C == '\\')
      Out << '\\' << C;
    अन्यथा यदि (isPrint(C) && C != '"')
      Out << C;
    अन्यथा
      Out << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

शून्य llvm::printHTMLEscaped(StringRef String, raw_ostream &Out) {
  लागि (वर्ण C : String) {
    यदि (C == '&')
      Out << "&amp;";
    अन्यथा यदि (C == '<')
      Out << "&lt;";
    अन्यथा यदि (C == '>')
      Out << "&gt;";
    अन्यथा यदि (C == '\"')
      Out << "&quot;";
    अन्यथा यदि (C == '\'')
      Out << "&apos;";
    अन्यथा
      Out << C;
  }
}

शून्य llvm::printLowerCase(StringRef String, raw_ostream &Out) {
  लागि (अचर वर्ण C : String)
    Out << toLower(C);
}

शून्य llvm::printPercentEncoded(StringRef String, raw_ostream &Out) {
  लागि (अचिन्हित वर्ण C : String) {
    यदि (isAlnum(C) || StringRef("-_.~").contains(C))
      Out << C;
    अन्यथा
      Out << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convertToSnakeFromCamelCase(StringRef आगत_2) {
  यदि (आगत_2.empty())
    फर्काऊ "";

  std::string snakeCase;
  snakeCase.reserve(आगत_2.size());
  स्वतः check = [&आगत_2](माप_प्रकार j, function_ref<तार्किक(पूर्णाङ्क)> predicate) {
    फर्काऊ j < आगत_2.size() && predicate(आगत_2[j]);
  };
  लागि (माप_प्रकार i = 0; i < आगत_2.size(); ++i) {
    snakeCase.थप_back(छोटे(आगत_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    यदि (check(i, है_बड़ा) && check(i + 1, है_बड़ा) && check(i + 2, है_छोटा))
      snakeCase.थप_back('_');
    यदि ((check(i, है_छोटा) || check(i, है_अंक)) && check(i + 1, है_बड़ा))
      snakeCase.थप_back('_');
  }
  फर्काऊ snakeCase;
}

std::string llvm::convertToCamelFromSnakeCase(StringRef आगत_2,
                                              तार्किक capitalizeFirst) {
  यदि (आगत_2.empty())
    फर्काऊ "";

  std::string output;
  output.reserve(आगत_2.size());

  // Push the first character, capatilizing if necessary.
  यदि (capitalizeFirst && std::है_छोटा(आगत_2.front()))
    output.थप_back(llvm::toUpper(आगत_2.front()));
  अन्यथा
    output.थप_back(आगत_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  लागि (माप_प्रकार pos = 1, e = आगत_2.size(); pos < e; ++pos) {
    यदि (आगत_2[pos] == '_' && pos != (e - 1) && std::है_छोटा(आगत_2[pos + 1]))
      output.थप_back(llvm::toUpper(आगत_2[++pos]));
    अन्यथा
      output.थप_back(आगत_2[pos]);
  }
  फर्काऊ output;
}
