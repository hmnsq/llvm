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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualOutputBackend.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualOutputError.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::vfs;

ਖਾਲੀ OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(ਅਚਲ Twine &ਰਸਤਾ,
                          std::optional<OutputConfig> Config) {
  SmallString<128> ਰਸਤਾ_Storage;
  ਰਸਤਾ.ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਰਸਤਾ_Storage);

  ਜੇ (Config) {
    // Check for invalid configs.
    ਜੇ (!Config->get_ਪਾਠ() && Config->getCRLF())
      ਵਾਪਸ make_error<OutputConfigError>(*Config, ਰਸਤਾ_Storage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  ਜੇ (Error E = createFileImpl(ਰਸਤਾ_Storage, Config).moveInto(Impl))
    ਵਾਪਸ std::move(E);
  ਪ੍ਰਤਿਜ੍ਞਾ(Impl && "Expected valid Impl or Error");
  ਵਾਪਸ OutputFile(ਰਸਤਾ_Storage, std::move(Impl));
}
