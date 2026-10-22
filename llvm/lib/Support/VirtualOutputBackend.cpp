//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements \c vfs::OutputBackend class methods.
///
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualOutputBackend.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualOutputError.h"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::vfs;

ଶୂନ୍ୟ OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(ଅଚଳ Twine &Path,
                          std::optional<OutputConfig> Config) {
  SmallString<128> PathStorage;
  Path.toVector(PathStorage);

  ଯଦି (Config) {
    // Check for invalid configs.
    ଯଦି (!Config->getText() && Config->getCRLF())
      ଫେରନ୍ତୁ make_error<OutputConfigError>(*Config, PathStorage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  ଯଦି (Error E = createFileImpl(PathStorage, Config).moveInto(Impl))
    ଫେରନ୍ତୁ std::move(E);
  ପ୍ରତିଜ୍ଞା(Impl && "Expected valid Impl or Error");
  ଫେରନ୍ତୁ OutputFile(PathStorage, std::move(Impl));
}
