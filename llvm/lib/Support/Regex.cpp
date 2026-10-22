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

ورتوں نطاق llvm;

Regex::Regex() : preg(خالی_سوچک), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  بنانشانی flags = 0;
  preg = جديد llvm_regex();
  preg->re_endp = regex.سرا();
  إذا (Flags & IgnoreCase)
    flags |= REG_ICASE;
  إذا (Flags & Newline)
    flags |= REG_NEWLINE;
  إذا (!(Flags & BasicRegex))
    flags |= REG_EXTENDED;
  error = llvm_regcomp(preg, regex.انکڑے(), flags|REG_PEND);
}

Regex::Regex(StringRef regex, بنانشانی Flags)
    : Regex(regex, static_cast<RegexFlags>(Flags)) {}

Regex::Regex(Regex &&regex) {
  preg = regex.preg;
  error = regex.error;
  regex.preg = خالی_سوچک;
  regex.error = REG_BADPAT;
}

Regex::~Regex() {
  إذا (preg) {
    llvm_regfree(preg);
    مٹاؤ preg;
  }
}

نطاق {

/// Utility to convert a regex error code into a human-readable string.
فراغ Regex_Error_منزل_String(صحيح error, بنية llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, خالی_سوچک, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

منطقي Regex::isValid(std::string &Error) ثابت {
  إذا (!error)
    أرجع سچ;

  Regex_Error_منزل_String(error, preg, Error);
  أرجع خطأ;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
بنانشانی Regex::get_عدد_Matches() ثابت {
  أرجع preg->re_nsub;
}

منطقي Regex::میل(StringRef String, Small_سمتیہ_Impl<StringRef> *Matches,
                  std::string *Error) ثابت {
  // Reset error, if given.
  إذا (Error && !Error->خالی_تھاواں())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  إذا (Error ? !isValid(*Error) : !isValid())
    أرجع خطأ;

  بنانشانی nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  إذا (String.انکڑے() == خالی_سوچک)
    String = "";

  // pmatch needs to have at least one element.
  Small_سمتیہ<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.ناپ_2();

  صحيح rc = llvm_regexec(preg, String.انکڑے(), nmatch, pm.انکڑے(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  إذا (rc == REG_NOMATCH)
    أرجع خطأ;
  إذا (rc != 0) {
    إذا (Error)
      Regex_Error_منزل_String(error, preg, *Error);
    أرجع خطأ;
  }

  // There was a match.

  إذا (Matches) { // match position requested
    Matches->clear();

    لكل (بنانشانی i = 0; i != nmatch; ++i) {
      إذا (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->ڈالو_پٹھ(StringRef());
        استمر;
      }
      أكد(pm[i].rm_eo >= pm[i].rm_so);
      Matches->ڈالو_پٹھ(StringRef(String.انکڑے()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  أرجع سچ;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ثابت {
  Small_سمتیہ<StringRef, 8> Matches;

  // Return the input if there was no match.
  إذا (!میل(String, &Matches, Error))
    أرجع std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  طالما (!Repl.خالی_تھاواں()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> ونڈو_2 = Repl.ونڈو('\\');

    // Add the skipped substring.
    Res += ونڈو_2.پہلا_2;

    // Check for termination and trailing backslash.
    إذا (ونڈو_2.دوجا_2.خالی_تھاواں()) {
      إذا (Repl.ناپ_2() != ونڈو_2.پہلا_2.ناپ_2() &&
          Error && Error->خالی_تھاواں())
        *Error = "replacement string contained trailing backslash";
      اكسر;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = ونڈو_2.دوجا_2;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    بدل (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    حالة 'g':
      إذا (Repl.ناپ_2() >= 4 && Repl[1] == '<') {
        size_t سرا_2 = Repl.لبھو('>');
        StringRef Ref = Repl.ٹکڑا(2, سرا_2);
        بنانشانی Ref_قدر;
        إذا (سرا_2 != StringRef::npos && !Ref.getAsInteger(10, Ref_قدر)) {
          Repl = Repl.substr(سرا_2 + 1);
          إذا (Ref_قدر < Matches.ناپ_2())
            Res += Matches[Ref_قدر];
          وإلا إذا (Error && Error->خالی_تھاواں())
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
      إذا (Repl.ناپ_2() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += حرف(hex_Digit_قدر(Repl[1]) * 16 + hex_Digit_قدر(Repl[2]));
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
      StringRef Ref = Repl.ٹکڑا(0, Repl.لبھو_پہلا_not_دا("0123456789"));
      Repl = Repl.substr(Ref.ناپ_2());

      بنانشانی Ref_قدر;
      إذا (!Ref.getAsInteger(10, Ref_قدر) &&
          Ref_قدر < Matches.ناپ_2())
        Res += Matches[Ref_قدر];
      وإلا إذا (Error && Error->خالی_تھاواں())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      اكسر;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].سرا(), String.سرا() - Matches[0].سرا());

  أرجع Res;
}

// These are the special characters matched in functions like "p_ere_exp".
ساكن ثابت حرف RegexMetachars[] = "()^$|*+?.[]\\{}";

منطقي Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  أرجع Str.لبھو_پہلا_دا(RegexMetachars) == StringRef::npos;
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
