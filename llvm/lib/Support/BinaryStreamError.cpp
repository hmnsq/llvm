//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/BinaryStreamError.h"
#شامل "llvm/Support/raw_ostream.h"

استعمال نامخلا llvm;

حرف ثنائی_دھارا_Error::ID = 0;

ثنائی_دھارا_Error::ثنائی_دھارا_Error(دھارا_error_رمز C)
    : ثنائی_دھارا_Error(C, "") {}

ثنائی_دھارا_Error::ثنائی_دھارا_Error(StringRef Context)
    : ثنائی_دھارا_Error(دھارا_error_رمز::unspecified, Context) {}

ثنائی_دھارا_Error::ثنائی_دھارا_Error(دھارا_error_رمز C, StringRef Context)
    : رمز(C) {
  Err_پیغام = "Stream Error: ";
  انتخاب (C) {
  حالت دھارا_error_رمز::unspecified:
    Err_پیغام += "An unspecified error has occurred.";
    توڑو;
  حالت دھارا_error_رمز::دھارا_too_short:
    Err_پیغام += "The stream is too short to perform the requested operation.";
    توڑو;
  حالت دھارا_error_رمز::invalid_array_سائز:
    Err_پیغام += "The buffer size is not a multiple of the array element size.";
    توڑو;
  حالت دھارا_error_رمز::invalid_انحراف:
    Err_پیغام += "The specified offset is invalid for the current stream.";
    توڑو;
  حالت دھارا_error_رمز::filesystem_error:
    Err_پیغام += "An I/O error occurred on the file system.";
    توڑو;
  }

  اگر (!Context.خالی_خانے()) {
    Err_پیغام += "  ";
    Err_پیغام += Context;
  }
}

خالی ثنائی_دھارا_Error::لوگارتھم(raw_ostream &OS) ثابت { OS << Err_پیغام; }

StringRef ثنائی_دھارا_Error::get_Error_پیغام() ثابت { واپس Err_پیغام; }

std::error_رمز ثنائی_دھارا_Error::convert_مقصد_Error_رمز() ثابت {
  واپس inconvertible_Error_رمز();
}
