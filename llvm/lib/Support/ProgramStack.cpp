//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/ProgramStack.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/Compiler.h"

#ifdef LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#endif

#ifdef _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#endif

#અન્તર્ભાવ "llvm/Support/thread.h"

#અન્તર્ભાવ <cstdlib>

using નામાકાશ llvm;

uintptr_t llvm::get_ઢગલો_Pointer() {
#જો __GNUC__ || __has_builtin(__builtin_frame_address)
  પાછા (uintptr_t)__builtin_frame_address(0);
#અથવાજો defined(_MSC_VER)
  પાછા (uintptr_t)_AddressOfReturnAddress();
#નહીંતર
  અસ્થિર અક્ષર Char_On_ઢગલો = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  અક્ષર *અસ્થિર Ptr = &Char_On_ઢગલો;
  પાછા (uintptr_t)Ptr;
#endif
}

unsigned llvm::get_Default_ઢગલો_Size() {
#ifdef LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  પાછા RL.rlim_cur;
#નહીંતર
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  પાછા 8 << 20;
#endif
}

શૂન્ય llvm::run_On_New_ઢગલો(unsigned ઢગલો_Size, function_ref<શૂન્ય()> Fn) {
  llvm::thread Thread(
      ઢગલો_Size == 0 ? std::nullopt : std::optional<unsigned>(ઢગલો_Size), Fn);
  Thread.join();
}
