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

स्थिर तार्किक isStdout(StringRef Filename) { प्रत्यागम Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(मिच्छ) {
  // Arrange for the file to be deleted if the process is killed.
  जइ (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  जइ (isStdout(Filename))
    प्रत्यागम;

  // Delete the file if the client hasn't told us not to.
  जइ (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  जइ (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    प्रत्यागम;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  जइ (EC)
    Installer.Keep = सच्च;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, पूर्णाङ्क FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, सच्च);
  OS = &*OSHolder;
}
