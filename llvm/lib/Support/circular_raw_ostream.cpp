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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/circular_raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <algorithm>
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਖਾਲੀ circular_raw_ostream::write_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) {
  ਜੇ (ਬਫਰ_ਮਾਪ == 0) {
    The_ਧਾਰਾ->ਲਿਖ(Ptr, ਮਾਪ);
    ਵਾਪਸ;
  }

  // Write into the buffer, wrapping if necessary.
  ਜਦੋਂ (ਮਾਪ != 0) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ_2 =
      std::min(ਬਿਨਾਨਿਸ਼ਾਨੀ(ਮਾਪ), ਬਿਨਾਨਿਸ਼ਾਨੀ(ਬਫਰ_ਮਾਪ - (Cur - ਬਫਰ_Array)));
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(Cur, Ptr, ਬਾਈਟਾਂ_2);
    ਮਾਪ -= ਬਾਈਟਾਂ_2;
    Cur += ਬਾਈਟਾਂ_2;
    ਜੇ (Cur == ਬਫਰ_Array + ਬਫਰ_ਮਾਪ) {
      // Reset the output pointer to the start of the buffer.
      Cur = ਬਫਰ_Array;
      Filled = ਸੱਚ;
    }
  }
}

ਖਾਲੀ circular_raw_ostream::flush_ਬਫਰ_With_Banner() {
  ਜੇ (ਬਫਰ_ਮਾਪ != 0) {
    // Write out the buffer
    The_ਧਾਰਾ->ਲਿਖ(Banner, std::ਲੜੀ_ਲੰਬਾਈ(Banner));
    flush_ਬਫਰ();
  }
}
