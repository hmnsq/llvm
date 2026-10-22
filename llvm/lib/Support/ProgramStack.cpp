//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/ProgramStack.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/Compiler.h"

#ifdef LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#endif

#ifdef _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#endif

#അന്തര്ഭാവ "llvm/Support/thread.h"

#അന്തര്ഭാവ <cstdlib>

using നാമാകാശ llvm;

uintptr_t llvm::get_അട്ടി_Pointer() {
#എങ്കിൽ __GNUC__ || __has_builtin(__builtin_frame_address)
  തിരികെ (uintptr_t)__builtin_frame_address(0);
#അല്ലെങ്കിലെങ്കിൽ defined(_MSC_VER)
  തിരികെ (uintptr_t)_AddressOfReturnAddress();
#അല്ലെങ്കിൽ
  അസ്ഥിര അക്ഷരം Char_On_അട്ടി = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  അക്ഷരം *അസ്ഥിര Ptr = &Char_On_അട്ടി;
  തിരികെ (uintptr_t)Ptr;
#endif
}

unsigned llvm::get_Default_അട്ടി_Size() {
#ifdef LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  തിരികെ RL.rlim_cur;
#അല്ലെങ്കിൽ
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  തിരികെ 8 << 20;
#endif
}

ശൂന്യം llvm::run_On_New_അട്ടി(unsigned അട്ടി_Size, function_ref<ശൂന്യം()> Fn) {
  llvm::thread Thread(
      അട്ടി_Size == 0 ? std::nullopt : std::optional<unsigned>(അട്ടി_Size), Fn);
  Thread.join();
}
