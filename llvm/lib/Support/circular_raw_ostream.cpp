//===- circular_raw_ostream.cpp - Implement circular_raw_ostream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This implements support for circular buffered streams.
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/circular_raw_ostream.h"
#অন্তর্ভাব <algorithm>
using নামাকাশ llvm;

শূন্য circular_raw_ostream::write_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2) {
  যদি (বাফার_আকার == 0) {
    TheStream->লিখ(Ptr, আকার_2);
    ফেরত;
  }

  // Write into the buffer, wrapping if necessary.
  যখন (আকার_2 != 0) {
    unsigned Bytes =
      std::min(unsigned(আকার_2), unsigned(বাফার_আকার - (Cur - বাফার_Array)));
    memcpy(Cur, Ptr, Bytes);
    আকার_2 -= Bytes;
    Cur += Bytes;
    যদি (Cur == বাফার_Array + বাফার_আকার) {
      // Reset the output pointer to the start of the buffer.
      Cur = বাফার_Array;
      Filled = সত্য;
    }
  }
}

শূন্য circular_raw_ostream::flush_বাফার_With_Banner() {
  যদি (বাফার_আকার != 0) {
    // Write out the buffer
    TheStream->লিখ(Banner, std::strlen(Banner));
    flush_বাফার();
  }
}
