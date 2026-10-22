//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/ProgramStack.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/Compiler.h"

#ifdef LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#endif

#ifdef _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#endif

#अन्तर्भाव "llvm/Support/thread.h"

#अन्तर्भाव <cstdlib>

using नामाकाश llvm;

uintptr_t llvm::get_राशिः_Pointer() {
#सचे __GNUC__ || __has_builtin(__builtin_frame_address)
  पटिनिवत्त (uintptr_t)__builtin_frame_address(0);
#नोचेद्यदि defined(_MSC_VER)
  पटिनिवत्त (uintptr_t)_AddressOfReturnAddress();
#अञ्ञथा
  चञ्चल वर्ण Char_On_राशिः = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  वर्ण *चञ्चल Ptr = &Char_On_राशिः;
  पटिनिवत्त (uintptr_t)Ptr;
#endif
}

unsigned llvm::get_Default_राशिः_Size() {
#ifdef LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  पटिनिवत्त RL.rlim_cur;
#अञ्ञथा
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  पटिनिवत्त 8 << 20;
#endif
}

शून्य llvm::run_On_New_राशिः(unsigned राशिः_Size, function_ref<शून्य()> Fn) {
  llvm::thread Thread(
      राशिः_Size == 0 ? std::nullopt : std::optional<unsigned>(राशिः_Size), Fn);
  Thread.join();
}
