//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/BinaryStreamError.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;

ৱর্ণ দ্বিমিক_Stream_Error::ID = 0;

দ্বিমিক_Stream_Error::দ্বিমিক_Stream_Error(stream_error_code C)
    : দ্বিমিক_Stream_Error(C, "") {}

দ্বিমিক_Stream_Error::দ্বিমিক_Stream_Error(StringRef Context)
    : দ্বিমিক_Stream_Error(stream_error_code::unspecified, Context) {}

দ্বিমিক_Stream_Error::দ্বিমিক_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_বাৰ্তা = "Stream Error: ";
  ৱিকল্প (C) {
  পক্ষ stream_error_code::unspecified:
    Err_বাৰ্তা += "An unspecified error has occurred.";
    ভঙ্গ;
  পক্ষ stream_error_code::stream_too_short:
    Err_বাৰ্তা += "The stream is too short to perform the requested operation.";
    ভঙ্গ;
  পক্ষ stream_error_code::invalid_array_size:
    Err_বাৰ্তা += "The buffer size is not a multiple of the array element size.";
    ভঙ্গ;
  পক্ষ stream_error_code::invalid_offset:
    Err_বাৰ্তা += "The specified offset is invalid for the current stream.";
    ভঙ্গ;
  পক্ষ stream_error_code::filesystem_error:
    Err_বাৰ্তা += "An I/O error occurred on the file system.";
    ভঙ্গ;
  }

  যদি (!Context.empty()) {
    Err_বাৰ্তা += "  ";
    Err_বাৰ্তা += Context;
  }
}

শূন্য দ্বিমিক_Stream_Error::log(raw_ostream &OS) ধ্রুৱ { OS << Err_বাৰ্তা; }

StringRef দ্বিমিক_Stream_Error::getErrorMessage() ধ্রুৱ { ঘূৰাই Err_বাৰ্তা; }

std::error_code দ্বিমিক_Stream_Error::convertToErrorCode() ধ্রুৱ {
  ঘূৰাই inconvertibleErrorCode();
}
