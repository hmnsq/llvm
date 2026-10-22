//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/BinaryStreamError.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

char दालावाहान_Stream_Error::ID = 0;

दालावाहान_Stream_Error::दालावाहान_Stream_Error(stream_error_code C)
    : दालावाहान_Stream_Error(C, "") {}

दालावाहान_Stream_Error::दालावाहान_Stream_Error(StringRef Context)
    : दालावाहान_Stream_Error(stream_error_code::unspecified, Context) {}

दालावाहान_Stream_Error::दालावाहान_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_मेनसाहे = "Stream Error: ";
  switch (C) {
  case stream_error_code::unspecified:
    Err_मेनसाहे += "An unspecified error has occurred.";
    break;
  case stream_error_code::stream_too_short:
    Err_मेनसाहे += "The stream is too short to perform the requested operation.";
    break;
  case stream_error_code::invalid_array_size:
    Err_मेनसाहे += "The buffer size is not a multiple of the array element size.";
    break;
  case stream_error_code::invalid_offset:
    Err_मेनसाहे += "The specified offset is invalid for the current stream.";
    break;
  case stream_error_code::filesystem_error:
    Err_मेनसाहे += "An I/O error occurred on the file system.";
    break;
  }

  if (!Context.empty()) {
    Err_मेनसाहे += "  ";
    Err_मेनसाहे += Context;
  }
}

void दालावाहान_Stream_Error::log(raw_ostream &OS) const { OS << Err_मेनसाहे; }

StringRef दालावाहान_Stream_Error::getErrorMessage() const { return Err_मेनसाहे; }

std::error_code दालावाहान_Stream_Error::convertToErrorCode() const {
  return inconvertibleErrorCode();
}
