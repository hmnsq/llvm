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

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MVS__

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/AutoConvert.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਥਿਰ ਪੂਰਨ saved_Std_Handle_Auto_Conversion_ਬਹੁਲਕ[3] = {-1, -1, -1};

ਪੂਰਨ disablezOSAutoConversion(ਪੂਰਨ FD) {
  ਸਥਿਰ ਅਚਲ ਬਣਤਰ f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  ਵਾਪਸ fcntl(FD, F_CONTROL_CVT, &Convert);
}

ਪੂਰਨ restorezOSStdHandleAutoConversion(ਪੂਰਨ FD) {
  ਪ੍ਰਤਿਜ੍ਞਾ(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  ਜੇ (saved_Std_Handle_Auto_Conversion_ਬਹੁਲਕ[FD] == -1)
    ਵਾਪਸ 0;
  ਬਣਤਰ f_cnvrt Cvt = {
      saved_Std_Handle_Auto_Conversion_ਬਹੁਲਕ[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  ਵਾਪਸ (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

ਪੂਰਨ enablezOSAutoConversion(ਪੂਰਨ FD) {
  ਬਣਤਰ f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  ਜੇ (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    ਵਾਪਸ -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  ਜੇ (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    ਵਾਪਸ 0;

  // Save the state of std handles before we make changes to it.
  ਜੇ ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      saved_Std_Handle_Auto_Conversion_ਬਹੁਲਕ[FD] == -1)
    saved_Std_Handle_Auto_Conversion_ਬਹੁਲਕ[FD] = Query.cvtcmd;

  ਜੇ (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  ਨਹੀਂਤਾਂ
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  ਵਾਪਸ fcntl(FD, F_CONTROL_CVT, &Query);
}

ਪੂਰਨ enablezOSAutoConversionCcsid(ਪੂਰਨ FD, ਪੂਰਨ ccsid) {
  ਬਣਤਰ f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  ਜੇ (ccsid == FT_UNTAGGED)
    ਵਾਪਸ -1;
  cvt.fccsid = ccsid;
  ਵਾਪਸ fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_ਕੋਡ llvm::setz_OSFile_ਟੈਗ(sys::fs::file_t FD, ਪੂਰਨ CCSID,
                                    ਤਾਰਕਿਕ Is_ਪਾਠ) {
  ਪ੍ਰਤਿਜ੍ਞਾ((!Is_ਪਾਠ || (CCSID != FT_UNTAGGED && CCSID != FT_ਦੁਆਧਾਰੀ)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  ਬਣਤਰ file_ਟੈਗ ਟੈਗ;
  ਟੈਗ.ft_ccsid = CCSID;
  ਟੈਗ.ft_txtflag = Is_ਪਾਠ;
  ਟੈਗ.ft_deferred = 0;
  ਟੈਗ.ft_rsvflags = 0;

  ਜੇ (fcntl(FD.get(), F_SETTAG, &ਟੈਗ) == -1) {
    ਜੇ (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    ਨਹੀਂਤਾਂ
      ਵਾਪਸ errno_As_Error_ਕੋਡ();
  }
  ਵਾਪਸ std::error_ਕੋਡ();
}

ErrorOr<__ccsid_t> llvm::getz_OSFile_ਟੈਗ(ਅਚਲ Twine &FileName,
                                       ਅਚਲ sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  ਜੇ (FD != -1) {
    ਬਣਤਰ f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    ਜੇ (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      ਵਾਪਸ std::error_ਕੋਡ(errno, std::generic_category());
    ਵਾਪਸ Query.fccsid;
  }
  ਬਣਤਰ stat Attr;
  ਜੇ (stat(FileName.str().c_str(), &Attr) == -1)
    ਵਾਪਸ std::error_ਕੋਡ(errno, std::generic_category());
  ਵਾਪਸ Attr.st_ਟੈਗ.ft_ccsid;
}

ErrorOr<ਤਾਰਕਿਕ> llvm::needzOSConversion(ਅਚਲ Twine &FileName,
                                      ਅਚਲ sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getz_OSFile_ਟੈਗ(FileName, FD);
  ਜੇ (std::error_ਕੋਡ EC = Ccsid.getError())
    ਵਾਪਸ EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  ਚੋਣ (*Ccsid) {
  ਹਾਲਤ CCSID_UTF_8:
  ਹਾਲਤ CCSID_ISO8859_1:
  ਹਾਲਤ FT_ਦੁਆਧਾਰੀ:
    ਵਾਪਸ ਝੂਠ;
  ਮੂਲ:
    ਵਾਪਸ ਸੱਚ;
  }
}

std::error_ਕੋਡ
llvm::copy_File_ਟੈਗ_ਗੁਣ(ਅਚਲ std::string &ਸਰੋਤ,
                            ਅਚਲ sys::fs::file_t DestinationFD) {
  ਬਣਤਰ stat ਸਰੋਤ_ਗੁਣ;
  ਜੇ (stat(ਸਰੋਤ.c_str(), &ਸਰੋਤ_ਗੁਣ) == -1)
    ਵਾਪਸ std::error_ਕੋਡ(errno, std::generic_category());

  ਜੇ (ਸਰੋਤ_ਗੁਣ.st_ਟੈਗ.ft_txtflag)
    ਜੇ (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     ਸਰੋਤ_ਗੁਣ.st_ਟੈਗ.ft_ccsid) == -1)
      ਵਾਪਸ errno_As_Error_ਕੋਡ();

  ਵਾਪਸ setz_OSFile_ਟੈਗ(DestinationFD, ਸਰੋਤ_ਗੁਣ.st_ਟੈਗ.ft_ccsid,
                       ਸਰੋਤ_ਗੁਣ.st_ਟੈਗ.ft_txtflag);
}

#ਜੇ_ਅੰਤ //__MVS__
