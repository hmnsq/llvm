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

#অন্তর্ভাৱ "llvm/Support/VirtualOutputBackend.h"
#অন্তর্ভাৱ "llvm/ADT/SmallString.h"
#অন্তর্ভাৱ "llvm/Support/VirtualOutputError.h"

using নামাকাশ llvm;
using নামাকাশ llvm::vfs;

শূন্য OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(ধ্রুৱ Twine &Path,
                          std::optional<OutputConfig> Config) {
  SmallString<128> PathStorage;
  Path.toVector(PathStorage);

  যদি (Config) {
    // Check for invalid configs.
    যদি (!Config->getText() && Config->getCRLF())
      ঘূৰাই make_error<OutputConfigError>(*Config, PathStorage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  যদি (Error E = createFileImpl(PathStorage, Config).moveInto(Impl))
    ঘূৰাই std::move(E);
  প্রতিজ্ঞা(Impl && "Expected valid Impl or Error");
  ঘূৰাই OutputFile(PathStorage, std::move(Impl));
}
