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

#ಅನ್ತರ್ಭಾವ "llvm/Support/ToolOutputFile.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"
using ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ತಾರ್ಕಿಕ isStdout(StringRef Filename) { ಪ್ರತ್ಯಾಗಮ Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(ಅಸತ್ಯ) {
  // Arrange for the file to be deleted if the process is killed.
  ಯದಿ (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  ಯದಿ (isStdout(Filename))
    ಪ್ರತ್ಯಾಗಮ;

  // Delete the file if the client hasn't told us not to.
  ಯದಿ (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  ಯದಿ (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    ಪ್ರತ್ಯಾಗಮ;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  ಯದಿ (EC)
    Installer.Keep = ಸತ್ಯ;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, ಪೂರ್ಣಾಙ್ಕ FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, ಸತ್ಯ);
  OS = &*OSHolder;
}
