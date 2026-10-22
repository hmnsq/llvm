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

کارول نطاق llvm;

Regex::Regex() : preg(تش_اشاره), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  بینښه flags = 0;
  preg = جديد llvm_regex();
  preg->re_endp = regex.پای_ټکی();
  إذا (Flags & IgnoreCase)
    flags |= REG_ICASE;
  إذا (Flags & Newline)
    flags |= REG_NEWLINE;
  إذا (!(Flags & BasicRegex))
    flags |= REG_EXTENDED;
  error = llvm_regcomp(preg, regex.معلومات(), flags|REG_PEND);
}

Regex::Regex(StringRef regex, بینښه Flags)
    : Regex(regex, static_cast<RegexFlags>(Flags)) {}

Regex::Regex(Regex &&regex) {
  preg = regex.preg;
  error = regex.error;
  regex.preg = تش_اشاره;
  regex.error = REG_BADPAT;
}

Regex::~Regex() {
  إذا (preg) {
    llvm_regfree(preg);
    ړنګول preg;
  }
}

نطاق {

/// Utility to convert a regex error code into a human-readable string.
فراغ Regex_Error_منزل_String(صحيح error, بنية llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, تش_اشاره, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

منطقي Regex::isValid(std::string &Error) ثابت {
  إذا (!error)
    أرجع ریښتیا;

  Regex_Error_منزل_String(error, preg, Error);
  أرجع خطأ;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
بینښه Regex::get_عدد_Matches() ثابت {
  أرجع preg->re_nsub;
}

منطقي Regex::سمون(StringRef String, Small_ویکټور_Impl<StringRef> *Matches,
                  std::string *Error) ثابت {
  // Reset error, if given.
  إذا (Error && !Error->تشه())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  إذا (Error ? !isValid(*Error) : !isValid())
    أرجع خطأ;

  بینښه nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  إذا (String.معلومات() == تش_اشاره)
    String = "";

  // pmatch needs to have at least one element.
  Small_ویکټور<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.اندازه_2();

  صحيح rc = llvm_regexec(preg, String.معلومات(), nmatch, pm.معلومات(), REG_STARTEND);

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

    لكل (بینښه i = 0; i != nmatch; ++i) {
      إذا (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->ټېل_شا(StringRef());
        استمر;
      }
      أكد(pm[i].rm_eo >= pm[i].rm_so);
      Matches->ټېل_شا(StringRef(String.معلومات()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  أرجع ریښتیا;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ثابت {
  Small_ویکټور<StringRef, 8> Matches;

  // Return the input if there was no match.
  إذا (!سمون(String, &Matches, Error))
    أرجع std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  طالما (!Repl.تشه()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> وېشل_2 = Repl.وېشل('\\');

    // Add the skipped substring.
    Res += وېشل_2.لومړی_2;

    // Check for termination and trailing backslash.
    إذا (وېشل_2.دوهم_2.تشه()) {
      إذا (Repl.اندازه_2() != وېشل_2.لومړی_2.اندازه_2() &&
          Error && Error->تشه())
        *Error = "replacement string contained trailing backslash";
      اكسر;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = وېشل_2.دوهم_2;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    بدل (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    حالة 'g':
      إذا (Repl.اندازه_2() >= 4 && Repl[1] == '<') {
        size_t پای_ټکی_2 = Repl.پیدا_کړه('>');
        StringRef Ref = Repl.ټوټه(2, پای_ټکی_2);
        بینښه Ref_ارزښت;
        إذا (پای_ټکی_2 != StringRef::npos && !Ref.getAsInteger(10, Ref_ارزښت)) {
          Repl = Repl.substr(پای_ټکی_2 + 1);
          إذا (Ref_ارزښت < Matches.اندازه_2())
            Res += Matches[Ref_ارزښت];
          وإلا إذا (Error && Error->تشه())
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
      إذا (Repl.اندازه_2() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += حرف(hex_Digit_ارزښت(Repl[1]) * 16 + hex_Digit_ارزښت(Repl[2]));
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
      StringRef Ref = Repl.ټوټه(0, Repl.پیدا_کړه_لومړی_not_د("0123456789"));
      Repl = Repl.substr(Ref.اندازه_2());

      بینښه Ref_ارزښت;
      إذا (!Ref.getAsInteger(10, Ref_ارزښت) &&
          Ref_ارزښت < Matches.اندازه_2())
        Res += Matches[Ref_ارزښت];
      وإلا إذا (Error && Error->تشه())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      اكسر;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].پای_ټکی(), String.پای_ټکی() - Matches[0].پای_ټکی());

  أرجع Res;
}

// These are the special characters matched in functions like "p_ere_exp".
ساكن ثابت حرف RegexMetachars[] = "()^$|*+?.[]\\{}";

منطقي Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  أرجع Str.پیدا_کړه_لومړی_د(RegexMetachars) == StringRef::npos;
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
