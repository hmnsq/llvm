//===- SourceMgrDiagnosticVerifier.cpp -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SourceMgrDiagnosticVerifier.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

/// Given a diagnostic kind, return a human readable string for it.
ਸਥਿਰ StringRef get_Diag_ਕਿਸਮ_Str(ਸਰੋਤ_Mgr::Diag_ਕਿਸਮ ਕਿਸਮ_2) {
  ਚੋਣ (ਕਿਸਮ_2) {
  ਹਾਲਤ ਸਰੋਤ_Mgr::DK_Note:
    ਵਾਪਸ "note";
  ਹਾਲਤ ਸਰੋਤ_Mgr::DK_Warning:
    ਵਾਪਸ "warning";
  ਹਾਲਤ ਸਰੋਤ_Mgr::DK_Error:
    ਵਾਪਸ "error";
  ਹਾਲਤ ਸਰੋਤ_Mgr::DK_Remark:
    ਵਾਪਸ "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

ਤਾਰਕਿਕ ਸਰੋਤ_Mgr_Diagnostic_Verifier::ExpectedDiag::emitError(
    raw_ostream &os, ਸਰੋਤ_Mgr &mgr, ਅਚਲ Twine &ਸੁਨੇਹਾ) ਅਚਲ {
  ਜੇ (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::get_From_ਸੂਚਕ(fileLoc.get_ਸੂਚਕ() +
                                                 substring.ਮਾਪ_2()));
    mgr.Print_ਸੁਨੇਹਾ(os, fileLoc, ਸਰੋਤ_Mgr::DK_Error, ਸੁਨੇਹਾ, range);
  } ਨਹੀਂਤਾਂ {
    mgr.Print_ਸੁਨੇਹਾ(os, fileLoc, ਸਰੋਤ_Mgr::DK_Error, ਸੁਨੇਹਾ);
  }
  ਵਾਪਸ ਝੂਠ;
}

ਤਾਰਕਿਕ ਸਰੋਤ_Mgr_Diagnostic_Verifier::ExpectedDiag::ਮੇਲ(StringRef str) ਅਚਲ {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  ਜੇ (substringRegex)
    ਵਾਪਸ substringRegex->ਮੇਲ(str);
  ਵਾਪਸ str.ਸ਼ਾਮਲ_ਹੈ(substring);
}

ਤਾਰਕਿਕ ਸਰੋਤ_Mgr_Diagnostic_Verifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             ਸਰੋਤ_Mgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef str_ਮੰਜ਼ਿਲ_Process = substring;
  ਜਦੋਂ (!str_ਮੰਜ਼ਿਲ_Process.ਖਾਲੀ_ਥਾਵਾਂ()) {
    // Find the next regex block.
    size_t regexIt = str_ਮੰਜ਼ਿਲ_Process.ਲੱਭੋ("{{");
    ਜੇ (regexIt == StringRef::npos) {
      regexOS << Regex::escape(str_ਮੰਜ਼ਿਲ_Process);
      ਰੋਕੋ;
    }
    regexOS << Regex::escape(str_ਮੰਜ਼ਿਲ_Process.take_front(regexIt));
    str_ਮੰਜ਼ਿਲ_Process = str_ਮੰਜ਼ਿਲ_Process.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regex_ਸਿਰਾ_It = str_ਮੰਜ਼ਿਲ_Process.ਲੱਭੋ("}}");
    ਜੇ (regex_ਸਿਰਾ_It == StringRef::npos)
      ਵਾਪਸ emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regex_ਖੰਡ = str_ਮੰਜ਼ਿਲ_Process.take_front(regex_ਸਿਰਾ_It);

    // Validate that the regex is actually valid.
    std::string regexError;
    ਜੇ (!Regex(regex_ਖੰਡ).isValid(regexError))
      ਵਾਪਸ emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regex_ਖੰਡ << ')';
    str_ਮੰਜ਼ਿਲ_Process = str_ਮੰਜ਼ਿਲ_Process.drop_front(regex_ਸਿਰਾ_It + 2);
  }
  substringRegex = Regex(regexStr);
  ਵਾਪਸ ਸੱਚ;
}

MutableArrayRef<ਸਰੋਤ_Mgr_Diagnostic_Verifier::ExpectedDiag>
ਸਰੋਤ_Mgr_Diagnostic_Verifier::computeExpectedDiags(raw_ostream &os,
                                                  ਸਰੋਤ_Mgr &mgr,
                                                  ਅਚਲ ਸਮ੍ਰਿਤੀ_ਬਫਰ *buf) {
  // If the buffer is invalid, return an empty list.
  ਜੇ (!buf)
    ਵਾਪਸ {};
  ਸਵੈਚਾਲਿਤ &expectedDiags = expected_Diags_ਪ੍ਰਤੀ_File[buf->get_ਬਫਰ_Identifier()];

  // The number of the last line that did not correlate to a designator.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਆਖਰੀ_Non_Designator_ਸਤਰ = 0;

  // The indices of designators that apply to the next non designator line.
  Small_ਸਦਿਸ਼<ਬਿਨਾਨਿਸ਼ਾਨੀ, 1> designators_For_ਅਗਲਾ_ਸਤਰ;

  // Scan the file for expected-* designators.
  Small_ਸਦਿਸ਼<StringRef, 100> ਸਤਰਾਂ_2;
  buf->get_ਬਫਰ().ਵੰਡੋ(ਸਤਰਾਂ_2, '\n');
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No_2 = 0, e = ਸਤਰਾਂ_2.ਮਾਪ_2(); ਸਤਰ_No_2 < e; ++ਸਤਰ_No_2) {
    Small_ਸਦਿਸ਼<StringRef, 4> matches;
    ਜੇ (!expected.ਮੇਲ(ਸਤਰਾਂ_2[ਸਤਰ_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      ਜੇ (!designators_For_ਅਗਲਾ_ਸਤਰ.ਖਾਲੀ_ਥਾਵਾਂ()) {
        ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ diag_ਸੂਚਕਾਂਕ : designators_For_ਅਗਲਾ_ਸਤਰ)
          expectedDiags[diag_ਸੂਚਕਾਂਕ].ਸਤਰ_No_2 = ਸਤਰ_No_2 + 1;
        designators_For_ਅਗਲਾ_ਸਤਰ.clear();
      }
      ਆਖਰੀ_Non_Designator_ਸਤਰ = ਸਤਰ_No_2;
      ਜਾਰੀ;
    }

    // Point to the start of expected-*.
    SMLoc expected_ਸ਼ੁਰੂ = SMLoc::get_From_ਸੂਚਕ(matches[0].ਅੰਕੜੇ());

    ਸਰੋਤ_Mgr::Diag_ਕਿਸਮ ਕਿਸਮ_2;
    ਜੇ (matches[1] == "error")
      ਕਿਸਮ_2 = ਸਰੋਤ_Mgr::DK_Error;
    ਨਹੀਂਤਾਂ ਜੇ (matches[1] == "warning")
      ਕਿਸਮ_2 = ਸਰੋਤ_Mgr::DK_Warning;
    ਨਹੀਂਤਾਂ ਜੇ (matches[1] == "remark")
      ਕਿਸਮ_2 = ਸਰੋਤ_Mgr::DK_Remark;
    ਨਹੀਂਤਾਂ {
      ਪ੍ਰਤਿਜ੍ਞਾ(matches[1] == "note");
      ਕਿਸਮ_2 = ਸਰੋਤ_Mgr::DK_Note;
    }
    ExpectedDiag ਰਿਕਾਰਡ_2(ਕਿਸਮ_2, ਸਤਰ_No_2 + 1, expected_ਸ਼ੁਰੂ, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    ਜੇ (!matches[2].ਖਾਲੀ_ਥਾਵਾਂ() && !ਰਿਕਾਰਡ_2.computeRegex(os, mgr)) {
      ok = ਝੂਠ;
      ਜਾਰੀ;
    }

    StringRef ਵਿੱਥ_ਮੇਲ = matches[3];
    ਜੇ (!ਵਿੱਥ_ਮੇਲ.ਖਾਲੀ_ਥਾਵਾਂ()) {
      ਵਿੱਥ_ਮੇਲ = ਵਿੱਥ_ਮੇਲ.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      ਜੇ (ਵਿੱਥ_ਮੇਲ[0] == '+' || ਵਿੱਥ_ਮੇਲ[0] == '-') {
        ਪੂਰਨ ਵਿੱਥ_2;
        ਵਿੱਥ_ਮੇਲ.drop_front().getAsInteger(0, ਵਿੱਥ_2);

        ਜੇ (ਵਿੱਥ_ਮੇਲ.front() == '+')
          ਰਿਕਾਰਡ_2.ਸਤਰ_No_2 += ਵਿੱਥ_2;
        ਨਹੀਂਤਾਂ
          ਰਿਕਾਰਡ_2.ਸਤਰ_No_2 -= ਵਿੱਥ_2;
      } ਨਹੀਂਤਾਂ ਜੇ (ਵਿੱਥ_ਮੇਲ.consume_front("unknown")) {
        // This is matching unknown locations.
        ਰਿਕਾਰਡ_2.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_ਪਿੱਠ(std::move(ਰਿਕਾਰਡ_2));
        ਜਾਰੀ;
      } ਨਹੀਂਤਾਂ ਜੇ (ਵਿੱਥ_ਮੇਲ.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        ਰਿਕਾਰਡ_2.ਸਤਰ_No_2 = ਆਖਰੀ_Non_Designator_ਸਤਰ + 1;
      } ਨਹੀਂਤਾਂ {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        ਪ੍ਰਤਿਜ੍ਞਾ(ਵਿੱਥ_ਮੇਲ.consume_front("below"));
        designators_For_ਅਗਲਾ_ਸਤਰ.ਧੱਕੋ_ਪਿੱਠ(expectedDiags.ਮਾਪ_2());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        ਰਿਕਾਰਡ_2.ਸਤਰ_No_2 = e;
      }
    }
    expectedDiags.emplace_ਪਿੱਠ(std::move(ਰਿਕਾਰਡ_2));
  }
  ਵਾਪਸ expectedDiags;
}

std::optional<MutableArrayRef<ਸਰੋਤ_Mgr_Diagnostic_Verifier::ExpectedDiag>>
ਸਰੋਤ_Mgr_Diagnostic_Verifier::getExpectedDiags(StringRef bufName) {
  ਸਵੈਚਾਲਿਤ expectedDiags = expected_Diags_ਪ੍ਰਤੀ_File.ਲੱਭੋ(bufName);
  ਜੇ (expectedDiags != expected_Diags_ਪ੍ਰਤੀ_File.ਸਿਰਾ())
    ਵਾਪਸ MutableArrayRef<ExpectedDiag>(expectedDiags->ਦੂਜਾ_2);
  ਵਾਪਸ std::nullopt;
}

ਸਰੋਤ_Mgr_Diagnostic_Verifier::ਮੇਲ_ਨਤੀਜਾ
ਸਰੋਤ_Mgr_Diagnostic_Verifier::process(raw_ostream &os, ਸਰੋਤ_Mgr &mgr,
                                     ਸਰੋਤ_Mgr::Diag_ਕਿਸਮ ਕਿਸਮ_2, ਤਾਰਕਿਕ hasLoc,
                                     ਅਚਲ ਸਮ੍ਰਿਤੀ_ਬਫਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_No_2,
                                     StringRef ਸੁਨੇਹਾ_2, ਤਾਰਕਿਕ reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  ਜੇ (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    ਜੇ (buf) {
      ਜੇ (ਸਵੈਚਾਲਿਤ maybeDiags = getExpectedDiags(buf->get_ਬਫਰ_Identifier()))
        diags = *maybeDiags;
      ਨਹੀਂਤਾਂ
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } ਨਹੀਂਤਾਂ {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *near_ਖੁੰਝ = ਖਾਲੀ_ਸੂਚਕ;

  // If this was an expected error, remember that we saw it and return.
  ਲਈ (ਸਵੈਚਾਲਿਤ &e : diags) {
    // File line must match (unless it's an unknown location).
    ਜੇ (hasLoc && e.ਸਤਰ_No_2 != ਸਤਰ_No_2)
      ਜਾਰੀ;
    ਜੇ (e.ਮੇਲ(ਸੁਨੇਹਾ_2)) {
      ਜੇ (e.ਕਿਸਮ_2 == ਕਿਸਮ_2) {
        e.matched = ਸੱਚ;
        ਵਾਪਸ ਮੇਲ_ਨਤੀਜਾ::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      near_ਖੁੰਝ = &e;
    }
  }

  ਜੇ (!reportUnexpected)
    ਵਾਪਸ ਮੇਲ_ਨਤੀਜਾ::Ignored;

  ok = ਝੂਠ;

  // Otherwise, emit an error for the near miss.
  ਜੇ (near_ਖੁੰਝ) {
    mgr.Print_ਸੁਨੇਹਾ(os, near_ਖੁੰਝ->fileLoc, ਸਰੋਤ_Mgr::DK_Error,
                     "'" + get_Diag_ਕਿਸਮ_Str(ਕਿਸਮ_2) +
                         "' diagnostic emitted when expecting a '" +
                         get_Diag_ਕਿਸਮ_Str(near_ਖੁੰਝ->ਕਿਸਮ_2) + "'");
    ਵਾਪਸ ਮੇਲ_ਨਤੀਜਾ::Near_ਖੁੰਝ;
  }
  ਵਾਪਸ ਮੇਲ_ਨਤੀਜਾ::Unexpected;
}

ਤਾਰਕਿਕ ਸਰੋਤ_Mgr_Diagnostic_Verifier::verify(raw_ostream &os, ਸਰੋਤ_Mgr &mgr) {
  // Verify that all expected errors were seen.
  ਸਵੈਚਾਲਿਤ checkExpectedDiags = [&](ExpectedDiag &diag) {
    ਜੇ (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + get_Diag_ਕਿਸਮ_Str(diag.ਕਿਸਮ_2) + " \"" +
                         diag.substring + "\" was not produced");
      ok = ਝੂਠ;
    }
  };
  ਲਈ (ਸਵੈਚਾਲਿਤ &expectedDiagsPair : expected_Diags_ਪ੍ਰਤੀ_File)
    ਲਈ (ਸਵੈਚਾਲਿਤ &diag : expectedDiagsPair.ਦੂਜਾ_2)
      checkExpectedDiags(diag);
  ਲਈ (ਸਵੈਚਾਲਿਤ &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expected_Diags_ਪ੍ਰਤੀ_File.clear();
  ਵਾਪਸ ok;
}
