//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/BinaryStreamError.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;

অক্ষর দ্বিমিক_Stream_Error::ID = 0;

দ্বিমিক_Stream_Error::দ্বিমিক_Stream_Error(stream_error_code C)
    : দ্বিমিক_Stream_Error(C, "") {}

দ্বিমিক_Stream_Error::দ্বিমিক_Stream_Error(StringRef Context)
    : দ্বিমিক_Stream_Error(stream_error_code::unspecified, Context) {}

দ্বিমিক_Stream_Error::দ্বিমিক_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_বার্তা = "Stream Error: ";
  নির্বাচন (C) {
  ক্ষেত্র stream_error_code::unspecified:
    Err_বার্তা += "An unspecified error has occurred.";
    বিরতি;
  ক্ষেত্র stream_error_code::stream_too_short:
    Err_বার্তা += "The stream is too short to perform the requested operation.";
    বিরতি;
  ক্ষেত্র stream_error_code::invalid_array_আকার:
    Err_বার্তা += "The buffer size is not a multiple of the array element size.";
    বিরতি;
  ক্ষেত্র stream_error_code::invalid_offset:
    Err_বার্তা += "The specified offset is invalid for the current stream.";
    বিরতি;
  ক্ষেত্র stream_error_code::filesystem_error:
    Err_বার্তা += "An I/O error occurred on the file system.";
    বিরতি;
  }

  যদি (!Context.empty()) {
    Err_বার্তা += "  ";
    Err_বার্তা += Context;
  }
}

শূন্য দ্বিমিক_Stream_Error::log(raw_ostream &OS) ধ্রুবক { OS << Err_বার্তা; }

StringRef দ্বিমিক_Stream_Error::getErrorMessage() ধ্রুবক { ফেরত Err_বার্তা; }

std::error_code দ্বিমিক_Stream_Error::convertToErrorCode() ধ্রুবক {
  ফেরত inconvertibleErrorCode();
}
