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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/circular_raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <algorithm>
using ନାମାକାଶ llvm;

ଶୂନ୍ୟ circular_raw_ostream::write_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) {
  ଯଦି (BufferSize == 0) {
    TheStream->ଲିଖ(Ptr, Size);
    ଫେରନ୍ତୁ;
  }

  // Write into the buffer, wrapping if necessary.
  ଯେତେବେଳେ (Size != 0) {
    unsigned Bytes =
      std::min(unsigned(Size), unsigned(BufferSize - (Cur - BufferArray)));
    memcpy(Cur, Ptr, Bytes);
    Size -= Bytes;
    Cur += Bytes;
    ଯଦି (Cur == BufferArray + BufferSize) {
      // Reset the output pointer to the start of the buffer.
      Cur = BufferArray;
      Filled = ସତ୍ୟ;
    }
  }
}

ଶୂନ୍ୟ circular_raw_ostream::flushBufferWithBanner() {
  ଯଦି (BufferSize != 0) {
    // Write out the buffer
    TheStream->ଲିଖ(Banner, std::strlen(Banner));
    flushBuffer();
  }
}
