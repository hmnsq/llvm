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

#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cctype>

استعمال نامخلا llvm;

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<StringRef, StringRef> llvm::getToken(StringRef ماخذ,
                                               StringRef Delimiters) {
  // Figure out where the token starts.
  StringRef::سائز_type آغاز = ماخذ.ڈھونڈو_پہلا_not_کا(Delimiters);

  // Find the next occurrence of the delimiter.
  StringRef::سائز_type انتہا_2 = ماخذ.ڈھونڈو_پہلا_کا(Delimiters, آغاز);

  واپس {ماخذ.قاش(آغاز, انتہا_2), ماخذ.substr(انتہا_2)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
خالی llvm::بانٹو_String(StringRef ماخذ,
                       Small_سمتیہ_Impl<StringRef> &برآمد_Fragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(ماخذ, Delimiters);
  جب (!S.پہلا_2.خالی_خانے()) {
    برآمد_Fragments.ڈالو_عقب(S.پہلا_2);
    S = getToken(S.دوسرا_2, Delimiters);
  }
}

خالی llvm::printEscapedString(StringRef Name, raw_ostream &برآمد) {
  برائے (غیرعلامتی حرف C : Name) {
    اگر (C == '\\')
      برآمد << '\\' << C;
    ورنہ اگر (isPrint(C) && C != '"')
      برآمد << C;
    ورنہ
      برآمد << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

خالی llvm::printHTMLEscaped(StringRef String, raw_ostream &برآمد) {
  برائے (حرف C : String) {
    اگر (C == '&')
      برآمد << "&amp;";
    ورنہ اگر (C == '<')
      برآمد << "&lt;";
    ورنہ اگر (C == '>')
      برآمد << "&gt;";
    ورنہ اگر (C == '\"')
      برآمد << "&quot;";
    ورنہ اگر (C == '\'')
      برآمد << "&apos;";
    ورنہ
      برآمد << C;
  }
}

خالی llvm::print_زیریں_Case(StringRef String, raw_ostream &برآمد) {
  برائے (ثابت حرف C : String)
    برآمد << مقصد_زیریں(C);
}

خالی llvm::printPercentEncoded(StringRef String, raw_ostream &برآمد) {
  برائے (غیرعلامتی حرف C : String) {
    اگر (isAlnum(C) || StringRef("-_.~").شامل_ہے(C))
      برآمد << C;
    ورنہ
      برآمد << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convert_مقصد_Snake_From_Camel_Case(StringRef داخلہ_2) {
  اگر (داخلہ_2.خالی_خانے())
    واپس "";

  std::string snakeCase;
  snakeCase.reserve(داخلہ_2.سائز_2());
  خودکار check = [&داخلہ_2](size_t j, function_ref<منطقی(صحیح)> predicate) {
    واپس j < داخلہ_2.سائز_2() && predicate(داخلہ_2[j]);
  };
  برائے (size_t i = 0; i < داخلہ_2.سائز_2(); ++i) {
    snakeCase.ڈالو_عقب(tolower(داخلہ_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    اگر (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.ڈالو_عقب('_');
    اگر ((check(i, islower) || check(i, ہندسہ_ہے)) && check(i + 1, isupper))
      snakeCase.ڈالو_عقب('_');
  }
  واپس snakeCase;
}

std::string llvm::convert_مقصد_Camel_From_Snake_Case(StringRef داخلہ_2,
                                              منطقی capitalize_پہلا) {
  اگر (داخلہ_2.خالی_خانے())
    واپس "";

  std::string output;
  output.reserve(داخلہ_2.سائز_2());

  // Push the first character, capatilizing if necessary.
  اگر (capitalize_پہلا && std::islower(داخلہ_2.front()))
    output.ڈالو_عقب(llvm::مقصد_بالائی(داخلہ_2.front()));
  ورنہ
    output.ڈالو_عقب(داخلہ_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  برائے (size_t pos = 1, e = داخلہ_2.سائز_2(); pos < e; ++pos) {
    اگر (داخلہ_2[pos] == '_' && pos != (e - 1) && std::islower(داخلہ_2[pos + 1]))
      output.ڈالو_عقب(llvm::مقصد_بالائی(داخلہ_2[++pos]));
    ورنہ
      output.ڈالو_عقب(داخلہ_2[pos]);
  }
  واپس output;
}
