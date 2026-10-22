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

#அந்தர்பாவ "llvm/Support/Process.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"
#அந்தர்பாவ "llvm/Support/CrashRecoveryContext.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/Path.h"

#அந்தர்பாவ <optional>
#அந்தர்பாவ <stdlib.h> // for _Exit

using நாமாகாஶ llvm;
using நாமாகாஶ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::FindInEnvPath(StringRef EnvName, StringRef FileName, எழுத்து Separator) {
  திரும்பு FindInEnvPath(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::FindInEnvPath(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, எழுத்து Separator) {
  ப்ரதிஜ்ஞா(!path::is_absolute(FileName));
  std::optional<std::string> FoundPath;
  std::optional<std::string> OptPath = Process::GetEnv(EnvName);
  எனில் (!OptPath)
    திரும்பு FoundPath;

  மாறிலி எழுத்து EnvPathSeparatorStr[] = {Separator, '\0'};
  SmallVector<StringRef, 8> Dirs;
  SplitString(*OptPath, Dirs, EnvPathSeparatorStr);

  க்கு (StringRef Dir : Dirs) {
    எனில் (Dir.empty())
      தொடர்;

    எனில் (any_of(IgnoreList, [&](StringRef S) { திரும்பு fs::equivalent(S, Dir); }))
      தொடர்;

    SmallString<128> FilePath(Dir);
    path::append(FilePath, FileName);
    எனில் (fs::exists(Twine(FilePath))) {
      FoundPath = std::string(FilePath);
      நிறுத்து;
    }
  }

  திரும்பு FoundPath;
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
நிலையான மாறிலி எழுத்து colorcodes[2][2][16][11] = {
    { ALLCOLORS("3", "9", ""), ALLCOLORS("3", "9", "1;"),},
    { ALLCOLORS("4", "10", ""), ALLCOLORS("4", "10", "1;")}
};
// clang-format on

// A CMake option controls wheter we emit core dumps by default. An application
// may disable core dumps by calling Process::PreventCoreFiles().
நிலையான தருக்க coreFilesPrevented = !LLVM_ENABLE_CRASH_DUMPS;

தருக்க Process::AreCoreFilesPrevented() { திரும்பு coreFilesPrevented; }

[[noreturn]] வெறுமை Process::Exit(முழு RetCode, தருக்க NoCleanup) {
  எனில் (CrashRecoveryContext *CRC = CrashRecoveryContext::GetCurrent())
    CRC->HandleExit(RetCode);

  எனில் (NoCleanup)
    ExitNoCleanup(RetCode);
  இல்லையெனில்
    ::exit(RetCode);
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#அந்தர்பாவ "Unix/Process.inc"
#endif
#ifdef _WIN32
#அந்தர்பாவ "Windows/Process.inc"
#endif
