//===- SourceMgrDiagnosticVerifier.cpp -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/SourceMgrDiagnosticVerifier.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

using नामाकाश llvm;

/// Given a diagnostic kind, return a human readable string for it.
स्थिर StringRef getDiagKindStr(SourceMgr::DiagKind kind) {
  निवड (kind) {
  स्थिती SourceMgr::DK_Note:
    परत "note";
  स्थिती SourceMgr::DK_Warning:
    परत "warning";
  स्थिती SourceMgr::DK_Error:
    परत "error";
  स्थिती SourceMgr::DK_Remark:
    परत "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

तार्किक SourceMgrDiagnosticVerifier::ExpectedDiag::emitError(
    raw_ostream &os, SourceMgr &mgr, स्थिरांक Twine &संदेश) स्थिरांक {
  जर (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.size()));
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, संदेश, range);
  } अन्यथा {
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, संदेश);
  }
  परत असत्य;
}

तार्किक SourceMgrDiagnosticVerifier::ExpectedDiag::match(StringRef str) स्थिरांक {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  जर (substringRegex)
    परत substringRegex->match(str);
  परत str.contains(substring);
}

तार्किक SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef strToProcess = substring;
  पर्यंत (!strToProcess.empty()) {
    // Find the next regex block.
    माप_प्रकार regexIt = strToProcess.find("{{");
    जर (regexIt == StringRef::npos) {
      regexOS << Regex::escape(strToProcess);
      थांबा;
    }
    regexOS << Regex::escape(strToProcess.take_front(regexIt));
    strToProcess = strToProcess.drop_front(regexIt + 2);

    // Find the end of the regex block.
    माप_प्रकार regexEndIt = strToProcess.find("}}");
    जर (regexEndIt == StringRef::npos)
      परत emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regexBlock = strToProcess.take_front(regexEndIt);

    // Validate that the regex is actually valid.
    std::string regexError;
    जर (!Regex(regexBlock).isValid(regexError))
      परत emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regexBlock << ')';
    strToProcess = strToProcess.drop_front(regexEndIt + 2);
  }
  substringRegex = Regex(regexStr);
  परत सत्य;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  स्थिरांक MemoryBuffer *buf) {
  // If the buffer is invalid, return an empty list.
  जर (!buf)
    परत {};
  स्वतः &expectedDiags = expectedDiagsPerFile[buf->getBufferIdentifier()];

  // The number of the last line that did not correlate to a designator.
  अचिन्हित last_Non_Designator_ओळ = 0;

  // The indices of designators that apply to the next non designator line.
  SmallVector<अचिन्हित, 1> designators_For_Next_ओळ;

  // Scan the file for expected-* designators.
  SmallVector<StringRef, 100> ओळी_2;
  buf->getBuffer().split(ओळी_2, '\n');
  साठी (अचिन्हित ओळ_No_2 = 0, e = ओळी_2.size(); ओळ_No_2 < e; ++ओळ_No_2) {
    SmallVector<StringRef, 4> matches;
    जर (!expected.match(ओळी_2[ओळ_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      जर (!designators_For_Next_ओळ.empty()) {
        साठी (अचिन्हित diagIndex : designators_For_Next_ओळ)
          expectedDiags[diagIndex].ओळ_No_2 = ओळ_No_2 + 1;
        designators_For_Next_ओळ.clear();
      }
      last_Non_Designator_ओळ = ओळ_No_2;
      सुरूठेवा;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].data());

    SourceMgr::DiagKind kind;
    जर (matches[1] == "error")
      kind = SourceMgr::DK_Error;
    अन्यथा जर (matches[1] == "warning")
      kind = SourceMgr::DK_Warning;
    अन्यथा जर (matches[1] == "remark")
      kind = SourceMgr::DK_Remark;
    अन्यथा {
      प्रतिज्ञा(matches[1] == "note");
      kind = SourceMgr::DK_Note;
    }
    ExpectedDiag record(kind, ओळ_No_2 + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    जर (!matches[2].empty() && !record.computeRegex(os, mgr)) {
      ok = असत्य;
      सुरूठेवा;
    }

    StringRef offsetMatch = matches[3];
    जर (!offsetMatch.empty()) {
      offsetMatch = offsetMatch.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      जर (offsetMatch[0] == '+' || offsetMatch[0] == '-') {
        पूर्णांक offset;
        offsetMatch.drop_front().getAsInteger(0, offset);

        जर (offsetMatch.front() == '+')
          record.ओळ_No_2 += offset;
        अन्यथा
          record.ओळ_No_2 -= offset;
      } अन्यथा जर (offsetMatch.consume_front("unknown")) {
        // This is matching unknown locations.
        record.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_back(std::move(record));
        सुरूठेवा;
      } अन्यथा जर (offsetMatch.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        record.ओळ_No_2 = last_Non_Designator_ओळ + 1;
      } अन्यथा {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        प्रतिज्ञा(offsetMatch.consume_front("below"));
        designators_For_Next_ओळ.ढकला_back(expectedDiags.size());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        record.ओळ_No_2 = e;
      }
    }
    expectedDiags.emplace_back(std::move(record));
  }
  परत expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  स्वतः expectedDiags = expectedDiagsPerFile.find(bufName);
  जर (expectedDiags != expectedDiagsPerFile.end())
    परत MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  परत std::nullopt;
}

SourceMgrDiagnosticVerifier::MatchResult
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::DiagKind kind, तार्किक hasLoc,
                                     स्थिरांक MemoryBuffer *buf, अचिन्हित ओळ_No_2,
                                     StringRef message, तार्किक reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  जर (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    जर (buf) {
      जर (स्वतः maybeDiags = getExpectedDiags(buf->getBufferIdentifier()))
        diags = *maybeDiags;
      अन्यथा
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } अन्यथा {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *nearMiss = nullptr;

  // If this was an expected error, remember that we saw it and return.
  साठी (स्वतः &e : diags) {
    // File line must match (unless it's an unknown location).
    जर (hasLoc && e.ओळ_No_2 != ओळ_No_2)
      सुरूठेवा;
    जर (e.match(message)) {
      जर (e.kind == kind) {
        e.matched = सत्य;
        परत MatchResult::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      nearMiss = &e;
    }
  }

  जर (!reportUnexpected)
    परत MatchResult::Ignored;

  ok = असत्य;

  // Otherwise, emit an error for the near miss.
  जर (nearMiss) {
    mgr.PrintMessage(os, nearMiss->fileLoc, SourceMgr::DK_Error,
                     "'" + getDiagKindStr(kind) +
                         "' diagnostic emitted when expecting a '" +
                         getDiagKindStr(nearMiss->kind) + "'");
    परत MatchResult::NearMiss;
  }
  परत MatchResult::Unexpected;
}

तार्किक SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  स्वतः checkExpectedDiags = [&](ExpectedDiag &diag) {
    जर (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + getDiagKindStr(diag.kind) + " \"" +
                         diag.substring + "\" was not produced");
      ok = असत्य;
    }
  };
  साठी (स्वतः &expectedDiagsPair : expectedDiagsPerFile)
    साठी (स्वतः &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  साठी (स्वतः &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expectedDiagsPerFile.clear();
  परत ok;
}
