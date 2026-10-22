//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamError.h"
#ضمن "llvm/Support/raw_ostream.h"

استفاده نطاق llvm;

حرف دودویی_جریان_Error::ID = 0;

دودویی_جریان_Error::دودویی_جریان_Error(جریان_error_کد C)
    : دودویی_جریان_Error(C, "") {}

دودویی_جریان_Error::دودویی_جریان_Error(StringRef Context)
    : دودویی_جریان_Error(جریان_error_کد::unspecified, Context) {}

دودویی_جریان_Error::دودویی_جریان_Error(جریان_error_کد C, StringRef Context)
    : کد(C) {
  Err_پیام = "Stream Error: ";
  بدل (C) {
  حالة جریان_error_کد::unspecified:
    Err_پیام += "An unspecified error has occurred.";
    اكسر;
  حالة جریان_error_کد::جریان_too_short:
    Err_پیام += "The stream is too short to perform the requested operation.";
    اكسر;
  حالة جریان_error_کد::invalid_array_اندازه:
    Err_پیام += "The buffer size is not a multiple of the array element size.";
    اكسر;
  حالة جریان_error_کد::invalid_جابه_جایی:
    Err_پیام += "The specified offset is invalid for the current stream.";
    اكسر;
  حالة جریان_error_کد::filesystem_error:
    Err_پیام += "An I/O error occurred on the file system.";
    اكسر;
  }

  إذا (!Context.خالی()) {
    Err_پیام += "  ";
    Err_پیام += Context;
  }
}

فراغ دودویی_جریان_Error::لگاریتم(raw_ostream &OS) ثابت { OS << Err_پیام; }

StringRef دودویی_جریان_Error::get_Error_پیام() ثابت { أرجع Err_پیام; }

std::error_کد دودویی_جریان_Error::convert_مقصد_Error_کد() ثابت {
  أرجع inconvertible_Error_کد();
}
