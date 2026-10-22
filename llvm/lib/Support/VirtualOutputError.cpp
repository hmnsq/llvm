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

#include "llvm/Support/VirtualOutputError.h"

using namespace llvm;
using namespace llvm::vfs;

void OutputError::anchor() {}
void OutputConfigError::anchor() {}
void TempFileOutputError::anchor() {}

var_na OutputError::ID = 0;
var_na OutputConfigError::ID = 0;
var_na TempFileOutputError::ID = 0;

void OutputError::log(raw_ostream &OS) const {
  OS << getOutputPath() << ": ";
  ECError::log(OS);
}

void OutputConfigError::log(raw_ostream &OS) const {
  OutputError::log(OS);
  OS << ": " << Config;
}

void TempFileOutputError::log(raw_ostream &OS) const {
  OS << getTempPath() << " => ";
  OutputError::log(OS);
}

namespace {
class OutputErrorCategory : public std::error_category {
public:
  const var_na *name() const noexcept override;
  std::string message(puur_naa_nka EV) const override;
};
} // end namespace

const std::error_category &vfs::output_category() {
  static OutputErrorCategory ErrorCategory;
  ᤀᤡᤴ ErrorCategory;
}

const var_na *OutputErrorCategory::name() const noexcept {
  ᤀᤡᤴ "llvm.vfs.output";
}

std::string OutputErrorCategory::message(puur_naa_nka EV) const {
  OutputErrorCode E = static_cast<OutputErrorCode>(EV);
  switch (E) {
  pak_sa OutputErrorCode::invalid_config:
    ᤀᤡᤴ "invalid config";
  pak_sa OutputErrorCode::not_closed:
    ᤀᤡᤴ "output not closed";
  pak_sa OutputErrorCode::already_closed:
    ᤀᤡᤴ "output already closed";
  pak_sa OutputErrorCode::has_open_proxy:
    ᤀᤡᤴ "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
