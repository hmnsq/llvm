//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamError.h"
#ضمن "llvm/Support/raw_ostream.h"

استعمال نطاق llvm;

حرف ٻٽو_وهڪرو_Error::ID = 0;

ٻٽو_وهڪرو_Error::ٻٽو_وهڪرو_Error(وهڪرو_error_ڪوڊ C)
    : ٻٽو_وهڪرو_Error(C, "") {}

ٻٽو_وهڪرو_Error::ٻٽو_وهڪرو_Error(StringRef Context)
    : ٻٽو_وهڪرو_Error(وهڪرو_error_ڪوڊ::unspecified, Context) {}

ٻٽو_وهڪرو_Error::ٻٽو_وهڪرو_Error(وهڪرو_error_ڪوڊ C, StringRef Context)
    : ڪوڊ(C) {
  Err_نياپو = "Stream Error: ";
  بدل (C) {
  حالة وهڪرو_error_ڪوڊ::unspecified:
    Err_نياپو += "An unspecified error has occurred.";
    اكسر;
  حالة وهڪرو_error_ڪوڊ::وهڪرو_too_short:
    Err_نياپو += "The stream is too short to perform the requested operation.";
    اكسر;
  حالة وهڪرو_error_ڪوڊ::invalid_array_سائيز:
    Err_نياپو += "The buffer size is not a multiple of the array element size.";
    اكسر;
  حالة وهڪرو_error_ڪوڊ::invalid_وٿي:
    Err_نياپو += "The specified offset is invalid for the current stream.";
    اكسر;
  حالة وهڪرو_error_ڪوڊ::filesystem_error:
    Err_نياپو += "An I/O error occurred on the file system.";
    اكسر;
  }

  إذا (!Context.خالي_جايون()) {
    Err_نياپو += "  ";
    Err_نياپو += Context;
  }
}

فراغ ٻٽو_وهڪرو_Error::لاگرٿم(raw_ostream &OS) ثابت { OS << Err_نياپو; }

StringRef ٻٽو_وهڪرو_Error::get_Error_پيغام() ثابت { أرجع Err_نياپو; }

std::error_ڪوڊ ٻٽو_وهڪرو_Error::convert_منزل_Error_ڪوڊ() ثابت {
  أرجع inconvertible_Error_ڪوڊ();
}
