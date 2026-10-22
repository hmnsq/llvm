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

#अन्तर्भाव "llvm/Support/VirtualOutputError.h"

using नामाकाश llvm;
using नामाकाश llvm::vfs;

शून्य OutputError::anchor() {}
शून्य OutputConfigError::anchor() {}
शून्य TempFileOutputError::anchor() {}

अक्षर OutputError::ID = 0;
अक्षर OutputConfigError::ID = 0;
अक्षर TempFileOutputError::ID = 0;

शून्य OutputError::log(raw_ostream &OS) स्थिरांक {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

शून्य OutputConfigError::log(raw_ostream &OS) स्थिरांक {
  OutputError::log(OS);
  OS << ": " << Config;
}

शून्य TempFileOutputError::log(raw_ostream &OS) स्थिरांक {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

नामाकाश {
वर्ग OutputErrorCategory : सार्वजनिक std::error_category {
सार्वजनिक:
  स्थिरांक अक्षर *name() स्थिरांक noexcept override;
  std::string message(पूर्णांक EV) स्थिरांक override;
};
} // end namespace

स्थिरांक std::error_category &vfs::output_category() {
  स्थिर OutputErrorCategory ErrorCategory;
  परत ErrorCategory;
}

स्थिरांक अक्षर *OutputErrorCategory::name() स्थिरांक noexcept {
  परत "llvm.vfs.output";
}

std::string OutputErrorCategory::message(पूर्णांक EV) स्थिरांक {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  निवड (E) {
  स्थिती OutputErrorCode::invalid_config:
    परत "invalid config";
  स्थिती OutputErrorCode::not_closed:
    परत "output not closed";
  स्थिती OutputErrorCode::already_closed:
    परत "output already closed";
  स्थिती OutputErrorCode::has_open_proxy:
    परत "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
