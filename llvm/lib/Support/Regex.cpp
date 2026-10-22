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

using ನಾಮಾಕಾಶ llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  unsigned flags = 0;
  preg = ನೂತನ llvm_regex();
  preg->re_endp = regex.end();
  ಯದಿ (Flags & IgnoreCase)
    flags |= REG_ICASE;
  ಯದಿ (Flags & Newline)
    flags |= REG_NEWLINE;
  ಯದಿ (!(Flags & BasicRegex))
    flags |= REG_EXTENDED;
  error = llvm_regcomp(preg, regex.data(), flags|REG_PEND);
}

Regex::Regex(StringRef regex, unsigned Flags)
    : Regex(regex, static_cast<RegexFlags>(Flags)) {}

Regex::Regex(Regex &&regex) {
  preg = regex.preg;
  error = regex.error;
  regex.preg = nullptr;
  regex.error = REG_BADPAT;
}

Regex::~Regex() {
  ಯದಿ (preg) {
    llvm_regfree(preg);
    delete preg;
  }
}

ನಾಮಾಕಾಶ {

/// Utility to convert a regex error code into a human-readable string.
ಶೂನ್ಯ RegexErrorToString(ಪೂರ್ಣಾಙ್ಕ error, ಸಂರಚನಾ llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

ತಾರ್ಕಿಕ Regex::isValid(std::string &Error) ಧ್ರುವ {
  ಯದಿ (!error)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  RegexErrorToString(error, preg, Error);
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
unsigned Regex::get_ಅಂಕೆ_Matches() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ preg->re_nsub;
}

ತಾರ್ಕಿಕ Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) ಧ್ರುವ {
  // Reset error, if given.
  ಯದಿ (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  ಯದಿ (Error ? !isValid(*Error) : !isValid())
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  unsigned nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  ಯದಿ (String.data() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.size();

  ಪೂರ್ಣಾಙ್ಕ rc = llvm_regexec(preg, String.data(), nmatch, pm.data(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  ಯದಿ (rc == REG_NOMATCH)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (rc != 0) {
    ಯದಿ (Error)
      RegexErrorToString(error, preg, *Error);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  // There was a match.

  ಯದಿ (Matches) { // match position requested
    Matches->clear();

    ಪ್ರತಿ (unsigned i = 0; i != nmatch; ++i) {
      ಯದಿ (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->ತಳ್ಳು_back(StringRef());
        ಅನುವರ್ತ;
      }
      ಪ್ರತಿಜ್ಞಾ(pm[i].rm_eo >= pm[i].rm_so);
      Matches->ತಳ್ಳು_back(StringRef(String.data()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ಧ್ರುವ {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  ಯದಿ (!match(String, &Matches, Error))
    ಪ್ರತ್ಯಾಗಮ std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  ಯಾವತ್ (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    ಯದಿ (Split.second.empty()) {
      ಯದಿ (Repl.size() != Split.first.size() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      ಭಙ್ಗ;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    ವಿಕಲ್ಪ (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    ಪಕ್ಷ 'g':
      ಯದಿ (Repl.size() >= 4 && Repl[1] == '<') {
        size_t End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        unsigned Ref_ಮೌಲ್ಯ;
        ಯದಿ (End != StringRef::npos && !Ref.getAsInteger(10, Ref_ಮೌಲ್ಯ)) {
          Repl = Repl.substr(End + 1);
          ಯದಿ (Ref_ಮೌಲ್ಯ < Matches.size())
            Res += Matches[Ref_ಮೌಲ್ಯ];
          ಅನ್ಯಥಾ ಯದಿ (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          ಭಙ್ಗ;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    ಸ್ವಭಾವ:
      Res += Repl[0];
      Repl = Repl.substr(1);
      ಭಙ್ಗ;

      // Single character escapes.
    ಪಕ್ಷ 't':
      Res += '\t';
      Repl = Repl.substr(1);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      ಭಙ್ಗ;

      // Hex character escapes.
    ಪಕ್ಷ 'x':
      ಯದಿ (Repl.size() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += ವರ್ಣ(hex_Digit_ಮೌಲ್ಯ(Repl[1]) * 16 + hex_Digit_ಮೌಲ್ಯ(Repl[2]));
        Repl = Repl.substr(3);
      } ಅನ್ಯಥಾ {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      ಭಙ್ಗ;

      // Decimal escapes are backreferences.
    ಪಕ್ಷ '0': ಪಕ್ಷ '1': ಪಕ್ಷ '2': ಪಕ್ಷ '3': ಪಕ್ಷ '4':
    ಪಕ್ಷ '5': ಪಕ್ಷ '6': ಪಕ್ಷ '7': ಪಕ್ಷ '8': ಪಕ್ಷ '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.size());

      unsigned Ref_ಮೌಲ್ಯ;
      ಯದಿ (!Ref.getAsInteger(10, Ref_ಮೌಲ್ಯ) &&
          Ref_ಮೌಲ್ಯ < Matches.size())
        Res += Matches[Ref_ಮೌಲ್ಯ];
      ಅನ್ಯಥಾ ಯದಿ (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      ಭಙ್ಗ;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  ಪ್ರತ್ಯಾಗಮ Res;
}

// These are the special characters matched in functions like "p_ere_exp".
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ RegexMetachars[] = "()^$|*+?.[]\\{}";

ತಾರ್ಕಿಕ Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  ಪ್ರತ್ಯಾಗಮ Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  ಪ್ರತಿ (ವರ್ಣ C : String) {
    ಯದಿ (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  ಪ್ರತ್ಯಾಗಮ RegexStr;
}
