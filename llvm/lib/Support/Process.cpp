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

#അന്തര്ഭാവ "llvm/Support/Process.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h"
#അന്തര്ഭാവ "llvm/Support/CrashRecoveryContext.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/Path.h"

#അന്തര്ഭാവ <optional>
#അന്തര്ഭാവ <stdlib.h> // for _Exit

using നാമാകാശ llvm;
using നാമാകാശ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::FindInEnvPath(StringRef EnvName, StringRef FileName, അക്ഷരം Separator) {
  തിരികെ FindInEnvPath(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::FindInEnvPath(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, അക്ഷരം Separator) {
  പ്രതിജ്ഞാ(!path::is_absolute(FileName));
  std::optional<std::string> FoundPath;
  std::optional<std::string> OptPath = Process::GetEnv(EnvName);
  എങ്കിൽ (!OptPath)
    തിരികെ FoundPath;

  സ്ഥിരം അക്ഷരം EnvPathSeparatorStr[] = {Separator, '\0'};
  SmallVector<StringRef, 8> Dirs;
  SplitString(*OptPath, Dirs, EnvPathSeparatorStr);

  വേണ്ടി (StringRef Dir : Dirs) {
    എങ്കിൽ (Dir.empty())
      തുടരുക;

    എങ്കിൽ (any_of(IgnoreList, [&](StringRef S) { തിരികെ fs::equivalent(S, Dir); }))
      തുടരുക;

    SmallString<128> FilePath(Dir);
    path::append(FilePath, FileName);
    എങ്കിൽ (fs::exists(Twine(FilePath))) {
      FoundPath = std::string(FilePath);
      നിർത്തുക;
    }
  }

  തിരികെ FoundPath;
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
സ്ഥിതം സ്ഥിരം അക്ഷരം colorcodes[2][2][16][11] = {
    { ALLCOLORS("3", "9", ""), ALLCOLORS("3", "9", "1;"),},
    { ALLCOLORS("4", "10", ""), ALLCOLORS("4", "10", "1;")}
};
// clang-format on

// A CMake option controls wheter we emit core dumps by default. An application
// may disable core dumps by calling Process::PreventCoreFiles().
സ്ഥിതം തർക്കം coreFilesPrevented = !LLVM_ENABLE_CRASH_DUMPS;

തർക്കം Process::AreCoreFilesPrevented() { തിരികെ coreFilesPrevented; }

[[noreturn]] ശൂന്യം Process::Exit(പൂർണ്ണസംഖ്യ RetCode, തർക്കം NoCleanup) {
  എങ്കിൽ (CrashRecoveryContext *CRC = CrashRecoveryContext::GetCurrent())
    CRC->HandleExit(RetCode);

  എങ്കിൽ (NoCleanup)
    ExitNoCleanup(RetCode);
  അല്ലെങ്കിൽ
    ::exit(RetCode);
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#അന്തര്ഭാവ "Unix/Process.inc"
#endif
#ifdef _WIN32
#അന്തര്ഭാവ "Windows/Process.inc"
#endif
