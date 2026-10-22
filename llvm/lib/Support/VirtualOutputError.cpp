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

#અન્તર્ભાવ "llvm/Support/VirtualOutputError.h"

using નામાકાશ llvm;
using નામાકાશ llvm::vfs;

શૂન્ય OutputError::anchor() {}
શૂન્ય OutputConfigError::anchor() {}
શૂન્ય TempFileOutputError::anchor() {}

અક્ષર OutputError::ID = 0;
અક્ષર OutputConfigError::ID = 0;
અક્ષર TempFileOutputError::ID = 0;

શૂન્ય OutputError::log(raw_ostream &OS) અચલ {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

શૂન્ય OutputConfigError::log(raw_ostream &OS) અચલ {
  OutputError::log(OS);
  OS << ": " << Config;
}

શૂન્ય TempFileOutputError::log(raw_ostream &OS) અચલ {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

નામાકાશ {
વર્ગ OutputErrorCategory : જાહેર std::error_category {
જાહેર:
  અચલ અક્ષર *name() અચલ noexcept override;
  std::string message(પૂર્ણાંક EV) અચલ override;
};
} // end namespace

અચલ std::error_category &vfs::output_category() {
  સ્થિર OutputErrorCategory ErrorCategory;
  પાછા ErrorCategory;
}

અચલ અક્ષર *OutputErrorCategory::name() અચલ noexcept {
  પાછા "llvm.vfs.output";
}

std::string OutputErrorCategory::message(પૂર્ણાંક EV) અચલ {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  પસંદગી (E) {
  સ્થિતિ OutputErrorCode::invalid_config:
    પાછા "invalid config";
  સ્થિતિ OutputErrorCode::not_closed:
    પાછા "output not closed";
  સ્થિતિ OutputErrorCode::already_closed:
    પાછા "output already closed";
  સ્થિતિ OutputErrorCode::has_open_proxy:
    પાછા "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
