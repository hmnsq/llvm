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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cctype>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<StringRef, StringRef> llvm::getToken(StringRef ਸਰੋਤ,
                                               StringRef Delimiters) {
  // Figure out where the token starts.
  StringRef::ਮਾਪ_type ਸ਼ੁਰੂ = ਸਰੋਤ.ਲੱਭੋ_ਪਹਿਲਾ_not_ਦਾ(Delimiters);

  // Find the next occurrence of the delimiter.
  StringRef::ਮਾਪ_type ਸਿਰਾ_2 = ਸਰੋਤ.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(Delimiters, ਸ਼ੁਰੂ);

  ਵਾਪਸ {ਸਰੋਤ.ਟੁਕੜਾ(ਸ਼ੁਰੂ, ਸਿਰਾ_2), ਸਰੋਤ.substr(ਸਿਰਾ_2)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
ਖਾਲੀ llvm::ਵੰਡੋ_String(StringRef ਸਰੋਤ,
                       Small_ਸਦਿਸ਼_Impl<StringRef> &ਬਾਹਰਲਾ_Fragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(ਸਰੋਤ, Delimiters);
  ਜਦੋਂ (!S.ਪਹਿਲਾ_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਬਾਹਰਲਾ_Fragments.ਧੱਕੋ_ਪਿੱਠ(S.ਪਹਿਲਾ_2);
    S = getToken(S.ਦੂਜਾ_2, Delimiters);
  }
}

ਖਾਲੀ llvm::printEscapedString(StringRef Name, raw_ostream &ਬਾਹਰਲਾ) {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ C : Name) {
    ਜੇ (C == '\\')
      ਬਾਹਰਲਾ << '\\' << C;
    ਨਹੀਂਤਾਂ ਜੇ (isPrint(C) && C != '"')
      ਬਾਹਰਲਾ << C;
    ਨਹੀਂਤਾਂ
      ਬਾਹਰਲਾ << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

ਖਾਲੀ llvm::printHTMLEscaped(StringRef String, raw_ostream &ਬਾਹਰਲਾ) {
  ਲਈ (ਅੱਖਰ C : String) {
    ਜੇ (C == '&')
      ਬਾਹਰਲਾ << "&amp;";
    ਨਹੀਂਤਾਂ ਜੇ (C == '<')
      ਬਾਹਰਲਾ << "&lt;";
    ਨਹੀਂਤਾਂ ਜੇ (C == '>')
      ਬਾਹਰਲਾ << "&gt;";
    ਨਹੀਂਤਾਂ ਜੇ (C == '\"')
      ਬਾਹਰਲਾ << "&quot;";
    ਨਹੀਂਤਾਂ ਜੇ (C == '\'')
      ਬਾਹਰਲਾ << "&apos;";
    ਨਹੀਂਤਾਂ
      ਬਾਹਰਲਾ << C;
  }
}

ਖਾਲੀ llvm::print_ਹੇਠਲਾ_Case(StringRef String, raw_ostream &ਬਾਹਰਲਾ) {
  ਲਈ (ਅਚਲ ਅੱਖਰ C : String)
    ਬਾਹਰਲਾ << ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(C);
}

ਖਾਲੀ llvm::printPercentEncoded(StringRef String, raw_ostream &ਬਾਹਰਲਾ) {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ C : String) {
    ਜੇ (isAlnum(C) || StringRef("-_.~").ਸ਼ਾਮਲ_ਹੈ(C))
      ਬਾਹਰਲਾ << C;
    ਨਹੀਂਤਾਂ
      ਬਾਹਰਲਾ << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convert_ਮੰਜ਼ਿਲ_Snake_From_Camel_Case(StringRef ਨਿਵੇਸ਼_2) {
  ਜੇ (ਨਿਵੇਸ਼_2.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ "";

  std::string snakeCase;
  snakeCase.reserve(ਨਿਵੇਸ਼_2.ਮਾਪ_2());
  ਸਵੈਚਾਲਿਤ check = [&ਨਿਵੇਸ਼_2](size_t j, function_ref<ਤਾਰਕਿਕ(ਪੂਰਨ)> predicate) {
    ਵਾਪਸ j < ਨਿਵੇਸ਼_2.ਮਾਪ_2() && predicate(ਨਿਵੇਸ਼_2[j]);
  };
  ਲਈ (size_t i = 0; i < ਨਿਵੇਸ਼_2.ਮਾਪ_2(); ++i) {
    snakeCase.ਧੱਕੋ_ਪਿੱਠ(tolower(ਨਿਵੇਸ਼_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    ਜੇ (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.ਧੱਕੋ_ਪਿੱਠ('_');
    ਜੇ ((check(i, islower) || check(i, ਅੰਕ_ਹੈ)) && check(i + 1, isupper))
      snakeCase.ਧੱਕੋ_ਪਿੱਠ('_');
  }
  ਵਾਪਸ snakeCase;
}

std::string llvm::convert_ਮੰਜ਼ਿਲ_Camel_From_Snake_Case(StringRef ਨਿਵੇਸ਼_2,
                                              ਤਾਰਕਿਕ capitalize_ਪਹਿਲਾ) {
  ਜੇ (ਨਿਵੇਸ਼_2.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ "";

  std::string output;
  output.reserve(ਨਿਵੇਸ਼_2.ਮਾਪ_2());

  // Push the first character, capatilizing if necessary.
  ਜੇ (capitalize_ਪਹਿਲਾ && std::islower(ਨਿਵੇਸ਼_2.front()))
    output.ਧੱਕੋ_ਪਿੱਠ(llvm::ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(ਨਿਵੇਸ਼_2.front()));
  ਨਹੀਂਤਾਂ
    output.ਧੱਕੋ_ਪਿੱਠ(ਨਿਵੇਸ਼_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  ਲਈ (size_t pos = 1, e = ਨਿਵੇਸ਼_2.ਮਾਪ_2(); pos < e; ++pos) {
    ਜੇ (ਨਿਵੇਸ਼_2[pos] == '_' && pos != (e - 1) && std::islower(ਨਿਵੇਸ਼_2[pos + 1]))
      output.ਧੱਕੋ_ਪਿੱਠ(llvm::ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(ਨਿਵੇਸ਼_2[++pos]));
    ਨਹੀਂਤਾਂ
      output.ਧੱਕੋ_ਪਿੱਠ(ਨਿਵੇਸ਼_2[pos]);
  }
  ਵਾਪਸ output;
}
