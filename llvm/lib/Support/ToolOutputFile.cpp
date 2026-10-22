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
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ತಾರ್ಕಿಕ isStdout(StringRef Filename) { ಹಿಂತಿರುಗಿ Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(ಸುಳ್ಳು) {
  // Arrange for the file to be deleted if the process is killed.
  ಆದರೆ (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  ಆದರೆ (isStdout(Filename))
    ಹಿಂತಿರುಗಿ;

  // Delete the file if the client hasn't told us not to.
  ಆದರೆ (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_ಸಂಕೇತ &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  ಆದರೆ (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_ಸಂಕೇತ();
    ಹಿಂತಿರುಗಿ;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  ಆದರೆ (EC)
    Installer.Keep = ನಿಜ;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, ಪೂರ್ಣಾಂಕ FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, ನಿಜ);
  OS = &*OSHolder;
}
