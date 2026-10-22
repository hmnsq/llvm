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

استعمال نامخلا llvm;
استعمال نامخلا llvm::vfs;

خالی OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(ثابت Twine &راستہ,
                          std::optional<OutputConfig> Config) {
  SmallString<128> راستہ_Storage;
  راستہ.مقصد_سمتیہ(راستہ_Storage);

  اگر (Config) {
    // Check for invalid configs.
    اگر (!Config->get_متن() && Config->getCRLF())
      واپس make_error<OutputConfigError>(*Config, راستہ_Storage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  اگر (Error E = createFileImpl(راستہ_Storage, Config).moveInto(Impl))
    واپس std::move(E);
  أكد(Impl && "Expected valid Impl or Error");
  واپس OutputFile(راستہ_Storage, std::move(Impl));
}
