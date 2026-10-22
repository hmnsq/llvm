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

ورتوں نطاق llvm;
ورتوں نطاق sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::لبھو_وچ_Env_رستہ(StringRef EnvName, StringRef FileName, حرف Separator) {
  أرجع لبھو_وچ_Env_رستہ(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::لبھو_وچ_Env_رستہ(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, حرف Separator) {
  أكد(!رستہ_2::is_absolute(FileName));
  std::optional<std::string> لبھیا_رستہ;
  std::optional<std::string> Opt_رستہ = Process::GetEnv(EnvName);
  إذا (!Opt_رستہ)
    أرجع لبھیا_رستہ;

  ثابت حرف Env_رستہ_Separator_Str[] = {Separator, '\0'};
  Small_سمتیہ<StringRef, 8> Dirs;
  ونڈو_String(*Opt_رستہ, Dirs, Env_رستہ_Separator_Str);

  لكل (StringRef Dir : Dirs) {
    إذا (Dir.خالی_تھاواں())
      استمر;

    إذا (any_دا(IgnoreList, [&](StringRef S) { أرجع fs::equivalent(S, Dir); }))
      استمر;

    SmallString<128> File_رستہ(Dir);
    رستہ_2::append(File_رستہ, FileName);
    إذا (fs::exists(Twine(File_رستہ))) {
      لبھیا_رستہ = std::string(File_رستہ);
      اكسر;
    }
  }

  أرجع لبھیا_رستہ;
}

// clang-format off
#تعریف_کرو COLOR(FGBG, کوڈ_3, BOLD) "\033[0;" BOLD FGBG کوڈ_3 "m"

#تعریف_کرو ALLCOLORS(FGBG, BRIGHT, BOLD) \
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

[[noreturn]] فراغ Process::Exit(صحيح Ret_کوڈ, منطقي NoCleanup) {
  إذا (CrashRecoveryContext *چکری_جانچ = CrashRecoveryContext::GetCurrent())
    چکری_جانچ->HandleExit(Ret_کوڈ);

  إذا (NoCleanup)
    ExitNoCleanup(Ret_کوڈ);
  وإلا
    ::باہر_نکلو(Ret_کوڈ);
}

// Include the platform-specific parts of this class.
#جے_تعریف_شدہ LLVM_ON_UNIX
#ضمن "Unix/Process.inc"
#جے_انت
#جے_تعریف_شدہ _WIN32
#ضمن "Windows/Process.inc"
#جے_انت
