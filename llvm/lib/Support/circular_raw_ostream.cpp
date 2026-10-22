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

#அந்தர்பாவ "llvm/Support/circular_raw_ostream.h"
#அந்தர்பாவ <algorithm>
using நாமாகாஶ llvm;

வெறுமை circular_raw_ostream::write_impl(மாறிலி எழுத்து *Ptr, size_t Size) {
  எனில் (BufferSize == 0) {
    TheStream->லிக(Ptr, Size);
    திரும்பு;
  }

  // Write into the buffer, wrapping if necessary.
  போது (Size != 0) {
    unsigned Bytes =
      std::min(unsigned(Size), unsigned(BufferSize - (Cur - BufferArray)));
    memcpy(Cur, Ptr, Bytes);
    Size -= Bytes;
    Cur += Bytes;
    எனில் (Cur == BufferArray + BufferSize) {
      // Reset the output pointer to the start of the buffer.
      Cur = BufferArray;
      Filled = உண்மை;
    }
  }
}

வெறுமை circular_raw_ostream::flushBufferWithBanner() {
  எனில் (BufferSize != 0) {
    // Write out the buffer
    TheStream->லிக(Banner, std::strlen(Banner));
    flushBuffer();
  }
}
