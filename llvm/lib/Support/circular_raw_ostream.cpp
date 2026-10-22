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

#অন্তর্ভাৱ "llvm/Support/circular_raw_ostream.h"
#অন্তর্ভাৱ <algorithm>
using নামাকাশ llvm;

শূন্য circular_raw_ostream::write_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size) {
  যদি (BufferSize == 0) {
    TheStream->লিখ(Ptr, Size);
    ঘূৰাই;
  }

  // Write into the buffer, wrapping if necessary.
  যেতিয়ালৈ (Size != 0) {
    unsigned Bytes =
      std::min(unsigned(Size), unsigned(BufferSize - (Cur - BufferArray)));
    memcpy(Cur, Ptr, Bytes);
    Size -= Bytes;
    Cur += Bytes;
    যদি (Cur == BufferArray + BufferSize) {
      // Reset the output pointer to the start of the buffer.
      Cur = BufferArray;
      Filled = সঁচা;
    }
  }
}

শূন্য circular_raw_ostream::flushBufferWithBanner() {
  যদি (BufferSize != 0) {
    // Write out the buffer
    TheStream->লিখ(Banner, std::strlen(Banner));
    flushBuffer();
  }
}
