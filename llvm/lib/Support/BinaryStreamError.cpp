//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamError.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಅಕ್ಷರ ದ್ವಿಮಾನ_ಹರಿವು_Error::ID = 0;

ದ್ವಿಮಾನ_ಹರಿವು_Error::ದ್ವಿಮಾನ_ಹರಿವು_Error(ಹರಿವು_error_ಸಂಕೇತ C)
    : ದ್ವಿಮಾನ_ಹರಿವು_Error(C, "") {}

ದ್ವಿಮಾನ_ಹರಿವು_Error::ದ್ವಿಮಾನ_ಹರಿವು_Error(StringRef Context)
    : ದ್ವಿಮಾನ_ಹರಿವು_Error(ಹರಿವು_error_ಸಂಕೇತ::unspecified, Context) {}

ದ್ವಿಮಾನ_ಹರಿವು_Error::ದ್ವಿಮಾನ_ಹರಿವು_Error(ಹರಿವು_error_ಸಂಕೇತ C, StringRef Context)
    : ಸಂಕೇತ_2(C) {
  Err_ಸಂದೇಶ = "Stream Error: ";
  ಆಯ್ಕೆ (C) {
  ಸ್ಥಿತಿ ಹರಿವು_error_ಸಂಕೇತ::unspecified:
    Err_ಸಂದೇಶ += "An unspecified error has occurred.";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ ಹರಿವು_error_ಸಂಕೇತ::ಹರಿವು_too_short:
    Err_ಸಂದೇಶ += "The stream is too short to perform the requested operation.";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ ಹರಿವು_error_ಸಂಕೇತ::invalid_array_ಗಾತ್ರ:
    Err_ಸಂದೇಶ += "The buffer size is not a multiple of the array element size.";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ ಹರಿವು_error_ಸಂಕೇತ::invalid_ಸ್ಥಾನಾಂತರ:
    Err_ಸಂದೇಶ += "The specified offset is invalid for the current stream.";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ ಹರಿವು_error_ಸಂಕೇತ::filesystem_error:
    Err_ಸಂದೇಶ += "An I/O error occurred on the file system.";
    ನಿಲ್ಲಿಸು;
  }

  ಆದರೆ (!Context.ಖಾಲಿಗಳು()) {
    Err_ಸಂದೇಶ += "  ";
    Err_ಸಂದೇಶ += Context;
  }
}

ಶೂನ್ಯ ದ್ವಿಮಾನ_ಹರಿವು_Error::ಲಘುಗಣಕ(raw_ostream &OS) ಸ್ಥಿರಾಂಕ { OS << Err_ಸಂದೇಶ; }

StringRef ದ್ವಿಮಾನ_ಹರಿವು_Error::get_Error_ಸಂದೇಶ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Err_ಸಂದೇಶ; }

std::error_ಸಂಕೇತ ದ್ವಿಮಾನ_ಹರಿವು_Error::convert_ಗಮ್ಯ_Error_ಸಂಕೇತ() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ inconvertible_Error_ಸಂಕೇತ();
}
