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
  स्थिर ध्रुव संरचना f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  पटिनिवत्त fcntl(FD, F_CONTROL_CVT, &Convert);
}

पूर्णाङ्क restorezOSStdHandleAutoConversion(पूर्णाङ्क FD) {
  प्रतिज्ञा(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  सचे (savedStdHandleAutoConversionMode[FD] == -1)
    पटिनिवत्त 0;
  संरचना f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  पटिनिवत्त (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

पूर्णाङ्क enablezOSAutoConversion(पूर्णाङ्क FD) {
  संरचना f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  सचे (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    पटिनिवत्त -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  सचे (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    पटिनिवत्त 0;

  // Save the state of std handles before we make changes to it.
  सचे ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  सचे (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  अञ्ञथा
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  पटिनिवत्त fcntl(FD, F_CONTROL_CVT, &Query);
}

पूर्णाङ्क enablezOSAutoConversionCcsid(पूर्णाङ्क FD, पूर्णाङ्क ccsid) {
  संरचना f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  सचे (ccsid == FT_UNTAGGED)
    पटिनिवत्त -1;
  cvt.fccsid = ccsid;
  पटिनिवत्त fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, पूर्णाङ्क CCSID,
                                    तार्किक IsText) {
  प्रतिज्ञा((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_द्विमान)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  संरचना file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  सचे (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    सचे (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    अञ्ञथा
      पटिनिवत्त errnoAsErrorCode();
  }
  पटिनिवत्त std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(ध्रुव Twine &FileName,
                                       ध्रुव sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  सचे (FD != -1) {
    संरचना f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    सचे (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      पटिनिवत्त std::error_code(errno, std::generic_category());
    पटिनिवत्त Query.fccsid;
  }
  संरचना stat Attr;
  सचे (stat(FileName.str().c_str(), &Attr) == -1)
    पटिनिवत्त std::error_code(errno, std::generic_category());
  पटिनिवत्त Attr.st_tag.ft_ccsid;
}

ErrorOr<तार्किक> llvm::needzOSConversion(ध्रुव Twine &FileName,
                                      ध्रुव sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  सचे (std::error_code EC = Ccsid.getError())
    पटिनिवत्त EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  विकल्प (*Ccsid) {
  पक्ष CCSID_UTF_8:
  पक्ष CCSID_ISO8859_1:
  पक्ष FT_द्विमान:
    पटिनिवत्त मिच्छा;
  स्वभाव:
    पटिनिवत्त सच्च;
  }
}

std::error_code
llvm::copyFileTagAttributes(ध्रुव std::string &Source,
                            ध्रुव sys::fs::file_t DestinationFD) {
  संरचना stat SourceAttributes;
  सचे (stat(Source.c_str(), &SourceAttributes) == -1)
    पटिनिवत्त std::error_code(errno, std::generic_category());

  सचे (SourceAttributes.st_tag.ft_txtflag)
    सचे (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      पटिनिवत्त errnoAsErrorCode();

  पटिनिवत्त setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
