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

#అన్తర్భావ "llvm/Support/Regex.h"
#అన్తర్భావ "regex_impl.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/ADT/Twine.h"

#అన్తర్భావ <cassert>
#అన్తర్భావ <string>

ఉపయోగించి నామాకాశ llvm;

Regex::Regex() : preg(శూన్యసూచిక), error(REG_BADPAT) {}

Regex::Regex(StringRef regex, RegexFlags Flags) {
  గుర్తులేని flags = 0;
  preg = కొత్త llvm_regex();
  preg->re_endp = regex.చివర();
  ఒకవేళ (Flags & IgnoreCase)
    flags |= REG_ICASE;
  ఒకవేళ (Flags & Newline)
    flags |= REG_NEWLINE;
  ఒకవేళ (!(Flags & BasicRegex))
    flags |= REG_EXTENDED;
  error = llvm_regcomp(preg, regex.దత్తాంశం(), flags|REG_PEND);
}

Regex::Regex(StringRef regex, గుర్తులేని Flags)
    : Regex(regex, static_cast<RegexFlags>(Flags)) {}

Regex::Regex(Regex &&regex) {
  preg = regex.preg;
  error = regex.error;
  regex.preg = శూన్యసూచిక;
  regex.error = REG_BADPAT;
}

Regex::~Regex() {
  ఒకవేళ (preg) {
    llvm_regfree(preg);
    తొలగించు preg;
  }
}

నామాకాశ {

/// Utility to convert a regex error code into a human-readable string.
శూన్య Regex_Error_గమ్యం_String(పూర్ణాంకం error, నిర్మాణం llvm_regex *preg,
                        std::string &Error) {
  size_t len = llvm_regerror(error, preg, శూన్యసూచిక, 0);

  Error.resize(len - 1);
  llvm_regerror(error, preg, &Error[0], len);
}

} // namespace

తార్కిక Regex::isValid(std::string &Error) స్థిరరాశి {
  ఒకవేళ (!error)
    తిరిగి నిజం;

  Regex_Error_గమ్యం_String(error, preg, Error);
  తిరిగి అబద్ధం;
}

/// getNumMatches - In a valid regex, return the number of parenthesized
/// matches it contains.
గుర్తులేని Regex::get_అంకె_Matches() స్థిరరాశి {
  తిరిగి preg->re_nsub;
}

తార్కిక Regex::సరిపోలిక(StringRef String, Small_సదిశ_Impl<StringRef> *Matches,
                  std::string *Error) స్థిరరాశి {
  // Reset error, if given.
  ఒకవేళ (Error && !Error->ఖాళీలు())
    *Error = "";

  // Check if the regex itself didn't successfully compile.
  ఒకవేళ (Error ? !isValid(*Error) : !isValid())
    తిరిగి అబద్ధం;

  గుర్తులేని nmatch = Matches ? preg->re_nsub+1 : 0;

  // Update null string to empty string.
  ఒకవేళ (String.దత్తాంశం() == శూన్యసూచిక)
    String = "";

  // pmatch needs to have at least one element.
  Small_సదిశ<llvm_regmatch_t, 8> pm;
  pm.resize(nmatch > 0 ? nmatch : 1);
  pm[0].rm_so = 0;
  pm[0].rm_eo = String.కొలత_2();

  పూర్ణాంకం rc = llvm_regexec(preg, String.దత్తాంశం(), nmatch, pm.దత్తాంశం(), REG_STARTEND);

  // Failure to match is not an error, it's just a normal return value.
  // Any other error code is considered abnormal, and is logged in the Error.
  ఒకవేళ (rc == REG_NOMATCH)
    తిరిగి అబద్ధం;
  ఒకవేళ (rc != 0) {
    ఒకవేళ (Error)
      Regex_Error_గమ్యం_String(error, preg, *Error);
    తిరిగి అబద్ధం;
  }

  // There was a match.

  ఒకవేళ (Matches) { // match position requested
    Matches->clear();

    కొరకు (గుర్తులేని i = 0; i != nmatch; ++i) {
      ఒకవేళ (pm[i].rm_so == -1) {
        // this group didn't match
        Matches->నెట్టు_వెనుక(StringRef());
        కొనసాగు;
      }
      ప్రతిజ్ఞా(pm[i].rm_eo >= pm[i].rm_so);
      Matches->నెట్టు_వెనుక(StringRef(String.దత్తాంశం()+pm[i].rm_so,
                                   pm[i].rm_eo-pm[i].rm_so));
    }
  }

  తిరిగి నిజం;
}

std::string Regex::sub(StringRef Repl, StringRef String,
                       std::string *Error) స్థిరరాశి {
  Small_సదిశ<StringRef, 8> Matches;

  // Return the input if there was no match.
  ఒకవేళ (!సరిపోలిక(String, &Matches, Error))
    తిరిగి std::string(String);

  // Otherwise splice in the replacement string, starting with the prefix before
  // the match.
  std::string Res(String.begin(), Matches[0].begin());

  // Then the replacement string, honoring possible substitutions.
  అయితే (!Repl.ఖాళీలు()) {
    // Skip to the next escape.
    std::pair<StringRef, StringRef> విభజించు_2 = Repl.విభజించు('\\');

    // Add the skipped substring.
    Res += విభజించు_2.మొదటి_2;

    // Check for termination and trailing backslash.
    ఒకవేళ (విభజించు_2.రెండవ_2.ఖాళీలు()) {
      ఒకవేళ (Repl.కొలత_2() != విభజించు_2.మొదటి_2.కొలత_2() &&
          Error && Error->ఖాళీలు())
        *Error = "replacement string contained trailing backslash";
      ఆపు;
    }

    // Otherwise update the replacement string and interpret escapes.
    Repl = విభజించు_2.రెండవ_2;

    // FIXME: We should have a StringExtras function for mapping C99 escapes.
    ఎంపిక (Repl[0]) {

      // Backreference with the "\g<ref>" syntax
    స్థితి 'g':
      ఒకవేళ (Repl.కొలత_2() >= 4 && Repl[1] == '<') {
        size_t చివర_2 = Repl.కనుగొను('>');
        StringRef Ref = Repl.ముక్క(2, చివర_2);
        గుర్తులేని Ref_విలువ;
        ఒకవేళ (చివర_2 != StringRef::npos && !Ref.getAsInteger(10, Ref_విలువ)) {
          Repl = Repl.substr(చివర_2 + 1);
          ఒకవేళ (Ref_విలువ < Matches.కొలత_2())
            Res += Matches[Ref_విలువ];
          లేకపోతే ఒకవేళ (Error && Error->ఖాళీలు())
            *Error =
                ("invalid backreference string 'g<" + Twine(Ref) + ">'").str();
          ఆపు;
        }
      }
      [[fallthrough]];

      // Treat all unrecognized characters as self-quoting.
    అప్రమేయ:
      Res += Repl[0];
      Repl = Repl.substr(1);
      ఆపు;

      // Single character escapes.
    స్థితి 't':
      Res += '\t';
      Repl = Repl.substr(1);
      ఆపు;
    స్థితి 'n':
      Res += '\n';
      Repl = Repl.substr(1);
      ఆపు;

      // Hex character escapes.
    స్థితి 'x':
      ఒకవేళ (Repl.కొలత_2() >= 3 && isHexDigit(Repl[1]) && isHexDigit(Repl[2])) {
        Res += అక్షరం(hex_Digit_విలువ(Repl[1]) * 16 + hex_Digit_విలువ(Repl[2]));
        Repl = Repl.substr(3);
      } లేకపోతే {
        Res += Repl[0];
        Repl = Repl.substr(1);
      }
      ఆపు;

      // Decimal escapes are backreferences.
    స్థితి '0': స్థితి '1': స్థితి '2': స్థితి '3': స్థితి '4':
    స్థితి '5': స్థితి '6': స్థితి '7': స్థితి '8': స్థితి '9': {
      // Extract the backreference number.
      StringRef Ref = Repl.ముక్క(0, Repl.కనుగొను_మొదటి_not_యొక్క("0123456789"));
      Repl = Repl.substr(Ref.కొలత_2());

      గుర్తులేని Ref_విలువ;
      ఒకవేళ (!Ref.getAsInteger(10, Ref_విలువ) &&
          Ref_విలువ < Matches.కొలత_2())
        Res += Matches[Ref_విలువ];
      లేకపోతే ఒకవేళ (Error && Error->ఖాళీలు())
        *Error = ("invalid backreference string '" + Twine(Ref) + "'").str();
      ఆపు;
    }
    }
  }

  // And finally the suffix.
  Res += StringRef(Matches[0].చివర(), String.చివర() - Matches[0].చివర());

  తిరిగి Res;
}

// These are the special characters matched in functions like "p_ere_exp".
స్థిర స్థిరరాశి అక్షరం RegexMetachars[] = "()^$|*+?.[]\\{}";

తార్కిక Regex::isLiteralERE(StringRef Str) {
  // Check for regex metacharacters.  This list was derived from our regex
  // implementation in regcomp.c and double checked against the POSIX extended
  // regular expression specification.
  తిరిగి Str.కనుగొను_మొదటి_యొక్క(RegexMetachars) == StringRef::npos;
}

std::string Regex::escape(StringRef String) {
  std::string RegexStr;
  కొరకు (అక్షరం C : String) {
    ఒకవేళ (strchr(RegexMetachars, C))
      RegexStr += '\\';
    RegexStr += C;
  }

  తిరిగి RegexStr;
}
