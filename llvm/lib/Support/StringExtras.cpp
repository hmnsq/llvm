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

استفاده نطاق llvm;

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<StringRef, StringRef> llvm::getToken(StringRef منبع,
                                               StringRef Delimiters) {
  // Figure out where the token starts.
  StringRef::اندازه_type شروع = منبع.پیدا_کن_اولی_not_از(Delimiters);

  // Find the next occurrence of the delimiter.
  StringRef::اندازه_type انتها_2 = منبع.پیدا_کن_اولی_از(Delimiters, شروع);

  أرجع {منبع.برش(شروع, انتها_2), منبع.substr(انتها_2)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
فراغ llvm::تقسیم_String(StringRef منبع,
                       Small_وکتور_Impl<StringRef> &بیرون_Fragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(منبع, Delimiters);
  طالما (!S.اولی_2.خالی()) {
    بیرون_Fragments.افزودن_پشت(S.اولی_2);
    S = getToken(S.دومی_2, Delimiters);
  }
}

فراغ llvm::printEscapedString(StringRef Name, raw_ostream &بیرون) {
  لكل (بیعلامت حرف C : Name) {
    إذا (C == '\\')
      بیرون << '\\' << C;
    وإلا إذا (isPrint(C) && C != '"')
      بیرون << C;
    وإلا
      بیرون << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

فراغ llvm::printHTMLEscaped(StringRef String, raw_ostream &بیرون) {
  لكل (حرف C : String) {
    إذا (C == '&')
      بیرون << "&amp;";
    وإلا إذا (C == '<')
      بیرون << "&lt;";
    وإلا إذا (C == '>')
      بیرون << "&gt;";
    وإلا إذا (C == '\"')
      بیرون << "&quot;";
    وإلا إذا (C == '\'')
      بیرون << "&apos;";
    وإلا
      بیرون << C;
  }
}

فراغ llvm::print_پایینی_Case(StringRef String, raw_ostream &بیرون) {
  لكل (ثابت حرف C : String)
    بیرون << مقصد_پایینی(C);
}

فراغ llvm::printPercentEncoded(StringRef String, raw_ostream &بیرون) {
  لكل (بیعلامت حرف C : String) {
    إذا (isAlnum(C) || StringRef("-_.~").شامل_است(C))
      بیرون << C;
    وإلا
      بیرون << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convert_مقصد_Snake_From_Camel_Case(StringRef درون_داد_2) {
  إذا (درون_داد_2.خالی())
    أرجع "";

  std::string snakeCase;
  snakeCase.reserve(درون_داد_2.اندازه_2());
  خودکار check = [&درون_داد_2](size_t j, function_ref<منطقي(صحيح)> predicate) {
    أرجع j < درون_داد_2.اندازه_2() && predicate(درون_داد_2[j]);
  };
  لكل (size_t i = 0; i < درون_داد_2.اندازه_2(); ++i) {
    snakeCase.افزودن_پشت(tolower(درون_داد_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    إذا (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.افزودن_پشت('_');
    إذا ((check(i, islower) || check(i, آیا_رقم)) && check(i + 1, isupper))
      snakeCase.افزودن_پشت('_');
  }
  أرجع snakeCase;
}

std::string llvm::convert_مقصد_Camel_From_Snake_Case(StringRef درون_داد_2,
                                              منطقي capitalize_اولی) {
  إذا (درون_داد_2.خالی())
    أرجع "";

  std::string output;
  output.reserve(درون_داد_2.اندازه_2());

  // Push the first character, capatilizing if necessary.
  إذا (capitalize_اولی && std::islower(درون_داد_2.front()))
    output.افزودن_پشت(llvm::مقصد_فوقانی(درون_داد_2.front()));
  وإلا
    output.افزودن_پشت(درون_داد_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  لكل (size_t pos = 1, e = درون_داد_2.اندازه_2(); pos < e; ++pos) {
    إذا (درون_داد_2[pos] == '_' && pos != (e - 1) && std::islower(درون_داد_2[pos + 1]))
      output.افزودن_پشت(llvm::مقصد_فوقانی(درون_داد_2[++pos]));
    وإلا
      output.افزودن_پشت(درون_داد_2[pos]);
  }
  أرجع output;
}
