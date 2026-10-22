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

#समावेश "llvm/Support/raw_os_ostream.h"
#समावेश <ostream>
using namespace llvm;

//===----------------------------------------------------------------------===//
//  raw_os_ostream
//===----------------------------------------------------------------------===//

raw_os_ostream::~raw_os_ostream() {
  सफाई();
}

व्योम raw_os_ostream::write_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार) {
  OS.ग_लिखो(Ptr, आकार);
}

uint64_t raw_os_ostream::current_pos() स्थिर { वापस OS.tellp(); }
