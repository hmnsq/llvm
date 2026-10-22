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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Regex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "regex_impl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Twine.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <string>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

Regex::Regex() : preg(ਖਾਲੀ_ਸੂਚਕ), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ flags = 0;
  preg = ਨਵਾਂ llvm_regex();
  preg->re_endp = regex.ਸਿਰਾ();
  ਜੇ (Flags & IgnoreCase)
    flags |= REG_ICASE;
  ਜੇ (Flags & Newline)
    flags |= REG_NEWLINE;
  ਜੇ (!(Flags & BasicRegex))
    flags |= REG_EXTENDED;
  error = llvm_regcomp(preg, regex.ਅੰਕੜੇ(), flags|REG_PEND);
}

Regex::Regex(StringRef regex, ਬਿਨਾਨਿਸ਼ਾਨੀ Flags)
    : Regex(regex, static_cast<RegexFlags>(Flags)) {}

Regex::Regex(Regex &&regex) {
  preg = regex.preg;
  error = regex.error;
  regex.preg = ਖਾਲੀ_ਸੂਚਕ;
  regex.error = REG_BADPAT;
}

Regex::~Regex() {
  ਜੇ (preg) {
    llvm_regfree(preg);
    ਮਿਟਾਓ preg;
  }
}

ਨਾਮਾਕਾਸ਼ {

/// Utility to convert a regex error code into a human-readable string.
ਖਾਲੀ Regex_Error_ਮੰਜ਼ਿਲ_String(ਪੂਰਨ error, ਬਣਤਰ llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, ਖਾਲੀ_ਸੂਚਕ, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

ਤਾਰਕਿਕ Regex::isValid(std::string &Error) ਅਚਲ {
  ਜੇ (!error)
    ਵਾਪਸ ਸੱਚ;

  Regex_Error_ਮੰਜ਼ਿਲ_String(error, preg, Error);
  ਵਾਪਸ ਝੂਠ;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
ਬਿਨਾਨਿਸ਼ਾਨੀ Regex::get_ਅੰਕ_Matches() ਅਚਲ {
  ਵਾਪਸ preg->re_nsub;
}

ਤਾਰਕਿਕ Regex::ਮੇਲ(StringRef String, Small_ਸਦਿਸ਼_Impl<StringRef> *Matches,
                  std::string *Error) ਅਚਲ {
  // Reset error, if given.
  ਜੇ (Error && !Error->ਖਾਲੀ_ਥਾਵਾਂ())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  ਜੇ (Error ? !isValid(*Error) : !isValid())
    ਵਾਪਸ ਝੂਠ;

  ਬਿਨਾਨਿਸ਼ਾਨੀ nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  ਜੇ (String.ਅੰਕੜੇ() == ਖਾਲੀ_ਸੂਚਕ)
    String = "";

  // pmatch needs to have at least one element.
  Small_ਸਦਿਸ਼<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.ਮਾਪ_2();

  ਪੂਰਨ rc = llvm_regexec(preg, String.ਅੰਕੜੇ(), nmatch, pm.ਅੰਕੜੇ(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  ਜੇ (rc == REG_NOMATCH)
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (rc != 0) {
    ਜੇ (Error)
      Regex_Error_ਮੰਜ਼ਿਲ_String(error, preg, *Error);
    ਵਾਪਸ ਝੂਠ;
  }

  // There was a match.

  ਜੇ (Matches) { // match position requested
    Matches->clear();

    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != nmatch; ++i) {
      ਜੇ (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->ਧੱਕੋ_ਪਿੱਠ(StringRef());
        ਜਾਰੀ;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ(pm[i].rm_eo >= pm[i].rm_so);
      Matches->ਧੱਕੋ_ਪਿੱਠ(StringRef(String.ਅੰਕੜੇ()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  ਵਾਪਸ ਸੱਚ;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ਅਚਲ {
  Small_ਸਦਿਸ਼<StringRef, 8> Matches;

  // Return the input if there was no match.
  ਜੇ (!ਮੇਲ(String, &Matches, Error))
    ਵਾਪਸ std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  ਜਦੋਂ (!Repl.ਖਾਲੀ_ਥਾਵਾਂ()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> ਵੰਡੋ_2 = Repl.ਵੰਡੋ('\\');

    // Add the skipped substring.
    Res += ਵੰਡੋ_2.ਪਹਿਲਾ_2;

    // Check for termination and trailing backslash.
    ਜੇ (ਵੰਡੋ_2.ਦੂਜਾ_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
      ਜੇ (Repl.ਮਾਪ_2() != ਵੰਡੋ_2.ਪਹਿਲਾ_2.ਮਾਪ_2() &&
          Error && Error->ਖਾਲੀ_ਥਾਵਾਂ())
        *Error = "replacement string contained trailing backslash";
      ਰੋਕੋ;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = ਵੰਡੋ_2.ਦੂਜਾ_2;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    ਚੋਣ (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    ਹਾਲਤ 'g':
      ਜੇ (Repl.ਮਾਪ_2() >= 4 && Repl[1] == '<') {
        size_t ਸਿਰਾ_2 = Repl.ਲੱਭੋ('>');
        StringRef Ref = Repl.ਟੁਕੜਾ(2, ਸਿਰਾ_2);
        ਬਿਨਾਨਿਸ਼ਾਨੀ Ref_ਮੁੱਲ;
        ਜੇ (ਸਿਰਾ_2 != StringRef::npos && !Ref.getAsInteger(10, Ref_ਮੁੱਲ)) {
          Repl = Repl.substr(ਸਿਰਾ_2 + 1);
          ਜੇ (Ref_ਮੁੱਲ < Matches.ਮਾਪ_2())
            Res += Matches[Ref_ਮੁੱਲ];
          ਨਹੀਂਤਾਂ ਜੇ (Error && Error->ਖਾਲੀ_ਥਾਵਾਂ())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          ਰੋਕੋ;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    ਮੂਲ:
      Res += Repl[0];
      Repl = Repl.substr(1);
      ਰੋਕੋ;

      // Single character escapes.
    ਹਾਲਤ 't':
      Res += '\t';
      Repl = Repl.substr(1);
      ਰੋਕੋ;
    ਹਾਲਤ 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      ਰੋਕੋ;

      // Hex character escapes.
    ਹਾਲਤ 'x':
      ਜੇ (Repl.ਮਾਪ_2() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += ਅੱਖਰ(hex_Digit_ਮੁੱਲ(Repl[1]) * 16 + hex_Digit_ਮੁੱਲ(Repl[2]));
        Repl = Repl.substr(3);
      } ਨਹੀਂਤਾਂ {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      ਰੋਕੋ;

      // Decimal escapes are backreferences.
    ਹਾਲਤ '0': ਹਾਲਤ '1': ਹਾਲਤ '2': ਹਾਲਤ '3': ਹਾਲਤ '4':
    ਹਾਲਤ '5': ਹਾਲਤ '6': ਹਾਲਤ '7': ਹਾਲਤ '8': ਹਾਲਤ '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.ਟੁਕੜਾ(0, Repl.ਲੱਭੋ_ਪਹਿਲਾ_not_ਦਾ("0123456789"));
      Repl = Repl.substr(Ref.ਮਾਪ_2());

      ਬਿਨਾਨਿਸ਼ਾਨੀ Ref_ਮੁੱਲ;
      ਜੇ (!Ref.getAsInteger(10, Ref_ਮੁੱਲ) &&
          Ref_ਮੁੱਲ < Matches.ਮਾਪ_2())
        Res += Matches[Ref_ਮੁੱਲ];
      ਨਹੀਂਤਾਂ ਜੇ (Error && Error->ਖਾਲੀ_ਥਾਵਾਂ())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      ਰੋਕੋ;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].ਸਿਰਾ(), String.ਸਿਰਾ() - Matches[0].ਸਿਰਾ());

  ਵਾਪਸ Res;
}

// These are the special characters matched in functions like "p_ere_exp".
ਸਥਿਰ ਅਚਲ ਅੱਖਰ RegexMetachars[] = "()^$|*+?.[]\\{}";

ਤਾਰਕਿਕ Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  ਵਾਪਸ Str.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  ਲਈ (ਅੱਖਰ C : String) {
    ਜੇ (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  ਵਾਪਸ RegexStr;
}
