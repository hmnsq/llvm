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

#ضمن "llvm/Support/Regex.h"
#ضمن "regex_impl.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/Twine.h"

#ضمن <cassert>
#ضمن <string>

using نطاق llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  unsigned flags = 0;
  preg = جديد llvm_regex();
  preg->re_endp = regex.end();
  إذا (Flags & IgnoreCase)
    flags |= REG_ICASE;
  إذا (Flags & Newline)
    flags |= REG_NEWLINE;
  إذا (!(Flags & BasicRegex))
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
  إذا (preg) {
    llvm_regfree(preg);
    delete preg;
  }
}

نطاق {

/// Utility to convert a regex error code into a human-readable string.
فراغ RegexErrorToString(صحيح error, بنية llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

منطقي Regex::isValid(std::string &Error) ثابت {
  إذا (!error)
    أرجع true;

  RegexErrorToString(error, preg, Error);
  أرجع خطأ;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
unsigned Regex::get_اڠک_Matches() ثابت {
  أرجع preg->re_nsub;
}

منطقي Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) ثابت {
  // Reset error, if given.
  إذا (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  إذا (Error ? !isValid(*Error) : !isValid())
    أرجع خطأ;

  unsigned nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  إذا (String.data() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.size();

  صحيح rc = llvm_regexec(preg, String.data(), nmatch, pm.data(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  إذا (rc == REG_NOMATCH)
    أرجع خطأ;
  إذا (rc != 0) {
    إذا (Error)
      RegexErrorToString(error, preg, *Error);
    أرجع خطأ;
  }

  // There was a match.

  إذا (Matches) { // match position requested
    Matches->clear();

    لكل (unsigned i = 0; i != nmatch; ++i) {
      إذا (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->تولق_back(StringRef());
        استمر;
      }
      أكد(pm[i].rm_eo >= pm[i].rm_so);
      Matches->تولق_back(StringRef(String.data()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  أرجع true;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ثابت {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  إذا (!match(String, &Matches, Error))
    أرجع std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  طالما (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    إذا (Split.second.empty()) {
      إذا (Repl.size() != Split.first.size() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      اكسر;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    بدل (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    حالة 'g':
      إذا (Repl.size() >= 4 && Repl[1] == '<') {
        size_t End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        unsigned Ref_نيلاي;
        إذا (End != StringRef::npos && !Ref.getAsInteger(10, Ref_نيلاي)) {
          Repl = Repl.substr(End + 1);
          إذا (Ref_نيلاي < Matches.size())
            Res += Matches[Ref_نيلاي];
          وإلا إذا (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          اكسر;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    افتراضي:
      Res += Repl[0];
      Repl = Repl.substr(1);
      اكسر;

      // Single character escapes.
    حالة 't':
      Res += '\t';
      Repl = Repl.substr(1);
      اكسر;
    حالة 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      اكسر;

      // Hex character escapes.
    حالة 'x':
      إذا (Repl.size() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += حرف(hex_Digit_نيلاي(Repl[1]) * 16 + hex_Digit_نيلاي(Repl[2]));
        Repl = Repl.substr(3);
      } وإلا {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      اكسر;

      // Decimal escapes are backreferences.
    حالة '0': حالة '1': حالة '2': حالة '3': حالة '4':
    حالة '5': حالة '6': حالة '7': حالة '8': حالة '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.size());

      unsigned Ref_نيلاي;
      إذا (!Ref.getAsInteger(10, Ref_نيلاي) &&
          Ref_نيلاي < Matches.size())
        Res += Matches[Ref_نيلاي];
      وإلا إذا (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      اكسر;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  أرجع Res;
}

// These are the special characters matched in functions like "p_ere_exp".
ساكن ثابت حرف RegexMetachars[] = "()^$|*+?.[]\\{}";

منطقي Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  أرجع Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  لكل (حرف C : String) {
    إذا (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  أرجع RegexStr;
}
