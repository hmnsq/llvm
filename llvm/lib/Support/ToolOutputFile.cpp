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
using نطاق llvm;

ساكن منطقي isStdout(StringRef Filename) { أرجع Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(خطأ) {
  // Arrange for the file to be deleted if the process is killed.
  𐤀𐤌 (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  𐤀𐤌 (isStdout(Filename))
    أرجع;

  // Delete the file if the client hasn't told us not to.
  𐤀𐤌 (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  𐤀𐤌 (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    أرجع;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  𐤀𐤌 (EC)
    Installer.Keep = true;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, صحيح FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, true);
  OS = &*OSHolder;
}
