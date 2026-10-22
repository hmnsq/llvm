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

#అన్తర్భావ "llvm/Support/ToolOutputFile.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/Signals.h"
ఉపయోగించి నామాకాశ llvm;

స్థిర తార్కిక isStdout(StringRef Filename) { తిరిగి Filename == "-"; }

CleanupInstaller::CleanupInstaller(StringRef Filename)
    : Filename(std::string(Filename)), Keep(అబద్ధం) {
  // Arrange for the file to be deleted if the process is killed.
  ఒకవేళ (!isStdout(Filename))
    sys::RemoveFileOnSignal(Filename);
}

CleanupInstaller::~CleanupInstaller() {
  ఒకవేళ (isStdout(Filename))
    తిరిగి;

  // Delete the file if the client hasn't told us not to.
  ఒకవేళ (!Keep)
    sys::fs::remove(Filename);

  // Ok, the file is successfully written and closed, or deleted. There's no
  // further need to clean it up on signals.
  sys::DontRemoveFileOnSignal(Filename);
}

ToolOutputFile::ToolOutputFile(StringRef Filename, std::error_సంకేతం &EC,
                               sys::fs::OpenFlags Flags)
    : Installer(Filename) {
  ఒకవేళ (isStdout(Filename)) {
    OS = &outs();
    EC = std::error_సంకేతం();
    తిరిగి;
  }
  OSHolder.emplace(Filename, EC, Flags);
  OS = &*OSHolder;
  // If open fails, no cleanup is needed.
  ఒకవేళ (EC)
    Installer.Keep = నిజం;
}

ToolOutputFile::ToolOutputFile(StringRef Filename, పూర్ణాంకం FD)
    : Installer(Filename) {
  OSHolder.emplace(FD, నిజం);
  OS = &*OSHolder;
}
