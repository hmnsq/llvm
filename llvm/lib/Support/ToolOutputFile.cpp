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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ToolOutputFile.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Signals.h"
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਥਿਰ ਤਾਰਕਿਕ isStdout(StringRef Filename) { ਵਾਪਸ Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(ਝੂਠ) {
  // Arrange for the file to be deleted if the process is killed.
  ਜੇ (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  ਜੇ (isStdout(Filename))
    ਵਾਪਸ;

  // Delete the file if the client hasn't told us not to.
  ਜੇ (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_ਕੋਡ &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  ਜੇ (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_ਕੋਡ();
    ਵਾਪਸ;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  ਜੇ (EC)
    Installer.Keep = ਸੱਚ;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, ਪੂਰਨ FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, ਸੱਚ);
  OS = &*OSHolder;
}
