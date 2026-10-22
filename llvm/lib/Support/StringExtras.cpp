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

ورتوں نطاق llvm;

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<StringRef, StringRef> llvm::getToken(StringRef سروت,
                                               StringRef Delimiters) {
  // Figure out where the token starts.
  StringRef::ناپ_type شروع = سروت.لبھو_پہلا_not_دا(Delimiters);

  // Find the next occurrence of the delimiter.
  StringRef::ناپ_type سرا_2 = سروت.لبھو_پہلا_دا(Delimiters, شروع);

  أرجع {سروت.ٹکڑا(شروع, سرا_2), سروت.substr(سرا_2)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
فراغ llvm::ونڈو_String(StringRef سروت,
                       Small_سمتیہ_Impl<StringRef> &باہرلا_Fragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(سروت, Delimiters);
  طالما (!S.پہلا_2.خالی_تھاواں()) {
    باہرلا_Fragments.ڈالو_پٹھ(S.پہلا_2);
    S = getToken(S.دوجا_2, Delimiters);
  }
}

فراغ llvm::printEscapedString(StringRef Name, raw_ostream &باہرلا) {
  لكل (بنانشانی حرف C : Name) {
    إذا (C == '\\')
      باہرلا << '\\' << C;
    وإلا إذا (isPrint(C) && C != '"')
      باہرلا << C;
    وإلا
      باہرلا << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

فراغ llvm::printHTMLEscaped(StringRef String, raw_ostream &باہرلا) {
  لكل (حرف C : String) {
    إذا (C == '&')
      باہرلا << "&amp;";
    وإلا إذا (C == '<')
      باہرلا << "&lt;";
    وإلا إذا (C == '>')
      باہرلا << "&gt;";
    وإلا إذا (C == '\"')
      باہرلا << "&quot;";
    وإلا إذا (C == '\'')
      باہرلا << "&apos;";
    وإلا
      باہرلا << C;
  }
}

فراغ llvm::print_ہیٹھلا_Case(StringRef String, raw_ostream &باہرلا) {
  لكل (ثابت حرف C : String)
    باہرلا << منزل_ہیٹھلا(C);
}

فراغ llvm::printPercentEncoded(StringRef String, raw_ostream &باہرلا) {
  لكل (بنانشانی حرف C : String) {
    إذا (isAlnum(C) || StringRef("-_.~").شامل_اے(C))
      باہرلا << C;
    وإلا
      باہرلا << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convert_منزل_Snake_From_Camel_Case(StringRef داخلہ_2) {
  إذا (داخلہ_2.خالی_تھاواں())
    أرجع "";

  std::string snakeCase;
  snakeCase.reserve(داخلہ_2.ناپ_2());
  خودکار check = [&داخلہ_2](size_t j, function_ref<منطقي(صحيح)> predicate) {
    أرجع j < داخلہ_2.ناپ_2() && predicate(داخلہ_2[j]);
  };
  لكل (size_t i = 0; i < داخلہ_2.ناپ_2(); ++i) {
    snakeCase.ڈالو_پٹھ(tolower(داخلہ_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    إذا (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.ڈالو_پٹھ('_');
    إذا ((check(i, islower) || check(i, ہندسہ_اے)) && check(i + 1, isupper))
      snakeCase.ڈالو_پٹھ('_');
  }
  أرجع snakeCase;
}

std::string llvm::convert_منزل_Camel_From_Snake_Case(StringRef داخلہ_2,
                                              منطقي capitalize_پہلا) {
  إذا (داخلہ_2.خالی_تھاواں())
    أرجع "";

  std::string output;
  output.reserve(داخلہ_2.ناپ_2());

  // Push the first character, capatilizing if necessary.
  إذا (capitalize_پہلا && std::islower(داخلہ_2.front()))
    output.ڈالو_پٹھ(llvm::منزل_اتلا(داخلہ_2.front()));
  وإلا
    output.ڈالو_پٹھ(داخلہ_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  لكل (size_t pos = 1, e = داخلہ_2.ناپ_2(); pos < e; ++pos) {
    إذا (داخلہ_2[pos] == '_' && pos != (e - 1) && std::islower(داخلہ_2[pos + 1]))
      output.ڈالو_پٹھ(llvm::منزل_اتلا(داخلہ_2[++pos]));
    وإلا
      output.ڈالو_پٹھ(داخلہ_2[pos]);
  }
  أرجع output;
}
