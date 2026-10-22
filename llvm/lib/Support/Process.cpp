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

#समावेश "llvm/Support/Process.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Config/llvm-config.h"
#समावेश "llvm/Support/CrashRecoveryContext.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/Path.h"

#समावेश <optional>
#समावेश <stdlib.h> // for _Exit

using namespace llvm;
using namespace sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::FindInEnvPath(StringRef EnvName, StringRef FileName, अक्षर Separator) {
  वापस FindInEnvPath(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::FindInEnvPath(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, अक्षर Separator) {
  निश्चित(!path::is_absolute(FileName));
  std::optional<std::string> FoundPath;
  std::optional<std::string> OptPath = Process::GetEnv(EnvName);
  अगर (!OptPath)
    वापस FoundPath;

  स्थिर अक्षर EnvPathSeparatorStr[] = {Separator, '\0'};
  SmallVector<StringRef, 8> Dirs;
  SplitString(*OptPath, Dirs, EnvPathSeparatorStr);

  क्रम (StringRef Dir : Dirs) {
    अगर (Dir.empty())
      जारी;

    अगर (any_of(IgnoreList, [&](StringRef S) { वापस fs::equivalent(S, Dir); }))
      जारी;

    SmallString<128> FilePath(Dir);
    path::append(FilePath, FileName);
    अगर (fs::exists(Twine(FilePath))) {
      FoundPath = std::string(FilePath);
      अवरोध;
    }
  }

  वापस FoundPath;
}

// clang-format off
#घोषणा COLOR(FGBG, CODE, BOLD) "\033[0;" BOLD FGBG CODE "m"

#घोषणा ALLCOLORS(FGBG, BRIGHT, BOLD) \
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
अटल स्थिर अक्षर colorcodes[2][2][16][11] = {
    { ALLCOLORS("3", "9", ""), ALLCOLORS("3", "9", "1;"),},
    { ALLCOLORS("4", "10", ""), ALLCOLORS("4", "10", "1;")}
};
// clang-format on

// A CMake option controls wheter we emit core dumps by default. An application
// may disable core dumps by calling Process::PreventCoreFiles().
अटल bool coreFilesPrevented = !LLVM_ENABLE_CRASH_DUMPS;

bool Process::AreCoreFilesPrevented() { वापस coreFilesPrevented; }

[[noreturn]] व्योम Process::Exit(पूर्णांक RetCode, bool NoCleanup) {
  अगर (CrashRecoveryContext *CRC = CrashRecoveryContext::GetCurrent())
    CRC->HandleExit(RetCode);

  अगर (NoCleanup)
    ExitNoCleanup(RetCode);
  अन्यथा
    ::निकास(RetCode);
}

// Include the platform-specific parts of this class.
#अगर_घोषित LLVM_ON_UNIX
#समावेश "Unix/Process.inc"
#पूर्ण_अगर
#अगर_घोषित _WIN32
#समावेश "Windows/Process.inc"
#पूर्ण_अगर
