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
    प्रत्यागम "note";
  पक्ष SourceMgr::DK_Warning:
    प्रत्यागम "warning";
  पक्ष SourceMgr::DK_Error:
    प्रत्यागम "error";
  पक्ष SourceMgr::DK_Remark:
    प्रत्यागम "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

तार्किक SourceMgrDiagnosticVerifier::ExpectedDiag::emitError(
    raw_ostream &os, SourceMgr &mgr, ध्रुव Twine &संदेश) ध्रुव {
  यदि (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.size()));
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, संदेश, range);
  } अन्यथा {
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, संदेश);
  }
  प्रत्यागम असत्य;
}

तार्किक SourceMgrDiagnosticVerifier::ExpectedDiag::match(StringRef str) ध्रुव {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  यदि (substringRegex)
    प्रत्यागम substringRegex->match(str);
  प्रत्यागम str.contains(substring);
}

तार्किक SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef strToProcess = substring;
  यावत् (!strToProcess.empty()) {
    // Find the next regex block.
    माप_प्रकार regexIt = strToProcess.find("{{");
    यदि (regexIt == StringRef::npos) {
      regexOS << Regex::escape(strToProcess);
      भङ्ग;
    }
    regexOS << Regex::escape(strToProcess.take_front(regexIt));
    strToProcess = strToProcess.drop_front(regexIt + 2);

    // Find the end of the regex block.
    माप_प्रकार regexEndIt = strToProcess.find("}}");
    यदि (regexEndIt == StringRef::npos)
      प्रत्यागम emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regexBlock = strToProcess.take_front(regexEndIt);

    // Validate that the regex is actually valid.
    std::string regexError;
    यदि (!Regex(regexBlock).isValid(regexError))
      प्रत्यागम emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regexBlock << ')';
    strToProcess = strToProcess.drop_front(regexEndIt + 2);
  }
  substringRegex = Regex(regexStr);
  प्रत्यागम सत्य;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  ध्रुव MemoryBuffer *buf) {
  // If the buffer is invalid, return an empty list.
  यदि (!buf)
    प्रत्यागम {};
  स्वतः &expectedDiags = expectedDiagsPerFile[buf->getBufferIdentifier()];

  // The number of the last line that did not correlate to a designator.
  अचिन्हित last_Non_Designator_पंक्ति = 0;

  // The indices of designators that apply to the next non designator line.
  SmallVector<अचिन्हित, 1> designators_For_Next_पंक्ति;

  // Scan the file for expected-* designators.
  SmallVector<StringRef, 100> पंक्तियाँ_2;
  buf->getBuffer().split(पंक्तियाँ_2, '\n');
  प्रति (अचिन्हित पंक्ति_No_2 = 0, e = पंक्तियाँ_2.size(); पंक्ति_No_2 < e; ++पंक्ति_No_2) {
    SmallVector<StringRef, 4> matches;
    यदि (!expected.match(पंक्तियाँ_2[पंक्ति_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      यदि (!designators_For_Next_पंक्ति.empty()) {
        प्रति (अचिन्हित diagIndex : designators_For_Next_पंक्ति)
          expectedDiags[diagIndex].पंक्ति_No_2 = पंक्ति_No_2 + 1;
        designators_For_Next_पंक्ति.clear();
      }
      last_Non_Designator_पंक्ति = पंक्ति_No_2;
      अनुवर्त;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].data());

    SourceMgr::DiagKind kind;
    यदि (matches[1] == "error")
      kind = SourceMgr::DK_Error;
    अन्यथा यदि (matches[1] == "warning")
      kind = SourceMgr::DK_Warning;
    अन्यथा यदि (matches[1] == "remark")
      kind = SourceMgr::DK_Remark;
    अन्यथा {
      प्रतिज्ञा(matches[1] == "note");
      kind = SourceMgr::DK_Note;
    }
    ExpectedDiag record(kind, पंक्ति_No_2 + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    यदि (!matches[2].empty() && !record.computeRegex(os, mgr)) {
      ok = असत्य;
      अनुवर्त;
    }

    StringRef offsetMatch = matches[3];
    यदि (!offsetMatch.empty()) {
      offsetMatch = offsetMatch.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      यदि (offsetMatch[0] == '+' || offsetMatch[0] == '-') {
        पूर्णाङ्क offset;
        offsetMatch.drop_front().getAsInteger(0, offset);

        यदि (offsetMatch.front() == '+')
          record.पंक्ति_No_2 += offset;
        अन्यथा
          record.पंक्ति_No_2 -= offset;
      } अन्यथा यदि (offsetMatch.consume_front("unknown")) {
        // This is matching unknown locations.
        record.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_back(std::move(record));
        अनुवर्त;
      } अन्यथा यदि (offsetMatch.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        record.पंक्ति_No_2 = last_Non_Designator_पंक्ति + 1;
      } अन्यथा {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        प्रतिज्ञा(offsetMatch.consume_front("below"));
        designators_For_Next_पंक्ति.धकेलो_back(expectedDiags.size());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        record.पंक्ति_No_2 = e;
      }
    }
    expectedDiags.emplace_back(std::move(record));
  }
  प्रत्यागम expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  स्वतः expectedDiags = expectedDiagsPerFile.find(bufName);
  यदि (expectedDiags != expectedDiagsPerFile.end())
    प्रत्यागम MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  प्रत्यागम std::nullopt;
}

SourceMgrDiagnosticVerifier::MatchResult
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::DiagKind kind, तार्किक hasLoc,
                                     ध्रुव MemoryBuffer *buf, अचिन्हित पंक्ति_No_2,
                                     StringRef message, तार्किक reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  यदि (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    यदि (buf) {
      यदि (स्वतः maybeDiags = getExpectedDiags(buf->getBufferIdentifier()))
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
  प्रति (स्वतः &e : diags) {
    // File line must match (unless it's an unknown location).
    यदि (hasLoc && e.पंक्ति_No_2 != पंक्ति_No_2)
      अनुवर्त;
    यदि (e.match(message)) {
      यदि (e.kind == kind) {
        e.matched = सत्य;
        प्रत्यागम MatchResult::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      nearMiss = &e;
    }
  }

  यदि (!reportUnexpected)
    प्रत्यागम MatchResult::Ignored;

  ok = असत्य;

  // Otherwise, emit an error for the near miss.
  यदि (nearMiss) {
    mgr.PrintMessage(os, nearMiss->fileLoc, SourceMgr::DK_Error,
                     "'" + getDiagKindStr(kind) +
                         "' diagnostic emitted when expecting a '" +
                         getDiagKindStr(nearMiss->kind) + "'");
    प्रत्यागम MatchResult::NearMiss;
  }
  प्रत्यागम MatchResult::Unexpected;
}

तार्किक SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  स्वतः checkExpectedDiags = [&](ExpectedDiag &diag) {
    यदि (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + getDiagKindStr(diag.kind) + " \"" +
                         diag.substring + "\" was not produced");
      ok = असत्य;
    }
  };
  प्रति (स्वतः &expectedDiagsPair : expectedDiagsPerFile)
    प्रति (स्वतः &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  प्रति (स्वतः &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expectedDiagsPerFile.clear();
  प्रत्यागम ok;
}
