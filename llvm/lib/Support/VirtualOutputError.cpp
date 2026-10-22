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

वर्ण OutputError::ID = 0;
वर्ण OutputConfigError::ID = 0;
वर्ण TempFileOutputError::ID = 0;

शून्य OutputError::log(raw_ostream &OS) ध्रुव {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

शून्य OutputConfigError::log(raw_ostream &OS) ध्रुव {
  OutputError::log(OS);
  OS << ": " << Config;
}

शून्य TempFileOutputError::log(raw_ostream &OS) ध्रुव {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

नामाकाश {
वग्ग OutputErrorCategory : सार्वजनिक std::error_category {
सार्वजनिक:
  ध्रुव वर्ण *name() ध्रुव noexcept override;
  std::string message(पूर्णाङ्क EV) ध्रुव override;
};
} // end namespace

ध्रुव std::error_category &vfs::output_category() {
  स्थिर OutputErrorCategory ErrorCategory;
  पटिनिवत्त ErrorCategory;
}

ध्रुव वर्ण *OutputErrorCategory::name() ध्रुव noexcept {
  पटिनिवत्त "llvm.vfs.output";
}

std::string OutputErrorCategory::message(पूर्णाङ्क EV) ध्रुव {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  विकल्प (E) {
  पक्ष OutputErrorCode::invalid_config:
    पटिनिवत्त "invalid config";
  पक्ष OutputErrorCode::not_closed:
    पटिनिवत्त "output not closed";
  पक्ष OutputErrorCode::already_closed:
    पटिनिवत्त "output already closed";
  पक्ष OutputErrorCode::has_open_proxy:
    पटिनिवत्त "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
