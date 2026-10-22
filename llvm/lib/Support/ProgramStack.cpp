//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/ProgramStack.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"

#ifdef LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#endif

#ifdef _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#endif

#ಅನ್ತರ್ಭಾವ "llvm/Support/thread.h"

#ಅನ್ತರ್ಭಾವ <cstdlib>

using ನಾಮಾಕಾಶ llvm;

uintptr_t llvm::get_ರಾಶಿ_Pointer() {
#ಯದಿ __GNUC__ || __has_builtin(__builtin_frame_address)
  ಪ್ರತ್ಯಾಗಮ (uintptr_t)__builtin_frame_address(0);
#ನೋಚೇದ್ಯದಿ defined(_MSC_VER)
  ಪ್ರತ್ಯಾಗಮ (uintptr_t)_AddressOfReturnAddress();
#ಅನ್ಯಥಾ
  ಚಞ್ಚಲ ವರ್ಣ Char_On_ರಾಶಿ = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  ವರ್ಣ *ಚಞ್ಚಲ Ptr = &Char_On_ರಾಶಿ;
  ಪ್ರತ್ಯಾಗಮ (uintptr_t)Ptr;
#endif
}

unsigned llvm::get_Default_ರಾಶಿ_Size() {
#ifdef LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  ಪ್ರತ್ಯಾಗಮ RL.rlim_cur;
#ಅನ್ಯಥಾ
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  ಪ್ರತ್ಯಾಗಮ 8 << 20;
#endif
}

ಶೂನ್ಯ llvm::run_On_New_ರಾಶಿ(unsigned ರಾಶಿ_Size, function_ref<ಶೂನ್ಯ()> Fn) {
  llvm::thread Thread(
      ರಾಶಿ_Size == 0 ? std::nullopt : std::optional<unsigned>(ರಾಶಿ_Size), Fn);
  Thread.join();
}
