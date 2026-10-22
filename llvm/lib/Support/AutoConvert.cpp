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

#اگر_تعریف_شدہ __MVS__

#شامل "llvm/Support/AutoConvert.h"
#شامل <cassert>
#شامل <fcntl.h>
#شامل <sys/stat.h>
#شامل <unistd.h>

استعمال نامخلا llvm;

ساکن صحیح saved_Std_Handle_Auto_Conversion_منوال[3] = {-1, -1, -1};

صحیح disablezOSAutoConversion(صحیح FD) {
  ساکن ثابت ساخت f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  واپس fcntl(FD, F_CONTROL_CVT, &Convert);
}

صحیح restorezOSStdHandleAutoConversion(صحیح FD) {
  أكد(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  اگر (saved_Std_Handle_Auto_Conversion_منوال[FD] == -1)
    واپس 0;
  ساخت f_cnvrt Cvt = {
      saved_Std_Handle_Auto_Conversion_منوال[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  واپس (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

صحیح enablezOSAutoConversion(صحیح FD) {
  ساخت f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  اگر (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    واپس -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  اگر (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    واپس 0;

  // Save the state of std handles before we make changes to it.
  اگر ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      saved_Std_Handle_Auto_Conversion_منوال[FD] == -1)
    saved_Std_Handle_Auto_Conversion_منوال[FD] = Query.cvtcmd;

  اگر (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  ورنہ
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  واپس fcntl(FD, F_CONTROL_CVT, &Query);
}

صحیح enablezOSAutoConversionCcsid(صحیح FD, صحیح ccsid) {
  ساخت f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  اگر (ccsid == FT_UNTAGGED)
    واپس -1;
  cvt.fccsid = ccsid;
  واپس fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_رمز llvm::setz_OSFile_لیبل(sys::fs::file_t FD, صحیح CCSID,
                                    منطقی Is_متن) {
  أكد((!Is_متن || (CCSID != FT_UNTAGGED && CCSID != FT_ثنائی)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  ساخت file_لیبل لیبل;
  لیبل.ft_ccsid = CCSID;
  لیبل.ft_txtflag = Is_متن;
  لیبل.ft_deferred = 0;
  لیبل.ft_rsvflags = 0;

  اگر (fcntl(FD.get(), F_SETTAG, &لیبل) == -1) {
    اگر (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    ورنہ
      واپس errno_As_Error_رمز();
  }
  واپس std::error_رمز();
}

ErrorOr<__ccsid_t> llvm::getz_OSFile_لیبل(ثابت Twine &FileName,
                                       ثابت sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  اگر (FD != -1) {
    ساخت f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    اگر (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      واپس std::error_رمز(errno, std::generic_category());
    واپس Query.fccsid;
  }
  ساخت stat Attr;
  اگر (stat(FileName.str().c_str(), &Attr) == -1)
    واپس std::error_رمز(errno, std::generic_category());
  واپس Attr.st_لیبل.ft_ccsid;
}

ErrorOr<منطقی> llvm::needzOSConversion(ثابت Twine &FileName,
                                      ثابت sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getz_OSFile_لیبل(FileName, FD);
  اگر (std::error_رمز EC = Ccsid.getError())
    واپس EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  انتخاب (*Ccsid) {
  حالت CCSID_UTF_8:
  حالت CCSID_ISO8859_1:
  حالت FT_ثنائی:
    واپس جھوٹ;
  طےشدہ:
    واپس سچ;
  }
}

std::error_رمز
llvm::copy_File_لیبل_اوصاف(ثابت std::string &ماخذ,
                            ثابت sys::fs::file_t DestinationFD) {
  ساخت stat ماخذ_اوصاف;
  اگر (stat(ماخذ.c_str(), &ماخذ_اوصاف) == -1)
    واپس std::error_رمز(errno, std::generic_category());

  اگر (ماخذ_اوصاف.st_لیبل.ft_txtflag)
    اگر (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     ماخذ_اوصاف.st_لیبل.ft_ccsid) == -1)
      واپس errno_As_Error_رمز();

  واپس setz_OSFile_لیبل(DestinationFD, ماخذ_اوصاف.st_لیبل.ft_ccsid,
                       ماخذ_اوصاف.st_لیبل.ft_txtflag);
}

#اگر_ختم //__MVS__
