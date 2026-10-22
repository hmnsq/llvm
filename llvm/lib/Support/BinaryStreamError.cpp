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

حرف BinaryStreamError::ID = 0;

BinaryStreamError::BinaryStreamError(stream_error_code C)
    : BinaryStreamError(C, "") {}

BinaryStreamError::BinaryStreamError(StringRef Context)
    : BinaryStreamError(stream_error_code::unspecified, Context) {}

BinaryStreamError::BinaryStreamError(stream_error_code C, StringRef Context)
    : Code(C) {
  ErrMsg = "Stream Error: ";
  بدل (C) {
  حالة stream_error_code::unspecified:
    ErrMsg += "An unspecified error has occurred.";
    اكسر;
  حالة stream_error_code::stream_too_short:
    ErrMsg += "The stream is too short to perform the requested operation.";
    اكسر;
  حالة stream_error_code::invalid_array_size:
    ErrMsg += "The buffer size is not a multiple of the array element size.";
    اكسر;
  حالة stream_error_code::invalid_offset:
    ErrMsg += "The specified offset is invalid for the current stream.";
    اكسر;
  حالة stream_error_code::filesystem_error:
    ErrMsg += "An I/O error occurred on the file system.";
    اكسر;
  }

  𐤀𐤌 (!Context.empty()) {
    ErrMsg += "  ";
    ErrMsg += Context;
  }
}

فراغ BinaryStreamError::log(raw_ostream &OS) ثابت { OS << ErrMsg; }

StringRef BinaryStreamError::getErrorMessage() ثابت { أرجع ErrMsg; }

std::error_code BinaryStreamError::convertToErrorCode() ثابت {
  أرجع inconvertibleErrorCode();
}
