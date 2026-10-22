//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/BinaryStreamError.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

using നാമാകാശ llvm;

അക്ഷരം ദ്വിമാന_Stream_Error::ID = 0;

ദ്വിമാന_Stream_Error::ദ്വിമാന_Stream_Error(stream_error_code C)
    : ദ്വിമാന_Stream_Error(C, "") {}

ദ്വിമാന_Stream_Error::ദ്വിമാന_Stream_Error(StringRef Context)
    : ദ്വിമാന_Stream_Error(stream_error_code::unspecified, Context) {}

ദ്വിമാന_Stream_Error::ദ്വിമാന_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_സന്ദേശം = "Stream Error: ";
  തിരഞ്ഞെടുപ്പ് (C) {
  അവസ്ഥ stream_error_code::unspecified:
    Err_സന്ദേശം += "An unspecified error has occurred.";
    നിർത്തുക;
  അവസ്ഥ stream_error_code::stream_too_short:
    Err_സന്ദേശം += "The stream is too short to perform the requested operation.";
    നിർത്തുക;
  അവസ്ഥ stream_error_code::invalid_array_size:
    Err_സന്ദേശം += "The buffer size is not a multiple of the array element size.";
    നിർത്തുക;
  അവസ്ഥ stream_error_code::invalid_offset:
    Err_സന്ദേശം += "The specified offset is invalid for the current stream.";
    നിർത്തുക;
  അവസ്ഥ stream_error_code::filesystem_error:
    Err_സന്ദേശം += "An I/O error occurred on the file system.";
    നിർത്തുക;
  }

  എങ്കിൽ (!Context.empty()) {
    Err_സന്ദേശം += "  ";
    Err_സന്ദേശം += Context;
  }
}

ശൂന്യം ദ്വിമാന_Stream_Error::log(raw_ostream &OS) സ്ഥിരം { OS << Err_സന്ദേശം; }

StringRef ദ്വിമാന_Stream_Error::getErrorMessage() സ്ഥിരം { തിരികെ Err_സന്ദേശം; }

std::error_code ദ്വിമാന_Stream_Error::convertToErrorCode() സ്ഥിരം {
  തിരികെ inconvertibleErrorCode();
}
