//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ProgramStack.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/thread.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <cstdlib>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

uintptr_t llvm::get_ਢੇਰ_ਸੂਚਕ() {
#ਜੇ __GNUC__ || __has_builtin(__builtin_frame_address)
  ਵਾਪਸ (uintptr_t)__builtin_frame_address(0);
#ਜਾਂਜੇ defined(_MSC_VER)
  ਵਾਪਸ (uintptr_t)ਪਤਾ_ਦਾ_Return_ਪਤਾ();
#ਨਹੀਂਤਾਂ
  ਅਸਥਿਰ ਅੱਖਰ Char_On_ਢੇਰ = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  ਅੱਖਰ *ਅਸਥਿਰ Ptr = &Char_On_ਢੇਰ;
  ਵਾਪਸ (uintptr_t)Ptr;
#ਜੇ_ਅੰਤ
}

ਬਿਨਾਨਿਸ਼ਾਨੀ llvm::get_Default_ਢੇਰ_ਮਾਪ() {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  ਵਾਪਸ RL.rlim_cur;
#ਨਹੀਂਤਾਂ
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  ਵਾਪਸ 8 << 20;
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ llvm::ਲੜੀਵਾਰ_On_New_ਢੇਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਢੇਰ_ਮਾਪ, function_ref<ਖਾਲੀ()> Fn) {
  llvm::thread Thread(
      ਢੇਰ_ਮਾਪ == 0 ? std::nullopt : std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ>(ਢੇਰ_ਮਾਪ), Fn);
  Thread.join();
}
