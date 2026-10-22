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
  विकल्प (kind) {
  पक्ष SourceMgr::DK_Note:
    पटिनिवत्त "note";
  पक्ष SourceMgr::DK_Warning:
    पटिनिवत्त "warning";
  पक्ष SourceMgr::DK_Error:
    पटिनिवत्त "error";
  पक्ष SourceMgr::DK_Remark:
    पटिनिवत्त "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

तार्किक SourceMgrDiagnosticVerifier::ExpectedDiag::emitError(
    raw_ostream &os, SourceMgr &mgr, ध्रुव Twine &सन्देशः) ध्रुव {
  सचे (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.size()));
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, सन्देशः, range);
  } अञ्ञथा {
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, सन्देशः);
  }
  पटिनिवत्त मिच्छा;
}

तार्किक SourceMgrDiagnosticVerifier::ExpectedDiag::match(StringRef str) ध्रुव {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  सचे (substringRegex)
    पटिनिवत्त substringRegex->match(str);
  पटिनिवत्त str.contains(substring);
}

तार्किक SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef strToProcess = substring;
  याव (!strToProcess.empty()) {
    // Find the next regex block.
    size_t regexIt = strToProcess.find("{{");
    सचे (regexIt == StringRef::npos) {
      regexOS << Regex::escape(strToProcess);
      भङ्ग;
    }
    regexOS << Regex::escape(strToProcess.take_front(regexIt));
    strToProcess = strToProcess.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regexEndIt = strToProcess.find("}}");
    सचे (regexEndIt == StringRef::npos)
      पटिनिवत्त emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regexBlock = strToProcess.take_front(regexEndIt);

    // Validate that the regex is actually valid.
    std::string regexError;
    सचे (!Regex(regexBlock).isValid(regexError))
      पटिनिवत्त emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regexBlock << ')';
    strToProcess = strToProcess.drop_front(regexEndIt + 2);
  }
  substringRegex = Regex(regexStr);
  पटिनिवत्त सच्च;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  ध्रुव MemoryBuffer *buf) {
  // If the buffer is invalid, return an empty list.
  सचे (!buf)
    पटिनिवत्त {};
  auto &expectedDiags = expectedDiagsPerFile[buf->getBufferIdentifier()];

  // The number of the last line that did not correlate to a designator.
  unsigned last_Non_Designator_पङ्क्तिः = 0;

  // The indices of designators that apply to the next non designator line.
  SmallVector<unsigned, 1> designators_For_Next_पङ्क्तिः;

  // Scan the file for expected-* designators.
  SmallVector<StringRef, 100> पङ्क्तयः_2;
  buf->getBuffer().split(पङ्क्तयः_2, '\n');
  अत्थाय (unsigned पङ्क्तिः_No_2 = 0, e = पङ्क्तयः_2.size(); पङ्क्तिः_No_2 < e; ++पङ्क्तिः_No_2) {
    SmallVector<StringRef, 4> matches;
    सचे (!expected.match(पङ्क्तयः_2[पङ्क्तिः_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      सचे (!designators_For_Next_पङ्क्तिः.empty()) {
        अत्थाय (unsigned diagIndex : designators_For_Next_पङ्क्तिः)
          expectedDiags[diagIndex].पङ्क्तिः_No_2 = पङ्क्तिः_No_2 + 1;
        designators_For_Next_पङ्क्तिः.clear();
      }
      last_Non_Designator_पङ्क्तिः = पङ्क्तिः_No_2;
      अनुवर्त;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].data());

    SourceMgr::DiagKind kind;
    सचे (matches[1] == "error")
      kind = SourceMgr::DK_Error;
    अञ्ञथा सचे (matches[1] == "warning")
      kind = SourceMgr::DK_Warning;
    अञ्ञथा सचे (matches[1] == "remark")
      kind = SourceMgr::DK_Remark;
    अञ्ञथा {
      प्रतिज्ञा(matches[1] == "note");
      kind = SourceMgr::DK_Note;
    }
    ExpectedDiag record(kind, पङ्क्तिः_No_2 + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    सचे (!matches[2].empty() && !record.computeRegex(os, mgr)) {
      ok = मिच्छा;
      अनुवर्त;
    }

    StringRef offsetMatch = matches[3];
    सचे (!offsetMatch.empty()) {
      offsetMatch = offsetMatch.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      सचे (offsetMatch[0] == '+' || offsetMatch[0] == '-') {
        पूर्णाङ्क offset;
        offsetMatch.drop_front().getAsInteger(0, offset);

        सचे (offsetMatch.front() == '+')
          record.पङ्क्तिः_No_2 += offset;
        अञ्ञथा
          record.पङ्क्तिः_No_2 -= offset;
      } अञ्ञथा सचे (offsetMatch.consume_front("unknown")) {
        // This is matching unknown locations.
        record.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_back(std::move(record));
        अनुवर्त;
      } अञ्ञथा सचे (offsetMatch.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        record.पङ्क्तिः_No_2 = last_Non_Designator_पङ्क्तिः + 1;
      } अञ्ञथा {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        प्रतिज्ञा(offsetMatch.consume_front("below"));
        designators_For_Next_पङ्क्तिः.प्रक्षिप_back(expectedDiags.size());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        record.पङ्क्तिः_No_2 = e;
      }
    }
    expectedDiags.emplace_back(std::move(record));
  }
  पटिनिवत्त expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  auto expectedDiags = expectedDiagsPerFile.find(bufName);
  सचे (expectedDiags != expectedDiagsPerFile.end())
    पटिनिवत्त MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  पटिनिवत्त std::nullopt;
}

SourceMgrDiagnosticVerifier::MatchResult
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::DiagKind kind, तार्किक hasLoc,
                                     ध्रुव MemoryBuffer *buf, unsigned पङ्क्तिः_No_2,
                                     StringRef message, तार्किक reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  सचे (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    सचे (buf) {
      सचे (auto maybeDiags = getExpectedDiags(buf->getBufferIdentifier()))
        diags = *maybeDiags;
      अञ्ञथा
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } अञ्ञथा {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *nearMiss = nullptr;

  // If this was an expected error, remember that we saw it and return.
  अत्थाय (auto &e : diags) {
    // File line must match (unless it's an unknown location).
    सचे (hasLoc && e.पङ्क्तिः_No_2 != पङ्क्तिः_No_2)
      अनुवर्त;
    सचे (e.match(message)) {
      सचे (e.kind == kind) {
        e.matched = सच्च;
        पटिनिवत्त MatchResult::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      nearMiss = &e;
    }
  }

  सचे (!reportUnexpected)
    पटिनिवत्त MatchResult::Ignored;

  ok = मिच्छा;

  // Otherwise, emit an error for the near miss.
  सचे (nearMiss) {
    mgr.PrintMessage(os, nearMiss->fileLoc, SourceMgr::DK_Error,
                     "'" + getDiagKindStr(kind) +
                         "' diagnostic emitted when expecting a '" +
                         getDiagKindStr(nearMiss->kind) + "'");
    पटिनिवत्त MatchResult::NearMiss;
  }
  पटिनिवत्त MatchResult::Unexpected;
}

तार्किक SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  auto checkExpectedDiags = [&](ExpectedDiag &diag) {
    सचे (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + getDiagKindStr(diag.kind) + " \"" +
                         diag.substring + "\" was not produced");
      ok = मिच्छा;
    }
  };
  अत्थाय (auto &expectedDiagsPair : expectedDiagsPerFile)
    अत्थाय (auto &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  अत्थाय (auto &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expectedDiagsPerFile.clear();
  पटिनिवत्त ok;
}
