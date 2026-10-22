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

#ضمن "llvm/Support/Process.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/CrashRecoveryContext.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/Path.h"

#ضمن <optional>
#ضمن <stdlib.h> // for _Exit

using نطاق llvm;
using نطاق sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::FindInEnvPath(StringRef EnvName, StringRef FileName, حرف Separator) {
  أرجع FindInEnvPath(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::FindInEnvPath(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, حرف Separator) {
  أكد(!path::is_absolute(FileName));
  std::optional<std::string> FoundPath;
  std::optional<std::string> OptPath = Process::GetEnv(EnvName);
  𐤀𐤌 (!OptPath)
    أرجع FoundPath;

  ثابت حرف EnvPathSeparatorStr[] = {Separator, '\0'};
  SmallVector<StringRef, 8> Dirs;
  SplitString(*OptPath, Dirs, EnvPathSeparatorStr);

  لكل (StringRef Dir : Dirs) {
    𐤀𐤌 (Dir.empty())
      استمر;

    𐤀𐤌 (any_of(IgnoreList, [&](StringRef S) { أرجع fs::equivalent(S, Dir); }))
      استمر;

    SmallString<128> FilePath(Dir);
    path::append(FilePath, FileName);
    𐤀𐤌 (fs::exists(Twine(FilePath))) {
      FoundPath = std::string(FilePath);
      اكسر;
    }
  }

  أرجع FoundPath;
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
ساكن ثابت حرف colorcodes[2][2][16][11] = {
    { ALLCOLORS("3", "9", ""), ALLCOLORS("3", "9", "1;"),},
    { ALLCOLORS("4", "10", ""), ALLCOLORS("4", "10", "1;")}
};
// clang-format on

// A CMake option controls wheter we emit core dumps by default. An application
// may disable core dumps by calling Process::PreventCoreFiles().
ساكن منطقي coreFilesPrevented = !LLVM_ENABLE_CRASH_DUMPS;

منطقي Process::AreCoreFilesPrevented() { أرجع coreFilesPrevented; }

[[noreturn]] فراغ Process::Exit(صحيح RetCode, منطقي NoCleanup) {
  𐤀𐤌 (CrashRecoveryContext *CRC = CrashRecoveryContext::GetCurrent())
    CRC->HandleExit(RetCode);

  𐤀𐤌 (NoCleanup)
    ExitNoCleanup(RetCode);
  وإلا
    ::exit(RetCode);
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ضمن "Unix/Process.inc"
#endif
#ifdef _WIN32
#ضمن "Windows/Process.inc"
#endif
