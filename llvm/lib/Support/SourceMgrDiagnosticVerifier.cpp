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

using نطاق llvm;

/// Given a diagnostic kind, return a human readable string for it.
ساكن StringRef getDiagKindStr(SourceMgr::DiagKind kind) {
  بدل (kind) {
  حالة SourceMgr::DK_Note:
    إرجاع "note";
  حالة SourceMgr::DK_Warning:
    إرجاع "warning";
  حالة SourceMgr::DK_Error:
    إرجاع "error";
  حالة SourceMgr::DK_Remark:
    إرجاع "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

منطقي SourceMgrDiagnosticVerifier::ExpectedDiag::emitError(
    raw_ostream &os, SourceMgr &mgr, ثابت Twine &رسالة) ثابت {
  إذا (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.size()));
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, رسالة, range);
  } وإلا {
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, رسالة);
  }
  إرجاع خطأ;
}

منطقي SourceMgrDiagnosticVerifier::ExpectedDiag::match(StringRef str) ثابت {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  إذا (substringRegex)
    إرجاع substringRegex->match(str);
  إرجاع str.contains(substring);
}

منطقي SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef strToProcess = substring;
  بينما (!strToProcess.empty()) {
    // Find the next regex block.
    size_t regexIt = strToProcess.find("{{");
    إذا (regexIt == StringRef::npos) {
      regexOS << Regex::escape(strToProcess);
      اكسر;
    }
    regexOS << Regex::escape(strToProcess.take_front(regexIt));
    strToProcess = strToProcess.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regexEndIt = strToProcess.find("}}");
    إذا (regexEndIt == StringRef::npos)
      إرجاع emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regexBlock = strToProcess.take_front(regexEndIt);

    // Validate that the regex is actually valid.
    std::string regexError;
    إذا (!Regex(regexBlock).isValid(regexError))
      إرجاع emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regexBlock << ')';
    strToProcess = strToProcess.drop_front(regexEndIt + 2);
  }
  substringRegex = Regex(regexStr);
  إرجاع true;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  ثابت MemoryBuffer *buf) {
  // If the buffer is invalid, return an empty list.
  إذا (!buf)
    إرجاع {};
  auto &expectedDiags = expectedDiagsPerFile[buf->getBufferIdentifier()];

  // The number of the last line that did not correlate to a designator.
  unsigned last_Non_Designator_سطر = 0;

  // The indices of designators that apply to the next non designator line.
  SmallVector<unsigned, 1> designators_For_Next_سطر;

  // Scan the file for expected-* designators.
  SmallVector<StringRef, 100> أسطر_2;
  buf->getBuffer().split(أسطر_2, '\n');
  لكل (unsigned سطر_No_2 = 0, e = أسطر_2.size(); سطر_No_2 < e; ++سطر_No_2) {
    SmallVector<StringRef, 4> matches;
    إذا (!expected.match(أسطر_2[سطر_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      إذا (!designators_For_Next_سطر.empty()) {
        لكل (unsigned diagIndex : designators_For_Next_سطر)
          expectedDiags[diagIndex].سطر_No_2 = سطر_No_2 + 1;
        designators_For_Next_سطر.clear();
      }
      last_Non_Designator_سطر = سطر_No_2;
      استمر;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].data());

    SourceMgr::DiagKind kind;
    إذا (matches[1] == "error")
      kind = SourceMgr::DK_Error;
    وإلا إذا (matches[1] == "warning")
      kind = SourceMgr::DK_Warning;
    وإلا إذا (matches[1] == "remark")
      kind = SourceMgr::DK_Remark;
    وإلا {
      أكد(matches[1] == "note");
      kind = SourceMgr::DK_Note;
    }
    ExpectedDiag record(kind, سطر_No_2 + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    إذا (!matches[2].empty() && !record.computeRegex(os, mgr)) {
      ok = خطأ;
      استمر;
    }

    StringRef offsetMatch = matches[3];
    إذا (!offsetMatch.empty()) {
      offsetMatch = offsetMatch.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      إذا (offsetMatch[0] == '+' || offsetMatch[0] == '-') {
        صحيح offset;
        offsetMatch.drop_front().getAsInteger(0, offset);

        إذا (offsetMatch.front() == '+')
          record.سطر_No_2 += offset;
        وإلا
          record.سطر_No_2 -= offset;
      } وإلا إذا (offsetMatch.consume_front("unknown")) {
        // This is matching unknown locations.
        record.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_back(std::move(record));
        استمر;
      } وإلا إذا (offsetMatch.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        record.سطر_No_2 = last_Non_Designator_سطر + 1;
      } وإلا {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        أكد(offsetMatch.consume_front("below"));
        designators_For_Next_سطر.ادفع_back(expectedDiags.size());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        record.سطر_No_2 = e;
      }
    }
    expectedDiags.emplace_back(std::move(record));
  }
  إرجاع expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  auto expectedDiags = expectedDiagsPerFile.find(bufName);
  إذا (expectedDiags != expectedDiagsPerFile.end())
    إرجاع MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  إرجاع std::nullopt;
}

SourceMgrDiagnosticVerifier::MatchResult
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::DiagKind kind, منطقي hasLoc,
                                     ثابت MemoryBuffer *buf, unsigned سطر_No_2,
                                     StringRef message, منطقي reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  إذا (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    إذا (buf) {
      إذا (auto maybeDiags = getExpectedDiags(buf->getBufferIdentifier()))
        diags = *maybeDiags;
      وإلا
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } وإلا {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *nearMiss = nullptr;

  // If this was an expected error, remember that we saw it and return.
  لكل (auto &e : diags) {
    // File line must match (unless it's an unknown location).
    إذا (hasLoc && e.سطر_No_2 != سطر_No_2)
      استمر;
    إذا (e.match(message)) {
      إذا (e.kind == kind) {
        e.matched = true;
        إرجاع MatchResult::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      nearMiss = &e;
    }
  }

  إذا (!reportUnexpected)
    إرجاع MatchResult::Ignored;

  ok = خطأ;

  // Otherwise, emit an error for the near miss.
  إذا (nearMiss) {
    mgr.PrintMessage(os, nearMiss->fileLoc, SourceMgr::DK_Error,
                     "'" + getDiagKindStr(kind) +
                         "' diagnostic emitted when expecting a '" +
                         getDiagKindStr(nearMiss->kind) + "'");
    إرجاع MatchResult::NearMiss;
  }
  إرجاع MatchResult::Unexpected;
}

منطقي SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  auto checkExpectedDiags = [&](ExpectedDiag &diag) {
    إذا (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + getDiagKindStr(diag.kind) + " \"" +
                         diag.substring + "\" was not produced");
      ok = خطأ;
    }
  };
  لكل (auto &expectedDiagsPair : expectedDiagsPerFile)
    لكل (auto &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  لكل (auto &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expectedDiagsPerFile.clear();
  إرجاع ok;
}
