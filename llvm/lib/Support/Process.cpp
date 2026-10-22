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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CrashRecoveryContext.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <optional>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h> // for _Exit

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::ਲੱਭੋ_ਵਿੱਚ_Env_ਰਸਤਾ(StringRef EnvName, StringRef FileName, ਅੱਖਰ Separator) {
  ਵਾਪਸ ਲੱਭੋ_ਵਿੱਚ_Env_ਰਸਤਾ(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::ਲੱਭੋ_ਵਿੱਚ_Env_ਰਸਤਾ(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, ਅੱਖਰ Separator) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਰਸਤਾ_2::is_absolute(FileName));
  std::optional<std::string> ਮਿਲਿਆ_ਰਸਤਾ;
  std::optional<std::string> Opt_ਰਸਤਾ = Process::GetEnv(EnvName);
  ਜੇ (!Opt_ਰਸਤਾ)
    ਵਾਪਸ ਮਿਲਿਆ_ਰਸਤਾ;

  ਅਚਲ ਅੱਖਰ Env_ਰਸਤਾ_Separator_Str[] = {Separator, '\0'};
  Small_ਸਦਿਸ਼<StringRef, 8> Dirs;
  ਵੰਡੋ_String(*Opt_ਰਸਤਾ, Dirs, Env_ਰਸਤਾ_Separator_Str);

  ਲਈ (StringRef Dir : Dirs) {
    ਜੇ (Dir.ਖਾਲੀ_ਥਾਵਾਂ())
      ਜਾਰੀ;

    ਜੇ (any_ਦਾ(IgnoreList, [&](StringRef S) { ਵਾਪਸ fs::equivalent(S, Dir); }))
      ਜਾਰੀ;

    SmallString<128> File_ਰਸਤਾ(Dir);
    ਰਸਤਾ_2::append(File_ਰਸਤਾ, FileName);
    ਜੇ (fs::exists(Twine(File_ਰਸਤਾ))) {
      ਮਿਲਿਆ_ਰਸਤਾ = std::string(File_ਰਸਤਾ);
      ਰੋਕੋ;
    }
  }

  ਵਾਪਸ ਮਿਲਿਆ_ਰਸਤਾ;
}

// clang-format off
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLOR(FGBG, ਕੋਡ_3, BOLD) "\033[0;" BOLD FGBG ਕੋਡ_3 "m"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALLCOLORS(FGBG, BRIGHT, BOLD) \
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
ਸਥਿਰ ਅਚਲ ਅੱਖਰ colorcodes[2][2][16][11] = {
    { ALLCOLORS("3", "9", ""), ALLCOLORS("3", "9", "1;"),},
    { ALLCOLORS("4", "10", ""), ALLCOLORS("4", "10", "1;")}
};
// clang-format on

// A CMake option controls wheter we emit core dumps by default. An application
// may disable core dumps by calling Process::PreventCoreFiles().
ਸਥਿਰ ਤਾਰਕਿਕ coreFilesPrevented = !LLVM_ENABLE_CRASH_DUMPS;

ਤਾਰਕਿਕ Process::AreCoreFilesPrevented() { ਵਾਪਸ coreFilesPrevented; }

[[noreturn]] ਖਾਲੀ Process::Exit(ਪੂਰਨ Ret_ਕੋਡ, ਤਾਰਕਿਕ NoCleanup) {
  ਜੇ (CrashRecoveryContext *ਚੱਕਰੀ_ਜਾਂਚ = CrashRecoveryContext::GetCurrent())
    ਚੱਕਰੀ_ਜਾਂਚ->HandleExit(Ret_ਕੋਡ);

  ਜੇ (NoCleanup)
    ExitNoCleanup(Ret_ਕੋਡ);
  ਨਹੀਂਤਾਂ
    ::ਬਾਹਰ_ਨਿਕਲੋ(Ret_ਕੋਡ);
}

// Include the platform-specific parts of this class.
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ON_UNIX
#ਅਨ੍ਤਰ੍ਭਾਵ "Unix/Process.inc"
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਅਨ੍ਤਰ੍ਭਾਵ "Windows/Process.inc"
#ਜੇ_ਅੰਤ
