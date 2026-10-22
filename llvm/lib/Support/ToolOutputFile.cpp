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

#அந்தர்பாவ "llvm/Support/ToolOutputFile.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/Signals.h"
using நாமாகாஶ llvm;

நிலையான தருக்க isStdout(StringRef Filename) { திரும்பு Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(பொய்) {
  // Arrange for the file to be deleted if the process is killed.
  எனில் (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  எனில் (isStdout(Filename))
    திரும்பு;

  // Delete the file if the client hasn't told us not to.
  எனில் (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  எனில் (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    திரும்பு;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  எனில் (EC)
    Installer.Keep = உண்மை;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, முழு FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, உண்மை);
  OS = &*OSHolder;
}
