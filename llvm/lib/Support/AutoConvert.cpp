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

static ចំនួនគត់ savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

ចំនួនគត់ disablezOSAutoConversion(ចំនួនគត់ FD) {
  static const sa_nracanaa f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  ត្រឡប់ fcntl(FD, F_CONTROL_CVT, &Convert);
}

ចំនួនគត់ restorezOSStdHandleAutoConversion(ចំនួនគត់ FD) {
  assert(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  ប្រសិនបើ (savedStdHandleAutoConversionMode[FD] == -1)
    ត្រឡប់ 0;
  sa_nracanaa f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  ត្រឡប់ (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

ចំនួនគត់ enablezOSAutoConversion(ចំនួនគត់ FD) {
  sa_nracanaa f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  ប្រសិនបើ (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    ត្រឡប់ -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  ប្រសិនបើ (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    ត្រឡប់ 0;

  // Save the state of std handles before we make changes to it.
  ប្រសិនបើ ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  ប្រសិនបើ (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  បើមិនដូច្នេះ
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  ត្រឡប់ fcntl(FD, F_CONTROL_CVT, &Query);
}

ចំនួនគត់ enablezOSAutoConversionCcsid(ចំនួនគត់ FD, ចំនួនគត់ ccsid) {
  sa_nracanaa f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  ប្រសិនបើ (ccsid == FT_UNTAGGED)
    ត្រឡប់ -1;
  cvt.fccsid = ccsid;
  ត្រឡប់ fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, ចំនួនគត់ CCSID,
                                    bool IsText) {
  assert((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_BINARY)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  sa_nracanaa file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  ប្រសិនបើ (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    ប្រសិនបើ (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    បើមិនដូច្នេះ
      ត្រឡប់ errnoAsErrorCode();
  }
  ត្រឡប់ std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(const Twine &FileName,
                                       const sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  ប្រសិនបើ (FD != -1) {
    sa_nracanaa f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    ប្រសិនបើ (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      ត្រឡប់ std::error_code(errno, std::generic_category());
    ត្រឡប់ Query.fccsid;
  }
  sa_nracanaa stat Attr;
  ប្រសិនបើ (stat(FileName.str().c_str(), &Attr) == -1)
    ត្រឡប់ std::error_code(errno, std::generic_category());
  ត្រឡប់ Attr.st_tag.ft_ccsid;
}

ErrorOr<bool> llvm::needzOSConversion(const Twine &FileName,
                                      const sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  ប្រសិនបើ (std::error_code EC = Ccsid.getError())
    ត្រឡប់ EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  switch (*Ccsid) {
  pak_sa CCSID_UTF_8:
  pak_sa CCSID_ISO8859_1:
  pak_sa FT_BINARY:
    ត្រឡប់ false;
  default:
    ត្រឡប់ true;
  }
}

std::error_code
llvm::copyFileTagAttributes(const std::string &Source,
                            const sys::fs::file_t DestinationFD) {
  sa_nracanaa stat SourceAttributes;
  ប្រសិនបើ (stat(Source.c_str(), &SourceAttributes) == -1)
    ត្រឡប់ std::error_code(errno, std::generic_category());

  ប្រសិនបើ (SourceAttributes.st_tag.ft_txtflag)
    ប្រសិនបើ (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      ត្រឡប់ errnoAsErrorCode();

  ត្រឡប់ setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
