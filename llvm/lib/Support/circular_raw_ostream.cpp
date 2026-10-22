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
ورتوں نطاق llvm;

فراغ circular_raw_ostream::write_impl(ثابت حرف *Ptr, size_t ناپ) {
  إذا (بفر_ناپ == 0) {
    The_دھارا->اكتب(Ptr, ناپ);
    أرجع;
  }

  // Write into the buffer, wrapping if necessary.
  طالما (ناپ != 0) {
    بنانشانی بائٹاں_2 =
      std::min(بنانشانی(ناپ), بنانشانی(بفر_ناپ - (Cur - بفر_Array)));
    حافظہ_نقل(Cur, Ptr, بائٹاں_2);
    ناپ -= بائٹاں_2;
    Cur += بائٹاں_2;
    إذا (Cur == بفر_Array + بفر_ناپ) {
      // Reset the output pointer to the start of the buffer.
      Cur = بفر_Array;
      Filled = سچ;
    }
  }
}

فراغ circular_raw_ostream::flush_بفر_With_Banner() {
  إذا (بفر_ناپ != 0) {
    // Write out the buffer
    The_دھارا->اكتب(Banner, std::لڑی_لمبائی(Banner));
    flush_بفر();
  }
}
