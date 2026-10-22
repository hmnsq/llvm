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

حرف دووانەیی_Stream_Error::ID = 0;

دووانەیی_Stream_Error::دووانەیی_Stream_Error(stream_error_code C)
    : دووانەیی_Stream_Error(C, "") {}

دووانەیی_Stream_Error::دووانەیی_Stream_Error(StringRef Context)
    : دووانەیی_Stream_Error(stream_error_code::unspecified, Context) {}

دووانەیی_Stream_Error::دووانەیی_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_پەیام = "Stream Error: ";
  بدل (C) {
  حالة stream_error_code::unspecified:
    Err_پەیام += "An unspecified error has occurred.";
    اكسر;
  حالة stream_error_code::stream_too_short:
    Err_پەیام += "The stream is too short to perform the requested operation.";
    اكسر;
  حالة stream_error_code::invalid_array_size:
    Err_پەیام += "The buffer size is not a multiple of the array element size.";
    اكسر;
  حالة stream_error_code::invalid_offset:
    Err_پەیام += "The specified offset is invalid for the current stream.";
    اكسر;
  حالة stream_error_code::filesystem_error:
    Err_پەیام += "An I/O error occurred on the file system.";
    اكسر;
  }

  إذا (!Context.empty()) {
    Err_پەیام += "  ";
    Err_پەیام += Context;
  }
}

فراغ دووانەیی_Stream_Error::log(raw_ostream &OS) ثابت { OS << Err_پەیام; }

StringRef دووانەیی_Stream_Error::getErrorMessage() ثابت { أرجع Err_پەیام; }

std::error_code دووانەیی_Stream_Error::convertToErrorCode() ثابت {
  أرجع inconvertibleErrorCode();
}
