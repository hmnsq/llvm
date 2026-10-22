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

استعمال نطاق llvm;

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<StringRef, StringRef> llvm::getToken(StringRef ذريعو,
                                               StringRef Delimiters) {
  // Figure out where the token starts.
  StringRef::سائيز_type شروعات = ذريعو.ڳوليو_پهريون_not_جو(Delimiters);

  // Find the next occurrence of the delimiter.
  StringRef::سائيز_type پڇاڙي_2 = ذريعو.ڳوليو_پهريون_جو(Delimiters, شروعات);

  أرجع {ذريعو.ٽڪرو(شروعات, پڇاڙي_2), ذريعو.substr(پڇاڙي_2)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
فراغ llvm::ورهايو_String(StringRef ذريعو,
                       Small_ويڪٽر_Impl<StringRef> &ٻاهريون_Fragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(ذريعو, Delimiters);
  طالما (!S.پهريون_2.خالي_جايون()) {
    ٻاهريون_Fragments.ڌڪو_پٺ(S.پهريون_2);
    S = getToken(S.ٻيو_2, Delimiters);
  }
}

فراغ llvm::printEscapedString(StringRef Name, raw_ostream &ٻاهريون) {
  لكل (بنانشان حرف C : Name) {
    إذا (C == '\\')
      ٻاهريون << '\\' << C;
    وإلا إذا (isPrint(C) && C != '"')
      ٻاهريون << C;
    وإلا
      ٻاهريون << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

فراغ llvm::printHTMLEscaped(StringRef String, raw_ostream &ٻاهريون) {
  لكل (حرف C : String) {
    إذا (C == '&')
      ٻاهريون << "&amp;";
    وإلا إذا (C == '<')
      ٻاهريون << "&lt;";
    وإلا إذا (C == '>')
      ٻاهريون << "&gt;";
    وإلا إذا (C == '\"')
      ٻاهريون << "&quot;";
    وإلا إذا (C == '\'')
      ٻاهريون << "&apos;";
    وإلا
      ٻاهريون << C;
  }
}

فراغ llvm::print_هيٺاهون_Case(StringRef String, raw_ostream &ٻاهريون) {
  لكل (ثابت حرف C : String)
    ٻاهريون << منزل_هيٺاهون(C);
}

فراغ llvm::printPercentEncoded(StringRef String, raw_ostream &ٻاهريون) {
  لكل (بنانشان حرف C : String) {
    إذا (isAlnum(C) || StringRef("-_.~").شامل_آهي(C))
      ٻاهريون << C;
    وإلا
      ٻاهريون << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convert_منزل_Snake_From_Camel_Case(StringRef داخلا_2) {
  إذا (داخلا_2.خالي_جايون())
    أرجع "";

  std::string snakeCase;
  snakeCase.reserve(داخلا_2.سائيز_2());
  خودڪار check = [&داخلا_2](size_t j, function_ref<منطقي(صحيح)> predicate) {
    أرجع j < داخلا_2.سائيز_2() && predicate(داخلا_2[j]);
  };
  لكل (size_t i = 0; i < داخلا_2.سائيز_2(); ++i) {
    snakeCase.ڌڪو_پٺ(tolower(داخلا_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    إذا (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.ڌڪو_پٺ('_');
    إذا ((check(i, islower) || check(i, انگ_آهي)) && check(i + 1, isupper))
      snakeCase.ڌڪو_پٺ('_');
  }
  أرجع snakeCase;
}

std::string llvm::convert_منزل_Camel_From_Snake_Case(StringRef داخلا_2,
                                              منطقي capitalize_پهريون) {
  إذا (داخلا_2.خالي_جايون())
    أرجع "";

  std::string output;
  output.reserve(داخلا_2.سائيز_2());

  // Push the first character, capatilizing if necessary.
  إذا (capitalize_پهريون && std::islower(داخلا_2.front()))
    output.ڌڪو_پٺ(llvm::منزل_مٿانهون(داخلا_2.front()));
  وإلا
    output.ڌڪو_پٺ(داخلا_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  لكل (size_t pos = 1, e = داخلا_2.سائيز_2(); pos < e; ++pos) {
    إذا (داخلا_2[pos] == '_' && pos != (e - 1) && std::islower(داخلا_2[pos + 1]))
      output.ڌڪو_پٺ(llvm::منزل_مٿانهون(داخلا_2[++pos]));
    وإلا
      output.ڌڪو_پٺ(داخلا_2[pos]);
  }
  أرجع output;
}
