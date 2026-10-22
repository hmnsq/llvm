//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/BinaryStreamError.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

using நாமாகாஶ llvm;

எழுத்து இருமம்_Stream_Error::ID = 0;

இருமம்_Stream_Error::இருமம்_Stream_Error(stream_error_code C)
    : இருமம்_Stream_Error(C, "") {}

இருமம்_Stream_Error::இருமம்_Stream_Error(StringRef Context)
    : இருமம்_Stream_Error(stream_error_code::unspecified, Context) {}

இருமம்_Stream_Error::இருமம்_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_செய்தி = "Stream Error: ";
  தேர்வு (C) {
  நிலை stream_error_code::unspecified:
    Err_செய்தி += "An unspecified error has occurred.";
    நிறுத்து;
  நிலை stream_error_code::stream_too_short:
    Err_செய்தி += "The stream is too short to perform the requested operation.";
    நிறுத்து;
  நிலை stream_error_code::invalid_array_size:
    Err_செய்தி += "The buffer size is not a multiple of the array element size.";
    நிறுத்து;
  நிலை stream_error_code::invalid_offset:
    Err_செய்தி += "The specified offset is invalid for the current stream.";
    நிறுத்து;
  நிலை stream_error_code::filesystem_error:
    Err_செய்தி += "An I/O error occurred on the file system.";
    நிறுத்து;
  }

  எனில் (!Context.empty()) {
    Err_செய்தி += "  ";
    Err_செய்தி += Context;
  }
}

வெறுமை இருமம்_Stream_Error::log(raw_ostream &OS) மாறிலி { OS << Err_செய்தி; }

StringRef இருமம்_Stream_Error::getErrorMessage() மாறிலி { திரும்பு Err_செய்தி; }

std::error_code இருமம்_Stream_Error::convertToErrorCode() மாறிலி {
  திரும்பு inconvertibleErrorCode();
}
