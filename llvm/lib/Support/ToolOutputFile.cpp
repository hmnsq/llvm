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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/ToolOutputFile.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Signals.h"
using ନାମାକାଶ llvm;

ସ୍ଥିର ତାର୍କିକ isStdout(StringRef Filename) { ଫେରନ୍ତୁ Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(ମିଥ୍ୟା) {
  // Arrange for the file to be deleted if the process is killed.
  ଯଦି (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  ଯଦି (isStdout(Filename))
    ଫେରନ୍ତୁ;

  // Delete the file if the client hasn't told us not to.
  ଯଦି (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  ଯଦି (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_code();
    ଫେରନ୍ତୁ;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  ଯଦି (EC)
    Installer.Keep = ସତ୍ୟ;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, ସତ୍ୟ);
  OS = &*OSHolder;
}
