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

#ضمن "llvm/Support/VirtualOutputError.h"

using نطاق llvm;
using نطاق llvm::vfs;

فراغ OutputError::anchor() {}
فراغ OutputConfigError::anchor() {}
فراغ TempFileOutputError::anchor() {}

حرف OutputError::ID = 0;
حرف OutputConfigError::ID = 0;
حرف TempFileOutputError::ID = 0;

فراغ OutputError::log(raw_ostream &OS) ثابت {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

فراغ OutputConfigError::log(raw_ostream &OS) ثابت {
  OutputError::log(OS);
  OS << ": " << Config;
}

فراغ TempFileOutputError::log(raw_ostream &OS) ثابت {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

نطاق {
صنف OutputErrorCategory : عام std::error_category {
عام:
  ثابت حرف *name() ثابت noexcept override;
  std::string message(صحيح EV) ثابت override;
};
} // end namespace

ثابت std::error_category &vfs::output_category() {
  ساكن OutputErrorCategory ErrorCategory;
  ܦܢܐ ErrorCategory;
}

ثابت حرف *OutputErrorCategory::name() ثابت noexcept {
  ܦܢܐ "llvm.vfs.output";
}

std::string OutputErrorCategory::message(صحيح EV) ثابت {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  بدل (E) {
  حالة OutputErrorCode::invalid_config:
    ܦܢܐ "invalid config";
  حالة OutputErrorCode::not_closed:
    ܦܢܐ "output not closed";
  حالة OutputErrorCode::already_closed:
    ܦܢܐ "output already closed";
  حالة OutputErrorCode::has_open_proxy:
    ܦܢܐ "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
