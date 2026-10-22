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

वर्ण द्विआधारी_Stream_Error::ID = 0;

द्विआधारी_Stream_Error::द्विआधारी_Stream_Error(stream_error_code C)
    : द्विआधारी_Stream_Error(C, "") {}

द्विआधारी_Stream_Error::द्विआधारी_Stream_Error(StringRef Context)
    : द्विआधारी_Stream_Error(stream_error_code::unspecified, Context) {}

द्विआधारी_Stream_Error::द्विआधारी_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_सन्देश = "Stream Error: ";
  विकल्प (C) {
  पक्ष stream_error_code::unspecified:
    Err_सन्देश += "An unspecified error has occurred.";
    रोक;
  पक्ष stream_error_code::stream_too_short:
    Err_सन्देश += "The stream is too short to perform the requested operation.";
    रोक;
  पक्ष stream_error_code::invalid_array_size:
    Err_सन्देश += "The buffer size is not a multiple of the array element size.";
    रोक;
  पक्ष stream_error_code::invalid_offset:
    Err_सन्देश += "The specified offset is invalid for the current stream.";
    रोक;
  पक्ष stream_error_code::filesystem_error:
    Err_सन्देश += "An I/O error occurred on the file system.";
    रोक;
  }

  यदि (!Context.empty()) {
    Err_सन्देश += "  ";
    Err_सन्देश += Context;
  }
}

शून्य द्विआधारी_Stream_Error::log(raw_ostream &OS) अचर { OS << Err_सन्देश; }

StringRef द्विआधारी_Stream_Error::getErrorMessage() अचर { फर्काऊ Err_सन्देश; }

std::error_code द्विआधारी_Stream_Error::convertToErrorCode() अचर {
  फर्काऊ inconvertibleErrorCode();
}
