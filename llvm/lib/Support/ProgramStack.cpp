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

#यदि_परिभाषितम् LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#यदि_अन्तः

#यदि_परिभाषितम् _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#यदि_अन्तः

#अन्तर्भाव "llvm/Support/thread.h"

#अन्तर्भाव <cstdlib>

उपयुज्य नामाकाश llvm;

uintptr_t llvm::get_राशिः_Pointer() {
#यदि __GNUC__ || __has_builtin(__builtin_frame_address)
  प्रत्यागम (uintptr_t)__builtin_frame_address(0);
#नोचेद्यदि defined(_MSC_VER)
  प्रत्यागम (uintptr_t)स्थानाङ्कः_सम्बन्धि_Return_स्थानाङ्कः();
#अन्यथा
  चञ्चल वर्ण Char_On_राशिः = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  वर्ण *चञ्चल Ptr = &Char_On_राशिः;
  प्रत्यागम (uintptr_t)Ptr;
#यदि_अन्तः
}

अचिह्नित llvm::get_Default_राशिः_आकार() {
#यदि_परिभाषितम् LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  प्रत्यागम RL.rlim_cur;
#अन्यथा
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  प्रत्यागम 8 << 20;
#यदि_अन्तः
}

शून्य llvm::सततम्_On_New_राशिः(अचिह्नित राशिः_आकार, function_ref<शून्य()> Fn) {
  llvm::thread Thread(
      राशिः_आकार == 0 ? std::nullopt : std::optional<अचिह्नित>(राशिः_आकार), Fn);
  Thread.join();
}
