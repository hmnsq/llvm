//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/ProgramStack.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"

#ifdef LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#endif

#ifdef _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#endif

#ଅନ୍ତର୍ଭାଵ "llvm/Support/thread.h"

#ଅନ୍ତର୍ଭାଵ <cstdlib>

using ନାମାକାଶ llvm;

uintptr_t llvm::get_ଗଦା_Pointer() {
#ଯଦି __GNUC__ || __has_builtin(__builtin_frame_address)
  ଫେରନ୍ତୁ (uintptr_t)__builtin_frame_address(0);
#ନଚେତଯଦି defined(_MSC_VER)
  ଫେରନ୍ତୁ (uintptr_t)_AddressOfReturnAddress();
#ଅନ୍ୟଥା
  ଅସ୍ଥିର ଅକ୍ଷର Char_On_ଗଦା = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  ଅକ୍ଷର *ଅସ୍ଥିର Ptr = &Char_On_ଗଦା;
  ଫେରନ୍ତୁ (uintptr_t)Ptr;
#endif
}

unsigned llvm::get_Default_ଗଦା_Size() {
#ifdef LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  ଫେରନ୍ତୁ RL.rlim_cur;
#ଅନ୍ୟଥା
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  ଫେରନ୍ତୁ 8 << 20;
#endif
}

ଶୂନ୍ୟ llvm::run_On_New_ଗଦା(unsigned ଗଦା_Size, function_ref<ଶୂନ୍ୟ()> Fn) {
  llvm::thread Thread(
      ଗଦା_Size == 0 ? std::nullopt : std::optional<unsigned>(ଗଦା_Size), Fn);
  Thread.join();
}
