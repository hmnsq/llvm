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

#अन्तर्भाव "llvm/Support/VirtualOutputBackend.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/Support/VirtualOutputError.h"

using नामाकाश llvm;
using नामाकाश llvm::vfs;

शून्य OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(स्थिरांक Twine &Path,
                          std::optional<OutputConfig> Config) {
  SmallString<128> PathStorage;
  Path.toVector(PathStorage);

  जर (Config) {
    // Check for invalid configs.
    जर (!Config->getText() && Config->getCRLF())
      परत make_error<OutputConfigError>(*Config, PathStorage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  जर (Error E = createFileImpl(PathStorage, Config).moveInto(Impl))
    परत std::move(E);
  प्रतिज्ञा(Impl && "Expected valid Impl or Error");
  परत OutputFile(PathStorage, std::move(Impl));
}
