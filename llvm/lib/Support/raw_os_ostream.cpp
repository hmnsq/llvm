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

#அந்தர்பாவ "llvm/Support/raw_os_ostream.h"
#அந்தர்பாவ <ostream>
using நாமாகாஶ llvm;

//===----------------------------------------------------------------------===//
//  raw_os_ostream
//===----------------------------------------------------------------------===//

raw_os_ostream::~raw_os_ostream() {
  flush();
}

வெறுமை raw_os_ostream::write_impl(மாறிலி எழுத்து *Ptr, size_t Size) {
  OS.லிக(Ptr, Size);
}

uint64_t raw_os_ostream::current_pos() மாறிலி { திரும்பு OS.tellp(); }
