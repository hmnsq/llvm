//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BinaryStreamError.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਅੱਖਰ ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error::ID = 0;

ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error(ਧਾਰਾ_error_ਕੋਡ C)
    : ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error(C, "") {}

ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error(StringRef Context)
    : ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error(ਧਾਰਾ_error_ਕੋਡ::unspecified, Context) {}

ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error(ਧਾਰਾ_error_ਕੋਡ C, StringRef Context)
    : ਕੋਡ(C) {
  Err_ਸੁਨੇਹਾ = "Stream Error: ";
  ਚੋਣ (C) {
  ਹਾਲਤ ਧਾਰਾ_error_ਕੋਡ::unspecified:
    Err_ਸੁਨੇਹਾ += "An unspecified error has occurred.";
    ਰੋਕੋ;
  ਹਾਲਤ ਧਾਰਾ_error_ਕੋਡ::ਧਾਰਾ_too_short:
    Err_ਸੁਨੇਹਾ += "The stream is too short to perform the requested operation.";
    ਰੋਕੋ;
  ਹਾਲਤ ਧਾਰਾ_error_ਕੋਡ::invalid_array_ਮਾਪ:
    Err_ਸੁਨੇਹਾ += "The buffer size is not a multiple of the array element size.";
    ਰੋਕੋ;
  ਹਾਲਤ ਧਾਰਾ_error_ਕੋਡ::invalid_ਵਿੱਥ:
    Err_ਸੁਨੇਹਾ += "The specified offset is invalid for the current stream.";
    ਰੋਕੋ;
  ਹਾਲਤ ਧਾਰਾ_error_ਕੋਡ::filesystem_error:
    Err_ਸੁਨੇਹਾ += "An I/O error occurred on the file system.";
    ਰੋਕੋ;
  }

  ਜੇ (!Context.ਖਾਲੀ_ਥਾਵਾਂ()) {
    Err_ਸੁਨੇਹਾ += "  ";
    Err_ਸੁਨੇਹਾ += Context;
  }
}

ਖਾਲੀ ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error::ਲਘੂਗਣਕ(raw_ostream &OS) ਅਚਲ { OS << Err_ਸੁਨੇਹਾ; }

StringRef ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error::get_Error_ਸੁਨੇਹਾ() ਅਚਲ { ਵਾਪਸ Err_ਸੁਨੇਹਾ; }

std::error_ਕੋਡ ਦੁਆਧਾਰੀ_ਧਾਰਾ_Error::convert_ਮੰਜ਼ਿਲ_Error_ਕੋਡ() ਅਚਲ {
  ਵਾਪਸ inconvertible_Error_ਕੋਡ();
}
