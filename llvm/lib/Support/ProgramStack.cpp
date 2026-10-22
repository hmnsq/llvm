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

uintptr_t llvm::getStackPointer() {
#nếu __GNUC__ || __has_builtin(__builtin_frame_address)
  trả_về (uintptr_t)__builtin_frame_address(0);
#elif defined(_MSC_VER)
  trả_về (uintptr_t)_AddressOfReturnAddress();
#không_thì
  volatile char CharOnStack = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  char *volatile Ptr = &CharOnStack;
  trả_về (uintptr_t)Ptr;
#endif
}

unsigned llvm::getDefaultStackSize() {
#ifdef LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  trả_về RL.rlim_cur;
#không_thì
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  trả_về 8 << 20;
#endif
}

void llvm::runOnNewStack(unsigned StackSize, function_ref<void()> Fn) {
  llvm::thread Thread(
      StackSize == 0 ? std::nullopt : std::optional<unsigned>(StackSize), Fn);
  Thread.join();
}
