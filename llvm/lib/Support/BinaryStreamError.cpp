//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/BinaryStreamError.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

using નામાકાશ llvm;

અક્ષર દ્વિઅંકી_Stream_Error::ID = 0;

દ્વિઅંકી_Stream_Error::દ્વિઅંકી_Stream_Error(stream_error_code C)
    : દ્વિઅંકી_Stream_Error(C, "") {}

દ્વિઅંકી_Stream_Error::દ્વિઅંકી_Stream_Error(StringRef Context)
    : દ્વિઅંકી_Stream_Error(stream_error_code::unspecified, Context) {}

દ્વિઅંકી_Stream_Error::દ્વિઅંકી_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_સંદેશ = "Stream Error: ";
  પસંદગી (C) {
  સ્થિતિ stream_error_code::unspecified:
    Err_સંદેશ += "An unspecified error has occurred.";
    તોડો;
  સ્થિતિ stream_error_code::stream_too_short:
    Err_સંદેશ += "The stream is too short to perform the requested operation.";
    તોડો;
  સ્થિતિ stream_error_code::invalid_array_size:
    Err_સંદેશ += "The buffer size is not a multiple of the array element size.";
    તોડો;
  સ્થિતિ stream_error_code::invalid_offset:
    Err_સંદેશ += "The specified offset is invalid for the current stream.";
    તોડો;
  સ્થિતિ stream_error_code::filesystem_error:
    Err_સંદેશ += "An I/O error occurred on the file system.";
    તોડો;
  }

  જો (!Context.empty()) {
    Err_સંદેશ += "  ";
    Err_સંદેશ += Context;
  }
}

શૂન્ય દ્વિઅંકી_Stream_Error::log(raw_ostream &OS) અચલ { OS << Err_સંદેશ; }

StringRef દ્વિઅંકી_Stream_Error::getErrorMessage() અચલ { પાછા Err_સંદેશ; }

std::error_code દ્વિઅંકી_Stream_Error::convertToErrorCode() અચલ {
  પાછા inconvertibleErrorCode();
}
