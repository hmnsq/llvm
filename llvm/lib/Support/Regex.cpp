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

استعمال نامخلا llvm;

Regex::Regex() : preg(خالی_اشاریہ), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  غیرعلامتی flags = 0;
  preg = نیا llvm_regex();
  preg->re_endp = regex.انتہا();
  اگر (Flags & IgnoreCase)
    flags |= REG_ICASE;
  اگر (Flags & Newline)
    flags |= REG_NEWLINE;
  اگر (!(Flags & BasicRegex))
    flags |= REG_EXTENDED;
  error = llvm_regcomp(preg, regex.ڈیٹا(), flags|REG_PEND);
}

Regex::Regex(StringRef regex, غیرعلامتی Flags)
    : Regex(regex, static_cast<RegexFlags>(Flags)) {}

Regex::Regex(Regex &&regex) {
  preg = regex.preg;
  error = regex.error;
  regex.preg = خالی_اشاریہ;
  regex.error = REG_BADPAT;
}

Regex::~Regex() {
  اگر (preg) {
    llvm_regfree(preg);
    حذف preg;
  }
}

نامخلا {

/// Utility to convert a regex error code into a human-readable string.
خالی Regex_Error_مقصد_String(صحیح error, ساخت llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, خالی_اشاریہ, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

منطقی Regex::isValid(std::string &Error) ثابت {
  اگر (!error)
    واپس سچ;

  Regex_Error_مقصد_String(error, preg, Error);
  واپس جھوٹ;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
غیرعلامتی Regex::get_عدد_Matches() ثابت {
  واپس preg->re_nsub;
}

منطقی Regex::مطابقت(StringRef String, Small_سمتیہ_Impl<StringRef> *Matches,
                  std::string *Error) ثابت {
  // Reset error, if given.
  اگر (Error && !Error->خالی_خانے())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  اگر (Error ? !isValid(*Error) : !isValid())
    واپس جھوٹ;

  غیرعلامتی nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  اگر (String.ڈیٹا() == خالی_اشاریہ)
    String = "";

  // pmatch needs to have at least one element.
  Small_سمتیہ<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.سائز_2();

  صحیح rc = llvm_regexec(preg, String.ڈیٹا(), nmatch, pm.ڈیٹا(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  اگر (rc == REG_NOMATCH)
    واپس جھوٹ;
  اگر (rc != 0) {
    اگر (Error)
      Regex_Error_مقصد_String(error, preg, *Error);
    واپس جھوٹ;
  }

  // There was a match.

  اگر (Matches) { // match position requested
    Matches->clear();

    برائے (غیرعلامتی i = 0; i != nmatch; ++i) {
      اگر (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->ڈالو_عقب(StringRef());
        جاری;
      }
      أكد(pm[i].rm_eo >= pm[i].rm_so);
      Matches->ڈالو_عقب(StringRef(String.ڈیٹا()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  واپس سچ;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ثابت {
  Small_سمتیہ<StringRef, 8> Matches;

  // Return the input if there was no match.
  اگر (!مطابقت(String, &Matches, Error))
    واپس std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  جب (!Repl.خالی_خانے()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> بانٹو_2 = Repl.بانٹو('\\');

    // Add the skipped substring.
    Res += بانٹو_2.پہلا_2;

    // Check for termination and trailing backslash.
    اگر (بانٹو_2.دوسرا_2.خالی_خانے()) {
      اگر (Repl.سائز_2() != بانٹو_2.پہلا_2.سائز_2() &&
          Error && Error->خالی_خانے())
        *Error = "replacement string contained trailing backslash";
      توڑو;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = بانٹو_2.دوسرا_2;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    انتخاب (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    حالت 'g':
      اگر (Repl.سائز_2() >= 4 && Repl[1] == '<') {
        size_t انتہا_2 = Repl.ڈھونڈو('>');
        StringRef Ref = Repl.قاش(2, انتہا_2);
        غیرعلامتی Ref_قدر;
        اگر (انتہا_2 != StringRef::npos && !Ref.getAsInteger(10, Ref_قدر)) {
          Repl = Repl.substr(انتہا_2 + 1);
          اگر (Ref_قدر < Matches.سائز_2())
            Res += Matches[Ref_قدر];
          ورنہ اگر (Error && Error->خالی_خانے())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          توڑو;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    طےشدہ:
      Res += Repl[0];
      Repl = Repl.substr(1);
      توڑو;

      // Single character escapes.
    حالت 't':
      Res += '\t';
      Repl = Repl.substr(1);
      توڑو;
    حالت 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      توڑو;

      // Hex character escapes.
    حالت 'x':
      اگر (Repl.سائز_2() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += حرف(hex_Digit_قدر(Repl[1]) * 16 + hex_Digit_قدر(Repl[2]));
        Repl = Repl.substr(3);
      } ورنہ {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      توڑو;

      // Decimal escapes are backreferences.
    حالت '0': حالت '1': حالت '2': حالت '3': حالت '4':
    حالت '5': حالت '6': حالت '7': حالت '8': حالت '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.قاش(0, Repl.ڈھونڈو_پہلا_not_کا("0123456789"));
      Repl = Repl.substr(Ref.سائز_2());

      غیرعلامتی Ref_قدر;
      اگر (!Ref.getAsInteger(10, Ref_قدر) &&
          Ref_قدر < Matches.سائز_2())
        Res += Matches[Ref_قدر];
      ورنہ اگر (Error && Error->خالی_خانے())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      توڑو;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].انتہا(), String.انتہا() - Matches[0].انتہا());

  واپس Res;
}

// These are the special characters matched in functions like "p_ere_exp".
ساکن ثابت حرف RegexMetachars[] = "()^$|*+?.[]\\{}";

منطقی Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  واپس Str.ڈھونڈو_پہلا_کا(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  برائے (حرف C : String) {
    اگر (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  واپس RegexStr;
}
