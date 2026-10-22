//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/BinaryStreamError.h"
#شامل "llvm/Support/raw_ostream.h"

using فضاینام llvm;

نویسه دودویی_Stream_Error::ID = 0;

دودویی_Stream_Error::دودویی_Stream_Error(stream_error_code C)
    : دودویی_Stream_Error(C, "") {}

دودویی_Stream_Error::دودویی_Stream_Error(StringRef Context)
    : دودویی_Stream_Error(stream_error_code::unspecified, Context) {}

دودویی_Stream_Error::دودویی_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_پیام = "Stream Error: ";
  انتخاب (C) {
  حالت stream_error_code::unspecified:
    Err_پیام += "An unspecified error has occurred.";
    بشکن;
  حالت stream_error_code::stream_too_short:
    Err_پیام += "The stream is too short to perform the requested operation.";
    بشکن;
  حالت stream_error_code::invalid_array_size:
    Err_پیام += "The buffer size is not a multiple of the array element size.";
    بشکن;
  حالت stream_error_code::invalid_offset:
    Err_پیام += "The specified offset is invalid for the current stream.";
    بشکن;
  حالت stream_error_code::filesystem_error:
    Err_پیام += "An I/O error occurred on the file system.";
    بشکن;
  }

  اگر (!Context.empty()) {
    Err_پیام += "  ";
    Err_پیام += Context;
  }
}

تهی دودویی_Stream_Error::log(raw_ostream &OS) ثابت { OS << Err_پیام; }

StringRef دودویی_Stream_Error::getErrorMessage() ثابت { بازگشت Err_پیام; }

std::error_code دودویی_Stream_Error::convertToErrorCode() ثابت {
  بازگشت inconvertibleErrorCode();
}
