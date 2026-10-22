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

#অন্তর্ভাব "llvm/Support/VirtualOutputBackend.h"
#অন্তর্ভাব "llvm/ADT/SmallString.h"
#অন্তর্ভাব "llvm/Support/VirtualOutputError.h"

using নামাকাশ llvm;
using নামাকাশ llvm::vfs;

শূন্য OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(ধ্রুবক Twine &Path,
                          std::optional<OutputConfig> Config) {
  SmallString<128> PathStorage;
  Path.toVector(PathStorage);

  যদি (Config) {
    // Check for invalid configs.
    যদি (!Config->getText() && Config->getCRLF())
      ফেরত make_error<OutputConfigError>(*Config, PathStorage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  যদি (Error E = createFileImpl(PathStorage, Config).moveInto(Impl))
    ফেরত std::move(E);
  প্রতিজ্ঞা(Impl && "Expected valid Impl or Error");
  ফেরত OutputFile(PathStorage, std::move(Impl));
}
