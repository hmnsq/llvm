//===--- ToolOutputFile.cpp - Implement the ToolOutputFile class --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This implements the ToolOutputFile class.
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/ToolOutputFile.h"
#অন্তর্ভাৱ "llvm/Support/FileSystem.h"
#অন্তর্ভাৱ "llvm/Support/Signals.h"
using নামাকাশ llvm;

স্থির তার্কিক isStdout(StringRef Filename) { ঘূৰাই Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(মিছা) {
  // Arrange for the file to be deleted if the process is killed.
  যদি (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  যদি (isStdout(Filename))
    ঘূৰাই;

  // Delete the file if the client hasn't told us not to.
  যদি (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  যদি (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    ঘূৰাই;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  যদি (EC)
    Installer.Keep = সঁচা;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, পূর্ণাঙ্ক FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, সঁচা);
  OS = &*OSHolder;
}
