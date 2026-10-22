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

using فضاینام llvm;

/// Given a diagnostic kind, return a human readable string for it.
ایستا StringRef getDiagKindStr(SourceMgr::DiagKind kind) {
  انتخاب (kind) {
  حالت SourceMgr::DK_Note:
    بازگشت "note";
  حالت SourceMgr::DK_Warning:
    بازگشت "warning";
  حالت SourceMgr::DK_Error:
    بازگشت "error";
  حالت SourceMgr::DK_Remark:
    بازگشت "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

منطقی SourceMgrDiagnosticVerifier::ExpectedDiag::emitError(
    raw_ostream &os, SourceMgr &mgr, ثابت Twine &پیام) ثابت {
  اگر (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.size()));
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, پیام, range);
  } وگرنه {
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, پیام);
  }
  بازگشت نادرست;
}

منطقی SourceMgrDiagnosticVerifier::ExpectedDiag::match(StringRef str) ثابت {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  اگر (substringRegex)
    بازگشت substringRegex->match(str);
  بازگشت str.contains(substring);
}

منطقی SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef strToProcess = substring;
  وقتی (!strToProcess.empty()) {
    // Find the next regex block.
    size_t regexIt = strToProcess.find("{{");
    اگر (regexIt == StringRef::npos) {
      regexOS << Regex::escape(strToProcess);
      بشکن;
    }
    regexOS << Regex::escape(strToProcess.take_front(regexIt));
    strToProcess = strToProcess.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regexEndIt = strToProcess.find("}}");
    اگر (regexEndIt == StringRef::npos)
      بازگشت emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regexBlock = strToProcess.take_front(regexEndIt);

    // Validate that the regex is actually valid.
    std::string regexError;
    اگر (!Regex(regexBlock).isValid(regexError))
      بازگشت emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regexBlock << ')';
    strToProcess = strToProcess.drop_front(regexEndIt + 2);
  }
  substringRegex = Regex(regexStr);
  بازگشت درست;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  ثابت MemoryBuffer *buf) {
  // If the buffer is invalid, return an empty list.
  اگر (!buf)
    بازگشت {};
  auto &expectedDiags = expectedDiagsPerFile[buf->getBufferIdentifier()];

  // The number of the last line that did not correlate to a designator.
  unsigned last_Non_Designator_سطر = 0;

  // The indices of designators that apply to the next non designator line.
  SmallVector<unsigned, 1> designators_For_Next_سطر;

  // Scan the file for expected-* designators.
  SmallVector<StringRef, 100> سطرها_2;
  buf->getBuffer().split(سطرها_2, '\n');
  برای (unsigned سطر_No_2 = 0, e = سطرها_2.size(); سطر_No_2 < e; ++سطر_No_2) {
    SmallVector<StringRef, 4> matches;
    اگر (!expected.match(سطرها_2[سطر_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      اگر (!designators_For_Next_سطر.empty()) {
        برای (unsigned diagIndex : designators_For_Next_سطر)
          expectedDiags[diagIndex].سطر_No_2 = سطر_No_2 + 1;
        designators_For_Next_سطر.clear();
      }
      last_Non_Designator_سطر = سطر_No_2;
      ادامه;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].data());

    SourceMgr::DiagKind kind;
    اگر (matches[1] == "error")
      kind = SourceMgr::DK_Error;
    وگرنه اگر (matches[1] == "warning")
      kind = SourceMgr::DK_Warning;
    وگرنه اگر (matches[1] == "remark")
      kind = SourceMgr::DK_Remark;
    وگرنه {
      أكد(matches[1] == "note");
      kind = SourceMgr::DK_Note;
    }
    ExpectedDiag record(kind, سطر_No_2 + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    اگر (!matches[2].empty() && !record.computeRegex(os, mgr)) {
      ok = نادرست;
      ادامه;
    }

    StringRef offsetMatch = matches[3];
    اگر (!offsetMatch.empty()) {
      offsetMatch = offsetMatch.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      اگر (offsetMatch[0] == '+' || offsetMatch[0] == '-') {
        صحیح offset;
        offsetMatch.drop_front().getAsInteger(0, offset);

        اگر (offsetMatch.front() == '+')
          record.سطر_No_2 += offset;
        وگرنه
          record.سطر_No_2 -= offset;
      } وگرنه اگر (offsetMatch.consume_front("unknown")) {
        // This is matching unknown locations.
        record.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_back(std::move(record));
        ادامه;
      } وگرنه اگر (offsetMatch.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        record.سطر_No_2 = last_Non_Designator_سطر + 1;
      } وگرنه {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        أكد(offsetMatch.consume_front("below"));
        designators_For_Next_سطر.افزودن_back(expectedDiags.size());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        record.سطر_No_2 = e;
      }
    }
    expectedDiags.emplace_back(std::move(record));
  }
  بازگشت expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  auto expectedDiags = expectedDiagsPerFile.find(bufName);
  اگر (expectedDiags != expectedDiagsPerFile.end())
    بازگشت MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  بازگشت std::nullopt;
}

SourceMgrDiagnosticVerifier::MatchResult
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::DiagKind kind, منطقی hasLoc,
                                     ثابت MemoryBuffer *buf, unsigned سطر_No_2,
                                     StringRef message, منطقی reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  اگر (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    اگر (buf) {
      اگر (auto maybeDiags = getExpectedDiags(buf->getBufferIdentifier()))
        diags = *maybeDiags;
      وگرنه
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } وگرنه {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *nearMiss = nullptr;

  // If this was an expected error, remember that we saw it and return.
  برای (auto &e : diags) {
    // File line must match (unless it's an unknown location).
    اگر (hasLoc && e.سطر_No_2 != سطر_No_2)
      ادامه;
    اگر (e.match(message)) {
      اگر (e.kind == kind) {
        e.matched = درست;
        بازگشت MatchResult::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      nearMiss = &e;
    }
  }

  اگر (!reportUnexpected)
    بازگشت MatchResult::Ignored;

  ok = نادرست;

  // Otherwise, emit an error for the near miss.
  اگر (nearMiss) {
    mgr.PrintMessage(os, nearMiss->fileLoc, SourceMgr::DK_Error,
                     "'" + getDiagKindStr(kind) +
                         "' diagnostic emitted when expecting a '" +
                         getDiagKindStr(nearMiss->kind) + "'");
    بازگشت MatchResult::NearMiss;
  }
  بازگشت MatchResult::Unexpected;
}

منطقی SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  auto checkExpectedDiags = [&](ExpectedDiag &diag) {
    اگر (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + getDiagKindStr(diag.kind) + " \"" +
                         diag.substring + "\" was not produced");
      ok = نادرست;
    }
  };
  برای (auto &expectedDiagsPair : expectedDiagsPerFile)
    برای (auto &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  برای (auto &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expectedDiagsPerFile.clear();
  بازگشت ok;
}
