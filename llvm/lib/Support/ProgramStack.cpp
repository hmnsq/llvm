//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ProgramStack.h"
#include "llvm/Config/config.h"
#include "llvm/Support/Compiler.h"

#ifdef LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#endif

#ifdef _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#endif

#include "llvm/Support/thread.h"

#include <cstdlib>

using namespace llvm;

uintptr_t llvm::get_सालानसान_Pointer() {
#if __GNUC__ || __has_builtin(__builtin_frame_address)
  return (uintptr_t)__builtin_frame_address(0);
#elif defined(_MSC_VER)
  return (uintptr_t)_AddressOfReturnAddress();
#else
  volatile char Char_On_सालानसान = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  char *volatile Ptr = &Char_On_सालानसान;
  return (uintptr_t)Ptr;
#endif
}

unsigned llvm::get_Default_सालानसान_Size() {
#ifdef LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  return RL.rlim_cur;
#else
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  return 8 << 20;
#endif
}

void llvm::run_On_New_सालानसान(unsigned सालानसान_Size, function_ref<void()> Fn) {
  llvm::thread Thread(
      सालानसान_Size == 0 ? std::nullopt : std::optional<unsigned>(सालानसान_Size), Fn);
  Thread.join();
}
