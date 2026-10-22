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
using ನಾಮಾಕಾಶ llvm;

//===----------------------------------------------------------------------===//
//  raw_os_ostream
//===----------------------------------------------------------------------===//

raw_os_ostream::~raw_os_ostream() {
  flush();
}

ಶೂನ್ಯ raw_os_ostream::write_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) {
  OS.ಲಿಖ(Ptr, Size);
}

uint64_t raw_os_ostream::current_pos() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ OS.tellp(); }
