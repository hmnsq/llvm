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

#अगर_घोषित __MVS__

#समावेश "llvm/Support/AutoConvert.h"
#समावेश <cassert>
#समावेश <fcntl.h>
#समावेश <sys/stat.h>
#समावेश <unistd.h>

using namespace llvm;

अटल पूर्णांक savedStdHandleAutoConversionMode[3] = {-1, -1, -1};

पूर्णांक disablezOSAutoConversion(पूर्णांक FD) {
  अटल स्थिर काष्ठा f_cnvrt Convert = {
      SETCVTOFF, // cvtcmd
      0,         // pccsid
      0,         // fccsid
  };

  वापस fcntl(FD, F_CONTROL_CVT, &Convert);
}

पूर्णांक restorezOSStdHandleAutoConversion(पूर्णांक FD) {
  निश्चित(FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO);
  अगर (savedStdHandleAutoConversionMode[FD] == -1)
    वापस 0;
  काष्ठा f_cnvrt Cvt = {
      savedStdHandleAutoConversionMode[FD], // cvtcmd
      0,                                    // pccsid
      0,                                    // fccsid
  };
  वापस (fcntl(FD, F_CONTROL_CVT, &Cvt));
}

पूर्णांक enablezOSAutoConversion(पूर्णांक FD) {
  काष्ठा f_cnvrt Query = {
      QUERYCVT, // cvtcmd
      0,        // pccsid
      0,        // fccsid
  };

  अगर (fcntl(FD, F_CONTROL_CVT, &Query) == -1)
    वापस -1;

  // We don't need conversion for UTF-8 tagged files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  // When the pccsid is not ISO8859-1, autoconversion is still needed.
  अगर (Query.pccsid == CCSID_ISO8859_1 &&
      (Query.fccsid == CCSID_UTF_8 || Query.fccsid == CCSID_ISO8859_1))
    वापस 0;

  // Save the state of std handles before we make changes to it.
  अगर ((FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO) &&
      savedStdHandleAutoConversionMode[FD] == -1)
    savedStdHandleAutoConversionMode[FD] = Query.cvtcmd;

  अगर (FD == STDOUT_FILENO || FD == STDERR_FILENO)
    Query.cvtcmd = SETCVTON;
  अन्यथा
    Query.cvtcmd = SETCVTALL;

  Query.pccsid =
      (FD == STDIN_FILENO || FD == STDOUT_FILENO || FD == STDERR_FILENO)
          ? 0
          : CCSID_UTF_8;
  // Assume untagged files to be IBM-1047 encoded.
  Query.fccsid = (Query.fccsid == FT_UNTAGGED) ? CCSID_IBM_1047 : Query.fccsid;
  वापस fcntl(FD, F_CONTROL_CVT, &Query);
}

पूर्णांक enablezOSAutoConversionCcsid(पूर्णांक FD, पूर्णांक ccsid) {
  काष्ठा f_cnvrt cvt = {
      SETCVTALL,   // cvtcmd
      CCSID_UTF_8, // pccsid
      0,           // fccsid
  };
  अगर (ccsid == FT_UNTAGGED)
    वापस -1;
  cvt.fccsid = ccsid;
  वापस fcntl(FD, F_CONTROL_CVT, &cvt);
}

std::error_code llvm::setzOSFileTag(sys::fs::file_t FD, पूर्णांक CCSID,
                                    bool IsText) {
  निश्चित((!IsText || (CCSID != FT_UNTAGGED && CCSID != FT_द्विआधारी)) &&
         "FT_UNTAGGED and FT_BINARY are not allowed for text files");
  काष्ठा file_tag Tag;
  Tag.ft_ccsid = CCSID;
  Tag.ft_txtflag = IsText;
  Tag.ft_deferred = 0;
  Tag.ft_rsvflags = 0;

  अगर (fcntl(FD.लो(), F_SETTAG, &Tag) == -1) {
    अगर (त्रुटि_सं == ENOSYS)
      // Some file systems do not support filetags.
      // Ignore ENOSYS error to allow compilation.
      त्रुटि_सं = 0;
    अन्यथा
      वापस errnoAsErrorCode();
  }
  वापस std::error_code();
}

ErrorOr<__ccsid_t> llvm::getzOSFileTag(स्थिर Twine &FileName,
                                       स्थिर sys::fs::file_t FD) {
  // If we have a file descriptor, use it to find out file tagging. Otherwise we
  // need to use stat() with the file path.
  अगर (FD != -1) {
    काष्ठा f_cnvrt Query = {
        QUERYCVT, // cvtcmd
        0,        // pccsid
        0,        // fccsid
    };
    अगर (fcntl(FD.लो(), F_CONTROL_CVT, &Query) == -1)
      वापस std::error_code(त्रुटि_सं, std::generic_category());
    वापस Query.fccsid;
  }
  काष्ठा stat Attr;
  अगर (stat(FileName.str().c_str(), &Attr) == -1)
    वापस std::error_code(त्रुटि_सं, std::generic_category());
  वापस Attr.st_tag.ft_ccsid;
}

ErrorOr<bool> llvm::needzOSConversion(स्थिर Twine &FileName,
                                      स्थिर sys::fs::file_t FD) {
  ErrorOr<__ccsid_t> Ccsid = getzOSFileTag(FileName, FD);
  अगर (std::error_code EC = Ccsid.getError())
    वापस EC;
  // We don't need conversion for UTF-8 tagged files or binary files.
  // TODO: Remove the assumption of ISO8859-1 = UTF-8 here when we fully resolve
  // problems related to UTF-8 tagged source files.
  चयन (*Ccsid) {
  हाल CCSID_UTF_8:
  हाल CCSID_ISO8859_1:
  हाल FT_द्विआधारी:
    वापस false;
  शेष:
    वापस true;
  }
}

std::error_code
llvm::copyFileTagAttributes(स्थिर std::string &Source,
                            स्थिर sys::fs::file_t DestinationFD) {
  काष्ठा stat SourceAttributes;
  अगर (stat(Source.c_str(), &SourceAttributes) == -1)
    वापस std::error_code(त्रुटि_सं, std::generic_category());

  अगर (SourceAttributes.st_tag.ft_txtflag)
    अगर (enablezOSAutoConversionCcsid(DestinationFD.लो(),
                                     SourceAttributes.st_tag.ft_ccsid) == -1)
      वापस errnoAsErrorCode();

  वापस setzOSFileTag(DestinationFD, SourceAttributes.st_tag.ft_ccsid,
                       SourceAttributes.st_tag.ft_txtflag);
}

#पूर्ण_अगर //__MVS__
