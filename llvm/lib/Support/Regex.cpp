//===-- Regex.cpp - Regular Expression matcher implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a POSIX regular expression matcher.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/Regex.h"
#ಅನ್ತರ್ಭಾವ "regex_impl.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"

#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <string>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

Regex::Regex() : preg(ಶೂನ್ಯಸೂಚಕ), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  ಚಿಹ್ನೆರಹಿತ flags = 0;
  preg = ಹೊಸ llvm_regex();
  preg->re_endp = regex.ಕೊನೆ();
  ಆದರೆ (Flags & IgnoreCase)
    flags |= REG_ICASE;
  ಆದರೆ (Flags & Newline)
    flags |= REG_NEWLINE;
  ಆದರೆ (!(Flags & BasicRegex))
    flags |= REG_EXTENDED;
  error = llvm_regcomp(preg, regex.ದತ್ತಾಂಶ(), flags|REG_PEND);
}

Regex::Regex(StringRef regex, ಚಿಹ್ನೆರಹಿತ Flags)
    : Regex(regex, static_cast<RegexFlags>(Flags)) {}

Regex::Regex(Regex &&regex) {
  preg = regex.preg;
  error = regex.error;
  regex.preg = ಶೂನ್ಯಸೂಚಕ;
  regex.error = REG_BADPAT;
}

Regex::~Regex() {
  ಆದರೆ (preg) {
    llvm_regfree(preg);
    ಅಳಿಸು preg;
  }
}

ನಾಮಾಕಾಶ {

/// Utility to convert a regex error code into a human-readable string.
ಶೂನ್ಯ Regex_Error_ಗಮ್ಯ_String(ಪೂರ್ಣಾಂಕ error, ರಚನೆ llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, ಶೂನ್ಯಸೂಚಕ, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

ತಾರ್ಕಿಕ Regex::isValid(std::string &Error) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (!error)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  Regex_Error_ಗಮ್ಯ_String(error, preg, Error);
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
ಚಿಹ್ನೆರಹಿತ Regex::get_ಅಂಕೆ_Matches() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ preg->re_nsub;
}

ತಾರ್ಕಿಕ Regex::ಹೊಂದಿಕೆ(StringRef String, Small_ಸದಿಶ_Impl<StringRef> *Matches,
                  std::string *Error) ಸ್ಥಿರಾಂಕ {
  // Reset error, if given.
  ಆದರೆ (Error && !Error->ಖಾಲಿಗಳು())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  ಆದರೆ (Error ? !isValid(*Error) : !isValid())
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಚಿಹ್ನೆರಹಿತ nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  ಆದರೆ (String.ದತ್ತಾಂಶ() == ಶೂನ್ಯಸೂಚಕ)
    String = "";

  // pmatch needs to have at least one element.
  Small_ಸದಿಶ<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.ಗಾತ್ರ_3();

  ಪೂರ್ಣಾಂಕ rc = llvm_regexec(preg, String.ದತ್ತಾಂಶ(), nmatch, pm.ದತ್ತಾಂಶ(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  ಆದರೆ (rc == REG_NOMATCH)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (rc != 0) {
    ಆದರೆ (Error)
      Regex_Error_ಗಮ್ಯ_String(error, preg, *Error);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  // There was a match.

  ಆದರೆ (Matches) { // match position requested
    Matches->clear();

    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != nmatch; ++i) {
      ಆದರೆ (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->ತಳ್ಳು_ಹಿಂಭಾಗ(StringRef());
        ಮುಂದುವರಿ;
      }
      ಪ್ರತಿಜ್ಞಾ(pm[i].rm_eo >= pm[i].rm_so);
      Matches->ತಳ್ಳು_ಹಿಂಭಾಗ(StringRef(String.ದತ್ತಾಂಶ()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ಸ್ಥಿರಾಂಕ {
  Small_ಸದಿಶ<StringRef, 8> Matches;

  // Return the input if there was no match.
  ಆದರೆ (!ಹೊಂದಿಕೆ(String, &Matches, Error))
    ಹಿಂತಿರುಗಿ std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  ಆಗ (!Repl.ಖಾಲಿಗಳು()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> ವಿಭಜಿಸು_2 = Repl.ವಿಭಜಿಸು('\\');

    // Add the skipped substring.
    Res += ವಿಭಜಿಸು_2.ಮೊದಲ_2;

    // Check for termination and trailing backslash.
    ಆದರೆ (ವಿಭಜಿಸು_2.ಎರಡನೇ_2.ಖಾಲಿಗಳು()) {
      ಆದರೆ (Repl.ಗಾತ್ರ_3() != ವಿಭಜಿಸು_2.ಮೊದಲ_2.ಗಾತ್ರ_3() &&
          Error && Error->ಖಾಲಿಗಳು())
        *Error = "replacement string contained trailing backslash";
      ನಿಲ್ಲಿಸು;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = ವಿಭಜಿಸು_2.ಎರಡನೇ_2;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    ಆಯ್ಕೆ (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    ಸ್ಥಿತಿ 'g':
      ಆದರೆ (Repl.ಗಾತ್ರ_3() >= 4 && Repl[1] == '<') {
        size_t ಕೊನೆ_2 = Repl.ಕಂಡುಹಿಡಿ('>');
        StringRef Ref = Repl.ತುಂಡು(2, ಕೊನೆ_2);
        ಚಿಹ್ನೆರಹಿತ Ref_ಮೌಲ್ಯ;
        ಆದರೆ (ಕೊನೆ_2 != StringRef::npos && !Ref.getAsInteger(10, Ref_ಮೌಲ್ಯ)) {
          Repl = Repl.substr(ಕೊನೆ_2 + 1);
          ಆದರೆ (Ref_ಮೌಲ್ಯ < Matches.ಗಾತ್ರ_3())
            Res += Matches[Ref_ಮೌಲ್ಯ];
          ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Error && Error->ಖಾಲಿಗಳು())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          ನಿಲ್ಲಿಸು;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    ಪೂರ್ವನಿಯೋಜಿತ:
      Res += Repl[0];
      Repl = Repl.substr(1);
      ನಿಲ್ಲಿಸು;

      // Single character escapes.
    ಸ್ಥಿತಿ 't':
      Res += '\t';
      Repl = Repl.substr(1);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      ನಿಲ್ಲಿಸು;

      // Hex character escapes.
    ಸ್ಥಿತಿ 'x':
      ಆದರೆ (Repl.ಗಾತ್ರ_3() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += ಅಕ್ಷರ(hex_Digit_ಮೌಲ್ಯ(Repl[1]) * 16 + hex_Digit_ಮೌಲ್ಯ(Repl[2]));
        Repl = Repl.substr(3);
      } ಇಲ್ಲದಿದ್ದರೆ {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      ನಿಲ್ಲಿಸು;

      // Decimal escapes are backreferences.
    ಸ್ಥಿತಿ '0': ಸ್ಥಿತಿ '1': ಸ್ಥಿತಿ '2': ಸ್ಥಿತಿ '3': ಸ್ಥಿತಿ '4':
    ಸ್ಥಿತಿ '5': ಸ್ಥಿತಿ '6': ಸ್ಥಿತಿ '7': ಸ್ಥಿತಿ '8': ಸ್ಥಿತಿ '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.ತುಂಡು(0, Repl.ಕಂಡುಹಿಡಿ_ಮೊದಲ_not_ದ("0123456789"));
      Repl = Repl.substr(Ref.ಗಾತ್ರ_3());

      ಚಿಹ್ನೆರಹಿತ Ref_ಮೌಲ್ಯ;
      ಆದರೆ (!Ref.getAsInteger(10, Ref_ಮೌಲ್ಯ) &&
          Ref_ಮೌಲ್ಯ < Matches.ಗಾತ್ರ_3())
        Res += Matches[Ref_ಮೌಲ್ಯ];
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Error && Error->ಖಾಲಿಗಳು())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      ನಿಲ್ಲಿಸು;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].ಕೊನೆ(), String.ಕೊನೆ() - Matches[0].ಕೊನೆ());

  ಹಿಂತಿರುಗಿ Res;
}

// These are the special characters matched in functions like "p_ere_exp".
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ RegexMetachars[] = "()^$|*+?.[]\\{}";

ತಾರ್ಕಿಕ Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  ಹಿಂತಿರುಗಿ Str.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  ಗಾಗಿ (ಅಕ್ಷರ C : String) {
    ಆದರೆ (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  ಹಿಂತಿರುಗಿ RegexStr;
}
