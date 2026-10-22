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

#അന്തര്ഭാവ "llvm/Support/Regex.h"
#അന്തര്ഭാവ "regex_impl.h"
#അന്തര്ഭാവ "llvm/ADT/SmallVector.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/ADT/Twine.h"

#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <string>

using നാമാകാശ llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  unsigned flags = 0;
  preg = പുതിയ llvm_regex();
  preg->re_endp = regex.end();
  എങ്കിൽ (Flags & IgnoreCase)
    flags |= REG_ICASE;
  എങ്കിൽ (Flags & Newline)
    flags |= REG_NEWLINE;
  എങ്കിൽ (!(Flags & BasicRegex))
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
  എങ്കിൽ (preg) {
    llvm_regfree(preg);
    delete preg;
  }
}

നാമാകാശ {

/// Utility to convert a regex error code into a human-readable string.
ശൂന്യം RegexErrorToString(പൂർണ്ണസംഖ്യ error, ഘടന llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

തർക്കം Regex::isValid(std::string &Error) സ്ഥിരം {
  എങ്കിൽ (!error)
    തിരികെ സത്യം;

  RegexErrorToString(error, preg, Error);
  തിരികെ അസത്യം;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
unsigned Regex::get_അക്കം_Matches() സ്ഥിരം {
  തിരികെ preg->re_nsub;
}

തർക്കം Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) സ്ഥിരം {
  // Reset error, if given.
  എങ്കിൽ (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  എങ്കിൽ (Error ? !isValid(*Error) : !isValid())
    തിരികെ അസത്യം;

  unsigned nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  എങ്കിൽ (String.data() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.size();

  പൂർണ്ണസംഖ്യ rc = llvm_regexec(preg, String.data(), nmatch, pm.data(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  എങ്കിൽ (rc == REG_NOMATCH)
    തിരികെ അസത്യം;
  എങ്കിൽ (rc != 0) {
    എങ്കിൽ (Error)
      RegexErrorToString(error, preg, *Error);
    തിരികെ അസത്യം;
  }

  // There was a match.

  എങ്കിൽ (Matches) { // match position requested
    Matches->clear();

    വേണ്ടി (unsigned i = 0; i != nmatch; ++i) {
      എങ്കിൽ (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->തള്ളുക_back(StringRef());
        തുടരുക;
      }
      പ്രതിജ്ഞാ(pm[i].rm_eo >= pm[i].rm_so);
      Matches->തള്ളുക_back(StringRef(String.data()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  തിരികെ സത്യം;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) സ്ഥിരം {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  എങ്കിൽ (!match(String, &Matches, Error))
    തിരികെ std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  ആകുമ്പോൾ (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    എങ്കിൽ (Split.second.empty()) {
      എങ്കിൽ (Repl.size() != Split.first.size() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      നിർത്തുക;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    തിരഞ്ഞെടുപ്പ് (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    അവസ്ഥ 'g':
      എങ്കിൽ (Repl.size() >= 4 && Repl[1] == '<') {
        size_t End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        unsigned Ref_മൂല്യം;
        എങ്കിൽ (End != StringRef::npos && !Ref.getAsInteger(10, Ref_മൂല്യം)) {
          Repl = Repl.substr(End + 1);
          എങ്കിൽ (Ref_മൂല്യം < Matches.size())
            Res += Matches[Ref_മൂല്യം];
          അല്ലെങ്കിൽ എങ്കിൽ (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          നിർത്തുക;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    സ്ഥിരസ്ഥിതി:
      Res += Repl[0];
      Repl = Repl.substr(1);
      നിർത്തുക;

      // Single character escapes.
    അവസ്ഥ 't':
      Res += '\t';
      Repl = Repl.substr(1);
      നിർത്തുക;
    അവസ്ഥ 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      നിർത്തുക;

      // Hex character escapes.
    അവസ്ഥ 'x':
      എങ്കിൽ (Repl.size() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += അക്ഷരം(hex_Digit_മൂല്യം(Repl[1]) * 16 + hex_Digit_മൂല്യം(Repl[2]));
        Repl = Repl.substr(3);
      } അല്ലെങ്കിൽ {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      നിർത്തുക;

      // Decimal escapes are backreferences.
    അവസ്ഥ '0': അവസ്ഥ '1': അവസ്ഥ '2': അവസ്ഥ '3': അവസ്ഥ '4':
    അവസ്ഥ '5': അവസ്ഥ '6': അവസ്ഥ '7': അവസ്ഥ '8': അവസ്ഥ '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.size());

      unsigned Ref_മൂല്യം;
      എങ്കിൽ (!Ref.getAsInteger(10, Ref_മൂല്യം) &&
          Ref_മൂല്യം < Matches.size())
        Res += Matches[Ref_മൂല്യം];
      അല്ലെങ്കിൽ എങ്കിൽ (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      നിർത്തുക;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  തിരികെ Res;
}

// These are the special characters matched in functions like "p_ere_exp".
സ്ഥിതം സ്ഥിരം അക്ഷരം RegexMetachars[] = "()^$|*+?.[]\\{}";

തർക്കം Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  തിരികെ Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  വേണ്ടി (അക്ഷരം C : String) {
    എങ്കിൽ (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  തിരികെ RegexStr;
}
