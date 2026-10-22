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

उपयुज्य नामाकाश llvm;

Regex::Regex() : preg(शून्यसूचकः), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  अचिह्नित flags = 0;
  preg = नूतन llvm_regex();
  preg->re_endp = regex.अन्तः();
  यदि (Flags & IgnoreCase)
    flags |= REG_ICASE;
  यदि (Flags & Newline)
    flags |= REG_NEWLINE;
  यदि (!(Flags & BasicRegex))
    flags |= REG_EXTENDED;
  error = llvm_regcomp(preg, regex.दत्त(), flags|REG_PEND);
}

Regex::Regex(StringRef regex, अचिह्नित Flags)
    : Regex(regex, static_cast<RegexFlags>(Flags)) {}

Regex::Regex(Regex &&regex) {
  preg = regex.preg;
  error = regex.error;
  regex.preg = शून्यसूचकः;
  regex.error = REG_BADPAT;
}

Regex::~Regex() {
  यदि (preg) {
    llvm_regfree(preg);
    विलोपय preg;
  }
}

नामाकाश {

/// Utility to convert a regex error code into a human-readable string.
शून्य Regex_Error_गन्तव्यम्_String(पूर्णाङ्क error, संरचना llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, शून्यसूचकः, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

तार्किक Regex::isValid(std::string &Error) ध्रुव {
  यदि (!error)
    प्रत्यागम सत्य;

  Regex_Error_गन्तव्यम्_String(error, preg, Error);
  प्रत्यागम असत्य;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
अचिह्नित Regex::get_अङ्कः_Matches() ध्रुव {
  प्रत्यागम preg->re_nsub;
}

तार्किक Regex::मेलनम्(StringRef String, Small_सदिशम्_Impl<StringRef> *Matches,
                  std::string *Error) ध्रुव {
  // Reset error, if given.
  यदि (Error && !Error->रिक्तानि())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  यदि (Error ? !isValid(*Error) : !isValid())
    प्रत्यागम असत्य;

  अचिह्नित nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  यदि (String.दत्त() == शून्यसूचकः)
    String = "";

  // pmatch needs to have at least one element.
  Small_सदिशम्<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.आकार_2();

  पूर्णाङ्क rc = llvm_regexec(preg, String.दत्त(), nmatch, pm.दत्त(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  यदि (rc == REG_NOMATCH)
    प्रत्यागम असत्य;
  यदि (rc != 0) {
    यदि (Error)
      Regex_Error_गन्तव्यम्_String(error, preg, *Error);
    प्रत्यागम असत्य;
  }

  // There was a match.

  यदि (Matches) { // match position requested
    Matches->clear();

    प्रति (अचिह्नित i = 0; i != nmatch; ++i) {
      यदि (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->प्रक्षिप_पृष्ठभागः(StringRef());
        अनुवर्त;
      }
      प्रतिज्ञा(pm[i].rm_eo >= pm[i].rm_so);
      Matches->प्रक्षिप_पृष्ठभागः(StringRef(String.दत्त()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  प्रत्यागम सत्य;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ध्रुव {
  Small_सदिशम्<StringRef, 8> Matches;

  // Return the input if there was no match.
  यदि (!मेलनम्(String, &Matches, Error))
    प्रत्यागम std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  यावत् (!Repl.रिक्तानि()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> विभज_2 = Repl.विभज('\\');

    // Add the skipped substring.
    Res += विभज_2.first;

    // Check for termination and trailing backslash.
    यदि (विभज_2.second.रिक्तानि()) {
      यदि (Repl.आकार_2() != विभज_2.first.आकार_2() &&
          Error && Error->रिक्तानि())
        *Error = "replacement string contained trailing backslash";
      भङ्ग;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = विभज_2.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    विकल्प (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    पक्ष 'g':
      यदि (Repl.आकार_2() >= 4 && Repl[1] == '<') {
        size_t अन्तः_2 = Repl.find('>');
        StringRef Ref = Repl.खण्डिका(2, अन्तः_2);
        अचिह्नित Ref_मानम्;
        यदि (अन्तः_2 != StringRef::npos && !Ref.getAsInteger(10, Ref_मानम्)) {
          Repl = Repl.substr(अन्तः_2 + 1);
          यदि (Ref_मानम् < Matches.आकार_2())
            Res += Matches[Ref_मानम्];
          अन्यथा यदि (Error && Error->रिक्तानि())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          भङ्ग;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    स्वभाव:
      Res += Repl[0];
      Repl = Repl.substr(1);
      भङ्ग;

      // Single character escapes.
    पक्ष 't':
      Res += '\t';
      Repl = Repl.substr(1);
      भङ्ग;
    पक्ष 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      भङ्ग;

      // Hex character escapes.
    पक्ष 'x':
      यदि (Repl.आकार_2() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += वर्ण(hex_Digit_मानम्(Repl[1]) * 16 + hex_Digit_मानम्(Repl[2]));
        Repl = Repl.substr(3);
      } अन्यथा {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      भङ्ग;

      // Decimal escapes are backreferences.
    पक्ष '0': पक्ष '1': पक्ष '2': पक्ष '3': पक्ष '4':
    पक्ष '5': पक्ष '6': पक्ष '7': पक्ष '8': पक्ष '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.खण्डिका(0, Repl.find_first_not_सम्बन्धि("0123456789"));
      Repl = Repl.substr(Ref.आकार_2());

      अचिह्नित Ref_मानम्;
      यदि (!Ref.getAsInteger(10, Ref_मानम्) &&
          Ref_मानम् < Matches.आकार_2())
        Res += Matches[Ref_मानम्];
      अन्यथा यदि (Error && Error->रिक्तानि())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      भङ्ग;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].अन्तः(), String.अन्तः() - Matches[0].अन्तः());

  प्रत्यागम Res;
}

// These are the special characters matched in functions like "p_ere_exp".
स्थिर ध्रुव वर्ण RegexMetachars[] = "()^$|*+?.[]\\{}";

तार्किक Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  प्रत्यागम Str.find_first_सम्बन्धि(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  प्रति (वर्ण C : String) {
    यदि (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  प्रत्यागम RegexStr;
}
