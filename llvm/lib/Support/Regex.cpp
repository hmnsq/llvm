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

#அந்தர்பாவ "llvm/Support/Regex.h"
#அந்தர்பாவ "regex_impl.h"
#அந்தர்பாவ "llvm/ADT/SmallVector.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/ADT/Twine.h"

#அந்தர்பாவ <cassert>
#அந்தர்பாவ <string>

using நாமாகாஶ llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  unsigned flags = 0;
  preg = புதிய llvm_regex();
  preg->re_endp = regex.end();
  எனில் (Flags & IgnoreCase)
    flags |= REG_ICASE;
  எனில் (Flags & Newline)
    flags |= REG_NEWLINE;
  எனில் (!(Flags & BasicRegex))
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
  எனில் (preg) {
    llvm_regfree(preg);
    delete preg;
  }
}

நாமாகாஶ {

/// Utility to convert a regex error code into a human-readable string.
வெறுமை RegexErrorToString(முழு error, கட்டமைப்பு llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

தருக்க Regex::isValid(std::string &Error) மாறிலி {
  எனில் (!error)
    திரும்பு உண்மை;

  RegexErrorToString(error, preg, Error);
  திரும்பு பொய்;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
unsigned Regex::get_இலக்கம்_Matches() மாறிலி {
  திரும்பு preg->re_nsub;
}

தருக்க Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) மாறிலி {
  // Reset error, if given.
  எனில் (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  எனில் (Error ? !isValid(*Error) : !isValid())
    திரும்பு பொய்;

  unsigned nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  எனில் (String.data() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.size();

  முழு rc = llvm_regexec(preg, String.data(), nmatch, pm.data(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  எனில் (rc == REG_NOMATCH)
    திரும்பு பொய்;
  எனில் (rc != 0) {
    எனில் (Error)
      RegexErrorToString(error, preg, *Error);
    திரும்பு பொய்;
  }

  // There was a match.

  எனில் (Matches) { // match position requested
    Matches->clear();

    க்கு (unsigned i = 0; i != nmatch; ++i) {
      எனில் (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->தள்ளு_back(StringRef());
        தொடர்;
      }
      ப்ரதிஜ்ஞா(pm[i].rm_eo >= pm[i].rm_so);
      Matches->தள்ளு_back(StringRef(String.data()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  திரும்பு உண்மை;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) மாறிலி {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  எனில் (!match(String, &Matches, Error))
    திரும்பு std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  போது (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    எனில் (Split.second.empty()) {
      எனில் (Repl.size() != Split.first.size() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      நிறுத்து;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    தேர்வு (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    நிலை 'g':
      எனில் (Repl.size() >= 4 && Repl[1] == '<') {
        size_t End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        unsigned Ref_மதிப்பு;
        எனில் (End != StringRef::npos && !Ref.getAsInteger(10, Ref_மதிப்பு)) {
          Repl = Repl.substr(End + 1);
          எனில் (Ref_மதிப்பு < Matches.size())
            Res += Matches[Ref_மதிப்பு];
          இல்லையெனில் எனில் (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          நிறுத்து;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    இயல்பு:
      Res += Repl[0];
      Repl = Repl.substr(1);
      நிறுத்து;

      // Single character escapes.
    நிலை 't':
      Res += '\t';
      Repl = Repl.substr(1);
      நிறுத்து;
    நிலை 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      நிறுத்து;

      // Hex character escapes.
    நிலை 'x':
      எனில் (Repl.size() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += எழுத்து(hex_Digit_மதிப்பு(Repl[1]) * 16 + hex_Digit_மதிப்பு(Repl[2]));
        Repl = Repl.substr(3);
      } இல்லையெனில் {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      நிறுத்து;

      // Decimal escapes are backreferences.
    நிலை '0': நிலை '1': நிலை '2': நிலை '3': நிலை '4':
    நிலை '5': நிலை '6': நிலை '7': நிலை '8': நிலை '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.size());

      unsigned Ref_மதிப்பு;
      எனில் (!Ref.getAsInteger(10, Ref_மதிப்பு) &&
          Ref_மதிப்பு < Matches.size())
        Res += Matches[Ref_மதிப்பு];
      இல்லையெனில் எனில் (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      நிறுத்து;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  திரும்பு Res;
}

// These are the special characters matched in functions like "p_ere_exp".
நிலையான மாறிலி எழுத்து RegexMetachars[] = "()^$|*+?.[]\\{}";

தருக்க Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  திரும்பு Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  க்கு (எழுத்து C : String) {
    எனில் (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  திரும்பு RegexStr;
}
