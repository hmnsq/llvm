//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/ProgramStack.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Support/Compiler.h"

#ifdef LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#endif

#ifdef _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#endif

#অন্তর্ভাব "llvm/Support/thread.h"

#অন্তর্ভাব <cstdlib>

using নামাকাশ llvm;

uintptr_t llvm::get_স্তূপ_Pointer() {
#যদি __GNUC__ || __has_builtin(__builtin_frame_address)
  প্রত্যাগম (uintptr_t)__builtin_frame_address(0);
#নোচেদ্যদি defined(_MSC_VER)
  প্রত্যাগম (uintptr_t)_AddressOfReturnAddress();
#অন্যথা
  চঞ্চল বর্ণ Char_On_স্তূপ = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  বর্ণ *চঞ্চল Ptr = &Char_On_স্তূপ;
  প্রত্যাগম (uintptr_t)Ptr;
#endif
}

unsigned llvm::get_Default_স্তূপ_Size() {
#ifdef LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  প্রত্যাগম RL.rlim_cur;
#অন্যথা
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  প্রত্যাগম 8 << 20;
#endif
}

শূন্য llvm::run_On_New_স্তূপ(unsigned স্তূপ_Size, function_ref<শূন্য()> Fn) {
  llvm::thread Thread(
      স্তূপ_Size == 0 ? std::nullopt : std::optional<unsigned>(স্তূপ_Size), Fn);
  Thread.join();
}
