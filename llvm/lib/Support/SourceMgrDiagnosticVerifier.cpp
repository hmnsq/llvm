//===- SourceMgrDiagnosticVerifier.cpp -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/SourceMgrDiagnosticVerifier.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/raw_ostream.h"

استعمال نامخلا llvm;

/// Given a diagnostic kind, return a human readable string for it.
ساکن StringRef get_Diag_قسم_Str(ماخذ_Mgr::Diag_قسم قسم_2) {
  انتخاب (قسم_2) {
  حالت ماخذ_Mgr::DK_Note:
    واپس "note";
  حالت ماخذ_Mgr::DK_Warning:
    واپس "warning";
  حالت ماخذ_Mgr::DK_Error:
    واپس "error";
  حالت ماخذ_Mgr::DK_Remark:
    واپس "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

منطقی ماخذ_Mgr_Diagnostic_Verifier::ExpectedDiag::emitError(
    raw_ostream &os, ماخذ_Mgr &mgr, ثابت Twine &پیغام) ثابت {
  اگر (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::get_From_اشارہ_گر(fileLoc.get_اشارہ_گر() +
                                                 substring.سائز_2()));
    mgr.Print_پیغام(os, fileLoc, ماخذ_Mgr::DK_Error, پیغام, range);
  } ورنہ {
    mgr.Print_پیغام(os, fileLoc, ماخذ_Mgr::DK_Error, پیغام);
  }
  واپس جھوٹ;
}

منطقی ماخذ_Mgr_Diagnostic_Verifier::ExpectedDiag::مطابقت(StringRef str) ثابت {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  اگر (substringRegex)
    واپس substringRegex->مطابقت(str);
  واپس str.شامل_ہے(substring);
}

منطقی ماخذ_Mgr_Diagnostic_Verifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             ماخذ_Mgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef str_مقصد_Process = substring;
  جب (!str_مقصد_Process.خالی_خانے()) {
    // Find the next regex block.
    size_t regexIt = str_مقصد_Process.ڈھونڈو("{{");
    اگر (regexIt == StringRef::npos) {
      regexOS << Regex::escape(str_مقصد_Process);
      توڑو;
    }
    regexOS << Regex::escape(str_مقصد_Process.take_front(regexIt));
    str_مقصد_Process = str_مقصد_Process.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regex_انتہا_It = str_مقصد_Process.ڈھونڈو("}}");
    اگر (regex_انتہا_It == StringRef::npos)
      واپس emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regex_بلاک = str_مقصد_Process.take_front(regex_انتہا_It);

    // Validate that the regex is actually valid.
    std::string regexError;
    اگر (!Regex(regex_بلاک).isValid(regexError))
      واپس emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regex_بلاک << ')';
    str_مقصد_Process = str_مقصد_Process.drop_front(regex_انتہا_It + 2);
  }
  substringRegex = Regex(regexStr);
  واپس سچ;
}

MutableArrayRef<ماخذ_Mgr_Diagnostic_Verifier::ExpectedDiag>
ماخذ_Mgr_Diagnostic_Verifier::computeExpectedDiags(raw_ostream &os,
                                                  ماخذ_Mgr &mgr,
                                                  ثابت حافظہ_بفر *buf) {
  // If the buffer is invalid, return an empty list.
  اگر (!buf)
    واپس {};
  خودکار &expectedDiags = expected_Diags_فی_File[buf->get_بفر_Identifier()];

  // The number of the last line that did not correlate to a designator.
  غیرعلامتی آخری_Non_Designator_سطر = 0;

  // The indices of designators that apply to the next non designator line.
  Small_سمتیہ<غیرعلامتی, 1> designators_For_اگلا_سطر;

  // Scan the file for expected-* designators.
  Small_سمتیہ<StringRef, 100> سطریں_2;
  buf->get_بفر().بانٹو(سطریں_2, '\n');
  برائے (غیرعلامتی سطر_No_2 = 0, e = سطریں_2.سائز_2(); سطر_No_2 < e; ++سطر_No_2) {
    Small_سمتیہ<StringRef, 4> matches;
    اگر (!expected.مطابقت(سطریں_2[سطر_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      اگر (!designators_For_اگلا_سطر.خالی_خانے()) {
        برائے (غیرعلامتی diag_اشاریہ : designators_For_اگلا_سطر)
          expectedDiags[diag_اشاریہ].سطر_No_2 = سطر_No_2 + 1;
        designators_For_اگلا_سطر.clear();
      }
      آخری_Non_Designator_سطر = سطر_No_2;
      جاری;
    }

    // Point to the start of expected-*.
    SMLoc expected_آغاز = SMLoc::get_From_اشارہ_گر(matches[0].ڈیٹا());

    ماخذ_Mgr::Diag_قسم قسم_2;
    اگر (matches[1] == "error")
      قسم_2 = ماخذ_Mgr::DK_Error;
    ورنہ اگر (matches[1] == "warning")
      قسم_2 = ماخذ_Mgr::DK_Warning;
    ورنہ اگر (matches[1] == "remark")
      قسم_2 = ماخذ_Mgr::DK_Remark;
    ورنہ {
      أكد(matches[1] == "note");
      قسم_2 = ماخذ_Mgr::DK_Note;
    }
    ExpectedDiag ریکارڈ_2(قسم_2, سطر_No_2 + 1, expected_آغاز, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    اگر (!matches[2].خالی_خانے() && !ریکارڈ_2.computeRegex(os, mgr)) {
      ok = جھوٹ;
      جاری;
    }

    StringRef انحراف_مطابقت = matches[3];
    اگر (!انحراف_مطابقت.خالی_خانے()) {
      انحراف_مطابقت = انحراف_مطابقت.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      اگر (انحراف_مطابقت[0] == '+' || انحراف_مطابقت[0] == '-') {
        صحیح انحراف_2;
        انحراف_مطابقت.drop_front().getAsInteger(0, انحراف_2);

        اگر (انحراف_مطابقت.front() == '+')
          ریکارڈ_2.سطر_No_2 += انحراف_2;
        ورنہ
          ریکارڈ_2.سطر_No_2 -= انحراف_2;
      } ورنہ اگر (انحراف_مطابقت.consume_front("unknown")) {
        // This is matching unknown locations.
        ریکارڈ_2.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_عقب(std::move(ریکارڈ_2));
        جاری;
      } ورنہ اگر (انحراف_مطابقت.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        ریکارڈ_2.سطر_No_2 = آخری_Non_Designator_سطر + 1;
      } ورنہ {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        أكد(انحراف_مطابقت.consume_front("below"));
        designators_For_اگلا_سطر.ڈالو_عقب(expectedDiags.سائز_2());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        ریکارڈ_2.سطر_No_2 = e;
      }
    }
    expectedDiags.emplace_عقب(std::move(ریکارڈ_2));
  }
  واپس expectedDiags;
}

std::optional<MutableArrayRef<ماخذ_Mgr_Diagnostic_Verifier::ExpectedDiag>>
ماخذ_Mgr_Diagnostic_Verifier::getExpectedDiags(StringRef bufName) {
  خودکار expectedDiags = expected_Diags_فی_File.ڈھونڈو(bufName);
  اگر (expectedDiags != expected_Diags_فی_File.انتہا())
    واپس MutableArrayRef<ExpectedDiag>(expectedDiags->دوسرا_2);
  واپس std::nullopt;
}

ماخذ_Mgr_Diagnostic_Verifier::مطابقت_نتیجہ
ماخذ_Mgr_Diagnostic_Verifier::process(raw_ostream &os, ماخذ_Mgr &mgr,
                                     ماخذ_Mgr::Diag_قسم قسم_2, منطقی hasLoc,
                                     ثابت حافظہ_بفر *buf, غیرعلامتی سطر_No_2,
                                     StringRef پیغام_2, منطقی reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  اگر (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    اگر (buf) {
      اگر (خودکار maybeDiags = getExpectedDiags(buf->get_بفر_Identifier()))
        diags = *maybeDiags;
      ورنہ
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } ورنہ {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *near_ناکامی = خالی_اشاریہ;

  // If this was an expected error, remember that we saw it and return.
  برائے (خودکار &e : diags) {
    // File line must match (unless it's an unknown location).
    اگر (hasLoc && e.سطر_No_2 != سطر_No_2)
      جاری;
    اگر (e.مطابقت(پیغام_2)) {
      اگر (e.قسم_2 == قسم_2) {
        e.matched = سچ;
        واپس مطابقت_نتیجہ::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      near_ناکامی = &e;
    }
  }

  اگر (!reportUnexpected)
    واپس مطابقت_نتیجہ::Ignored;

  ok = جھوٹ;

  // Otherwise, emit an error for the near miss.
  اگر (near_ناکامی) {
    mgr.Print_پیغام(os, near_ناکامی->fileLoc, ماخذ_Mgr::DK_Error,
                     "'" + get_Diag_قسم_Str(قسم_2) +
                         "' diagnostic emitted when expecting a '" +
                         get_Diag_قسم_Str(near_ناکامی->قسم_2) + "'");
    واپس مطابقت_نتیجہ::Near_ناکامی;
  }
  واپس مطابقت_نتیجہ::Unexpected;
}

منطقی ماخذ_Mgr_Diagnostic_Verifier::verify(raw_ostream &os, ماخذ_Mgr &mgr) {
  // Verify that all expected errors were seen.
  خودکار checkExpectedDiags = [&](ExpectedDiag &diag) {
    اگر (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + get_Diag_قسم_Str(diag.قسم_2) + " \"" +
                         diag.substring + "\" was not produced");
      ok = جھوٹ;
    }
  };
  برائے (خودکار &expectedDiagsPair : expected_Diags_فی_File)
    برائے (خودکار &diag : expectedDiagsPair.دوسرا_2)
      checkExpectedDiags(diag);
  برائے (خودکار &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expected_Diags_فی_File.clear();
  واپس ok;
}
