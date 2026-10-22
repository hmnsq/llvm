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
  case SourceMgr::DK_Note:
    返回 "note";
  case SourceMgr::DK_Warning:
    返回 "warning";
  case SourceMgr::DK_Error:
    返回 "error";
  case SourceMgr::DK_Remark:
    返回 "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

bool SourceMgrDiagnosticVerifier::ExpectedDiag::emitError(
    raw_ostream &os, SourceMgr &mgr, const Twine &msg) const {
  如果 (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.size()));
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, msg, range);
  } 否则 {
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, msg);
  }
  返回 false;
}

bool SourceMgrDiagnosticVerifier::ExpectedDiag::match(StringRef str) const {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  如果 (substringRegex)
    返回 substringRegex->match(str);
  返回 str.contains(substring);
}

bool SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef strToProcess = substring;
  当 (!strToProcess.empty()) {
    // Find the next regex block.
    size_t regexIt = strToProcess.find("{{");
    如果 (regexIt == StringRef::npos) {
      regexOS << Regex::escape(strToProcess);
      break;
    }
    regexOS << Regex::escape(strToProcess.take_front(regexIt));
    strToProcess = strToProcess.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regexEndIt = strToProcess.find("}}");
    如果 (regexEndIt == StringRef::npos)
      返回 emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regexBlock = strToProcess.take_front(regexEndIt);

    // Validate that the regex is actually valid.
    std::string regexError;
    如果 (!Regex(regexBlock).isValid(regexError))
      返回 emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regexBlock << ')';
    strToProcess = strToProcess.drop_front(regexEndIt + 2);
  }
  substringRegex = Regex(regexStr);
  返回 true;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  const MemoryBuffer *buf) {
  // If the buffer is invalid, return an empty list.
  如果 (!buf)
    返回 {};
  auto &expectedDiags = expectedDiagsPerFile[buf->getBufferIdentifier()];

  // The number of the last line that did not correlate to a designator.
  unsigned lastNonDesignatorLine = 0;

  // The indices of designators that apply to the next non designator line.
  SmallVector<unsigned, 1> designatorsForNextLine;

  // Scan the file for expected-* designators.
  SmallVector<StringRef, 100> lines;
  buf->getBuffer().split(lines, '\n');
  为 (unsigned lineNo = 0, e = lines.size(); lineNo < e; ++lineNo) {
    SmallVector<StringRef, 4> matches;
    如果 (!expected.match(lines[lineNo].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      如果 (!designatorsForNextLine.empty()) {
        为 (unsigned diagIndex : designatorsForNextLine)
          expectedDiags[diagIndex].lineNo = lineNo + 1;
        designatorsForNextLine.clear();
      }
      lastNonDesignatorLine = lineNo;
      continue;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].data());

    SourceMgr::DiagKind kind;
    如果 (matches[1] == "error")
      kind = SourceMgr::DK_Error;
    否则 如果 (matches[1] == "warning")
      kind = SourceMgr::DK_Warning;
    否则 如果 (matches[1] == "remark")
      kind = SourceMgr::DK_Remark;
    否则 {
      assert(matches[1] == "note");
      kind = SourceMgr::DK_Note;
    }
    ExpectedDiag record(kind, lineNo + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    如果 (!matches[2].empty() && !record.computeRegex(os, mgr)) {
      ok = false;
      continue;
    }

    StringRef offsetMatch = matches[3];
    如果 (!offsetMatch.empty()) {
      offsetMatch = offsetMatch.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      如果 (offsetMatch[0] == '+' || offsetMatch[0] == '-') {
        整数 offset;
        offsetMatch.drop_front().getAsInteger(0, offset);

        如果 (offsetMatch.front() == '+')
          record.lineNo += offset;
        否则
          record.lineNo -= offset;
      } 否则 如果 (offsetMatch.consume_front("unknown")) {
        // This is matching unknown locations.
        record.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_back(std::move(record));
        continue;
      } 否则 如果 (offsetMatch.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        record.lineNo = lastNonDesignatorLine + 1;
      } 否则 {
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
  返回 expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  auto expectedDiags = expectedDiagsPerFile.find(bufName);
  如果 (expectedDiags != expectedDiagsPerFile.end())
    返回 MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  返回 std::nullopt;
}

SourceMgrDiagnosticVerifier::MatchResult
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::DiagKind kind, bool hasLoc,
                                     const MemoryBuffer *buf, unsigned lineNo,
                                     StringRef message, bool reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  如果 (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    如果 (buf) {
      如果 (auto maybeDiags = getExpectedDiags(buf->getBufferIdentifier()))
        diags = *maybeDiags;
      否则
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } 否则 {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *nearMiss = nullptr;

  // If this was an expected error, remember that we saw it and return.
  为 (auto &e : diags) {
    // File line must match (unless it's an unknown location).
    如果 (hasLoc && e.lineNo != lineNo)
      continue;
    如果 (e.match(message)) {
      如果 (e.kind == kind) {
        e.matched = true;
        返回 MatchResult::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      nearMiss = &e;
    }
  }

  如果 (!reportUnexpected)
    返回 MatchResult::Ignored;

  ok = false;

  // Otherwise, emit an error for the near miss.
  如果 (nearMiss) {
    mgr.PrintMessage(os, nearMiss->fileLoc, SourceMgr::DK_Error,
                     "'" + getDiagKindStr(kind) +
                         "' diagnostic emitted when expecting a '" +
                         getDiagKindStr(nearMiss->kind) + "'");
    返回 MatchResult::NearMiss;
  }
  返回 MatchResult::Unexpected;
}

bool SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  auto checkExpectedDiags = [&](ExpectedDiag &diag) {
    如果 (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + getDiagKindStr(diag.kind) + " \"" +
                         diag.substring + "\" was not produced");
      ok = false;
    }
  };
  为 (auto &expectedDiagsPair : expectedDiagsPerFile)
    为 (auto &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  为 (auto &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expectedDiagsPerFile.clear();
  返回 ok;
}
