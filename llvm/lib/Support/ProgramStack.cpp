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

#که_تعریف_شوی LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#د_که_پای

#که_تعریف_شوی _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#د_که_پای

#ضمن "llvm/Support/thread.h"

#ضمن <cstdlib>

کارول نطاق llvm;

uintptr_t llvm::get_ډېری_اشاره_ګر() {
#إذا __GNUC__ || __has_builtin(__builtin_frame_address)
  أرجع (uintptr_t)__builtin_frame_address(0);
#وإلاإذا defined(_MSC_VER)
  أرجع (uintptr_t)پته_د_Return_پته();
#وإلا
  متطاير حرف Char_On_ډېری = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  حرف *متطاير Ptr = &Char_On_ډېری;
  أرجع (uintptr_t)Ptr;
#د_که_پای
}

بینښه llvm::get_Default_ډېری_اندازه() {
#که_تعریف_شوی LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  أرجع RL.rlim_cur;
#وإلا
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  أرجع 8 << 20;
#د_که_پای
}

فراغ llvm::پرلپسې_On_New_ډېری(بینښه ډېری_اندازه, function_ref<فراغ()> Fn) {
  llvm::thread Thread(
      ډېری_اندازه == 0 ? std::nullopt : std::optional<بینښه>(ډېری_اندازه), Fn);
  Thread.join();
}
