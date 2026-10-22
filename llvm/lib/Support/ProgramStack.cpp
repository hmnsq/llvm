//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/ProgramStack.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Support/Compiler.h"

#ifdef LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#endif

#ifdef _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#endif

#அந்தர்பாவ "llvm/Support/thread.h"

#அந்தர்பாவ <cstdlib>

using நாமாகாஶ llvm;

uintptr_t llvm::get_அடுக்கு_Pointer() {
#எனில் __GNUC__ || __has_builtin(__builtin_frame_address)
  திரும்பு (uintptr_t)__builtin_frame_address(0);
#அல்லதெனில் defined(_MSC_VER)
  திரும்பு (uintptr_t)_AddressOfReturnAddress();
#இல்லையெனில்
  நிலையற்ற எழுத்து Char_On_அடுக்கு = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  எழுத்து *நிலையற்ற Ptr = &Char_On_அடுக்கு;
  திரும்பு (uintptr_t)Ptr;
#endif
}

unsigned llvm::get_Default_அடுக்கு_Size() {
#ifdef LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  திரும்பு RL.rlim_cur;
#இல்லையெனில்
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  திரும்பு 8 << 20;
#endif
}

வெறுமை llvm::run_On_New_அடுக்கு(unsigned அடுக்கு_Size, function_ref<வெறுமை()> Fn) {
  llvm::thread Thread(
      அடுக்கு_Size == 0 ? std::nullopt : std::optional<unsigned>(அடுக்கு_Size), Fn);
  Thread.join();
}
