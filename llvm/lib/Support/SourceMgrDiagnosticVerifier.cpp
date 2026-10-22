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
    החזר "note";
  حالة SourceMgr::DK_Warning:
    החזר "warning";
  حالة SourceMgr::DK_Error:
    החזר "error";
  حالة SourceMgr::DK_Remark:
    החזר "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

منطقي SourceMgrDiagnosticVerifier::ExpectedDiag::emitError(
    raw_ostream &os, SourceMgr &mgr, ثابت Twine &msg) ثابت {
  אם (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.size()));
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, msg, range);
  } אחרת {
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, msg);
  }
  החזר שקר;
}

منطقي SourceMgrDiagnosticVerifier::ExpectedDiag::match(StringRef str) ثابت {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  אם (substringRegex)
    החזר substringRegex->match(str);
  החזר str.contains(substring);
}

منطقي SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef strToProcess = substring;
  כלעוד (!strToProcess.empty()) {
    // Find the next regex block.
    size_t regexIt = strToProcess.find("{{");
    אם (regexIt == StringRef::npos) {
      regexOS << Regex::escape(strToProcess);
      اكسر;
    }
    regexOS << Regex::escape(strToProcess.take_front(regexIt));
    strToProcess = strToProcess.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regexEndIt = strToProcess.find("}}");
    אם (regexEndIt == StringRef::npos)
      החזר emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regexBlock = strToProcess.take_front(regexEndIt);

    // Validate that the regex is actually valid.
    std::string regexError;
    אם (!Regex(regexBlock).isValid(regexError))
      החזר emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regexBlock << ')';
    strToProcess = strToProcess.drop_front(regexEndIt + 2);
  }
  substringRegex = Regex(regexStr);
  החזר אמת;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  ثابت MemoryBuffer *buf) {
  // If the buffer is invalid, return an empty list.
  אם (!buf)
    החזר {};
  auto &expectedDiags = expectedDiagsPerFile[buf->getBufferIdentifier()];

  // The number of the last line that did not correlate to a designator.
  unsigned lastNonDesignatorLine = 0;

  // The indices of designators that apply to the next non designator line.
  SmallVector<unsigned, 1> designatorsForNextLine;

  // Scan the file for expected-* designators.
  SmallVector<StringRef, 100> lines;
  buf->getBuffer().split(lines, '\n');
  עבור (unsigned lineNo = 0, e = lines.size(); lineNo < e; ++lineNo) {
    SmallVector<StringRef, 4> matches;
    אם (!expected.match(lines[lineNo].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      אם (!designatorsForNextLine.empty()) {
        עבור (unsigned diagIndex : designatorsForNextLine)
          expectedDiags[diagIndex].lineNo = lineNo + 1;
        designatorsForNextLine.clear();
      }
      lastNonDesignatorLine = lineNo;
      استمر;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].data());

    SourceMgr::DiagKind kind;
    אם (matches[1] == "error")
      kind = SourceMgr::DK_Error;
    אחרת אם (matches[1] == "warning")
      kind = SourceMgr::DK_Warning;
    אחרת אם (matches[1] == "remark")
      kind = SourceMgr::DK_Remark;
    אחרת {
      أكد(matches[1] == "note");
      kind = SourceMgr::DK_Note;
    }
    ExpectedDiag record(kind, lineNo + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    אם (!matches[2].empty() && !record.computeRegex(os, mgr)) {
      ok = שקר;
      استمر;
    }

    StringRef offsetMatch = matches[3];
    אם (!offsetMatch.empty()) {
      offsetMatch = offsetMatch.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      אם (offsetMatch[0] == '+' || offsetMatch[0] == '-') {
        صحيح offset;
        offsetMatch.drop_front().getAsInteger(0, offset);

        אם (offsetMatch.front() == '+')
          record.lineNo += offset;
        אחרת
          record.lineNo -= offset;
      } אחרת אם (offsetMatch.consume_front("unknown")) {
        // This is matching unknown locations.
        record.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_back(std::move(record));
        استمر;
      } אחרת אם (offsetMatch.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        record.lineNo = lastNonDesignatorLine + 1;
      } אחרת {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        أكد(offsetMatch.consume_front("below"));
        designatorsForNextLine.push_back(expectedDiags.size());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        record.lineNo = e;
      }
    }
    expectedDiags.emplace_back(std::move(record));
  }
  החזר expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  auto expectedDiags = expectedDiagsPerFile.find(bufName);
  אם (expectedDiags != expectedDiagsPerFile.end())
    החזר MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  החזר std::nullopt;
}

SourceMgrDiagnosticVerifier::MatchResult
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::DiagKind kind, منطقي hasLoc,
                                     ثابت MemoryBuffer *buf, unsigned lineNo,
                                     StringRef message, منطقي reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  אם (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    אם (buf) {
      אם (auto maybeDiags = getExpectedDiags(buf->getBufferIdentifier()))
        diags = *maybeDiags;
      אחרת
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } אחרת {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *nearMiss = nullptr;

  // If this was an expected error, remember that we saw it and return.
  עבור (auto &e : diags) {
    // File line must match (unless it's an unknown location).
    אם (hasLoc && e.lineNo != lineNo)
      استمر;
    אם (e.match(message)) {
      אם (e.kind == kind) {
        e.matched = אמת;
        החזר MatchResult::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      nearMiss = &e;
    }
  }

  אם (!reportUnexpected)
    החזר MatchResult::Ignored;

  ok = שקר;

  // Otherwise, emit an error for the near miss.
  אם (nearMiss) {
    mgr.PrintMessage(os, nearMiss->fileLoc, SourceMgr::DK_Error,
                     "'" + getDiagKindStr(kind) +
                         "' diagnostic emitted when expecting a '" +
                         getDiagKindStr(nearMiss->kind) + "'");
    החזר MatchResult::NearMiss;
  }
  החזר MatchResult::Unexpected;
}

منطقي SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  auto checkExpectedDiags = [&](ExpectedDiag &diag) {
    אם (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + getDiagKindStr(diag.kind) + " \"" +
                         diag.substring + "\" was not produced");
      ok = שקר;
    }
  };
  עבור (auto &expectedDiagsPair : expectedDiagsPerFile)
    עבור (auto &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  עבור (auto &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expectedDiagsPerFile.clear();
  החזר ok;
}
