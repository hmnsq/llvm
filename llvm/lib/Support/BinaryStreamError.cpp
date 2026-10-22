//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamError.h"
#ضمن "llvm/Support/raw_ostream.h"

using نطاق llvm;

حرف بيناري_Stream_Error::ID = 0;

بيناري_Stream_Error::بيناري_Stream_Error(stream_error_code C)
    : بيناري_Stream_Error(C, "") {}

بيناري_Stream_Error::بيناري_Stream_Error(StringRef Context)
    : بيناري_Stream_Error(stream_error_code::unspecified, Context) {}

بيناري_Stream_Error::بيناري_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_ڤسنن = "Stream Error: ";
  بدل (C) {
  حالة stream_error_code::unspecified:
    Err_ڤسنن += "An unspecified error has occurred.";
    اكسر;
  حالة stream_error_code::stream_too_short:
    Err_ڤسنن += "The stream is too short to perform the requested operation.";
    اكسر;
  حالة stream_error_code::invalid_array_size:
    Err_ڤسنن += "The buffer size is not a multiple of the array element size.";
    اكسر;
  حالة stream_error_code::invalid_offset:
    Err_ڤسنن += "The specified offset is invalid for the current stream.";
    اكسر;
  حالة stream_error_code::filesystem_error:
    Err_ڤسنن += "An I/O error occurred on the file system.";
    اكسر;
  }

  إذا (!Context.empty()) {
    Err_ڤسنن += "  ";
    Err_ڤسنن += Context;
  }
}

فراغ بيناري_Stream_Error::log(raw_ostream &OS) ثابت { OS << Err_ڤسنن; }

StringRef بيناري_Stream_Error::getErrorMessage() ثابت { أرجع Err_ڤسنن; }

std::error_code بيناري_Stream_Error::convertToErrorCode() ثابت {
  أرجع inconvertibleErrorCode();
}
