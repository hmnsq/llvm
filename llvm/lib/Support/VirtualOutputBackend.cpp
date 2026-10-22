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

#समावेश "llvm/Support/VirtualOutputBackend.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/Support/VirtualOutputError.h"

using namespace llvm;
using namespace llvm::vfs;

व्योम OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(स्थिर Twine &Path,
                          std::optional<OutputConfig> Config) {
  SmallString<128> PathStorage;
  Path.toVector(PathStorage);

  अगर (Config) {
    // Check for invalid configs.
    अगर (!Config->getText() && Config->getCRLF())
      वापस make_error<OutputConfigError>(*Config, PathStorage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  अगर (Error E = createFileImpl(PathStorage, Config).moveInto(Impl))
    वापस std::move(E);
  निश्चित(Impl && "Expected valid Impl or Error");
  वापस OutputFile(PathStorage, std::move(Impl));
}
