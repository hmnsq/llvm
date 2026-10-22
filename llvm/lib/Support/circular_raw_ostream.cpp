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
کارول نطاق llvm;

فراغ circular_raw_ostream::write_impl(ثابت حرف *Ptr, size_t اندازه) {
  إذا (بفر_اندازه == 0) {
    The_بهیر->اكتب(Ptr, اندازه);
    أرجع;
  }

  // Write into the buffer, wrapping if necessary.
  طالما (اندازه != 0) {
    بینښه بایټونه_2 =
      std::min(بینښه(اندازه), بینښه(بفر_اندازه - (Cur - بفر_Array)));
    حافظه_نقل(Cur, Ptr, بایټونه_2);
    اندازه -= بایټونه_2;
    Cur += بایټونه_2;
    إذا (Cur == بفر_Array + بفر_اندازه) {
      // Reset the output pointer to the start of the buffer.
      Cur = بفر_Array;
      Filled = ریښتیا;
    }
  }
}

فراغ circular_raw_ostream::flush_بفر_With_Banner() {
  إذا (بفر_اندازه != 0) {
    // Write out the buffer
    The_بهیر->اكتب(Banner, std::مزي_اوږدوالی(Banner));
    flush_بفر();
  }
}
