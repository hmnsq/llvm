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

#ضمن "llvm/Support/ToolOutputFile.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/Signals.h"
کارول نطاق llvm;

ساكن منطقي isStdout(StringRef Filename) { أرجع Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(خطأ) {
  // Arrange for the file to be deleted if the process is killed.
  إذا (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  إذا (isStdout(Filename))
    أرجع;

  // Delete the file if the client hasn't told us not to.
  إذا (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_کوډ &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  إذا (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_کوډ();
    أرجع;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  إذا (EC)
    Installer.Keep = ریښتیا;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, صحيح FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, ریښتیا);
  OS = &*OSHolder;
}
