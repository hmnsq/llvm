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

استعمال نامخلا llvm;
استعمال نامخلا llvm::vfs;

خالی OutputError::anchor() {}
خالی OutputConfigError::anchor() {}
خالی عارضی_File_Output_Error::anchor() {}

حرف OutputError::ID = 0;
حرف OutputConfigError::ID = 0;
حرف عارضی_File_Output_Error::ID = 0;

خالی OutputError::لوگارتھم(raw_ostream &OS) ثابت {
  OS << get_Output_راستہ() << ": ";
  ECError::لوگارتھم(OS);
}

خالی OutputConfigError::لوگارتھم(raw_ostream &OS) ثابت {
  OutputError::لوگارتھم(OS);
  OS << ": " << Config;
}

خالی عارضی_File_Output_Error::لوگارتھم(raw_ostream &OS) ثابت {
  OS << get_عارضی_راستہ() << " => ";
  OutputError::لوگارتھم(OS);
}

نامخلا {
جماعت OutputErrorCategory : عوامی std::error_category {
عوامی:
  ثابت حرف *name() ثابت noexcept override;
  std::string پیغام_2(صحیح EV) ثابت override;
};
} // end namespace

ثابت std::error_category &vfs::output_category() {
  ساکن OutputErrorCategory ErrorCategory;
  واپس ErrorCategory;
}

ثابت حرف *OutputErrorCategory::name() ثابت noexcept {
  واپس "llvm.vfs.output";
}

std::string OutputErrorCategory::پیغام_2(صحیح EV) ثابت {
  Output_Error_رمز E = static_cast<Output_Error_رمز>(EV);
  انتخاب (E) {
  حالت Output_Error_رمز::invalid_config:
    واپس "invalid config";
  حالت Output_Error_رمز::not_closed:
    واپس "output not closed";
  حالت Output_Error_رمز::already_closed:
    واپس "output already closed";
  حالت Output_Error_رمز::has_open_proxy:
    واپس "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
