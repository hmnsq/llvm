//===- SourceMgrDiagnosticVerifier.cpp -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/SourceMgrDiagnosticVerifier.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/raw_ostream.h"

استعمال نطاق llvm;

/// Given a diagnostic kind, return a human readable string for it.
ساكن StringRef get_Diag_قسم_Str(ذريعو_Mgr::Diag_قسم قسم) {
  بدل (قسم) {
  حالة ذريعو_Mgr::DK_Note:
    أرجع "note";
  حالة ذريعو_Mgr::DK_Warning:
    أرجع "warning";
  حالة ذريعو_Mgr::DK_Error:
    أرجع "error";
  حالة ذريعو_Mgr::DK_Remark:
    أرجع "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

منطقي ذريعو_Mgr_Diagnostic_Verifier::ExpectedDiag::emitError(
    raw_ostream &os, ذريعو_Mgr &mgr, ثابت Twine &نياپو) ثابت {
  إذا (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::get_From_اشارو(fileLoc.get_اشارو() +
                                                 substring.سائيز_2()));
    mgr.Print_پيغام(os, fileLoc, ذريعو_Mgr::DK_Error, نياپو, range);
  } وإلا {
    mgr.Print_پيغام(os, fileLoc, ذريعو_Mgr::DK_Error, نياپو);
  }
  أرجع خطأ;
}

منطقي ذريعو_Mgr_Diagnostic_Verifier::ExpectedDiag::ميلاپ(StringRef str) ثابت {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  إذا (substringRegex)
    أرجع substringRegex->ميلاپ(str);
  أرجع str.شامل_آهي(substring);
}

منطقي ذريعو_Mgr_Diagnostic_Verifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             ذريعو_Mgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef str_منزل_Process = substring;
  طالما (!str_منزل_Process.خالي_جايون()) {
    // Find the next regex block.
    size_t regexIt = str_منزل_Process.ڳوليو("{{");
    إذا (regexIt == StringRef::npos) {
      regexOS << Regex::escape(str_منزل_Process);
      اكسر;
    }
    regexOS << Regex::escape(str_منزل_Process.take_front(regexIt));
    str_منزل_Process = str_منزل_Process.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regex_پڇاڙي_It = str_منزل_Process.ڳوليو("}}");
    إذا (regex_پڇاڙي_It == StringRef::npos)
      أرجع emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regex_بلاڪ = str_منزل_Process.take_front(regex_پڇاڙي_It);

    // Validate that the regex is actually valid.
    std::string regexError;
    إذا (!Regex(regex_بلاڪ).isValid(regexError))
      أرجع emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regex_بلاڪ << ')';
    str_منزل_Process = str_منزل_Process.drop_front(regex_پڇاڙي_It + 2);
  }
  substringRegex = Regex(regexStr);
  أرجع سچ;
}

MutableArrayRef<ذريعو_Mgr_Diagnostic_Verifier::ExpectedDiag>
ذريعو_Mgr_Diagnostic_Verifier::computeExpectedDiags(raw_ostream &os,
                                                  ذريعو_Mgr &mgr,
                                                  ثابت يادگيري_بفر *buf) {
  // If the buffer is invalid, return an empty list.
  إذا (!buf)
    أرجع {};
  خودڪار &expectedDiags = expected_Diags_في_File[buf->get_بفر_Identifier()];

  // The number of the last line that did not correlate to a designator.
  بنانشان آخري_Non_Designator_سٽ = 0;

  // The indices of designators that apply to the next non designator line.
  Small_ويڪٽر<بنانشان, 1> designators_For_ايندڙ_سٽ;

  // Scan the file for expected-* designators.
  Small_ويڪٽر<StringRef, 100> سٽون_2;
  buf->get_بفر().ورهايو(سٽون_2, '\n');
  لكل (بنانشان سٽ_No_2 = 0, e = سٽون_2.سائيز_2(); سٽ_No_2 < e; ++سٽ_No_2) {
    Small_ويڪٽر<StringRef, 4> matches;
    إذا (!expected.ميلاپ(سٽون_2[سٽ_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      إذا (!designators_For_ايندڙ_سٽ.خالي_جايون()) {
        لكل (بنانشان diag_اشاريو : designators_For_ايندڙ_سٽ)
          expectedDiags[diag_اشاريو].سٽ_No_2 = سٽ_No_2 + 1;
        designators_For_ايندڙ_سٽ.clear();
      }
      آخري_Non_Designator_سٽ = سٽ_No_2;
      استمر;
    }

    // Point to the start of expected-*.
    SMLoc expected_شروعات = SMLoc::get_From_اشارو(matches[0].معلومات());

    ذريعو_Mgr::Diag_قسم قسم;
    إذا (matches[1] == "error")
      قسم = ذريعو_Mgr::DK_Error;
    وإلا إذا (matches[1] == "warning")
      قسم = ذريعو_Mgr::DK_Warning;
    وإلا إذا (matches[1] == "remark")
      قسم = ذريعو_Mgr::DK_Remark;
    وإلا {
      أكد(matches[1] == "note");
      قسم = ذريعو_Mgr::DK_Note;
    }
    ExpectedDiag رڪارڊ_2(قسم, سٽ_No_2 + 1, expected_شروعات, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    إذا (!matches[2].خالي_جايون() && !رڪارڊ_2.computeRegex(os, mgr)) {
      ok = خطأ;
      استمر;
    }

    StringRef وٿي_ميلاپ = matches[3];
    إذا (!وٿي_ميلاپ.خالي_جايون()) {
      وٿي_ميلاپ = وٿي_ميلاپ.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      إذا (وٿي_ميلاپ[0] == '+' || وٿي_ميلاپ[0] == '-') {
        صحيح وٿي_2;
        وٿي_ميلاپ.drop_front().getAsInteger(0, وٿي_2);

        إذا (وٿي_ميلاپ.front() == '+')
          رڪارڊ_2.سٽ_No_2 += وٿي_2;
        وإلا
          رڪارڊ_2.سٽ_No_2 -= وٿي_2;
      } وإلا إذا (وٿي_ميلاپ.consume_front("unknown")) {
        // This is matching unknown locations.
        رڪارڊ_2.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_پٺ(std::move(رڪارڊ_2));
        استمر;
      } وإلا إذا (وٿي_ميلاپ.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        رڪارڊ_2.سٽ_No_2 = آخري_Non_Designator_سٽ + 1;
      } وإلا {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        أكد(وٿي_ميلاپ.consume_front("below"));
        designators_For_ايندڙ_سٽ.ڌڪو_پٺ(expectedDiags.سائيز_2());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        رڪارڊ_2.سٽ_No_2 = e;
      }
    }
    expectedDiags.emplace_پٺ(std::move(رڪارڊ_2));
  }
  أرجع expectedDiags;
}

std::optional<MutableArrayRef<ذريعو_Mgr_Diagnostic_Verifier::ExpectedDiag>>
ذريعو_Mgr_Diagnostic_Verifier::getExpectedDiags(StringRef bufName) {
  خودڪار expectedDiags = expected_Diags_في_File.ڳوليو(bufName);
  إذا (expectedDiags != expected_Diags_في_File.پڇاڙي())
    أرجع MutableArrayRef<ExpectedDiag>(expectedDiags->ٻيو_2);
  أرجع std::nullopt;
}

ذريعو_Mgr_Diagnostic_Verifier::ميلاپ_نتيجو
ذريعو_Mgr_Diagnostic_Verifier::process(raw_ostream &os, ذريعو_Mgr &mgr,
                                     ذريعو_Mgr::Diag_قسم قسم, منطقي hasLoc,
                                     ثابت يادگيري_بفر *buf, بنانشان سٽ_No_2,
                                     StringRef پيغام, منطقي reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  إذا (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    إذا (buf) {
      إذا (خودڪار maybeDiags = getExpectedDiags(buf->get_بفر_Identifier()))
        diags = *maybeDiags;
      وإلا
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } وإلا {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *near_ناڪامي = خالي_اشارو;

  // If this was an expected error, remember that we saw it and return.
  لكل (خودڪار &e : diags) {
    // File line must match (unless it's an unknown location).
    إذا (hasLoc && e.سٽ_No_2 != سٽ_No_2)
      استمر;
    إذا (e.ميلاپ(پيغام)) {
      إذا (e.قسم == قسم) {
        e.matched = سچ;
        أرجع ميلاپ_نتيجو::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      near_ناڪامي = &e;
    }
  }

  إذا (!reportUnexpected)
    أرجع ميلاپ_نتيجو::Ignored;

  ok = خطأ;

  // Otherwise, emit an error for the near miss.
  إذا (near_ناڪامي) {
    mgr.Print_پيغام(os, near_ناڪامي->fileLoc, ذريعو_Mgr::DK_Error,
                     "'" + get_Diag_قسم_Str(قسم) +
                         "' diagnostic emitted when expecting a '" +
                         get_Diag_قسم_Str(near_ناڪامي->قسم) + "'");
    أرجع ميلاپ_نتيجو::Near_ناڪامي;
  }
  أرجع ميلاپ_نتيجو::Unexpected;
}

منطقي ذريعو_Mgr_Diagnostic_Verifier::verify(raw_ostream &os, ذريعو_Mgr &mgr) {
  // Verify that all expected errors were seen.
  خودڪار checkExpectedDiags = [&](ExpectedDiag &diag) {
    إذا (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + get_Diag_قسم_Str(diag.قسم) + " \"" +
                         diag.substring + "\" was not produced");
      ok = خطأ;
    }
  };
  لكل (خودڪار &expectedDiagsPair : expected_Diags_في_File)
    لكل (خودڪار &diag : expectedDiagsPair.ٻيو_2)
      checkExpectedDiags(diag);
  لكل (خودڪار &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expected_Diags_في_File.clear();
  أرجع ok;
}
