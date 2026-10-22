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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MVS__

#ಅನ್ತರ್ಭಾವ "llvm/Support/AutoConvert.h"
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ saved_Std_Handle_Auto_Conversion_ಬಹುಲಕ[3] = {-1, -1, -1};

ಪೂರ್ಣಾಂಕ disablezOSAutoConversion(ಪೂರ್ಣಾಂಕ FD) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  ಹಿಂತಿರುಗಿ fcntl(FD, F_CONTROL_CVT, &Convert);
}

ಪೂರ್ಣಾಂಕ restorezOSStdHandleAutoConversion(ಪೂರ್ಣಾಂಕ FD) {
  ಪ್ರತಿಜ್ಞಾ(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  ಆದರೆ (saved_Std_Handle_Auto_Conversion_ಬಹುಲಕ[FD] == -1)
    ಹಿಂತಿರುಗಿ 0;
  ರಚನೆ f_cnvrt Cvt = {
      saved_Std_Handle_Auto_Conversion_ಬಹುಲಕ[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  ಹಿಂತಿರುಗಿ (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

ಪೂರ್ಣಾಂಕ enablezOSAutoConversion(ಪೂರ್ಣಾಂಕ FD) {
  ರಚನೆ f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  ಆದರೆ (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    ಹಿಂತಿರುಗಿ -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  ಆದರೆ (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    ಹಿಂತಿರುಗಿ 0;

  // Save the state of std handles before we make changes to it.
  ಆದರೆ ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      saved_Std_Handle_Auto_Conversion_ಬಹುಲಕ[FD] == -1)
    saved_Std_Handle_Auto_Conversion_ಬಹುಲಕ[FD] = Query.cvtcmd;

  ಆದರೆ (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  ಇಲ್ಲದಿದ್ದರೆ
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  ಹಿಂತಿರುಗಿ fcntl(FD, F_CONTROL_CVT, &Query);
}

ಪೂರ್ಣಾಂಕ enablezOSAutoConversionCcsid(ಪೂರ್ಣಾಂಕ FD, ಪೂರ್ಣಾಂಕ ccsid) {
  ರಚನೆ f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  ಆದರೆ (ccsid == FT_UNTAGGED)
    ಹಿಂತಿರುಗಿ -1;
  cvt.fccsid = ccsid;
  ಹಿಂತಿರುಗಿ fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_ಸಂಕೇತ llvm::setz_OSFile_ಟ್ಯಾಗ್(sys::fs::file_t FD, ಪೂರ್ಣಾಂಕ CCSID,
                                    ತಾರ್ಕಿಕ Is_ಪಠ್ಯ) {
  ಪ್ರತಿಜ್ಞಾ((!Is_ಪಠ್ಯ || (CCSID != FT_UNTAGGED && CCSID != FT_ದ್ವಿಮಾನ)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  ರಚನೆ file_ಟ್ಯಾಗ್ ಟ್ಯಾಗ್;
  ಟ್ಯಾಗ್.ft_ccsid = CCSID;
  ಟ್ಯಾಗ್.ft_txtflag = Is_ಪಠ್ಯ;
  ಟ್ಯಾಗ್.ft_deferred = 0;
  ಟ್ಯಾಗ್.ft_rsvflags = 0;

  ಆದರೆ (fcntl(FD.get(), F_SETTAG, &ಟ್ಯಾಗ್) == -1) {
    ಆದರೆ (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    ಇಲ್ಲದಿದ್ದರೆ
      ಹಿಂತಿರುಗಿ errno_As_Error_ಸಂಕೇತ();
  }
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

ErrorOr<__ccsid_t> llvm::getz_OSFile_ಟ್ಯಾಗ್(ಸ್ಥಿರಾಂಕ Twine &FileName,
                                       ಸ್ಥಿರಾಂಕ sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  ಆದರೆ (FD != -1) {
    ರಚನೆ f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    ಆದರೆ (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ(errno, std::generic_category());
    ಹಿಂತಿರುಗಿ Query.fccsid;
  }
  ರಚನೆ stat Attr;
  ಆದರೆ (stat(FileName.str().c_str(), &Attr) == -1)
    ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ(errno, std::generic_category());
  ಹಿಂತಿರುಗಿ Attr.st_ಟ್ಯಾಗ್.ft_ccsid;
}

ErrorOr<ತಾರ್ಕಿಕ> llvm::needzOSConversion(ಸ್ಥಿರಾಂಕ Twine &FileName,
                                      ಸ್ಥಿರಾಂಕ sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getz_OSFile_ಟ್ಯಾಗ್(FileName, FD);
  ಆದರೆ (std::error_ಸಂಕೇತ EC = Ccsid.getError())
    ಹಿಂತಿರುಗಿ EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  ಆಯ್ಕೆ (*Ccsid) {
  ಸ್ಥಿತಿ CCSID_UTF_8:
  ಸ್ಥಿತಿ CCSID_ISO8859_1:
  ಸ್ಥಿತಿ FT_ದ್ವಿಮಾನ:
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
}

std::error_ಸಂಕೇತ
llvm::copy_File_ಟ್ಯಾಗ್_ಗುಣಲಕ್ಷಣಗಳು(ಸ್ಥಿರಾಂಕ std::string &ಮೂಲ,
                            ಸ್ಥಿರಾಂಕ sys::fs::file_t DestinationFD) {
  ರಚನೆ stat ಮೂಲ_ಗುಣಲಕ್ಷಣಗಳು;
  ಆದರೆ (stat(ಮೂಲ.c_str(), &ಮೂಲ_ಗುಣಲಕ್ಷಣಗಳು) == -1)
    ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ(errno, std::generic_category());

  ಆದರೆ (ಮೂಲ_ಗುಣಲಕ್ಷಣಗಳು.st_ಟ್ಯಾಗ್.ft_txtflag)
    ಆದರೆ (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     ಮೂಲ_ಗುಣಲಕ್ಷಣಗಳು.st_ಟ್ಯಾಗ್.ft_ccsid) == -1)
      ಹಿಂತಿರುಗಿ errno_As_Error_ಸಂಕೇತ();

  ಹಿಂತಿರುಗಿ setz_OSFile_ಟ್ಯಾಗ್(DestinationFD, ಮೂಲ_ಗುಣಲಕ್ಷಣಗಳು.st_ಟ್ಯಾಗ್.ft_ccsid,
                       ಮೂಲ_ಗುಣಲಕ್ಷಣಗಳು.st_ಟ್ಯಾಗ್.ft_txtflag);
}

#ಆದರೆ_ಅಂತ್ಯ //__MVS__
