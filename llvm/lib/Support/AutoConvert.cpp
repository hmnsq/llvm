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

#അന്തര്ഭാവ "llvm/Support/AutoConvert.h"
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <unistd.h>

using നാമാകാശ llvm;

സ്ഥിതം പൂർണ്ണസംഖ്യ savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

പൂർണ്ണസംഖ്യ disablezOSAutoConversion(പൂർണ്ണസംഖ്യ FD) {
  സ്ഥിതം സ്ഥിരം ഘടന f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  തിരികെ fcntl(FD, F_CONTROL_CVT, &Convert);
}

പൂർണ്ണസംഖ്യ restorezOSStdHandleAutoConversion(പൂർണ്ണസംഖ്യ FD) {
  പ്രതിജ്ഞാ(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  എങ്കിൽ (savedStdHandleAutoConversionMode[FD] == -1)
    തിരികെ 0;
  ഘടന f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  തിരികെ (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

പൂർണ്ണസംഖ്യ enablezOSAutoConversion(പൂർണ്ണസംഖ്യ FD) {
  ഘടന f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  എങ്കിൽ (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    തിരികെ -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  എങ്കിൽ (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    തിരികെ 0;

  // Save the state of std handles before we make changes to it.
  എങ്കിൽ ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  എങ്കിൽ (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  അല്ലെങ്കിൽ
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  തിരികെ fcntl(FD, F_CONTROL_CVT, &Query);
}

പൂർണ്ണസംഖ്യ enablezOSAutoConversionCcsid(പൂർണ്ണസംഖ്യ FD, പൂർണ്ണസംഖ്യ ccsid) {
  ഘടന f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  എങ്കിൽ (ccsid == FT_UNTAGGED)
    തിരികെ -1;
  cvt.fccsid = ccsid;
  തിരികെ fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, പൂർണ്ണസംഖ്യ CCSID,
                                    തർക്കം IsText) {
  പ്രതിജ്ഞാ((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_ദ്വിമാന)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  ഘടന file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  എങ്കിൽ (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    എങ്കിൽ (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    അല്ലെങ്കിൽ
      തിരികെ errnoAsErrorCode();
  }
  തിരികെ std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(സ്ഥിരം Twine &FileName,
                                       സ്ഥിരം sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  എങ്കിൽ (FD != -1) {
    ഘടന f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    എങ്കിൽ (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      തിരികെ std::error_code(errno, std::generic_category());
    തിരികെ Query.fccsid;
  }
  ഘടന stat Attr;
  എങ്കിൽ (stat(FileName.str().c_str(), &Attr) == -1)
    തിരികെ std::error_code(errno, std::generic_category());
  തിരികെ Attr.st_tag.ft_ccsid;
}

ErrorOr<തർക്കം> llvm::needzOSConversion(സ്ഥിരം Twine &FileName,
                                      സ്ഥിരം sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  എങ്കിൽ (std::error_code EC = Ccsid.getError())
    തിരികെ EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  തിരഞ്ഞെടുപ്പ് (*Ccsid) {
  അവസ്ഥ CCSID_UTF_8:
  അവസ്ഥ CCSID_ISO8859_1:
  അവസ്ഥ FT_ദ്വിമാന:
    തിരികെ അസത്യം;
  സ്ഥിരസ്ഥിതി:
    തിരികെ സത്യം;
  }
}

std::error_code
llvm::copyFileTagAttributes(സ്ഥിരം std::string &Source,
                            സ്ഥിരം sys::fs::file_t DestinationFD) {
  ഘടന stat SourceAttributes;
  എങ്കിൽ (stat(Source.c_str(), &SourceAttributes) == -1)
    തിരികെ std::error_code(errno, std::generic_category());

  എങ്കിൽ (SourceAttributes.st_tag.ft_txtflag)
    എങ്കിൽ (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      തിരികെ errnoAsErrorCode();

  തിരികെ setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
