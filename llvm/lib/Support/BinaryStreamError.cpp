//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/BinaryStreamError.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

using ନାମାକାଶ llvm;

ଅକ୍ଷର ଦ୍ୱିମିକ_Stream_Error::ID = 0;

ଦ୍ୱିମିକ_Stream_Error::ଦ୍ୱିମିକ_Stream_Error(stream_error_code C)
    : ଦ୍ୱିମିକ_Stream_Error(C, "") {}

ଦ୍ୱିମିକ_Stream_Error::ଦ୍ୱିମିକ_Stream_Error(StringRef Context)
    : ଦ୍ୱିମିକ_Stream_Error(stream_error_code::unspecified, Context) {}

ଦ୍ୱିମିକ_Stream_Error::ଦ୍ୱିମିକ_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_ସନ୍ଦେଶ = "Stream Error: ";
  ବାଛ (C) {
  ସ୍ଥିତି stream_error_code::unspecified:
    Err_ସନ୍ଦେଶ += "An unspecified error has occurred.";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି stream_error_code::stream_too_short:
    Err_ସନ୍ଦେଶ += "The stream is too short to perform the requested operation.";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି stream_error_code::invalid_array_size:
    Err_ସନ୍ଦେଶ += "The buffer size is not a multiple of the array element size.";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି stream_error_code::invalid_offset:
    Err_ସନ୍ଦେଶ += "The specified offset is invalid for the current stream.";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି stream_error_code::filesystem_error:
    Err_ସନ୍ଦେଶ += "An I/O error occurred on the file system.";
    ଭାଙ୍ଗ;
  }

  ଯଦି (!Context.empty()) {
    Err_ସନ୍ଦେଶ += "  ";
    Err_ସନ୍ଦେଶ += Context;
  }
}

ଶୂନ୍ୟ ଦ୍ୱିମିକ_Stream_Error::log(raw_ostream &OS) ଅଚଳ { OS << Err_ସନ୍ଦେଶ; }

StringRef ଦ୍ୱିମିକ_Stream_Error::getErrorMessage() ଅଚଳ { ଫେରନ୍ତୁ Err_ସନ୍ଦେଶ; }

std::error_code ଦ୍ୱିମିକ_Stream_Error::convertToErrorCode() ଅଚଳ {
  ଫେରନ୍ତୁ inconvertibleErrorCode();
}
