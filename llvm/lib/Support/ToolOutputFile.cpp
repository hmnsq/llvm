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

#അന്തര്ഭാവ "llvm/Support/ToolOutputFile.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/Signals.h"
using നാമാകാശ llvm;

സ്ഥിതം തർക്കം isStdout(StringRef Filename) { തിരികെ Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(അസത്യം) {
  // Arrange for the file to be deleted if the process is killed.
  എങ്കിൽ (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  എങ്കിൽ (isStdout(Filename))
    തിരികെ;

  // Delete the file if the client hasn't told us not to.
  എങ്കിൽ (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  എങ്കിൽ (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    തിരികെ;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  എങ്കിൽ (EC)
    Installer.Keep = സത്യം;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, പൂർണ്ണസംഖ്യ FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, സത്യം);
  OS = &*OSHolder;
}
