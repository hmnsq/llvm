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

#समावेश "llvm/Support/Regex.h"
#समावेश "regex_impl.h"
#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/ADT/Twine.h"

#समावेश <cassert>
#समावेश <string>

using namespace llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  अचिन्हित flags = 0;
  preg = नया llvm_regex();
  preg->re_endp = regex.end();
  अगर (Flags & IgnoreCase)
    flags |= REG_ICASE;
  अगर (Flags & Newline)
    flags |= REG_NEWLINE;
  अगर (!(Flags & BasicRegex))
    flags |= REG_EXTENDED;
  error = llvm_regcomp(preg, regex.तथ्य(), flags|REG_PEND);
}

Regex::Regex(StringRef regex, अचिन्हित Flags)
    : Regex(regex, static_cast<RegexFlags>(Flags)) {}

Regex::Regex(Regex &&regex) {
  preg = regex.preg;
  error = regex.error;
  regex.preg = nullptr;
  regex.error = REG_BADPAT;
}

Regex::~Regex() {
  अगर (preg) {
    llvm_regfree(preg);
    मिटाओ preg;
  }
}

namespace {

/// Utility to convert a regex error code into a human-readable string.
व्योम RegexErrorToString(पूर्णांक error, काष्ठा llvm_regex *preg,
                        std::string &Error) {
  माप_प्रकार len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

bool Regex::isValid(std::string &Error) स्थिर {
  अगर (!error)
    वापस true;

  RegexErrorToString(error, preg, Error);
  वापस false;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
अचिन्हित Regex::get_अंक_Matches() स्थिर {
  वापस preg->re_nsub;
}

bool Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) स्थिर {
  // Reset error, if given.
  अगर (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  अगर (Error ? !isValid(*Error) : !isValid())
    वापस false;

  अचिन्हित nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  अगर (String.तथ्य() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.आकार_2();

  पूर्णांक rc = llvm_regexec(preg, String.तथ्य(), nmatch, pm.तथ्य(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  अगर (rc == REG_NOMATCH)
    वापस false;
  अगर (rc != 0) {
    अगर (Error)
      RegexErrorToString(error, preg, *Error);
    वापस false;
  }

  // There was a match.

  अगर (Matches) { // match position requested
    Matches->सफा();

    क्रम (अचिन्हित i = 0; i != nmatch; ++i) {
      अगर (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->धकेलो_back(StringRef());
        जारी;
      }
      निश्चित(pm[i].rm_eo >= pm[i].rm_so);
      Matches->धकेलो_back(StringRef(String.तथ्य()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  वापस true;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) स्थिर {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  अगर (!match(String, &Matches, Error))
    वापस std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  जबतक (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    अगर (Split.second.empty()) {
      अगर (Repl.आकार_2() != Split.first.आकार_2() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      अवरोध;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    चयन (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    हाल 'g':
      अगर (Repl.आकार_2() >= 4 && Repl[1] == '<') {
        माप_प्रकार End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        अचिन्हित Ref_मूल्य;
        अगर (End != StringRef::npos && !Ref.getAsInteger(10, Ref_मूल्य)) {
          Repl = Repl.substr(End + 1);
          अगर (Ref_मूल्य < Matches.आकार_2())
            Res += Matches[Ref_मूल्य];
          अन्यथा अगर (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          अवरोध;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    शेष:
      Res += Repl[0];
      Repl = Repl.substr(1);
      अवरोध;

      // Single character escapes.
    हाल 't':
      Res += '\t';
      Repl = Repl.substr(1);
      अवरोध;
    हाल 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      अवरोध;

      // Hex character escapes.
    हाल 'x':
      अगर (Repl.आकार_2() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += अक्षर(hex_Digit_मूल्य(Repl[1]) * 16 + hex_Digit_मूल्य(Repl[2]));
        Repl = Repl.substr(3);
      } अन्यथा {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      अवरोध;

      // Decimal escapes are backreferences.
    हाल '0': हाल '1': हाल '2': हाल '3': हाल '4':
    हाल '5': हाल '6': हाल '7': हाल '8': हाल '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.आकार_2());

      अचिन्हित Ref_मूल्य;
      अगर (!Ref.getAsInteger(10, Ref_मूल्य) &&
          Ref_मूल्य < Matches.आकार_2())
        Res += Matches[Ref_मूल्य];
      अन्यथा अगर (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      अवरोध;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  वापस Res;
}

// These are the special characters matched in functions like "p_ere_exp".
अटल स्थिर अक्षर RegexMetachars[] = "()^$|*+?.[]\\{}";

bool Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  वापस Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  क्रम (अक्षर C : String) {
    अगर (म_अक्षर(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  वापस RegexStr;
}
