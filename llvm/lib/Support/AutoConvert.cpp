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

#నిర్వచితమైతే __MVS__

#అన్తర్భావ "llvm/Support/AutoConvert.h"
#అన్తర్భావ <cassert>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <unistd.h>

ఉపయోగించి నామాకాశ llvm;

స్థిర పూర్ణాంకం saved_Std_Handle_Auto_Conversion_బహుళకం[3] = {-1, -1, -1};

పూర్ణాంకం disablezOSAutoConversion(పూర్ణాంకం FD) {
  స్థిర స్థిరరాశి నిర్మాణం f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  తిరిగి fcntl(FD, F_CONTROL_CVT, &Convert);
}

పూర్ణాంకం restorezOSStdHandleAutoConversion(పూర్ణాంకం FD) {
  ప్రతిజ్ఞా(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  ఒకవేళ (saved_Std_Handle_Auto_Conversion_బహుళకం[FD] == -1)
    తిరిగి 0;
  నిర్మాణం f_cnvrt Cvt = {
      saved_Std_Handle_Auto_Conversion_బహుళకం[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  తిరిగి (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

పూర్ణాంకం enablezOSAutoConversion(పూర్ణాంకం FD) {
  నిర్మాణం f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  ఒకవేళ (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    తిరిగి -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  ఒకవేళ (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    తిరిగి 0;

  // Save the state of std handles before we make changes to it.
  ఒకవేళ ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      saved_Std_Handle_Auto_Conversion_బహుళకం[FD] == -1)
    saved_Std_Handle_Auto_Conversion_బహుళకం[FD] = Query.cvtcmd;

  ఒకవేళ (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  లేకపోతే
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  తిరిగి fcntl(FD, F_CONTROL_CVT, &Query);
}

పూర్ణాంకం enablezOSAutoConversionCcsid(పూర్ణాంకం FD, పూర్ణాంకం ccsid) {
  నిర్మాణం f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  ఒకవేళ (ccsid == FT_UNTAGGED)
    తిరిగి -1;
  cvt.fccsid = ccsid;
  తిరిగి fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_సంకేతం llvm::setz_OSFile_ట్యాగ్(sys::fs::file_t FD, పూర్ణాంకం CCSID,
                                    తార్కిక Is_పాఠ్యం) {
  ప్రతిజ్ఞా((!Is_పాఠ్యం || (CCSID != FT_UNTAGGED && CCSID != FT_ద్విమాన)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  నిర్మాణం file_ట్యాగ్ ట్యాగ్;
  ట్యాగ్.ft_ccsid = CCSID;
  ట్యాగ్.ft_txtflag = Is_పాఠ్యం;
  ట్యాగ్.ft_deferred = 0;
  ట్యాగ్.ft_rsvflags = 0;

  ఒకవేళ (fcntl(FD.get(), F_SETTAG, &ట్యాగ్) == -1) {
    ఒకవేళ (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    లేకపోతే
      తిరిగి errno_As_Error_సంకేతం();
  }
  తిరిగి std::error_సంకేతం();
}

ErrorOr<__ccsid_t> llvm::getz_OSFile_ట్యాగ్(స్థిరరాశి Twine &FileName,
                                       స్థిరరాశి sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  ఒకవేళ (FD != -1) {
    నిర్మాణం f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    ఒకవేళ (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      తిరిగి std::error_సంకేతం(errno, std::generic_category());
    తిరిగి Query.fccsid;
  }
  నిర్మాణం stat Attr;
  ఒకవేళ (stat(FileName.str().c_str(), &Attr) == -1)
    తిరిగి std::error_సంకేతం(errno, std::generic_category());
  తిరిగి Attr.st_ట్యాగ్.ft_ccsid;
}

ErrorOr<తార్కిక> llvm::needzOSConversion(స్థిరరాశి Twine &FileName,
                                      స్థిరరాశి sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getz_OSFile_ట్యాగ్(FileName, FD);
  ఒకవేళ (std::error_సంకేతం EC = Ccsid.getError())
    తిరిగి EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  ఎంపిక (*Ccsid) {
  స్థితి CCSID_UTF_8:
  స్థితి CCSID_ISO8859_1:
  స్థితి FT_ద్విమాన:
    తిరిగి అబద్ధం;
  అప్రమేయ:
    తిరిగి నిజం;
  }
}

std::error_సంకేతం
llvm::copy_File_ట్యాగ్_లక్షణాలు(స్థిరరాశి std::string &ఆధారం,
                            స్థిరరాశి sys::fs::file_t DestinationFD) {
  నిర్మాణం stat ఆధారం_లక్షణాలు;
  ఒకవేళ (stat(ఆధారం.c_str(), &ఆధారం_లక్షణాలు) == -1)
    తిరిగి std::error_సంకేతం(errno, std::generic_category());

  ఒకవేళ (ఆధారం_లక్షణాలు.st_ట్యాగ్.ft_txtflag)
    ఒకవేళ (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     ఆధారం_లక్షణాలు.st_ట్యాగ్.ft_ccsid) == -1)
      తిరిగి errno_As_Error_సంకేతం();

  తిరిగి setz_OSFile_ట్యాగ్(DestinationFD, ఆధారం_లక్షణాలు.st_ట్యాగ్.ft_ccsid,
                       ఆధారం_లక్షణాలు.st_ట్యాగ్.ft_txtflag);
}

#అయితే_ముగింపు //__MVS__
