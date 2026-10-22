//===- SourceMgrDiagnosticVerifier.cpp -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/SourceMgrDiagnosticVerifier.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

using നാമാകാശ llvm;

/// Given a diagnostic kind, return a human readable string for it.
സ്ഥിതം StringRef getDiagKindStr(SourceMgr::DiagKind kind) {
  തിരഞ്ഞെടുപ്പ് (kind) {
  അവസ്ഥ SourceMgr::DK_Note:
    തിരികെ "note";
  അവസ്ഥ SourceMgr::DK_Warning:
    തിരികെ "warning";
  അവസ്ഥ SourceMgr::DK_Error:
    തിരികെ "error";
  അവസ്ഥ SourceMgr::DK_Remark:
    തിരികെ "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

തർക്കം SourceMgrDiagnosticVerifier::ExpectedDiag::emitError(
    raw_ostream &os, SourceMgr &mgr, സ്ഥിരം Twine &സന്ദേശം) സ്ഥിരം {
  എങ്കിൽ (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.size()));
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, സന്ദേശം, range);
  } അല്ലെങ്കിൽ {
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, സന്ദേശം);
  }
  തിരികെ അസത്യം;
}

തർക്കം SourceMgrDiagnosticVerifier::ExpectedDiag::match(StringRef str) സ്ഥിരം {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  എങ്കിൽ (substringRegex)
    തിരികെ substringRegex->match(str);
  തിരികെ str.contains(substring);
}

തർക്കം SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef strToProcess = substring;
  ആകുമ്പോൾ (!strToProcess.empty()) {
    // Find the next regex block.
    size_t regexIt = strToProcess.find("{{");
    എങ്കിൽ (regexIt == StringRef::npos) {
      regexOS << Regex::escape(strToProcess);
      നിർത്തുക;
    }
    regexOS << Regex::escape(strToProcess.take_front(regexIt));
    strToProcess = strToProcess.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regexEndIt = strToProcess.find("}}");
    എങ്കിൽ (regexEndIt == StringRef::npos)
      തിരികെ emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regexBlock = strToProcess.take_front(regexEndIt);

    // Validate that the regex is actually valid.
    std::string regexError;
    എങ്കിൽ (!Regex(regexBlock).isValid(regexError))
      തിരികെ emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regexBlock << ')';
    strToProcess = strToProcess.drop_front(regexEndIt + 2);
  }
  substringRegex = Regex(regexStr);
  തിരികെ സത്യം;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  സ്ഥിരം MemoryBuffer *buf) {
  // If the buffer is invalid, return an empty list.
  എങ്കിൽ (!buf)
    തിരികെ {};
  auto &expectedDiags = expectedDiagsPerFile[buf->getBufferIdentifier()];

  // The number of the last line that did not correlate to a designator.
  unsigned last_Non_Designator_വരി = 0;

  // The indices of designators that apply to the next non designator line.
  SmallVector<unsigned, 1> designators_For_Next_വരി;

  // Scan the file for expected-* designators.
  SmallVector<StringRef, 100> വരികൾ_2;
  buf->getBuffer().split(വരികൾ_2, '\n');
  വേണ്ടി (unsigned വരി_No_2 = 0, e = വരികൾ_2.size(); വരി_No_2 < e; ++വരി_No_2) {
    SmallVector<StringRef, 4> matches;
    എങ്കിൽ (!expected.match(വരികൾ_2[വരി_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      എങ്കിൽ (!designators_For_Next_വരി.empty()) {
        വേണ്ടി (unsigned diagIndex : designators_For_Next_വരി)
          expectedDiags[diagIndex].വരി_No_2 = വരി_No_2 + 1;
        designators_For_Next_വരി.clear();
      }
      last_Non_Designator_വരി = വരി_No_2;
      തുടരുക;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].data());

    SourceMgr::DiagKind kind;
    എങ്കിൽ (matches[1] == "error")
      kind = SourceMgr::DK_Error;
    അല്ലെങ്കിൽ എങ്കിൽ (matches[1] == "warning")
      kind = SourceMgr::DK_Warning;
    അല്ലെങ്കിൽ എങ്കിൽ (matches[1] == "remark")
      kind = SourceMgr::DK_Remark;
    അല്ലെങ്കിൽ {
      പ്രതിജ്ഞാ(matches[1] == "note");
      kind = SourceMgr::DK_Note;
    }
    ExpectedDiag record(kind, വരി_No_2 + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    എങ്കിൽ (!matches[2].empty() && !record.computeRegex(os, mgr)) {
      ok = അസത്യം;
      തുടരുക;
    }

    StringRef offsetMatch = matches[3];
    എങ്കിൽ (!offsetMatch.empty()) {
      offsetMatch = offsetMatch.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      എങ്കിൽ (offsetMatch[0] == '+' || offsetMatch[0] == '-') {
        പൂർണ്ണസംഖ്യ offset;
        offsetMatch.drop_front().getAsInteger(0, offset);

        എങ്കിൽ (offsetMatch.front() == '+')
          record.വരി_No_2 += offset;
        അല്ലെങ്കിൽ
          record.വരി_No_2 -= offset;
      } അല്ലെങ്കിൽ എങ്കിൽ (offsetMatch.consume_front("unknown")) {
        // This is matching unknown locations.
        record.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_back(std::move(record));
        തുടരുക;
      } അല്ലെങ്കിൽ എങ്കിൽ (offsetMatch.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        record.വരി_No_2 = last_Non_Designator_വരി + 1;
      } അല്ലെങ്കിൽ {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        പ്രതിജ്ഞാ(offsetMatch.consume_front("below"));
        designators_For_Next_വരി.തള്ളുക_back(expectedDiags.size());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        record.വരി_No_2 = e;
      }
    }
    expectedDiags.emplace_back(std::move(record));
  }
  തിരികെ expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  auto expectedDiags = expectedDiagsPerFile.find(bufName);
  എങ്കിൽ (expectedDiags != expectedDiagsPerFile.end())
    തിരികെ MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  തിരികെ std::nullopt;
}

SourceMgrDiagnosticVerifier::MatchResult
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::DiagKind kind, തർക്കം hasLoc,
                                     സ്ഥിരം MemoryBuffer *buf, unsigned വരി_No_2,
                                     StringRef message, തർക്കം reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  എങ്കിൽ (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    എങ്കിൽ (buf) {
      എങ്കിൽ (auto maybeDiags = getExpectedDiags(buf->getBufferIdentifier()))
        diags = *maybeDiags;
      അല്ലെങ്കിൽ
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } അല്ലെങ്കിൽ {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *nearMiss = nullptr;

  // If this was an expected error, remember that we saw it and return.
  വേണ്ടി (auto &e : diags) {
    // File line must match (unless it's an unknown location).
    എങ്കിൽ (hasLoc && e.വരി_No_2 != വരി_No_2)
      തുടരുക;
    എങ്കിൽ (e.match(message)) {
      എങ്കിൽ (e.kind == kind) {
        e.matched = സത്യം;
        തിരികെ MatchResult::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      nearMiss = &e;
    }
  }

  എങ്കിൽ (!reportUnexpected)
    തിരികെ MatchResult::Ignored;

  ok = അസത്യം;

  // Otherwise, emit an error for the near miss.
  എങ്കിൽ (nearMiss) {
    mgr.PrintMessage(os, nearMiss->fileLoc, SourceMgr::DK_Error,
                     "'" + getDiagKindStr(kind) +
                         "' diagnostic emitted when expecting a '" +
                         getDiagKindStr(nearMiss->kind) + "'");
    തിരികെ MatchResult::NearMiss;
  }
  തിരികെ MatchResult::Unexpected;
}

തർക്കം SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  auto checkExpectedDiags = [&](ExpectedDiag &diag) {
    എങ്കിൽ (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + getDiagKindStr(diag.kind) + " \"" +
                         diag.substring + "\" was not produced");
      ok = അസത്യം;
    }
  };
  വേണ്ടി (auto &expectedDiagsPair : expectedDiagsPerFile)
    വേണ്ടി (auto &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  വേണ്ടി (auto &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expectedDiagsPerFile.clear();
  തിരികെ ok;
}
