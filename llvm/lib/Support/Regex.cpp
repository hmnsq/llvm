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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Regex.h"
#ଅନ୍ତର୍ଭାଵ "regex_impl.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallVector.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Twine.h"

#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <string>

using ନାମାକାଶ llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  unsigned flags = 0;
  preg = ନୂତନ llvm_regex();
  preg->re_endp = regex.end();
  ଯଦି (Flags & IgnoreCase)
    flags |= REG_ICASE;
  ଯଦି (Flags & Newline)
    flags |= REG_NEWLINE;
  ଯଦି (!(Flags & BasicRegex))
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
  ଯଦି (preg) {
    llvm_regfree(preg);
    delete preg;
  }
}

ନାମାକାଶ {

/// Utility to convert a regex error code into a human-readable string.
ଶୂନ୍ୟ RegexErrorToString(ପୂର୍ଣ୍ଣସଂଖ୍ୟା error, ସଂରଚନା llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

ତାର୍କିକ Regex::isValid(std::string &Error) ଅଚଳ {
  ଯଦି (!error)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  RegexErrorToString(error, preg, Error);
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
unsigned Regex::get_ଅଙ୍କ_Matches() ଅଚଳ {
  ଫେରନ୍ତୁ preg->re_nsub;
}

ତାର୍କିକ Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) ଅଚଳ {
  // Reset error, if given.
  ଯଦି (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  ଯଦି (Error ? !isValid(*Error) : !isValid())
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  unsigned nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  ଯଦି (String.data() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.size();

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = llvm_regexec(preg, String.data(), nmatch, pm.data(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  ଯଦି (rc == REG_NOMATCH)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (rc != 0) {
    ଯଦି (Error)
      RegexErrorToString(error, preg, *Error);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  // There was a match.

  ଯଦି (Matches) { // match position requested
    Matches->clear();

    ପାଇଁ (unsigned i = 0; i != nmatch; ++i) {
      ଯଦି (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->ଠେଲ_back(StringRef());
        ଜାରି;
      }
      ପ୍ରତିଜ୍ଞା(pm[i].rm_eo >= pm[i].rm_so);
      Matches->ଠେଲ_back(StringRef(String.data()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) ଅଚଳ {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  ଯଦି (!match(String, &Matches, Error))
    ଫେରନ୍ତୁ std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  ଯେତେବେଳେ (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    ଯଦି (Split.second.empty()) {
      ଯଦି (Repl.size() != Split.first.size() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      ଭାଙ୍ଗ;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    ବାଛ (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    ସ୍ଥିତି 'g':
      ଯଦି (Repl.size() >= 4 && Repl[1] == '<') {
        size_t End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        unsigned Ref_ମୂଲ୍ୟ;
        ଯଦି (End != StringRef::npos && !Ref.getAsInteger(10, Ref_ମୂଲ୍ୟ)) {
          Repl = Repl.substr(End + 1);
          ଯଦି (Ref_ମୂଲ୍ୟ < Matches.size())
            Res += Matches[Ref_ମୂଲ୍ୟ];
          ଅନ୍ୟଥା ଯଦି (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          ଭାଙ୍ଗ;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      Res += Repl[0];
      Repl = Repl.substr(1);
      ଭାଙ୍ଗ;

      // Single character escapes.
    ସ୍ଥିତି 't':
      Res += '\t';
      Repl = Repl.substr(1);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      ଭାଙ୍ଗ;

      // Hex character escapes.
    ସ୍ଥିତି 'x':
      ଯଦି (Repl.size() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += ଅକ୍ଷର(hex_Digit_ମୂଲ୍ୟ(Repl[1]) * 16 + hex_Digit_ମୂଲ୍ୟ(Repl[2]));
        Repl = Repl.substr(3);
      } ଅନ୍ୟଥା {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      ଭାଙ୍ଗ;

      // Decimal escapes are backreferences.
    ସ୍ଥିତି '0': ସ୍ଥିତି '1': ସ୍ଥିତି '2': ସ୍ଥିତି '3': ସ୍ଥିତି '4':
    ସ୍ଥିତି '5': ସ୍ଥିତି '6': ସ୍ଥିତି '7': ସ୍ଥିତି '8': ସ୍ଥିତି '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.size());

      unsigned Ref_ମୂଲ୍ୟ;
      ଯଦି (!Ref.getAsInteger(10, Ref_ମୂଲ୍ୟ) &&
          Ref_ମୂଲ୍ୟ < Matches.size())
        Res += Matches[Ref_ମୂଲ୍ୟ];
      ଅନ୍ୟଥା ଯଦି (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      ଭାଙ୍ଗ;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  ଫେରନ୍ତୁ Res;
}

// These are the special characters matched in functions like "p_ere_exp".
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର RegexMetachars[] = "()^$|*+?.[]\\{}";

ତାର୍କିକ Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  ଫେରନ୍ତୁ Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  ପାଇଁ (ଅକ୍ଷର C : String) {
    ଯଦି (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  ଫେରନ୍ତୁ RegexStr;
}
