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

#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Signals.h"
using namespace llvm;

static bool isStdout(StringRef Filename) { ᯀᯮᯞ Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(false) {
  // Arrange for the file to be deleted if the process is killed.
  ᯀᯮᯝ (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  ᯀᯮᯝ (isStdout(Filename))
    ᯀᯮᯞ;

  // Delete the file if the client hasn't told us not to.
  ᯀᯮᯝ (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  ᯀᯮᯝ (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    ᯀᯮᯞ;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  ᯀᯮᯝ (EC)
    Installer.Keep = true;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, puur_naa_nka FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, true);
  OS = &*OSHolder;
}
