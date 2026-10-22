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

#अन्तर्भाव "llvm/Support/ToolOutputFile.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/Signals.h"
using नामाकाश llvm;

स्थिर तार्किक isStdout(StringRef Filename) { परत Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(असत्य) {
  // Arrange for the file to be deleted if the process is killed.
  जर (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  जर (isStdout(Filename))
    परत;

  // Delete the file if the client hasn't told us not to.
  जर (!Keep)
    sys::fs::हटाओ(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  जर (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    परत;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  जर (EC)
    Installer.Keep = सत्य;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, पूर्णांक FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, सत्य);
  OS = &*OSHolder;
}
