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

#ضمن "llvm/Support/VirtualOutputBackend.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/Support/VirtualOutputError.h"

using نطاق llvm;
using نطاق llvm::vfs;

فراغ OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(ثابت Twine &Path,
                          std::optional<OutputConfig> Config) {
  SmallString<128> PathStorage;
  Path.toVector(PathStorage);

  ܐܢ (Config) {
    // Check for invalid configs.
    ܐܢ (!Config->getText() && Config->getCRLF())
      ܦܢܐ make_error<OutputConfigError>(*Config, PathStorage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  ܐܢ (Error E = createFileImpl(PathStorage, Config).moveInto(Impl))
    ܦܢܐ std::move(E);
  أكد(Impl && "Expected valid Impl or Error");
  ܦܢܐ OutputFile(PathStorage, std::move(Impl));
}
