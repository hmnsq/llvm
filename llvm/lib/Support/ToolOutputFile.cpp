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

#অন্তর্ভাব "llvm/Support/ToolOutputFile.h"
#অন্তর্ভাব "llvm/Support/FileSystem.h"
#অন্তর্ভাব "llvm/Support/Signals.h"
using নামাকাশ llvm;

স্থির তার্কিক isStdout(StringRef Filename) { প্রত্যাগম Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(অসত্য) {
  // Arrange for the file to be deleted if the process is killed.
  যদি (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  যদি (isStdout(Filename))
    প্রত্যাগম;

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
    প্রত্যাগম;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  যদি (EC)
    Installer.Keep = সত্য;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, পূর্ণাঙ্ক FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, সত্য);
  OS = &*OSHolder;
}
