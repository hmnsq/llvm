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

استفاده نطاق llvm;
استفاده نطاق sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::پیدا_کن_درون_Env_مسیر(StringRef EnvName, StringRef FileName, حرف Separator) {
  أرجع پیدا_کن_درون_Env_مسیر(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::پیدا_کن_درون_Env_مسیر(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, حرف Separator) {
  أكد(!مسیر_2::is_absolute(FileName));
  std::optional<std::string> یافت_مسیر;
  std::optional<std::string> Opt_مسیر = Process::GetEnv(EnvName);
  إذا (!Opt_مسیر)
    أرجع یافت_مسیر;

  ثابت حرف Env_مسیر_Separator_Str[] = {Separator, '\0'};
  Small_وکتور<StringRef, 8> Dirs;
  تقسیم_String(*Opt_مسیر, Dirs, Env_مسیر_Separator_Str);

  لكل (StringRef Dir : Dirs) {
    إذا (Dir.خالی())
      استمر;

    إذا (any_از(IgnoreList, [&](StringRef S) { أرجع fs::equivalent(S, Dir); }))
      استمر;

    SmallString<128> File_مسیر(Dir);
    مسیر_2::append(File_مسیر, FileName);
    إذا (fs::exists(Twine(File_مسیر))) {
      یافت_مسیر = std::string(File_مسیر);
      اكسر;
    }
  }

  أرجع یافت_مسیر;
}

// clang-format off
#تعریف_کن COLOR(FGBG, کد_3, BOLD) "\033[0;" BOLD FGBG کد_3 "m"

#تعریف_کن ALLCOLORS(FGBG, BRIGHT, BOLD) \
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

[[noreturn]] فراغ Process::Exit(صحيح Ret_کد, منطقي NoCleanup) {
  إذا (CrashRecoveryContext *بررسی_چرخهای = CrashRecoveryContext::GetCurrent())
    بررسی_چرخهای->HandleExit(Ret_کد);

  إذا (NoCleanup)
    ExitNoCleanup(Ret_کد);
  وإلا
    ::خروج(Ret_کد);
}

// Include the platform-specific parts of this class.
#اگر_تعریف_شده LLVM_ON_UNIX
#ضمن "Unix/Process.inc"
#پایان_اگر
#اگر_تعریف_شده _WIN32
#ضمن "Windows/Process.inc"
#پایان_اگر
