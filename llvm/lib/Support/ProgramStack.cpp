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

#ifdef LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#endif

#ifdef _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#endif

#ضمن "llvm/Support/thread.h"

#ضمن <cstdlib>

using نطاق llvm;

uintptr_t llvm::get_مكدس_Pointer() {
#إذا __GNUC__ || __has_builtin(__builtin_frame_address)
  إرجاع (uintptr_t)__builtin_frame_address(0);
#وإلاإذا defined(_MSC_VER)
  إرجاع (uintptr_t)_AddressOfReturnAddress();
#وإلا
  متطاير حرف Char_On_مكدس = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  حرف *متطاير Ptr = &Char_On_مكدس;
  إرجاع (uintptr_t)Ptr;
#endif
}

unsigned llvm::get_Default_مكدس_Size() {
#ifdef LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  إرجاع RL.rlim_cur;
#وإلا
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  إرجاع 8 << 20;
#endif
}

فراغ llvm::run_On_New_مكدس(unsigned مكدس_Size, function_ref<فراغ()> Fn) {
  llvm::thread Thread(
      مكدس_Size == 0 ? std::nullopt : std::optional<unsigned>(مكدس_Size), Fn);
  Thread.join();
}
