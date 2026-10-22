//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/ProgramStack.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/Compiler.h"

#اگر_تعریف_شدہ LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#اگر_ختم

#اگر_تعریف_شدہ _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#اگر_ختم

#شامل "llvm/Support/thread.h"

#شامل <cstdlib>

استعمال نامخلا llvm;

uintptr_t llvm::get_ڈھیر_اشارہ_گر() {
#اگر __GNUC__ || __has_builtin(__builtin_frame_address)
  واپس (uintptr_t)__builtin_frame_address(0);
#وگرنہ_اگر defined(_MSC_VER)
  واپس (uintptr_t)پتہ_کا_Return_پتہ();
#ورنہ
  متلون حرف Char_On_ڈھیر = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  حرف *متلون Ptr = &Char_On_ڈھیر;
  واپس (uintptr_t)Ptr;
#اگر_ختم
}

غیرعلامتی llvm::get_Default_ڈھیر_سائز() {
#اگر_تعریف_شدہ LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  واپس RL.rlim_cur;
#ورنہ
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  واپس 8 << 20;
#اگر_ختم
}

خالی llvm::تسلسل_On_New_ڈھیر(غیرعلامتی ڈھیر_سائز, function_ref<خالی()> Fn) {
  llvm::thread Thread(
      ڈھیر_سائز == 0 ? std::nullopt : std::optional<غیرعلامتی>(ڈھیر_سائز), Fn);
  Thread.join();
}
