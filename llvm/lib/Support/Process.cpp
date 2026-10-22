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

کارول نطاق llvm;
کارول نطاق sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::پیدا_کړه_دننه_Env_لار(StringRef EnvName, StringRef FileName, حرف Separator) {
  أرجع پیدا_کړه_دننه_Env_لار(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::پیدا_کړه_دننه_Env_لار(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, حرف Separator) {
  أكد(!لار_2::is_absolute(FileName));
  std::optional<std::string> وموندل_لار;
  std::optional<std::string> Opt_لار = Process::GetEnv(EnvName);
  إذا (!Opt_لار)
    أرجع وموندل_لار;

  ثابت حرف Env_لار_Separator_Str[] = {Separator, '\0'};
  Small_ویکټور<StringRef, 8> Dirs;
  وېشل_String(*Opt_لار, Dirs, Env_لار_Separator_Str);

  لكل (StringRef Dir : Dirs) {
    إذا (Dir.تشه())
      استمر;

    إذا (any_د(IgnoreList, [&](StringRef S) { أرجع fs::equivalent(S, Dir); }))
      استمر;

    SmallString<128> File_لار(Dir);
    لار_2::append(File_لار, FileName);
    إذا (fs::exists(Twine(File_لار))) {
      وموندل_لار = std::string(File_لار);
      اكسر;
    }
  }

  أرجع وموندل_لار;
}

// clang-format off
#تعریف_کړه COLOR(FGBG, کوډ_3, BOLD) "\033[0;" BOLD FGBG کوډ_3 "m"

#تعریف_کړه ALLCOLORS(FGBG, BRIGHT, BOLD) \
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

[[noreturn]] فراغ Process::Exit(صحيح Ret_کوډ, منطقي NoCleanup) {
  إذا (CrashRecoveryContext *دوراني_ازموینه = CrashRecoveryContext::GetCurrent())
    دوراني_ازموینه->HandleExit(Ret_کوډ);

  إذا (NoCleanup)
    ExitNoCleanup(Ret_کوډ);
  وإلا
    ::وتل(Ret_کوډ);
}

// Include the platform-specific parts of this class.
#که_تعریف_شوی LLVM_ON_UNIX
#ضمن "Unix/Process.inc"
#د_که_پای
#که_تعریف_شوی _WIN32
#ضمن "Windows/Process.inc"
#د_که_پای
