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
استفاده نطاق llvm;

فراغ circular_raw_ostream::write_impl(ثابت حرف *Ptr, size_t اندازه) {
  إذا (میانگیر_اندازه == 0) {
    The_جریان->اكتب(Ptr, اندازه);
    أرجع;
  }

  // Write into the buffer, wrapping if necessary.
  طالما (اندازه != 0) {
    بیعلامت بایتها_2 =
      std::min(بیعلامت(اندازه), بیعلامت(میانگیر_اندازه - (Cur - میانگیر_Array)));
    کپی_حافظه(Cur, Ptr, بایتها_2);
    اندازه -= بایتها_2;
    Cur += بایتها_2;
    إذا (Cur == میانگیر_Array + میانگیر_اندازه) {
      // Reset the output pointer to the start of the buffer.
      Cur = میانگیر_Array;
      Filled = درست;
    }
  }
}

فراغ circular_raw_ostream::flush_میانگیر_With_Banner() {
  إذا (میانگیر_اندازه != 0) {
    // Write out the buffer
    The_جریان->اكتب(Banner, std::طول_رشته(Banner));
    flush_میانگیر();
  }
}
