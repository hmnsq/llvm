//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/BinaryStreamError.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

using नामाकाश llvm;

वर्ण द्विमान_Stream_Error::ID = 0;

द्विमान_Stream_Error::द्विमान_Stream_Error(stream_error_code C)
    : द्विमान_Stream_Error(C, "") {}

द्विमान_Stream_Error::द्विमान_Stream_Error(StringRef Context)
    : द्विमान_Stream_Error(stream_error_code::unspecified, Context) {}

द्विमान_Stream_Error::द्विमान_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_संदेश = "Stream Error: ";
  विकल्प (C) {
  पक्ष stream_error_code::unspecified:
    Err_संदेश += "An unspecified error has occurred.";
    भङ्ग;
  पक्ष stream_error_code::stream_too_short:
    Err_संदेश += "The stream is too short to perform the requested operation.";
    भङ्ग;
  पक्ष stream_error_code::invalid_array_size:
    Err_संदेश += "The buffer size is not a multiple of the array element size.";
    भङ्ग;
  पक्ष stream_error_code::invalid_offset:
    Err_संदेश += "The specified offset is invalid for the current stream.";
    भङ्ग;
  पक्ष stream_error_code::filesystem_error:
    Err_संदेश += "An I/O error occurred on the file system.";
    भङ्ग;
  }

  यदि (!Context.empty()) {
    Err_संदेश += "  ";
    Err_संदेश += Context;
  }
}

शून्य द्विमान_Stream_Error::log(raw_ostream &OS) ध्रुव { OS << Err_संदेश; }

StringRef द्विमान_Stream_Error::getErrorMessage() ध्रुव { प्रत्यागम Err_संदेश; }

std::error_code द्विमान_Stream_Error::convertToErrorCode() ध्रुव {
  प्रत्यागम inconvertibleErrorCode();
}
