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

#அந்தர்பாவ "llvm/Support/VirtualOutputError.h"

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::vfs;

வெறுமை OutputError::anchor() {}
வெறுமை OutputConfigError::anchor() {}
வெறுமை TempFileOutputError::anchor() {}

எழுத்து OutputError::ID = 0;
எழுத்து OutputConfigError::ID = 0;
எழுத்து TempFileOutputError::ID = 0;

வெறுமை OutputError::log(raw_ostream &OS) மாறிலி {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

வெறுமை OutputConfigError::log(raw_ostream &OS) மாறிலி {
  OutputError::log(OS);
  OS << ": " << Config;
}

வெறுமை TempFileOutputError::log(raw_ostream &OS) மாறிலி {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

நாமாகாஶ {
வகுப்பு OutputErrorCategory : பொது std::error_category {
பொது:
  மாறிலி எழுத்து *name() மாறிலி noexcept override;
  std::string message(முழு EV) மாறிலி override;
};
} // end namespace

மாறிலி std::error_category &vfs::output_category() {
  நிலையான OutputErrorCategory ErrorCategory;
  திரும்பு ErrorCategory;
}

மாறிலி எழுத்து *OutputErrorCategory::name() மாறிலி noexcept {
  திரும்பு "llvm.vfs.output";
}

std::string OutputErrorCategory::message(முழு EV) மாறிலி {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  தேர்வு (E) {
  நிலை OutputErrorCode::invalid_config:
    திரும்பு "invalid config";
  நிலை OutputErrorCode::not_closed:
    திரும்பு "output not closed";
  நிலை OutputErrorCode::already_closed:
    திரும்பு "output already closed";
  நிலை OutputErrorCode::has_open_proxy:
    திரும்பு "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
