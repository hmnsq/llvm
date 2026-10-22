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

#অন্তর্ভাব "llvm/Support/Regex.h"
#অন্তর্ভাব "regex_impl.h"
#অন্তর্ভাব "llvm/ADT/SmallVector.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/ADT/Twine.h"

#অন্তর্ভাব <cassert>
#অন্তর্ভাব <string>

using নামাকাশ llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  unsigned flags = 0;
  preg = নতুন llvm_regex();
  preg->re_endp = regex.end();
  যদি (Flags & IgnoreCase)
    flags |= REG_ICASE;
  যদি (Flags & Newline)
    flags |= REG_NEWLINE;
  যদি (!(Flags & BasicRegex))
    flags |= REG_EXTENDED;
  error = llvm_regcomp(preg, regex.তথ্য(), flags|REG_PEND);
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
শূন্য RegexErrorToString(পূর্ণসংখ্যা error, কাঠামো llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

যৌক্তিক Regex::isValid(std::string &Error) ধ্রুবক {
  যদি (!error)
    ফেরত সত্য;

  RegexErrorToString(error, preg, Error);
  ফেরত মিথ্যা;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
unsigned Regex::get_অঙ্ক_Matches() ধ্রুবক {
  ফেরত preg->re_nsub;
}

যৌক্তিক Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) ধ্রুবক {
  // Reset error, if given.
  যদি (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  যদি (Error ? !isValid(*Error) : !isValid())
    ফেরত মিথ্যা;

  unsigned nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  যদি (String.তথ্য() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.আকার_3();

  পূর্ণসংখ্যা rc = llvm_regexec(preg, String.তথ্য(), nmatch, pm.তথ্য(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  যদি (rc == REG_NOMATCH)
    ফেরত মিথ্যা;
  যদি (rc != 0) {
    যদি (Error)
      RegexErrorToString(error, preg, *Error);
    ফেরত মিথ্যা;
  }

  // There was a match.

  যদি (Matches) { // match position requested
    Matches->clear();

    জন্য (unsigned i = 0; i != nmatch; ++i) {
      যদি (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->ঠেলো_back(StringRef());
        চালু;
      }
      প্রতিজ্ঞা(pm[i].rm_eo >= pm[i].rm_so);
      Matches->ঠেলো_back(StringRef(String.তথ্য()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  ফেরত সত্য;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ধ্রুবক {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  যদি (!match(String, &Matches, Error))
    ফেরত std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  যখন (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    যদি (Split.second.empty()) {
      যদি (Repl.আকার_3() != Split.first.আকার_3() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      বিরতি;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    নির্বাচন (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    ক্ষেত্র 'g':
      যদি (Repl.আকার_3() >= 4 && Repl[1] == '<') {
        size_t End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        unsigned Ref_মান;
        যদি (End != StringRef::npos && !Ref.getAsInteger(10, Ref_মান)) {
          Repl = Repl.substr(End + 1);
          যদি (Ref_মান < Matches.আকার_3())
            Res += Matches[Ref_মান];
          অন্যথা যদি (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          বিরতি;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    পূর্বনির্ধারিত:
      Res += Repl[0];
      Repl = Repl.substr(1);
      বিরতি;

      // Single character escapes.
    ক্ষেত্র 't':
      Res += '\t';
      Repl = Repl.substr(1);
      বিরতি;
    ক্ষেত্র 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      বিরতি;

      // Hex character escapes.
    ক্ষেত্র 'x':
      যদি (Repl.আকার_3() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += অক্ষর(hex_Digit_মান(Repl[1]) * 16 + hex_Digit_মান(Repl[2]));
        Repl = Repl.substr(3);
      } অন্যথা {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      বিরতি;

      // Decimal escapes are backreferences.
    ক্ষেত্র '0': ক্ষেত্র '1': ক্ষেত্র '2': ক্ষেত্র '3': ক্ষেত্র '4':
    ক্ষেত্র '5': ক্ষেত্র '6': ক্ষেত্র '7': ক্ষেত্র '8': ক্ষেত্র '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.আকার_3());

      unsigned Ref_মান;
      যদি (!Ref.getAsInteger(10, Ref_মান) &&
          Ref_মান < Matches.আকার_3())
        Res += Matches[Ref_মান];
      অন্যথা যদি (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      বিরতি;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  ফেরত Res;
}

// These are the special characters matched in functions like "p_ere_exp".
স্থির ধ্রুবক অক্ষর RegexMetachars[] = "()^$|*+?.[]\\{}";

যৌক্তিক Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  ফেরত Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  জন্য (অক্ষর C : String) {
    যদি (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  ফেরত RegexStr;
}
