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

استفاده نطاق llvm;

/// Given a diagnostic kind, return a human readable string for it.
ساكن StringRef get_Diag_نوع_Str(منبع_Mgr::Diag_نوع نوع_2) {
  بدل (نوع_2) {
  حالة منبع_Mgr::DK_Note:
    أرجع "note";
  حالة منبع_Mgr::DK_Warning:
    أرجع "warning";
  حالة منبع_Mgr::DK_Error:
    أرجع "error";
  حالة منبع_Mgr::DK_Remark:
    أرجع "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

منطقي منبع_Mgr_Diagnostic_Verifier::ExpectedDiag::emitError(
    raw_ostream &os, منبع_Mgr &mgr, ثابت Twine &پیام) ثابت {
  إذا (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::get_From_اشاره_گر(fileLoc.get_اشاره_گر() +
                                                 substring.اندازه_2()));
    mgr.Print_پیام(os, fileLoc, منبع_Mgr::DK_Error, پیام, range);
  } وإلا {
    mgr.Print_پیام(os, fileLoc, منبع_Mgr::DK_Error, پیام);
  }
  أرجع خطأ;
}

منطقي منبع_Mgr_Diagnostic_Verifier::ExpectedDiag::تطابق(StringRef str) ثابت {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  إذا (substringRegex)
    أرجع substringRegex->تطابق(str);
  أرجع str.شامل_است(substring);
}

منطقي منبع_Mgr_Diagnostic_Verifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             منبع_Mgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef str_مقصد_Process = substring;
  طالما (!str_مقصد_Process.خالی()) {
    // Find the next regex block.
    size_t regexIt = str_مقصد_Process.پیدا_کن("{{");
    إذا (regexIt == StringRef::npos) {
      regexOS << Regex::escape(str_مقصد_Process);
      اكسر;
    }
    regexOS << Regex::escape(str_مقصد_Process.take_front(regexIt));
    str_مقصد_Process = str_مقصد_Process.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regex_انتها_It = str_مقصد_Process.پیدا_کن("}}");
    إذا (regex_انتها_It == StringRef::npos)
      أرجع emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regex_بلوک = str_مقصد_Process.take_front(regex_انتها_It);

    // Validate that the regex is actually valid.
    std::string regexError;
    إذا (!Regex(regex_بلوک).isValid(regexError))
      أرجع emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regex_بلوک << ')';
    str_مقصد_Process = str_مقصد_Process.drop_front(regex_انتها_It + 2);
  }
  substringRegex = Regex(regexStr);
  أرجع درست;
}

MutableArrayRef<منبع_Mgr_Diagnostic_Verifier::ExpectedDiag>
منبع_Mgr_Diagnostic_Verifier::computeExpectedDiags(raw_ostream &os,
                                                  منبع_Mgr &mgr,
                                                  ثابت حافظه_میانگیر *buf) {
  // If the buffer is invalid, return an empty list.
  إذا (!buf)
    أرجع {};
  خودکار &expectedDiags = expected_Diags_در_هر_File[buf->get_میانگیر_Identifier()];

  // The number of the last line that did not correlate to a designator.
  بیعلامت آخرین_Non_Designator_سطر = 0;

  // The indices of designators that apply to the next non designator line.
  Small_وکتور<بیعلامت, 1> designators_For_بعدی_سطر;

  // Scan the file for expected-* designators.
  Small_وکتور<StringRef, 100> سطرها_2;
  buf->get_میانگیر().تقسیم(سطرها_2, '\n');
  لكل (بیعلامت سطر_No_2 = 0, e = سطرها_2.اندازه_2(); سطر_No_2 < e; ++سطر_No_2) {
    Small_وکتور<StringRef, 4> matches;
    إذا (!expected.تطابق(سطرها_2[سطر_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      إذا (!designators_For_بعدی_سطر.خالی()) {
        لكل (بیعلامت diag_شاخص : designators_For_بعدی_سطر)
          expectedDiags[diag_شاخص].سطر_No_2 = سطر_No_2 + 1;
        designators_For_بعدی_سطر.clear();
      }
      آخرین_Non_Designator_سطر = سطر_No_2;
      استمر;
    }

    // Point to the start of expected-*.
    SMLoc expected_شروع = SMLoc::get_From_اشاره_گر(matches[0].داده());

    منبع_Mgr::Diag_نوع نوع_2;
    إذا (matches[1] == "error")
      نوع_2 = منبع_Mgr::DK_Error;
    وإلا إذا (matches[1] == "warning")
      نوع_2 = منبع_Mgr::DK_Warning;
    وإلا إذا (matches[1] == "remark")
      نوع_2 = منبع_Mgr::DK_Remark;
    وإلا {
      أكد(matches[1] == "note");
      نوع_2 = منبع_Mgr::DK_Note;
    }
    ExpectedDiag رکورد_2(نوع_2, سطر_No_2 + 1, expected_شروع, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    إذا (!matches[2].خالی() && !رکورد_2.computeRegex(os, mgr)) {
      ok = خطأ;
      استمر;
    }

    StringRef جابه_جایی_تطابق = matches[3];
    إذا (!جابه_جایی_تطابق.خالی()) {
      جابه_جایی_تطابق = جابه_جایی_تطابق.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      إذا (جابه_جایی_تطابق[0] == '+' || جابه_جایی_تطابق[0] == '-') {
        صحيح جابه_جایی_2;
        جابه_جایی_تطابق.drop_front().getAsInteger(0, جابه_جایی_2);

        إذا (جابه_جایی_تطابق.front() == '+')
          رکورد_2.سطر_No_2 += جابه_جایی_2;
        وإلا
          رکورد_2.سطر_No_2 -= جابه_جایی_2;
      } وإلا إذا (جابه_جایی_تطابق.consume_front("unknown")) {
        // This is matching unknown locations.
        رکورد_2.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_پشت(std::move(رکورد_2));
        استمر;
      } وإلا إذا (جابه_جایی_تطابق.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        رکورد_2.سطر_No_2 = آخرین_Non_Designator_سطر + 1;
      } وإلا {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        أكد(جابه_جایی_تطابق.consume_front("below"));
        designators_For_بعدی_سطر.افزودن_پشت(expectedDiags.اندازه_2());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        رکورد_2.سطر_No_2 = e;
      }
    }
    expectedDiags.emplace_پشت(std::move(رکورد_2));
  }
  أرجع expectedDiags;
}

std::optional<MutableArrayRef<منبع_Mgr_Diagnostic_Verifier::ExpectedDiag>>
منبع_Mgr_Diagnostic_Verifier::getExpectedDiags(StringRef bufName) {
  خودکار expectedDiags = expected_Diags_در_هر_File.پیدا_کن(bufName);
  إذا (expectedDiags != expected_Diags_در_هر_File.انتها())
    أرجع MutableArrayRef<ExpectedDiag>(expectedDiags->دومی_2);
  أرجع std::nullopt;
}

منبع_Mgr_Diagnostic_Verifier::تطابق_نتیجه
منبع_Mgr_Diagnostic_Verifier::process(raw_ostream &os, منبع_Mgr &mgr,
                                     منبع_Mgr::Diag_نوع نوع_2, منطقي hasLoc,
                                     ثابت حافظه_میانگیر *buf, بیعلامت سطر_No_2,
                                     StringRef پیام_2, منطقي reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  إذا (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    إذا (buf) {
      إذا (خودکار maybeDiags = getExpectedDiags(buf->get_میانگیر_Identifier()))
        diags = *maybeDiags;
      وإلا
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } وإلا {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *near_نیافتن = اشاره_تهی;

  // If this was an expected error, remember that we saw it and return.
  لكل (خودکار &e : diags) {
    // File line must match (unless it's an unknown location).
    إذا (hasLoc && e.سطر_No_2 != سطر_No_2)
      استمر;
    إذا (e.تطابق(پیام_2)) {
      إذا (e.نوع_2 == نوع_2) {
        e.matched = درست;
        أرجع تطابق_نتیجه::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      near_نیافتن = &e;
    }
  }

  إذا (!reportUnexpected)
    أرجع تطابق_نتیجه::Ignored;

  ok = خطأ;

  // Otherwise, emit an error for the near miss.
  إذا (near_نیافتن) {
    mgr.Print_پیام(os, near_نیافتن->fileLoc, منبع_Mgr::DK_Error,
                     "'" + get_Diag_نوع_Str(نوع_2) +
                         "' diagnostic emitted when expecting a '" +
                         get_Diag_نوع_Str(near_نیافتن->نوع_2) + "'");
    أرجع تطابق_نتیجه::Near_نیافتن;
  }
  أرجع تطابق_نتیجه::Unexpected;
}

منطقي منبع_Mgr_Diagnostic_Verifier::verify(raw_ostream &os, منبع_Mgr &mgr) {
  // Verify that all expected errors were seen.
  خودکار checkExpectedDiags = [&](ExpectedDiag &diag) {
    إذا (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + get_Diag_نوع_Str(diag.نوع_2) + " \"" +
                         diag.substring + "\" was not produced");
      ok = خطأ;
    }
  };
  لكل (خودکار &expectedDiagsPair : expected_Diags_در_هر_File)
    لكل (خودکار &diag : expectedDiagsPair.دومی_2)
      checkExpectedDiags(diag);
  لكل (خودکار &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expected_Diags_در_هر_File.clear();
  أرجع ok;
}
