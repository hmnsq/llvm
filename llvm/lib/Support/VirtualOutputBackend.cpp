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

#அந்தர்பாவ "llvm/Support/VirtualOutputBackend.h"
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/Support/VirtualOutputError.h"

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::vfs;

வெறுமை OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(மாறிலி Twine &Path,
                          std::optional<OutputConfig> Config) {
  SmallString<128> PathStorage;
  Path.toVector(PathStorage);

  எனில் (Config) {
    // Check for invalid configs.
    எனில் (!Config->getText() && Config->getCRLF())
      திரும்பு make_error<OutputConfigError>(*Config, PathStorage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  எனில் (Error E = createFileImpl(PathStorage, Config).moveInto(Impl))
    திரும்பு std::move(E);
  ப்ரதிஜ்ஞா(Impl && "Expected valid Impl or Error");
  திரும்பு OutputFile(PathStorage, std::move(Impl));
}
