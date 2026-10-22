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

#અન્તર્ભાવ "llvm/Support/AutoConvert.h"
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <unistd.h>

using નામાકાશ llvm;

સ્થિર પૂર્ણાંક savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

પૂર્ણાંક disablezOSAutoConversion(પૂર્ણાંક FD) {
  સ્થિર અચલ સંરચના f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  પાછા fcntl(FD, F_CONTROL_CVT, &Convert);
}

પૂર્ણાંક restorezOSStdHandleAutoConversion(પૂર્ણાંક FD) {
  પ્રતિજ્ઞા(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  જો (savedStdHandleAutoConversionMode[FD] == -1)
    પાછા 0;
  સંરચના f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  પાછા (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

પૂર્ણાંક enablezOSAutoConversion(પૂર્ણાંક FD) {
  સંરચના f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  જો (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    પાછા -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  જો (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    પાછા 0;

  // Save the state of std handles before we make changes to it.
  જો ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  જો (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  નહીંતર
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  પાછા fcntl(FD, F_CONTROL_CVT, &Query);
}

પૂર્ણાંક enablezOSAutoConversionCcsid(પૂર્ણાંક FD, પૂર્ણાંક ccsid) {
  સંરચના f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  જો (ccsid == FT_UNTAGGED)
    પાછા -1;
  cvt.fccsid = ccsid;
  પાછા fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, પૂર્ણાંક CCSID,
                                    તાર્કિક IsText) {
  પ્રતિજ્ઞા((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_દ્વિઅંકી)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  સંરચના file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  જો (fcntl(FD.get(), F_SETTAG, &Tag) == -1) {
    જો (errno == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      errno = 0;
    નહીંતર
      પાછા errnoAsErrorCode();
  }
  પાછા std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(અચલ Twine &FileName,
                                       અચલ sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  જો (FD != -1) {
    સંરચના f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    જો (fcntl(FD.get(), F_CONTROL_CVT, &Query) == -1)
      પાછા std::error_code(errno, std::generic_category());
    પાછા Query.fccsid;
  }
  સંરચના stat Attr;
  જો (stat(FileName.str().c_str(), &Attr) == -1)
    પાછા std::error_code(errno, std::generic_category());
  પાછા Attr.st_tag.ft_ccsid;
}

ErrorOr<તાર્કિક> llvm::needzOSConversion(અચલ Twine &FileName,
                                      અચલ sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  જો (std::error_code EC = Ccsid.getError())
    પાછા EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  પસંદગી (*Ccsid) {
  સ્થિતિ CCSID_UTF_8:
  સ્થિતિ CCSID_ISO8859_1:
  સ્થિતિ FT_દ્વિઅંકી:
    પાછા ખોટું;
  મૂળભૂત:
    પાછા સાચું;
  }
}

std::error_code
llvm::copyFileTagAttributes(અચલ std::string &Source,
                            અચલ sys::fs::file_t DestinationFD) {
  સંરચના stat SourceAttributes;
  જો (stat(Source.c_str(), &SourceAttributes) == -1)
    પાછા std::error_code(errno, std::generic_category());

  જો (SourceAttributes.st_tag.ft_txtflag)
    જો (enablezOSAutoConversionCcsid(DestinationFD.get(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      પાછા errnoAsErrorCode();

  પાછા setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#endif //__MVS__
