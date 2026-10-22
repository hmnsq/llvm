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

استفاده نطاق llvm;
استفاده نطاق llvm::vfs;

فراغ OutputError::anchor() {}
فراغ OutputConfigError::anchor() {}
فراغ موقت_File_Output_Error::anchor() {}

حرف OutputError::ID = 0;
حرف OutputConfigError::ID = 0;
حرف موقت_File_Output_Error::ID = 0;

فراغ OutputError::لگاریتم(raw_ostream &OS) ثابت {
  OS << get_Output_مسیر() << ": ";
  ECError::لگاریتم(OS);
}

فراغ OutputConfigError::لگاریتم(raw_ostream &OS) ثابت {
  OutputError::لگاریتم(OS);
  OS << ": " << Config;
}

فراغ موقت_File_Output_Error::لگاریتم(raw_ostream &OS) ثابت {
  OS << get_موقت_مسیر() << " => ";
  OutputError::لگاریتم(OS);
}

نطاق {
صنف OutputErrorCategory : عام std::error_category {
عام:
  ثابت حرف *name() ثابت noexcept override;
  std::string پیام_2(صحيح EV) ثابت override;
};
} // end namespace

ثابت std::error_category &vfs::output_category() {
  ساكن OutputErrorCategory ErrorCategory;
  أرجع ErrorCategory;
}

ثابت حرف *OutputErrorCategory::name() ثابت noexcept {
  أرجع "llvm.vfs.output";
}

std::string OutputErrorCategory::پیام_2(صحيح EV) ثابت {
  Output_Error_کد E = static_cast<Output_Error_کد>(EV);
  بدل (E) {
  حالة Output_Error_کد::invalid_config:
    أرجع "invalid config";
  حالة Output_Error_کد::not_closed:
    أرجع "output not closed";
  حالة Output_Error_کد::already_closed:
    أرجع "output already closed";
  حالة Output_Error_کد::has_open_proxy:
    أرجع "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
