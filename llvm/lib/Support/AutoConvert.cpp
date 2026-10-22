//===- AutoConvert.cpp - Auto conversion between ASCII/EBCDIC -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains functions used for auto conversion between
// ASCII/EBCDIC codepages specific to z/OS.
//
//===----------------------------------------------------------------------===//

#ifdef __MVS__

#अन्तर्भाव "llvm/Support/AutoConvert.h"
#अन्तर्भाव <cassert>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <unistd.h>

using नामाकाश llvm;

स्थिर पूर्णाङ्क savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

पूर्णाङ्क disablezOSAutoConversion(पूर्णाङ्क FD) {
  स्थिर अचर संरचना f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  फर्काऊ fcntl(FD, F_CONTROL_CVT, &Convert);
}

पूर्णाङ्क restorezOSStdHandleAutoConversion(पूर्णाङ्क FD) {
  प्रतिज्ञा(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  यदि (savedStdHandleAutoConversionMode[FD] == -1)
    फर्काऊ 0;
  संरचना f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  फर्काऊ (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

पूर्णाङ्क enablezOSAutoConversion(पूर्णाङ्क FD) {
  संरचना f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  यदि (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    फर्काऊ -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  यदि (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    फर्काऊ 0;

  // Save the state of std handles before we make changes to it.
  यदि ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  यदि (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  अन्यथा
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  फर्काऊ fcntl(FD, F_CONTROL_CVT, &Query);
}

पूर्णाङ्क enablezOSAutoConversionCcsid(पूर्णाङ्क FD, पूर्णाङ्क ccsid) {
  संरचना f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  यदि (ccsid == FT_UNTAGGED)
    फर्काऊ -1;
  cvt.fccsid = ccsid;
  फर्काऊ fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, पूर्णाङ्क CCSID,
                                    तार्किक IsText) {
  प्रतिज्ञा((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_द्विआधारी)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  संरचना file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  यदि (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    यदि (त्रुटि_सं == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      त्रुटि_सं = 0;
    अन्यथा
      फर्काऊ errnoAsErrorCode();
  }
  फर्काऊ std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(अचर Twine &FileName,
                                       अचर sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  यदि (FD != -1) {
    संरचना f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    यदि (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      फर्काऊ std::error_code(त्रुटि_सं, std::generic_category());
    फर्काऊ Query.fccsid;
  }
  संरचना stat Attr;
  यदि (stat(FileName.str().c_str(), &Attr) == -1)
    फर्काऊ std::error_code(त्रुटि_सं, std::generic_category());
  फर्काऊ Attr.st_tag.ft_ccsid;
}

ErrorOr<तार्किक> llvm::needzOSConversion(अचर Twine &FileName,
                                      अचर sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  यदि (std::error_code EC = Ccsid.getError())
    फर्काऊ EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  विकल्प (*Ccsid) {
  पक्ष CCSID_UTF_8:
  पक्ष CCSID_ISO8859_1:
  पक्ष FT_द्विआधारी:
    फर्काऊ असत्य;
  स्वभाव:
    फर्काऊ सत्य;
  }
}

std::error_code
llvm::copyFileTagAttributes(अचर std::string &Source,
                            अचर sys::fs::file_t DestinationFD) {
  संरचना stat SourceAttributes;
  यदि (stat(Source.c_str(), &SourceAttributes) == -1)
    फर्काऊ std::error_code(त्रुटि_सं, std::generic_category());

  यदि (SourceAttributes.st_tag.ft_txtflag)
    यदि (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      फर्काऊ errnoAsErrorCode();

  फर्काऊ setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
