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

کارول نطاق llvm;
کارول نطاق llvm::vfs;

فراغ OutputError::anchor() {}
فراغ OutputConfigError::anchor() {}
فراغ لنډمهاله_File_Output_Error::anchor() {}

حرف OutputError::ID = 0;
حرف OutputConfigError::ID = 0;
حرف لنډمهاله_File_Output_Error::ID = 0;

فراغ OutputError::لوګاریتم(raw_ostream &OS) ثابت {
  OS << get_Output_لار() << ": ";
  ECError::لوګاریتم(OS);
}

فراغ OutputConfigError::لوګاریتم(raw_ostream &OS) ثابت {
  OutputError::لوګاریتم(OS);
  OS << ": " << Config;
}

فراغ لنډمهاله_File_Output_Error::لوګاریتم(raw_ostream &OS) ثابت {
  OS << get_لنډمهاله_لار() << " => ";
  OutputError::لوګاریتم(OS);
}

نطاق {
صنف OutputErrorCategory : عام std::error_category {
عام:
  ثابت حرف *name() ثابت noexcept override;
  std::string پیغام_2(صحيح EV) ثابت override;
};
} // end namespace

ثابت std::error_category &vfs::output_category() {
  ساكن OutputErrorCategory ErrorCategory;
  أرجع ErrorCategory;
}

ثابت حرف *OutputErrorCategory::name() ثابت noexcept {
  أرجع "llvm.vfs.output";
}

std::string OutputErrorCategory::پیغام_2(صحيح EV) ثابت {
  Output_Error_کوډ E = static_cast<Output_Error_کوډ>(EV);
  بدل (E) {
  حالة Output_Error_کوډ::invalid_config:
    أرجع "invalid config";
  حالة Output_Error_کوډ::not_closed:
    أرجع "output not closed";
  حالة Output_Error_کوډ::already_closed:
    أرجع "output already closed";
  حالة Output_Error_کوډ::has_open_proxy:
    أرجع "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
