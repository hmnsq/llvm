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
    ပြန်ပေး "note";
  pak_sa SourceMgr::DK_Warning:
    ပြန်ပေး "warning";
  pak_sa SourceMgr::DK_Error:
    ပြန်ပေး "error";
  pak_sa SourceMgr::DK_Remark:
    ပြန်ပေး "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

bool SourceMgrDiagnosticVerifier::ExpectedDiag::emitError(
    raw_ostream &os, SourceMgr &mgr, const Twine &msg) const {
  အကယ်၍ (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.size()));
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, msg, range);
  } မဟုတ်ပါက {
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, msg);
  }
  ပြန်ပေး false;
}

bool SourceMgrDiagnosticVerifier::ExpectedDiag::match(StringRef str) const {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  အကယ်၍ (substringRegex)
    ပြန်ပေး substringRegex->match(str);
  ပြန်ပေး str.contains(substring);
}

bool SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef strToProcess = substring;
  နေစဉ် (!strToProcess.empty()) {
    // Find the next regex block.
    size_t regexIt = strToProcess.find("{{");
    အကယ်၍ (regexIt == StringRef::npos) {
      regexOS << Regex::escape(strToProcess);
      bha_nga;
    }
    regexOS << Regex::escape(strToProcess.take_front(regexIt));
    strToProcess = strToProcess.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regexEndIt = strToProcess.find("}}");
    အကယ်၍ (regexEndIt == StringRef::npos)
      ပြန်ပေး emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regexBlock = strToProcess.take_front(regexEndIt);

    // Validate that the regex is actually valid.
    std::string regexError;
    အကယ်၍ (!Regex(regexBlock).isValid(regexError))
      ပြန်ပေး emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regexBlock << ')';
    strToProcess = strToProcess.drop_front(regexEndIt + 2);
  }
  substringRegex = Regex(regexStr);
  ပြန်ပေး true;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  const MemoryBuffer *buf) {
  // If the buffer is invalid, return an empty list.
  အကယ်၍ (!buf)
    ပြန်ပေး {};
  auto &expectedDiags = expectedDiagsPerFile[buf->getBufferIdentifier()];

  // The number of the last line that did not correlate to a designator.
  unsigned lastNonDesignatorLine = 0;

  // The indices of designators that apply to the next non designator line.
  SmallVector<unsigned, 1> designatorsForNextLine;

  // Scan the file for expected-* designators.
  SmallVector<StringRef, 100> lines;
  buf->getBuffer().split(lines, '\n');
  အတွက် (unsigned lineNo = 0, e = lines.size(); lineNo < e; ++lineNo) {
    SmallVector<StringRef, 4> matches;
    အကယ်၍ (!expected.match(lines[lineNo].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      အကယ်၍ (!designatorsForNextLine.empty()) {
        အတွက် (unsigned diagIndex : designatorsForNextLine)
          expectedDiags[diagIndex].lineNo = lineNo + 1;
        designatorsForNextLine.clear();
      }
      lastNonDesignatorLine = lineNo;
      continue;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].data());

    SourceMgr::DiagKind kind;
    အကယ်၍ (matches[1] == "error")
      kind = SourceMgr::DK_Error;
    မဟုတ်ပါက အကယ်၍ (matches[1] == "warning")
      kind = SourceMgr::DK_Warning;
    မဟုတ်ပါက အကယ်၍ (matches[1] == "remark")
      kind = SourceMgr::DK_Remark;
    မဟုတ်ပါက {
      assert(matches[1] == "note");
      kind = SourceMgr::DK_Note;
    }
    ExpectedDiag record(kind, lineNo + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    အကယ်၍ (!matches[2].empty() && !record.computeRegex(os, mgr)) {
      ok = false;
      continue;
    }

    StringRef offsetMatch = matches[3];
    အကယ်၍ (!offsetMatch.empty()) {
      offsetMatch = offsetMatch.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      အကယ်၍ (offsetMatch[0] == '+' || offsetMatch[0] == '-') {
        ကိန်းပြည့် offset;
        offsetMatch.drop_front().getAsInteger(0, offset);

        အကယ်၍ (offsetMatch.front() == '+')
          record.lineNo += offset;
        မဟုတ်ပါက
          record.lineNo -= offset;
      } မဟုတ်ပါက အကယ်၍ (offsetMatch.consume_front("unknown")) {
        // This is matching unknown locations.
        record.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_back(std::move(record));
        continue;
      } မဟုတ်ပါက အကယ်၍ (offsetMatch.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        record.lineNo = lastNonDesignatorLine + 1;
      } မဟုတ်ပါက {
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
  ပြန်ပေး expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  auto expectedDiags = expectedDiagsPerFile.find(bufName);
  အကယ်၍ (expectedDiags != expectedDiagsPerFile.end())
    ပြန်ပေး MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  ပြန်ပေး std::nullopt;
}

SourceMgrDiagnosticVerifier::MatchResult
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::DiagKind kind, bool hasLoc,
                                     const MemoryBuffer *buf, unsigned lineNo,
                                     StringRef message, bool reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  အကယ်၍ (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    အကယ်၍ (buf) {
      အကယ်၍ (auto maybeDiags = getExpectedDiags(buf->getBufferIdentifier()))
        diags = *maybeDiags;
      မဟုတ်ပါက
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } မဟုတ်ပါက {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *nearMiss = nullptr;

  // If this was an expected error, remember that we saw it and return.
  အတွက် (auto &e : diags) {
    // File line must match (unless it's an unknown location).
    အကယ်၍ (hasLoc && e.lineNo != lineNo)
      continue;
    အကယ်၍ (e.match(message)) {
      အကယ်၍ (e.kind == kind) {
        e.matched = true;
        ပြန်ပေး MatchResult::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      nearMiss = &e;
    }
  }

  အကယ်၍ (!reportUnexpected)
    ပြန်ပေး MatchResult::Ignored;

  ok = false;

  // Otherwise, emit an error for the near miss.
  အကယ်၍ (nearMiss) {
    mgr.PrintMessage(os, nearMiss->fileLoc, SourceMgr::DK_Error,
                     "'" + getDiagKindStr(kind) +
                         "' diagnostic emitted when expecting a '" +
                         getDiagKindStr(nearMiss->kind) + "'");
    ပြန်ပေး MatchResult::NearMiss;
  }
  ပြန်ပေး MatchResult::Unexpected;
}

bool SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  auto checkExpectedDiags = [&](ExpectedDiag &diag) {
    အကယ်၍ (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + getDiagKindStr(diag.kind) + " \"" +
                         diag.substring + "\" was not produced");
      ok = false;
    }
  };
  အတွက် (auto &expectedDiagsPair : expectedDiagsPerFile)
    အတွက် (auto &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  အတွက် (auto &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expectedDiagsPerFile.clear();
  ပြန်ပေး ok;
}
