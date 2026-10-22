//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamError.h"
#ضمن "llvm/Support/raw_ostream.h"

کارول نطاق llvm;

حرف دوګونی_بهیر_Error::ID = 0;

دوګونی_بهیر_Error::دوګونی_بهیر_Error(بهیر_error_کوډ C)
    : دوګونی_بهیر_Error(C, "") {}

دوګونی_بهیر_Error::دوګونی_بهیر_Error(StringRef Context)
    : دوګونی_بهیر_Error(بهیر_error_کوډ::unspecified, Context) {}

دوګونی_بهیر_Error::دوګونی_بهیر_Error(بهیر_error_کوډ C, StringRef Context)
    : کوډ(C) {
  Err_پیغام = "Stream Error: ";
  بدل (C) {
  حالة بهیر_error_کوډ::unspecified:
    Err_پیغام += "An unspecified error has occurred.";
    اكسر;
  حالة بهیر_error_کوډ::بهیر_too_short:
    Err_پیغام += "The stream is too short to perform the requested operation.";
    اكسر;
  حالة بهیر_error_کوډ::invalid_array_اندازه:
    Err_پیغام += "The buffer size is not a multiple of the array element size.";
    اكسر;
  حالة بهیر_error_کوډ::invalid_دننه_واټن:
    Err_پیغام += "The specified offset is invalid for the current stream.";
    اكسر;
  حالة بهیر_error_کوډ::filesystem_error:
    Err_پیغام += "An I/O error occurred on the file system.";
    اكسر;
  }

  إذا (!Context.تشه()) {
    Err_پیغام += "  ";
    Err_پیغام += Context;
  }
}

فراغ دوګونی_بهیر_Error::لوګاریتم(raw_ostream &OS) ثابت { OS << Err_پیغام; }

StringRef دوګونی_بهیر_Error::get_Error_پیغام() ثابت { أرجع Err_پیغام; }

std::error_کوډ دوګونی_بهیر_Error::convert_منزل_Error_کوډ() ثابت {
  أرجع inconvertible_Error_کوډ();
}
