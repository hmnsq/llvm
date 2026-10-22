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

#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualOutputBackend.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualOutputError.h"

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::vfs;

ಶೂನ್ಯ OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(ಧ್ರುವ Twine &Path,
                          std::optional<OutputConfig> Config) {
  SmallString<128> PathStorage;
  Path.toVector(PathStorage);

  ಯದಿ (Config) {
    // Check for invalid configs.
    ಯದಿ (!Config->getText() && Config->getCRLF())
      ಪ್ರತ್ಯಾಗಮ make_error<OutputConfigError>(*Config, PathStorage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  ಯದಿ (Error E = createFileImpl(PathStorage, Config).moveInto(Impl))
    ಪ್ರತ್ಯಾಗಮ std::move(E);
  ಪ್ರತಿಜ್ಞಾ(Impl && "Expected valid Impl or Error");
  ಪ್ರತ್ಯಾಗಮ OutputFile(PathStorage, std::move(Impl));
}
