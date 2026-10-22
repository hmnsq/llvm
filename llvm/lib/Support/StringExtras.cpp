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

उपयुज्य नामाकाश llvm;

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<StringRef, StringRef> llvm::getToken(StringRef Source,
                                               StringRef Delimiters) {
  // Figure out where the token starts.
  StringRef::आकार_type Start = Source.find_first_not_सम्बन्धि(Delimiters);

  // Find the next occurrence of the delimiter.
  StringRef::आकार_type अन्तः_2 = Source.find_first_सम्बन्धि(Delimiters, Start);

  प्रत्यागम {Source.खण्डिका(Start, अन्तः_2), Source.substr(अन्तः_2)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
शून्य llvm::विभज_String(StringRef Source,
                       Small_सदिशम्_Impl<StringRef> &बहिर्गमनम्_Fragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(Source, Delimiters);
  यावत् (!S.first.रिक्तानि()) {
    बहिर्गमनम्_Fragments.प्रक्षिप_पृष्ठभागः(S.first);
    S = getToken(S.second, Delimiters);
  }
}

शून्य llvm::printEscapedString(StringRef Name, raw_ostream &बहिर्गमनम्) {
  प्रति (अचिह्नित वर्ण C : Name) {
    यदि (C == '\\')
      बहिर्गमनम् << '\\' << C;
    अन्यथा यदि (isPrint(C) && C != '"')
      बहिर्गमनम् << C;
    अन्यथा
      बहिर्गमनम् << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

शून्य llvm::printHTMLEscaped(StringRef String, raw_ostream &बहिर्गमनम्) {
  प्रति (वर्ण C : String) {
    यदि (C == '&')
      बहिर्गमनम् << "&amp;";
    अन्यथा यदि (C == '<')
      बहिर्गमनम् << "&lt;";
    अन्यथा यदि (C == '>')
      बहिर्गमनम् << "&gt;";
    अन्यथा यदि (C == '\"')
      बहिर्गमनम् << "&quot;";
    अन्यथा यदि (C == '\'')
      बहिर्गमनम् << "&apos;";
    अन्यथा
      बहिर्गमनम् << C;
  }
}

शून्य llvm::print_अधस्तनम्_Case(StringRef String, raw_ostream &बहिर्गमनम्) {
  प्रति (ध्रुव वर्ण C : String)
    बहिर्गमनम् << गन्तव्यम्_अधस्तनम्(C);
}

शून्य llvm::printPercentEncoded(StringRef String, raw_ostream &बहिर्गमनम्) {
  प्रति (अचिह्नित वर्ण C : String) {
    यदि (isAlnum(C) || StringRef("-_.~").अन्तर्भवति(C))
      बहिर्गमनम् << C;
    अन्यथा
      बहिर्गमनम् << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convert_गन्तव्यम्_Snake_From_Camel_Case(StringRef निवेशः_2) {
  यदि (निवेशः_2.रिक्तानि())
    प्रत्यागम "";

  std::string snakeCase;
  snakeCase.reserve(निवेशः_2.आकार_2());
  स्वचालित check = [&निवेशः_2](size_t j, function_ref<तार्किक(पूर्णाङ्क)> predicate) {
    प्रत्यागम j < निवेशः_2.आकार_2() && predicate(निवेशः_2[j]);
  };
  प्रति (size_t i = 0; i < निवेशः_2.आकार_2(); ++i) {
    snakeCase.प्रक्षिप_पृष्ठभागः(tolower(निवेशः_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    यदि (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.प्रक्षिप_पृष्ठभागः('_');
    यदि ((check(i, islower) || check(i, अङ्कः_वा)) && check(i + 1, isupper))
      snakeCase.प्रक्षिप_पृष्ठभागः('_');
  }
  प्रत्यागम snakeCase;
}

std::string llvm::convert_गन्तव्यम्_Camel_From_Snake_Case(StringRef निवेशः_2,
                                              तार्किक capitalizeFirst) {
  यदि (निवेशः_2.रिक्तानि())
    प्रत्यागम "";

  std::string output;
  output.reserve(निवेशः_2.आकार_2());

  // Push the first character, capatilizing if necessary.
  यदि (capitalizeFirst && std::islower(निवेशः_2.front()))
    output.प्रक्षिप_पृष्ठभागः(llvm::गन्तव्यम्_ऊर्ध्वस्थम्(निवेशः_2.front()));
  अन्यथा
    output.प्रक्षिप_पृष्ठभागः(निवेशः_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  प्रति (size_t pos = 1, e = निवेशः_2.आकार_2(); pos < e; ++pos) {
    यदि (निवेशः_2[pos] == '_' && pos != (e - 1) && std::islower(निवेशः_2[pos + 1]))
      output.प्रक्षिप_पृष्ठभागः(llvm::गन्तव्यम्_ऊर्ध्वस्थम्(निवेशः_2[++pos]));
    अन्यथा
      output.प्रक्षिप_पृष्ठभागः(निवेशः_2[pos]);
  }
  प्रत्यागम output;
}
