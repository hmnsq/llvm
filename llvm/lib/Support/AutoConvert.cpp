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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/AutoConvert.h"
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

using ନାମାକାଶ llvm;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

ପୂର୍ଣ୍ଣସଂଖ୍ୟା disablezOSAutoConversion(ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD) {
  ସ୍ଥିର ଅଚଳ ସଂରଚନା f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  ଫେରନ୍ତୁ fcntl(FD, F_CONTROL_CVT, &Convert);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା restorezOSStdHandleAutoConversion(ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD) {
  ପ୍ରତିଜ୍ଞା(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  ଯଦି (savedStdHandleAutoConversionMode[FD] == -1)
    ଫେରନ୍ତୁ 0;
  ସଂରଚନା f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  ଫେରନ୍ତୁ (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା enablezOSAutoConversion(ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD) {
  ସଂରଚନା f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  ଯଦି (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    ଫେରନ୍ତୁ -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  ଯଦି (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    ଫେରନ୍ତୁ 0;

  // Save the state of std handles before we make changes to it.
  ଯଦି ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  ଯଦି (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  ଅନ୍ୟଥା
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  ଫେରନ୍ତୁ fcntl(FD, F_CONTROL_CVT, &Query);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା enablezOSAutoConversionCcsid(ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ccsid) {
  ସଂରଚନା f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  ଯଦି (ccsid == FT_UNTAGGED)
    ଫେରନ୍ତୁ -1;
  cvt.fccsid = ccsid;
  ଫେରନ୍ତୁ fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, ପୂର୍ଣ୍ଣସଂଖ୍ୟା CCSID,
                                    ତାର୍କିକ IsText) {
  ପ୍ରତିଜ୍ଞା((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_ଦ୍ୱିମିକ)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  ସଂରଚନା file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  ଯଦି (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    ଯଦି (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    ଅନ୍ୟଥା
      ଫେରନ୍ତୁ errnoAsErrorCode();
  }
  ଫେରନ୍ତୁ std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(ଅଚଳ Twine &FileName,
                                       ଅଚଳ sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  ଯଦି (FD != -1) {
    ସଂରଚନା f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    ଯଦି (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      ଫେରନ୍ତୁ std::error_code(errno, std::generic_category());
    ଫେରନ୍ତୁ Query.fccsid;
  }
  ସଂରଚନା stat Attr;
  ଯଦି (stat(FileName.str().c_str(), &Attr) == -1)
    ଫେରନ୍ତୁ std::error_code(errno, std::generic_category());
  ଫେରନ୍ତୁ Attr.st_tag.ft_ccsid;
}

ErrorOr<ତାର୍କିକ> llvm::needzOSConversion(ଅଚଳ Twine &FileName,
                                      ଅଚଳ sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  ଯଦି (std::error_code EC = Ccsid.getError())
    ଫେରନ୍ତୁ EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  ବାଛ (*Ccsid) {
  ସ୍ଥିତି CCSID_UTF_8:
  ସ୍ଥିତି CCSID_ISO8859_1:
  ସ୍ଥିତି FT_ଦ୍ୱିମିକ:
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
}

std::error_code
llvm::copyFileTagAttributes(ଅଚଳ std::string &Source,
                            ଅଚଳ sys::fs::file_t DestinationFD) {
  ସଂରଚନା stat SourceAttributes;
  ଯଦି (stat(Source.c_str(), &SourceAttributes) == -1)
    ଫେରନ୍ତୁ std::error_code(errno, std::generic_category());

  ଯଦି (SourceAttributes.st_tag.ft_txtflag)
    ଯଦି (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      ଫେରନ୍ତୁ errnoAsErrorCode();

  ଫେରନ୍ତୁ setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
