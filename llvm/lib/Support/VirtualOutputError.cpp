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

استعمال نطاق llvm;
استعمال نطاق llvm::vfs;

فراغ OutputError::anchor() {}
فراغ OutputConfigError::anchor() {}
فراغ عارضي_File_Output_Error::anchor() {}

حرف OutputError::ID = 0;
حرف OutputConfigError::ID = 0;
حرف عارضي_File_Output_Error::ID = 0;

فراغ OutputError::لاگرٿم(raw_ostream &OS) ثابت {
  OS << get_Output_رستو() << ": ";
  ECError::لاگرٿم(OS);
}

فراغ OutputConfigError::لاگرٿم(raw_ostream &OS) ثابت {
  OutputError::لاگرٿم(OS);
  OS << ": " << Config;
}

فراغ عارضي_File_Output_Error::لاگرٿم(raw_ostream &OS) ثابت {
  OS << get_عارضي_رستو() << " => ";
  OutputError::لاگرٿم(OS);
}

نطاق {
صنف OutputErrorCategory : عام std::error_category {
عام:
  ثابت حرف *name() ثابت noexcept override;
  std::string پيغام(صحيح EV) ثابت override;
};
} // end namespace

ثابت std::error_category &vfs::output_category() {
  ساكن OutputErrorCategory ErrorCategory;
  أرجع ErrorCategory;
}

ثابت حرف *OutputErrorCategory::name() ثابت noexcept {
  أرجع "llvm.vfs.output";
}

std::string OutputErrorCategory::پيغام(صحيح EV) ثابت {
  Output_Error_ڪوڊ E = static_cast<Output_Error_ڪوڊ>(EV);
  بدل (E) {
  حالة Output_Error_ڪوڊ::invalid_config:
    أرجع "invalid config";
  حالة Output_Error_ڪوڊ::not_closed:
    أرجع "output not closed";
  حالة Output_Error_ڪوڊ::already_closed:
    أرجع "output already closed";
  حالة Output_Error_ڪوڊ::has_open_proxy:
    أرجع "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
