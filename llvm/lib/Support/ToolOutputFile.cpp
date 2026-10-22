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

#समावेश "llvm/Support/ToolOutputFile.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/Signals.h"
using namespace llvm;

अटल bool isStdout(StringRef Filename) { वापस Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(false) {
  // Arrange for the file to be deleted if the process is killed.
  अगर (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  अगर (isStdout(Filename))
    वापस;

  // Delete the file if the client hasn't told us not to.
  अगर (!Keep)
    sys::fs::हटाओ(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  अगर (isStdout(Filename)) {
    OS = &म_दो();
    EC = std::error_code();
    वापस;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  अगर (EC)
    Installer.Keep = true;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, पूर्णांक FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, true);
  OS = &*OSHolder;
}
