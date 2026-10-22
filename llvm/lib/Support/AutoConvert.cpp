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

स्थिर पूर्णांक savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

पूर्णांक disablezOSAutoConversion(पूर्णांक FD) {
  स्थिर स्थिरांक संरचना f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  परत fcntl(FD, F_CONTROL_CVT, &Convert);
}

पूर्णांक restorezOSStdHandleAutoConversion(पूर्णांक FD) {
  प्रतिज्ञा(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  जर (savedStdHandleAutoConversionMode[FD] == -1)
    परत 0;
  संरचना f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  परत (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

पूर्णांक enablezOSAutoConversion(पूर्णांक FD) {
  संरचना f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  जर (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    परत -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  जर (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    परत 0;

  // Save the state of std handles before we make changes to it.
  जर ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  जर (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  अन्यथा
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  परत fcntl(FD, F_CONTROL_CVT, &Query);
}

पूर्णांक enablezOSAutoConversionCcsid(पूर्णांक FD, पूर्णांक ccsid) {
  संरचना f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  जर (ccsid == FT_UNTAGGED)
    परत -1;
  cvt.fccsid = ccsid;
  परत fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, पूर्णांक CCSID,
                                    तार्किक IsText) {
  प्रतिज्ञा((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_द्विमान)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  संरचना file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  जर (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    जर (त्रुटि_सं == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      त्रुटि_सं = 0;
    अन्यथा
      परत errnoAsErrorCode();
  }
  परत std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(स्थिरांक Twine &FileName,
                                       स्थिरांक sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  जर (FD != -1) {
    संरचना f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    जर (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      परत std::error_code(त्रुटि_सं, std::generic_category());
    परत Query.fccsid;
  }
  संरचना stat Attr;
  जर (stat(FileName.str().c_str(), &Attr) == -1)
    परत std::error_code(त्रुटि_सं, std::generic_category());
  परत Attr.st_tag.ft_ccsid;
}

ErrorOr<तार्किक> llvm::needzOSConversion(स्थिरांक Twine &FileName,
                                      स्थिरांक sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  जर (std::error_code EC = Ccsid.getError())
    परत EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  निवड (*Ccsid) {
  स्थिती CCSID_UTF_8:
  स्थिती CCSID_ISO8859_1:
  स्थिती FT_द्विमान:
    परत असत्य;
  पूर्वनिर्धारित:
    परत सत्य;
  }
}

std::error_code
llvm::copyFileTagAttributes(स्थिरांक std::string &Source,
                            स्थिरांक sys::fs::file_t DestinationFD) {
  संरचना stat SourceAttributes;
  जर (stat(Source.c_str(), &SourceAttributes) == -1)
    परत std::error_code(त्रुटि_सं, std::generic_category());

  जर (SourceAttributes.st_tag.ft_txtflag)
    जर (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      परत errnoAsErrorCode();

  परत setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
