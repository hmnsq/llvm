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

#ಅನ್ತರ್ಭಾವ "llvm/Support/AutoConvert.h"
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

using ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

ಪೂರ್ಣಾಙ್ಕ disablezOSAutoConversion(ಪೂರ್ಣಾಙ್ಕ FD) {
  ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  ಪ್ರತ್ಯಾಗಮ fcntl(FD, F_CONTROL_CVT, &Convert);
}

ಪೂರ್ಣಾಙ್ಕ restorezOSStdHandleAutoConversion(ಪೂರ್ಣಾಙ್ಕ FD) {
  ಪ್ರತಿಜ್ಞಾ(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  ಯದಿ (savedStdHandleAutoConversionMode[FD] == -1)
    ಪ್ರತ್ಯಾಗಮ 0;
  ಸಂರಚನಾ f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  ಪ್ರತ್ಯಾಗಮ (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

ಪೂರ್ಣಾಙ್ಕ enablezOSAutoConversion(ಪೂರ್ಣಾಙ್ಕ FD) {
  ಸಂರಚನಾ f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  ಯದಿ (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    ಪ್ರತ್ಯಾಗಮ -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  ಯದಿ (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    ಪ್ರತ್ಯಾಗಮ 0;

  // Save the state of std handles before we make changes to it.
  ಯದಿ ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  ಯದಿ (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  ಅನ್ಯಥಾ
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  ಪ್ರತ್ಯಾಗಮ fcntl(FD, F_CONTROL_CVT, &Query);
}

ಪೂರ್ಣಾಙ್ಕ enablezOSAutoConversionCcsid(ಪೂರ್ಣಾಙ್ಕ FD, ಪೂರ್ಣಾಙ್ಕ ccsid) {
  ಸಂರಚನಾ f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  ಯದಿ (ccsid == FT_UNTAGGED)
    ಪ್ರತ್ಯಾಗಮ -1;
  cvt.fccsid = ccsid;
  ಪ್ರತ್ಯಾಗಮ fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, ಪೂರ್ಣಾಙ್ಕ CCSID,
                                    ತಾರ್ಕಿಕ IsText) {
  ಪ್ರತಿಜ್ಞಾ((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_ದ್ವಿಮಾನ)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  ಸಂರಚನಾ file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  ಯದಿ (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    ಯದಿ (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    ಅನ್ಯಥಾ
      ಪ್ರತ್ಯಾಗಮ errnoAsErrorCode();
  }
  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(ಧ್ರುವ Twine &FileName,
                                       ಧ್ರುವ sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  ಯದಿ (FD != -1) {
    ಸಂರಚನಾ f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    ಯದಿ (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      ಪ್ರತ್ಯಾಗಮ std::error_code(errno, std::generic_category());
    ಪ್ರತ್ಯಾಗಮ Query.fccsid;
  }
  ಸಂರಚನಾ stat Attr;
  ಯದಿ (stat(FileName.str().c_str(), &Attr) == -1)
    ಪ್ರತ್ಯಾಗಮ std::error_code(errno, std::generic_category());
  ಪ್ರತ್ಯಾಗಮ Attr.st_tag.ft_ccsid;
}

ErrorOr<ತಾರ್ಕಿಕ> llvm::needzOSConversion(ಧ್ರುವ Twine &FileName,
                                      ಧ್ರುವ sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  ಯದಿ (std::error_code EC = Ccsid.getError())
    ಪ್ರತ್ಯಾಗಮ EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  ವಿಕಲ್ಪ (*Ccsid) {
  ಪಕ್ಷ CCSID_UTF_8:
  ಪಕ್ಷ CCSID_ISO8859_1:
  ಪಕ್ಷ FT_ದ್ವಿಮಾನ:
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಸ್ವಭಾವ:
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
}

std::error_code
llvm::copyFileTagAttributes(ಧ್ರುವ std::string &Source,
                            ಧ್ರುವ sys::fs::file_t DestinationFD) {
  ಸಂರಚನಾ stat SourceAttributes;
  ಯದಿ (stat(Source.c_str(), &SourceAttributes) == -1)
    ಪ್ರತ್ಯಾಗಮ std::error_code(errno, std::generic_category());

  ಯದಿ (SourceAttributes.st_tag.ft_txtflag)
    ಯದಿ (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      ಪ್ರತ್ಯಾಗಮ errnoAsErrorCode();

  ಪ್ರತ್ಯಾಗಮ setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
