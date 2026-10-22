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

#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"
#अन्तर्भाव "llvm/Support/CrashRecoveryContext.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/Path.h"

#अन्तर्भाव <optional>
#अन्तर्भाव <stdlib.h> // for _Exit

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::Find_अन्तर्_Env_मार्गः(StringRef EnvName, StringRef FileName, वर्ण Separator) {
  प्रत्यागम Find_अन्तर्_Env_मार्गः(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::Find_अन्तर्_Env_मार्गः(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, वर्ण Separator) {
  प्रतिज्ञा(!मार्गः_2::is_absolute(FileName));
  std::optional<std::string> प्राप्तम्_मार्गः;
  std::optional<std::string> Opt_मार्गः = Process::GetEnv(EnvName);
  यदि (!Opt_मार्गः)
    प्रत्यागम प्राप्तम्_मार्गः;

  ध्रुव वर्ण Env_मार्गः_Separator_Str[] = {Separator, '\0'};
  Small_सदिशम्<StringRef, 8> Dirs;
  विभज_String(*Opt_मार्गः, Dirs, Env_मार्गः_Separator_Str);

  प्रति (StringRef Dir : Dirs) {
    यदि (Dir.रिक्तानि())
      अनुवर्त;

    यदि (any_सम्बन्धि(IgnoreList, [&](StringRef S) { प्रत्यागम fs::equivalent(S, Dir); }))
      अनुवर्त;

    SmallString<128> File_मार्गः(Dir);
    मार्गः_2::append(File_मार्गः, FileName);
    यदि (fs::exists(Twine(File_मार्गः))) {
      प्राप्तम्_मार्गः = std::string(File_मार्गः);
      भङ्ग;
    }
  }

  प्रत्यागम प्राप्तम्_मार्गः;
}

// clang-format off
#परिभाषय COLOR(FGBG, संहिता_3, BOLD) "\033[0;" BOLD FGBG संहिता_3 "m"

#परिभाषय ALLCOLORS(FGBG, BRIGHT, BOLD) \
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
स्थिर ध्रुव वर्ण colorcodes[2][2][16][11] = {
    { ALLCOLORS("3", "9", ""), ALLCOLORS("3", "9", "1;"),},
    { ALLCOLORS("4", "10", ""), ALLCOLORS("4", "10", "1;")}
};
// clang-format on

// A CMake option controls wheter we emit core dumps by default. An application
// may disable core dumps by calling Process::PreventCoreFiles().
स्थिर तार्किक coreFilesPrevented = !LLVM_ENABLE_CRASH_DUMPS;

तार्किक Process::AreCoreFilesPrevented() { प्रत्यागम coreFilesPrevented; }

[[noreturn]] शून्य Process::Exit(पूर्णाङ्क Ret_संहिता, तार्किक NoCleanup) {
  यदि (CrashRecoveryContext *CRC = CrashRecoveryContext::GetCurrent())
    CRC->HandleExit(Ret_संहिता);

  यदि (NoCleanup)
    ExitNoCleanup(Ret_संहिता);
  अन्यथा
    ::निर्गच्छ(Ret_संहिता);
}

// Include the platform-specific parts of this class.
#यदि_परिभाषितम् LLVM_ON_UNIX
#अन्तर्भाव "Unix/Process.inc"
#यदि_अन्तः
#यदि_परिभाषितम् _WIN32
#अन्तर्भाव "Windows/Process.inc"
#यदि_अन्तः
