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

#অন্তর্ভাব "llvm/Support/AutoConvert.h"
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <unistd.h>

using নামাকাশ llvm;

স্থির পূর্ণসংখ্যা savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

পূর্ণসংখ্যা disablezOSAutoConversion(পূর্ণসংখ্যা FD) {
  স্থির ধ্রুবক কাঠামো f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  ফেরত fcntl(FD, F_CONTROL_CVT, &Convert);
}

পূর্ণসংখ্যা restorezOSStdHandleAutoConversion(পূর্ণসংখ্যা FD) {
  প্রতিজ্ঞা(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  যদি (savedStdHandleAutoConversionMode[FD] == -1)
    ফেরত 0;
  কাঠামো f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  ফেরত (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

পূর্ণসংখ্যা enablezOSAutoConversion(পূর্ণসংখ্যা FD) {
  কাঠামো f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  যদি (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    ফেরত -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  যদি (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    ফেরত 0;

  // Save the state of std handles before we make changes to it.
  যদি ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  যদি (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  অন্যথা
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  ফেরত fcntl(FD, F_CONTROL_CVT, &Query);
}

পূর্ণসংখ্যা enablezOSAutoConversionCcsid(পূর্ণসংখ্যা FD, পূর্ণসংখ্যা ccsid) {
  কাঠামো f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  যদি (ccsid == FT_UNTAGGED)
    ফেরত -1;
  cvt.fccsid = ccsid;
  ফেরত fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, পূর্ণসংখ্যা CCSID,
                                    যৌক্তিক IsText) {
  প্রতিজ্ঞা((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_দ্বিমিক)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  কাঠামো file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  যদি (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    যদি (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    অন্যথা
      ফেরত errnoAsErrorCode();
  }
  ফেরত std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(ধ্রুবক Twine &FileName,
                                       ধ্রুবক sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  যদি (FD != -1) {
    কাঠামো f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    যদি (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      ফেরত std::error_code(errno, std::generic_category());
    ফেরত Query.fccsid;
  }
  কাঠামো stat Attr;
  যদি (stat(FileName.str().c_str(), &Attr) == -1)
    ফেরত std::error_code(errno, std::generic_category());
  ফেরত Attr.st_tag.ft_ccsid;
}

ErrorOr<যৌক্তিক> llvm::needzOSConversion(ধ্রুবক Twine &FileName,
                                      ধ্রুবক sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  যদি (std::error_code EC = Ccsid.getError())
    ফেরত EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  নির্বাচন (*Ccsid) {
  ক্ষেত্র CCSID_UTF_8:
  ক্ষেত্র CCSID_ISO8859_1:
  ক্ষেত্র FT_দ্বিমিক:
    ফেরত মিথ্যা;
  পূর্বনির্ধারিত:
    ফেরত সত্য;
  }
}

std::error_code
llvm::copyFileTagAttributes(ধ্রুবক std::string &Source,
                            ধ্রুবক sys::fs::file_t DestinationFD) {
  কাঠামো stat SourceAttributes;
  যদি (stat(Source.c_str(), &SourceAttributes) == -1)
    ফেরত std::error_code(errno, std::generic_category());

  যদি (SourceAttributes.st_tag.ft_txtflag)
    যদি (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      ফেরত errnoAsErrorCode();

  ফেরত setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
