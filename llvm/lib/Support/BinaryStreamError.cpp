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

حرف ئىككىلىك_Stream_Error::ID = 0;

ئىككىلىك_Stream_Error::ئىككىلىك_Stream_Error(stream_error_code C)
    : ئىككىلىك_Stream_Error(C, "") {}

ئىككىلىك_Stream_Error::ئىككىلىك_Stream_Error(StringRef Context)
    : ئىككىلىك_Stream_Error(stream_error_code::unspecified, Context) {}

ئىككىلىك_Stream_Error::ئىككىلىك_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_ئۇچۇر = "Stream Error: ";
  بدل (C) {
  حالة stream_error_code::unspecified:
    Err_ئۇچۇر += "An unspecified error has occurred.";
    اكسر;
  حالة stream_error_code::stream_too_short:
    Err_ئۇچۇر += "The stream is too short to perform the requested operation.";
    اكسر;
  حالة stream_error_code::invalid_array_size:
    Err_ئۇچۇر += "The buffer size is not a multiple of the array element size.";
    اكسر;
  حالة stream_error_code::invalid_offset:
    Err_ئۇچۇر += "The specified offset is invalid for the current stream.";
    اكسر;
  حالة stream_error_code::filesystem_error:
    Err_ئۇچۇر += "An I/O error occurred on the file system.";
    اكسر;
  }

  إذا (!Context.empty()) {
    Err_ئۇچۇر += "  ";
    Err_ئۇچۇر += Context;
  }
}

فراغ ئىككىلىك_Stream_Error::log(raw_ostream &OS) ثابت { OS << Err_ئۇچۇر; }

StringRef ئىككىلىك_Stream_Error::getErrorMessage() ثابت { أرجع Err_ئۇچۇر; }

std::error_code ئىككىلىك_Stream_Error::convertToErrorCode() ثابت {
  أرجع inconvertibleErrorCode();
}
