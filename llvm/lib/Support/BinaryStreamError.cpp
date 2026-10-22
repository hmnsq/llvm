//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/BinaryStreamError.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

उपयुज्य नामाकाश llvm;

वर्ण द्विमान_प्रवाहः_Error::ID = 0;

द्विमान_प्रवाहः_Error::द्विमान_प्रवाहः_Error(प्रवाहः_error_संहिता C)
    : द्विमान_प्रवाहः_Error(C, "") {}

द्विमान_प्रवाहः_Error::द्विमान_प्रवाहः_Error(StringRef Context)
    : द्विमान_प्रवाहः_Error(प्रवाहः_error_संहिता::unspecified, Context) {}

द्विमान_प्रवाहः_Error::द्विमान_प्रवाहः_Error(प्रवाहः_error_संहिता C, StringRef Context)
    : संहिता(C) {
  Err_सन्देशः = "Stream Error: ";
  विकल्प (C) {
  पक्ष प्रवाहः_error_संहिता::unspecified:
    Err_सन्देशः += "An unspecified error has occurred.";
    भङ्ग;
  पक्ष प्रवाहः_error_संहिता::प्रवाहः_too_short:
    Err_सन्देशः += "The stream is too short to perform the requested operation.";
    भङ्ग;
  पक्ष प्रवाहः_error_संहिता::invalid_array_आकार:
    Err_सन्देशः += "The buffer size is not a multiple of the array element size.";
    भङ्ग;
  पक्ष प्रवाहः_error_संहिता::invalid_विस्थापनम्:
    Err_सन्देशः += "The specified offset is invalid for the current stream.";
    भङ्ग;
  पक्ष प्रवाहः_error_संहिता::filesystem_error:
    Err_सन्देशः += "An I/O error occurred on the file system.";
    भङ्ग;
  }

  यदि (!Context.रिक्तानि()) {
    Err_सन्देशः += "  ";
    Err_सन्देशः += Context;
  }
}

शून्य द्विमान_प्रवाहः_Error::लघुगणकः(raw_ostream &OS) ध्रुव { OS << Err_सन्देशः; }

StringRef द्विमान_प्रवाहः_Error::get_Error_सन्देशः() ध्रुव { प्रत्यागम Err_सन्देशः; }

std::error_संहिता द्विमान_प्रवाहः_Error::convert_गन्तव्यम्_Error_संहिता() ध्रुव {
  प्रत्यागम inconvertible_Error_संहिता();
}
