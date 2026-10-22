//===- SourceMgrDiagnosticVerifier.cpp -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/SourceMgrDiagnosticVerifier.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

/// Given a diagnostic kind, return a human readable string for it.
static StringRef getDiagKindStr(SourceMgr::DiagKind kind) {
  switch (kind) {
  pak_sa SourceMgr::DK_Note:
    ᬯᬮᬶ "note";
  pak_sa SourceMgr::DK_Warning:
    ᬯᬮᬶ "warning";
  pak_sa SourceMgr::DK_Error:
    ᬯᬮᬶ "error";
  pak_sa SourceMgr::DK_Remark:
    ᬯᬮᬶ "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

bool SourceMgrDiagnosticVerifier::ExpectedDiag::emitError(
    raw_ostream &os, SourceMgr &mgr, const Twine &msg) const {
  ᬬᬾᬦ᭄ (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.size()));
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, msg, range);
  } ᬮᬶᬬ {
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, msg);
  }
  ᬯᬮᬶ false;
}

bool SourceMgrDiagnosticVerifier::ExpectedDiag::match(StringRef str) const {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  ᬬᬾᬦ᭄ (substringRegex)
    ᬯᬮᬶ substringRegex->match(str);
  ᬯᬮᬶ str.contains(substring);
}

bool SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef strToProcess = substring;
  ᬲᭂᬤᭂᬮ (!strToProcess.empty()) {
    // Find the next regex block.
    size_t regexIt = strToProcess.find("{{");
    ᬬᬾᬦ᭄ (regexIt == StringRef::npos) {
      regexOS << Regex::escape(strToProcess);
      bha_nga;
    }
    regexOS << Regex::escape(strToProcess.take_front(regexIt));
    strToProcess = strToProcess.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regexEndIt = strToProcess.find("}}");
    ᬬᬾᬦ᭄ (regexEndIt == StringRef::npos)
      ᬯᬮᬶ emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regexBlock = strToProcess.take_front(regexEndIt);

    // Validate that the regex is actually valid.
    std::string regexError;
    ᬬᬾᬦ᭄ (!Regex(regexBlock).isValid(regexError))
      ᬯᬮᬶ emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regexBlock << ')';
    strToProcess = strToProcess.drop_front(regexEndIt + 2);
  }
  substringRegex = Regex(regexStr);
  ᬯᬮᬶ true;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  const MemoryBuffer *buf) {
  // If the buffer is invalid, return an empty list.
  ᬬᬾᬦ᭄ (!buf)
    ᬯᬮᬶ {};
  auto &expectedDiags = expectedDiagsPerFile[buf->getBufferIdentifier()];

  // The number of the last line that did not correlate to a designator.
  unsigned lastNonDesignatorLine = 0;

  // The indices of designators that apply to the next non designator line.
  SmallVector<unsigned, 1> designatorsForNextLine;

  // Scan the file for expected-* designators.
  SmallVector<StringRef, 100> lines;
  buf->getBuffer().split(lines, '\n');
  for (unsigned lineNo = 0, e = lines.size(); lineNo < e; ++lineNo) {
    SmallVector<StringRef, 4> matches;
    ᬬᬾᬦ᭄ (!expected.match(lines[lineNo].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      ᬬᬾᬦ᭄ (!designatorsForNextLine.empty()) {
        for (unsigned diagIndex : designatorsForNextLine)
          expectedDiags[diagIndex].lineNo = lineNo + 1;
        designatorsForNextLine.clear();
      }
      lastNonDesignatorLine = lineNo;
      continue;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].data());

    SourceMgr::DiagKind kind;
    ᬬᬾᬦ᭄ (matches[1] == "error")
      kind = SourceMgr::DK_Error;
    ᬮᬶᬬ ᬬᬾᬦ᭄ (matches[1] == "warning")
      kind = SourceMgr::DK_Warning;
    ᬮᬶᬬ ᬬᬾᬦ᭄ (matches[1] == "remark")
      kind = SourceMgr::DK_Remark;
    ᬮᬶᬬ {
      assert(matches[1] == "note");
      kind = SourceMgr::DK_Note;
    }
    ExpectedDiag record(kind, lineNo + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    ᬬᬾᬦ᭄ (!matches[2].empty() && !record.computeRegex(os, mgr)) {
      ok = false;
      continue;
    }

    StringRef offsetMatch = matches[3];
    ᬬᬾᬦ᭄ (!offsetMatch.empty()) {
      offsetMatch = offsetMatch.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      ᬬᬾᬦ᭄ (offsetMatch[0] == '+' || offsetMatch[0] == '-') {
        puur_naa_nka offset;
        offsetMatch.drop_front().getAsInteger(0, offset);

        ᬬᬾᬦ᭄ (offsetMatch.front() == '+')
          record.lineNo += offset;
        ᬮᬶᬬ
          record.lineNo -= offset;
      } ᬮᬶᬬ ᬬᬾᬦ᭄ (offsetMatch.consume_front("unknown")) {
        // This is matching unknown locations.
        record.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_back(std::move(record));
        continue;
      } ᬮᬶᬬ ᬬᬾᬦ᭄ (offsetMatch.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        record.lineNo = lastNonDesignatorLine + 1;
      } ᬮᬶᬬ {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        assert(offsetMatch.consume_front("below"));
        designatorsForNextLine.push_back(expectedDiags.size());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        record.lineNo = e;
      }
    }
    expectedDiags.emplace_back(std::move(record));
  }
  ᬯᬮᬶ expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  auto expectedDiags = expectedDiagsPerFile.find(bufName);
  ᬬᬾᬦ᭄ (expectedDiags != expectedDiagsPerFile.end())
    ᬯᬮᬶ MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  ᬯᬮᬶ std::nullopt;
}

SourceMgrDiagnosticVerifier::MatchResult
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::DiagKind kind, bool hasLoc,
                                     const MemoryBuffer *buf, unsigned lineNo,
                                     StringRef message, bool reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  ᬬᬾᬦ᭄ (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    ᬬᬾᬦ᭄ (buf) {
      ᬬᬾᬦ᭄ (auto maybeDiags = getExpectedDiags(buf->getBufferIdentifier()))
        diags = *maybeDiags;
      ᬮᬶᬬ
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } ᬮᬶᬬ {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *nearMiss = nullptr;

  // If this was an expected error, remember that we saw it and return.
  for (auto &e : diags) {
    // File line must match (unless it's an unknown location).
    ᬬᬾᬦ᭄ (hasLoc && e.lineNo != lineNo)
      continue;
    ᬬᬾᬦ᭄ (e.match(message)) {
      ᬬᬾᬦ᭄ (e.kind == kind) {
        e.matched = true;
        ᬯᬮᬶ MatchResult::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      nearMiss = &e;
    }
  }

  ᬬᬾᬦ᭄ (!reportUnexpected)
    ᬯᬮᬶ MatchResult::Ignored;

  ok = false;

  // Otherwise, emit an error for the near miss.
  ᬬᬾᬦ᭄ (nearMiss) {
    mgr.PrintMessage(os, nearMiss->fileLoc, SourceMgr::DK_Error,
                     "'" + getDiagKindStr(kind) +
                         "' diagnostic emitted when expecting a '" +
                         getDiagKindStr(nearMiss->kind) + "'");
    ᬯᬮᬶ MatchResult::NearMiss;
  }
  ᬯᬮᬶ MatchResult::Unexpected;
}

bool SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  auto checkExpectedDiags = [&](ExpectedDiag &diag) {
    ᬬᬾᬦ᭄ (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + getDiagKindStr(diag.kind) + " \"" +
                         diag.substring + "\" was not produced");
      ok = false;
    }
  };
  for (auto &expectedDiagsPair : expectedDiagsPerFile)
    for (auto &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  for (auto &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expectedDiagsPerFile.clear();
  ᬯᬮᬶ ok;
}
