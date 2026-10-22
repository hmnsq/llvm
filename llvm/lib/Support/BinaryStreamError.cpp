//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/BinaryStreamError.h"
#समावेश "llvm/Support/raw_ostream.h"

using namespace llvm;

अक्षर द्विआधारी_Stream_Error::ID = 0;

द्विआधारी_Stream_Error::द्विआधारी_Stream_Error(stream_error_code C)
    : द्विआधारी_Stream_Error(C, "") {}

द्विआधारी_Stream_Error::द्विआधारी_Stream_Error(StringRef Context)
    : द्विआधारी_Stream_Error(stream_error_code::unspecified, Context) {}

द्विआधारी_Stream_Error::द्विआधारी_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_संदेश = "Stream Error: ";
  चयन (C) {
  हाल stream_error_code::unspecified:
    Err_संदेश += "An unspecified error has occurred.";
    अवरोध;
  हाल stream_error_code::stream_too_short:
    Err_संदेश += "The stream is too short to perform the requested operation.";
    अवरोध;
  हाल stream_error_code::invalid_array_आकार:
    Err_संदेश += "The buffer size is not a multiple of the array element size.";
    अवरोध;
  हाल stream_error_code::invalid_offset:
    Err_संदेश += "The specified offset is invalid for the current stream.";
    अवरोध;
  हाल stream_error_code::filesystem_error:
    Err_संदेश += "An I/O error occurred on the file system.";
    अवरोध;
  }

  अगर (!Context.empty()) {
    Err_संदेश += "  ";
    Err_संदेश += Context;
  }
}

व्योम द्विआधारी_Stream_Error::log(raw_ostream &OS) स्थिर { OS << Err_संदेश; }

StringRef द्विआधारी_Stream_Error::getErrorMessage() स्थिर { वापस Err_संदेश; }

std::error_code द्विआधारी_Stream_Error::convertToErrorCode() स्थिर {
  वापस inconvertibleErrorCode();
}
