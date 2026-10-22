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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::vfs;

ಶೂನ್ಯ OutputError::anchor() {}
ಶೂನ್ಯ OutputConfigError::anchor() {}
ಶೂನ್ಯ ತಾತ್ಕಾಲಿಕ_File_Output_Error::anchor() {}

ಅಕ್ಷರ OutputError::ID = 0;
ಅಕ್ಷರ OutputConfigError::ID = 0;
ಅಕ್ಷರ ತಾತ್ಕಾಲಿಕ_File_Output_Error::ID = 0;

ಶೂನ್ಯ OutputError::ಲಘುಗಣಕ(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  OS << get_Output_ಮಾರ್ಗ() << ": ";
  ECError::ಲಘುಗಣಕ(OS);
}

ಶೂನ್ಯ OutputConfigError::ಲಘುಗಣಕ(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  OutputError::ಲಘುಗಣಕ(OS);
  OS << ": " << Config;
}

ಶೂನ್ಯ ತಾತ್ಕಾಲಿಕ_File_Output_Error::ಲಘುಗಣಕ(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  OS << get_ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ() << " => ";
  OutputError::ಲಘುಗಣಕ(OS);
}

ನಾಮಾಕಾಶ {
ವರ್ಗ OutputErrorCategory : ಸಾರ್ವಜನಿಕ std::error_category {
ಸಾರ್ವಜನಿಕ:
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name() ಸ್ಥಿರಾಂಕ noexcept override;
  std::string ಸಂದೇಶ_2(ಪೂರ್ಣಾಂಕ EV) ಸ್ಥಿರಾಂಕ override;
};
} // end namespace

ಸ್ಥಿರಾಂಕ std::error_category &vfs::output_category() {
  ಸ್ಥಿರ OutputErrorCategory ErrorCategory;
  ಹಿಂತಿರುಗಿ ErrorCategory;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *OutputErrorCategory::name() ಸ್ಥಿರಾಂಕ noexcept {
  ಹಿಂತಿರುಗಿ "llvm.vfs.output";
}

std::string OutputErrorCategory::ಸಂದೇಶ_2(ಪೂರ್ಣಾಂಕ EV) ಸ್ಥಿರಾಂಕ {
  Output_Error_ಸಂಕೇತ E = static_cast<Output_Error_ಸಂಕೇತ>(EV);
  ಆಯ್ಕೆ (E) {
  ಸ್ಥಿತಿ Output_Error_ಸಂಕೇತ::invalid_config:
    ಹಿಂತಿರುಗಿ "invalid config";
  ಸ್ಥಿತಿ Output_Error_ಸಂಕೇತ::not_closed:
    ಹಿಂತಿರುಗಿ "output not closed";
  ಸ್ಥಿತಿ Output_Error_ಸಂಕೇತ::already_closed:
    ಹಿಂತಿರುಗಿ "output already closed";
  ಸ್ಥಿತಿ Output_Error_ಸಂಕೇತ::has_open_proxy:
    ಹಿಂತಿರುಗಿ "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
