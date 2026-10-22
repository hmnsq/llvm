//===- SourceMgrDiagnosticVerifier.cpp -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/SourceMgrDiagnosticVerifier.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

using நாமாகாஶ llvm;

/// Given a diagnostic kind, return a human readable string for it.
நிலையான StringRef getDiagKindStr(SourceMgr::DiagKind kind) {
  தேர்வு (kind) {
  நிலை SourceMgr::DK_Note:
    திரும்பு "note";
  நிலை SourceMgr::DK_Warning:
    திரும்பு "warning";
  நிலை SourceMgr::DK_Error:
    திரும்பு "error";
  நிலை SourceMgr::DK_Remark:
    திரும்பு "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

தருக்க SourceMgrDiagnosticVerifier::ExpectedDiag::emitError(
    raw_ostream &os, SourceMgr &mgr, மாறிலி Twine &செய்தி) மாறிலி {
  எனில் (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.size()));
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, செய்தி, range);
  } இல்லையெனில் {
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, செய்தி);
  }
  திரும்பு பொய்;
}

தருக்க SourceMgrDiagnosticVerifier::ExpectedDiag::match(StringRef str) மாறிலி {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  எனில் (substringRegex)
    திரும்பு substringRegex->match(str);
  திரும்பு str.contains(substring);
}

தருக்க SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef strToProcess = substring;
  போது (!strToProcess.empty()) {
    // Find the next regex block.
    size_t regexIt = strToProcess.find("{{");
    எனில் (regexIt == StringRef::npos) {
      regexOS << Regex::escape(strToProcess);
      நிறுத்து;
    }
    regexOS << Regex::escape(strToProcess.take_front(regexIt));
    strToProcess = strToProcess.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regexEndIt = strToProcess.find("}}");
    எனில் (regexEndIt == StringRef::npos)
      திரும்பு emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regexBlock = strToProcess.take_front(regexEndIt);

    // Validate that the regex is actually valid.
    std::string regexError;
    எனில் (!Regex(regexBlock).isValid(regexError))
      திரும்பு emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regexBlock << ')';
    strToProcess = strToProcess.drop_front(regexEndIt + 2);
  }
  substringRegex = Regex(regexStr);
  திரும்பு உண்மை;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  மாறிலி MemoryBuffer *buf) {
  // If the buffer is invalid, return an empty list.
  எனில் (!buf)
    திரும்பு {};
  auto &expectedDiags = expectedDiagsPerFile[buf->getBufferIdentifier()];

  // The number of the last line that did not correlate to a designator.
  unsigned last_Non_Designator_வரி = 0;

  // The indices of designators that apply to the next non designator line.
  SmallVector<unsigned, 1> designators_For_Next_வரி;

  // Scan the file for expected-* designators.
  SmallVector<StringRef, 100> வரிகள்_2;
  buf->getBuffer().split(வரிகள்_2, '\n');
  க்கு (unsigned வரி_No_2 = 0, e = வரிகள்_2.size(); வரி_No_2 < e; ++வரி_No_2) {
    SmallVector<StringRef, 4> matches;
    எனில் (!expected.match(வரிகள்_2[வரி_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      எனில் (!designators_For_Next_வரி.empty()) {
        க்கு (unsigned diagIndex : designators_For_Next_வரி)
          expectedDiags[diagIndex].வரி_No_2 = வரி_No_2 + 1;
        designators_For_Next_வரி.clear();
      }
      last_Non_Designator_வரி = வரி_No_2;
      தொடர்;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].data());

    SourceMgr::DiagKind kind;
    எனில் (matches[1] == "error")
      kind = SourceMgr::DK_Error;
    இல்லையெனில் எனில் (matches[1] == "warning")
      kind = SourceMgr::DK_Warning;
    இல்லையெனில் எனில் (matches[1] == "remark")
      kind = SourceMgr::DK_Remark;
    இல்லையெனில் {
      ப்ரதிஜ்ஞா(matches[1] == "note");
      kind = SourceMgr::DK_Note;
    }
    ExpectedDiag record(kind, வரி_No_2 + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    எனில் (!matches[2].empty() && !record.computeRegex(os, mgr)) {
      ok = பொய்;
      தொடர்;
    }

    StringRef offsetMatch = matches[3];
    எனில் (!offsetMatch.empty()) {
      offsetMatch = offsetMatch.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      எனில் (offsetMatch[0] == '+' || offsetMatch[0] == '-') {
        முழு offset;
        offsetMatch.drop_front().getAsInteger(0, offset);

        எனில் (offsetMatch.front() == '+')
          record.வரி_No_2 += offset;
        இல்லையெனில்
          record.வரி_No_2 -= offset;
      } இல்லையெனில் எனில் (offsetMatch.consume_front("unknown")) {
        // This is matching unknown locations.
        record.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_back(std::move(record));
        தொடர்;
      } இல்லையெனில் எனில் (offsetMatch.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        record.வரி_No_2 = last_Non_Designator_வரி + 1;
      } இல்லையெனில் {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        ப்ரதிஜ்ஞா(offsetMatch.consume_front("below"));
        designators_For_Next_வரி.தள்ளு_back(expectedDiags.size());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        record.வரி_No_2 = e;
      }
    }
    expectedDiags.emplace_back(std::move(record));
  }
  திரும்பு expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  auto expectedDiags = expectedDiagsPerFile.find(bufName);
  எனில் (expectedDiags != expectedDiagsPerFile.end())
    திரும்பு MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  திரும்பு std::nullopt;
}

SourceMgrDiagnosticVerifier::MatchResult
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::DiagKind kind, தருக்க hasLoc,
                                     மாறிலி MemoryBuffer *buf, unsigned வரி_No_2,
                                     StringRef message, தருக்க reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  எனில் (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    எனில் (buf) {
      எனில் (auto maybeDiags = getExpectedDiags(buf->getBufferIdentifier()))
        diags = *maybeDiags;
      இல்லையெனில்
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } இல்லையெனில் {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *nearMiss = nullptr;

  // If this was an expected error, remember that we saw it and return.
  க்கு (auto &e : diags) {
    // File line must match (unless it's an unknown location).
    எனில் (hasLoc && e.வரி_No_2 != வரி_No_2)
      தொடர்;
    எனில் (e.match(message)) {
      எனில் (e.kind == kind) {
        e.matched = உண்மை;
        திரும்பு MatchResult::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      nearMiss = &e;
    }
  }

  எனில் (!reportUnexpected)
    திரும்பு MatchResult::Ignored;

  ok = பொய்;

  // Otherwise, emit an error for the near miss.
  எனில் (nearMiss) {
    mgr.PrintMessage(os, nearMiss->fileLoc, SourceMgr::DK_Error,
                     "'" + getDiagKindStr(kind) +
                         "' diagnostic emitted when expecting a '" +
                         getDiagKindStr(nearMiss->kind) + "'");
    திரும்பு MatchResult::NearMiss;
  }
  திரும்பு MatchResult::Unexpected;
}

தருக்க SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  auto checkExpectedDiags = [&](ExpectedDiag &diag) {
    எனில் (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + getDiagKindStr(diag.kind) + " \"" +
                         diag.substring + "\" was not produced");
      ok = பொய்;
    }
  };
  க்கு (auto &expectedDiagsPair : expectedDiagsPerFile)
    க்கு (auto &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  க்கு (auto &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expectedDiagsPerFile.clear();
  திரும்பு ok;
}
