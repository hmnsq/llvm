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

#شامل "llvm/Support/Process.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/CrashRecoveryContext.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/Path.h"

#شامل <optional>
#شامل <stdlib.h> // for _Exit

استعمال نامخلا llvm;
استعمال نامخلا sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::ڈھونڈو_اندر_Env_راستہ(StringRef EnvName, StringRef FileName, حرف Separator) {
  واپس ڈھونڈو_اندر_Env_راستہ(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::ڈھونڈو_اندر_Env_راستہ(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, حرف Separator) {
  أكد(!راستہ_2::is_absolute(FileName));
  std::optional<std::string> ملا_راستہ;
  std::optional<std::string> Opt_راستہ = Process::GetEnv(EnvName);
  اگر (!Opt_راستہ)
    واپس ملا_راستہ;

  ثابت حرف Env_راستہ_Separator_Str[] = {Separator, '\0'};
  Small_سمتیہ<StringRef, 8> Dirs;
  بانٹو_String(*Opt_راستہ, Dirs, Env_راستہ_Separator_Str);

  برائے (StringRef Dir : Dirs) {
    اگر (Dir.خالی_خانے())
      جاری;

    اگر (any_کا(IgnoreList, [&](StringRef S) { واپس fs::equivalent(S, Dir); }))
      جاری;

    SmallString<128> File_راستہ(Dir);
    راستہ_2::append(File_راستہ, FileName);
    اگر (fs::exists(Twine(File_راستہ))) {
      ملا_راستہ = std::string(File_راستہ);
      توڑو;
    }
  }

  واپس ملا_راستہ;
}

// clang-format off
#تعریف_کرو COLOR(FGBG, رمز_3, BOLD) "\033[0;" BOLD FGBG رمز_3 "m"

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
ساکن ثابت حرف colorcodes[2][2][16][11] = {
    { ALLCOLORS("3", "9", ""), ALLCOLORS("3", "9", "1;"),},
    { ALLCOLORS("4", "10", ""), ALLCOLORS("4", "10", "1;")}
};
// clang-format on

// A CMake option controls wheter we emit core dumps by default. An application
// may disable core dumps by calling Process::PreventCoreFiles().
ساکن منطقی coreFilesPrevented = !LLVM_ENABLE_CRASH_DUMPS;

منطقی Process::AreCoreFilesPrevented() { واپس coreFilesPrevented; }

[[noreturn]] خالی Process::Exit(صحیح Ret_رمز, منطقی NoCleanup) {
  اگر (CrashRecoveryContext *چکری_جانچ = CrashRecoveryContext::GetCurrent())
    چکری_جانچ->HandleExit(Ret_رمز);

  اگر (NoCleanup)
    ExitNoCleanup(Ret_رمز);
  ورنہ
    ::باہر_نکلو(Ret_رمز);
}

// Include the platform-specific parts of this class.
#اگر_تعریف_شدہ LLVM_ON_UNIX
#شامل "Unix/Process.inc"
#اگر_ختم
#اگر_تعریف_شدہ _WIN32
#شامل "Windows/Process.inc"
#اگر_ختم
