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

#شامل "llvm/Support/Regex.h"
#شامل "regex_impl.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/Twine.h"

#شامل <cassert>
#شامل <string>

using فضاینام llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  unsigned flags = 0;
  preg = جدید llvm_regex();
  preg->re_endp = regex.end();
  اگر (Flags & IgnoreCase)
    flags |= REG_ICASE;
  اگر (Flags & Newline)
    flags |= REG_NEWLINE;
  اگر (!(Flags & BasicRegex))
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
  اگر (preg) {
    llvm_regfree(preg);
    delete preg;
  }
}

فضاینام {

/// Utility to convert a regex error code into a human-readable string.
تهی RegexErrorToString(صحیح error, ساختار llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

منطقی Regex::isValid(std::string &Error) ثابت {
  اگر (!error)
    بازگشت درست;

  RegexErrorToString(error, preg, Error);
  بازگشت نادرست;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
unsigned Regex::get_رقم_Matches() ثابت {
  بازگشت preg->re_nsub;
}

منطقی Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) ثابت {
  // Reset error, if given.
  اگر (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  اگر (Error ? !isValid(*Error) : !isValid())
    بازگشت نادرست;

  unsigned nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  اگر (String.data() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.size();

  صحیح rc = llvm_regexec(preg, String.data(), nmatch, pm.data(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  اگر (rc == REG_NOMATCH)
    بازگشت نادرست;
  اگر (rc != 0) {
    اگر (Error)
      RegexErrorToString(error, preg, *Error);
    بازگشت نادرست;
  }

  // There was a match.

  اگر (Matches) { // match position requested
    Matches->clear();

    برای (unsigned i = 0; i != nmatch; ++i) {
      اگر (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->افزودن_back(StringRef());
        ادامه;
      }
      أكد(pm[i].rm_eo >= pm[i].rm_so);
      Matches->افزودن_back(StringRef(String.data()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  بازگشت درست;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ثابت {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  اگر (!match(String, &Matches, Error))
    بازگشت std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  وقتی (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    اگر (Split.second.empty()) {
      اگر (Repl.size() != Split.first.size() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      بشکن;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    انتخاب (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    حالت 'g':
      اگر (Repl.size() >= 4 && Repl[1] == '<') {
        size_t End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        unsigned Ref_مقدار;
        اگر (End != StringRef::npos && !Ref.getAsInteger(10, Ref_مقدار)) {
          Repl = Repl.substr(End + 1);
          اگر (Ref_مقدار < Matches.size())
            Res += Matches[Ref_مقدار];
          وگرنه اگر (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          بشکن;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    پیشفرض:
      Res += Repl[0];
      Repl = Repl.substr(1);
      بشکن;

      // Single character escapes.
    حالت 't':
      Res += '\t';
      Repl = Repl.substr(1);
      بشکن;
    حالت 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      بشکن;

      // Hex character escapes.
    حالت 'x':
      اگر (Repl.size() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += نویسه(hex_Digit_مقدار(Repl[1]) * 16 + hex_Digit_مقدار(Repl[2]));
        Repl = Repl.substr(3);
      } وگرنه {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      بشکن;

      // Decimal escapes are backreferences.
    حالت '0': حالت '1': حالت '2': حالت '3': حالت '4':
    حالت '5': حالت '6': حالت '7': حالت '8': حالت '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.size());

      unsigned Ref_مقدار;
      اگر (!Ref.getAsInteger(10, Ref_مقدار) &&
          Ref_مقدار < Matches.size())
        Res += Matches[Ref_مقدار];
      وگرنه اگر (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      بشکن;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  بازگشت Res;
}

// These are the special characters matched in functions like "p_ere_exp".
ایستا ثابت نویسه RegexMetachars[] = "()^$|*+?.[]\\{}";

منطقی Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  بازگشت Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  برای (نویسه C : String) {
    اگر (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  بازگشت RegexStr;
}
