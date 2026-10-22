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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_os_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <ostream>
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

//===----------------------------------------------------------------------===//
//  raw_os_ostream
//===----------------------------------------------------------------------===//

raw_os_ostream::~raw_os_ostream() {
  flush();
}

ਖਾਲੀ raw_os_ostream::write_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) {
  OS.ਲਿਖ(Ptr, ਮਾਪ);
}

uint64_t raw_os_ostream::current_pos() ਅਚਲ { ਵਾਪਸ OS.tellp(); }
