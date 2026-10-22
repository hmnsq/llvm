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

#समावेश "llvm/Support/VirtualOutputError.h"

using namespace llvm;
using namespace llvm::vfs;

व्योम OutputError::anchor() {}
व्योम OutputConfigError::anchor() {}
व्योम अस्थायी_File_Output_Error::anchor() {}

अक्षर OutputError::ID = 0;
अक्षर OutputConfigError::ID = 0;
अक्षर अस्थायी_File_Output_Error::ID = 0;

व्योम OutputError::log(raw_ostream &OS) स्थिर {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

व्योम OutputConfigError::log(raw_ostream &OS) स्थिर {
  OutputError::log(OS);
  OS << ": " << Config;
}

व्योम अस्थायी_File_Output_Error::log(raw_ostream &OS) स्थिर {
  OS << get_अस्थायी_Path() << " => ";
  OutputError::log(OS);
}

namespace {
श्रेणी OutputErrorCategory : खुला std::error_category {
खुला:
  स्थिर अक्षर *name() स्थिर noexcept override;
  std::string message(पूर्णांक EV) स्थिर override;
};
} // end namespace

स्थिर std::error_category &vfs::output_category() {
  अटल OutputErrorCategory ErrorCategory;
  वापस ErrorCategory;
}

स्थिर अक्षर *OutputErrorCategory::name() स्थिर noexcept {
  वापस "llvm.vfs.output";
}

std::string OutputErrorCategory::message(पूर्णांक EV) स्थिर {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  चयन (E) {
  हाल OutputErrorCode::invalid_config:
    वापस "invalid config";
  हाल OutputErrorCode::not_closed:
    वापस "output not closed";
  हाल OutputErrorCode::already_closed:
    वापस "output already closed";
  हाल OutputErrorCode::has_open_proxy:
    वापस "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
