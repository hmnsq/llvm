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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_os_ostream.h"
#ଅନ୍ତର୍ଭାଵ <ostream>
using ନାମାକାଶ llvm;

//===----------------------------------------------------------------------===//
//  raw_os_ostream
//===----------------------------------------------------------------------===//

raw_os_ostream::~raw_os_ostream() {
  flush();
}

ଶୂନ୍ୟ raw_os_ostream::write_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) {
  OS.ଲିଖ(Ptr, Size);
}

uint64_t raw_os_ostream::current_pos() ଅଚଳ { ଫେରନ୍ତୁ OS.tellp(); }
