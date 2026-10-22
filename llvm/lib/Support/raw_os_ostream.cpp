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

#অন্তর্ভাব "llvm/Support/raw_os_ostream.h"
#অন্তর্ভাব <ostream>
using নামাকাশ llvm;

//===----------------------------------------------------------------------===//
//  raw_os_ostream
//===----------------------------------------------------------------------===//

raw_os_ostream::~raw_os_ostream() {
  flush();
}

শূন্য raw_os_ostream::write_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2) {
  OS.লিখ(Ptr, আকার_2);
}

uint64_t raw_os_ostream::current_pos() ধ্রুবক { ফেরত OS.tellp(); }
