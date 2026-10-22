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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::vfs;

ಶೂನ್ಯ OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(ಸ್ಥಿರಾಂಕ Twine &ಮಾರ್ಗ,
                          std::optional<OutputConfig> Config) {
  SmallString<128> ಮಾರ್ಗ_Storage;
  ಮಾರ್ಗ.ಗಮ್ಯ_ಸದಿಶ(ಮಾರ್ಗ_Storage);

  ಆದರೆ (Config) {
    // Check for invalid configs.
    ಆದರೆ (!Config->get_ಪಠ್ಯ() && Config->getCRLF())
      ಹಿಂತಿರುಗಿ make_error<OutputConfigError>(*Config, ಮಾರ್ಗ_Storage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  ಆದರೆ (Error E = createFileImpl(ಮಾರ್ಗ_Storage, Config).moveInto(Impl))
    ಹಿಂತಿರುಗಿ std::move(E);
  ಪ್ರತಿಜ್ಞಾ(Impl && "Expected valid Impl or Error");
  ಹಿಂತಿರುಗಿ OutputFile(ಮಾರ್ಗ_Storage, std::move(Impl));
}
