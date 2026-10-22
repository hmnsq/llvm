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

#অন্তর্ভাব "llvm/Support/VirtualOutputError.h"

using নামাকাশ llvm;
using নামাকাশ llvm::vfs;

শূন্য OutputError::anchor() {}
শূন্য OutputConfigError::anchor() {}
শূন্য TempFileOutputError::anchor() {}

বর্ণ OutputError::ID = 0;
বর্ণ OutputConfigError::ID = 0;
বর্ণ TempFileOutputError::ID = 0;

শূন্য OutputError::log(raw_ostream &OS) ধ্রুব {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

শূন্য OutputConfigError::log(raw_ostream &OS) ধ্রুব {
  OutputError::log(OS);
  OS << ": " << Config;
}

শূন্য TempFileOutputError::log(raw_ostream &OS) ধ্রুব {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

নামাকাশ {
বর্গ OutputErrorCategory : সার্বজনিক std::error_category {
সার্বজনিক:
  ধ্রুব বর্ণ *name() ধ্রুব noexcept override;
  std::string message(পূর্ণাঙ্ক EV) ধ্রুব override;
};
} // end namespace

ধ্রুব std::error_category &vfs::output_category() {
  স্থির OutputErrorCategory ErrorCategory;
  প্রত্যাগম ErrorCategory;
}

ধ্রুব বর্ণ *OutputErrorCategory::name() ধ্রুব noexcept {
  প্রত্যাগম "llvm.vfs.output";
}

std::string OutputErrorCategory::message(পূর্ণাঙ্ক EV) ধ্রুব {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  বিকল্প (E) {
  পক্ষ OutputErrorCode::invalid_config:
    প্রত্যাগম "invalid config";
  পক্ষ OutputErrorCode::not_closed:
    প্রত্যাগম "output not closed";
  পক্ষ OutputErrorCode::already_closed:
    প্রত্যাগম "output already closed";
  পক্ষ OutputErrorCode::has_open_proxy:
    প্রত্যাগম "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
