//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/ProgramStack.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/Compiler.h"

#నిర్వచితమైతే LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#అయితే_ముగింపు

#నిర్వచితమైతే _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#అయితే_ముగింపు

#అన్తర్భావ "llvm/Support/thread.h"

#అన్తర్భావ <cstdlib>

ఉపయోగించి నామాకాశ llvm;

uintptr_t llvm::get_దొంతర_సూచిక() {
#ఒకవేళ __GNUC__ || __has_builtin(__builtin_frame_address)
  తిరిగి (uintptr_t)__builtin_frame_address(0);
#లేదావేళ defined(_MSC_VER)
  తిరిగి (uintptr_t)చిరునామా_యొక్క_Return_చిరునామా();
#లేకపోతే
  అస్థిర అక్షరం Char_On_దొంతర = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  అక్షరం *అస్థిర Ptr = &Char_On_దొంతర;
  తిరిగి (uintptr_t)Ptr;
#అయితే_ముగింపు
}

గుర్తులేని llvm::get_Default_దొంతర_కొలత() {
#నిర్వచితమైతే LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  తిరిగి RL.rlim_cur;
#లేకపోతే
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  తిరిగి 8 << 20;
#అయితే_ముగింపు
}

శూన్య llvm::నిరంతరం_On_New_దొంతర(గుర్తులేని దొంతర_కొలత, function_ref<శూన్య()> Fn) {
  llvm::thread Thread(
      దొంతర_కొలత == 0 ? std::nullopt : std::optional<గుర్తులేని>(దొంతర_కొలత), Fn);
  Thread.join();
}
