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

#અન્તર્ભાવ "llvm/Support/VirtualOutputBackend.h"
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/Support/VirtualOutputError.h"

using નામાકાશ llvm;
using નામાકાશ llvm::vfs;

શૂન્ય OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(અચલ Twine &Path,
                          std::optional<OutputConfig> Config) {
  SmallString<128> PathStorage;
  Path.toVector(PathStorage);

  જો (Config) {
    // Check for invalid configs.
    જો (!Config->getText() && Config->getCRLF())
      પાછા make_error<OutputConfigError>(*Config, PathStorage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  જો (Error E = createFileImpl(PathStorage, Config).moveInto(Impl))
    પાછા std::move(E);
  પ્રતિજ્ઞા(Impl && "Expected valid Impl or Error");
  પાછા OutputFile(PathStorage, std::move(Impl));
}
