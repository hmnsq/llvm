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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cctype>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<StringRef, StringRef> llvm::getToken(StringRef ಮೂಲ,
                                               StringRef Delimiters) {
  // Figure out where the token starts.
  StringRef::ಗಾತ್ರ_type ಆರಂಭ = ಮೂಲ.ಕಂಡುಹಿಡಿ_ಮೊದಲ_not_ದ(Delimiters);

  // Find the next occurrence of the delimiter.
  StringRef::ಗಾತ್ರ_type ಕೊನೆ_2 = ಮೂಲ.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(Delimiters, ಆರಂಭ);

  ಹಿಂತಿರುಗಿ {ಮೂಲ.ತುಂಡು(ಆರಂಭ, ಕೊನೆ_2), ಮೂಲ.substr(ಕೊನೆ_2)};
}

/// SplitString - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
ಶೂನ್ಯ llvm::ವಿಭಜಿಸು_String(StringRef ಮೂಲ,
                       Small_ಸದಿಶ_Impl<StringRef> &ಹೊರಹೋಗು_Fragments,
                       StringRef Delimiters) {
  std::pair<StringRef, StringRef> S = getToken(ಮೂಲ, Delimiters);
  ಆಗ (!S.ಮೊದಲ_2.ಖಾಲಿಗಳು()) {
    ಹೊರಹೋಗು_Fragments.ತಳ್ಳು_ಹಿಂಭಾಗ(S.ಮೊದಲ_2);
    S = getToken(S.ಎರಡನೇ_2, Delimiters);
  }
}

ಶೂನ್ಯ llvm::printEscapedString(StringRef Name, raw_ostream &ಹೊರಹೋಗು) {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ C : Name) {
    ಆದರೆ (C == '\\')
      ಹೊರಹೋಗು << '\\' << C;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isPrint(C) && C != '"')
      ಹೊರಹೋಗು << C;
    ಇಲ್ಲದಿದ್ದರೆ
      ಹೊರಹೋಗು << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

ಶೂನ್ಯ llvm::printHTMLEscaped(StringRef String, raw_ostream &ಹೊರಹೋಗು) {
  ಗಾಗಿ (ಅಕ್ಷರ C : String) {
    ಆದರೆ (C == '&')
      ಹೊರಹೋಗು << "&amp;";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (C == '<')
      ಹೊರಹೋಗು << "&lt;";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (C == '>')
      ಹೊರಹೋಗು << "&gt;";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (C == '\"')
      ಹೊರಹೋಗು << "&quot;";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (C == '\'')
      ಹೊರಹೋಗು << "&apos;";
    ಇಲ್ಲದಿದ್ದರೆ
      ಹೊರಹೋಗು << C;
  }
}

ಶೂನ್ಯ llvm::print_ಅಧೋ_Case(StringRef String, raw_ostream &ಹೊರಹೋಗು) {
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ C : String)
    ಹೊರಹೋಗು << ಗಮ್ಯ_ಅಧೋ(C);
}

ಶೂನ್ಯ llvm::printPercentEncoded(StringRef String, raw_ostream &ಹೊರಹೋಗು) {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ C : String) {
    ಆದರೆ (isAlnum(C) || StringRef("-_.~").ಒಳಗೊಂಡಿದೆ(C))
      ಹೊರಹೋಗು << C;
    ಇಲ್ಲದಿದ್ದರೆ
      ಹೊರಹೋಗು << '%' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
}

std::string llvm::convert_ಗಮ್ಯ_Snake_From_Camel_Case(StringRef ಒಳಹರಿವು_2) {
  ಆದರೆ (ಒಳಹರಿವು_2.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ "";

  std::string snakeCase;
  snakeCase.reserve(ಒಳಹರಿವು_2.ಗಾತ್ರ_3());
  ಸ್ವಯಂಚಾಲಿತ check = [&ಒಳಹರಿವು_2](size_t j, function_ref<ತಾರ್ಕಿಕ(ಪೂರ್ಣಾಂಕ)> predicate) {
    ಹಿಂತಿರುಗಿ j < ಒಳಹರಿವು_2.ಗಾತ್ರ_3() && predicate(ಒಳಹರಿವು_2[j]);
  };
  ಗಾಗಿ (size_t i = 0; i < ಒಳಹರಿವು_2.ಗಾತ್ರ_3(); ++i) {
    snakeCase.ತಳ್ಳು_ಹಿಂಭಾಗ(tolower(ಒಳಹರಿವು_2[i]));
    // Handles "runs" of capitals, such as in OPName -> op_name.
    ಆದರೆ (check(i, isupper) && check(i + 1, isupper) && check(i + 2, islower))
      snakeCase.ತಳ್ಳು_ಹಿಂಭಾಗ('_');
    ಆದರೆ ((check(i, islower) || check(i, ಅಂಕಿಯೇ)) && check(i + 1, isupper))
      snakeCase.ತಳ್ಳು_ಹಿಂಭಾಗ('_');
  }
  ಹಿಂತಿರುಗಿ snakeCase;
}

std::string llvm::convert_ಗಮ್ಯ_Camel_From_Snake_Case(StringRef ಒಳಹರಿವು_2,
                                              ತಾರ್ಕಿಕ capitalize_ಮೊದಲ) {
  ಆದರೆ (ಒಳಹರಿವು_2.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ "";

  std::string output;
  output.reserve(ಒಳಹರಿವು_2.ಗಾತ್ರ_3());

  // Push the first character, capatilizing if necessary.
  ಆದರೆ (capitalize_ಮೊದಲ && std::islower(ಒಳಹರಿವು_2.front()))
    output.ತಳ್ಳು_ಹಿಂಭಾಗ(llvm::ಗಮ್ಯ_ಊರ್ಧ್ವ(ಒಳಹರಿವು_2.front()));
  ಇಲ್ಲದಿದ್ದರೆ
    output.ತಳ್ಳು_ಹಿಂಭಾಗ(ಒಳಹರಿವು_2.front());

  // Walk the input converting any `*_[a-z]` snake case into `*[A-Z]` camelCase.
  ಗಾಗಿ (size_t pos = 1, e = ಒಳಹರಿವು_2.ಗಾತ್ರ_3(); pos < e; ++pos) {
    ಆದರೆ (ಒಳಹರಿವು_2[pos] == '_' && pos != (e - 1) && std::islower(ಒಳಹರಿವು_2[pos + 1]))
      output.ತಳ್ಳು_ಹಿಂಭಾಗ(llvm::ಗಮ್ಯ_ಊರ್ಧ್ವ(ಒಳಹರಿವು_2[++pos]));
    ಇಲ್ಲದಿದ್ದರೆ
      output.ತಳ್ಳು_ಹಿಂಭಾಗ(ಒಳಹರಿವು_2[pos]);
  }
  ಹಿಂತಿರುಗಿ output;
}
