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

#અન્તર્ભાવ "llvm/Support/ToolOutputFile.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/Signals.h"
using નામાકાશ llvm;

સ્થિર તાર્કિક isStdout(StringRef Filename) { પાછા Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(ખોટું) {
  // Arrange for the file to be deleted if the process is killed.
  જો (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  જો (isStdout(Filename))
    પાછા;

  // Delete the file if the client hasn't told us not to.
  જો (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  જો (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    પાછા;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  જો (EC)
    Installer.Keep = સાચું;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, પૂર્ણાંક FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, સાચું);
  OS = &*OSHolder;
}
