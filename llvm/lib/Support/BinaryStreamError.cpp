//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamError.h"
#ضمن "llvm/Support/raw_ostream.h"

ورتوں نطاق llvm;

حرف ثنائی_دھارا_Error::ID = 0;

ثنائی_دھارا_Error::ثنائی_دھارا_Error(دھارا_error_کوڈ C)
    : ثنائی_دھارا_Error(C, "") {}

ثنائی_دھارا_Error::ثنائی_دھارا_Error(StringRef Context)
    : ثنائی_دھارا_Error(دھارا_error_کوڈ::unspecified, Context) {}

ثنائی_دھارا_Error::ثنائی_دھارا_Error(دھارا_error_کوڈ C, StringRef Context)
    : کوڈ(C) {
  Err_پیغام = "Stream Error: ";
  بدل (C) {
  حالة دھارا_error_کوڈ::unspecified:
    Err_پیغام += "An unspecified error has occurred.";
    اكسر;
  حالة دھارا_error_کوڈ::دھارا_too_short:
    Err_پیغام += "The stream is too short to perform the requested operation.";
    اكسر;
  حالة دھارا_error_کوڈ::invalid_array_ناپ:
    Err_پیغام += "The buffer size is not a multiple of the array element size.";
    اكسر;
  حالة دھارا_error_کوڈ::invalid_وتھ:
    Err_پیغام += "The specified offset is invalid for the current stream.";
    اكسر;
  حالة دھارا_error_کوڈ::filesystem_error:
    Err_پیغام += "An I/O error occurred on the file system.";
    اكسر;
  }

  إذا (!Context.خالی_تھاواں()) {
    Err_پیغام += "  ";
    Err_پیغام += Context;
  }
}

فراغ ثنائی_دھارا_Error::لوگارتھم(raw_ostream &OS) ثابت { OS << Err_پیغام; }

StringRef ثنائی_دھارا_Error::get_Error_سنیہا() ثابت { أرجع Err_پیغام; }

std::error_کوڈ ثنائی_دھارا_Error::convert_منزل_Error_کوڈ() ثابت {
  أرجع inconvertible_Error_کوڈ();
}
