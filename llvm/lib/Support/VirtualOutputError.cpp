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

#അന്തര്ഭാവ "llvm/Support/VirtualOutputError.h"

using നാമാകാശ llvm;
using നാമാകാശ llvm::vfs;

ശൂന്യം OutputError::anchor() {}
ശൂന്യം OutputConfigError::anchor() {}
ശൂന്യം TempFileOutputError::anchor() {}

അക്ഷരം OutputError::ID = 0;
അക്ഷരം OutputConfigError::ID = 0;
അക്ഷരം TempFileOutputError::ID = 0;

ശൂന്യം OutputError::log(raw_ostream &OS) സ്ഥിരം {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

ശൂന്യം OutputConfigError::log(raw_ostream &OS) സ്ഥിരം {
  OutputError::log(OS);
  OS << ": " << Config;
}

ശൂന്യം TempFileOutputError::log(raw_ostream &OS) സ്ഥിരം {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

നാമാകാശ {
വർഗ്ഗം OutputErrorCategory : പൊതു std::error_category {
പൊതു:
  സ്ഥിരം അക്ഷരം *name() സ്ഥിരം noexcept override;
  std::string message(പൂർണ്ണസംഖ്യ EV) സ്ഥിരം override;
};
} // end namespace

സ്ഥിരം std::error_category &vfs::output_category() {
  സ്ഥിതം OutputErrorCategory ErrorCategory;
  തിരികെ ErrorCategory;
}

സ്ഥിരം അക്ഷരം *OutputErrorCategory::name() സ്ഥിരം noexcept {
  തിരികെ "llvm.vfs.output";
}

std::string OutputErrorCategory::message(പൂർണ്ണസംഖ്യ EV) സ്ഥിരം {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  തിരഞ്ഞെടുപ്പ് (E) {
  അവസ്ഥ OutputErrorCode::invalid_config:
    തിരികെ "invalid config";
  അവസ്ഥ OutputErrorCode::not_closed:
    തിരികെ "output not closed";
  അവസ്ഥ OutputErrorCode::already_closed:
    തിരികെ "output already closed";
  അവസ്ഥ OutputErrorCode::has_open_proxy:
    തിരികെ "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
