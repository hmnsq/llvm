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

उपयुज्य नामाकाश llvm;

/// Given a diagnostic kind, return a human readable string for it.
स्थिर StringRef get_Diag_विधा_Str(SourceMgr::Diag_विधा विधा) {
  विकल्प (विधा) {
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
    raw_ostream &os, SourceMgr &mgr, ध्रुव Twine &सन्देशः) ध्रुव {
  यदि (fileLoc.isValid()) {
    SMRange range(fileLoc, SMLoc::getFromPointer(fileLoc.getPointer() +
                                                 substring.आकार_2()));
    mgr.Print_सन्देशः(os, fileLoc, SourceMgr::DK_Error, सन्देशः, range);
  } अन्यथा {
    mgr.Print_सन्देशः(os, fileLoc, SourceMgr::DK_Error, सन्देशः);
  }
  प्रत्यागम असत्य;
}

तार्किक SourceMgrDiagnosticVerifier::ExpectedDiag::मेलनम्(StringRef str) ध्रुव {
  // If this isn't a regex diagnostic, we simply check if the string was
  // contained.
  यदि (substringRegex)
    प्रत्यागम substringRegex->मेलनम्(str);
  प्रत्यागम str.अन्तर्भवति(substring);
}

तार्किक SourceMgrDiagnosticVerifier::ExpectedDiag::computeRegex(raw_ostream &os,
                                                             SourceMgr &mgr) {
  std::string regexStr;
  raw_string_ostream regexOS(regexStr);
  StringRef str_गन्तव्यम्_Process = substring;
  यावत् (!str_गन्तव्यम्_Process.रिक्तानि()) {
    // Find the next regex block.
    size_t regexIt = str_गन्तव्यम्_Process.find("{{");
    यदि (regexIt == StringRef::npos) {
      regexOS << Regex::escape(str_गन्तव्यम्_Process);
      भङ्ग;
    }
    regexOS << Regex::escape(str_गन्तव्यम्_Process.take_front(regexIt));
    str_गन्तव्यम्_Process = str_गन्तव्यम्_Process.drop_front(regexIt + 2);

    // Find the end of the regex block.
    size_t regex_अन्तः_It = str_गन्तव्यम्_Process.find("}}");
    यदि (regex_अन्तः_It == StringRef::npos)
      प्रत्यागम emitError(os, mgr, "found start of regex with no end '}}'");
    StringRef regex_खण्डम् = str_गन्तव्यम्_Process.take_front(regex_अन्तः_It);

    // Validate that the regex is actually valid.
    std::string regexError;
    यदि (!Regex(regex_खण्डम्).isValid(regexError))
      प्रत्यागम emitError(os, mgr, "invalid regex: " + regexError);

    regexOS << '(' << regex_खण्डम् << ')';
    str_गन्तव्यम्_Process = str_गन्तव्यम्_Process.drop_front(regex_अन्तः_It + 2);
  }
  substringRegex = Regex(regexStr);
  प्रत्यागम सत्य;
}

MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>
SourceMgrDiagnosticVerifier::computeExpectedDiags(raw_ostream &os,
                                                  SourceMgr &mgr,
                                                  ध्रुव स्मृतिः_बफर *buf) {
  // If the buffer is invalid, return an empty list.
  यदि (!buf)
    प्रत्यागम {};
  स्वचालित &expectedDiags = expected_Diags_प्रत्येकम्_File[buf->get_बफर_Identifier()];

  // The number of the last line that did not correlate to a designator.
  अचिह्नित चरमः_Non_Designator_पङ्क्तिः = 0;

  // The indices of designators that apply to the next non designator line.
  Small_सदिशम्<अचिह्नित, 1> designators_For_अग्र_पङ्क्तिः;

  // Scan the file for expected-* designators.
  Small_सदिशम्<StringRef, 100> पङ्क्तयः_2;
  buf->get_बफर().विभज(पङ्क्तयः_2, '\n');
  प्रति (अचिह्नित पङ्क्तिः_No_2 = 0, e = पङ्क्तयः_2.आकार_2(); पङ्क्तिः_No_2 < e; ++पङ्क्तिः_No_2) {
    Small_सदिशम्<StringRef, 4> matches;
    यदि (!expected.मेलनम्(पङ्क्तयः_2[पङ्क्तिः_No_2].rtrim(), &matches)) {
      // Check for designators that apply to this line.
      यदि (!designators_For_अग्र_पङ्क्तिः.रिक्तानि()) {
        प्रति (अचिह्नित diag_सूचकांक : designators_For_अग्र_पङ्क्तिः)
          expectedDiags[diag_सूचकांक].पङ्क्तिः_No_2 = पङ्क्तिः_No_2 + 1;
        designators_For_अग्र_पङ्क्तिः.clear();
      }
      चरमः_Non_Designator_पङ्क्तिः = पङ्क्तिः_No_2;
      अनुवर्त;
    }

    // Point to the start of expected-*.
    SMLoc expectedStart = SMLoc::getFromPointer(matches[0].दत्त());

    SourceMgr::Diag_विधा विधा;
    यदि (matches[1] == "error")
      विधा = SourceMgr::DK_Error;
    अन्यथा यदि (matches[1] == "warning")
      विधा = SourceMgr::DK_Warning;
    अन्यथा यदि (matches[1] == "remark")
      विधा = SourceMgr::DK_Remark;
    अन्यथा {
      प्रतिज्ञा(matches[1] == "note");
      विधा = SourceMgr::DK_Note;
    }
    ExpectedDiag लेखः_2(विधा, पङ्क्तिः_No_2 + 1, expectedStart, matches[5]);

    // Check to see if this is a regex match, i.e. it includes the `-re`.
    यदि (!matches[2].रिक्तानि() && !लेखः_2.computeRegex(os, mgr)) {
      ok = असत्य;
      अनुवर्त;
    }

    StringRef विस्थापनम्_मेलनम् = matches[3];
    यदि (!विस्थापनम्_मेलनम्.रिक्तानि()) {
      विस्थापनम्_मेलनम् = विस्थापनम्_मेलनम्.drop_front(1);

      // Get the integer value without the @ and +/- prefix.
      यदि (विस्थापनम्_मेलनम्[0] == '+' || विस्थापनम्_मेलनम्[0] == '-') {
        पूर्णाङ्क विस्थापनम्_2;
        विस्थापनम्_मेलनम्.drop_front().getAsInteger(0, विस्थापनम्_2);

        यदि (विस्थापनम्_मेलनम्.front() == '+')
          लेखः_2.पङ्क्तिः_No_2 += विस्थापनम्_2;
        अन्यथा
          लेखः_2.पङ्क्तिः_No_2 -= विस्थापनम्_2;
      } अन्यथा यदि (विस्थापनम्_मेलनम्.consume_front("unknown")) {
        // This is matching unknown locations.
        लेखः_2.fileLoc = SMLoc();
        expectedUnknownLocDiags.emplace_पृष्ठभागः(std::move(लेखः_2));
        अनुवर्त;
      } अन्यथा यदि (विस्थापनम्_मेलनम्.consume_front("above")) {
        // If the designator applies 'above' we add it to the last non
        // designator line.
        लेखः_2.पङ्क्तिः_No_2 = चरमः_Non_Designator_पङ्क्तिः + 1;
      } अन्यथा {
        // Otherwise, this is a 'below' designator and applies to the next
        // non-designator line.
        प्रतिज्ञा(विस्थापनम्_मेलनम्.consume_front("below"));
        designators_For_अग्र_पङ्क्तिः.प्रक्षिप_पृष्ठभागः(expectedDiags.आकार_2());

        // Set the line number to the last in the case that this designator
        // ends up dangling.
        लेखः_2.पङ्क्तिः_No_2 = e;
      }
    }
    expectedDiags.emplace_पृष्ठभागः(std::move(लेखः_2));
  }
  प्रत्यागम expectedDiags;
}

std::optional<MutableArrayRef<SourceMgrDiagnosticVerifier::ExpectedDiag>>
SourceMgrDiagnosticVerifier::getExpectedDiags(StringRef bufName) {
  स्वचालित expectedDiags = expected_Diags_प्रत्येकम्_File.find(bufName);
  यदि (expectedDiags != expected_Diags_प्रत्येकम्_File.अन्तः())
    प्रत्यागम MutableArrayRef<ExpectedDiag>(expectedDiags->second);
  प्रत्यागम std::nullopt;
}

SourceMgrDiagnosticVerifier::मेलनम्_परिणाम
SourceMgrDiagnosticVerifier::process(raw_ostream &os, SourceMgr &mgr,
                                     SourceMgr::Diag_विधा विधा, तार्किक hasLoc,
                                     ध्रुव स्मृतिः_बफर *buf, अचिह्नित पङ्क्तिः_No_2,
                                     StringRef सन्देशः_2, तार्किक reportUnexpected) {
  MutableArrayRef<ExpectedDiag> diags;
  यदि (hasLoc) {
    // If the buffer couldn't be resolved, `diags` stays empty: a diagnostic
    // with a location in an unknown file can never match anything.
    यदि (buf) {
      यदि (स्वचालित maybeDiags = getExpectedDiags(buf->get_बफर_Identifier()))
        diags = *maybeDiags;
      अन्यथा
        diags = computeExpectedDiags(os, mgr, buf);
    }
  } अन्यथा {
    diags = expectedUnknownLocDiags;
  }

  // Search for a matching expected diagnostic.
  // If we find something that is close then emit a more specific error.
  ExpectedDiag *near_अलब्धिः = शून्यसूचकः;

  // If this was an expected error, remember that we saw it and return.
  प्रति (स्वचालित &e : diags) {
    // File line must match (unless it's an unknown location).
    यदि (hasLoc && e.पङ्क्तिः_No_2 != पङ्क्तिः_No_2)
      अनुवर्त;
    यदि (e.मेलनम्(सन्देशः_2)) {
      यदि (e.विधा == विधा) {
        e.matched = सत्य;
        प्रत्यागम मेलनम्_परिणाम::Matched;
      }

      // If this only differs based on the diagnostic kind, then consider it
      // to be a near miss.
      near_अलब्धिः = &e;
    }
  }

  यदि (!reportUnexpected)
    प्रत्यागम मेलनम्_परिणाम::Ignored;

  ok = असत्य;

  // Otherwise, emit an error for the near miss.
  यदि (near_अलब्धिः) {
    mgr.Print_सन्देशः(os, near_अलब्धिः->fileLoc, SourceMgr::DK_Error,
                     "'" + get_Diag_विधा_Str(विधा) +
                         "' diagnostic emitted when expecting a '" +
                         get_Diag_विधा_Str(near_अलब्धिः->विधा) + "'");
    प्रत्यागम मेलनम्_परिणाम::Near_अलब्धिः;
  }
  प्रत्यागम मेलनम्_परिणाम::Unexpected;
}

तार्किक SourceMgrDiagnosticVerifier::verify(raw_ostream &os, SourceMgr &mgr) {
  // Verify that all expected errors were seen.
  स्वचालित checkExpectedDiags = [&](ExpectedDiag &diag) {
    यदि (!diag.matched) {
      diag.emitError(os, mgr,
                     "expected " + get_Diag_विधा_Str(diag.विधा) + " \"" +
                         diag.substring + "\" was not produced");
      ok = असत्य;
    }
  };
  प्रति (स्वचालित &expectedDiagsPair : expected_Diags_प्रत्येकम्_File)
    प्रति (स्वचालित &diag : expectedDiagsPair.second)
      checkExpectedDiags(diag);
  प्रति (स्वचालित &diag : expectedUnknownLocDiags)
    checkExpectedDiags(diag);
  expected_Diags_प्रत्येकम्_File.clear();
  प्रत्यागम ok;
}
