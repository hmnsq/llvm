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

#অন্তর্ভাৱ "llvm/Support/Regex.h"
#অন্তর্ভাৱ "regex_impl.h"
#অন্তর্ভাৱ "llvm/ADT/SmallVector.h"
#অন্তর্ভাৱ "llvm/ADT/StringExtras.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/ADT/Twine.h"

#অন্তর্ভাৱ <cassert>
#অন্তর্ভাৱ <string>

using নামাকাশ llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  unsigned flags = 0;
  preg = নূতন llvm_regex();
  preg->re_endp = regex.end();
  যদি (Flags & IgnoreCase)
    flags |= REG_ICASE;
  যদি (Flags & Newline)
    flags |= REG_NEWLINE;
  যদি (!(Flags & BasicRegex))
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
  যদি (preg) {
    llvm_regfree(preg);
    delete preg;
  }
}

নামাকাশ {

/// Utility to convert a regex error code into a human-readable string.
শূন্য RegexErrorToString(পূর্ণাঙ্ক error, সংরচনা llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

তার্কিক Regex::isValid(std::string &Error) ধ্রুৱ {
  যদি (!error)
    ঘূৰাই সঁচা;

  RegexErrorToString(error, preg, Error);
  ঘূৰাই মিছা;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
unsigned Regex::get_অঙ্ক_Matches() ধ্রুৱ {
  ঘূৰাই preg->re_nsub;
}

তার্কিক Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) ধ্রুৱ {
  // Reset error, if given.
  যদি (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  যদি (Error ? !isValid(*Error) : !isValid())
    ঘূৰাই মিছা;

  unsigned nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  যদি (String.data() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.size();

  পূর্ণাঙ্ক rc = llvm_regexec(preg, String.data(), nmatch, pm.data(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  যদি (rc == REG_NOMATCH)
    ঘূৰাই মিছা;
  যদি (rc != 0) {
    যদি (Error)
      RegexErrorToString(error, preg, *Error);
    ঘূৰাই মিছা;
  }

  // There was a match.

  যদি (Matches) { // match position requested
    Matches->clear();

    বাবে (unsigned i = 0; i != nmatch; ++i) {
      যদি (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->ঠেলো_back(StringRef());
        অনুৱর্ত;
      }
      প্রতিজ্ঞা(pm[i].rm_eo >= pm[i].rm_so);
      Matches->ঠেলো_back(StringRef(String.data()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  ঘূৰাই সঁচা;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ধ্রুৱ {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  যদি (!match(String, &Matches, Error))
    ঘূৰাই std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  যেতিয়ালৈ (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    যদি (Split.second.empty()) {
      যদি (Repl.size() != Split.first.size() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      ভঙ্গ;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    ৱিকল্প (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    পক্ষ 'g':
      যদি (Repl.size() >= 4 && Repl[1] == '<') {
        size_t End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        unsigned Ref_মান;
        যদি (End != StringRef::npos && !Ref.getAsInteger(10, Ref_মান)) {
          Repl = Repl.substr(End + 1);
          যদি (Ref_মান < Matches.size())
            Res += Matches[Ref_মান];
          অন্যথা যদি (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          ভঙ্গ;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    স্ৱভাৱ:
      Res += Repl[0];
      Repl = Repl.substr(1);
      ভঙ্গ;

      // Single character escapes.
    পক্ষ 't':
      Res += '\t';
      Repl = Repl.substr(1);
      ভঙ্গ;
    পক্ষ 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      ভঙ্গ;

      // Hex character escapes.
    পক্ষ 'x':
      যদি (Repl.size() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += ৱর্ণ(hex_Digit_মান(Repl[1]) * 16 + hex_Digit_মান(Repl[2]));
        Repl = Repl.substr(3);
      } অন্যথা {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      ভঙ্গ;

      // Decimal escapes are backreferences.
    পক্ষ '0': পক্ষ '1': পক্ষ '2': পক্ষ '3': পক্ষ '4':
    পক্ষ '5': পক্ষ '6': পক্ষ '7': পক্ষ '8': পক্ষ '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.size());

      unsigned Ref_মান;
      যদি (!Ref.getAsInteger(10, Ref_মান) &&
          Ref_মান < Matches.size())
        Res += Matches[Ref_মান];
      অন্যথা যদি (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      ভঙ্গ;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  ঘূৰাই Res;
}

// These are the special characters matched in functions like "p_ere_exp".
স্থির ধ্রুৱ ৱর্ণ RegexMetachars[] = "()^$|*+?.[]\\{}";

তার্কিক Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  ঘূৰাই Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  বাবে (ৱর্ণ C : String) {
    যদি (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  ঘূৰাই RegexStr;
}
