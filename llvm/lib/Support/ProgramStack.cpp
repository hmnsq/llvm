//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/ProgramStack.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/Compiler.h"

#अगर_घोषित LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#पूर्ण_अगर

#अगर_घोषित _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#पूर्ण_अगर

#समावेश "llvm/Support/thread.h"

#समावेश <cstdlib>

using namespace llvm;

uintptr_t llvm::get_ढेर_Pointer() {
#अगर __GNUC__ || __has_builtin(__builtin_frame_address)
  वापस (uintptr_t)__builtin_frame_address(0);
#या_अगर defined(_MSC_VER)
  वापस (uintptr_t)_AddressOfReturnAddress();
#अन्यथा
  अस्थिर अक्षर Char_On_ढेर = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  अक्षर *अस्थिर Ptr = &Char_On_ढेर;
  वापस (uintptr_t)Ptr;
#पूर्ण_अगर
}

अचिन्हित llvm::get_Default_ढेर_आकार() {
#अगर_घोषित LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  वापस RL.rlim_cur;
#अन्यथा
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  वापस 8 << 20;
#पूर्ण_अगर
}

व्योम llvm::run_On_New_ढेर(अचिन्हित ढेर_आकार, function_ref<व्योम()> Fn) {
  llvm::thread Thread(
      ढेर_आकार == 0 ? std::nullopt : std::optional<अचिन्हित>(ढेर_आकार), Fn);
  Thread.join();
}
