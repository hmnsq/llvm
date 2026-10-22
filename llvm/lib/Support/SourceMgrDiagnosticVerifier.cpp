//===- SourceMgrDiagnosticVerifier.cpp -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/SourceMgrDiagnosticVerifier.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/raw_ostream.h"

using namespace llvm;

/// Given a diagnostic kind, return a human readable string for it.
अटल StringRef getDiagKindStr(SourceMgr::DiagKind kind) {
  चयन (kind) {
  हाल SourceMgr::DK_Note:
    वापस "note";
  हाल SourceMgr::DK_Warning:
    वापस "warning";
  हाल SourceMgr::DK_Error:
    वापस "error";
  हाल SourceMgr::DK_Remark:
    वापस "remark";
  }
  llvm_unreachable("Unknown SourceMgr::DiagKind");
}

bool SourceMgrDiagnosticVerifier::ExpectedDiag::emitError(
    raw_ostream &os, SourceMgr &mgr, स्थिर Twine &संदेश) स्थिर {
  अगर (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.आकार_2()));
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, संदेश, range);
  } अन्यथा {
    mgr.PrintMessage(os, fileLoc, SourceMgr::DK_Error, संदेश);
  }
  वापस false;
}

bool SourceMgrDiagnosticVerifier::ExpectedDiag::match(StringRef str) स्थिर {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  अगर (substringRegex)
    वापस substringRegex->match(str);
  वापस str.contains(substring);
}

bool SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef strToProcess = substring;
  जबतक (!strToProcess.empty()) {
    // Find the next regex block.
    माप_प्रकार regexIt = strToProcess.find("{{");
    अगर (regexIt == StringRef::npos) {
      regexOS << Regex::escape(strToProcess);
      अवरोध;
    }
    regexOS << Regex::escape(strToProcess.take_front(regexIt));
    strToProcess = strToProcess.drop_front(regexIt + 2);

    // Find the end of the regex block.
    माप_प्रकार regexEndIt = strToProcess.find("}}");
    अगर (regexEndIt == StringRef::npos)
      वापस emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regexBlock = strToProcess.take_front(regexEndIt);

    // Validate that the regex is actually valid.
    std::string regexError;
    अगर (!Regex(regexBlock).isValid(regexError))
      वापस emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regexBlock << ')';
    strToProcess = strToProcess.drop_front(regexEndIt + 2);
  }
  substringRegex = Regex(regexStr);
  वापस true;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  स्थिर Memory_बफर *buf) {
  // If the buffer is invalid, return an empty list.
  अगर (!buf)
    वापस {};
  स्वतः &expectedDiags = expectedDiagsPerFile[buf->get_बफर_Identifier()];

  // The number of the last line that did not correlate to a designator.
  अचिन्हित last_Non_Designator_पंक्ति = 0;

  // The indices of designators that apply to the next non designator line.
  SmallVector<अचिन्हित, 1> designators_For_अग्र_पंक्ति;

  // Scan the file for expected-* designators.
  SmallVector<StringRef, 100> पंक्तियाँ_2;
  buf->get_बफर().split(पंक्तियाँ_2, '\n');
  क्रम (अचिन्हित पंक्ति_No_2 = 0, e = पंक्तियाँ_2.आकार_2(); पंक्ति_No_2 < e; ++पंक्ति_No_2) {
    SmallVector<StringRef, 4> matches;
    अगर (!expected.match(पंक्तियाँ_2[पंक्ति_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      अगर (!designators_For_अग्र_पंक्ति.empty()) {
        क्रम (अचिन्हित diag_सूचकांक : designators_For_अग्र_पंक्ति)
          expectedDiags[diag_सूचकांक].पंक्ति_No_2 = पंक्ति_No_2 + 1;
        designators_For_अग्र_पंक्ति.सफा();
      }
      last_Non_Designator_पंक्ति = पंक्ति_No_2;
      जारी;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].तथ्य());

    SourceMgr::DiagKind kind;
    अगर (matches[1] == "error")
      kind = SourceMgr::DK_Error;
    अन्यथा अगर (matches[1] == "warning")
      kind = SourceMgr::DK_Warning;
    अन्यथा अगर (matches[1] == "remark")
      kind = SourceMgr::DK_Remark;
    अन्यथा {
      निश्चित(matches[1] == "note");
      kind = SourceMgr::DK_Note;
    }
    ExpectedDiag record(kind, पंक्ति_No_2 + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    अगर (!matches[2].empty() && !record.computeRegex(os, mgr)) {
      ok = false;
      जारी;
    }

    StringRef offsetMatch = matches[3];
    अगर (!offsetMatch.empty()) {
      offsetMatch = offsetMatch.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      अगर (offsetMatch[0] == '+' || offsetMatch[0] == '-') {
        पूर्णांक offset;
        offsetMatch.drop_front().getAsInteger(0, offset);

        अगर (offsetMatch.front() == '+')
          record.पंक्ति_No_2 += offset;
        अन्यथा
          record.पंक्ति_No_2 -= offset;
      } अन्यथा अगर (offsetMatch.consume_front("unknown")) {
        // This is matching unknown locations.
        record.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_back(std::move(record));
        जारी;
      } अन्यथा अगर (offsetMatch.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        record.पंक्ति_No_2 = last_Non_Designator_पंक्ति + 1;
      } अन्यथा {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        निश्चित(offsetMatch.consume_front("below"));
        designators_For_अग्र_पंक्ति.धकेलो_back(expectedDiags.आकार_2());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        record.पंक्ति_No_2 = e;
      }
    }
    expectedDiags.emplace_back(std::move(record));
  }
  वापस expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  स्वतः expectedDiags = expectedDiagsPerFile.find(bufName);
  अगर (expectedDiags != expectedDiagsPerFile.end())
    वापस MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  वापस std::nullopt;
}

SourceMgrDiagnosticVerifier::Match_परिणाम
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::DiagKind kind, bool hasLoc,
                                     स्थिर Memory_बफर *buf, अचिन्हित पंक्ति_No_2,
                                     StringRef message, bool reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  अगर (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    अगर (buf) {
      अगर (स्वतः maybeDiags = getExpectedDiags(buf->get_बफर_Identifier()))
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
  क्रम (स्वतः &e : diags) {
    // File line must match (unless it's an unknown location).
    अगर (hasLoc && e.पंक्ति_No_2 != पंक्ति_No_2)
      जारी;
    अगर (e.match(message)) {
      अगर (e.kind == kind) {
        e.matched = true;
        वापस Match_परिणाम::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      nearMiss = &e;
    }
  }

  अगर (!reportUnexpected)
    वापस Match_परिणाम::Ignored;

  ok = false;

  // Otherwise, emit an error for the near miss.
  अगर (nearMiss) {
    mgr.PrintMessage(os, nearMiss->fileLoc, SourceMgr::DK_Error,
                     "'" + getDiagKindStr(kind) +
                         "' diagnostic emitted when expecting a '" +
                         getDiagKindStr(nearMiss->kind) + "'");
    वापस Match_परिणाम::NearMiss;
  }
  वापस Match_परिणाम::Unexpected;
}

bool SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  स्वतः checkExpectedDiags = [&](ExpectedDiag &diag) {
    अगर (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + getDiagKindStr(diag.kind) + " \"" +
                         diag.substring + "\" was not produced");
      ok = false;
    }
  };
  क्रम (स्वतः &expectedDiagsPair : expectedDiagsPerFile)
    क्रम (स्वतः &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  क्रम (स्वतः &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expectedDiagsPerFile.सफा();
  वापस ok;
}
