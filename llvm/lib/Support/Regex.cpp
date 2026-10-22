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

#include "llvm/Support/Regex.h"
#include "regex_impl.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"

#include <cassert>
#include <string>

using namespace llvm;

Regex::Regex() : preg(nullptr), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  unsigned flags = 0;
  preg = new llvm_regex();
  preg->re_endp = regex.end();
  ຖ້າ (Flags & IgnoreCase)
    flags |= REG_ICASE;
  ຖ້າ (Flags & Newline)
    flags |= REG_NEWLINE;
  ຖ້າ (!(Flags & BasicRegex))
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
  ຖ້າ (preg) {
    llvm_regfree(preg);
    delete preg;
  }
}

namespace {

/// Utility to convert a regex error code into a human-readable string.
void RegexErrorToString(ຈຳນວນເຕັມ error, sa_nracanaa llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, nullptr, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

bool Regex::isValid(std::string &Error) const {
  ຖ້າ (!error)
    ສົ່ງຄືນ true;

  RegexErrorToString(error, preg, Error);
  ສົ່ງຄືນ false;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
unsigned Regex::getNumMatches() const {
  ສົ່ງຄືນ preg->re_nsub;
}

bool Regex::match(StringRef String, SmallVectorImpl<StringRef> *Matches,
                  std::string *Error) const {
  // Reset error, if given.
  ຖ້າ (Error && !Error->empty())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  ຖ້າ (Error ? !isValid(*Error) : !isValid())
    ສົ່ງຄືນ false;

  unsigned nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  ຖ້າ (String.data() == nullptr)
    String = "";

  // pmatch needs to have at least one element.
  SmallVector<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.size();

  ຈຳນວນເຕັມ rc = llvm_regexec(preg, String.data(), nmatch, pm.data(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  ຖ້າ (rc == REG_NOMATCH)
    ສົ່ງຄືນ false;
  ຖ້າ (rc != 0) {
    ຖ້າ (Error)
      RegexErrorToString(error, preg, *Error);
    ສົ່ງຄືນ false;
  }

  // There was a match.

  ຖ້າ (Matches) { // match position requested
    Matches->clear();

    ສຳລັບ (unsigned i = 0; i != nmatch; ++i) {
      ຖ້າ (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->push_back(StringRef());
        continue;
      }
      assert(pm[i].rm_eo >= pm[i].rm_so);
      Matches->push_back(StringRef(String.data()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  ສົ່ງຄືນ true;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) const {
  SmallVector<StringRef, 8> Matches;

  // Return the input if there was no match.
  ຖ້າ (!match(String, &Matches, Error))
    ສົ່ງຄືນ std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  ໃນຂະນະທີ່ (!Repl.empty()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> Split = Repl.split('\\');

    // Add the skipped substring.
    Res += Split.first;

    // Check for termination and trailing backslash.
    ຖ້າ (Split.second.empty()) {
      ຖ້າ (Repl.size() != Split.first.size() &&
          Error && Error->empty())
        *Error = "replacement string contained trailing backslash";
      bha_nga;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = Split.second;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    switch (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    pak_sa 'g':
      ຖ້າ (Repl.size() >= 4 && Repl[1] == '<') {
        size_t End = Repl.find('>');
        StringRef Ref = Repl.slice(2, End);
        unsigned RefValue;
        ຖ້າ (End != StringRef::npos && !Ref.getAsInteger(10, RefValue)) {
          Repl = Repl.substr(End + 1);
          ຖ້າ (RefValue < Matches.size())
            Res += Matches[RefValue];
          ບໍ່ດັ່ງນັ້ນ ຖ້າ (Error && Error->empty())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          bha_nga;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    default:
      Res += Repl[0];
      Repl = Repl.substr(1);
      bha_nga;

      // Single character escapes.
    pak_sa 't':
      Res += '\t';
      Repl = Repl.substr(1);
      bha_nga;
    pak_sa 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      bha_nga;

      // Hex character escapes.
    pak_sa 'x':
      ຖ້າ (Repl.size() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += var_na(hexDigitValue(Repl[1]) * 16 + hexDigitValue(Repl[2]));
        Repl = Repl.substr(3);
      } ບໍ່ດັ່ງນັ້ນ {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      bha_nga;

      // Decimal escapes are backreferences.
    pak_sa '0': pak_sa '1': pak_sa '2': pak_sa '3': pak_sa '4':
    pak_sa '5': pak_sa '6': pak_sa '7': pak_sa '8': pak_sa '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.slice(0, Repl.find_first_not_of("0123456789"));
      Repl = Repl.substr(Ref.size());

      unsigned RefValue;
      ຖ້າ (!Ref.getAsInteger(10, RefValue) &&
          RefValue < Matches.size())
        Res += Matches[RefValue];
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (Error && Error->empty())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      bha_nga;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].end(), String.end() - Matches[0].end());

  ສົ່ງຄືນ Res;
}

// These are the special characters matched in functions like "p_ere_exp".
static const var_na RegexMetachars[] = "()^$|*+?.[]\\{}";

bool Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  ສົ່ງຄືນ Str.find_first_of(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  ສຳລັບ (var_na C : String) {
    ຖ້າ (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  ສົ່ງຄືນ RegexStr;
}
