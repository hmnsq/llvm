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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualOutputError.h"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::vfs;

ଶୂନ୍ୟ OutputError::anchor() {}
ଶୂନ୍ୟ OutputConfigError::anchor() {}
ଶୂନ୍ୟ TempFileOutputError::anchor() {}

ଅକ୍ଷର OutputError::ID = 0;
ଅକ୍ଷର OutputConfigError::ID = 0;
ଅକ୍ଷର TempFileOutputError::ID = 0;

ଶୂନ୍ୟ OutputError::log(raw_ostream &OS) ଅଚଳ {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

ଶୂନ୍ୟ OutputConfigError::log(raw_ostream &OS) ଅଚଳ {
  OutputError::log(OS);
  OS << ": " << Config;
}

ଶୂନ୍ୟ TempFileOutputError::log(raw_ostream &OS) ଅଚଳ {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

ନାମାକାଶ {
ଶ୍ରେଣୀ OutputErrorCategory : ସର୍ବସାଧାରଣ std::error_category {
ସର୍ବସାଧାରଣ:
  ଅଚଳ ଅକ୍ଷର *name() ଅଚଳ noexcept override;
  std::string message(ପୂର୍ଣ୍ଣସଂଖ୍ୟା EV) ଅଚଳ override;
};
} // end namespace

ଅଚଳ std::error_category &vfs::output_category() {
  ସ୍ଥିର OutputErrorCategory ErrorCategory;
  ଫେରନ୍ତୁ ErrorCategory;
}

ଅଚଳ ଅକ୍ଷର *OutputErrorCategory::name() ଅଚଳ noexcept {
  ଫେରନ୍ତୁ "llvm.vfs.output";
}

std::string OutputErrorCategory::message(ପୂର୍ଣ୍ଣସଂଖ୍ୟା EV) ଅଚଳ {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  ବାଛ (E) {
  ସ୍ଥିତି OutputErrorCode::invalid_config:
    ଫେରନ୍ତୁ "invalid config";
  ସ୍ଥିତି OutputErrorCode::not_closed:
    ଫେରନ୍ତୁ "output not closed";
  ସ୍ଥିତି OutputErrorCode::already_closed:
    ଫେରନ୍ତୁ "output already closed";
  ସ୍ଥିତି OutputErrorCode::has_open_proxy:
    ଫେରନ୍ତୁ "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
