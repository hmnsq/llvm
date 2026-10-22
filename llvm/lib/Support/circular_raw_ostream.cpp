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

#شامل "llvm/Support/circular_raw_ostream.h"
#شامل <algorithm>
استعمال نامخلا llvm;

خالی circular_raw_ostream::write_impl(ثابت حرف *Ptr, size_t سائز) {
  اگر (بفر_سائز == 0) {
    The_دھارا->اكتب(Ptr, سائز);
    واپس;
  }

  // Write into the buffer, wrapping if necessary.
  جب (سائز != 0) {
    غیرعلامتی بائٹیں_2 =
      std::min(غیرعلامتی(سائز), غیرعلامتی(بفر_سائز - (Cur - بفر_Array)));
    حافظہ_نقل(Cur, Ptr, بائٹیں_2);
    سائز -= بائٹیں_2;
    Cur += بائٹیں_2;
    اگر (Cur == بفر_Array + بفر_سائز) {
      // Reset the output pointer to the start of the buffer.
      Cur = بفر_Array;
      Filled = سچ;
    }
  }
}

خالی circular_raw_ostream::flush_بفر_With_Banner() {
  اگر (بفر_سائز != 0) {
    // Write out the buffer
    The_دھارا->اكتب(Banner, std::لڑی_لمبائی(Banner));
    flush_بفر();
  }
}
