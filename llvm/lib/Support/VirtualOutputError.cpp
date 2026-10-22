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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualOutputError.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::vfs;

ਖਾਲੀ OutputError::anchor() {}
ਖਾਲੀ OutputConfigError::anchor() {}
ਖਾਲੀ ਆਰਜ਼ੀ_File_Output_Error::anchor() {}

ਅੱਖਰ OutputError::ID = 0;
ਅੱਖਰ OutputConfigError::ID = 0;
ਅੱਖਰ ਆਰਜ਼ੀ_File_Output_Error::ID = 0;

ਖਾਲੀ OutputError::ਲਘੂਗਣਕ(raw_ostream &OS) ਅਚਲ {
  OS << get_Output_ਰਸਤਾ() << ": ";
  ECError::ਲਘੂਗਣਕ(OS);
}

ਖਾਲੀ OutputConfigError::ਲਘੂਗਣਕ(raw_ostream &OS) ਅਚਲ {
  OutputError::ਲਘੂਗਣਕ(OS);
  OS << ": " << Config;
}

ਖਾਲੀ ਆਰਜ਼ੀ_File_Output_Error::ਲਘੂਗਣਕ(raw_ostream &OS) ਅਚਲ {
  OS << get_ਆਰਜ਼ੀ_ਰਸਤਾ() << " => ";
  OutputError::ਲਘੂਗਣਕ(OS);
}

ਨਾਮਾਕਾਸ਼ {
ਵਰਗ OutputErrorCategory : ਜਨਤਕ std::error_category {
ਜਨਤਕ:
  ਅਚਲ ਅੱਖਰ *name() ਅਚਲ noexcept override;
  std::string ਸੁਨੇਹਾ_2(ਪੂਰਨ EV) ਅਚਲ override;
};
} // end namespace

ਅਚਲ std::error_category &vfs::output_category() {
  ਸਥਿਰ OutputErrorCategory ErrorCategory;
  ਵਾਪਸ ErrorCategory;
}

ਅਚਲ ਅੱਖਰ *OutputErrorCategory::name() ਅਚਲ noexcept {
  ਵਾਪਸ "llvm.vfs.output";
}

std::string OutputErrorCategory::ਸੁਨੇਹਾ_2(ਪੂਰਨ EV) ਅਚਲ {
  Output_Error_ਕੋਡ E = static_cast<Output_Error_ਕੋਡ>(EV);
  ਚੋਣ (E) {
  ਹਾਲਤ Output_Error_ਕੋਡ::invalid_config:
    ਵਾਪਸ "invalid config";
  ਹਾਲਤ Output_Error_ਕੋਡ::not_closed:
    ਵਾਪਸ "output not closed";
  ਹਾਲਤ Output_Error_ਕੋਡ::already_closed:
    ਵਾਪਸ "output already closed";
  ਹਾਲਤ Output_Error_ਕੋਡ::has_open_proxy:
    ਵਾਪਸ "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
