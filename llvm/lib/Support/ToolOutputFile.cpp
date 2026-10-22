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

#شامل "llvm/Support/ToolOutputFile.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/Signals.h"
using فضاینام llvm;

ایستا منطقی isStdout(StringRef Filename) { بازگشت Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(نادرست) {
  // Arrange for the file to be deleted if the process is killed.
  اگر (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  اگر (isStdout(Filename))
    بازگشت;

  // Delete the file if the client hasn't told us not to.
  اگر (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  اگر (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    بازگشت;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  اگر (EC)
    Installer.Keep = درست;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, صحیح FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, درست);
  OS = &*OSHolder;
}
