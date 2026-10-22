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

#अन्तर्भाव "llvm/Support/Regex.h"
#अन्तर्भाव "regex_impl.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/Twine.h"

#अन्तर्भाव <cassert>
#अन्तर्भाव <string>

using नामाकाश llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  unsigned flags = 0;
  preg = नूतन llvm_regex();
  preg->re_endp = regex.end();
  सचे (Flags & IgnoreCase)
    flags |= REG_ICASE;
  सचे (Flags & Newline)
    flags |= REG_NEWLINE;
  सचे (!(Flags & BasicRegex))
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
  सचे (preg) {
    llvm_regfree(preg);
    delete preg;
  }
}

नामाकाश {

/// Utility to convert a regex error code into a human-readable string.
शून्य RegexErrorToString(पूर्णाङ्क error, संरचना llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

तार्किक Regex::isValid(std::string &Error) ध्रुव {
  सचे (!error)
    पटिनिवत्त सच्च;

  RegexErrorToString(error, preg, Error);
  पटिनिवत्त मिच्छा;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
unsigned Regex::get_अङ्कः_Matches() ध्रुव {
  पटिनिवत्त preg->re_nsub;
}

तार्किक Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) ध्रुव {
  // Reset error, if given.
  सचे (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  सचे (Error ? !isValid(*Error) : !isValid())
    पटिनिवत्त मिच्छा;

  unsigned nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  सचे (String.data() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.size();

  पूर्णाङ्क rc = llvm_regexec(preg, String.data(), nmatch, pm.data(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  सचे (rc == REG_NOMATCH)
    पटिनिवत्त मिच्छा;
  सचे (rc != 0) {
    सचे (Error)
      RegexErrorToString(error, preg, *Error);
    पटिनिवत्त मिच्छा;
  }

  // There was a match.

  सचे (Matches) { // match position requested
    Matches->clear();

    अत्थाय (unsigned i = 0; i != nmatch; ++i) {
      सचे (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->प्रक्षिप_back(StringRef());
        अनुवर्त;
      }
      प्रतिज्ञा(pm[i].rm_eo >= pm[i].rm_so);
      Matches->प्रक्षिप_back(StringRef(String.data()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  पटिनिवत्त सच्च;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ध्रुव {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  सचे (!match(String, &Matches, Error))
    पटिनिवत्त std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  याव (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    सचे (Split.second.empty()) {
      सचे (Repl.size() != Split.first.size() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      भङ्ग;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    विकल्प (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    पक्ष 'g':
      सचे (Repl.size() >= 4 && Repl[1] == '<') {
        size_t End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        unsigned Ref_मानम्;
        सचे (End != StringRef::npos && !Ref.getAsInteger(10, Ref_मानम्)) {
          Repl = Repl.substr(End + 1);
          सचे (Ref_मानम् < Matches.size())
            Res += Matches[Ref_मानम्];
          अञ्ञथा सचे (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          भङ्ग;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    स्वभाव:
      Res += Repl[0];
      Repl = Repl.substr(1);
      भङ्ग;

      // Single character escapes.
    पक्ष 't':
      Res += '\t';
      Repl = Repl.substr(1);
      भङ्ग;
    पक्ष 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      भङ्ग;

      // Hex character escapes.
    पक्ष 'x':
      सचे (Repl.size() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += वर्ण(hex_Digit_मानम्(Repl[1]) * 16 + hex_Digit_मानम्(Repl[2]));
        Repl = Repl.substr(3);
      } अञ्ञथा {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      भङ्ग;

      // Decimal escapes are backreferences.
    पक्ष '0': पक्ष '1': पक्ष '2': पक्ष '3': पक्ष '4':
    पक्ष '5': पक्ष '6': पक्ष '7': पक्ष '8': पक्ष '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.size());

      unsigned Ref_मानम्;
      सचे (!Ref.getAsInteger(10, Ref_मानम्) &&
          Ref_मानम् < Matches.size())
        Res += Matches[Ref_मानम्];
      अञ्ञथा सचे (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      भङ्ग;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  पटिनिवत्त Res;
}

// These are the special characters matched in functions like "p_ere_exp".
स्थिर ध्रुव वर्ण RegexMetachars[] = "()^$|*+?.[]\\{}";

तार्किक Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  पटिनिवत्त Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  अत्थाय (वर्ण C : String) {
    सचे (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  पटिनिवत्त RegexStr;
}
