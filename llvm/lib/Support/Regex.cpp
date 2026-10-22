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

#અન્તર્ભાવ "llvm/Support/Regex.h"
#અન્તર્ભાવ "regex_impl.h"
#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/ADT/Twine.h"

#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <string>

using નામાકાશ llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  unsigned flags = 0;
  preg = નવું llvm_regex();
  preg->re_endp = regex.end();
  જો (Flags & IgnoreCase)
    flags |= REG_ICASE;
  જો (Flags & Newline)
    flags |= REG_NEWLINE;
  જો (!(Flags & BasicRegex))
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
  જો (preg) {
    llvm_regfree(preg);
    delete preg;
  }
}

નામાકાશ {

/// Utility to convert a regex error code into a human-readable string.
શૂન્ય RegexErrorToString(પૂર્ણાંક error, સંરચના llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

તાર્કિક Regex::isValid(std::string &Error) અચલ {
  જો (!error)
    પાછા સાચું;

  RegexErrorToString(error, preg, Error);
  પાછા ખોટું;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
unsigned Regex::get_અંક_Matches() અચલ {
  પાછા preg->re_nsub;
}

તાર્કિક Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) અચલ {
  // Reset error, if given.
  જો (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  જો (Error ? !isValid(*Error) : !isValid())
    પાછા ખોટું;

  unsigned nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  જો (String.data() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.size();

  પૂર્ણાંક rc = llvm_regexec(preg, String.data(), nmatch, pm.data(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  જો (rc == REG_NOMATCH)
    પાછા ખોટું;
  જો (rc != 0) {
    જો (Error)
      RegexErrorToString(error, preg, *Error);
    પાછા ખોટું;
  }

  // There was a match.

  જો (Matches) { // match position requested
    Matches->clear();

    માટે (unsigned i = 0; i != nmatch; ++i) {
      જો (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->ધકેલો_back(StringRef());
        ચાલુ;
      }
      પ્રતિજ્ઞા(pm[i].rm_eo >= pm[i].rm_so);
      Matches->ધકેલો_back(StringRef(String.data()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  પાછા સાચું;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) અચલ {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  જો (!match(String, &Matches, Error))
    પાછા std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  જ્યારે (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    જો (Split.second.empty()) {
      જો (Repl.size() != Split.first.size() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      તોડો;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    પસંદગી (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    સ્થિતિ 'g':
      જો (Repl.size() >= 4 && Repl[1] == '<') {
        size_t End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        unsigned Ref_કિંમત;
        જો (End != StringRef::npos && !Ref.getAsInteger(10, Ref_કિંમત)) {
          Repl = Repl.substr(End + 1);
          જો (Ref_કિંમત < Matches.size())
            Res += Matches[Ref_કિંમત];
          નહીંતર જો (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          તોડો;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    મૂળભૂત:
      Res += Repl[0];
      Repl = Repl.substr(1);
      તોડો;

      // Single character escapes.
    સ્થિતિ 't':
      Res += '\t';
      Repl = Repl.substr(1);
      તોડો;
    સ્થિતિ 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      તોડો;

      // Hex character escapes.
    સ્થિતિ 'x':
      જો (Repl.size() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += અક્ષર(hex_Digit_કિંમત(Repl[1]) * 16 + hex_Digit_કિંમત(Repl[2]));
        Repl = Repl.substr(3);
      } નહીંતર {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      તોડો;

      // Decimal escapes are backreferences.
    સ્થિતિ '0': સ્થિતિ '1': સ્થિતિ '2': સ્થિતિ '3': સ્થિતિ '4':
    સ્થિતિ '5': સ્થિતિ '6': સ્થિતિ '7': સ્થિતિ '8': સ્થિતિ '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.size());

      unsigned Ref_કિંમત;
      જો (!Ref.getAsInteger(10, Ref_કિંમત) &&
          Ref_કિંમત < Matches.size())
        Res += Matches[Ref_કિંમત];
      નહીંતર જો (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      તોડો;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  પાછા Res;
}

// These are the special characters matched in functions like "p_ere_exp".
સ્થિર અચલ અક્ષર RegexMetachars[] = "()^$|*+?.[]\\{}";

તાર્કિક Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  પાછા Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  માટે (અક્ષર C : String) {
    જો (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  પાછા RegexStr;
}
