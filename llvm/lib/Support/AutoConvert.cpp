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

#اگر_تعریف_شده __MVS__

#ضمن "llvm/Support/AutoConvert.h"
#ضمن <cassert>
#ضمن <fcntl.h>
#ضمن <sys/stat.h>
#ضمن <unistd.h>

استفاده نطاق llvm;

ساكن صحيح saved_Std_Handle_Auto_Conversion_نما[3] = {-1, -1, -1};

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
  إذا (saved_Std_Handle_Auto_Conversion_نما[FD] == -1)
    أرجع 0;
  بنية f_cnvrt Cvt = {
      saved_Std_Handle_Auto_Conversion_نما[FD], // cvtcmd
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

  إذا (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    أرجع -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  إذا (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    أرجع 0;

  // Save the state of std handles before we make changes to it.
  إذا ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      saved_Std_Handle_Auto_Conversion_نما[FD] == -1)
    saved_Std_Handle_Auto_Conversion_نما[FD] = Query.cvtcmd;

  إذا (FD == STDOUT_FILENO || FD == STDERR_FILENO)
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
  إذا (ccsid == FT_UNTAGGED)
    أرجع -1;
  cvt.fccsid = ccsid;
  أرجع fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_کد llvm::setz_OSFile_برچسب(sys::fs::file_t FD, صحيح CCSID,
                                    منطقي Is_متن) {
  أكد((!Is_متن || (CCSID != FT_UNTAGGED && CCSID != FT_دودویی)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  بنية file_برچسب برچسب;
  برچسب.ft_ccsid = CCSID;
  برچسب.ft_txtflag = Is_متن;
  برچسب.ft_deferred = 0;
  برچسب.ft_rsvflags = 0;

  إذا (fcntl(FD.get(), F_SETTAG, &برچسب) == -1) {
    إذا (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    وإلا
      أرجع errno_As_Error_کد();
  }
  أرجع std::error_کد();
}

ErrorOr<__ccsid_t> llvm::getz_OSFile_برچسب(ثابت Twine &FileName,
                                       ثابت sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  إذا (FD != -1) {
    بنية f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    إذا (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      أرجع std::error_کد(errno, std::generic_category());
    أرجع Query.fccsid;
  }
  بنية stat Attr;
  إذا (stat(FileName.str().c_str(), &Attr) == -1)
    أرجع std::error_کد(errno, std::generic_category());
  أرجع Attr.st_برچسب.ft_ccsid;
}

ErrorOr<منطقي> llvm::needzOSConversion(ثابت Twine &FileName,
                                      ثابت sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getz_OSFile_برچسب(FileName, FD);
  إذا (std::error_کد EC = Ccsid.getError())
    أرجع EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  بدل (*Ccsid) {
  حالة CCSID_UTF_8:
  حالة CCSID_ISO8859_1:
  حالة FT_دودویی:
    أرجع خطأ;
  افتراضي:
    أرجع درست;
  }
}

std::error_کد
llvm::copy_File_برچسب_صفات(ثابت std::string &منبع,
                            ثابت sys::fs::file_t DestinationFD) {
  بنية stat منبع_صفات;
  إذا (stat(منبع.c_str(), &منبع_صفات) == -1)
    أرجع std::error_کد(errno, std::generic_category());

  إذا (منبع_صفات.st_برچسب.ft_txtflag)
    إذا (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     منبع_صفات.st_برچسب.ft_ccsid) == -1)
      أرجع errno_As_Error_کد();

  أرجع setz_OSFile_برچسب(DestinationFD, منبع_صفات.st_برچسب.ft_ccsid,
                       منبع_صفات.st_برچسب.ft_txtflag);
}

#پایان_اگر //__MVS__
