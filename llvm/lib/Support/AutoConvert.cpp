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

#شامل "llvm/Support/AutoConvert.h"
#شامل <cassert>
#شامل <fcntl.h>
#شامل <sys/stat.h>
#شامل <unistd.h>

using فضاینام llvm;

ایستا صحیح savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

صحیح disablezOSAutoConversion(صحیح FD) {
  ایستا ثابت ساختار f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  بازگشت fcntl(FD, F_CONTROL_CVT, &Convert);
}

صحیح restorezOSStdHandleAutoConversion(صحیح FD) {
  أكد(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  اگر (savedStdHandleAutoConversionMode[FD] == -1)
    بازگشت 0;
  ساختار f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  بازگشت (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

صحیح enablezOSAutoConversion(صحیح FD) {
  ساختار f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  اگر (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    بازگشت -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  اگر (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    بازگشت 0;

  // Save the state of std handles before we make changes to it.
  اگر ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  اگر (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  وگرنه
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  بازگشت fcntl(FD, F_CONTROL_CVT, &Query);
}

صحیح enablezOSAutoConversionCcsid(صحیح FD, صحیح ccsid) {
  ساختار f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  اگر (ccsid == FT_UNTAGGED)
    بازگشت -1;
  cvt.fccsid = ccsid;
  بازگشت fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, صحیح CCSID,
                                    منطقی IsText) {
  أكد((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_دودویی)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  ساختار file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  اگر (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    اگر (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    وگرنه
      بازگشت errnoAsErrorCode();
  }
  بازگشت std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(ثابت Twine &FileName,
                                       ثابت sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  اگر (FD != -1) {
    ساختار f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    اگر (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      بازگشت std::error_code(errno, std::generic_category());
    بازگشت Query.fccsid;
  }
  ساختار stat Attr;
  اگر (stat(FileName.str().c_str(), &Attr) == -1)
    بازگشت std::error_code(errno, std::generic_category());
  بازگشت Attr.st_tag.ft_ccsid;
}

ErrorOr<منطقی> llvm::needzOSConversion(ثابت Twine &FileName,
                                      ثابت sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  اگر (std::error_code EC = Ccsid.getError())
    بازگشت EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  انتخاب (*Ccsid) {
  حالت CCSID_UTF_8:
  حالت CCSID_ISO8859_1:
  حالت FT_دودویی:
    بازگشت نادرست;
  پیشفرض:
    بازگشت درست;
  }
}

std::error_code
llvm::copyFileTagAttributes(ثابت std::string &Source,
                            ثابت sys::fs::file_t DestinationFD) {
  ساختار stat SourceAttributes;
  اگر (stat(Source.c_str(), &SourceAttributes) == -1)
    بازگشت std::error_code(errno, std::generic_category());

  اگر (SourceAttributes.st_tag.ft_txtflag)
    اگر (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      بازگشت errnoAsErrorCode();

  بازگشت setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
