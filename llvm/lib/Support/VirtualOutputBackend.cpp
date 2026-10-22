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

کارول نطاق llvm;
کارول نطاق llvm::vfs;

فراغ OutputBackend::anchor() {}

Expected<OutputFile>
OutputBackend::createFile(ثابت Twine &لار,
                          std::optional<OutputConfig> Config) {
  SmallString<128> لار_Storage;
  لار.منزل_ویکټور(لار_Storage);

  إذا (Config) {
    // Check for invalid configs.
    إذا (!Config->get_متن() && Config->getCRLF())
      أرجع make_error<OutputConfigError>(*Config, لار_Storage);
  }

  std::unique_ptr<OutputFileImpl> Impl;
  إذا (Error E = createFileImpl(لار_Storage, Config).moveInto(Impl))
    أرجع std::move(E);
  أكد(Impl && "Expected valid Impl or Error");
  أرجع OutputFile(لار_Storage, std::move(Impl));
}
