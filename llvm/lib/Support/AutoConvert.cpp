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

static 整数 savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

整数 disablezOSAutoConversion(整数 FD) {
  static const struct f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  返回 fcntl(FD, F_CONTROL_CVT, &Convert);
}

整数 restorezOSStdHandleAutoConversion(整数 FD) {
  assert(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  如果 (savedStdHandleAutoConversionMode[FD] == -1)
    返回 0;
  struct f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  返回 (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

整数 enablezOSAutoConversion(整数 FD) {
  struct f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  如果 (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    返回 -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  如果 (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    返回 0;

  // Save the state of std handles before we make changes to it.
  如果 ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  如果 (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  否则
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  返回 fcntl(FD, F_CONTROL_CVT, &Query);
}

整数 enablezOSAutoConversionCcsid(整数 FD, 整数 ccsid) {
  struct f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  如果 (ccsid == FT_UNTAGGED)
    返回 -1;
  cvt.fccsid = ccsid;
  返回 fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, 整数 CCSID,
                                    bool IsText) {
  assert((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_BINARY)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  struct file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  如果 (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    如果 (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    否则
      返回 errnoAsErrorCode();
  }
  返回 std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(const Twine &FileName,
                                       const sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  如果 (FD != -1) {
    struct f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    如果 (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      返回 std::error_code(errno, std::generic_category());
    返回 Query.fccsid;
  }
  struct stat Attr;
  如果 (stat(FileName.str().c_str(), &Attr) == -1)
    返回 std::error_code(errno, std::generic_category());
  返回 Attr.st_tag.ft_ccsid;
}

ErrorOr<bool> llvm::needzOSConversion(const Twine &FileName,
                                      const sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  如果 (std::error_code EC = Ccsid.getError())
    返回 EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  switch (*Ccsid) {
  case CCSID_UTF_8:
  case CCSID_ISO8859_1:
  case FT_BINARY:
    返回 false;
  default:
    返回 true;
  }
}

std::error_code
llvm::copyFileTagAttributes(const std::string &Source,
                            const sys::fs::file_t DestinationFD) {
  struct stat SourceAttributes;
  如果 (stat(Source.c_str(), &SourceAttributes) == -1)
    返回 std::error_code(errno, std::generic_category());

  如果 (SourceAttributes.st_tag.ft_txtflag)
    如果 (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      返回 errnoAsErrorCode();

  返回 setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
