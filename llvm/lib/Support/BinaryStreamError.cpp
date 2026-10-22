//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamError.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

using ನಾಮಾಕಾಶ llvm;

ವರ್ಣ ದ್ವಿಮಾನ_Stream_Error::ID = 0;

ದ್ವಿಮಾನ_Stream_Error::ದ್ವಿಮಾನ_Stream_Error(stream_error_code C)
    : ದ್ವಿಮಾನ_Stream_Error(C, "") {}

ದ್ವಿಮಾನ_Stream_Error::ದ್ವಿಮಾನ_Stream_Error(StringRef Context)
    : ದ್ವಿಮಾನ_Stream_Error(stream_error_code::unspecified, Context) {}

ದ್ವಿಮಾನ_Stream_Error::ದ್ವಿಮಾನ_Stream_Error(stream_error_code C, StringRef Context)
    : Code(C) {
  Err_ಸಂದೇಶ = "Stream Error: ";
  ವಿಕಲ್ಪ (C) {
  ಪಕ್ಷ stream_error_code::unspecified:
    Err_ಸಂದೇಶ += "An unspecified error has occurred.";
    ಭಙ್ಗ;
  ಪಕ್ಷ stream_error_code::stream_too_short:
    Err_ಸಂದೇಶ += "The stream is too short to perform the requested operation.";
    ಭಙ್ಗ;
  ಪಕ್ಷ stream_error_code::invalid_array_size:
    Err_ಸಂದೇಶ += "The buffer size is not a multiple of the array element size.";
    ಭಙ್ಗ;
  ಪಕ್ಷ stream_error_code::invalid_offset:
    Err_ಸಂದೇಶ += "The specified offset is invalid for the current stream.";
    ಭಙ್ಗ;
  ಪಕ್ಷ stream_error_code::filesystem_error:
    Err_ಸಂದೇಶ += "An I/O error occurred on the file system.";
    ಭಙ್ಗ;
  }

  ಯದಿ (!Context.empty()) {
    Err_ಸಂದೇಶ += "  ";
    Err_ಸಂದೇಶ += Context;
  }
}

ಶೂನ್ಯ ದ್ವಿಮಾನ_Stream_Error::log(raw_ostream &OS) ಧ್ರುವ { OS << Err_ಸಂದೇಶ; }

StringRef ದ್ವಿಮಾನ_Stream_Error::getErrorMessage() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Err_ಸಂದೇಶ; }

std::error_code ದ್ವಿಮಾನ_Stream_Error::convertToErrorCode() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ inconvertibleErrorCode();
}
