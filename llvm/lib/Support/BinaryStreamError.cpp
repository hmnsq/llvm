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

حرف ثنائي_Stream_Error::ID = 0;

ثنائي_Stream_Error::ثنائي_Stream_Error(stream_error_code C)
    : ثنائي_Stream_Error(C, "") {}

ثنائي_Stream_Error::ثنائي_Stream_Error(StringRef Context)
    : ثنائي_Stream_Error(stream_error_code::unspecified, Context) {}

ثنائي_Stream_Error::ثنائي_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_رسالة = "Stream Error: ";
  بدل (C) {
  حالة stream_error_code::unspecified:
    Err_رسالة += "An unspecified error has occurred.";
    اكسر;
  حالة stream_error_code::stream_too_short:
    Err_رسالة += "The stream is too short to perform the requested operation.";
    اكسر;
  حالة stream_error_code::invalid_array_size:
    Err_رسالة += "The buffer size is not a multiple of the array element size.";
    اكسر;
  حالة stream_error_code::invalid_offset:
    Err_رسالة += "The specified offset is invalid for the current stream.";
    اكسر;
  حالة stream_error_code::filesystem_error:
    Err_رسالة += "An I/O error occurred on the file system.";
    اكسر;
  }

  إذا (!Context.empty()) {
    Err_رسالة += "  ";
    Err_رسالة += Context;
  }
}

فراغ ثنائي_Stream_Error::log(raw_ostream &OS) ثابت { OS << Err_رسالة; }

StringRef ثنائي_Stream_Error::getErrorMessage() ثابت { إرجاع Err_رسالة; }

std::error_code ثنائي_Stream_Error::convertToErrorCode() ثابت {
  إرجاع inconvertibleErrorCode();
}
