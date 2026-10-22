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

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::vfs;

शून्य OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(ध्रुव Twine &मार्गः,
                          std::optional<OutputConfig> Config) {
  SmallString<128> मार्गः_Storage;
  मार्गः.गन्तव्यम्_सदिशम्(मार्गः_Storage);

  यदि (Config) {
    // Check for invalid configs.
    यदि (!Config->getText() && Config->getCRLF())
      प्रत्यागम make_error<OutputConfigError>(*Config, मार्गः_Storage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  यदि (Error E = createFileImpl(मार्गः_Storage, Config).moveInto(Impl))
    प्रत्यागम std::move(E);
  प्रतिज्ञा(Impl && "Expected valid Impl or Error");
  प्रत्यागम OutputFile(मार्गः_Storage, std::move(Impl));
}
