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

#ضمن "llvm/Support/circular_raw_ostream.h"
#ضمن <algorithm>
استعمال نطاق llvm;

فراغ circular_raw_ostream::write_impl(ثابت حرف *Ptr, size_t سائيز) {
  إذا (بفر_سائيز == 0) {
    The_وهڪرو->اكتب(Ptr, سائيز);
    أرجع;
  }

  // Write into the buffer, wrapping if necessary.
  طالما (سائيز != 0) {
    بنانشان بائيٽون_2 =
      std::min(بنانشان(سائيز), بنانشان(بفر_سائيز - (Cur - بفر_Array)));
    يادگيري_نقل(Cur, Ptr, بائيٽون_2);
    سائيز -= بائيٽون_2;
    Cur += بائيٽون_2;
    إذا (Cur == بفر_Array + بفر_سائيز) {
      // Reset the output pointer to the start of the buffer.
      Cur = بفر_Array;
      Filled = سچ;
    }
  }
}

فراغ circular_raw_ostream::flush_بفر_With_Banner() {
  إذا (بفر_سائيز != 0) {
    // Write out the buffer
    The_وهڪرو->اكتب(Banner, std::لڙي_ڊيگهه(Banner));
    flush_بفر();
  }
}
