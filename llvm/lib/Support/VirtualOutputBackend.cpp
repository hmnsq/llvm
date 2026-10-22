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

#شامل "llvm/Support/VirtualOutputBackend.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/Support/VirtualOutputError.h"

using فضاینام llvm;
using فضاینام llvm::vfs;

تهی OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(ثابت Twine &Path,
                          std::optional<OutputConfig> Config) {
  SmallString<128> PathStorage;
  Path.toVector(PathStorage);

  اگر (Config) {
    // Check for invalid configs.
    اگر (!Config->getText() && Config->getCRLF())
      بازگشت make_error<OutputConfigError>(*Config, PathStorage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  اگر (Error E = createFileImpl(PathStorage, Config).moveInto(Impl))
    بازگشت std::move(E);
  أكد(Impl && "Expected valid Impl or Error");
  بازگشت OutputFile(PathStorage, std::move(Impl));
}
