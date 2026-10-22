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
শূন্য অস্থায়ী_File_Output_Error::anchor() {}

অক্ষর OutputError::ID = 0;
অক্ষর OutputConfigError::ID = 0;
অক্ষর অস্থায়ী_File_Output_Error::ID = 0;

শূন্য OutputError::log(raw_ostream &OS) ধ্রুবক {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

শূন্য OutputConfigError::log(raw_ostream &OS) ধ্রুবক {
  OutputError::log(OS);
  OS << ": " << Config;
}

শূন্য অস্থায়ী_File_Output_Error::log(raw_ostream &OS) ধ্রুবক {
  OS << get_অস্থায়ী_Path() << " => ";
  OutputError::log(OS);
}

নামাকাশ {
শ্রেণী OutputErrorCategory : সর্বজনীন std::error_category {
সর্বজনীন:
  ধ্রুবক অক্ষর *name() ধ্রুবক noexcept override;
  std::string message(পূর্ণসংখ্যা EV) ধ্রুবক override;
};
} // end namespace

ধ্রুবক std::error_category &vfs::output_category() {
  স্থির OutputErrorCategory ErrorCategory;
  ফেরত ErrorCategory;
}

ধ্রুবক অক্ষর *OutputErrorCategory::name() ধ্রুবক noexcept {
  ফেরত "llvm.vfs.output";
}

std::string OutputErrorCategory::message(পূর্ণসংখ্যা EV) ধ্রুবক {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  নির্বাচন (E) {
  ক্ষেত্র OutputErrorCode::invalid_config:
    ফেরত "invalid config";
  ক্ষেত্র OutputErrorCode::not_closed:
    ফেরত "output not closed";
  ক্ষেত্র OutputErrorCode::already_closed:
    ফেরত "output already closed";
  ক্ষেত্র OutputErrorCode::has_open_proxy:
    ফেরত "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
