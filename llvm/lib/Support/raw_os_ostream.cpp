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

#अन्तर्भाव "llvm/Support/raw_os_ostream.h"
#अन्तर्भाव <ostream>
using नामाकाश llvm;

//===----------------------------------------------------------------------===//
//  raw_os_ostream
//===----------------------------------------------------------------------===//

raw_os_ostream::~raw_os_ostream() {
  flush();
}

शून्य raw_os_ostream::write_impl(स्थिरांक अक्षर *Ptr, माप_प्रकार Size) {
  OS.लिख(Ptr, Size);
}

uint64_t raw_os_ostream::current_pos() स्थिरांक { परत OS.tellp(); }
