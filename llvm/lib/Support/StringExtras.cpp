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

کارول نطاق llvm;

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<StringRef, StringRef> llvm::getToken(StringRef سرچینه,
                                               StringRef Delimiters) {
  // Figure out where the token starts.
  StringRef::اندازه_type پیل = سرچینه.پیدا_کړه_لومړی_not_د(Delimiters);

  // Find the next occurrence of the delimiter.
  StringRef::اندازه_type پای_ټکی_2 = سرچینه.پیدا_کړه_لومړی_د(Delimiters, پیل);

  أرجع {سرچینه.ټوټه(پیل, پای_ټکی_2), سرچینه.substr(پای_ټکی_2)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
فراغ llvm::وېشل_String(StringRef سرچینه,
                       Small_ویکټور_Impl<StringRef> &بهر_Fragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(سرچینه, Delimiters);
  طالما (!S.لومړی_2.تشه()) {
    بهر_Fragments.ټېل_شا(S.لومړی_2);
    S = getToken(S.دوهم_2, Delimiters);
  }
}

فراغ llvm::printEscapedString(StringRef Name, raw_ostream &بهر) {
  لكل (بینښه حرف C : Name) {
    إذا (C == '\\')
      بهر << '\\' << C;
    وإلا إذا (isPrint(C) && C != '"')
      بهر << C;
    وإلا
      بهر << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

فراغ llvm::printHTMLEscaped(StringRef String, raw_ostream &بهر) {
  لكل (حرف C : String) {
    إذا (C == '&')
      بهر << "&amp;";
    وإلا إذا (C == '<')
      بهر << "&lt;";
    وإلا إذا (C == '>')
      بهر << "&gt;";
    وإلا إذا (C == '\"')
      بهر << "&quot;";
    وإلا إذا (C == '\'')
      بهر << "&apos;";
    وإلا
      بهر << C;
  }
}

فراغ llvm::print_ښکتنی_Case(StringRef String, raw_ostream &بهر) {
  لكل (ثابت حرف C : String)
    بهر << منزل_ښکتنی(C);
}

فراغ llvm::printPercentEncoded(StringRef String, raw_ostream &بهر) {
  لكل (بینښه حرف C : String) {
    إذا (isAlnum(C) || StringRef("-_.~").لري(C))
      بهر << C;
    وإلا
      بهر << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convert_منزل_Snake_From_Camel_Case(StringRef ننوت_2) {
  إذا (ننوت_2.تشه())
    أرجع "";

  std::string snakeCase;
  snakeCase.reserve(ننوت_2.اندازه_2());
  خپلکاری check = [&ننوت_2](size_t j, function_ref<منطقي(صحيح)> predicate) {
    أرجع j < ننوت_2.اندازه_2() && predicate(ننوت_2[j]);
  };
  لكل (size_t i = 0; i < ننوت_2.اندازه_2(); ++i) {
    snakeCase.ټېل_شا(tolower(ننوت_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    إذا (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.ټېل_شا('_');
    إذا ((check(i, islower) || check(i, که_عدد)) && check(i + 1, isupper))
      snakeCase.ټېل_شا('_');
  }
  أرجع snakeCase;
}

std::string llvm::convert_منزل_Camel_From_Snake_Case(StringRef ننوت_2,
                                              منطقي capitalize_لومړی) {
  إذا (ننوت_2.تشه())
    أرجع "";

  std::string output;
  output.reserve(ننوت_2.اندازه_2());

  // Push the first character, capatilizing if necessary.
  إذا (capitalize_لومړی && std::islower(ننوت_2.front()))
    output.ټېل_شا(llvm::منزل_پاسنی(ننوت_2.front()));
  وإلا
    output.ټېل_شا(ننوت_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  لكل (size_t pos = 1, e = ننوت_2.اندازه_2(); pos < e; ++pos) {
    إذا (ننوت_2[pos] == '_' && pos != (e - 1) && std::islower(ننوت_2[pos + 1]))
      output.ټېل_شا(llvm::منزل_پاسنی(ننوت_2[++pos]));
    وإلا
      output.ټېل_شا(ننوت_2[pos]);
  }
  أرجع output;
}
