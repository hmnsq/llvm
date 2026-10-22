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
  𐡄𐡍 (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  𐡄𐡍 (isStdout(Filename))
    أرجع;

  // Delete the file if the client hasn't told us not to.
  𐡄𐡍 (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  𐡄𐡍 (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    أرجع;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  𐡄𐡍 (EC)
    Installer.Keep = 𐡒𐡔𐡈;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, صحيح FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, 𐡒𐡔𐡈);
  OS = &*OSHolder;
}
