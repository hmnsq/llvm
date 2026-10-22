//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ProgramStack.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Compiler.h"

#اگر_تعریف_شده LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#پایان_اگر

#اگر_تعریف_شده _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#پایان_اگر

#ضمن "llvm/Support/thread.h"

#ضمن <cstdlib>

استفاده نطاق llvm;

uintptr_t llvm::get_پشته_اشاره_گر() {
#إذا __GNUC__ || __has_builtin(__builtin_frame_address)
  أرجع (uintptr_t)__builtin_frame_address(0);
#وإلاإذا defined(_MSC_VER)
  أرجع (uintptr_t)آدرس_از_Return_آدرس();
#وإلا
  متطاير حرف Char_On_پشته = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  حرف *متطاير Ptr = &Char_On_پشته;
  أرجع (uintptr_t)Ptr;
#پایان_اگر
}

بیعلامت llvm::get_Default_پشته_اندازه() {
#اگر_تعریف_شده LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  أرجع RL.rlim_cur;
#وإلا
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  أرجع 8 << 20;
#پایان_اگر
}

فراغ llvm::پیاپی_On_New_پشته(بیعلامت پشته_اندازه, function_ref<فراغ()> Fn) {
  llvm::thread Thread(
      پشته_اندازه == 0 ? std::nullopt : std::optional<بیعلامت>(پشته_اندازه), Fn);
  Thread.join();
}
