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

#അന്തര്ഭാവ "llvm/Support/VirtualOutputBackend.h"
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/Support/VirtualOutputError.h"

using നാമാകാശ llvm;
using നാമാകാശ llvm::vfs;

ശൂന്യം OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(സ്ഥിരം Twine &Path,
                          std::optional<OutputConfig> Config) {
  SmallString<128> PathStorage;
  Path.toVector(PathStorage);

  എങ്കിൽ (Config) {
    // Check for invalid configs.
    എങ്കിൽ (!Config->getText() && Config->getCRLF())
      തിരികെ make_error<OutputConfigError>(*Config, PathStorage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  എങ്കിൽ (Error E = createFileImpl(PathStorage, Config).moveInto(Impl))
    തിരികെ std::move(E);
  പ്രതിജ്ഞാ(Impl && "Expected valid Impl or Error");
  തിരികെ OutputFile(PathStorage, std::move(Impl));
}
