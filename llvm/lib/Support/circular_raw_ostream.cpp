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

#અન્તર્ભાવ "llvm/Support/circular_raw_ostream.h"
#અન્તર્ભાવ <algorithm>
using નામાકાશ llvm;

શૂન્ય circular_raw_ostream::write_impl(અચલ અક્ષર *Ptr, size_t Size) {
  જો (BufferSize == 0) {
    TheStream->લિખ(Ptr, Size);
    પાછા;
  }

  // Write into the buffer, wrapping if necessary.
  જ્યારે (Size != 0) {
    unsigned Bytes =
      std::min(unsigned(Size), unsigned(BufferSize - (Cur - BufferArray)));
    memcpy(Cur, Ptr, Bytes);
    Size -= Bytes;
    Cur += Bytes;
    જો (Cur == BufferArray + BufferSize) {
      // Reset the output pointer to the start of the buffer.
      Cur = BufferArray;
      Filled = સાચું;
    }
  }
}

શૂન્ય circular_raw_ostream::flushBufferWithBanner() {
  જો (BufferSize != 0) {
    // Write out the buffer
    TheStream->લિખ(Banner, std::strlen(Banner));
    flushBuffer();
  }
}
