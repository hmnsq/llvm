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

ورتوں نطاق llvm;
ورتوں نطاق llvm::vfs;

فراغ OutputError::anchor() {}
فراغ OutputConfigError::anchor() {}
فراغ عارضی_File_Output_Error::anchor() {}

حرف OutputError::ID = 0;
حرف OutputConfigError::ID = 0;
حرف عارضی_File_Output_Error::ID = 0;

فراغ OutputError::لوگارتھم(raw_ostream &OS) ثابت {
  OS << get_Output_رستہ() << ": ";
  ECError::لوگارتھم(OS);
}

فراغ OutputConfigError::لوگارتھم(raw_ostream &OS) ثابت {
  OutputError::لوگارتھم(OS);
  OS << ": " << Config;
}

فراغ عارضی_File_Output_Error::لوگارتھم(raw_ostream &OS) ثابت {
  OS << get_عارضی_رستہ() << " => ";
  OutputError::لوگارتھم(OS);
}

نطاق {
صنف OutputErrorCategory : عام std::error_category {
عام:
  ثابت حرف *name() ثابت noexcept override;
  std::string سنیہا(صحيح EV) ثابت override;
};
} // end namespace

ثابت std::error_category &vfs::output_category() {
  ساكن OutputErrorCategory ErrorCategory;
  أرجع ErrorCategory;
}

ثابت حرف *OutputErrorCategory::name() ثابت noexcept {
  أرجع "llvm.vfs.output";
}

std::string OutputErrorCategory::سنیہا(صحيح EV) ثابت {
  Output_Error_کوڈ E = static_cast<Output_Error_کوڈ>(EV);
  بدل (E) {
  حالة Output_Error_کوڈ::invalid_config:
    أرجع "invalid config";
  حالة Output_Error_کوڈ::not_closed:
    أرجع "output not closed";
  حالة Output_Error_کوڈ::already_closed:
    أرجع "output already closed";
  حالة Output_Error_کوڈ::has_open_proxy:
    أرجع "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
