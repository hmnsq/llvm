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

#جے_تعریف_شدہ __MVS__

#ضمن "llvm/Support/AutoConvert.h"
#ضمن <cassert>
#ضمن <fcntl.h>
#ضمن <sys/stat.h>
#ضمن <unistd.h>

ورتوں نطاق llvm;

ساكن صحيح saved_Std_Handle_Auto_Conversion_منوال[3] = {-1, -1, -1};

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
  إذا (saved_Std_Handle_Auto_Conversion_منوال[FD] == -1)
    أرجع 0;
  بنية f_cnvrt Cvt = {
      saved_Std_Handle_Auto_Conversion_منوال[FD], // cvtcmd
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
      saved_Std_Handle_Auto_Conversion_منوال[FD] == -1)
    saved_Std_Handle_Auto_Conversion_منوال[FD] = Query.cvtcmd;

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

std::error_کوڈ llvm::setz_OSFile_ٹیگ(sys::fs::file_t FD, صحيح CCSID,
                                    منطقي Is_متن) {
  أكد((!Is_متن || (CCSID != FT_UNTAGGED && CCSID != FT_ثنائی)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  بنية file_ٹیگ ٹیگ;
  ٹیگ.ft_ccsid = CCSID;
  ٹیگ.ft_txtflag = Is_متن;
  ٹیگ.ft_deferred = 0;
  ٹیگ.ft_rsvflags = 0;

  إذا (fcntl(FD.get(), F_SETTAG, &ٹیگ) == -1) {
    إذا (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    وإلا
      أرجع errno_As_Error_کوڈ();
  }
  أرجع std::error_کوڈ();
}

ErrorOr<__ccsid_t> llvm::getz_OSFile_ٹیگ(ثابت Twine &FileName,
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
      أرجع std::error_کوڈ(errno, std::generic_category());
    أرجع Query.fccsid;
  }
  بنية stat Attr;
  إذا (stat(FileName.str().c_str(), &Attr) == -1)
    أرجع std::error_کوڈ(errno, std::generic_category());
  أرجع Attr.st_ٹیگ.ft_ccsid;
}

ErrorOr<منطقي> llvm::needzOSConversion(ثابت Twine &FileName,
                                      ثابت sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getz_OSFile_ٹیگ(FileName, FD);
  إذا (std::error_کوڈ EC = Ccsid.getError())
    أرجع EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  بدل (*Ccsid) {
  حالة CCSID_UTF_8:
  حالة CCSID_ISO8859_1:
  حالة FT_ثنائی:
    أرجع خطأ;
  افتراضي:
    أرجع سچ;
  }
}

std::error_کوڈ
llvm::copy_File_ٹیگ_گن(ثابت std::string &سروت,
                            ثابت sys::fs::file_t DestinationFD) {
  بنية stat سروت_گن;
  إذا (stat(سروت.c_str(), &سروت_گن) == -1)
    أرجع std::error_کوڈ(errno, std::generic_category());

  إذا (سروت_گن.st_ٹیگ.ft_txtflag)
    إذا (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     سروت_گن.st_ٹیگ.ft_ccsid) == -1)
      أرجع errno_As_Error_کوڈ();

  أرجع setz_OSFile_ٹیگ(DestinationFD, سروت_گن.st_ٹیگ.ft_ccsid,
                       سروت_گن.st_ٹیگ.ft_txtflag);
}

#جے_انت //__MVS__
