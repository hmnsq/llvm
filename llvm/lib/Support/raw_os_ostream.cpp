//===--- raw_os_ostream.cpp - Implement the raw_os_ostream class ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This implements support adapting raw_ostream to std::ostream.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_os_ostream.h"
#ಅನ್ತರ್ಭಾವ <ostream>
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

//===----------------------------------------------------------------------===//
//  raw_os_ostream
//===----------------------------------------------------------------------===//

raw_os_ostream::~raw_os_ostream() {
  flush();
}

ಶೂನ್ಯ raw_os_ostream::write_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) {
  OS.ಲಿಖ(Ptr, ಗಾತ್ರ_2);
}

uint64_t raw_os_ostream::current_pos() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ OS.tellp(); }
