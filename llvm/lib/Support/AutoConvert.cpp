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

#ضمن "llvm/Support/AutoConvert.h"
#ضمن <cassert>
#ضمن <fcntl.h>
#ضمن <sys/stat.h>
#ضمن <unistd.h>

using نطاق llvm;

ساكن صحيح savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

صحيح disablezOSAutoConversion(صحيح FD) {
  ساكن ثابت بنية f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  أرجع fcntl(FD, F_CONTROL_CVT, &Convert);
}

صحيح restorezOSStdHandleAutoConversion(صحيح FD) {
  أكد(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  𐡄𐡍 (savedStdHandleAutoConversionMode[FD] == -1)
    أرجع 0;
  بنية f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  أرجع (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

صحيح enablezOSAutoConversion(صحيح FD) {
  بنية f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  𐡄𐡍 (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    أرجع -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  𐡄𐡍 (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    أرجع 0;

  // Save the state of std handles before we make changes to it.
  𐡄𐡍 ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  𐡄𐡍 (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  وإلا
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  أرجع fcntl(FD, F_CONTROL_CVT, &Query);
}

صحيح enablezOSAutoConversionCcsid(صحيح FD, صحيح ccsid) {
  بنية f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  𐡄𐡍 (ccsid == FT_UNTAGGED)
    أرجع -1;
  cvt.fccsid = ccsid;
  أرجع fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, صحيح CCSID,
                                    منطقي IsText) {
  أكد((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_BINARY)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  بنية file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  𐡄𐡍 (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    𐡄𐡍 (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    وإلا
      أرجع errnoAsErrorCode();
  }
  أرجع std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(ثابت Twine &FileName,
                                       ثابت sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  𐡄𐡍 (FD != -1) {
    بنية f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    𐡄𐡍 (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      أرجع std::error_code(errno, std::generic_category());
    أرجع Query.fccsid;
  }
  بنية stat Attr;
  𐡄𐡍 (stat(FileName.str().c_str(), &Attr) == -1)
    أرجع std::error_code(errno, std::generic_category());
  أرجع Attr.st_tag.ft_ccsid;
}

ErrorOr<منطقي> llvm::needzOSConversion(ثابت Twine &FileName,
                                      ثابت sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  𐡄𐡍 (std::error_code EC = Ccsid.getError())
    أرجع EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  بدل (*Ccsid) {
  حالة CCSID_UTF_8:
  حالة CCSID_ISO8859_1:
  حالة FT_BINARY:
    أرجع خطأ;
  افتراضي:
    أرجع 𐡒𐡔𐡈;
  }
}

std::error_code
llvm::copyFileTagAttributes(ثابت std::string &Source,
                            ثابت sys::fs::file_t DestinationFD) {
  بنية stat SourceAttributes;
  𐡄𐡍 (stat(Source.c_str(), &SourceAttributes) == -1)
    أرجع std::error_code(errno, std::generic_category());

  𐡄𐡍 (SourceAttributes.st_tag.ft_txtflag)
    𐡄𐡍 (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      أرجع errnoAsErrorCode();

  أرجع setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
