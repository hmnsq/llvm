//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the errors for output virtualization.
///
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualOutputError.h"

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::vfs;

ಶೂನ್ಯ OutputError::anchor() {}
ಶೂನ್ಯ OutputConfigError::anchor() {}
ಶೂನ್ಯ TempFileOutputError::anchor() {}

ವರ್ಣ OutputError::ID = 0;
ವರ್ಣ OutputConfigError::ID = 0;
ವರ್ಣ TempFileOutputError::ID = 0;

ಶೂನ್ಯ OutputError::log(raw_ostream &OS) ಧ್ರುವ {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

ಶೂನ್ಯ OutputConfigError::log(raw_ostream &OS) ಧ್ರುವ {
  OutputError::log(OS);
  OS << ": " << Config;
}

ಶೂನ್ಯ TempFileOutputError::log(raw_ostream &OS) ಧ್ರುವ {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

ನಾಮಾಕಾಶ {
ವರ್ಗ OutputErrorCategory : ಸಾರ್ವಜನಿಕ std::error_category {
ಸಾರ್ವಜನಿಕ:
  ಧ್ರುವ ವರ್ಣ *name() ಧ್ರುವ noexcept override;
  std::string message(ಪೂರ್ಣಾಙ್ಕ EV) ಧ್ರುವ override;
};
} // end namespace

ಧ್ರುವ std::error_category &vfs::output_category() {
  ಸ್ಥಿರ OutputErrorCategory ErrorCategory;
  ಪ್ರತ್ಯಾಗಮ ErrorCategory;
}

ಧ್ರುವ ವರ್ಣ *OutputErrorCategory::name() ಧ್ರುವ noexcept {
  ಪ್ರತ್ಯಾಗಮ "llvm.vfs.output";
}

std::string OutputErrorCategory::message(ಪೂರ್ಣಾಙ್ಕ EV) ಧ್ರುವ {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  ವಿಕಲ್ಪ (E) {
  ಪಕ್ಷ OutputErrorCode::invalid_config:
    ಪ್ರತ್ಯಾಗಮ "invalid config";
  ಪಕ್ಷ OutputErrorCode::not_closed:
    ಪ್ರತ್ಯಾಗಮ "output not closed";
  ಪಕ್ಷ OutputErrorCode::already_closed:
    ಪ್ರತ್ಯಾಗಮ "output already closed";
  ಪಕ್ಷ OutputErrorCode::has_open_proxy:
    ಪ್ರತ್ಯಾಗಮ "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
