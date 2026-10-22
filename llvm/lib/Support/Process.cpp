//===-- Process.cpp - Implement OS Process Concept --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the operating system Process concept.
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Process.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/CrashRecoveryContext.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Path.h"

#ଅନ୍ତର୍ଭାଵ <optional>
#ଅନ୍ତର୍ଭାଵ <stdlib.h> // for _Exit

using ନାମାକାଶ llvm;
using ନାମାକାଶ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::FindInEnvPath(StringRef EnvName, StringRef FileName, ଅକ୍ଷର Separator) {
  ଫେରନ୍ତୁ FindInEnvPath(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::FindInEnvPath(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, ଅକ୍ଷର Separator) {
  ପ୍ରତିଜ୍ଞା(!path::is_absolute(FileName));
  std::optional<std::string> FoundPath;
  std::optional<std::string> OptPath = Process::GetEnv(EnvName);
  ଯଦି (!OptPath)
    ଫେରନ୍ତୁ FoundPath;

  ଅଚଳ ଅକ୍ଷର EnvPathSeparatorStr[] = {Separator, '\0'};
  SmallVector<StringRef, 8> Dirs;
  SplitString(*OptPath, Dirs, EnvPathSeparatorStr);

  ପାଇଁ (StringRef Dir : Dirs) {
    ଯଦି (Dir.empty())
      ଜାରି;

    ଯଦି (any_of(IgnoreList, [&](StringRef S) { ଫେରନ୍ତୁ fs::equivalent(S, Dir); }))
      ଜାରି;

    SmallString<128> FilePath(Dir);
    path::append(FilePath, FileName);
    ଯଦି (fs::exists(Twine(FilePath))) {
      FoundPath = std::string(FilePath);
      ଭାଙ୍ଗ;
    }
  }

  ଫେରନ୍ତୁ FoundPath;
}

// clang-format off
#define COLOR(FGBG, CODE, BOLD) "\033[0;" BOLD FGBG CODE "m"

#define ALLCOLORS(FGBG, BRIGHT, BOLD) \
  {                           \
    COLOR(FGBG, "0", BOLD),   \
    COLOR(FGBG, "1", BOLD),   \
    COLOR(FGBG, "2", BOLD),   \
    COLOR(FGBG, "3", BOLD),   \
    COLOR(FGBG, "4", BOLD),   \
    COLOR(FGBG, "5", BOLD),   \
    COLOR(FGBG, "6", BOLD),   \
    COLOR(FGBG, "7", BOLD),   \
    COLOR(BRIGHT, "0", BOLD), \
    COLOR(BRIGHT, "1", BOLD), \
    COLOR(BRIGHT, "2", BOLD), \
    COLOR(BRIGHT, "3", BOLD), \
    COLOR(BRIGHT, "4", BOLD), \
    COLOR(BRIGHT, "5", BOLD), \
    COLOR(BRIGHT, "6", BOLD), \
    COLOR(BRIGHT, "7", BOLD), \
  }

//                           bg
//                           |  bold
//                           |  |
//                           |  |   codes
//                           |  |   |
//                           |  |   |
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର colorcodes[2][2][16][11] = {
    { ALLCOLORS("3", "9", ""), ALLCOLORS("3", "9", "1;"),},
    { ALLCOLORS("4", "10", ""), ALLCOLORS("4", "10", "1;")}
};
// clang-format on

// A CMake option controls wheter we emit core dumps by default. An application
// may disable core dumps by calling Process::PreventCoreFiles().
ସ୍ଥିର ତାର୍କିକ coreFilesPrevented = !LLVM_ENABLE_CRASH_DUMPS;

ତାର୍କିକ Process::AreCoreFilesPrevented() { ଫେରନ୍ତୁ coreFilesPrevented; }

[[noreturn]] ଶୂନ୍ୟ Process::Exit(ପୂର୍ଣ୍ଣସଂଖ୍ୟା RetCode, ତାର୍କିକ NoCleanup) {
  ଯଦି (CrashRecoveryContext *CRC = CrashRecoveryContext::GetCurrent())
    CRC->HandleExit(RetCode);

  ଯଦି (NoCleanup)
    ExitNoCleanup(RetCode);
  ଅନ୍ୟଥା
    ::exit(RetCode);
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ଅନ୍ତର୍ଭାଵ "Unix/Process.inc"
#endif
#ifdef _WIN32
#ଅନ୍ତର୍ଭାଵ "Windows/Process.inc"
#endif
