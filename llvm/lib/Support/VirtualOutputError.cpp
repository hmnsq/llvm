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

#অন্তর্ভাৱ "llvm/Support/VirtualOutputError.h"

using নামাকাশ llvm;
using নামাকাশ llvm::vfs;

শূন্য OutputError::anchor() {}
শূন্য OutputConfigError::anchor() {}
শূন্য TempFileOutputError::anchor() {}

ৱর্ণ OutputError::ID = 0;
ৱর্ণ OutputConfigError::ID = 0;
ৱর্ণ TempFileOutputError::ID = 0;

শূন্য OutputError::log(raw_ostream &OS) ধ্রুৱ {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

শূন্য OutputConfigError::log(raw_ostream &OS) ধ্রুৱ {
  OutputError::log(OS);
  OS << ": " << Config;
}

শূন্য TempFileOutputError::log(raw_ostream &OS) ধ্রুৱ {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

নামাকাশ {
শ্ৰেণী OutputErrorCategory : সার্ৱজনিক std::error_category {
সার্ৱজনিক:
  ধ্রুৱ ৱর্ণ *name() ধ্রুৱ noexcept override;
  std::string message(পূর্ণাঙ্ক EV) ধ্রুৱ override;
};
} // end namespace

ধ্রুৱ std::error_category &vfs::output_category() {
  স্থির OutputErrorCategory ErrorCategory;
  ঘূৰাই ErrorCategory;
}

ধ্রুৱ ৱর্ণ *OutputErrorCategory::name() ধ্রুৱ noexcept {
  ঘূৰাই "llvm.vfs.output";
}

std::string OutputErrorCategory::message(পূর্ণাঙ্ক EV) ধ্রুৱ {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  ৱিকল্প (E) {
  পক্ষ OutputErrorCode::invalid_config:
    ঘূৰাই "invalid config";
  পক্ষ OutputErrorCode::not_closed:
    ঘূৰাই "output not closed";
  পক্ষ OutputErrorCode::already_closed:
    ঘূৰাই "output already closed";
  পক্ষ OutputErrorCode::has_open_proxy:
    ঘূৰাই "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
