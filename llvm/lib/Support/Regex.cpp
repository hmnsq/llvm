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

#अन्तर्भाव "llvm/Support/Regex.h"
#अन्तर्भाव "regex_impl.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/Twine.h"

#अन्तर्भाव <cassert>
#अन्तर्भाव <string>

using नामाकाश llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  अचिन्हित flags = 0;
  preg = नवीन llvm_regex();
  preg->re_endp = regex.end();
  जर (Flags & IgnoreCase)
    flags |= REG_ICASE;
  जर (Flags & Newline)
    flags |= REG_NEWLINE;
  जर (!(Flags & BasicRegex))
    flags |= REG_EXTENDED;
  error = llvm_regcomp(preg, regex.data(), flags|REG_PEND);
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
  जर (preg) {
    llvm_regfree(preg);
    delete preg;
  }
}

नामाकाश {

/// Utility to convert a regex error code into a human-readable string.
शून्य RegexErrorToString(पूर्णांक error, संरचना llvm_regex *preg,
                        std::string &Error) {
  माप_प्रकार len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

तार्किक Regex::isValid(std::string &Error) स्थिरांक {
  जर (!error)
    परत सत्य;

  RegexErrorToString(error, preg, Error);
  परत असत्य;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
अचिन्हित Regex::get_अंक_Matches() स्थिरांक {
  परत preg->re_nsub;
}

तार्किक Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) स्थिरांक {
  // Reset error, if given.
  जर (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  जर (Error ? !isValid(*Error) : !isValid())
    परत असत्य;

  अचिन्हित nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  जर (String.data() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.size();

  पूर्णांक rc = llvm_regexec(preg, String.data(), nmatch, pm.data(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  जर (rc == REG_NOMATCH)
    परत असत्य;
  जर (rc != 0) {
    जर (Error)
      RegexErrorToString(error, preg, *Error);
    परत असत्य;
  }

  // There was a match.

  जर (Matches) { // match position requested
    Matches->clear();

    साठी (अचिन्हित i = 0; i != nmatch; ++i) {
      जर (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->ढकला_back(StringRef());
        सुरूठेवा;
      }
      प्रतिज्ञा(pm[i].rm_eo >= pm[i].rm_so);
      Matches->ढकला_back(StringRef(String.data()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  परत सत्य;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) स्थिरांक {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  जर (!match(String, &Matches, Error))
    परत std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  पर्यंत (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    जर (Split.second.empty()) {
      जर (Repl.size() != Split.first.size() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      थांबा;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    निवड (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    स्थिती 'g':
      जर (Repl.size() >= 4 && Repl[1] == '<') {
        माप_प्रकार End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        अचिन्हित Ref_मूल्य;
        जर (End != StringRef::npos && !Ref.getAsInteger(10, Ref_मूल्य)) {
          Repl = Repl.substr(End + 1);
          जर (Ref_मूल्य < Matches.size())
            Res += Matches[Ref_मूल्य];
          अन्यथा जर (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          थांबा;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    पूर्वनिर्धारित:
      Res += Repl[0];
      Repl = Repl.substr(1);
      थांबा;

      // Single character escapes.
    स्थिती 't':
      Res += '\t';
      Repl = Repl.substr(1);
      थांबा;
    स्थिती 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      थांबा;

      // Hex character escapes.
    स्थिती 'x':
      जर (Repl.size() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += अक्षर(hex_Digit_मूल्य(Repl[1]) * 16 + hex_Digit_मूल्य(Repl[2]));
        Repl = Repl.substr(3);
      } अन्यथा {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      थांबा;

      // Decimal escapes are backreferences.
    स्थिती '0': स्थिती '1': स्थिती '2': स्थिती '3': स्थिती '4':
    स्थिती '5': स्थिती '6': स्थिती '7': स्थिती '8': स्थिती '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.size());

      अचिन्हित Ref_मूल्य;
      जर (!Ref.getAsInteger(10, Ref_मूल्य) &&
          Ref_मूल्य < Matches.size())
        Res += Matches[Ref_मूल्य];
      अन्यथा जर (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      थांबा;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  परत Res;
}

// These are the special characters matched in functions like "p_ere_exp".
स्थिर स्थिरांक अक्षर RegexMetachars[] = "()^$|*+?.[]\\{}";

तार्किक Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  परत Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  साठी (अक्षर C : String) {
    जर (म_अक्षर(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  परत RegexStr;
}
