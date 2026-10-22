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
  אם (Flags & IgnoreCase)
    flags |= REG_ICASE;
  אם (Flags & Newline)
    flags |= REG_NEWLINE;
  אם (!(Flags & BasicRegex))
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
  אם (preg) {
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
  אם (!error)
    החזר אמת;

  RegexErrorToString(error, preg, Error);
  החזר שקר;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
unsigned Regex::getNumMatches() ثابت {
  החזר preg->re_nsub;
}

منطقي Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) ثابت {
  // Reset error, if given.
  אם (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  אם (Error ? !isValid(*Error) : !isValid())
    החזר שקר;

  unsigned nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  אם (String.data() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.size();

  صحيح rc = llvm_regexec(preg, String.data(), nmatch, pm.data(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  אם (rc == REG_NOMATCH)
    החזר שקר;
  אם (rc != 0) {
    אם (Error)
      RegexErrorToString(error, preg, *Error);
    החזר שקר;
  }

  // There was a match.

  אם (Matches) { // match position requested
    Matches->clear();

    עבור (unsigned i = 0; i != nmatch; ++i) {
      אם (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->push_back(StringRef());
        استمر;
      }
      أكد(pm[i].rm_eo >= pm[i].rm_so);
      Matches->push_back(StringRef(String.data()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  החזר אמת;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ثابت {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  אם (!match(String, &Matches, Error))
    החזר std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  כלעוד (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    אם (Split.second.empty()) {
      אם (Repl.size() != Split.first.size() &&
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
      אם (Repl.size() >= 4 && Repl[1] == '<') {
        size_t End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        unsigned RefValue;
        אם (End != StringRef::npos && !Ref.getAsInteger(10, RefValue)) {
          Repl = Repl.substr(End + 1);
          אם (RefValue < Matches.size())
            Res += Matches[RefValue];
          אחרת אם (Error && Error->empty())
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
      אם (Repl.size() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += حرف(hexDigitValue(Repl[1]) * 16 + hexDigitValue(Repl[2]));
        Repl = Repl.substr(3);
      } אחרת {
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

      unsigned RefValue;
      אם (!Ref.getAsInteger(10, RefValue) &&
          RefValue < Matches.size())
        Res += Matches[RefValue];
      אחרת אם (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      اكسر;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  החזר Res;
}

// These are the special characters matched in functions like "p_ere_exp".
ساكن ثابت حرف RegexMetachars[] = "()^$|*+?.[]\\{}";

منطقي Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  החזר Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  עבור (حرف C : String) {
    אם (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  החזר RegexStr;
}
