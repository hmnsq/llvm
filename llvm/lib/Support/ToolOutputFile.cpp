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

static bool isStdout(StringRef Filename) { 반환 Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(false) {
  // Arrange for the file to be deleted if the process is killed.
  만약 (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  만약 (isStdout(Filename))
    반환;

  // Delete the file if the client hasn't told us not to.
  만약 (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  만약 (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    반환;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  만약 (EC)
    Installer.Keep = true;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, 정수 FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, true);
  OS = &*OSHolder;
}
