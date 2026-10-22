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

#شامل "llvm/Support/VirtualOutputError.h"

using فضاینام llvm;
using فضاینام llvm::vfs;

تهی OutputError::anchor() {}
تهی OutputConfigError::anchor() {}
تهی TempFileOutputError::anchor() {}

نویسه OutputError::ID = 0;
نویسه OutputConfigError::ID = 0;
نویسه TempFileOutputError::ID = 0;

تهی OutputError::log(raw_ostream &OS) ثابت {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

تهی OutputConfigError::log(raw_ostream &OS) ثابت {
  OutputError::log(OS);
  OS << ": " << Config;
}

تهی TempFileOutputError::log(raw_ostream &OS) ثابت {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

فضاینام {
رده OutputErrorCategory : عمومی std::error_category {
عمومی:
  ثابت نویسه *name() ثابت noexcept override;
  std::string message(صحیح EV) ثابت override;
};
} // end namespace

ثابت std::error_category &vfs::output_category() {
  ایستا OutputErrorCategory ErrorCategory;
  بازگشت ErrorCategory;
}

ثابت نویسه *OutputErrorCategory::name() ثابت noexcept {
  بازگشت "llvm.vfs.output";
}

std::string OutputErrorCategory::message(صحیح EV) ثابت {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  انتخاب (E) {
  حالت OutputErrorCode::invalid_config:
    بازگشت "invalid config";
  حالت OutputErrorCode::not_closed:
    بازگشت "output not closed";
  حالت OutputErrorCode::already_closed:
    بازگشت "output already closed";
  حالت OutputErrorCode::has_open_proxy:
    بازگشت "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
