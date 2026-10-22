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

#include "llvm/Support/AutoConvert.h"
#include <cassert>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace llvm;

static số_nguyên savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

số_nguyên disablezOSAutoConversion(số_nguyên FD) {
  static const struct f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  trả_về fcntl(FD, F_CONTROL_CVT, &Convert);
}

số_nguyên restorezOSStdHandleAutoConversion(số_nguyên FD) {
  assert(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  nếu (savedStdHandleAutoConversionMode[FD] == -1)
    trả_về 0;
  struct f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  trả_về (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

số_nguyên enablezOSAutoConversion(số_nguyên FD) {
  struct f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  nếu (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    trả_về -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  nếu (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    trả_về 0;

  // Save the state of std handles before we make changes to it.
  nếu ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  nếu (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  không_thì
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  trả_về fcntl(FD, F_CONTROL_CVT, &Query);
}

số_nguyên enablezOSAutoConversionCcsid(số_nguyên FD, số_nguyên ccsid) {
  struct f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  nếu (ccsid == FT_UNTAGGED)
    trả_về -1;
  cvt.fccsid = ccsid;
  trả_về fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, số_nguyên CCSID,
                                    bool IsText) {
  assert((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_BINARY)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  struct file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  nếu (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    nếu (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    không_thì
      trả_về errnoAsErrorCode();
  }
  trả_về std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(const Twine &FileName,
                                       const sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  nếu (FD != -1) {
    struct f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    nếu (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      trả_về std::error_code(errno, std::generic_category());
    trả_về Query.fccsid;
  }
  struct stat Attr;
  nếu (stat(FileName.str().c_str(), &Attr) == -1)
    trả_về std::error_code(errno, std::generic_category());
  trả_về Attr.st_tag.ft_ccsid;
}

ErrorOr<bool> llvm::needzOSConversion(const Twine &FileName,
                                      const sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  nếu (std::error_code EC = Ccsid.getError())
    trả_về EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  switch (*Ccsid) {
  case CCSID_UTF_8:
  case CCSID_ISO8859_1:
  case FT_BINARY:
    trả_về false;
  default:
    trả_về true;
  }
}

std::error_code
llvm::copyFileTagAttributes(const std::string &Source,
                            const sys::fs::file_t DestinationFD) {
  struct stat SourceAttributes;
  nếu (stat(Source.c_str(), &SourceAttributes) == -1)
    trả_về std::error_code(errno, std::generic_category());

  nếu (SourceAttributes.st_tag.ft_txtflag)
    nếu (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      trả_về errnoAsErrorCode();

  trả_về setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
