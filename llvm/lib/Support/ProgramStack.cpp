//===--- RunOnNewStack.cpp - Crash Recovery -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/ProgramStack.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_ON_UNIX
# include <sys/resource.h> // for getrlimit
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
# include <intrin.h>  // for _AddressOfReturnAddress
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "llvm/Support/thread.h"

#ಅನ್ತರ್ಭಾವ <cstdlib>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

uintptr_t llvm::get_ರಾಶಿ_ಸೂಚಕ() {
#ಆದರೆ __GNUC__ || __has_builtin(__builtin_frame_address)
  ಹಿಂತಿರುಗಿ (uintptr_t)__builtin_frame_address(0);
#ಅಥವಾಆದರೆ defined(_MSC_VER)
  ಹಿಂತಿರುಗಿ (uintptr_t)ವಿಳಾಸ_ದ_Return_ವಿಳಾಸ();
#ಇಲ್ಲದಿದ್ದರೆ
  ಅಸ್ಥಿರ ಅಕ್ಷರ Char_On_ರಾಶಿ = 0;
  // The volatile store here is intended to escape the local variable, to
  // prevent the compiler from optimizing CharOnStack into anything other
  // than a char on the stack.
  //
  // Tested on: MSVC 2015 - 2019, GCC 4.9 - 9, Clang 3.2 - 9, ICC 13 - 19.
  ಅಕ್ಷರ *ಅಸ್ಥಿರ Ptr = &Char_On_ರಾಶಿ;
  ಹಿಂತಿರುಗಿ (uintptr_t)Ptr;
#ಆದರೆ_ಅಂತ್ಯ
}

ಚಿಹ್ನೆರಹಿತ llvm::get_Default_ರಾಶಿ_ಗಾತ್ರ() {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LLVM_ON_UNIX
  rlimit RL;
  getrlimit(RLIMIT_STACK, &RL);
  ಹಿಂತಿರುಗಿ RL.rlim_cur;
#ಇಲ್ಲದಿದ್ದರೆ
  // Clang recursively parses, instantiates templates, and evaluates constant
  // expressions. We've found 8MiB to be a reasonable stack size given the way
  // Clang works and the way C++ is commonly written.
  ಹಿಂತಿರುಗಿ 8 << 20;
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ llvm::ಸತತ_On_New_ರಾಶಿ(ಚಿಹ್ನೆರಹಿತ ರಾಶಿ_ಗಾತ್ರ, function_ref<ಶೂನ್ಯ()> Fn) {
  llvm::thread Thread(
      ರಾಶಿ_ಗಾತ್ರ == 0 ? std::nullopt : std::optional<ಚಿಹ್ನೆರಹಿತ>(ರಾಶಿ_ಗಾತ್ರ), Fn);
  Thread.join();
}
