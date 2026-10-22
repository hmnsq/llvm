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

استعمال نطاق llvm;
استعمال نطاق sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::ڳوليو_اندريون_Env_رستو(StringRef EnvName, StringRef FileName, حرف Separator) {
  أرجع ڳوليو_اندريون_Env_رستو(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::ڳوليو_اندريون_Env_رستو(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, حرف Separator) {
  أكد(!رستو_2::is_absolute(FileName));
  std::optional<std::string> مليو_رستو;
  std::optional<std::string> Opt_رستو = Process::GetEnv(EnvName);
  إذا (!Opt_رستو)
    أرجع مليو_رستو;

  ثابت حرف Env_رستو_Separator_Str[] = {Separator, '\0'};
  Small_ويڪٽر<StringRef, 8> Dirs;
  ورهايو_String(*Opt_رستو, Dirs, Env_رستو_Separator_Str);

  لكل (StringRef Dir : Dirs) {
    إذا (Dir.خالي_جايون())
      استمر;

    إذا (any_جو(IgnoreList, [&](StringRef S) { أرجع fs::equivalent(S, Dir); }))
      استمر;

    SmallString<128> File_رستو(Dir);
    رستو_2::append(File_رستو, FileName);
    إذا (fs::exists(Twine(File_رستو))) {
      مليو_رستو = std::string(File_رستو);
      اكسر;
    }
  }

  أرجع مليو_رستو;
}

// clang-format off
#وصف_ڪريو COLOR(FGBG, ڪوڊ_3, BOLD) "\033[0;" BOLD FGBG ڪوڊ_3 "m"

#وصف_ڪريو ALLCOLORS(FGBG, BRIGHT, BOLD) \
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

[[noreturn]] فراغ Process::Exit(صحيح Ret_ڪوڊ, منطقي NoCleanup) {
  إذا (CrashRecoveryContext *چڪري_جاچ = CrashRecoveryContext::GetCurrent())
    چڪري_جاچ->HandleExit(Ret_ڪوڊ);

  إذا (NoCleanup)
    ExitNoCleanup(Ret_ڪوڊ);
  وإلا
    ::ٻاهر_نڪرو(Ret_ڪوڊ);
}

// Include the platform-specific parts of this class.
#جيڪڏهن_وصف_ٿيل LLVM_ON_UNIX
#ضمن "Unix/Process.inc"
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل _WIN32
#ضمن "Windows/Process.inc"
#جيڪڏهن_پورو
